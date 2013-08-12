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

#include <com/sun/star/document/XDocumentProperties.hpp>


using namespace ::com::sun::star;


static CharSet lcl_GetDefaultTextEncodingForRTF()
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
            int bReadNewDoc )
    : SvRTFParser( rIn, 5 ),
    rStrm(rIn),
    pInsPos( 0 ),
    pAttrPool( &rPool ),
    m_xDocProps( i_xDocProps ),
    pRTFDefaults( 0 ),
    nVersionNo( 0 )
{
    bNewDoc = bReadNewDoc;

    bChkStyleAttr = bCalcValue = bReadDocInfo = bIsInReadStyleTab = sal_False;
    bIsLeftToRightDef = sal_True;

    {
        RTFPlainAttrMapIds aTmp( rPool );
        aPlainMap.insert( aPlainMap.begin(), (sal_uInt16*)&aTmp,
                (sal_uInt16*)&aTmp + (sizeof( RTFPlainAttrMapIds ) / sizeof(sal_uInt16)) );
    }
    {
        RTFPardAttrMapIds aTmp( rPool );
        aPardMap.insert( aPardMap.begin(), (sal_uInt16*)&aTmp,
                (sal_uInt16*)&aTmp + (sizeof( RTFPardAttrMapIds ) / sizeof(sal_uInt16)) );
    }
    pDfltFont = new Font;
    pDfltColor = new Color;
}

void SvxRTFParser::EnterEnvironment()
{
}

void SvxRTFParser::LeaveEnvironment()
{
}

void SvxRTFParser::ResetPard()
{
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

void SvxRTFParser::SetInsPos( const SvxPosition& rNew )
{
    if( pInsPos )
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
    if( !aFontTbl.empty() )
        ClearFontTbl();
    if( !aStyleTbl.empty() )
        ClearStyleTbl();
    if( !aAttrStack.empty() )
        ClearAttrStack();

    bIsSetDfltTab = sal_False;
    bNewGroup = sal_False;
    nDfltFont = 0;

    sBaseURL.Erase();

    // generate the correct WhichId table from the set WhichIds.
    BuildWhichTbl();

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
                if( !aFontTbl.empty() )
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
        // no Break, aToken is set as Text
    case RTF_TEXTTOKEN:
        {
            InsertText();
            // all collected Attributes are set
            for( sal_uInt16 n = aAttrSetList.size(); n; )
            {
                SvxRTFItemStackType* pStkSet = &aAttrSetList[--n];
                SetAttrSet( *pStkSet );
                aAttrSetList.pop_back();
            }
        }
        break;


    case RTF_PAR:
        InsertPara();
        break;
    case '{':
        if (bNewGroup)          // Nesting!
            _GetAttrSet();
        EnterEnvironment();
        bNewGroup = true;
        break;
    case '}':
        if( !bNewGroup )        // Empty Group ??
            AttrGroupEnd();
        LeaveEnvironment();
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
    // Could also appear in the RTF-filewithout the IGNORE-Flag; all Groups
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
            nToken = SkipToken( -1 );
            if( '{' == GetStackPtr( -1 )->nTokenId )
                nToken = SkipToken( -1 );

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
                nToken = SkipToken( -1 );
                if( '{' == GetStackPtr( -1 )->nTokenId )
                {
                    nToken = SkipToken( -1 );
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
    int nToken, bSaveChkStyleAttr = bChkStyleAttr;
    sal_uInt16 nStyleNo = 0;
    int _nOpenBrakets = 1;      // the first was already detected earlier!!
    SvxRTFStyleType* pStyle = new SvxRTFStyleType( *pAttrPool, &aWhichMap[0] );
    pStyle->aAttrSet.Put( GetRTFDefaults() );

    bIsInReadStyleTab = sal_True;
    bChkStyleAttr = sal_False;      // Do not check Attribute against the Styles

    while( _nOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '}':       if( --_nOpenBrakets && IsParserWorking() )
                            // Style has been completely read,
                            // so this is still a stable status
                            SaveState( RTF_STYLESHEET );
                        break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ) &&
                            RTF_PN != nToken )
                    nToken = SkipToken( -2 );
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

        case RTF_SBASEDON:  pStyle->nBasedOn = sal_uInt16(nTokenValue); pStyle->bBasedOnIsSet=sal_True; break;
        case RTF_SNEXT:     pStyle->nNext = sal_uInt16(nTokenValue);    break;
        case RTF_OUTLINELEVEL:
        case RTF_SOUTLVL:   pStyle->nOutlineNo = sal_uInt8(nTokenValue);    break;
        case RTF_S:         nStyleNo = (short)nTokenValue;          break;
        case RTF_CS:        nStyleNo = (short)nTokenValue;
                            pStyle->bIsCharFmt = sal_True;
                            break;

        case RTF_TEXTTOKEN:
            {
                pStyle->sName = DelCharAtEnd( aToken, ';' );

                if( !aStyleTbl.empty() )
                {
                    aStyleTbl.erase(nStyleNo);
                }
                // All data from the font is available, so off to the table
                aStyleTbl.insert( nStyleNo , pStyle);
                pStyle = new SvxRTFStyleType( *pAttrPool, &aWhichMap[0] );
                pStyle->aAttrSet.Put( GetRTFDefaults() );
                nStyleNo = 0;
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
                    nToken = SkipToken( -1 );
                    if( '{' == GetStackPtr( -1 )->nTokenId )
                    {
                        nToken = SkipToken( -1 );
                    }
                }
                ReadAttr( nToken, &pStyle->aAttrSet );
                break;
            }
            break;
        }
    }
    delete pStyle;          // Delete the Last Style
    SkipToken( -1 );        // the closing brace is evaluated "above"

    // Flag back to old state
    bChkStyleAttr = bSaveChkStyleAttr;
    bIsInReadStyleTab = sal_False;
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

            // else no break !!

        case ';':
            if( IsParserWorking() )
            {
                // one color is finished, fill in the table
                // try to map the values to SV internal names
                ColorPtr pColor = new Color( nRed, nGreen, nBlue );
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
    SkipToken( -1 );        // the closing brace is evaluated "above"
}

