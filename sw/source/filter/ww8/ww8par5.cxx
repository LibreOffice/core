/*************************************************************************
 *
 *  $RCSfile: ww8par5.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: cmc $ $Date: 2001-03-05 13:13:24 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include <ctype.h>              // tolower
#include <stdio.h>              // sscanf()

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#include <tools/solar.h>
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _LINKMGR_HXX //autogen
#include <so3/linkmgr.hxx>
#endif

#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif

#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>              // fuer SwPam
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>          // class SwFmtFld
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>           // class SwDateTimeField
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>          // class SwPageNumberField
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>           // class SwGetRefField
#endif
#ifndef _BOOKMRK_HXX            // class SwBookmark
#include <bookmrk.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>           // class SwSetExpField
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>            // class SwDBField
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>           // ReadFilterFlags
#endif
#ifndef _SECTION_HXX
#include <section.hxx>          // class SwSection
#endif
#ifndef _WW8SCAN_HXX
#include <ww8scan.hxx>          // WW8FieldDesc
#endif
#ifndef _FLTSHELL_HXX
#include <fltshell.hxx>         // fuer den Attribut Stack
#endif
#ifndef _WW8PAR_HXX
#include <ww8par.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FMTINFMT_HXX
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTURL_HXX
#include <fmturl.hxx>
#endif
#ifndef _CHPFLD_HXX
#include <chpfld.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _TXTFTN_HXX
#include <txtftn.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _SWDBDATA_HXX
#include <swdbdata.hxx>
#endif
#ifndef _FMTRUBY_HXX
#include <fmtruby.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif

#define WWF_INVISIBLE 86            // Bit-Nummer fuer Invisible ( IniFlags )
#define MAX_FIELDLEN 64000

using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::ucb;
using namespace ::rtl;

extern void sw3io_ConvertFromOldField( SwDoc& rDoc, USHORT& rWhich,
                                USHORT& rSubType, ULONG &rFmt,
                                USHORT nVersion );

class _ReadFieldParams
{
    String aData;
    xub_StrLen nLen, nFnd, nNext, nSavPtr;
//  sal_Unicode cLastChar;

public:
    _ReadFieldParams( const String& rData );
    ~_ReadFieldParams();

    xub_StrLen GoToTokenParam();
    long SkipToNextToken();
    xub_StrLen GetTokenSttPtr() const   { return nFnd;  }
    void SetNextPtr( xub_StrLen _nNext ) { nNext = _nNext; }

    xub_StrLen FindNextStringPiece( xub_StrLen _nStart = STRING_NOTFOUND );
    BOOL GetTokenSttFromTo(xub_StrLen* _pFrom, xub_StrLen* _pTo, xub_StrLen _nMax);

    String GetResult() const;
};


_ReadFieldParams::_ReadFieldParams( const String& _rData )
    : aData( _rData ), nLen( _rData.Len() ), nNext( 0 )
{
    /*
        erstmal nach einer oeffnenden Klammer oder einer Leerstelle oder einem
        Anfuehrungszeichen oder einem Backslash suchen, damit der Feldbefehl
        (also INCLUDEPICTURE bzw EINFUeGENGRAFIK bzw ...) ueberlesen wird
    */
    while( (nLen > nNext) && (aData.GetChar( nNext ) == ' ') )
        ++nNext;

    sal_Unicode c;
    while(     nLen > nNext
            && (c = aData.GetChar( nNext )) != ' '
            && c != '"'
            && c != '\\'
            && c != 132  )
        ++nNext;

    nFnd      = nNext;
    nSavPtr   = nNext;
//  cLastChar = aData.GetChar( nSavPtr );
}


_ReadFieldParams::~_ReadFieldParams()
{
//  aData.SetChar( nSavPtr, cLastChar );
}


String _ReadFieldParams::GetResult() const
{
    return    (STRING_NOTFOUND == nFnd)
            ? aEmptyStr
            : aData.Copy( nFnd, (nSavPtr - nFnd) );
}


xub_StrLen _ReadFieldParams::GoToTokenParam()
{
    xub_StrLen nOld = nNext;
    if( -2 == SkipToNextToken() )
        return GetTokenSttPtr();
    nNext = nOld;
    return STRING_NOTFOUND;
}

long _ReadFieldParams::SkipToNextToken() // ret: -2: NOT a '\' parameter but normal Text
{
    long nRet = -1;     // Ende
//  aData.SetChar( nSavPtr, cLastChar );
    if(    (STRING_NOTFOUND != nNext)
        && (nLen > nNext)
        && STRING_NOTFOUND != ( nFnd = FindNextStringPiece( nNext ) ))
    {
        nSavPtr = nNext;
//      cLastChar = aData.GetChar( nSavPtr );

        if(    '\\' == aData.GetChar( nFnd )
            && '\\' != aData.GetChar( nFnd + 1 ) ) // Options-Parameter gefunden
        {
            nRet = aData.GetChar( ++nFnd );
            nNext = ++nFnd;             // und dahinter setzen
        }
        else
        {
            nRet = -2;
            if(    (STRING_NOTFOUND != nSavPtr )
                && ('"' == aData.GetChar( nSavPtr - 1 )) )
            {
                nSavPtr--;
//              cLastChar = aData.GetChar( nSavPtr );
            }
//          aData.SetChar( nSavPtr, 0 );
        }
    }
    return nRet;
}

// FindNextPara sucht naechsten Backslash-Parameter oder naechste Zeichenkette
// bis zum Blank oder naechsten "\" oder zum schliessenden Anfuehrungszeichen
// oder zum String-Ende von pStr.
//
// Ausgabe ppNext (falls ppNext != 0) Suchbeginn fuer naechsten Parameter bzw. 0
//
// Returnwert: 0 falls String-Ende erreicht,
//             ansonsten Anfang des Paramters bzw. der Zeichenkette
//
xub_StrLen _ReadFieldParams::FindNextStringPiece(const xub_StrLen nStart)
{
    xub_StrLen  n = ( STRING_NOTFOUND == nStart ) ? nFnd : nStart;  // Anfang
    xub_StrLen n2;          // Ende

    nNext = STRING_NOTFOUND;        // Default fuer nicht gefunden

    while( (nLen > n) && (aData.GetChar( n ) == ' ') )
        ++n;

    if( nLen == n )
        return STRING_NOTFOUND;     // String End reached!

    if(     (aData.GetChar( n ) == '"')     // Anfuehrungszeichen vor Para?
        ||  (aData.GetChar( n ) == 132) )
    {
        n++;                        // Anfuehrungszeichen ueberlesen
        n2 = n;                     // ab hier nach Ende suchen
        while(     (nLen > n2)
                && (aData.GetChar( n2 ) != '"')
                && (aData.GetChar( n2 ) != 147) )
            n2++;                   // Ende d. Paras suchen
    }
    else                        // keine Anfuehrungszeichen
    {
        n2 = n;                     // ab hier nach Ende suchen
        while( (nLen > n2) && (aData.GetChar( n2 ) != ' ') ) // Ende d. Paras suchen
        {
            if( aData.GetChar( n2 ) == '\\' )
            {
                if( aData.GetChar( n2+1 ) == '\\' )
                    n2 += 2;        // Doppel-Backslash -> OK
                else
                {
                    if( n2 > n )
                        n2--;
                    break;          // einfach-Backslash -> Ende
                }
            }
            else
                n2++;               // kein Backslash -> OK
        }
    }
    if( nLen > n2 )
    {
        if(aData.GetChar( n2 ) != ' ') n2++;
        nNext = n2;
    }
    return n;
}



// read parameters "1-3" or 1-3 with both values between 1 and nMax
BOOL _ReadFieldParams::GetTokenSttFromTo(USHORT* pFrom, USHORT* pTo, USHORT nMax)
{
    USHORT nStart = 0;
    USHORT nEnd   = 0;
    xub_StrLen n = GoToTokenParam();
    if( STRING_NOTFOUND != n )
    {

        String sParams( GetResult() );

        xub_StrLen nIndex = 0;
        String sStart( sParams.GetToken(0, '-', nIndex) );
        if( STRING_NOTFOUND != nIndex )
        {
            nStart = sStart.ToInt32();
            nEnd   = sParams.Copy( nIndex ).ToInt32();
        }
    }
    if( pFrom ) *pFrom = nStart;
    if( pTo )   *pTo   = nEnd;

    return nStart && nEnd && (nMax >= nStart) && (nMax >= nEnd);
}




/*  */


//----------------------------------------
//              Bookmarks
//----------------------------------------

long SwWW8ImplReader::Read_Book( WW8PLCFManResult* pRes, BOOL bStartAttr )
{
    if( !bStartAttr )
    {
        ASSERT( bStartAttr, "Read_Book::Nanu ?" );
        pEndStck->SetAttr( *pPaM->GetPoint(), RES_FLTR_BOOKMARK );
        return 0;
    }
    WW8PLCFx_Book* pB = pPlcxMan->GetBook();    // muesste auch ueber pRes.nCo2OrIdx gehen
    if( !pB ){
        ASSERT( pB, "WW8PLCFx_Book - Pointer nicht da" );
        return 0;
    }

    if( pB->GetIsEnd() ){
#if 0
        rDoc.Insert( *pPaM, 'E', CHARSET_ANSI );
#endif
        pEndStck->SetAttr( *pPaM->GetPoint(), RES_FLTR_BOOKMARK,
                           TRUE, pB->GetHandle() );
        return 0;
    }

    eBookStatus eB = pB->GetStatus();
    if( ( eB & BOOK_IGNORE ) != 0 )
        return 0;                               // Bookmark zu ignorieren
#if 0
        rDoc.Insert( *pPaM, 'A', CHARSET_ANSI );
#endif

    const String* pName = pB->GetName();
    if(    !pName
        || pName->EqualsIgnoreCaseAscii( "_Toc" ) ) // "_Toc*" ist ueberfluessig
        return 0;

//JP 16.11.98: ToUpper darf auf keinen Fall gemacht werden, weil der Bookmark-
//              name ein Hyperlink-Ziel sein kann!

    // Fuer UEbersetzung Bookmark -> Variable setzen
    long nLen = pB->GetLen();
    if( nLen > MAX_FIELDLEN )
        nLen = MAX_FIELDLEN;

    String sTxt;
    long nOldPos = pStrm->Tell();
    nLen = pSBase->WW8ReadString( *pStrm, sTxt, pB->GetStartPos(), nLen,
                                    eStructCharSet );

    pStrm->Seek( nOldPos );
    String aVal;
    QuoteString( aVal, sTxt );          // Nur druckbare Zeichen !
                                        // incl. Zeichensatz - UEbersetzung

    pEndStck->NewAttr( *pPaM->GetPoint(), SwFltBookmark( *pName, aVal,
                        pB->GetHandle(), ( eB & BOOK_ONLY_REF ) != 0 ) );
    return 0;
}

//----------------------------------------------------------------------
//    allgemeine Hilfsroutinen zum Auseinanderdroeseln der Parameter
//----------------------------------------------------------------------

// ConvertFFileName uebersetzt FeldParameter-Namen u. ae. in den
// System-Zeichensatz.
// Gleichzeitig werden doppelte Backslashes in einzelne uebersetzt.
void SwWW8ImplReader::ConvertFFileName( String& rName, const String& rOrg )
{
    rName = rOrg;
    rName.SearchAndReplaceAllAscii( "\\\\", String( '\\' ));

    // ggfs. anhaengende Anfuehrungszeichen entfernen
    if( rName.Len() &&  '"' == rName.GetChar( rName.Len()-1 ))
        rName.Erase( rName.Len()-1, 1);
}

// ConvertUFNneme uebersetzt FeldParameter-Namen u. ae. in den
// System-Zeichensatz und Upcased sie ( z.B. fuer Ref-Felder )
void SwWW8ImplReader::ConvertUFName( String& rName )
{
    GetAppCharClass().toUpper( rName );
}

