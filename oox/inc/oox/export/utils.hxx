/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _OOX_EXPORT_UTILS_HXX_
#define _OOX_EXPORT_UTILS_HXX_

#define S(x) String( RTL_CONSTASCII_USTRINGPARAM( x ) )
#define US(x) OUString( RTL_CONSTASCII_USTRINGPARAM( x ))
#define I32S(x) OString::valueOf( (sal_Int32) x ).getStr()
#define I64S(x) OString::valueOf( (sal_Int64) x ).getStr()
#define H32S(x) OString::valueOf( (sal_Int32) x, 16 ).getStr()
#define H64S(x) OString::valueOf( (sal_Int64) x, 16 ).getStr()
#define IS(x) OString::valueOf( x ).getStr()
#define USS(x) OUStringToOString( x, RTL_TEXTENCODING_UTF8 ).getStr()
#define ST(x) ByteString( x, RTL_TEXTENCODING_UTF8 ).GetBuffer()

#ifndef DBG
#  if OSL_DEBUG_LEVEL > 0
#    define DBG(x) x
#  else
#    define DBG(x)
#  endif
#endif

// ---------------------------------------------------------------------------------------------

static inline sal_Int64 PPTtoEMU( INT32 nPPT )
{
    return (sal_Int64)( (double)nPPT * 1587.5 );
}

// ---------------------------------------------------------------------------------------------

static inline sal_Int64 MM100toEMU( INT32 nMM100 )
{
    return (sal_Int64)nMM100 * 360;
}

// ---------------------------------------------------------------------------------------------

static inline sal_Int64 TwipsToEMU( sal_Int32 nTwips )
{
    return sal_Int64( nTwips ) * 635;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
