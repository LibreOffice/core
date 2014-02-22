/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
            const OString s("@<<@123>>");

            t = pExtString;

            sal_Int32 n = t.indexOf(s);
            if( n != -1 )
                t = t.copy(n + s.getLength());

            t = comphelper::string::stripEnd(t, '(');

            eOc = lcl_KnownAddIn( t );

            if( eOc == ocNoName )
                t = OString("L123_") + t;
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
        {
            if (nAnz >= 1)
                IncToken( eParam[ nAnz - 1 ] );
        }
            break;
        case ocFind:
        case ocHLookup:
        case ocVLookup:
        {
            IncToken( eParam[ 0 ] );
        }
            break;
        case ocMid:
        case ocReplace:
        {
            if (nAnz >= 2)
                IncToken( eParam[ nAnz - 2 ] );
        }
            break;
        case ocZins:
        {
            
            OSL_ENSURE( nAnz == 3,
                "*LotusToSc::DoFunc(): ZINS() hat 3 Parameter!" );
            nAnz = 4;
            eParam[ 3 ] = eParam[ 0 ];  
            eParam[ 0 ] = eParam[ 2 ];  
            NegToken( eParam[ 1 ] );    
            eParam[ 2 ] = n0Token;      
        }
            break;
        default:;
    }

    if( !bAddIn )
        aPool << eOc;

    aPool << ocOpen;

    if( nAnz > 0 )
    {
            

        sal_Int16 nLast = nAnz - 1;

        if( eOc == ocRMZ )
        {   
            
            OSL_ENSURE( nAnz == 3,
                "+LotusToSc::DoFunc(): ocRMZ hat genau 3 Parameter!" );
            aPool << eParam[ 1 ] << ocSep << eParam[ 0 ] << ocSep
                << ocNegSub << eParam[ 2 ];
        }
        else
        {   
            
            aPool << eParam[ nLast ];

            sal_Int16 nNull = -1;   
            for( nLauf = nLast - 1 ; nLauf >= 0 ; nLauf-- )
            {
                if( nLauf != nNull )
                    aPool << ocSep << eParam[ nLauf ];
            }
        }
    }


    
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

    aPool >> aStack;

    if( bNeg )
    {
        aPool << ocOpen << ocSub << aStack << ocClose;
        aPool >> aStack;
    }
}


