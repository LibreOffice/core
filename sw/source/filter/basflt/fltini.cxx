/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <string.h>
#include <stdio.h>                      // sscanf
#include <hintids.hxx>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/parhtml.hxx>
#include <sot/storage.hxx>
#include <comphelper/classids.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <shellio.hxx>
#include <docsh.hxx>
#include <wdocsh.hxx>
#include <fltini.hxx>
#include <hints.hxx>
#include <init.hxx>
#include <frmatr.hxx>
#include <fmtfsize.hxx>
#include <swtable.hxx>
#include <fmtcntnt.hxx>
#include <editeng/boxitem.hxx>
#include <frmfmt.hxx>
#include <numrule.hxx>
#include <ndtxt.hxx>
#include <swfltopt.hxx>
#include <swerror.h>
#include <osl/module.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <rtl/uri.hxx>

using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star;

SwRead ReadAscii = 0, ReadHTML = 0, ReadXML = 0;

Reader* GetRTFReader();
Reader* GetWW8Reader();

// Note: if editing, please don't forget to modify also the enum
// ReaderWriterEnum and aFilterDetect in shellio.hxx
SwReaderWriterEntry aReaderWriter[] =
{
    SwReaderWriterEntry( &::GetRTFReader, &::GetRTFWriter,  sal_True  ),
    SwReaderWriterEntry( 0,               &::GetASCWriter,  sal_False ),
    SwReaderWriterEntry( &::GetWW8Reader, &::GetWW8Writer,  sal_True  ),
    SwReaderWriterEntry( &::GetWW8Reader, &::GetWW8Writer,  sal_True  ),
    SwReaderWriterEntry( &::GetRTFReader, &::GetRTFWriter,  sal_True  ),
    SwReaderWriterEntry( 0,               &::GetHTMLWriter, sal_True  ),
    SwReaderWriterEntry( 0,               0,                sal_True  ),
    SwReaderWriterEntry( &::GetWW8Reader, 0,                sal_True  ),
    SwReaderWriterEntry( 0,               &::GetXMLWriter,  sal_True  ),
    SwReaderWriterEntry( 0,               &::GetASCWriter,  sal_False ),
    SwReaderWriterEntry( 0,               &::GetASCWriter,  sal_True  )
};

Reader* SwReaderWriterEntry::GetReader()
{
    if ( pReader )
        return pReader;
    else if ( fnGetReader )
    {
        pReader = (*fnGetReader)();
        return pReader;
    }
    return NULL;
}

void SwReaderWriterEntry::GetWriter( const OUString& rNm, const OUString& rBaseURL, WriterRef& xWrt ) const
{
    if ( fnGetWriter )
        (*fnGetWriter)( rNm, rBaseURL, xWrt );
    else
        xWrt = WriterRef(0);
}

SwRead SwGetReaderXML() // SW_DLLPUBLIC
{
        return ReadXML;
}

inline void _SetFltPtr( sal_uInt16 rPos, SwRead pReader )
{
        aReaderWriter[ rPos ].pReader = pReader;
}

namespace {

#ifndef DISABLE_DYNLOADING

extern "C" { static void SAL_CALL thisModule() {} }

#endif

}

namespace sw {

Filters::Filters()
{
    _SetFltPtr( READER_WRITER_BAS, (ReadAscii = new AsciiReader) );
    _SetFltPtr( READER_WRITER_HTML, (ReadHTML = new HTMLReader) );
    _SetFltPtr( READER_WRITER_WW1, new WW1Reader );
    _SetFltPtr( READER_WRITER_XML, (ReadXML = new XMLReader)  );
    _SetFltPtr( READER_WRITER_TEXT_DLG, ReadAscii );
    _SetFltPtr( READER_WRITER_TEXT, ReadAscii );
}

Filters::~Filters()
{
    // die Reader vernichten
    for( sal_uInt16 n = 0; n < MAXFILTER; ++n )
    {
        SwReaderWriterEntry& rEntry = aReaderWriter[n];
        if( rEntry.bDelReader && rEntry.pReader )
            delete rEntry.pReader, rEntry.pReader = NULL;
    }
}

#ifndef DISABLE_DYNLOADING

oslGenericFunction Filters::GetMswordLibSymbol( const char *pSymbol )
{
    if (!msword_.is())
    {
        bool ok = msword_.loadRelative( &thisModule, SVLIBRARY( "msword" ), SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY );
        SAL_WARN_IF(!ok, "sw", "failed to load msword library");
    }
    if (msword_.is())
        return msword_.getFunctionSymbol( OUString::createFromAscii( pSymbol ) );
    return NULL;
}

#endif

}

