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


#include <ctype.h>
#include <tools/diagnose_ex.h>
#include <rtl/tencinfo.h>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svtools/rtftoken.h>
#include <svl/itempool.hxx>

#include <comphelper/string.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/svxrtf.hxx>
#include <editeng/editids.hrc>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>


using namespace ::com::sun::star;


static rtl_TextEncoding lcl_GetDefaultTextEncodingForRTF()
{

    OUString aLangString( Application::GetSettings().GetLanguageTag().getLanguage());

    if ( aLangString == "ru" || aLangString == "uk" )
        return RTL_TEXTENCODING_MS_1251;
    if ( aLangString == "tr" )
        return RTL_TEXTENCODING_MS_1254;
    else
        return RTL_TEXTENCODING_MS_1252;
}

// -------------- Methods --------------------

SvxRTFParser::SvxRTFParser( SfxItemPool& rPool, SvStream& rIn,
            uno::Reference<document::XDocumentProperties> i_xDocProps,
            bool const bReadNewDoc )
    : SvRTFParser( rIn, 5 )
    , aPlainMap(rPool)
    , aPardMap(rPool)
    , pInsPos( nullptr )
    , pAttrPool( &rPool )
    , m_xDocProps( i_xDocProps )
    , pRTFDefaults( nullptr )
    , nVersionNo( 0 )
    , nDfltFont( 0)
    , bNewDoc( bReadNewDoc )
    , bNewGroup( false)
    , bIsSetDfltTab( false)
    , bChkStyleAttr( false )
    , bCalcValue( false )
    , bPardTokenRead( false)
    , bReadDocInfo( false )
    , bIsLeftToRightDef( true)
    , bIsInReadStyleTab( false)
{
    pDfltFont = new vcl::Font;
    pDfltColor = new Color;
}

SvxRTFParser::~SvxRTFParser()
{
    if( !aColorTbl.empty() )
        ClearColorTbl();
    if( !aAttrStack.empty() )
        ClearAttrStack();

    delete pRTFDefaults;

    delete pInsPos;
    delete pDfltFont;
    delete pDfltColor;
}

void SvxRTFParser::SetInsPos( const EditPosition& rNew )
{
    delete pInsPos;
    pInsPos = rNew.Clone();
}

SvParserState SvxRTFParser::CallParser()
{
    DBG_ASSERT( pInsPos, "no insertion position");

    if( !pInsPos )
        return SVPAR_ERROR;

    if( !aColorTbl.empty() )
        ClearColorTbl();
    if (!m_FontTable.empty())
        ClearFontTbl();
    if (!m_StyleTable.empty())
        ClearStyleTbl();
    if( !aAttrStack.empty() )
        ClearAttrStack();

    bIsSetDfltTab = false;
    bNewGroup = false;
    nDfltFont = 0;

    sBaseURL.clear();

    // generate the correct WhichId table from the set WhichIds.
    BuildWhichTable();

    return SvRTFParser::CallParser();
}

void SvxRTFParser::Continue( int nToken )
{
    SvRTFParser::Continue( nToken );

    if( SVPAR_PENDING != GetStatus() )
    {
        SetAllAttrOfStk();
    //Regardless of what "color 0" is, word defaults to auto as the default colour.
    //e.g. see #i7713#
     }
}


// is called for each token that is recognized in CallParser
void SvxRTFParser::NextToken( int nToken )
{
    sal_Unicode cCh;
    switch( nToken )
    {
    case RTF_COLORTBL:      ReadColorTable();       break;
    case RTF_FONTTBL:       ReadFontTable();        break;
    case RTF_STYLESHEET:    ReadStyleTable();       break;

    case RTF_DEFF:
            if( bNewDoc )
            {
                if (!m_FontTable.empty())
                    // Can immediately be set
                    SetDefault( nToken, nTokenValue );
                else
                    // is set after reading the font table
                    nDfltFont = int(nTokenValue);
            }
            break;

    case RTF_DEFTAB:
    case RTF_DEFLANG:
            if( bNewDoc )
                SetDefault( nToken, nTokenValue );
            break;


    case RTF_PICT:          ReadBitmapData();       break;

    case RTF_LINE:          cCh = '\n'; goto INSINGLECHAR;
    case RTF_TAB:           cCh = '\t'; goto INSINGLECHAR;
    case RTF_SUBENTRYINDEX: cCh = ':';  goto INSINGLECHAR;

    case RTF_EMDASH:		cCh = 0x2014;	goto INSINGLECHAR;
    case RTF_ENDASH:		cCh = 0x2013;	goto INSINGLECHAR;
    case RTF_BULLET:		cCh = 0x2022;	goto INSINGLECHAR;
    case RTF_LQUOTE:		cCh = 0x2018;	goto INSINGLECHAR;
    case RTF_RQUOTE:		cCh = 0x2019;	goto INSINGLECHAR;
    case RTF_LDBLQUOTE:		cCh = 0x201C;	goto INSINGLECHAR;
    case RTF_RDBLQUOTE:		cCh = 0x201D;	goto INSINGLECHAR;
INSINGLECHAR:
        aToken = OUString(cCh);
        SAL_FALLTHROUGH; // aToken is set as Text
    case RTF_TEXTTOKEN:
        {
            InsertText();
            // all collected Attributes are set
            for (size_t n = m_AttrSetList.size(); n; )
            {
                auto const& pStkSet = m_AttrSetList[--n];
                SetAttrSet( *pStkSet );
                m_AttrSetList.pop_back();
            }
        }
        break;


    case RTF_PAR:
        InsertPara();
        break;
    case '{':
        if (bNewGroup)          // Nesting!
            _GetAttrSet();
        bNewGroup = true;
        break;
    case '}':
        if( !bNewGroup )        // Empty Group ??
            AttrGroupEnd();
        bNewGroup = false;
        break;
    case RTF_INFO:
        if (bReadDocInfo && bNewDoc && m_xDocProps.is())
            ReadInfo();
        else
            SkipGroup();
        break;

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // First overwrite all (all have to be in one group!!)
    // Could also appear in the RTF-file without the IGNORE-Flag; all Groups
    // with the IGNORE-Flag are overwritten in the default branch.

    case RTF_SWG_PRTDATA:
    case RTF_FIELD:
    case RTF_ATNID:
    case RTF_ANNOTATION:

    case RTF_BKMKSTART:
    case RTF_BKMKEND:
    case RTF_BKMK_KEY:
    case RTF_XE:
    case RTF_TC:
    case RTF_NEXTFILE:
    case RTF_TEMPLATE:
    // RTF_SHPRSLT disabled for #i19718#
                            SkipGroup();
                            break;
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    case RTF_PGDSCNO:
    case RTF_PGBRK:
    case RTF_SHADOW:
            if( RTF_IGNOREFLAG != GetStackPtr( -1 )->nTokenId )
                break;
            nToken = SkipToken();
            if( '{' == GetStackPtr( -1 )->nTokenId )
                nToken = SkipToken();

            ReadAttr( nToken, &GetAttrSet() );
            break;

    default:
        switch( nToken & ~(0xff | RTF_SWGDEFS) )
        {
        case RTF_PARFMT:        // hier gibts keine Swg-Defines
            ReadAttr( nToken, &GetAttrSet() );
            break;

        case RTF_CHRFMT:
        case RTF_BRDRDEF:
        case RTF_TABSTOPDEF:

            if( RTF_SWGDEFS & nToken)
            {
                if( RTF_IGNOREFLAG != GetStackPtr( -1 )->nTokenId )
                    break;
                nToken = SkipToken();
                if( '{' == GetStackPtr( -1 )->nTokenId )
                {
                    nToken = SkipToken();
                }
            }
            ReadAttr( nToken, &GetAttrSet() );
            break;
        default:
            {
                if( /*( '{' == GetStackPtr( -1 )->nTokenId ) ||*/
                    ( RTF_IGNOREFLAG == GetStackPtr( -1 )->nTokenId &&
                      '{' == GetStackPtr( -2 )->nTokenId ) )
                    SkipGroup();
            }
            break;
        }
        break;
    }
}

