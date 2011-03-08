/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

#include <ctype.h>
#include <tools/datetime.hxx>
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


SV_IMPL_PTRARR( SvxRTFColorTbl, ColorPtr )
SV_IMPL_PTRARR( SvxRTFItemStackList, SvxRTFItemStackType* )

CharSet lcl_GetDefaultTextEncodingForRTF()
{

    ::com::sun::star::lang::Locale aLocale;
    ::rtl::OUString aLangString;

    aLocale = Application::GetSettings().GetLocale();
    aLangString = aLocale.Language;

    if ( aLangString.equals( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ru" )) )
      || aLangString.equals( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "uk" )) ) )
        return RTL_TEXTENCODING_MS_1251;
    if ( aLangString.equals( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "tr" )) ) )
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
    aColorTbl( 16, 4 ),
    aFontTbl( 16, 4 ),
    pInsPos( 0 ),
    pAttrPool( &rPool ),
    m_xDocProps( i_xDocProps ),
    pRTFDefaults( 0 ),
    nVersionNo( 0 )
{
    bNewDoc = bReadNewDoc;

    bChkStyleAttr = bCalcValue = bReadDocInfo = bIsInReadStyleTab = FALSE;
    bIsLeftToRightDef = TRUE;

    {
        RTFPlainAttrMapIds aTmp( rPool );
        aPlainMap.Insert( (USHORT*)&aTmp,
                    sizeof( RTFPlainAttrMapIds ) / sizeof(USHORT), 0 );
    }
    {
        RTFPardAttrMapIds aTmp( rPool );
        aPardMap.Insert( (USHORT*)&aTmp,
                    sizeof( RTFPardAttrMapIds ) / sizeof(USHORT), 0 );
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
    if( aColorTbl.Count() )
        ClearColorTbl();
    if( aFontTbl.Count() )
        ClearFontTbl();
    if( aStyleTbl.Count() )
        ClearStyleTbl();
    if( aAttrStack.Count() )
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

    if( aColorTbl.Count() )
        ClearColorTbl();
    if( aFontTbl.Count() )
        ClearFontTbl();
    if( aStyleTbl.Count() )
        ClearStyleTbl();
    if( aAttrStack.Count() )
        ClearAttrStack();

    bIsSetDfltTab = FALSE;
    bNewGroup = FALSE;
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
                if( aFontTbl.Count() )
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

    case RTF_EMDASH:        cCh = 151;  goto INSINGLECHAR;
    case RTF_ENDASH:        cCh = 150;  goto INSINGLECHAR;
    case RTF_BULLET:        cCh = 149;  goto INSINGLECHAR;
    case RTF_LQUOTE:        cCh = 145;  goto INSINGLECHAR;
    case RTF_RQUOTE:        cCh = 146;  goto INSINGLECHAR;
    case RTF_LDBLQUOTE:     cCh = 147;  goto INSINGLECHAR;
    case RTF_RDBLQUOTE:     cCh = 148;  goto INSINGLECHAR;
INSINGLECHAR:
        aToken = ByteString::ConvertToUnicode( (sal_Char)cCh,
                                            RTL_TEXTENCODING_MS_1252 );

        // no Break, aToken is set as Text
    case RTF_TEXTTOKEN:
        {
            InsertText();
            // all collected Attributes are set
            for( USHORT n = aAttrSetList.Count(); n; )
            {
                SvxRTFItemStackType* pStkSet = aAttrSetList[--n];
                SetAttrSet( *pStkSet );
                aAttrSetList.DeleteAndDestroy( n );
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
    short nStyleNo = 0;
    int _nOpenBrakets = 1;      // the first was already detected earlier!!
    SvxRTFStyleType* pStyle = new SvxRTFStyleType( *pAttrPool, aWhichMap.GetData() );
    pStyle->aAttrSet.Put( GetRTFDefaults() );

    bIsInReadStyleTab = TRUE;
    bChkStyleAttr = FALSE;      // Do not check Attribute against the Styles

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

        case RTF_SBASEDON:  pStyle->nBasedOn = USHORT(nTokenValue); pStyle->bBasedOnIsSet=TRUE; break;
        case RTF_SNEXT:     pStyle->nNext = USHORT(nTokenValue);    break;
        case RTF_OUTLINELEVEL:
        case RTF_SOUTLVL:   pStyle->nOutlineNo = BYTE(nTokenValue); break;
        case RTF_S:         nStyleNo = (short)nTokenValue;          break;
        case RTF_CS:        nStyleNo = (short)nTokenValue;
                            pStyle->bIsCharFmt = TRUE;
                            break;

        case RTF_TEXTTOKEN:
            {
                pStyle->sName = DelCharAtEnd( aToken, ';' );

                if( aStyleTbl.Count() )
                {
                    SvxRTFStyleType* pOldSt = aStyleTbl.Remove( nStyleNo );
                    if( pOldSt )
                        delete pOldSt;
                }
                // All data from the font is available, so off to the table
                aStyleTbl.Insert( nStyleNo, pStyle );
                pStyle = new SvxRTFStyleType( *pAttrPool, aWhichMap.GetData() );
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
    bIsInReadStyleTab = FALSE;
}

void SvxRTFParser::ReadColorTable()
{
    int nToken;
    BYTE nRed = 0xff, nGreen = 0xff, nBlue = 0xff;

    while( '}' != ( nToken = GetNextToken() ) && IsParserWorking() )
    {
        switch( nToken )
        {
        case RTF_RED:   nRed = BYTE(nTokenValue);       break;
        case RTF_GREEN: nGreen = BYTE(nTokenValue);     break;
        case RTF_BLUE:  nBlue = BYTE(nTokenValue);      break;

        case RTF_TEXTTOKEN:
            if( 1 == aToken.Len()
                    ? aToken.GetChar( 0 ) != ';'
                    : STRING_NOTFOUND == aToken.Search( ';' ) )
                break;      // At least the ';' must be found

            // else no break !!

        case ';':
            if( IsParserWorking() )
            {
                // one color is finished, fill in the table
                // try to map the values to SV internal names
                ColorPtr pColor = new Color( nRed, nGreen, nBlue );
                if( !aColorTbl.Count() &&
                    BYTE(-1) == nRed && BYTE(-1) == nGreen && BYTE(-1) == nBlue )
                    pColor->SetColor( COL_AUTO );
                aColorTbl.Insert( pColor, aColorTbl.Count() );
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
    BOOL bIsAltFntNm = FALSE, bCheckNewFont;

    CharSet nSystemChar = lcl_GetDefaultTextEncodingForRTF();
    pFont->SetCharSet( nSystemChar );
    SetEncoding( nSystemChar );

    while( _nOpenBrakets && IsParserWorking() )
    {
        bCheckNewFont = FALSE;
        switch( ( nToken = GetNextToken() ))
        {
            case '}':
                bIsAltFntNm = FALSE;
                // Style has been completely read,
                // so this is still a stable status
                if( --_nOpenBrakets <= 1 && IsParserWorking() )
                    SaveState( RTF_FONTTBL );
                bCheckNewFont = TRUE;
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
                        (BYTE)nTokenValue);
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
                bCheckNewFont = TRUE;
                nInsFontNo = nFontNo;
                nFontNo = (short)nTokenValue;
                break;
            case RTF_FALT:
                bIsAltFntNm = TRUE;
                break;
            case RTF_TEXTTOKEN:
                DelCharAtEnd( aToken, ';' );
                if ( aToken.Len() )
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
            aFontTbl.Insert( nInsFontNo, pFont );
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
    BOOL bWeiter = TRUE;
    int nToken;
    while( bWeiter && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case RTF_YR:    aDT.Year = (USHORT)nTokenValue;     break;
        case RTF_MO:    aDT.Month = (USHORT)nTokenValue;    break;
        case RTF_DY:    aDT.Day = (USHORT)nTokenValue;      break;
        case RTF_HR:    aDT.Hours = (USHORT)nTokenValue;    break;
        case RTF_MIN:   aDT.Minutes = (USHORT)nTokenValue;  break;
        default:
            bWeiter = FALSE;
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
                ::rtl::OUString sTemp = GetTextToEndGroup( sStr );
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
    aColorTbl.DeleteAndDestroy( 0, aColorTbl.Count() );
}

void SvxRTFParser::ClearFontTbl()
{
    for( ULONG nCnt = aFontTbl.Count(); nCnt; )
        delete aFontTbl.GetObject( --nCnt );
}

void SvxRTFParser::ClearStyleTbl()
{
    for( ULONG nCnt = aStyleTbl.Count(); nCnt; )
        delete aStyleTbl.GetObject( --nCnt );
}

void SvxRTFParser::ClearAttrStack()
{
    SvxRTFItemStackType* pTmp;
    for( ULONG nCnt = aAttrStack.Count(); nCnt; --nCnt )
    {
        pTmp = aAttrStack.Pop();
        delete pTmp;
    }
}

String& SvxRTFParser::DelCharAtEnd( String& rStr, const sal_Unicode cDel )
{
    if( rStr.Len() && ' ' == rStr.GetChar( 0 ))
        rStr.EraseLeadingChars();
    if( rStr.Len() && ' ' == rStr.GetChar( rStr.Len()-1 ))
        rStr.EraseTrailingChars();
    if( rStr.Len() && cDel == rStr.GetChar( rStr.Len()-1 ))
        rStr.Erase( rStr.Len()-1 );
    return rStr;
}


const Font& SvxRTFParser::GetFont( USHORT nId )
{
    const Font* pFont = aFontTbl.Get( nId );
    if( !pFont )
    {
        const SvxFontItem& rDfltFont = (const SvxFontItem&)
                        pAttrPool->GetDefaultItem(
                    ((RTFPlainAttrMapIds*)aPlainMap.GetData())->nFont );
        pDfltFont->SetName( rDfltFont.GetStyleName() );
        pDfltFont->SetFamily( rDfltFont.GetFamily() );
        pFont = pDfltFont;
    }
    return *pFont;
}

SvxRTFItemStackType* SvxRTFParser::_GetAttrSet( int bCopyAttr )
{
    SvxRTFItemStackType* pAkt = aAttrStack.Top();
    SvxRTFItemStackType* pNew;
    if( pAkt )
        pNew = new SvxRTFItemStackType( *pAkt, *pInsPos, bCopyAttr );
    else
        pNew = new SvxRTFItemStackType( *pAttrPool, aWhichMap.GetData(),
                                        *pInsPos );
    pNew->SetRTFDefaults( GetRTFDefaults() );

    aAttrStack.Push( pNew );
    bNewGroup = FALSE;
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

    SvxRTFStyleType* pStyle;
    if( !IsChkStyleAttr() ||
        !rStkType.GetAttrSet().Count() ||
        0 == ( pStyle = aStyleTbl.Get( rStkType.nStyleNo ) ))
    {
        for( USHORT nWhich = aIter.GetCurWhich(); nWhich; nWhich = aIter.NextWhich() )
        {
            if( SFX_WHICH_MAX > nWhich &&
                SFX_ITEM_SET == rSet.GetItemState( nWhich, FALSE, &pItem ) &&
                     rPool.GetDefaultItem( nWhich ) == *pItem )
                rSet.ClearItem( nWhich );       // delete
        }
    }
    else
    {
        // Delete all Attributes, which are already defined in the Style,
        // from the current AttrSet.
        SfxItemSet &rStyleSet = pStyle->aAttrSet;
        const SfxPoolItem* pSItem;
        for( USHORT nWhich = aIter.GetCurWhich(); nWhich; nWhich = aIter.NextWhich() )
        {
            if( SFX_ITEM_SET == rStyleSet.GetItemState( nWhich, TRUE, &pSItem ))
            {
                if( SFX_ITEM_SET == rSet.GetItemState( nWhich, FALSE, &pItem )
                    && *pItem == *pSItem )
                    rSet.ClearItem( nWhich );       // delete
            }
            else if( SFX_WHICH_MAX > nWhich &&
                    SFX_ITEM_SET == rSet.GetItemState( nWhich, FALSE, &pItem ) &&
                     rPool.GetDefaultItem( nWhich ) == *pItem )
                rSet.ClearItem( nWhich );       // delete
        }
    }
}

void SvxRTFParser::AttrGroupEnd()   // process the current, delete from Stack
{
    if( aAttrStack.Count() )
    {
        SvxRTFItemStackType *pOld = aAttrStack.Pop();
        SvxRTFItemStackType *pAkt = aAttrStack.Top();

        do {        // middle check loop
            ULONG nOldSttNdIdx = pOld->pSttNd->GetIdx();
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
                while( TRUE )
                {
                    if( SFX_ITEM_SET == pAkt->aAttrSet.GetItemState(
                        pItem->Which(), FALSE, &pGet ) &&
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
                ULONG nNd = pInsPos->GetNodeIdx();
                MovePos( FALSE );
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
                                    *pOld, *pInsPos, TRUE );
                        pNew->aAttrSet.SetParent( pOld->aAttrSet.GetParent() );

                        // Delete all paragraph attributes from pNew
                        for( USHORT n = 0; n < aPardMap.Count() &&
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

                                aAttrSetList.Insert( pOld, aAttrSetList.Count() );
                                aAttrSetList.Insert( pNew, aAttrSetList.Count() );
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
                    if( bCrsrBack && 50 < pAkt->pChildList->Count() )
                    {
                        // at the beginning of a paragraph? Move back one position
                        MovePos( TRUE );
                        bCrsrBack = FALSE;

                        // Open a new Group.
                        SvxRTFItemStackType* pNew = new SvxRTFItemStackType(
                                                *pAkt, *pInsPos, TRUE );
                        pNew->SetRTFDefaults( GetRTFDefaults() );

                        // Set all until here valid Attributes
                        AttrGroupEnd();
                        pAkt = aAttrStack.Top();  // can be changed after AttrGroupEnd!
                        pNew->aAttrSet.SetParent( pAkt ? &pAkt->aAttrSet : 0 );
                        aAttrStack.Push( pNew );
                        pAkt = pNew;
                    }
                }
                else
                    // Last off the stack, thus cache it until the next text was
                    // read. (Span no attributes!)
                    aAttrSetList.Insert( pOld, aAttrSetList.Count() );

                pOld = 0;
            }

            if( bCrsrBack )
                // at the beginning of a paragraph? Move back one position
                MovePos( TRUE );

        } while( FALSE );

        if( pOld )
            delete pOld;

        bNewGroup = FALSE;
    }
}

void SvxRTFParser::SetAllAttrOfStk()        // end all Attr. and set it into doc
{
    // Yet to get all Attrbutes from the stack!
    while( aAttrStack.Count() )
        AttrGroupEnd();

    for( USHORT n = aAttrSetList.Count(); n; )
    {
        SvxRTFItemStackType* pStkSet = aAttrSetList[--n];
        SetAttrSet( *pStkSet );
        aAttrSetList.DeleteAndDestroy( n );
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
        for( USHORT n = 0; n < rSet.pChildList->Count(); ++n )
            SetAttrSet( *(*rSet.pChildList)[ n ] );
}

    // Has no Text been inserted yet? (SttPos from the top Stack entry!)
int SvxRTFParser::IsAttrSttPos()
{
    SvxRTFItemStackType* pAkt = aAttrStack.Top();
    return !pAkt || (pAkt->pSttNd->GetIdx() == pInsPos->GetNodeIdx() &&
        pAkt->nSttCnt == pInsPos->GetCntIdx());
}


void SvxRTFParser::SetAttrInDoc( SvxRTFItemStackType & )
{
}

void SvxRTFParser::BuildWhichTbl()
{
    if( aWhichMap.Count() )
        aWhichMap.Remove( 0, aWhichMap.Count() );
    aWhichMap.Insert( (USHORT)0, (USHORT)0 );

    // Building a Which-Map 'rWhichMap' from an Array of
    // 'pWhichIds' frm Which-Ids. It has the long 'nWhichIds'.
    // The Which-Map is not going to be deleted.
    SvParser::BuildWhichTbl( aWhichMap, (USHORT*)aPardMap.GetData(), aPardMap.Count() );
    SvParser::BuildWhichTbl( aWhichMap, (USHORT*)aPlainMap.GetData(), aPlainMap.Count() );
}

const SfxItemSet& SvxRTFParser::GetRTFDefaults()
{
    if( !pRTFDefaults )
    {
        pRTFDefaults = new SfxItemSet( *pAttrPool, aWhichMap.GetData() );
        USHORT nId;
        if( 0 != ( nId = ((RTFPardAttrMapIds*)aPardMap.GetData())->nScriptSpace ))
        {
            SvxScriptSpaceItem aItem( FALSE, nId );
            if( bNewDoc )
                pAttrPool->SetPoolDefaultItem( aItem );
            else
                pRTFDefaults->Put( aItem );
        }
    }
    return *pRTFDefaults;
}

/**/

SvxRTFStyleType::SvxRTFStyleType( SfxItemPool& rPool, const USHORT* pWhichRange )
    : aAttrSet( rPool, pWhichRange )
{
    nOutlineNo = BYTE(-1);          // not set
    nBasedOn = 0;
    bBasedOnIsSet = FALSE;
    nNext = 0;
    bIsCharFmt = FALSE;
}


SvxRTFItemStackType::SvxRTFItemStackType(
        SfxItemPool& rPool, const USHORT* pWhichRange,
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
         pChildList = new SvxRTFItemStackList( 4, 16 );
    pChildList->Insert( pIns, pChildList->Count() );
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
    USHORT nCount = pChildList ? pChildList->Count() : 0;
    for (USHORT i = 0; i < nCount; ++i)
    {
        SvxRTFItemStackType* pStk = (*pChildList)[i];
        pStk->MoveFullNode(rOldNode, rNewNode);
    }
}

bool SvxRTFParser::UncompressableStackEntry(const SvxRTFItemStackType &) const
{
    return false;
}

void SvxRTFItemStackType::Compress( const SvxRTFParser& rParser )
{
    DBG_ASSERT( pChildList, "There is no child list" );

    USHORT n;
    SvxRTFItemStackType* pTmp = (*pChildList)[0];

    if( !pTmp->aAttrSet.Count() ||
        pSttNd->GetIdx() != pTmp->pSttNd->GetIdx() ||
        nSttCnt != pTmp->nSttCnt )
        return;

    SvxNodeIdx* pLastNd = pTmp->pEndNd;
    xub_StrLen nLastCnt = pTmp->nEndCnt;

    SfxItemSet aMrgSet( pTmp->aAttrSet );
    for( n = 1; n < pChildList->Count(); ++n )
    {
        pTmp = (*pChildList)[n];
        if( pTmp->pChildList )
            pTmp->Compress( rParser );

        if( !pTmp->nSttCnt
            ? (pLastNd->GetIdx()+1 != pTmp->pSttNd->GetIdx() ||
               !rParser.IsEndPara( pLastNd, nLastCnt ) )
            : ( pTmp->nSttCnt != nLastCnt ||
                pLastNd->GetIdx() != pTmp->pSttNd->GetIdx() ))
        {
            while( ++n < pChildList->Count() )
                if( (pTmp = (*pChildList)[n])->pChildList )
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
                USHORT nWhich = aIter.GetCurItem()->Which();
                if( SFX_ITEM_SET != pTmp->aAttrSet.GetItemState( nWhich,
                      FALSE, &pItem ) || *pItem != *aIter.GetCurItem() )
                    aMrgSet.ClearItem( nWhich );

                if( aIter.IsAtEnd() )
                    break;
                aIter.NextItem();
            } while( TRUE );

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

    for( n = 0; n < pChildList->Count(); ++n )
    {
        pTmp = (*pChildList)[n];
        pTmp->aAttrSet.Differentiate( aMrgSet );

        if( !pTmp->pChildList && !pTmp->aAttrSet.Count() && !pTmp->nStyleNo )
        {
            pChildList->Remove( n );
            delete pTmp;
            --n;
            continue;
        }
    }
    if( !pChildList->Count() )
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
            USHORT nWhich = aIter.GetCurItem()->Which();
            if( SFX_ITEM_SET != aAttrSet.GetItemState( nWhich, FALSE ))
                aAttrSet.Put( *aIter.GetCurItem() );

            if( aIter.IsAtEnd() )
                break;
            aIter.NextItem();
        } while( TRUE );
    }
}

/**/

RTFPlainAttrMapIds::RTFPlainAttrMapIds( const SfxItemPool& rPool )
{
    nCaseMap = rPool.GetTrueWhich( SID_ATTR_CHAR_CASEMAP, FALSE );
    nBgColor = rPool.GetTrueWhich( SID_ATTR_BRUSH_CHAR, FALSE );
    nColor = rPool.GetTrueWhich( SID_ATTR_CHAR_COLOR, FALSE );
    nContour = rPool.GetTrueWhich( SID_ATTR_CHAR_CONTOUR, FALSE );
    nCrossedOut = rPool.GetTrueWhich( SID_ATTR_CHAR_STRIKEOUT, FALSE );
    nEscapement = rPool.GetTrueWhich( SID_ATTR_CHAR_ESCAPEMENT, FALSE );
    nFont = rPool.GetTrueWhich( SID_ATTR_CHAR_FONT, FALSE );
    nFontHeight = rPool.GetTrueWhich( SID_ATTR_CHAR_FONTHEIGHT, FALSE );
    nKering = rPool.GetTrueWhich( SID_ATTR_CHAR_KERNING, FALSE );
    nLanguage = rPool.GetTrueWhich( SID_ATTR_CHAR_LANGUAGE, FALSE );
    nPosture = rPool.GetTrueWhich( SID_ATTR_CHAR_POSTURE, FALSE );
    nShadowed = rPool.GetTrueWhich( SID_ATTR_CHAR_SHADOWED, FALSE );
    nUnderline = rPool.GetTrueWhich( SID_ATTR_CHAR_UNDERLINE, FALSE );
    nOverline = rPool.GetTrueWhich( SID_ATTR_CHAR_OVERLINE, FALSE );
    nWeight = rPool.GetTrueWhich( SID_ATTR_CHAR_WEIGHT, FALSE );
    nWordlineMode = rPool.GetTrueWhich( SID_ATTR_CHAR_WORDLINEMODE, FALSE );
    nAutoKerning = rPool.GetTrueWhich( SID_ATTR_CHAR_AUTOKERN, FALSE );

    nCJKFont = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_FONT, FALSE );
    nCJKFontHeight = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT, FALSE );
    nCJKLanguage = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_LANGUAGE, FALSE );
    nCJKPosture = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_POSTURE, FALSE );
    nCJKWeight = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_WEIGHT, FALSE );
    nCTLFont = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_FONT, FALSE );
    nCTLFontHeight = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_FONTHEIGHT, FALSE );
    nCTLLanguage = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_LANGUAGE, FALSE );
    nCTLPosture = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_POSTURE, FALSE );
    nCTLWeight = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_WEIGHT, FALSE );
    nEmphasis = rPool.GetTrueWhich( SID_ATTR_CHAR_EMPHASISMARK, FALSE );
    nTwoLines = rPool.GetTrueWhich( SID_ATTR_CHAR_TWO_LINES, FALSE );
    nRuby = 0; //rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_RUBY, FALSE );
    nCharScaleX = rPool.GetTrueWhich( SID_ATTR_CHAR_SCALEWIDTH, FALSE );
    nHorzVert = rPool.GetTrueWhich( SID_ATTR_CHAR_ROTATED, FALSE );
    nRelief = rPool.GetTrueWhich( SID_ATTR_CHAR_RELIEF, FALSE );
    nHidden = rPool.GetTrueWhich( SID_ATTR_CHAR_HIDDEN, FALSE );
}