namespace SwReaderWriter {

Reader* GetReader( ReaderWriterEnum eReader )
{
    return aReaderWriter[eReader].GetReader();
}

void GetWriter( const OUString& rFltName, const OUString& rBaseURL, WriterRef& xRet )
{
    for( sal_uInt16 n = 0; n < MAXFILTER; ++n )
        if ( aFilterDetect[n].IsFilter( rFltName ) )
        {
            aReaderWriter[n].GetWriter( rFltName, rBaseURL, xRet );
            break;
        }
}

SwRead GetReader( const OUString& rFltName )
{
    SwRead pRead = 0;
    for( sal_uInt16 n = 0; n < MAXFILTER; ++n )
        if ( aFilterDetect[n].IsFilter( rFltName ) )
        {
            pRead = aReaderWriter[n].GetReader();
            // fuer einige Reader noch eine Sonderbehandlung:
            if ( pRead )
                    pRead->SetFltName( rFltName );
            break;
        }
    return pRead;
}

} // namespace SwReaderWriter

void Writer::SetVersion( const OUString&, long ) {}


sal_Bool Writer::IsStgWriter() const { return sal_False; }

sal_Bool StgWriter::IsStgWriter() const { return sal_True; }




sal_Bool SwReader::NeedsPasswd( const Reader& /*rOptions*/ )
{
    return sal_False;
}


sal_Bool SwReader::CheckPasswd( const OUString& /*rPasswd*/, const Reader& /*rOptions*/ )
{
    return sal_True;
}



//-----------------------------------------------------------------------
// Filter Flags lesen, wird von WW8 / W4W / EXCEL / LOTUS benutzt.
//-----------------------------------------------------------------------

/*
<FilterFlags>
        <Excel_Lotus>
                <MinRow cfg:type="long">0</MinRow>
                <MaxRow cfg:type="long">0</MaxRow>
                <MinCol cfg:type="long">0</MinCol>
                <MaxCol cfg:type="long">0</MaxCol>
        </Excel_Lotus>
        <W4W>
                <W4WHD cfg:type="long">0</W4WHD>
                <W4WFT cfg:type="long">0</W4WFT>
                <W4W000 cfg:type="long">0</W4W000>
        </W4W>
        <WinWord>
                <WW1F cfg:type="long">0</WW1F>
                <WW cfg:type="long">0</WW>
                <WW8 cfg:type="long">0</WW8>
                <WWF cfg:type="long">0</WWF>
                <WWFA0 cfg:type="long">0</WWFA0>
                <WWFA1 cfg:type="long">0</WWFA1>
                <WWFA2 cfg:type="long">0</WWFA2>
                <WWFB0 cfg:type="long">0</WWFB0>
                <WWFB1 cfg:type="long">0</WWFB1>
                <WWFB2 cfg:type="long">0</WWFB2>
                <WWFLX cfg:type="long">0</WWFLX>
                <WWFLY cfg:type="long">0</WWFLY>
                <WWFT cfg:type="long">0</WWFT>
                <WWWR cfg:type="long">0</WWWR>
        </WinWord>
        <Writer>
                <SW3Imp cfg:type="long">0</SW3Imp>
        </Writer>
</FilterFlags>
*/

#define FILTER_OPTION_ROOT      OUString("Office.Writer/FilterFlags")

SwFilterOptions::SwFilterOptions( sal_uInt16 nCnt, const sal_Char** ppNames,
                                                                sal_uInt32* pValues )
        : ConfigItem( FILTER_OPTION_ROOT )
{
        GetValues( nCnt, ppNames, pValues );
}

void SwFilterOptions::GetValues( sal_uInt16 nCnt, const sal_Char** ppNames,
                                                                        sal_uInt32* pValues )
{
        Sequence<OUString> aNames( nCnt );
        OUString* pNames = aNames.getArray();
        sal_uInt16 n;

        for( n = 0; n < nCnt; ++n )
                pNames[ n ] = OUString::createFromAscii( ppNames[ n ] );
        Sequence<Any> aValues = GetProperties( aNames );

        if( nCnt == aValues.getLength() )
        {
                const Any* pAnyValues = aValues.getConstArray();
                for( n = 0; n < nCnt; ++n )
                        pValues[ n ] = pAnyValues[ n ].hasValue()
                                                        ? *(sal_uInt32*)pAnyValues[ n ].getValue()
                                                        : 0;
        }
        else
                for( n = 0; n < nCnt; ++n )
                        pValues[ n ] = 0;
}

