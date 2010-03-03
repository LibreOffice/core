/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: $
 * $Revision: $
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

#include "com/sun/star/task/DocumentPasswordRequest.hpp"
#include "com/sun/star/task/DocumentPasswordRequest2.hpp"
#include "com/sun/star/task/DocumentMSPasswordRequest.hpp"
#include "com/sun/star/task/MasterPasswordRequest.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionPassword.hpp"
#include "com/sun/star/task/XInteractionRetry.hpp"
#include "com/sun/star/ucb/XInteractionSupplyAuthentication2.hpp"
#include "com/sun/star/ucb/URLAuthenticationRequest.hpp"

#include "osl/diagnose.h"
#include "rtl/digest.h"
#include "vos/mutex.hxx"
#include "tools/errcode.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/svapp.hxx"

#include "ids.hrc"
#include "getcontinuations.hxx"
#include "passwordcontainer.hxx"
#include "loginerr.hxx"
#include "logindlg.hxx"
#include "masterpasscrtdlg.hxx"
#include "masterpassworddlg.hxx"
#include "passcrtdlg.hxx"
#include "passworddlg.hxx"

#include "iahndl.hxx"

using namespace com::sun::star;

namespace {

void
executeLoginDialog(
    Window * pParent,
    LoginErrorInfo & rInfo,
    rtl::OUString const & rRealm)
    SAL_THROW((uno::RuntimeException))
{
    try
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        bool bAccount = (rInfo.GetFlags() & LOGINERROR_FLAG_MODIFY_ACCOUNT)
                            != 0;
        bool bSavePassword = rInfo.GetIsPersistentPassword()
                             || rInfo.GetIsSavePassword();
        bool bCanUseSysCreds = rInfo.GetCanUseSystemCredentials();

        sal_uInt16 nFlags = 0;
        if (rInfo.GetPath().Len() == 0)
            nFlags |= LF_NO_PATH;
        if (rInfo.GetErrorText().Len() == 0)
            nFlags |= LF_NO_ERRORTEXT;
        if (!bAccount)
            nFlags |= LF_NO_ACCOUNT;
        if (!(rInfo.GetFlags() & LOGINERROR_FLAG_MODIFY_USER_NAME))
            nFlags |= LF_USERNAME_READONLY;

        if (!bSavePassword)
            nFlags |= LF_NO_SAVEPASSWORD;

        if (!bCanUseSysCreds)
            nFlags |= LF_NO_USESYSCREDS;

        std::auto_ptr< ResMgr > xManager(
            ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        UniString aRealm(rRealm);
        std::auto_ptr< LoginDialog > xDialog(
            new LoginDialog(pParent,
                            nFlags,
                            rInfo.GetServer(),
                            &aRealm,
                            xManager.get()));
        if (rInfo.GetErrorText().Len() != 0)
            xDialog->SetErrorText(rInfo.GetErrorText());
        xDialog->SetName(rInfo.GetUserName());
        if (bAccount)
            xDialog->ClearAccount();
        else
            xDialog->ClearPassword();
        xDialog->SetPassword(rInfo.GetPassword());

        if (bSavePassword)
        {
            xDialog->
                SetSavePasswordText(ResId(rInfo.GetIsPersistentPassword() ?
                                              RID_SAVE_PASSWORD :
                                              RID_KEEP_PASSWORD,
                                          *xManager.get()));
            xDialog->SetSavePassword(rInfo.GetIsSavePassword());
        }

        if ( bCanUseSysCreds )
            xDialog->SetUseSystemCredentials(
                rInfo.GetIsUseSystemCredentials() );

        rInfo.SetResult(xDialog->Execute() == RET_OK ? ERRCODE_BUTTON_OK :
                                                       ERRCODE_BUTTON_CANCEL);
        rInfo.SetUserName(xDialog->GetName());
        rInfo.SetPassword(xDialog->GetPassword());
        rInfo.SetAccount(xDialog->GetAccount());
        rInfo.SetSavePassword(xDialog->IsSavePassword());

        if ( bCanUseSysCreds )
          rInfo.SetIsUseSystemCredentials( xDialog->IsUseSystemCredentials() );
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  uno::Reference< uno::XInterface >());
    }
}

