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

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svl/zforlist.hxx>
#include <tools/color.hxx>
#include <i18nlangtag/lang.h>
#include <basic/sberrors.hxx>
#include "sbxconv.hxx"
#include <runtime.hxx>
#include <sbintern.hxx>
#include <math.h>
#include <memory>
#include <config_features.h>


double ImpGetDate( const SbxValues* p )
{
    double nRes;
    SbxValue* pVal;

    switch( +p->eType )
    {
    case SbxNULL:
        SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
        [[fallthrough]];
    case SbxEMPTY:
        nRes = 0;
        break;
    case SbxCHAR:
        nRes = p->nChar;
        break;
    case SbxBYTE:
        nRes = p->nByte;
        break;
    case SbxINTEGER:
    case SbxBOOL:
        nRes = p->nInteger;
        break;
    case SbxERROR:
    case SbxUSHORT:
        nRes = p->nUShort;
        break;
    case SbxLONG:
        nRes = static_cast<double>(p->nLong);
        break;
    case SbxULONG:
        nRes = static_cast<double>(p->nULong);
        break;
    case SbxSINGLE:
        nRes = p->nSingle;
        break;
    case SbxDATE:
    case SbxDOUBLE:
        nRes = p->nDouble;
        break;
    case SbxCURRENCY:
        nRes = ImpCurrencyToDouble( p->nInt64 );
        break;
    case SbxSALINT64:
        nRes = static_cast< double >(p->nInt64);
        break;
    case SbxSALUINT64:
        nRes = ImpSalUInt64ToDouble( p->uInt64 );
        break;
    case SbxDECIMAL:
    case SbxBYREF | SbxDECIMAL:
        if( p->pDecimal )
        {
            p->pDecimal->getDouble( nRes );
        }
        else
        {
            nRes = 0.0;
        }
        break;
    case SbxBYREF | SbxSTRING:
    case SbxSTRING:
    case SbxLPSTR:
#if HAVE_FEATURE_SCRIPTING
        if( !p->pOUString )
        {
            nRes = 0;
        }
        else
        {
            LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();
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

            sal_uInt32 nIndex;
            sal_Int32 nCheckPos = 0;
            SvNumFormatType nType = SvNumFormatType::DEFINED | SvNumFormatType::DATE | SvNumFormatType::TIME | SvNumFormatType::CURRENCY
                                    | SvNumFormatType::NUMBER | SvNumFormatType::SCIENTIFIC | SvNumFormatType::FRACTION;

            // Default templates of the formatter have only two-digit
            // date. Therefore register an own format.

            // HACK, because the number formatter in PutandConvertEntry replace the wildcard
            // for month, day, year not according to the configuration.
            // Problem: Print Year(Date) under Engl. OS
            // quod vide basic/source/runtime/runtime.cxx

            SvtSysLocale aSysLocale;
            DateOrder eDate = aSysLocale.GetLocaleData().getDateOrder();
            OUString aDateStr;
            switch( eDate )
            {
                default:
                case DateOrder::MDY: aDateStr = "MM/DD/YYYY"; break;
                case DateOrder::DMY: aDateStr = "DD/MM/YYYY"; break;
                case DateOrder::YMD: aDateStr = "YYYY/MM/DD"; break;
            }

            OUString aStr = aDateStr + " HH:MM:SS";

            pFormatter->PutandConvertEntry( aStr, nCheckPos, nType,
                                            nIndex, LANGUAGE_ENGLISH_US, eLangType, true);
            bool bSuccess = pFormatter->IsNumberFormat( *p->pOUString, nIndex, nRes );
            if ( bSuccess )
            {
                SvNumFormatType nType_ = pFormatter->GetType( nIndex );
                if(!(nType_ & ( SvNumFormatType::DATETIME | SvNumFormatType::DATE |
                                SvNumFormatType::TIME | SvNumFormatType::DEFINED )))
                {
                    bSuccess = false;
                }
            }

            if ( !bSuccess )
            {
                SbxBase::SetError( ERRCODE_BASIC_CONVERSION ); nRes = 0;
            }
        }
#else
        nRes = 0;
#endif
        break;
    case SbxOBJECT:
        pVal = dynamic_cast<SbxValue*>( p->pObj );
        if( pVal )
        {
            nRes = pVal->GetDate();
        }
        else
        {
            SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT ); nRes = 0;
        }
        break;
    case SbxBYREF | SbxCHAR:
        nRes = *p->pChar;
        break;
    case SbxBYREF | SbxBYTE:
        nRes = *p->pByte;
        break;
    case SbxBYREF | SbxINTEGER:
    case SbxBYREF | SbxBOOL:
        nRes = *p->pInteger;
        break;
    case SbxBYREF | SbxLONG:
        nRes = *p->pLong;
        break;
    case SbxBYREF | SbxULONG:
        nRes = *p->pULong;
        break;
    case SbxBYREF | SbxERROR:
    case SbxBYREF | SbxUSHORT:
        nRes = *p->pUShort;
        break;
    case SbxBYREF | SbxSINGLE:
        nRes = *p->pSingle;
        break;
    case SbxBYREF | SbxDATE:
    case SbxBYREF | SbxDOUBLE:
        nRes = *p->pDouble;
        break;
    case SbxBYREF | SbxCURRENCY:
        nRes = ImpCurrencyToDouble( *p->pnInt64 );
        break;
    case SbxBYREF | SbxSALINT64:
        nRes = static_cast< double >(*p->pnInt64);
        break;
    case SbxBYREF | SbxSALUINT64:
        nRes = ImpSalUInt64ToDouble( *p->puInt64 );
        break;
    default:
        SbxBase::SetError( ERRCODE_BASIC_CONVERSION ); nRes = 0;
        break;
    }
    return nRes;
}

