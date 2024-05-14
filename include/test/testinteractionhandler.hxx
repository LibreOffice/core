/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_TESTINTERACTIONHANDLER_HXX
#define INCLUDED_TEST_TESTINTERACTIONHANDLER_HXX

#include <sal/config.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionPassword2.hpp>
#include <com/sun/star/task/DocumentPasswordRequest2.hpp>
#include <com/sun/star/task/DocumentMSPasswordRequest2.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

class TestInteractionHandler : public cppu::WeakImplHelper<css::lang::XServiceInfo,
                                                           css::lang::XInitialization,
                                                           css::task::XInteractionHandler2>
{
    OUString msPassword;
    bool mbPasswordRequested;

    TestInteractionHandler(const TestInteractionHandler&) = delete;
    TestInteractionHandler& operator=(const TestInteractionHandler&) = delete;

public:
    TestInteractionHandler(const OUString& sPassword)
        : msPassword(sPassword)
        , mbPasswordRequested(false)
    {}

    bool wasPasswordRequested()
    {
        return mbPasswordRequested;
    }

    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.uui.TestInteractionHandler"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & rServiceName) override
    {
        return cppu::supportsService(this, rServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { u"com.sun.star.task.InteractionHandler"_ustr,
        // added to indicate support for configuration.backend.MergeRecoveryRequest
                 u"com.sun.star.configuration.backend.InteractionHandler"_ustr,
        // for backwards compatibility
                 u"com.sun.star.uui.InteractionHandler"_ustr };
    }

    virtual void SAL_CALL initialize(css::uno::Sequence<css::uno::Any> const & /*rArguments*/) override
    {}

    virtual void SAL_CALL handle(css::uno::Reference<css::task::XInteractionRequest> const & rRequest) override
    {
        handleInteractionRequest(rRequest);
    }

    virtual sal_Bool SAL_CALL handleInteractionRequest(const css::uno::Reference<css::task::XInteractionRequest>& rRequest) override
    {
        mbPasswordRequested = false;

        css::uno::Sequence<css::uno::Reference<css::task::XInteractionContinuation>> const &rContinuations = rRequest->getContinuations();
        css::uno::Any const aRequest(rRequest->getRequest());

        if (handlePasswordRequest(rContinuations, aRequest))
            return true;

        for (auto const & cont : rContinuations)
        {
            css::uno::Reference<css::task::XInteractionApprove> xApprove(cont, css::uno::UNO_QUERY);
            if (xApprove.is())
                xApprove->select();
        }

        return true;
    }

    bool handlePasswordRequest(const css::uno::Sequence<css::uno::Reference<css::task::XInteractionContinuation>> &rContinuations,
                               const css::uno::Any& rRequest)
    {
        bool bPasswordRequestFound = false;
        bool bIsRequestPasswordToModify = false;
        css::task::PasswordRequestMode mode{};

        css::task::DocumentPasswordRequest2 passwordRequest2;
        if (rRequest >>= passwordRequest2)
        {
            bIsRequestPasswordToModify = passwordRequest2.IsRequestPasswordToModify;
            mode = passwordRequest2.Mode;
            bPasswordRequestFound = true;
        }
        css::task::DocumentMSPasswordRequest2 passwordMSRequest2;
        if (rRequest >>= passwordMSRequest2)
        {
            bIsRequestPasswordToModify = passwordMSRequest2.IsRequestPasswordToModify;
            mode = passwordMSRequest2.Mode;
            bPasswordRequestFound = true;
        }

        if (!bPasswordRequestFound)
        {
            mbPasswordRequested = false;
            return false;
        }
        mbPasswordRequested = true;

        for (auto const & cont : rContinuations)
        {
            if (mode == css::task::PasswordRequestMode_PASSWORD_REENTER)
            {   // cancel re-enter of wrong password, to avoid infinite loop
                css::uno::Reference<css::task::XInteractionAbort> const xAbort(cont, css::uno::UNO_QUERY);
                if (xAbort.is())
                    xAbort->select();
            }
            else if (bIsRequestPasswordToModify)
            {
                css::uno::Reference<css::task::XInteractionPassword2> const xIPW2(cont, css::uno::UNO_QUERY);
                xIPW2->setPasswordToModify(msPassword);
                xIPW2->select();
            }
            else
            {
                css::uno::Reference<css::task::XInteractionPassword> const xIPW(cont, css::uno::UNO_QUERY);
                if (xIPW.is())
                {
                    xIPW->setPassword(msPassword);
                    xIPW->select();
                }
            }
        }
        return true;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
