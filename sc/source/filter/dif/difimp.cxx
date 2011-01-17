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
#include "precompiled_sc.hxx"

//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svl/zforlist.hxx>

#include "dif.hxx"
#include "filter.hxx"
#include "fprogressbar.hxx"
#include "scerrors.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "attrib.hxx"
#include "ftools.hxx"

#include <math.h>

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


FltError ScFormatFilterPluginImpl::ScImportDif( SvStream& rIn, ScDocument* pDoc, const ScAddress& rInsPos,
                        const CharSet eVon, sal_uInt32 nDifOption )
{
    DifParser   aDifParser( rIn, nDifOption, *pDoc, eVon );

    const sal_Bool  bPlain = aDifParser.IsPlain();

    SCTAB       nBaseTab = rInsPos.Tab();

    TOPIC       eTopic = T_UNKNOWN;
    sal_Bool        bSyntErrWarn = sal_False;
    sal_Bool        bOverflowWarn = sal_False;

    String&     rData = aDifParser.aData;
    sal_Bool        bData = sal_False;

    SCCOL       nNumCols = 0;
    SCROW       nNumRows = 0;

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
                    bSyntErrWarn = sal_True;
                if( bData )
                    pDoc->RenameTab( nBaseTab, rData );
            }
                break;
            case T_VECTORS:
            {
                if( aDifParser.nVector != 0 )
                    bSyntErrWarn = sal_True;
                if( aDifParser.nVal > MAXCOL + 1 )
                    nNumCols = SCCOL_MAX;
                else
                    nNumCols = static_cast<SCCOL>(aDifParser.nVal);
            }
                break;
            case T_TUPLES:
            {
                if( aDifParser.nVector != 0 )
                    bSyntErrWarn = sal_True;
                if( aDifParser.nVal > MAXROW + 1 )
                    nNumRows = SCROW_MAX;
                else
                    nNumRows = static_cast<SCROW>(aDifParser.nVal);
            }
                break;
            case T_DATA:
            {
                if( aDifParser.nVector != 0 || aDifParser.nVal != 0 )
                    bSyntErrWarn = sal_True;
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
            default:
                DBG_ERRORFILE( "ScImportDif - missing enum" );
        }

    }


    if( eTopic == T_DATA )
    {   // Ab hier kommen die Daten
        SCCOL               nBaseCol = rInsPos.Col();

        SCCOL               nColCnt = SCCOL_MAX;
        SCROW               nRowCnt = rInsPos.Row();
        DifAttrCache        aAttrCache( bPlain );

        DATASET             eAkt = D_UNKNOWN;

        while( eAkt != D_EOD )
            {
            eAkt = aDifParser.GetNextDataset();

            aPrgrsBar.Progress();

            switch( eAkt )
                {
                case D_BOT:
                    if( nColCnt < SCCOL_MAX )
                        nRowCnt++;
                    nColCnt = nBaseCol;
                    break;
                case D_EOD:
                    break;
                case D_NUMERIC:                 // Numbercell
                    if( nColCnt == SCCOL_MAX )
                        nColCnt = nBaseCol;

                    if( ValidCol(nColCnt) && ValidRow(nRowCnt) )
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

                        pDoc->PutCell( nColCnt, nRowCnt, nBaseTab, pCell, ( sal_Bool ) sal_True );
                    }
                    else
                        bOverflowWarn = sal_True;

                    nColCnt++;
                    break;
                case D_STRING:                  // Textcell
                    if( nColCnt == SCCOL_MAX )
                        nColCnt = nBaseCol;

                    if( ValidCol(nColCnt) && ValidRow(nRowCnt) )
                    {
                        if( rData.Len() > 0 )
                        {
                            pDoc->PutCell( nColCnt, nRowCnt, nBaseTab,
                                ScBaseCell::CreateTextCell( rData, pDoc ), ( sal_Bool ) sal_True );
                        }
                    }
                    else
                        bOverflowWarn = sal_True;

                    nColCnt++;
                    break;
                case D_UNKNOWN:
                    break;
                case D_SYNT_ERROR:
                    break;
                default:
                    DBG_ERROR( "ScImportDif - missing enum" );
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


DifParser::DifParser( SvStream& rNewIn, const sal_uInt32 nOption, ScDocument& rDoc, CharSet e ) :
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
    String                  aLine;

    nVector = 0;
    nVal = 0;
    TOPIC eRet = T_UNKNOWN;

    while( eS != S_END )
    {
        if( !ReadNextLine( aLine ) )
        {
            eS = S_END;
            eRet = T_END;
        }

        switch( eS )
        {
            case S_START:
            {
                const sal_Unicode*  pRef;
                sal_uInt16          nCnt = 0;
                sal_Bool            bSearch = sal_True;

                pRef = ppKeys[ nCnt ];

                while( bSearch )
                {
                    if( aLine == pRef )
                    {
                        eRet = pTopics[ nCnt ];
                        bSearch = sal_False;
                    }
                    else
                    {
                        nCnt++;
                        pRef = ppKeys[ nCnt ];
                        if( !*pRef )
                            bSearch = sal_False;
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

                if( pCur && *pCur == ',' )
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
            case S_END:
                DBG_ERRORFILE( "DifParser::GetNextTopic - unexpected state" );
                break;
            case S_UNKNOWN:
                // 2 Zeilen ueberlesen
                ReadNextLine( aLine );
            case S_ERROR_L2:                // Fehler in Line 2 aufgetreten
                // eine Zeile ueberlesen
                ReadNextLine( aLine );
                eS = S_END;
                break;
            default:
                DBG_ERRORFILE( "DifParser::GetNextTopic - missing enum" );
        }
    }

    return eRet;
}


static void lcl_DeEscapeQuotesDif( String& rString )
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

// Determine if passed in string is numeric data and set fVal/nNumFormat if so
DATASET DifParser::GetNumberDataset( const sal_Unicode* pPossibleNumericData )
{
    DATASET eRet = D_SYNT_ERROR;
    if( bPlain )
    {
        if( ScanFloatVal( pPossibleNumericData ) )
            eRet = D_NUMERIC;
        else
            eRet = D_SYNT_ERROR;
    }
    else
    {   // ...und zur Strafe mit'm Numberformatter...
        DBG_ASSERT( pNumFormatter, "-DifParser::GetNextDataset(): No Formatter, more fun!" );
        String aTestVal( pPossibleNumericData );
        sal_uInt32 nFormat = 0;
        double fTmpVal;
        if( pNumFormatter->IsNumberFormat( aTestVal, nFormat, fTmpVal ) )
        {
            fVal = fTmpVal;
            nNumFormat = nFormat;
            eRet = D_NUMERIC;
        }
        else
            eRet = D_SYNT_ERROR;
    }
    return eRet;
}

bool DifParser::ReadNextLine( String& rStr )
{
    if( aLookAheadLine.Len() == 0 )
    {
        return rIn.ReadUniOrByteStringLine( rStr );
    }
    else
    {
        rStr = aLookAheadLine;
        aLookAheadLine.Erase();
        return true;
    }
}

// Look ahead in the stream to determine if the next line is the first line of
// a valid data record structure
bool DifParser::LookAhead()
{
    const sal_Unicode* pAktBuffer;
    bool bValidStructure = false;

    DBG_ASSERT( aLookAheadLine.Len() == 0, "*DifParser::LookAhead(): LookAhead called twice in a row" );
    rIn.ReadUniOrByteStringLine( aLookAheadLine );

    pAktBuffer = aLookAheadLine.GetBuffer();

    switch( *pAktBuffer )
    {
        case '-':                   // Special Datatype
            pAktBuffer++;

            if( Is1_0( pAktBuffer ) )
            {
                bValidStructure = true;
            }
            break;
        case '0':                   // Numeric Data
            pAktBuffer++;
            if( *pAktBuffer == ',' )
            {
                pAktBuffer++;
                bValidStructure = ( GetNumberDataset(pAktBuffer) != D_SYNT_ERROR );
            }
            break;
        case '1':                   // String Data
            if( Is1_0( aLookAheadLine.GetBuffer() ) )
            {
                bValidStructure = true;
            }
            break;
    }
    return bValidStructure;
}

DATASET DifParser::GetNextDataset( void )
{
    DATASET             eRet = D_UNKNOWN;
    String              aLine;
    const sal_Unicode*      pAktBuffer;

    ReadNextLine( aLine );

    pAktBuffer = aLine.GetBuffer();

    switch( *pAktBuffer )
    {
        case '-':                   // Special Datatype
            pAktBuffer++;

            if( Is1_0( pAktBuffer ) )
            {
                ReadNextLine( aLine );
                if( IsBOT( aLine.GetBuffer() ) )
                    eRet = D_BOT;
                else if( IsEOD( aLine.GetBuffer() ) )
                    eRet = D_EOD;
            }
            break;
        case '0':                   // Numeric Data
            pAktBuffer++;           // Wert in fVal, 2. Zeile in aData
            if( *pAktBuffer == ',' )
            {
                pAktBuffer++;
                eRet = GetNumberDataset(pAktBuffer);
                ReadNextLine( aData );
                if ( eRet == D_SYNT_ERROR )
                {   // for broken records write "#ERR: data" to cell
                    String aTmp( RTL_CONSTASCII_USTRINGPARAM( "#ERR: " ));
                    aTmp += pAktBuffer;
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
                ReadNextLine( aLine );
                xub_StrLen nLineLength = aLine.Len();
                const sal_Unicode* pLine = aLine.GetBuffer();

                if( nLineLength >= 1 && *pLine == '"' )
                {
                    // Quotes are not always escaped (duplicated), see lcl_DeEscapeQuotesDif
                    // A look ahead into the next line is needed in order to deal with
                    // multiline strings containing quotes
                    if( LookAhead() )
                    {
                        // Single line string
                        if( nLineLength >= 2 && pLine[nLineLength - 1] == '"' )
                        {
                            aData = aLine.Copy( 1, nLineLength - 2 );
                            lcl_DeEscapeQuotesDif( aData );
                            eRet = D_STRING;
                        }
                    }
                    else
                    {
                        // Multiline string
                        aData = aLine.Copy( 1 );
                        bool bContinue = true;
                        while ( bContinue )
                        {
                            aData.Append( '\n' );
                            bContinue = !rIn.IsEof() && ReadNextLine( aLine );
                            if( bContinue )
                            {
                                nLineLength = aLine.Len();
                                if( nLineLength >= 1 )
                                {
                                    pLine = aLine.GetBuffer();
                                    bContinue = !LookAhead();
                                    if( bContinue )
                                    {
                                        aData.Append( aLine );
                                    }
                                    else if( pLine[nLineLength - 1] == '"' )
                                    {
                                        aData.Append( pLine, nLineLength - 1 );
                                        lcl_DeEscapeQuotesDif( aData );
                                        eRet = D_STRING;
                                    }
                                }
                            }
                        };
                    }
                }
            }
            break;
    }

    if( eRet == D_UNKNOWN )
        ReadNextLine( aLine );

    if( rIn.IsEof() )
        eRet = D_EOD;

    return eRet;
}


const sal_Unicode* DifParser::ScanIntVal( const sal_Unicode* pStart, sal_uInt32& rRet )
{
    // eat leading whitespace, not specified, but seen in the wild
    while (*pStart == ' ' || *pStart == '\t')
        ++pStart;

    sal_Unicode     cAkt = *pStart;

    if( IsNumber( cAkt ) )
        rRet = ( sal_uInt32 ) ( cAkt - '0' );
    else
        return NULL;

    pStart++;
    cAkt = *pStart;

    while( IsNumber( cAkt ) && rRet < ( 0xFFFFFFFF / 10 ) )
    {
        rRet *= 10;
        rRet += ( sal_uInt32 ) ( cAkt - '0' );

        pStart++;
        cAkt = *pStart;
    }

    return pStart;
}


sal_Bool DifParser::ScanFloatVal( const sal_Unicode* pStart )
    {
    double                  fNewVal = 0.0;
    sal_Bool                    bNeg = sal_False;
    double                  fFracPos = 1.0;
    sal_Int32                   nExp = 0;
    sal_Bool                    bExpNeg = sal_False;
    sal_Bool                    bExpOverflow = sal_False;
    static const sal_uInt16     nExpLimit = 4096;   // ACHTUNG: muss genauer ermittelt werden!

    sal_Unicode             cAkt;
    sal_Bool                    bRet = sal_False;

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
                            bRet = sal_True;        // no
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
                            bRet = sal_True;        // no
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
                        nExp += ( sal_uInt16 ) ( cAkt - '0' );
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
                        nExp += ( sal_uInt16 ) ( cAkt - '0' );
                    }
                    else
                    {
                        bExpOverflow = sal_True;
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
                    bRet = sal_True;        // damit sinnvoll weitergeparst werden kann
                    eS = S_END;
                }
                break;
            case S_END:
                DBG_ERRORFILE( "DifParser::ScanFloatVal - unexpected state" );
                break;
            default:
                DBG_ERRORFILE( "DifParser::ScanFloatVal - missing enum" );
        }
        pStart++;
    }

    if( bRet )
    {
        if( bExpOverflow )
            return sal_False;       // ACHTUNG: hier muss noch differenziert werden

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
    ENTRY*  pEntry = ( ENTRY* ) List::First();

    while( pEntry )
    {
        delete pEntry;
        pEntry = ( ENTRY* ) List::Next();
    }
}


void DifColumn::SetLogical( SCROW nRow )
{
    DBG_ASSERT( ValidRow(nRow), "*DifColumn::SetLogical(): Row zu gross!" );

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


void DifColumn::SetNumFormat( SCROW nRow, const sal_uInt32 nNumFormat )
{
    DBG_ASSERT( ValidRow(nRow), "*DifColumn::SetNumFormat(): Row zu gross!" );

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


void DifColumn::NewEntry( const SCROW nPos, const sal_uInt32 nNumFormat )
{
    pAkt = new ENTRY;
    pAkt->nStart = pAkt->nEnd = nPos;
    pAkt->nNumFormat = nNumFormat;
    List::Insert( pAkt, LIST_APPEND );
}


void DifColumn::Apply( ScDocument& rDoc, const SCCOL nCol, const SCTAB nTab, const ScPatternAttr& rPattAttr )
{
    ENTRY*  pEntry = ( ENTRY* ) List::First();

    while( pEntry )
    {
        rDoc.ApplyPatternAreaTab( nCol, pEntry->nStart, nCol, pEntry->nEnd,
                nTab, rPattAttr );
        pEntry = ( ENTRY* ) List::Next();
    }
}


void DifColumn::Apply( ScDocument& rDoc, const SCCOL nCol, const SCTAB nTab )
{
    ScPatternAttr   aAttr( rDoc.GetPool() );
    SfxItemSet&     rItemSet = aAttr.GetItemSet();

    ENTRY*          pEntry = ( ENTRY* ) List::First();

    while( pEntry )
        {
        DBG_ASSERT( pEntry->nNumFormat > 0,
            "+DifColumn::Apply(): Numberformat darf hier nicht 0 sein!" );
        rItemSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, pEntry->nNumFormat ) );

        rDoc.ApplyPatternAreaTab( nCol, pEntry->nStart, nCol, pEntry->nEnd, nTab, aAttr );

        rItemSet.ClearItem();

        pEntry = ( ENTRY* ) List::Next();
    }
}


DifAttrCache::DifAttrCache( const sal_Bool bNewPlain )
{
    bPlain = bNewPlain;
    ppCols = new DifColumn *[ MAXCOL + 1 ];
    for( SCCOL nCnt = 0 ; nCnt <= MAXCOL ; nCnt++ )
        ppCols[ nCnt ] = NULL;
}


DifAttrCache::~DifAttrCache()
{
    for( SCCOL nCnt = 0 ; nCnt <= MAXCOL ; nCnt++ )
    {
        if( ppCols[ nCnt ] )
            delete ppCols[ nCnt ];
    }
}


void DifAttrCache::SetNumFormat( const SCCOL nCol, const SCROW nRow, const sal_uInt32 nNumFormat )
{
    DBG_ASSERT( ValidCol(nCol), "-DifAttrCache::SetNumFormat(): Col zu gross!" );
    DBG_ASSERT( !bPlain, "*DifAttrCache::SetNumFormat(): sollte nicht Plain sein!" );

    if( !ppCols[ nCol ] )
        ppCols[ nCol ] = new DifColumn;

    ppCols[ nCol ]->SetNumFormat( nRow, nNumFormat );
}


void DifAttrCache::Apply( ScDocument& rDoc, SCTAB nTab )
{
    if( bPlain )
    {
        ScPatternAttr*  pPatt = NULL;

        for( SCCOL nCol = 0 ; nCol <= MAXCOL ; nCol++ )
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
        for( SCCOL nCol = 0 ; nCol <= MAXCOL ; nCol++ )
        {
            if( ppCols[ nCol ] )
                ppCols[ nCol ]->Apply( rDoc, nCol, nTab );
        }
    }
}