void
handleAuthenticationRequest_(
    Window * pParent,
    uno::Reference< task::XInteractionHandler > const & xIH,
    uno::Reference< lang::XMultiServiceFactory > const & xServiceFactory,
    ucb::AuthenticationRequest const & rRequest,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    const rtl::OUString & rURL)
    SAL_THROW((uno::RuntimeException))
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

    //////////////////////////
    // First, try to obatin credentials from password container service.
    uui::PasswordContainerHelper aPwContainerHelper(xServiceFactory);
    if (aPwContainerHelper.handleAuthenticationRequest(rRequest,
                                                       xSupplyAuthentication,
                                                       rURL,
                                                       xIH))
    {
        xSupplyAuthentication->select();
        return;
    }

    //////////////////////////
    // Second, try to obtain credentials from user via password dialog.
    bool bRemember;
    bool bRememberPersistent;
    if (xSupplyAuthentication.is())
    {
        ucb::RememberAuthentication eDefault;
        uno::Sequence< ucb::RememberAuthentication >
            aModes(xSupplyAuthentication->getRememberPasswordModes(eDefault));
        bRemember = eDefault != ucb::RememberAuthentication_NO;
        bRememberPersistent = false;
        for (sal_Int32 i = 0; i < aModes.getLength(); ++i)
            if (aModes[i] == ucb::RememberAuthentication_PERSISTENT)
            {
                bRememberPersistent = true;
                break;
            }
    }
    else
    {
        bRemember = false;
        bRememberPersistent = false;
    }

    sal_Bool bCanUseSystemCredentials;
    sal_Bool bDefaultUseSystemCredentials;
    if (xSupplyAuthentication2.is())
    {
        bCanUseSystemCredentials
            = xSupplyAuthentication2->canUseSystemCredentials(
                bDefaultUseSystemCredentials);
    }
    else
    {
        bCanUseSystemCredentials = sal_False;
        bDefaultUseSystemCredentials = sal_False;
    }

    LoginErrorInfo aInfo;
    aInfo.SetTitle(rRequest.ServerName);
    aInfo.SetServer(rRequest.ServerName);
    if (rRequest.HasAccount)
        aInfo.SetAccount(rRequest.Account);
    if (rRequest.HasUserName)
        aInfo.SetUserName(rRequest.UserName);
    if (rRequest.HasPassword)
        aInfo.SetPassword(rRequest.Password);
    aInfo.SetErrorText(rRequest.Diagnostic);
    aInfo.SetPersistentPassword(bRememberPersistent);
    aInfo.SetSavePassword(bRemember);
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
                       rRequest.HasRealm ? rRequest.Realm : rtl::OUString());
    switch (aInfo.GetResult())
    {
    case ERRCODE_BUTTON_OK:
        if (xSupplyAuthentication.is())
        {
            if (xSupplyAuthentication->canSetUserName())
                xSupplyAuthentication->setUserName(aInfo.GetUserName());
            if (xSupplyAuthentication->canSetPassword())
                xSupplyAuthentication->setPassword(aInfo.GetPassword());
            xSupplyAuthentication->
                setRememberPassword(
                    aInfo.GetIsSavePassword() ?
                       bRememberPersistent ?
                       ucb::RememberAuthentication_PERSISTENT :
                           ucb::RememberAuthentication_SESSION :
                               ucb::RememberAuthentication_NO);
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

        //////////////////////////
        // Third, store credentials in password container.

        if ( aInfo.GetIsUseSystemCredentials() )
        {
            if (aInfo.GetIsSavePassword())
            {
                aPwContainerHelper.addRecord(
                    rURL.getLength() ? rURL : rRequest.ServerName,
                    rtl::OUString(), // empty u/p -> sys creds
                    uno::Sequence< rtl::OUString >(),
                    xIH,
                    bRememberPersistent);
            }
        }
        // Empty user name can not be valid:
        else if (aInfo.GetUserName().Len() != 0)
        {
            if (aInfo.GetIsSavePassword())
            {
                uno::Sequence< rtl::OUString >
                    aPassList(aInfo.GetAccount().Len() == 0 ? 1 : 2);
                aPassList[0] = aInfo.GetPassword();
                if (aInfo.GetAccount().Len() != 0)
                    aPassList[1] = aInfo.GetAccount();

                aPwContainerHelper.addRecord(
                    rURL.getLength() ? rURL : rRequest.ServerName,
                    aInfo.GetUserName(),
                    aPassList,
                    xIH,
                    bRememberPersistent);
            }
        }
        break;

    case ERRCODE_BUTTON_RETRY:
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
    Window * pParent,
    LoginErrorInfo & rInfo,
    task::PasswordRequestMode nMode)
        SAL_THROW((uno::RuntimeException))
{
    rtl::OString aMaster;
    try
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        std::auto_ptr< ResMgr > xManager(
            ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        if( nMode == task::PasswordRequestMode_PASSWORD_CREATE )
        {
            std::auto_ptr< MasterPasswordCreateDialog > xDialog(
                new MasterPasswordCreateDialog(pParent, xManager.get()));
            rInfo.SetResult(xDialog->Execute()
                == RET_OK ? ERRCODE_BUTTON_OK : ERRCODE_BUTTON_CANCEL);
            aMaster = rtl::OUStringToOString(
                xDialog->GetMasterPassword(), RTL_TEXTENCODING_UTF8);
        }
        else
        {
            std::auto_ptr< MasterPasswordDialog > xDialog(
                new MasterPasswordDialog(pParent, nMode, xManager.get()));
            rInfo.SetResult(xDialog->Execute()
                == RET_OK ? ERRCODE_BUTTON_OK : ERRCODE_BUTTON_CANCEL);
            aMaster = rtl::OUStringToOString(
                xDialog->GetMasterPassword(), RTL_TEXTENCODING_UTF8);
        }
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  uno::Reference< uno::XInterface >());
    }

    sal_uInt8 aKey[RTL_DIGEST_LENGTH_MD5];
    rtl_digest_PBKDF2(aKey,
                      RTL_DIGEST_LENGTH_MD5,
                      reinterpret_cast< sal_uInt8 const * >(aMaster.getStr()),
                      aMaster.getLength(),
                      reinterpret_cast< sal_uInt8 const * >(
                          "3B5509ABA6BC42D9A3A1F3DAD49E56A51"),
                      32,
                      1000);

    rtl::OUStringBuffer aBuffer;
    for (int i = 0; i < RTL_DIGEST_LENGTH_MD5; ++i)
    {
        aBuffer.append(static_cast< sal_Unicode >('a' + (aKey[i] >> 4)));
        aBuffer.append(static_cast< sal_Unicode >('a' + (aKey[i] & 15)));
    }
    rInfo.SetPassword(aBuffer.makeStringAndClear());
}

