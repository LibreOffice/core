/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <memory>

#include "vos/mutex.hxx"
#include "vcl/svapp.hxx"
#include "vcl/msgbox.hxx"

#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/task/XInteractionDisapprove.hpp"
#include "com/sun/star/task/XInteractionRetry.hpp"

#include "tools/errinf.hxx" // ErrorHandler, ErrorContext, ...
#include "svtools/svtools.hrc" // RID_ERRHDL

#include "ids.hrc"
#include "getcontinuations.hxx"

#include "iahndl.hxx"

using namespace com::sun::star;

namespace {

sal_uInt16
executeErrorDialog(
    Window * pParent,
    task::InteractionClassification eClassification,
    rtl::OUString const & rContext,
    rtl::OUString const & rMessage,
    WinBits nButtonMask)
    SAL_THROW((uno::RuntimeException))
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    rtl::OUStringBuffer aText(rContext);
    if (rContext.getLength() != 0 && rMessage.getLength() != 0)
        aText.appendAscii(RTL_CONSTASCII_STRINGPARAM(":\n"));
            //TODO! must be internationalized
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
            if ((nButtonMask & 0x01F00000) == WB_DEF_OK)
                //TODO! missing win bit button mask define (want to ignore
                // any default button settings)...
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
        throw uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
            uno::Reference< uno::XInterface >());
    }

    sal_uInt16 aResult = xBox->Execute();
    switch( aResult )
    {
    case BUTTONID_OK:
        aResult = ERRCODE_BUTTON_OK;
        break;
    case BUTTONID_CANCEL:
        aResult = ERRCODE_BUTTON_CANCEL;
        break;
    case BUTTONID_YES:
        aResult = ERRCODE_BUTTON_YES;
        break;
    case BUTTONID_NO:
        aResult = ERRCODE_BUTTON_NO;
        break;
    case BUTTONID_RETRY:
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
    std::vector< rtl::OUString > const & rArguments,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    rtl::OUString & rErrorString)
        SAL_THROW((uno::RuntimeException))
{
    if (bObtainErrorStringOnly)
    {
        bHasErrorString = isInformationalErrorMessageRequest(rContinuations);
        if (!bHasErrorString)
            return;
    }

    rtl::OUString aMessage;
    {
        enum Source { SOURCE_DEFAULT, SOURCE_CNT, SOURCE_SVX, SOURCE_UUI };
        static char const * const aManager[4]
            = { CREATEVERSIONRESMGR_NAME(ofa),
                CREATEVERSIONRESMGR_NAME(cnt),
                CREATEVERSIONRESMGR_NAME(svx),
                CREATEVERSIONRESMGR_NAME(uui) };
        static sal_uInt16 const aId[4]
            = { RID_ERRHDL,
                RID_CHAOS_START + 12,
                // cf. chaos/source/inc/cntrids.hrc, where
                // #define RID_CHAOS_ERRHDL (RID_CHAOS_START + 12)
                RID_SVX_START + 350, // RID_SVXERRCODE
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

        vos::OGuard aGuard(Application::GetSolarMutex());
        std::auto_ptr< ResMgr > xManager;
        xManager.reset(ResMgr::CreateResMgr(aManager[eSource]));
        if (!xManager.get())
            return;
        ResId aResId(aId[eSource], *xManager.get());
        if (!ErrorResource(aResId).getString(nErrorCode, &aMessage))
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
        //
        // The mapping has five properties on which the code to select the
        // correct continuation relies:
        // 1  The OK button is mapped to Approve if that is available,
        //    otherwise to Abort if that is available, otherwise to none.
        // 2  The CANCEL button is always mapped to Abort.
        // 3  The RETRY button is always mapped to Retry.
        // 4  The NO button is always mapped to Disapprove.
        // 5  The YES button is always mapped to Approve.
        //
        // Because the WinBits button combinations are quite restricted, not
        // every request can be served here.
        //
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
        rtl::OUString aContext(getContextProperty());
        if (aContext.getLength() == 0 && nErrorCode != 0)
        {
            vos::OGuard aGuard(Application::GetSolarMutex());
            ErrorContext * pContext = ErrorContext::GetContext();
            if (pContext)
            {
                UniString aContextString;
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
