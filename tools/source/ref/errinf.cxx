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
#include <tools/debug.hxx>
#include <tools/errinf.hxx>
#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>
#include <vcl/window.hxx>

class ErrorHandler;

namespace {
typedef void (* DisplayFnPtr)();
}

struct EDcrData
{
public:
    ErrorHandler               *pFirstHdl;
    ErrorContext               *pFirstCtx;
    DisplayFnPtr               pDsp;
    bool                   bIsWindowDsp;

    DynamicErrorInfo            *ppDcr[ERRCODE_DYNAMIC_COUNT];
    sal_uInt16                  nNextDcr;
                                EDcrData();
};

struct TheEDcrData: public rtl::Static<EDcrData, TheEDcrData> {};

class EDcr_Impl
{
    sal_uIntPtr                 lErrId;
    sal_uInt16                  nMask;

    void                        RegisterEDcr(DynamicErrorInfo *);
    static void                 UnRegisterEDcr(DynamicErrorInfo *);
    static ErrorInfo           *GetDynamicErrorInfo(sal_uIntPtr lId);

friend class DynamicErrorInfo;
friend class ErrorInfo;
};

EDcrData::EDcrData()
    : pFirstHdl(0)
    , pFirstCtx(0)
    , pDsp(0)
    , bIsWindowDsp(false)
    , nNextDcr(0)
{
    for(sal_uInt16 n=0;n<ERRCODE_DYNAMIC_COUNT;n++)
        ppDcr[n]=0;
}

void EDcr_Impl::RegisterEDcr(DynamicErrorInfo *pDcr)
{
    // Register dynamic identifier
    EDcrData& pData=TheEDcrData::get();
    lErrId= (((sal_uIntPtr)pData.nNextDcr + 1) << ERRCODE_DYNAMIC_SHIFT) +
        pDcr->GetErrorCode();
    DynamicErrorInfo **ppDcr=pData.ppDcr;
    sal_uInt16 nNext=pData.nNextDcr;

    if(ppDcr[nNext])
    {
        delete ppDcr[nNext];
    }
    ppDcr[nNext]=pDcr;
    if(++pData.nNextDcr>=ERRCODE_DYNAMIC_COUNT)
        pData.nNextDcr=0;
}

void EDcr_Impl::UnRegisterEDcr(DynamicErrorInfo *pDcr)
{
    DynamicErrorInfo **ppDcr=TheEDcrData::get().ppDcr;
    sal_uIntPtr lIdx=(
        ((sal_uIntPtr)(*pDcr) & ERRCODE_DYNAMIC_MASK)>>ERRCODE_DYNAMIC_SHIFT)-1;
    DBG_ASSERT(ppDcr[lIdx]==pDcr,"ErrHdl: Error nicht gefunden");
    if(ppDcr[lIdx]==pDcr)
        ppDcr[lIdx]=0;
}

TYPEINIT0(ErrorInfo);
TYPEINIT1(DynamicErrorInfo, ErrorInfo);
TYPEINIT1(StringErrorInfo, DynamicErrorInfo);
TYPEINIT1(TwoStringErrorInfo, DynamicErrorInfo);
TYPEINIT1(MessageInfo, DynamicErrorInfo);

ErrorInfo *ErrorInfo::GetErrorInfo(sal_uIntPtr lId)
{
    if(lId & ERRCODE_DYNAMIC_MASK)
        return EDcr_Impl::GetDynamicErrorInfo(lId);
    else
        return new ErrorInfo(lId);
}

DynamicErrorInfo::operator sal_uIntPtr() const
{
    return pImpl->lErrId;
}

DynamicErrorInfo::DynamicErrorInfo(sal_uIntPtr lArgUserId, sal_uInt16 nMask)
: ErrorInfo(lArgUserId)
{
    pImpl=new EDcr_Impl;
    pImpl->RegisterEDcr(this);
    pImpl->nMask=nMask;
}

DynamicErrorInfo::~DynamicErrorInfo()
{
    EDcr_Impl::UnRegisterEDcr(this);
    delete pImpl;
}

ErrorInfo* EDcr_Impl::GetDynamicErrorInfo(sal_uIntPtr lId)
{
    sal_uIntPtr lIdx=((lId & ERRCODE_DYNAMIC_MASK)>>ERRCODE_DYNAMIC_SHIFT)-1;
    DynamicErrorInfo* pDcr=TheEDcrData::get().ppDcr[lIdx];
    if(pDcr && (sal_uIntPtr)(*pDcr)==lId)
        return pDcr;
    else
        return new ErrorInfo(lId & ~ERRCODE_DYNAMIC_MASK);
}

sal_uInt16 DynamicErrorInfo::GetDialogMask() const
{
    return pImpl->nMask;
}

StringErrorInfo::StringErrorInfo(
    sal_uIntPtr UserId, const OUString& aStringP,  sal_uInt16 nFlags)
: DynamicErrorInfo(UserId, nFlags), aString(aStringP)
{
}

class ErrHdl_Impl
{
public:
    ErrorHandler        *pNext;
    static bool         CreateString(const ErrorHandler *pStart,
                                     const ErrorInfo*, OUString&, sal_uInt16&);
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
    ErrorContext *pNext;
    vcl::Window  *pWin; // should be VclPtr for strong lifecycle
};

ErrorContext::ErrorContext(vcl::Window *pWinP)
{
    pImpl = new ErrorContextImpl();
    ErrorContext *&pHdl = TheEDcrData::get().pFirstCtx;
    pImpl->pWin = pWinP;
    pImpl->pNext = pHdl;
    pHdl = this;
}