RTFPardAttrMapIds ::RTFPardAttrMapIds ( const SfxItemPool& rPool )
{
    nLinespacing = rPool.GetTrueWhich( SID_ATTR_PARA_LINESPACE, FALSE );
    nAdjust = rPool.GetTrueWhich( SID_ATTR_PARA_ADJUST, FALSE );
    nTabStop = rPool.GetTrueWhich( SID_ATTR_TABSTOP, FALSE );
    nHyphenzone = rPool.GetTrueWhich( SID_ATTR_PARA_HYPHENZONE, FALSE );
    nLRSpace = rPool.GetTrueWhich( SID_ATTR_LRSPACE, FALSE );
    nULSpace = rPool.GetTrueWhich( SID_ATTR_ULSPACE, FALSE );
    nBrush = rPool.GetTrueWhich( SID_ATTR_BRUSH, FALSE );
    nBox = rPool.GetTrueWhich( SID_ATTR_BORDER_OUTER, FALSE );
    nShadow = rPool.GetTrueWhich( SID_ATTR_BORDER_SHADOW, FALSE );
    nOutlineLvl = rPool.GetTrueWhich( SID_ATTR_PARA_OUTLLEVEL, FALSE );
    nSplit = rPool.GetTrueWhich( SID_ATTR_PARA_SPLIT, FALSE );
    nKeep = rPool.GetTrueWhich( SID_ATTR_PARA_KEEP, FALSE );
    nFontAlign = rPool.GetTrueWhich( SID_PARA_VERTALIGN, FALSE );
    nScriptSpace = rPool.GetTrueWhich( SID_ATTR_PARA_SCRIPTSPACE, FALSE );
    nHangPunct = rPool.GetTrueWhich( SID_ATTR_PARA_HANGPUNCTUATION, FALSE );
    nForbRule = rPool.GetTrueWhich( SID_ATTR_PARA_FORBIDDEN_RULES, FALSE );
    nDirection = rPool.GetTrueWhich( SID_ATTR_FRAMEDIRECTION, FALSE );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