void lcl_ConvertSequenceName( SwWW8ImplReader& rReader, String& rSequenceName )
{
    rReader.ConvertUFName( rSequenceName );
    if(    '0' <= rSequenceName.GetChar( 0 )
        && '9' >= rSequenceName.GetChar( 0 ) )
        rSequenceName.Insert('_', 0);
}
/*
// FindNextPara sucht Naechsten Parameter und setzt
// eine 0 hinter sein Ende ( im Original-String )!
// Bei Anfuehrungszeichen
// ( normal oder typographisch ) wird der ganze String genommen.
// Escape-Chars ( Backslash ) werden nicht beachtet.
// Rueckgabe: wenn ppNext != 0 dann Suchbeginn fuer naechsten Parameter
//            Return 0 fuer nicht gefunden oder Anfang des Paramters
static char* FindNextPara( char* pStr, char** ppNext = 0 )
{
    if( ppNext )
        *ppNext = 0;                // Default fuer nicht gefunden
    char* p = pStr;                 // Anfang
    char* p2;                       // Ende

    while( *p == ' ' )              // Spaces vor Para ueberlesen
        p++;
    if( !*p )
        return 0;
    if( *p == '"' || *(BYTE*)p == 132 ){    // Anfuehrungszeichen vor Para
        p++;                        // Anfuehrungszeichen ueberlesen
        p2 = p;                     // ab hier nach Ende suchen
        while( *p2 != '"' && *(BYTE*)p2 != 147 && *p2 != 0 )
            p2++;                   // Ende d. Paras suchen
    }else{                          // keine Anfuehrungszeichen
        p2 = p;                     // ab hier nach Ende suchen
        while( *p2 != ' ' && *p2 != 0 ){ // Ende d. Paras suchen
            if( *p2 == '\\' ){
                if( *(p2 + 1) == '\\' )
                    p2 += 2;        // Doppel-Backslash -> OK
                else
                    break;          // einfach-Backslash -> Ende
            }else{
                p2++;               // kein Backslash -> OK
            }
        }
    }
    *p2 = 0;                        // p ist jetzt String auf Para
    if( ppNext )
        *ppNext = p2 + 1;
    return p;
}

// FindNextPara2 funktioniert wie FindNextPara, jedoch wird beim
// Weglassen von Anfuehrungszeichen bis zum Backslash gesucht
// und nicht nur bis zum Space. Wird fuer Macro-AnzeigeTexte benutzt.
static char* FindNextPara2( char* pStr, char** ppNext = 0 )
{
    if( ppNext )
        *ppNext = 0;                // Default fuer nicht gefunden
                                    // #64447#
    char* p = pStr;                 // Anfang
    char* p2;                       // Ende

    while( *p == ' ' )              // Spaces vor Para ueberlesen
        p++;
    if( !*p )
        return 0;
    if( *p == '"' || *(BYTE*)p == 132 ){    // Anfuehrungszeichen vor Para
        p++;                        // Anfuehrungszeichen ueberlesen
        p2 = p;                     // ab hier nach Ende suchen
        while( *p2 != '"' && *(BYTE*)p2 != 147 && *p2 != 0 )
            p2++;                   // Ende d. Paras suchen
    }else{                          // keine Anfuehrungszeichen
        p2 = p;                     // ab hier nach Ende suchen
        while( *p2 != 0 && *p2 != '\\' )
            p2++;                   // Ende d. Paras suchen
    }
    *p2 = 0;                        // p ist jetzt String auf Para
    if( ppNext )
        *ppNext = p2 + 1;
    return p;
}

// Find1stPara sucht 1. Parameter und setzt
// eine 0 hinter sein Ende ( im Original-String )!
// Dabei wird das 2. Wort des Strings gesucht. Bei Anfuehrungszeichen
// ( normal oder typographisch ) wird der ganze String genommen.
// Escape-Chars ( Backslash ) werden nicht beachtet.
// Rueckgabe: wenn ppNext != 0 dann Suchbeginn fuer naechsten Parameter
//            Return 0 fuer nicht gefunden oder Anfang des Paramters
static char* Find1stPara( char* pStr, char** ppNext = 0 )
{
    // Anfang
    char* p = pStr;

    while( *p == ' ' )              // evtl. Spaces vor Feldtyp ueberlesen
        p++;
    while( *p != ' ' && *p != 0     // FeldTyp ueberlesen
           && *p != '"' && *(BYTE*)p != 132 )

        p++;

    // Ende
    char* p2 = pStr;

    while( *p == ' ' )              // Spaces vor Para ueberlesen
        p++;
    if( !*p )
        return 0;
    if( *p == '"' || *(BYTE*)p == 132 ){    // Anfuehrungszeichen vor Para
        p++;                        // Anfuehrungszeichen ueberlesen
        p2 = p;                     // ab hier nach Ende suchen
        while( *p2 != '"' && *(BYTE*)p2 != 147 && *p2 != 0 )
            p2++;                   // Ende d. Paras suchen
    }else{                          // keine Anfuehrungszeichen
        p2 = p;                     // ab hier nach Ende suchen
        while( *p2 != ' ' && *p2 != 0 )
            p2++;                   // Ende d. Paras suchen
    }
    *p2 = 0;                        // p ist jetzt String auf Para
    if( ppNext )
        *ppNext = p2 + 1;
    return p;
}
*/

// FindParaStart() finds 1st Parameter that follows '\' and cToken
// and returns start of this parameter or STRING_NOT_FOUND.
xub_StrLen FindParaStart( const String& rStr, sal_Unicode cToken, sal_Unicode cToken2 )
{
    BOOL bStr = FALSE;          // innerhalb String ignorieren

    for( xub_StrLen nBuf=0; nBuf+1 < rStr.Len(); nBuf++ )
    {
        if( rStr.GetChar( nBuf ) == '"' )
            bStr = !bStr;

        if(    !bStr
            && rStr.GetChar( nBuf ) == '\\'
            && (    rStr.GetChar( nBuf + 1 ) == cToken
                 || rStr.GetChar( nBuf + 1 ) == cToken2 ) )
        {
            nBuf += 2;
            // skip spaces between cToken and it's parameters
            while(    nBuf < rStr.Len()
                   && rStr.GetChar( nBuf ) == ' ' )
                nBuf++;
            return nBuf < rStr.Len() ? nBuf : STRING_NOTFOUND; // return start of parameters
        }
    }
    return STRING_NOTFOUND;
}

// FindPara() findet den ersten Parameter mit '\' und cToken. Es wird
// ein neuer String allokiert ( der vom Aufrufer deallokiert werden muss )
// und alles, was zum Parameter gehoert, wird in ihm zurueckgeliefert.
String FindPara( const String& rStr, sal_Unicode cToken, sal_Unicode cToken2 )
{
    xub_StrLen n2;                                          // Ende
    xub_StrLen n = FindParaStart( rStr, cToken, cToken2 );  // Anfang
    if( STRING_NOTFOUND == n )
        return aEmptyStr;

    if(    rStr.GetChar( n ) == '"'
        || rStr.GetChar( n ) == 132 )
    {                               // Anfuehrungszeichen vor Para
        n++;                        // Anfuehrungszeichen ueberlesen
        n2 = n;                     // ab hier nach Ende suchen
        while(     n2 < rStr.Len()
                && rStr.GetChar( n2 ) != 147
                && rStr.GetChar( n2 ) != '"' )
            n2++;                   // Ende d. Paras suchen
    }
    else
    {                           // keine Anfuehrungszeichen
        n2 = n;                     // ab hier nach Ende suchen
        while(     n2 < rStr.Len()
                && rStr.GetChar( n2 ) != ' ' )
            n2++;                   // Ende d. Paras suchen
    }
    return rStr.Copy( n, n2-n );
}


static SvxExtNumType GetNumTypeFromName( const String& rStr, BOOL bAllowPageDesc = FALSE )
{
    SvxExtNumType eTyp = bAllowPageDesc ? SVX_NUM_PAGEDESC : SVX_NUM_ARABIC;
    if( rStr.EqualsIgnoreCaseAscii( "Arabi", 0, 5 ) )  // Arabisch, Arabic
        eTyp = SVX_NUM_ARABIC;
    else if( rStr.EqualsIgnoreCaseAscii( "misch", 2, 5 ) )  // r"omisch
        eTyp = SVX_NUM_ROMAN_LOWER;
    else if( rStr.EqualsIgnoreCaseAscii( "MISCH", 2, 5 ) )  // R"OMISCH
        eTyp = SVX_NUM_ROMAN_UPPER;
    else if( rStr.EqualsIgnoreCaseAscii( "alphabeti", 0, 9 ) )// alphabetisch, alphabetic
        eTyp =  ( rStr.GetChar( 0 ) == 'A' )
                ? SVX_NUM_CHARS_UPPER_LETTER_N
                : SVX_NUM_CHARS_LOWER_LETTER_N;
    else if( rStr.EqualsIgnoreCaseAscii( "roman", 0, 5 ) )  // us
        eTyp =  ( rStr.GetChar( 0 ) == 'R' )
                ? SVX_NUM_ROMAN_UPPER
                : SVX_NUM_ROMAN_LOWER;
    return eTyp;
}


static SvxExtNumType GetNumberPara( String& rStr, BOOL bAllowPageDesc = FALSE )
{
    String s( FindPara( rStr, '*', '*' ) );     // Ziffernart
    SvxExtNumType aType = GetNumTypeFromName( s, bAllowPageDesc );
    return aType;
}


ULONG MSDateTimeFormatToSwFormat( String& rParams, SwDoc& rDoc, USHORT nLang )
{
    // get Doc Numberformatter
    SvNumberFormatter* pFormatter = rDoc.GetNumberFormatter();

    // tell the Formatter about the new entry
    UINT16 nCheckPos = 0;
    INT16  nType = NUMBERFORMAT_DEFINED;
    ULONG  nKey = 0;

//  if( !
    pFormatter->PutandConvertEntry(  rParams, nCheckPos, nType, nKey,
            nLang, LANGUAGE_SYSTEM )
;//)
//      nKey = 0;

    return nKey;
}

static short GetTimeDatePara( SwDoc& rDoc, String& rStr, ULONG& rFormat )
{
    String aParams( FindPara( rStr, '@', '@' ) );// Date/Time
    if( 0 == aParams.Len() )
    {                               // No Date / Time
        rFormat =  NF_DATE_SYS_NNNNDMMMMYYYY;
        return NUMBERFORMAT_DATE;
    }

    const SvxLanguageItem& rLang = (SvxLanguageItem&)(rDoc.GetAttrPool().
                    GetDefaultItem( RES_CHRATR_LANGUAGE ));

    ULONG nNumFmtIdx = MSDateTimeFormatToSwFormat( aParams, rDoc,
                                                    rLang.GetValue() );

    short nNumFmtType = NUMBERFORMAT_UNDEFINED;
    if( nNumFmtIdx )
    {
        SvNumberFormatter* pFormatter = rDoc.GetNumberFormatter();
        nNumFmtType = pFormatter->GetType( nNumFmtIdx );
    }
    rFormat = nNumFmtIdx;

    return nNumFmtType;

}



//-----------------------------------------
//              Felder
//-----------------------------------------
// Am Ende des Einlesens entsprechende Felder updaten ( z.Zt. die Referenzen )
void SwWW8ImplReader::UpdateFields()
{
//  rDoc.GetSysFldType( RES_GETREFFLD )->UpdateFlds();  // Referenzen
//  rDoc.UpdateFlds();                                  // SetExp-Fields
//  rDoc.UpdateFlds();              // alles ???
//  rDoc.UpdateExpFlds();                               // SetExp-Fields
    rDoc.SetUpdateExpFldStat();                 // JP: neu fuer alles wichtige
    rDoc.SetInitDBFields( TRUE );               // Datenbank-Felder auch
}

typedef eF_ResT (SwWW8ImplReader:: *FNReadField)( WW8FieldDesc*, String& );
static FNReadField aWW8FieldTab[93] = {
0,
0,
0,
&SwWW8ImplReader::Read_F_Ref,               // 3
0,
0,
&SwWW8ImplReader::Read_F_Set,               // 6
0,
&SwWW8ImplReader::Read_F_Tox,               // 8
0,
0,
0,
&SwWW8ImplReader::Read_F_Seq,               // 12
&SwWW8ImplReader::Read_F_Tox,               // 13
&SwWW8ImplReader::Read_F_DocInfo,           // 14
&SwWW8ImplReader::Read_F_DocInfo,           // 15
&SwWW8ImplReader::Read_F_DocInfo,           // 16
&SwWW8ImplReader::Read_F_Author,            // 17
&SwWW8ImplReader::Read_F_DocInfo,           // 18
&SwWW8ImplReader::Read_F_DocInfo,           // 19
&SwWW8ImplReader::Read_F_DocInfo,           // 20
&SwWW8ImplReader::Read_F_DocInfo,           // 21
&SwWW8ImplReader::Read_F_DocInfo,           // 22
&SwWW8ImplReader::Read_F_DocInfo,           // 23
&SwWW8ImplReader::Read_F_DocInfo,           // 24
&SwWW8ImplReader::Read_F_DocInfo,           // 25
&SwWW8ImplReader::Read_F_Anz,               // 26
&SwWW8ImplReader::Read_F_Anz,               // 27
&SwWW8ImplReader::Read_F_Anz,               // 28
&SwWW8ImplReader::Read_F_FileName,          // 29
&SwWW8ImplReader::Read_F_TemplName,         // 30
&SwWW8ImplReader::Read_F_DateTime,          // 31
&SwWW8ImplReader::Read_F_DateTime,          // 32
&SwWW8ImplReader::Read_F_CurPage,           // 33
0,
0,
0,
&SwWW8ImplReader::Read_F_PgRef,             // 37
&SwWW8ImplReader::Read_F_InputVar,          // 38
&SwWW8ImplReader::Read_F_Input,             // 39
0,
&SwWW8ImplReader::Read_F_DBNext,            // 41
0,
0,
&SwWW8ImplReader::Read_F_DBNum,             // 44
0,
0,
0,
0,
&SwWW8ImplReader::Read_F_Equation,          // 49
0,
&SwWW8ImplReader::Read_F_Macro,             // 51
&SwWW8ImplReader::Read_F_ANumber,           // 52
&SwWW8ImplReader::Read_F_ANumber,           // 53
&SwWW8ImplReader::Read_F_ANumber,           // 54
0,


0,      // 56: VERKNUePFUNG     // fehlt noch !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


&SwWW8ImplReader::Read_F_Symbol,            // 57
&SwWW8ImplReader::Read_F_Embedd,            // 58
&SwWW8ImplReader::Read_F_DBField,           // 59
0,
0,
0,
0,
0,
0,
0,
&SwWW8ImplReader::Read_F_IncludePicture,    // 67
&SwWW8ImplReader::Read_F_IncludeText,       // 68
0,
&SwWW8ImplReader::Read_F_FormTextBox,       // 70
&SwWW8ImplReader::Read_F_FormCheckBox,      // 71
&SwWW8ImplReader::Read_F_NoteReference,     // 72
0, /*&SwWW8ImplReader::Read_F_Tox*/
0,
0,
0,
0,
0,
0,
0,
0,
0,
&SwWW8ImplReader::Read_F_FormListBox,       // 83
0,                                          // 84
&SwWW8ImplReader::Read_F_DocInfo,           // 85
0,                                          // 86
&SwWW8ImplReader::Read_F_OCX,               // 87
&SwWW8ImplReader::Read_F_Hyperlink,         // 88
0,                                          // 89
0,                                          // 90
0,                                          // 91
0                                           // 92 - Dummy leer Methode
 };                                         // 92   == alle ueber 91



