/*************************************************************************
 *
 *  $RCSfile: svxrtf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#define ITEMID_FONT 0
#include <ctype.h>

#ifdef MAC
#include "mac_start.h"

#ifndef __TYPES__
  #include <Types.h>
#endif

#ifndef __FONTS__
  #include <Fonts.h>
#endif

#include "mac_end.h"
#endif

#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen wg. SfxWhichIter
#include <svtools/whiter.hxx>
#endif
#ifndef _RTFTOKEN_H
#include <svtools/rtftoken.h>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#include "fontitem.hxx"

#include "svxrtf.hxx"
#include "svxids.hrc"



SV_IMPL_PTRARR( SvxRTFColorTbl, ColorPtr )
SV_IMPL_PTRARR( SvxRTFItemStackList, SvxRTFItemStackType* )


// -------------- Methoden --------------------

SvxRTFParser::SvxRTFParser( SfxItemPool& rPool, SvStream& rIn,
                            int bReadNewDoc )
    : SvRTFParser( rIn, 5 ),
    pAttrPool( &rPool ),
    pInsPos( 0 ),
    aColorTbl( 16, 4 ),
    aFontTbl( 16, 4 ),
    nVersionNo( 0 ),
    pSfxInfo( 0 )
{
    bNewDoc = bReadNewDoc;

    bChkStyleAttr = FALSE;
    bCalcValue = FALSE;
    bReadDocInfo = FALSE;

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

    delete pInsPos;
#ifndef SVX_LIGHT
    delete pSfxInfo;
#endif
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
    DBG_ASSERT( pInsPos, "keine Einfuegeposition" );

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

    // erzeuge aus den gesetzten WhichIds die richtige WhichId-Tabelle.
    BuildWhichTbl();

    return SvRTFParser::CallParser();
}

void SvxRTFParser::Continue( int nToken )
{
    SvRTFParser::Continue( nToken );

    if( SVPAR_PENDING != GetStatus() )
        SetAllAttrOfStk();
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
                    SetDefault( nToken, (short)nTokenValue );
                else
                    // wird nach einlesen der Fonttabelle gesetzt
                    nDfltFont = int(nTokenValue);
            }
            break;

    case RTF_DEFTAB:
    case RTF_DEFLANG:
            if( bNewDoc )
                SetDefault( nToken, (short)nTokenValue );
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
        aToken = ByteString::ConvertToUnicode( cCh,
                                            RTL_TEXTENCODING_MS_1252 );

        // kein Break, aToken wird als Text gesetzt
    case RTF_TEXTTOKEN:
        {
            InsertText();
            // alle angesammelten Attribute setzen
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
        if( bNewGroup )         // Verschachtelung !!
            _GetAttrSet();
        bNewGroup = TRUE;
        break;

    case '}':
        if( !bNewGroup )        // leere Gruppe ??
            AttrGroupEnd();
        bNewGroup = FALSE;
        break;

    case RTF_INFO:
#ifndef SVX_LIGHT
        if( bReadDocInfo && bNewDoc )
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
    case RTF_SHPRSLT:   // RTF_SHP fehlt noch !!
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
    int nOpenBrakets = 1;       // die erste wurde schon vorher erkannt !!
    SvxRTFStyleType* pStyle = new SvxRTFStyleType( *pAttrPool, aWhichMap.GetData() );

    bChkStyleAttr = FALSE;      // Attribute nicht gegen die Styles checken

    while( nOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '}':       if( --nOpenBrakets && IsParserWorking() )
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
                ++nOpenBrakets;
            }
            break;

        case RTF_SBASEDON:  pStyle->nBasedOn = USHORT(nTokenValue); break;
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
                        --nOpenBrakets;     // korrigieren!!
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
}

void SvxRTFParser::ReadColorTable()
{
    int nToken;
    BYTE nRed = 0, nGreen = 0, nBlue = 0;

    while( '}' != ( nToken = GetNextToken() ) && IsParserWorking() )
    {
        switch( nToken )
        {
        case RTF_RED:   nRed = BYTE(nTokenValue);       break;
        case RTF_GREEN: nGreen = BYTE(nTokenValue);     break;
        case RTF_BLUE:  nBlue = BYTE(nTokenValue);      break;

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
                ColorPtr pColor = 0;
                if( !pColor )
                    pColor = new Color( nRed, nGreen, nBlue );
                aColorTbl.Insert( pColor, aColorTbl.Count() );
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
    int nOpenBrakets = 1;       // die erste wurde schon vorher erkannt !!
    Font* pFont = new Font();
    short nFontNo;

    while( nOpenBrakets && IsParserWorking() )
    {
        switch( ( nToken = GetNextToken() ))
        {
        case '}':       if( --nOpenBrakets && IsParserWorking() )
                            // Style konnte vollstaendig gelesen werden,
                            // also ist das noch ein stabiler Status
                            SaveState( RTF_FONTTBL );
                        break;

        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                // Unknown und alle bekannten nicht ausgewerteten Gruppen
                // sofort ueberspringen
                else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ) &&
                        RTF_PANOSE != nToken && RTF_FALT != nToken &&
                        RTF_FALT != nToken && RTF_FNAME != nToken &&
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
                ++nOpenBrakets;
            }
            break;

        case RTF_FROMAN:    pFont->SetFamily( FAMILY_ROMAN );       break;
        case RTF_FSWISS:    pFont->SetFamily( FAMILY_SWISS );       break;
        case RTF_FMODERN:   pFont->SetFamily( FAMILY_MODERN );      break;
        case RTF_FSCRIPT:   pFont->SetFamily( FAMILY_SCRIPT );      break;
        case RTF_FDECOR:    pFont->SetFamily( FAMILY_DECORATIVE );  break;

        // bei technischen/symbolischen Font wird der CharSet ungeschaltet!!
        case RTF_FTECH:     pFont->SetCharSet( RTL_TEXTENCODING_SYMBOL );   // kein break
        case RTF_FNIL:      pFont->SetFamily( FAMILY_DONTKNOW );    break;

        case RTF_FCHARSET:
            if( -1 != nTokenValue )
                pFont->SetCharSet( rtl_getTextEncodingFromWindowsCharset(
                                        (BYTE)nTokenValue ) );
            break;

// AlternativName
//      case RTF_FALT:
//          break;

        case RTF_FPRQ:
            switch( nTokenValue )
            {
            case 1:     pFont->SetPitch( PITCH_FIXED );     break;
            case 2:     pFont->SetPitch( PITCH_VARIABLE );  break;
            }
            break;

        case RTF_F:         nFontNo = (short)nTokenValue;   break;

        case RTF_TEXTTOKEN:
            {
                if( DelCharAtEnd( aToken, ';' ).Len() )
                {
                    // alle Daten vom Font vorhanden, also ab in die Tabelle
                    pFont->SetName( aToken );
                    aFontTbl.Insert( nFontNo, pFont );
                    pFont = new Font();
                }
            }
            break;
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
    int nOpenBrakets = 1, nToken;       // die erste wurde schon vorher erkannt !!

    while( nOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '}':       --nOpenBrakets; break;
        case '{':
            {
                // gleich herausfiltern
                ReadUnknownData();
                nToken = GetNextToken();
                if( '}' != nToken )
                    eState = SVPAR_ERROR;
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

DateTime& SvxRTFParser::GetDateTimeStamp( DateTime& rDT )
{
    DateTime aDT;
    BOOL bWeiter = TRUE;
    int nToken;
    while( bWeiter && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case RTF_YR:    aDT.SetYear( (USHORT)nTokenValue  );    break;
        case RTF_MO:    aDT.SetMonth( (USHORT)nTokenValue  );   break;
        case RTF_DY:    aDT.SetDay( (USHORT)nTokenValue  );     break;
        case RTF_HR:    aDT.SetHour( (USHORT)nTokenValue );     break;
        case RTF_MIN:   aDT.SetMin( (USHORT)nTokenValue  );     break;
        default:
            bWeiter = FALSE;
        }
    }
    rDT = aDT;
    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
    return rDT;
}

void SvxRTFParser::ReadInfo( const sal_Char* pChkForVerNo )
{
#ifndef SVX_LIGHT
    int nOpenBrakets = 1, nToken;       // die erste wurde schon vorher erkannt !!
    pSfxInfo = new SfxDocumentInfo;
    String sStr, sComment;
    long nVersNo;
    SfxStamp aCreate, aModified;
    DateTime aDT;

    while( nOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '}':       --nOpenBrakets; break;
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
                ++nOpenBrakets;
            }
            break;

        case RTF_TITLE:
            pSfxInfo->SetTitle( GetTextToEndGroup( sStr ) );
            break;
        case RTF_SUBJECT:
            pSfxInfo->SetTheme( GetTextToEndGroup( sStr ) );
            break;
        case RTF_AUTHOR:
            aCreate.SetName( GetTextToEndGroup( sStr ) );
            break;
        case RTF_OPERATOR:
            aModified.SetName( GetTextToEndGroup( sStr ) );
            break;
        case RTF_KEYWORDS:
            pSfxInfo->SetKeywords( GetTextToEndGroup( sStr ) );
            break;
        case RTF_DOCCOMM:
            pSfxInfo->SetComment( GetTextToEndGroup( sStr ) );
            break;

        case RTF_HLINKBASE:
            sBaseURL = GetTextToEndGroup( sStr ) ;
            break;

        case RTF_CREATIM:
            aCreate.SetTime( GetDateTimeStamp( aDT ) );
            break;

        case RTF_REVTIM:
            aModified.SetTime( GetDateTimeStamp( aDT ) );
            break;

        case RTF_PRINTIM:
            {
                SfxStamp aTmp;
                aTmp.SetTime( GetDateTimeStamp( aDT ) );
                pSfxInfo->SetPrinted( aTmp );
            }
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

    pSfxInfo->SetCreated( aCreate );
    pSfxInfo->SetChanged( aModified );

    if( pChkForVerNo &&
        COMPARE_EQUAL == sComment.CompareToAscii( pChkForVerNo ))
        nVersionNo = nVersNo;

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
#endif
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
#ifdef MAC
        Str255 aMacString;
        GetFontName( nId, aMacString );
        if( 0 != aMacString[0] )        // FontName gefunden ?
        {
            String aFontNm( aMacString.GetBuffer() + 1,
                            (xub_StrLen)aMacString.GetChar(0),
                            RTL_TEXTENCODING_APPLE_ROMAN );


            pDfltFont->SetName( aFontNm );
            pDfltFont->SetFamily( FAMILY_DONTKNOW );
        }
        else
#endif
        {
            const SvxFontItem& rDfltFont = (const SvxFontItem&)
                            pAttrPool->GetDefaultItem(
                        ((RTFPlainAttrMapIds*)aPlainMap.GetData())->nFont );
            pDfltFont->SetName( rDfltFont.GetStyleName() );
            pDfltFont->SetFamily( rDfltFont.GetFamily() );
        }
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
        pNew = new SvxRTFItemStackType( *pAttrPool, aWhichMap.GetData(), *pInsPos );

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
                rSet.ClearItem( nWhich );       // loeschen
        }
    }
    else
    {
        // alle Attribute, die schon vom Style definiert sind, aus dem
        // akt. AttrSet entfernen
        SfxItemSet &rStyleSet = pStyle->aAttrSet;
        const SfxPoolItem* pSItem;
        for( USHORT nWhich = aIter.GetCurWhich(); nWhich; nWhich = aIter.NextWhich() )
        {
            if( SFX_ITEM_SET == rStyleSet.GetItemState( nWhich, TRUE, &pSItem ))
            {
                // JP 22.06.99: im Style und im Set gleich gesetzt -> loeschen
                if( SFX_ITEM_SET == rSet.GetItemState( nWhich, FALSE, &pItem )
                    && *pItem == *pSItem )
                    rSet.ClearItem( nWhich );       // loeschen
            }
                // Bug 59571 - falls nicht im Style gesetzt und gleich mit
                //              dem PoolDefault -> auch dann loeschen
            else if( SFX_WHICH_MAX > nWhich &&
                    SFX_ITEM_SET == rSet.GetItemState( nWhich, FALSE, &pItem ) &&
                     rPool.GetDefaultItem( nWhich ) == *pItem )
                rSet.ClearItem( nWhich );       // loeschen
        }
    }
}

void SvxRTFParser::AttrGroupEnd()   // den akt. Bearbeiten, vom Stack loeschen
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
                break;          // keine Attribute oder Bereich

            // setze nur die Attribute, die unterschiedlich zum Parent sind
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

            // setze alle Attribute, die von Start bis hier
            // definiert sind.
            int bCrsrBack = !pInsPos->GetCntIdx();
            if( bCrsrBack )
            {
                // am Absatzanfang ? eine Position zurueck
                ULONG nNd = pInsPos->GetNodeIdx();
                MovePos( FALSE );
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
                            for( USHORT n = 0; n < aPardMap.Count() &&
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
                                    *pOld, *pInsPos, TRUE );
                        pNew->aAttrSet.SetParent( pOld->aAttrSet.GetParent() );

                        // loesche aus pNew alle Absatz Attribute
                        for( USHORT n = 0; n < aPardMap.Count() &&
                                            pNew->aAttrSet.Count(); ++n )
                            if( aPardMap[n] )
                                pNew->aAttrSet.ClearItem( aPardMap[n] );

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
                                _ClearStyleAttr( *pOld );

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

                if( IsChkStyleAttr() )
                    _ClearStyleAttr( *pOld );

                if( pAkt )
                    pAkt->Add( pOld );
                else
                    // letzter vom Stack, also zwischenspeichern, bis der
                    // naechste Text eingelesen wurde. (keine Attribute
                    // aufspannen!!)
                    aAttrSetList.Insert( pOld, aAttrSetList.Count() );

                pOld = 0;
            }

            if( bCrsrBack )
                // am Absatzanfang ? eine Position zurueck
                MovePos( TRUE );

        } while( FALSE );

        if( pOld )
            delete pOld;

        bNewGroup = FALSE;
    }
}

void SvxRTFParser::SetAllAttrOfStk()        // end all Attr. and set it into doc
{
    // noch alle Attrbute vom Stack holen !!
    while( aAttrStack.Count() )
        AttrGroupEnd();

    for( USHORT n = aAttrSetList.Count(); n; )
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
        for( USHORT n = 0; n < rSet.pChildList->Count(); ++n )
            SetAttrSet( *(*rSet.pChildList)[ n ] );
}

    // wurde noch kein Text eingefuegt ? (SttPos vom obersten StackEintrag!)
int SvxRTFParser::IsAttrSttPos()
{
    SvxRTFItemStackType* pAkt = aAttrStack.Top();
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
    aWhichMap.Insert( (USHORT)0, (USHORT)0 );

    // Aufbau einer Which-Map 'rWhichMap' aus einem Array von
    // 'pWhichIds' von Which-Ids. Es hat die Lange 'nWhichIds'.
    // Die Which-Map wird nicht geloescht.
    SvParser::BuildWhichTbl( aWhichMap, (USHORT*)aPardMap.GetData(), aPardMap.Count() );
    SvParser::BuildWhichTbl( aWhichMap, (USHORT*)aPlainMap.GetData(), aPlainMap.Count() );
}

/**/

