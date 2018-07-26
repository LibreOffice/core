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

#include <basiccharclass.hxx>
#include <scanner.hxx>
#include <sbintern.hxx>
#include <runtime.hxx>

#include <i18nlangtag/lang.h>
#include <svl/zforlist.hxx>
#include <vcl/svapp.hxx>

SbiScanner::SbiScanner( const OUString& rBuf, StarBASIC* p ) : aBuf( rBuf )
{
    pBasic   = p;
    nLineIdx = -1;
    nVal     = 0;
    eScanType = SbxVARIANT;
    nErrors  = 0;
    nBufPos  = 0;
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
    nSaveLineIdx = -1;
}

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

void SbiScanner::GenError( ErrCode code )
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
            if ( code.anyOf(
                    ERRCODE_BASIC_EXPECTED,
                    ERRCODE_BASIC_UNEXPECTED,
                    ERRCODE_BASIC_SYMBOL_EXPECTED,
                    ERRCODE_BASIC_LABEL_EXPECTED) )
            {
                    nc = nCol1;
                    if( nc > nCol2 ) nCol2 = nc;
            }
            bRes = pBasic->CError( code, aError, nLine, nc, nCol2 );
        }
        bAbort = bAbort || !bRes  || ( code == ERRCODE_BASIC_NO_MEMORY || code == ERRCODE_BASIC_PROG_TOO_LARGE );
    }
    nErrors++;
}


// used by SbiTokenizer::MayBeLabel() to detect a label
bool SbiScanner::DoesColonFollow()
{
    if(nCol < aLine.getLength() && aLine[nCol] == ':')
    {
        ++nLineIdx; ++nCol;
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
        return SbxINTEGER;
    case '&':
        return SbxLONG;
    case '!':
        return SbxSINGLE;
    case '#':
        return SbxDOUBLE;
    case '@':
        return SbxCURRENCY;
    case '$':
        return SbxSTRING;
    default:
        return SbxVARIANT;
    }
}

// reading the next symbol into the variables aSym, nVal and eType
// return value is sal_False at EOF or errors
#define BUF_SIZE 80

void SbiScanner::scanAlphanumeric()
{
    sal_Int32 n = nCol;
    while(nCol < aLine.getLength() && (BasicCharClass::isAlphaNumeric(aLine[nCol], bCompatible) || aLine[nCol] == '_'))
    {
        ++nLineIdx;
        ++nCol;
    }
    aSym = aLine.copy(n, nCol - n);
}