void SvxRTFParser::ReadStyleTable()
{
    int bSaveChkStyleAttr = bChkStyleAttr ? 1 : 0;
    sal_uInt16 nStyleNo = 0;
    bool bHasStyleNo = false;
    int _nOpenBrakets = 1;      // the first was already detected earlier!!
    ::std::unique_ptr<SvxRTFStyleType> pStyle(
            new SvxRTFStyleType( *pAttrPool, &aWhichMap[0] ));
    pStyle->aAttrSet.Put( GetRTFDefaults() );

    bIsInReadStyleTab = true;
    bChkStyleAttr = false;      // Do not check Attribute against the Styles

    while( _nOpenBrakets && IsParserWorking() )
    {
        int nToken = GetNextToken();
        switch( nToken )
        {
        case '}':       if( --_nOpenBrakets && IsParserWorking() )
                            // Style has been completely read,
                            // so this is still a stable status
                            SaveState( RTF_STYLESHEET );
                        break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    SkipToken();
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ) &&
                            RTF_PN != nToken )
                    SkipToken( -2 );
                else
                {
                    // filter out at once
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++_nOpenBrakets;
            }
            break;

        case RTF_SBASEDON:  pStyle->nBasedOn = sal_uInt16(nTokenValue); pStyle->bBasedOnIsSet=true; break;
        case RTF_SNEXT:     pStyle->nNext = sal_uInt16(nTokenValue);    break;
        case RTF_OUTLINELEVEL:
        case RTF_SOUTLVL:   pStyle->nOutlineNo = sal_uInt8(nTokenValue);    break;
        case RTF_S:         nStyleNo = (short)nTokenValue;
                            bHasStyleNo = true;
                            break;
        case RTF_CS:        nStyleNo = (short)nTokenValue;
                            bHasStyleNo = true;
                            pStyle->bIsCharFmt = true;
                            break;

        case RTF_TEXTTOKEN:
            if (bHasStyleNo)
            {
                pStyle->sName = DelCharAtEnd( aToken, ';' );

                if (!m_StyleTable.empty())
                {
                    m_StyleTable.erase(nStyleNo);
                }
                // All data from the font is available, so off to the table
                m_StyleTable.insert(std::make_pair(nStyleNo, std::move(pStyle)));
                pStyle.reset(new SvxRTFStyleType( *pAttrPool, &aWhichMap[0] ));
                pStyle->aAttrSet.Put( GetRTFDefaults() );
                nStyleNo = 0;
                bHasStyleNo = false;
            }
            break;
        default:
            switch( nToken & ~(0xff | RTF_SWGDEFS) )
            {
            case RTF_PARFMT:        // hier gibts keine Swg-Defines
                ReadAttr( nToken, &pStyle->aAttrSet );
                break;

            case RTF_CHRFMT:
            case RTF_BRDRDEF:
            case RTF_TABSTOPDEF:

                if( RTF_SWGDEFS & nToken)
                {
                    if( RTF_IGNOREFLAG != GetStackPtr( -1 )->nTokenId )
                        break;
                    nToken = SkipToken();
                    if( '{' == GetStackPtr( -1 )->nTokenId )
                    {
                        nToken = SkipToken();
                    }
                }
                ReadAttr( nToken, &pStyle->aAttrSet );
                break;
            }
            break;
        }
    }
    pStyle.reset();         // Delete the Last Style
    SkipToken();        // the closing brace is evaluated "above"

    // Flag back to old state
    bChkStyleAttr = bSaveChkStyleAttr;
    bIsInReadStyleTab = false;
}

