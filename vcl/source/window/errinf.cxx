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
    OStringBuffer aErr("Action: ");
    aErr.append(OUStringToOString(rAction, RTL_TEXTENCODING_ASCII_US));
    aErr.append(" Error: ");
    aErr.append(OUStringToOString(rErr, RTL_TEXTENCODING_ASCII_US));
    OSL_FAIL(aErr.getStr());
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

bool ErrorHandler::GetErrorString(sal_uInt32 nErrCodeId, OUString& rErrStr)
{
    OUString aErr;

    if(!nErrCodeId || nErrCodeId == ERRCODE_ABORT)
        return false;

    ErrorInfo *pInfo = ErrorInfo::GetErrorInfo(nErrCodeId);

    if (ErrorStringFactory::CreateString(pInfo,aErr))
    {
        rErrStr = aErr;
        return true;
    }

    delete pInfo;
    return false;
}

DialogMask ErrorHandler::HandleError(sal_uInt32 nErrCodeId, DialogMask nFlags)
{
    if (nErrCodeId == ERRCODE_NONE || nErrCodeId == ERRCODE_ABORT)
        return DialogMask::NONE;

    ErrorRegistry &rData = TheErrorRegistry::get();
    vcl::Window *pParent = nullptr;
    ErrorInfo *pInfo = ErrorInfo::GetErrorInfo(nErrCodeId);
    OUString aAction;

    if (!rData.contexts.empty())
    {
        rData.contexts.front()->GetString(pInfo->GetErrorCode(), aAction);

        for(ErrorContext *pCtx : rData.contexts)
        {
            if(pCtx->GetParent())
            {
                pParent=pCtx->GetParent();
                break;
            }
        }
    }

    bool bWarning = ((nErrCodeId & ERRCODE_WARNING_MASK) == ERRCODE_WARNING_MASK);
    DialogMask nErrFlags = DialogMask::ButtonDefaultsOk | DialogMask::ButtonsOk;
    if (bWarning)
        nErrFlags |= DialogMask::MessageWarning;
    else
        nErrFlags |= DialogMask::MessageError;

    DynamicErrorInfo* pDynPtr = dynamic_cast<DynamicErrorInfo*>(pInfo);
    if(pDynPtr)
    {
        DialogMask nDynFlags = pDynPtr->GetDialogMask();
        if( nDynFlags != DialogMask::NONE )
            nErrFlags = nDynFlags;
    }

    OUString aErr;
    if (ErrorStringFactory::CreateString(pInfo, aErr))
    {
        if(!rData.pDsp)
        {
            OStringBuffer aStr("Action: ");
            aStr.append(OUStringToOString(aAction, RTL_TEXTENCODING_ASCII_US));
            aStr.append("\nError: ");
            aStr.append(OUStringToOString(aErr, RTL_TEXTENCODING_ASCII_US));
            OSL_FAIL(aStr.getStr());
        }
        else
        {
            delete pInfo;

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

    OSL_FAIL("Error not handled");
    // Error 1 (ERRCODE_ABORT) is classified as a General Error in sfx
    if (pInfo->GetErrorCode() != ERRCODE_ABORT)
        HandleError(ERRCODE_ABORT);
    else
        OSL_FAIL("ERRCODE_ABORT not handled");

    delete pInfo;
    return DialogMask::NONE;
}

struct ImplErrorContext
{
    vcl::Window *pWin; // FIXME: should be VclPtr for strong lifecycle
};

ErrorContext::ErrorContext(vcl::Window *pWinP)
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

vcl::Window* ErrorContext::GetParent()
{
    return pImpl ? pImpl->pWin : nullptr;
}

class ImplDynamicErrorInfo
{
    friend class DynamicErrorInfo;
    friend class ErrorInfo;

private:
    void                        RegisterError(DynamicErrorInfo *);
    static void                 UnRegisterError(DynamicErrorInfo const *);
    static ErrorInfo*           GetDynamicErrorInfo(sal_uInt32 nId);

    ErrCode                     nErrId;
    DialogMask                  nMask;

};

void ImplDynamicErrorInfo::RegisterError(DynamicErrorInfo *pDynErrInfo)
{
    // Register dynamic identifier
    ErrorRegistry& rData = TheErrorRegistry::get();
    nErrId = (((sal_uInt32)rData.nNextError + 1) << ERRCODE_DYNAMIC_SHIFT) +
             pDynErrInfo->GetErrorCode();

    if(rData.ppDynErrInfo[rData.nNextError])
        delete rData.ppDynErrInfo[rData.nNextError];

    rData.ppDynErrInfo[rData.nNextError] = pDynErrInfo;

    if(++rData.nNextError>=ERRCODE_DYNAMIC_COUNT)
        rData.nNextError=0;
}

void ImplDynamicErrorInfo::UnRegisterError(DynamicErrorInfo const *pDynErrInfo)
{
    DynamicErrorInfo **ppDynErrInfo = TheErrorRegistry::get().ppDynErrInfo;
    sal_uInt32 nIdx = (((sal_uInt32)(*pDynErrInfo) & ERRCODE_DYNAMIC_MASK) >> ERRCODE_DYNAMIC_SHIFT) - 1;
    DBG_ASSERT(ppDynErrInfo[nIdx] == pDynErrInfo, "ErrHdl: Error not found");

    if(ppDynErrInfo[nIdx]==pDynErrInfo)
        ppDynErrInfo[nIdx]=nullptr;
}

ErrorInfo* ImplDynamicErrorInfo::GetDynamicErrorInfo(sal_uInt32 nId)
{
    sal_uInt32 nIdx = ((nId & ERRCODE_DYNAMIC_MASK)>>ERRCODE_DYNAMIC_SHIFT)-1;
    DynamicErrorInfo* pDynErrInfo = TheErrorRegistry::get().ppDynErrInfo[nIdx];

    if(pDynErrInfo && (sal_uInt32)(*pDynErrInfo)==nId)
        return pDynErrInfo;
    else
        return new ErrorInfo(nId & ~ERRCODE_DYNAMIC_MASK);
}

ErrorInfo *ErrorInfo::GetErrorInfo(sal_uInt32 nId)
{
    if(nId & ERRCODE_DYNAMIC_MASK)
        return ImplDynamicErrorInfo::GetDynamicErrorInfo(nId);
    else
        return new ErrorInfo(nId);
}

ErrorInfo::~ErrorInfo()
{
}

DynamicErrorInfo::DynamicErrorInfo(sal_uInt32 nArgUserId, DialogMask nMask)
: ErrorInfo(nArgUserId),
  pImpl(new ImplDynamicErrorInfo)
{
    pImpl->RegisterError(this);
    pImpl->nMask=nMask;
}

DynamicErrorInfo::~DynamicErrorInfo()
{
    ImplDynamicErrorInfo::UnRegisterError(this);
}

DynamicErrorInfo::operator sal_uInt32() const
{
    return pImpl->nErrId;
}

DialogMask DynamicErrorInfo::GetDialogMask() const
{
    return pImpl->nMask;
}

StringErrorInfo::StringErrorInfo(
    sal_uInt32 nArgUserId, const OUString& aStringP, DialogMask nMask)
: DynamicErrorInfo(nArgUserId, nMask), aString(aStringP)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