void SvxRTFParser::ReadFontTable()
{
    int nToken;
    int _nOpenBrakets = 1;      // the first was already detected earlier!!
    Font* pFont = new Font();
    short nFontNo(0), nInsFontNo (0);
    String sAltNm, sFntNm;
    sal_Bool bIsAltFntNm = sal_False, bCheckNewFont;

    CharSet nSystemChar = lcl_GetDefaultTextEncodingForRTF();
    pFont->SetCharSet( nSystemChar );
    SetEncoding( nSystemChar );

    while( _nOpenBrakets && IsParserWorking() )
    {
        bCheckNewFont = sal_False;
        switch( ( nToken = GetNextToken() ))
        {
            case '}':
                bIsAltFntNm = sal_False;
                // Style has been completely read,
                // so this is still a stable status
                if( --_nOpenBrakets <= 1 && IsParserWorking() )
                    SaveState( RTF_FONTTBL );
                bCheckNewFont = sal_True;
                nInsFontNo = nFontNo;
                break;
            case '{':
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                // immediately skip unknown and all known but non-evaluated
                // groups
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ) &&
                        RTF_PANOSE != nToken && RTF_FNAME != nToken &&
                        RTF_FONTEMB != nToken && RTF_FONTFILE != nToken )
                    nToken = SkipToken( -2 );
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
            // for technical/symbolic font of the CharSet is changed!
            case RTF_FTECH:
                pFont->SetCharSet( RTL_TEXTENCODING_SYMBOL );
                // deliberate fall through
            case RTF_FNIL:
                pFont->SetFamily( FAMILY_DONTKNOW );
                break;
            case RTF_FCHARSET:
                if (-1 != nTokenValue)
                {
                    CharSet nCharSet = rtl_getTextEncodingFromWindowsCharset(
                        (sal_uInt8)nTokenValue);
                    pFont->SetCharSet(nCharSet);
                    //When we're in a font, the fontname is in the font
                    //charset, except for symbol fonts I believe
                    if (nCharSet == RTL_TEXTENCODING_SYMBOL)
                        nCharSet = RTL_TEXTENCODING_DONTKNOW;
                    SetEncoding(nCharSet);
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
                bCheckNewFont = sal_True;
                nInsFontNo = nFontNo;
                nFontNo = (short)nTokenValue;
                break;
            case RTF_FALT:
                bIsAltFntNm = sal_True;
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

        if( bCheckNewFont && 1 >= _nOpenBrakets && sFntNm.Len() )  // one font is ready
        {
            // All data from the font is available, so off to the table
            if (sAltNm.Len())
                (sFntNm += ';' ) += sAltNm;

            pFont->SetName( sFntNm );
            aFontTbl.insert( nInsFontNo, pFont );
            pFont = new Font();
            pFont->SetCharSet( nSystemChar );
            sAltNm.Erase();
            sFntNm.Erase();
        }
    }
    // the last one we have to delete manually
    delete pFont;
    SkipToken( -1 );        // the closing brace is evaluated "above"

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

String& SvxRTFParser::GetTextToEndGroup( String& rStr )
{
    rStr.Erase( 0 );
    int _nOpenBrakets = 1, nToken;  // the first was already detected earlier!!

    while( _nOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '}':       --_nOpenBrakets;    break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                    nToken = SkipToken( -2 );
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

        case RTF_TEXTTOKEN:
            rStr += aToken;
            break;
        }
    }
    SkipToken( -1 );        // the closing brace is evaluated "above"
    return rStr;
}

