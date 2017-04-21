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

#include <limits.h>
#include <vcl/errinf.hxx>
#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>
#include <vcl/window.hxx>
#include <vector>

class ErrorHandler;

namespace {
  typedef void (* DisplayFnPtr)();
}

struct ErrorRegistry
{
public:
    std::vector<ErrorHandler*>  errorHandlers;
    std::vector<ErrorContext*>  contexts;
    DisplayFnPtr                pDsp;
    bool                        bIsWindowDsp;

    DynamicErrorInfo*           ppDynErrInfo[ERRCODE_DYNAMIC_COUNT];
    sal_uInt16                  nNextError;
                                ErrorRegistry();
};

struct TheErrorRegistry: public rtl::Static<ErrorRegistry, TheErrorRegistry> {};

class DynamicErrorInfo_Impl
{
private:
    friend class DynamicErrorInfo;
    friend class ErrorInfo;

    ErrCode                     lErrId;
    DialogMask                  nMask;

    void                        RegisterError(DynamicErrorInfo *);
    static void                 UnRegisterError(DynamicErrorInfo const *);
    static ErrorInfo*           GetDynamicErrorInfo(sal_uInt32 lId);

};

ErrorRegistry::ErrorRegistry()
    : pDsp(nullptr)
    , bIsWindowDsp(false)
    , nNextError(0)
{
    for(DynamicErrorInfo*& rp : ppDynErrInfo)
        rp = nullptr;
}

void DynamicErrorInfo_Impl::RegisterError(DynamicErrorInfo *pDynErrInfo)
{
    // Register dynamic identifier
    ErrorRegistry& rData = TheErrorRegistry::get();
    lErrId = (((sal_uInt32)rData.nNextError + 1) << ERRCODE_DYNAMIC_SHIFT) +
             pDynErrInfo->GetErrorCode();

    if(rData.ppDynErrInfo[rData.nNextError])
        delete rData.ppDynErrInfo[rData.nNextError];

    rData.ppDynErrInfo[rData.nNextError] = pDynErrInfo;

    if(++rData.nNextError>=ERRCODE_DYNAMIC_COUNT)
        rData.nNextError=0;
}

void DynamicErrorInfo_Impl::UnRegisterError(DynamicErrorInfo const *pDynErrInfo)
{
    DynamicErrorInfo **ppDynErrInfo = TheErrorRegistry::get().ppDynErrInfo;
    sal_uInt32 lIdx = (((sal_uInt32)(*pDynErrInfo) & ERRCODE_DYNAMIC_MASK) >> ERRCODE_DYNAMIC_SHIFT) - 1;
    DBG_ASSERT(ppDynErrInfo[lIdx] == pDynErrInfo,"ErrHdl: Error not found");

    if(ppDynErrInfo[lIdx]==pDynErrInfo)
        ppDynErrInfo[lIdx]=nullptr;
}

ErrorInfo::~ErrorInfo()
{
}


ErrorInfo *ErrorInfo::GetErrorInfo(sal_uInt32 lId)
{
    if(lId & ERRCODE_DYNAMIC_MASK)
        return DynamicErrorInfo_Impl::GetDynamicErrorInfo(lId);
    else
        return new ErrorInfo(lId);
}

DynamicErrorInfo::operator sal_uInt32() const
{
    return pImpl->lErrId;
}

DynamicErrorInfo::DynamicErrorInfo(sal_uInt32 lArgUserId, DialogMask nMask)
: ErrorInfo(lArgUserId),
  pImpl(new DynamicErrorInfo_Impl)
{
    pImpl->RegisterError(this);
    pImpl->nMask=nMask;
}

DynamicErrorInfo::~DynamicErrorInfo()
{
    DynamicErrorInfo_Impl::UnRegisterError(this);
}

ErrorInfo* DynamicErrorInfo_Impl::GetDynamicErrorInfo(sal_uInt32 lId)
{
    sal_uInt32 lIdx = ((lId & ERRCODE_DYNAMIC_MASK)>>ERRCODE_DYNAMIC_SHIFT)-1;
    DynamicErrorInfo* pDynErrInfo = TheErrorRegistry::get().ppDynErrInfo[lIdx];

    if(pDynErrInfo && (sal_uInt32)(*pDynErrInfo)==lId)
        return pDynErrInfo;
    else
        return new ErrorInfo(lId & ~ERRCODE_DYNAMIC_MASK);
}

DialogMask DynamicErrorInfo::GetDialogMask() const
{
    return pImpl->nMask;
}

StringErrorInfo::StringErrorInfo(
    sal_uInt32 UserId, const OUString& aStringP, DialogMask nMask)
