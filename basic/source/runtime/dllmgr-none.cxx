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

#if defined(_WIN32)
#include <prewin.h>
#include <postwin.h>
#endif

#include <basic/sberrors.hxx>
#include <basic/sbx.hxx>
#include <basic/sbxvar.hxx>
#include <rtl/ustring.hxx>
#include <osl/time.h>

#include "dllmgr.hxx"

namespace {

// Overcome the mess of Currency vs. custom types etc.
ErrCode returnInt64InOutArg(SbxArray *pArgs, SbxVariable &rRetVal,
                            sal_Int64 nValue)
{
    if (!rRetVal.PutLong(1) && !rRetVal.PutInteger(1))
        return ERRCODE_BASIC_BAD_ARGUMENT;
    if (!pArgs || pArgs->Count() != 2)
        return ERRCODE_BASIC_BAD_ARGUMENT;
    SbxVariable* pOut = pArgs->Get(1);
    if (!pOut)
        return ERRCODE_BASIC_BAD_ARGUMENT;
    if (pOut->IsCurrency())
    {
        pOut->PutCurrency(nValue);
        return ERRCODE_NONE;
    }
    if (pOut->IsObject())
    {
        // FIXME: should we clone this and use pOut->PutObject ?
        SbxObject* pObj = dynamic_cast<SbxObject*>( pOut->GetObject() );
        if (!pObj)
            return ERRCODE_BASIC_BAD_ARGUMENT;

        // We expect two Longs but other mappings could be possible too.
        SbxArray* pProps = pObj->GetProperties();
        if (pProps->Count() != 2)
            return ERRCODE_BASIC_BAD_ARGUMENT;
        SbxVariable* pLow = pProps->Get(0);
        SbxVariable* pHigh = pProps->Get(1);
        if (!pLow || !pLow->IsLong() ||
            !pHigh || !pHigh->IsLong())
            return ERRCODE_BASIC_BAD_ARGUMENT;
        pLow->PutLong(nValue & 0xffffffff);
        pHigh->PutLong(nValue >> 32);
        return ERRCODE_NONE;
    }
    return ERRCODE_BASIC_BAD_ARGUMENT;
}

ErrCode builtin_kernel32(std::u16string_view aFuncName, SbxArray *pArgs,
                         SbxVariable &rRetVal)
{
    sal_Int64 nNanoSecsPerSec = 1000.0*1000*1000;
    if (aFuncName == u"QueryPerformanceFrequency")
        return returnInt64InOutArg(pArgs, rRetVal, nNanoSecsPerSec);

    else if (aFuncName == u"QueryPerformanceCounter")
    {
        TimeValue aNow;
        osl_getSystemTime( &aNow );
        sal_Int64 nStamp = aNow.Nanosec + aNow.Seconds * nNanoSecsPerSec;
        return returnInt64InOutArg(pArgs, rRetVal, nStamp);
    }
    return ERRCODE_BASIC_NOT_IMPLEMENTED;
}

};

ErrCode SbiDllMgr::Call(
    std::u16string_view aFuncName, std::u16string_view aDllName,
    SbxArray *pArgs, SbxVariable &rRetVal,
    SAL_UNUSED_PARAMETER bool /* bCDecl */)
{
    if (aDllName == u"kernel32")
        return builtin_kernel32(aFuncName, pArgs, rRetVal);
    else
        return ERRCODE_BASIC_NOT_IMPLEMENTED;
}

void SbiDllMgr::FreeDll(SAL_UNUSED_PARAMETER OUString const &) {}

SbiDllMgr::SbiDllMgr() = default;

SbiDllMgr::~SbiDllMgr() = default;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
