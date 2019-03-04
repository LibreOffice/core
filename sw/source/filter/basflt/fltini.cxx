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
#include <hintids.hxx>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/any.hxx>
#include <tools/svlibrary.h>
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
#include <swdll.hxx>
#include <iodetect.hxx>
#include <osl/module.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star;

Reader *ReadAscii = nullptr, *ReadHTML = nullptr, *ReadXML = nullptr;

static Reader* GetRTFReader();
static Reader* GetWW8Reader();
static Reader* GetDOCXReader();

// Note: if editing, please don't forget to modify also the enum
// ReaderWriterEnum and aFilterDetect in iodetect.hxx & iodetect.cxx
static SwReaderWriterEntry aReaderWriter[] =
{
    SwReaderWriterEntry( &::GetRTFReader, &::GetRTFWriter,  true  ),
    SwReaderWriterEntry( nullptr,               &::GetASCWriter,  false ),
    SwReaderWriterEntry( &::GetWW8Reader, nullptr,          true  ),
    SwReaderWriterEntry( &::GetWW8Reader, &::GetWW8Writer,  true  ),
    SwReaderWriterEntry( &::GetRTFReader, &::GetRTFWriter,  true  ),
    SwReaderWriterEntry( nullptr,               &::GetHTMLWriter, true  ),
    SwReaderWriterEntry( &::GetWW8Reader, nullptr,                true  ),
    SwReaderWriterEntry( nullptr,               &::GetXMLWriter,  true  ),
    SwReaderWriterEntry( nullptr,               &::GetASCWriter,  false ),
    SwReaderWriterEntry( nullptr,               &::GetASCWriter,  true  ),
    SwReaderWriterEntry( &::GetDOCXReader,      nullptr,          true  )
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
    return nullptr;
}

void SwReaderWriterEntry::GetWriter( const OUString& rNm, const OUString& rBaseURL, WriterRef& xWrt ) const
{
    if ( fnGetWriter )
        (*fnGetWriter)( rNm, rBaseURL, xWrt );
    else
        xWrt = WriterRef(nullptr);
}

Reader* SwGetReaderXML() // SW_DLLPUBLIC
{
        return ReadXML;
}

static void SetFltPtr( sal_uInt16 rPos, Reader* pReader )
{
        aReaderWriter[ rPos ].pReader = pReader;
}

namespace sw {

Filters::Filters()
{
    ReadAscii = new AsciiReader;
    ReadHTML = new HTMLReader;
    ReadXML = new XMLReader;
    SetFltPtr( READER_WRITER_BAS, ReadAscii );
    SetFltPtr( READER_WRITER_HTML, ReadHTML );
    SetFltPtr( READER_WRITER_XML, ReadXML );
    SetFltPtr( READER_WRITER_TEXT_DLG, ReadAscii );
    SetFltPtr( READER_WRITER_TEXT, ReadAscii );
}

Filters::~Filters()
{
    // kill Readers
    for(SwReaderWriterEntry & rEntry : aReaderWriter)
    {
        if( rEntry.bDelReader && rEntry.pReader )
        {
            delete rEntry.pReader;
            rEntry.pReader = nullptr;
        }
    }
}

#ifndef DISABLE_DYNLOADING

oslGenericFunction Filters::GetMswordLibSymbol( const char *pSymbol )
{
    if (!msword_.is())
    {
        OUString url("$LO_LIB_DIR/" SVLIBRARY("msword"));
        rtl::Bootstrap::expandMacros(url);
        bool ok = msword_.load( url, SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY );
        SAL_WARN_IF(!ok, "sw", "failed to load msword library");
    }
    if (msword_.is())
        return msword_.getFunctionSymbol( OUString::createFromAscii( pSymbol ) );
    return nullptr;
}

#endif

}

