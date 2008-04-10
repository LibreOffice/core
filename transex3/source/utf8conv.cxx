/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: utf8conv.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_transex3.hxx"
#include "utf8conv.hxx"

//
// class UTF8Converter
//

#define MAX_CONV_BUFFER_SIZE    0xFF00

#define TO_CVTFLAGS     (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |\
                          RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |\
                          RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT)

#define FROM_CVTFLAGS   (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |\
                             RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT |\
                            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE |\
                            RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0)

/*****************************************************************************/
void UTF8Converter::Convert( ByteString &rBuffer,
    rtl_TextEncoding nSourceENC, rtl_TextEncoding nDestENC )
/*****************************************************************************/
{
    String sTemp( rBuffer, nSourceENC );
    rBuffer = ByteString( sTemp, nDestENC );
}

/*****************************************************************************/
ByteString UTF8Converter::ConvertToUTF8(
    const ByteString &rASCII, rtl_TextEncoding nEncoding )
/*****************************************************************************/
{
    ByteString sReturn( rASCII );
    Convert( sReturn, nEncoding, RTL_TEXTENCODING_UTF8 );
    return sReturn;
}

/*****************************************************************************/
ByteString UTF8Converter::ConvertFromUTF8(
    const ByteString &rUTF8, rtl_TextEncoding nEncoding )
/*****************************************************************************/
{
    ByteString sReturn( rUTF8 );
    Convert( sReturn, RTL_TEXTENCODING_UTF8, nEncoding );
    return sReturn;
}
