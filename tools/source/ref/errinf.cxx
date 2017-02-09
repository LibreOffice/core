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
#include <tools/errinf.hxx>
#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>
#include <vcl/window.hxx>
#include <vector>

class ErrorHandler;

namespace {
  typedef void (* DisplayFnPtr)();
}

struct EDcrData
{
public:
    std::vector<ErrorHandler*>  errorHandlers;
    std::vector<ErrorContext*>  contexts;
    DisplayFnPtr                pDsp;
    bool                        bIsWindowDsp;

    DynamicErrorInfo            *ppDcr[ERRCODE_DYNAMIC_COUNT];
    sal_uInt16                  nNextDcr;
                                EDcrData();
};

struct TheEDcrData: public rtl::Static<EDcrData, TheEDcrData> {};

class DynamicErrorInfo_Impl
{
    ErrCode                 lErrId;
    ErrorHandlerFlags           nMask;

    void                        RegisterEDcr(DynamicErrorInfo *);
    static void                 UnRegisterEDcr(DynamicErrorInfo const *);
    static ErrorInfo           *GetDynamicErrorInfo(ErrCode lId);

friend class DynamicErrorInfo;
friend class ErrorInfo;
};

EDcrData::EDcrData()
    : pDsp(nullptr)
    , bIsWindowDsp(false)
    , nNextDcr(0)
{
    for(DynamicErrorInfo*& rp : ppDcr)
        rp = nullptr;
}

void DynamicErrorInfo_Impl::RegisterEDcr(DynamicErrorInfo *pDcr)
{
    // Register dynamic identifier
    EDcrData& rData = TheEDcrData::get();
    lErrId = ((rData.nNextDcr + 1) << ERRCODE_DYNAMIC_SHIFT) +
             (sal_uInt32)pDcr->GetErrorCode();

    if(rData.ppDcr[rData.nNextDcr])
    {
        delete rData.ppDcr[rData.nNextDcr];
    }
    rData.ppDcr[rData.nNextDcr] = pDcr;
    if(++rData.nNextDcr>=ERRCODE_DYNAMIC_COUNT)
        rData.nNextDcr=0;
}

void DynamicErrorInfo_Impl::UnRegisterEDcr(DynamicErrorInfo const *pDcr)
{
    DynamicErrorInfo **ppDcr = TheEDcrData::get().ppDcr;
    sal_uInt32 lIdx = ((((sal_uInt32)pDcr->GetErrorCode()) & ERRCODE_DYNAMIC_MASK) >> ERRCODE_DYNAMIC_SHIFT) - 1;
    DBG_ASSERT(ppDcr[lIdx]==pDcr,"ErrHdl: Error nicht gefunden");
    if(ppDcr[lIdx]==pDcr)
        ppDcr[lIdx]=nullptr;
}

ErrorInfo::~ErrorInfo()
{
}


ErrorInfo *ErrorInfo::GetErrorInfo(ErrCode lId)
{
    if((sal_uInt32)lId & ERRCODE_DYNAMIC_MASK)
        return DynamicErrorInfo_Impl::GetDynamicErrorInfo(lId);
    else
        return new ErrorInfo(lId);
}

DynamicErrorInfo::operator ErrCode() const
{
    return pImpl->lErrId;
}

DynamicErrorInfo::DynamicErrorInfo(sal_uIntPtr lArgUserId, ErrorHandlerFlags nMask)
: ErrorInfo(lArgUserId),
  pImpl(new DynamicErrorInfo_Impl)
{
    pImpl->RegisterEDcr(this);
    pImpl->nMask=nMask;
}

DynamicErrorInfo::~DynamicErrorInfo()
{
    DynamicErrorInfo_Impl::UnRegisterEDcr(this);
}

ErrorInfo* DynamicErrorInfo_Impl::GetDynamicErrorInfo(ErrCode nErrCode)
{
    sal_uIntPtr lIdx = (((sal_uInt32)nErrCode & ERRCODE_DYNAMIC_MASK)>>ERRCODE_DYNAMIC_SHIFT)-1;
    DynamicErrorInfo* pDcr = TheEDcrData::get().ppDcr[lIdx];
    if(pDcr && static_cast<ErrCode>(*pDcr) == nErrCode)
        return pDcr;
    else
        return new ErrorInfo((sal_uInt32)nErrCode & ~ERRCODE_DYNAMIC_MASK);
}

ErrorHandlerFlags DynamicErrorInfo::GetDialogMask() const
{
    return pImpl->nMask;
}

StringErrorInfo::StringErrorInfo(
    sal_uIntPtr UserId, const OUString& aStringP, ErrorHandlerFlags nMask)
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
    OStringBuffer aErr("Aktion: ");
    aErr.append(OUStringToOString(rAction, RTL_TEXTENCODING_ASCII_US));
    aErr.append(" Fehler: ");
    aErr.append(OUStringToOString(rErr, RTL_TEXTENCODING_ASCII_US));
    OSL_FAIL(aErr.getStr());
}

// FIXME: this is a horrible reverse dependency on VCL
struct ErrorContextImpl
{
    vcl::Window  *pWin; // should be VclPtr for strong lifecycle
};

ErrorContext::ErrorContext(vcl::Window *pWinP)
    : pImpl( new ErrorContextImpl )
{
    pImpl->pWin = pWinP;
    TheEDcrData::get().contexts.insert(TheEDcrData::get().contexts.begin(), this);
}

ErrorContext::~ErrorContext()
{
    auto &rContexts = TheEDcrData::get().contexts;
    rContexts.erase( ::std::remove(rContexts.begin(), rContexts.end(), this), rContexts.end());
}

