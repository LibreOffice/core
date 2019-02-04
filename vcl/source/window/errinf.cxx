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

#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>

#include <tools/debug.hxx>
#include <vcl/errinf.hxx>
#include <vcl/window.hxx>

#include <vector>
#include <limits.h>

class ErrorHandler;
class TheErrorRegistry: public rtl::Static<ErrorRegistry, TheErrorRegistry> {};

class ErrorStringFactory
{
public:
    static bool CreateString(const ErrorInfo*, OUString&);
};

bool ErrorStringFactory::CreateString(const ErrorInfo* pInfo, OUString& rStr)
{
    for(const ErrorHandler *pHdlr : TheErrorRegistry::get().errorHandlers)
    {
        if(pHdlr->CreateString(pInfo, rStr))
            return true;
    }
    return false;
}

ErrorRegistry::ErrorRegistry()
    : pDsp(nullptr)
    , bIsWindowDsp(false)
    , nNextError(0)
{
    for(DynamicErrorInfo*& rp : ppDynErrInfo)
        rp = nullptr;
}

void ErrorRegistry::RegisterDisplay(BasicDisplayErrorFunc *aDsp)
{
    ErrorRegistry &rData = TheErrorRegistry::get();
    rData.bIsWindowDsp = false;
    rData.pDsp = reinterpret_cast< DisplayFnPtr >(aDsp);
}

void ErrorRegistry::RegisterDisplay(WindowDisplayErrorFunc *aDsp)
{
    ErrorRegistry &rData = TheErrorRegistry::get();
    rData.bIsWindowDsp = true;
    rData.pDsp = reinterpret_cast< DisplayFnPtr >(aDsp);
}

static void aDspFunc(const OUString &rErr, const OUString &rAction)
{
    SAL_WARN("vcl", "Action: " << rAction << " Error: " << rErr);
}

ErrorHandler::ErrorHandler()
{
    ErrorRegistry &rData = TheErrorRegistry::get();
    rData.errorHandlers.insert(rData.errorHandlers.begin(), this);

    if(!rData.pDsp)
        ErrorRegistry::RegisterDisplay(&aDspFunc);
}

ErrorHandler::~ErrorHandler()
{
    auto &rErrorHandlers = TheErrorRegistry::get().errorHandlers;
    rErrorHandlers.erase( ::std::remove(rErrorHandlers.begin(), rErrorHandlers.end(), this),
                          rErrorHandlers.end());
}

bool ErrorHandler::GetErrorString(ErrCode nErrCodeId, OUString& rErrStr)
{
    OUString aErr;

    if(!nErrCodeId || nErrCodeId == ERRCODE_ABORT)
        return false;

    std::unique_ptr<ErrorInfo> pInfo = ErrorInfo::GetErrorInfo(nErrCodeId);

    if (ErrorStringFactory::CreateString(pInfo.get(),aErr))
    {
        rErrStr = aErr;
        return true;
    }

    return false;
}

DialogMask ErrorHandler::HandleError(ErrCode nErrCodeId, weld::Window *pParent, DialogMask nFlags)
{
    if (nErrCodeId == ERRCODE_NONE || nErrCodeId == ERRCODE_ABORT)
        return DialogMask::NONE;

    ErrorRegistry &rData = TheErrorRegistry::get();
    std::unique_ptr<ErrorInfo> pInfo = ErrorInfo::GetErrorInfo(nErrCodeId);
    OUString aAction;

    if (!rData.contexts.empty())
    {
        rData.contexts.front()->GetString(pInfo->GetErrorCode(), aAction);

        for(ErrorContext *pCtx : rData.contexts)
        {
            if(pCtx->GetParent())
            {
                pParent = pCtx->GetParent();
                break;
            }
        }
    }

    bool bWarning = nErrCodeId.IsWarning();
    DialogMask nErrFlags = DialogMask::ButtonDefaultsOk | DialogMask::ButtonsOk;
    if (bWarning)
        nErrFlags |= DialogMask::MessageWarning;
    else
        nErrFlags |= DialogMask::MessageError;

    DynamicErrorInfo* pDynPtr = dynamic_cast<DynamicErrorInfo*>(pInfo.get());
    if(pDynPtr)
    {
        DialogMask nDynFlags = pDynPtr->GetDialogMask();
        if( nDynFlags != DialogMask::NONE )
            nErrFlags = nDynFlags;
    }

    OUString aErr;
    if (ErrorStringFactory::CreateString(pInfo.get(), aErr))
    {
        if(!rData.pDsp)
        {
            SAL_WARN( "vcl", "Action: " << aAction <<  "Error: " << aErr);
        }
        else
        {
            if(!rData.bIsWindowDsp)
            {
                (*reinterpret_cast<BasicDisplayErrorFunc*>(rData.pDsp))(aErr,aAction);
                return DialogMask::NONE;
            }
            else
            {
                if (nFlags != DialogMask::MAX)
                    nErrFlags = nFlags;

                return (*reinterpret_cast<WindowDisplayErrorFunc*>(rData.pDsp))(
                    pParent, nErrFlags, aErr, aAction);
            }
        }
    }

    SAL_WARN( "vcl", "Error not handled " << pInfo->GetErrorCode());
    // Error 1 (ERRCODE_ABORT) is classified as a General Error in sfx
    if (pInfo->GetErrorCode() != ERRCODE_ABORT)
        HandleError(ERRCODE_ABORT);
    else
        OSL_FAIL("ERRCODE_ABORT not handled");

    return DialogMask::NONE;
}

