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

#include <rtl/ustrbuf.hxx>
#include <vcl/errcode.hxx>
#include <vcl/svapp.hxx>

#include <basic/sbx.hxx>
#include <basic/sberrors.hxx>
#include <basic/sbxvar.hxx>
#include "sbxconv.hxx"


static OUString ImpCurrencyToString( sal_Int64 rVal )
{
    bool isNeg = ( rVal < 0 );
    sal_Int64 absVal = isNeg ? -rVal : rVal;

    sal_Unicode const cDecimalSep = '.';

    OUString aAbsStr = OUString::number( absVal );
    OUStringBuffer aBuf;

    sal_Int32 initialLen = aAbsStr.getLength();

    bool bLessThanOne = false;
    if ( initialLen  <= 4 )  // if less the 1
        bLessThanOne = true;

    sal_Int32 nCapacity = 6; // minimum e.g. 0.0000

    if ( !bLessThanOne )
    {
        nCapacity = initialLen + 1;
    }

    if ( isNeg )
        ++nCapacity;

    aBuf.setLength( nCapacity );


    sal_Int32 nDigitCount = 0;
    sal_Int32 nInsertIndex = nCapacity - 1;
    sal_Int32 nEndIndex = isNeg ? 1 : 0;

    for ( sal_Int32 charCpyIndex = aAbsStr.getLength() - 1; nInsertIndex >= nEndIndex;  ++nDigitCount )
    {
        if ( nDigitCount == 4 )
            aBuf[nInsertIndex--] = cDecimalSep;
        if ( nDigitCount < initialLen )
            aBuf[nInsertIndex--] = aAbsStr[ charCpyIndex-- ];
        else
        // Handle leading 0's to right of decimal point
        // Note: in VBA the stringification is a little more complex
        // but more natural as only the necessary digits
        // to the right of the decimal places are displayed
        // It would be great to conditionally be able to display like that too

        // Val   OOo (Cur)  VBA (Cur)
        // ---   ---------  ---------
        // 0     0.0000     0
        // 0.1   0.1000     0.1

            aBuf[nInsertIndex--] = '0';
    }
    if ( isNeg )
            aBuf[nInsertIndex] = '-';

    aAbsStr = aBuf.makeStringAndClear();
    return aAbsStr;
}


static sal_Int64 ImpStringToCurrency( const OUString &rStr )
{

    sal_Int32   nFractDigit = 4;

    sal_Unicode const cDeciPnt = '.';
    sal_Unicode const c1000Sep = ',';

    // lets use the existing string number conversions
    // there is a performance impact here ( multiple string copies )
    // but better I think than a home brewed string parser, if we need a parser
    // we should share some existing ( possibly from calc is there a currency
    // conversion there ? #TODO check )

    OUString sTmp( rStr.trim() );
    const sal_Unicode* p =  sTmp.getStr();

    // normalise string number by removing thousand & decimal point separators
    OUStringBuffer sNormalisedNumString( sTmp.getLength() +  nFractDigit );

    if ( *p == '-'  || *p == '+' )
        sNormalisedNumString.append( *p );

    while ( *p >= '0' && *p <= '9' )
    {
        sNormalisedNumString.append( *p++ );
        // #TODO in vba mode set runtime error when a space ( or other )
        // illegal character is found
        if( *p == c1000Sep )
            p++;
    }

    bool bRoundUp = false;

    if( *p == cDeciPnt )
    {
        p++;
        while( nFractDigit && *p >= '0' && *p <= '9' )
        {
            sNormalisedNumString.append( *p++ );
            nFractDigit--;
        }
        // Consume trailing content
        if ( p != nullptr )
        {
            // Round up if necessary
            if( *p >= '5' && *p <= '9' )
                bRoundUp = true;
            while( *p >= '0' && *p <= '9' )
                p++;
        }

    }
    // can we raise error here ? ( previous behaviour was more forgiving )
    // so... not sure that could break existing code, let's see if anyone
    // complains.

    if ( p != sTmp.getStr() + sTmp.getLength() )
        SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
    while( nFractDigit )
    {
        sNormalisedNumString.append( '0' );
        nFractDigit--;
    }

    sal_Int64 result = sNormalisedNumString.makeStringAndClear().toInt64();

    if ( bRoundUp )
        ++result;
    return result;
}


