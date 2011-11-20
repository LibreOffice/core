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
#include "precompiled_basic.hxx"

#include "basiccharclass.hxx"
#include "sbcomp.hxx"

#include <vcl/svapp.hxx>

SbiScanner::SbiScanner( const ::rtl::OUString& rBuf, StarBASIC* p ) : aBuf( rBuf )
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
    bUsedForHilite =
    bCompatible =
    bVBASupportOn =
    bPrevLineExtentsComment = sal_False;
    bHash    =
    bErrors  = sal_True;
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
    if( !bError && bErrors )
    {
        bool bRes = true;
        // report only one error per statement
        bError = true;
        if( pBasic )
        {
            // in case of EXPECTED or UNEXPECTED it always refers
            // to the last token, so take the Col1 over
            sal_uInt16 nc = nColLock ? nSavedCol1 : nCol1;
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
        bAbort |= !bRes |
             ( code == SbERR_NO_MEMORY || code == SbERR_PROG_TOO_LARGE );
    }
    if( bErrors )
        nErrors++;
}


// used by SbiTokenizer::MayBeLabel() to detect a label
bool SbiScanner::DoesColonFollow()
{
    if( pLine && *pLine == ':' )
    {
        pLine++; nCol++;
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

void SbiScanner::scanGoto()
{
    short nTestCol = nCol;
    while(nTestCol < aLine.getLength() && theBasicCharClass::get().isWhitespace(aLine[nTestCol]))
        nTestCol++;

    if(nTestCol + 1 < aLine.getLength())
    {
        ::rtl::OUString aTestSym = aLine.copy(nTestCol, 2);
        if(aTestSym.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("to")))
        {
            aSym = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("goto"));
            pLine += (nTestCol - nCol) + 2;
            nCol = nTestCol + 2;
        }
    }
}