void SwFilterOptions::Commit() {}
void SwFilterOptions::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}



void StgReader::SetFltName( const OUString& rFltNm )
{
        if( SW_STORAGE_READER & GetReaderType() )
                aFltName = rFltNm;
}



SwRelNumRuleSpaces::SwRelNumRuleSpaces( SwDoc& rDoc, sal_Bool bNDoc )
{
        pNumRuleTbl = new SwNumRuleTbl();
        pNumRuleTbl->reserve(8);
        if( !bNDoc )
                pNumRuleTbl->insert( pNumRuleTbl->begin(),
                    rDoc.GetNumRuleTbl().begin(), rDoc.GetNumRuleTbl().end() );
}

SwRelNumRuleSpaces::~SwRelNumRuleSpaces()
{
        if( pNumRuleTbl )
        {
                pNumRuleTbl->clear();
                delete pNumRuleTbl;
        }
}

void CalculateFlySize(SfxItemSet& rFlySet, const SwNodeIndex& rAnchor,
        SwTwips nPageWidth)
{
        const SfxPoolItem* pItem = 0;
        if( SFX_ITEM_SET != rFlySet.GetItemState( RES_FRM_SIZE, sal_True, &pItem ) ||
                MINFLY > ((SwFmtFrmSize*)pItem)->GetWidth() )
        {
                SwFmtFrmSize aSz((SwFmtFrmSize&)rFlySet.Get(RES_FRM_SIZE, sal_True));
                if (pItem)
                        aSz = (SwFmtFrmSize&)(*pItem);

                SwTwips nWidth;
                // dann die Breite des Flys selbst bestimmen. Ist eine Tabelle
                // defininiert, dann benutze deren Breite, sonst die Breite der
                // Seite
                const SwTableNode* pTblNd = rAnchor.GetNode().FindTableNode();
                if( pTblNd )
                        nWidth = pTblNd->GetTable().GetFrmFmt()->GetFrmSize().GetWidth();
                else
                        nWidth = nPageWidth;

                const SwNodeIndex* pSttNd = ((SwFmtCntnt&)rFlySet.Get( RES_CNTNT )).
                                                                        GetCntntIdx();
                if( pSttNd )
                {
                        bool bOnlyOneNode = true;
                        sal_uLong nMinFrm = 0;
                        sal_uLong nMaxFrm = 0;
                        SwTxtNode* pFirstTxtNd = 0;
                        SwNodeIndex aIdx( *pSttNd, 1 );
                        SwNodeIndex aEnd( *pSttNd->GetNode().EndOfSectionNode() );
                        while( aIdx < aEnd )
                        {
                                SwTxtNode *pTxtNd = aIdx.GetNode().GetTxtNode();
                                if( pTxtNd )
                                {
                                        if( !pFirstTxtNd )
                                                pFirstTxtNd = pTxtNd;
                                        else if( pFirstTxtNd != pTxtNd )
                                        {
                                                // forget it
                                                bOnlyOneNode = false;
                                                break;
                                        }

                                        sal_uLong nAbsMinCnts;
                                        pTxtNd->GetMinMaxSize( aIdx.GetIndex(), nMinFrm,
                                                                                        nMaxFrm, nAbsMinCnts );
                                }
                                ++aIdx;
                        }

                        if( bOnlyOneNode )
                        {
                                if( nMinFrm < MINLAY && pFirstTxtNd )
                                {
                                        // if the first node dont contained any content, then
                                        // insert one char in it calc again and delete once again
                                        SwIndex aNdIdx( pFirstTxtNd );
                                        pFirstTxtNd->InsertText(OUString("MM"), aNdIdx);
                                        sal_uLong nAbsMinCnts;
                                        pFirstTxtNd->GetMinMaxSize( pFirstTxtNd->GetIndex(),
                                                                                        nMinFrm, nMaxFrm, nAbsMinCnts );
                                        aNdIdx -= 2;
                    pFirstTxtNd->EraseText( aNdIdx, 2 );
                }

                                // Umrandung und Abstand zum Inhalt beachten
                                const SvxBoxItem& rBoxItem = (SvxBoxItem&)rFlySet.Get( RES_BOX );
                                sal_uInt16 nLine = BOX_LINE_LEFT;
                                for( int i = 0; i < 2; ++i )
                                {
                                        const editeng::SvxBorderLine* pLn = rBoxItem.GetLine( nLine );
                                        if( pLn )
                                        {
                                                sal_uInt16 nWidthTmp = pLn->GetOutWidth() + pLn->GetInWidth();
                        nWidthTmp = nWidthTmp + rBoxItem.GetDistance( nLine );
                                                nMinFrm += nWidthTmp;
                                                nMaxFrm += nWidthTmp;
                                        }
                                        nLine = BOX_LINE_RIGHT;
                                }

                                // Mindestbreite fuer Inhalt einhalten
                                if( nMinFrm < MINLAY )
                                        nMinFrm = MINLAY;
                                if( nMaxFrm < MINLAY )
                                        nMaxFrm = MINLAY;

                                if( nWidth > (sal_uInt16)nMaxFrm )
                                        nWidth = nMaxFrm;
                                else if( nWidth > (sal_uInt16)nMinFrm )
                                        nWidth = nMinFrm;
                        }
                }

                if( MINFLY > nWidth )
                        nWidth = MINFLY;

                aSz.SetWidth( nWidth );
                if( MINFLY > aSz.GetHeight() )
                        aSz.SetHeight( MINFLY );
                rFlySet.Put( aSz );
        }
        else if( MINFLY > ((SwFmtFrmSize*)pItem)->GetHeight() )
        {
                SwFmtFrmSize aSz( *(SwFmtFrmSize*)pItem );
                aSz.SetHeight( MINFLY );
                rFlySet.Put( aSz );
        }
}

