/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

#include <config_oauth2.h>

#include <com/sun/star/task/DocumentPasswordRequest.hpp>
#include <com/sun/star/task/DocumentPasswordRequest2.hpp>
#include <com/sun/star/task/DocumentMSPasswordRequest.hpp>
#include <com/sun/star/task/DocumentMSPasswordRequest2.hpp>
#include <com/sun/star/task/MasterPasswordRequest.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionPassword.hpp>
#include <com/sun/star/task/XInteractionPassword2.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/ucb/XInteractionAuthFallback.hpp>
#include <com/sun/star/ucb/XInteractionSupplyAuthentication.hpp>
#include <com/sun/star/ucb/URLAuthenticationRequest.hpp>

#include <osl/diagnose.h>
#include <rtl/digest.h>
#include <rtl/ustrbuf.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/errinf.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include "authfallbackdlg.hxx"
#include <strings.hrc>
#include "getcontinuations.hxx"
#include "passwordcontainer.hxx"
#include "loginerr.hxx"
#include "masterpasscrtdlg.hxx"
#include "masterpassworddlg.hxx"
#include "passworddlg.hxx"

#include "iahndl.hxx"
#include "iahndl-oauth2.hxx"

#include <memory>

using namespace com::sun::star;
using namespace ::cpo;

namespace {

void
handleAuthenticationRequest_(
    uno::Reference< task::XInteractionHandler2 > const & xIH,
    uno::Reference< cpo::uno::XComponentContext > const & xContext,
    ucb::AuthenticationRequest const & rRequest,
    cpo::uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    const OUString & rURL)
{
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< ucb::XInteractionSupplyAuthentication >
        xSupplyAuthentication;
    getContinuations(rContinuations, &xAbort, &xSupplyAuthentication);

    // Use the credentials from the password container service, if it has them.
    uui::PasswordContainerHelper aPwContainerHelper(xContext);
    if (aPwContainerHelper.handleAuthenticationRequest(rRequest,
                                                       xSupplyAuthentication,
                                                       rURL,
                                                       xIH))
    {
        xSupplyAuthentication->select();
        return;
    }

    // Without stored credentials the request is aborted.
    if (xAbort.is())
        xAbort->select();
}

void
executeMasterPasswordDialog(
    weld::Window* pParent,
    LoginErrorInfo & rInfo,
    task::PasswordRequestMode nMode)
{
    OString aMaster;
    {
        SolarMutexGuard aGuard;

        std::locale aResLocale(Translate::Create("uui"));
        if( nMode == task::PasswordRequestMode_PASSWORD_CREATE )
        {
            MasterPasswordCreateDialog aDialog(pParent, aResLocale);
            rInfo.SetResult(aDialog.run()
                == RET_OK ? DialogMask::ButtonsOk : DialogMask::ButtonsCancel);
            aMaster = OUStringToOString(
                aDialog.GetMasterPassword(), RTL_TEXTENCODING_UTF8);
        }
        else
        {
            if (nMode == css::task::PasswordRequestMode_PASSWORD_REENTER)
            {
                OUString aErrorMsg(Translate::get(STR_ERROR_MASTERPASSWORD_WRONG, aResLocale));
                std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(
                    pParent, VclMessageType::Warning, VclButtonsType::Ok, aErrorMsg));
                xErrorBox->run();
            }

            MasterPasswordDialog aDialog(pParent);
            rInfo.SetResult(aDialog.run()
                == RET_OK ? DialogMask::ButtonsOk : DialogMask::ButtonsCancel);
            aMaster = OUStringToOString(
                aDialog.GetMasterPassword(), RTL_TEXTENCODING_UTF8);
        }
    }

    sal_uInt8 aKey[RTL_DIGEST_LENGTH_MD5];
    // FIXME this is subject to the SHA1-bug tdf#114939 - but this
    // MasterPassword stuff is just stored in the UserInstallation,
    // so no interop concerns
    rtl_digest_PBKDF2(aKey,
                      RTL_DIGEST_LENGTH_MD5,
                      reinterpret_cast< sal_uInt8 const * >(aMaster.getStr()),
                      aMaster.getLength(),
                      reinterpret_cast< sal_uInt8 const * >(
                          "3B5509ABA6BC42D9A3A1F3DAD49E56A51"),
                      32,
                      1000);

