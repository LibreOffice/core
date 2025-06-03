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

#include <rtl/math.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svl/numformat.hxx>
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
    switch( +p->eType )
    {
    case SbxEMPTY:
        return 0;
    case SbxCHAR:
        return p->nChar;
    case SbxBYTE:
        return p->nByte;
    case SbxINTEGER:
    case SbxBOOL:
        return p->nInteger;
    case SbxERROR:
    case SbxUSHORT:
        return p->nUShort;
    case SbxLONG:
        return p->nLong;
    case SbxULONG:
        return p->nULong;
    case SbxSINGLE:
        return p->nSingle;
    case SbxDATE:
    case SbxDOUBLE:
        return p->nDouble;
    case SbxCURRENCY:
        return CurTo<double>(p->nInt64);
    case SbxSALINT64:
        return static_cast<double>(p->nInt64);
    case SbxSALUINT64:
        return static_cast<double>(p->uInt64);
    case SbxDECIMAL:
    case SbxBYREF | SbxDECIMAL:
        if (p->pDecimal)
            if (double d; p->pDecimal->getDouble(d))
                return d;
        return 0;
    case SbxBYREF | SbxSTRING:
    case SbxSTRING:
    case SbxLPSTR:
#if HAVE_FEATURE_SCRIPTING
        if (p->pOUString)
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
            if (double d; pFormatter->IsNumberFormat(*p->pOUString, nIndex, d))
            {
                SvNumFormatType nType_ = pFormatter->GetType( nIndex );
                if(nType_ & ( SvNumFormatType::DATETIME | SvNumFormatType::DATE |
                                SvNumFormatType::TIME | SvNumFormatType::DEFINED ))
                {
                    return d;
                }
            }

            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
        }
#endif
        return 0;
    case SbxOBJECT:
        if (SbxValue* pVal = dynamic_cast<SbxValue*>(p->pObj))
            return pVal->GetDate();
        SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
        return 0;
    case SbxBYREF | SbxCHAR:
        return *p->pChar;
    case SbxBYREF | SbxBYTE:
        return *p->pByte;
    case SbxBYREF | SbxINTEGER:
    case SbxBYREF | SbxBOOL:
        return *p->pInteger;
    case SbxBYREF | SbxLONG:
        return *p->pLong;
    case SbxBYREF | SbxULONG:
        return *p->pULong;
    case SbxBYREF | SbxERROR:
    case SbxBYREF | SbxUSHORT:
        return *p->pUShort;
    case SbxBYREF | SbxSINGLE:
        return *p->pSingle;
    case SbxBYREF | SbxDATE:
    case SbxBYREF | SbxDOUBLE:
        return *p->pDouble;
    case SbxBYREF | SbxCURRENCY:
        return CurTo<double>(*p->pnInt64);
    case SbxBYREF | SbxSALINT64:
        return static_cast<double>(*p->pnInt64);
    case SbxBYREF | SbxSALUINT64:
        return static_cast<double>(*p->puInt64);
    default:
        SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
        return 0;
    }
}

void ImpPutDate( SbxValues* p, double n )
{
    switch( +p->eType )
    {
    case SbxDATE:
    case SbxDOUBLE:
        p->nDouble = n;
        break;
    case SbxBYREF | SbxDATE:
    case SbxBYREF | SbxDOUBLE:
        *p->pDouble = n;
        break;
    case SbxCHAR:
        assignWithOverflowTo(p->nChar, n);
        break;
    case SbxBYREF | SbxCHAR:
        assignWithOverflowTo(*p->pChar, n);
        break;
    case SbxBYTE:
        assignWithOverflowTo(p->nByte, n);
        break;
    case SbxBYREF | SbxBYTE:
        assignWithOverflowTo(*p->pByte, n);
        break;
    case SbxINTEGER:
    case SbxBOOL:
        assignWithOverflowTo(p->nInteger, n);
        break;
    case SbxBYREF | SbxINTEGER:
    case SbxBYREF | SbxBOOL:
        assignWithOverflowTo(*p->pInteger, n);
        break;
    case SbxERROR:
    case SbxUSHORT:
        assignWithOverflowTo(p->nUShort, n);
        break;
    case SbxBYREF | SbxERROR:
    case SbxBYREF | SbxUSHORT:
        assignWithOverflowTo(*p->pUShort, n);
        break;
    case SbxLONG:
        assignWithOverflowTo(p->nLong, n);
        break;
    case SbxBYREF | SbxLONG:
        assignWithOverflowTo(*p->pLong, n);
        break;
    case SbxULONG:
        assignWithOverflowTo(p->nULong, n);
        break;
    case SbxBYREF | SbxULONG:
        assignWithOverflowTo(*p->pULong, n);
        break;
    case SbxCURRENCY:
        assignWithOverflowTo(p->nInt64, CurFrom(n));
        break;
    case SbxBYREF | SbxCURRENCY:
        assignWithOverflowTo(*p->pnInt64, CurFrom(n));
        break;
    case SbxSALINT64:
        assignWithOverflowTo(p->nInt64, n);
        break;
    case SbxBYREF | SbxSALINT64:
        assignWithOverflowTo(*p->pnInt64, n);
        break;
    case SbxSALUINT64:
        assignWithOverflowTo(p->uInt64, n);
        break;
    case SbxBYREF | SbxSALUINT64:
        assignWithOverflowTo(*p->puInt64, n);
        break;
    case SbxSINGLE:
        assignWithOverflowTo(p->nSingle, n);
        break;
    case SbxBYREF | SbxSINGLE:
        assignWithOverflowTo(*p->pSingle, n);
        break;

    case SbxDECIMAL:
    case SbxBYREF | SbxDECIMAL:
        ImpCreateDecimal(p)->setWithOverflow(n);
        break;

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
        if (auto pVal = dynamic_cast<SbxValue*>(p->pObj))
        {
            pVal->PutDate( n );
        }
        else
        {
            SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
        }
        break;

    default:
        SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