ErrorContext::~ErrorContext()
{
    ErrorContext **ppCtx=&(TheEDcrData::get().pFirstCtx);
    while(*ppCtx && *ppCtx!=this)
        ppCtx=&((*ppCtx)->pImpl->pNext);
    if(*ppCtx)
        *ppCtx=(*ppCtx)->pImpl->pNext;
    delete pImpl;
}

ErrorContext *ErrorContext::GetContext()
{
    return TheEDcrData::get().pFirstCtx;
}

ErrorHandler::ErrorHandler()
{
    pImpl=new ErrHdl_Impl;
    EDcrData &pData=TheEDcrData::get();
    ErrorHandler *&pHdl=pData.pFirstHdl;
    pImpl->pNext=pHdl;
    pHdl=this;
    if(!pData.pDsp)
        RegisterDisplay(&aDspFunc);
}

ErrorHandler::~ErrorHandler()
{
    ErrorHandler **ppHdl=&(TheEDcrData::get().pFirstHdl);
    while(*ppHdl && *ppHdl!=this)
        ppHdl=&((*ppHdl)->pImpl->pNext);
    if(*ppHdl)
        *ppHdl=(*ppHdl)->pImpl->pNext;
    delete pImpl;
}

vcl::Window* ErrorContext::GetParent()
{
    return pImpl ? pImpl->pWin : NULL;
}

void ErrorHandler::RegisterDisplay(WindowDisplayErrorFunc *aDsp)
{
    EDcrData &pData=TheEDcrData::get();
    pData.bIsWindowDsp=true;
    pData.pDsp = reinterpret_cast< DisplayFnPtr >(aDsp);
}

void ErrorHandler::RegisterDisplay(BasicDisplayErrorFunc *aDsp)
{
    EDcrData &pData=TheEDcrData::get();
    pData.bIsWindowDsp=false;
    pData.pDsp = reinterpret_cast< DisplayFnPtr >(aDsp);
}

/** Handles an error.

    If nFlags is not set, the DynamicErrorInfo flags or the
    resource flags will be used.
    Thus:

    1. nFlags,
    2. Resource Flags
    3. Dynamic Flags
    4. Default ERRCODE_BUTTON_OK, ERRCODE_MSG_ERROR

    @param lId               error id
    @param nFlags            error flags.
    @param bJustCreateString ???
    @param rError            ???

    @return ???
*/
sal_uInt16 ErrorHandler::HandleError_Impl(
    sal_uIntPtr lId, sal_uInt16 nFlags, bool bJustCreateString, OUString & rError)
{
    OUString aErr;
    OUString aAction;
    if(!lId || lId == ERRCODE_ABORT)
        return 0;
    EDcrData &pData=TheEDcrData::get();
    ErrorInfo *pInfo=ErrorInfo::GetErrorInfo(lId);
    ErrorContext *pCtx=ErrorContext::GetContext();
    if(pCtx)
        pCtx->GetString(pInfo->GetErrorCode(), aAction);
    vcl::Window *pParent=0;
    // Remove parent from context
    for(;pCtx;pCtx=pCtx->pImpl->pNext)
        if(pCtx->GetParent())
        {
            pParent=pCtx->GetParent();
            break;
        }

    bool bWarning = ((lId & ERRCODE_WARNING_MASK) == ERRCODE_WARNING_MASK);
    sal_uInt16 nErrFlags = ERRCODE_BUTTON_DEF_OK | ERRCODE_BUTTON_OK;
    if (bWarning)
        nErrFlags |= ERRCODE_MSG_WARNING;
    else
        nErrFlags |= ERRCODE_MSG_ERROR;

    DynamicErrorInfo* pDynPtr=PTR_CAST(DynamicErrorInfo,pInfo);
    if(pDynPtr)
    {
        sal_uInt16 nDynFlags = pDynPtr->GetDialogMask();
        if( nDynFlags )
            nErrFlags = nDynFlags;
    }

    if(ErrHdl_Impl::CreateString(pData.pFirstHdl,pInfo,aErr,nErrFlags))
    {
        if (bJustCreateString)
        {
            rError = aErr;
            return 1;
        }
        else
        {
            if(!pData.pDsp)
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
                if(!pData.bIsWindowDsp)
                {
                    (*reinterpret_cast<BasicDisplayErrorFunc*>(pData.pDsp))(aErr,aAction);
                    return 0;
                }
                else
                {
                    if (nFlags != USHRT_MAX)
                        nErrFlags = nFlags;
                    return (*reinterpret_cast<WindowDisplayErrorFunc*>(pData.pDsp))(
                        pParent, nErrFlags, aErr, aAction);
                }
            }
        }
    }
    OSL_FAIL("Error nicht behandelt");
    // Error 1 is General Error in the Sfx
    if(pInfo->GetErrorCode()!=1)
    {
        HandleError_Impl(1, USHRT_MAX, bJustCreateString, rError);
    }
    else
    {
        OSL_FAIL("Error 1 nicht gehandeled");
    }
    delete pInfo;
    return 0;
}

// static
bool ErrorHandler::GetErrorString(sal_uIntPtr lId, OUString& rStr)
{
    return (bool)HandleError_Impl( lId, USHRT_MAX, true, rStr );
}

/** Handles an error.

    @see ErrorHandler::HandleError_Impl
*/
sal_uInt16 ErrorHandler::HandleError(sal_uIntPtr lId, sal_uInt16 nFlags)
{
    OUString aDummy;
    return HandleError_Impl( lId, nFlags, false, aDummy );
}

bool ErrHdl_Impl::CreateString( const ErrorHandler *pStart,
                                    const ErrorInfo* pInfo, OUString& pStr,
                                    sal_uInt16 &rFlags)
{
    for(const ErrorHandler *pHdl=pStart;pHdl;pHdl=pHdl->pImpl->pNext)
    {
        if(pHdl->CreateString( pInfo, pStr, rFlags))
            return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