    OUStringBuffer aBuffer;
    for (sal_uInt8 i : aKey)
    {
        // match PasswordContainer::DecodePasswords aMasterPasswd.copy(index * 2, 2).toUInt32(16));
        aBuffer.append(OUString::number(i >> 4, 16) + OUString::number(i & 15, 16));
    }
    rInfo.SetPassword(aBuffer.makeStringAndClear());
}

void
handleMasterPasswordRequest_(
    weld::Window * pParent,
    task::PasswordRequestMode nMode,
    cpo::uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations)
{
    uno::Reference< task::XInteractionRetry > xRetry;
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< ucb::XInteractionSupplyAuthentication >
        xSupplyAuthentication;
    getContinuations(rContinuations, &xRetry, &xAbort, &xSupplyAuthentication);
    LoginErrorInfo aInfo;

    // in case of master password a hash code is returned
    executeMasterPasswordDialog(pParent, aInfo, nMode);

    switch (aInfo.GetResult())
    {
    case DialogMask::ButtonsOk:
        if (xSupplyAuthentication.is())
        {
            if (xSupplyAuthentication->canSetPassword())
                xSupplyAuthentication->setPassword(aInfo.GetPassword());
            xSupplyAuthentication->select();
        }
        break;

    case DialogMask::ButtonsRetry:
        if (xRetry.is())
            xRetry->select();
        break;

    default:
        if (xAbort.is())
            xAbort->select();
        break;
    }
}

void
executePasswordDialog(
    weld::Window * pParent,
    LoginErrorInfo & rInfo,
    task::PasswordRequestMode nMode,
    const OUString& aDocName,
    sal_uInt16 nMaxPasswordLen,
    bool bIsPasswordToModify,
    bool bIsSimplePasswordRequest )
{
    SolarMutexGuard aGuard;

    std::locale aResLocale(Translate::Create("uui"));
    if( nMode == task::PasswordRequestMode_PASSWORD_CREATE )
    {
        if (bIsSimplePasswordRequest)
        {
            std::unique_ptr<PasswordDialog> xDialog(new PasswordDialog(pParent, nMode,
                aResLocale, aDocName, bIsPasswordToModify, bIsSimplePasswordRequest));
            xDialog->SetMinLen(0);

            rInfo.SetResult(xDialog->run() == RET_OK ? DialogMask::ButtonsOk : DialogMask::ButtonsCancel);
            rInfo.SetPassword(xDialog->GetPassword());
        }
        else
        {
            VclAbstractDialogFactory * pFact = VclAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractPasswordToOpenModifyDialog> const pDialog(
                pFact->CreatePasswordToOpenModifyDialog(pParent, nMaxPasswordLen, bIsPasswordToModify));

            rInfo.SetResult( pDialog->Execute() == RET_OK ? DialogMask::ButtonsOk : DialogMask::ButtonsCancel );
            rInfo.SetPassword( pDialog->GetPasswordToOpen() );
            rInfo.SetPasswordToModify( pDialog->GetPasswordToModify() );
            rInfo.SetRecommendToOpenReadonly( pDialog->IsRecommendToOpenReadonly() );
        }
    }
    else // enter password or reenter password
    {
        if (nMode == task::PasswordRequestMode_PASSWORD_REENTER)
        {
            TranslateId pOpenToModifyErrStrId = bIsPasswordToModify
                                                    ? STR_ERROR_PASSWORD_TO_MODIFY_WRONG
                                                    : STR_ERROR_PASSWORD_TO_OPEN_WRONG;
            TranslateId pErrStrId = bIsSimplePasswordRequest ? STR_ERROR_SIMPLE_PASSWORD_WRONG
                                                             : pOpenToModifyErrStrId;
            OUString aErrorMsg(Translate::get(pErrStrId, aResLocale));
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                pParent, VclMessageType::Warning, VclButtonsType::Ok, aErrorMsg));
            xBox->run();
        }

        std::unique_ptr<PasswordDialog> xDialog(new PasswordDialog(pParent, nMode,
            aResLocale, aDocName, bIsPasswordToModify, bIsSimplePasswordRequest));
        xDialog->SetMinLen(0);

        rInfo.SetResult(xDialog->run() == RET_OK ? DialogMask::ButtonsOk : DialogMask::ButtonsCancel);
        rInfo.SetPassword(bIsPasswordToModify ? OUString() : xDialog->GetPassword());
        rInfo.SetPasswordToModify(bIsPasswordToModify ? xDialog->GetPassword() : OUString());
    }
}

