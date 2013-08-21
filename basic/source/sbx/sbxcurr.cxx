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


#include <tools/errcode.hxx>
#include <vcl/svapp.hxx>        // for SvtSysLocale

#include <basic/sbx.hxx>
#include <basic/sbxvar.hxx>
#include "sbxconv.hxx"


static OUString ImpCurrencyToString( const sal_Int64 &rVal )
{
    bool isNeg = ( rVal < 0 );
    sal_Int64 absVal = isNeg ? -rVal : rVal;

    sal_Unicode cDecimalSep = '.';
#ifdef MAYBEFUTURE
    sal_Unicode cThousandSep = ',';
    ImpGetIntntlSep( cDecimalSep, cThousandSep );
#endif

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
#ifdef MAYBEFUTURE
        if ( initialLen > 5 )
        {
            sal_Int32 nThouSeparators = ( initialLen - 5 ) / 3;
            nCapacity += nThouSeparators;
        }
#endif
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
#ifdef MAYBEFUTURE
        if ( nDigitCount > 4 && ! ( ( nDigitCount - 4  ) % 3) )
            aBuf[nInsertIndex--] = cThousandSep;
#endif
        if ( nDigitCount < initialLen )
            aBuf[nInsertIndex--] = aAbsStr[ charCpyIndex-- ];
        else
        // Handle leading 0's to right of decimal point
        // Note: in VBA the stringification is a little more complex
        // but more natural as only the necessary digits
        // to the right of the decimal places are displayed
        // It would be great to conditionally be able to display like that too
        //
        // Val   OOo (Cur)  VBA (Cur)
        // ---   ---------  ---------
        // 0     0.0000     0
        // 0.1   0.1000     0.1

            aBuf[nInsertIndex--] = (sal_Unicode)'0';
    }
    if ( isNeg )
            aBuf[nInsertIndex] = (sal_Unicode)'-';

    aAbsStr = aBuf.makeStringAndClear();
    return aAbsStr;
}


