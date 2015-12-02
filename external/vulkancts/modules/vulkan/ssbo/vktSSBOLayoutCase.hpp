#ifndef _VKTSSBOLAYOUTCASE_HPP
#define _VKTSSBOLAYOUTCASE_HPP
/*------------------------------------------------------------------------
 * Vulkan Conformance Tests
 * ------------------------
 *
 * Copyright (c) 2015 The Khronos Group Inc.
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * The Materials are Confidential Information as defined by the
 * Khronos Membership Agreement until designated non-confidential by Khronos,
 * at which point this condition clause shall be removed.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 *//*!
 * \file
 * \brief SSBO layout tests.
 *//*--------------------------------------------------------------------*/

#include "tcuDefs.hpp"
#include "tcuTestCase.hpp"
#include "gluShaderUtil.hpp"
#include "gluVarType.hpp"

namespace glu
{
class RenderContext;
}

namespace deqp
{
namespace gles31
{

// Buffer block details.
namespace bb
{

enum BufferVarFlags
{
	LAYOUT_SHARED		= (1<<0),
	LAYOUT_PACKED		= (1<<1),
	LAYOUT_STD140		= (1<<2),
	LAYOUT_STD430		= (1<<3),
	LAYOUT_ROW_MAJOR	= (1<<4),
	LAYOUT_COLUMN_MAJOR	= (1<<5),	//!< \note Lack of both flags means column-major matrix.
	LAYOUT_MASK			= LAYOUT_SHARED|LAYOUT_PACKED|LAYOUT_STD140|LAYOUT_STD430|LAYOUT_ROW_MAJOR|LAYOUT_COLUMN_MAJOR,

	// \todo [2013-10-14 pyry] Investigate adding these.
/*	QUALIFIER_COHERENT	= (1<<6),
	QUALIFIER_VOLATILE	= (1<<7),
	QUALIFIER_RESTRICT	= (1<<8),
	QUALIFIER_READONLY	= (1<<9),
	QUALIFIER_WRITEONLY	= (1<<10),*/

	ACCESS_READ			= (1<<11),	//!< Buffer variable is read in the shader.
	ACCESS_WRITE		= (1<<12),	//!< Buffer variable is written in the shader.
};

class BufferVar
{
public:
						BufferVar		(const char* name, const glu::VarType& type, deUint32 flags);

	const char*			getName			(void) const { return m_name.c_str();	}
	const glu::VarType&	getType			(void) const { return m_type;			}
	deUint32			getFlags		(void) const { return m_flags;			}

private:
	std::string			m_name;
	glu::VarType		m_type;
	deUint32			m_flags;
};

class BufferBlock
{
public:
	typedef std::vector<BufferVar>::iterator		iterator;
	typedef std::vector<BufferVar>::const_iterator	const_iterator;

							BufferBlock				(const char* blockName);

	const char*				getBlockName			(void) const { return m_blockName.c_str();		}
	const char*				getInstanceName			(void) const { return m_instanceName.empty() ? DE_NULL : m_instanceName.c_str();	}
	bool					isArray					(void) const { return m_arraySize > 0;			}
	int						getArraySize			(void) const { return m_arraySize;				}
	deUint32				getFlags				(void) const { return m_flags;					}

	void					setInstanceName			(const char* name)			{ m_instanceName = name;			}
	void					setFlags				(deUint32 flags)			{ m_flags = flags;					}
	void					addMember				(const BufferVar& var)		{ m_variables.push_back(var);		}
	void					setArraySize			(int arraySize);

	int						getLastUnsizedArraySize	(int instanceNdx) const		{ return m_lastUnsizedArraySizes[instanceNdx];	}
	void					setLastUnsizedArraySize	(int instanceNdx, int size)	{ m_lastUnsizedArraySizes[instanceNdx] = size;	}

	inline iterator			begin					(void)			{ return m_variables.begin();	}
	inline const_iterator	begin					(void) const	{ return m_variables.begin();	}
	inline iterator			end						(void)			{ return m_variables.end();		}
	inline const_iterator	end						(void) const	{ return m_variables.end();		}

private:
	std::string				m_blockName;
	std::string				m_instanceName;
	std::vector<BufferVar>	m_variables;
	int						m_arraySize;				//!< Array size or 0 if not interface block array.
	std::vector<int>		m_lastUnsizedArraySizes;	//!< Sizes of last unsized array element, can be different per instance.
	deUint32				m_flags;
};

class ShaderInterface
{
public:
									ShaderInterface			(void);
									~ShaderInterface		(void);

	glu::StructType&				allocStruct				(const char* name);
	const glu::StructType*			findStruct				(const char* name) const;
	void							getNamedStructs			(std::vector<const glu::StructType*>& structs) const;

	BufferBlock&					allocBlock				(const char* name);

	int								getNumBlocks			(void) const	{ return (int)m_bufferBlocks.size();	}
	const BufferBlock&				getBlock				(int ndx) const	{ return *m_bufferBlocks[ndx];			}

private:
									ShaderInterface			(const ShaderInterface&);
	ShaderInterface&				operator=				(const ShaderInterface&);

	std::vector<glu::StructType*>	m_structs;
	std::vector<BufferBlock*>		m_bufferBlocks;
};

class BufferLayout;

} // bb

class SSBOLayoutCase : public tcu::TestCase
{
public:
	enum BufferMode
	{
		BUFFERMODE_SINGLE = 0,	//!< Single buffer shared between uniform blocks.
		BUFFERMODE_PER_BLOCK,	//!< Per-block buffers

		BUFFERMODE_LAST
	};

								SSBOLayoutCase				(tcu::TestContext& testCtx, glu::RenderContext& renderCtx, const char* name, const char* description, glu::GLSLVersion glslVersion, BufferMode bufferMode);
								~SSBOLayoutCase				(void);

	IterateResult				iterate						(void);

protected:
	bool						compareStdBlocks			(const bb::BufferLayout& refLayout, const bb::BufferLayout& cmpLayout) const;
	bool						compareSharedBlocks			(const bb::BufferLayout& refLayout, const bb::BufferLayout& cmpLayout) const;
	bool						compareTypes				(const bb::BufferLayout& refLayout, const bb::BufferLayout& cmpLayout) const;
	bool						checkLayoutIndices			(const bb::BufferLayout& layout) const;
	bool						checkLayoutBounds			(const bb::BufferLayout& layout) const;
	bool						checkIndexQueries			(deUint32 program, const bb::BufferLayout& layout) const;

	bool						execute						(deUint32 program);

	glu::RenderContext&			m_renderCtx;
	glu::GLSLVersion			m_glslVersion;
	BufferMode					m_bufferMode;
	bb::ShaderInterface			m_interface;

private:
								SSBOLayoutCase				(const SSBOLayoutCase&);
	SSBOLayoutCase&				operator=					(const SSBOLayoutCase&);
};

} // gles31
} // deqp

#endif // _VKTSSBOLAYOUTCASE_HPP
