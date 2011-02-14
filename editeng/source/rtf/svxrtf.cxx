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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */


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


SV_IMPL_PTRARR( SvxRTFItemStackList, SvxRTFItemStackType* )

CharSet lcl_GetDefaultTextEncodingForRTF()
{

    ::com::sun::star::lang::Locale aLocale;
    ::rtl::OUString aLangString;

    aLocale = Application::GetSettings().GetLocale();
    aLangString = aLocale.Language;

    if ( aLangString.equals( ::rtl::OUString::createFromAscii( "ru" ) )
      || aLangString.equals( ::rtl::OUString::createFromAscii( "uk" ) ) )
        return RTL_TEXTENCODING_MS_1251;
    if ( aLangString.equals( ::rtl::OUString::createFromAscii( "tr" ) ) )
        return RTL_TEXTENCODING_MS_1254;
    else
        return RTL_TEXTENCODING_MS_1252;
}

// -------------- Methoden --------------------

SvxRTFParser::SvxRTFParser( SfxItemPool& rPool, SvStream& rIn,
            uno::Reference<document::XDocumentProperties> i_xDocProps,
            int bReadNewDoc )
    : SvRTFParser( rIn, 5 ),
    rStrm(rIn),
    aFontTbl( 16, 4 ),
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
        aPlainMap.Insert( (sal_uInt16*)&aTmp,
                    sizeof( RTFPlainAttrMapIds ) / sizeof(sal_uInt16), 0 );
    }
    {
        RTFPardAttrMapIds aTmp( rPool );
        aPardMap.Insert( (sal_uInt16*)&aTmp,
                    sizeof( RTFPardAttrMapIds ) / sizeof(sal_uInt16), 0 );
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
    if( aFontTbl.Count() )
        ClearFontTbl();
    if( aStyleTbl.Count() )
        ClearStyleTbl();
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
    DBG_ASSERT( pInsPos, "no insertion" );

    if( !pInsPos )
        return SVPAR_ERROR;

    if( !aColorTbl.empty() )
        ClearColorTbl();
    if( aFontTbl.Count() )
        ClearFontTbl();
    if( aStyleTbl.Count() )
        ClearStyleTbl();
    if( !aAttrStack.empty() )
        ClearAttrStack();

    bIsSetDfltTab = sal_False;
    bNewGroup = sal_False;
    nDfltFont = 0;

    sBaseURL.Erase();

    // erzeuge aus den gesetzten WhichIds die richtige WhichId-Tabelle.
    BuildWhichTbl();

    return SvRTFParser::CallParser();
}

void SvxRTFParser::Continue( int nToken )
{
    SvRTFParser::Continue( nToken );

    if( SVPAR_PENDING != GetStatus() )
    {
        SetAllAttrOfStk();
#if 0
    //Regardless of what "color 0" is, word defaults to auto as the default colour.
    //e.g. see #i7713#
        if( bNewDoc && ((RTFPlainAttrMapIds*)aPlainMap.GetData())->nColor )
            pAttrPool->SetPoolDefaultItem( SvxColorItem( GetColor( 0 ),
                        ((RTFPlainAttrMapIds*)aPlainMap.GetData())->nColor ));
#endif
     }
}