static sal_Int64 ImpStringToCurrency( const OUString &rStr )
{

    sal_Int32   nFractDigit = 4;

    sal_Unicode cDeciPnt = sal_Unicode('.');
    sal_Unicode c1000Sep = sal_Unicode(',');

#ifdef MAYBEFUTURE
    sal_Unicode cLocaleDeciPnt, cLocale1000Sep;
    ImpGetIntntlSep( cLocaleDeciPnt, cLocale1000Sep );

        // score each set of separators (Locale and Basic) on total number of matches
        // if one set has more matches use that set
        // if tied use the set with the only or rightmost decimal separator match
        // currency is fixed pt system: usually expect the decimal pt, 1000sep may occur
    sal_Int32 LocaleScore = 0;
    sal_Int32 LocaleLastDeci = -1;
    sal_Int32 LOBasicScore = 0;
    sal_Int32 LOBasicLastDeci = -1;

    for( int idx=0; idx<rStr.getLength(); idx++ )
    {
        if ( *(p+idx) == cLocaleDeciPnt )
        {
            LocaleScore++;
            LocaleLastDeci = idx;
        }
        if ( *(p+idx) == cLocale1000Sep )
            LocaleScore++;

        if ( *(p+idx) == cDeciPnt )
        {
            LOBasicScore++;
            LOBasicLastDeci = idx;
        }
        if ( *(p+idx) == c1000Sep )
            LOBasicScore++;
    }
    if ( ( LocaleScore > LOBasicScore )
       ||( LocaleScore = LOBasicScore && LocaleLastDeci > LOBasicLastDeci ) )
    {
        cDeciPnt = cLocaleDeciPnt;
        c1000Sep = cLocale1000Sep;
    }
#endif

    // lets use the existing string number conversions
    // there is a performance impact here ( multiple string copies )
    // but better I think than a home brewed string parser, if we need a parser
    // we should share some existing ( possibly from calc is there a currency
    // conversion there ? #TODO check )

    OUString sTmp( rStr.trim() );
    const sal_Unicode* p =  sTmp.getStr();

    // normalise string number by removeing thousands & decimal point separators
    OUStringBuffer sNormalisedNumString( sTmp.getLength() +  nFractDigit );

    if ( *p == '-'  || *p == '+' )
        sNormalisedNumString.append( *p );

    while ( ( *p >= '0' && *p <= '9' ) )
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
        if ( p != NULL )
        {
            // Round up if necessary
            if( *p >= '5' && *p <= '9' )
                bRoundUp = true;
            while( *p >= '0' && *p <= '9' )
                p++;
        }

    }
    // can we raise error here ? ( previous behaviour was more forgiving )
    // so... not sure that could bread existing code, lets see if anyone
    // complains.

    if ( p != sTmp.getStr() + sTmp.getLength() )
        SbxBase::SetError( SbxERR_CONVERSION );
    while( nFractDigit )
    {
        sNormalisedNumString.append( sal_Unicode('0') );
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
            SbxBase::SetError( SbxERR_CONVERSION );
            nRes = 0; break;
        case SbxEMPTY:
            nRes = 0; break;
        case SbxCURRENCY:
            nRes = p->nInt64; break;
        case SbxBYTE:
            nRes = (sal_Int64)CURRENCY_FACTOR * (sal_Int64)(p->nByte);
            break;
        case SbxCHAR:
            nRes = (sal_Int64)CURRENCY_FACTOR * (sal_Int64)(p->pChar);
            break;
        case SbxBOOL:
        case SbxINTEGER:
            nRes = (sal_Int64)CURRENCY_FACTOR * (sal_Int64)(p->nInteger);
            break;
        case SbxUSHORT:
            nRes = (sal_Int64)CURRENCY_FACTOR * (sal_Int64)(p->nUShort);
            break;
        case SbxLONG:
            nRes = (sal_Int64)CURRENCY_FACTOR * (sal_Int64)(p->nLong);
            break;
        case SbxULONG:
            nRes = (sal_Int64)CURRENCY_FACTOR * (sal_Int64)(p->nULong);
            break;

        case SbxSALINT64:
        {
            nRes = p->nInt64 * CURRENCY_FACTOR; break;
#if 0
            // Huh, is the 'break' above intentional? That means this
            // is unreachable, obviously. Avoid warning by ifdeffing
            // this out for now. Do not delete this #if 0 block unless
            // you know for sure the 'break' above is intentional.
            if ( nRes > SbxMAXSALINT64 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXSALINT64;
            }
#endif
        }
        case SbxSALUINT64:
            nRes = p->nInt64 * CURRENCY_FACTOR; break;
#if 0
            // As above
            if ( nRes > SbxMAXSALINT64 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXSALINT64;
            }
            else if ( nRes < SbxMINSALINT64 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINSALINT64;
            }
            break;
#endif
//TODO: bring back SbxINT64 types here for limits -1 with flag value at SAL_MAX/MIN
        case SbxSINGLE:
            if( p->nSingle * CURRENCY_FACTOR + 0.5 > (float)SAL_MAX_INT64
             || p->nSingle * CURRENCY_FACTOR - 0.5 < (float)SAL_MIN_INT64 )
            {
                nRes = SAL_MAX_INT64;
                if( p->nSingle * CURRENCY_FACTOR - 0.5 < (float)SAL_MIN_INT64 )
                    nRes = SAL_MIN_INT64;
                SbxBase::SetError( SbxERR_OVERFLOW );
                break;
            }
            nRes = ImpDoubleToCurrency( (double)p->nSingle );
            break;

        case SbxDATE:
        case SbxDOUBLE:
            if( p->nDouble * CURRENCY_FACTOR + 0.5 > (double)SAL_MAX_INT64
             || p->nDouble * CURRENCY_FACTOR - 0.5 < (double)SAL_MIN_INT64 )
            {
                nRes = SAL_MAX_INT64;
                if( p->nDouble * CURRENCY_FACTOR - 0.5 < (double)SAL_MIN_INT64 )
                    nRes = SAL_MIN_INT64;
                SbxBase::SetError( SbxERR_OVERFLOW );
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
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetCurrency();
            else
            {
                SbxBase::SetError( SbxERR_NO_OBJECT );
                nRes=0;
            }
            break;
        }

        case SbxBYREF | SbxCHAR:
            nRes = (sal_Int64)CURRENCY_FACTOR * (sal_Int64)(*p->pChar);
            break;
        case SbxBYREF | SbxBYTE:
            nRes = (sal_Int64)CURRENCY_FACTOR * (sal_Int64)(*p->pByte);
            break;
        case SbxBYREF | SbxBOOL:
        case SbxBYREF | SbxINTEGER:
            nRes = (sal_Int64)CURRENCY_FACTOR * (sal_Int64)(*p->pInteger);
            break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            nRes = (sal_Int64)CURRENCY_FACTOR * (sal_Int64)(*p->pUShort);
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
            SbxBase::SetError( SbxERR_CONVERSION );
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
            p->nSingle = (float)( r / CURRENCY_FACTOR ); break;
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
                SbxBase::SetError( SbxERR_OVERFLOW );
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
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutCurrency( r );
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
        {
            sal_Int64 val = r / CURRENCY_FACTOR;
            if( val > SbxMAXCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); val = SbxMAXCHAR;
            }
            else if( val < SbxMINCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); val = SbxMINCHAR;
            }
            *p->pChar = (sal_Unicode) val; break;
        }
        case SbxBYREF | SbxBYTE:
        {
            sal_Int64 val = r / CURRENCY_FACTOR;
            if( val > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); val = SbxMAXBYTE;
            }
            else if( val < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); val = 0;
            }
            *p->pByte = (sal_uInt8) val; break;
        }
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
        {
            sal_Int64 val = r / CURRENCY_FACTOR;
            if( r > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); val = SbxMAXINT;
            }
            else if( r < SbxMININT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); val = SbxMININT;
            }
            *p->pInteger = (sal_uInt16) val; break;
        }
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
        {
            sal_Int64 val = r / CURRENCY_FACTOR;
            if( val > SbxMAXUINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); val = SbxMAXUINT;
            }
            else if( val < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); val = 0;
            }
            *p->pUShort = (sal_uInt16) val; break;
        }
        case SbxBYREF | SbxLONG:
        {
            sal_Int64 val = r / CURRENCY_FACTOR;
            if( val > SbxMAXLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); val = SbxMAXLNG;
            }
            else if( val < SbxMINLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); val = SbxMINLNG;
            }
            *p->pLong = (sal_Int32) val; break;
        }
        case SbxBYREF | SbxULONG:
        {
            sal_Int64 val = r / CURRENCY_FACTOR;
            if( val > SbxMAXULNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); val = SbxMAXULNG;
            }
            else if( val < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); val = 0;
            }
            *p->pULong = (sal_uInt32) val; break;
        }
        case SbxBYREF | SbxCURRENCY:
            *p->pnInt64 = r; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = r / CURRENCY_FACTOR; break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = (sal_uInt64)r / CURRENCY_FACTOR; break;
        case SbxBYREF | SbxSINGLE:
            p->nSingle = (float)( r / CURRENCY_FACTOR ); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = ImpCurrencyToDouble( r ); break;
        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
