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

#include <com/sun/star/document/ChangedByOthersRequest.hpp>
#include <com/sun/star/document/LockedDocumentRequest.hpp>
#include <com/sun/star/document/LockedOnSavingRequest.hpp>
#include <com/sun/star/document/LockFileIgnoreRequest.hpp>
#include <com/sun/star/document/LockFileCorruptRequest.hpp>
#include <com/sun/star/document/OwnLockOnDocumentRequest.hpp>
#include <com/sun/star/document/ReloadEditableRequest.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>

#include <unotools/resmgr.hxx>
#include <vcl/svapp.hxx>
#include <officecfg/Office/Common.hxx>

#include <strings.hrc>
#include "getcontinuations.hxx"
#include "openlocked.hxx"
#include "trylater.hxx"
#include "alreadyopen.hxx"
#include "filechanged.hxx"
#include "lockfailed.hxx"
#include "lockcorrupt.hxx"
#include "reloadeditable.hxx"

#include "iahndl.hxx"

#define UUI_DOC_LOAD_LOCK       0
#define UUI_DOC_OWN_LOAD_LOCK   1
#define UUI_DOC_SAVE_LOCK       2
#define UUI_DOC_OWN_SAVE_LOCK   3

using namespace com::sun::star;

namespace {

void handleReloadEditableRequest_(
    weld::Window* pParent,
    const OUString& aDocumentURL,
    uno::Sequence<uno::Reference<task::XInteractionContinuation>> const&
        rContinuations)
{
    uno::Reference<task::XInteractionApprove> xApprove;
    uno::Reference<task::XInteractionAbort> xAbort;
    getContinuations(rContinuations, &xApprove, &xAbort);

    if (!xApprove.is() || !xAbort.is())
        return;

    SolarMutexGuard aGuard;
    std::locale aResLocale = Translate::Create("uui");

    OUString aMessage;
    std::vector<OUString> aArguments;
    aArguments.push_back(aDocumentURL);

    aMessage = Translate::get(STR_RELOADEDITABLE_MSG, aResLocale);
    aMessage = UUIInteractionHelper::replaceMessageWithArguments(aMessage, aArguments);

    ReloadEditableQueryBox aDialog(pParent, aResLocale, aMessage);
    int nResult = aDialog.run();

    if (nResult == RET_YES)
        xApprove->select();
    else
        xAbort->select();
}

void
handleLockedDocumentRequest_(
    weld::Window * pParent,
    const OUString& aDocumentURL,
    const OUString& aInfo,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    sal_uInt16 nMode )
{
    uno::Reference< task::XInteractionApprove > xApprove;
    uno::Reference< task::XInteractionDisapprove > xDisapprove;
    uno::Reference< task::XInteractionAbort > xAbort;
    // In case an option to ignore lock and open the file is available
    uno::Reference< task::XInteractionRetry > xRetry;
    getContinuations(rContinuations, &xApprove, &xDisapprove, &xAbort, &xRetry);

    if ( !xApprove.is() || !xDisapprove.is() || !xAbort.is() )
        return;

    SolarMutexGuard aGuard;
    std::locale aResLocale = Translate::Create("uui");

    OUString aMessage;
    std::vector< OUString > aArguments;
    aArguments.push_back( aDocumentURL );

    bool bAllowOverride = xRetry.is() && officecfg::Office::Common::Misc::AllowOverrideLocking::get();

    sal_Int32 nResult = RET_CANCEL;
    if ( nMode == UUI_DOC_LOAD_LOCK )
    {
        aArguments.push_back( !aInfo.isEmpty()
                              ? aInfo
                              : Translate::get( STR_UNKNOWNUSER, aResLocale) );
        aArguments.push_back( bAllowOverride
                              ? Translate::get( STR_OPENLOCKED_ALLOWIGNORE_MSG, aResLocale )
                              : "" );
        aMessage = Translate::get(STR_OPENLOCKED_MSG, aResLocale);
        aMessage = UUIInteractionHelper::replaceMessageWithArguments(
            aMessage, aArguments );

        OpenLockedQueryBox aDialog(pParent, aResLocale, aMessage, bAllowOverride);
        nResult = aDialog.run();
    }
    else if ( nMode == UUI_DOC_SAVE_LOCK )
    {
        aArguments.push_back( !aInfo.isEmpty()
                              ? aInfo
                              : Translate::get( STR_UNKNOWNUSER,
                                           aResLocale ) );
        aMessage = Translate::get(bAllowOverride ? STR_OVERWRITE_IGNORELOCK_MSG : STR_TRYLATER_MSG,
            aResLocale);
        aMessage = UUIInteractionHelper::replaceMessageWithArguments(
            aMessage, aArguments );

        TryLaterQueryBox aDialog(pParent, aResLocale, aMessage, bAllowOverride);
        nResult = aDialog.run();
    }
    else if ( nMode == UUI_DOC_OWN_LOAD_LOCK ||
              nMode == UUI_DOC_OWN_SAVE_LOCK )
    {
        aArguments.push_back( aInfo );
        aMessage = Translate::get(nMode == UUI_DOC_OWN_SAVE_LOCK
                                      ? STR_ALREADYOPEN_SAVE_MSG
                                      : STR_ALREADYOPEN_MSG,
                                  aResLocale );
        aMessage = UUIInteractionHelper::replaceMessageWithArguments(
            aMessage, aArguments );

        AlreadyOpenQueryBox aDialog(pParent, aResLocale, aMessage, nMode == UUI_DOC_OWN_SAVE_LOCK);
        nResult = aDialog.run();
    }

    if ( nResult == RET_YES )
        xApprove->select();
    else if ( nResult == RET_NO )
        xDisapprove->select();
    else if ( nResult == RET_IGNORE && xRetry.is() )
        xRetry->select();
    else
        xAbort->select();
}

void
handleChangedByOthersRequest_(
    weld::Window * pParent,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations )
{
    uno::Reference< task::XInteractionApprove > xApprove;
    uno::Reference< task::XInteractionAbort > xAbort;
    getContinuations(rContinuations, &xApprove, &xAbort);

    if ( !xApprove.is() || !xAbort.is() )
        return;

    SolarMutexGuard aGuard;
    std::locale aResLocale = Translate::Create("uui");
    FileChangedQueryBox aDialog(pParent, aResLocale);
    sal_Int32 nResult = aDialog.run();

    if ( nResult == RET_YES )
        xApprove->select();
    else
        xAbort->select();
}

const sal_uInt16  UUI_DOC_CreateErrDlg  = 0;
const sal_uInt16  UUI_DOC_CorruptErrDlg = 1;



void
handleLockFileProblemRequest_(
    weld::Window * pParent,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations, sal_uInt16 nWhichDlg )
{
    uno::Reference< task::XInteractionApprove > xApprove;
    uno::Reference< task::XInteractionAbort > xAbort;
    getContinuations(rContinuations, &xApprove, &xAbort);

    if ( !xApprove.is() || !xAbort.is() )
        return;

    SolarMutexGuard aGuard;
    std::locale aResLocale = Translate::Create("uui");

    sal_Int32 nResult;

    if (nWhichDlg == UUI_DOC_CreateErrDlg)
    {
        LockFailedQueryBox aDialog(pParent, aResLocale);
        nResult = aDialog.run();
    }
    else
    {
        LockCorruptQueryBox aDialog(pParent, aResLocale);
        nResult = aDialog.run();
    }

    if ( nResult == RET_OK )
        xApprove->select();
    else
        xAbort->select();
}

} // namespace

