/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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



SvxRTFParser::SvxRTFParser( SfxItemPool& rPool, SvStream& rIn,
            uno::Reference<document::XDocumentProperties> i_xDocProps,
            int bReadNewDoc )
    : SvRTFParser( rIn, 5 )
    , rStrm(rIn)
    , pInsPos( 0 )
    , pAttrPool( &rPool )
    , m_xDocProps( i_xDocProps )
    , pRTFDefaults( 0 )
    , nVersionNo( 0 )
    , nDfltFont( 0)
    , bNewDoc( bReadNewDoc )
    , bNewGroup( sal_False)
    , bIsSetDfltTab( sal_False)
    , bChkStyleAttr( sal_False )
    , bCalcValue( sal_False )
    , bPardTokenRead( sal_False)
    , bReadDocInfo( sal_False )
    , bIsLeftToRightDef( sal_True)
    , bIsInReadStyleTab( sal_False)
{

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

    sBaseURL = "";

    
    BuildWhichTbl();

    return SvRTFParser::CallParser();
}

void SvxRTFParser::Continue( int nToken )
{
    SvRTFParser::Continue( nToken );

    if( SVPAR_PENDING != GetStatus() )
    {
        SetAllAttrOfStk();
    
    
     }
}



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
                    
                    SetDefault( nToken, nTokenValue );
                else
                    
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
        
    case RTF_TEXTTOKEN:
        {
            InsertText();
            
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
        if (bNewGroup)          
            _GetAttrSet();
        EnterEnvironment();
        bNewGroup = true;
        break;
    case '}':
        if( !bNewGroup )        
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
    
                            SkipGroup();
                            break;
    

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
        case RTF_PARFMT:        
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
    int _nOpenBrakets = 1;      
    SvxRTFStyleType* pStyle = new SvxRTFStyleType( *pAttrPool, &aWhichMap[0] );
    pStyle->aAttrSet.Put( GetRTFDefaults() );

    bIsInReadStyleTab = sal_True;
    bChkStyleAttr = sal_False;      

    while( _nOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '}':       if( --_nOpenBrakets && IsParserWorking() )
                            
                            
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
                
                aStyleTbl.insert( nStyleNo , pStyle);
                pStyle = new SvxRTFStyleType( *pAttrPool, &aWhichMap[0] );
                pStyle->aAttrSet.Put( GetRTFDefaults() );
                nStyleNo = 0;
            }
            break;
        default:
            switch( nToken & ~(0xff | RTF_SWGDEFS) )
            {
            case RTF_PARFMT:        
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
    delete pStyle;          
    SkipToken( -1 );        

    
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
                break;      

            

        case ';':
            if( IsParserWorking() )
            {
                
                
                ColorPtr pColor = new Color( nRed, nGreen, nBlue );
                if( aColorTbl.empty() &&
                    sal_uInt8(-1) == nRed && sal_uInt8(-1) == nGreen && sal_uInt8(-1) == nBlue )
                    pColor->SetColor( COL_AUTO );
                aColorTbl.push_back( pColor );
                nRed = 0, nGreen = 0, nBlue = 0;

                
                
                SaveState( RTF_COLORTBL );
            }
            break;
        }
    }
    SkipToken( -1 );        
}

