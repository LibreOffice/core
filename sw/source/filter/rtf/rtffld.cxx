/*************************************************************************
 *
 *  $RCSfile: rtffld.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:56 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include <ctype.h>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _RTFTOKEN_H
#include <svtools/rtftoken.h>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif

#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _SWPARRTF_HXX
#include <swparrtf.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif


// bestimme, ob es sich um ein IMPORT/TOC - Feld handelt.
// return:  0 - weder noch,
//          1 - TOC
//          2 - IMPORT
//          3 - INDEX
enum RTF_FLD_TYPES {
    RTFFLD_UNKNOWN = 0,
    RTFFLD_TOC,
    RTFFLD_IMPORT,
    RTFFLD_INDEX,
    RTFFLD_SYMBOL,
    RTFFLD_PAGE,
    RTFFLD_DATE,
    RTFFLD_DATA,
    RTFFLD_MERGEFLD,
    RTFFLD_HYPERLINK
};

static RTF_FLD_TYPES _WhichFld( String& rName, String& rNext )
{
    // Strings sind PascalStrings; Laenge steht an 1. Stellen, dadurch wird
    // sich der Aufruf von strlen erspart!!!
    sal_Char __READONLY_DATA sTOC[]=        "\3toc";
    sal_Char __READONLY_DATA sIMPORT[]=     "\6import";
    sal_Char __READONLY_DATA sINDEX[]=      "\5index";
    sal_Char __READONLY_DATA sSYMBOL[]=     "\6symbol";
    sal_Char __READONLY_DATA sPAGE[]=       "\4page";
    sal_Char __READONLY_DATA sDATE[]=       "\4date";
    sal_Char __READONLY_DATA sDATA[]=       "\4data";
    sal_Char __READONLY_DATA sMERGEFLD[]=   "\10mergefield";
    sal_Char __READONLY_DATA sIMPORT2[]=    "\16includepicture";
    sal_Char __READONLY_DATA sHYPERLINK[]=  "\x9hyperlink";

    struct _Dummy_RTF_FLD_TYPES
    {
        RTF_FLD_TYPES eFldType;
        const sal_Char* pFldNm;
    };
    __READONLY_DATA _Dummy_RTF_FLD_TYPES aFldNmArr[ RTFFLD_HYPERLINK+1 ] = {
            RTFFLD_TOC,         sTOC,
            RTFFLD_IMPORT,      sIMPORT,
            RTFFLD_INDEX,       sINDEX,
            RTFFLD_SYMBOL,      sSYMBOL,
            RTFFLD_PAGE,        sPAGE,
            RTFFLD_DATE,        sDATE,
            RTFFLD_DATA,        sDATA,
            RTFFLD_MERGEFLD,    sMERGEFLD,
            RTFFLD_IMPORT,      sIMPORT2,
            RTFFLD_HYPERLINK,   sHYPERLINK
    };


    if( !rName.Len() )
        return RTFFLD_UNKNOWN;

    String sNm( rName );
    sNm = sNm.EraseLeadingChars().GetToken(0, ' ');
    ASSERT( sNm.Len(), "Feldname hat keine Laenge!" );
    if( !sNm.Len() )
        return RTFFLD_UNKNOWN;

    xub_StrLen nTokenStt = rName.Search( sNm );
    sNm.ToLowerAscii();

    for( int n = 0; n < sizeof( aFldNmArr ) / sizeof( aFldNmArr[0]); ++n )
    {
        const sal_Char* pCmp = aFldNmArr[n].pFldNm;
        int nLen = *pCmp++;
        xub_StrLen nFndPos = sNm.SearchAscii( pCmp );
        if( STRING_NOTFOUND != nFndPos &&
            ( !nFndPos || !isalpha(sNm.GetChar( nFndPos-1 )) ) &&
            ( nFndPos+nLen == sNm.Len() || !isalpha(sNm.GetChar(nFndPos+nLen) ) ) )
        {
//          rName = sNm.Copy( nFndPos, nLen );
            rName = rName.Copy( nFndPos, nLen );
            nFndPos += nTokenStt + nLen;
            while( rNext.GetChar( nFndPos ) == ' ' )    ++nFndPos;
            rNext.Erase( 0, nFndPos );
            rNext.EraseTrailingChars();
            return aFldNmArr[n].eFldType;
        }
    }
    return RTFFLD_UNKNOWN;      // nichts gefunden.
}

static USHORT CheckNumberFmtStr( const String& rNStr )
{
    // zur Kontrolle, falls jemand meint, neue Werte zufuegen zu muessen
#define NUMBERTAB_SZ 8
    sal_Char __READONLY_DATA
        sNType0[] = "\10ALPHABETIC",       /* CHARS_UPPER_LETTER*/
        sNType1[] = "\10alphabetic",       /* CHARS_LOWER_LETTER*/
        sNType2[] = "\5ROMAN",            /* ROMAN_UPPER       */
        sNType3[] = "\5roman",            /* ROMAN_LOWER       */
        sNType4[] = "\6ARABIC",           /* ARABIC            */
        sNType5[] = "\4NONE",             /* NUMBER_NONE       */
        sNType6[] = "\4CHAR",             /* CHAR_SPECIAL      */
        sNType7[] = "\4PAGE";             /* PAGEDESC          */
    static const sal_Char* __READONLY_DATA aNumberTypeTab[ NUMBERTAB_SZ ] =
    {
        sNType0, sNType1, sNType2, sNType3, sNType4,
        sNType5, sNType6, sNType7
    };


    for( USHORT n = SVX_NUM_CHARS_UPPER_LETTER;  n <= SVX_NUM_PAGEDESC; n++ )
    {
        const sal_Char* pCmp = aNumberTypeTab[ n ];
        int nLen = *pCmp++;
        if( rNStr.EqualsAscii( pCmp, 0, nLen ))
            return 2 <= n ? n : (n + SVX_NUM_CHARS_UPPER_LETTER_N);
    }
    return SVX_NUM_PAGEDESC;        // default-Wert
}

