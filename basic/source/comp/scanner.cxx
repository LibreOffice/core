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

#include "basiccharclass.hxx"
#include "sbcomp.hxx"

#include <vcl/svapp.hxx>

SbiScanner::SbiScanner( const OUString& rBuf, StarBASIC* p ) : aBuf( rBuf )
{
    pBasic   = p;
    pLine    = NULL;
    nVal     = 0;
    eScanType = SbxVARIANT;
    nErrors  = 0;
    nBufPos  = 0;
    nCurCol1 = 0;
    nSavedCol1 = 0;
    nColLock = 0;
    nLine    = 0;
    nCol1    = 0;
    nCol2    = 0;
    nCol     = 0;
    bError   =
    bAbort   =
    bSpaces  =
    bNumber  =
    bSymbol  =
    bCompatible =
    bVBASupportOn =
    bInStatement =
    bPrevLineExtentsComment = false;
    bHash    = true;
}

SbiScanner::~SbiScanner()
{}

void SbiScanner::LockColumn()
{
    if( !nColLock++ )
        nSavedCol1 = nCol1;
}

void SbiScanner::UnlockColumn()
{
    if( nColLock )
        nColLock--;
}

void SbiScanner::GenError( SbError code )
{
    if( GetSbData()->bBlockCompilerError )
    {
        bAbort = true;
        return;
    }
    if( !bError )
    {
        bool bRes = true;
        // report only one error per statement
        bError = true;
        if( pBasic )
        {
            // in case of EXPECTED or UNEXPECTED it always refers
            // to the last token, so take the Col1 over
            sal_Int32 nc = nColLock ? nSavedCol1 : nCol1;
            switch( code )
            {
                case SbERR_EXPECTED:
                case SbERR_UNEXPECTED:
                case SbERR_SYMBOL_EXPECTED:
                case SbERR_LABEL_EXPECTED:
                    nc = nCol1;
                    if( nc > nCol2 ) nCol2 = nc;
                    break;
            }
            bRes = pBasic->CError( code, aError, nLine, nc, nCol2 );
        }
        bAbort = bAbort || !bRes  || ( code == SbERR_NO_MEMORY || code == SbERR_PROG_TOO_LARGE );
    }
    nErrors++;
}


// used by SbiTokenizer::MayBeLabel() to detect a label
bool SbiScanner::DoesColonFollow()
{
    if(nCol < aLine.getLength() && aLine[nCol] == ':')
    {
        ++pLine; ++nCol;
        return true;
    }
    else
        return false;
}

// test for legal suffix
static SbxDataType GetSuffixType( sal_Unicode c )
{
    switch (c)
    {
    case '%':
        return SbxDataType(SbxINTEGER);
    case '&':
        return SbxDataType(SbxLONG);
    case '!':
        return SbxDataType(SbxSINGLE);
    case '#':
        return SbxDataType(SbxDOUBLE);
    case '@':
        return SbxDataType(SbxCURRENCY);
    case '$':
        return SbxDataType(SbxSTRING);
    default:
        return SbxDataType(SbxVARIANT);
    }
}

// reading the next symbol into the variables aSym, nVal and eType
// return value is sal_False at EOF or errors
#define BUF_SIZE 80

void SbiScanner::scanAlphanumeric()
{
    sal_Int32 n = nCol;
    while(nCol < aLine.getLength() && (theBasicCharClass::get().isAlphaNumeric(aLine[nCol], bCompatible) || aLine[nCol] == '_'))
    {
        ++pLine;
        ++nCol;
    }
    aSym = aLine.copy(n, nCol - n);
}

void SbiScanner::scanGoto()
{
    sal_Int32 n = nCol;
    while(n < aLine.getLength() && theBasicCharClass::get().isWhitespace(aLine[n]))
        ++n;

    if(n + 1 < aLine.getLength())
    {
        OUString aTemp = aLine.copy(n, 2);
        if(aTemp.equalsIgnoreAsciiCase("to"))
        {
            aSym = OUString("goto");
            pLine += n + 2 - nCol;
            nCol = n + 2;
        }
    }
}