void
handlePasswordRequest_(
    weld::Window * pParent,
    task::PasswordRequestMode nMode,
    cpo::uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    const OUString& aDocumentName,
    sal_uInt16 nMaxPasswordLen,
    bool bIsPasswordToModify,
    bool bIsSimplePasswordRequest = false )
{
    uno::Reference< task::XInteractionRetry > xRetry;
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< task::XInteractionPassword > xPassword;
    uno::Reference< task::XInteractionPassword2 > xPassword2;
    getContinuations(rContinuations, &xRetry, &xAbort, &xPassword2, &xPassword);

    if ( xPassword2.is() && !xPassword.is() )
        xPassword.set( xPassword2, uno::UNO_QUERY_THROW );

    LoginErrorInfo aInfo;

    executePasswordDialog( pParent, aInfo, nMode,
            aDocumentName, nMaxPasswordLen, bIsPasswordToModify, bIsSimplePasswordRequest );

    switch (aInfo.GetResult())
    {
    case DialogMask::ButtonsOk:
        OSL_ENSURE( !bIsPasswordToModify || xPassword2.is(), "PasswordToModify is requested, but there is no Interaction!" );
        if (xPassword.is())
        {
            if (xPassword2.is())
            {
                xPassword2->setPasswordToModify( aInfo.GetPasswordToModify() );
                xPassword2->setRecommendReadOnly( aInfo.IsRecommendToOpenReadonly() );
            }

            xPassword->setPassword(aInfo.GetPassword());
            xPassword->select();
        }
        break;

    case DialogMask::ButtonsRetry:
        if (xRetry.is())
            xRetry->select();
        break;

    default:
        if (xAbort.is())
            xAbort->select();
        break;
    }
}

} // namespace

bool
UUIInteractionHelper::handleAuthenticationRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    cpo::uno::Any aAnyRequest(rRequest->getRequest());

    ucb::URLAuthenticationRequest aURLAuthenticationRequest;
    if (aAnyRequest >>= aURLAuthenticationRequest)
    {
        handleAuthenticationRequest_(getInteractionHandler(),
                                     m_xContext,
                                     aURLAuthenticationRequest,
                                     rRequest->getContinuations(),
                                     aURLAuthenticationRequest.URL);
        return true;
    }

    ucb::AuthenticationRequest aAuthenticationRequest;
    if (aAnyRequest >>= aAuthenticationRequest)
    {
        handleAuthenticationRequest_(getInteractionHandler(),
                                     m_xContext,
                                     aAuthenticationRequest,
                                     rRequest->getContinuations(),
                                     OUString());
        return true;
    }
    return false;
}

bool
UUIInteractionHelper::handleMasterPasswordRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    cpo::uno::Any aAnyRequest(rRequest->getRequest());

    task::MasterPasswordRequest aMasterPasswordRequest;
    if (aAnyRequest >>= aMasterPasswordRequest)
    {
        uno::Reference<awt::XWindow> xParent = getParentXWindow();

        handleMasterPasswordRequest_(Application::GetFrameWeld(xParent),
                                     aMasterPasswordRequest.Mode,
                                     rRequest->getContinuations());
        return true;
    }
    return false;
}

