/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "com/sun/star/document/ChangedByOthersRequest.hpp"
#include "com/sun/star/document/LockedDocumentRequest.hpp"
#include "com/sun/star/document/LockedOnSavingRequest.hpp"
#include "com/sun/star/document/LockFileIgnoreRequest.hpp"
#include "com/sun/star/document/OwnLockOnDocumentRequest.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/task/XInteractionDisapprove.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionRequest.hpp"

#include "osl/mutex.hxx"
#include "vcl/svapp.hxx"
#include "vcl/msgbox.hxx"

#include "ids.hrc"
#include "getcontinuations.hxx"
#include "openlocked.hxx"
#include "trylater.hxx"
#include "alreadyopen.hxx"
#include "filechanged.hxx"
#include "lockfailed.hxx"

#include "iahndl.hxx"

#define UUI_DOC_LOAD_LOCK       0
#define UUI_DOC_OWN_LOAD_LOCK   1
#define UUI_DOC_SAVE_LOCK       2
#define UUI_DOC_OWN_SAVE_LOCK   3

using namespace com::sun::star;

namespace {

void
handleLockedDocumentRequest_(
    Window * pParent,
    const ::rtl::OUString& aDocumentURL,
    const ::rtl::OUString& aInfo,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    sal_uInt16 nMode )
    SAL_THROW((uno::RuntimeException))
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
        std::auto_ptr< ResMgr > xManager(
            ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        if (!xManager.get())
            return;

        ::rtl::OUString aMessage;
        std::vector< rtl::OUString > aArguments;
        aArguments.push_back( aDocumentURL );

        sal_Int32 nResult = RET_CANCEL;
        if ( nMode == UUI_DOC_LOAD_LOCK )
        {
            aArguments.push_back( aInfo.getLength()
                                  ? aInfo
                                  : ::rtl::OUString( String(
                                        ResId( STR_UNKNOWNUSER,
                                               *xManager.get() ) ) ) );
            aMessage = String( ResId( STR_OPENLOCKED_MSG, *xManager.get() ) );
            aMessage = UUIInteractionHelper::replaceMessageWithArguments(
                aMessage, aArguments );

            std::auto_ptr< OpenLockedQueryBox > xDialog(new OpenLockedQueryBox(
                            pParent, xManager.get(), aMessage ) );
            nResult = xDialog->Execute();
        }
        else if ( nMode == UUI_DOC_SAVE_LOCK )
        {
            aArguments.push_back( aInfo.getLength()
                                  ? aInfo
                                  : ::rtl::OUString( String(
                                        ResId( STR_UNKNOWNUSER,
                                               *xManager.get() ) ) ) );
            aMessage = String( ResId( STR_TRYLATER_MSG, *xManager.get() ) );
            aMessage = UUIInteractionHelper::replaceMessageWithArguments(
                aMessage, aArguments );

            std::auto_ptr< TryLaterQueryBox > xDialog(
                new TryLaterQueryBox( pParent, xManager.get(), aMessage ) );
            nResult = xDialog->Execute();
        }
        else if ( nMode == UUI_DOC_OWN_LOAD_LOCK ||
                  nMode == UUI_DOC_OWN_SAVE_LOCK )
        {
            aArguments.push_back( aInfo );
            aMessage = String( ResId( nMode == UUI_DOC_OWN_SAVE_LOCK
                                          ? STR_ALREADYOPEN_SAVE_MSG
                                          : STR_ALREADYOPEN_MSG,
                                      *xManager.get() ) );
            aMessage = UUIInteractionHelper::replaceMessageWithArguments(
                aMessage, aArguments );

            std::auto_ptr< AlreadyOpenQueryBox > xDialog(
                new AlreadyOpenQueryBox( pParent,
                                         xManager.get(),
                                         aMessage,
                                         nMode == UUI_DOC_OWN_SAVE_LOCK ) );
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
        throw uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  uno::Reference< uno::XInterface >());
    }
}

void
handleChangedByOthersRequest_(
    Window * pParent,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations )
    SAL_THROW((uno::RuntimeException))
{
    uno::Reference< task::XInteractionApprove > xApprove;
    uno::Reference< task::XInteractionAbort > xAbort;
    getContinuations(rContinuations, &xApprove, &xAbort);

    if ( !xApprove.is() || !xAbort.is() )
        return;

    try
    {
        SolarMutexGuard aGuard;
        std::auto_ptr< ResMgr > xManager(
            ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        if (!xManager.get())
            return;

        std::auto_ptr< FileChangedQueryBox > xDialog(
            new FileChangedQueryBox( pParent, xManager.get() ) );
        sal_Int32 nResult = xDialog->Execute();

        if ( nResult == RET_YES )
            xApprove->select();
        else
            xAbort->select();
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  uno::Reference< uno::XInterface >());
    }
}

void
handleLockFileIgnoreRequest_(
    Window * pParent,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations )
    SAL_THROW((uno::RuntimeException))
{
    uno::Reference< task::XInteractionApprove > xApprove;
    uno::Reference< task::XInteractionAbort > xAbort;
    getContinuations(rContinuations, &xApprove, &xAbort);

    if ( !xApprove.is() || !xAbort.is() )
        return;

    try
    {
        SolarMutexGuard aGuard;
        std::auto_ptr< ResMgr > xManager(
            ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        if (!xManager.get())
            return;

        std::auto_ptr< LockFailedQueryBox > xDialog(
            new LockFailedQueryBox( pParent, xManager.get() ) );
        sal_Int32 nResult = xDialog->Execute();

        if ( nResult == RET_OK )
            xApprove->select();
        else
            xAbort->select();
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  uno::Reference< uno::XInterface >());
    }
}

} // namespace

bool
UUIInteractionHelper::handleLockedDocumentRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    SAL_THROW((::com::sun::star::uno::RuntimeException))
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
    SAL_THROW((uno::RuntimeException))
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
    SAL_THROW((uno::RuntimeException))
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