: DynamicErrorInfo(UserId, nMask), aString(aStringP)
{
}

class ErrorHandler_Impl
{
public:
    static bool         CreateString(const ErrorInfo*, OUString&);
};

static void aDspFunc(const OUString &rErr, const OUString &rAction)
{
    OStringBuffer aErr("Action: ");
    aErr.append(OUStringToOString(rAction, RTL_TEXTENCODING_ASCII_US));
    aErr.append(" Error: ");
    aErr.append(OUStringToOString(rErr, RTL_TEXTENCODING_ASCII_US));
    OSL_FAIL(aErr.getStr());
}

struct ErrorContextImpl
{
    vcl::Window  *pWin; // FIXME: should be VclPtr for strong lifecycle
};

ErrorContext::ErrorContext(vcl::Window *pWinP)
    : pImpl( new ErrorContextImpl )
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


ErrorHandler::ErrorHandler()
{
    ErrorRegistry &rData = TheErrorRegistry::get();
    rData.errorHandlers.insert(rData.errorHandlers.begin(), this);

    if(!rData.pDsp)
        RegisterDisplay(&aDspFunc);
}

ErrorHandler::~ErrorHandler()
{
    auto &rErrorHandlers = TheErrorRegistry::get().errorHandlers;
    rErrorHandlers.erase( ::std::remove(rErrorHandlers.begin(), rErrorHandlers.end(), this),
                          rErrorHandlers.end());
}

vcl::Window* ErrorContext::GetParent()
{
    return pImpl ? pImpl->pWin : nullptr;
}

void ErrorHandler::RegisterDisplay(WindowDisplayErrorFunc *aDsp)
{
    ErrorRegistry &rData    = TheErrorRegistry::get();
    rData.bIsWindowDsp = true;
    rData.pDsp = reinterpret_cast< DisplayFnPtr >(aDsp);
}

void ErrorHandler::RegisterDisplay(BasicDisplayErrorFunc *aDsp)
{
    ErrorRegistry &rData = TheErrorRegistry::get();
    rData.bIsWindowDsp = false;
    rData.pDsp = reinterpret_cast< DisplayFnPtr >(aDsp);
}

/** Handles an error.

    If nFlags is not set, the DynamicErrorInfo flags or the
    resource flags will be used.
    Thus:

    1. nFlags,
    2. Resource Flags
    3. Dynamic Flags
    4. Default ButtonsOk, MessageError

    @param nErrCodeId        error id
    @param nFlags            error flags.
    @param bJustCreateString ???
    @param rError            ???

    @return ???
*/
DialogMask ErrorHandler::HandleError_Impl(
    sal_uInt32 nErrCodeId, DialogMask nFlags, bool bJustCreateString, OUString & rError)
{
    OUString aErr;
    OUString aAction;

    if(!nErrCodeId || nErrCodeId == ERRCODE_ABORT)
        return DialogMask::NONE;

    ErrorRegistry &rData = TheErrorRegistry::get();
    vcl::Window *pParent = nullptr;
    ErrorInfo *pInfo = ErrorInfo::GetErrorInfo(nErrCodeId);
    if (!rData.contexts.empty())
    {
        rData.contexts.front()->GetString(pInfo->GetErrorCode(), aAction);
        // Remove parent from context
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

    if(ErrorHandler_Impl::CreateString(pInfo,aErr))
    {
        if (bJustCreateString)
        {
            rError = aErr;
            return DialogMask::ButtonsOk;
        }
        else
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
    }

    OSL_FAIL("Error not handled");
    // Error 1 is classified as a General Error in sfx
    if(pInfo->GetErrorCode()!=1)
        HandleError_Impl(1, DialogMask::MAX, bJustCreateString, rError);
    else
        OSL_FAIL("Error 1 not handled");

    delete pInfo;
    return DialogMask::NONE;
}

// static
bool ErrorHandler::GetErrorString(sal_uInt32 lId, OUString& rStr)
{
    return HandleError_Impl( lId, DialogMask::MAX, true, rStr ) != DialogMask::NONE;
}

/** Handles an error.

    @see ErrorHandler::HandleError_Impl
*/
DialogMask ErrorHandler::HandleError(sal_uInt32 lId, DialogMask nFlags)
{
    OUString aDummy;
    return HandleError_Impl( lId, nFlags, false, aDummy );
}

bool ErrorHandler_Impl::CreateString(const ErrorInfo* pInfo, OUString& rStr)
{
    for(const ErrorHandler *pHdl : TheErrorRegistry::get().errorHandlers)
    {
        if(pHdl->CreateString(pInfo, rStr))
            return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