ErrorContext *ErrorContext::GetContext()
{
    return TheEDcrData::get().contexts.empty() ? nullptr : TheEDcrData::get().contexts.front();
}


ErrorHandler::ErrorHandler()
{
    EDcrData &rData = TheEDcrData::get();
    rData.errorHandlers.insert(rData.errorHandlers.begin(), this);
    if(!rData.pDsp)
        RegisterDisplay(&aDspFunc);
}

ErrorHandler::~ErrorHandler()
{
    auto &rErrorHandlers = TheEDcrData::get().errorHandlers;
    rErrorHandlers.erase( ::std::remove(rErrorHandlers.begin(), rErrorHandlers.end(), this), rErrorHandlers.end());
}

vcl::Window* ErrorContext::GetParent()
{
    return pImpl ? pImpl->pWin : nullptr;
}

void ErrorHandler::RegisterDisplay(WindowDisplayErrorFunc *aDsp)
{
    EDcrData &rData    = TheEDcrData::get();
    rData.bIsWindowDsp = true;
    rData.pDsp         = reinterpret_cast< DisplayFnPtr >(aDsp);
}

void ErrorHandler::RegisterDisplay(BasicDisplayErrorFunc *aDsp)
{
    EDcrData &rData    = TheEDcrData::get();
    rData.bIsWindowDsp = false;
    rData.pDsp         = reinterpret_cast< DisplayFnPtr >(aDsp);
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
ErrorHandlerFlags ErrorHandler::HandleError_Impl(
    ErrCode nErrCodeId, ErrorHandlerFlags nFlags, bool bJustCreateString, OUString & rError)
{
    OUString aErr;
    OUString aAction;
    if(!nErrCodeId || nErrCodeId == ERRCODE_ABORT)
        return ErrorHandlerFlags::NONE;
    EDcrData &rData      = TheEDcrData::get();
    vcl::Window *pParent = nullptr;
    ErrorInfo *pInfo     = ErrorInfo::GetErrorInfo(nErrCodeId);
    if (!rData.contexts.empty())
    {
        rData.contexts.front()->GetString(pInfo->GetErrorCode(), aAction);
        // Remove parent from context
        for(ErrorContext *pCtx : rData.contexts)
            if(pCtx->GetParent())
            {
                pParent=pCtx->GetParent();
                break;
            }
    }

    bool bWarning = nErrCodeId.IsWarning();
    ErrorHandlerFlags nErrFlags = ErrorHandlerFlags::ButtonDefaultsOk | ErrorHandlerFlags::ButtonsOk;
    if (bWarning)
        nErrFlags |= ErrorHandlerFlags::MessageWarning;
    else
        nErrFlags |= ErrorHandlerFlags::MessageError;

    DynamicErrorInfo* pDynPtr=dynamic_cast<DynamicErrorInfo*>(pInfo);
    if(pDynPtr)
    {
        ErrorHandlerFlags nDynFlags = pDynPtr->GetDialogMask();
        if( nDynFlags != ErrorHandlerFlags::NONE )
            nErrFlags = nDynFlags;
    }

    if(ErrorHandler_Impl::CreateString(pInfo,aErr))
    {
        if (bJustCreateString)
        {
            rError = aErr;
            return ErrorHandlerFlags::ButtonsOk;
        }
        else
        {
            if(!rData.pDsp)
            {
                OStringBuffer aStr("Action: ");
                aStr.append(OUStringToOString(aAction, RTL_TEXTENCODING_ASCII_US));
                aStr.append("\nFehler: ");
                aStr.append(OUStringToOString(aErr, RTL_TEXTENCODING_ASCII_US));
                OSL_FAIL(aStr.getStr());
            }
            else
            {
                delete pInfo;
                if(!rData.bIsWindowDsp)
                {
                    (*reinterpret_cast<BasicDisplayErrorFunc*>(rData.pDsp))(aErr,aAction);
                    return ErrorHandlerFlags::NONE;
                }
                else
                {
                    if (nFlags != ErrorHandlerFlags::MAX)
                        nErrFlags = nFlags;
                    return (*reinterpret_cast<WindowDisplayErrorFunc*>(rData.pDsp))(
                        pParent, nErrFlags, aErr, aAction);
                }
            }
        }
    }
    OSL_FAIL("Error nicht behandelt");
    // Error 1 is General Error in the Sfx
    if(pInfo->GetErrorCode()!=1)
    {
        HandleError_Impl(1, ErrorHandlerFlags::MAX, bJustCreateString, rError);
    }
    else
    {
        OSL_FAIL("Error 1 nicht gehandeled");
    }
    delete pInfo;
    return ErrorHandlerFlags::NONE;
}

// static
bool ErrorHandler::GetErrorString(sal_uIntPtr lId, OUString& rStr)
{
    return HandleError_Impl( lId, ErrorHandlerFlags::MAX, true, rStr ) != ErrorHandlerFlags::NONE;
}

/** Handles an error.

    @see ErrorHandler::HandleError_Impl
*/
ErrorHandlerFlags ErrorHandler::HandleError(sal_uIntPtr lId, ErrorHandlerFlags nFlags)
{
    OUString aDummy;
    return HandleError_Impl( lId, nFlags, false, aDummy );
}

bool ErrorHandler_Impl::CreateString(const ErrorInfo* pInfo, OUString& rStr)
{
    for(const ErrorHandler *pHdl : TheEDcrData::get().errorHandlers)
    {
        if(pHdl->CreateString(pInfo, rStr))
            return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
