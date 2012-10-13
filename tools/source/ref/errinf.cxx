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
#include <tools/shl.hxx>
#include <tools/debug.hxx>
#include <tools/errinf.hxx>
#include <tools/string.hxx>
#include <rtl/strbuf.hxx>

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
    sal_Bool                   bIsWindowDsp;

    DynamicErrorInfo            *ppDcr[ERRCODE_DYNAMIC_COUNT];
    sal_uInt16                  nNextDcr;
                                EDcrData();

static  EDcrData                *GetData();
};

class EDcr_Impl
{
    sal_uIntPtr                 lErrId;
    sal_uInt16                  nMask;

    void                        RegisterEDcr(DynamicErrorInfo *);
    void                        UnRegisterEDcr(DynamicErrorInfo *);
    static ErrorInfo           *GetDynamicErrorInfo(sal_uIntPtr lId);

friend class DynamicErrorInfo;
friend class ErrorInfo;
};

EDcrData::EDcrData()
{
    for(sal_uInt16 n=0;n<ERRCODE_DYNAMIC_COUNT;n++)
        ppDcr[n]=0;
    nNextDcr=0;
    pFirstHdl=0;
    pDsp=0;
    pFirstCtx=0;
}

EDcrData *EDcrData::GetData()
{
#ifdef BOOTSTRAP
    return 0x0;
#else
    EDcrData **ppDat=(EDcrData **)GetAppData(SHL_ERR);
    if(!*ppDat)
    {
        return (*ppDat=new EDcrData);
    }
    else
        return *ppDat;
#endif
}

void EDcr_Impl::RegisterEDcr(DynamicErrorInfo *pDcr)
{
    // Register dynamic identifier
    EDcrData* pData=EDcrData::GetData();
    lErrId= (((sal_uIntPtr)pData->nNextDcr + 1) << ERRCODE_DYNAMIC_SHIFT) +
        pDcr->GetErrorCode();
    DynamicErrorInfo **ppDcr=pData->ppDcr;
    sal_uInt16 nNext=pData->nNextDcr;

    if(ppDcr[nNext])
    {
        delete ppDcr[nNext];
    }
    ppDcr[nNext]=pDcr;
    if(++pData->nNextDcr>=ERRCODE_DYNAMIC_COUNT)
        pData->nNextDcr=0;
}

void EDcr_Impl::UnRegisterEDcr(DynamicErrorInfo *pDcr)
{

    EDcrData* pData=EDcrData::GetData();
    DynamicErrorInfo **ppDcr=pData->ppDcr;
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
    pImpl->UnRegisterEDcr(this);
    delete pImpl;
}

ErrorInfo* EDcr_Impl::GetDynamicErrorInfo(sal_uIntPtr lId)
{
    sal_uIntPtr lIdx=((lId & ERRCODE_DYNAMIC_MASK)>>ERRCODE_DYNAMIC_SHIFT)-1;
    DynamicErrorInfo* pDcr=EDcrData::GetData()->ppDcr[lIdx];
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
    static sal_Bool         CreateString(const ErrorHandler *pStart,
                                     const ErrorInfo*, OUString&, sal_uInt16&);
};

static void aDspFunc(const OUString &rErr, const OUString &rAction)
{
    rtl::OStringBuffer aErr(RTL_CONSTASCII_STRINGPARAM("Aktion: "));
    aErr.append(rtl::OUStringToOString(rAction, RTL_TEXTENCODING_ASCII_US));
    aErr.append(RTL_CONSTASCII_STRINGPARAM(" Fehler: "));
    aErr.append(rtl::OUStringToOString(rErr, RTL_TEXTENCODING_ASCII_US));
    OSL_FAIL(aErr.getStr());
}

ErrorContext::ErrorContext(Window *pWinP)
{
    EDcrData *pData=EDcrData::GetData();
    ErrorContext *&pHdl=pData->pFirstCtx;
    pWin=pWinP;
    pNext=pHdl;
    pHdl=this;
}

ErrorContext::~ErrorContext()
{
    ErrorContext **ppCtx=&(EDcrData::GetData()->pFirstCtx);
    while(*ppCtx && *ppCtx!=this)
        ppCtx=&((*ppCtx)->pNext);
    if(*ppCtx)
        *ppCtx=(*ppCtx)->pNext;
}

ErrorContext *ErrorContext::GetContext()
{
    return EDcrData::GetData()->pFirstCtx;
}

ErrorHandler::ErrorHandler()
{
    pImpl=new ErrHdl_Impl;
    EDcrData *pData=EDcrData::GetData();
    ErrorHandler *&pHdl=pData->pFirstHdl;
    pImpl->pNext=pHdl;
    pHdl=this;
    if(!pData->pDsp)
        RegisterDisplay(&aDspFunc);
}

ErrorHandler::~ErrorHandler()
{
    ErrorHandler **ppHdl=&(EDcrData::GetData()->pFirstHdl);
    while(*ppHdl && *ppHdl!=this)
        ppHdl=&((*ppHdl)->pImpl->pNext);
    if(*ppHdl)
        *ppHdl=(*ppHdl)->pImpl->pNext;
    delete pImpl;
}