// wird fuer jedes Token gerufen, das in CallParser erkannt wird
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
                    // koennen wir sofort setzen
                    SetDefault( nToken, nTokenValue );
                else
                    // wird nach einlesen der Fonttabelle gesetzt
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

        // kein Break, aToken wird als Text gesetzt
    case RTF_TEXTTOKEN:
        {
            InsertText();
            // alle angesammelten Attribute setzen
            for( sal_uInt16 n = aAttrSetList.Count(); n; )
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
        if (bNewGroup)          // Verschachtelung !!
            _GetAttrSet();
        EnterEnvironment();
        bNewGroup = true;
        break;
    case '}':
        if( !bNewGroup )        // leere Gruppe ??
            AttrGroupEnd();
        LeaveEnvironment();
        bNewGroup = false;
        break;
    case RTF_INFO:
#ifndef SVX_LIGHT
        if (bReadDocInfo && bNewDoc && m_xDocProps.is())
            ReadInfo();
        else
#endif
            SkipGroup();
        break;

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // erstmal gesamt ueberlesen (muessen alle in einer Gruppe stehen !!)
    // Koennen auch ohne dem IGNORE-Flag im RTF-File auftreten; alle Gruppen
    // mit IGNORE-Flag werden im default-Zweig ueberlesen.

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
#if 0
    //disabled for #i19718#
    case RTF_SHPRSLT:   // RTF_SHP fehlt noch !!
#endif
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
    int _nOpenBrakets = 1;      // die erste wurde schon vorher erkannt !!
    SvxRTFStyleType* pStyle = new SvxRTFStyleType( *pAttrPool, aWhichMap.GetData() );
    pStyle->aAttrSet.Put( GetRTFDefaults() );

    bIsInReadStyleTab = sal_True;
    bChkStyleAttr = sal_False;      // Attribute nicht gegen die Styles checken

    while( _nOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '}':       if( --_nOpenBrakets && IsParserWorking() )
                            // Style konnte vollstaendig gelesen werden,
                            // also ist das noch ein stabiler Status
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
                    // gleich herausfiltern
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

/*
??? soll man das umsetzen ???
                if( !pStyle->sName.Len() )
                    pStyle->sName = "Standard";
*/
                // sollte die Nummer doppelt vergeben werden ?
                if( aStyleTbl.Count() )
                {
                    SvxRTFStyleType* pOldSt = aStyleTbl.Remove( nStyleNo );
                    if( pOldSt )
                        delete pOldSt;
                }
                // alle Daten vom Style vorhanden, also ab in die Tabelle
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
#if 0
                        --_nOpenBrakets;        // korrigieren!!
#endif
                    }
                }
                ReadAttr( nToken, &pStyle->aAttrSet );
                break;
            }
            break;
        }
    }
    delete pStyle;          // loesche das letze Style
    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet

    // Flag wieder auf alten Zustand
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

        case RTF_TEXTTOKEN:     // oder sollte irgendein Unsin darumstehen?
            if( 1 == aToken.Len()
                    ? aToken.GetChar( 0 ) != ';'
                    : STRING_NOTFOUND == aToken.Search( ';' ) )
                break;      // es muss zumindestens das ';' gefunden werden

            // else kein break !!

        case ';':
            if( IsParserWorking() )
            {
                // eine Farbe ist Fertig, in die Tabelle eintragen
                // versuche die Werte auf SV interne Namen zu mappen
                ColorPtr pColor = new Color( nRed, nGreen, nBlue );
                if( aColorTbl.empty() &&
                    sal_uInt8(-1) == nRed && sal_uInt8(-1) == nGreen && sal_uInt8(-1) == nBlue )
                    pColor->SetColor( COL_AUTO );
                aColorTbl.push_back( pColor );
                nRed = 0, nGreen = 0, nBlue = 0;

                // Color konnte vollstaendig gelesen werden,
                // also ist das noch ein stabiler Status
                SaveState( RTF_COLORTBL );
            }
            break;
        }
    }
    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
}