// Read_Field liest ein Feld ein oder, wenn es nicht gelesen werden kann,
// wird 0 zurueckgegeben, so dass das Feld vom Aufrufer textuell gelesen wird.
// Returnwert: Gesamtlaenge des Feldes ( zum UEberlesen )
long SwWW8ImplReader::Read_Field( WW8PLCFManResult* pRes, BOOL )
{
    ASSERT( ( sizeof( aWW8FieldTab ) / sizeof( *aWW8FieldTab ) == 93 ),
            "FeldFunc-Tabelle stimmt nicht" );
    if( nIniFlags & WW8FL_NO_FLD )
        return 0;

    WW8FieldDesc aF;

    WW8PLCFx_FLD* pF = pPlcxMan->GetFld();
    ASSERT( pF, "WW8PLCFx_FLD - Pointer nicht da" );
    if( !pF ) return 0;

    BOOL bOk = pF->GetPara( pRes->nCp2OrIdx, aF );

    USHORT n = ( aF.nId <= 91 ) ? aF.nId : 92; // alle > 91 werden 92
    USHORT nI = n / 32;                     // # des UINT32
    ULONG nMask = 1 << ( n % 32 );          // Maske fuer Bits

    if( nFieldTagAlways[nI] & nMask )       // Flag: Tag it
        return Read_F_Tag( &aF );           // Resultat nicht als Text

    if( !bOk || !aF.nId )                   // Feld kaputt
        return aF.nLen;                     // -> ignorieren

    if( aF.nId > 91
//      || ( aF.nOpt & 0x40 )               // 0x40-Result Nest -> OK
                             )              // WW: Nested Field
        if( nFieldTagBad[nI] & nMask )      // Flag: Tag it when bad
            return Read_F_Tag( &aF );       // Resultat nicht als Text
        else
            return aF.nLen;

    if( aWW8FieldTab[aF.nId] == 0           // keine Routine vorhanden
        || aF.bCodeNest )
    {
        if( nFieldTagBad[nI] & nMask )      // Flag: Tag it when bad
            return Read_F_Tag( &aF );       // Resultat nicht als Text
                                            // Lese nur Resultat
        if( aF.bResNest )
            return aF.nLen;                 // Result nested -> nicht brauchbar

        return aF.nLen - aF.nLRes - 1;  // so viele ueberlesen, das Resultfeld
                                        // wird wie Haupttext eingelesen

    }
    else
    {                                   // Lies Feld
        long nOldPos = pStrm->Tell();
        String aStr;
        aF.nLCode = pSBase->WW8ReadString( *pStrm, aStr, pPlcxMan->GetCpOfs()+
                                            aF.nSCode, aF.nLCode, eTextCharSet );

        eF_ResT eRes = (this->*aWW8FieldTab[aF.nId])( &aF, aStr );
        pStrm->Seek( nOldPos );

        switch ( eRes )
        {
        case F_OK:
                    return aF.nLen;                     // alles OK

        case F_TEXT:
                    return aF.nLen - aF.nLRes - 2;      // so viele ueberlesen,
                            // das Resultfeld wird wie Haupttext eingelesen
                            //JP 15.07.99: attributes can start at char 0x14
                            // so skip one char more back == "-2"

        case F_TAGTXT:
                    if(  ( nFieldTagBad[nI] & nMask ) ) // Flag: Tag bad
                        return Read_F_Tag( &aF );       // Taggen
                    return aF.nLen - aF.nLRes - 2;  // oder Text-Resultat

        case F_TAGIGN:
                    if(  ( nFieldTagBad[nI] & nMask ) ) // Flag: Tag bad
                        return Read_F_Tag( &aF );       // Taggen
                    return aF.nLen;                 // oder ignorieren

        case F_READ_FSPA:
                return aF.nLen - aF.nLRes - 2; // auf Char 1 positionieren

        default:    return aF.nLen;                     // ignorieren
        }
    }
/*
    if( !bOk || !aF.nId  || aF.nId > 84
        || ( aF.nOpt & 0x40 ) )             // WW sagt: Nested Field
        return aF.nLen;


    if( ( ( aF.nOpt & 0x08 ) )              // User Edited
        || aF.nId > 84                      // unbekannte Id
        || aWW8FieldTab[aF.nId] == 0            // keine Routine vorhanden
        || aF.bCodeNest ){


                                            // Lese nur Resultat
        if( aF.bResNest )
            return aF.nLen;                 // Result nested -> nicht brauchbar
        else
            return aF.nLen - aF.nLRes - 1;  // so viele ueberlesen, das Resultfeld
                                            // wird wie Haupttext eingelesen

    }else{                                  // Lese Feld
        long nOldPos = pStrm->Tell();
        pStrm->Seek( WW8Cp2Fc( pPlcxMan->GetCpOfs() + aF.nSCode,
                            pWwFib->fcMin ) );
        char* p = new char[ aF.nLCode + 10 ];
        pStrm->Read( p, aF.nLCode );
        for( long i=aF.nLCode; i < aF.nLCode + 10; i++ )
            p[i] = 0;                   // Nullen dahinter
        (this->*aWW8FieldTab[aF.nId])( &aF, p );
        delete[] p;
        pStrm->Seek( nOldPos );
    }
    return aF.nLen;
*/
}

//-----------------------------------------
//        Felder Taggen
//-----------------------------------------

// QuoteChar ist ohne Laengenschutz

void SwWW8ImplReader::QuoteChar( String& rStr, const sal_Unicode c, BOOL bAllowCr )
{
    if ( c < 0x0020 || c == 0x00fe || c == 0x00ff )  // < 32, >0xfe
    {
        switch( c )
        {
        case 0x0b:
        case 0x0c:
        case 0x0d: if(  bAllowCr )
                   {
                     rStr += '\n';
                     break;
                   }
                    // no break
        default:{
                    // als Hex-Zahl mit \x  davor
                    String sTmp( String::CreateFromInt32( c, 16 ) );
                    if( 1 == sTmp.Len() )
                        sTmp.Insert( '0', 0 );
                    rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "\\x" ) );
                    rStr += sTmp;
                }
        }
    }
    else
        rStr += c;                  // normaler Text
}

// QuoteString() wandelt CRs abhaengig von nFieldIniFlags in '\n' oder "\0x0d"
void SwWW8ImplReader::QuoteString( String& rStr, const String& rOrg )
{
    BOOL bAllowCr = SwFltGetFlag( nFieldFlags, SwFltControlStack::ALLOW_FLD_CR );
    xub_StrLen nMax = rOrg.Len();
    xub_StrLen i = 0;
    while( nMax > i && ( rStr.Len() < MAX_FIELDLEN - 4 ) )
    {
        QuoteChar( rStr, rOrg.GetChar( i ), bAllowCr );
        i++;
    }
}

// MakeTagString() gibt als Returnwert die Position des ersten
// CR / Zeilenende / Seitenumbruch in pText und wandelt auch nur bis dort
// Wenn keins dieser Sonderzeichen enthalten ist, wird 0 zurueckgeliefert.
void SwWW8ImplReader::MakeTagString( String& rStr, const String& rOrg )
{
    BOOL bAllowCr = SwFltGetFlag( nFieldFlags, SwFltControlStack::TAGS_IN_TEXT )
                    || SwFltGetFlag( nFieldFlags, SwFltControlStack::ALLOW_FLD_CR );


    xub_StrLen nMax = rOrg.Len();
    xub_StrLen i = 0;
    while( nMax > i && ( rStr.Len() < MAX_FIELDLEN - 4 ) )
    {
        sal_Unicode c = rOrg.GetChar( i );
        switch( c )
        {
            case 132:                       // Typographische Anfuehrungszeichen
            case 148:                       // gegen normale tauschen
            case 147: rStr += '"'; break;
            case 19:  rStr += '{'; break;   // 19..21 zu {|}
            case 20:  rStr += '|'; break;
            case 21:  rStr += '}'; break;
            case '\\':                      // \{|} per \ Taggen
            case '{':
            case '|':
            case '}': rStr += '\\'; rStr += c; break;
            default:  SwWW8ImplReader::QuoteChar( rStr, c, bAllowCr ); break;
        }
        i++;
    }
}

void SwWW8ImplReader::InsertTagField( const USHORT nId, const String& rTagText )
{
    String aName( WW8_ASCII2STR( "WwFieldTag" ) );
    if( SwFltGetFlag( nFieldFlags, SwFltControlStack::TAGS_DO_ID ) ) // Nummer?
        aName += String::CreateFromInt32( nId );                    // ausgeben ?

    if( SwFltGetFlag(nFieldFlags, SwFltControlStack::TAGS_IN_TEXT))
    {
        aName += rTagText;                                          // als Txt taggen
        rDoc.Insert( *pPaM, aName, FALSE );
    }
    else
    {                                                   // normal tagggen

        SwFieldType* pFT = rDoc.InsertFldType(
                                SwSetExpFieldType( &rDoc, aName, GSE_STRING ) );
        SwSetExpField aFld( (SwSetExpFieldType*)pFT, rTagText );                            // SUB_INVISIBLE
        USHORT nSubType = ( SwFltGetFlag( nFieldFlags, SwFltControlStack::TAGS_VISIBLE ) ) ? 0 : SUB_INVISIBLE;
        aFld.SetSubType(nSubType);

        rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    }
}

long SwWW8ImplReader::Read_F_Tag( WW8FieldDesc* pF )
{
    long nOldPos = pStrm->Tell();

    WW8_CP nStart = pF->nSCode - 1;         // mit 0x19 am Anfang
    long nL = pF->nLen;                     // Gesamtlaenge mit Resultat u. Nest
    if( nL > MAX_FIELDLEN )
        nL = MAX_FIELDLEN;                  // MaxLaenge, durch Quoten
                                            // max. 4* so gross
    String sFTxt;
    nL = pSBase->WW8ReadString( *pStrm, sFTxt,
                                pPlcxMan->GetCpOfs() + nStart, nL, eStructCharSet);


    String aTagText;
    MakeTagString( aTagText, sFTxt );
    InsertTagField( pF->nId, aTagText );

    pStrm->Seek( nOldPos );
    return pF->nLen;
}


//-----------------------------------------
//        normale Felder
//-----------------------------------------

// Read_F_Nul() dient dazu, Felder, die ein belegtes ResultatFeld haben, dieses
// aber trotzdem keinen Text darstellt, sondern z.B. Variablen-Werte, komplett
// zu ignorieren.
// Noetig z.B. fuer Feld 6 "Set" == "Bestimmen".
eF_ResT SwWW8ImplReader::Read_F_Nul( WW8FieldDesc*, String& rStr )
{
    return F_OK;
}

eF_ResT SwWW8ImplReader::Read_F_Input( WW8FieldDesc* pF, String& rStr )
{
    String aDef;
    String aQ;
    long nRet;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            if( !aQ.Len() )
                aQ = aReadParam.GetResult();
            break;
        case 'd':
        case 'D':
            {
                xub_StrLen n = aReadParam.GoToTokenParam();
                if( STRING_NOTFOUND != n )
                    aDef = aReadParam.GetResult();
            }
            break;
        }
    }
    if( !aDef.Len() )
        aDef = GetFieldResult( pF );

    SwInputField aFld( (SwInputFieldType*)rDoc.GetSysFldType( RES_INPUTFLD ),
                        aDef, aQ, INP_TXT, 0 ); // sichtbar ( geht z.Zt. nicht anders )
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );

    return F_OK;
}

// GetFieldResult alloziert einen String und liest das Feld-Resultat ein
String SwWW8ImplReader::GetFieldResult( WW8FieldDesc* pF )
{
    long nOldPos = pStrm->Tell();

    WW8_CP nStart = pF->nSRes;              // Start Resultat
    long nL = pF->nLRes;                    // Laenge Resultat
    if( !nL )
        return aEmptyStr;                           // kein Resultat

    if( nL > MAX_FIELDLEN )
        nL = MAX_FIELDLEN;                  // MaxLaenge, durch Quoten
                                            // max. 4* so gross

    String sRes;
    nL = pSBase->WW8ReadString( *pStrm, sRes, pPlcxMan->GetCpOfs() + nStart,
                                nL, eStructCharSet );

    pStrm->Seek( nOldPos );
    return sRes;
}

// "FRAGE"
eF_ResT SwWW8ImplReader::Read_F_InputVar( WW8FieldDesc* pF, String& rStr )
{
    String aVar;
    String aQ;
    String aDef;
    long nRet;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            if( !aVar.Len() )
                aVar = aReadParam.GetResult();
            else if( !aQ.Len() )
                aQ = aReadParam.GetResult();
            break;
        case 'd':
        case 'D':
            {
                xub_StrLen n = aReadParam.GoToTokenParam();
                if( STRING_NOTFOUND != n )
                    aDef = aReadParam.GetResult();
            }
            break;
        }
    }
    if( !aVar.Len() )
        return F_TAGIGN;  // macht ohne Textmarke keinen Sinn
    if( !aDef.Len() )
        aDef = GetFieldResult( pF );



    SwFieldType* pFT = rDoc.InsertFldType( SwSetExpFieldType( &rDoc, aVar, GSE_STRING ) );
    SwSetExpField aFld( (SwSetExpFieldType*)pFT, aVar );
    aFld.SetSubType(SUB_INVISIBLE);
    aFld.SetInputFlag( TRUE );
    aFld.SetPromptText( aQ );
    aFld.SetPar2( aDef );
    pPlcxMan->GetBook()->SetStatus( pF->nSCode, pF->nSCode + pF->nLen,
                aVar, BOOK_ONLY_REF );
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );


    return F_OK;
}

// "AUTONR"
eF_ResT SwWW8ImplReader::Read_F_ANumber( WW8FieldDesc*, String& rStr )
{
    if( !pNumFldType ){     // 1. Mal
        SwSetExpFieldType aT( &rDoc, WW8_ASCII2STR("AutoNr"), GSE_SEQ );
        pNumFldType = rDoc.InsertFldType( aT );
    }
    SwSetExpField aFld( (SwSetExpFieldType*)pNumFldType, aEmptyStr,
                        GetNumberPara( rStr ) );
    aFld.SetValue( ++nFldNum );
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}