void ErrorHandler::RegisterDisplay(WindowDisplayErrorFunc *aDsp)
{
    EDcrData *pData=EDcrData::GetData();
    pData->bIsWindowDsp=sal_True;
    pData->pDsp = reinterpret_cast< DisplayFnPtr >(aDsp);
}

void ErrorHandler::RegisterDisplay(BasicDisplayErrorFunc *aDsp)
{
    EDcrData *pData=EDcrData::GetData();
    pData->bIsWindowDsp=sal_False;
    pData->pDsp = reinterpret_cast< DisplayFnPtr >(aDsp);
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
    sal_uIntPtr lId, sal_uInt16 nFlags, sal_Bool bJustCreateString, OUString & rError)
{
    OUString aErr;
    OUString aAction;
    if(!lId || lId == ERRCODE_ABORT)
        return 0;
    EDcrData *pData=EDcrData::GetData();
    ErrorInfo *pInfo=ErrorInfo::GetErrorInfo(lId);
    ErrorContext *pCtx=ErrorContext::GetContext();
    if(pCtx)
        pCtx->GetString(pInfo->GetErrorCode(), aAction);
    Window *pParent=0;
    // Remove parent from context
    for(;pCtx;pCtx=pCtx->pNext)
        if(pCtx->GetParent())
        {
            pParent=pCtx->GetParent();
            break;
        }

    sal_Bool bWarning = ((lId & ERRCODE_WARNING_MASK) == ERRCODE_WARNING_MASK);
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

    if(ErrHdl_Impl::CreateString(pData->pFirstHdl,pInfo,aErr,nErrFlags))
    {
        if (bJustCreateString)
        {
            rError = aErr;
            return 1;
        }
        else
        {
            if(!pData->pDsp)
            {
                rtl::OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM("Action: "));
                aStr.append(rtl::OUStringToOString(aAction, RTL_TEXTENCODING_ASCII_US));
                aStr.append(RTL_CONSTASCII_STRINGPARAM("\nFehler: "));
                aStr.append(rtl::OUStringToOString(aErr, RTL_TEXTENCODING_ASCII_US));
                OSL_FAIL(aStr.getStr());
            }
            else
            {
                delete pInfo;
                if(!pData->bIsWindowDsp)
                {
                    (*(BasicDisplayErrorFunc*)pData->pDsp)(aErr,aAction);
                    return 0;
                }
                else
                {
                    if (nFlags != USHRT_MAX)
                        nErrFlags = nFlags;
                    return (*(WindowDisplayErrorFunc*)pData->pDsp)(
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
sal_Bool ErrorHandler::GetErrorString(sal_uIntPtr lId, OUString& rStr)
{
    return (sal_Bool)HandleError_Impl( lId, USHRT_MAX, sal_True, rStr );
}

/** Handles an error.

    @see ErrorHandler::HandleError_Impl
*/
sal_uInt16 ErrorHandler::HandleError(sal_uIntPtr lId, sal_uInt16 nFlags)
{
    OUString aDummy;
    return HandleError_Impl( lId, nFlags, sal_False, aDummy );
}

sal_Bool ErrHdl_Impl::CreateString( const ErrorHandler *pStart,
                                    const ErrorInfo* pInfo, OUString& pStr,
                                    sal_uInt16 &rFlags)
{
    for(const ErrorHandler *pHdl=pStart;pHdl;pHdl=pHdl->pImpl->pNext)
    {
        if(pHdl->CreateString( pInfo, pStr, rFlags))
            return sal_True;
    }
    return sal_False;
}

sal_Bool SimpleErrorHandler::CreateString(
    const ErrorInfo *pInfo, OUString &rStr, sal_uInt16 &) const
{
    sal_uIntPtr nId = pInfo->GetErrorCode();
    rtl::OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM("Id "));
    aStr.append(static_cast<sal_Int32>(nId));
    aStr.append(RTL_CONSTASCII_STRINGPARAM(
        " only handled by SimpleErrorHandler"));
    aStr.append(RTL_CONSTASCII_STRINGPARAM("\nErrorCode: "));
    aStr.append(static_cast<sal_Int32>(
        nId & ((1L <<  ERRCODE_CLASS_SHIFT)  - 1 )));
    aStr.append(RTL_CONSTASCII_STRINGPARAM("\nErrorClass: "));
    aStr.append(static_cast<sal_Int32>(
        (nId & ERRCODE_CLASS_MASK) >> ERRCODE_CLASS_SHIFT));
    aStr.append(RTL_CONSTASCII_STRINGPARAM("\nErrorArea: "));
    aStr.append(static_cast<sal_Int32>((nId & ERRCODE_ERROR_MASK &
            ~((1 << ERRCODE_AREA_SHIFT ) -1 ) ) >> ERRCODE_AREA_SHIFT));
    DynamicErrorInfo *pDyn=PTR_CAST(DynamicErrorInfo,pInfo);
    if(pDyn)
    {
        aStr.append(RTL_CONSTASCII_STRINGPARAM("\nDId "));
        aStr.append(static_cast<sal_Int32>(*pDyn));
    }
    rStr = rtl::OStringToOUString(aStr.makeStringAndClear(),
        RTL_TEXTENCODING_ASCII_US);
    return sal_True;
}

SimpleErrorHandler::SimpleErrorHandler()
 : ErrorHandler()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