void
handleMasterPasswordRequest_(
    Window * pParent,
    task::PasswordRequestMode nMode,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((uno::RuntimeException))
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
    case ERRCODE_BUTTON_OK:
        if (xSupplyAuthentication.is())
        {
            if (xSupplyAuthentication->canSetPassword())
                xSupplyAuthentication->setPassword(aInfo.GetPassword());
            xSupplyAuthentication->select();
        }
        break;

    case ERRCODE_BUTTON_RETRY:
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
    Window * pParent,
    LoginErrorInfo & rInfo,
    task::PasswordRequestMode nMode,
    ::rtl::OUString aDocName,
    bool bMSCryptoMode)
       SAL_THROW((uno::RuntimeException))
{
    try
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        std::auto_ptr< ResMgr > xManager(
            ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        if( nMode == task::PasswordRequestMode_PASSWORD_CREATE )
        {
            std::auto_ptr< PasswordCreateDialog > xDialog(
                new PasswordCreateDialog(pParent,
                                         xManager.get(),
                                         bMSCryptoMode));

            rInfo.SetResult(xDialog->Execute()
                == RET_OK ? ERRCODE_BUTTON_OK : ERRCODE_BUTTON_CANCEL);
            rInfo.SetPassword( xDialog->GetPassword() );
        }
        else
        {
            std::auto_ptr< PasswordDialog > xDialog(
                new PasswordDialog(pParent, nMode, xManager.get(), aDocName));

            rInfo.SetResult(xDialog->Execute()
                == RET_OK ? ERRCODE_BUTTON_OK : ERRCODE_BUTTON_CANCEL);
            rInfo.SetPassword( xDialog->GetPassword() );
        }
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
            uno::Reference< uno::XInterface>());
    }
}