namespace
{

struct CharSetNameMap
{
    rtl_TextEncoding eCode;
    const sal_Char* pName;
};

const CharSetNameMap *GetCharSetNameMap()
{
    static const CharSetNameMap aMapArr[] =
    {
#   define IMPLENTRY(X) { RTL_TEXTENCODING_##X, "" #X "" }
        IMPLENTRY(DONTKNOW),
        IMPLENTRY(MS_1252),
        IMPLENTRY(APPLE_ROMAN),
        IMPLENTRY(IBM_437),
        IMPLENTRY(IBM_850),
        IMPLENTRY(IBM_860),
        IMPLENTRY(IBM_861),
        IMPLENTRY(IBM_863),
        IMPLENTRY(IBM_865),
        IMPLENTRY(SYMBOL),
        IMPLENTRY(ASCII_US),
        IMPLENTRY(ISO_8859_1),
        IMPLENTRY(ISO_8859_2),
        IMPLENTRY(ISO_8859_3),
        IMPLENTRY(ISO_8859_4),
        IMPLENTRY(ISO_8859_5),
        IMPLENTRY(ISO_8859_6),
        IMPLENTRY(ISO_8859_7),
        IMPLENTRY(ISO_8859_8),
        IMPLENTRY(ISO_8859_9),
        IMPLENTRY(ISO_8859_14),
        IMPLENTRY(ISO_8859_15),
        IMPLENTRY(IBM_737),
        IMPLENTRY(IBM_775),
        IMPLENTRY(IBM_852),
        IMPLENTRY(IBM_855),
        IMPLENTRY(IBM_857),
        IMPLENTRY(IBM_862),
        IMPLENTRY(IBM_864),
        IMPLENTRY(IBM_866),
        IMPLENTRY(IBM_869),
        IMPLENTRY(MS_874),
        IMPLENTRY(MS_1250),
        IMPLENTRY(MS_1251),
        IMPLENTRY(MS_1253),
        IMPLENTRY(MS_1254),
        IMPLENTRY(MS_1255),
        IMPLENTRY(MS_1256),
        IMPLENTRY(MS_1257),
        IMPLENTRY(MS_1258),
        IMPLENTRY(APPLE_ARABIC),
        IMPLENTRY(APPLE_CENTEURO),
        IMPLENTRY(APPLE_CROATIAN),
        IMPLENTRY(APPLE_CYRILLIC),
        IMPLENTRY(APPLE_DEVANAGARI),
        IMPLENTRY(APPLE_FARSI),
        IMPLENTRY(APPLE_GREEK),
        IMPLENTRY(APPLE_GUJARATI),
        IMPLENTRY(APPLE_GURMUKHI),
        IMPLENTRY(APPLE_HEBREW),
        IMPLENTRY(APPLE_ICELAND),
        IMPLENTRY(APPLE_ROMANIAN),
        IMPLENTRY(APPLE_THAI),
        IMPLENTRY(APPLE_TURKISH),
        IMPLENTRY(APPLE_UKRAINIAN),
        IMPLENTRY(APPLE_CHINSIMP),
        IMPLENTRY(APPLE_CHINTRAD),
        IMPLENTRY(APPLE_JAPANESE),
        IMPLENTRY(APPLE_KOREAN),
        IMPLENTRY(MS_932),
        IMPLENTRY(MS_936),
        IMPLENTRY(MS_949),
        IMPLENTRY(MS_950),
        IMPLENTRY(SHIFT_JIS),
        IMPLENTRY(GB_2312),
        IMPLENTRY(GBT_12345),
        IMPLENTRY(GBK),
        IMPLENTRY(BIG5),
        IMPLENTRY(EUC_JP),
        IMPLENTRY(EUC_CN),
        IMPLENTRY(EUC_TW),
        IMPLENTRY(ISO_2022_JP),
        IMPLENTRY(ISO_2022_CN),
        IMPLENTRY(KOI8_R),
        IMPLENTRY(KOI8_U),
        IMPLENTRY(UTF7),
        IMPLENTRY(UTF8),
        IMPLENTRY(ISO_8859_10),
        IMPLENTRY(ISO_8859_13),
        IMPLENTRY(EUC_KR),
        IMPLENTRY(ISO_2022_KR),
        IMPLENTRY(JIS_X_0201),
        IMPLENTRY(JIS_X_0208),
        IMPLENTRY(JIS_X_0212),
        IMPLENTRY(MS_1361),
        IMPLENTRY(GB_18030),
        IMPLENTRY(BIG5_HKSCS),
        IMPLENTRY(TIS_620),
        IMPLENTRY(PT154),
        IMPLENTRY(UCS4),
        IMPLENTRY(UCS2),
        IMPLENTRY(UNICODE),
        {0,0}       //Last
    };
    return &aMapArr[0];
}

/*
 Get a rtl_TextEncoding from its name
 */
rtl_TextEncoding CharSetFromName(const OUString& rChrSetStr)
{
    const CharSetNameMap *pStart = GetCharSetNameMap();
    rtl_TextEncoding nRet = pStart->eCode;

    for(const CharSetNameMap *pMap = pStart; pMap->pName; ++pMap)
    {
        if(rChrSetStr.equalsIgnoreAsciiCaseAscii(pMap->pName))
        {
            nRet = pMap->eCode;
            break;
        }
    }

    OSL_ENSURE(nRet != pStart->eCode, "TXT: That was an unknown language!");

        return nRet;
}

/*
 Get the String name of an rtl_TextEncoding
 */
OUString NameFromCharSet(rtl_TextEncoding nChrSet)
{
    const CharSetNameMap *pStart = GetCharSetNameMap();
    const char *pRet = pStart->pName;

    for(const CharSetNameMap *pMap = pStart; pMap->pName; ++pMap)
    {
        if (nChrSet == pMap->eCode)
        {
            pRet = pMap->pName;
            break;
        }
    }

    OSL_ENSURE(pRet != pStart->pName, "TXT: That was an unknown language!");

    return OUString::createFromAscii(pRet);
}

}