void SvxRTFParser::ReadColorTable()
{
    int nToken;
    sal_uInt8 nRed = 0xff, nGreen = 0xff, nBlue = 0xff;

    while( '}' != ( nToken = GetNextToken() ) && IsParserWorking() )
    {
        switch( nToken )
        {
        case RTF_RED:   nRed = sal_uInt8(nTokenValue);      break;
        case RTF_GREEN: nGreen = sal_uInt8(nTokenValue);        break;
        case RTF_BLUE:  nBlue = sal_uInt8(nTokenValue);     break;

        case RTF_TEXTTOKEN:
            if( 1 == aToken.getLength()
                    ? aToken[ 0 ] != ';'
                    : -1 == aToken.indexOf( ";" ) )
                break;      // At least the ';' must be found

            SAL_FALLTHROUGH;

        case ';':
            if( IsParserWorking() )
            {
                // one color is finished, fill in the table
                // try to map the values to SV internal names
                Color* pColor = new Color( nRed, nGreen, nBlue );
                if( aColorTbl.empty() &&
                    sal_uInt8(-1) == nRed && sal_uInt8(-1) == nGreen && sal_uInt8(-1) == nBlue )
                    pColor->SetColor( COL_AUTO );
                aColorTbl.push_back( pColor );
                nRed = 0, nGreen = 0, nBlue = 0;

                // Color has been completely read,
                // so this is still a stable status
                SaveState( RTF_COLORTBL );
            }
            break;
        }
    }
    SkipToken();        // the closing brace is evaluated "above"
}

void SvxRTFParser::ReadFontTable()
{
    int _nOpenBrakets = 1;      // the first was already detected earlier!!
    std::unique_ptr<vcl::Font> pFont(new vcl::Font);
    short nFontNo(0), nInsFontNo (0);
    OUString sAltNm, sFntNm;
    bool bIsAltFntNm = false;

    rtl_TextEncoding nSystemChar = lcl_GetDefaultTextEncodingForRTF();
    pFont->SetCharSet( nSystemChar );
    SetEncoding( nSystemChar );

    while( _nOpenBrakets && IsParserWorking() )
    {
        bool bCheckNewFont = false;
        int nToken = GetNextToken();
        switch( nToken )
        {
            case '}':
                bIsAltFntNm = false;
                // Style has been completely read,
                // so this is still a stable status
                if( --_nOpenBrakets <= 1 && IsParserWorking() )
                    SaveState( RTF_FONTTBL );
                bCheckNewFont = true;
                nInsFontNo = nFontNo;
                break;
            case '{':
                if( RTF_IGNOREFLAG != GetNextToken() )
                    SkipToken();
                // immediately skip unknown and all known but non-evaluated
                // groups
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ) &&
                        RTF_PANOSE != nToken && RTF_FNAME != nToken &&
                        RTF_FONTEMB != nToken && RTF_FONTFILE != nToken )
                    SkipToken( -2 );
                else
                {
                    // filter out at once
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++_nOpenBrakets;
                break;
            case RTF_FROMAN:
                pFont->SetFamily( FAMILY_ROMAN );
                break;
            case RTF_FSWISS:
                pFont->SetFamily( FAMILY_SWISS );
                break;
            case RTF_FMODERN:
                pFont->SetFamily( FAMILY_MODERN );
                break;
            case RTF_FSCRIPT:
                pFont->SetFamily( FAMILY_SCRIPT );
                break;
            case RTF_FDECOR:
                pFont->SetFamily( FAMILY_DECORATIVE );
                break;
            // for technical/symbolic font of the rtl_TextEncoding is changed!
            case RTF_FTECH:
                pFont->SetCharSet( RTL_TEXTENCODING_SYMBOL );
                SAL_FALLTHROUGH;
            case RTF_FNIL:
                pFont->SetFamily( FAMILY_DONTKNOW );
                break;
            case RTF_FCHARSET:
                if (-1 != nTokenValue)
                {
                    rtl_TextEncoding nrtl_TextEncoding = rtl_getTextEncodingFromWindowsCharset(
                        (sal_uInt8)nTokenValue);
                    pFont->SetCharSet(nrtl_TextEncoding);
                    //When we're in a font, the fontname is in the font
                    //charset, except for symbol fonts I believe
                    if (nrtl_TextEncoding == RTL_TEXTENCODING_SYMBOL)
                        nrtl_TextEncoding = RTL_TEXTENCODING_DONTKNOW;
                    SetEncoding(nrtl_TextEncoding);
                }
                break;
            case RTF_FPRQ:
                switch( nTokenValue )
                {
                    case 1:
                        pFont->SetPitch( PITCH_FIXED );
                        break;
                    case 2:
                        pFont->SetPitch( PITCH_VARIABLE );
                        break;
                }
                break;
            case RTF_F:
                bCheckNewFont = true;
                nInsFontNo = nFontNo;
                nFontNo = (short)nTokenValue;
                break;
            case RTF_FALT:
                bIsAltFntNm = true;
                break;
            case RTF_TEXTTOKEN:
                DelCharAtEnd( aToken, ';' );
                if ( !aToken.isEmpty() )
                {
                    if( bIsAltFntNm )
                        sAltNm = aToken;
                    else
                        sFntNm = aToken;
                }
                break;
        }

        if( bCheckNewFont && 1 >= _nOpenBrakets && !sFntNm.isEmpty() )  // one font is ready
        {
            // All data from the font is available, so off to the table
            if (!sAltNm.isEmpty())
                sFntNm = sFntNm + ";" + sAltNm;

            pFont->SetName( sFntNm );
            m_FontTable.insert(std::make_pair(nInsFontNo, std::move(pFont)));
            pFont.reset(new vcl::Font);
            pFont->SetCharSet( nSystemChar );
            sAltNm.clear();
            sFntNm.clear();
        }
    }
    // the last one we have to delete manually
    pFont.reset();
    SkipToken();        // the closing brace is evaluated "above"

    // set the default font in the Document
    if( bNewDoc && IsParserWorking() )
        SetDefault( RTF_DEFF, nDfltFont );
}

void SvxRTFParser::ReadBitmapData()
{
    SvRTFParser::ReadBitmapData();
}

void SvxRTFParser::ReadOLEData()
{
    SvRTFParser::ReadOLEData();
}

