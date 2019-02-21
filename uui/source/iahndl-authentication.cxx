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
#include <com/sun/star/ucb/XInteractionSupplyAuthentication2.hpp>
#include <com/sun/star/ucb/URLAuthenticationRequest.hpp>

#include <osl/diagnose.h>
#include <rtl/digest.h>
#include <unotools/resmgr.hxx>
#include <vcl/errcode.hxx>
#include <vcl/errinf.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include "authfallbackdlg.hxx"
#include <strings.hrc>
#include "getcontinuations.hxx"
#include "passwordcontainer.hxx"
#include "loginerr.hxx"
#include "logindlg.hxx"
#include "masterpasscrtdlg.hxx"
#include "masterpassworddlg.hxx"
#include "passworddlg.hxx"

#include "iahndl.hxx"

#include <memory>

using namespace com::sun::star;

namespace {

void
executeLoginDialog(
    weld::Window* pParent,
    LoginErrorInfo & rInfo,
    OUString const & rRealm)
{
    SolarMutexGuard aGuard;

    bool bAccount = (rInfo.GetFlags() & LOGINERROR_FLAG_MODIFY_ACCOUNT) != 0;
    bool bSavePassword   = rInfo.GetCanRememberPassword();
    bool bCanUseSysCreds = rInfo.GetCanUseSystemCredentials();

    LoginFlags nFlags = LoginFlags::NONE;
    if (rInfo.GetErrorText().isEmpty())
        nFlags |= LoginFlags::NoErrorText;
    if (!bAccount)
        nFlags |= LoginFlags::NoAccount;
    if (!(rInfo.GetFlags() & LOGINERROR_FLAG_MODIFY_USER_NAME))
        nFlags |= LoginFlags::UsernameReadonly;

    if (!bSavePassword)
        nFlags |= LoginFlags::NoSavePassword;

    if (!bCanUseSysCreds)
        nFlags |= LoginFlags::NoUseSysCreds;

    LoginDialog aDialog(pParent, nFlags, rInfo.GetServer(), rRealm);
    if (!rInfo.GetErrorText().isEmpty())
        aDialog.SetErrorText(rInfo.GetErrorText());
    aDialog.SetName(rInfo.GetUserName());
    if (bAccount)
        aDialog.ClearAccount();
    else
        aDialog.ClearPassword();
    aDialog.SetPassword(rInfo.GetPassword());

    if (bSavePassword)
    {
        std::locale aLocale(Translate::Create("uui"));
        aDialog.SetSavePasswordText(
            Translate::get(rInfo.GetIsRememberPersistent()
                      ? RID_SAVE_PASSWORD
                      : RID_KEEP_PASSWORD,
                  aLocale));

        aDialog.SetSavePassword(rInfo.GetIsRememberPassword());
    }

    if ( bCanUseSysCreds )
        aDialog.SetUseSystemCredentials( rInfo.GetIsUseSystemCredentials() );

    rInfo.SetResult(aDialog.run() == RET_OK ? DialogMask::ButtonsOk :
                                              DialogMask::ButtonsCancel);
    rInfo.SetUserName(aDialog.GetName());
    rInfo.SetPassword(aDialog.GetPassword());
    rInfo.SetAccount(aDialog.GetAccount());
    rInfo.SetIsRememberPassword(aDialog.IsSavePassword());

    if ( bCanUseSysCreds )
      rInfo.SetIsUseSystemCredentials( aDialog.IsUseSystemCredentials() );
}

void getRememberModes(
    uno::Sequence< ucb::RememberAuthentication > const & rRememberModes,
    ucb::RememberAuthentication & rPreferredMode,
    ucb::RememberAuthentication & rAlternateMode )
{
    sal_Int32 nCount = rRememberModes.getLength();
    OSL_ENSURE( (nCount > 0) && (nCount < 4),
                "ucb::RememberAuthentication sequence size mismatch!" );
    if ( nCount == 1 )
    {
        rPreferredMode = rAlternateMode = rRememberModes[ 0 ];
        return;
    }
    else
    {
        bool bHasRememberModeSession = false;
        bool bHasRememberModePersistent = false;

        for (sal_Int32 i = 0; i < nCount; ++i)
        {
            switch ( rRememberModes[i] )
            {
            case ucb::RememberAuthentication_NO:
                break;
            case ucb::RememberAuthentication_SESSION:
                bHasRememberModeSession = true;
                break;
            case ucb::RememberAuthentication_PERSISTENT:
                bHasRememberModePersistent = true;
                break;
            default:
                SAL_WARN( "uui", "Unsupported RememberAuthentication value" << static_cast<sal_Int32>(rRememberModes[i]) );
                break;
            }
        }

        if (bHasRememberModePersistent)
        {
            rPreferredMode = ucb::RememberAuthentication_PERSISTENT;
            if (bHasRememberModeSession)
                rAlternateMode = ucb::RememberAuthentication_SESSION;
            else
                rAlternateMode = ucb::RememberAuthentication_NO;
        }
        else
        {
            rPreferredMode = ucb::RememberAuthentication_SESSION;
            rAlternateMode = ucb::RememberAuthentication_NO;
        }
    }
}

void
handleAuthenticationRequest_(
    weld::Window * pParent,
    uno::Reference< task::XInteractionHandler2 > const & xIH,
    uno::Reference< uno::XComponentContext > const & xContext,
    ucb::AuthenticationRequest const & rRequest,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    const OUString & rURL)
{
    uno::Reference< task::XInteractionRetry > xRetry;
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< ucb::XInteractionSupplyAuthentication >
        xSupplyAuthentication;
    uno::Reference< ucb::XInteractionSupplyAuthentication2 >
        xSupplyAuthentication2;
    getContinuations(rContinuations, &xRetry, &xAbort, &xSupplyAuthentication);
    if (xSupplyAuthentication.is())
        xSupplyAuthentication2.set(xSupplyAuthentication, uno::UNO_QUERY);


    // First, try to obtain credentials from password container service.
    uui::PasswordContainerHelper aPwContainerHelper(xContext);
    if (aPwContainerHelper.handleAuthenticationRequest(rRequest,
                                                       xSupplyAuthentication,
                                                       rURL,
                                                       xIH))
    {
        xSupplyAuthentication->select();
        return;
    }


    // Second, try to obtain credentials from user via password dialog.
    ucb::RememberAuthentication eDefaultRememberMode
        = ucb::RememberAuthentication_SESSION;
    ucb::RememberAuthentication ePreferredRememberMode
        = eDefaultRememberMode;
    ucb::RememberAuthentication eAlternateRememberMode
        = ucb::RememberAuthentication_NO;

    if (xSupplyAuthentication.is())
    {
        getRememberModes(
            xSupplyAuthentication->getRememberPasswordModes(
                eDefaultRememberMode),
            ePreferredRememberMode,
            eAlternateRememberMode);
    }

    bool bCanUseSystemCredentials;
    sal_Bool bDefaultUseSystemCredentials;
    if (xSupplyAuthentication2.is())
    {
        bCanUseSystemCredentials
            = xSupplyAuthentication2->canUseSystemCredentials(
                bDefaultUseSystemCredentials);
    }
    else
    {
        bCanUseSystemCredentials = false;
        bDefaultUseSystemCredentials = false;
    }

    LoginErrorInfo aInfo;
    aInfo.SetServer(rRequest.ServerName);
    if (rRequest.HasAccount)
        aInfo.SetAccount(rRequest.Account);
    if (rRequest.HasUserName)
        aInfo.SetUserName(rRequest.UserName);
    if (rRequest.HasPassword)
        aInfo.SetPassword(rRequest.Password);
    aInfo.SetErrorText(rRequest.Diagnostic);

    aInfo.SetCanRememberPassword(
        ePreferredRememberMode != eAlternateRememberMode);
    aInfo.SetIsRememberPassword(
        ePreferredRememberMode == eDefaultRememberMode);
    aInfo.SetIsRememberPersistent(
        ePreferredRememberMode == ucb::RememberAuthentication_PERSISTENT);

    aInfo.SetCanUseSystemCredentials(bCanUseSystemCredentials);
    aInfo.SetIsUseSystemCredentials( bDefaultUseSystemCredentials );
    aInfo.SetModifyAccount(rRequest.HasAccount
                           && xSupplyAuthentication.is()
                           && xSupplyAuthentication->canSetAccount());
    aInfo.SetModifyUserName(rRequest.HasUserName
                            && xSupplyAuthentication.is()
                            && xSupplyAuthentication->canSetUserName());
    executeLoginDialog(pParent,
                       aInfo,
                       rRequest.HasRealm ? rRequest.Realm : OUString());
    switch (aInfo.GetResult())
    {
    case DialogMask::ButtonsOk:
        if (xSupplyAuthentication.is())
        {
            if (xSupplyAuthentication->canSetUserName())
                xSupplyAuthentication->setUserName(aInfo.GetUserName());
            if (xSupplyAuthentication->canSetPassword())
                xSupplyAuthentication->setPassword(aInfo.GetPassword());

            if (ePreferredRememberMode != eAlternateRememberMode)
            {
                // user had the choice.
                if (aInfo.GetIsRememberPassword())
                    xSupplyAuthentication->setRememberPassword(
                        ePreferredRememberMode);
                else
                    xSupplyAuthentication->setRememberPassword(
                        eAlternateRememberMode);
            }
            else
            {
                // user had no choice.
                xSupplyAuthentication->setRememberPassword(
                    ePreferredRememberMode);
            }

            if (rRequest.HasRealm)
            {
                if (xSupplyAuthentication->canSetRealm())
                    xSupplyAuthentication->setRealm(aInfo.GetAccount());
            }
            else if (xSupplyAuthentication->canSetAccount())
                xSupplyAuthentication->setAccount(aInfo.GetAccount());

            if ( xSupplyAuthentication2.is() && bCanUseSystemCredentials )
                xSupplyAuthentication2->setUseSystemCredentials(
                    aInfo.GetIsUseSystemCredentials() );

            xSupplyAuthentication->select();
        }


        // Third, store credentials in password container.

          if ( aInfo.GetIsUseSystemCredentials() )
          {
              if (aInfo.GetIsRememberPassword())
              {
                  if (!aPwContainerHelper.addRecord(
                          !rURL.isEmpty() ? rURL : rRequest.ServerName,
                          OUString(), // empty u/p -> sys creds
                          uno::Sequence< OUString >(),
                          xIH,
                          ePreferredRememberMode
                              == ucb::RememberAuthentication_PERSISTENT))
                  {
                      xSupplyAuthentication->setRememberPassword(
                          ucb::RememberAuthentication_NO);
                  }
              }
              else if (eAlternateRememberMode
                           == ucb::RememberAuthentication_SESSION)
              {
                  if (!aPwContainerHelper.addRecord(
                          !rURL.isEmpty() ? rURL : rRequest.ServerName,
                          OUString(), // empty u/p -> sys creds
                          uno::Sequence< OUString >(),
                          xIH,
                          false /* SESSION */))
                  {
                      xSupplyAuthentication->setRememberPassword(
                          ucb::RememberAuthentication_NO);
                  }
              }
          }
          // Empty user name can not be valid:
          else if (!aInfo.GetUserName().isEmpty())
          {
              uno::Sequence< OUString >
                  aPassList(aInfo.GetAccount().isEmpty() ? 1 : 2);
              aPassList[0] = aInfo.GetPassword();
              if (!aInfo.GetAccount().isEmpty())
                  aPassList[1] = aInfo.GetAccount();

              if (aInfo.GetIsRememberPassword())
              {
                  if (!aPwContainerHelper.addRecord(
                          !rURL.isEmpty() ? rURL : rRequest.ServerName,
                          aInfo.GetUserName(),
                          aPassList,
                          xIH,
                          ePreferredRememberMode
                              == ucb::RememberAuthentication_PERSISTENT))
                  {
                      xSupplyAuthentication->setRememberPassword(
                          ucb::RememberAuthentication_NO);
                  }
              }
              else if (eAlternateRememberMode
                           == ucb::RememberAuthentication_SESSION)
              {
                  if (!aPwContainerHelper.addRecord(
                          !rURL.isEmpty() ? rURL : rRequest.ServerName,
                          aInfo.GetUserName(),
                          aPassList,
                          xIH,
                          false /* SESSION */))
                  {
                      xSupplyAuthentication->setRememberPassword(
                          ucb::RememberAuthentication_NO);
                  }
              }
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
            MasterPasswordDialog aDialog(pParent, nMode, aResLocale);
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
        aBuffer.append(static_cast< sal_Unicode >('a' + (i >> 4)));
        aBuffer.append(static_cast< sal_Unicode >('a' + (i & 15)));
    }
    rInfo.SetPassword(aBuffer.makeStringAndClear());
}

void
handleMasterPasswordRequest_(
    weld::Window * pParent,
    task::PasswordRequestMode nMode,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
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
    bool bMSCryptoMode,
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
            const sal_uInt16 nMaxPasswdLen = bMSCryptoMode ? 15 : 0;   // 0 -> allow any length

            VclAbstractDialogFactory * pFact = VclAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractPasswordToOpenModifyDialog> const pDialog(
                pFact->CreatePasswordToOpenModifyDialog(pParent, nMaxPasswdLen, bIsPasswordToModify));

            rInfo.SetResult( pDialog->Execute() == RET_OK ? DialogMask::ButtonsOk : DialogMask::ButtonsCancel );
            rInfo.SetPassword( pDialog->GetPasswordToOpen() );
            rInfo.SetPasswordToModify( pDialog->GetPasswordToModify() );
            rInfo.SetRecommendToOpenReadonly( pDialog->IsRecommendToOpenReadonly() );
        }
    }
    else // enter password or reenter password
    {
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
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    const OUString& aDocumentName,
    bool bMSCryptoMode,
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
            aDocumentName, bMSCryptoMode, bIsPasswordToModify, bIsSimplePasswordRequest );

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
    uno::Any aAnyRequest(rRequest->getRequest());
    uno::Reference<awt::XWindow> xParent = getParentXWindow();