// for the automatic conversion (mail/news/...)
// The user data contains the options for the ascii import/export filter.
// The format is:
//      1. CharSet - as ascii chars
//      2. LineEnd - as CR/LR/CRLF
//      3. Fontname
//      4. Language
// the delimetercharacter is ","
//

void SwAsciiOptions::ReadUserData( const OUString& rStr )
{
    sal_Int32 nToken = 0;
    sal_uInt16 nCnt = 0;
    do {
        const OUString sToken = rStr.getToken( 0, ',', nToken );
        if (!sToken.isEmpty())
        {
            switch( nCnt )
            {
            case 0:         // CharSet
                eCharSet = CharSetFromName(sToken);
                break;
            case 1:         // LineEnd
                if (sToken.equalsIgnoreAsciiCase("CRLF"))
                    eCRLF_Flag = LINEEND_CRLF;
                else if (sToken.equalsIgnoreAsciiCase("LF"))
                    eCRLF_Flag = LINEEND_LF;
                else
                    eCRLF_Flag = LINEEND_CR;
                break;
            case 2:         // fontname
                sFont = sToken;
                break;
            case 3:         // Language
                nLanguage = LanguageTag::convertToLanguageTypeWithFallback( sToken );
                break;
            }
        }
        ++nCnt;
    } while( -1 != nToken );
}