OUString& SvxRTFParser::GetTextToEndGroup( OUString& rStr )
{
    rStr.clear();
    int _nOpenBrakets = 1;  // the first was already detected earlier!!

    while( _nOpenBrakets && IsParserWorking() )
    {
        switch( GetNextToken() )
        {
        case '}':       --_nOpenBrakets;    break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    SkipToken();
                else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                    SkipToken( -2 );
                else
                {
                    // filter out at once
                    ReadUnknownData();
                    int nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++_nOpenBrakets;
            }
            break;

        case RTF_TEXTTOKEN:
            rStr += aToken;
            break;
        }
    }
    SkipToken();        // the closing brace is evaluated "above"
    return rStr;
}

util::DateTime SvxRTFParser::GetDateTimeStamp( )
{
    util::DateTime aDT;
    bool bContinue = true;

    while( bContinue && IsParserWorking() )
    {
        int nToken = GetNextToken();
        switch( nToken )
        {
        case RTF_YR:    aDT.Year = (sal_uInt16)nTokenValue;     break;
        case RTF_MO:    aDT.Month = (sal_uInt16)nTokenValue;    break;
        case RTF_DY:    aDT.Day = (sal_uInt16)nTokenValue;      break;
        case RTF_HR:    aDT.Hours = (sal_uInt16)nTokenValue;    break;
        case RTF_MIN:   aDT.Minutes = (sal_uInt16)nTokenValue;  break;
        default:
            bContinue = false;
        }
    }
    SkipToken();        // the closing brace is evaluated "above"
    return aDT;
}

void SvxRTFParser::ReadInfo()
{
    int _nOpenBrakets = 1;  // the first was already detected earlier!!
    DBG_ASSERT(m_xDocProps.is(),
        "SvxRTFParser::ReadInfo: no DocumentProperties");
    OUString sStr, sComment;
    long nVersNo = 0;

    while( _nOpenBrakets && IsParserWorking() )
    {
        int nToken = GetNextToken();
        switch( nToken )
        {
        case '}':       --_nOpenBrakets;    break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    SkipToken();
                else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                    SkipToken( -2 );
                else
                {
                    // filter out at once
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++_nOpenBrakets;
            }
            break;

        case RTF_TITLE:
            m_xDocProps->setTitle( GetTextToEndGroup( sStr ) );
            break;
        case RTF_SUBJECT:
            m_xDocProps->setSubject( GetTextToEndGroup( sStr ) );
            break;
        case RTF_AUTHOR:
            m_xDocProps->setAuthor( GetTextToEndGroup( sStr ) );
            break;
        case RTF_OPERATOR:
            m_xDocProps->setModifiedBy( GetTextToEndGroup( sStr ) );
            break;
        case RTF_KEYWORDS:
            {
                OUString sTemp = GetTextToEndGroup( sStr );
                m_xDocProps->setKeywords(
                    ::comphelper::string::convertCommaSeparated(sTemp) );
                break;
            }
        case RTF_DOCCOMM:
            m_xDocProps->setDescription( GetTextToEndGroup( sStr ) );
            break;

        case RTF_HLINKBASE:
            sBaseURL = GetTextToEndGroup( sStr ) ;
            break;

        case RTF_CREATIM:
            m_xDocProps->setCreationDate( GetDateTimeStamp() );
            break;

        case RTF_REVTIM:
            m_xDocProps->setModificationDate( GetDateTimeStamp() );
            break;

        case RTF_PRINTIM:
            m_xDocProps->setPrintDate( GetDateTimeStamp() );
            break;

        case RTF_COMMENT:
            GetTextToEndGroup( sComment );
            break;

        case RTF_BUPTIM:
            SkipGroup();
            break;

        case RTF_VERN:
            nVersNo = nTokenValue;
            break;

        case RTF_EDMINS:
        case RTF_ID:
        case RTF_VERSION:
        case RTF_NOFPAGES:
        case RTF_NOFWORDS:
        case RTF_NOFCHARS:
            NextToken( nToken );
            break;
        }
    }

    SkipToken();        // the closing brace is evaluated "above"
}


void SvxRTFParser::ClearColorTbl()
{
    while ( !aColorTbl.empty() )
    {
        delete aColorTbl.back();
        aColorTbl.pop_back();
    }
}

void SvxRTFParser::ClearFontTbl()
{
    m_FontTable.clear();
}

void SvxRTFParser::ClearStyleTbl()
{
    m_StyleTable.clear();
}

void SvxRTFParser::ClearAttrStack()
{
    for( size_t nCnt = aAttrStack.size(); nCnt; --nCnt )
    {
        SvxRTFItemStackType* pTmp = aAttrStack.back();
        aAttrStack.pop_back();
        delete pTmp;
    }
}

OUString& SvxRTFParser::DelCharAtEnd( OUString& rStr, const sal_Unicode cDel )
{
    if( !rStr.isEmpty() && ' ' == rStr[ 0 ])
        rStr = comphelper::string::stripStart(rStr, ' ');
    if( !rStr.isEmpty() && ' ' == rStr[ rStr.getLength()-1 ])
        rStr = comphelper::string::stripEnd(rStr, ' ');
    if( !rStr.isEmpty() && cDel == rStr[ rStr.getLength()-1 ])
        rStr = rStr.copy( 0, rStr.getLength()-1 );
    return rStr;
}


const vcl::Font& SvxRTFParser::GetFont( sal_uInt16 nId )
{
    SvxRTFFontTbl::const_iterator it = m_FontTable.find( nId );
    if (it != m_FontTable.end())
    {
        return *it->second;
    }
    const SvxFontItem& rDfltFont = static_cast<const SvxFontItem&>(
                    pAttrPool->GetDefaultItem( aPlainMap.nFont ));
    pDfltFont->SetName( rDfltFont.GetStyleName() );
    pDfltFont->SetFamily( rDfltFont.GetFamily() );
    return *pDfltFont;
}