struct ImplErrorContext
{
    weld::Window *pWin;
};

ErrorContext::ErrorContext(weld::Window *pWinP)
    : pImpl( new ImplErrorContext )
{
    pImpl->pWin = pWinP;
    TheErrorRegistry::get().contexts.insert(TheErrorRegistry::get().contexts.begin(), this);
}

ErrorContext::~ErrorContext()
{
    auto &rContexts = TheErrorRegistry::get().contexts;
    rContexts.erase( ::std::remove(rContexts.begin(), rContexts.end(), this), rContexts.end());
}

ErrorContext *ErrorContext::GetContext()
{
    return TheErrorRegistry::get().contexts.empty() ? nullptr : TheErrorRegistry::get().contexts.front();
}

weld::Window* ErrorContext::GetParent()
{
    return pImpl ? pImpl->pWin : nullptr;
}

class ImplDynamicErrorInfo
{
    friend class DynamicErrorInfo;
    friend class ErrorInfo;

private:
    explicit ImplDynamicErrorInfo(DialogMask nInMask)
        : nMask(nInMask)
    {
    }
    void                        RegisterError(DynamicErrorInfo *);
    static void                 UnRegisterError(DynamicErrorInfo const *);
    static std::unique_ptr<ErrorInfo> GetDynamicErrorInfo(ErrCode nId);

    ErrCode                     nErrId;
    DialogMask const            nMask;

};

void ImplDynamicErrorInfo::RegisterError(DynamicErrorInfo *pDynErrInfo)
{
    // Register dynamic identifier
    ErrorRegistry& rData = TheErrorRegistry::get();
    nErrId = ErrCode(((sal_uInt32(rData.nNextError) + 1) << ERRCODE_DYNAMIC_SHIFT) +
                     sal_uInt32(pDynErrInfo->GetErrorCode()));

    if(rData.ppDynErrInfo[rData.nNextError])
        delete rData.ppDynErrInfo[rData.nNextError];

    rData.ppDynErrInfo[rData.nNextError] = pDynErrInfo;

    if(++rData.nNextError>=ERRCODE_DYNAMIC_COUNT)
        rData.nNextError=0;
}

void ImplDynamicErrorInfo::UnRegisterError(DynamicErrorInfo const *pDynErrInfo)
{
    DynamicErrorInfo **ppDynErrInfo = TheErrorRegistry::get().ppDynErrInfo;
    sal_uInt32 nIdx = ErrCode(*pDynErrInfo).GetDynamic() - 1;
    DBG_ASSERT(ppDynErrInfo[nIdx] == pDynErrInfo, "ErrHdl: Error not found");

    if(ppDynErrInfo[nIdx]==pDynErrInfo)
        ppDynErrInfo[nIdx]=nullptr;
}

std::unique_ptr<ErrorInfo> ImplDynamicErrorInfo::GetDynamicErrorInfo(ErrCode nId)
{
    sal_uInt32 nIdx = nId.GetDynamic() - 1;
    DynamicErrorInfo* pDynErrInfo = TheErrorRegistry::get().ppDynErrInfo[nIdx];

    if(pDynErrInfo && ErrCode(*pDynErrInfo)==nId)
        return std::unique_ptr<ErrorInfo>(pDynErrInfo);
    else
        return std::make_unique<ErrorInfo>(nId.StripDynamic());
}

std::unique_ptr<ErrorInfo> ErrorInfo::GetErrorInfo(ErrCode nId)
{
    if(nId.IsDynamic())
        return ImplDynamicErrorInfo::GetDynamicErrorInfo(nId);
    else
        return std::make_unique<ErrorInfo>(nId);
}

ErrorInfo::~ErrorInfo()
{
}

DynamicErrorInfo::DynamicErrorInfo(ErrCode nArgUserId, DialogMask nMask)
: ErrorInfo(nArgUserId),
  pImpl(new ImplDynamicErrorInfo(nMask))
{
    pImpl->RegisterError(this);
}

DynamicErrorInfo::~DynamicErrorInfo()
{
    ImplDynamicErrorInfo::UnRegisterError(this);
}

DynamicErrorInfo::operator ErrCode() const
{
    return pImpl->nErrId;
}

DialogMask DynamicErrorInfo::GetDialogMask() const
{
    return pImpl->nMask;
}

StringErrorInfo::StringErrorInfo(
    ErrCode nArgUserId, const OUString& aStringP, DialogMask nMask)
: DynamicErrorInfo(nArgUserId, nMask), aString(aStringP)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