void LotusToSc::LotusRelToScRel( sal_uInt16 nCol, sal_uInt16 nRow, ScSingleRefData& rSRD )
{
    
    if( nCol & 0x8000 )
    {
        if( nCol & 0x0080 )
            nCol |= 0xFF00;
        else
            nCol &= 0x00FF;
        
        rSRD.SetRelCol(static_cast<SCCOL>(static_cast<sal_Int16>(nCol)));
    }
    else
    {
        rSRD.SetAbsCol(static_cast<SCCOL>(nCol & 0x00FF));
    }

    
    if( nRow & 0x8000 )
    {
        rSRD.SetRowRel(true);
        
        switch( eTyp )
        {
            
            
            
            case eWK_1:
                if( nRow & 0x0400 )
                    nRow |= 0xF800;
                else
                    nRow &= 0x07FF;
                break;
            
            
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
            
            
            
            case eWK_1:
                nRow &= 0x07FF;
                break;
            
            
            case eWK_2:
                nRow &= 0x3FFF;
                break;
            default:
                OSL_FAIL( "*LotusToSc::LotusRelToScRel(): etwas vergessen...?" );
        }
    }

    if( rSRD.IsRowRel() )
        
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


LotusToSc::LotusToSc( SvStream &rStream, rtl_TextEncoding e, bool b ) :
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
    {   
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

    while( eType )      
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
                    nBytesLeft = 0; 

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
                OUString  aTmp(ScfTools::read_zeroTerminated_uInt8s_ToOUString(aIn, nBytesLeft, eSrcChar));
                aStack << aPool.Store( aTmp );
            }
                break;
            case FT_NOP:
                break;
            
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
                OUString  aTmp(ScfTools::read_zeroTerminated_uInt8s_ToOUString(aIn, nBytesLeft, eSrcChar));
                if( rRangeNameBufferWK3.FindRel( aTmp, nRngIndex ) )
                    aStack << aPool.Store( nRngIndex );
                else
            {
                    OUString  aText( "NRREF ");
                    aText += aTmp;
                    aStack << aPool.Store( aText );
            }
            }
                break;
            case FT_Absnref:
            {
                OUString aTmp(ScfTools::read_zeroTerminated_uInt8s_ToOUString(aIn, nBytesLeft, eSrcChar));
                if( rRangeNameBufferWK3.FindAbs( aTmp, nRngIndex ) )
                    aStack << aPool.Store( nRngIndex );
                else
                {
                    OUString  aText( "ABSNREF " );
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
        aIn.SeekRel( nBytesLeft );  

    rRest = 0;

    return ConvOK;
}


FUNC_TYPE LotusToSc::IndexToType( sal_uInt8 nIndex )
{
    static const FUNC_TYPE pType[ 256 ] =
    {                       
        FT_ConstFloat,      
        FT_Variable,        
        FT_Range,           
        FT_Return,          
        FT_Braces,          
        FT_ConstInt,        
        FT_ConstString,     
        FT_NOP,             
        FT_Neg,             
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_FuncFix1,        
        FT_NOP,             
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_FuncFix0,        
        FT_FuncFix0,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix0,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncVar,         
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix0,        
        FT_FuncFix0,        
        FT_FuncFix0,        
        FT_FuncFix3,        
        FT_FuncFix0,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncFix3,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncFix3,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix3,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncFix3,        
        FT_FuncFix2,        
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncFix2,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncFix2,        
        FT_FuncFix4,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_NotImpl,         
        FT_FuncFix1,        
        FT_FuncFix3,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix3,        
        FT_FuncFix4,        
        FT_FuncFix4,        
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_FuncVar,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_FuncVar,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_FuncVar,         
    };
    return pType[ nIndex ];
}


DefTokenId LotusToSc::IndexToToken( sal_uInt8 nIndex )
{
    static const DefTokenId pToken[ 256 ] =
    {                       
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocNegSub,           
        ocAdd,              
        ocSub,              
        ocMul,              
        ocDiv,              
        ocPow,              
        ocEqual,            
        ocNotEqual,         
        ocLessEqual,        
        ocGreaterEqual,     
        ocLess,             
        ocGreater,          
        ocAnd,              
        ocOr,               
        ocNot,              
        ocPush,             
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNotAvail,         
        ocNoName,           
        ocAbs,              
        ocInt,              
        ocSqrt,             
        ocLog10,            
        ocLn,               
        ocPi,               
        ocSin,              
        ocCos,              
        ocTan,              
        ocArcTan2,          
        ocArcTan,           
        ocArcSin,           
        ocArcCos,           
        ocExp,              
        ocMod,              
        ocChose,            
        ocIsNA,             
        ocIsError,          
        ocFalse,            
        ocTrue,             
        ocRandom,           
        ocGetDate,          
        ocGetActDate,       
        ocRMZ,              
        ocBW,               
        ocZW,               
        ocIf,               
        ocGetDay,           
        ocGetMonth,         
        ocGetYear,          
        ocRound,            
        ocGetTime,          
        ocGetHour,          
        ocGetMin,           
        ocGetSec,           
        ocIsValue,          
        ocIsString,         
        ocLen,              
        ocValue,            
        ocFixed,            
        ocMid,              
        ocChar,             
        ocCode,             
        ocFind,             
        ocGetDateValue,     
        ocGetTimeValue,     
        ocNoName,           
        ocSum,              
        ocAverage,          
        ocCount,            
        ocMin,              
        ocMax,              
        ocVLookup,          
        ocNPV,              
        ocVar,              
        ocNormDist,         
        ocIRR,              
        ocHLookup,          
        ocDBSum,            
        ocDBAverage,        
        ocDBCount,          
        ocDBMin,            
        ocDBMax,            
        ocDBVar,            
        ocDBStdDev,         
        ocIndex,            
        ocColumns,          
        ocRows,             
        ocRept,             
        ocUpper,            
        ocLower,            
        ocLeft,             
        ocRight,            
        ocReplace,          
        ocPropper,          
        ocNoName,           
        ocTrim,             
        ocClean,            
        ocFalse,            
        ocTrue,             
        ocExact,            
        ocNoName,           
        ocIndirect,         
        ocZins,             
        ocNoName,           
        ocNoName,           
        ocLIA,              
        ocDIA,              
        ocGDA,              
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName            
    };

    return pToken[ nIndex ];
}


FUNC_TYPE LotusToSc::IndexToTypeWK123( sal_uInt8 nIndex )
{
    static const FUNC_TYPE pType[ 256 ] =
    {                       
        FT_Const10Float,    
        FT_Cref,            
        FT_Rref,            
        FT_Return,          
        FT_Braces,          
        FT_Snum,            
        FT_ConstString,     
        FT_Nrref,           
        FT_Absnref,         
        FT_Erref,           
        FT_Ecref,           
        FT_Econstant,       
        FT_NotImpl,         
        FT_NotImpl,         
        FT_Neg,             
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_Op,              
        FT_FuncFix1,        
        FT_NOP,             
        FT_Op,              
        FT_FuncFix0,        
        FT_FuncFix0,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix0,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncVar,         
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix0,        
        FT_FuncFix0,        
        FT_FuncFix0,        
        FT_FuncFix3,        
        FT_FuncFix0,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncFix3,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncFix3,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix3,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncFix3,        
        FT_FuncFix2,        
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncFix2,        
        FT_FuncFix3,        
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncFix2,        
        FT_FuncFix4,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix2,        
        FT_NotImpl,         
        FT_FuncFix1,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix4,        
        FT_FuncFix4,        
        FT_Splfunc,         
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncVar,         
        FT_FuncFix1,        
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncFix4,        
        FT_NOP,             
        FT_FuncFix0,        
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncFix2,        
        FT_NOP,             
        FT_FuncFix0,        
        FT_FuncVar,         
        FT_FuncFix1,        
        FT_FuncFix3,        
        FT_FuncVar,         
        FT_FuncFix2,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncFix4,        
        FT_FuncFix4,        
        FT_FuncFix4,        
        FT_FuncFix1,        
        FT_FuncFix1,        
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncVar,         
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_FuncFix3,        
        FT_NotImpl,         
        FT_FuncFix2,        
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_FuncVar,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
        FT_NotImpl,         
    };
    return pType[ nIndex ];
}


DefTokenId LotusToSc::IndexToTokenWK123( sal_uInt8 nIndex )
{
    static const DefTokenId pToken[ 256 ] =
    {                       
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocPush,             
        ocNegSub,           
        ocAdd,              
        ocSub,              
        ocMul,              
        ocDiv,              
        ocPow,              
        ocEqual,            
        ocNotEqual,         
        ocLessEqual,        
        ocGreaterEqual,     
        ocLess,             
        ocGreater,          
        ocAnd,              
        ocOr,               
        ocNot,              
        ocPush,             
        ocAmpersand,        
        ocNotAvail,         
        ocNoName,           
        ocAbs,              
        ocInt,              
        ocSqrt,             
        ocLog10,            
        ocLn,               
        ocPi,               
        ocSin,              
        ocCos,              
        ocTan,              
        ocArcTan2,          
        ocArcTan,           
        ocArcSin,           
        ocArcCos,           
        ocExp,              
        ocMod,              
        ocChose,            
        ocIsNA,             
        ocIsError,          
        ocFalse,            
        ocTrue,             
        ocRandom,           
        ocGetDate,          
        ocGetActDate,       
        ocRMZ,              
        ocBW,               
        ocZW,               
        ocIf,               
        ocGetDay,           
        ocGetMonth,         
        ocGetYear,          
        ocRound,            
        ocGetTime,          
        ocGetHour,          
        ocGetMin,           
        ocGetSec,           
        ocIsValue,          
        ocIsString,         
        ocLen,              
        ocValue,            
        ocFixed,            
        ocMid,              
        ocChar,             
        ocCode,             
        ocFind,             
        ocGetDateValue,     
        ocGetTimeValue,     
        ocNoName,           
        ocSum,              
        ocAverage,          
        ocCount,            
        ocMin,              
        ocMax,              
        ocVLookup,          
        ocNPV,              
        ocVar,              
        ocStDev,            
        ocIRR,              
        ocHLookup,          
        ocDBSum,            
        ocDBAverage,        
        ocDBCount,          
        ocDBMin,            
        ocDBMax,            
        ocDBVar,            
        ocDBStdDev,         
        ocIndex,            
        ocColumns,          
        ocRows,             
        ocRept,             
        ocUpper,            
        ocLower,            
        ocLeft,             
        ocRight,            
        ocReplace,          
        ocPropper,          
        ocNoName,           
        ocTrim,             
        ocClean,            
        ocNoName,           
        ocNoName,           
        ocExact,            
        ocNoName,           
        ocIndirect,         
        ocZins,             
        ocNoName,           
        ocNoName,           
        ocLIA,              
        ocDIA,              
        ocGDA,              
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocSumProduct,       
        ocNoName,           
        ocDBGet,            
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocGetActDate,       
        ocNoName,           
        ocDBVarP,           
        ocDBStdDevP,        
        ocVarP,             
        ocStDevP,           
        ocGetDiffDate360,   
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocGetDayOfWeek,     
        ocGetDiffDate,      
        ocRank,             
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocCount2,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocGetDiffDate360,   
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName,           
        ocNoName            
    };

    return pToken[ nIndex ];
}




const sal_Char* GetAddInName( const sal_uInt8 n )
{
    static const sal_Char*  pNames[ 256 ] =
    {
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        "ZELLZEIGER",               
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        "ZELLE",                    
        NULL,                       
        NULL,                       
        "F",                        
        "W",                        
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        "ANN",                      
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        "SplFunc",                  
        "BLAETTER",                 
        "INFO",                     
        NULL,                       
        "ISTBEREICH",               
        NULL,                       
        "DABFRAGE",                 
        "KOORD",                    
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        "ISTDEFZUS",                
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        "DATUMFOLGE",               
        "DEZIMAL",                  
        "HEX",                      
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        "PURMITTELWERT",            
        "PURquantity",                
        "PURMAX",                   
        "PURMIN",                   
        "PURSTDABW",                
        "PURVAR",                   
        "PURSTDABWP",               
        "PURVARP",                  
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL,                       
        NULL                        
    };

    return pNames[ n ];
}


static DefTokenId lcl_KnownAddIn( const OString& rTest )
{
    DefTokenId  eId = ocNoName;

    if (rTest == "FACT")
            eId = ocFact;
    else if (rTest == "ISEMPTY")
            eId=ocIsEmpty;
    else if (rTest == "DEGTORAD")
            eId=ocRad;
    else if (rTest == "RADTODEG")
            eId=ocDeg;
    else if (rTest == "SIGN")
            eId=ocPlusMinus;
    else if (rTest == "ACOSH")
            eId=ocArcCosHyp;
    else if (rTest == "ACOTH")
            eId=ocArcCotHyp;
    else if (rTest == "ASINH")
            eId=ocArcSinHyp;
    else if (rTest == "ATANH")
            eId=ocArcTanHyp;
    else if (rTest == "COSH")
            eId=ocCosHyp;
    else if (rTest == "COTH")
            eId=ocCotHyp;
    else if (rTest == "SINH")
            eId=ocSinHyp;
    else if (rTest == "TANH")
            eId=ocTanHyp;
    else if (rTest == "EVEN")
            eId=ocIsEven;
    else if (rTest == "ODD")
            eId=ocIsOdd;
    else if (rTest == "ACOT")
            eId=ocArcCot;
    else if (rTest == "COT")
            eId=ocCot;
    else if (rTest == "TRUNC")
            eId=ocTrunc;
    else if (rTest == "GEOMEAN")
            eId=ocGeoMean;
    else if (rTest == "HARMEAN")
            eId=ocHarMean;
    else if (rTest == "CORREL")
            eId=ocCorrel;
    else if (rTest == "MEDIAN")
            eId=ocMedian;
    else if (rTest == "COV")
            eId=ocCovar;
    else if (rTest == "SKEWNESS")
            eId=ocSchiefe;
    else if (rTest == "CHITEST")
            eId=ocChiTest;
    else if (rTest == "FTEST")
            eId=ocFTest;
    else if (rTest == "AVEDEV")
            eId=ocAveDev;
    else if (rTest == "PRODUCT")
            eId=ocProduct;
    else if (rTest == "PERMUT")
            eId=ocVariationen;
    else if (rTest == "GAMMALN")
            eId=ocGammaLn;
    else if (rTest =="POISSON")
            eId=ocPoissonDist;
    else if (rTest == "NORMAL")
            eId=ocNormDist;
    else if (rTest == "CRITBINOMIAL")
            eId=ocKritBinom;
    return eId;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