SvxRTFItemStackType* SvxRTFParser::_GetAttrSet()
{
    SvxRTFItemStackType* pAkt = aAttrStack.empty() ? nullptr : aAttrStack.back();
    SvxRTFItemStackType* pNew;
    if( pAkt )
        pNew = new SvxRTFItemStackType( *pAkt, *pInsPos, false/*bCopyAttr*/ );
    else
        pNew = new SvxRTFItemStackType( *pAttrPool, &aWhichMap[0],
                                        *pInsPos );
    pNew->SetRTFDefaults( GetRTFDefaults() );

    aAttrStack.push_back( pNew );
    bNewGroup = false;
    return pNew;
}


void SvxRTFParser::_ClearStyleAttr( SvxRTFItemStackType& rStkType )
{
    // check attributes to the attributes of the stylesheet or to
    // the default attrs of the document
    SfxItemSet &rSet = rStkType.GetAttrSet();
    const SfxItemPool& rPool = *rSet.GetPool();
    const SfxPoolItem* pItem;
    SfxWhichIter aIter( rSet );

    if( !IsChkStyleAttr() ||
        !rStkType.GetAttrSet().Count() ||
        m_StyleTable.count( rStkType.nStyleNo ) == 0 )
    {
        for( sal_uInt16 nWhich = aIter.GetCurWhich(); nWhich; nWhich = aIter.NextWhich() )
        {
            if( SFX_WHICH_MAX > nWhich &&
                SfxItemState::SET == rSet.GetItemState( nWhich, false, &pItem ) &&
                     rPool.GetDefaultItem( nWhich ) == *pItem )
                rSet.ClearItem( nWhich );       // delete
        }
    }
    else
    {
        // Delete all Attributes, which are already defined in the Style,
        // from the current AttrSet.
        auto const& pStyle = m_StyleTable.find(rStkType.nStyleNo)->second;
        SfxItemSet &rStyleSet = pStyle->aAttrSet;
        const SfxPoolItem* pSItem;
        for( sal_uInt16 nWhich = aIter.GetCurWhich(); nWhich; nWhich = aIter.NextWhich() )
        {
            if( SfxItemState::SET == rStyleSet.GetItemState( nWhich, true, &pSItem ))
            {
                if( SfxItemState::SET == rSet.GetItemState( nWhich, false, &pItem )
                    && *pItem == *pSItem )
                    rSet.ClearItem( nWhich );       // delete
            }
            else if( SFX_WHICH_MAX > nWhich &&
                    SfxItemState::SET == rSet.GetItemState( nWhich, false, &pItem ) &&
                     rPool.GetDefaultItem( nWhich ) == *pItem )
                rSet.ClearItem( nWhich );       // delete
        }
    }
}

void SvxRTFParser::AttrGroupEnd()   // process the current, delete from Stack
{
    if( !aAttrStack.empty() )
    {
        SvxRTFItemStackType *pOld = aAttrStack.empty() ? nullptr : aAttrStack.back();
        aAttrStack.pop_back();
        SvxRTFItemStackType *pAkt = aAttrStack.empty() ? nullptr : aAttrStack.back();

        do {        // middle check loop
            sal_Int32 nOldSttNdIdx = pOld->pSttNd->GetIdx();
            if (!pOld->m_pChildList &&
                ((!pOld->aAttrSet.Count() && !pOld->nStyleNo ) ||
                (nOldSttNdIdx == pInsPos->GetNodeIdx() &&
                pOld->nSttCnt == pInsPos->GetCntIdx() )))
                break;          // no attributes or Area

            // set only the attributes that are different from the parent
            if( pAkt && pOld->aAttrSet.Count() )
            {
                SfxItemIter aIter( pOld->aAttrSet );
                const SfxPoolItem* pItem = aIter.GetCurItem(), *pGet;
                while( true )
                {
                    if( SfxItemState::SET == pAkt->aAttrSet.GetItemState(
                        pItem->Which(), false, &pGet ) &&
                        *pItem == *pGet )
                        pOld->aAttrSet.ClearItem( pItem->Which() );

                    if( aIter.IsAtEnd() )
                        break;
                    pItem = aIter.NextItem();
                }

                if (!pOld->aAttrSet.Count() && !pOld->m_pChildList &&
                    !pOld->nStyleNo )
                    break;
            }

            // Set all attributes which have been defined from start until here
            bool bCrsrBack = !pInsPos->GetCntIdx();
            if( bCrsrBack )
            {
                // at the beginning of a paragraph? Move back one position
                sal_Int32 nNd = pInsPos->GetNodeIdx();
                MovePos(false);
                // if can not move backward then later don't move forward !
                bCrsrBack = nNd != pInsPos->GetNodeIdx();
            }

            if( ( pOld->pSttNd->GetIdx() < pInsPos->GetNodeIdx() ||
                ( pOld->pSttNd->GetIdx() == pInsPos->GetNodeIdx() &&
                pOld->nSttCnt <= pInsPos->GetCntIdx() ))
                )
            {
                if( !bCrsrBack )
                {
                    // all pard attributes are only valid until the previous
                    // paragraph !!
                    if( nOldSttNdIdx == pInsPos->GetNodeIdx() )
                    {
                    }
                    else
                    {
                        // Now it gets complicated:
                        // - all character attributes sre keep the area
                        // - all paragraph attributes to get the area
                        //   up to the previous paragraph
                        ::std::unique_ptr<SvxRTFItemStackType> pNew(
                            new SvxRTFItemStackType(*pOld, *pInsPos, true));
                        pNew->aAttrSet.SetParent( pOld->aAttrSet.GetParent() );

                        // Delete all paragraph attributes from pNew
                        for( sal_uInt16 n = 0; n < (sizeof(aPardMap) / sizeof(sal_uInt16)) &&
                                            pNew->aAttrSet.Count(); ++n )
                            if( reinterpret_cast<sal_uInt16*>(&aPardMap)[n] )
                                pNew->aAttrSet.ClearItem( reinterpret_cast<sal_uInt16*>(&aPardMap)[n] );
                        pNew->SetRTFDefaults( GetRTFDefaults() );

                        // Were there any?
                        if( pNew->aAttrSet.Count() == pOld->aAttrSet.Count() )
                        {
                            pNew.reset();
                        }
                        else
                        {
                            pNew->nStyleNo = 0;

                            // Now span the real area of pNew from old
                            SetEndPrevPara( pOld->pEndNd, pOld->nEndCnt );
                            pNew->nSttCnt = 0;

                            if( IsChkStyleAttr() )
                            {
                                _ClearStyleAttr( *pOld );
                                _ClearStyleAttr( *pNew );   //#i10381#, methinks.
                            }

                            if( pAkt )
                            {
                                pAkt->Add(std::unique_ptr<SvxRTFItemStackType>(pOld));
                                pAkt->Add(std::move(pNew));
                            }
                            else
                            {
                                // Last off the stack, thus cache it until the next text was
                                // read. (Span no attributes!)

                                m_AttrSetList.push_back(std::unique_ptr<SvxRTFItemStackType>(pOld));
                                m_AttrSetList.push_back(std::move(pNew));
                            }
                            pOld = nullptr;   // Do not delete pOld
                            break;
                        }
                    }
                }

                pOld->pEndNd = pInsPos->MakeNodeIdx();
                pOld->nEndCnt = pInsPos->GetCntIdx();

                /*
                #i21422#
                If the parent (pAkt) sets something e.g. , and the child (pOld)
                unsets it and the style both are based on has it unset then
                clearing the pOld by looking at the style is clearly a disaster
                as the text ends up with pAkts bold and not pOlds no bold, this
                should be rethought out. For the moment its safest to just do
                the clean if we have no parent, all we suffer is too many
                redundant properties.
                */
                if (IsChkStyleAttr() && !pAkt)
                    _ClearStyleAttr( *pOld );

                if( pAkt )
                {
                    pAkt->Add(std::unique_ptr<SvxRTFItemStackType>(pOld));
                    // split up and create new entry, because it make no sense
                    // to create a "so long" depend list. Bug 95010
                    if (bCrsrBack && 50 < pAkt->m_pChildList->size())
                    {
                        // at the beginning of a paragraph? Move back one position
                        MovePos();
                        bCrsrBack = false;

                        // Open a new Group.
                        SvxRTFItemStackType* pNew = new SvxRTFItemStackType(
                                                *pAkt, *pInsPos, true );
                        pNew->SetRTFDefaults( GetRTFDefaults() );

                        // Set all until here valid Attributes
                        AttrGroupEnd();
                        pAkt = aAttrStack.empty() ? nullptr : aAttrStack.back();  // can be changed after AttrGroupEnd!
                        pNew->aAttrSet.SetParent( pAkt ? &pAkt->aAttrSet : nullptr );
                        aAttrStack.push_back( pNew );
                    }
                }
                else
                    // Last off the stack, thus cache it until the next text was
                    // read. (Span no attributes!)
                    m_AttrSetList.push_back(std::unique_ptr<SvxRTFItemStackType>(pOld));

                pOld = nullptr;
            }

            if( bCrsrBack )
                // at the beginning of a paragraph? Move back one position
                MovePos();

        } while( false );

        delete pOld;

        bNewGroup = false;
    }
}