void
handlePasswordRequest_(
    Window * pParent,
    task::PasswordRequestMode nMode,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    ::rtl::OUString aDocumentName,
    bool bMSCryptoMode )
    SAL_THROW((uno::RuntimeException))
{
    uno::Reference< task::XInteractionRetry > xRetry;
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< task::XInteractionPassword > xPassword;
    getContinuations(rContinuations, &xRetry, &xAbort, &xPassword);
    LoginErrorInfo aInfo;

    executePasswordDialog(pParent,
                          aInfo,
                          nMode,
                          aDocumentName,
                          bMSCryptoMode);

    switch (aInfo.GetResult())
    {
    case ERRCODE_BUTTON_OK:
        if (xPassword.is())
        {
            xPassword->setPassword(aInfo.GetPassword());
            xPassword->select();
        }
        break;

    case ERRCODE_BUTTON_RETRY:
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
    SAL_THROW((uno::RuntimeException))
{
    uno::Any aAnyRequest(rRequest->getRequest());

    ucb::URLAuthenticationRequest aURLAuthenticationRequest;
    if (aAnyRequest >>= aURLAuthenticationRequest)
    {
        handleAuthenticationRequest_(getParentProperty(),
                                     getInteractionHandler(),
                                     m_xServiceFactory,
                                     aURLAuthenticationRequest,
                                     rRequest->getContinuations(),
                                     aURLAuthenticationRequest.URL);
        return true;
    }

    ucb::AuthenticationRequest aAuthenticationRequest;
    if (aAnyRequest >>= aAuthenticationRequest)
    {
        handleAuthenticationRequest_(getParentProperty(),
                                     getInteractionHandler(),
                                     m_xServiceFactory,
                                     aAuthenticationRequest,
                                     rRequest->getContinuations(),
                                     rtl::OUString());
        return true;
    }
    return false;
}

bool
UUIInteractionHelper::handleMasterPasswordRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    SAL_THROW((uno::RuntimeException))
{
    uno::Any aAnyRequest(rRequest->getRequest());

    task::MasterPasswordRequest aMasterPasswordRequest;
    if (aAnyRequest >>= aMasterPasswordRequest)
    {
        handleMasterPasswordRequest_(getParentProperty(),
                                     aMasterPasswordRequest.Mode,
                                     rRequest->getContinuations());
        return true;
    }
    return false;
}

bool
UUIInteractionHelper::handlePasswordRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    SAL_THROW((uno::RuntimeException))
{
    // parameters to be filled for the call to handlePasswordRequest_
    Window * pParent = getParentProperty();
    task::PasswordRequestMode nMode = task::PasswordRequestMode_PASSWORD_ENTER;
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const & rContinuations = rRequest->getContinuations();
    ::rtl::OUString aDocumentName;
    bool bMSCryptoMode          = false;
    bool bIsPasswordToModify    = false;

    bool bDoHandleRequest = false;

    uno::Any aAnyRequest(rRequest->getRequest());

    task::DocumentPasswordRequest aDocumentPasswordRequest;
    if (!bDoHandleRequest && (aAnyRequest >>= aDocumentPasswordRequest))
    {
        nMode               = aDocumentPasswordRequest.Mode;
        aDocumentName       = aDocumentPasswordRequest.Name;
        OSL_ENSURE( bMSCryptoMode == false, "bMSCryptoMode should be false" );
        OSL_ENSURE( bIsPasswordToModify == false, "bIsPasswordToModify should be false" );

        bDoHandleRequest = true;
    }

    task::DocumentMSPasswordRequest aDocumentMSPasswordRequest;
    if (!bDoHandleRequest && (aAnyRequest >>= aDocumentMSPasswordRequest))
    {
        nMode               = aDocumentMSPasswordRequest.Mode;
        aDocumentName       = aDocumentMSPasswordRequest.Name;
        bMSCryptoMode       = true;
        OSL_ENSURE( bIsPasswordToModify == false, "bIsPasswordToModify should be false" );

        bDoHandleRequest = true;
    }

    if (bDoHandleRequest)
    {
        handlePasswordRequest_( pParent, nMode, rContinuations, aDocumentName, bMSCryptoMode );
        return true;
    }

    return false;
}

