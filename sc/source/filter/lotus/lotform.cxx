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

#include "decl.h"
#include "lotform.hxx"
#include "compiler.hxx"
#include "lotrange.hxx"
#include "namebuff.hxx"
#include "root.hxx"
#include "ftools.hxx"
#include "tool.h"

#include <math.h>
#include <comphelper/string.hxx>

extern WKTYP                eTyp;

static const sal_Char*      GetAddInName( const sal_uInt8 nIndex );

static DefTokenId           lcl_KnownAddIn(const OString& rTest);

//extern double decipher_Number123( sal_uInt32 nValue );


void LotusToSc::DoFunc( DefTokenId eOc, sal_uInt8 nAnz, const sal_Char* pExtString )
{
    TokenId                     eParam[ 256 ];
    sal_Int32                       nLauf;
    TokenId                     nMerk0, nMerk1;

    sal_Bool                        bAddIn = false;
    sal_Bool                        bNeg = false;

    OSL_ENSURE( nAnz < 128, "-LotusToSc::DoFunc(): Neee! -so viel kann ich nicht!" );

    if( eOc == ocNoName )
    {
        OString t;
        if( pExtString )
        {
            const OString s(RTL_CONSTASCII_STRINGPARAM("@<<@123>>"));

            t = pExtString;

            sal_Int32 n = t.indexOf(s);
            if( n != -1 )
                t = t.copy(n + s.getLength());

            t = comphelper::string::stripEnd(t, '(');

            eOc = lcl_KnownAddIn( t );

            if( eOc == ocNoName )
                t = OString(RTL_CONSTASCII_STRINGPARAM("L123_")) + t;
        }
        else
            t = "#UNKNOWN FUNC NAME#";

        if( eOc == ocNoName )
        {
            bAddIn = sal_True;
            nMerk0 = aPool.Store(eOc, OStringToOUString(t, eSrcChar));

            aPool << nMerk0;
        }
    }

    for( nLauf = 0 ; nLauf < nAnz ; nLauf++ )
        aStack >> eParam[ nLauf ];

    // Spezialfaelle...
    switch( eOc )
    {
        case ocIndex:
            OSL_ENSURE( nAnz > 2, "+LotusToSc::DoFunc(): ocIndex braucht mind. 2 Parameter!" );
            nMerk0 = eParam[ 0 ];
            eParam[ 0 ] = eParam[ 1 ];
            eParam[ 1 ] = nMerk0;
            IncToken( eParam[ 0 ] );
            IncToken( eParam[ 1 ] );
            break;
        case ocIRR:
        {
            OSL_ENSURE( nAnz == 2, "+LotusToSc::DoFunc(): ocIRR hat nur 2 Parameter!" );
            nMerk0 = eParam[ 0 ];
            eParam[ 0 ] = eParam[ 1 ];
            eParam[ 1 ] = nMerk0;
        }
            break;
        case ocGetYear:
        {
            nMerk0 = aPool.Store( 1900.0 );
            aPool << ocOpen;
        }
            break;
        case ocChose:
        {// 1. Parameter ++
            if (nAnz >= 1)
                IncToken( eParam[ nAnz - 1 ] );
        }
            break;
        case ocFind:
        case ocHLookup:
        case ocVLookup:
        {// letzten Parameter ++
            IncToken( eParam[ 0 ] );
        }
            break;
        case ocMid:
        case ocReplace:
        {// 2. Parameter ++
            if (nAnz >= 2)
                IncToken( eParam[ nAnz - 2 ] );
        }
            break;
        case ocZins:
        {
            // neue Anzahl = 4!
            OSL_ENSURE( nAnz == 3,
                "*LotusToSc::DoFunc(): ZINS() hat 3 Parameter!" );
            nAnz = 4;
            eParam[ 3 ] = eParam[ 0 ];  // 3. -> 1.
            eParam[ 0 ] = eParam[ 2 ];  // 1. -> 4.
            NegToken( eParam[ 1 ] );    // 2. -> -2. (+ 2. -> 3.)
            eParam[ 2 ] = n0Token;      //    -> 2. als Default
        }
            break;
        default:;
    }
    // ................


    if( !bAddIn )
        aPool << eOc;

    aPool << ocOpen;

    if( nAnz > 0 )
    {
            // ACHTUNG: 0 ist der letzte Parameter, nAnz-1 der erste

        sal_Int16 nLast = nAnz - 1;

        if( eOc == ocRMZ )
        {   // Extrawurst ocRMZ letzter Parameter negiert!
            // zusaetzlich: 1. -> 3., 3. -> 2., 2. -> 1.
            OSL_ENSURE( nAnz == 3,
                "+LotusToSc::DoFunc(): ocRMZ hat genau 3 Parameter!" );
            aPool << eParam[ 1 ] << ocSep << eParam[ 0 ] << ocSep
                << ocNegSub << eParam[ 2 ];
        }
        else
        {   // Normalfall
            // [Parameter{;Parameter}]
            aPool << eParam[ nLast ];

            sal_Int16 nNull = -1;   // gibt einen auszulassenden Parameter an
            for( nLauf = nLast - 1 ; nLauf >= 0 ; nLauf-- )
            {
                if( nLauf != nNull )
                    aPool << ocSep << eParam[ nLauf ];
            }
        }
    }


    // Spezialfaelle...
    if( eOc == ocGetYear )
    {
        aPool << ocClose << ocSub << nMerk0;
    }
    else if( eOc == ocFixed )
    {
        aPool << ocSep << ocTrue << ocOpen << ocClose;
    }
    else if( eOc == ocFind )
    {
        nMerk1 = aPool.Store();
        DecToken( nMerk1 );
        aPool << nMerk1;
    }

    aPool << ocClose;

    // ................

    aPool >> aStack;

    if( bNeg )
    {
        aPool << ocOpen << ocSub << aStack << ocClose;
        aPool >> aStack;
    }
}


void LotusToSc::LotusRelToScRel( sal_uInt16 nCol, sal_uInt16 nRow, ScSingleRefData& rSRD )
{
    // Col-Bemachung
    if( nCol & 0x8000 )
    {
        if( nCol & 0x0080 )
            nCol |= 0xFF00;
        else
            nCol &= 0x00FF;
        // #i36252# first cast unsigned 16-bit to signed 16-bit, and then to SCCOL
        rSRD.SetRelCol(static_cast<SCCOL>(static_cast<sal_Int16>(nCol)));
    }
    else
    {
        rSRD.SetAbsCol(static_cast<SCCOL>(nCol & 0x00FF));
    }

    // Row-Bemachung
    if( nRow & 0x8000 )
    {
        rSRD.SetRowRel(true);
        // vorzeichenrichtige Erweiterung
        switch( eTyp )
        {
            // 5432 1098 7654 3210
            // 8421 8421 8421 8421
            //       xxx xxxx xxxx
            case eWK_1:
                if( nRow & 0x0400 )
                    nRow |= 0xF800;
                else
                    nRow &= 0x07FF;
                break;
            // 8421 8421 8421 8421
            //    x xxxx xxxx xxxx
            case eWK_2:
                if( nRow & 0x1000 )
                    nRow |= 0xE000;
                else
                    nRow &= 0x1FFF;
                break;
            default:
                OSL_FAIL( "*LotusToSc::LotusRelToScRel(): etwas vergessen...?" );
        }
    }
    else
    {
        rSRD.SetRowRel(false);
        switch( eTyp )
        {
            // 5432 1098 7654 3210
            // 8421 8421 8421 8421
            //       xxx xxxx xxxx
            case eWK_1:
                nRow &= 0x07FF;
                break;
            // 8421 8421 8421 8421
            //   xx xxxx xxxx xxxx
            case eWK_2:
                nRow &= 0x3FFF;
                break;
            default:
                OSL_FAIL( "*LotusToSc::LotusRelToScRel(): etwas vergessen...?" );
        }
    }

    if( rSRD.IsRowRel() )
        // #i36252# first cast unsigned 16-bit to signed 16-bit, and then to SCROW
        rSRD.SetRelRow(static_cast<SCROW>(static_cast<sal_Int16>(nRow)));
    else
        rSRD.SetAbsRow(static_cast<SCROW>(nRow));
}


void LotusToSc::ReadSRD( ScSingleRefData& rSRD, sal_uInt8 nRelBit )
{
    sal_uInt8           nTab, nCol;
    sal_uInt16          nRow;

    Read( nRow );
    Read( nTab );
    Read( nCol );

    bool b3D = (static_cast<SCTAB>(nTab) != aEingPos.Tab());

    rSRD.SetColRel( ( nRelBit & 0x01 ) != 0 );
    rSRD.SetRowRel( ( nRelBit & 0x02 ) != 0 );
    rSRD.SetTabRel( ( ( nRelBit & 0x04) != 0 ) || !b3D );
    rSRD.SetFlag3D( b3D );

    rSRD.SetAddress(ScAddress(nCol, nRow, nTab), aEingPos);
}


void LotusToSc::IncToken( TokenId &rParam )
{
    aPool << ocOpen << rParam << nAddToken;
    rParam = aPool.Store();
}


void LotusToSc::DecToken( TokenId &rParam )
{
    aPool << ocOpen << rParam << nSubToken;
    rParam = aPool.Store();
}


void LotusToSc::NegToken( TokenId &rParam )
{
    aPool << ocNegSub << ocOpen << rParam << ocClose;
    rParam = aPool.Store();
}


