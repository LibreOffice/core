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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"
#include <rtl/tencinfo.h>
#include <tools/tenccvt.hxx>

// =======================================================================

rtl_TextEncoding GetExtendedCompatibilityTextEncoding( rtl_TextEncoding eEncoding )
{
    // Latin1
    if ( eEncoding == RTL_TEXTENCODING_ISO_8859_1 )
        return RTL_TEXTENCODING_MS_1252;
    // Turkey
    else if ( eEncoding == RTL_TEXTENCODING_ISO_8859_9 )
        return RTL_TEXTENCODING_MS_1254;
    else
        return eEncoding;
}

// -----------------------------------------------------------------------

rtl_TextEncoding GetExtendedTextEncoding( rtl_TextEncoding eEncoding )
{
    // Cyr
    if ( eEncoding == RTL_TEXTENCODING_ISO_8859_5 )
        return RTL_TEXTENCODING_MS_1251;
    // Greek (2 Characters different: A1 (0x2018/0x0385), A2 (0x2019/0x0386) -
    // so it is handled in this function and not in GetExtendedCompatibilityTextEncoding)
    else if ( eEncoding == RTL_TEXTENCODING_ISO_8859_7 )
        return RTL_TEXTENCODING_MS_1253;
    // East-Europe - Latin2
    else if ( eEncoding == RTL_TEXTENCODING_ISO_8859_2 )
        return RTL_TEXTENCODING_MS_1250;
    // Latin-15 - Latin 1 mit Euro-Sign
    else if ( eEncoding == RTL_TEXTENCODING_ISO_8859_15 )
        return RTL_TEXTENCODING_MS_1252;
    else
        return GetExtendedCompatibilityTextEncoding( eEncoding );
}

// -----------------------------------------------------------------------

rtl_TextEncoding GetOneByteTextEncoding( rtl_TextEncoding eEncoding )
{
    rtl_TextEncodingInfo aTextEncInfo;
    aTextEncInfo.StructSize = sizeof( aTextEncInfo );
    if ( rtl_getTextEncodingInfo( eEncoding, &aTextEncInfo ) )
    {
        if ( aTextEncInfo.MaximumCharSize > 1 )
            return RTL_TEXTENCODING_MS_1252;
        else
            return eEncoding;
    }
    else
        return RTL_TEXTENCODING_MS_1252;
}

// -----------------------------------------------------------------------

rtl_TextEncoding GetSOLoadTextEncoding( rtl_TextEncoding eEncoding, sal_uInt16 /* nVersion = SOFFICE_FILEFORMAT_50 */ )
{
    return GetExtendedCompatibilityTextEncoding( GetOneByteTextEncoding( eEncoding ) );
}

// -----------------------------------------------------------------------

rtl_TextEncoding GetSOStoreTextEncoding( rtl_TextEncoding eEncoding, sal_uInt16 /* nVersion = SOFFICE_FILEFORMAT_50 */ )
{
    return GetExtendedTextEncoding( GetOneByteTextEncoding( eEncoding ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