util::DateTime SvxRTFParser::GetDateTimeStamp( )
{
    util::DateTime aDT;
    sal_Bool bWeiter = sal_True;

    while( bWeiter && IsParserWorking() )
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
            bWeiter = sal_False;
        }
    }
    SkipToken( -1 );        // the closing brace is evaluated "above"
    return aDT;
}

void SvxRTFParser::ReadInfo( const sal_Char* pChkForVerNo )
{
    int _nOpenBrakets = 1, nToken;  // the first was already detected earlier!!
    DBG_ASSERT(m_xDocProps.is(),
        "SvxRTFParser::ReadInfo: no DocumentProperties");
    String sStr, sComment;
    long nVersNo = 0;

    while( _nOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '}':       --_nOpenBrakets;    break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                    nToken = SkipToken( -2 );
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

//      default:
        }
    }

    if( pChkForVerNo &&
        COMPARE_EQUAL == sComment.CompareToAscii( pChkForVerNo ))
        nVersionNo = nVersNo;

    SkipToken( -1 );        // the closing brace is evaluated "above"
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
    aFontTbl.clear();
}

void SvxRTFParser::ClearStyleTbl()
{
    aStyleTbl.clear();
}

void SvxRTFParser::ClearAttrStack()
{
    SvxRTFItemStackType* pTmp;
    for( size_t nCnt = aAttrStack.size(); nCnt; --nCnt )
    {
        pTmp = aAttrStack.back();
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


const Font& SvxRTFParser::GetFont( sal_uInt16 nId )
{
    SvxRTFFontTbl::const_iterator it = aFontTbl.find( nId );
    const Font* pFont;
    if( it == aFontTbl.end() )
    {
        const SvxFontItem& rDfltFont = (const SvxFontItem&)
                        pAttrPool->GetDefaultItem(
                    ((RTFPlainAttrMapIds*)&aPlainMap[0])->nFont );
        pDfltFont->SetName( rDfltFont.GetStyleName() );
        pDfltFont->SetFamily( rDfltFont.GetFamily() );
        pFont = pDfltFont;
    }
    else
        pFont = it->second;
    return *pFont;
}

SvxRTFItemStackType* SvxRTFParser::_GetAttrSet( int bCopyAttr )
{
    SvxRTFItemStackType* pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();
    SvxRTFItemStackType* pNew;
    if( pAkt )
        pNew = new SvxRTFItemStackType( *pAkt, *pInsPos, bCopyAttr );
    else
        pNew = new SvxRTFItemStackType( *pAttrPool, &aWhichMap[0],
                                        *pInsPos );
    pNew->SetRTFDefaults( GetRTFDefaults() );

    aAttrStack.push_back( pNew );
    bNewGroup = sal_False;
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
        aStyleTbl.count( rStkType.nStyleNo ) == 0 )
    {
        for( sal_uInt16 nWhich = aIter.GetCurWhich(); nWhich; nWhich = aIter.NextWhich() )
        {
            if( SFX_WHICH_MAX > nWhich &&
                SFX_ITEM_SET == rSet.GetItemState( nWhich, sal_False, &pItem ) &&
                     rPool.GetDefaultItem( nWhich ) == *pItem )
                rSet.ClearItem( nWhich );       // delete
        }
    }
    else
    {
        // Delete all Attributes, which are already defined in the Style,
        // from the current AttrSet.
        SvxRTFStyleType* pStyle = aStyleTbl.find( rStkType.nStyleNo )->second;
        SfxItemSet &rStyleSet = pStyle->aAttrSet;
        const SfxPoolItem* pSItem;
        for( sal_uInt16 nWhich = aIter.GetCurWhich(); nWhich; nWhich = aIter.NextWhich() )
        {
            if( SFX_ITEM_SET == rStyleSet.GetItemState( nWhich, sal_True, &pSItem ))
            {
                if( SFX_ITEM_SET == rSet.GetItemState( nWhich, sal_False, &pItem )
                    && *pItem == *pSItem )
                    rSet.ClearItem( nWhich );       // delete
            }
            else if( SFX_WHICH_MAX > nWhich &&
                    SFX_ITEM_SET == rSet.GetItemState( nWhich, sal_False, &pItem ) &&
                     rPool.GetDefaultItem( nWhich ) == *pItem )
                rSet.ClearItem( nWhich );       // delete
        }
    }
}

void SvxRTFParser::AttrGroupEnd()   // process the current, delete from Stack
{
    if( !aAttrStack.empty() )
    {
        SvxRTFItemStackType *pOld = aAttrStack.empty() ? 0 : aAttrStack.back();
        aAttrStack.pop_back();
        SvxRTFItemStackType *pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();

        do {        // middle check loop
            sal_Int32 nOldSttNdIdx = pOld->pSttNd->GetIdx();
            if( !pOld->pChildList &&
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
                    if( SFX_ITEM_SET == pAkt->aAttrSet.GetItemState(
                        pItem->Which(), sal_False, &pGet ) &&
                        *pItem == *pGet )
                        pOld->aAttrSet.ClearItem( pItem->Which() );

                    if( aIter.IsAtEnd() )
                        break;
                    pItem = aIter.NextItem();
                }

                if( !pOld->aAttrSet.Count() && !pOld->pChildList &&
                    !pOld->nStyleNo )
                    break;
            }

            // Set all attributes which have been defined from start until here
            int bCrsrBack = !pInsPos->GetCntIdx();
            if( bCrsrBack )
            {
                // at the beginning of a paragraph? Move back one position
                sal_Int32 nNd = pInsPos->GetNodeIdx();
                MovePos( sal_False );
                // if can not move backward then later dont move forward !
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
                        SvxRTFItemStackType* pNew = new SvxRTFItemStackType(
                                    *pOld, *pInsPos, sal_True );
                        pNew->aAttrSet.SetParent( pOld->aAttrSet.GetParent() );

                        // Delete all paragraph attributes from pNew
                        for( sal_uInt16 n = 0; n < aPardMap.size() &&
                                            pNew->aAttrSet.Count(); ++n )
                            if( aPardMap[n] )
                                pNew->aAttrSet.ClearItem( aPardMap[n] );
                        pNew->SetRTFDefaults( GetRTFDefaults() );

                        // Were there any?
                        if( pNew->aAttrSet.Count() == pOld->aAttrSet.Count() )
                            delete pNew;
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
                                pAkt->Add( pOld );
                                pAkt->Add( pNew );
                            }
                            else
                            {
                                // Last off the stack, thus cache it until the next text was
                                // read. (Span no attributes!)

                                aAttrSetList.push_back( pOld );
                                aAttrSetList.push_back( pNew );
                            }
                            pOld = 0;   // Do not delete pOld
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
                    pAkt->Add( pOld );
                    // split up and create new entry, because it make no sense
                    // to create a "so long" depend list. Bug 95010
                    if( bCrsrBack && 50 < pAkt->pChildList->size() )
                    {
                        // at the beginning of a paragraph? Move back one position
                        MovePos( sal_True );
                        bCrsrBack = sal_False;

                        // Open a new Group.
                        SvxRTFItemStackType* pNew = new SvxRTFItemStackType(
                                                *pAkt, *pInsPos, sal_True );
                        pNew->SetRTFDefaults( GetRTFDefaults() );

                        // Set all until here valid Attributes
                        AttrGroupEnd();
                        pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();  // can be changed after AttrGroupEnd!
                        pNew->aAttrSet.SetParent( pAkt ? &pAkt->aAttrSet : 0 );
                        aAttrStack.push_back( pNew );
                        pAkt = pNew;
                    }
                }
                else
                    // Last off the stack, thus cache it until the next text was
                    // read. (Span no attributes!)
                    aAttrSetList.push_back( pOld );

                pOld = 0;
            }

            if( bCrsrBack )
                // at the beginning of a paragraph? Move back one position
                MovePos( sal_True );

        } while( false );

        if( pOld )
            delete pOld;

        bNewGroup = sal_False;
    }
}