void SvxRTFParser::ReadFontTable()
{
    int nToken;
    int _nOpenBrakets = 1;      // die erste wurde schon vorher erkannt !!
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
                // Style konnte vollstaendig gelesen werden,
                // also ist das noch ein stabiler Status
                if( --_nOpenBrakets <= 1 && IsParserWorking() )
                    SaveState( RTF_FONTTBL );
                bCheckNewFont = sal_True;
                nInsFontNo = nFontNo;
                break;
            case '{':
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                // Unknown und alle bekannten nicht ausgewerteten Gruppen
                // sofort ueberspringen
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ) &&
                        RTF_PANOSE != nToken && RTF_FNAME != nToken &&
                        RTF_FONTEMB != nToken && RTF_FONTFILE != nToken )
                    nToken = SkipToken( -2 );
                else
                {
                    // gleich herausfiltern
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
            // bei technischen/symbolischen Font wird der CharSet ungeschaltet!!
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
            // alle Daten vom Font vorhanden, also ab in die Tabelle
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
    // den letzen muessen wir selbst loeschen
    delete pFont;
    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet

    // setze den default Font am Doc
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
    int _nOpenBrakets = 1, nToken;      // die erste wurde schon vorher erkannt !!

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
                    // gleich herausfiltern
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
    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
    return rStr;
}