// "SEQ"
eF_ResT SwWW8ImplReader::Read_F_Seq( WW8FieldDesc*, String& rStr )
{
    String aSequenceName;
    String aBook;
    BOOL bHidden    = FALSE;
    BOOL bFormat    = FALSE;
    BOOL bShowLast  = FALSE;
    BOOL bCountOn   = TRUE;
    String sStart, sLevel;
    SvxExtNumType eNumFormat = SVX_NUM_ARABIC;
    long nRet;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            if( !aSequenceName.Len() )
                aSequenceName = aReadParam.GetResult();
            else if( !aBook.Len() )
                aBook = aReadParam.GetResult();
            break;

        case 'h':
            if( !bFormat )
                bHidden = TRUE;             // Hidden-Flag aktivieren
            break;

        case '*':
            bFormat = TRUE;                 // Format-Flag aktivieren
            bHidden = FALSE;                // Hidden-Flag deaktivieren
            nRet = aReadParam.SkipToNextToken();
            if( -2 == nRet )
                eNumFormat = GetNumTypeFromName( aReadParam.GetResult() );
            break;

        case 'r':
            bShowLast = FALSE;              // Zaehler neu setzen
            bCountOn  = FALSE;
            nRet = aReadParam.SkipToNextToken();
            if( -2 == nRet )
                sStart = aReadParam.GetResult();
            break;

        case 'c':
            bShowLast = TRUE;               // zuletzt verwendete Nummer anzeigen
            bCountOn  = FALSE;
            break;

        case 'n':
            bCountOn  = TRUE;               // Nummer um eins erhoehen (default)
            bShowLast = FALSE;
            break;

        case 's':                       // Outline Level
            if( -2 == aReadParam.SkipToNextToken() )
                sLevel = aReadParam.GetResult();
            break;
        }
    }
    if(    !aSequenceName.Len()
        && !aBook.Len() )
        return F_TAGIGN;

    SwSetExpFieldType* pFT = (SwSetExpFieldType*)rDoc.InsertFldType(
                        SwSetExpFieldType( &rDoc, aSequenceName, GSE_SEQ ) );
    SwSetExpField aFld( pFT, aEmptyStr, eNumFormat );

    if( sStart.Len() )
        aFld.SetFormula( ( aSequenceName += '=' ) += sStart );

    if( sLevel.Len() )
    {
        USHORT nLvl = (USHORT)sLevel.ToInt32();
        if( nLvl )
        {
            if( MAXLEVEL <= --nLvl )
                nLvl = MAXLEVEL - 1;
            pFT->SetOutlineLvl( nLvl );
        }
    }

    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}


eF_ResT SwWW8ImplReader::Read_F_DocInfo( WW8FieldDesc* pF, String& rStr )
{
    USHORT nSub;
    USHORT nReg = DI_SUB_AUTHOR;    // RegInfoFormat, DefaultFormat fuer DocInfoFelder
    BOOL   bDateTime = FALSE;

    if( 85 == pF->nId )
    {
        /*
        There are up to 26 fields that may be meant by 'DocumentProperty'.
        Which of them is to be inserted here ?
        This Problem can only be solved by implementing a name matching
        method that compares the given Parameter String with the four
        possible name sets (english, german, french, spanish)
        */
        String aStr( rStr );
        xub_StrLen nPos1 = aStr.Search( '"' );
        if( (STRING_NOTFOUND != nPos1) && (nPos1+1 < aStr.Len()) )
        {
            xub_StrLen nPos2 = aStr.SearchAndReplace( '"', '\0', nPos1+1 );
            if( STRING_NOTFOUND != nPos2 )
            {
                aStr.Erase(0, nPos1+1);
                static sal_Char __READONLY_DATA* aName10 = "\x0F"; // SW field code
                static sal_Char  __READONLY_DATA* aName11 // German
                    = "TITEL";
                static sal_Char  __READONLY_DATA* aName12 // French
                    = "TITRE";
                static sal_Char  __READONLY_DATA* aName13 // English
                    = "TITLE";
                static sal_Char  __READONLY_DATA* aName14 // Spanish
                    = "TITRO";
                static sal_Char  __READONLY_DATA* aName20 = "\x15"; // SW filed code
                static sal_Char  __READONLY_DATA* aName21 // German
                    = "ERSTELLDATUM";
                static sal_Char  __READONLY_DATA* aName22 // French
                    = "CRÉÉ";
                static sal_Char  __READONLY_DATA* aName23 // English
                    = "CREATED";
                static sal_Char  __READONLY_DATA* aName24 // Spanish
                    = "CREADO";
                static sal_Char  __READONLY_DATA* aName30 = "\x16"; // SW filed code
                static sal_Char  __READONLY_DATA* aName31 // German
                    = "ZULETZTGESPEICHERTZEIT";
                static sal_Char  __READONLY_DATA* aName32 // French
                    = "DERNIERENREGISTREMENT";
                static sal_Char  __READONLY_DATA* aName33 // English
                    = "SAVED";
                static sal_Char  __READONLY_DATA* aName34 // Spanish
                    = "MODIFICADO";
                static sal_Char  __READONLY_DATA* aName40 = "\x17"; // SW filed code
                static sal_Char  __READONLY_DATA* aName41 // German
                    = "ZULETZTGEDRUCKT";
                static sal_Char  __READONLY_DATA* aName42 // French
                    = "DERNIÈREIMPRESSION";
                static sal_Char  __READONLY_DATA* aName43 // English
                    = "LASTPRINTED";
                static sal_Char  __READONLY_DATA* aName44 // Spanish
                    = "HUPS PUPS";
                static sal_Char  __READONLY_DATA* aName50 = "\x18"; // SW filed code
                static sal_Char  __READONLY_DATA* aName51 // German
                    = "ÜBERARBEITUNGSNUMMER";
                static sal_Char  __READONLY_DATA* aName52 // French
                    = "NUMÉRODEREVISION";
                static sal_Char  __READONLY_DATA* aName53 // English
                    = "REVISIONNUMBER";
                static sal_Char  __READONLY_DATA* aName54 // Spanish
                    = "SNUBBEL BUBBEL";
                static const USHORT nFldCnt  = 5;




                // additional fields are to be coded soon!   :-)




                static const USHORT nLangCnt = 4;
                static const sal_Char * aNameSet_26[nFldCnt][nLangCnt+1] = {
                    {aName10, aName11, aName12, aName13, aName14},
                    {aName20, aName21, aName22, aName23, aName24},
                    {aName30, aName31, aName32, aName33, aName34},
                    {aName40, aName41, aName42, aName43, aName44},
                    {aName50, aName51, aName52, aName53, aName54}
                };
                BOOL   bFldFound= FALSE;
                USHORT nFIdx;
                for(USHORT nLIdx=1; !bFldFound && (nLangCnt > nLIdx); ++nLIdx)
                {
                    for(nFIdx = 0;  !bFldFound && (nFldCnt  > nFIdx); ++nFIdx)
                    {
                        if( aStr.Equals( String( aNameSet_26[nFIdx][nLIdx], RTL_TEXTENCODING_MS_1252 ) ) )
                        {
                            bFldFound = TRUE;
                            pF->nId   = aNameSet_26[nFIdx][0][0];
                        }
                    }
                }
                if( !bFldFound )
                    return F_TAGTXT; // Error: show field as string
            }
        }
    }

    switch( pF->nId )
    {
        case 14: nSub = DI_KEYS;        /* kann alle INFO-Vars!! */      break;
        case 15: nSub = DI_TITEL;                                        break;
        case 16: nSub = DI_THEMA;                                        break;
        case 18: nSub = DI_KEYS;                                         break;
        case 19: nSub = DI_COMMENT;                                      break;
        case 20: nSub = DI_CHANGE; nReg = DI_SUB_AUTHOR;                 break;
        case 21: nSub = DI_CREATE; nReg = DI_SUB_DATE; bDateTime = TRUE; break;
        case 23: nSub = DI_PRINT;  nReg = DI_SUB_DATE; bDateTime = TRUE; break;
        case 24: nSub = DI_DOCNO;                                        break;
        case 22: nSub = DI_CHANGE; nReg = DI_SUB_DATE; bDateTime = TRUE; break;
        case 25: nSub = DI_CHANGE; nReg = DI_SUB_TIME; bDateTime = TRUE; break;
    }

    ULONG nFormat = 0;

    if( bDateTime )
    {
        short nDT = GetTimeDatePara( rDoc, rStr, nFormat );
        switch( nDT )
        {
        case NUMBERFORMAT_DATE:     nReg = DI_SUB_DATE; break;
        case NUMBERFORMAT_TIME:     nReg = DI_SUB_TIME; break;
        case NUMBERFORMAT_DATETIME: nReg = DI_SUB_DATE; break;
        default:
            {
                nReg = DI_SUB_DATE;
            }
        }
    }

    SwDocInfoField aFld( (SwDocInfoFieldType*)
                    rDoc.GetSysFldType( RES_DOCINFOFLD ), nSub|nReg, nFormat );
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}

eF_ResT SwWW8ImplReader::Read_F_Author( WW8FieldDesc*, String& )
{
        // SH: Das SwAuthorField bezeichnet nicht den urspruenglichen
        // Autor, sondern den aktuellen Benutzer, also besser ueber DocInfo
        // (#56149)
    SwDocInfoField aFld( (SwDocInfoFieldType*)
                     rDoc.GetSysFldType( RES_DOCINFOFLD ),
                     DI_CREATE|DI_SUB_AUTHOR );
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}

eF_ResT SwWW8ImplReader::Read_F_TemplName( WW8FieldDesc*, String& )
{
    SwTemplNameField aFld( (SwTemplNameFieldType*)
                     rDoc.GetSysFldType( RES_TEMPLNAMEFLD ), FF_NAME );
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}

// Sowohl das Datum- wie auch das Uhrzeit-Feld kann fuer Datum, fuer Uhrzeit
// oder fuer beides benutzt werden.
eF_ResT SwWW8ImplReader::Read_F_DateTime( WW8FieldDesc*pF, String& rStr )
{                                               // Datum/Uhrzeit - Feld
    ULONG nFormat = 0;

    short nDT = GetTimeDatePara( rDoc, rStr, nFormat );

    if( NUMBERFORMAT_UNDEFINED == nDT )             // no D/T-Formatstring
    {
        if( 32 == pF->nId )
        {
            nDT     = NUMBERFORMAT_TIME;
            nFormat = rDoc.GetNumberFormatter()->GetFormatIndex(
                        NF_TIME_START, LANGUAGE_SYSTEM );
        }
        else
        {
            nDT     = NUMBERFORMAT_DATE;
            nFormat = rDoc.GetNumberFormatter()->GetFormatIndex(
                        NF_DATE_START, LANGUAGE_SYSTEM );
        }
    }

    if( nDT & NUMBERFORMAT_DATE )
    {
        SwDateTimeField aFld( (SwDateTimeFieldType*)
                        rDoc.GetSysFldType( RES_DATETIMEFLD ), DATEFLD, nFormat );
        rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    }
    else if( nDT == NUMBERFORMAT_TIME )
    {
        SwDateTimeField aFld( (SwDateTimeFieldType*)
                        rDoc.GetSysFldType( RES_DATETIMEFLD ), TIMEFLD, nFormat );
        rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    }

    return F_OK;
}

eF_ResT SwWW8ImplReader::Read_F_FileName( WW8FieldDesc*, String& )
{
    SwFileNameField aFld( (SwFileNameFieldType*)
                          rDoc.GetSysFldType( RES_FILENAMEFLD ) );
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}

eF_ResT SwWW8ImplReader::Read_F_Anz( WW8FieldDesc* pF, String& rStr )
{                                               // SeitenZahl - Feld
    USHORT nSub = DS_PAGE;
    switch ( pF->nId ){
    case 27: nSub = DS_WORD; break;             // Wordzahl
    case 28: nSub = DS_CHAR; break;             // Zeichenzahl
    }
    SwDocStatField aFld( (SwDocStatFieldType*)
                         rDoc.GetSysFldType( RES_DOCSTATFLD ), nSub,
                         GetNumberPara( rStr ) );
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}

eF_ResT SwWW8ImplReader::Read_F_CurPage( WW8FieldDesc*, String& rStr )
{
    // zusaetzlich mit Kapitelnummer?
    if( bPgChpLevel )
    {
        SwChapterField aFld( (SwChapterFieldType*)
                    rDoc.GetSysFldType( RES_CHAPTERFLD ), CF_NUMBER );
        aFld.SetLevel( nPgChpLevel );
        rDoc.Insert( *pPaM, SwFmtFld( aFld ) );

        static sal_Char __READONLY_DATA aDelim[] = "-.:\x97\x96";
        BYTE nDelim = nPgChpDelim;
        if( nDelim > 4 )
            nDelim = 0;

        sal_Unicode c = ByteString::ConvertToUnicode( aDelim[ nDelim ],
                                        RTL_TEXTENCODING_MS_1252 );
        if( '-' == c )
            rDoc.Insert( *pPaM, CHAR_HARDHYPHEN );
        else
            rDoc.Insert( *pPaM, SwFmtHardBlank( c ));
    }
                                               // Seitennummer
    SwPageNumberField aFld( (SwPageNumberFieldType*)
                    rDoc.GetSysFldType( RES_PAGENUMBERFLD ), PG_RANDOM,
                    GetNumberPara( rStr, TRUE ) );
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}