void LotusToSc::Reset( const ScAddress& rEingPos )
{
    LotusConverterBase::Reset( rEingPos );

    TokenId nEins = aPool.Store( 1.0 );

    aPool << ocClose << ocAdd << nEins;
    nAddToken = aPool.Store();

    aPool << ocClose << ocSub << nEins;
    nSubToken = aPool.Store();

    n0Token = aPool.Store( 0.0 );
}


LotusToSc::LotusToSc( SvStream &rStream, CharSet e, sal_Bool b ) :
    LotusConverterBase( rStream, 128 )
{
    eSrcChar = e;
    bWK3 = false;
    bWK123 = b;
}


typedef FUNC_TYPE ( FuncType1 ) ( sal_uInt8 );
typedef DefTokenId ( FuncType2 ) ( sal_uInt8 );


ConvErr LotusToSc::Convert( const ScTokenArray*& rpErg, sal_Int32& rRest,
    const FORMULA_TYPE /*eFT*/ )
{
    sal_uInt8               nOc;
    sal_uInt8               nAnz;
    sal_uInt8               nRelBits;
    sal_uInt16              nStrLen;
    sal_uInt16              nRngIndex;
    FUNC_TYPE           eType = FT_NOP;
    TokenId             nMerk0;
    DefTokenId          eOc;
    const sal_Char*     pExtName = 0;
    RangeNameBufferWK3& rRangeNameBufferWK3 = *pLotusRoot->pRngNmBffWK3;

    ScComplexRefData        aCRD;
    aCRD.InitFlags();
    ScSingleRefData&        rR = aCRD.Ref1;

    LR_ID               nId;
    TokenId             nNewId;

    LotusRangeList&     rRangeList = *pLotusRoot->pRangeNames;

    FuncType1*          pIndexToType;
    FuncType2*          pIndexToToken;

    if( bWK3 )
    {   // for > WK3
        pIndexToType = IndexToTypeWK123;
        pIndexToToken = IndexToTokenWK123;
    }
    else if( bWK123 )
    {
        pIndexToType = IndexToTypeWK123;
        pIndexToToken = IndexToTokenWK123;
    }
    else
    {
        pIndexToType = IndexToType;
        pIndexToToken = IndexToToken;

        rR.SetRelTab(0);
        rR.SetFlag3D( false );
    }

    aCRD.Ref2 = rR;

    nBytesLeft = rRest;

    while( eType )      // != FT_Return (==0)
    {
        Read( nOc );

        if( nBytesLeft < 0 )
        {
            rpErg = aPool[ aStack.Get() ];
            return ConvErrCount;
        }

        eType = ( pIndexToType )( nOc );
        eOc = ( pIndexToToken)( nOc );
        if( eOc == ocNoName )
            pExtName = GetAddInName( nOc );

        switch( eType )
        {
            case FT_Return:
                if( bWK3 || bWK123 )
                    nBytesLeft = 0; // wird ab WK3 nicht benutzt

                rRest = nBytesLeft;
                break;
            case FT_NotImpl:
            case FT_FuncFix0:   DoFunc( eOc, 0, pExtName ); break;
            case FT_FuncFix1:   DoFunc( eOc, 1, pExtName ); break;
            case FT_FuncFix2:   DoFunc( eOc, 2, pExtName ); break;
            case FT_FuncFix3:   DoFunc( eOc, 3, pExtName ); break;
            case FT_FuncFix4:   DoFunc( eOc, 4, pExtName ); break;
                case FT_FuncVar:
                Read( nAnz );
                DoFunc( eOc, nAnz, pExtName );
                break;
            case FT_Neg:
                aPool << ocOpen << ocNegSub << aStack << ocClose;
                aPool >> aStack;
                break;
            case FT_Op:
                aStack >> nMerk0;
                aPool << aStack << eOc << nMerk0;
                aPool >> aStack;
                break;
            case FT_ConstFloat:
            {
                double  fDouble;
                Read( fDouble );
                aStack << aPool.Store( fDouble );
            }
                break;
            case FT_Variable:
            {
                sal_uInt16  nCol, nRow;
                Read( nCol );
                Read( nRow );

                LotusRelToScRel( nCol, nRow, rR );

                if( bWK3 )
                    nNewId = aPool.Store( rR );
                else
                {
                    nId = rRangeList.GetIndex(rR.Col(), rR.Row());

                    if( nId == ID_FAIL )
                        // kein Range dazu
                        nNewId = aPool.Store( rR );
                    else
                        nNewId = aPool.Store( ( sal_uInt16 ) nId );
                }

                aStack << nNewId;
            }
                break;
            case FT_Range:
            {
                sal_uInt16  nColS, nRowS, nColE, nRowE;
                Read( nColS );
                Read( nRowS );
                Read( nColE );
                Read( nRowE );

                LotusRelToScRel( nColS, nRowS, rR );
                LotusRelToScRel( nColE, nRowE, aCRD.Ref2 );

                if( bWK3 )
                    nNewId = aPool.Store( aCRD );
                else
                {
                    nId = rRangeList.GetIndex(rR.Col(), rR.Row(), aCRD.Ref2.Col(), aCRD.Ref2.Row());

                    if( nId == ID_FAIL )
                        // kein Range dazu
                        nNewId = aPool.Store( aCRD );
                    else
                        nNewId = aPool.Store( ( sal_uInt16 ) nId );
                }

                aStack << nNewId;
            }
                break;
            case FT_Braces:
                aPool << ocOpen << aStack << ocClose;
                aPool >> aStack;
                break;
            case FT_ConstInt:
            {
                sal_Int16   nVal;
                Read( nVal );
                aStack << aPool.Store( ( double ) nVal );
            }
                break;
            case FT_ConstString:
            {
                String  aTmp(ScfTools::read_zeroTerminated_uInt8s_ToOUString(aIn, nBytesLeft, eSrcChar));
                aStack << aPool.Store( aTmp );
            }
                break;
            case FT_NOP:
                break;
            // ------------------------------------------ fuer > WK3 -
            case FT_Cref:
                Read( nRelBits );
                ReadSRD( rR, nRelBits );
                aStack << aPool.Store( rR );
                break;
            case FT_Rref:
                Read( nRelBits );
                ReadCRD( aCRD, nRelBits );
                aStack << aPool.Store( aCRD );
                break;
            case FT_Nrref:
            {
                String  aTmp(ScfTools::read_zeroTerminated_uInt8s_ToOUString(aIn, nBytesLeft, eSrcChar));
                if( rRangeNameBufferWK3.FindRel( aTmp, nRngIndex ) )
                    aStack << aPool.Store( nRngIndex );
                else
            {
                    String  aText( RTL_CONSTASCII_USTRINGPARAM( "NRREF " ) );
                    aText += aTmp;
                    aStack << aPool.Store( aText );
            }
            }
                break;
            case FT_Absnref:
            {
                String aTmp(ScfTools::read_zeroTerminated_uInt8s_ToOUString(aIn, nBytesLeft, eSrcChar));
                if( rRangeNameBufferWK3.FindAbs( aTmp, nRngIndex ) )
                    aStack << aPool.Store( nRngIndex );
                else
                {
                    String  aText( RTL_CONSTASCII_USTRINGPARAM( "ABSNREF " ) );
                    aText += aTmp;
                    aStack << aPool.Store( aText );
                }
            }
                break;
            case FT_Erref:
                Ignore( 4 );
                aPool << ocBad;
                aPool >> aStack;
                break;
            case FT_Ecref:
                Ignore( 5 );
                aPool << ocBad;
                aPool >> aStack;
                break;
            case FT_Econstant:
                Ignore( 10 );
                aPool << ocBad;
                aPool >> aStack;
                break;
            case FT_Splfunc:
            {
                Read( nAnz );
                Read( nStrLen );

                if( nStrLen )
                {
					sal_Char*	p = new (::std::nothrow) sal_Char[ nStrLen + 1 ];
                    if (p)
                    {
                        aIn.Read( p, nStrLen );
                        p[ nStrLen ] = 0x00;

                        DoFunc( ocNoName, nAnz, p );

                        delete[] p;
                    }
                    else
                        DoFunc( ocNoName, nAnz, NULL );
                }
                else
                    DoFunc( ocNoName, nAnz, NULL );
            }
                break;
            case FT_Const10Float:
                    if ( bWK123 )
                    {
                            double fValue;
                            Read( fValue );
                            aStack << aPool.Store( fValue );
                    }
                    else aStack << aPool.Store( ScfTools::ReadLongDouble( aIn ) );
                    break;
            case FT_Snum:
                    if ( bWK123 )
                {
                         sal_uInt32   nValue;

                         Read( nValue );
                     double  fValue = Snum32ToDouble( nValue );
                     aStack << aPool.Store( fValue );
                }
                else
                {
                        sal_Int16 nVal;
                        Read( nVal );
                        aStack << aPool.Store( SnumToDouble( nVal ) );
                }
                break;
                default:
                OSL_FAIL( "*LotusToSc::Convert(): unbekannter enum!" );
        }
    }

    rpErg = aPool[ aStack.Get() ];

    OSL_ENSURE( nBytesLeft >= 0, "*LotusToSc::Convert(): zuviel verarbeitet!");
    OSL_ENSURE( nBytesLeft <= 0, "*LotusToSc::Convert(): wat is mit'm Rest?" );

    if( rRest )
        aIn.SeekRel( nBytesLeft );  // eventuellen Rest/Ueberlauf korrigieren

    rRest = 0;

    return ConvOK;
}


