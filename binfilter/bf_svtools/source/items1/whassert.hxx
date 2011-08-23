/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFX_WHASSERT_HXX
#define _SFX_WHASSERT_HXX

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

namespace binfilter
{

//------------------------------------------------------------------------

#ifdef DBG_UTIL
#define SFX_ASSERT( bCondition, nId, sMessage ) 							\
{																			\
    if ( DbgIsAssert() )													\
    {																		\
        if ( !(bCondition) )												\
        {																	\
            ByteString aMsg( sMessage );									\
            aMsg.Append(RTL_CONSTASCII_STRINGPARAM("\nwith Id/Pos: "));     \
            aMsg += ByteString::CreateFromInt32( nId );						\
            DbgOut( aMsg.GetBuffer(), DBG_OUT_ERROR, __FILE__, __LINE__);	\
        }																	\
    }																		\
}
#else
#define SFX_ASSERT( bCondition, nId, sMessage )
#endif

}

#endif