eF_ResT SwWW8ImplReader::Read_F_Symbol( WW8FieldDesc*, String& rStr )
{                                               // Symbol-Zeichensatz
/*
    char* p;
    char* pQ = Find1stPara( pStr, &p );         // 1. Para ist Ascii-Wert
    USHORT nC = String( pQ );
    if( !nC )                                   // ungueltige Syntax
        return F_TAGIGN;                        // -> kein 0-Zeichen in Text
    char* pCSet = FindPara( p, 'f', 'F' );          // \f -> Zeichensatz
    char* pSiz = FindPara( p, 's', 'S' );           // \s -> FontSize
    if( pCSet ){                                // Charset angegeben ?
        String aName( pCSet );
        if( eStructCharSet != eSysCharSet )             // OS2, WIN mit Mac-Doc,...
            aName.Convert( eStructCharSet, eSysCharSet );
        SvxFontItem aFont(  FAMILY_DONTKNOW, aName, // "WingDings",
                            aEmptyStr,
                            PITCH_DONTKNOW, CHARSET_SYMBOL);
        NewAttr( aFont );              // neuer Font
    }
    if( pSiz ){                                 // Size angegeben ?
        ULONG nSiz = String( pSiz );            // in pt
        SvxFontHeightItem aSz( nSiz * 20 );     // pT -> twip
        NewAttr( aSz );
    }
*/
    String aQ;
    String aName;
    String aSiz;
    long nRet;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            if( !aQ.Len() )
                aQ = aReadParam.GetResult();
            break;
        case 'f':
        case 'F':
            {
                xub_StrLen n = aReadParam.GoToTokenParam();
                if( STRING_NOTFOUND != n )
                    aName = aReadParam.GetResult();
            }
            break;
        case 's':
        case 'S':
            {
                xub_StrLen n = aReadParam.GoToTokenParam();
                if( STRING_NOTFOUND != n )
                    aSiz = aReadParam.GetResult();
            }
            break;
        }
    }
    if( !aQ.Len() )
        return F_TAGIGN;                        // -> kein 0-Zeichen in Text

    if( aName.Len() )                           // Font Name angegeben ?
    {
        SvxFontItem aFont(  FAMILY_DONTKNOW, aName, // "WingDings",
                            aEmptyStr,
                            PITCH_DONTKNOW, RTL_TEXTENCODING_SYMBOL );
        NewAttr( aFont );                       // neuer Font
    }

    if( aSiz.Len() )                            // Size angegeben ?
    {
        SvxFontHeightItem aSz( aSiz.ToInt32() * 20 ); // pT -> twip
        NewAttr( aSz );
    }

    rDoc.Insert( *pPaM, aQ.GetChar( 0 ) );

    if( aSiz.Len() )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_FONTSIZE );
    if( aName.Len() )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_FONT );

    return F_OK;
}


// "EINBETTEN"
eF_ResT SwWW8ImplReader::Read_F_Embedd( WW8FieldDesc*, String& rStr )
{
    String sHost;

    long nRet;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            sHost = aReadParam.GetResult();
            break;

        case 's':
            // use ObjectSize
            break;
        }
    }

    if( bObj && nPicLocFc )
        nObjLocFc = nPicLocFc;
    bEmbeddObj = TRUE;
    return F_TEXT;
}


// "BESTIMMEN"
eF_ResT SwWW8ImplReader::Read_F_Set( WW8FieldDesc* pF, String& rStr )
{
    String aName;
    String aVal;
    long nRet;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            if( !aName.Len() )
                aName = aReadParam.GetResult();
            else if( !aVal.Len() )
                aVal = aReadParam.GetResult();
            break;
        }
    }
    SwFieldType* pFT = rDoc.InsertFldType(
                            SwSetExpFieldType( &rDoc, aName, GSE_STRING ) );
    SwSetExpField aFld( (SwSetExpFieldType*)pFT, aVal );
    aFld.SetSubType(SUB_INVISIBLE);
    pPlcxMan->GetBook()->SetStatus( pF->nSCode, pF->nSCode + pF->nLen,
         aName, BOOK_IGNORE );
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}

// "REF"
eF_ResT SwWW8ImplReader::Read_F_Ref( WW8FieldDesc*, String& rStr )
{                                                       // Reference - Field
    String aBkmName;
    BOOL bChapterNr = FALSE;
    BOOL bObenUnten = FALSE;

    long nRet;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            if( !aBkmName.Len() ) // get name of bookmark
            {
                aBkmName = aReadParam.GetResult();
            }
            break;

        case 'r':
            bChapterNr = TRUE; // activate flag 'Chapter Number'
            break;

        case 'p':
            bObenUnten = TRUE;
            break;
        case 'h':
            break;
        default:
            // unimplemented switch: just do 'nix nought nothing'  :-)
            break;
        }
    }

    if ( SwFltGetFlag( nFieldFlags, SwFltControlStack::HYPO ) )
    {
        SwGetExpField aFld( (SwGetExpFieldType*)
                        rDoc.GetSysFldType( RES_GETEXPFLD ),
                        aBkmName, GSE_STRING, VVF_SYS );
        rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
        pEndStck->SetBookRef( aBkmName, FALSE );
    }
    else
    {
        SwGetRefField aFld( (SwGetRefFieldType*)
                    rDoc.GetSysFldType( RES_GETREFFLD ),
                    aBkmName,
                    REF_BOOKMARK,
                    0,
                    REF_CONTENT );
        rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
        if( bObenUnten )
        {
            SwGetRefField aFld2( (SwGetRefFieldType*)
                        rDoc.GetSysFldType( RES_GETREFFLD ),
                        aBkmName,
                        REF_BOOKMARK,
                        0,
                        REF_UPDOWN );
            rDoc.Insert( *pPaM, SwFmtFld( aFld2 ) );
        }
    }
    return F_OK;
}

eF_ResT SwWW8ImplReader::Read_F_NoteReference( WW8FieldDesc*, String& rStr )
{                                                   // Note Reference - Field
    String aBkmName;
    BOOL bChapterNr = FALSE;
    BOOL bObenUnten = FALSE;

    long nRet;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            if( !aBkmName.Len() ) // get name of foot/endnote
            {
                aBkmName = aReadParam.GetResult();
            }
            break;

        case 'r':
            bChapterNr = TRUE; // activate flag 'Chapter Number'
            break;

        case 'p':
            bObenUnten = TRUE;
            break;
        case 'h':
            break;
        default:
            // unimplemented switch: just do 'nix nought nothing'  :-)
            break;
        }
    }

    if ( SwFltGetFlag( nFieldFlags, SwFltControlStack::HYPO ) )
    {
        SwGetExpField aFld( (SwGetExpFieldType*)
                        rDoc.GetSysFldType( RES_GETEXPFLD ),
                        aBkmName, GSE_STRING, VVF_SYS );
        rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
        pEndStck->SetBookRef( aBkmName, FALSE );
    }
    else
    {   // set Sequence No of corresponding Foot-/Endnote to Zero
        // (will be corrected in
        SwGetRefField aFld( (SwGetRefFieldType*)
                    rDoc.GetSysFldType( RES_GETREFFLD ),
                    aBkmName,
                    REF_FOOTNOTE,
                    0,
                    REF_ONLYNUMBER
                    );
        pRefFldStck->NewAttr(*pPaM->GetPoint(), SwFmtFld( aFld ));
        if( bObenUnten )
        {
//          rDoc.Insert( *pPaM, SwFmtHardBlank(' ') );
            SwGetRefField aFld2( (SwGetRefFieldType*)
                        rDoc.GetSysFldType( RES_GETREFFLD ),
                        aBkmName,
                        REF_FOOTNOTE,
                        0,
                        REF_UPDOWN
                        );
            pRefFldStck->NewAttr(*pPaM->GetPoint(), SwFmtFld( aFld2 ));
        }
    }
    return F_OK;
}

// "SEITENREF"
eF_ResT SwWW8ImplReader::Read_F_PgRef( WW8FieldDesc*, String& rStr )
{
    String aName;
    long nRet;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            if( !aName.Len() )
                aName = aReadParam.GetResult();
            break;
        }
    }
    if ( SwFltGetFlag( nFieldFlags, SwFltControlStack::HYPO ) )
    {
        SwGetRefField aFld( (SwGetRefFieldType*)
                        rDoc.GetSysFldType( RES_GETREFFLD ), aName, 0, 0,
                        REF_PAGE );
        rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
        pEndStck->SetBookRef( aName, TRUE );
    }
    else
    {
        SwGetRefField aFld( (SwGetRefFieldType*)
                    rDoc.GetSysFldType( RES_GETREFFLD ),
                    aName,
                    REF_BOOKMARK,
                    0,
                    REF_PAGE );
        rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    }
    return F_OK;
}

// "MACROSCHALTFL"ACHE"
eF_ResT SwWW8ImplReader::Read_F_Macro( WW8FieldDesc*, String& rStr )
{
    String aName;
    String aVText;
    long nRet;
    BOOL bNewVText = TRUE;
    BOOL bBracket  = FALSE;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            if( !aName.Len() )
                aName = aReadParam.GetResult();
            else if( !aVText.Len() || bBracket )
            {
                if( bBracket )
                    aVText += ' ';
                aVText += aReadParam.GetResult();
                if( bNewVText )
                {
                    bBracket  = aVText.EqualsIgnoreCaseAscii('[', 1, 0);
                    bNewVText = FALSE;
                }
                else if( aVText.GetChar( aVText.Len()-1 ) == ']' )
                    bBracket  = FALSE;
            }
            break;
        }
    }
    if( !aName.Len() )
        return F_TAGIGN;  // makes no sense without Makro-Name

    aName.InsertAscii( "StarOffice.Standard.Modul1.", 0 );

    SwMacroField aFld( (SwMacroFieldType*)
                    rDoc.GetSysFldType( RES_MACROFLD ), aName, aVText );
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}

// "EINF"UGENGRAFIK"
eF_ResT SwWW8ImplReader::Read_F_IncludePicture( WW8FieldDesc*, String& rStr )
{
    String aGrfName;
    BOOL bEmbedded = TRUE;

    long nRet;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            if( !aGrfName.Len() )
            {
                ConvertFFileName( aGrfName, aReadParam.GetResult() );
                aGrfName = INetURLObject::RelToAbs( aGrfName );
            }
            break;

        case 'd':
            bEmbedded = FALSE;          // Embedded-Flag deaktivieren
            break;

        case 'c':// den Converter-Namen ueberlesen
            aReadParam.FindNextStringPiece();
            break;
        }
    }

    BOOL bExist = FALSE;
    INetURLObject aGrURL(URIHelper::SmartRelToAbs(aGrfName));
/*  try
    {
        ::ucb::Content aTestContent(
            aGrURL.GetMainURL(),
            uno::Reference< XCommandEnvironment >());
        bExist = aTestContent.isDocument();
    }
    catch(...){}
*/
    if( bExist || !bEmbedded )
    {
        /*
            Besonderheit:

            Wir setzen jetzt den Link ins Doc und merken uns den SwFlyFrmFmt.
            Da wir ja unten auf jjeden Fall mit Return-Wert F_READ_FSPA enden,
            wird der Skip-Wert so bemessen, dass das folgende Char-1 eingelesen
            wird.
            Wenn wir dann in SwWW8ImplReader::ImportGraf() reinlaufen, wird
            erkannt, dass wir soeben einen Grafik-Link inserted haben und
            das passende SwAttrSet wird ins Frame-Format eingesetzt.
        */
        SfxItemSet aFlySet( rDoc.GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
        aFlySet.Put( SwFmtAnchor( FLY_IN_CNTNT ) );
        aFlySet.Put( SwFmtVertOrient( 0, VERT_TOP, FRAME ));
        pFlyFmtOfJustInsertedGraphic = rDoc.Insert( *pPaM,
                                                    aGrfName,
                                                    aEmptyStr,
                                                    0,          // Graphic*
                                                    &aFlySet,
                                                    0);         // SwFrmFmt*
        String aName;
        if(MakeUniqueGraphName(aName, aGrURL.GetBase()))
            pFlyFmtOfJustInsertedGraphic->SetName( aName );
    }
    return F_READ_FSPA;
}

// "EINFUEGENTEXT"
eF_ResT SwWW8ImplReader::Read_F_IncludeText( WW8FieldDesc*, String& rStr )
{
    String aPara;
    String aBook;
    long nRet;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            if( !aPara.Len() )
                aPara = aReadParam.GetResult();
            else if( !aBook.Len() )
                aBook = aReadParam.GetResult();
            break;
        case '*':
            //Skip over MERGEFORMAT
            nRet = aReadParam.SkipToNextToken();
            break;
        }
    }
    ConvertFFileName( aPara, aPara );
    //aPara = INetURLObject::RelToAbs( aPara );
    aPara = URIHelper::SmartRelToAbs( aPara );

    if( aBook.Len() && aBook.GetChar( 0 ) != '\\' )
    {
        // Bereich aus Quelle ( kein Switch ) ?
        ConvertUFName( aBook );
        aPara += cTokenSeperator;
        aPara += cTokenSeperator;
        aPara += aBook;
    }
    String aStr(WW8_ASCII2STR( "WW" ));
    SwSection* pSection = new SwSection( FILE_LINK_SECTION,
                                    rDoc.GetUniqueSectionName( &aStr ) );
    pSection->SetLinkFileName( aPara );
    pSection->SetProtect( TRUE );
    NewAttr( SwFltSection( pSection ) );
    pEndStck->SetAttr( *pPaM->GetPoint(), RES_FLTR_SECTION );

//  rDoc.AppendTxtNode( *pPaM->GetPoint() );
    return F_OK;
}

// "SERIENDRUCKFELD"
eF_ResT SwWW8ImplReader::Read_F_DBField( WW8FieldDesc*, String& rStr )
{
    String aName;
    long nRet;
    _ReadFieldParams aReadParam( rStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            if( !aName.Len() )
                aName = aReadParam.GetResult();
            break;
        }
    }
    SwDBFieldType aD( &rDoc, aName, SwDBData() );   // Datenbank: Nichts
    SwFieldType* pFT = rDoc.InsertFldType( aD );
    SwDBField aFld( (SwDBFieldType*)pFT );
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}

// "N"ACHSTER"
eF_ResT SwWW8ImplReader::Read_F_DBNext( WW8FieldDesc*, String& )
{
    SwDBNextSetFieldType aN;
    SwFieldType* pFT = rDoc.InsertFldType( aN );
    SwDBNextSetField aFld( (SwDBNextSetFieldType*)pFT, aEmptyStr, aEmptyStr,
                            SwDBData() );       // Datenbank: Nichts
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}

// "DATENSATZ"
eF_ResT SwWW8ImplReader::Read_F_DBNum( WW8FieldDesc*, String& )
{
    SwDBSetNumberFieldType aN;
    SwFieldType* pFT = rDoc.InsertFldType( aN );
    SwDBSetNumberField aFld( (SwDBSetNumberFieldType*)pFT,
                           SwDBData() );            // Datenbank: Nichts
    rDoc.Insert( *pPaM, SwFmtFld( aFld ) );
    return F_OK;
}