bool
UUIInteractionHelper::handlePasswordRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    // parameters to be filled for the call to handlePasswordRequest_
    uno::Reference<awt::XWindow> xParent = getParentXWindow();
    task::PasswordRequestMode nMode = task::PasswordRequestMode_PASSWORD_ENTER;
    cpo::uno::Sequence< uno::Reference< task::XInteractionContinuation > > const aContinuations = rRequest->getContinuations();
    OUString aDocumentName;
    sal_uInt16 nMaxPasswordLen  = 0;        // any length
    bool bIsPasswordToModify    = false;

    bool bDoHandleRequest = false;

    cpo::uno::Any aAnyRequest(rRequest->getRequest());

    do
    {
        task::DocumentPasswordRequest2 aDocumentPasswordRequest2;
        if (aAnyRequest >>= aDocumentPasswordRequest2)
        {
            nMode               = aDocumentPasswordRequest2.Mode;
            aDocumentName       = aDocumentPasswordRequest2.Name;
            bIsPasswordToModify = aDocumentPasswordRequest2.IsRequestPasswordToModify;

            bDoHandleRequest = true;
            break;  // do
        }

        task::DocumentPasswordRequest aDocumentPasswordRequest;
        if (aAnyRequest >>= aDocumentPasswordRequest)
        {
            nMode               = aDocumentPasswordRequest.Mode;
            aDocumentName       = aDocumentPasswordRequest.Name;

            bDoHandleRequest = true;
            break;  // do
        }

        task::DocumentMSPasswordRequest2 aDocumentMSPasswordRequest2;
        if (aAnyRequest >>= aDocumentMSPasswordRequest2)
        {
            nMode               = aDocumentMSPasswordRequest2.Mode;
            aDocumentName       = aDocumentMSPasswordRequest2.Name;
            nMaxPasswordLen     = 15;
            bIsPasswordToModify = aDocumentMSPasswordRequest2.IsRequestPasswordToModify;

            bDoHandleRequest = true;
            break;  // do
        }

        task::DocumentMSPasswordRequest aDocumentMSPasswordRequest;
        if (aAnyRequest >>= aDocumentMSPasswordRequest)
        {
            nMode               = aDocumentMSPasswordRequest.Mode;
            aDocumentName       = aDocumentMSPasswordRequest.Name;
            nMaxPasswordLen     = 15;

            bDoHandleRequest = true;
            break;  // do
        }
    }
    while (false);

    if (bDoHandleRequest)
    {
        handlePasswordRequest_( Application::GetFrameWeld(xParent), nMode, aContinuations,
                aDocumentName, nMaxPasswordLen, bIsPasswordToModify );
        return true;
    }

    task::PasswordRequest aPasswordRequest;
    if( aAnyRequest >>= aPasswordRequest )
    {
        handlePasswordRequest_(Application::GetFrameWeld(xParent),
                               aPasswordRequest.Mode,
                               rRequest->getContinuations(),
                               OUString(),
                               0     /* sal_uInt16 nMaxPasswordLen */,
                               false /* bool bIsPasswordToModify */,
                               true  /* bool bIsSimplePasswordRequest */ );
        return true;
    }

    return false;
}

void
UUIInteractionHelper::handleAuthFallbackRequest( const OUString & instructions,
        const OUString & url,
        cpo::uno::Sequence< uno::Reference< task::XInteractionContinuation > > const & rContinuations )
{
    OUString code;
    if (url.indexOf(OAUTH2_REDIRECT_URI_PREFIX) >= 0)
    {
        OAuth2Request request(url);
        request.run();
        code = request.getRetCode();
    }
    else
    {
        uno::Reference<awt::XWindow> xParent = getParentXWindow();
        AuthFallbackDlg dlg(Application::GetFrameWeld(xParent), instructions, url);
        if (dlg.run() == RET_OK)
            code = dlg.GetCode();
    }
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< ucb::XInteractionAuthFallback > xAuthFallback;
    getContinuations(rContinuations, &xAbort, &xAuthFallback);

    if (!code.isEmpty() && xAuthFallback.is())
    {
        xAuthFallback->setCode(code);
        xAuthFallback->select( );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