FUNC_TYPE LotusToSc::IndexToType( sal_uInt8 nIndex )
{
    static const FUNC_TYPE pType[ 256 ] =
    {                       // Code Bezeichnung
        FT_ConstFloat,      //    0 8-Byte-IEEE-Float
        FT_Variable,        //    1 Variable
        FT_Range,           //    2 Bereich
        FT_Return,          //    3 return
        FT_Braces,          //    4 Klammer
        FT_ConstInt,        //    5 2-Byte-Integer
        FT_ConstString,     //    6 ASCII-String
        FT_NOP,             //    7 NOP
        FT_Neg,             //    8 Negation
        FT_Op,              //    9 Addition
        FT_Op,              //   10 Subtraktion
        FT_Op,              //   11 Multiplikation
        FT_Op,              //   12 Division
        FT_Op,              //   13 Potenzierung
        FT_Op,              //   14 Gleichheit
        FT_Op,              //   15 Ungleich
        FT_Op,              //   16 Kleiner-gleich
        FT_Op,              //   17 Groesser-gleich
        FT_Op,              //   18 Kleiner
        FT_Op,              //   19 Groesser
        FT_Op,              //   20 And (logisch)
        FT_Op,              //   21 Or (logisch)
        FT_FuncFix1,        //   22 Not (logisch)
        FT_NOP,             //   23 unaeres Plus
        FT_NotImpl,         //   24
        FT_NotImpl,         //   25
        FT_NotImpl,         //   26
        FT_NotImpl,         //   27
        FT_NotImpl,         //   28
        FT_NotImpl,         //   29
        FT_NotImpl,         //   30
        FT_FuncFix0,        //   31 Not applicable
        FT_FuncFix0,        //   32 Error
        FT_FuncFix1,        //   33 Betrag ABS()
        FT_FuncFix1,        //   34 Ganzzahl INT()
        FT_FuncFix1,        //   35 Quadratwurzel
        FT_FuncFix1,        //   36 Zehnerlogarithmus
        FT_FuncFix1,        //   37 Natuerlicher Logarithmus
        FT_FuncFix0,        //   38 PI
        FT_FuncFix1,        //   39 Sinus
        FT_FuncFix1,        //   40 Cosinus
        FT_FuncFix1,        //   41 Tangens
        FT_FuncFix2,        //   42 Arcus-Tangens 2 (4.Quadrant)   <----- richtig? -
        FT_FuncFix1,        //   43 Arcus-Tangens (2.Quadrant)
        FT_FuncFix1,        //   44 Arcus-Sinus
        FT_FuncFix1,        //   45 Arcus-Cosinus
        FT_FuncFix1,        //   46 Exponentialfunktion
        FT_FuncFix2,        //   47 Modulo
        FT_FuncVar,         //   48 Auswahl
        FT_FuncFix1,        //   49 Is not applicable?
        FT_FuncFix1,        //   50 Is Error?
        FT_FuncFix0,        //   51 FALSE
        FT_FuncFix0,        //   52 TRUE
        FT_FuncFix0,        //   53 Zufallszahl
        FT_FuncFix3,        //   54 Datum
        FT_FuncFix0,        //   55 Heute
        FT_FuncFix3,        //   56 Payment
        FT_FuncFix3,        //   57 Present Value
        FT_FuncFix3,        //   58 Future Value
        FT_FuncFix3,        //   59 If ... then ... else ...
        FT_FuncFix1,        //   60 Tag des Monats
        FT_FuncFix1,        //   61 Monat
        FT_FuncFix1,        //   62 Jahr
        FT_FuncFix2,        //   63 Runden
        FT_FuncFix3,        //   64 Zeit
        FT_FuncFix1,        //   65 Stunde
        FT_FuncFix1,        //   66 Minute
        FT_FuncFix1,        //   67 Sekunde
        FT_FuncFix1,        //   68 Ist Zahl?
        FT_FuncFix1,        //   69 Ist Text?
        FT_FuncFix1,        //   70 Len()
        FT_FuncFix1,        //   71 Val()
        FT_FuncFix2,        //   72 String()
        FT_FuncFix3,        //   73 Mid()
        FT_FuncFix1,        //   74 Char()
        FT_FuncFix1,        //   75 Ascii()
        FT_FuncFix3,        //   76 Find()
        FT_FuncFix1,        //   77 Datevalue
        FT_FuncFix1,        //   78 Timevalue
        FT_FuncFix1,        //   79 Cellpointer
        FT_FuncVar,         //   80 Sum()
        FT_FuncVar,         //   81 Avg()
        FT_FuncVar,         //   82 Cnt()
        FT_FuncVar,         //   83 Min()
        FT_FuncVar,         //   84 Max()
        FT_FuncFix3,        //   85 Vlookup()
        FT_FuncFix2,        //   86 Npv()
        FT_FuncVar,         //   87 Var()
        FT_FuncVar,         //   88 Std()
        FT_FuncFix2,        //   89 Irr()
        FT_FuncFix3,        //   90 Hlookup()
        FT_FuncFix3,        //   91 ?
        FT_FuncFix3,        //   92 ?
        FT_FuncFix3,        //   93 ?
        FT_FuncFix3,        //   94 ?
        FT_FuncFix3,        //   95 ?
        FT_FuncFix3,        //   96 ?
        FT_FuncFix3,        //   97 ?
        FT_FuncFix3,        //   98 Index()                         <- richtig? -
        FT_FuncFix1,        //   99 Cols()
        FT_FuncFix1,        //  100 Rows()
        FT_FuncFix2,        //  101 Repeat()
        FT_FuncFix1,        //  102 Upper()
        FT_FuncFix1,        //  103 Lower()
        FT_FuncFix2,        //  104 Left()
        FT_FuncFix2,        //  105 Right()
        FT_FuncFix4,        //  106 Replace()
        FT_FuncFix1,        //  107 Proper()
        FT_FuncFix2,        //  108 Cell()
        FT_FuncFix1,        //  109 Trim()
        FT_FuncFix1,        //  110 Clean()
        FT_FuncFix1,        //  111 F()
        FT_FuncFix1,        //  112 Wert() (oder W()?)
        FT_FuncFix2,        //  113 Exact()
        FT_NotImpl,         //  114 Call()
        FT_FuncFix1,        //  115 @@()
        FT_FuncFix3,        //  116 Rate()
        FT_FuncFix1,        //  117 Term()
        FT_FuncFix1,        //  118 Cterm()
        FT_FuncFix3,        //  119 Sln()
        FT_FuncFix4,        //  120 Syd(), Soy()
        FT_FuncFix4,        //  121 Ddb()
        FT_NotImpl,         //  122
        FT_NotImpl,         //  123
        FT_NotImpl,         //  124
        FT_NotImpl,         //  125
        FT_NotImpl,         //  126
        FT_NotImpl,         //  127
        FT_NotImpl,         //  128
        FT_NotImpl,         //  129
        FT_NotImpl,         //  130
        FT_NotImpl,         //  131
        FT_NotImpl,         //  132
        FT_NotImpl,         //  133
        FT_NotImpl,         //  134
        FT_NotImpl,         //  135
        FT_NotImpl,         //  136
        FT_NotImpl,         //  137
        FT_NotImpl,         //  138
        FT_NotImpl,         //  139
        FT_NotImpl,         //  140
        FT_NotImpl,         //  141
        FT_NotImpl,         //  142
        FT_NotImpl,         //  143
        FT_NotImpl,         //  144
        FT_NotImpl,         //  145
        FT_NotImpl,         //  146
        FT_NotImpl,         //  147
        FT_NotImpl,         //  148
        FT_NotImpl,         //  149
        FT_NotImpl,         //  150
        FT_NotImpl,         //  151
        FT_NotImpl,         //  152
        FT_NotImpl,         //  153
        FT_NotImpl,         //  154
        FT_NotImpl,         //  155
        FT_FuncVar,         //  156 ?
        FT_NotImpl,         //  157
        FT_NotImpl,         //  158
        FT_NotImpl,         //  159
        FT_NotImpl,         //  160
        FT_NotImpl,         //  161
        FT_NotImpl,         //  162
        FT_NotImpl,         //  163
        FT_NotImpl,         //  164
        FT_NotImpl,         //  165
        FT_NotImpl,         //  166
        FT_NotImpl,         //  167
        FT_NotImpl,         //  168
        FT_NotImpl,         //  169
        FT_NotImpl,         //  170
        FT_NotImpl,         //  171
        FT_NotImpl,         //  172
        FT_NotImpl,         //  173
        FT_NotImpl,         //  174
        FT_NotImpl,         //  175
        FT_NotImpl,         //  176
        FT_NotImpl,         //  177
        FT_NotImpl,         //  178
        FT_NotImpl,         //  179
        FT_NotImpl,         //  180
        FT_NotImpl,         //  181
        FT_NotImpl,         //  182
        FT_NotImpl,         //  183
        FT_NotImpl,         //  184
        FT_NotImpl,         //  185
        FT_NotImpl,         //  186
        FT_NotImpl,         //  187
        FT_NotImpl,         //  188
        FT_NotImpl,         //  189
        FT_NotImpl,         //  190
        FT_NotImpl,         //  191
        FT_NotImpl,         //  192
        FT_NotImpl,         //  193
        FT_NotImpl,         //  194
        FT_NotImpl,         //  195
        FT_NotImpl,         //  196
        FT_NotImpl,         //  197
        FT_NotImpl,         //  198
        FT_NotImpl,         //  199
        FT_NotImpl,         //  200
        FT_NotImpl,         //  201
        FT_NotImpl,         //  202
        FT_NotImpl,         //  203
        FT_NotImpl,         //  204
        FT_NotImpl,         //  205
        FT_FuncVar,         //  206 ?
        FT_NotImpl,         //  207
        FT_NotImpl,         //  208
        FT_NotImpl,         //  209
        FT_NotImpl,         //  210
        FT_NotImpl,         //  211
        FT_NotImpl,         //  212
        FT_NotImpl,         //  213
        FT_NotImpl,         //  214
        FT_NotImpl,         //  215
        FT_NotImpl,         //  216
        FT_NotImpl,         //  217
        FT_NotImpl,         //  218
        FT_NotImpl,         //  219
        FT_NotImpl,         //  220
        FT_NotImpl,         //  221
        FT_NotImpl,         //  222
        FT_NotImpl,         //  223
        FT_NotImpl,         //  224
        FT_NotImpl,         //  225
        FT_NotImpl,         //  226
        FT_NotImpl,         //  227
        FT_NotImpl,         //  228
        FT_NotImpl,         //  229
        FT_NotImpl,         //  230
        FT_NotImpl,         //  231
        FT_NotImpl,         //  232
        FT_NotImpl,         //  233
        FT_NotImpl,         //  234
        FT_NotImpl,         //  235
        FT_NotImpl,         //  236
        FT_NotImpl,         //  237
        FT_NotImpl,         //  238
        FT_NotImpl,         //  239
        FT_NotImpl,         //  240
        FT_NotImpl,         //  241
        FT_NotImpl,         //  242
        FT_NotImpl,         //  243
        FT_NotImpl,         //  244
        FT_NotImpl,         //  245
        FT_NotImpl,         //  246
        FT_NotImpl,         //  247
        FT_NotImpl,         //  248
        FT_NotImpl,         //  249
        FT_NotImpl,         //  250
        FT_NotImpl,         //  251
        FT_NotImpl,         //  252
        FT_NotImpl,         //  253
        FT_NotImpl,         //  254
        FT_FuncVar,         //  255 ?
    };
    return pType[ nIndex ];
}


