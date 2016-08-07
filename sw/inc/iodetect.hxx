/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _IODETECT_HXX_
#define _IODETECT_HXX_

#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <osl/endian.h>
#include <errhdl.hxx>       // for ASSERT
#include <tools/string.hxx>
#include <swddllapi.h>

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

extern SWD_DLLPUBLIC SwIoDetect aFilterDetect[];

// Die folgende Klasse ist ein Wrappe fuer die Basic-I/O-Funktionen
// des Writer 3.0. Alles ist statisch. Alle u.a. Filternamen sind die
// Writer-internen Namen, d.h. die namen, die in INSTALL.INI vor dem
// Gleichheitszeichen stehen, z.b. SWG oder ASCII.

class SwIoSystem
{
public:
    // suche ueber den internen FormatNamen den Filtereintrag
    SWD_DLLPUBLIC static const SfxFilter*
        GetFilterOfFormat( const String& rFormat,
            const SfxFilterContainer* pCnt = 0 );

    // Feststellen des zu verwendenden Filters fuer die uebergebene
    // Datei. Der Filtername wird zurueckgeliefert. Konnte kein Filter
    // zurueckgeliefert werden, wird der Name des ASCII-Filters geliefert!
    SWD_DLLPUBLIC static const SfxFilter*
        GetFileFilter( const String& rFileName,
            const String& rPrefFltName,
            SfxMedium* pMedium = 0 );

    // Feststellen ob das File in dem vorgegebenen Format vorliegt.
    // Z.z werden nur unsere eigene Filter unterstuetzt!!
    static sal_Bool IsFileFilter( SfxMedium& rMedium, const String& rFmtName,
            const SfxFilter** ppFlt = 0 );

    static sal_Bool IsValidStgFilter( SotStorage& , const SfxFilter& );
    static sal_Bool IsValidStgFilter( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rStg, const SfxFilter& rFilter);

    SWD_DLLPUBLIC static bool
        IsDetectableText( const sal_Char* pBuf, sal_uLong &rLen,
            CharSet *pCharSet=0, bool *pSwap=0, LineEnd *pLineEnd=0, bool bEncodedFilter = false );
    //    static bool IsDetectableW4W(const String& rFileName, const String& rUserData);

    static const SfxFilter* GetTextFilter( const sal_Char* pBuf, sal_uLong nLen );

    SWD_DLLPUBLIC static const String
        GetSubStorageName( const SfxFilter& rFltr );
};

#endif