void SbiScanner::scanGoto()
{
    sal_Int32 n = nCol;
    while(n < aLine.getLength() && BasicCharClass::isWhitespace(aLine[n]))
        ++n;

    if(n + 1 < aLine.getLength())
    {
        OUString aTemp = aLine.copy(n, 2);
        if(aTemp.equalsIgnoreAsciiCase("to"))
        {
            aSym = "goto";
            nLineIdx += n + 2 - nCol;
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
    while(nBufPos < nEnd && BasicCharClass::isWhitespace(aBuf[nEnd - 1]))
        --nEnd;

    aLine = aBuf.copy(nBufPos, nEnd - nBufPos);

    // Fast-forward past the line ending
    if(n + 1 < nLen && aBuf[n] == '\r' && aBuf[n + 1] == '\n')
        n += 2;
    else if(n < nLen)
        ++n;

    nBufPos = n;
    nLineIdx = 0;

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
    aSym.clear();
    bHash = bSymbol = bNumber = bSpaces = false;
    bool bCompilerDirective = false;

    // read in line?
    if (nLineIdx == -1)
    {
        if(!readLine())
            return false;

        nOldLine = nLine;
        nOldCol1 = nOldCol2 = 0;
    }

    const sal_Int32 nLineIdxScanStart = nLineIdx;

    if(nCol < aLine.getLength() && BasicCharClass::isWhitespace(aLine[nCol]))
    {
        bSpaces = true;
        while(nCol < aLine.getLength() && BasicCharClass::isWhitespace(aLine[nCol]))
        {
            ++nLineIdx;
            ++nCol;
        }
    }

    nCol1 = nCol;

    // only blank line?
    if(nCol >= aLine.getLength())
        goto eoln;

    if( bPrevLineExtentsComment )
        goto PrevLineCommentLbl;

    if(nCol < aLine.getLength() && aLine[nCol] == '#')
    {
        sal_Int32 nLineTempIdx = nLineIdx;
        do
        {
            nLineTempIdx++;
        } while (nLineTempIdx < aLine.getLength() && !BasicCharClass::isWhitespace(aLine[nLineTempIdx]) && aLine[nLineTempIdx] != '#');
        // leave it if it is a date literal - it will be handled later
        if (nLineTempIdx >= aLine.getLength() || aLine[nLineTempIdx] != '#')
        {
            ++nLineIdx;
            ++nCol;
            //ignore compiler directives (# is first non-space character)
            if (nOldCol2 == 0)
                bCompilerDirective = true;
            else
                bHash = true;
        }
    }

    // copy character if symbol
    if(nCol < aLine.getLength() && (BasicCharClass::isAlpha(aLine[nCol], bCompatible) || aLine[nCol] == '_'))
    {
        // if there's nothing behind '_' , it's the end of a line!
        if(nCol + 1 == aLine.getLength() && aLine[nCol] == '_')
        {
            // Note that nCol is not incremented here...
            ++nLineIdx;
            goto eoln;
        }

        bSymbol = true;

        scanAlphanumeric();

        // Special handling for "go to"
        if(nCol < aLine.getLength() && bCompatible && aSym.equalsIgnoreAsciiCase("go"))
            scanGoto();

        // replace closing '_' by space when end of line is following
        // (wrong line continuation otherwise)
        if (nCol == aLine.getLength() && aLine[nCol - 1] == '_')
        {
            // We are going to modify a potentially shared string, so force
            // a copy, so that aSym is not modified by the following operation
            OUString aSymCopy( aSym.getStr(), aSym.getLength() );
            aSym = aSymCopy;

            // HACK: modifying a potentially shared string here!
            const_cast<sal_Unicode*>(aLine.getStr())[nLineIdx - 1] = ' ';
        }

        // type recognition?
        // don't test the exclamation mark
        // if there's a symbol behind it
        else if((nCol >= aLine.getLength() || aLine[nCol] != '!') ||
                (nCol + 1 >= aLine.getLength() || !BasicCharClass::isAlpha(aLine[nCol + 1], bCompatible)))
        {
            if(nCol < aLine.getLength())
            {
                SbxDataType t(GetSuffixType(aLine[nCol]));
                if( t != SbxVARIANT )
                {
                    eScanType = t;
                    ++nLineIdx;
                    ++nCol;
                }
            }
        }
    }

    // read in and convert if number
    else if((nCol < aLine.getLength() && rtl::isAsciiDigit(aLine[nCol])) ||
            (nCol + 1 < aLine.getLength() && aLine[nCol] == '.' && rtl::isAsciiDigit(aLine[nCol + 1])))
    {
        short exp = 0;
        short dec = 0;
        eScanType = SbxDOUBLE;
        bool bScanError = false;
        bool bBufOverflow = false;
        // All this because of 'D' or 'd' floating point type, sigh..
        while(!bScanError && nCol < aLine.getLength() && strchr("0123456789.DEde", aLine[nCol]))
        {
            // from 4.1.1996: buffer full? -> go on scanning empty
            if( (p-buf) == (BUF_SIZE-1) )
            {
                bBufOverflow = true;
                ++nLineIdx;
                ++nCol;
                continue;
            }
            // point or exponent?
            if(aLine[nCol] == '.')
            {
                if( ++dec > 1 )
                    bScanError = true;
                else
                    *p++ = '.';
            }
            else if(strchr("DdEe", aLine[nCol]))
            {
                if (++exp > 1)
                    bScanError = true;
                else
                {
                    *p++ = 'E';
                    if (nCol + 1 < aLine.getLength() && (aLine[nCol+1] == '+' || aLine[nCol+1] == '-'))
                    {
                        ++nLineIdx;
                        ++nCol;
                        if( (p-buf) == (BUF_SIZE-1) )
                        {
                            bBufOverflow = true;
                            continue;
                        }
                        *p++ = aLine[nCol];
                    }
                }
            }
            else
            {
                *p++ = aLine[nCol];
            }
            ++nLineIdx;
            ++nCol;
        }
        *p = 0;
        aSym = p; bNumber = true;

        // For bad characters, scan and parse errors generate only one error.
        ErrCode nError = ERRCODE_NONE;
        if (bScanError)
        {
            --nLineIdx;
            --nCol;
            aError = OUString( aLine[nCol]);
            nError = ERRCODE_BASIC_BAD_CHAR_IN_NUMBER;
        }

        rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
        const sal_Unicode* pParseEnd = buf;
        nVal = rtl_math_uStringToDouble( buf, buf+(p-buf), '.', ',', &eStatus, &pParseEnd );
        if (pParseEnd != buf+(p-buf))
        {
            // e.g. "12e" or "12e+", or with bScanError "12d"+"E".
            sal_Int32 nChars = buf+(p-buf) - pParseEnd;
            nLineIdx -= nChars;
            nCol -= nChars;
            // For bScanError, nLineIdx and nCol were already decremented, just
            // add that character to the parse end.
            if (bScanError)
                ++nChars;
            // Copy error position from original string, not the buffer
            // replacement where "12dE" => "12EE".
            aError = aLine.copy( nCol, nChars);
            nError = ERRCODE_BASIC_BAD_CHAR_IN_NUMBER;
        }
        else if (eStatus != rtl_math_ConversionStatus_Ok)
        {
            // Keep the scan error and character at position, if any.
            if (!nError)
                nError = ERRCODE_BASIC_MATH_OVERFLOW;
        }

        if (nError)
            GenError( nError );

        if( !dec && !exp )
        {
            if( nVal >= SbxMININT && nVal <= SbxMAXINT )
                eScanType = SbxINTEGER;
            else if( nVal >= SbxMINLNG && nVal <= SbxMAXLNG )
                    eScanType = SbxLONG;
        }

        if( bBufOverflow )
            GenError( ERRCODE_BASIC_MATH_OVERFLOW );

        // type recognition?
        if( nCol < aLine.getLength() )
        {
            SbxDataType t(GetSuffixType(aLine[nCol]));
            if( t != SbxVARIANT )
            {
                eScanType = t;
                ++nLineIdx;
                ++nCol;
            }
       }
    }

    // Hex/octal number? Read in and convert:
    else if(aLine.getLength() - nCol > 1 && aLine[nCol] == '&')
    {
        ++nLineIdx; ++nCol;
        sal_Unicode base = 16;
        sal_Unicode xch  = aLine[nCol];
        ++nLineIdx; ++nCol;
        switch( rtl::toAsciiUpperCase( xch ) )
        {
            case 'O':
                base = 8;
                break;
            case 'H':
                break;
            default :
                // treated as an operator
                --nLineIdx; --nCol; nCol1 = nCol-1;
                aSym = "&";
                return true;
        }
        bNumber = true;
        // Hex literals are signed Integers ( as defined by basic
        // e.g. -2,147,483,648 through 2,147,483,647 (signed)
        sal_uInt64 lu = 0;
        bool bOverflow = false;
        while(nCol < aLine.getLength() && BasicCharClass::isAlphaNumeric(aLine[nCol], false))
        {
            sal_Unicode ch = rtl::toAsciiUpperCase(aLine[nCol]);
            ++nLineIdx; ++nCol;
            if( ((base == 16 ) && rtl::isAsciiHexDigit( ch ) ) ||
                     ((base == 8) && rtl::isAsciiOctalDigit( ch )))
            {
                int i = ch  - '0';
                if( i > 9 ) i -= 7;
                lu = ( lu * base ) + i;
                if( lu > SAL_MAX_UINT32 )
                {
                    bOverflow = true;
                }
            }
            else
            {
                aError = OUString(ch);
                GenError( ERRCODE_BASIC_BAD_CHAR_IN_NUMBER );
            }
        }
        if(nCol < aLine.getLength() && aLine[nCol] == '&')
        {
            ++nLineIdx;
            ++nCol;
        }
        sal_Int32 ls = static_cast<sal_Int32>(lu);
        nVal = static_cast<double>(ls);
        eScanType = ( ls >= SbxMININT && ls <= SbxMAXINT ) ? SbxINTEGER : SbxLONG;
        if( bOverflow )
            GenError( ERRCODE_BASIC_MATH_OVERFLOW );
    }

    // Strings:
    else if (nLineIdx < aLine.getLength() && (aLine[nLineIdx] == '"' || aLine[nLineIdx] == '['))
    {
        sal_Unicode cSep = aLine[nLineIdx];
        if( cSep == '[' )
        {
            bSymbol = true;
            cSep = ']';
        }
        sal_Int32 n = nCol + 1;
        while (nLineIdx < aLine.getLength())
        {
            do
            {
                nLineIdx++;
                nCol++;
            }
            while (nLineIdx < aLine.getLength() && (aLine[nLineIdx] != cSep));
            if (nLineIdx < aLine.getLength() && aLine[nLineIdx] == cSep)
            {
                nLineIdx++; nCol++;
                if (nLineIdx >= aLine.getLength() || aLine[nLineIdx] != cSep || cSep == ']')
                {
                    // If VBA Interop then doesn't eat the [] chars
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
                        eScanType = SbxSTRING;
                    break;
                }
            }
            else
            {
                aError = OUString(cSep);
                GenError( ERRCODE_BASIC_EXPECTED );
            }
        }
    }

    // Date:
    else if (nLineIdx < aLine.getLength() && aLine[nLineIdx] == '#')
    {
        sal_Int32 n = nCol + 1;
        do
        {
            nLineIdx++;
            nCol++;
        }
        while (nLineIdx < aLine.getLength() && (aLine[nLineIdx] != '#'));
        if (nLineIdx < aLine.getLength() && aLine[nLineIdx] == '#')
        {
            nLineIdx++; nCol++;
            aSym = aLine.copy( n, nCol - n - 1 );

            // parse date literal
            std::shared_ptr<SvNumberFormatter> pFormatter;
            if (GetSbData()->pInst)
            {
                pFormatter = GetSbData()->pInst->GetNumberFormatter();
            }
            else
            {
                sal_uInt32 nDummy;
                pFormatter = SbiInstance::PrepareNumberFormatter( nDummy, nDummy, nDummy );
            }
            sal_uInt32 nIndex = pFormatter->GetStandardIndex( LANGUAGE_ENGLISH_US);
            bool bSuccess = pFormatter->IsNumberFormat(aSym, nIndex, nVal);
            if( bSuccess )
            {
                SvNumFormatType nType_ = pFormatter->GetType(nIndex);
                if( !(nType_ & SvNumFormatType::DATE) )
                    bSuccess = false;
            }

            if (!bSuccess)
                GenError( ERRCODE_BASIC_CONVERSION );

            bNumber = true;
            eScanType = SbxDOUBLE;
        }
        else
        {
            aError = OUString('#');
            GenError( ERRCODE_BASIC_EXPECTED );
        }
    }
    // invalid characters:
    else if (nLineIdx < aLine.getLength() && aLine[nLineIdx] >= 0x7F)
    {
        GenError( ERRCODE_BASIC_SYNTAX ); nLineIdx++; nCol++;
    }
    // other groups:
    else
    {
        sal_Int32 n = 1;
        auto nChar = nLineIdx < aLine.getLength() ? aLine[nLineIdx] : 0;
        ++nLineIdx;
        if (nLineIdx < aLine.getLength())
        {
            switch (nChar)
            {
                case '<': if( aLine[nLineIdx] == '>' || aLine[nLineIdx] == '=' ) n = 2; break;
                case '>': if( aLine[nLineIdx] == '=' ) n = 2; break;
                case ':': if( aLine[nLineIdx] == '=' ) n = 2; break;
            }
        }
        aSym = aLine.copy(nCol, std::min(n, aLine.getLength() - nCol));
        nLineIdx += n-1; nCol = nCol + n;
    }

    nCol2 = nCol-1;

PrevLineCommentLbl:

    if( bPrevLineExtentsComment || (eScanType != SbxSTRING &&
                                    ( bCompilerDirective ||
                                      aSym.startsWith("'") ||
                                      aSym.equalsIgnoreAsciiCase( "REM" ) ) ) )
    {
        bPrevLineExtentsComment = false;
        aSym = "REM";
        sal_Int32 nLen = aLine.getLength() - nLineIdx;
        if( bCompatible && aLine[nLineIdx + nLen - 1] == '_' && aLine[nLineIdx + nLen - 2] == ' ' )
            bPrevLineExtentsComment = true;
        nCol2 = nCol2 + nLen;
        nLineIdx = -1;
    }

    if (nLineIdx == nLineIdxScanStart)
    {
        GenError( ERRCODE_BASIC_SYMBOL_EXPECTED );
        return false;
    }

    return true;


eoln:
    if( nCol && aLine[--nLineIdx] == '_' )
    {
        nLineIdx = -1;
        bool bRes = NextSym();
        if( aSym.startsWith(".") )
        {
            // object _
            //    .Method
            // ^^^  <- spaces is legal in MSO VBA
            bSpaces = false;
        }
        return bRes;
    }
    else
    {
        nLineIdx = -1;
        nLine = nOldLine;
        nCol1 = nOldCol1;
        nCol2 = nOldCol2;
        aSym = "\n";
        nColLock = 0;
        return true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