DefTokenId LotusToSc::IndexToToken( sal_uInt8 nIndex )
{
    static const DefTokenId pToken[ 256 ] =
    {                       // Code Bezeichnung
        ocPush,             //    0 8-Byte-IEEE-Float
        ocPush,             //    1 Variable
        ocPush,             //    2 Bereich
        ocPush,             //    3 return
        ocPush,             //    4 Klammer
        ocPush,             //    5 2-Byte-Integer
        ocPush,             //    6 ASCII-String
        ocPush,             //    7 NOP
        ocNegSub,           //    8 Negation
        ocAdd,              //    9 Addition
        ocSub,              //   10 Subtraktion
        ocMul,              //   11 Multiplikation
        ocDiv,              //   12 Division
        ocPow,              //   13 Potenzierung
        ocEqual,            //   14 Gleichheit
        ocNotEqual,         //   15 Ungleich
        ocLessEqual,        //   16 Kleiner-gleich
        ocGreaterEqual,     //   17 Groesser-gleich
        ocLess,             //   18 Kleiner
        ocGreater,          //   19 Groesser
        ocAnd,              //   20 And (logisch)
        ocOr,               //   21 Or (logisch)
        ocNot,              //   22 Not (logisch)
        ocPush,             //   23 unaeres Plus
        ocNoName,           //   24
        ocNoName,           //   25
        ocNoName,           //   26
        ocNoName,           //   27
        ocNoName,           //   28
        ocNoName,           //   29
        ocNoName,           //   30
        ocNotAvail,         //   31 Not available
        ocNoName,           //   32 Error
        ocAbs,              //   33 Betrag ABS()
        ocInt,              //   34 Ganzzahl INT()
        ocSqrt,             //   35 Quadratwurzel
        ocLog10,            //   36 Zehnerlogarithmus
        ocLn,               //   37 Natuerlicher Logarithmus
        ocPi,               //   38 PI
        ocSin,              //   39 Sinus
        ocCos,              //   40 Cosinus
        ocTan,              //   41 Tangens
        ocArcTan2,          //   42 Arcus-Tangens 2 (4.Quadrant)
        ocArcTan,           //   43 Arcus-Tangens (2.Quadrant)
        ocArcSin,           //   44 Arcus-Sinus
        ocArcCos,           //   45 Arcus-Cosinus
        ocExp,              //   46 Exponentialfunktion
        ocMod,              //   47 Modulo
        ocChose,            //   48 Auswahl
        ocIsNA,             //   49 Is not available?
        ocIsError,          //   50 Is Error?
        ocFalse,            //   51 FALSE
        ocTrue,             //   52 TRUE
        ocRandom,           //   53 Zufallszahl
        ocGetDate,          //   54 Datum
        ocGetActDate,       //   55 Heute
        ocRMZ,              //   56 Payment
        ocBW,               //   57 Present Value
        ocZW,               //   58 Future Value
        ocIf,               //   59 If ... then ... else ...
        ocGetDay,           //   60 Tag des Monats
        ocGetMonth,         //   61 Monat
        ocGetYear,          //   62 Jahr
        ocRound,            //   63 Runden
        ocGetTime,          //   64 Zeit
        ocGetHour,          //   65 Stunde
        ocGetMin,           //   66 Minute
        ocGetSec,           //   67 Sekunde
        ocIsValue,          //   68 Ist Zahl?
        ocIsString,         //   69 Ist Text?
        ocLen,              //   70 Len()
        ocValue,            //   71 Val()
        ocFixed,            //   72 String()    ocFixed ersatzweise + Spezialfall
        ocMid,              //   73 Mid()
        ocChar,             //   74 Char()
        ocCode,             //   75 Ascii()
        ocFind,             //   76 Find()
        ocGetDateValue,     //   77 Datevalue
        ocGetTimeValue,     //   78 Timevalue
        ocNoName,           //   79 Cellpointer
        ocSum,              //   80 Sum()
        ocAverage,          //   81 Avg()
        ocCount,            //   82 Cnt()
        ocMin,              //   83 Min()
        ocMax,              //   84 Max()
        ocVLookup,          //   85 Vlookup()
        ocNPV,              //   86 Npv()
        ocVar,              //   87 Var()
        ocNormDist,         //   88 Std()
        ocIRR,              //   89 Irr()
        ocHLookup,          //   90 Hlookup()
        ocDBSum,            //   91 XlfDsum
        ocDBAverage,        //   92 XlfDaverage
        ocDBCount,          //   93 XlfDcount
        ocDBMin,            //   94 XlfDmin
        ocDBMax,            //   95 XlfDmax
        ocDBVar,            //   96 XlfDvar
        ocDBStdDev,         //   97 XlfDstdev
        ocIndex,            //   98 Index()
        ocColumns,          //   99 Cols()
        ocRows,             //  100 Rows()
        ocRept,             //  101 Repeat()
        ocUpper,            //  102 Upper()
        ocLower,            //  103 Lower()
        ocLeft,             //  104 Left()
        ocRight,            //  105 Right()
        ocReplace,          //  106 Replace()
        ocPropper,          //  107 Proper()
        ocNoName,           //  108 Cell()
        ocTrim,             //  109 Trim()
        ocClean,            //  110 Clean()
        ocFalse,            //  111 F()
        ocTrue,             //  112 W()
        ocExact,            //  113 Exact()
        ocNoName,           //  114 Call()
        ocIndirect,         //  115 @@()
        ocZins,             //  116 Rate()
        ocNoName,           //  117 Term()
        ocNoName,           //  118 Cterm()
        ocLIA,              //  119 Sln()
        ocDIA,              //  120 Syd(), Soy()
        ocGDA,              //  121 Ddb()
        ocNoName,           //  122
        ocNoName,           //  123
        ocNoName,           //  124
        ocNoName,           //  125
        ocNoName,           //  126
        ocNoName,           //  127
        ocNoName,           //  128
        ocNoName,           //  129
        ocNoName,           //  130
        ocNoName,           //  131
        ocNoName,           //  132
        ocNoName,           //  133
        ocNoName,           //  134
        ocNoName,           //  135
        ocNoName,           //  136
        ocNoName,           //  137
        ocNoName,           //  138
        ocNoName,           //  139
        ocNoName,           //  140
        ocNoName,           //  141
        ocNoName,           //  142
        ocNoName,           //  143
        ocNoName,           //  144
        ocNoName,           //  145
        ocNoName,           //  146
        ocNoName,           //  147
        ocNoName,           //  148
        ocNoName,           //  149
        ocNoName,           //  150
        ocNoName,           //  151
        ocNoName,           //  152
        ocNoName,           //  153
        ocNoName,           //  154
        ocNoName,           //  155
        ocNoName,           //  156 ?
        ocNoName,           //  157
        ocNoName,           //  158
        ocNoName,           //  159
        ocNoName,           //  160
        ocNoName,           //  161
        ocNoName,           //  162
        ocNoName,           //  163
        ocNoName,           //  164
        ocNoName,           //  165
        ocNoName,           //  166
        ocNoName,           //  167
        ocNoName,           //  168
        ocNoName,           //  169
        ocNoName,           //  170
        ocNoName,           //  171
        ocNoName,           //  172
        ocNoName,           //  173
        ocNoName,           //  174
        ocNoName,           //  175
        ocNoName,           //  176
        ocNoName,           //  177
        ocNoName,           //  178
        ocNoName,           //  179
        ocNoName,           //  180
        ocNoName,           //  181
        ocNoName,           //  182
        ocNoName,           //  183
        ocNoName,           //  184
        ocNoName,           //  185
        ocNoName,           //  186
        ocNoName,           //  187
        ocNoName,           //  188
        ocNoName,           //  189
        ocNoName,           //  190
        ocNoName,           //  191
        ocNoName,           //  192
        ocNoName,           //  193
        ocNoName,           //  194
        ocNoName,           //  195
        ocNoName,           //  196
        ocNoName,           //  197
        ocNoName,           //  198
        ocNoName,           //  199
        ocNoName,           //  200
        ocNoName,           //  201
        ocNoName,           //  202
        ocNoName,           //  203
        ocNoName,           //  204
        ocNoName,           //  205
        ocNoName,           //  206 ?
        ocNoName,           //  207
        ocNoName,           //  208
        ocNoName,           //  209
        ocNoName,           //  210
        ocNoName,           //  211
        ocNoName,           //  212
        ocNoName,           //  213
        ocNoName,           //  214
        ocNoName,           //  215
        ocNoName,           //  216
        ocNoName,           //  217
        ocNoName,           //  218
        ocNoName,           //  219
        ocNoName,           //  220
        ocNoName,           //  221
        ocNoName,           //  222
        ocNoName,           //  223
        ocNoName,           //  224
        ocNoName,           //  225
        ocNoName,           //  226
        ocNoName,           //  227
        ocNoName,           //  228
        ocNoName,           //  229
        ocNoName,           //  230
        ocNoName,           //  231
        ocNoName,           //  232
        ocNoName,           //  233
        ocNoName,           //  234
        ocNoName,           //  235
        ocNoName,           //  236
        ocNoName,           //  237
        ocNoName,           //  238
        ocNoName,           //  239
        ocNoName,           //  240
        ocNoName,           //  241
        ocNoName,           //  242
        ocNoName,           //  243
        ocNoName,           //  244
        ocNoName,           //  245
        ocNoName,           //  246
        ocNoName,           //  247
        ocNoName,           //  248
        ocNoName,           //  249
        ocNoName,           //  250
        ocNoName,           //  251
        ocNoName,           //  252
        ocNoName,           //  253
        ocNoName,           //  254
        ocNoName            //  255 ?
    };

    return pToken[ nIndex ];
}