void SvxRTFParser::ReadFontTable()
{
    int nToken;
    int _nOpenBrakets = 1;      
    Font* pFont = new Font();
    short nFontNo(0), nInsFontNo (0);
    OUString sAltNm, sFntNm;
    sal_Bool bIsAltFntNm = sal_False, bCheckNewFont;

    rtl_TextEncoding nSystemChar = lcl_GetDefaultTextEncodingForRTF();
    pFont->SetCharSet( nSystemChar );
    SetEncoding( nSystemChar );

    while( _nOpenBrakets && IsParserWorking() )
    {
        bCheckNewFont = sal_False;
        switch( ( nToken = GetNextToken() ))
        {
            case '}':
                bIsAltFntNm = sal_False;
                
                
                if( --_nOpenBrakets <= 1 && IsParserWorking() )
                    SaveState( RTF_FONTTBL );
                bCheckNewFont = sal_True;
                nInsFontNo = nFontNo;
                break;
            case '{':
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                
                
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ) &&
                        RTF_PANOSE != nToken && RTF_FNAME != nToken &&
                        RTF_FONTEMB != nToken && RTF_FONTFILE != nToken )
                    nToken = SkipToken( -2 );
                else
                {
                    
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
            
            case RTF_FTECH:
                pFont->SetCharSet( RTL_TEXTENCODING_SYMBOL );
                
            case RTF_FNIL:
                pFont->SetFamily( FAMILY_DONTKNOW );
                break;
            case RTF_FCHARSET:
                if (-1 != nTokenValue)
                {
                    rtl_TextEncoding nrtl_TextEncoding = rtl_getTextEncodingFromWindowsCharset(
                        (sal_uInt8)nTokenValue);
                    pFont->SetCharSet(nrtl_TextEncoding);
                    
                    
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

        if( bCheckNewFont && 1 >= _nOpenBrakets && !sFntNm.isEmpty() )  
        {
            
            if (!sAltNm.isEmpty())
                sFntNm = sFntNm + ";" + sAltNm;

            pFont->SetName( sFntNm );
            aFontTbl.insert( nInsFontNo, pFont );
            pFont = new Font();
            pFont->SetCharSet( nSystemChar );
            sAltNm = "";
            sFntNm = "";
        }
    }
    
    delete pFont;
    SkipToken( -1 );        

    
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
    rStr = "";
    int _nOpenBrakets = 1, nToken;  

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
    SkipToken( -1 );        
    return rStr;
}

util::DateTime SvxRTFParser::GetDateTimeStamp( )
{
    util::DateTime aDT;
    sal_Bool bContinue = sal_True;

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
            bContinue = sal_False;
        }
    }
    SkipToken( -1 );        
    return aDT;
}

void SvxRTFParser::ReadInfo( const sal_Char* pChkForVerNo )
{
    int _nOpenBrakets = 1, nToken;  
    DBG_ASSERT(m_xDocProps.is(),
        "SvxRTFParser::ReadInfo: no DocumentProperties");
    OUString sStr, sComment;
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

    if( pChkForVerNo &&
        sComment == OUString::createFromAscii( pChkForVerNo ) )
        nVersionNo = nVersNo;

    SkipToken( -1 );        
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
                SFX_ITEM_SET == rSet.GetItemState( nWhich, false, &pItem ) &&
                     rPool.GetDefaultItem( nWhich ) == *pItem )
                rSet.ClearItem( nWhich );       
        }
    }
    else
    {
        
        
        SvxRTFStyleType* pStyle = aStyleTbl.find( rStkType.nStyleNo )->second;
        SfxItemSet &rStyleSet = pStyle->aAttrSet;
        const SfxPoolItem* pSItem;
        for( sal_uInt16 nWhich = aIter.GetCurWhich(); nWhich; nWhich = aIter.NextWhich() )
        {
            if( SFX_ITEM_SET == rStyleSet.GetItemState( nWhich, true, &pSItem ))
            {
                if( SFX_ITEM_SET == rSet.GetItemState( nWhich, false, &pItem )
                    && *pItem == *pSItem )
                    rSet.ClearItem( nWhich );       
            }
            else if( SFX_WHICH_MAX > nWhich &&
                    SFX_ITEM_SET == rSet.GetItemState( nWhich, false, &pItem ) &&
                     rPool.GetDefaultItem( nWhich ) == *pItem )
                rSet.ClearItem( nWhich );       
        }
    }
}

