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
#include <com/sun/star/document/OwnLockOnDocumentRequest.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include "ids.hrc"
#include "getcontinuations.hxx"
#include "openlocked.hxx"
#include "trylater.hxx"
#include "alreadyopen.hxx"
#include "filechanged.hxx"
#include "lockfailed.hxx"

#include "iahndl.hxx"

#include <memory>

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
        std::unique_ptr< ResMgr > xManager(ResMgr::CreateResMgr("uui"));
        if (!xManager.get())
            return;

        OUString aMessage;
        std::vector< OUString > aArguments;
        aArguments.push_back( aDocumentURL );

        sal_Int32 nResult = RET_CANCEL;
        if ( nMode == UUI_DOC_LOAD_LOCK )
        {
            aArguments.push_back( !aInfo.isEmpty()
                                  ? aInfo
                                  : ResId( STR_UNKNOWNUSER,
                                               *xManager.get() ).toString() );
            aMessage = ResId(STR_OPENLOCKED_MSG, *xManager.get()).toString();
            aMessage = UUIInteractionHelper::replaceMessageWithArguments(
                aMessage, aArguments );

            ScopedVclPtrInstance< OpenLockedQueryBox > xDialog(pParent, xManager.get(), aMessage);
            nResult = xDialog->Execute();
        }
        else if ( nMode == UUI_DOC_SAVE_LOCK )
        {
            aArguments.push_back( !aInfo.isEmpty()
                                  ? aInfo
                                  : ResId( STR_UNKNOWNUSER,
                                               *xManager.get() ).toString() );
            aMessage = ResId(STR_TRYLATER_MSG, *xManager.get()).toString();
            aMessage = UUIInteractionHelper::replaceMessageWithArguments(
                aMessage, aArguments );

            ScopedVclPtrInstance< TryLaterQueryBox > xDialog( pParent, xManager.get(), aMessage );
            nResult = xDialog->Execute();
        }
        else if ( nMode == UUI_DOC_OWN_LOAD_LOCK ||
                  nMode == UUI_DOC_OWN_SAVE_LOCK )
        {
            aArguments.push_back( aInfo );
            aMessage = ResId(nMode == UUI_DOC_OWN_SAVE_LOCK
                                          ? STR_ALREADYOPEN_SAVE_MSG
                                          : STR_ALREADYOPEN_MSG,
                                      *xManager.get() ).toString();
            aMessage = UUIInteractionHelper::replaceMessageWithArguments(
                aMessage, aArguments );

            ScopedVclPtrInstance< AlreadyOpenQueryBox > xDialog( pParent,
                                         xManager.get(),
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
        std::unique_ptr< ResMgr > xManager(ResMgr::CreateResMgr("uui"));
        if (!xManager.get())
            return;

        ScopedVclPtrInstance< FileChangedQueryBox > xDialog(pParent, xManager.get());
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

void
handleLockFileIgnoreRequest_(
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
        std::unique_ptr< ResMgr > xManager(ResMgr::CreateResMgr("uui"));
        if (!xManager.get())
            return;

        ScopedVclPtrInstance< LockFailedQueryBox > xDialog(pParent, xManager.get());
        sal_Int32 nResult = xDialog->Execute();

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
UUIInteractionHelper::handleLockFileIgnoreRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    uno::Any aAnyRequest(rRequest->getRequest());

    document::LockFileIgnoreRequest aLockFileIgnoreRequest;
    if (aAnyRequest >>= aLockFileIgnoreRequest )
    {
        handleLockFileIgnoreRequest_( getParentProperty(),
                                      rRequest->getContinuations() );
        return true;
    }
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