namespace SwReaderWriter {

Reader* GetRtfReader()
{
    return aReaderWriter[READER_WRITER_RTF].GetReader();
}

Reader* GetDOCXReader()
{
    return aReaderWriter[READER_WRITER_DOCX].GetReader();
}

void GetWriter( const OUString& rFltName, const OUString& rBaseURL, WriterRef& xRet )
{
    for( int n = 0; n < MAXFILTER; ++n )
        if ( aFilterDetect[n].IsFilter( rFltName ) )
        {
            aReaderWriter[n].GetWriter( rFltName, rBaseURL, xRet );
            break;
        }
}

Reader* GetReader( const OUString& rFltName )
{
    Reader* pRead = nullptr;
    for( int n = 0; n < MAXFILTER; ++n )
    {
        if ( aFilterDetect[n].IsFilter( rFltName ) )
        {
            pRead = aReaderWriter[n].GetReader();
            // add special treatment for some readers
            if ( pRead )
                pRead->SetFltName( rFltName );
            break;
        }
    }
    return pRead;
}

} // namespace SwReaderWriter

bool Writer::IsStgWriter() const { return false; }

bool StgWriter::IsStgWriter() const { return true; }

// Read Filter Flags; used by WW8 / W4W / EXCEL / LOTUS

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

SwFilterOptions::SwFilterOptions( sal_uInt16 nCnt, const sal_Char** ppNames,
                                                                sal_uInt64* pValues )
    : ConfigItem( "Office.Writer/FilterFlags" )
{
    GetValues( nCnt, ppNames, pValues );
}

void SwFilterOptions::GetValues( sal_uInt16 nCnt, const sal_Char** ppNames,
                                                                        sal_uInt64* pValues )
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
                                            ? *o3tl::doAccess<sal_uInt64>(pAnyValues[ n ])
                                            : 0;
    }
    else
    {
        for( n = 0; n < nCnt; ++n )
            pValues[ n ] = 0;
    }
}

void SwFilterOptions::ImplCommit() {}
void SwFilterOptions::Notify( const css::uno::Sequence< OUString >& ) {}

void StgReader::SetFltName( const OUString& rFltNm )
{
    if( SwReaderType::Storage & GetReaderType() )
        aFltName = rFltNm;
}

SwRelNumRuleSpaces::SwRelNumRuleSpaces( SwDoc const & rDoc, bool bNDoc )
{
    pNumRuleTable.reset(new SwNumRuleTable);
    pNumRuleTable->reserve(8);
    if( !bNDoc )
        pNumRuleTable->insert( pNumRuleTable->begin(),
            rDoc.GetNumRuleTable().begin(), rDoc.GetNumRuleTable().end() );
}

SwRelNumRuleSpaces::~SwRelNumRuleSpaces()
{
    if( pNumRuleTable )
        pNumRuleTable->clear();
}

