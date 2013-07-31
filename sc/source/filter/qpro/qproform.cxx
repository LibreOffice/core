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


#include <rtl/strbuf.hxx>
#include <sal/config.h>
#include <sal/macros.h>
#include "qpro.hxx"

#include "qproform.hxx"
#include "formel.hxx"
#include "compiler.hxx"
#include <tokstack.hxx>
#include "ftools.hxx"

void QProToSc::ReadSRD( ScSingleRefData& rSRD, sal_Int8 nPage, sal_Int8 nCol, sal_uInt16 nRelBit )
{
    sal_uInt16 nTmp = nRelBit & 0x1fff;
    rSRD.InitAddress( ScAddress( nCol, (~nTmp + 1), 0 ) );
    if( nRelBit & 0x4000 )
    {
        rSRD.SetRelCol(nCol);
    }
    else
    {
        rSRD.SetAbsCol(nCol);
    }

    if( nRelBit & 0x2000 )
    {
        SCROW nRelRow = (~nTmp + 1);
        nRelRow = (sal_Int16)(nTmp << 3); // This looks weird... Mistake?
        nRelRow /= 8;
        rSRD.SetRelRow(nRelRow);
    }
    else
    {
        rSRD.SetAbsRow(nTmp);
    }
    if( nRelBit & 0x8000 )
    {
        rSRD.SetRelTab(nPage);
    }
    else
    {
        rSRD.SetAbsTab(nPage);
    }
    if (rSRD.toAbs(aEingPos).Tab() != aEingPos.Tab())
        rSRD.SetFlag3D(true);
}

QProToSc::QProToSc( SvStream& rStream, const ScAddress& rRefPos ) :
    ConverterBase( 128 ),
    maIn( rStream )
{
    aEingPos = rRefPos;
}

void QProToSc::DoFunc( DefTokenId eOc, sal_uInt16 nArgs, const sal_Char* pExtString )
{
    TokenId  eParam[ nBufSize ];
    sal_Int32    nCount;
    TokenId  nPush, nPush1;

    sal_Bool bAddIn = false;
    sal_Bool bNeg = false;

    if( eOc == ocNoName )
    {
        bAddIn = sal_True;
        if( pExtString )
        {
            OStringBuffer s(RTL_CONSTASCII_STRINGPARAM("QPRO_"));
            s.append(pExtString);
            nPush = aPool.Store(eOc, OStringToOUString(s.makeStringAndClear(), maIn.GetStreamCharSet()));
            aPool << nPush;
        }
        else
            aPool << ocNoName;
    }

    if( nArgs < nBufSize )
    {
        for( nCount = 0; nCount < nArgs ; nCount++ )
            aStack >> eParam[ nCount ];
    }
    else
        return;

    switch( eOc )
    {
        case ocIndex:
            nPush = eParam[ 0 ];
            eParam[ 0 ] = eParam[ 1 ];
            eParam[ 1 ] = nPush;
            IncToken( eParam[ 0 ] );
            IncToken( eParam[ 1 ] );
            break;

        case ocIRR:
            nPush = eParam[ 0 ];
            eParam[ 0 ] = eParam[ 1 ];
            eParam[ 1 ] = nPush;
            break;

        case ocGetYear:
            nPush = aPool.Store( 1900.0 );
            aPool << ocOpen;
            break;

        default:
            break;
    }

    if( !bAddIn )
        aPool << eOc;

    aPool << ocOpen;

    if( nArgs> 0 )
    {
        sal_Int16 nLast = nArgs- 1;

        if( eOc == ocZGZ )
            aPool << eParam[ 2 ] << ocSep << eParam[ 1 ] << ocSep << eParam[ 0 ];
        if( eOc == ocZinsZ )
            aPool << eParam[ 3 ] << ocSep << eParam[ 2 ] << ocSep << eParam[ 1 ] << ocSep << eParam[ 0 ];
        else
        {
            sal_Int16 nNull = -1;
            aPool << eParam[ nLast ];
            for( nCount = nLast - 1 ; nCount >= 0 ; nCount-- )
            {
                if( nCount != nNull )
                    aPool << ocSep << eParam[ nCount ];
            }
        }
    }

    if( eOc == ocGetYear )
        aPool << ocClose << ocSub << nPush;
    else if( eOc == ocFixed )
        aPool << ocSep << ocTrue << ocOpen << ocClose;

    aPool << ocClose;
    aPool >> aStack;

    if( bNeg )
    {
        aPool << ocOpen << ocSub << aStack << ocClose;
        aPool >> aStack;
    }
}