void SvxRTFParser::SetAllAttrOfStk()        // end all Attr. and set it into doc
{
    // repeat until all attributes will be taken from stack
    while( !aAttrStack.empty() )
        AttrGroupEnd();

    for( sal_uInt16 n = aAttrSetList.size(); n; )
    {
        SvxRTFItemStackType* pStkSet = &aAttrSetList[--n];
        SetAttrSet( *pStkSet );
        aAttrSetList.pop_back();
    }
}

// sets all the attributes that are different from the current
void SvxRTFParser::SetAttrSet( SvxRTFItemStackType &rSet )
{
    // Was DefTab never read? then set to default
    if( !bIsSetDfltTab )
        SetDefault( RTF_DEFTAB, 720 );

    if( rSet.pChildList )
        rSet.Compress( *this );
    if( rSet.aAttrSet.Count() || rSet.nStyleNo )
        SetAttrInDoc( rSet );

    // then process all the children
    if( rSet.pChildList )
        for( sal_uInt16 n = 0; n < rSet.pChildList->size(); ++n )
            SetAttrSet( (*rSet.pChildList)[ n ] );
}

    // Has no Text been inserted yet? (SttPos from the top Stack entry!)
int SvxRTFParser::IsAttrSttPos()
{
    SvxRTFItemStackType* pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();
    return !pAkt || (pAkt->pSttNd->GetIdx() == pInsPos->GetNodeIdx() &&
        pAkt->nSttCnt == pInsPos->GetCntIdx());
}