/*
    EQ , only the usage for
    a. Combined Characters supported, must be exactly in the form that word
    only accepts as combined charactersm, i.e.
    eq \o(\s\up Y(XXX),\s\do Y(XXX))
    b. Ruby Text supported, must be in the form that word recognizes as being
    ruby text
    ...
*/
eF_ResT SwWW8ImplReader::Read_F_Equation( WW8FieldDesc* pF, String& rStr )
{
    _ReadFieldParams aReadParam( rStr );
    long cChar = aReadParam.SkipToNextToken();
    if ('o' == cChar)
        Read_SubF_Combined(aReadParam);
    else if ('*' == cChar)
        Read_SubF_Ruby(aReadParam);
    return F_OK;
}

void SwWW8ImplReader::Read_SubF_Combined( _ReadFieldParams& rReadParam)
{
    String sCombinedCharacters;
    if ((-2 == rReadParam.SkipToNextToken()) &&
            rReadParam.GetResult().EqualsIgnoreCaseAscii('(', 1, 0))
    {
        for (int i=0;i<2;i++)
        {
            if ('s' == rReadParam.SkipToNextToken())
            {
                long cChar = rReadParam.SkipToNextToken();
                if (-2 != rReadParam.SkipToNextToken())
                    break;
                String sF = rReadParam.GetResult();
                if ((('u' == cChar) && sF.EqualsIgnoreCaseAscii('p', 1, 0))
                || (('d' == cChar) && sF.EqualsIgnoreCaseAscii('o', 1, 0)))
                {
                    if (-2 == rReadParam.SkipToNextToken())
                    {
                        String sPart = rReadParam.GetResult();
                        xub_StrLen nBegin = sPart.Search('(');

                        //Word disallows brackets in this field, which
                        //aids figuring out the case of an end of )) vs )
                        xub_StrLen nEnd = sPart.Search(')');

                        if ((nBegin != STRING_NOTFOUND) &&
                            (nEnd != STRING_NOTFOUND))
                        {
                            sCombinedCharacters +=
                                sPart.Copy(nBegin+1,nEnd-nBegin-1);
                        }
                    }
                }
            }
        }
    }
    if (sCombinedCharacters.Len())
    {
        SwCombinedCharField aFld((SwCombinedCharFieldType*)
            rDoc.GetSysFldType(RES_COMBINED_CHARS),sCombinedCharacters);
        rDoc.Insert(*pPaM,SwFmtFld(aFld));
    }
}

void SwWW8ImplReader::Read_SubF_Ruby( _ReadFieldParams& rReadParam)
{
    USHORT nJustificationCode=0;
    String sFontName;
    UINT32 nFontSize=0;
    String sRuby;
    String sText;
    long nRet;
    while( -1 != ( nRet = rReadParam.SkipToNextToken() ))
    {
        switch( nRet )
        {
        case -2:
            {
                String sTemp = rReadParam.GetResult();
                if( sTemp.EqualsIgnoreCaseAscii( "jc", 0, 2 ) )
                {
                    sTemp.Erase(0,2);
                    nJustificationCode = static_cast<USHORT>(sTemp.ToInt32());
                }
                else if( sTemp.EqualsIgnoreCaseAscii( "hps", 0, 3 ) )
                {
                    sTemp.Erase(0,3);
                    nFontSize= static_cast<UINT32>(sTemp.ToInt32());
                }
                else if( sTemp.EqualsIgnoreCaseAscii( "Font:", 0, 5 ) )
                {
                    sTemp.Erase(0,5);
                    sFontName = sTemp;
                }
            }
            break;
        case '*':
            break;
        case 'o':
            while( -1 != ( nRet = rReadParam.SkipToNextToken() ))
            {
                if ('u' == nRet)
                {
                    if (-2 == rReadParam.SkipToNextToken() &&
                      (rReadParam.GetResult().EqualsIgnoreCaseAscii('p', 1, 0)))
                    {
                        if (-2 == rReadParam.SkipToNextToken())
                        {
                            String sPart = rReadParam.GetResult();
                            xub_StrLen nBegin = sPart.Search('(');

                            //Word disallows brackets in this field,
                            xub_StrLen nEnd = sPart.Search(')');

                            if ((nBegin != STRING_NOTFOUND) &&
                                (nEnd != STRING_NOTFOUND))
                            {
                                sRuby = sPart.Copy(nBegin+1,nEnd-nBegin-1);
                            }
                            nBegin = sPart.Search(',',nEnd);
                            nEnd = sPart.SearchBackward(')');
                            if ((nBegin != STRING_NOTFOUND) &&
                                (nEnd != STRING_NOTFOUND))
                            {
                                sText = sPart.Copy(nBegin+1,nEnd-nBegin-1);
                            }
                        }
                    }
                }

            }
            break;
        }
    }

    //Translate and apply
    if (sRuby.Len() && sText.Len() && sFontName.Len() && nFontSize)
    {
        switch (nJustificationCode)
        {
        case 0:
            nJustificationCode=1;
            break;
        case 1:
            nJustificationCode=3;
            break;
        case 2:
            nJustificationCode=4;
            break;
        default:
        case 3:
            nJustificationCode=0;
            break;
        case 4:
            nJustificationCode=2;
            break;
        }

        SwFmtRuby aRuby(sRuby);
        SwCharFmt *pCharFmt=0;
        //Make a guess at which of asian of western we should be setting
        USHORT nScript;
        if (pBreakIt->xBreak.is())
            nScript = pBreakIt->xBreak->getScriptType(sRuby, 0);
        else
            nScript = com::sun::star::i18n::ScriptType::ASIAN;

        //Check to see if we already have a ruby charstyle that this fits
        for(USHORT i=0;i<aRubyCharFmts.Count();i++)
        {
            SwCharFmt *pFmt = aRubyCharFmts[i];
            const SvxFontHeightItem &rF =
                (const SvxFontHeightItem &)(pFmt->GetAttr(
                GetWhichOfScript(RES_CHRATR_FONTSIZE,nScript)));
            if (rF.GetHeight() == nFontSize*10)
            {
                const SvxFontItem &rF =
                    (const SvxFontItem &)(pFmt->GetAttr(
                    GetWhichOfScript(RES_CHRATR_FONT,nScript)));
                if (rF.GetFamilyName().Equals(sFontName))
                {
                    pCharFmt=pFmt;
                    break;
                }
            }
        }

        //Create a new char style if necessary
        if (!pCharFmt)
        {
            String aNm;
            //Take this as the base name
            rDoc.GetPoolNm(RES_POOLCHR_RUBYTEXT,aNm);
            aNm+=String::CreateFromInt32(aRubyCharFmts.Count()+1);
            pCharFmt = rDoc.MakeCharFmt(aNm,( SwCharFmt*)rDoc.GetDfltCharFmt());
            SvxFontHeightItem aHeightItem(nFontSize*10);
            SvxFontItem aFontItem(FAMILY_DONTKNOW,sFontName,
                aEmptyStr,PITCH_DONTKNOW,RTL_TEXTENCODING_DONTKNOW);
            aHeightItem.SetWhich(GetWhichOfScript(RES_CHRATR_FONTSIZE,nScript));
            aFontItem.SetWhich(GetWhichOfScript(RES_CHRATR_FONT,nScript));
            pCharFmt->SetAttr(aHeightItem);
            pCharFmt->SetAttr(aFontItem);
            aRubyCharFmts.Insert(pCharFmt,aRubyCharFmts.Count());
        }

        //Set the charstyle and justification
        aRuby.SetCharFmtName(pCharFmt->GetName());
        aRuby.SetCharFmtId(pCharFmt->GetPoolFmtId());
        aRuby.SetAdjustment(nJustificationCode);

        NewAttr(aRuby);
        rDoc.Insert( *pPaM, sText );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_TXTATR_CJK_RUBY );
        }
}

//-----------------------------------------
//        Verzeichnis-Felder
//-----------------------------------------

void lcl_toxMatchACSwitch(  SwWW8ImplReader& rReader,
                            SwDoc& rDoc,
                            SwTOXBase& rBase,
                            _ReadFieldParams& rParam,
                            SwCaptionDisplay eCaptionType)
{
    xub_StrLen n = rParam.GoToTokenParam();
    if( STRING_NOTFOUND != n )
    {
        SwTOXType* pType = (SwTOXType*)rDoc.GetTOXType( TOX_ILLUSTRATIONS, 0);
        pType->Add( &rBase );
        rBase.SetCaptionDisplay( eCaptionType );
        // Read Sequence Name and store in TOXBase
        String sSeqName( rParam.GetResult() );
        lcl_ConvertSequenceName( rReader, sSeqName );
        rBase.SetSequenceName( sSeqName );
    }
}


void lcl_toxMatchTSwitch(SwWW8ImplReader& rReader,
                         SwTOXBase& rBase,
                         _ReadFieldParams& rParam)
{
    xub_StrLen n = rParam.GoToTokenParam();
    if( STRING_NOTFOUND != n )
    {
        String sParams( rParam.GetResult() );
        if( sParams.Len() )
        {
            xub_StrLen nIndex = 0;

            String sTemplate( sParams.GetToken(0, ';', nIndex) );
            if( STRING_NOTFOUND == nIndex )
            {
                const SwFmt* pStyle
                        = rReader.GetStyleWithOrgWWName( sTemplate );
                if( pStyle )
                {
                    sTemplate = pStyle->GetName();
                }
                // Store Style for Level 0 into TOXBase
                rBase.SetStyleNames( sTemplate, 0 );
            }
            else while( STRING_NOTFOUND != nIndex )
            {
                sal_Int32 nLevel = sParams.GetToken(0, ';', nIndex).ToInt32();

                if( (0 < nLevel) && (MAXLEVEL >= nLevel) )
                {
                    nLevel--;
                    // Store Style and Level into TOXBase
                    const SwFmt* pStyle
                            = rReader.GetStyleWithOrgWWName( sTemplate );
                    if( pStyle )
                    {
                        sTemplate = pStyle->GetName();
                    }
                    String sStyles( rBase.GetStyleNames( nLevel ) );
                    if( sStyles.Len() )
                        sStyles += TOX_STYLE_DELIMITER;
                    sStyles += sTemplate;
                    rBase.SetStyleNames( sStyles, nLevel );
                }
                // read next style name...
                sTemplate = sParams.GetToken(0, ';', nIndex);
            }
        }
    }
}


