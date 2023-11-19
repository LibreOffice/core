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
#include <sal/log.hxx>

#include <tools/debug.hxx>
#include <utility>
#include <vcl/errinf.hxx>

#include <algorithm>
#include <vector>

class ErrorHandler;

namespace {

    ErrorRegistry& GetErrorRegistry()
    {
        static ErrorRegistry gErrorRegistry;
        return gErrorRegistry;
    }

}

bool ErrorStringFactory::CreateString(const ErrCodeMsg& nInfo, OUString& rStr)
{
    for(const ErrorHandler *pHdlr : GetErrorRegistry().errorHandlers)
    {
        if(pHdlr->CreateString(nInfo, rStr))
            return true;
    }
    return false;
}

ErrorRegistry::ErrorRegistry()
    : pDsp(nullptr)
    , bIsWindowDsp(false)
    , m_bLock(false)
{
}

void ErrorRegistry::RegisterDisplay(BasicDisplayErrorFunc *aDsp)
{
    ErrorRegistry &rData = GetErrorRegistry();
    rData.bIsWindowDsp = false;
    rData.pDsp = reinterpret_cast< DisplayFnPtr >(aDsp);
}

void ErrorRegistry::RegisterDisplay(WindowDisplayErrorFunc *aDsp)
{
    ErrorRegistry &rData = GetErrorRegistry();
    rData.bIsWindowDsp = true;
    rData.pDsp = reinterpret_cast< DisplayFnPtr >(aDsp);
}

void ErrorRegistry::SetLock(bool bLock)
{
    ErrorRegistry& rData = GetErrorRegistry();
    rData.m_bLock = bLock;
}

bool ErrorRegistry::GetLock()
{
    ErrorRegistry& rData = GetErrorRegistry();
    return rData.m_bLock;
}

void ErrorRegistry::Reset()
{
    ErrorRegistry &rData = GetErrorRegistry();
    rData = ErrorRegistry();
}

static void aDspFunc(const OUString &rErr, const OUString &rAction)
{
    SAL_WARN("vcl", "Action: " << rAction << " Error: " << rErr);
}

ErrorHandler::ErrorHandler()
{
    ErrorRegistry &rData = GetErrorRegistry();
    rData.errorHandlers.insert(rData.errorHandlers.begin(), this);

    if(!rData.pDsp)
        ErrorRegistry::RegisterDisplay(&aDspFunc);
}

ErrorHandler::~ErrorHandler()
{
    auto &rErrorHandlers = GetErrorRegistry().errorHandlers;
    std::erase(rErrorHandlers, this);
}

bool ErrorHandler::GetErrorString(const ErrCodeMsg& nErrCode, OUString& rErrStr)
{
    OUString aErr;

    if(!nErrCode || nErrCode == ERRCODE_ABORT)
        return false;

    if (ErrorStringFactory::CreateString(nErrCode, aErr))
    {
        rErrStr = aErr;
        return true;
    }

    return false;
}

DialogMask ErrorHandler::HandleError(const ErrCodeMsg& nErr, weld::Window *pParent, DialogMask nFlags)
{
    if (nErr == ERRCODE_NONE || nErr == ERRCODE_ABORT)
        return DialogMask::NONE;

    ErrorRegistry &rData = GetErrorRegistry();
    OUString aAction;

    if (!rData.contexts.empty())
    {
        rData.contexts.front()->GetString(nErr, aAction);

        for(ErrorContext *pCtx : rData.contexts)
        {
            if(pCtx->GetParent())
            {
                pParent = pCtx->GetParent();
                break;
            }
        }
    }

    bool bWarning = nErr.IsWarning();
    DialogMask nErrFlags = DialogMask::ButtonDefaultsOk | DialogMask::ButtonsOk;
    if (bWarning)
        nErrFlags |= DialogMask::MessageWarning;
    else
        nErrFlags |= DialogMask::MessageError;

    if( nErr.GetDialogMask() != DialogMask::NONE )
        nErrFlags = nErr.GetDialogMask();

    OUString aErr;
    if (ErrorStringFactory::CreateString(nErr, aErr))
    {
        if (!rData.pDsp || rData.m_bLock)
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

    SAL_WARN( "vcl", "Error not handled " << nErr);
    // Error 1 (ERRCODE_ABORT) is classified as a General Error in sfx
    if (nErr.GetCode() != ERRCODE_ABORT)
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
    GetErrorRegistry().contexts.insert(GetErrorRegistry().contexts.begin(), this);
}

ErrorContext::~ErrorContext()
{
    auto &rContexts = GetErrorRegistry().contexts;
    std::erase(rContexts, this);
}

ErrorContext *ErrorContext::GetContext()
{
    return GetErrorRegistry().contexts.empty() ? nullptr : GetErrorRegistry().contexts.front();
}

weld::Window* ErrorContext::GetParent()
{
    return pImpl ? pImpl->pWin : nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
