/*************************************************************************
 *
 *  $RCSfile: iahndl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sb $ $Date: 2000-11-10 10:13:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif
#ifndef _SVTOOLS_HRC
#include <svtools/svtools.hrc>
#endif

#ifndef _COM_SUN_STAR_TASK_CLASSIFIEDINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/ClassifiedInteractionRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONAPPROVE_HPP_
#include <com/sun/star/task/XInteractionApprove.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONDISAPPROVE_HPP_
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONRETRY_HPP_
#include <com/sun/star/task/XInteractionRetry.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_AUTHENTICATIONREQUEST_HPP_
#include <com/sun/star/ucb/AuthenticationRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_HANDLECOOKIESREQUEST_HPP_
#include <com/sun/star/ucb/HandleCookiesRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEBADTRANSFERURLEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVECHAOSEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveCHAOSException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEFILEIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveFileIOException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKCONNECTEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveNetworkConnectException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveNetworkException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKGENERALEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveNetworkGeneralException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKOFFLINEEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveNetworkOffLineException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKREADEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveNetworkReadException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKRESOLVENAMEEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveNetworkResolveNameException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKWRITEEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveNetworkWriteException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEWRONGMEDIUMEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveWrongMediumException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XINTERACTIONCOOKIEHANDLING_HPP_
#include <com/sun/star/ucb/XInteractionCookieHandling.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XINTERACTIONSUPPLYAUTHENTICATION_HPP_
#include <com/sun/star/ucb/XInteractionSupplyAuthentication.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef SVTOOLS_HTTPCOOK_HXX
#include <svtools/httpcook.hxx>
#endif
#ifndef _LOGINERR_HXX
#include <svtools/loginerr.hxx>
#endif
#ifndef _EINF_HXX
#include <tools/errinf.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef UUI_COOKIEDG_HXX
#include <cookiedg.hxx>
#endif
#ifndef UUI_IAHNDL_HXX
#include <iahndl.hxx>
#endif
#ifndef UUI_IDS_HRC
#include <ids.hrc>
#endif
#ifndef UUI_LOGINDLG_HXX
#include <logindlg.hxx>
#endif

using namespace com::sun::star;

namespace unnamed_uui_iahndl {} using namespace unnamed_uui_iahndl;
    // unnamed namespaces don't work well yet...

//============================================================================
namespace unnamed_uui_iahndl {

class SimpleErrorContext: public ErrorContext
{
    UniString m_aContext;

public:
    SimpleErrorContext(UniString const & rTheContext): m_aContext(rTheContext)
    {}

    virtual BOOL GetString(ULONG, UniString & rCtxStr);
};

//============================================================================
USHORT executeErrorDialog(ULONG nID, USHORT nMask);

//============================================================================
void executeLoginDialog(LoginErrorInfo & rInfo);

//============================================================================
void executeCookieDialog(CntHTTPCookieRequest & rRequest);

}

//============================================================================
//
//  UUIInteractionHandler
//
//============================================================================

// static
sal_Char const UUIInteractionHandler::m_aImplementationName[]
    = "UUIInteractionHandler";

//============================================================================
// virtual
uno::Any SAL_CALL UUIInteractionHandler::queryInterface(uno::Type const &
                                                            rType)
    throw (uno::RuntimeException)
{
    uno::Any
        aRet(cppu::queryInterface(rType,
                                  static_cast< lang::XServiceInfo * >(this),
                                  static_cast< lang::XTypeProvider * >(this),
                                  static_cast< task::XInteractionHandler * >(
                                      this)));
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface(rType);
}

//============================================================================
// virtual
void SAL_CALL UUIInteractionHandler::acquire() throw (uno::RuntimeException)
{
    OWeakObject::acquire();
}

//============================================================================
// virtual
void SAL_CALL UUIInteractionHandler::release() throw (uno::RuntimeException)
{
    OWeakObject::release();
}

//============================================================================
// virtual
rtl::OUString SAL_CALL UUIInteractionHandler::getImplementationName()
    throw (uno::RuntimeException)
{
    return rtl::OUString::createFromAscii(m_aImplementationName);
}

//============================================================================
// virtual
sal_Bool SAL_CALL UUIInteractionHandler::supportsService(rtl::OUString const &
                                                             rServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > aNames(getSupportedServiceNames_static());
    for (sal_Int32 i = 0; i < aNames.getLength(); ++i)
        if (aNames[i] == rServiceName)
            return true;
    return false;
}

//============================================================================
// virtual
uno::Sequence< rtl::OUString > SAL_CALL
UUIInteractionHandler::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

//============================================================================
// virtual
uno::Sequence< uno::Type > SAL_CALL UUIInteractionHandler::getTypes()
    throw (uno::RuntimeException)
{
    static cppu::OTypeCollection * pCollection = 0;
    if (!pCollection)
    {
        osl::Guard< osl::Mutex > aGuard(osl::Mutex::getGlobalMutex());
        if (!pCollection)
        {
            static cppu::OTypeCollection
                aTheCollection(
                    getCppuType(
                        static_cast< uno::Reference< lang::XServiceInfo >
                                         const * >(
                            0)),
                    getCppuType(
                        static_cast< uno::Reference< lang::XTypeProvider >
                                         const * >(
                            0)),
                    getCppuType(
                        static_cast<
                                uno::Reference< task::XInteractionHandler >
                                    const * >(
                            0)));
            pCollection = &aTheCollection;
        }
    }
    return pCollection->getTypes();
}

//============================================================================
// virtual
uno::Sequence< sal_Int8 > SAL_CALL
UUIInteractionHandler::getImplementationId() throw (uno::RuntimeException)
{
    static cppu::OImplementationId * pID = 0;
    if (!pID)
    {
        osl::Guard< osl::Mutex > aGuard(osl::Mutex::getGlobalMutex());
        if (!pID)
        {
            static cppu::OImplementationId aTheID(false);
            pID = &aTheID;
        }
    }
    return pID->getImplementationId();
}

//============================================================================
// virtual
void SAL_CALL
UUIInteractionHandler::handle(
        uno::Reference< task::XInteractionRequest > const & rRequest)
    throw (uno::RuntimeException)
{
    if (!rRequest.is())
        return;

    uno::Any aTheRequest(rRequest->getRequest());
    uno::Sequence< uno::Reference< task::XInteractionContinuation > >
        aContinuations(rRequest->getContinuations());

    ucb::AuthenticationRequest aAuthenticationRequest;
    if (aTheRequest >>= aAuthenticationRequest)
    {
        uno::Reference< task::XInteractionAbort > xAbort;
        uno::Reference< task::XInteractionRetry > xRetry;
        uno::Reference< ucb::XInteractionSupplyAuthentication >
            xSupplyAuthentication;
        for (sal_Int32 i = 0; i < aContinuations.getLength(); ++i)
        {
            if (!xAbort.is())
            {
                xAbort = uno::Reference< task::XInteractionAbort >(
                             aContinuations[i], uno::UNO_QUERY);
                if (xAbort.is())
                    continue;
            }
            if (!xRetry.is())
            {
                xRetry = uno::Reference< task::XInteractionRetry >(
                             aContinuations[i], uno::UNO_QUERY);
                if (xRetry.is())
                    continue;
            }
            if (!xSupplyAuthentication.is())
            {
                xSupplyAuthentication
                    = uno::Reference< ucb::XInteractionSupplyAuthentication >(
                          aContinuations[i], uno::UNO_QUERY);
                if (xSupplyAuthentication.is())
                    continue;
            }
        }
        bool bRemember;
        bool bRememberPersistent;
        if (xSupplyAuthentication.is())
        {
            ucb::RememberAuthentication eDefault;
            uno::Sequence< ucb::RememberAuthentication >
                aModes(xSupplyAuthentication->
                           getRememberPasswordModes(eDefault));
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
        LoginErrorInfo aInfo;
        aInfo.SetTitle(aAuthenticationRequest.ServerName);
        aInfo.SetServer(aAuthenticationRequest.ServerName);
        DBG_ASSERT(!(aAuthenticationRequest.HasRealm
                     && aAuthenticationRequest.HasAccount),
                   "UUIInteractionHandler::handle():"
                       " AuthenticationRequest with both Realm and Account");
        if (aAuthenticationRequest.HasRealm)
            aInfo.SetAccount(aAuthenticationRequest.Realm);
        else if (aAuthenticationRequest.HasAccount)
            aInfo.SetAccount(aAuthenticationRequest.Account);
        if (aAuthenticationRequest.HasUserName)
            aInfo.SetUserName(aAuthenticationRequest.UserName);
        if (aAuthenticationRequest.HasPassword)
            aInfo.SetPassword(aAuthenticationRequest.Password);
        aInfo.SetErrorText(aAuthenticationRequest.Diagnostic);
        aInfo.SetPersistentPassword(bRememberPersistent);
        aInfo.SetSavePassword(bRemember);
        aInfo.SetModifyAccount(aAuthenticationRequest.HasRealm
                               || aAuthenticationRequest.HasAccount);
        aInfo.SetModifyUserName(aAuthenticationRequest.HasUserName
                                && xSupplyAuthentication.is()
                                && xSupplyAuthentication->canSetUserName());
        executeLoginDialog(aInfo);
        switch (aInfo.GetResult())
        {
            case ERRCODE_BUTTON_OK:
                if (xSupplyAuthentication.is())
                {
                    xSupplyAuthentication->setUserName(aInfo.GetUserName());
                    xSupplyAuthentication->setPassword(aInfo.GetPassword());
                    xSupplyAuthentication->
                        setRememberPassword(
                            aInfo.GetIsSavePassword() ?
                                bRememberPersistent ?
                                    ucb::RememberAuthentication_PERSISTENT :
                                    ucb::RememberAuthentication_SESSION :
                                ucb::RememberAuthentication_NO);
                    if (aAuthenticationRequest.HasRealm)
                        xSupplyAuthentication->setRealm(aInfo.GetAccount());
                    else
                        xSupplyAuthentication->setAccount(aInfo.GetAccount());
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
        return;
    }

    ucb::HandleCookiesRequest aCookiesRequest;
    if (aTheRequest >>= aCookiesRequest)
    {
        List aCookies;
        {for (sal_Int32 i = 0; i < aCookiesRequest.Cookies.getLength(); ++i)
        {
            CntHTTPCookie * pCookie = new CntHTTPCookie;
            pCookie->m_aName = UniString(aCookiesRequest.Cookies[i].Name);
            pCookie->m_aValue = UniString(aCookiesRequest.Cookies[i].Value);
            pCookie->m_aDomain = UniString(aCookiesRequest.Cookies[i].Domain);
            pCookie->m_aPath = UniString(aCookiesRequest.Cookies[i].Path);
            pCookie->m_aExpires
                = DateTime(Date(aCookiesRequest.Cookies[i].Expires.Day,
                                aCookiesRequest.Cookies[i].Expires.Month,
                                aCookiesRequest.Cookies[i].Expires.Year),
                           Time(aCookiesRequest.Cookies[i].Expires.Hours,
                                aCookiesRequest.Cookies[i].Expires.Minutes,
                                aCookiesRequest.Cookies[i].Expires.Seconds,
                                aCookiesRequest.Cookies[i].Expires.
                                    HundredthSeconds));
            pCookie->m_nFlags = aCookiesRequest.Cookies[i].Secure ?
                                    CNTHTTP_COOKIE_FLAG_SECURE : 0;
            switch (aCookiesRequest.Cookies[i].Policy)
            {
                case ucb::CookiePolicy_CONFIRM:
                    pCookie->m_nPolicy = CNTHTTP_COOKIE_POLICY_INTERACTIVE;
                    break;

                case ucb::CookiePolicy_ACCEPT:
                    pCookie->m_nPolicy = CNTHTTP_COOKIE_POLICY_ACCEPTED;
                    break;

                case ucb::CookiePolicy_IGNORE:
                    pCookie->m_nPolicy = CNTHTTP_COOKIE_POLICY_BANNED;
                    break;
            }
            aCookies.Insert(pCookie, LIST_APPEND);
        }}
        CntHTTPCookieRequest aRequest(aCookiesRequest.URL,
                                      aCookies,
                                      aCookiesRequest.Request
                                              == ucb::CookieRequest_RECEIVE ?
                                          CNTHTTP_COOKIE_REQUEST_RECV :
                                          CNTHTTP_COOKIE_REQUEST_SEND);
        executeCookieDialog(aRequest);
        for (sal_Int32 i = 0; i < aContinuations.getLength(); ++i)
        {
            uno::Reference< ucb::XInteractionCookieHandling >
                xCookieHandling(aContinuations[i], uno::UNO_QUERY);
            if (xCookieHandling.is())
            {
                switch (aRequest.m_nRet)
                {
                    case CNTHTTP_COOKIE_POLICY_INTERACTIVE:
                        xCookieHandling->
                            setGeneralPolicy(ucb::CookiePolicy_CONFIRM);
                        break;

                    case CNTHTTP_COOKIE_POLICY_ACCEPTED:
                        xCookieHandling->
                            setGeneralPolicy(ucb::CookiePolicy_ACCEPT);
                        break;

                    case CNTHTTP_COOKIE_POLICY_BANNED:
                        xCookieHandling->
                            setGeneralPolicy(ucb::CookiePolicy_IGNORE);
                        break;
                }
                for (sal_Int32 j = 0; j < aCookiesRequest.Cookies.getLength();
                     ++j)
                    if (aCookiesRequest.Cookies[j].Policy
                            == ucb::CookiePolicy_CONFIRM)
                        switch (static_cast< CntHTTPCookie * >(
                                        aCookies.GetObject(j))->
                                    m_nPolicy)
                        {
                            case CNTHTTP_COOKIE_POLICY_ACCEPTED:
                                xCookieHandling->
                                    setSpecificPolicy(aCookiesRequest.
                                                          Cookies[j],
                                                      true);
                                break;

                            case CNTHTTP_COOKIE_POLICY_BANNED:
                                xCookieHandling->
                                    setSpecificPolicy(aCookiesRequest.
                                                          Cookies[j],
                                                      false);
                                break;
                        }
                xCookieHandling->select();
                break;
            }
        }
        while (aCookies.Count() != 0)
            delete static_cast< CntHTTPCookie * >(
                       aCookies.Remove(aCookies.Count() - 1));
        return;
    }

    task::InteractionClassification eClassification
        = task::InteractionClassification_ERROR;
    task::ClassifiedInteractionRequest aClassifiedInteractionRequest;
    if (aTheRequest >>= aClassifiedInteractionRequest)
        eClassification = aClassifiedInteractionRequest.Classification;

    sal_uInt32 nErrorID = ERRCODE_NONE;
    bool bErrorFlags = false;
    SimpleErrorContext * pContext = 0;
    bool bExecuted = false;
    USHORT nButton;
    ucb::InteractiveIOException aIOException;
    ucb::InteractiveNetworkException aNetworkException;
    ucb::InteractiveCHAOSException aCHAOSException;
    ucb::InteractiveBadTransferURLException aTransferException;
    ucb::InteractiveWrongMediumException aWrongMediumException;
    if (aTheRequest >>= aIOException)
    {
        static sal_uInt32 const aID[ucb::IOErrorCode_WRONG_VERSION + 1]
            = { ERRCODE_IO_ABORT, // ABORT
                ERRCODE_IO_ACCESSDENIED, // ACCESS_DENIED
                ERRCODE_IO_ALREADYEXISTS, // ALREADY_EXISTING
                ERRCODE_IO_BADCRC, // BAD_CRC
                ERRCODE_IO_CANTCREATE, // CANT_CREATE
                ERRCODE_IO_CANTREAD, // CANT_READ
                ERRCODE_IO_CANTSEEK, // CANT_SEEK
                ERRCODE_IO_CANTTELL, // CANT_TELL
                ERRCODE_IO_CANTWRITE, // CANT_WRITE
                ERRCODE_IO_CURRENTDIR, // CURRENT_DIRECTORY
                ERRCODE_IO_DEVICENOTREADY, // DEVICE_NOT_READY
                ERRCODE_IO_NOTSAMEDEVICE, // DIFFERENT_DEVICES
                ERRCODE_IO_GENERAL, // GENERAL
                ERRCODE_IO_INVALIDACCESS, // INVALID_ACCESS
                ERRCODE_IO_INVALIDCHAR, // INVALID_CHARACTER
                ERRCODE_IO_INVALIDDEVICE, // INVALID_DEVICE
                ERRCODE_IO_INVALIDLENGTH, // INVALID_LENGTH
                ERRCODE_IO_INVALIDPARAMETER, // INVALID_PARAMETER
                ERRCODE_IO_ISWILDCARD, // IS_WILDCARD
                ERRCODE_IO_LOCKVIOLATION, // LOCKING_VIOLATION
                ERRCODE_IO_MISPLACEDCHAR, // MISPLACED_CHARACTER
                ERRCODE_IO_NAMETOOLONG, // NAME_TOO_LONG
                ERRCODE_IO_NOTEXISTS, // NOT_EXISTING
                ERRCODE_IO_NOTEXISTSPATH, // NOT_EXISTING_PATH
                ERRCODE_IO_NOTSUPPORTED, // NOT_SUPPORTED
                ERRCODE_IO_NOTADIRECTORY, // NO_DIRECTORY
                ERRCODE_IO_NOTAFILE, // NO_FILE
                ERRCODE_IO_OUTOFSPACE, // OUT_OF_DISK_SPACE
                ERRCODE_IO_TOOMANYOPENFILES, // OUT_OF_FILE_HANDLES
                ERRCODE_IO_OUTOFMEMORY, // OUT_OF_MEMORY
                ERRCODE_IO_PENDING, // PENDING
                ERRCODE_IO_RECURSIVE, // RECURSIVE
                ERRCODE_IO_UNKNOWN, // UNKNOWN
                ERRCODE_IO_WRITEPROTECTED, // WRITE_PROTECTED
                ERRCODE_IO_WRONGFORMAT, // WRONG_FORMAT
                ERRCODE_IO_WRONGVERSION }; // WRONG_VERSION
        nErrorID = aID[aIOException.Code];
        bErrorFlags = true;

        ucb::InteractiveFileIOException aFileIOException;
        if (aTheRequest >>= aFileIOException)
        {
            vos::OGuard aGuard(Application::GetSolarMutex());

            ResMgr * pManager
                = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui));
            UniString aTheContext(ResId(STR_ERROR_FILEIO, pManager));
            delete pManager;

            aTheContext.SearchAndReplaceAscii("($URL1)",
                                              aFileIOException.FileName);
            pContext = new SimpleErrorContext(aTheContext);
        }
    }
    else if (aTheRequest >>= aNetworkException)
    {
        ucb::InteractiveNetworkOffLineException aOffLineException;
        ucb::InteractiveNetworkResolveNameException aResolveNameException;
        ucb::InteractiveNetworkConnectException aConnectException;
        ucb::InteractiveNetworkReadException aReadException;
        ucb::InteractiveNetworkWriteException aWriteException;
        nErrorID = (aTheRequest >>= aOffLineException) ?
                       ERRCODE_INET_OFFLINE :
                   (aTheRequest >>= aResolveNameException) ?
                       *new StringErrorInfo(ERRCODE_INET_NAME_RESOLVE,
                                            aResolveNameException.Server) :
                   (aTheRequest >>= aConnectException) ?
                       *new StringErrorInfo(ERRCODE_INET_CONNECT,
                                            aConnectException.Server) :
                   (aTheRequest >>= aReadException) ?
                       *new StringErrorInfo(ERRCODE_INET_READ,
                                            aReadException.Diagnostic) :
                   (aTheRequest >>= aWriteException) ?
                       *new StringErrorInfo(ERRCODE_INET_WRITE,
                                            aWriteException.Diagnostic) :
                       ERRCODE_INET_GENERAL;
    }
    else if (aTheRequest >>= aCHAOSException)
        nErrorID
            = aCHAOSException.Arguments.getLength() >= 2 ?
                  *new TwoStringErrorInfo(aCHAOSException.ID,
                                          aCHAOSException.Arguments[0],
                                          aCHAOSException.Arguments[1]) :
              aCHAOSException.Arguments.getLength() == 1 ?
                  *new StringErrorInfo(aCHAOSException.ID,
                                       aCHAOSException.Arguments[0]) :
                  aCHAOSException.ID;
    else if (aTheRequest >>= aTransferException)
    {
        DBG_ERROR("UUIInteractionHandler::handle():"
                      " Can't handle TransferException");
        nButton = ERRCODE_BUTTON_CANCEL;
        bExecuted = true;
    }
    else if (aTheRequest >>= aWrongMediumException)
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        ResMgr * pManager
            = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui));
        UniString aText(ResId(STR_ERROR_WRONGMEDIUM, pManager));
        delete pManager;

        sal_Int32 nMedium;
        aWrongMediumException.Medium >>= nMedium;
        aText.SearchAndReplaceAscii("$$",
                                    UniString::CreateFromInt32(nMedium + 1));

        ErrorBox aErrorBox(0, WB_OK_CANCEL, aText);
        nButton = aErrorBox.Execute();
        bExecuted = true;
    }

    sal_uInt16 nFlags = 0;
    if (bErrorFlags)
    {
        bool bCanApprove = false;
        for (sal_Int32 i = 0; i < aContinuations.getLength(); ++i)
        {
            if (uno::Reference< task::XInteractionApprove >::query(
                        aContinuations[i]).
                    is())
                bCanApprove = true;
            else if (uno::Reference< task::XInteractionDisapprove >::query(
                             aContinuations[i]).
                         is())
                nFlags |= ERRCODE_BUTTON_NO;
            else if (uno::Reference< task::XInteractionAbort >::query(
                             aContinuations[i]).
                         is())
                nFlags |= ERRCODE_BUTTON_CANCEL;
            else if (uno::Reference< task::XInteractionRetry >::query(
                             aContinuations[i]).
                         is())
                nFlags |= ERRCODE_BUTTON_RETRY;
        }
        if (bCanApprove)
            nFlags |= nFlags & ERRCODE_BUTTON_NO ? ERRCODE_BUTTON_YES :
                                                   ERRCODE_BUTTON_NO;
        switch (eClassification)
        {
            case task::InteractionClassification_ERROR:
                nFlags |= ERRCODE_MSG_ERROR;
                break;

            case task::InteractionClassification_WARNING:
                nFlags |= ERRCODE_MSG_WARNING;
                break;

            case task::InteractionClassification_INFO:
                nFlags |= ERRCODE_MSG_INFO;
                break;

            case task::InteractionClassification_QUERY:
                nFlags |= ERRCODE_MSG_QUERY;
                break;
        }
    }

    if (!bExecuted)
        nButton
            = executeErrorDialog(nErrorID, bErrorFlags ? nFlags : USHRT_MAX);

    if (pContext)
    {
        vos::OGuard aGuard(Application::GetSolarMutex());
        delete pContext;
    }

    switch (nButton)
    {
        case ERRCODE_BUTTON_OK:
        case ERRCODE_BUTTON_YES:
            {for (sal_Int32 i = 0; i < aContinuations.getLength(); ++i)
            {
                uno::Reference< task::XInteractionApprove >
                    xApprove(aContinuations[i], uno::UNO_QUERY);
                if (xApprove.is())
                {
                    xApprove->select();
                    break;
                }
            }}
            break;

        case ERRCODE_BUTTON_CANCEL:
            {for (sal_Int32 i = 0; i < aContinuations.getLength(); ++i)
            {
                uno::Reference< task::XInteractionAbort >
                    xAbort(aContinuations[i], uno::UNO_QUERY);
                if (xAbort.is())
                {
                    xAbort->select();
                    break;
                }
            }}
            break;

        case ERRCODE_BUTTON_RETRY:
            {for (sal_Int32 i = 0; i < aContinuations.getLength(); ++i)
            {
                uno::Reference< task::XInteractionRetry >
                    xRetry(aContinuations[i], uno::UNO_QUERY);
                if (xRetry.is())
                {
                    xRetry->select();
                    break;
                }
            }}
            break;

        case ERRCODE_BUTTON_NO:
            {for (sal_Int32 i = 0; i < aContinuations.getLength(); ++i)
            {
                uno::Reference< task::XInteractionDisapprove >
                    xDisapprove(aContinuations[i], uno::UNO_QUERY);
                if (xDisapprove.is())
                {
                    xDisapprove->select();
                    break;
                }
            }}
            break;
    }
}

//============================================================================
// static
uno::Sequence< rtl::OUString >
UUIInteractionHandler::getSupportedServiceNames_static()
{
    uno::Sequence< rtl::OUString > aNames(1);
    aNames[0] = rtl::OUString::createFromAscii(
                    "com.sun.star.uui.InteractionHandler");
    return aNames;
}

//============================================================================
// static
uno::Reference< uno::XInterface >
#if SUPD >= 590
SAL_CALL
#endif // SUPD
UUIInteractionHandler::createInstance(
    uno::Reference< lang::XMultiServiceFactory > const &)
{
    return static_cast< uno::XWeak * >(static_cast< cppu::OWeakObject * >(
                                           new UUIInteractionHandler));
}

//============================================================================
//
//  SimpleErrorContext
//
//============================================================================

// virtual
BOOL SimpleErrorContext::GetString(ULONG, UniString & rCtxStr)
{
    rCtxStr = m_aContext;
    return true;
}

//============================================================================
//
//  executeErrorDialog
//
//============================================================================

namespace unnamed_uui_iahndl {

USHORT executeErrorDialog(ULONG nID, USHORT nMask)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    ResMgr * pManager1 = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(ofa));
    SfxErrorHandler * pHandler1
        = new SfxErrorHandler(RID_ERRHDL, ERRCODE_AREA_TOOLS,
                              ERRCODE_AREA_LIB1 - 1, pManager1);
    ResMgr * pManager2 = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(cnt));
    SfxErrorHandler * pHandler2
        = new SfxErrorHandler(RID_CHAOS_START + 12, ERRCODE_AREA_CHAOS,
                              ERRCODE_AREA_CHAOS_END, pManager2);
        // cf. chaos/source/inc/cntrids.hrc, where
        // #define RID_CHAOS_ERRHDL (RID_CHAOS_START + 12)

    // Needed because within ErrorHandler::HanldeError() ResIds are created
    // without a ResMgr---they require a default ResMgr:
    ResMgr * pDefaultManager = Resource::GetResManager();
    Resource::SetResManager(pManager1);

    USHORT nButton = ErrorHandler::HandleError(nID, nMask);

    Resource::SetResManager(pDefaultManager);

    delete pHandler2;
    delete pManager2;
    delete pHandler1;
    delete pManager1;
    return nButton;
}

//============================================================================
//
//  executeLoginDialog
//
//============================================================================

void executeLoginDialog(LoginErrorInfo & rInfo)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    bool bAccount = (rInfo.GetFlags() & LOGINERROR_FLAG_MODIFY_ACCOUNT) != 0;

    sal_uInt16 nFlags = 0;
    if (rInfo.GetPath().Len() == 0)
        nFlags |= LF_NO_PATH;
    if (rInfo.GetErrorText().Len() == 0)
        nFlags |= LF_NO_ERRORTEXT;
    if (!bAccount)
        nFlags |= LF_NO_ACCOUNT;
    if (!(rInfo.GetFlags() & LOGINERROR_FLAG_MODIFY_USER_NAME))
        nFlags |= LF_USERNAME_READONLY;

    ResMgr * pManager = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui));
    LoginDialog * pDialog = new LoginDialog(0, nFlags, rInfo.GetServer(),
                                            rInfo.GetAccount(), pManager);
    if (rInfo.GetErrorText().Len() != 0)
        pDialog->SetErrorText(rInfo.GetErrorText());
    pDialog->SetName(rInfo.GetUserName());
    if (bAccount)
        pDialog->ClearAccount();
    else
        pDialog->ClearPassword();
    pDialog->SetPassword(rInfo.GetPassword());
    pDialog->SetSavePasswordText(ResId(rInfo.GetIsPersistentPassword() ?
                                           RID_SAVE_PASSWORD :
                                           RID_KEEP_PASSWORD,
                                       pManager));
    pDialog->SetSavePassword(rInfo.GetIsSavePassword());

    rInfo.SetResult(pDialog->Execute() == RET_OK ? ERRCODE_BUTTON_OK :
                                                   ERRCODE_BUTTON_CANCEL);
    rInfo.SetUserName(pDialog->GetName());
    rInfo.SetPassword(pDialog->GetPassword());
    rInfo.SetAccount(pDialog->GetAccount());
    rInfo.SetSavePassword(pDialog->IsSavePassword());
    delete pDialog;
    delete pManager;
}

//============================================================================
//
//  executeCookieDialog
//
//============================================================================

void executeCookieDialog(CntHTTPCookieRequest & rRequest)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    ResMgr * pManager = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui));
    CookiesDialog * pDialog = new CookiesDialog(0, &rRequest, pManager);
    pDialog->Execute();
    delete pDialog;
    delete pManager;
}

}
