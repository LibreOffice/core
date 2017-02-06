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

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>

#include <tools/errinf.hxx>
#include <svtools/svtools.hrc>
#include <svx/dialogs.hrc>

#include "ids.hrc"
#include "getcontinuations.hxx"

#include "iahndl.hxx"
#include <memory>

using namespace com::sun::star;

namespace {

ErrorHandlerFlags
executeErrorDialog(
    vcl::Window * pParent,
    task::InteractionClassification eClassification,
    OUString const & rContext,
    OUString const & rMessage,
    WinBits nButtonMask)
{
    SolarMutexGuard aGuard;

    OUStringBuffer aText(rContext);
    if (!rContext.isEmpty() && !rMessage.isEmpty())
        aText.append(":\n");
            //TODO! must be internationalized
    aText.append(rMessage);

    VclPtr< MessBox > xBox;
    try
    {
        switch (eClassification)
        {
        case task::InteractionClassification_ERROR:
            xBox.reset(VclPtr<ErrorBox>::Create(pParent,
                                    nButtonMask,
                                    aText.makeStringAndClear()));
            break;

        case task::InteractionClassification_WARNING:
            xBox.reset(VclPtr<WarningBox>::Create(pParent,
                                      nButtonMask,
                                      aText.makeStringAndClear()));
            break;

        case task::InteractionClassification_INFO:
#           define WB_DEF_BUTTONS (WB_DEF_OK | WB_DEF_CANCEL | WB_DEF_RETRY)
            //(want to ignore any default button settings)...
            if ((nButtonMask & WB_DEF_BUTTONS) == WB_DEF_OK)
                xBox.reset(VclPtr<InfoBox>::Create(pParent,
                                       aText.makeStringAndClear()));
            else
                xBox.reset(VclPtr<ErrorBox>::Create(pParent,
                                        nButtonMask,
                                        aText.makeStringAndClear()));
            break;

        case task::InteractionClassification_QUERY:
            xBox.reset(VclPtr<QueryBox>::Create(pParent,
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
        throw uno::RuntimeException("out of memory");
    }

    sal_uInt16 aMessResult = xBox->Execute();

    xBox.disposeAndClear();

    ErrorHandlerFlags aResult = ErrorHandlerFlags::NONE;
    switch( aMessResult )
    {
    case RET_OK:
        aResult = ErrorHandlerFlags::ButtonsOk;
        break;
    case RET_CANCEL:
        aResult = ErrorHandlerFlags::ButtonsCancel;
        break;
    case RET_YES:
        aResult = ErrorHandlerFlags::ButtonsYes;
        break;
    case RET_NO:
        aResult = ErrorHandlerFlags::ButtonsNo;
        break;
    case RET_RETRY:
        aResult = ErrorHandlerFlags::ButtonsRetry;
        break;
    default: assert(false);
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
                // cf. chaos/source/inc/cntrids.hrc, where
                // #define RID_CHAOS_ERRHDL (RID_CHAOS_START + 12)
                RID_SVXERRCODE,
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
        std::unique_ptr< ResMgr > xManager;
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
        //TODO! It can happen that the buttons calculated below do not match
        // the error text from the resource (e.g., some text that is not a
        // question, but YES and NO buttons).  Some error texts have
        // ExtraData that specifies a set of buttons, but that data is not
        // really useful, because a single error text may well make sense
        // both with only an OK button and with RETRY and CANCEL buttons.

        uno::Reference< task::XInteractionApprove > xApprove;
        uno::Reference< task::XInteractionDisapprove > xDisapprove;
        uno::Reference< task::XInteractionRetry > xRetry;
        uno::Reference< task::XInteractionAbort > xAbort;
        getContinuations(
            rContinuations, &xApprove, &xDisapprove, &xRetry, &xAbort);

        // The following mapping uses the bit mask
        //     Approve = 8,
        //     Disapprove = 4,
        //     Retry = 2,
        //     Abort = 1

        // The mapping has five properties on which the code to select the
        // correct continuation relies:
        // 1  The OK button is mapped to Approve if that is available,
        //    otherwise to Abort if that is available, otherwise to none.
        // 2  The CANCEL button is always mapped to Abort.
        // 3  The RETRY button is always mapped to Retry.
        // 4  The NO button is always mapped to Disapprove.
        // 5  The YES button is always mapped to Approve.

        // Because the WinBits button combinations are quite restricted, not
        // every request can be served here.

        // Finally, it seems to be better to leave default button
        // determination to VCL (the favouring of CANCEL as default button
        // seems to not always be what the user wants)...
        WinBits const aButtonMask[16]
            = { 0,
                WB_OK /*| WB_DEF_OK*/, // Abort
                0,
                WB_RETRY_CANCEL /*| WB_DEF_CANCEL*/, // Retry, Abort
                0,
                0,
                0,
                0,
                WB_OK /*| WB_DEF_OK*/, // Approve
                WB_OK_CANCEL /*| WB_DEF_CANCEL*/, // Approve, Abort
                0,
                0,
                WB_YES_NO /*| WB_DEF_NO*/, // Approve, Disapprove
                WB_YES_NO_CANCEL /*| WB_DEF_CANCEL*/,
                // Approve, Disapprove, Abort
                0,
                0 };

        WinBits nButtonMask = aButtonMask[(xApprove.is() ? 8 : 0)
                                          | (xDisapprove.is() ? 4 : 0)
                                          | (xRetry.is() ? 2 : 0)
                                          | (xAbort.is() ? 1 : 0)];
        if (nButtonMask == 0)
            return;

        //TODO! remove this backwards compatibility?
        OUString aContext(m_aContextParam);
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

        ErrorHandlerFlags nResult = executeErrorDialog(
            getParentProperty(), eClassification, aContext, aMessage, nButtonMask );

        switch (nResult)
        {
        case ErrorHandlerFlags::ButtonsOk:
            OSL_ENSURE(xApprove.is() || xAbort.is(), "unexpected situation");
            if (xApprove.is())
                xApprove->select();
            else if (xAbort.is())
                xAbort->select();
            break;

        case ErrorHandlerFlags::ButtonsCancel:
            OSL_ENSURE(xAbort.is(), "unexpected situation");
            if (xAbort.is())
                xAbort->select();
            break;

        case ErrorHandlerFlags::ButtonsRetry:
            OSL_ENSURE(xRetry.is(), "unexpected situation");
            if (xRetry.is())
                xRetry->select();
            break;

        case ErrorHandlerFlags::ButtonsNo:
            OSL_ENSURE(xDisapprove.is(), "unexpected situation");
            if (xDisapprove.is())
                xDisapprove->select();
            break;

        case ErrorHandlerFlags::ButtonsYes:
            OSL_ENSURE(xApprove.is(), "unexpected situation");
            if (xApprove.is())
                xApprove->select();
            break;

        default: break;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