FUNC_TYPE LotusToSc::IndexToTypeWK123( sal_uInt8 nIndex )
{
    static const FUNC_TYPE pType[ 256 ] =
    {                       // Code Bezeichnung
        FT_Const10Float,    //    0 8-Byte-IEEE-Long-Number
        FT_Cref,            //    1 Cell Reference
        FT_Rref,            //    2 Area Reference
        FT_Return,          //    3 return
        FT_Braces,          //    4 Klammer
        FT_Snum,            //    5 Short-Number
        FT_ConstString,     //    6 ASCII-String
        FT_Nrref,           //    7 Named range reference
        FT_Absnref,         //    8 Absolut named range
        FT_Erref,           //    9 Err range reference
        FT_Ecref,           //   10 Err cell reference
        FT_Econstant,       //   11 Err constant
        FT_NotImpl,         //   12
        FT_NotImpl,         //   13
        FT_Neg,             //   14 Negation
        FT_Op,              //   15 Addition
        FT_Op,              //   16 Subtraktion
        FT_Op,              //   17 Multiplikation
        FT_Op,              //   18 Division
        FT_Op,              //   19 Potenzierung
        FT_Op,              //   20 Gleichheit
        FT_Op,              //   21 Ungleich
        FT_Op,              //   22 Kleiner-gleich
        FT_Op,              //   23 Groesser-gleich
        FT_Op,              //   24 Kleiner
        FT_Op,              //   25 Groesser
        FT_Op,              //   26 And (logisch)
        FT_Op,              //   27 Or (logisch)
        FT_FuncFix1,        //   28 Not (logisch)
        FT_NOP,             //   29 unaeres Plus
        FT_Op,              //   30 Concatenation
        FT_FuncFix0,        //   31 Not applicable
        FT_FuncFix0,        //   32 Error
        FT_FuncFix1,        //   33 Betrag ABS()
        FT_FuncFix1,        //   34 Ganzzahl INT()
        FT_FuncFix1,        //   35 Quadratwurzel
        FT_FuncFix1,        //   36 Zehnerlogarithmus
        FT_FuncFix1,        //   37 Natuerlicher Logarithmus
        FT_FuncFix0,        //   38 PI
        FT_FuncFix1,        //   39 Sinus
        FT_FuncFix1,        //   40 Cosinus
        FT_FuncFix1,        //   41 Tangens
        FT_FuncFix2,        //   42 Arcus-Tangens 2 (4.Quadrant)
        FT_FuncFix1,        //   43 Arcus-Tangens (2.Quadrant)
        FT_FuncFix1,        //   44 Arcus-Sinus
        FT_FuncFix1,        //   45 Arcus-Cosinus
        FT_FuncFix1,        //   46 Exponentialfunktion
        FT_FuncFix2,        //   47 Modulo
        FT_FuncVar,         //   48 Auswahl
        FT_FuncFix1,        //   49 Is not applicable?
        FT_FuncFix1,        //   50 Is Error?
        FT_FuncFix0,        //   51 FALSE
        FT_FuncFix0,        //   52 TRUE
        FT_FuncFix0,        //   53 Zufallszahl
        FT_FuncFix3,        //   54 Datum
        FT_FuncFix0,        //   55 Heute
        FT_FuncFix3,        //   56 Payment
        FT_FuncFix3,        //   57 Present Value
        FT_FuncFix3,        //   58 Future Value
        FT_FuncFix3,        //   59 If ... then ... else ...
        FT_FuncFix1,        //   60 Tag des Monats
        FT_FuncFix1,        //   61 Monat
        FT_FuncFix1,        //   62 Jahr
        FT_FuncFix2,        //   63 Runden
        FT_FuncFix3,        //   64 Zeit
        FT_FuncFix1,        //   65 Stunde
        FT_FuncFix1,        //   66 Minute
        FT_FuncFix1,        //   67 Sekunde
        FT_FuncFix1,        //   68 Ist Zahl?
        FT_FuncFix1,        //   69 Ist Text?
        FT_FuncFix1,        //   70 Len()
        FT_FuncFix1,        //   71 Val()
        FT_FuncFix2,        //   72 String()
        FT_FuncFix3,        //   73 Mid()
        FT_FuncFix1,        //   74 Char()
        FT_FuncFix1,        //   75 Ascii()
        FT_FuncFix3,        //   76 Find()
        FT_FuncFix1,        //   77 Datevalue
        FT_FuncFix1,        //   78 Timevalue
        FT_FuncFix1,        //   79 Cellpointer
        FT_FuncVar,         //   80 Sum()
        FT_FuncVar,         //   81 Avg()
        FT_FuncVar,         //   82 Cnt()
        FT_FuncVar,         //   83 Min()
        FT_FuncVar,         //   84 Max()
        FT_FuncFix3,        //   85 Vlookup()
        FT_FuncFix2,        //   86 Npv()
        FT_FuncVar,         //   87 Var()
        FT_FuncVar,         //   88 Std()
        FT_FuncFix2,        //   89 Irr()
        FT_FuncFix3,        //   90 Hlookup()
        FT_FuncVar,         //   91 Dsum                 <-------- neu! -
        FT_FuncVar,         //   92 Davg                 <-------- neu! -
        FT_FuncVar,         //   93 Dcnt                 <-------- neu! -
        FT_FuncVar,         //   94 Dmin                 <-------- neu! -
        FT_FuncVar,         //   95 Dmax                 <-------- neu! -
        FT_FuncVar,         //   96 Dvar                 <-------- neu! -
        FT_FuncVar,         //   97 Dstd                 <-------- neu! -
        FT_FuncVar,         //   98 Index()              <-------- change! -
        FT_FuncFix1,        //   99 Cols()               <-------- neu! -
        FT_FuncFix1,        //  100 Rows()               <-------- neu! -
        FT_FuncFix2,        //  101 Repeat()             <-------- neu! -
        FT_FuncFix1,        //  102 Upper()              <-------- neu! -
        FT_FuncFix1,        //  103 Lower()              <-------- neu! -
        FT_FuncFix2,        //  104 Left()               <-------- neu! -
        FT_FuncFix2,        //  105 Right()              <-------- neu! -
        FT_FuncFix4,        //  106 Replace()            <-------- neu! -
        FT_FuncFix1,        //  107 Proper()             <-------- neu! -
        FT_FuncFix2,        //  108 Cell()               <-------- neu! -
        FT_FuncFix1,        //  109 Trim()               <-------- neu! -
        FT_FuncFix1,        //  110 Clean()              <-------- neu! -
        FT_FuncFix1,        //  111 S()                  <--------- change in Bez. -
        FT_FuncFix1,        //  112 N()                  <--------- change in Bez. -
        FT_FuncFix2,        //  113 Exact()              <-------- neu! -
        FT_NotImpl,         //  114 App                  <--------- change in Bez. -
        FT_FuncFix1,        //  115 @@()                 <-------- neu! -
        FT_FuncFix3,        //  116 Rate()               <-------- neu! -
        FT_FuncFix3,        //  117 Term()               <--------- change in Anz.
        FT_FuncFix3,        //  118 Cterm()              <--------- change in Anz.
        FT_FuncFix3,        //  119 Sln()                <-------- neu! -
        FT_FuncFix4,        //  120 Syd()                <-------- neu! -
        FT_FuncFix4,        //  121 Ddb()                <-------- neu! -
        FT_Splfunc,         //  122 Splfunc              <-------- neu! -
        FT_FuncFix1,        //  123 Sheets               <-------- neu! -
        FT_FuncFix1,        //  124 Info                 <-------- neu! -
        FT_FuncVar,         //  125 Sumproduct           <-------- neu! -
        FT_FuncFix1,        //  126 Isrange              <-------- neu! -
        FT_FuncVar,         //  127 Dget                 <-------- neu! -
        FT_FuncVar,         //  128 Dquery               <-------- neu! -
        FT_FuncFix4,        //  129 Coord                <-------- neu! -
        FT_NOP,             //  130 Reserved (internal)  <-------- neu! -
        FT_FuncFix0,        //  131 Today                <-------- neu! -
        FT_FuncVar,         //  132 Vdb                  <-------- neu! -
        FT_FuncVar,         //  133 Dvars                <-------- neu! -
        FT_FuncVar,         //  134 Dstds                <-------- neu! -
        FT_FuncVar,         //  135 Vars                 <-------- neu! -
        FT_FuncVar,         //  136 Stds                 <-------- neu! -
        FT_FuncFix2,        //  137 D360                 <-------- neu! -
        FT_NOP,             //  138 Reserved (internal)  <-------- neu! -
        FT_FuncFix0,        //  139 Isapp                <-------- neu! - Anzahl ? -
        FT_FuncVar,         //  140 Isaaf                <-------- neu! - Anzahl ? -
        FT_FuncFix1,        //  141 Weekday              <-------- neu! -
        FT_FuncFix3,        //  142 Datedif              <-------- neu! -
        FT_FuncVar,         //  143 Rank                 <-------- neu! -
        FT_FuncFix2,        //  144 Numberstring         <-------- neu! -
        FT_FuncFix1,        //  145 Datestring           <-------- neu! -
        FT_FuncFix1,        //  146 Decimal              <-------- neu! -
        FT_FuncFix1,        //  147 Hex                  <-------- neu! -
        FT_FuncFix4,        //  148 Db                   <-------- neu! -
        FT_FuncFix4,        //  149 Pmti                 <-------- neu! -
        FT_FuncFix4,        //  150 Spi                  <-------- neu! -
        FT_FuncFix1,        //  151 Fullp                <-------- neu! -
        FT_FuncFix1,        //  152 Halfp                <-------- neu! -
        FT_FuncVar,         //  153 Pureavg              <-------- neu! -
        FT_FuncVar,         //  154 Purecount            <-------- neu! -
        FT_FuncVar,         //  155 Puremax              <-------- neu! -
        FT_FuncVar,         //  156 Puremin              <-------- neu! -
        FT_FuncVar,         //  157 Purestd              <-------- neu! -
        FT_FuncVar,         //  158 Purevar              <-------- neu! -
        FT_FuncVar,         //  159 Purestds             <-------- neu! -
        FT_FuncVar,         //  160 Purevars             <-------- neu! -
        FT_FuncFix3,        //  161 Pmt2                 <-------- neu! -
        FT_FuncFix3,        //  162 Pv2                  <-------- neu! -
        FT_FuncFix3,        //  163 Fv2                  <-------- neu! -
        FT_FuncFix3,        //  164 Term2                <-------- neu! -
        FT_NotImpl,         //  165 ---                  <-------- neu! - Anzahl ? -
        FT_FuncFix2,        //  166 D360 (US-Version)
        FT_NotImpl,         //  167
        FT_NotImpl,         //  168
        FT_NotImpl,         //  169
        FT_NotImpl,         //  170
        FT_NotImpl,         //  171
        FT_NotImpl,         //  172
        FT_NotImpl,         //  173
        FT_NotImpl,         //  174
        FT_NotImpl,         //  175
        FT_NotImpl,         //  176
        FT_NotImpl,         //  177
        FT_NotImpl,         //  178
        FT_NotImpl,         //  179
        FT_NotImpl,         //  180
        FT_NotImpl,         //  181
        FT_NotImpl,         //  182
        FT_NotImpl,         //  183
        FT_NotImpl,         //  184
        FT_NotImpl,         //  185
        FT_FuncVar,         //  186 Solver               <-------- neu! -
        FT_NotImpl,         //  187
        FT_NotImpl,         //  188
        FT_NotImpl,         //  189
        FT_NotImpl,         //  190
        FT_NotImpl,         //  191
        FT_NotImpl,         //  192
        FT_NotImpl,         //  193
        FT_NotImpl,         //  194
        FT_NotImpl,         //  195
        FT_NotImpl,         //  196
        FT_NotImpl,         //  197
        FT_NotImpl,         //  198
        FT_NotImpl,         //  199
        FT_NotImpl,         //  200
        FT_NotImpl,         //  201
        FT_NotImpl,         //  202
        FT_NotImpl,         //  203
        FT_NotImpl,         //  204
        FT_NotImpl,         //  205
        FT_NotImpl,         //  206
        FT_NotImpl,         //  207
        FT_NotImpl,         //  208
        FT_NotImpl,         //  209
        FT_NotImpl,         //  210
        FT_NotImpl,         //  211
        FT_NotImpl,         //  212
        FT_NotImpl,         //  213
        FT_NotImpl,         //  214
        FT_NotImpl,         //  215
        FT_NotImpl,         //  216
        FT_NotImpl,         //  217
        FT_NotImpl,         //  218
        FT_NotImpl,         //  219
        FT_NotImpl,         //  220
        FT_NotImpl,         //  221
        FT_NotImpl,         //  222
        FT_NotImpl,         //  223
        FT_NotImpl,         //  224
        FT_NotImpl,         //  225
        FT_NotImpl,         //  226
        FT_NotImpl,         //  227
        FT_NotImpl,         //  228
        FT_NotImpl,         //  229
        FT_NotImpl,         //  230
        FT_NotImpl,         //  231
        FT_NotImpl,         //  232
        FT_NotImpl,         //  233
        FT_NotImpl,         //  234
        FT_NotImpl,         //  235
        FT_NotImpl,         //  236
        FT_NotImpl,         //  237
        FT_NotImpl,         //  238
        FT_NotImpl,         //  239
        FT_NotImpl,         //  240
        FT_NotImpl,         //  241
        FT_NotImpl,         //  242
        FT_NotImpl,         //  243
        FT_NotImpl,         //  244
        FT_NotImpl,         //  245
        FT_NotImpl,         //  246
        FT_NotImpl,         //  247
        FT_NotImpl,         //  248
        FT_NotImpl,         //  249
        FT_NotImpl,         //  250
        FT_NotImpl,         //  251
        FT_NotImpl,         //  252
        FT_NotImpl,         //  253
        FT_NotImpl,         //  254
        FT_NotImpl,         //  255
    };
    return pType[ nIndex ];
}