void CalculateFlySize(SfxItemSet& rFlySet, const SwNodeIndex& rAnchor,
        SwTwips nPageWidth)
{
    const SfxPoolItem* pItem = nullptr;
    if( SfxItemState::SET != rFlySet.GetItemState( RES_FRM_SIZE, true, &pItem ) ||
            MINFLY > static_cast<const SwFormatFrameSize*>(pItem)->GetWidth() )
    {
        SwFormatFrameSize aSz(rFlySet.Get(RES_FRM_SIZE));
        if (pItem)
            aSz = static_cast<const SwFormatFrameSize&>(*pItem);

        SwTwips nWidth;
        // determine the width; if there is a table use the width of the table;
        // otherwise use the width of the page
        const SwTableNode* pTableNd = rAnchor.GetNode().FindTableNode();
        if( pTableNd )
            nWidth = pTableNd->GetTable().GetFrameFormat()->GetFrameSize().GetWidth();
        else
            nWidth = nPageWidth;

        const SwNodeIndex* pSttNd = rFlySet.Get( RES_CNTNT ).GetContentIdx();
        if( pSttNd )
        {
            bool bOnlyOneNode = true;
            sal_uLong nMinFrame = 0;
            sal_uLong nMaxFrame = 0;
            SwTextNode* pFirstTextNd = nullptr;
            SwNodeIndex aIdx( *pSttNd, 1 );
            SwNodeIndex aEnd( *pSttNd->GetNode().EndOfSectionNode() );
            while( aIdx < aEnd )
            {
                SwTextNode *pTextNd = aIdx.GetNode().GetTextNode();
                if( pTextNd )
                {
                    if( !pFirstTextNd )
                        pFirstTextNd = pTextNd;
                    else if( pFirstTextNd != pTextNd )
                    {
                        // forget it
                        bOnlyOneNode = false;
                        break;
                    }

                    sal_uLong nAbsMinCnts;
                    pTextNd->GetMinMaxSize( aIdx.GetIndex(), nMinFrame, nMaxFrame, nAbsMinCnts );
                }
                ++aIdx;
            }

            if( bOnlyOneNode )
            {
                if( nMinFrame < MINLAY && pFirstTextNd )
                {
                    // if the first node don't contained any content, then
                    // insert one char in it calc again and delete once again
                    SwIndex aNdIdx( pFirstTextNd );
                    pFirstTextNd->InsertText("MM", aNdIdx);
                    sal_uLong nAbsMinCnts;
                    pFirstTextNd->GetMinMaxSize( pFirstTextNd->GetIndex(),
                                                                    nMinFrame, nMaxFrame, nAbsMinCnts );
                    aNdIdx -= 2;
                    pFirstTextNd->EraseText( aNdIdx, 2 );
                }

                // consider border and distance to content
                const SvxBoxItem& rBoxItem = rFlySet.Get( RES_BOX );
                SvxBoxItemLine nLine = SvxBoxItemLine::LEFT;
                for( int i = 0; i < 2; ++i )
                {
                    const editeng::SvxBorderLine* pLn = rBoxItem.GetLine( nLine );
                    if( pLn )
                    {
                        sal_uInt16 nWidthTmp = pLn->GetOutWidth() + pLn->GetInWidth();
                        nWidthTmp = nWidthTmp + rBoxItem.GetDistance( nLine );
                        nMinFrame += nWidthTmp;
                        nMaxFrame += nWidthTmp;
                    }
                    nLine = SvxBoxItemLine::RIGHT;
                }

                // enforce minimum width for contents
                if( nMinFrame < MINLAY )
                    nMinFrame = MINLAY;
                if( nMaxFrame < MINLAY )
                    nMaxFrame = MINLAY;

                if( nWidth > static_cast<sal_uInt16>(nMaxFrame) )
                    nWidth = nMaxFrame;
                else if( nWidth > static_cast<sal_uInt16>(nMinFrame) )
                    nWidth = nMinFrame;
            }
        }

        if( MINFLY > nWidth )
            nWidth = MINFLY;

        aSz.SetWidth( nWidth );
        if( MINFLY > aSz.GetHeight() )
            aSz.SetHeight( MINFLY );
        rFlySet.Put( aSz );
    }
    else if( MINFLY > static_cast<const SwFormatFrameSize*>(pItem)->GetHeight() )
    {
        SwFormatFrameSize aSz( *static_cast<const SwFormatFrameSize*>(pItem) );
        aSz.SetHeight( MINFLY );
        rFlySet.Put( aSz );
    }
}

