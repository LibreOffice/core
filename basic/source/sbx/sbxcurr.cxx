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

#include <basic/sberrors.hxx>
#include <basic/sbxvar.hxx>
#include <o3tl/string_view.hxx>
#include <svl/numformat.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <sbintern.hxx>
#include <runtime.hxx>
#include "sbxconv.hxx"


static OUString ImpCurrencyToString( sal_Int64 rVal )
{
    sal_Unicode cDecimalSep, cThousandSepDummy, cDecimalSepAltDummy;
    ImpGetIntntlSep(cDecimalSep, cThousandSepDummy, cDecimalSepAltDummy);

    auto strNum = OUString::number(rVal);
    std::u16string_view aAbsStr(strNum);
    OUStringBuffer aBuf(22);
    if (rVal < 0)
    {
        aBuf.append('-');
        assert(aAbsStr[0] == '-');
        aAbsStr = aAbsStr.substr(1); // skip the minus
    }
    size_t hasFractDigits = std::min(aAbsStr.length(), size_t(4));
    size_t hasWholeDigits = aAbsStr.length() - hasFractDigits;

    if (hasWholeDigits > 0)
        aBuf.append(aAbsStr.substr(0, hasWholeDigits));
    else
        aBuf.append('0');
    aBuf.append(cDecimalSep);
    // Handle leading 0's to right of decimal separator
    // Note: in VBA the stringification is a little more complex
    // but more natural as only the necessary digits
    // to the right of the decimal places are displayed
    // It would be great to conditionally be able to display like that too

    // Val   OOo (Cur)  VBA (Cur)
    // ---   ---------  ---------
    // 0     0.0000     0
    // 0.1   0.1000     0.1
    for (size_t i = 4; i > hasFractDigits; --i)
        aBuf.append('0');
    aBuf.append(aAbsStr.substr(aAbsStr.length() - hasFractDigits, hasFractDigits));

    return aBuf.makeStringAndClear();
}

static sal_Int64 ImpStringToCurrency(const rtl::OUString& rStr)
{
    LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();
    std::shared_ptr<SvNumberFormatter> pFormatter;
    if (GetSbData()->pInst)
    {
        pFormatter = GetSbData()->pInst->GetNumberFormatter();
    }
    else
    {
        sal_uInt32 n; // Dummy
        pFormatter = SbiInstance::PrepareNumberFormatter(/*date index*/ n, /*time index*/ n,
                                                         /*date time index*/ n);
    }

    // Passing a locale index switches IsNumberFormat() to use that locale,
    // in case the formatter wasn't default-created with it.
    sal_uInt32 nIndex = pFormatter->GetStandardIndex(eLangType);

    double fResult = 0.0;
    bool bSuccess = pFormatter->IsNumberFormat(rStr, nIndex, fResult);
    if (bSuccess)
    {
        SvNumFormatType nType = pFormatter->GetType(nIndex);
        if (!(nType & (SvNumFormatType::CURRENCY | SvNumFormatType::NUMBER)))
        {
            bSuccess = false;
        }
    }

    if (!bSuccess)
    {
        SbxBase::SetError(ERRCODE_BASIC_CONVERSION);
    }

    return ImpDoubleToCurrency(fResult);
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
            nRes = CurFrom(p->nByte);
            break;
        case SbxCHAR:
            nRes = CurFrom(p->nChar);
            break;
        case SbxBOOL:
        case SbxINTEGER:
            nRes = CurFrom(p->nInteger);
            break;
        case SbxUSHORT:
            nRes = CurFrom(p->nUShort);
            break;
        case SbxLONG:
            nRes = CurFrom(p->nLong);
            break;
        case SbxULONG:
            nRes = CurFrom(p->nULong);
            break;
        case SbxSALINT64:
            nRes = CurFrom(p->nInt64);
            break;
        case SbxSALUINT64:
            nRes = CurFrom(p->uInt64);
            break;
        case SbxSINGLE:
            nRes = ImpDoubleToCurrency(p->nSingle);
            break;

        case SbxDATE:
        case SbxDOUBLE:
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
            nRes = CurFrom(*p->pChar);
            break;
        case SbxBYREF | SbxBYTE:
            nRes = CurFrom(*p->pByte);
            break;
        case SbxBYREF | SbxBOOL:
        case SbxBYREF | SbxINTEGER:
            nRes = CurFrom(*p->pInteger);
            break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            nRes = CurFrom(*p->pUShort);
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
    switch( +p->eType )
    {
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble =  ImpCurrencyToDouble( r ); break;
        case SbxSALUINT64:
            p->uInt64 = CurTo<sal_uInt64>(r); break;
        case SbxSALINT64:
            p->nInt64 = CurTo<sal_Int64>(r); break;

        case SbxCURRENCY:
            p->nInt64 = r; break;

        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            SbxDecimal* pDec = ImpCreateDecimal( p );
            if( !pDec->setDouble( ImpCurrencyToDouble( r ) ) )
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
        case SbxCHAR:
            p->nChar = CurTo<sal_Unicode>(r); break;
        case SbxBYREF | SbxCHAR:
            *p->pChar = CurTo<sal_Unicode>(r); break;
        case SbxBYTE:
            p->nByte = CurTo<sal_uInt8>(r); break;
        case SbxBYREF | SbxBYTE:
            *p->pByte = CurTo<sal_uInt8>(r); break;
        case SbxINTEGER:
        case SbxBOOL:
            p->nInteger = CurTo<sal_Int16>(r); break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            *p->pInteger = CurTo<sal_Int16>(r); break;
        case SbxERROR:
        case SbxUSHORT:
            p->nUShort = CurTo<sal_uInt16>(r); break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            *p->pUShort = CurTo<sal_uInt16>(r); break;
        case SbxLONG:
            p->nLong = CurTo<sal_Int32>(r); break;
        case SbxBYREF | SbxLONG:
            *p->pLong = CurTo<sal_Int32>(r); break;
        case SbxULONG:
            p->nULong = CurTo<sal_uInt32>(r); break;
        case SbxBYREF | SbxULONG:
            *p->pULong = CurTo<sal_uInt32>(r); break;
        case SbxBYREF | SbxCURRENCY:
            *p->pnInt64 = r; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = CurTo<sal_Int64>(r); break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = CurTo<sal_uInt64>(r); break;
        case SbxSINGLE:
        case SbxBYREF | SbxSINGLE:
            p->nSingle = r / float(CURRENCY_FACTOR); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = ImpCurrencyToDouble( r ); break;
        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