sal_Int64 ImpGetCurrency( const SbxValues* p )
{
    SbxValues   aTmp;
    sal_Int64  nRes;
start:
    switch( +p->eType )
    {
        case SbxERROR:
        case SbxNULL:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
            nRes = 0; break;
        case SbxEMPTY:
            nRes = 0; break;
        case SbxCURRENCY:
            nRes = p->nInt64; break;
        case SbxBYTE:
            nRes = sal_Int64(CURRENCY_FACTOR) * static_cast<sal_Int64>(p->nByte);
            break;
        case SbxCHAR:
            nRes = sal_Int64(CURRENCY_FACTOR) * reinterpret_cast<sal_Int64>(p->pChar);
            break;
        case SbxBOOL:
        case SbxINTEGER:
            nRes = sal_Int64(CURRENCY_FACTOR) * static_cast<sal_Int64>(p->nInteger);
            break;
        case SbxUSHORT:
            nRes = sal_Int64(CURRENCY_FACTOR) * static_cast<sal_Int64>(p->nUShort);
            break;
        case SbxLONG:
            nRes = sal_Int64(CURRENCY_FACTOR) * static_cast<sal_Int64>(p->nLong);
            break;
        case SbxULONG:
            nRes = sal_Int64(CURRENCY_FACTOR) * static_cast<sal_Int64>(p->nULong);
            break;

        case SbxSALINT64:
        {
            nRes = p->nInt64 * CURRENCY_FACTOR; break;
#if 0
            // Huh, is the 'break' above intentional? That means this
            // is unreachable, obviously. Avoid warning by ifdeffing
            // this out for now. Do not delete this #if 0 block unless
            // you know for sure the 'break' above is intentional.
            if ( nRes > SAL_MAX_INT64 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SAL_MAX_INT64;
            }
#endif
        }
        case SbxSALUINT64:
            nRes = p->nInt64 * CURRENCY_FACTOR; break;
#if 0
            // As above
            if ( nRes > SAL_MAX_INT64 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SAL_MAX_INT64;
            }
            else if ( nRes < SAL_MIN_INT64 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SAL_MIN_INT64;
            }
            break;
#endif
//TODO: bring back SbxINT64 types here for limits -1 with flag value at SAL_MAX/MIN
        case SbxSINGLE:
            if( p->nSingle * CURRENCY_FACTOR + 0.5 > float(SAL_MAX_INT64)
             || p->nSingle * CURRENCY_FACTOR - 0.5 < float(SAL_MIN_INT64) )
            {
                nRes = SAL_MAX_INT64;
                if( p->nSingle * CURRENCY_FACTOR - 0.5 < float(SAL_MIN_INT64) )
                    nRes = SAL_MIN_INT64;
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW );
                break;
            }
            nRes = ImpDoubleToCurrency( static_cast<double>(p->nSingle) );
            break;

        case SbxDATE:
        case SbxDOUBLE:
            if( p->nDouble * CURRENCY_FACTOR + 0.5 > double(SAL_MAX_INT64)
             || p->nDouble * CURRENCY_FACTOR - 0.5 < double(SAL_MIN_INT64) )
            {
                nRes = SAL_MAX_INT64;
                if( p->nDouble * CURRENCY_FACTOR - 0.5 < double(SAL_MIN_INT64) )
                    nRes = SAL_MIN_INT64;
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW );
                break;
            }
            nRes = ImpDoubleToCurrency( p->nDouble );
            break;

        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            double d = 0.0;
            if( p->pDecimal )
                p->pDecimal->getDouble( d );
            nRes = ImpDoubleToCurrency( d );
            break;
            }


        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pOUString )
                nRes=0;
            else
                nRes = ImpStringToCurrency( *p->pOUString );
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                nRes = pVal->GetCurrency();
            else
            {
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
                nRes=0;
            }
            break;
        }

        case SbxBYREF | SbxCHAR:
            nRes = sal_Int64(CURRENCY_FACTOR) * static_cast<sal_Int64>(*p->pChar);
            break;
        case SbxBYREF | SbxBYTE:
            nRes = sal_Int64(CURRENCY_FACTOR) * static_cast<sal_Int64>(*p->pByte);
            break;
        case SbxBYREF | SbxBOOL:
        case SbxBYREF | SbxINTEGER:
            nRes = sal_Int64(CURRENCY_FACTOR) * static_cast<sal_Int64>(*p->pInteger);
            break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            nRes = sal_Int64(CURRENCY_FACTOR) * static_cast<sal_Int64>(*p->pUShort);
            break;

        // from here on had to be tested
        case SbxBYREF | SbxLONG:
            aTmp.nLong = *p->pLong; goto ref;
        case SbxBYREF | SbxULONG:
            aTmp.nULong = *p->pULong; goto ref;
        case SbxBYREF | SbxSINGLE:
            aTmp.nSingle = *p->pSingle; goto ref;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            aTmp.nDouble = *p->pDouble; goto ref;
        case SbxBYREF | SbxCURRENCY:
        case SbxBYREF | SbxSALINT64:
            aTmp.nInt64 = *p->pnInt64; goto ref;
        case SbxBYREF | SbxSALUINT64:
            aTmp.uInt64 = *p->puInt64; goto ref;
        ref:
            aTmp.eType = SbxDataType( p->eType & ~SbxBYREF );
            p = &aTmp; goto start;

        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
            nRes=0;
    }
    return nRes;
}