eF_ResT SwWW8ImplReader::Read_F_Tox( WW8FieldDesc* pF, String& rStr )
{
    if( nIniFlags & WW8FL_NO_TOX )
        return F_OK;            // abgeschaltet -> ignorieren
    if( ( pF->nLRes < 3 ) )
        return F_TAGIGN;        // Nur Stuss -> ignorieren

    TOXTypes eTox;                              // Baue ToxBase zusammen
    switch( pF->nId )
    {
        case  8: eTox = TOX_INDEX; break;
        case 13: eTox = TOX_CONTENT; break;
        default: eTox = TOX_USER; break;
    }
    USHORT nCreateOf = (eTox == TOX_CONTENT) ? TOX_OUTLINELEVEL
                                             : TOX_MARK;
    USHORT nIndexCols = 0;
    const SwTOXType* pType = rDoc.GetTOXType( eTox, 0 );
    SwForm aForm( eTox );
    SwTOXBase* pBase = new SwTOXBase( pType, aForm, nCreateOf, aEmptyStr );
                                // Name des Verzeichnisses
    switch( eTox ){
    case TOX_INDEX:
        {
            USHORT eOptions = TOI_SAME_ENTRY | TOI_CASE_SENSITIVE;


            // TOX_OUTLINELEVEL setzen wir genau dann, wenn
            // die Parameter \o in 1 bis 9 liegen
            // oder der Parameter \f existiert
            // oder GARKEINE Switches Parameter angegeben sind.
            USHORT eCreateFrom = 0;
            USHORT nMaxLevel = 0;
            long nRet;
            _ReadFieldParams aReadParam( rStr );
            while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
            {
                switch( nRet )
                {
                case 'c':
                    {
                        xub_StrLen n = aReadParam.GoToTokenParam();
                        if( STRING_NOTFOUND != n )
                        {
                            String sParams( aReadParam.GetResult() );
                            if( sParams.Len() ) // if NO String just ignore the \c
                                nIndexCols = sParams.ToInt32();
                        }
                    }
                    break;
                case 'e':
                    {
                        xub_StrLen n = aReadParam.GoToTokenParam();
                        if( STRING_NOTFOUND != n )  // if NO String just ignore the \e
                        {
                            String sDelimiter( aReadParam.GetResult() );
                            SwForm aForm( pBase->GetTOXForm() );

                            // Attention: if TOX_CONTENT brave
                            //            GetFormMax() returns MAXLEVEL + 1  !!
                            USHORT nEnd = aForm.GetFormMax()-1;

                            for(USHORT nLevel = 1;
                                   nLevel <= nEnd;
                                   ++nLevel)
                            {
                                // Levels count from 1
                                // Level 0 is reserved for CAPTION

                                // Delimiter statt Tabstop vor der Seitenzahl einsetzen,
                                // falls es eine Seitenzahl gibt:
                                FormTokenType ePrevType = TOKEN_END;
                                FormTokenType eType;
                                SwFormTokenEnumerator aEnumer =
                                    aForm.CreateTokenEnumerator( nLevel );
                                do
                                {
                                    eType = aEnumer.GetNextTokenType();
                                    switch( eType )
                                    {
                                        case TOKEN_PAGE_NUMS:
                                        {
                                            if( TOKEN_TAB_STOP == ePrevType )
                                            {
                                                // remove Tab
                                                aEnumer.GetPrevTokenType();
                                                aEnumer.RemoveCurToken();
                                                // insert new Token
                                                if( 0x09 == sDelimiter.GetChar( 0 ) )
                                                {
                                                    SwFormToken aToken( TOKEN_TAB_STOP );
                                                    aToken.eTabAlign = SVX_TAB_ADJUST_END;
                                                    aEnumer.InsertToken( aToken );
                                                }
                                                else
                                                {
                                                    SwFormToken aToken( TOKEN_TEXT );
                                                    aToken.sText = sDelimiter;
                                                    aEnumer.InsertToken( aToken );
                                                }
                                                aForm.SetPattern( nLevel,
                                                        aEnumer.GetPattern() );
                                            }
                                            eType = TOKEN_END;
                                            break;
                                        }
                                    }
                                    ePrevType = eType;
                                }
                                while( TOKEN_END != eType );
                            }
                            pBase->SetTOXForm( aForm );
                        }
                    }
                    break;
                case 'h':
                    {
                        eOptions |= TOI_ALPHA_DELIMITTER;
                    }
                    break;
                case 'r':
                    {
                        SwForm aForm( pBase->GetTOXForm() );
                        aForm.SetCommaSeparated( TRUE );
                        pBase->SetTOXForm( aForm );
                    }
                    break;
                /*
                // the following switches are not (yet) supported
                // by good old StarWriter:
                case 'b':
                case 'd':
                case 'f':
                case 'g':
                case 'I':
                case 'p':
                case 's':
                    break;
                */
                }
            }
            /*
            const TOXTypes eType = pBase->GetTOXType()->GetType();
            switch( eType )
            {
                case TOX_CONTENT:
                    if( eCreateFrom )
                        pBase->SetCreate( eCreateFrom );
                    break;
                case TOX_ILLUSTRATIONS:
                    if( !eCreateFrom )
                        eCreateFrom = TOX_SEQUENCE;
                    pBase->SetCreate( eCreateFrom );
                    break;
            }
            */
            pBase->SetOptions( eOptions );
        }
        break;





    case TOX_CONTENT:
        {
            // TOX_OUTLINELEVEL setzen wir genau dann, wenn
            // die Parameter \o in 1 bis 9 liegen
            // oder der Parameter \f existiert
            // oder GARKEINE Switches Parameter angegeben sind.
            USHORT eCreateFrom = 0;
            USHORT nMaxLevel = 0;
            long nRet;
            _ReadFieldParams aReadParam( rStr );
            while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
            {
                switch( nRet )
                {
                case 'a':
                case 'c':
                    lcl_toxMatchACSwitch(*this, rDoc, *pBase, aReadParam,
                                           ('c' == nRet)
                                         ? CAPTION_COMPLETE
                                         : CAPTION_TEXT );
                    break;
                case 'o':
                    {
                        USHORT nVal;
                        if( !aReadParam.GetTokenSttFromTo(0, &nVal, MAXLEVEL) )
                        {
                            nVal = aForm.GetFormMax()-1;
                        }
                        if( nMaxLevel < nVal )
                            nMaxLevel = nVal;
                        eCreateFrom |= TOX_OUTLINELEVEL;
                    }
                    break;
                case 'f':
                    eCreateFrom |= TOX_MARK;
                    break;
                case 'l':
                    {
                        USHORT nVal;
                        if( aReadParam.GetTokenSttFromTo(0, &nVal, MAXLEVEL) )
                        {
                            if( nMaxLevel < nVal )
                                nMaxLevel = nVal;
                            eCreateFrom |= TOX_MARK;
                        }
                    }
                    break;
                case 't': // paragraphs using special styles shall
                          // provide the TOX's content
                    lcl_toxMatchTSwitch(*this, *pBase, aReadParam);
                    eCreateFrom |= TOX_TEMPLATE;
                    break;
                case 'p':
                    {
                        xub_StrLen n = aReadParam.GoToTokenParam();
                        if( STRING_NOTFOUND != n )  // if NO String just ignore the \p
                        {
                            String sDelimiter( aReadParam.GetResult() );
                            SwForm aForm( pBase->GetTOXForm() );

                            // Attention: if TOX_CONTENT brave
                            //            GetFormMax() returns MAXLEVEL + 1  !!
                            USHORT nEnd = aForm.GetFormMax()-1;

                            for(USHORT nLevel = 1;
                                   nLevel <= nEnd;
                                   ++nLevel)
                            {
                                // Levels count from 1
                                // Level 0 is reserved for CAPTION

                                // Delimiter statt Tabstop vor der Seitenzahl einsetzen,
                                // falls es eine Seitenzahl gibt:
                                FormTokenType ePrevType = TOKEN_END;
                                FormTokenType eType;
                                SwFormTokenEnumerator aEnumer =
                                    aForm.CreateTokenEnumerator( nLevel );
                                do
                                {
                                    eType = aEnumer.GetNextTokenType();
                                    switch( eType )
                                    {
                                        case TOKEN_PAGE_NUMS:
                                        {
                                            if( TOKEN_TAB_STOP == ePrevType )
                                            {
                                                // remove Tab
                                                aEnumer.GetPrevTokenType();
                                                aEnumer.RemoveCurToken();
                                                // insert new Token
                                                SwFormToken aToken( TOKEN_TEXT );
                                                aToken.sText = sDelimiter;

                                                aEnumer.InsertToken( aToken );
                                                aForm.SetPattern( nLevel,
                                                        aEnumer.GetPattern() );
                                            }
                                            eType = TOKEN_END;
                                            break;
                                        }
                                    }
                                    ePrevType = eType;
                                }
                                while( TOKEN_END != eType );
                            }
                            pBase->SetTOXForm( aForm );
                        }
                    }
                    break;
                case 'n': // don't print page numbers
                    {
                        // read START and END param
                        USHORT nStart, nEnd;
                        if( !aReadParam.GetTokenSttFromTo(  &nStart,
                                                            &nEnd,
                                                            MAXLEVEL ) )
                        {
                            nStart = 1;
                            nEnd = aForm.GetFormMax()-1;
                        }
                        // remove page numbers from this levels
                        SwForm aForm( pBase->GetTOXForm() );
                        if( aForm.GetFormMax() <= nEnd)
                            nEnd = aForm.GetFormMax()-1;
                        for(USHORT nLevel = nStart;
                               nLevel <= nEnd;
                               ++nLevel)
                        {
                            // Levels count from 1
                            // Level 0 is reserved for CAPTION

                            // Seitenzahl und ggfs. davorstehenden Tabstop entfernen:
                            FormTokenType eType;
                            SwFormTokenEnumerator aEnumer =
                                aForm.CreateTokenEnumerator( nLevel );
                            do
                            {
                                eType = aEnumer.GetNextTokenType();
                                switch( eType )
                                {
                                    case TOKEN_PAGE_NUMS:
                                    {
                                        aEnumer.RemoveCurToken();
                                        if( TOKEN_TAB_STOP
                                            == aEnumer.GetPrevTokenType() )
                                        {
                                            aEnumer.RemoveCurToken();
                                            aForm.SetPattern( nLevel,
                                                    aEnumer.GetPattern() );
                                        }
                                        eType = TOKEN_END;
                                        break;
                                    }
                                }
                            }
                            while( TOKEN_END != eType );
                        }
                        pBase->SetTOXForm( aForm );
                    }
                    break;

                /*
                // the following switches are not (yet) supported
                // by good old StarWriter:
                case 'b':
                case 's':
                case 'd':
                    break;
                */
                }
            }
            if( !nMaxLevel )
                nMaxLevel = MAXLEVEL;
            pBase->SetLevel( nMaxLevel );

            const TOXTypes eType = pBase->GetTOXType()->GetType();
            switch( eType )
            {
                case TOX_CONTENT:
                    if( eCreateFrom )
                        pBase->SetCreate( eCreateFrom );
                    break;
                case TOX_ILLUSTRATIONS:
                    if( !eCreateFrom )
                        eCreateFrom = TOX_SEQUENCE;
                    pBase->SetCreate( eCreateFrom );
                    break;
            }
        }
        break;
    case TOX_USER:
        break;
    } // ToxBase fertig

    // Update fuer TOX anstossen
    rDoc.SetUpdateTOX( TRUE );

    const SwPosition* pPos = pPaM->GetPoint();

    SwFltTOX aFltTOX( pBase, nIndexCols );

    // test if there is already a break item on this node
    SwCntntNode* pNd = pPos->nNode.GetNode().GetCntntNode();
    if( pNd )
    {
        const SfxItemSet* pSet = pNd->GetpSwAttrSet();
        if( pSet )
        {
            if( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, FALSE ) )
                aFltTOX.SetHadBreakItem( TRUE );
            if( SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, FALSE ) )
                aFltTOX.SetHadPageDescItem( TRUE );
        }
    }

    // Setze Anfang in Stack
    pEndStck->NewAttr( *pPos, aFltTOX );
    if( 1 < nIndexCols )
        bDontCreateSep = TRUE;

    // Setze Ende in Stack
    pEndStck->SetAttr( *pPos, RES_FLTR_TOX );

    return F_OK;
}

eF_ResT SwWW8ImplReader::Read_F_Hyperlink( WW8FieldDesc* pF, String& rStr )
{
    eF_ResT eRet = F_OK;
    String sURL, sTarget, sMark;
    BOOL bDataImport=FALSE;

    // JP 02.12.98: es gibt Hyperlink-Felder, die am Ende eine '\x01' stehen
    //              haben. Die wollen wir aber nicht beachten
    /*
    if( pStr[ pF->nLCode - 1 ] < ' ' )
        pStr[ pF->nLCode - 1 ] = 0;
    */



/*
    //caolan
    if( rStr.GetChar( pF->nLCode - 1 ) < ' ' )
    {
#if DEBUG
        if( 0x01 == rStr.GetChar( pF->nLCode-1 ) )
            bDataImport=ImportURL( sURL, sMark, pF->nSCode + pF->nLCode-1 );
#endif
        rStr.SetChar( pF->nLCode - 1, 0 );
    }
*/

    rStr.EraseTrailingChars( 1 );



    if (!bDataImport)
    {
        long nRet;
        _ReadFieldParams aReadParam( rStr );
        while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
            switch( nRet )
            {
            case -2:
                if( !sURL.Len() )
                {
                    ConvertFFileName( sURL, aReadParam.GetResult() );
                    //#82900# Need the more sophisticated url converter. cmc
                    //sURL = INetURLObject::RelToAbs( sURL );
                    sURL = URIHelper::SmartRelToAbs( sURL );
                }
                break;

            case 'n':
                sTarget = WW8_ASCII2STR( "_blank" );
                break;

            case 'l':
                nRet = aReadParam.SkipToNextToken();
                if( -2 == nRet )
                {
                    sMark = aReadParam.GetResult();
                    if( sMark.Len() && '"' == sMark.GetChar( sMark.Len()-1 ))
                        sMark.Erase( sMark.Len() - 1 );

                }
                break;

            case 'h':
            case 'm':
            case 's':
            case 't':
                ASSERT( !this, "Auswertung fehlt noch - Daten unbekannt" );
                break;
            }
    }

    String sDef( GetFieldResult( pF ) );                // das Resultat uebernehmen
    if( ( sURL.Len() || sMark.Len() ) && sDef.Len() )
    {
        if( sMark.Len() )
            ( sURL += INET_MARK_TOKEN ) += sMark;

        const xub_StrLen nLen = sDef.Len();
        if(    4 < nLen
            && 0x13 == sDef.GetChar( 0 )
            && 0x14 == sDef.GetChar( nLen-3 )
            && 0x01 == sDef.GetChar( nLen-2 )
            && 0x15 == sDef.GetChar( nLen-1 ) )
        {

            WW8ReaderSave aSave( this );        // rettet Flags u.ae. u. setzt sie zurueck
            bNeverCallProcessSpecial = TRUE;
            ReadText( pF->nSRes+pF->nLRes-2, 1, pPlcxMan->GetManType() );
            aSave.Restore( this );

            if( pFmtOfJustInsertedGraphicOrOLE )
            {
                SwFmtURL aURL;
                aURL.SetTargetFrameName( sTarget );
                aURL.SetURL( sURL, FALSE );
                pFmtOfJustInsertedGraphicOrOLE->SetAttr( aURL );
                pFmtOfJustInsertedGraphicOrOLE = 0;
            }
            eRet = F_OK;
        }
        else
        {
            SwFmtINetFmt aURL( sURL, sTarget );
//          aURL.SetVisitedFmt( "??" );
//          aURL.SetINetFmt( "??" );
            pRefFldStck->NewAttr( *pPaM->GetPoint(), aURL );

            // das Ende als "relative" Pos auf den Stack setzen
            pPaM->SetMark();
            pPaM->GetMark()->nContent += sDef.Len();
            /*#83156#
            We need to know the length of this content field here, but we
            do not truly know the length of the final result as we may
            have nested fields, but we have fundamental problems with
            nested fields both in OOo and in this filter, we cannot handle
            them properly. So for now we have to dump the raw content of the
            field, field commands and all, as the payload of the hyperlink.

            We also know in advance that characters less that 0x20 will not
            be inserted in the document, as they will be stripped out, e.g
            the field codes themselves 0x13,0x14,0x15. So we must adjust our
            expected length by these.

            */
            for(xub_StrLen i=sDef.Len();i>0;i--)
                if (sDef.GetChar(i-1) < 0x20)
                    pPaM->GetMark()->nContent--;
            pRefFldStck->SetAttr( *pPaM->GetMark(), RES_TXTATR_INETFMT, FALSE );
            pPaM->DeleteMark();

            eRet = F_TEXT;
        }
    }
    return eRet;
}


void SwWW8ImplReader::ImportTox( int nFldId, String aStr )
{
    BOOL bIdx = ( nFldId != 9 );
    TOXTypes eTox = ( !bIdx ) ? TOX_CONTENT : TOX_INDEX;    // Default

    USHORT nLevel = 1;

    xub_StrLen n;
    String sKey1;
    long nRet;
    _ReadFieldParams aReadParam( aStr );
    while( -1 != ( nRet = aReadParam.SkipToNextToken() ))
        switch( nRet )
        {
        case -2:
            if( !sKey1.Len() )
            {
                sKey1 = aReadParam.GetResult().GetToken(0, ':');
                if( !sKey1.Len() )
                    sKey1 = aReadParam.GetResult(); // PrimaryKey ohne ":", 2nd dahinter
            }
            break;

        case 'f':
            n = aReadParam.GoToTokenParam();
            if( STRING_NOTFOUND != n )
            {
                String sParams( aReadParam.GetResult() );
                if( 'C' != sParams.GetChar(0) && 'c' != sParams.GetChar(0) )
                    eTox = TOX_USER;
            }
            break;

        case 'l':
            n = aReadParam.GoToTokenParam();
            if( STRING_NOTFOUND != n )
            {
                String sParams( aReadParam.GetResult() );
                if(    sParams.Len() // if NO String just ignore the \l
                    && sParams.GetChar( 0 ) > '0'
                    && sParams.GetChar( 0 ) <= '9' )
                {
                    nLevel = (USHORT)sParams.ToInt32();
                }
            }
            break;
        }

    ASSERT( rDoc.GetTOXTypeCount( eTox ), "Doc.GetTOXTypeCount() == 0  :-(" );

    const SwTOXType* pT = rDoc.GetTOXType( eTox, 0 );
    SwTOXMark aM( pT );
    if( eTox != TOX_INDEX )
        aM.SetLevel( nLevel );

    if( sKey1.Len() )
    {
        aM.SetAlternativeText( sKey1 ); // WW/SW: unterschiedliche Reihenfolge
                                        // Text mit Key1 tauschen
    }
    else
        aM.SetAlternativeText( aStr );

    if( !aM.IsAlternativeText() )
    {
        pPaM->SetMark();
        pPaM->GetMark()->nContent += aStr.Len();
    }
    rDoc.Insert( *pPaM, aM );
    if( !aM.IsAlternativeText() )
        pPaM->DeleteMark();
}