extern void sw3io_ConvertFromOldField( SwDoc& rDoc, USHORT& rWhich,
                                        USHORT& rSubType, ULONG &rFmt,
                                        USHORT nVersion );

class RtfFieldSwitch
{
    String sParam;
    xub_StrLen nCurPos;
public:
    RtfFieldSwitch( const String& rParam );
    sal_Unicode GetSwitch( String& rParam );

    BOOL IsAtEnd() const { return nCurPos >= sParam.Len(); }
};

RtfFieldSwitch::RtfFieldSwitch( const String& rParam )
    : sParam( rParam ), nCurPos( 0  )
{
    sParam.EraseTrailingChars().EraseLeadingChars();
}

sal_Unicode RtfFieldSwitch::GetSwitch( String& rParam )
{
    // beginnt ein Schalter?
    sal_Unicode c, cKey = 0;
    if( '\\' == (c = sParam.GetChar( nCurPos )) )
    {
        if( '\\' == ( c = sParam.GetChar( ++nCurPos )) )
            c = sParam.GetChar( ++nCurPos );

        cKey = c;

        while( ++nCurPos < sParam.Len() &&
                ' ' == ( c = sParam.GetChar( nCurPos )) )
            ;
    }

    // dann alles in Hochkommatas oder bis zum naechsten // als
    // Param returnen
    USHORT nOffset;
    if( '"' != c && '\'' != c )
        c = '\\', nOffset = 0;
    else
        nOffset = 1;

    sParam.Erase( 0, nCurPos + nOffset );
    rParam = sParam.GetToken( 0, c );
    sParam.Erase( 0, rParam.Len() + nOffset ).EraseLeadingChars();
    if( '\\' == c )
        rParam.EraseTrailingChars();
    nCurPos = 0;

    return cKey;
}