void QProToSc::IncToken( TokenId &rParam )
{
    aPool << ocOpen << rParam << mnAddToken;
    rParam = aPool.Store();
}

#define SAFEDEC_OR_RET(nRef, amt, ret) \
do { \
    if (nRef < amt)\
        return ret; \
    nRef-=amt; \
} while(0)

ConvErr QProToSc::Convert( const ScTokenArray*& pArray, sal_uInt16 /*nLen*/, const FORMULA_TYPE /*eFT*/ )
{
    sal_uInt8 nFmla[ nBufSize ], i, nArg, nArgArray[ nBufSize ];
    sal_Int8 nCol, nPage;
    sal_uInt16 nInt, nIntCount = 0, nStringCount = 0, nFloatCount = 0, nDLLCount = 0, nIntArray[ nBufSize ], nArgCount = 0;
    String sStringArray[ nBufSize ];
    sal_uInt16 nDummy, nDLLId, nDLLArray[ nBufSize ];
    sal_uInt16 nNote, nRef, nRelBits;
    TokenId nPush;
    ScComplexRefData aCRD;
    ScSingleRefData aSRD;
    FUNC_TYPE eType;
    DefTokenId eOc;
    double nFloatArray[ nBufSize ], nFloat;
    const sal_Char* pExtString = 0;

    aCRD.InitFlags();
    aSRD.InitFlags();
    maIn >> nRef;

    if( nRef < nBufSize )
    {
        for( i=0; i < nRef; i++)
        {
            maIn >> nFmla[i];

            if( nFmla[ i ] == 0x05 )
            {
                maIn >> nInt;
                nIntArray[ nIntCount ] = nInt;
        SAFEDEC_OR_RET(nRef, 2, ConvErrCount);
                nIntCount++;
            }

            if( nFmla[ i ] == 0x00 )
            {
                maIn >> nFloat;
                nFloatArray[ nFloatCount ] = nFloat;
        SAFEDEC_OR_RET(nRef, 8, ConvErrCount);
                nFloatCount++;
            }

            if( nFmla[ i ] == 0x1a )
            {
                maIn >> nArg >> nDummy >> nDLLId;
                nArgArray[ nArgCount ] = nArg;
                nDLLArray[ nDLLCount ] = nDLLId;
        SAFEDEC_OR_RET(nRef, 5, ConvErrCount);
                nDLLCount++;
                nArgCount++;
            }
            if( nFmla[ i ] == 0x06 )
            {
                String aTmp(::read_zeroTerminated_uInt8s_ToOUString(maIn, maIn.GetStreamCharSet()));
                sStringArray[ nStringCount ] = aTmp;
                nStringCount++;
        SAFEDEC_OR_RET(nRef, aTmp.Len() + 1, ConvErrCount);
            }
        }
    }
    else
        return ConvErrCount;

    i = 0, nIntCount = 0, nFloatCount = 0, nDLLCount = 0, nArgCount = 0, nStringCount =0;

    while( i < nRef && ( nFmla[ i ] != 0x03 ) )
    {
        eType = IndexToType( nFmla[ i ] );
        eOc = IndexToToken( nFmla[ i ] );
        if( eOc == ocNoName )
            pExtString = getString( nFmla[ i ] );

        switch( eType )
        {
            case FT_NotImpl:
                DoFunc( ocNoName, 0, pExtString );
                break;

            case FT_FuncFix0:
                DoFunc( eOc, 0, NULL );
                break;

            case FT_FuncFix1:
                DoFunc( eOc, 1, NULL );
                break;

            case FT_FuncFix2:
                DoFunc( eOc, 2, NULL );
                break;

            case FT_FuncFix3:
                DoFunc( eOc, 3, NULL );
                break;

            case FT_FuncFix4:
                DoFunc( eOc, 4, NULL );
                break;

            case FT_FuncFix5:
                DoFunc( eOc, 5, NULL );
                break;

            case FT_FuncFix6:
                DoFunc( eOc, 6, NULL );
                break;

            case FT_DLL:{
                eOc = IndexToDLLId( nDLLArray[ nDLLCount ] );
                sal_uInt8 nPar = nArgArray[ nArgCount ];
                DoFunc( eOc, nPar, NULL );
                nDLLCount++;
                nArgCount++;
                }
                break;

            case FT_Cref : // Single cell reference
                maIn >> nNote >> nCol >> nPage >> nRelBits;
                ReadSRD( aSRD, nPage, nCol, nRelBits );
                aStack << aPool.Store( aSRD );
                break;

            case FT_Range: // Block reference
                maIn >> nNote >> nCol >> nPage >> nRelBits;
                ReadSRD( aCRD.Ref1, nPage, nCol, nRelBits );
                maIn >> nCol >> nPage >> nRelBits;
                ReadSRD( aCRD.Ref2, nPage, nCol, nRelBits );
                // Sheet name of second corner is not displayed if identical
                if (aCRD.Ref1.IsFlag3D() && aCRD.Ref1.Tab() == aCRD.Ref2.Tab() &&
                        aCRD.Ref1.IsTabRel() == aCRD.Ref2.IsTabRel())
                    aCRD.Ref2.SetFlag3D( false);
                aStack << aPool.Store( aCRD );
                break;

            case FT_FuncVar:{ // Sum of a sequence of numbers
                sal_uInt8 nArgs;
                i++;
                nArgs = nFmla[ i ];
                DoFunc( eOc, nArgs, NULL );
                }
                break;

            case FT_Op: // operators
                aStack >> nPush;
                aPool << aStack << eOc << nPush;
                aPool >> aStack;
                break;

            case FT_Braces:
                aPool << ocOpen << aStack << ocClose;
                aPool >> aStack;
                break;

            case FT_ConstInt:{
                sal_uInt16 nVal;
                nVal = nIntArray[ nIntCount ];
                aStack << aPool.Store( ( double ) nVal );
                nIntCount++;
                }
                break;

            case FT_ConstFloat:{
                double nVal;
                nVal = nFloatArray[ nFloatCount ];
                aStack << aPool.Store( nVal );
                nFloatCount++;
                }
                break;

            case FT_ConstString:{
                String aLabel;
                aLabel = sStringArray[ nStringCount ];
                aStack << aPool.Store( aLabel );
                nStringCount++;
                }
                break;

            case FT_Neg:
                aPool <<  ocNegSub << aStack;
                aPool >> aStack;
                break;

            case FT_NOP:    // indicates invalid opcode.
            case FT_Return: // indicates end of formula
                break;
        }
        i++;
    }
    pArray = aPool[ aStack.Get() ];
    return ConvOK;
}