DefTokenId LotusToSc::IndexToTokenWK123( sal_uInt8 nIndex )
{
    static const DefTokenId pToken[ 256 ] =
    {                       // Code Bezeichnung
        ocPush,             //    0 8-Byte-IEEE-Long-Numbers
        ocPush,             //    1 Variable
        ocPush,             //    2 Bereich
        ocPush,             //    3 return
        ocPush,             //    4 Klammer
        ocPush,             //    5 Numbers
        ocPush,             //    6 ASCII-String
        ocPush,             //    7 Named range reference
        ocPush,             //    8 Absolut named range
        ocPush,             //    9 Err range reference
        ocPush,             //   10 Err cell reference
        ocPush,             //   11 Err constant
        ocPush,             //   12
        ocPush,             //   13
        ocNegSub,           //   14 Negation
        ocAdd,              //   15 Addition
        ocSub,              //   16 Subtraktion
        ocMul,              //   17 Multiplikation
        ocDiv,              //   18 Division
        ocPow,              //   19 Potenzierung
        ocEqual,            //   20 Gleichheit
        ocNotEqual,         //   21 Ungleich
        ocLessEqual,        //   22 Kleiner-gleich
        ocGreaterEqual,     //   23 Groesser-gleich
        ocLess,             //   24 Kleiner
        ocGreater,          //   25 Groesser
        ocAnd,              //   26 And (logisch)
        ocOr,               //   27 Or (logisch)
        ocNot,              //   28 Not (logisch)
        ocPush,             //   29 unaeres Plus
        ocAmpersand,        //   30 Concatenation
        ocNotAvail,         //   31 Not available
        ocNoName,           //   32 Error
        ocAbs,              //   33 Betrag ABS()
        ocInt,              //   34 Ganzzahl INT()
        ocSqrt,             //   35 Quadratwurzel
        ocLog10,            //   36 Zehnerlogarithmus
        ocLn,               //   37 Natuerlicher Logarithmus
        ocPi,               //   38 PI
        ocSin,              //   39 Sinus
        ocCos,              //   40 Cosinus
        ocTan,              //   41 Tangens
        ocArcTan2,          //   42 Arcus-Tangens 2 (4.Quadrant)
        ocArcTan,           //   43 Arcus-Tangens (2.Quadrant)
        ocArcSin,           //   44 Arcus-Sinus
        ocArcCos,           //   45 Arcus-Cosinus
        ocExp,              //   46 Exponentialfunktion
        ocMod,              //   47 Modulo
        ocChose,            //   48 Auswahl
        ocIsNA,             //   49 Is not available?
        ocIsError,          //   50 Is Error?
        ocFalse,            //   51 FALSE
        ocTrue,             //   52 TRUE
        ocRandom,           //   53 Zufallszahl
        ocGetDate,          //   54 Datum
        ocGetActDate,       //   55 Heute
        ocRMZ,              //   56 Payment
        ocBW,               //   57 Present Value
        ocZW,               //   58 Future Value
        ocIf,               //   59 If ... then ... else ...
        ocGetDay,           //   60 Tag des Monats
        ocGetMonth,         //   61 Monat
        ocGetYear,          //   62 Jahr
        ocRound,            //   63 Runden
        ocGetTime,          //   64 Zeit
        ocGetHour,          //   65 Stunde
        ocGetMin,           //   66 Minute
        ocGetSec,           //   67 Sekunde
        ocIsValue,          //   68 Ist Zahl?
        ocIsString,         //   69 Ist Text?
        ocLen,              //   70 Len()
        ocValue,            //   71 Val()
        ocFixed,            //   72 String()    ocFixed ersatzweise + Spezialfall
        ocMid,              //   73 Mid()
        ocChar,             //   74 Char()
        ocCode,             //   75 Ascii()
        ocFind,             //   76 Find()
        ocGetDateValue,     //   77 Datevalue
        ocGetTimeValue,     //   78 Timevalue
        ocNoName,           //   79 Cellpointer
        ocSum,              //   80 Sum()
        ocAverage,          //   81 Avg()
        ocCount,            //   82 Cnt()
        ocMin,              //   83 Min()
        ocMax,              //   84 Max()
        ocVLookup,          //   85 Vlookup()
        ocNPV,              //   86 Npv()
        ocVar,              //   87 Var()
        ocStDev,            //   88 Std()
        ocIRR,              //   89 Irr()
        ocHLookup,          //   90 Hlookup()
        ocDBSum,            //   91 XlfDsum
        ocDBAverage,        //   92 XlfDaverage
        ocDBCount,          //   93 XlfDcount
        ocDBMin,            //   94 XlfDmin
        ocDBMax,            //   95 XlfDmax
        ocDBVar,            //   96 XlfDvar
        ocDBStdDev,         //   97 XlfDstdev
        ocIndex,            //   98 Index()
        ocColumns,          //   99 Cols()
        ocRows,             //  100 Rows()
        ocRept,             //  101 Repeat()
        ocUpper,            //  102 Upper()
        ocLower,            //  103 Lower()
        ocLeft,             //  104 Left()
        ocRight,            //  105 Right()
        ocReplace,          //  106 Replace()
        ocPropper,          //  107 Proper()
        ocNoName,           //  108 Cell()
        ocTrim,             //  109 Trim()
        ocClean,            //  110 Clean()
        ocNoName,           //  111 F()     (Excel: T()?)
        ocNoName,           //  112 W()
        ocExact,            //  113 Exact()
        ocNoName,           //  114 Call()
        ocIndirect,         //  115 @@()
        ocZins,             //  116 Rate()
        ocNoName,           //  117 Term()
        ocNoName,           //  118 Cterm()
        ocLIA,              //  119 Sln()
        ocDIA,              //  120 Syd(), Soy()
        ocGDA,              //  121 Ddb()
        ocNoName,           //  122 Splfunc
        ocNoName,           //  123 Sheets
        ocNoName,           //  124 Info
        ocSumProduct,       //  125 Sumproduct
        ocNoName,           //  126 Isrange
        ocDBGet,            //  127 Dget
        ocNoName,           //  128 Dquery
        ocNoName,           //  129 Coord
        ocNoName,           //  130 Reserved (internal)
        ocGetActDate,       //  131 Today
        ocNoName,           //  132 Vdb
        ocDBVarP,           //  133 Dvars
        ocDBStdDevP,        //  134 Dstds
        ocVarP,             //  135 Vars
        ocStDevP,           //  136 Stds
        ocGetDiffDate360,   //  137 D360
        ocNoName,           //  138 Reserved (internal)
        ocNoName,           //  139 Isapp
        ocNoName,           //  140 Isaaf
        ocGetDayOfWeek,     //  141 Weekday
        ocGetDiffDate,      //  142 Datedif
        ocRank,             //  143 Rank
        ocNoName,           //  144 Numberstring
        ocNoName,           //  145 Datestring
        ocNoName,           //  146 Decimal
        ocNoName,           //  147 Hex
        ocNoName,           //  148 Db
        ocNoName,           //  149 Pmti
        ocNoName,           //  150 Spi
        ocNoName,           //  151 Fullp
        ocNoName,           //  152 Halfp
        ocNoName,           //  153 Pureavg
        ocCount2,           //  154 Purecount
        ocNoName,           //  155 Puremax
        ocNoName,           //  156 Puremin
        ocNoName,           //  157 Purestd
        ocNoName,           //  158 Purevar
        ocNoName,           //  159 Purestds
        ocNoName,           //  160 Purevars
        ocNoName,           //  161 Pmt2
        ocNoName,           //  162 Pv2
        ocNoName,           //  163 Fv2
        ocNoName,           //  164 Term2
        ocNoName,           //  165 ---                  <-------- neu! - Anzahl ? -
        ocGetDiffDate360,   //  166 D360 (US-Version, ersatzweise wie ander D360-Funktion)
        ocNoName,           //  167
        ocNoName,           //  168
        ocNoName,           //  169
        ocNoName,           //  170
        ocNoName,           //  171
        ocNoName,           //  172
        ocNoName,           //  173
        ocNoName,           //  174
        ocNoName,           //  175
        ocNoName,           //  176
        ocNoName,           //  177
        ocNoName,           //  178
        ocNoName,           //  179
        ocNoName,           //  180
        ocNoName,           //  181
        ocNoName,           //  182
        ocNoName,           //  183
        ocNoName,           //  184
        ocNoName,           //  185
        ocNoName,           //  186
        ocNoName,           //  187
        ocNoName,           //  188
        ocNoName,           //  189
        ocNoName,           //  190
        ocNoName,           //  191
        ocNoName,           //  192
        ocNoName,           //  193
        ocNoName,           //  194
        ocNoName,           //  195
        ocNoName,           //  196
        ocNoName,           //  197
        ocNoName,           //  198
        ocNoName,           //  199
        ocNoName,           //  200
        ocNoName,           //  201
        ocNoName,           //  202
        ocNoName,           //  203
        ocNoName,           //  204
        ocNoName,           //  205
        ocNoName,           //  206 ?
        ocNoName,           //  207
        ocNoName,           //  208
        ocNoName,           //  209
        ocNoName,           //  210
        ocNoName,           //  211
        ocNoName,           //  212
        ocNoName,           //  213
        ocNoName,           //  214
        ocNoName,           //  215
        ocNoName,           //  216
        ocNoName,           //  217
        ocNoName,           //  218
        ocNoName,           //  219
        ocNoName,           //  220
        ocNoName,           //  221
        ocNoName,           //  222
        ocNoName,           //  223
        ocNoName,           //  224
        ocNoName,           //  225
        ocNoName,           //  226
        ocNoName,           //  227
        ocNoName,           //  228
        ocNoName,           //  229
        ocNoName,           //  230
        ocNoName,           //  231
        ocNoName,           //  232
        ocNoName,           //  233
        ocNoName,           //  234
        ocNoName,           //  235
        ocNoName,           //  236
        ocNoName,           //  237
        ocNoName,           //  238
        ocNoName,           //  239
        ocNoName,           //  240
        ocNoName,           //  241
        ocNoName,           //  242
        ocNoName,           //  243
        ocNoName,           //  244
        ocNoName,           //  245
        ocNoName,           //  246
        ocNoName,           //  247
        ocNoName,           //  248
        ocNoName,           //  249
        ocNoName,           //  250
        ocNoName,           //  251
        ocNoName,           //  252
        ocNoName,           //  253
        ocNoName,           //  254
        ocNoName            //  255 ?
    };

    return pToken[ nIndex ];
}