namespace
{

struct CharSetNameMap
{
    rtl_TextEncoding const eCode;
    const sal_Char* pName;
};

const CharSetNameMap *GetCharSetNameMap()
{
    static const CharSetNameMap aMapArr[] =
    {
#   define IMPLENTRY(X) { RTL_TEXTENCODING_##X, #X }
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
        {0,nullptr}       //Last
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
//      2. LineEnd - as CR/LF/CRLF
//      3. Fontname
//      4. Language
//      5. Whether to include byte-order-mark - as true/false
// the delimiter character is ","

void SwAsciiOptions::ReadUserData( const OUString& rStr )
{
    sal_Int32 nToken = 0;
    OUString sToken = rStr.getToken(0, ',', nToken); // 1. Charset name
    if (!sToken.isEmpty())
        eCharSet = CharSetFromName(sToken);
    if (nToken >= 0 && !(sToken = rStr.getToken(0, ',', nToken)).isEmpty()) // 2. Line ending type
    {
        if (sToken.equalsIgnoreAsciiCase("CRLF"))
            eCRLF_Flag = LINEEND_CRLF;
        else if (sToken.equalsIgnoreAsciiCase("LF"))
            eCRLF_Flag = LINEEND_LF;
        else
            eCRLF_Flag = LINEEND_CR;
    }
    if (nToken >= 0 && !(sToken = rStr.getToken(0, ',', nToken)).isEmpty()) // 3. Font name
        sFont = sToken;
    if (nToken >= 0 && !(sToken = rStr.getToken(0, ',', nToken)).isEmpty()) // 4. Language tag
        nLanguage = LanguageTag::convertToLanguageTypeWithFallback(sToken);
    if (nToken >= 0 && !(sToken = rStr.getToken(0, ',', nToken)).isEmpty()) // 5. Include BOM?
        bIncludeBOM = !(sToken.equalsIgnoreAsciiCase("FALSE"));
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

    // 5. Whether to include byte-order-mark
    if( bIncludeBOM )
    {
        rStr += "true";
    }
    else
    {
        rStr += "false";
    }
    rStr += ",";
}

#ifdef DISABLE_DYNLOADING

extern "C" {
    Reader *ImportRTF();
    void ExportRTF( const OUString&, const OUString& rBaseURL, WriterRef& );
    Reader *ImportDOC();
    void ExportDOC( const OUString&, const OUString& rBaseURL, WriterRef& );
    Reader *ImportDOCX();
    sal_uInt32 SaveOrDelMSVBAStorage_ww8( SfxObjectShell&, SotStorage&, sal_Bool, const OUString& );
    sal_uInt32 GetSaveWarningOfMSVBAStorage_ww8( SfxObjectShell& );
}

#endif

Reader* GetRTFReader()
{
#ifndef DISABLE_DYNLOADING

    FnGetReader pFunction = reinterpret_cast<FnGetReader>( SwGlobals::getFilters().GetMswordLibSymbol( "ImportRTF" ) );

    if ( pFunction )
        return (*pFunction)();

    return nullptr;
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
        xRet = WriterRef(nullptr);
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

    return nullptr;
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
        xRet = WriterRef(nullptr);
#else
    ExportDOC( rFltName, rBaseURL, xRet );
#endif
}

Reader* GetDOCXReader()
{
#ifndef DISABLE_DYNLOADING
    FnGetReader pFunction = reinterpret_cast<FnGetReader>( SwGlobals::getFilters().GetMswordLibSymbol( "ImportDOCX" ) );

    if ( pFunction )
        return (*pFunction)();

    return nullptr;
#else
    return ImportDOCX();
#endif
}

typedef sal_uInt32 ( *SaveOrDel )( SfxObjectShell&, SotStorage&, sal_Bool, const OUString& );
typedef sal_uInt32 ( *GetSaveWarning )( SfxObjectShell& );

ErrCode SaveOrDelMSVBAStorage( SfxObjectShell& rDoc, SotStorage& rStor, bool bSaveInto, const OUString& rStorageName )
{
#ifndef DISABLE_DYNLOADING
    SaveOrDel pFunction = reinterpret_cast<SaveOrDel>( SwGlobals::getFilters().GetMswordLibSymbol( "SaveOrDelMSVBAStorage_ww8" ) );
    if( pFunction )
        return ErrCode(pFunction( rDoc, rStor, bSaveInto, rStorageName ));
    return ERRCODE_NONE;
#else
    return ErrCode(SaveOrDelMSVBAStorage_ww8( rDoc, rStor, bSaveInto, rStorageName ));
#endif
}

ErrCode GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocS )
{
#ifndef DISABLE_DYNLOADING
    GetSaveWarning pFunction = reinterpret_cast<GetSaveWarning>( SwGlobals::getFilters().GetMswordLibSymbol( "GetSaveWarningOfMSVBAStorage_ww8" ) );
    if( pFunction )
        return ErrCode(pFunction( rDocS ));
    return ERRCODE_NONE;
#else
    return ErrCode(GetSaveWarningOfMSVBAStorage_ww8( rDocS ));
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