static const struct
{
    DefTokenId nToken;
    FUNC_TYPE   nType;
} aFuncMap[] = {
    { ocPush, FT_ConstFloat },
    { ocPush, FT_Cref },
    { ocPush, FT_Range },
    { ocPush, FT_Return },
    { ocPush, FT_Braces },
    { ocPush, FT_ConstInt },
    { ocPush, FT_ConstString },
    { ocPush, FT_NOP },
    { ocNegSub, FT_Neg },                       // 0x08
    { ocAdd, FT_Op },
    { ocSub, FT_Op },
    { ocMul, FT_Op },
    { ocDiv, FT_Op },
    { ocPow, FT_Op },
    { ocEqual, FT_Op },
    { ocNotEqual, FT_Op },
    { ocLessEqual, FT_Op },                     // 0x10
    { ocGreaterEqual, FT_Op },
    { ocLess, FT_Op },
    { ocGreater, FT_Op },
    { ocAnd, FT_Op },
    { ocOr, FT_Op },
    { ocNot, FT_FuncFix1 },
    { ocPush, FT_NOP },     // Unary plus
    { ocAddress, FT_FuncFix4 }, // Address of
    { ocNoName, FT_NotImpl }, // Halt function
    { ocNoName, FT_DLL }, // DLL function
    { ocNoName, FT_NOP }, // Extended operands
    { ocNoName, FT_NOP }, // Extended operands
    { ocNoName, FT_NOP }, // Reserved
    { ocNoName, FT_NOP }, // Reserved
    { ocNotAvail, FT_FuncFix0 }, // NA
    { ocNoName, FT_FuncFix0 }, // Error         // 0x20
    { ocAbs, FT_FuncFix1 },
    { ocInt, FT_FuncFix1 },
    { ocSqrt, FT_FuncFix1 },
    { ocLog10, FT_FuncFix1 },
    { ocLn, FT_FuncFix1 },
    { ocPi, FT_FuncFix0 },
    { ocSin, FT_FuncFix1 },
    { ocCos, FT_FuncFix1 },
    { ocTan, FT_FuncFix1 },
    { ocArcTan2, FT_FuncFix2 },
    { ocArcTan, FT_FuncFix1 },
    { ocArcSin, FT_FuncFix1 },
    { ocArcCos, FT_FuncFix1 },
    { ocExp, FT_FuncFix1 },
    { ocMod, FT_FuncFix2 },
    { ocChose, FT_FuncVar },                    // 0x30
    { ocIsNA, FT_FuncFix1 },
    { ocIsError, FT_FuncFix1 },
    { ocFalse, FT_FuncFix0 },
    { ocTrue, FT_FuncFix0 },
    { ocRandom, FT_FuncFix0 },
    { ocGetDate, FT_FuncFix3 },
    { ocGetActTime, FT_FuncFix0 },
    { ocNoName, FT_NotImpl },    // QPro Pmt
    { ocNoName, FT_NotImpl },    // QPro Pv
    { ocNoName, FT_NotImpl },    // QPro Fv
    { ocIf, FT_FuncFix3 },
    { ocGetDay, FT_FuncFix1 },
    { ocGetMonth, FT_FuncFix1 },
    { ocGetYear, FT_FuncFix1 },
    { ocRound, FT_FuncFix2 },
    { ocGetTime, FT_FuncFix3 },                  // 0x40
    { ocGetHour, FT_FuncFix1 },
    { ocGetMin, FT_FuncFix1 },
    { ocGetSec, FT_FuncFix1 },
    { ocIsValue, FT_FuncFix1 },
    { ocIsString, FT_FuncFix1 },
    { ocLen, FT_FuncFix1 },
    { ocValue, FT_FuncFix1 },
    { ocFixed, FT_FuncFix2 },
    { ocMid, FT_FuncFix3 },
    { ocChar, FT_FuncFix1 },
    { ocCode, FT_FuncFix1 },
    { ocFind, FT_FuncFix3 },
    { ocGetDateValue, FT_FuncFix1 },
    { ocGetTimeValue, FT_FuncFix1 },
    { ocNoName, FT_NotImpl },
    { ocSum, FT_FuncVar },                     // 0x50
    { ocAverage, FT_FuncVar },
    { ocCount, FT_FuncVar },
    { ocMin, FT_FuncVar },
    { ocMax, FT_FuncVar },
    { ocVLookup, FT_FuncFix3 },
    { ocNPV, FT_FuncFix2 },
    { ocVar, FT_FuncVar },
    { ocNormDist, FT_FuncVar },
    { ocIRR, FT_FuncFix2 },
    { ocHLookup, FT_FuncFix3 },
    { ocDBSum, FT_FuncFix3 },
    { ocDBAverage, FT_FuncFix3 },
    { ocDBCount, FT_FuncFix3 },
    { ocDBMin, FT_FuncFix3 },
    { ocDBMax, FT_FuncFix3 },
    { ocDBVar, FT_FuncFix3 },                 // 0x60
    { ocDBStdDev, FT_FuncFix3 },
    { ocNoName, FT_NotImpl },
    { ocColumns, FT_FuncFix1 },
    { ocRows, FT_FuncFix1 },
    { ocRept, FT_FuncFix2 },
    { ocUpper, FT_FuncFix1 },
    { ocLower, FT_FuncFix1 },
    { ocLeft, FT_FuncFix2 },
    { ocRight, FT_FuncFix2 },
    { ocReplace, FT_FuncFix4 },
    { ocPropper, FT_FuncFix1 },
    { ocCell, FT_FuncFix2 },
    { ocTrim, FT_FuncFix1 },
    { ocClean, FT_FuncFix1 },
    { ocNoName, FT_NotImpl },
    { ocNoName, FT_NotImpl },               // 0x70
    { ocExact, FT_FuncFix2 },
    { ocNoName, FT_NotImpl }, // Call()
    { ocIndirect, FT_FuncFix1 },
    { ocZGZ, FT_FuncFix3 }, // Interest
    { ocNoName, FT_NotImpl },
    { ocNoName, FT_NotImpl },
    { ocLIA, FT_FuncFix3 },
    { ocDIA, FT_FuncFix4 },
    { ocGDA, FT_FuncFix4 },
    { ocStDevP, FT_FuncVar },
    { ocVarP, FT_FuncVar },
    { ocDBStdDevP, FT_FuncVar },
    { ocDBVarP, FT_FuncVar },
    { ocBW, FT_FuncFix3 },  // QPro Pval
    { ocRMZ, FT_FuncFix5 }, // QPro Paymt
    { ocZW, FT_FuncFix3 },  // QPro Fval   // 0x80
    { ocZZR, FT_FuncFix5 },
    { ocZins, FT_FuncFix5 },
    { ocZinsZ, FT_FuncFix4 },
    { ocKapz, FT_FuncFix6 },
    { ocSumProduct, FT_FuncFix2 },
    { ocNoName, FT_NotImpl },
    { ocNoName, FT_NotImpl },
    { ocNoName, FT_NotImpl },
    { ocNoName, FT_NotImpl },
    { ocDeg, FT_FuncFix1 },
    { ocRad, FT_FuncFix1 },
    { ocNoName, FT_NotImpl },
    { ocNoName, FT_NotImpl },
    { ocGetActDate, FT_FuncFix0 },
    { ocNPV, FT_FuncFix2 },
    { ocNoName, FT_NotImpl },                // 0x90
    { ocNoName, FT_NotImpl },
    { ocNoName, FT_NOP },
    { ocNoName, FT_NOP }, // 147
    { ocNoName, FT_NOP }, // 148
    { ocNoName, FT_NOP }, // 149
    { ocNoName, FT_NOP }, // 150
    { ocNoName, FT_NOP }, // 151
    { ocNoName, FT_NOP }, // 152
    { ocNoName, FT_NOP }, // 153
    { ocTable, FT_FuncFix1 },
    { ocNoName, FT_NOP }, // 155 - opcodes do not represent any function.
    { ocNoName, FT_NOP }, // 156
    { ocIndex,  FT_FuncFix4 },
    { ocNoName, FT_NotImpl },
    { ocNoName, FT_NotImpl }, // Gives the property of the particular object
    { ocNoName, FT_NotImpl },  // Dynamic Data Exchange Link // 0x100
    { ocNoName, FT_NotImpl }   // gives properties of DOS menus
};