const sal_Char* GetAddInName( const sal_uInt8 n )
{
    static const sal_Char*  pNames[ 256 ] =
    {
        NULL,                       //    0 8-Byte-IEEE-Float
        NULL,                       //    1 Variable
        NULL,                       //    2 Bereich
        NULL,                       //    3 return
        NULL,                       //    4 Klammer
        NULL,                       //    5 2-Byte-Integer
        NULL,                       //    6 ASCII-String
        NULL,                       //    7 Named range reference
        NULL,                       //    8 Absolut named range
        NULL,                       //    9 Err range reference
        NULL,                       //   10 Err cell reference
        NULL,                       //   11 Err constant
        NULL,                       //   12
        NULL,                       //   13
        NULL,                       //   14 Negation
        NULL,                       //   15 Addition
        NULL,                       //   16 Subtraktion
        NULL,                       //   17 Multiplikation
        NULL,                       //   18 Division
        NULL,                       //   19 Potenzierung
        NULL,                       //   20 Gleichheit
        NULL,                       //   21 Ungleich
        NULL,                       //   22 Kleiner-gleich
        NULL,                       //   23 Groesser-gleich
        NULL,                       //   24 Kleiner
        NULL,                       //   25 Groesser
        NULL,                       //   26 And (logisch)
        NULL,                       //   27 Or (logisch)
        NULL,                       //   28 Not (logisch)
        NULL,                       //   29 unaeres Plus
        NULL,                       //   30 Concatenation
        NULL,                       //   31 Not applicable
        NULL,                       //   32 Error
        NULL,                       //   33 Betrag ABS()
        NULL,                       //   34 Ganzzahl INT()
        NULL,                       //   35 Quadratwurzel
        NULL,                       //   36 Zehnerlogarithmus
        NULL,                       //   37 Natuerlicher Logarithmus
        NULL,                       //   38 PI
        NULL,                       //   39 Sinus
        NULL,                       //   40 Cosinus
        NULL,                       //   41 Tangens
        NULL,                       //   42 Arcus-Tangens 2 (4.Quadrant)
        NULL,                       //   43 Arcus-Tangens (2.Quadrant)
        NULL,                       //   44 Arcus-Sinus
        NULL,                       //   45 Arcus-Cosinus
        NULL,                       //   46 Exponentialfunktion
        NULL,                       //   47 Modulo
        NULL,                       //   48 Auswahl
        NULL,                       //   49 Is not applicable?
        NULL,                       //   50 Is Error?
        NULL,                       //   51 FALSE
        NULL,                       //   52 TRUE
        NULL,                       //   53 Zufallszahl
        NULL,                       //   54 Datum
        NULL,                       //   55 Heute
        NULL,                       //   56 Payment
        NULL,                       //   57 Present Value
        NULL,                       //   58 Future Value
        NULL,                       //   59 If ... then ... else ...
        NULL,                       //   60 Tag des Monats
        NULL,                       //   61 Monat
        NULL,                       //   62 Jahr
        NULL,                       //   63 Runden
        NULL,                       //   64 Zeit
        NULL,                       //   65 Stunde
        NULL,                       //   66 Minute
        NULL,                       //   67 Sekunde
        NULL,                       //   68 Ist Zahl?
        NULL,                       //   69 Ist Text?
        NULL,                       //   70 Len()
        NULL,                       //   71 Val()
        NULL,                       //   72 String()    ocFixed ersatzweise + Spezialfall
        NULL,                       //   73 Mid()
        NULL,                       //   74 Char()
        NULL,                       //   75 Ascii()
        NULL,                       //   76 Find()
        NULL,                       //   77 Datevalue
        NULL,                       //   78 Timevalue
        "ZELLZEIGER",               //   79 Cellpointer
        NULL,                       //   80 Sum()
        NULL,                       //   81 Avg()
        NULL,                       //   82 Cnt()
        NULL,                       //   83 Min()
        NULL,                       //   84 Max()
        NULL,                       //   85 Vlookup()
        NULL,                       //   86 Npv()
        NULL,                       //   87 Var()
        NULL,                       //   88 Std()
        NULL,                       //   89 Irr()
        NULL,                       //   90 Hlookup()
        NULL,                       //   91 XlfDsum
        NULL,                       //   92 XlfDaverage
        NULL,                       //   93 XlfDcount
        NULL,                       //   94 XlfDmin
        NULL,                       //   95 XlfDmax
        NULL,                       //   96 XlfDvar
        NULL,                       //   97 XlfDstdev
        NULL,                       //   98 Index()
        NULL,                       //   99 Cols()
        NULL,                       //  100 Rows()
        NULL,                       //  101 Repeat()
        NULL,                       //  102 Upper()
        NULL,                       //  103 Lower()
        NULL,                       //  104 Left()
        NULL,                       //  105 Right()
        NULL,                       //  106 Replace()
        NULL,                       //  107 Proper()
        "ZELLE",                    //  108 Cell()
        NULL,                       //  109 Trim()
        NULL,                       //  110 Clean()
        "F",                        //  111 F()     (Excel: T()?)
        "W",                        //  112 W()
        NULL,                       //  113 Exact()
        NULL,                       //  114 Call()
        NULL,                       //  115 @@()
        NULL,                       //  116 Rate()
        "ANN",                      //  117 Term()
        NULL,                       //  118 Cterm()
        NULL,                       //  119 Sln()
        NULL,                       //  120 Syd(), Soy()
        NULL,                       //  121 Ddb()
        "SplFunc",                  //  122 Splfunc
        "BLAETTER",                 //  123 Sheets
        "INFO",                     //  124 Info
        NULL,                       //  125 Sumproduct
        "ISTBEREICH",               //  126 Isrange
        NULL,                       //  127 Dget
        "DABFRAGE",                 //  128 Dquery
        "KOORD",                    //  129 Coord
        NULL,                       //  130 Reserved (internal)
        NULL,                       //  131 Today
        NULL,                       //  132 Vdb
        NULL,                       //  133 Dvars
        NULL,                       //  134 Dstds
        NULL,                       //  135 Vars
        NULL,                       //  136 Stds
        NULL,                       //  137 D360
        NULL,                       //  138 Reserved (internal)
        NULL,                       //  139 Isapp
        "ISTDEFZUS",                //  140 Isaaf
        NULL,                       //  141 Weekday
        NULL,                       //  142 Datedif
        NULL,                       //  143 Rank
        NULL,                       //  144 Numberstring
        "DATUMFOLGE",               //  145 Datestring
        "DEZIMAL",                  //  146 Decimal
        "HEX",                      //  147 Hex
        NULL,                       //  148 Db
        NULL,                       //  149 Pmti
        NULL,                       //  150 Spi
        NULL,                       //  151 Fullp
        NULL,                       //  152 Halfp
        "PURMITTELWERT",            //  153 Pureavg
        "PURANZAHL",                //  154 Purecount
        "PURMAX",                   //  155 Puremax
        "PURMIN",                   //  156 Puremin
        "PURSTDABW",                //  157 Purestd
        "PURVAR",                   //  158 Purevar
        "PURSTDABWP",               //  159 Purestds
        "PURVARP",                  //  160 Purevars
        NULL,                       //  161 Pmt2
        NULL,                       //  162 Pv2
        NULL,                       //  163 Fv2
        NULL,                       //  164 Term2
        NULL,                       //  165 ---                  <-------- neu! - Anzahl ? -
        NULL,                       //  166 D360 (US-Version, ersatzweise wie ander D360-Funktion)
        NULL,                       //  167
        NULL,                       //  168
        NULL,                       //  169
        NULL,                       //  170
        NULL,                       //  171
        NULL,                       //  172
        NULL,                       //  173
        NULL,                       //  174
        NULL,                       //  175
        NULL,                       //  176
        NULL,                       //  177
        NULL,                       //  178
        NULL,                       //  179
        NULL,                       //  180
        NULL,                       //  181
        NULL,                       //  182
        NULL,                       //  183
        NULL,                       //  184
        NULL,                       //  185
        NULL,                       //  186
        NULL,                       //  187
        NULL,                       //  188
        NULL,                       //  189
        NULL,                       //  190
        NULL,                       //  191
        NULL,                       //  192
        NULL,                       //  193
        NULL,                       //  194
        NULL,                       //  195
        NULL,                       //  196
        NULL,                       //  197
        NULL,                       //  198
        NULL,                       //  199
        NULL,                       //  200
        NULL,                       //  201
        NULL,                       //  202
        NULL,                       //  203
        NULL,                       //  204
        NULL,                       //  205
        NULL,                       //  206 ?
        NULL,                       //  207
        NULL,                       //  208
        NULL,                       //  209
        NULL,                       //  210
        NULL,                       //  211
        NULL,                       //  212
        NULL,                       //  213
        NULL,                       //  214
        NULL,                       //  215
        NULL,                       //  216
        NULL,                       //  217
        NULL,                       //  218
        NULL,                       //  219
        NULL,                       //  220
        NULL,                       //  221
        NULL,                       //  222
        NULL,                       //  223
        NULL,                       //  224
        NULL,                       //  225
        NULL,                       //  226
        NULL,                       //  227
        NULL,                       //  228
        NULL,                       //  229
        NULL,                       //  230
        NULL,                       //  231
        NULL,                       //  232
        NULL,                       //  233
        NULL,                       //  234
        NULL,                       //  235
        NULL,                       //  236
        NULL,                       //  237
        NULL,                       //  238
        NULL,                       //  239
        NULL,                       //  240
        NULL,                       //  241
        NULL,                       //  242
        NULL,                       //  243
        NULL,                       //  244
        NULL,                       //  245
        NULL,                       //  246
        NULL,                       //  247
        NULL,                       //  248
        NULL,                       //  249
        NULL,                       //  250
        NULL,                       //  251
        NULL,                       //  252
        NULL,                       //  253
        NULL,                       //  254
        NULL                        //  255 ?
    };

    return pNames[ n ];
}