int SwRTFParser::MakeFieldInst( String& rFieldStr )
{
    // sicher den Original-String fuer die FeldNamen (User/Datenbank)
    String aSaveStr( rFieldStr );
    SwFieldType * pFldType;
    xub_StrLen nPos = 0;
    USHORT nSubType;
    int nRet;

    switch( nRet = _WhichFld( rFieldStr, aSaveStr ) )
    {
    case RTFFLD_IMPORT:
        {
//JP 11.03.96: vertraegt sich nicht so ganz mit Internet!
//            if( STRING_NOTFOUND != ( nPos = aSaveStr.Search( '.' )))
//                aSaveStr.Erase( nPos+4 );

            if( aSaveStr.Len() )
            {
                sal_Unicode c = aSaveStr.GetChar( 0 );
                if( '"' == c || '\'' == c )
                {
                    aSaveStr.Erase( 0, 1 );
                    aSaveStr = aSaveStr.GetToken( 0, c );
                }

                rFieldStr = INetURLObject::RelToAbs( aSaveStr );
            }
//          SkipGroup();        // ueberlese den Rest
        }
        break;

    case RTFFLD_PAGE:
        {
            pFldType = pDoc->GetSysFldType( RES_PAGENUMBERFLD );
            SwPageNumberField aPF( (SwPageNumberFieldType*)pFldType,
                                    PG_RANDOM, SVX_NUM_ARABIC );
            if( STRING_NOTFOUND != ( nPos = aSaveStr.SearchAscii( "\\*" )) )
            {
                nPos += 2;
                while( aSaveStr.GetChar(nPos) == ' ' ) nPos++;
                aSaveStr.Erase( 0, nPos );

                // steht jetzt geanu auf dem Format-Namen
                aPF.ChangeFormat( CheckNumberFmtStr( aSaveStr ));
            }
            pDoc->Insert( *pPam, SwFmtFld( aPF ) );
            SkipGroup();        // ueberlese den Rest
        }
        break;
    case RTFFLD_DATE:
        {
            if( STRING_NOTFOUND == ( nPos = aSaveStr.SearchAscii( "\\@" )) )
            {
                // es fehlt die Format - Angabe: defaulten auf Datum
                pFldType = pDoc->GetSysFldType( RES_DATETIMEFLD );
                pDoc->Insert( *pPam, SwFmtFld( SwDateTimeField(
                                (SwDateTimeFieldType*)pFldType, DATEFLD )));
            }
            else
            {
                // versuche aus dem Formatstring zu erkennen, ob es ein
                // Datum oder Zeit oder Datum & Zeit Field ist
                // nur das Format interressiert
                aSaveStr.Erase( 0, aSaveStr.Search( '\"' )+1 );
                // alles hinter dem Format interressiert auch nicht mehr.
                aSaveStr.Erase( aSaveStr.Search( '\"' ) );
                aSaveStr.SearchAndReplaceAscii( "AM", aEmptyStr );
                aSaveStr.SearchAndReplaceAscii( "PM", aEmptyStr );

                xub_StrLen nDPos = aSaveStr.Search( 'M' ),  // M    -> Datum
                           nTPos = aSaveStr.Search( 'H' );  // H    -> 24h
                if( STRING_NOTFOUND == nTPos )
                    nTPos = aSaveStr.Search( 'h' );         // h    -> 12h

                SwField *pTFld = 0, *pDFld = 0;

                if( STRING_NOTFOUND != nTPos )
                {
                    pFldType = pDoc->GetSysFldType( RES_DATETIMEFLD );
                    pTFld = new SwDateTimeField( (SwDateTimeFieldType*)pFldType, TIMEFLD );
                    ((SwDateTimeField*)pTFld)->ChangeFormat(
                                'H' == aSaveStr.GetChar( nTPos )
                                        ? TF_SSMM_24
                                        : TF_SSMM_12);
                }

                if( STRING_NOTFOUND != nDPos )      // Datum ?
                {
                    static SwDateFormat aDateA[16] = {
                        DF_SHORT, DF_LMON, DF_LDAYMON, DF_LDAYMON,
                        DF_SCENT, DF_LDAYMON, DF_LDAYMON, DF_LDAYMON,
                        DF_SHORT, DF_LMON, DF_LDAYMONTH, DF_LDAYMONTH,
                        DF_SCENT, DF_LDAYMONTH, DF_LDAYMONTH, DF_LDAYMONTH
                    };
                                //  t, tt, T, TT -> no day of week
                                //  ttt, tttt, TTT, TTTT -> day of week
                    BOOL bDayOfWeek = STRING_NOTFOUND !=
                                            aSaveStr.SearchAscii( "ttt" ) ||
                                      STRING_NOTFOUND !=
                                            aSaveStr.SearchAscii( "TTT" ) ||
                                      STRING_NOTFOUND !=
                                            aSaveStr.SearchAscii( "ddd" ) ||
                                      STRING_NOTFOUND !=
                                            aSaveStr.SearchAscii( "DDD" );
                                //  M, MM -> numeric month
                                //  MMM, MMMM -> text. month
                    BOOL bLitMonth = STRING_NOTFOUND !=
                                            aSaveStr.SearchAscii( "MMM" );
                                //  MMMM -> full month
                    BOOL bFullMonth = STRING_NOTFOUND !=
                                            aSaveStr.SearchAscii( "MMMM" );
                                //  jj, JJ -> 2-col-year
                                //  jjjj, JJJJ -> 4-col-year
                    BOOL bFullYear = STRING_NOTFOUND !=
                                            aSaveStr.SearchAscii( "jjj" ) ||
                                      STRING_NOTFOUND !=
                                            aSaveStr.SearchAscii( "JJJ" ) ||
                                      STRING_NOTFOUND !=
                                            aSaveStr.SearchAscii( "yyy" ) ||
                                      STRING_NOTFOUND !=
                                            aSaveStr.SearchAscii( "YYY" );

                    USHORT i =  ( bLitMonth & 1 ) |
                                ( ( bDayOfWeek & 1 ) << 1 ) |
                                ( ( bFullYear & 1 ) << 2 ) |
                                ( ( bFullMonth & 1 ) << 3 );

                    pFldType = pDoc->GetSysFldType( RES_DATETIMEFLD );

                    nSubType = DATEFLD;
                    USHORT nWhich = RES_DATEFLD;
                    ULONG nFormat = aDateA[ i ];
                    sw3io_ConvertFromOldField( *pDoc, nWhich, nSubType, nFormat, 0x0110 );
                    pDFld = new SwDateTimeField( (SwDateTimeFieldType*)pFldType, DATEFLD, nFormat );
                }

                // Zeit kommt vor Date, alos Feld-Pointer swappen
                if( nTPos < nDPos )
                {
                    SwField* pTmp = pTFld;
                    pTFld = pDFld;
                    pDFld = pTmp;
                }

                if( pDFld )
                {
                    pDoc->Insert( *pPam, SwFmtFld( *pDFld ));
                    delete pDFld;
                }
                if( pTFld )
                {
                    pDoc->Insert( *pPam, SwFmtFld( *pTFld ));
                    delete pTFld;
                }
            }
            SkipGroup();        // ueberlese den Rest
        }
        break;
    case RTFFLD_DATA:
        {
            // Datenbank-FileName: nur der Filename interressiert
            // Zur Zeit werden nur SDF-Files verarbeitet, also suche nach
            // der Extension

            // im SWG geben die DATA Felder den Namen der Datenbank
            // an. Dieser kann als Field oder als DBInfo interpretiert
            // werden:
            //  \\data -> Datenbank-Name als Field
            //  DATA -> Datenbank-Info
            BOOL bField = rFieldStr.GetChar( 0 ) != 'D';

            // nur der Name interressiert
            if( STRING_NOTFOUND != (nPos = aSaveStr.Search( '.' )) )
                aSaveStr.Erase( nPos );

            if( bField )
            {
                pFldType = pDoc->GetSysFldType( RES_DBNAMEFLD );
                pDoc->Insert( *pPam, SwFmtFld( SwDBNameField(
                                (SwDBNameFieldType*)pFldType, aEmptyStr ) ));
            }
            else
                pDoc->ChgDBName( aSaveStr );        // MS: Keine DBInfo verwenden
            SkipGroup();        // ueberlese den Rest
        }
        break;
    case RTFFLD_MERGEFLD:
        {
            // ein Datenbank - Feld: nur der Name interressiert
            // bis zum Ende vom String ist das der Feldname
            SwDBFieldType aTmp( pDoc, aSaveStr, aEmptyStr );    // Hack(OM): Erstmal Leerstring
            SwDBField aDBFld( (SwDBFieldType*)pDoc->InsertFldType( aTmp ));

            aDBFld.ChangeFormat( UF_STRING );
            pDoc->Insert( *pPam, SwFmtFld( aDBFld ));
            SkipGroup();        // ueberlese den Rest
        }
        break;

    case RTFFLD_SYMBOL:
        {
            // loesche fuehrende Blanks
            if( IsNewGroup() )  GetAttrSet();
            SetNewGroup( TRUE );

            SfxItemSet& rSet = GetAttrSet();

            BOOL bCharIns = FALSE;
            RtfFieldSwitch aRFS( aSaveStr );
            while( !aRFS.IsAtEnd() )
            {
                String sParam;
                sal_Unicode cKey = aRFS.GetSwitch( sParam );
                if( sParam.Len() )
                    switch( cKey )
                    {
                    case 0:
                        if( !bCharIns )
                        {
                            sal_Unicode nChar = sParam.ToInt32();
                            if( nChar )
                            {
                                pDoc->Insert( *pPam, nChar );
                                bCharIns = TRUE;
                            }
                        }
                        break;

                    case 'f': case 'F':
                        // Font setzen
                        {
                            SvxRTFFontTbl& rTbl = GetFontTbl();
                            for( Font* pFont = rTbl.First(); pFont;
                                    pFont = rTbl.Next() )
                                if( pFont->GetName() == sParam )
                                {
                                    rSet.Put( SvxFontItem(
                                            pFont->GetFamily(),
                                            sParam,
                                            pFont->GetStyleName(),
                                            pFont->GetPitch(),
                                            pFont->GetCharSet(),
                                            RES_CHRATR_FONT ));
                                    break;
                                }
                        }
                        break;
                    case 'h': case 'H':
                        //??
                        break;
                    case 's': case 'S':
                        // Fontsize setzen
                        {
                            USHORT nVal = sParam.ToInt32();
                            nVal *= 20;
                            rSet.Put( SvxFontHeightItem( (const USHORT)nVal,
                                        100, RES_CHRATR_FONTSIZE ));
                        }
                        break;
                    }
            }

            if( !IsNewGroup() ) AttrGroupEnd();
            SetNewGroup( FALSE );

            SkipGroup();        // ueberlese den Rest
        }
        break;

    case RTFFLD_HYPERLINK:
        rFieldStr.Erase();
        if( aSaveStr.Len() )
        {
            // return String ist URL, # Mark, \1 Frame
            String sMark, sFrame;
            RtfFieldSwitch aRFS( aSaveStr );
            while( !aRFS.IsAtEnd() )
            {
                String sParam;
                sal_Unicode cKey = aRFS.GetSwitch( sParam );
                if( sParam.Len() )
                    switch( cKey )
                    {
                    case 0:
                        if( !rFieldStr.Len() )
                            rFieldStr = INetURLObject::RelToAbs( sParam );
                        break;

                    case 'l':   case 'L':   sMark = sParam;     break;
                    case 't':   case 'T':   sFrame = sParam;    break;
                    }
            }

            if( sMark.Len() )
                ( rFieldStr += INET_MARK_TOKEN ) += sMark;
            if( sFrame.Len() )
                ( rFieldStr += '\1' ) += sFrame;
        }
        break;

    case RTFFLD_TOC:
    case RTFFLD_INDEX:
        break;

    default:
        {
            // keines von den bekannten Feldern, also eine neues UserField
            aSaveStr.EraseLeadingChars().EraseTrailingChars();
            SwUserFieldType aTmp( pDoc, aSaveStr );
            SwUserField aUFld( (SwUserFieldType*)pDoc->InsertFldType( aTmp ));
            aUFld.ChangeFormat( UF_STRING );
            pDoc->Insert( *pPam, SwFmtFld( aUFld ));
            nRet = RTFFLD_UNKNOWN;
        }
        break;
    }
    return nRet;
}

