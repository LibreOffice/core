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

#include <memory>

#include <com/sun/star/document/ChangedByOthersRequest.hpp>
#include <com/sun/star/document/LockedDocumentRequest.hpp>
#include <com/sun/star/document/LockedOnSavingRequest.hpp>
#include <com/sun/star/document/LockFileIgnoreRequest.hpp>
#include <com/sun/star/document/LockFileCorruptRequest.hpp>
#include <com/sun/star/document/OwnLockOnDocumentRequest.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>

#include <tools/simplerm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include "strings.hrc"
#include "getcontinuations.hxx"
#include "openlocked.hxx"
#include "trylater.hxx"
#include "alreadyopen.hxx"
#include "filechanged.hxx"
#include "lockfailed.hxx"
#include "lockcorrupt.hxx"

#include "iahndl.hxx"

#define UUI_DOC_LOAD_LOCK       0
#define UUI_DOC_OWN_LOAD_LOCK   1
#define UUI_DOC_SAVE_LOCK       2
#define UUI_DOC_OWN_SAVE_LOCK   3

using namespace com::sun::star;

namespace {

void
handleLockedDocumentRequest_(
    vcl::Window * pParent,
    const OUString& aDocumentURL,
    const OUString& aInfo,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    sal_uInt16 nMode )
{
    uno::Reference< task::XInteractionApprove > xApprove;
    uno::Reference< task::XInteractionDisapprove > xDisapprove;
    uno::Reference< task::XInteractionAbort > xAbort;
    getContinuations(rContinuations, &xApprove, &xDisapprove, &xAbort);

    if ( !xApprove.is() || !xDisapprove.is() || !xAbort.is() )
        return;

    try
    {
        SolarMutexGuard aGuard;
        std::locale aResLocale = Translate::Create("uui", Application::GetSettings().GetUILanguageTag());

        OUString aMessage;
        std::vector< OUString > aArguments;
        aArguments.push_back( aDocumentURL );

        sal_Int32 nResult = RET_CANCEL;
        if ( nMode == UUI_DOC_LOAD_LOCK )
        {
            aArguments.push_back( !aInfo.isEmpty()
                                  ? aInfo
                                  : Translate::get( STR_UNKNOWNUSER, aResLocale) );
            aMessage = Translate::get(STR_OPENLOCKED_MSG, aResLocale);
            aMessage = UUIInteractionHelper::replaceMessageWithArguments(
                aMessage, aArguments );

            ScopedVclPtrInstance< OpenLockedQueryBox > xDialog(pParent, aResLocale, aMessage);
            nResult = xDialog->Execute();
        }
        else if ( nMode == UUI_DOC_SAVE_LOCK )
        {
            aArguments.push_back( !aInfo.isEmpty()
                                  ? aInfo
                                  : Translate::get( STR_UNKNOWNUSER,
                                               aResLocale ) );
            aMessage = Translate::get(STR_TRYLATER_MSG, aResLocale);
            aMessage = UUIInteractionHelper::replaceMessageWithArguments(
                aMessage, aArguments );

            ScopedVclPtrInstance< TryLaterQueryBox > xDialog(pParent, aResLocale, aMessage);
            nResult = xDialog->Execute();
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

            ScopedVclPtrInstance< AlreadyOpenQueryBox > xDialog( pParent,
                                         aResLocale,
                                         aMessage,
                                         nMode == UUI_DOC_OWN_SAVE_LOCK );
            nResult = xDialog->Execute();
        }

        if ( nResult == RET_YES )
            xApprove->select();
        else if ( nResult == RET_NO )
            xDisapprove->select();
        else
            xAbort->select();
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException("out of memory");
    }
}

void
handleChangedByOthersRequest_(
    vcl::Window * pParent,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations )
{
    uno::Reference< task::XInteractionApprove > xApprove;
    uno::Reference< task::XInteractionAbort > xAbort;
    getContinuations(rContinuations, &xApprove, &xAbort);

    if ( !xApprove.is() || !xAbort.is() )
        return;

    try
    {
        SolarMutexGuard aGuard;
        std::locale aResLocale = Translate::Create("uui", Application::GetSettings().GetUILanguageTag());
        ScopedVclPtrInstance< FileChangedQueryBox > xDialog(pParent, aResLocale);
        sal_Int32 nResult = xDialog->Execute();

        if ( nResult == RET_YES )
            xApprove->select();
        else
            xAbort->select();
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException("out of memory");
    }
}

const sal_uInt16  UUI_DOC_CreateErrDlg  = 0;
const sal_uInt16  UUI_DOC_CorruptErrDlg = 1;



void
handleLockFileProblemRequest_(
    vcl::Window * pParent,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations, sal_uInt16 nWhichDlg )
{
    uno::Reference< task::XInteractionApprove > xApprove;
    uno::Reference< task::XInteractionAbort > xAbort;
    getContinuations(rContinuations, &xApprove, &xAbort);

    if ( !xApprove.is() || !xAbort.is() )
        return;

    try
    {
        SolarMutexGuard aGuard;
        std::locale aResLocale = Translate::Create("uui", Application::GetSettings().GetUILanguageTag());

        sal_Int32 nResult;

        if (nWhichDlg == UUI_DOC_CreateErrDlg)
        {
            ScopedVclPtrInstance< LockFailedQueryBox > xDialog(pParent, aResLocale);
            nResult = xDialog->Execute();
        }
        else
        {
            ScopedVclPtrInstance< LockCorruptQueryBox > xDialog(pParent, aResLocale);
            nResult = xDialog->Execute();
        }

        if ( nResult == RET_OK )
            xApprove->select();
        else
            xAbort->select();
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException("out of memory");
    }
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
        handleLockedDocumentRequest_( getParentProperty(),
                                      aLockedDocumentRequest.DocumentURL,
                                      aLockedDocumentRequest.UserInfo,
                                      rRequest->getContinuations(),
                                      UUI_DOC_LOAD_LOCK );
        return true;
    }

    document::OwnLockOnDocumentRequest aOwnLockOnDocumentRequest;
    if (aAnyRequest >>= aOwnLockOnDocumentRequest )
    {
        handleLockedDocumentRequest_( getParentProperty(),
                                      aOwnLockOnDocumentRequest.DocumentURL,
                                      aOwnLockOnDocumentRequest.TimeInfo,
                                      rRequest->getContinuations(),
                                      aOwnLockOnDocumentRequest.IsStoring
                                          ? UUI_DOC_OWN_SAVE_LOCK
                                          : UUI_DOC_OWN_LOAD_LOCK );
        return true;
    }

    document::LockedOnSavingRequest aLockedOnSavingRequest;
    if (aAnyRequest >>= aLockedOnSavingRequest )
    {
        handleLockedDocumentRequest_( getParentProperty(),
                                      aLockedOnSavingRequest.DocumentURL,
                                      aLockedOnSavingRequest.UserInfo,
                                      rRequest->getContinuations(),
                                      UUI_DOC_SAVE_LOCK );
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
        handleChangedByOthersRequest_( getParentProperty(),
                                       rRequest->getContinuations() );
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
        handleLockFileProblemRequest_( getParentProperty(),
                                      rRequest->getContinuations(), UUI_DOC_CreateErrDlg );
        return true;
    }

    document::LockFileCorruptRequest aLockFileCorruptRequest;
    if (aAnyRequest >>= aLockFileCorruptRequest )
    {
        handleLockFileProblemRequest_( getParentProperty(),
                                      rRequest->getContinuations(), UUI_DOC_CorruptErrDlg );
        return true;
    }

    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
