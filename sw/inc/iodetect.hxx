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

#ifndef _IODETECT_HXX_
#define _IODETECT_HXX_

#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <osl/endian.h>
#include <errhdl.hxx>       // for ASSERT
#include <tools/string.hxx>
#include <swdllapi.h>

#define FILTER_RTF      "RTF"       // RTF-Filter
#define sRtfWH          "WH_RTF"
#define FILTER_TEXT     "TEXT"      // Text-Filter mit Default-CodeSet
#define FILTER_BAS      "BAS"       // StarBasic (identisch mit ANSI)
#define FILTER_WW8      "CWW8"      // WinWord 97-Filter
#define FILTER_TEXT_DLG "TEXT_DLG"  // text filter with encoding dialog
#define FILTER_XML      "CXML"      // XML filter
#define FILTER_XMLV     "CXMLV"     // XML filter
#define FILTER_XMLVW    "CXMLVWEB"  // XML filter
#define sHTML           "HTML"
#define sWW1            "WW1"
#define sWW5            "WW6"
#define sWW6            "CWW6"

#define sSWRITER        "swriter"
#define sSWRITERWEB     "swriter/web"

struct SwIoDetect
{
    const sal_Char* pName;
    sal_uInt16 nLen;

    inline SwIoDetect( const sal_Char *pN, sal_uInt16 nL )
        : pName( pN ), nLen( nL )
    {}

    inline int IsFilter( const String& rNm ) const
    {
        return pName && rNm.EqualsAscii( pName, 0, nLen );
    }

    const sal_Char* IsReader( const sal_Char* pHeader, sal_uLong nLen_,
            const String &rFileName, const String& rUserData ) const;
};

enum ReaderWriterEnum {
    READER_WRITER_RTF,
    READER_WRITER_BAS,
    READER_WRITER_WW6,
    READER_WRITER_WW8,
    READER_WRITER_RTF_WH,
    READER_WRITER_HTML,
    READER_WRITER_WW1,
    READER_WRITER_WW5,
    READER_WRITER_XML,
    READER_WRITER_TEXT_DLG,
    READER_WRITER_TEXT,
    MAXFILTER
};

extern SwIoDetect aFilterDetect[];

// Die folgende Klasse ist ein Wrappe fuer die Basic-I/O-Funktionen
// des Writer 3.0. Alles ist statisch. Alle u.a. Filternamen sind die
// Writer-internen Namen, d.h. die namen, die in INSTALL.INI vor dem
// Gleichheitszeichen stehen, z.b. SWG oder ASCII.

class SwIoSystem
{
public:
    // suche ueber den internen FormatNamen den Filtereintrag
    SW_DLLPUBLIC static const SfxFilter* GetFilterOfFormat( const String& rFormat,
            const SfxFilterContainer* pCnt = 0 );

    // Feststellen des zu verwendenden Filters fuer die uebergebene
    // Datei. Der Filtername wird zurueckgeliefert. Konnte kein Filter
    // zurueckgeliefert werden, wird der Name des ASCII-Filters geliefert!
    static const SfxFilter* GetFileFilter( const String& rFileName,
            const String& rPrefFltName,
            SfxMedium* pMedium = 0 );

    // Feststellen ob das File in dem vorgegebenen Format vorliegt.
    // Z.z werden nur unsere eigene Filter unterstuetzt!!
    static sal_Bool IsFileFilter( SfxMedium& rMedium, const String& rFmtName,
            const SfxFilter** ppFlt = 0 );

    static sal_Bool IsValidStgFilter( SotStorage& , const SfxFilter& );
    static sal_Bool IsValidStgFilter( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStg, const SfxFilter& rFilter);

    static bool IsDetectableText( const sal_Char* pBuf, sal_uLong &rLen,
            CharSet *pCharSet=0, bool *pSwap=0, LineEnd *pLineEnd=0, bool bEncodedFilter = false );
    //    static bool IsDetectableW4W(const String& rFileName, const String& rUserData);

    static const SfxFilter* GetTextFilter( const sal_Char* pBuf, sal_uLong nLen );

    static const String GetSubStorageName( const SfxFilter& rFltr );
};

#endif