void SvxRTFParser::SetAllAttrOfStk()        // end all Attr. and set it into doc
{
    // repeat until all attributes will be taken from stack
    while( !aAttrStack.empty() )
        AttrGroupEnd();

    for (size_t n = m_AttrSetList.size(); n; )
    {
        auto const& pStkSet = m_AttrSetList[--n];
        SetAttrSet( *pStkSet );
        m_AttrSetList.pop_back();
    }
}

// sets all the attributes that are different from the current
void SvxRTFParser::SetAttrSet( SvxRTFItemStackType &rSet )
{
    // Was DefTab never read? then set to default
    if( !bIsSetDfltTab )
        SetDefault( RTF_DEFTAB, 720 );

    if (rSet.m_pChildList)
        rSet.Compress( *this );
    if( rSet.aAttrSet.Count() || rSet.nStyleNo )
        SetAttrInDoc( rSet );

    // then process all the children
    if (rSet.m_pChildList)
        for (size_t n = 0; n < rSet.m_pChildList->size(); ++n)
            SetAttrSet( *(*rSet.m_pChildList)[ n ] );
}

// Has no text been inserted yet? (SttPos from the top Stack entry!)
bool SvxRTFParser::IsAttrSttPos()
{
    SvxRTFItemStackType* pAkt = aAttrStack.empty() ? nullptr : aAttrStack.back();
    return !pAkt || (pAkt->pSttNd->GetIdx() == pInsPos->GetNodeIdx() &&
        pAkt->nSttCnt == pInsPos->GetCntIdx());
}


void SvxRTFParser::SetAttrInDoc( SvxRTFItemStackType & )
{
}

void SvxRTFParser::BuildWhichTable()
{
    aWhichMap.clear();
    aWhichMap.push_back( 0 );

    // Building a Which-Map 'rWhichMap' from an array of
    // 'pWhichIds' from Which-Ids. It has the long 'nWhichIds'.
    // The Which-Map is not going to be deleted.
    SvParser::BuildWhichTable( aWhichMap, reinterpret_cast<sal_uInt16*>(&aPardMap), sizeof(aPardMap) / sizeof(sal_uInt16) );
    SvParser::BuildWhichTable( aWhichMap, reinterpret_cast<sal_uInt16*>(&aPlainMap), sizeof(aPlainMap) / sizeof(sal_uInt16) );
}

const SfxItemSet& SvxRTFParser::GetRTFDefaults()
{
    if( !pRTFDefaults )
    {
        pRTFDefaults = new SfxItemSet( *pAttrPool, &aWhichMap[0] );
        sal_uInt16 nId;
        if( 0 != ( nId = aPardMap.nScriptSpace ))
        {
            SvxScriptSpaceItem aItem( false, nId );
            if( bNewDoc )
                pAttrPool->SetPoolDefaultItem( aItem );
            else
                pRTFDefaults->Put( aItem );
        }
    }
    return *pRTFDefaults;
}


