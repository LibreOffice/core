/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tenccvt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:04:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _TOOLS_TENCCVT_HXX
#include <tenccvt.hxx>
#endif

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

rtl_TextEncoding GetSOLoadTextEncoding( rtl_TextEncoding eEncoding, USHORT /* nVersion = SOFFICE_FILEFORMAT_50 */ )
{
    return GetExtendedCompatibilityTextEncoding( GetOneByteTextEncoding( eEncoding ) );
}

// -----------------------------------------------------------------------

rtl_TextEncoding GetSOStoreTextEncoding( rtl_TextEncoding eEncoding, USHORT /* nVersion = SOFFICE_FILEFORMAT_50 */ )
{
    return GetExtendedTextEncoding( GetOneByteTextEncoding( eEncoding ) );
}
