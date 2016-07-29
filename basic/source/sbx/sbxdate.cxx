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
#include <tools/errcode.hxx>
#include <tools/color.hxx>
#include <i18nlangtag/lang.h>
#include <basic/sbx.hxx>
#include "sbxconv.hxx"
#include "math.h"
#include <comphelper/processfactory.hxx>
#include <memory>


double ImpGetDate( const SbxValues* p )
{
    double nRes;
    SbxValue* pVal;

    switch( +p->eType )
    {
    case SbxNULL:
        SbxBase::SetError( ERRCODE_SBX_CONVERSION );
        SAL_FALLTHROUGH;
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
        nRes = (double) p->nLong;
        break;
    case SbxULONG:
        nRes = (double) p->nULong;
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
        if( !p->pOUString )
        {
            nRes = 0;
        }
        else
        {
            LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();

            std::unique_ptr<SvNumberFormatter> pFormatter(new SvNumberFormatter( comphelper::getProcessComponentContext(), eLangType ));

            sal_uInt32 nIndex;
            sal_Int32 nCheckPos = 0;
            short nType = 127;

            // Default templates of the formatter have only two-digit
            // date. Therefore register an own format.

            // HACK, because the number formatter in PutandConvertEntry replace the wildcard
            // for month, day, year not according to the configuration.
            // Problem: Print Year(Date) under Engl. OS
            // quod vide basic/source/runtime/runtime.cxx

            SvtSysLocale aSysLocale;
            DateFormat eDate = aSysLocale.GetLocaleData().getDateFormat();
            OUString aDateStr;
            switch( eDate )
            {
                default:
                case MDY: aDateStr = "MM/DD/YYYY"; break;
                case DMY: aDateStr = "DD/MM/YYYY"; break;
                case YMD: aDateStr = "YYYY/MM/DD"; break;
            }

            OUString aStr = aDateStr + " HH:MM:SS";

            pFormatter->PutandConvertEntry( aStr, nCheckPos, nType,
                                            nIndex, LANGUAGE_ENGLISH_US, eLangType );
            bool bSuccess = pFormatter->IsNumberFormat( *p->pOUString, nIndex, nRes );
            if ( bSuccess )
            {
                short nType_ = pFormatter->GetType( nIndex );
                if(!(nType_ & ( css::util::NumberFormat::DATETIME | css::util::NumberFormat::DATE |
                                css::util::NumberFormat::TIME | css::util::NumberFormat::DEFINED )))
                {
                    bSuccess = false;
                }
            }

            if ( !bSuccess )
            {
                SbxBase::SetError( ERRCODE_SBX_CONVERSION ); nRes = 0;
            }
        }
        break;
    case SbxOBJECT:
        pVal = dynamic_cast<SbxValue*>( p->pObj );
        if( pVal )
        {
            nRes = pVal->GetDate();
        }
        else
        {
            SbxBase::SetError( ERRCODE_SBX_NO_OBJECT ); nRes = 0;
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
        SbxBase::SetError( ERRCODE_SBX_CONVERSION ); nRes = 0;
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
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
        }
        break;
    direct:
        aTmp.eType = SbxDataType( p->eType | SbxBYREF );
        p = &aTmp; goto start;

    case SbxBYREF | SbxSTRING:
    case SbxSTRING:
    case SbxLPSTR:
        {
            if( !p->pOUString )
            {
                p->pOUString = new OUString;
            }
            Color* pColor;

            LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();
            std::unique_ptr<SvNumberFormatter> pFormatter(new SvNumberFormatter( comphelper::getProcessComponentContext(), eLangType ));

            sal_uInt32 nIndex;
            sal_Int32 nCheckPos = 0;
            short nType;

            SvtSysLocale aSysLocale;
            DateFormat eDate = aSysLocale.GetLocaleData().getDateFormat();
            OUString aStr;
            // if the whole-number part is 0, we want no year!
            if( n <= -1.0 || n >= 1.0 )
            {
                // Time only if != 00:00:00
                if( rtl::math::approxEqual(floor( n ), n) )
                {
                    switch( eDate )
                    {
                    case MDY: aStr = "MM.TT.JJJJ"; break;
                    case DMY: aStr = "TT.MM.JJJJ"; break;
                    case YMD: aStr = "JJJJ.MM.TT"; break;
                    default:  aStr = "MM.TT.JJJJ";
                    }
                }
                else
                {
                    switch( eDate )
                    {
                    case MDY: aStr = "MM.TT.JJJJ HH:MM:SS"; break;
                    case DMY: aStr = "TT.MM.JJJJ HH:MM:SS"; break;
                    case YMD: aStr = "JJJJ.MM.TT HH:MM:SS"; break;
                    default:  aStr = "MM.TT.JJJJ HH:MM:SS";
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
                                            LANGUAGE_GERMAN,
                                            eLangType );
            pFormatter->GetOutputString( n, nIndex, *p->pOUString, &pColor );
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
            SbxBase::SetError( ERRCODE_SBX_NO_OBJECT );
        }
        break;
    case SbxBYREF | SbxCHAR:
        if( n > SbxMAXCHAR )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMAXCHAR;
        }
        else if( n < SbxMINCHAR )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMINCHAR;
        }
        *p->pChar = (sal_Unicode) n;
        break;
    case SbxBYREF | SbxBYTE:
        if( n > SbxMAXBYTE )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMAXBYTE;
        }
        else if( n < 0 )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = 0;
        }
        *p->pByte = (sal_uInt8) n;
        break;
    case SbxBYREF | SbxINTEGER:
    case SbxBYREF | SbxBOOL:
        if( n > SbxMAXINT )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMAXINT;
        }
        else if( n < SbxMININT )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMININT;
        }
        *p->pInteger = (sal_Int16) n;
        break;
    case SbxBYREF | SbxERROR:
    case SbxBYREF | SbxUSHORT:
        if( n > SbxMAXUINT )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMAXUINT;
        }
        else if( n < 0 )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = 0;
        }
        *p->pUShort = (sal_uInt16) n;
        break;
    case SbxBYREF | SbxLONG:
        if( n > SbxMAXLNG )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMAXLNG;
        }
        else if( n < SbxMINLNG )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMINLNG;
        }
        *p->pLong = (sal_Int32) n;
        break;
    case SbxBYREF | SbxULONG:
        if( n > SbxMAXULNG )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMAXULNG;
        }
        else if( n < 0 )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = 0;
        }
        *p->pULong = (sal_uInt32) n;
        break;
    case SbxBYREF | SbxSINGLE:
        if( n > SbxMAXSNG )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMAXSNG;
        }
        else if( n < SbxMINSNG )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMINSNG;
        }
        *p->pSingle = (float) n;
        break;
    case SbxBYREF | SbxSALINT64:
        *p->pnInt64 = ImpDoubleToSalInt64( n );
        break;
    case SbxBYREF | SbxSALUINT64:
        *p->puInt64 = ImpDoubleToSalUInt64( n );
        break;
    case SbxBYREF | SbxDATE:
    case SbxBYREF | SbxDOUBLE:
        *p->pDouble = (double) n;
        break;
    case SbxBYREF | SbxCURRENCY:
        if( n > SbxMAXCURR )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMAXCURR;
        }
        else if( n < SbxMINCURR )
        {
            SbxBase::SetError( ERRCODE_SBX_OVERFLOW ); n = SbxMINCURR;
        }
        *p->pnInt64 = ImpDoubleToCurrency( n );
        break;
    default:
        SbxBase::SetError( ERRCODE_SBX_CONVERSION );
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