void SvxRTFParser::SetAttrInDoc( SvxRTFItemStackType & )
{
}

void SvxRTFParser::BuildWhichTbl()
{
    aWhichMap.clear();
    aWhichMap.push_back( 0 );

    // Building a Which-Map 'rWhichMap' from an Array of
    // 'pWhichIds' frm Which-Ids. It has the long 'nWhichIds'.
    // The Which-Map is not going to be deleted.
    SvParser::BuildWhichTbl( aWhichMap, (sal_uInt16*)&aPardMap[0], aPardMap.size() );
    SvParser::BuildWhichTbl( aWhichMap, (sal_uInt16*)&aPlainMap[0], aPlainMap.size() );
}

const SfxItemSet& SvxRTFParser::GetRTFDefaults()
{
    if( !pRTFDefaults )
    {
        pRTFDefaults = new SfxItemSet( *pAttrPool, &aWhichMap[0] );
        sal_uInt16 nId;
        if( 0 != ( nId = ((RTFPardAttrMapIds*)&aPardMap[0])->nScriptSpace ))
        {
            SvxScriptSpaceItem aItem( sal_False, nId );
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
    bBasedOnIsSet = sal_False;          //$flr #117411#
    nNext = 0;
    bIsCharFmt = sal_False;
}


SvxRTFItemStackType::SvxRTFItemStackType(
        SfxItemPool& rPool, const sal_uInt16* pWhichRange,
        const SvxPosition& rPos )
    : aAttrSet( rPool, pWhichRange ),
    pChildList( 0 ),
    nStyleNo( 0 )
{
    pSttNd = rPos.MakeNodeIdx();
    nSttCnt = rPos.GetCntIdx();
    pEndNd = pSttNd;
    nEndCnt = nSttCnt;
}

SvxRTFItemStackType::SvxRTFItemStackType(
        const SvxRTFItemStackType& rCpy,
        const SvxPosition& rPos,
        int bCopyAttr )
    : aAttrSet( *rCpy.aAttrSet.GetPool(), rCpy.aAttrSet.GetRanges() ),
    pChildList( 0 ),
    nStyleNo( rCpy.nStyleNo )
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
    if( pChildList )
        delete pChildList;
    if( pSttNd != pEndNd )
        delete pEndNd;
    delete pSttNd;
}

void SvxRTFItemStackType::Add( SvxRTFItemStackType* pIns )
{
    if( !pChildList )
         pChildList = new SvxRTFItemStackList();
    pChildList->push_back( pIns );
}

void SvxRTFItemStackType::SetStartPos( const SvxPosition& rPos )
{
    if (pSttNd != pEndNd)
        delete pEndNd;
    delete pSttNd;
    pSttNd = rPos.MakeNodeIdx();
    pEndNd = pSttNd;
    nSttCnt = rPos.GetCntIdx();
}

void SvxRTFItemStackType::MoveFullNode(const SvxNodeIdx &rOldNode,
    const SvxNodeIdx &rNewNode)
{
    bool bSameEndAsStart = (pSttNd == pEndNd) ? true : false;

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
    sal_Int32 nCount = pChildList ? pChildList->size() : 0;
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        SvxRTFItemStackType* pStk = &(*pChildList)[i];
        pStk->MoveFullNode(rOldNode, rNewNode);
    }
}