SvxRTFStyleType::SvxRTFStyleType( SfxItemPool& rPool, const sal_uInt16* pWhichRange )
    : aAttrSet( rPool, pWhichRange )
{
    nOutlineNo = sal_uInt8(-1);         // not set
    nBasedOn = 0;
    bBasedOnIsSet = false;          //$flr #117411#
    nNext = 0;
    bIsCharFmt = false;
}


SvxRTFItemStackType::SvxRTFItemStackType(
        SfxItemPool& rPool, const sal_uInt16* pWhichRange,
        const EditPosition& rPos )
    : aAttrSet( rPool, pWhichRange )
    , m_pChildList( nullptr )
    , nStyleNo( 0 )
{
    pSttNd = rPos.MakeNodeIdx();
    nSttCnt = rPos.GetCntIdx();
    pEndNd = pSttNd;
    nEndCnt = nSttCnt;
}

SvxRTFItemStackType::SvxRTFItemStackType(
        const SvxRTFItemStackType& rCpy,
        const EditPosition& rPos,
        bool const bCopyAttr )
    : aAttrSet( *rCpy.aAttrSet.GetPool(), rCpy.aAttrSet.GetRanges() )
    , m_pChildList( nullptr )
    , nStyleNo( rCpy.nStyleNo )
{
    pSttNd = rPos.MakeNodeIdx();
    nSttCnt = rPos.GetCntIdx();
    pEndNd = pSttNd;
    nEndCnt = nSttCnt;

    aAttrSet.SetParent( &rCpy.aAttrSet );
    if( bCopyAttr )
        aAttrSet.Put( rCpy.aAttrSet );
}

SvxRTFItemStackType::~SvxRTFItemStackType()
{
    if (m_pChildList)
        delete m_pChildList;
    if( pSttNd != pEndNd )
        delete pEndNd;
    delete pSttNd;
}

void SvxRTFItemStackType::Add(std::unique_ptr<SvxRTFItemStackType> pIns)
{
    if (!m_pChildList)
         m_pChildList = new SvxRTFItemStackList();
    m_pChildList->push_back(std::move(pIns));
}

void SvxRTFItemStackType::SetStartPos( const EditPosition& rPos )
{
    if (pSttNd != pEndNd)
        delete pEndNd;
    delete pSttNd;
    pSttNd = rPos.MakeNodeIdx();
    pEndNd = pSttNd;
    nSttCnt = rPos.GetCntIdx();
}

void SvxRTFItemStackType::MoveFullNode(const EditNodeIdx &rOldNode,
    const EditNodeIdx &rNewNode)
{
    bool bSameEndAsStart = (pSttNd == pEndNd);

    if (GetSttNodeIdx() == rOldNode.GetIdx())
    {
        delete pSttNd;
        pSttNd = rNewNode.Clone();
        if (bSameEndAsStart)
            pEndNd = pSttNd;
    }

    if (!bSameEndAsStart && GetEndNodeIdx() == rOldNode.GetIdx())
    {
        delete pEndNd;
        pEndNd = rNewNode.Clone();
    }

    //And the same for all the children
    sal_Int32 nCount = m_pChildList ? m_pChildList->size() : 0;
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        auto const& pStk = (*m_pChildList)[i];
        pStk->MoveFullNode(rOldNode, rNewNode);
    }
}

void SvxRTFItemStackType::Compress( const SvxRTFParser& rParser )
{
    ENSURE_OR_RETURN_VOID(m_pChildList, "Compress: no ChildList" );
    ENSURE_OR_RETURN_VOID(!m_pChildList->empty(), "Compress: ChildList empty");

    SvxRTFItemStackType* pTmp = (*m_pChildList)[0].get();

    if( !pTmp->aAttrSet.Count() ||
        pSttNd->GetIdx() != pTmp->pSttNd->GetIdx() ||
        nSttCnt != pTmp->nSttCnt )
        return;

    EditNodeIdx* pLastNd = pTmp->pEndNd;
    sal_Int32 nLastCnt = pTmp->nEndCnt;

    SfxItemSet aMrgSet( pTmp->aAttrSet );
    for (size_t n = 1; n < m_pChildList->size(); ++n)
    {
        pTmp = (*m_pChildList)[n].get();
        if (pTmp->m_pChildList)
            pTmp->Compress( rParser );

        if( !pTmp->nSttCnt
            ? (pLastNd->GetIdx()+1 != pTmp->pSttNd->GetIdx() ||
               !rParser.IsEndPara( pLastNd, nLastCnt ) )
            : ( pTmp->nSttCnt != nLastCnt ||
                pLastNd->GetIdx() != pTmp->pSttNd->GetIdx() ))
        {
            while (++n < m_pChildList->size())
            {
                pTmp = (*m_pChildList)[n].get();
                if (pTmp->m_pChildList)
                    pTmp->Compress( rParser );
            }
            return;
        }

        if( n )
        {
            // Search for all which are set over the whole area
            SfxItemIter aIter( aMrgSet );
            const SfxPoolItem* pItem;
            do {
                sal_uInt16 nWhich = aIter.GetCurItem()->Which();
                if( SfxItemState::SET != pTmp->aAttrSet.GetItemState( nWhich,
                      false, &pItem ) || *pItem != *aIter.GetCurItem() )
                    aMrgSet.ClearItem( nWhich );

                if( aIter.IsAtEnd() )
                    break;
                aIter.NextItem();
            } while( true );

            if( !aMrgSet.Count() )
                return;
        }

        pLastNd = pTmp->pEndNd;
        nLastCnt = pTmp->nEndCnt;
    }

    if( pEndNd->GetIdx() != pLastNd->GetIdx() || nEndCnt != nLastCnt )
        return;

    // It can be merged
    aAttrSet.Put( aMrgSet );

    for (size_t n = 0; n < m_pChildList->size(); ++n)
    {
        pTmp = (*m_pChildList)[n].get();
        pTmp->aAttrSet.Differentiate( aMrgSet );

        if (!pTmp->m_pChildList && !pTmp->aAttrSet.Count() && !pTmp->nStyleNo)
        {
            m_pChildList->erase( m_pChildList->begin() + n );
            --n;
        }
    }
    if (m_pChildList->empty())
    {
        delete m_pChildList;
        m_pChildList = nullptr;
    }
}
void SvxRTFItemStackType::SetRTFDefaults( const SfxItemSet& rDefaults )
{
    if( rDefaults.Count() )
    {
        SfxItemIter aIter( rDefaults );
        do {
            sal_uInt16 nWhich = aIter.GetCurItem()->Which();
            if( SfxItemState::SET != aAttrSet.GetItemState( nWhich, false ))
                aAttrSet.Put( *aIter.GetCurItem() );

            if( aIter.IsAtEnd() )
                break;
            aIter.NextItem();
        } while( true );
    }
}