const int nIndexCount = sizeof( aFuncMap ) / sizeof( aFuncMap[ 0 ] );

DefTokenId QProToSc::IndexToToken( sal_uInt16 nIndex )
{
    if( nIndex < nIndexCount )
        return aFuncMap[ nIndex ].nToken;
    return ocNoName;
}

FUNC_TYPE QProToSc::IndexToType( sal_uInt8 nIndex )
{
    if( nIndex < nIndexCount )
        return aFuncMap[ nIndex ].nType;
    return FT_NotImpl;
}

DefTokenId QProToSc::IndexToDLLId( sal_uInt16 nIndex )
{
    DefTokenId eId;
    switch( nIndex )
    {
        case 0x0001:
            eId = ocAveDev;
            break;

        case 0x0024:
            eId = ocGCD;
            break;

        case 0x0025:
            eId = ocLCM;
            break;

        case 0x0027:
            eId = ocCeil;
            break;

        case 0x0028:
            eId = ocEven;
            break;

        case 0x0022:
            eId = ocFact;
            break;

        case 0x002a:
            eId = ocFloor;
            break;

        case 0x002d:
            eId = ocOdd;
            break;

        case 0x0006:
            eId = ocBetaDist;
            break;

        case 0x0008:
            eId = ocBetaInv;
            break;

        case 0x0010:
            eId = ocCovar;
            break;

        case 0x000b:
            eId = ocChiInv;
            break;

        case 0x003d:
            eId = ocLaufz;
            break;

        case 0x0019:
            eId = ocFInv;
            break;

        case 0x001a:
            eId = ocFisher;
            break;

        case 0x001b:
            eId = ocFisherInv;
            break;

        case 0x0030:
            eId = ocMedian;
            break;

        default:
            eId = ocNoName;
            break;
    }
    return eId;
}

