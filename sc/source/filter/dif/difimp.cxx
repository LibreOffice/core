/*************************************************************************
 *
 *  $RCSfile: difimp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:26 $
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

//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svtools/zforlist.hxx>
#include <math.h>

#include "dif.hxx"
#include "filter.hxx"
#include "fprogressbar.hxx"
#include "flttools.hxx"
#include "scerrors.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "attrib.hxx"


const sal_Unicode pKeyTABLE[]   = { 'T', 'A', 'B', 'L', 'E', 0 };
const sal_Unicode pKeyVECTORS[] = { 'V', 'E', 'C', 'T', 'O', 'R', 'S', 0 };
const sal_Unicode pKeyTUPLES[]  = { 'T', 'U', 'P', 'L', 'E', 'S', 0 };
const sal_Unicode pKeyDATA[]    = { 'D', 'A', 'T', 'A', 0 };
const sal_Unicode pKeyBOT[]     = { 'B', 'O', 'T', 0 };
const sal_Unicode pKeyEOD[]     = { 'E', 'O', 'D', 0 };
const sal_Unicode pKeyERROR[]   = { 'E', 'R', 'R', 'O', 'R', 0 };
const sal_Unicode pKeyTRUE[]    = { 'T', 'R', 'U', 'E', 0 };
const sal_Unicode pKeyFALSE[]   = { 'F', 'A', 'L', 'S', 'E', 0 };
const sal_Unicode pKeyNA[]      = { 'N', 'A', 0 };
const sal_Unicode pKeyV[]       = { 'V', 0 };
const sal_Unicode pKey1_0[]     = { '1', ',', '0', 0 };


FltError ScImportDif( SvStream& rIn, ScDocument* pDoc, const ScAddress& rInsPos,
                        const CharSet eVon, UINT32 nDifOption )
{
    DifParser   aDifParser( rIn, nDifOption, *pDoc, eVon );

    const BOOL  bPlain = aDifParser.IsPlain();

    UINT16      nBaseTab = rInsPos.Tab();

    TOPIC       eTopic = T_UNKNOWN;
    BOOL        bSyntErrWarn = FALSE;
    BOOL        bOverflowWarn = FALSE;

    String&     rData = aDifParser.aData;
    BOOL        bData = FALSE;

    UINT16      nNumCols = 0;
    UINT16      nNumRows = 0;

    rIn.Seek( 0 );

    ScfStreamProgressBar aPrgrsBar( rIn, pDoc->GetDocumentShell() );

    while( eTopic != T_DATA && eTopic != T_END )
    {
        eTopic = aDifParser.GetNextTopic();

        aPrgrsBar.Progress();

        bData = rData.Len() > 0;

        switch( eTopic )
        {
            case T_TABLE:
            {
                if( aDifParser.nVector != 0 || aDifParser.nVal != 1 )
                    bSyntErrWarn = TRUE;
                if( bData )
                    pDoc->RenameTab( nBaseTab, rData );
            }
                break;
            case T_VECTORS:
            {
                if( aDifParser.nVector != 0 )
                    bSyntErrWarn = TRUE;
                if( aDifParser.nVal > MAXCOL + 1 )
                    nNumCols = 0xFFFF;
                else
                    nNumCols = ( UINT16 ) aDifParser.nVal;
            }
                break;
            case T_TUPLES:
            {
                if( aDifParser.nVector != 0 )
                    bSyntErrWarn = TRUE;
                if( aDifParser.nVal > MAXROW + 1 )
                    nNumRows = 0xFFFF;
                else
                    nNumRows = ( UINT16 ) aDifParser.nVal;
            }
                break;
            case T_DATA:
            {
                if( aDifParser.nVector != 0 || aDifParser.nVal != 0 )
                    bSyntErrWarn = TRUE;
            }
                break;
            case T_LABEL:
            case T_COMMENT:
            case T_SIZE:
            case T_PERIODICITY:
            case T_MAJORSTART:
            case T_MINORSTART:
            case T_TRUELENGTH:
            case T_UINITS:
            case T_DISPLAYUNITS:
            case T_END:
            case T_UNKNOWN:
                break;
#ifdef DBG_UTIL
            default:
                DBG_ERROR( "*ScImportDif(): enum vergessen!" );
#endif
        }

    }


    if( eTopic == T_DATA )
    {   // Ab hier kommen die Daten
        UINT16              nBaseCol = rInsPos.Col();

        UINT16              nColCnt = 0xFFFF;
        UINT16              nRowCnt = rInsPos.Row();
        UINT32              nHandleLogic = 0xFFFFFFFF;
        DifAttrCache        aAttrCache( bPlain );

        DATASET             eAkt = D_UNKNOWN;

        while( eAkt != D_EOD )
            {
            eAkt = aDifParser.GetNextDataset();

            aPrgrsBar.Progress();

            switch( eAkt )
                {
                case D_BOT:
                    if( nColCnt < 0xFFFF )
                        nRowCnt++;
                    nColCnt = nBaseCol;
                    break;
                case D_EOD:
                    break;
                case D_NUMERIC:                 // Numbercell
                    if( nColCnt == 0xFFFF )
                        nColCnt = nBaseCol;

                    if( nColCnt <= MAXCOL && nRowCnt <= MAXROW )
                    {
                        ScBaseCell*     pCell;
                        if( DifParser::IsV( rData.GetBuffer() ) )
                        {
                            pCell = new ScValueCell( aDifParser.fVal );
                            if( !bPlain )
                                aAttrCache.SetNumFormat( nColCnt, nRowCnt,
                                    aDifParser.nNumFormat );
                        }
                        else if( rData == pKeyTRUE || rData == pKeyFALSE )
                        {
                            pCell = new ScValueCell( aDifParser.fVal );
                            if( bPlain )
                                aAttrCache.SetLogical( nColCnt, nRowCnt );
                            else
                                aAttrCache.SetNumFormat( nColCnt, nRowCnt,
                                    aDifParser.nNumFormat );
                        }
                        else if( rData == pKeyNA || rData == pKeyERROR  )
                            pCell = new ScStringCell( rData );
                        else
                        {
                            String aTmp( RTL_CONSTASCII_USTRINGPARAM( "#IND: " ));
                            aTmp += rData;
                            aTmp += sal_Unicode('?');
                            pCell = new ScStringCell( aTmp );
                        }

                        pDoc->PutCell( nColCnt, nRowCnt, nBaseTab, pCell, ( BOOL ) TRUE );
                    }
                    else
                        bOverflowWarn = TRUE;

                    nColCnt++;
                    break;
                case D_STRING:                  // Textcell
                    if( nColCnt == 0xFFFF )
                        nColCnt = nBaseCol;

                    if( nColCnt <= MAXCOL && nRowCnt <= MAXROW )
                    {
                        if( rData.Len() > 0 )
                        {
                            pDoc->PutCell( nColCnt, nRowCnt, nBaseTab,
                                ScBaseCell::CreateTextCell( rData, pDoc ), ( BOOL ) TRUE );
                        }
                    }
                    else
                        bOverflowWarn = TRUE;

                    nColCnt++;
                    break;
                case D_UNKNOWN:
                    break;
                case D_SYNT_ERROR:
                    break;
#ifdef DBG_UTIL
                default:
                    DBG_ERROR( "*ScImportDif(): enum vergessen!" );
#endif
            }
        }

        aAttrCache.Apply( *pDoc, nBaseTab );
    }
    else
        return eERR_FORMAT;

    if( bSyntErrWarn )
        //###############################################
        // ACHTUNG: Hier fehlt noch die richtige Warnung!
        return eERR_RNGOVRFLW;
        //###############################################
    else if( bOverflowWarn )
        return eERR_RNGOVRFLW;
    else
        return eERR_OK;
}


DifParser::DifParser( SvStream& rNewIn, const UINT32 nOption, ScDocument& rDoc, CharSet e ) :
    rIn( rNewIn )
{
    eCharSet = e;
    if ( rIn.GetStreamCharSet() != eCharSet )
    {
        DBG_ERRORFILE( "CharSet passed overrides and modifies StreamCharSet" );
        rIn.SetStreamCharSet( eCharSet );
    }
    if ( eCharSet == RTL_TEXTENCODING_UNICODE )
        rIn.StartReadingUnicodeText();

    bPlain = ( nOption == SC_DIFOPT_PLAIN );

    if( bPlain )
        pNumFormatter = NULL;
    else
        pNumFormatter = rDoc.GetFormatTable();
}


TOPIC DifParser::GetNextTopic( void )
{
    enum STATE { S_VectorVal, S_Data, S_END, S_START, S_UNKNOWN, S_ERROR_L2 };

    static const sal_Unicode pKeyLABEL[]        = { 'L', 'A', 'B', 'E', 'L', 0 };
    static const sal_Unicode pKeyCOMMENT[]      = { 'C', 'O', 'M', 'M', 'E', 'N', 'T', 0 };
    static const sal_Unicode pKeySIZE[]         = { 'S', 'I', 'Z', 'E', 0 };
    static const sal_Unicode pKeyPERIODICITY[]  = { 'P', 'E', 'R', 'I', 'O', 'D', 'I', 'C', 'I', 'T', 'Y', 0 };
    static const sal_Unicode pKeyMAJORSTART[]   = { 'M', 'A', 'J', 'O', 'R', 'S', 'T', 'A', 'R', 'T', 0 };
    static const sal_Unicode pKeyMINORSTART[]   = { 'M', 'I', 'N', 'O', 'R', 'S', 'T', 'A', 'R', 'T', 0 };
    static const sal_Unicode pKeyTRUELENGTH[]   = { 'T', 'R', 'U', 'E', 'L', 'E', 'N', 'G', 'T', 'H', 0 };
    static const sal_Unicode pKeyUINITS[]       = { 'U', 'I', 'N', 'I', 'T', 'S', 0 };
    static const sal_Unicode pKeyDISPLAYUNITS[] = { 'D', 'I', 'S', 'P', 'L', 'A', 'Y', 'U', 'N', 'I', 'T', 'S', 0 };
    static const sal_Unicode pKeyUNKNOWN[]      = { 0 };

    static const sal_Unicode*   ppKeys[] =
    {
        pKeyTABLE,              // 0
        pKeyVECTORS,
        pKeyTUPLES,
        pKeyDATA,
        pKeyLABEL,
        pKeyCOMMENT,            // 5
        pKeySIZE,
        pKeyPERIODICITY,
        pKeyMAJORSTART,
        pKeyMINORSTART,
        pKeyTRUELENGTH,         // 10
        pKeyUINITS,
        pKeyDISPLAYUNITS,
        pKeyUNKNOWN             // 13
    };

    static const TOPIC      pTopics[] =
    {
        T_TABLE,                // 0
        T_VECTORS,
        T_TUPLES,
        T_DATA,
        T_LABEL,
        T_COMMENT,              // 5
        T_SIZE,
        T_PERIODICITY,
        T_MAJORSTART,
        T_MINORSTART,
        T_TRUELENGTH,           // 10
        T_UINITS,
        T_DISPLAYUNITS,
        T_UNKNOWN               // 13
    };

    STATE                   eS = S_START;
    BOOL                    bValOverflow = FALSE;
    String                  aLine;

    nVector = 0;
    nVal = 0;
    TOPIC eRet = T_UNKNOWN;

    while( eS != S_END )
    {
        if( !rIn.ReadUniOrByteStringLine( aLine ) )
        {
            eS = S_END;
            eRet = T_END;
        }

        switch( eS )
        {
            case S_START:
            {
                const sal_Unicode*  pRef;
                UINT16          nCnt = 0;
                BOOL            bSearch = TRUE;

                pRef = ppKeys[ nCnt ];

                while( bSearch )
                {
                    if( aLine == pRef )
                    {
                        eRet = pTopics[ nCnt ];
                        bSearch = FALSE;
                    }
                    else
                    {
                        nCnt++;
                        pRef = ppKeys[ nCnt ];
                        if( !*pRef )
                            bSearch = FALSE;
                    }
                }

                if( *pRef )
                    eS = S_VectorVal;
                else
                    eS = S_UNKNOWN;
            }
                break;
            case S_VectorVal:
            {
                const sal_Unicode*      pCur = aLine.GetBuffer();

                pCur = ScanIntVal( pCur, nVector );

                if( *pCur == ',' )
                {
                    pCur++;
                    ScanIntVal( pCur, nVal );
                    eS = S_Data;
                }
                else
                    eS = S_ERROR_L2;
            }
                break;
            case S_Data:
                DBG_ASSERT( aLine.Len() >= 2,
                    "+GetNextTopic(): <String> ist zu kurz!" );
                if( aLine.Len() > 2 )
                    aData = aLine.Copy( 1, aLine.Len() - 2 );
                else
                    aData.Erase();
                eS = S_END;
                break;
#ifdef DBG_UTIL
            case S_END:
                DBG_ERROR( "+GetNextTopic(): Ende? Gibt's hier nicht!" );
                break;
#endif
            case S_UNKNOWN:
                // 2 Zeilen ueberlesen
                rIn.ReadUniOrByteStringLine( aLine );
            case S_ERROR_L2:                // Fehler in Line 2 aufgetreten
                // eine Zeile ueberlesen
                rIn.ReadUniOrByteStringLine( aLine );
                eS = S_END;
                break;
#ifdef DBG_UTIL
            default:
                DBG_ERROR( "*GetNextTopic((): enum vergessen!" );
#endif
        }
    }

    return eRet;
}


void lcl_DeEscapeQuotesDif( String& rString )
{
    //  Special handling for DIF import: Escaped (duplicated) quotes are resolved.
    //  Single quote characters are left in place because older versions didn't
    //  escape quotes in strings (and Excel doesn't when using the clipboard).
    //  The quotes around the string are removed before this function is called.

    static const sal_Unicode aDQ[] = { '"', '"', 0 };
    xub_StrLen nPos = 0;
    while ( (nPos = rString.Search( aDQ, nPos )) != STRING_NOTFOUND )
    {
        rString.Erase( nPos, 1 );
        ++nPos;
    }
}


DATASET DifParser::GetNextDataset( void )
{
    DATASET             eRet = D_UNKNOWN;
    String              aLine;
    const sal_Unicode*      pAkt;

    rIn.ReadUniOrByteStringLine( aLine );

    pAkt = aLine.GetBuffer();

    switch( *pAkt )
    {
        case '-':                   // Special Datatype
            pAkt++;

            if( Is1_0( pAkt ) )
            {
                rIn.ReadUniOrByteStringLine( aLine );
                if( IsBOT( aLine.GetBuffer() ) )
                    eRet = D_BOT;
                else if( IsEOD( aLine.GetBuffer() ) )
                    eRet = D_EOD;
            }
            break;
        case '0':                   // Numeric Data
            pAkt++;                 // Wert in fVal, 2. Zeile in aData
            if( *pAkt == ',' )
            {
                pAkt++;
                if( bPlain )
                {
                    if( ScanFloatVal( pAkt ) )
                        eRet = D_NUMERIC;
                    else
                        eRet = D_SYNT_ERROR;
                }
                else
                {   // ...und zur Strafe mit'm Numberformatter...
                    DBG_ASSERT( pNumFormatter, "-DifParser::GetNextDataset(): No Formatter, more fun!" );
                    String          aTestVal( pAkt );
                    ULONG           nFormat = 0;
                    double          fTmpVal;
                    if( pNumFormatter->IsNumberFormat( aTestVal, nFormat, fTmpVal ) )
                    {
                        fVal = fTmpVal;
                        nNumFormat = nFormat;
                        eRet = D_NUMERIC;
                    }
                    else
                        eRet = D_SYNT_ERROR;
                }
                rIn.ReadUniOrByteStringLine( aData );
                if ( eRet == D_SYNT_ERROR )
                {   // for broken records write "#ERR: data" to cell
                    String aTmp( RTL_CONSTASCII_USTRINGPARAM( "#ERR: " ));
                    aTmp += pAkt;
                    aTmp.AppendAscii( " (" );
                    aTmp += aData;
                    aTmp += sal_Unicode(')');
                    aData = aTmp;
                    eRet = D_STRING;
                }
            }
            break;
        case '1':                   // String Data
            if( Is1_0( aLine.GetBuffer() ) )
            {
                rIn.ReadUniOrByteStringLine( aLine );
                DBG_ASSERT( aLine.Len() >= 2,
                    "*DifParser::GetNextTopic(): Text ist zu kurz (mind. \"\")!" );
                aData = aLine.Copy( 1, aLine.Len() - 2 );
                lcl_DeEscapeQuotesDif( aData );
                eRet = D_STRING;
            }
            break;
    }

    if( eRet == D_UNKNOWN )
        rIn.ReadUniOrByteStringLine( aLine );

    if( rIn.IsEof() )
        eRet = D_EOD;

    return eRet;
}


const sal_Unicode* DifParser::ScanIntVal( const sal_Unicode* pStart, UINT32& rRet )
{
    sal_Unicode     cAkt = *pStart;

    if( IsNumber( cAkt ) )
        rRet = ( UINT32 ) ( cAkt - '0' );
    else
        return NULL;

    pStart++;
    cAkt = *pStart;

    while( IsNumber( cAkt ) && rRet < ( 0xFFFFFFFF / 10 ) )
    {
        rRet *= 10;
        rRet += ( UINT32 ) ( cAkt - '0' );

        pStart++;
        cAkt = *pStart;
    }

    return pStart;
}


BOOL DifParser::ScanFloatVal( const sal_Unicode* pStart )
    {
    double                  fNewVal = 0.0;
    BOOL                    bNeg = FALSE;
    double                  fFracPos;
    INT32                   nExp = 0;
    BOOL                    bExpNeg = FALSE;
    BOOL                    bExpOverflow = FALSE;
    static const UINT16     nExpLimit = 4096;   // ACHTUNG: muss genauer ermittelt werden!

    sal_Unicode             cAkt;
    BOOL                    bRet = FALSE;

    enum STATE { S_FIRST, S_PRE, S_POST, S_EXP_FIRST, S_EXP, S_END, S_FINDEND };

    STATE   eS = S_FIRST;

    fNewVal = 0.0;

    while( eS != S_END )
    {
        cAkt = *pStart;
        switch( eS )
        {
            case S_FIRST:
                if( IsNumber( cAkt ) )
                {
                    fNewVal *= 10;
                    fNewVal += cAkt - '0';
                    eS = S_PRE;
                }
                else
                {
                    switch( cAkt )
                    {
                        case ' ':
                        case '\t':
                        case '+':
                            break;
                        case '-':
                            bNeg = !bNeg;
                            break;
                        case '.':
                        case ',':                   //!
                            eS = S_POST;
                            fFracPos = 0.1;
                            break;
                        default:
                            eS = S_END;
                    }
                }
                break;
            case S_PRE:
                if( IsNumber( cAkt ) )
                {
                    fNewVal *= 10;
                    fNewVal += cAkt - '0';
                }
                else
                {
                    switch( cAkt )
                    {
                        case '.':
                        case ',':                   //!
                            eS = S_POST;
                            fFracPos = 0.1;
                            break;
                        case 'e':
                        case 'E':
                            eS = S_EXP;
                            break;
                        case 0x00:              // IsNumberEnding( cAkt )
                            bRet = TRUE;        // no
                        default:                // break!
                            eS = S_END;
                    }
                }
                break;
            case S_POST:
                if( IsNumber( cAkt ) )
                {
                    fNewVal += fFracPos * ( cAkt - '0' );
                    fFracPos /= 10.0;
                }
                else
                {
                    switch( cAkt )
                    {
                        case 'e':
                        case 'E':
                            eS = S_EXP_FIRST;
                            break;
                        case 0x00:              // IsNumberEnding( cAkt )
                            bRet = TRUE;        // no
                        default:                // break!
                            eS = S_END;
                    }
                }
                break;
            case S_EXP_FIRST:
                if( IsNumber( cAkt ) )
                {
                    if( nExp < nExpLimit )
                    {
                        nExp *= 10;
                        nExp += ( UINT16 ) ( cAkt - '0' );
                    }
                    eS = S_EXP;
                }
                else
                {
                    switch( cAkt )
                    {
                        case '+':
                            break;
                        case '-':
                            bExpNeg = !bExpNeg;
                            break;
                        default:
                            eS = S_END;
                    }
                }
                break;
            case S_EXP:
                if( IsNumber( cAkt ) )
                {
                    if( nExp < ( 0xFFFF / 10 ) )
                    {
                        nExp *= 10;
                        nExp += ( UINT16 ) ( cAkt - '0' );
                    }
                    else
                    {
                        bExpOverflow = TRUE;
                        eS = S_FINDEND;
                    }
                }
                else
                {
                    bRet = IsNumberEnding( cAkt );
                    eS = S_END;
                }
                break;
            case S_FINDEND:
                if( IsNumberEnding( cAkt ) )
                {
                    bRet = TRUE;        // damit sinnvoll weitergeparst werden kann
                    eS = S_END;
                }
                break;
#ifdef DBG_UTIL
            case S_END:
                DBG_ERROR( "*ScanVal(): S_END erreicht!" );
                break;
            default:
                DBG_ERROR( "*ScanVal(): State vergessen!" );
#endif
        }
        pStart++;
    }

    if( bRet )
    {
        if( bExpOverflow )
            return NULL;        // ACHTUNG: hier muss noch differenziert werden

        if( bNeg )
            fNewVal *= 1.0;

        if( bExpNeg )
            nExp *= -1;

        if( nExp != 0 )
            fNewVal *= pow( 10.0, ( double ) nExp );
        fVal = fNewVal;
    }

    return bRet;
}


DifColumn::~DifColumn( void )
{
    ENTRY*  pAkt = ( ENTRY* ) List::First();

    while( pAkt )
    {
        delete pAkt;
        pAkt = ( ENTRY* ) List::Next();
    }
}


void DifColumn::SetLogical( UINT16 nRow )
{
    DBG_ASSERT( nRow <= MAXROW, "*DifColumn::SetLogical(): Row zu gross!" );

    if( pAkt )
    {
        DBG_ASSERT( nRow > 0, "*DifColumn::SetLogical(): weitere koennen nicht 0 sein!" );
        nRow--;
        if( pAkt->nEnd == nRow )
            pAkt->nEnd++;
        else
            pAkt = NULL;
    }
    else
    {
        pAkt = new ENTRY;
        pAkt->nStart = pAkt->nEnd = nRow;
        List::Insert( pAkt, LIST_APPEND );
    }
}


void DifColumn::SetNumFormat( UINT16 nRow, const UINT32 nNumFormat )
{
    DBG_ASSERT( nRow <= MAXROW, "*DifColumn::SetNumFormat(): Row zu gross!" );

    if( nNumFormat > 0 )
    {
        if( pAkt )
        {
            DBG_ASSERT( nRow > 0,
                "*DifColumn::SetNumFormat(): weitere koennen nicht 0 sein!" );
            DBG_ASSERT( nRow > pAkt->nEnd,
                "*DifColumn::SetNumFormat(): Noch 'mal von vorne?" );

            if( pAkt->nNumFormat == nNumFormat && pAkt->nEnd == nRow - 1 )
                pAkt->nEnd = nRow;
            else
                NewEntry( nRow, nNumFormat );
            }
        else
            NewEntry( nRow, nNumFormat );
        }
    else
        pAkt = NULL;
}


void DifColumn::NewEntry( const UINT16 nPos, const UINT32 nNumFormat )
{
    pAkt = new ENTRY;
    pAkt->nStart = pAkt->nEnd = nPos;
    pAkt->nNumFormat = nNumFormat;
    List::Insert( pAkt, LIST_APPEND );
}


void DifColumn::Apply( ScDocument& rDoc, const UINT16 nCol, const UINT16 nTab, const ScPatternAttr& rPattAttr )
{
    ENTRY*  pAkt = ( ENTRY* ) List::First();

    while( pAkt )
    {
        rDoc.ApplyPatternAreaTab( nCol, pAkt->nStart, nCol, pAkt->nEnd,
                nTab, rPattAttr );
        pAkt = ( ENTRY* ) List::Next();
    }
}


void DifColumn::Apply( ScDocument& rDoc, const UINT16 nCol, const UINT16 nTab )
{
    ScPatternAttr   aAttr( rDoc.GetPool() );
    SfxItemSet&     rItemSet = aAttr.GetItemSet();

    ENTRY*          pAkt = ( ENTRY* ) List::First();

    while( pAkt )
        {
        DBG_ASSERT( pAkt->nNumFormat > 0,
            "+DifColumn::Apply(): Numberformat darf hier nicht 0 sein!" );
        rItemSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, pAkt->nNumFormat ) );

        rDoc.ApplyPatternAreaTab( nCol, pAkt->nStart, nCol, pAkt->nEnd, nTab, aAttr );

        rItemSet.ClearItem();

        pAkt = ( ENTRY* ) List::Next();
    }
}


DifAttrCache::DifAttrCache( const BOOL bNewPlain )
{
    bPlain = bNewPlain;
    ppCols = new DifColumn *[ MAXCOL + 1 ];
    for( UINT16 nCnt = 0 ; nCnt <= MAXCOL ; nCnt++ )
        ppCols[ nCnt ] = NULL;
}


DifAttrCache::~DifAttrCache()
{
    for( UINT16 nCnt = 0 ; nCnt <= MAXCOL ; nCnt++ )
    {
        if( ppCols[ nCnt ] )
            delete ppCols[ nCnt ];
    }
}


void DifAttrCache::SetNumFormat( const UINT16 nCol, const UINT16 nRow, const UINT32 nNumFormat )
{
    DBG_ASSERT( nCol <= MAXCOL, "-DifAttrCache::SetNumFormat(): Col zu gross!" );
    DBG_ASSERT( !bPlain, "*DifAttrCache::SetNumFormat(): sollte nicht Plain sein!" );

    if( !ppCols[ nCol ] )
        ppCols[ nCol ] = new DifColumn;

    ppCols[ nCol ]->SetNumFormat( nRow, nNumFormat );
}


void DifAttrCache::Apply( ScDocument& rDoc, UINT16 nTab )
{
    if( bPlain )
    {
        ScPatternAttr*  pPatt = NULL;

        for( UINT16 nCol = 0 ; nCol <= MAXCOL ; nCol++ )
        {
            if( ppCols[ nCol ] )
            {
                if( !pPatt )
                {
                    pPatt = new ScPatternAttr( rDoc.GetPool() );
                    pPatt->GetItemSet().Put( SfxUInt32Item( ATTR_VALUE_FORMAT,
                        rDoc.GetFormatTable()->GetStandardFormat( NUMBERFORMAT_LOGICAL ) ) );
                }

                ppCols[ nCol ]->Apply( rDoc, nCol, nTab, *pPatt );
            }
        }

        if( pPatt )
            delete pPatt;
    }
    else
    {
        for( UINT16 nCol = 0 ; nCol <= MAXCOL ; nCol++ )
        {
            if( ppCols[ nCol ] )
                ppCols[ nCol ]->Apply( rDoc, nCol, nTab );
        }
    }
}