void ImpPutDate( SbxValues* p, double n )
{
    SbxValues aTmp;
    SbxDecimal* pDec;
    SbxValue* pVal;

start:
    switch( +p->eType )
    {
    case SbxDATE:
    case SbxDOUBLE:
        p->nDouble = n;
        break;
        // from here will be tested
    case SbxCHAR:
        aTmp.pChar = &p->nChar;
        goto direct;
    case SbxBYTE:
        aTmp.pByte = &p->nByte;
        goto direct;
    case SbxINTEGER:
    case SbxBOOL:
        aTmp.pInteger = &p->nInteger;
        goto direct;
    case SbxLONG:
        aTmp.pLong = &p->nLong;
        goto direct;
    case SbxULONG:
        aTmp.pULong = &p->nULong;
        goto direct;
    case SbxERROR:
    case SbxUSHORT:
        aTmp.pUShort = &p->nUShort;
        goto direct;
    case SbxSINGLE:
        aTmp.pSingle = &p->nSingle;
        goto direct;
    case SbxCURRENCY:
    case SbxSALINT64:
        aTmp.pnInt64 = &p->nInt64;
        goto direct;
    case SbxSALUINT64:
        aTmp.puInt64 = &p->uInt64;
        goto direct;
    case SbxDECIMAL:
    case SbxBYREF | SbxDECIMAL:
        pDec = ImpCreateDecimal( p );
        if( !pDec->setDouble( n ) )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW );
        }
        break;
    direct:
        aTmp.eType = SbxDataType( p->eType | SbxBYREF );
        p = &aTmp; goto start;

    case SbxBYREF | SbxSTRING:
    case SbxSTRING:
    case SbxLPSTR:
        {
#if HAVE_FEATURE_SCRIPTING
            if( !p->pOUString )
            {
                p->pOUString = new OUString;
            }
            const Color* pColor;

            LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();
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

            sal_uInt32 nIndex;
            sal_Int32 nCheckPos = 0;
            SvNumFormatType nType;

            SvtSysLocale aSysLocale;
            DateOrder eDate = aSysLocale.GetLocaleData().getDateOrder();
            OUString aStr;
            // if the whole-number part is 0, we want no year!
            if( n <= -1.0 || n >= 1.0 )
            {
                // Time only if != 00:00:00
                if( rtl::math::approxEqual(floor( n ), n) )
                {
                    switch( eDate )
                    {
                        default:
                        case DateOrder::MDY: aStr = "MM/DD/YYYY"; break;
                        case DateOrder::DMY: aStr = "DD/MM/YYYY"; break;
                        case DateOrder::YMD: aStr = "YYYY/MM/DD"; break;
                    }
                }
                else
                {
                    switch( eDate )
                    {
                        default:
                        case DateOrder::MDY: aStr = "MM/DD/YYYY HH:MM:SS"; break;
                        case DateOrder::DMY: aStr = "DD/MM/YYYY HH:MM:SS"; break;
                        case DateOrder::YMD: aStr = "YYYY/MM/DD HH:MM:SS"; break;
                    }
                }
            }
            else
            {
                aStr = "HH:MM:SS";
            }
            pFormatter->PutandConvertEntry( aStr,
                                            nCheckPos,
                                            nType,
                                            nIndex,
                                            LANGUAGE_ENGLISH_US,
                                            eLangType, true);
            pFormatter->GetOutputString( n, nIndex, *p->pOUString, &pColor );
#endif
            break;
        }
    case SbxOBJECT:
        pVal = dynamic_cast<SbxValue*>( p->pObj );
        if( pVal )
        {
            pVal->PutDate( n );
        }
        else
        {
            SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
        }
        break;
    case SbxBYREF | SbxCHAR:
        if( n > SbxMAXCHAR )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXCHAR;
        }
        else if( n < SbxMINCHAR )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMINCHAR;
        }
        *p->pChar = static_cast<sal_Unicode>(n);
        break;
    case SbxBYREF | SbxBYTE:
        if( n > SbxMAXBYTE )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXBYTE;
        }
        else if( n < 0 )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = 0;
        }
        *p->pByte = static_cast<sal_uInt8>(n);
        break;
    case SbxBYREF | SbxINTEGER:
    case SbxBYREF | SbxBOOL:
        if( n > SbxMAXINT )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXINT;
        }
        else if( n < SbxMININT )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMININT;
        }
        *p->pInteger = static_cast<sal_Int16>(n);
        break;
    case SbxBYREF | SbxERROR:
    case SbxBYREF | SbxUSHORT:
        if( n > SbxMAXUINT )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXUINT;
        }
        else if( n < 0 )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = 0;
        }
        *p->pUShort = static_cast<sal_uInt16>(n);
        break;
    case SbxBYREF | SbxLONG:
        if( n > SbxMAXLNG )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXLNG;
        }
        else if( n < SbxMINLNG )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMINLNG;
        }
        *p->pLong = static_cast<sal_Int32>(n);
        break;
    case SbxBYREF | SbxULONG:
        if( n > SbxMAXULNG )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXULNG;
        }
        else if( n < 0 )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = 0;
        }
        *p->pULong = static_cast<sal_uInt32>(n);
        break;
    case SbxBYREF | SbxSINGLE:
        if( n > SbxMAXSNG )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXSNG;
        }
        else if( n < SbxMINSNG )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMINSNG;
        }
        *p->pSingle = static_cast<float>(n);
        break;
    case SbxBYREF | SbxSALINT64:
        *p->pnInt64 = ImpDoubleToSalInt64( n );
        break;
    case SbxBYREF | SbxSALUINT64:
        *p->puInt64 = ImpDoubleToSalUInt64( n );
        break;
    case SbxBYREF | SbxDATE:
    case SbxBYREF | SbxDOUBLE:
        *p->pDouble = n;
        break;
    case SbxBYREF | SbxCURRENCY:
        if( n > SbxMAXCURR )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXCURR;
        }
        else if( n < SbxMINCURR )
        {
            SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMINCURR;
        }
        *p->pnInt64 = ImpDoubleToCurrency( n );
        break;
    default:
        SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
