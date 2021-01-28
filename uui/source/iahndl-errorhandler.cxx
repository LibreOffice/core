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

#include <vcl/errinf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/weld.hxx>

#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>

#include <svx/svxerr.hxx>
#include <unotools/resmgr.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

#include <ids.hrc>
#include "getcontinuations.hxx"

#include "iahndl.hxx"
#include <memory>

using namespace com::sun::star;

namespace {

enum class MessageBoxStyle {
    NONE              = 0x0000,
    Ok                = 0x0001,
    OkCancel          = 0x0002,
    YesNo             = 0x0004,
    YesNoCancel       = 0x0008,
    RetryCancel       = 0x0010
};

DialogMask
executeErrorDialog(
    weld::Window* pParent,
    task::InteractionClassification eClassification,
    OUString const & rContext,
    std::u16string_view rMessage,
    MessageBoxStyle nButtonMask)
{
    SolarMutexGuard aGuard;

    OUStringBuffer aText(rContext);
    if (!rContext.isEmpty() && !rMessage.empty())
        aText.append(":\n");
            //TODO! must be internationalized
    aText.append(rMessage);

    std::unique_ptr<weld::MessageDialog> xBox;

    switch (eClassification)
    {
        case task::InteractionClassification_ERROR:
            xBox.reset(Application::CreateMessageDialog(pParent,
                        VclMessageType::Error, VclButtonsType::NONE, aText.makeStringAndClear()));
            break;
        case task::InteractionClassification_WARNING:
            xBox.reset(Application::CreateMessageDialog(pParent,
                        VclMessageType::Warning, VclButtonsType::NONE, aText.makeStringAndClear()));
            break;
        case task::InteractionClassification_INFO:
            xBox.reset(Application::CreateMessageDialog(pParent,
                        VclMessageType::Info, VclButtonsType::NONE, aText.makeStringAndClear()));
            break;
        case task::InteractionClassification_QUERY:
            xBox.reset(Application::CreateMessageDialog(pParent,
                        VclMessageType::Question, VclButtonsType::NONE, aText.makeStringAndClear()));
            break;
        default:
            assert(false);
            break;
    }


    switch (nButtonMask)
    {
        case MessageBoxStyle::NONE:
            break;
        case MessageBoxStyle::Ok:
            xBox->add_button(GetStandardText(StandardButtonType::OK), static_cast<int>(DialogMask::ButtonsOk));
            break;
        case MessageBoxStyle::OkCancel:
            xBox->add_button(GetStandardText(StandardButtonType::OK), static_cast<int>(DialogMask::ButtonsOk));
            xBox->add_button(GetStandardText(StandardButtonType::Cancel), static_cast<int>(DialogMask::ButtonsCancel));
            break;
        case MessageBoxStyle::YesNo:
            xBox->add_button(GetStandardText(StandardButtonType::Yes), static_cast<int>(DialogMask::ButtonsYes));
            xBox->add_button(GetStandardText(StandardButtonType::No), static_cast<int>(DialogMask::ButtonsNo));
            break;
        case MessageBoxStyle::YesNoCancel:
            xBox->add_button(GetStandardText(StandardButtonType::Yes), static_cast<int>(DialogMask::ButtonsYes));
            xBox->add_button(GetStandardText(StandardButtonType::No), static_cast<int>(DialogMask::ButtonsNo));
            xBox->add_button(GetStandardText(StandardButtonType::Cancel), static_cast<int>(DialogMask::ButtonsCancel));
            break;
        case MessageBoxStyle::RetryCancel:
            xBox->add_button(GetStandardText(StandardButtonType::Retry), static_cast<int>(DialogMask::ButtonsRetry));
            xBox->add_button(GetStandardText(StandardButtonType::Cancel), static_cast<int>(DialogMask::ButtonsCancel));
            break;
    }

    return static_cast<DialogMask>(xBox->run());
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
        enum Source { SOURCE_DEFAULT, SOURCE_SVX, SOURCE_UUI };
        static char const * const aManager[3] = { "svt", "svx", "uui" };
        static const ErrMsgCode* const aId[3]
            = { RID_ERRHDL,
                RID_SVXERRCODE,
                RID_UUI_ERRHDL };
        ErrCodeArea nErrorArea = nErrorCode.GetArea();
        Source eSource =
            nErrorArea < ErrCodeArea::Svx ? SOURCE_DEFAULT
                : nErrorArea == ErrCodeArea::Svx ? SOURCE_SVX : SOURCE_UUI;

        std::locale aResLocale = Translate::Create(aManager[eSource]);
        ErrorResource aErrorResource(aId[eSource], aResLocale);
        if (!aErrorResource.getString(nErrorCode, aMessage))
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
        MessageBoxStyle const aButtonMask[16]
            = { MessageBoxStyle::NONE,
                MessageBoxStyle::Ok /*| MessBoxStyle::DefaultOk*/, // Abort
                MessageBoxStyle::NONE,
                MessageBoxStyle::RetryCancel /*| MessBoxStyle::DefaultCancel*/, // Retry, Abort
                MessageBoxStyle::NONE,
                MessageBoxStyle::NONE,
                MessageBoxStyle::NONE,
                MessageBoxStyle::NONE,
                MessageBoxStyle::Ok /*| MessBoxStyle::DefaultOk*/, // Approve
                MessageBoxStyle::OkCancel /*| MessBoxStyle::DefaultCancel*/, // Approve, Abort
                MessageBoxStyle::NONE,
                MessageBoxStyle::NONE,
                MessageBoxStyle::YesNo /*| MessBoxStyle::DefaultNo*/, // Approve, Disapprove
                MessageBoxStyle::YesNoCancel /*| MessBoxStyle::DefaultCancel*/,
                // Approve, Disapprove, Abort
                MessageBoxStyle::NONE,
                MessageBoxStyle::NONE };

        MessageBoxStyle nButtonMask = aButtonMask[(xApprove.is() ? 8 : 0)
                                          | (xDisapprove.is() ? 4 : 0)
                                          | (xRetry.is() ? 2 : 0)
                                          | (xAbort.is() ? 1 : 0)];
        if (nButtonMask == MessageBoxStyle::NONE)
            return;

        //TODO! remove this backwards compatibility?
        OUString aContext(m_aContextParam);
        if (aContext.isEmpty() && nErrorCode != ERRCODE_NONE)
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

        uno::Reference<awt::XWindow> xParent = getParentXWindow();
        DialogMask nResult = executeErrorDialog(Application::GetFrameWeld(xParent),
                eClassification, aContext, aMessage, nButtonMask );

        switch (nResult)
        {
        case DialogMask::ButtonsOk:
            OSL_ENSURE(xApprove.is() || xAbort.is(), "unexpected situation");
            if (xApprove.is())
                xApprove->select();
            else if (xAbort.is())
                xAbort->select();
            break;

        case DialogMask::ButtonsCancel:
            OSL_ENSURE(xAbort.is(), "unexpected situation");
            if (xAbort.is())
                xAbort->select();
            break;

        case DialogMask::ButtonsRetry:
            OSL_ENSURE(xRetry.is(), "unexpected situation");
            if (xRetry.is())
                xRetry->select();
            break;

        case DialogMask::ButtonsNo:
            OSL_ENSURE(xDisapprove.is(), "unexpected situation");
            if (xDisapprove.is())
                xDisapprove->select();
            break;

        case DialogMask::ButtonsYes:
            OSL_ENSURE(xApprove.is(), "unexpected situation");
            if (xApprove.is())
                xApprove->select();
            break;

        default: break;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