bool
UUIInteractionHelper::handleLockedDocumentRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    uno::Any aAnyRequest(rRequest->getRequest());

    document::LockedDocumentRequest aLockedDocumentRequest;
    if (aAnyRequest >>= aLockedDocumentRequest )
    {
        uno::Reference<awt::XWindow> xParent = getParentXWindow();
        handleLockedDocumentRequest_(Application::GetFrameWeld(xParent),
                                     aLockedDocumentRequest.DocumentURL,
                                     aLockedDocumentRequest.UserInfo,
                                     rRequest->getContinuations(),
                                     UUI_DOC_LOAD_LOCK);
        return true;
    }

    document::OwnLockOnDocumentRequest aOwnLockOnDocumentRequest;
    if (aAnyRequest >>= aOwnLockOnDocumentRequest )
    {
        uno::Reference<awt::XWindow> xParent = getParentXWindow();
        handleLockedDocumentRequest_(Application::GetFrameWeld(xParent),
                                     aOwnLockOnDocumentRequest.DocumentURL,
                                     aOwnLockOnDocumentRequest.TimeInfo,
                                     rRequest->getContinuations(),
                                     aOwnLockOnDocumentRequest.IsStoring
                                         ? UUI_DOC_OWN_SAVE_LOCK
                                         : UUI_DOC_OWN_LOAD_LOCK);
        return true;
    }

    document::LockedOnSavingRequest aLockedOnSavingRequest;
    if (aAnyRequest >>= aLockedOnSavingRequest )
    {
        uno::Reference<awt::XWindow> xParent = getParentXWindow();
        handleLockedDocumentRequest_(Application::GetFrameWeld(xParent),
                                     aLockedOnSavingRequest.DocumentURL,
                                     aLockedOnSavingRequest.UserInfo,
                                     rRequest->getContinuations(),
                                     UUI_DOC_SAVE_LOCK);
        return true;
    }
    return false;
}

bool
UUIInteractionHelper::handleChangedByOthersRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    uno::Any aAnyRequest(rRequest->getRequest());

    document::ChangedByOthersRequest aChangedByOthersRequest;
    if (aAnyRequest >>= aChangedByOthersRequest )
    {
        uno::Reference<awt::XWindow> xParent = getParentXWindow();
        handleChangedByOthersRequest_(Application::GetFrameWeld(xParent),
                                      rRequest->getContinuations());
        return true;
    }
    return false;
}


bool
UUIInteractionHelper::handleLockFileProblemRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    uno::Any aAnyRequest(rRequest->getRequest());

    document::LockFileIgnoreRequest aLockFileIgnoreRequest;
    if (aAnyRequest >>= aLockFileIgnoreRequest )
    {
        uno::Reference<awt::XWindow> xParent = getParentXWindow();
        handleLockFileProblemRequest_(Application::GetFrameWeld(xParent),
                                      rRequest->getContinuations(), UUI_DOC_CreateErrDlg);
        return true;
    }

    document::LockFileCorruptRequest aLockFileCorruptRequest;
    if (aAnyRequest >>= aLockFileCorruptRequest )
    {
        uno::Reference<awt::XWindow> xParent = getParentXWindow();
        handleLockFileProblemRequest_(Application::GetFrameWeld(xParent),
                                      rRequest->getContinuations(), UUI_DOC_CorruptErrDlg);
        return true;
    }

    return false;
}

bool UUIInteractionHelper::handleReloadEditableRequest(
    uno::Reference<task::XInteractionRequest> const& rRequest)
{
    uno::Any aAnyRequest(rRequest->getRequest());

    document::ReloadEditableRequest aReloadEditableRequest;
    if (aAnyRequest >>= aReloadEditableRequest)
    {
        uno::Reference<awt::XWindow> xParent = getParentXWindow();
        handleReloadEditableRequest_(
            Application::GetFrameWeld(xParent), aReloadEditableRequest.DocumentURL,
            rRequest->getContinuations());
        return true;
    }

    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
