/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <memory>

#include "osl/mutex.hxx"
#include "vcl/svapp.hxx"
#include "vcl/msgbox.hxx"

#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/task/XInteractionDisapprove.hpp"
#include "com/sun/star/task/XInteractionRetry.hpp"

#include "tools/errinf.hxx"
#include "svtools/svtools.hrc"

#include "ids.hrc"
#include "getcontinuations.hxx"

#include "iahndl.hxx"

using namespace com::sun::star;

namespace {

sal_uInt16
executeErrorDialog(
    Window * pParent,
    task::InteractionClassification eClassification,
    OUString const & rContext,
    OUString const & rMessage,
    WinBits nButtonMask)
    SAL_THROW((uno::RuntimeException))
{
    SolarMutexGuard aGuard;

    OUStringBuffer aText(rContext);
    if (!rContext.isEmpty() && !rMessage.isEmpty())
        aText.appendAscii(":\n");
            
    aText.append(rMessage);

    std::auto_ptr< MessBox > xBox;
    try
    {
        switch (eClassification)
        {
        case task::InteractionClassification_ERROR:
            xBox.reset(new ErrorBox(pParent,
                                    nButtonMask,
                                    aText.makeStringAndClear()));
            break;

        case task::InteractionClassification_WARNING:
            xBox.reset(new WarningBox(pParent,
                                      nButtonMask,
                                      aText.makeStringAndClear()));
            break;

        case task::InteractionClassification_INFO:
#           define WB_DEF_BUTTONS (WB_DEF_OK | WB_DEF_CANCEL | WB_DEF_RETRY)
            
            if ((nButtonMask & WB_DEF_BUTTONS) == WB_DEF_OK)
                xBox.reset(new InfoBox(pParent,
                                       aText.makeStringAndClear()));
            else
                xBox.reset(new ErrorBox(pParent,
                                        nButtonMask,
                                        aText.makeStringAndClear()));
            break;

        case task::InteractionClassification_QUERY:
            xBox.reset(new QueryBox(pParent,
                                    nButtonMask,
                                    aText.makeStringAndClear()));
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException("out of memory",
            uno::Reference< uno::XInterface >());
    }

    sal_uInt16 aResult = xBox->Execute();
    switch( aResult )
    {
    case RET_OK:
        aResult = ERRCODE_BUTTON_OK;
        break;
    case RET_CANCEL:
        aResult = ERRCODE_BUTTON_CANCEL;
        break;
    case RET_YES:
        aResult = ERRCODE_BUTTON_YES;
        break;
    case RET_NO:
        aResult = ERRCODE_BUTTON_NO;
        break;
    case RET_RETRY:
        aResult = ERRCODE_BUTTON_RETRY;
        break;
    }

    return aResult;
}

}

void
UUIInteractionHelper::handleErrorHandlerRequest(
    task::InteractionClassification eClassification,
    ErrCode nErrorCode,
    std::vector< OUString > const & rArguments,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    OUString & rErrorString)
        SAL_THROW((uno::RuntimeException))
{
    if (bObtainErrorStringOnly)
    {
        bHasErrorString = isInformationalErrorMessageRequest(rContinuations);
        if (!bHasErrorString)
            return;
    }

    OUString aMessage;
    {
        enum Source { SOURCE_DEFAULT, SOURCE_CNT, SOURCE_SVX, SOURCE_UUI };
        static char const * const aManager[4] = { "ofa", "cnt", "svx", "uui" };
        static sal_uInt16 const aId[4]
            = { RID_ERRHDL,
                RID_CHAOS_START + 12,
                
                
                RID_SVX_START + 350, 
                RID_UUI_ERRHDL };
        ErrCode nErrorId = nErrorCode & ~ERRCODE_WARNING_MASK;
        Source eSource = nErrorId < ERRCODE_AREA_LIB1 ?
            SOURCE_DEFAULT :
            nErrorId >= ERRCODE_AREA_CHAOS
            && nErrorId < ERRCODE_AREA_CHAOS_END ?
            SOURCE_CNT :
            nErrorId >= ERRCODE_AREA_SVX
            && nErrorId <= ERRCODE_AREA_SVX_END ?
            SOURCE_SVX :
            SOURCE_UUI;

        SolarMutexGuard aGuard;
        std::auto_ptr< ResMgr > xManager;
        xManager.reset(ResMgr::CreateResMgr(aManager[eSource]));
        if (!xManager.get())
            return;
        ResId aResId(aId[eSource], *xManager.get());
        if (!ErrorResource(aResId).getString(nErrorCode, aMessage))
            return;
    }

    aMessage = replaceMessageWithArguments( aMessage, rArguments );

    if (bObtainErrorStringOnly)
    {
        rErrorString = aMessage;
        return;
    }
    else
    {
        
        
        
        
        
        

        uno::Reference< task::XInteractionApprove > xApprove;
        uno::Reference< task::XInteractionDisapprove > xDisapprove;
        uno::Reference< task::XInteractionRetry > xRetry;
        uno::Reference< task::XInteractionAbort > xAbort;
        getContinuations(
            rContinuations, &xApprove, &xDisapprove, &xRetry, &xAbort);

        
        
        
        
        
        //
        
        
        
        
        
        
        
        
        //
        
        
        //
        
        
        
        WinBits const aButtonMask[16]
            = { 0,
                WB_OK /*| WB_DEF_OK*/, 
                0,
                WB_RETRY_CANCEL /*| WB_DEF_CANCEL*/, 
                0,
                0,
                0,
                0,
                WB_OK /*| WB_DEF_OK*/, 
                WB_OK_CANCEL /*| WB_DEF_CANCEL*/, 
                0,
                0,
                WB_YES_NO /*| WB_DEF_NO*/, 
                WB_YES_NO_CANCEL /*| WB_DEF_CANCEL*/,
                
                0,
                0 };

        WinBits nButtonMask = aButtonMask[(xApprove.is() ? 8 : 0)
                                          | (xDisapprove.is() ? 4 : 0)
                                          | (xRetry.is() ? 2 : 0)
                                          | (xAbort.is() ? 1 : 0)];
        if (nButtonMask == 0)
            return;

        
        OUString aContext(getContextProperty());
        if (aContext.isEmpty() && nErrorCode != 0)
        {
            SolarMutexGuard aGuard;
            ErrorContext * pContext = ErrorContext::GetContext();
            if (pContext)
            {
                OUString aContextString;
                if (pContext->GetString(nErrorCode, aContextString))
                    aContext = aContextString;
            }
        }

        sal_uInt16 nResult = executeErrorDialog(
            getParentProperty(), eClassification, aContext, aMessage, nButtonMask );

        switch (nResult)
        {
        case ERRCODE_BUTTON_OK:
            OSL_ENSURE(xApprove.is() || xAbort.is(), "unexpected situation");
            if (xApprove.is())
                xApprove->select();
            else if (xAbort.is())
                xAbort->select();
            break;

        case ERRCODE_BUTTON_CANCEL:
            OSL_ENSURE(xAbort.is(), "unexpected situation");
            if (xAbort.is())
                xAbort->select();
            break;

        case ERRCODE_BUTTON_RETRY:
            OSL_ENSURE(xRetry.is(), "unexpected situation");
            if (xRetry.is())
                xRetry->select();
            break;

        case ERRCODE_BUTTON_NO:
            OSL_ENSURE(xDisapprove.is(), "unexpected situation");
            if (xDisapprove.is())
                xDisapprove->select();
            break;

        case ERRCODE_BUTTON_YES:
            OSL_ENSURE(xApprove.is(), "unexpected situation");
            if (xApprove.is())
                xApprove->select();
            break;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