bool SvxRTFParser::UncompressableStackEntry(const SvxRTFItemStackType &) const
{
    return false;
}

void SvxRTFItemStackType::Compress( const SvxRTFParser& rParser )
{
    ENSURE_OR_RETURN_VOID(pChildList, "Compress: no ChildList" );
    ENSURE_OR_RETURN_VOID(!pChildList->empty(), "Compress: ChildList empty");

    sal_uInt16 n;
    SvxRTFItemStackType* pTmp = &(*pChildList)[0];

    if( !pTmp->aAttrSet.Count() ||
        pSttNd->GetIdx() != pTmp->pSttNd->GetIdx() ||
        nSttCnt != pTmp->nSttCnt )
        return;

    SvxNodeIdx* pLastNd = pTmp->pEndNd;
    xub_StrLen nLastCnt = pTmp->nEndCnt;

    SfxItemSet aMrgSet( pTmp->aAttrSet );
    for( n = 1; n < pChildList->size(); ++n )
    {
        pTmp = &(*pChildList)[n];
        if( pTmp->pChildList )
            pTmp->Compress( rParser );

        if( !pTmp->nSttCnt
            ? (pLastNd->GetIdx()+1 != pTmp->pSttNd->GetIdx() ||
               !rParser.IsEndPara( pLastNd, nLastCnt ) )
            : ( pTmp->nSttCnt != nLastCnt ||
                pLastNd->GetIdx() != pTmp->pSttNd->GetIdx() ))
        {
            while( ++n < pChildList->size() )
                if( (pTmp = &(*pChildList)[n])->pChildList )
                    pTmp->Compress( rParser );
            return;
        }

        if (rParser.UncompressableStackEntry(*pTmp))
            return;

        if( n )
        {
            // Search for all which are set over the whole area
            SfxItemIter aIter( aMrgSet );
            const SfxPoolItem* pItem;
            do {
                sal_uInt16 nWhich = aIter.GetCurItem()->Which();
                if( SFX_ITEM_SET != pTmp->aAttrSet.GetItemState( nWhich,
                      sal_False, &pItem ) || *pItem != *aIter.GetCurItem() )
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

    for( n = 0; n < pChildList->size(); ++n )
    {
        pTmp = &(*pChildList)[n];
        pTmp->aAttrSet.Differentiate( aMrgSet );

        if( !pTmp->pChildList && !pTmp->aAttrSet.Count() && !pTmp->nStyleNo )
        {
            pChildList->erase( pChildList->begin() + n );
            --n;
            continue;
        }
    }
    if( pChildList->empty() )
    {
        delete pChildList;
        pChildList = 0;
    }
}
void SvxRTFItemStackType::SetRTFDefaults( const SfxItemSet& rDefaults )
{
    if( rDefaults.Count() )
    {
        SfxItemIter aIter( rDefaults );
        do {
            sal_uInt16 nWhich = aIter.GetCurItem()->Which();
            if( SFX_ITEM_SET != aAttrSet.GetItemState( nWhich, sal_False ))
                aAttrSet.Put( *aIter.GetCurItem() );

            if( aIter.IsAtEnd() )
                break;
            aIter.NextItem();
        } while( true );
    }
}


RTFPlainAttrMapIds::RTFPlainAttrMapIds( const SfxItemPool& rPool )
{
    nCaseMap = rPool.GetTrueWhich( SID_ATTR_CHAR_CASEMAP, sal_False );
    nBgColor = rPool.GetTrueWhich( SID_ATTR_BRUSH_CHAR, sal_False );
    nColor = rPool.GetTrueWhich( SID_ATTR_CHAR_COLOR, sal_False );
    nContour = rPool.GetTrueWhich( SID_ATTR_CHAR_CONTOUR, sal_False );
    nCrossedOut = rPool.GetTrueWhich( SID_ATTR_CHAR_STRIKEOUT, sal_False );
    nEscapement = rPool.GetTrueWhich( SID_ATTR_CHAR_ESCAPEMENT, sal_False );
    nFont = rPool.GetTrueWhich( SID_ATTR_CHAR_FONT, sal_False );
    nFontHeight = rPool.GetTrueWhich( SID_ATTR_CHAR_FONTHEIGHT, sal_False );
    nKering = rPool.GetTrueWhich( SID_ATTR_CHAR_KERNING, sal_False );
    nLanguage = rPool.GetTrueWhich( SID_ATTR_CHAR_LANGUAGE, sal_False );
    nPosture = rPool.GetTrueWhich( SID_ATTR_CHAR_POSTURE, sal_False );
    nShadowed = rPool.GetTrueWhich( SID_ATTR_CHAR_SHADOWED, sal_False );
    nUnderline = rPool.GetTrueWhich( SID_ATTR_CHAR_UNDERLINE, sal_False );
    nOverline = rPool.GetTrueWhich( SID_ATTR_CHAR_OVERLINE, sal_False );
    nWeight = rPool.GetTrueWhich( SID_ATTR_CHAR_WEIGHT, sal_False );
    nWordlineMode = rPool.GetTrueWhich( SID_ATTR_CHAR_WORDLINEMODE, sal_False );
    nAutoKerning = rPool.GetTrueWhich( SID_ATTR_CHAR_AUTOKERN, sal_False );

    nCJKFont = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_FONT, sal_False );
    nCJKFontHeight = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT, sal_False );
    nCJKLanguage = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_LANGUAGE, sal_False );
    nCJKPosture = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_POSTURE, sal_False );
    nCJKWeight = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_WEIGHT, sal_False );
    nCTLFont = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_FONT, sal_False );
    nCTLFontHeight = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_FONTHEIGHT, sal_False );
    nCTLLanguage = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_LANGUAGE, sal_False );
    nCTLPosture = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_POSTURE, sal_False );
    nCTLWeight = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_WEIGHT, sal_False );
    nEmphasis = rPool.GetTrueWhich( SID_ATTR_CHAR_EMPHASISMARK, sal_False );
    nTwoLines = rPool.GetTrueWhich( SID_ATTR_CHAR_TWO_LINES, sal_False );
    nRuby = 0; //rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_RUBY, sal_False );
    nCharScaleX = rPool.GetTrueWhich( SID_ATTR_CHAR_SCALEWIDTH, sal_False );
    nHorzVert = rPool.GetTrueWhich( SID_ATTR_CHAR_ROTATED, sal_False );
    nRelief = rPool.GetTrueWhich( SID_ATTR_CHAR_RELIEF, sal_False );
    nHidden = rPool.GetTrueWhich( SID_ATTR_CHAR_HIDDEN, sal_False );
}