SvxRTFStyleType::SvxRTFStyleType( SfxItemPool& rPool, const USHORT* pWhichRange )
    : aAttrSet( rPool, pWhichRange )
{
    nOutlineNo = BYTE(-1);          // nicht gesetzt
    nBasedOn = 0;
    nNext = 0;
    bIsCharFmt = FALSE;
}


SvxRTFItemStackType::SvxRTFItemStackType(
        SfxItemPool& rPool, const USHORT* pWhichRange,
        const SvxPosition& rPos )
    : aAttrSet( rPool, pWhichRange ),
    pChildList( 0 ),
    nStyleNo( 0)
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
         pChildList = new SvxRTFItemStackList;
    pChildList->Insert( pIns, pChildList->Count() );
}

void SvxRTFItemStackType::SetStartPos( const SvxPosition& rPos )
{
    delete pSttNd;
    pSttNd = rPos.MakeNodeIdx();
    nSttCnt = rPos.GetCntIdx();
    pEndNd = pSttNd;
    nEndCnt = nEndCnt;
}


void SvxRTFItemStackType::Compress( const SvxRTFParser& rParser )
{
    DBG_ASSERT( pChildList, "es gibt keine ChildListe" );

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

        if( n )
        {
            // suche alle, die ueber den gesamten Bereich gesetzt sind
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

/**/

RTFPlainAttrMapIds::RTFPlainAttrMapIds( const SfxItemPool& rPool )
{
    nCaseMap = rPool.GetTrueWhich( SID_ATTR_CHAR_CASEMAP, FALSE );
    nBgColor = 0;
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
    nWeight = rPool.GetTrueWhich( SID_ATTR_CHAR_WEIGHT, FALSE );
    nWordlineMode = rPool.GetTrueWhich( SID_ATTR_CHAR_WORDLINEMODE, FALSE );
    nAutoKerning = rPool.GetTrueWhich( SID_ATTR_CHAR_AUTOKERN, FALSE );
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
}