void SwAsciiOptions::WriteUserData(OUString& rStr)
{
    // 1. charset
    rStr = NameFromCharSet(eCharSet) + ",";

    // 2. LineEnd
    switch(eCRLF_Flag)
    {
    case LINEEND_CRLF:
        rStr += "CRLF";
        break;
    case LINEEND_CR:
        rStr += "CR";
        break;
    case LINEEND_LF:
        rStr += "LF";
        break;
    }
    rStr += ",";

    // 3. Fontname
    rStr += sFont + ",";

    // 4. Language
    if (nLanguage)
    {
        rStr += LanguageTag::convertToBcp47(nLanguage);
    }
    rStr += ",";
}

#ifdef DISABLE_DYNLOADING

extern "C" {
    Reader *ImportRTF();
    void ExportRTF( const String&, const String& rBaseURL, WriterRef& );
    Reader *ImportDOC();
    void ExportDOC( const String&, const String& rBaseURL, WriterRef& );
    sal_uLong SaveOrDelMSVBAStorage_ww8( SfxObjectShell&, SotStorage&, sal_Bool, const String& );
    sal_uLong GetSaveWarningOfMSVBAStorage_ww8( SfxObjectShell& );
}

#endif

Reader* GetRTFReader()
{
#ifndef DISABLE_DYNLOADING

    FnGetReader pFunction = reinterpret_cast<FnGetReader>( SwGlobals::getFilters().GetMswordLibSymbol( "ImportRTF" ) );

    if ( pFunction )
        return (*pFunction)();

    return NULL;
#else
    return ImportRTF();
#endif

}

void GetRTFWriter( const OUString& rFltName, const OUString& rBaseURL, WriterRef& xRet )
{
#ifndef DISABLE_DYNLOADING
    FnGetWriter pFunction = reinterpret_cast<FnGetWriter>( SwGlobals::getFilters().GetMswordLibSymbol( "ExportRTF" ) );

    if ( pFunction )
        (*pFunction)( rFltName, rBaseURL, xRet );
    else
        xRet = WriterRef(0);
#else
    ExportRTF( rFltName, rBaseURL, xRet );
#endif
}

Reader* GetWW8Reader()
{
#ifndef DISABLE_DYNLOADING
    FnGetReader pFunction = reinterpret_cast<FnGetReader>( SwGlobals::getFilters().GetMswordLibSymbol( "ImportDOC" ) );

    if ( pFunction )
        return (*pFunction)();

    return NULL;
#else
    return ImportDOC();
#endif
}

void GetWW8Writer( const OUString& rFltName, const OUString& rBaseURL, WriterRef& xRet )
{
#ifndef DISABLE_DYNLOADING
    FnGetWriter pFunction = reinterpret_cast<FnGetWriter>( SwGlobals::getFilters().GetMswordLibSymbol( "ExportDOC" ) );

    if ( pFunction )
        (*pFunction)( rFltName, rBaseURL, xRet );
    else
        xRet = WriterRef(0);
#else
    ExportDOC( rFltName, rBaseURL, xRet );
#endif
}

typedef sal_uLong ( SAL_CALL *SaveOrDel )( SfxObjectShell&, SotStorage&, sal_Bool, const String& );
typedef sal_uLong ( SAL_CALL *GetSaveWarning )( SfxObjectShell& );

sal_uLong SaveOrDelMSVBAStorage( SfxObjectShell& rDoc, SotStorage& rStor, sal_Bool bSaveInto, const OUString& rStorageName )
{
#ifndef DISABLE_DYNLOADING
    SaveOrDel pFunction = reinterpret_cast<SaveOrDel>( SwGlobals::getFilters().GetMswordLibSymbol( "SaveOrDelMSVBAStorage_ww8" ) );
    if( pFunction )
        return pFunction( rDoc, rStor, bSaveInto, rStorageName );
    return ERRCODE_NONE;
#else
    return SaveOrDelMSVBAStorage_ww8( rDoc, rStor, bSaveInto, rStorageName );
#endif
}

sal_uLong GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocS )
{
#ifndef DISABLE_DYNLOADING
    GetSaveWarning pFunction = reinterpret_cast<GetSaveWarning>( SwGlobals::getFilters().GetMswordLibSymbol( "GetSaveWarningOfMSVBAStorage_ww8" ) );
    if( pFunction )
        return pFunction( rDocS );
    return ERRCODE_NONE;
#else
    return GetSaveWarningOfMSVBAStorage_ww8( rDocS );
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