RTFPardAttrMapIds ::RTFPardAttrMapIds ( const SfxItemPool& rPool )
{
    nLinespacing = rPool.GetTrueWhich( SID_ATTR_PARA_LINESPACE, sal_False );
    nAdjust = rPool.GetTrueWhich( SID_ATTR_PARA_ADJUST, sal_False );
    nTabStop = rPool.GetTrueWhich( SID_ATTR_TABSTOP, sal_False );
    nHyphenzone = rPool.GetTrueWhich( SID_ATTR_PARA_HYPHENZONE, sal_False );
    nLRSpace = rPool.GetTrueWhich( SID_ATTR_LRSPACE, sal_False );
    nULSpace = rPool.GetTrueWhich( SID_ATTR_ULSPACE, sal_False );
    nBrush = rPool.GetTrueWhich( SID_ATTR_BRUSH, sal_False );
    nBox = rPool.GetTrueWhich( SID_ATTR_BORDER_OUTER, sal_False );
    nShadow = rPool.GetTrueWhich( SID_ATTR_BORDER_SHADOW, sal_False );
    nOutlineLvl = rPool.GetTrueWhich( SID_ATTR_PARA_OUTLLEVEL, sal_False );
    nSplit = rPool.GetTrueWhich( SID_ATTR_PARA_SPLIT, sal_False );
    nKeep = rPool.GetTrueWhich( SID_ATTR_PARA_KEEP, sal_False );
    nFontAlign = rPool.GetTrueWhich( SID_PARA_VERTALIGN, sal_False );
    nScriptSpace = rPool.GetTrueWhich( SID_ATTR_PARA_SCRIPTSPACE, sal_False );
    nHangPunct = rPool.GetTrueWhich( SID_ATTR_PARA_HANGPUNCTUATION, sal_False );
    nForbRule = rPool.GetTrueWhich( SID_ATTR_PARA_FORBIDDEN_RULES, sal_False );
    nDirection = rPool.GetTrueWhich( SID_ATTR_FRAMEDIRECTION, sal_False );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