static DefTokenId lcl_KnownAddIn( const OString& rTest )
{
    DefTokenId  eId = ocNoName;

    if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("FACT")))
            eId = ocFact;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("ISEMPTY")))
            eId=ocIsEmpty;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("DEGTORAD")))
            eId=ocRad;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("RADTODEG")))
            eId=ocDeg;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("SIGN")))
            eId=ocPlusMinus;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("ACOSH")))
            eId=ocArcCosHyp;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("ACOTH")))
            eId=ocArcCotHyp;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("ASINH")))
            eId=ocArcSinHyp;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("ATANH")))
            eId=ocArcTanHyp;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("COSH")))
            eId=ocCosHyp;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("COTH")))
            eId=ocCotHyp;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("SINH")))
            eId=ocSinHyp;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("TANH")))
            eId=ocTanHyp;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("EVEN")))
            eId=ocIsEven;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("ODD")))
            eId=ocIsOdd;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("ACOT")))
            eId=ocArcCot;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("COT")))
            eId=ocCot;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("TRUNC")))
            eId=ocTrunc;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("GEOMEAN")))
            eId=ocGeoMean;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("HARMEAN")))
            eId=ocHarMean;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("CORREL")))
            eId=ocCorrel;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("MEDIAN")))
            eId=ocMedian;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("COV")))
            eId=ocCovar;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("SKEWNESS")))
            eId=ocSchiefe;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("CHITEST")))
            eId=ocChiTest;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("FTEST")))
            eId=ocFTest;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("AVEDEV")))
            eId=ocAveDev;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("PRODUCT")))
            eId=ocProduct;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("PERMUT")))
            eId=ocVariationen;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("GAMMALN")))
            eId=ocGammaLn;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("POISSON")))
            eId=ocPoissonDist;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("NORMAL")))
            eId=ocNormDist;
    else if (rTest.equalsL(RTL_CONSTASCII_STRINGPARAM("CRITBINOMIAL")))
            eId=ocKritBinom;
    return eId;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