RTFPlainAttrMapIds::RTFPlainAttrMapIds( const SfxItemPool& rPool )
{
    nCaseMap = rPool.GetTrueWhich( SID_ATTR_CHAR_CASEMAP, false );
    nBgColor = rPool.GetTrueWhich( SID_ATTR_BRUSH_CHAR, false );
    nColor = rPool.GetTrueWhich( SID_ATTR_CHAR_COLOR, false );
    nContour = rPool.GetTrueWhich( SID_ATTR_CHAR_CONTOUR, false );
    nCrossedOut = rPool.GetTrueWhich( SID_ATTR_CHAR_STRIKEOUT, false );
    nEscapement = rPool.GetTrueWhich( SID_ATTR_CHAR_ESCAPEMENT, false );
    nFont = rPool.GetTrueWhich( SID_ATTR_CHAR_FONT, false );
    nFontHeight = rPool.GetTrueWhich( SID_ATTR_CHAR_FONTHEIGHT, false );
    nKering = rPool.GetTrueWhich( SID_ATTR_CHAR_KERNING, false );
    nLanguage = rPool.GetTrueWhich( SID_ATTR_CHAR_LANGUAGE, false );
    nPosture = rPool.GetTrueWhich( SID_ATTR_CHAR_POSTURE, false );
    nShadowed = rPool.GetTrueWhich( SID_ATTR_CHAR_SHADOWED, false );
    nUnderline = rPool.GetTrueWhich( SID_ATTR_CHAR_UNDERLINE, false );
    nOverline = rPool.GetTrueWhich( SID_ATTR_CHAR_OVERLINE, false );
    nWeight = rPool.GetTrueWhich( SID_ATTR_CHAR_WEIGHT, false );
    nWordlineMode = rPool.GetTrueWhich( SID_ATTR_CHAR_WORDLINEMODE, false );
    nAutoKerning = rPool.GetTrueWhich( SID_ATTR_CHAR_AUTOKERN, false );

    nCJKFont = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_FONT, false );
    nCJKFontHeight = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT, false );
    nCJKLanguage = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_LANGUAGE, false );
    nCJKPosture = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_POSTURE, false );
    nCJKWeight = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_WEIGHT, false );
    nCTLFont = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_FONT, false );
    nCTLFontHeight = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_FONTHEIGHT, false );
    nCTLLanguage = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_LANGUAGE, false );
    nCTLPosture = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_POSTURE, false );
    nCTLWeight = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_WEIGHT, false );
    nEmphasis = rPool.GetTrueWhich( SID_ATTR_CHAR_EMPHASISMARK, false );
    nTwoLines = rPool.GetTrueWhich( SID_ATTR_CHAR_TWO_LINES, false );
    nRuby = 0; //rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_RUBY, sal_False );
    nCharScaleX = rPool.GetTrueWhich( SID_ATTR_CHAR_SCALEWIDTH, false );
    nHorzVert = rPool.GetTrueWhich( SID_ATTR_CHAR_ROTATED, false );
    nRelief = rPool.GetTrueWhich( SID_ATTR_CHAR_RELIEF, false );
    nHidden = rPool.GetTrueWhich( SID_ATTR_CHAR_HIDDEN, false );
}

RTFPardAttrMapIds ::RTFPardAttrMapIds ( const SfxItemPool& rPool )
{
    nLinespacing = rPool.GetTrueWhich( SID_ATTR_PARA_LINESPACE, false );
    nAdjust = rPool.GetTrueWhich( SID_ATTR_PARA_ADJUST, false );
    nTabStop = rPool.GetTrueWhich( SID_ATTR_TABSTOP, false );
    nHyphenzone = rPool.GetTrueWhich( SID_ATTR_PARA_HYPHENZONE, false );
    nLRSpace = rPool.GetTrueWhich( SID_ATTR_LRSPACE, false );
    nULSpace = rPool.GetTrueWhich( SID_ATTR_ULSPACE, false );
    nBrush = rPool.GetTrueWhich( SID_ATTR_BRUSH, false );
    nBox = rPool.GetTrueWhich( SID_ATTR_BORDER_OUTER, false );
    nShadow = rPool.GetTrueWhich( SID_ATTR_BORDER_SHADOW, false );
    nOutlineLvl = rPool.GetTrueWhich( SID_ATTR_PARA_OUTLLEVEL, false );
    nSplit = rPool.GetTrueWhich( SID_ATTR_PARA_SPLIT, false );
    nKeep = rPool.GetTrueWhich( SID_ATTR_PARA_KEEP, false );
    nFontAlign = rPool.GetTrueWhich( SID_PARA_VERTALIGN, false );
    nScriptSpace = rPool.GetTrueWhich( SID_ATTR_PARA_SCRIPTSPACE, false );
    nHangPunct = rPool.GetTrueWhich( SID_ATTR_PARA_HANGPUNCTUATION, false );
    nForbRule = rPool.GetTrueWhich( SID_ATTR_PARA_FORBIDDEN_RULES, false );
    nDirection = rPool.GetTrueWhich( SID_ATTR_FRAMEDIRECTION, false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