void SvxRTFParser::AttrGroupEnd()   
{
    if( !aAttrStack.empty() )
    {
        SvxRTFItemStackType *pOld = aAttrStack.empty() ? 0 : aAttrStack.back();
        aAttrStack.pop_back();
        SvxRTFItemStackType *pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();

        do {        
            sal_Int32 nOldSttNdIdx = pOld->pSttNd->GetIdx();
            if( !pOld->pChildList &&
                ((!pOld->aAttrSet.Count() && !pOld->nStyleNo ) ||
                (nOldSttNdIdx == pInsPos->GetNodeIdx() &&
                pOld->nSttCnt == pInsPos->GetCntIdx() )))
                break;          

            
            if( pAkt && pOld->aAttrSet.Count() )
            {
                SfxItemIter aIter( pOld->aAttrSet );
                const SfxPoolItem* pItem = aIter.GetCurItem(), *pGet;
                while( true )
                {
                    if( SFX_ITEM_SET == pAkt->aAttrSet.GetItemState(
                        pItem->Which(), false, &pGet ) &&
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

            
            bool bCrsrBack = !pInsPos->GetCntIdx();
            if( bCrsrBack )
            {
                
                sal_Int32 nNd = pInsPos->GetNodeIdx();
                MovePos( sal_False );
                
                bCrsrBack = nNd != pInsPos->GetNodeIdx();
            }

            if( ( pOld->pSttNd->GetIdx() < pInsPos->GetNodeIdx() ||
                ( pOld->pSttNd->GetIdx() == pInsPos->GetNodeIdx() &&
                pOld->nSttCnt <= pInsPos->GetCntIdx() ))
                )
            {
                if( !bCrsrBack )
                {
                    
                    
                    if( nOldSttNdIdx == pInsPos->GetNodeIdx() )
                    {
                    }
                    else
                    {
                        
                        
                        
                        
                        SvxRTFItemStackType* pNew = new SvxRTFItemStackType(
                                    *pOld, *pInsPos, sal_True );
                        pNew->aAttrSet.SetParent( pOld->aAttrSet.GetParent() );

                        
                        for( sal_uInt16 n = 0; n < aPardMap.size() &&
                                            pNew->aAttrSet.Count(); ++n )
                            if( aPardMap[n] )
                                pNew->aAttrSet.ClearItem( aPardMap[n] );
                        pNew->SetRTFDefaults( GetRTFDefaults() );

                        
                        if( pNew->aAttrSet.Count() == pOld->aAttrSet.Count() )
                            delete pNew;
                        else
                        {
                            pNew->nStyleNo = 0;

                            
                            SetEndPrevPara( pOld->pEndNd, pOld->nEndCnt );
                            pNew->nSttCnt = 0;

                            if( IsChkStyleAttr() )
                            {
                                _ClearStyleAttr( *pOld );
                                _ClearStyleAttr( *pNew );   
                            }

                            if( pAkt )
                            {
                                pAkt->Add( pOld );
                                pAkt->Add( pNew );
                            }
                            else
                            {
                                
                                

                                aAttrSetList.push_back( pOld );
                                aAttrSetList.push_back( pNew );
                            }
                            pOld = 0;   
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
                    
                    
                    if( bCrsrBack && 50 < pAkt->pChildList->size() )
                    {
                        
                        MovePos( sal_True );
                        bCrsrBack = false;

                        
                        SvxRTFItemStackType* pNew = new SvxRTFItemStackType(
                                                *pAkt, *pInsPos, sal_True );
                        pNew->SetRTFDefaults( GetRTFDefaults() );

                        
                        AttrGroupEnd();
                        pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();  
                        pNew->aAttrSet.SetParent( pAkt ? &pAkt->aAttrSet : 0 );
                        aAttrStack.push_back( pNew );
                        pAkt = pNew;
                    }
                }
                else
                    
                    
                    aAttrSetList.push_back( pOld );

                pOld = 0;
            }

            if( bCrsrBack )
                
                MovePos( sal_True );

        } while( false );

        if( pOld )
            delete pOld;

        bNewGroup = sal_False;
    }
}

void SvxRTFParser::SetAllAttrOfStk()        
{
    
    while( !aAttrStack.empty() )
        AttrGroupEnd();

    for( sal_uInt16 n = aAttrSetList.size(); n; )
    {
        SvxRTFItemStackType* pStkSet = &aAttrSetList[--n];
        SetAttrSet( *pStkSet );
        aAttrSetList.pop_back();
    }
}


void SvxRTFParser::SetAttrSet( SvxRTFItemStackType &rSet )
{
    
    if( !bIsSetDfltTab )
        SetDefault( RTF_DEFTAB, 720 );

    if( rSet.pChildList )
        rSet.Compress( *this );
    if( rSet.aAttrSet.Count() || rSet.nStyleNo )
        SetAttrInDoc( rSet );

    
    if( rSet.pChildList )
        for( sal_uInt16 n = 0; n < rSet.pChildList->size(); ++n )
            SetAttrSet( (*rSet.pChildList)[ n ] );
}

    
bool SvxRTFParser::IsAttrSttPos()
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
    nOutlineNo = sal_uInt8(-1);         
    nBasedOn = 0;
    bBasedOnIsSet = sal_False;          
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
    sal_Int32 nLastCnt = pTmp->nEndCnt;

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
            
            SfxItemIter aIter( aMrgSet );
            const SfxPoolItem* pItem;
            do {
                sal_uInt16 nWhich = aIter.GetCurItem()->Which();
                if( SFX_ITEM_SET != pTmp->aAttrSet.GetItemState( nWhich,
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
            if( SFX_ITEM_SET != aAttrSet.GetItemState( nWhich, false ))
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
    nRuby = 0; 
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