    ucb::URLAuthenticationRequest aURLAuthenticationRequest;
    if (aAnyRequest >>= aURLAuthenticationRequest)
    {
        handleAuthenticationRequest_(Application::GetFrameWeld(xParent),
                                     getInteractionHandler(),
                                     m_xContext,
                                     aURLAuthenticationRequest,
                                     rRequest->getContinuations(),
                                     aURLAuthenticationRequest.URL);
        return true;
    }

    ucb::AuthenticationRequest aAuthenticationRequest;
    if (aAnyRequest >>= aAuthenticationRequest)
    {
        handleAuthenticationRequest_(Application::GetFrameWeld(xParent),
                                     getInteractionHandler(),
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
    uno::Any aAnyRequest(rRequest->getRequest());

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
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const & rContinuations = rRequest->getContinuations();
    OUString aDocumentName;
    bool bMSCryptoMode          = false;
    bool bIsPasswordToModify    = false;

    bool bDoHandleRequest = false;

    uno::Any aAnyRequest(rRequest->getRequest());

    task::DocumentPasswordRequest2 aDocumentPasswordRequest2;
    if (!bDoHandleRequest && (aAnyRequest >>= aDocumentPasswordRequest2))
    {
        nMode               = aDocumentPasswordRequest2.Mode;
        aDocumentName       = aDocumentPasswordRequest2.Name;
        OSL_ENSURE( !bMSCryptoMode, "bMSCryptoMode should be false" );
        bIsPasswordToModify = aDocumentPasswordRequest2.IsRequestPasswordToModify;

        bDoHandleRequest = true;
    }

    task::DocumentPasswordRequest aDocumentPasswordRequest;
    if (!bDoHandleRequest && (aAnyRequest >>= aDocumentPasswordRequest))
    {
        nMode               = aDocumentPasswordRequest.Mode;
        aDocumentName       = aDocumentPasswordRequest.Name;
        OSL_ENSURE( !bMSCryptoMode, "bMSCryptoMode should be false" );
        OSL_ENSURE( !bIsPasswordToModify, "bIsPasswordToModify should be false" );

        bDoHandleRequest = true;
    }

    task::DocumentMSPasswordRequest2 aDocumentMSPasswordRequest2;
    if (!bDoHandleRequest && (aAnyRequest >>= aDocumentMSPasswordRequest2))
    {
        nMode               = aDocumentMSPasswordRequest2.Mode;
        aDocumentName       = aDocumentMSPasswordRequest2.Name;
        bMSCryptoMode       = true;
        bIsPasswordToModify = aDocumentMSPasswordRequest2.IsRequestPasswordToModify;

        bDoHandleRequest = true;
    }

    task::DocumentMSPasswordRequest aDocumentMSPasswordRequest;
    if (!bDoHandleRequest && (aAnyRequest >>= aDocumentMSPasswordRequest))
    {
        nMode               = aDocumentMSPasswordRequest.Mode;
        aDocumentName       = aDocumentMSPasswordRequest.Name;
        bMSCryptoMode       = true;
        OSL_ENSURE( !bIsPasswordToModify, "bIsPasswordToModify should be false" );

        bDoHandleRequest = true;
    }

    if (bDoHandleRequest)
    {
        handlePasswordRequest_( Application::GetFrameWeld(xParent), nMode, rContinuations,
                aDocumentName, bMSCryptoMode, bIsPasswordToModify );
        return true;
    }

    task::PasswordRequest aPasswordRequest;
    if( aAnyRequest >>= aPasswordRequest )
    {
        handlePasswordRequest_(Application::GetFrameWeld(xParent),
                               aPasswordRequest.Mode,
                               rRequest->getContinuations(),
                               OUString(),
                               false /* bool bMSCryptoMode */,
                               false /* bool bIsPasswordToModify */,
                               true  /* bool bIsSimplePasswordRequest */ );
        return true;
    }

    return false;
}

void
UUIInteractionHelper::handleAuthFallbackRequest( const OUString & instructions,
        const OUString & url,
        uno::Sequence< uno::Reference< task::XInteractionContinuation > > const & rContinuations )
{
    uno::Reference<awt::XWindow> xParent = getParentXWindow();
    AuthFallbackDlg dlg(Application::GetFrameWeld(xParent), instructions, url);
    int retCode = dlg.run();
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< ucb::XInteractionAuthFallback > xAuthFallback;
    getContinuations(rContinuations, &xAbort, &xAuthFallback);

    if( retCode == RET_OK && xAuthFallback.is( ) )
    {
        xAuthFallback->setCode(dlg.GetCode());
        xAuthFallback->select( );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