bool SbiScanner::NextSym()
{
    // memorize for the EOLN-case
    sal_uInt16 nOldLine = nLine;
    sal_uInt16 nOldCol1 = nCol1;
    sal_uInt16 nOldCol2 = nCol2;
    sal_Unicode buf[ BUF_SIZE ], *p = buf;
    bHash = false;

    eScanType = SbxVARIANT;
    aSym = ::rtl::OUString();
    bSymbol =
    bNumber = bSpaces = false;

    // read in line?
    if( !pLine )
    {
        sal_Int32 n = nBufPos;
        sal_Int32 nLen = aBuf.getLength();
        if( nBufPos >= nLen )
            return false;
        const sal_Unicode* p2 = aBuf.getStr();
        p2 += n;
        while( ( n < nLen ) && ( *p2 != '\n' ) && ( *p2 != '\r' ) )
            p2++, n++;
        // #163944# ignore trailing whitespace
        sal_Int32 nCopyEndPos = n;
        while( (nBufPos < nCopyEndPos) && theBasicCharClass::get().isWhitespace( aBuf[ nCopyEndPos - 1 ] ) )
            --nCopyEndPos;
        aLine = aBuf.copy( nBufPos, nCopyEndPos - nBufPos );
        if( n < nLen )
        {
            if( *p2 == '\r' && *( p2+1 ) == '\n' )
                n += 2;
            else
                n++;
        }
        nBufPos = n;
        pLine = aLine.getStr();
        nOldLine = ++nLine;
        nCol = nCol1 = nCol2 = nOldCol1 = nOldCol2 = 0;
        nColLock = 0;
    }


    while( theBasicCharClass::get().isWhitespace( *pLine ) )
        pLine++, nCol++, bSpaces = true;

    nCol1 = nCol;

    // only blank line?
    if( !*pLine )
        goto eoln;

    if( bPrevLineExtentsComment )
        goto PrevLineCommentLbl;

    if( *pLine == '#' )
    {
        pLine++;
        nCol++;
        bHash = true;
    }

    // copy character if symbol
    if( theBasicCharClass::get().isAlpha( *pLine, bCompatible ) || *pLine == '_' )
    {
        // if there's nothing behind '_' , it's the end of a line!
        if( *pLine == '_' && !*(pLine+1) )
        {   pLine++;
            goto eoln;  }
        bSymbol = true;
        short n = nCol;
        for ( ; (theBasicCharClass::get().isAlphaNumeric( *pLine, bCompatible ) || ( *pLine == '_' ) ); pLine++ )
            nCol++;
        aSym = aLine.copy( n, nCol - n );

        // Special handling for "go to"
        if( bCompatible && *pLine && aSym.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM("go") ) )
            scanGoto();

        // replace closing '_' by space when end of line is following
        // (wrong line continuation otherwise)
        if( !bUsedForHilite && !*pLine && *(pLine-1) == '_' )
        {
            // We are going to modify a potentially shared string, so force
            // a copy, so that aSym is not modified by the following operation
            ::rtl::OUString aSymCopy( aSym.getStr(), aSym.getLength() );
            aSym = aSymCopy;

            // HACK: modifying a potentially shared string here!
            *((sal_Unicode*)(pLine-1)) = ' ';
        }
        // type recognition?
        // don't test the exclamation mark
        // if there's a symbol behind it
        else if( *pLine != '!' || !theBasicCharClass::get().isAlpha( pLine[ 1 ], bCompatible ) )
        {
            SbxDataType t = GetSuffixType( *pLine );
            if( t != SbxVARIANT )
            {
                eScanType = t;
                pLine++;
                nCol++;
            }
        }
    }

    // read in and convert if number
    else if( theBasicCharClass::get().isDigit( *pLine & 0xFF )
             || ( *pLine == '.' && theBasicCharClass::get().isDigit( *(pLine+1) & 0xFF ) ) )
    {
        short exp = 0;
        short comma = 0;
        short ndig = 0;
        short ncdig = 0;
        eScanType = SbxDOUBLE;
        bool bBufOverflow = false;
        while( strchr( "0123456789.DEde", *pLine ) && *pLine )
        {
            // from 4.1.1996: buffer full? -> go on scanning empty
            if( (p-buf) == (BUF_SIZE-1) )
            {
                bBufOverflow = true;
                pLine++, nCol++;
                continue;
            }
            // point or exponent?
            if( *pLine == '.' )
            {
                if( ++comma > 1 )
                {
                    pLine++; nCol++; continue;
                }
                else *p++ = *pLine++, nCol++;
            }
            else if( strchr( "DdEe", *pLine ) )
            {
                if (++exp > 1)
                {
                    pLine++; nCol++; continue;
                }
                *p++ = 'E'; pLine++; nCol++;

                if( *pLine == '+' )
                    pLine++, nCol++;
                else
                if( *pLine == '-' )
                    *p++ = *pLine++, nCol++;
            }
            else
            {
                *p++ = *pLine++, nCol++;
                if( comma && !exp ) ncdig++;
            }
            if (!exp) ndig++;
        }
        *p = 0;
        aSym = p; bNumber = true;

        if( comma > 1 || exp > 1 )
        {   aError = '.';
            GenError( SbERR_BAD_CHAR_IN_NUMBER );   }

        // #57844 use localized function
        nVal = rtl_math_uStringToDouble( buf, buf+(p-buf), '.', ',', NULL, NULL );

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
        SbxDataType t = GetSuffixType( *pLine );
        if( t != SbxVARIANT )
        {
            eScanType = t;
            pLine++;
            nCol++;
        }
    }

    // Hex/octal number? Read in and convert:
    else if( *pLine == '&' )
    {
        pLine++; nCol++;
        sal_Unicode cmp1[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F', 0 };
        sal_Unicode cmp2[] = { '0', '1', '2', '3', '4', '5', '6', '7', 0 };
        sal_Unicode *cmp = cmp1;
        sal_Unicode base = 16;
        sal_Unicode ndig = 8;
        sal_Unicode xch  = *pLine++ & 0xFF; nCol++;
        switch( toupper( xch ) )
        {
            case 'O':
                cmp = cmp2; base = 8; ndig = 11; break;
            case 'H':
                break;
            default :
                // treated as an operator
                pLine--; nCol--; nCol1 = nCol-1;
                aSym = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("&"));
                return SYMBOL;
        }
        bNumber = true;
        long l = 0;
        int i;
        bool bBufOverflow = false;
        while( theBasicCharClass::get().isAlphaNumeric( *pLine & 0xFF, bCompatible ) )
        {
            sal_Unicode ch = sal::static_int_cast< sal_Unicode >(
                toupper( *pLine & 0xFF ) );
            pLine++; nCol++;
            // from 4.1.1996: buffer full, go on scanning empty
            if( (p-buf) == (BUF_SIZE-1) )
                bBufOverflow = true;
            else if( String( cmp ).Search( ch ) != STRING_NOTFOUND )
            //else if( strchr( cmp, ch ) )
                *p++ = ch;
            else
            {
                aError = ch;
                GenError( SbERR_BAD_CHAR_IN_NUMBER );
            }
        }
        *p = 0;
        for( p = buf; *p; p++ )
        {
            i = (*p & 0xFF) - '0';
            if( i > 9 ) i -= 7;
            l = ( l * base ) + i;
            if( !ndig-- )
            {
                GenError( SbERR_MATH_OVERFLOW ); break;
            }
        }
        if( *pLine == '&' ) pLine++, nCol++;
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
        short n = nCol+1;
        while( *pLine )
        {
            do pLine++, nCol++;
            while( *pLine && ( *pLine != cSep ) );
            if( *pLine == cSep )
            {
                pLine++; nCol++;
                if( *pLine != cSep || cSep == ']' ) break;
            } else aError = cSep, GenError( SbERR_EXPECTED );
        }
        // If VBA Interop then doen't eat the [] chars
        if ( cSep == ']' && bVBASupportOn )
            aSym = aLine.copy( n - 1, nCol - n  + 1);
        else
            aSym = aLine.copy( n, nCol - n - 1 );
        // get out duplicate string delimiters
        ::rtl::OUStringBuffer aSymBuf;
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
        short n = 1;
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
                                    ( aSym[0] == '\'' || aSym.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM("REM") ) ) ) )
    {
        bPrevLineExtentsComment = false;
        aSym = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REM"));
        sal_uInt16 nLen = String( pLine ).Len();
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
        aSym = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
        nColLock = 0;
        return true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
