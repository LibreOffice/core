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

#include <sal/config.h>

#include <config_features.h>

#include <o3tl/safeint.hxx>
#include <comphelper/errcode.hxx>
#include <basic/sberrors.hxx>
#include "sbxconv.hxx"
#include <runtime.hxx>

#include <rtl/math.hxx>

namespace
{
template <class T>
concept number = std::is_arithmetic_v<T>;

template <SbxDataType MySbxType, number N> N ImpGet(const SbxValues* p)
{
    switch (+p->eType)
    {
        case SbxEMPTY:
            return 0;
        case SbxCHAR:
            return ConvertWithOverflowTo<N>(p->nChar);
        case SbxBYREF | SbxCHAR:
            return ConvertWithOverflowTo<N>(*p->pChar);
        case SbxBYTE:
            return ConvertWithOverflowTo<N>(p->nByte);
        case SbxBYREF | SbxBYTE:
            return ConvertWithOverflowTo<N>(*p->pByte);
        case SbxINTEGER:
        case SbxBOOL:
            return ConvertWithOverflowTo<N>(p->nInteger);
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            return ConvertWithOverflowTo<N>(*p->pInteger);
        case SbxERROR:
        case SbxUSHORT:
            return ConvertWithOverflowTo<N>(p->nUShort);
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            return ConvertWithOverflowTo<N>(*p->pUShort);
        case SbxLONG:
            return ConvertWithOverflowTo<N>(p->nLong);
        case SbxBYREF | SbxLONG:
            return ConvertWithOverflowTo<N>(*p->pLong);
        case SbxULONG:
            return ConvertWithOverflowTo<N>(p->nULong);
        case SbxBYREF | SbxULONG:
            return ConvertWithOverflowTo<N>(*p->pULong);
        case SbxCURRENCY:
            return CurTo<N>(p->nInt64);
        case SbxBYREF | SbxCURRENCY:
            return CurTo<N>(*p->pnInt64);
        case SbxSALINT64:
            return ConvertWithOverflowTo<N>(p->nInt64);
        case SbxBYREF | SbxSALINT64:
            return ConvertWithOverflowTo<N>(*p->pnInt64);
        case SbxSALUINT64:
            return ConvertWithOverflowTo<N>(p->uInt64);
        case SbxBYREF | SbxSALUINT64:
            return ConvertWithOverflowTo<N>(*p->puInt64);
        case SbxSINGLE:
            return ConvertWithOverflowTo<N>(p->nSingle);
        case SbxBYREF | SbxSINGLE:
            return ConvertWithOverflowTo<N>(*p->pSingle);
        case SbxDATE:
        case SbxDOUBLE:
            return ConvertWithOverflowTo<N>(p->nDouble);
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            return ConvertWithOverflowTo<N>(*p->pDouble);
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            if (p->pDecimal)
                return p->pDecimal->getWithOverflow<N>();
            return 0;
        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if (p->pOUString)
            {
                double d;
                SbxDataType t;
                if (ImpScan(*p->pOUString, d, t, nullptr) == ERRCODE_NONE)
                    return ConvertWithOverflowTo<N>(d);
            }
#if HAVE_FEATURE_SCRIPTING
            if (SbiRuntime::isVBAEnabled()) // VBA only behaviour
                SbxBase::SetError(ERRCODE_BASIC_CONVERSION);
#endif
            return 0;
        case SbxOBJECT:
            if (SbxValue* pVal = dynamic_cast<SbxValue*>(p->pObj))
            {
                SbxValues aObjVal(MySbxType);
                pVal->Get(aObjVal);
                return ImpGet<MySbxType, N>(&aObjVal);
            }
            SbxBase::SetError(ERRCODE_BASIC_NO_OBJECT);
            return 0;

        default:
            SbxBase::SetError(ERRCODE_BASIC_CONVERSION);
            return 0;
    }
}

template <typename T> void assignTo(SbxDecimal& dest, T n); // not defined
template <> void assignTo<sal_uInt8>(SbxDecimal& dest, sal_uInt8 n) { dest.setByte(n); }
template <> void assignTo<sal_Unicode>(SbxDecimal& dest, sal_Unicode n) { dest.setChar(n); }
template <> void assignTo<sal_Int16>(SbxDecimal& dest, sal_Int16 n) { dest.setShort(n); }
template <> void assignTo<sal_uInt16>(SbxDecimal& dest, sal_uInt16 n) { dest.setUShort(n); }
template <> void assignTo<sal_Int32>(SbxDecimal& dest, sal_Int32 n) { dest.setLong(n); }
template <> void assignTo<sal_uInt32>(SbxDecimal& dest, sal_uInt32 n) { dest.setULong(n); }
template <> void assignTo<sal_Int64>(SbxDecimal& dest, sal_Int64 n) { dest.setWithOverflow(n); }
template <> void assignTo<sal_uInt64>(SbxDecimal& dest, sal_uInt64 n) { dest.setWithOverflow(n); }
template <> void assignTo<double>(SbxDecimal& dest, double n) { dest.setWithOverflow(n); }
template <> void assignTo<float>(SbxDecimal& dest, float n) { dest.setWithOverflow(n); }

template <std::integral I> void assignTo(OUString& dest, I n, bool) { dest = OUString::number(n); }
void assignTo(OUString& dest, double n, bool bCoreString)
{
    // tdf#107953 - show 17 significant digits
    ImpCvtNum(n, 17, dest, bCoreString);
}
void assignTo(OUString& dest, float n, bool)
{
    // tdf#107953 - show 9 significant digits
    ImpCvtNum(n, 9, dest);
}

template <SbxDataType MySbxType, number N> void ImpPut(SbxValues* p, N n, bool bCoreString)
{
    switch (+p->eType)
    {
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

        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            assignTo(*ImpCreateDecimal(p), n);
            break;

        case SbxSINGLE:
            assignWithOverflowTo(p->nSingle, n);
            break;
        case SbxBYREF | SbxSINGLE:
            assignWithOverflowTo(*p->pSingle, n);
            break;

        case SbxDATE:
        case SbxDOUBLE:
            assignWithOverflowTo(p->nDouble, n);
            break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            assignWithOverflowTo(*p->pDouble, n);
            break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if (!p->pOUString)
                p->pOUString = new OUString;
            assignTo(*p->pOUString, n, bCoreString);
            break;

        case SbxOBJECT:
            if (SbxValue* pVal = dynamic_cast<SbxValue*>(p->pObj))
            {
                SbxValues newVal(MySbxType);
                ImpPut<MySbxType, N>(&newVal, n, bCoreString);
                pVal->Put(newVal);
            }
            else
                SbxBase::SetError(ERRCODE_BASIC_NO_OBJECT);
            break;

        default:
            SbxBase::SetError(ERRCODE_BASIC_CONVERSION);
    }
}
}