util::DateTime SvxRTFParser::GetDateTimeStamp( )
{
    util::DateTime aDT;
    sal_Bool bWeiter = sal_True;
    int nToken;
    while( bWeiter && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
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
    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
    return aDT;
}

void SvxRTFParser::ReadInfo( const sal_Char* pChkForVerNo )
{
#ifndef SVX_LIGHT
    int _nOpenBrakets = 1, nToken;      // die erste wurde schon vorher erkannt !!
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
                    // gleich herausfiltern
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

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
#endif
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
    for( sal_uInt32 nCnt = aFontTbl.Count(); nCnt; )
        delete aFontTbl.GetObject( --nCnt );
}

void SvxRTFParser::ClearStyleTbl()
{
    for( sal_uInt32 nCnt = aStyleTbl.Count(); nCnt; )
        delete aStyleTbl.GetObject( --nCnt );
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


const Font& SvxRTFParser::GetFont( sal_uInt16 nId )
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
    SvxRTFItemStackType* pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();
    SvxRTFItemStackType* pNew;
    if( pAkt )
        pNew = new SvxRTFItemStackType( *pAkt, *pInsPos, bCopyAttr );
    else
        pNew = new SvxRTFItemStackType( *pAttrPool, aWhichMap.GetData(),
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

    SvxRTFStyleType* pStyle;
    if( !IsChkStyleAttr() ||
        !rStkType.GetAttrSet().Count() ||
        0 == ( pStyle = aStyleTbl.Get( rStkType.nStyleNo ) ))
    {
        for( sal_uInt16 nWhich = aIter.GetCurWhich(); nWhich; nWhich = aIter.NextWhich() )
        {
            if( SFX_WHICH_MAX > nWhich &&
                SFX_ITEM_SET == rSet.GetItemState( nWhich, sal_False, &pItem ) &&
                     rPool.GetDefaultItem( nWhich ) == *pItem )
                rSet.ClearItem( nWhich );       // loeschen
        }
    }
    else
    {
        // alle Attribute, die schon vom Style definiert sind, aus dem
        // akt. AttrSet entfernen
        SfxItemSet &rStyleSet = pStyle->aAttrSet;
        const SfxPoolItem* pSItem;
        for( sal_uInt16 nWhich = aIter.GetCurWhich(); nWhich; nWhich = aIter.NextWhich() )
        {
            if( SFX_ITEM_SET == rStyleSet.GetItemState( nWhich, sal_True, &pSItem ))
            {
                // JP 22.06.99: im Style und im Set gleich gesetzt -> loeschen
                if( SFX_ITEM_SET == rSet.GetItemState( nWhich, sal_False, &pItem )
                    && *pItem == *pSItem )
                    rSet.ClearItem( nWhich );       // loeschen
            }
                // Bug 59571 - falls nicht im Style gesetzt und gleich mit
                //              dem PoolDefault -> auch dann loeschen
            else if( SFX_WHICH_MAX > nWhich &&
                    SFX_ITEM_SET == rSet.GetItemState( nWhich, sal_False, &pItem ) &&
                     rPool.GetDefaultItem( nWhich ) == *pItem )
                rSet.ClearItem( nWhich );       // loeschen
        }
    }
}

void SvxRTFParser::AttrGroupEnd()   // den akt. Bearbeiten, vom Stack loeschen
{
    if( !aAttrStack.empty() )
    {
        SvxRTFItemStackType *pOld = aAttrStack.empty() ? 0 : aAttrStack.back();
        aAttrStack.pop_back();
        SvxRTFItemStackType *pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();

        do {        // middle check loop
            sal_uLong nOldSttNdIdx = pOld->pSttNd->GetIdx();
            if( !pOld->pChildList &&
                ((!pOld->aAttrSet.Count() && !pOld->nStyleNo ) ||
                (nOldSttNdIdx == pInsPos->GetNodeIdx() &&
                pOld->nSttCnt == pInsPos->GetCntIdx() )))
                break;          // keine Attribute oder Bereich

            // setze nur die Attribute, die unterschiedlich zum Parent sind
            if( pAkt && pOld->aAttrSet.Count() )
            {
                SfxItemIter aIter( pOld->aAttrSet );
                const SfxPoolItem* pItem = aIter.GetCurItem(), *pGet;
                while( sal_True )
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

            // setze alle Attribute, die von Start bis hier
            // definiert sind.
            int bCrsrBack = !pInsPos->GetCntIdx();
            if( bCrsrBack )
            {
                // am Absatzanfang ? eine Position zurueck
                sal_uLong nNd = pInsPos->GetNodeIdx();
                MovePos( sal_False );
                // if can not move backward then later dont move forward !
                bCrsrBack = nNd != pInsPos->GetNodeIdx();
            }

            //Bug #46608#: ungueltige Bereiche ignorieren!
            if( ( pOld->pSttNd->GetIdx() < pInsPos->GetNodeIdx() ||
                ( pOld->pSttNd->GetIdx() == pInsPos->GetNodeIdx() &&
                pOld->nSttCnt <= pInsPos->GetCntIdx() ))
#if 0
//BUG 68555 - dont test for empty paragraph or any range
                && ( nOldSttNdIdx != pInsPos->GetNodeIdx() ||
                pOld->nSttCnt != pInsPos->GetCntIdx() ||
                !pOld->nSttCnt )
#endif
                )
            {
                if( !bCrsrBack )
                {
                    // alle pard-Attribute gelten nur bis zum vorherigen
                    // Absatz !!
                    if( nOldSttNdIdx == pInsPos->GetNodeIdx() )
                    {
#if 0
//BUG 68555 - dont reset pard attrs, if the group not begins not at start of
//              paragraph
                        // Bereich innerhalb eines Absatzes:
                        // alle Absatz-Attribute und StyleNo loeschen
                        // aber nur wenn mitten drin angefangen wurde
                        if( pOld->nSttCnt )
                        {
                            pOld->nStyleNo = 0;
                            for( sal_uInt16 n = 0; n < aPardMap.Count() &&
                                                pOld->aAttrSet.Count(); ++n )
                                if( aPardMap[n] )
                                    pOld->aAttrSet.ClearItem( aPardMap[n] );

                            if( !pOld->aAttrSet.Count() && !pOld->pChildList &&
                                !pOld->nStyleNo  )
                                break;  // auch dieser verlaesst uns jetzt
                        }
#endif
                    }
                    else
                    {
                        // jetzt wirds kompliziert:
                        // - alle Zeichen-Attribute behalten den Bereich,
                        // - alle Absatz-Attribute bekommen den Bereich
                        //          bis zum vorherigen Absatz
                        SvxRTFItemStackType* pNew = new SvxRTFItemStackType(
                                    *pOld, *pInsPos, sal_True );
                        pNew->aAttrSet.SetParent( pOld->aAttrSet.GetParent() );

                        // loesche aus pNew alle Absatz Attribute
                        for( sal_uInt16 n = 0; n < aPardMap.Count() &&
                                            pNew->aAttrSet.Count(); ++n )
                            if( aPardMap[n] )
                                pNew->aAttrSet.ClearItem( aPardMap[n] );
                        pNew->SetRTFDefaults( GetRTFDefaults() );

                        // gab es ueberhaupt welche ?
                        if( pNew->aAttrSet.Count() == pOld->aAttrSet.Count() )
                            delete pNew;        // das wars dann
                        else
                        {
                            pNew->nStyleNo = 0;

                            // spanne jetzt den richtigen Bereich auf
                            // pNew von alter
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
                                // letzter vom Stack, also zwischenspeichern, bis der
                                // naechste Text eingelesen wurde. (keine Attribute
                                // aufspannen!!)
                                aAttrSetList.Insert( pOld, aAttrSetList.Count() );
                                aAttrSetList.Insert( pNew, aAttrSetList.Count() );
                            }
                            pOld = 0;   // pOld nicht loeschen
                            break;      // das wars !!
                        }
                    }
                }

                pOld->pEndNd = pInsPos->MakeNodeIdx();
                pOld->nEndCnt = pInsPos->GetCntIdx();

#if 0
                if( IsChkStyleAttr() )
                    _ClearStyleAttr( *pOld );
#else
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
#endif

                if( pAkt )
                {
                    pAkt->Add( pOld );
                    // split up and create new entry, because it make no sense
                    // to create a "so long" depend list. Bug 95010
                    if( bCrsrBack && 50 < pAkt->pChildList->Count() )
                    {
                        // am Absatzanfang ? eine Position zurueck
                        MovePos( sal_True );
                        bCrsrBack = sal_False;

                        // eine neue Gruppe aufmachen
                        SvxRTFItemStackType* pNew = new SvxRTFItemStackType(
                                                *pAkt, *pInsPos, sal_True );
                        pNew->SetRTFDefaults( GetRTFDefaults() );

                        // alle bis hierher gueltigen Attribute "setzen"
                        AttrGroupEnd();
                        pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();  // can be changed after AttrGroupEnd!
                        pNew->aAttrSet.SetParent( pAkt ? &pAkt->aAttrSet : 0 );
                        aAttrStack.push_back( pNew );
                        pAkt = pNew;
                    }
                }
                else
                    // letzter vom Stack, also zwischenspeichern, bis der
                    // naechste Text eingelesen wurde. (keine Attribute
                    // aufspannen!!)
                    aAttrSetList.Insert( pOld, aAttrSetList.Count() );

                pOld = 0;
            }

            if( bCrsrBack )
                // am Absatzanfang ? eine Position zurueck
                MovePos( sal_True );

        } while( sal_False );

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

    for( sal_uInt16 n = aAttrSetList.Count(); n; )
    {
        SvxRTFItemStackType* pStkSet = aAttrSetList[--n];
        SetAttrSet( *pStkSet );
        aAttrSetList.DeleteAndDestroy( n );
    }
}

// setzt alle Attribute, die unterschiedlich zum aktuellen sind
void SvxRTFParser::SetAttrSet( SvxRTFItemStackType &rSet )
{
    // wurde DefTab nie eingelesen? dann setze auf default
    if( !bIsSetDfltTab )
        SetDefault( RTF_DEFTAB, 720 );

    if( rSet.pChildList )
        rSet.Compress( *this );
    if( rSet.aAttrSet.Count() || rSet.nStyleNo )
        SetAttrInDoc( rSet );

    // dann mal alle Childs abarbeiten
    if( rSet.pChildList )
        for( sal_uInt16 n = 0; n < rSet.pChildList->Count(); ++n )
            SetAttrSet( *(*rSet.pChildList)[ n ] );
}

    // Is text wasn't inserted? (Get SttPos from the top of stack!)
int SvxRTFParser::IsAttrSttPos()
{
    SvxRTFItemStackType* pAkt = aAttrStack.empty() ? 0 : aAttrStack.back();
    return !pAkt || (pAkt->pSttNd->GetIdx() == pInsPos->GetNodeIdx() &&
        pAkt->nSttCnt == pInsPos->GetCntIdx());
}


void SvxRTFParser::SetAttrInDoc( SvxRTFItemStackType & )
{
}

#ifdef USED
void SvxRTFParser::SaveState( int nToken )
{
    SvRTFParser::SaveState( nToken );
}

void SvxRTFParser::RestoreState()
{
    SvRTFParser::RestoreState();
}
#endif

void SvxRTFParser::BuildWhichTbl()
{
    if( aWhichMap.Count() )
        aWhichMap.Remove( 0, aWhichMap.Count() );
    aWhichMap.Insert( (sal_uInt16)0, (sal_uInt16)0 );

    // Aufbau einer Which-Map 'rWhichMap' aus einem Array von
    // 'pWhichIds' von Which-Ids. Es hat die Lange 'nWhichIds'.
    // Die Which-Map wird nicht geloescht.
    SvParser::BuildWhichTbl( aWhichMap, (sal_uInt16*)aPardMap.GetData(), aPardMap.Count() );
    SvParser::BuildWhichTbl( aWhichMap, (sal_uInt16*)aPlainMap.GetData(), aPlainMap.Count() );
}

const SfxItemSet& SvxRTFParser::GetRTFDefaults()
{
    if( !pRTFDefaults )
    {
        pRTFDefaults = new SfxItemSet( *pAttrPool, aWhichMap.GetData() );
        sal_uInt16 nId;
        if( 0 != ( nId = ((RTFPardAttrMapIds*)aPardMap.GetData())->nScriptSpace ))
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

/**/

SvxRTFStyleType::SvxRTFStyleType( SfxItemPool& rPool, const sal_uInt16* pWhichRange )
    : aAttrSet( rPool, pWhichRange )
{
    nOutlineNo = sal_uInt8(-1);         // nicht gesetzt
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
         pChildList = new SvxRTFItemStackList( 4, 16 );
    pChildList->Insert( pIns, pChildList->Count() );
}

#if 0
//cmc: This is the original. nEndCnt is redundantly assigned to itself, and
//pEndNd can leak if not equal to pSttNd.
void SvxRTFItemStackType::SetStartPos( const SvxPosition& rPos )
{
    delete pSttNd;
    pSttNd = rPos.MakeNodeIdx();
    nSttCnt = rPos.GetCntIdx();
    pEndNd = pSttNd;
    nEndCnt = nEndCnt;
}
#else
void SvxRTFItemStackType::SetStartPos( const SvxPosition& rPos )
{
    if (pSttNd != pEndNd)
        delete pEndNd;
    delete pSttNd;
    pSttNd = rPos.MakeNodeIdx();
    pEndNd = pSttNd;
    nSttCnt = rPos.GetCntIdx();
}
#endif

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
    sal_uInt16 nCount = pChildList ? pChildList->Count() : 0;
    for (sal_uInt16 i = 0; i < nCount; ++i)
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
    DBG_ASSERT( pChildList, "es gibt keine ChildListe" );

    sal_uInt16 n;
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
            // suche alle, die ueber den gesamten Bereich gesetzt sind
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
            } while( sal_True );

            if( !aMrgSet.Count() )
                return;
        }

        pLastNd = pTmp->pEndNd;
        nLastCnt = pTmp->nEndCnt;
    }

    if( pEndNd->GetIdx() != pLastNd->GetIdx() || nEndCnt != nLastCnt )
        return;

    // es kann zusammengefasst werden
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
            sal_uInt16 nWhich = aIter.GetCurItem()->Which();
            if( SFX_ITEM_SET != aAttrSet.GetItemState( nWhich, sal_False ))
                aAttrSet.Put( *aIter.GetCurItem() );

            if( aIter.IsAtEnd() )
                break;
            aIter.NextItem();
        } while( sal_True );
    }
}

/**/

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

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
