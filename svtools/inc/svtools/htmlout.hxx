/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlout.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:21:25 $
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

#ifndef _HTMLOUT_HXX
#define _HTMLOUT_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif

class Color;
class ImageMap;
class String;
class SvStream;
class SvxMacroTableDtor;
class SvNumberFormatter;
class SvDataObject;

struct HTMLOutEvent
{
    const sal_Char *pBasicName;
    const sal_Char *pJavaName;
    USHORT nEvent;
};

struct SVT_DLLPUBLIC HTMLOutContext
{
    rtl_TextEncoding m_eDestEnc;
    rtl_TextToUnicodeConverter m_hConv;
    rtl_TextToUnicodeContext   m_hContext;

    HTMLOutContext( rtl_TextEncoding eDestEnc );
    ~HTMLOutContext();
};

struct HTMLOutFuncs
{
#if defined(MAC) || defined(UNX)
    static const sal_Char sNewLine;     // nur \012 oder \015
#else
    static const sal_Char __FAR_DATA sNewLine[];    // \015\012
#endif

    SVT_DLLPUBLIC static void ConvertStringToHTML( const String& sSrc, ByteString& rDest,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        String *pNonConvertableChars = 0);

    SVT_DLLPUBLIC static SvStream& Out_AsciiTag( SvStream&, const sal_Char* pStr,
                                   BOOL bOn = TRUE,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252);
    SVT_DLLPUBLIC static SvStream& Out_Char( SvStream&, sal_Unicode cChar,
                        HTMLOutContext& rContext,
                        String *pNonConvertableChars = 0 );
    SVT_DLLPUBLIC static SvStream& Out_String( SvStream&, const String&,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        String *pNonConvertableChars = 0 );
    SVT_DLLPUBLIC static SvStream& Out_Hex( SvStream&, ULONG nHex, BYTE nLen,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252 );
    SVT_DLLPUBLIC static SvStream& Out_Color( SvStream&, const Color&,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252 );
    SVT_DLLPUBLIC static SvStream& Out_ImageMap( SvStream&, const String&, const ImageMap&, const String&,
                                   const HTMLOutEvent *pEventTable,
                                   BOOL bOutStarBasic,
                                   const sal_Char *pDelim = 0,
                                   const sal_Char *pIndentArea = 0,
                                   const sal_Char *pIndentMap = 0,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        String *pNonConvertableChars = 0 );
    SVT_DLLPUBLIC static SvStream& FlushToAscii( SvStream&, HTMLOutContext& rContext );

    SVT_DLLPUBLIC static SvStream& OutScript( SvStream& rStrm,
                                const String& rBaseURL,
                                const String& rSource,
                                const String& rLanguage,
                                ScriptType eScriptType,
                                const String& rSrc,
                                const String *pSBLibrary = 0,
                                const String *pSBModule = 0,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        String *pNonConvertableChars = 0 );

    // der 3. Parameter ist ein Array von HTMLOutEvents, das mit einem
    // nur aus 0 bestehen Eintrag terminiert ist.
    SVT_DLLPUBLIC static SvStream& Out_Events( SvStream&, const SvxMacroTableDtor&,
                                 const HTMLOutEvent*, BOOL bOutStarBasic,
                        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                        String *pNonConvertableChars = 0 );

    // <TD SDVAL="..." SDNUM="...">
    SVT_DLLPUBLIC static ByteString&    CreateTableDataOptionsValNum( ByteString& aStrTD,
                BOOL bValue, double fVal, ULONG nFormat,
                SvNumberFormatter& rFormatter,
                rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
                String *pNonConvertableChars = 0 );

    SVT_DLLPUBLIC static BOOL PrivateURLToInternalImg( String& rURL );
};

#endif