const sal_Char* QProToSc::getString( sal_uInt8 nIndex )
{
    const sal_Char* pExtString = 0;
    switch( nIndex )
    {
        case 57:
            pExtString = "Pv";
            break;

        case 58:
            pExtString = "Fv";
            break;

        case 98:
            pExtString = "Index2D";
            break;

        case 111:
            pExtString = "S";
            break;

        case 112:
            pExtString = "N";
            break;

        case 114:
            pExtString = "CALL";
            break;

        case 117:
            pExtString = "TERM";
            break;

        case 118:
            pExtString = "CTERM";
            break;

        case 134:
            pExtString = "MEMAVAIL";
            break;

        case 135:
            pExtString = "MEMEMSAVAIL";
            break;

        case 136:
            pExtString = "FILEEXISTS";
            break;

        case 137:
            pExtString = "CURVALUE";
            break;

        case 140:
            pExtString = "HEX";
            break;

        case 141:
            pExtString = "NUM";
            break;

        case 145:
            pExtString = "VERSION";
            break;

        case 157:
            pExtString = "INDEX3D";
            break;

        case 158:
            pExtString = "CELLINDEX3D";
            break;

        case 159:
            pExtString = "PROPERTY";
            break;

        case 160:
            pExtString = "DDE";
            break;

        case 161:
            pExtString = "COMMAND";
            break;

        default:
            pExtString = NULL;
            break;
    }
    return pExtString;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
