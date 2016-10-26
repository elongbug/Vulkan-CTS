/*-------------------------------------------------------------------------
 * drawElements Quality Program Tester Core
 * ----------------------------------------
 *
 * Copyright 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *//*!
 * \file
 * \brief Wayland Vulkan platform
 *//*--------------------------------------------------------------------*/

#include "tcuWaylandVulkanPlatform.hpp"
#include "tcuWayland.hpp"
#include "tcuFormatUtil.hpp"
#include "tcuFunctionLibrary.hpp"
#include "tcuVector.hpp"

#include "vkWsiPlatform.hpp"

#include "deUniquePtr.hpp"
#include "deMemory.h"
#include <sys/utsname.h>

namespace tcu
{
namespace wayland
{

using de::MovePtr;
using de::UniquePtr;


class VulkanWindow : public wayland::Window, public vk::wsi::WaylandWindowInterface
{
public:
	VulkanWindow (wayland::Display& vdisplay, int width, int height)
		: wayland::Window (vdisplay, width, height)
		, vk::wsi::WaylandWindowInterface	(vk::pt::WaylandSurfacePtr(m_surface))
	{
	}

	void resize (const UVec2& newSize)
	{
		setDimensions((int)newSize.x(), (int)newSize.y());
		vk::wsi::WaylandWindowInterface::resize(newSize);
	}
};

class VulkanDisplay : public wayland::Display, public vk::wsi::WaylandDisplayInterface
{
public:
	VulkanDisplay ()
		: wayland::Display (m_dummyeventstate,NULL)
		, vk::wsi::WaylandDisplayInterface	(vk::pt::WaylandDisplayPtr(m_display))
	{
	}

	vk::wsi::Window* createWindow (const Maybe<UVec2>& initialSize) const
	{
		const deUint32	width		= !initialSize ? 400 : initialSize->x();
		const deUint32	height		= !initialSize ? 300 : initialSize->y();

		return new VulkanWindow(dynamic_cast<wayland::Display&>(*const_cast<VulkanDisplay*>(this)), width, height);
	}

private:
	EventState m_dummyeventstate;
};

class VulkanLibrary : public vk::Library
{
public:
	VulkanLibrary (void)
		: m_library	("libvulkan.so.1")
		, m_driver	(m_library)
	{
	}

	const vk::PlatformInterface& getPlatformInterface (void) const
	{
		return m_driver;
	}

private:
	const tcu::DynamicFunctionLibrary	m_library;
	const vk::PlatformDriver			m_driver;
};


vk::Library* VulkanPlatform::createLibrary (void) const
{
	return new VulkanLibrary();
}

void VulkanPlatform::describePlatform (std::ostream& dst) const
{
	utsname		sysInfo;

	deMemset(&sysInfo, 0, sizeof(sysInfo));

	if (uname(&sysInfo) != 0)
		throw std::runtime_error("uname() failed");

	dst << "OS: " << sysInfo.sysname << " " << sysInfo.release << " " << sysInfo.version << "\n";
	dst << "CPU: " << sysInfo.machine << "\n";
}

void VulkanPlatform::getMemoryLimits (vk::PlatformMemoryLimits& limits) const
{
	limits.totalSystemMemory					= 256*1024*1024;
	limits.totalDeviceLocalMemory				= 128*1024*1024;
	limits.deviceMemoryAllocationGranularity	= 64*1024;
	limits.devicePageSize						= 4096;
	limits.devicePageTableEntrySize				= 8;
	limits.devicePageTableHierarchyLevels		= 3;
}


VulkanPlatform::VulkanPlatform ()
{
}

VulkanPlatform::~VulkanPlatform (void)
{
}

vk::wsi::Display* VulkanPlatform::createWsiDisplay (vk::wsi::Type wsiType) const
{
	if (wsiType != vk::wsi::TYPE_WAYLAND)
		TCU_THROW(NotSupportedError, "WSI type not supported");

	return new VulkanDisplay();
}

} // wayland
} // tcu