sal_Unicode ImpGetChar(const SbxValues* p) { return ImpGet<SbxCHAR, sal_Unicode>(p); }
sal_uInt8 ImpGetByte(const SbxValues* p) { return ImpGet<SbxBYTE, sal_uInt8>(p); }
sal_Int16 ImpGetInteger(const SbxValues* p) { return ImpGet<SbxINTEGER, sal_Int16>(p); }
sal_uInt16 ImpGetUShort(const SbxValues* p) { return ImpGet<SbxUSHORT, sal_uInt16>(p); }
sal_Int32 ImpGetLong(const SbxValues* p) { return ImpGet<SbxLONG, sal_Int32>(p); }
sal_uInt32 ImpGetULong(const SbxValues* p) { return ImpGet<SbxULONG, sal_uInt32>(p); }
sal_Int64 ImpGetInt64(const SbxValues* p) { return ImpGet<SbxSALINT64, sal_Int64>(p); }
sal_uInt64 ImpGetUInt64(const SbxValues* p) { return ImpGet<SbxSALUINT64, sal_uInt64>(p); }
double ImpGetDouble(const SbxValues* p) { return ImpGet<SbxDOUBLE, double>(p); }
float ImpGetSingle(const SbxValues* p) { return ImpGet<SbxSINGLE, float>(p); }

void ImpPutChar(SbxValues* p, sal_Unicode n) { ImpPut<SbxCHAR>(p, n, false); }
void ImpPutByte(SbxValues* p, sal_uInt8 n) { ImpPut<SbxBYTE>(p, n, false); }
void ImpPutInteger(SbxValues* p, sal_Int16 n) { ImpPut<SbxINTEGER>(p, n, false); }
void ImpPutUShort(SbxValues* p, sal_uInt16 n) { ImpPut<SbxUSHORT>(p, n, false); }
void ImpPutLong(SbxValues* p, sal_Int32 n) { ImpPut<SbxLONG>(p, n, false); }
void ImpPutULong(SbxValues* p, sal_uInt32 n) { ImpPut<SbxULONG>(p, n, false); }
void ImpPutInt64(SbxValues* p, sal_Int64 n) { ImpPut<SbxSALINT64>(p, n, false); }
void ImpPutUInt64(SbxValues* p, sal_uInt64 n) { ImpPut<SbxSALUINT64>(p, n, false); }
void ImpPutDouble(SbxValues* p, double n, bool bCoreString) { ImpPut<SbxDOUBLE>(p, n, bCoreString); }
void ImpPutSingle(SbxValues* p, float n) { ImpPut<SbxSINGLE>(p, n, false); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