bool SbiScanner::readLine()
{
    if(nBufPos >= aBuf.getLength())
        return false;

    sal_Int32 n = nBufPos;
    sal_Int32 nLen = aBuf.getLength();

    while(n < nLen && aBuf[n] != '\r' && aBuf[n] != '\n')
        ++n;

    // Trim trailing whitespace
    sal_Int32 nEnd = n;
    while(nBufPos < nEnd && theBasicCharClass::get().isWhitespace(aBuf[nEnd - 1]))
        --nEnd;

    aLine = aBuf.copy(nBufPos, nEnd - nBufPos);

    // Fast-forward past the line ending
    if(n + 1 < nLen && aBuf[n] == '\r' && aBuf[n + 1] == '\n')
        n += 2;
    else if(n < nLen)
        ++n;

    nBufPos = n;
    pLine = aLine.getStr();

    ++nLine;
    nCol = nCol1 = nCol2 = 0;
    nColLock = 0;

    return true;
}

bool SbiScanner::NextSym()
{
    // memorize for the EOLN-case
    sal_Int32 nOldLine = nLine;
    sal_Int32 nOldCol1 = nCol1;
    sal_Int32 nOldCol2 = nCol2;
    sal_Unicode buf[ BUF_SIZE ], *p = buf;

    eScanType = SbxVARIANT;
    aSym = OUString();
    bHash = bSymbol = bNumber = bSpaces = false;

    // read in line?
    if( !pLine )
    {
        if(!readLine())
            return false;

        nOldLine = nLine;
        nOldCol1 = nOldCol2 = 0;
    }

    if(nCol < aLine.getLength() && theBasicCharClass::get().isWhitespace(aLine[nCol]))
    {
        bSpaces = true;
        while(nCol < aLine.getLength() && theBasicCharClass::get().isWhitespace(aLine[nCol]))
            ++pLine, ++nCol;
    }

    nCol1 = nCol;

    // only blank line?
    if(nCol >= aLine.getLength())
        goto eoln;

    if( bPrevLineExtentsComment )
        goto PrevLineCommentLbl;

    if(nCol < aLine.getLength() && aLine[nCol] == '#')
    {
        ++pLine;
        ++nCol;
        bHash = true;
    }

    // copy character if symbol
    if(nCol < aLine.getLength() && (theBasicCharClass::get().isAlpha(aLine[nCol], bCompatible) || aLine[nCol] == '_'))
    {
        // if there's nothing behind '_' , it's the end of a line!
        if(nCol + 1 == aLine.getLength() && aLine[nCol] == '_')
        {
            // Note that nCol is not incremented here...
            ++pLine;
            goto eoln;
        }

        bSymbol = true;

        scanAlphanumeric();

        // Special handling for "go to"
        if(nCol < aLine.getLength() && bCompatible && aSym.equalsIgnoreAsciiCase("go"))
            scanGoto();

        // replace closing '_' by space when end of line is following
        // (wrong line continuation otherwise)
        if(nCol == aLine.getLength() && aLine[nCol - 1] == '_' )
        {
            // We are going to modify a potentially shared string, so force
            // a copy, so that aSym is not modified by the following operation
            OUString aSymCopy( aSym.getStr(), aSym.getLength() );
            aSym = aSymCopy;

            // HACK: modifying a potentially shared string here!
            *((sal_Unicode*)(pLine-1)) = ' ';
        }

        // type recognition?
        // don't test the exclamation mark
        // if there's a symbol behind it
        else if((nCol >= aLine.getLength() || aLine[nCol] != '!') ||
                (nCol + 1 >= aLine.getLength() || !theBasicCharClass::get().isAlpha(aLine[nCol + 1], bCompatible)))
        {
            if(nCol < aLine.getLength())
            {
                SbxDataType t(GetSuffixType(aLine[nCol]));
                if( t != SbxVARIANT )
                {
                    eScanType = t;
                    ++pLine;
                    ++nCol;
                }
            }
        }
    }

    // read in and convert if number
    else if((nCol < aLine.getLength() && theBasicCharClass::get().isDigit(aLine[nCol] & 0xFF)) ||
            (nCol + 1 < aLine.getLength() && aLine[nCol] == '.' && theBasicCharClass::get().isDigit(aLine[nCol + 1] & 0xFF)))
    {
        short exp = 0;
        short comma = 0;
        short ndig = 0;
        short ncdig = 0;
        eScanType = SbxDOUBLE;
        bool bBufOverflow = false;
        while(nCol < aLine.getLength() && strchr("0123456789.DEde", aLine[nCol]))
        {
            // from 4.1.1996: buffer full? -> go on scanning empty
            if( (p-buf) == (BUF_SIZE-1) )
            {
                bBufOverflow = true;
                ++pLine, ++nCol;
                continue;
            }
            // point or exponent?
            if(aLine[nCol] == '.')
            {
                if( ++comma > 1 )
                {
                    ++pLine; ++nCol; continue;
                }
                else
                {
                    *p = '.';
                    ++p, ++pLine, ++nCol;
                }
            }
            else if(strchr("DdEe", aLine[nCol]))
            {
                if (++exp > 1)
                {
                    ++pLine; ++nCol; continue;
                }

                *p = 'E';
                ++p, ++pLine, ++nCol;

                if(aLine[nCol] == '+')
                    ++pLine, ++nCol;
                else if(aLine[nCol] == '-')
                {
                    *p = '-';
                    ++p, ++pLine, ++nCol;
                }
            }
            else
            {
                *p = aLine[nCol];
                ++p, ++pLine, ++nCol;
                if( comma && !exp ) ++ncdig;
            }
            if (!exp) ++ndig;
        }
        *p = 0;
        aSym = p; bNumber = true;

        if( comma > 1 || exp > 1 )
        {   aError = OUString('.');
            GenError( SbERR_BAD_CHAR_IN_NUMBER );   }

        rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
        const sal_Unicode* pParseEnd = buf;
        nVal = rtl_math_uStringToDouble( buf, buf+(p-buf), '.', ',', &eStatus, &pParseEnd );
        if (eStatus != rtl_math_ConversionStatus_Ok || pParseEnd != buf+(p-buf))
            GenError( SbERR_MATH_OVERFLOW );

        ndig = ndig - comma;
        if( !comma && !exp )
        {
            if( nVal >= SbxMININT && nVal <= SbxMAXINT )
                eScanType = SbxINTEGER;
            else
            if( nVal >= SbxMINLNG && nVal <= SbxMAXLNG )
                eScanType = SbxLONG;
        }
        if( bBufOverflow )
            GenError( SbERR_MATH_OVERFLOW );

        // type recognition?
        SbxDataType t(GetSuffixType(aLine[nCol]));
        if( t != SbxVARIANT )
        {
            eScanType = t;
            ++pLine;
            ++nCol;
        }
    }

    // Hex/octal number? Read in and convert:
    else if(nCol < aLine.getLength() && aLine[nCol] == '&')
    {
        ++pLine; ++nCol;
        sal_Unicode cmp1[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F', 0 };
        sal_Unicode cmp2[] = { '0', '1', '2', '3', '4', '5', '6', '7', 0 };
        sal_Unicode *cmp = cmp1;
        sal_Unicode base = 16;
        sal_Unicode ndig = 8;
        sal_Unicode xch  = aLine[nCol] & 0xFF;
        ++pLine; ++nCol;
        switch( toupper( xch ) )
        {
            case 'O':
                cmp = cmp2; base = 8; ndig = 11; break;
            case 'H':
                break;
            default :
                // treated as an operator
                --pLine; --nCol; nCol1 = nCol-1;
                aSym = OUString("&");
                return true;
        }
        bNumber = true;
        // Hex literals are signed Integers ( as defined by basic
        // e.g. -2,147,483,648 through 2,147,483,647 (signed)
        sal_Int32 l = 0;
        int i;
        bool bBufOverflow = false;
        while(nCol < aLine.getLength() &&  theBasicCharClass::get().isAlphaNumeric(aLine[nCol] & 0xFF, bCompatible))
        {
            sal_Unicode ch = sal::static_int_cast< sal_Unicode >(
                toupper(aLine[nCol] & 0xFF));
            ++pLine; ++nCol;
            // from 4.1.1996: buffer full, go on scanning empty
            if( (p-buf) == (BUF_SIZE-1) )
                bBufOverflow = true;
            else if( OUString( cmp ).indexOf( ch ) != -1 )
                *p++ = ch;
            else
            {
                aError = OUString(ch);
                GenError( SbERR_BAD_CHAR_IN_NUMBER );
            }
        }
        *p = 0;
        for( p = buf; *p; ++p )
        {
            i = (*p & 0xFF) - '0';
            if( i > 9 ) i -= 7;
            l = ( l * base ) + i;
            if( !ndig-- )
            {
                GenError( SbERR_MATH_OVERFLOW ); break;
            }
        }
        if(nCol < aLine.getLength() && aLine[nCol] == '&') ++pLine, ++nCol;
        nVal = (double) l;
        eScanType = ( l >= SbxMININT && l <= SbxMAXINT ) ? SbxINTEGER : SbxLONG;
        if( bBufOverflow )
            GenError( SbERR_MATH_OVERFLOW );
    }

    // Strings:
    else if( *pLine == '"' || *pLine == '[' )
    {
        sal_Unicode cSep = *pLine;
        if( cSep == '[' )
            bSymbol = true, cSep = ']';
        sal_Int32 n = nCol + 1;
        while( *pLine )
        {
            do pLine++, nCol++;
            while( *pLine && ( *pLine != cSep ) );
            if( *pLine == cSep )
            {
                pLine++; nCol++;
                if( *pLine != cSep || cSep == ']' ) break;
            } else aError = OUString(cSep), GenError( SbERR_EXPECTED );
        }
        // If VBA Interop then doen't eat the [] chars
        if ( cSep == ']' && bVBASupportOn )
            aSym = aLine.copy( n - 1, nCol - n  + 1);
        else
            aSym = aLine.copy( n, nCol - n - 1 );
        // get out duplicate string delimiters
        OUStringBuffer aSymBuf;
        for ( sal_Int32 i = 0, len = aSym.getLength(); i < len; ++i )
        {
            aSymBuf.append( aSym[i] );
            if ( aSym[i] == cSep && ( i+1 < len ) && aSym[i+1] == cSep )
                ++i;
        }
        aSym = aSymBuf.makeStringAndClear();
        if( cSep != ']' )
            eScanType = ( cSep == '#' ) ? SbxDATE : SbxSTRING;
    }
    // invalid characters:
    else if( ( *pLine & 0xFF ) >= 0x7F )
    {
        GenError( SbERR_SYNTAX ); pLine++; nCol++;
    }
    // other groups:
    else
    {
        sal_Int32 n = 1;
        switch( *pLine++ )
        {
            case '<': if( *pLine == '>' || *pLine == '=' ) n = 2; break;
            case '>': if( *pLine == '=' ) n = 2; break;
            case ':': if( *pLine == '=' ) n = 2; break;
        }
        aSym = aLine.copy( nCol, n );
        pLine += n-1; nCol = nCol + n;
    }

    nCol2 = nCol-1;

PrevLineCommentLbl:

    if( bPrevLineExtentsComment || (eScanType != SbxSTRING &&
                                    ( aSym[0] == '\'' || aSym.equalsIgnoreAsciiCase( "REM" ) ) ) )
    {
        bPrevLineExtentsComment = false;
        aSym = OUString("REM");
        sal_Int32 nLen = rtl_ustr_getLength(pLine);
        if( bCompatible && pLine[ nLen - 1 ] == '_' && pLine[ nLen - 2 ] == ' ' )
            bPrevLineExtentsComment = true;
        nCol2 = nCol2 + nLen;
        pLine = NULL;
    }
    return true;


eoln:
    if( nCol && *--pLine == '_' )
    {
        pLine = NULL;
        bool bRes = NextSym();
        if( bVBASupportOn && aSym[0] == '.' )
        {
            // object _
            //    .Method
            // ^^^  <- spaces is legal in MSO VBA
            OSL_TRACE("*** resetting bSpaces***");
            bSpaces = false;
        }
        return bRes;
    }
    else
    {
        pLine = NULL;
        nLine = nOldLine;
        nCol1 = nOldCol1;
        nCol2 = nOldCol2;
        aSym = OUString("\n");
        nColLock = 0;
        return true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