void SwRTFParser::ReadField()
{
    int nRet = 0;
    int nOpenBrakets = 1;       // die erste wurde schon vorher erkannt !!
    int bFldInst = FALSE, bFldRslt = FALSE;
    String sFieldStr, sFieldNm;
    BYTE cCh;

    while( nOpenBrakets && IsParserWorking() )
        switch( GetNextToken() )
        {
        case '}':
            {
                --nOpenBrakets;
                if( 1 != nOpenBrakets || !bFldInst )
                    break;

                if( !bFldRslt )
                {
                    // FieldInst vollstaendig eingelesen, was ist es denn?
                    nRet = MakeFieldInst( sFieldStr );

                    // erstmal Index/Inhaltsverzeichniss ueberspringen
                    // und als normalen Text einfuegen. Spaeter mal auch dem
                    // SwPaM darum aufspannen.
                    if( RTFFLD_TOC == nRet || RTFFLD_INDEX == nRet )
                        return;

                    if( RTFFLD_IMPORT == nRet || RTFFLD_HYPERLINK == nRet )
                        sFieldNm = sFieldStr;

                    sFieldStr.Erase();
                }
                else if( RTFFLD_UNKNOWN == nRet ) // FieldResult wurde eingelesen
                {
                    // der String ist der Wert vom Feld
//                  sFieldStr.Insert( '"', 0 );
//                  sFieldStr.Insert( '"' );

                    // besorge mal das Feld:
                    SwTxtNode* pTxtNd = pPam->GetPoint()->nNode.GetNode().GetTxtNode();
                    SwTxtAttr* pFldAttr = pTxtNd->GetTxtAttr(
                            pPam->GetPoint()->nContent.GetIndex()-1 );

                    ((SwUserFieldType*)pFldAttr->GetFld().GetFld()->GetTyp())->
                                SetContent( sFieldStr );
                }
                else if( sFieldNm.Len() )
                {
                    if( RTFFLD_IMPORT == nRet )
                    {
                        // Grafik einfuegen
                        InsPicture( sFieldNm );
                        nRet = INT_MAX;
                    }
                    else if( RTFFLD_HYPERLINK == nRet && sFieldStr.Len() )
                    {
                        // im FieldStr steht der anzuzeigenden Text, im
                        pDoc->Insert( *pPam, sFieldStr );

                        String sTarget( sFieldNm.GetToken( 1, '\1' ));
                        if( sTarget.Len() )
                            sFieldNm.Erase( sFieldNm.Len() - sTarget.Len() -1 );

                        // oder ueber den Stack setzen??
                        pPam->SetMark();
                        pPam->GetMark()->nContent -= sFieldStr.Len();
                        pDoc->Insert( *pPam,
                                        SwFmtINetFmt( sFieldNm, sTarget ),
                                        SETATTR_DONTEXPAND );
                        pPam->DeleteMark();
                    }
                }
            }
            break;

        case '{':
            if( RTF_IGNOREFLAG != GetNextToken() )
                SkipToken( -1 );
            // Unknown und alle bekannten nicht ausgewerteten Gruppen
            // sofort ueberspringen
            else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                SkipToken( -2 );
            else
            {
                // gleich herausfiltern
                ReadUnknownData();
                if( '}' != GetNextToken() )
                    eState = SVPAR_ERROR;
                break;
            }
            ++nOpenBrakets;
            break;

        case RTF_DATAFIELD:
        case RTF_FIELD:
            SkipGroup();
            break;

        case RTF_FLDINST:
            bFldInst = TRUE;
            break;

        case RTF_FLDRSLT:
            bFldRslt = TRUE;
            break;

        case RTF_U:
            {
                if( nTokenValue )
                    sFieldStr += (sal_Unicode)nTokenValue;
                else
                    sFieldStr += aToken;
            }
            break;

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
            sFieldStr += ByteString::ConvertToUnicode( cCh,
                                               RTL_TEXTENCODING_MS_1252 );
            break;

        // kein Break, aToken wird als Text gesetzt
        case RTF_TEXTTOKEN:
            sFieldStr += aToken;
            break;

        case RTF_PICT:      // Pic-Daten einlesen!
            if( RTFFLD_IMPORT == nRet )
            {
                Graphic aGrf;
                SvxRTFPictureType aPicType;
                if( ReadBmpData( aGrf, aPicType ) )
                {
                    InsPicture( sFieldNm, &aGrf, &aPicType );
                    nRet = INT_MAX;
                }
                SkipGroup();
            }
            break;

        case RTF_BKMKSTART:
        case RTF_BKMKEND:
        case RTF_BKMK_KEY:
        case RTF_XE:
        case RTF_TC:
        case RTF_NEXTFILE:
        case RTF_TEMPLATE:
        case RTF_SHPRSLT:
            SkipGroup();
            break;
    }

    if( RTFFLD_IMPORT == nRet && sFieldNm.Len() )
    {
        // Grafik einfuegen
        InsPicture( sFieldNm );
    }

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/rtf/rtffld.cxx,v 1.1.1.1 2000-09-18 17:14:56 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.51  2000/09/18 16:04:50  willem.vandorp
      OpenOffice header added.

      Revision 1.50  2000/05/09 17:22:46  jp
      Changes for Unicode

      Revision 1.49  2000/03/23 19:01:03  jp
      Bug #74426#: ReadField: skip over some groups

      Revision 1.48  2000/03/10 15:56:55  jp
      Bug #74100#: read W2000 rtf-format

      Revision 1.47  2000/02/17 13:46:51  jp
      Bug #73098#: Import / Export problems

      Revision 1.46  2000/02/11 14:37:57  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.45  2000/01/25 20:12:58  jp
      Bug #72146#: read UniCode character

      Revision 1.44  1999/03/03 13:59:16  JP
      Bug #62735#: Symbolfelder komplett lesen


      Rev 1.43   03 Mar 1999 14:59:16   JP
   Bug #62735#: Symbolfelder komplett lesen

      Rev 1.42   16 Feb 1999 11:57:16   JP
   Task #61942#: Hyperlinks einlesen

      Rev 1.41   17 Nov 1998 10:45:58   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.40   11 Aug 1998 12:16:32   JP
   Bug #54796#: fehlender Numerierunstyp und Bugfixes

      Rev 1.39   06 Aug 1998 21:43:28   JP
   Bug #54796#: neue NumerierungsTypen (WW97 kompatibel)

      Rev 1.38   13 May 1998 18:07:18   JP
   PageField: auf PageDesc Format defaulten

      Rev 1.37   20 Feb 1998 13:36:30   MA
   headerfiles gewandert

      Rev 1.36   13 Feb 1998 19:22:02   MIB
   Felder: Schnittstellen-Anpassung fuer Konvertierungs-Funktionen

      Rev 1.35   27 Jan 1998 19:00:38   HR
   HP9000 Krams entfernt

      Rev 1.34   26 Nov 1997 15:05:34   MA
   headerfiles

      Rev 1.33   25 Nov 1997 12:03:02   TJ
   include svrtf.hxx

      Rev 1.32   03 Nov 1997 14:10:34   MA
   precomp entfernt

      Rev 1.31   15 Oct 1997 11:59:44   OM
   Feldumstellung

      Rev 1.30   14 Oct 1997 14:12:06   OM
   Feldumstellung

      Rev 1.29   09 Oct 1997 16:16:44   OM
   Feldumstellung

      Rev 1.28   29 Sep 1997 12:14:38   OM
   Feldumstellung

      Rev 1.27   24 Sep 1997 15:23:20   OM
   Feldumstellung

      Rev 1.26   15 Aug 1997 12:51:44   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.25   12 Nov 1996 18:28:12   sdo
   GCC-Parser

      Rev 1.24   29 Oct 1996 12:58:48   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.23   23 Oct 1996 19:49:08   JP
   String Umstellung: [] -> GetChar()

      Rev 1.22   28 Jun 1996 15:07:26   MA
   includes

      Rev 1.21   19 Jun 1996 11:57:52   MA
   headerfiles 323

      Rev 1.20   10 Jun 1996 11:23:38   JP
   ImportFeld: jetzt auch mit dem neuen WinWord 6.0 Format

      Rev 1.19   30 Apr 1996 09:36:28   OM
   Mehrere Datenbanken pro Dok

      Rev 1.17   11 Apr 1996 17:31:38   JP
   Umstellung - RelToAbs/AbsToRel

      Rev 1.16   10 Apr 1996 18:59:00   JP
   Umstellung relative/absolute Pfade fuer Grafiken

      Rev 1.15   11 Mar 1996 23:08:24   JP
   InsPicture: keine Extension abschneiden

      Rev 1.14   11 Jan 1996 22:04:00   JP
   Date/Time - Felder: die Formate richtig lesen/schreiben

      Rev 1.13   13 Dec 1995 21:04:10   JP
   Warnings entfernt

      Rev 1.12   24 Nov 1995 17:24:10   OM
   PCH->PRECOMPILED

      Rev 1.11   07 Nov 1995 20:06:20   JP
   Readonly-Daten hinter segeofglobals verschoben

      Rev 1.10   21 Aug 1995 21:30:38   JP
   svxitems-HeaderFile entfernt

      Rev 1.9   18 Aug 1995 19:05:50   mk
   HP9000 (MDA)

      Rev 1.8   09 Aug 1995 18:15:02   JP
   Daten vors SEG_EOFGLOBAL verschoben

      Rev 1.7   03 Apr 1995 20:51:30   JP
   fuer PreComp.Header eingerichtet

      Rev 1.6   13 Mar 1995 14:56:20   KH
   Mac jetzt richtig

      Rev 1.5   08 Feb 1995 09:43:08   JP
   alten RTF-Parser entfernt, Sw_RTF -> SwRTF

      Rev 1.4   18 Jan 1995 21:05:12   ER
   fld.hxx -> *fld*.hxx

      Rev 1.3   11 Jan 1995 19:36:28   JP
   RTF-Reader fertiggestellt

      Rev 1.2   09 Jan 1995 19:35:30   JP
   Symbolzeichen: kein Wert, nicht einfuegen

      Rev 1.1   06 Jan 1995 12:07:08   JP
   originalen Feldnamen returnen

      Rev 1.0   22 Dec 1994 17:34:36   JP
   Initial revision.

*************************************************************************/