void SwWW8ImplReader::Read_FldVanish( USHORT, BYTE*, short nLen )
{
    // Vorsicht: Bei Feldnamen mit Umlauten geht das MEMICMP nicht!
    const static sal_Char *aFldNames[] = {  "\x06""INHALT", "\x02""XE", // dt.
                                            "\x02""TC"  };              // us
    const static BYTE  aFldId[] = { 9, 4, 9 };

    if( nIniFlags & WW8FL_NO_FLD )
        return;

    if( nLen < 0 )
    {
        bIgnoreText = FALSE;
        return;
    }

    // our methode was called from
    // ''Skip attributes of field contents'' loop within ReadTextAttr()
    if( bIgnoreText )
        return;

    bIgnoreText = TRUE;
    long nOldPos = pStrm->Tell();
    USHORT nFieldLen;

    WW8_CP nStartCp = pSBase->WW8Fc2Cp( nOldPos );
    String sFieldName;
    nFieldLen = pSBase->WW8ReadString( *pStrm, sFieldName, nStartCp,
                                         500, eStructCharSet );
    pStrm->Seek( nOldPos );

    xub_StrLen nC = 0;
    if( 0x13 != sFieldName.GetChar( nC )) // Field Start Mark
    {
        if( 0x15 == sFieldName.GetChar( nC ))       // Field End Mark found
            bIgnoreText = FALSE;
        return;                 // kein Feld zu finden
    }

    nC++;
    while( ' '  == sFieldName.GetChar( nC ))
        nC++;

    for( int i = 0; i < 2; i++ )
    {
        const sal_Char* pName = aFldNames[i];
        USHORT nNameLen = *pName++;
        if( sFieldName.EqualsIgnoreCaseAscii( pName, nC, nNameLen ) )
        {
            ImportTox( aFldId[i], sFieldName.Copy( nC + nNameLen ) );
            break;                  // keine Mehrfachnennungen moeglich
        }
    }
    bIgnoreText = TRUE;
    pStrm->Seek( nOldPos );
}

// Read_Invisible ist fuer das "Unsichtbar"-Zeichenattribut. Mangels
// entsprechender Funktion im SW evtl. als Tag.
//
// ACHTUNG: Methode gelegentlich umstellen: unsichtbaren Text als
//                  *Feld* integrieren...
//
void SwWW8ImplReader::Read_Invisible( USHORT, BYTE* pData, short nLen )
{
    USHORT n = WWF_INVISIBLE;               // Bit-Nummer fuer Invisible
    USHORT nI = n / 32;                     // # des UINT32
    ULONG nMask = 1 << ( n % 32 );  // Maske fuer Bits

    if(    ( nFieldTagBad[    nI ] & nMask )
        || ( nFieldTagAlways[ nI ] & nMask ) )
    {
        String aTag( WW8_ASCII2STR( "{INVISIBLE " ) );

        if( nLen < 0 )
            aTag.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "END}" ) );
        else
            aTag.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "START}" ) );

        InsertTagField( n, aTag );
    }
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/ww8par5.cxx,v 1.13 2001-03-05 13:13:24 cmc Exp $


      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.12  2001/02/27 16:25:05  cmc
      #81314#,#81326# Index fixes

      Revision 1.11  2001/02/26 13:44:24  cmc
      Ruby Text Import

      Revision 1.10  2001/02/21 13:49:03  cmc
      Combined Characters Field Import

      Revision 1.9  2001/02/21 12:45:25  os
      use database struct instead of a combined string

      Revision 1.8  2001/01/31 14:32:46  cmc
      #83156# Hush ASSERT on nested hyperlink import

      Revision 1.7  2001/01/18 11:58:46  cmc
      #82900# Needed the more sophisticated relative url converter for hyperlinks

      Revision 1.6  2000/12/04 14:08:08  khz
      #78930# Pictures in Hyperlinks will be imported as Graphics with Hyperlink

      Revision 1.5  2000/11/24 11:04:38  khz
      WW field names use ANSI encoding - not plain 7-bit ASCII

      Revision 1.4  2000/11/20 14:11:17  jp
      Read_FieldIniFlags removed

      Revision 1.3  2000/10/25 14:19:04  khz
      code cleaned up

      Revision 1.2  2000/10/06 13:11:18  jp
      should changes: don't use IniManager

      Revision 1.1.1.1  2000/09/18 17:14:58  hr
      initial import

      Revision 1.71  2000/09/18 16:05:01  willem.vandorp
      OpenOffice header added.

      Revision 1.70  2000/08/31 06:29:28  jp
      use CharClass instead of international

      Revision 1.69  2000/08/04 10:56:54  jp
      Soft-/HardHyphens & HardBlanks changed from attribute to unicode character

      Revision 1.68  2000/07/24 16:39:08  khz
      #75701# set Break or PageDesc item BEHIND TOX if not found before the #0x0C

      Revision 1.67  2000/06/26 12:59:11  os
      INetURLObject::SmartRelToAbs removed

      Revision 1.66  2000/06/15 15:57:31  khz
      Prevent FindParaStart() from calling rStr.GetChar() behind end of string

      Revision 1.65  2000/06/13 08:25:46  os
      using UCB

      Revision 1.64  2000/06/06 15:00:34  khz
      Changes for Unicode

      Revision 1.63  2000/05/18 10:59:02  jp
      Changes for Unicode

      Revision 1.62  2000/05/17 16:40:46  khz
      Changes for unicode (2)

      Revision 1.60  2000/05/16 12:03:45  jp
      Changes for unicode

      Revision 1.59  2000/05/16 11:06:28  khz
      Unicode code-conversion

      Revision 1.58  2000/05/08 09:25:49  khz
      Task #74474# Find TRUE end of field code in WW8FldParaGuess class

      Revision 1.57  2000/05/05 16:07:41  khz
      Task #74474# don't create Sections while skipping result of multi-column index-field

      Revision 1.56  2000/03/27 11:44:40  cmc
      #74329# Added OCX Field Reader

      Revision 1.55  2000/03/22 09:55:35  khz
      Task #74378# release memory that was allocated by FindPara()

      Revision 1.54  2000/02/18 09:38:12  cmc
      #69372# Improved Hyperlink Importing for WW97

      Revision 1.53  2000/02/11 14:40:28  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.52  2000/01/17 11:22:15  khz
      Task #62941# import DateTime as single field (before 2 field separated by space)

      Revision 1.51  2000/01/11 10:17:43  khz
      Task #62941# import different formats for date- and time-fields

      Revision 1.50  2000/01/04 16:48:36  khz
      Task #70863# Indexes: parameter >> \e '0x09' << shows page-num at right col-margin

      Revision 1.49  1999/12/23 17:01:22  khz
      Task #70788# *De*activate bIgnoreText in Read_FldVanish() when 0x15 reached

      Revision 1.48  1999/12/21 09:25:48  khz
      Task #71111# Prevent from reading CONTENT when skiping field attributes

      Revision 1.47  1999/12/06 09:35:05  khz
      Task #70486# Now \t param on INDEX field may have one single NAME (without LEVEL)

      Revision 1.46  1999/12/02 16:39:28  khz
      Task #69857# Removed TOI_FF from default TOX_INDEX flags

      Revision 1.45  1999/11/26 12:51:17  khz
      Task #69898# TOC parameter \o will include *all* levels when no levels specified

      Revision 1.44  1999/11/26 12:27:21  khz
      Task #69892# TOC parameter \n will modify *all* levels when no levels specified

      Revision 1.43  1999/11/02 15:59:45  khz
      import new TOX_CONTENT and TOX_INDEX features (2)

      Revision 1.42  1999/10/29 12:07:39  khz
      import new TOX_CONTENT and TOX_INDEX features

      Revision 1.41  1999/10/13 21:09:26  khz
      Import Table Of Contents (0)

      Revision 1.40  1999/08/30 19:53:08  JP
      Bug #68219#: no static members - be reentrant


      Rev 1.39   30 Aug 1999 21:53:08   JP
   Bug #68219#: no static members - be reentrant

      Rev 1.38   09 Aug 1999 14:16:12   JP
   read W95 format from stream

      Rev 1.37   30 Jul 1999 17:53:36   KHZ
   Task #62955# name matching for docinfo fields (1)

      Rev 1.36   27 Jul 1999 11:23:44   KHZ
   Task #62941# find number format for date/time fields

      Rev 1.35   15 Jul 1999 19:03:48   JP
   Read_Field: if read only result then skip one char more back

      Rev 1.34   07 Jul 1999 08:08:20   OS
   extended indexes: Index names in ShellRes

      Rev 1.33   21 Jun 1999 12:16:12   KHZ
   Reference field (page, bookmark, footnote) part#2

      Rev 1.32   18 Jun 1999 15:53:52   KHZ
   Reference field (page, bookmark, footnote) part#1

      Rev 1.31   03 Jun 1999 16:59:56   KHZ
   Task #66418# UEberfluessiges ''xStrm->Seek( nOldPos );'' entfernt

      Rev 1.30   02 Jun 1999 09:32:24   KHZ
   Task #66227# a) kein Unicode bei Ver67 ;-)  b) Grafik in grupp. Textbox

      Rev 1.29   27 Apr 1999 14:39:10   KHZ
   Task #65353# ueberfluessiges AppendTxtNode in Read_F_IncludeText entfernt


      Rev 1.28   26 Apr 1999 19:22:54   JP
   Bug #65298#: Grf/Ole bei Zeichenbindung immer Oben zu Grundline anordnen

      Rev 1.27   18 Feb 1999 12:44:38   KHZ
   Task #61675# Falsche Offsetberechnung bei ImportTox() in Read_FldVanish()

      Rev 1.26   23 Dec 1998 16:15:00   KHZ
   Task #60444# Inhaltverzeichnis jetzt auch bei fehlenden Switches

      Rev 1.25   15 Dec 1998 10:50:44   KHZ
   Tasks #59580# und #58766# (Nachzieher aus SRV506)

      Rev 1.24   07 Dec 1998 11:27:28   KHZ
   Task #58766# Unicode in benutzerdef. Verzeichnissen

      Rev 1.23   02 Dec 1998 20:06:34   JP
   Bug #60045#: Marks von Hyperlinks einlesen

      Rev 1.22   02 Dec 1998 11:41:06   KHZ
   Task #59834# ein vergessenes >>delete[]( pDef )><<

      Rev 1.21   24 Nov 1998 21:11:22   JP
   Task #59822#: OLE-Objecte einlesen

      Rev 1.20   19 Nov 1998 10:37:02   TRI
   OS2 Anpassungen

      Rev 1.19   18 Nov 1998 16:29:46   HR
   doppeltes const raus

      Rev 1.18   18 Nov 1998 10:33:28   KHZ
   Task #59078# Compilerfehler unter Unix

      Rev 1.17   17 Nov 1998 20:25:12   JP
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.16   17 Nov 1998 20:05:56   JP
   Task #59476#: Seitenumrandung / Seitennummern / Seitennummernformat lesen

      Rev 1.15   17 Nov 1998 10:48:54   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.14   16 Nov 1998 18:59:18   KHZ
   Task #59078# Feldimport SEQ (fertig)

      Rev 1.13   16 Nov 1998 18:34:22   KHZ
   Task #59078# Feldimport SEQ (Teil 1)

      Rev 1.12   16 Nov 1998 13:19:52   JP
   Task #59476#: Hyperlinks lesen, ImportGrf vereinfacht, Bookmarks kein ToUpper

      Rev 1.11   05 Nov 1998 13:07:36   KHZ
   Task #58993# Befehl INCLUDEPICTURE liefert jetzt korrekten String

      Rev 1.10   30 Oct 1998 16:13:48   KHZ
   Task #57836# Beliebige Parameterreihenfolge bei Feld INCLUDEPICTURE moeglich

      Rev 1.9   30 Oct 1998 16:10:54   KHZ

      Rev 1.8   29 Oct 1998 15:12:24   KHZ
   Task #58620# GPF bei Feld EINFUEGENGRAFIK in Doc mit UNICODE

      Rev 1.7   15 Sep 1998 19:35:46   JP
   Bug #56310#: Teilfix - kein GPF mehr

      Rev 1.6   13 Sep 1998 15:50:22   HJS
   #56149# authorfield

      Rev 1.5   03 Sep 1998 22:16:14   KHZ
   Task #55189# Textboxen

      Rev 1.4   11 Aug 1998 14:39:46   KHZ
   Task #52607# neue Numerierungstypen (siehe auch WWPAR.CXX und Bug 54796)

      Rev 1.3   05 Aug 1998 09:32:08   KHZ
   Task #52607# Kodepflege

      Rev 1.2   30 Jun 1998 21:33:24   KHZ
   Header/Footer/Footnotes weitgehend ok

      Rev 1.1   23 Jun 1998 11:15:28   KHZ
   memicmp ersetzt durch MEMICMP

      Rev 1.0   16 Jun 1998 11:08:28   KHZ
   Initial revision.

*************************************************************************/