void ImpPutCurrency( SbxValues* p, const sal_Int64 r )
{
    SbxValues aTmp;
start:
    switch( +p->eType )
    {
        // Here are tests necessary
        case SbxCHAR:
            aTmp.pChar = &p->nChar; goto direct;
        case SbxBYTE:
            aTmp.pByte = &p->nByte; goto direct;
        case SbxINTEGER:
        case SbxBOOL:
            aTmp.pInteger = &p->nInteger; goto direct;
        case SbxLONG:
            aTmp.pLong = &p->nLong; goto direct;
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort; goto direct;
        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        // from here no longer
        case SbxSINGLE:
            p->nSingle = static_cast<float>( r / CURRENCY_FACTOR ); break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble =  ImpCurrencyToDouble( r ); break;
        case SbxSALUINT64:
            p->uInt64 = r / CURRENCY_FACTOR; break;
        case SbxSALINT64:
            p->nInt64 = r / CURRENCY_FACTOR; break;

        case SbxCURRENCY:
            p->nInt64 = r; break;

        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            SbxDecimal* pDec = ImpCreateDecimal( p );
            if( !pDec->setDouble( ImpCurrencyToDouble( r ) / CURRENCY_FACTOR ) )
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW );
            break;
            }
        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pOUString )
                p->pOUString = new OUString;

            *p->pOUString = ImpCurrencyToString( r );
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                pVal->PutCurrency( r );
            else
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
        {
            sal_Int64 val = r / CURRENCY_FACTOR;
            if( val > SbxMAXCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); val = SbxMAXCHAR;
            }
            else if( val < SbxMINCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); val = SbxMINCHAR;
            }
            *p->pChar = static_cast<sal_Unicode>(val); break;
        }
        case SbxBYREF | SbxBYTE:
        {
            sal_Int64 val = r / CURRENCY_FACTOR;
            if( val > SbxMAXBYTE )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); val = SbxMAXBYTE;
            }
            else if( val < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); val = 0;
            }
            *p->pByte = static_cast<sal_uInt8>(val); break;
        }
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
        {
            sal_Int64 val = r / CURRENCY_FACTOR;
            if( r > SbxMAXINT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); val = SbxMAXINT;
            }
            else if( r < SbxMININT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); val = SbxMININT;
            }
            *p->pInteger = static_cast<sal_uInt16>(val); break;
        }
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
        {
            sal_Int64 val = r / CURRENCY_FACTOR;
            if( val > SbxMAXUINT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); val = SbxMAXUINT;
            }
            else if( val < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); val = 0;
            }
            *p->pUShort = static_cast<sal_uInt16>(val); break;
        }
        case SbxBYREF | SbxLONG:
        {
            sal_Int64 val = r / CURRENCY_FACTOR;
            if( val > SbxMAXLNG )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); val = SbxMAXLNG;
            }
            else if( val < SbxMINLNG )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); val = SbxMINLNG;
            }
            *p->pLong = static_cast<sal_Int32>(val); break;
        }
        case SbxBYREF | SbxULONG:
        {
            sal_Int64 val = r / CURRENCY_FACTOR;
            if( val > SbxMAXULNG )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); val = SbxMAXULNG;
            }
            else if( val < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); val = 0;
            }
            *p->pULong = static_cast<sal_uInt32>(val); break;
        }
        case SbxBYREF | SbxCURRENCY:
            *p->pnInt64 = r; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = r / CURRENCY_FACTOR; break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = static_cast<sal_uInt64>(r) / CURRENCY_FACTOR; break;
        case SbxBYREF | SbxSINGLE:
            p->nSingle = static_cast<float>( r / CURRENCY_FACTOR ); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = ImpCurrencyToDouble( r ); break;
        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
