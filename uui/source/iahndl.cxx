/*************************************************************************
 *
 *  $RCSfile: iahndl.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 14:33:51 $
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

#ifndef UUI_IAHNDL_HXX
#include "iahndl.hxx"
#endif

#ifndef UUI_COOKIEDG_HXX
#include "cookiedg.hxx"
#endif
#ifndef UUI_IDS_HRC
#include "ids.hrc"
#endif
#ifndef UUI_LOGINDLG_HXX
#include "logindlg.hxx"
#endif
#ifndef UUI_MASTERPASSWORDDLG_HXX
#include "masterpassworddlg.hxx"
#endif
#ifndef UUI_PASSCRTDLG_HXX
#include "masterpasscrtdlg.hxx"
#endif
#ifndef UUI_PASSWORDDLG_HXX
#include "passworddlg.hxx"
#endif
#ifndef UUI_PASSCRTDLG_HXX
#include "passcrtdlg.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include "com/sun/star/awt/XWindow.hpp"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include "com/sun/star/beans/PropertyValue.hpp"
#endif
#ifndef _COM_SUN_STAR_JAVA_WRONGJAVAVERSIONEXCEPTION_HPP_
#include "com/sun/star/java/WrongJavaVersionException.hpp"
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#endif
#ifndef _COM_SUN_STAR_SYNC2_BADPARTNERSHIPEXCEPTION_HPP_
#include "com/sun/star/sync2/BadPartnershipException.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_INTERACTIONCLASSIFICATION_HPP_
#include "com/sun/star/task/InteractionClassification.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_NOMASTEREXCEPTION_HPP_
#include "com/sun/star/task/NoMasterException.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_PASSWORDREQUEST_HPP_
#include "com/sun/star/task/PasswordRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_MASTERPASSWORDREQUEST_HPP_
#include "com/sun/star/task/MasterPasswordRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_DOCUMENTPASSWORDREQUEST_HPP_
#include "com/sun/star/task/DocumentPasswordRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include "com/sun/star/task/XInteractionAbort.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONAPPROVE_HPP_
#include "com/sun/star/task/XInteractionApprove.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONCONTINUATION_HPP_
#include "com/sun/star/task/XInteractionContinuation.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONDISAPPROVE_HPP_
#include "com/sun/star/task/XInteractionDisapprove.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONRETRY_HPP_
#include "com/sun/star/task/XInteractionRetry.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_XPASSWORDCONTAINER_HPP_
#include "com/sun/star/task/XPasswordContainer.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDNAMECLASHEXCEPTION_HPP_
#include "com/sun/star/ucb/UnsupportedNameClashException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_AUTHENTICATIONREQUEST_HPP_
#include "com/sun/star/ucb/AuthenticationRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_HANDLECOOKIESREQUEST_HPP_
#include "com/sun/star/ucb/HandleCookiesRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAUGMENTEDIOEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveAugmentedIOException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVECHAOSEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveCHAOSException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKCONNECTEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveNetworkConnectException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveNetworkException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKGENERALEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveNetworkGeneralException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKOFFLINEEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveNetworkOffLineException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKREADEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveNetworkReadException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKRESOLVENAMEEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveNetworkResolveNameException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKWRITEEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveNetworkWriteException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEWRONGMEDIUMEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveWrongMediumException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAPPEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveAppException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASHEXCEPTION_HPP_
#include "com/sun/star/ucb/NameClashException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_XINTERACTIONCOOKIEHANDLING_HPP_
#include "com/sun/star/ucb/XInteractionCookieHandling.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_XINTERACTIONSUPPLYAUTHENTICATION_HPP_
#include "com/sun/star/ucb/XInteractionSupplyAuthentication.hpp"
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XINTERACTIONFILTERSELECT_HPP_
#include "com/sun/star/document/XInteractionFilterSelect.hpp"
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XINTERACTIONFILTEROPTIONS_HPP_
#include "com/sun/star/document/XInteractionFilterOptions.hpp"
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_BROKENPACKAGEREQUEST_HPP_
#include "com/sun/star/document/BrokenPackageRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONPASSWORD_HPP_
#include "com/sun/star/task/XInteractionPassword.hpp"
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_FILTEROPTIONSREQUEST_HPP_
#include "com/sun/star/document/FilterOptionsRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include "com/sun/star/container/XNameContainer.hpp"
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include "com/sun/star/uno/Any.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include "com/sun/star/uno/Exception.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_RuntimeEXCEPTION_HPP_
#include "com/sun/star/uno/RuntimeException.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include "com/sun/star/uno/XInterface.hpp"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif
#ifndef _OSL_MUTEX_HXX_
#include "osl/mutex.hxx"
#endif
#ifndef _RTL_DIGEST_H
#include "rtl/digest.h"
#endif
#ifndef _RTL_STRING_H_
#include "rtl/string.h"
#endif
#ifndef _RTL_STRING_HXX_
#include "rtl/string.hxx"
#endif
#ifndef _RTL_TEXTENC_H
#include "rtl/textenc.h"
#endif
#ifndef _RTL_USTRBUF_HXX_
#include "rtl/ustrbuf.hxx"
#endif
#ifndef _RTL_USTRING_H_
#include "rtl/ustring.h"
#endif
#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef SVTOOLS_HTTPCOOK_HXX
#include "svtools/httpcook.hxx"
#endif
#ifndef _LOGINERR_HXX
#include "svtools/loginerr.hxx"
#endif
#ifndef _SVTOOLS_HRC
#include "svtools/svtools.hrc"
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include "toolkit/helper/vclunohelper.hxx"
#endif
#ifndef _LIST_HXX
#include "tools/list.hxx"
#endif
#ifndef _TOOLS_RC_HXX
#include "tools/rc.hxx"
#endif
#ifndef __RSC
#include "tools/errinf.hxx"
#endif
#ifndef _TOOLS_RCID_H
#include "tools/rcid.h"
#endif
#ifndef _TOOLS_RESID_HXX
#include "tools/resid.hxx"
#endif
#ifndef _SOLAR_H
#include "tools/solar.h"
#endif
#ifndef _STRING_HXX
#include "tools/string.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include "vcl/msgbox.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include "vcl/svapp.hxx"
#endif
#ifndef _SV_WINTYPES_HXX
#include "vcl/wintypes.hxx"
#endif
#ifndef _VOS_MUTEX_HXX_
#include "vos/mutex.hxx"
#endif
#ifndef _UNOTOOLS_CONFIGMGR_HXX
#include <unotools/configmgr.hxx>
#endif

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif
#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif
#ifndef INCLUDED_NEW
#include <new>
#define INCLUDED_NEW
#endif
#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#include <stdio.h>

using namespace com::sun;

namespace {

class CookieList: public List
{
public:
    ~CookieList() SAL_THROW(());
};

CookieList::~CookieList() SAL_THROW(())
{
    while (Count() != 0)
        delete static_cast< CntHTTPCookie * >(Remove(Count() - 1));
}

class ErrorResource: private Resource
{
public:
    inline ErrorResource(ResId & rResId) SAL_THROW(()): Resource(rResId) {}

    inline ~ErrorResource() SAL_THROW(()) { FreeResource(); }

    bool getString(ErrCode nErrorCode, rtl::OUString * pString) const
        SAL_THROW(());
};

bool ErrorResource::getString(ErrCode nErrorCode, rtl::OUString * pString)
    const SAL_THROW(())
{
    OSL_ENSURE(pString, "specification violation");
    ResId aResId(static_cast< USHORT >(nErrorCode & ERRCODE_RES_MASK));
    aResId.SetRT(RSC_STRING);
    if (!IsAvailableRes(aResId))
        return false;
    aResId.SetAutoRelease(false);
    *pString = UniString(aResId);
    Resource::GetResManager()->PopContext();
    return true;
}

void
getContinuations(
    star::uno::Sequence< star::uno::Reference<
                             star::task::XInteractionContinuation > > const &
        rContinuations,
    star::uno::Reference< star::task::XInteractionApprove > * pApprove,
    star::uno::Reference< star::task::XInteractionDisapprove > * pDisapprove,
    star::uno::Reference< star::task::XInteractionRetry > * pRetry,
    star::uno::Reference< star::task::XInteractionAbort > * pAbort,
    star::uno::Reference< star::ucb::XInteractionSupplyAuthentication > *
        pSupplyAuthentication,
    star::uno::Reference< star::task::XInteractionPassword > * pPassword)
    SAL_THROW((star::uno::RuntimeException))
{
    for (sal_Int32 i = 0; i < rContinuations.getLength(); ++i)
    {
        if (pApprove && !pApprove->is())
        {
            *pApprove
                = star::uno::Reference< star::task::XInteractionApprove >(
                      rContinuations[i], star::uno::UNO_QUERY);
            if (pApprove->is())
                continue;
        }
        if (pDisapprove && !pDisapprove->is())
        {
            *pDisapprove
                = star::uno::Reference< star::task::XInteractionDisapprove >(
                      rContinuations[i], star::uno::UNO_QUERY);
            if (pDisapprove->is())
                continue;
        }
        if (pRetry && !pRetry->is())
        {
            *pRetry = star::uno::Reference< star::task::XInteractionRetry >(
                          rContinuations[i], star::uno::UNO_QUERY);
            if (pRetry->is())
                continue;
        }
        if (pAbort && !pAbort->is())
        {
            *pAbort = star::uno::Reference< star::task::XInteractionAbort >(
                          rContinuations[i], star::uno::UNO_QUERY);
            if (pAbort->is())
                continue;
        }
        if (pSupplyAuthentication && !pSupplyAuthentication->is())
        {
            *pSupplyAuthentication
                = star::uno::Reference<
                          star::ucb::XInteractionSupplyAuthentication >(
                      rContinuations[i], star::uno::UNO_QUERY);
            if (pSupplyAuthentication->is())
                continue;
        }
        if (pPassword && !pPassword->is())
        {
            *pPassword = star::uno::Reference< star::task::XInteractionPassword >(
                          rContinuations[i], star::uno::UNO_QUERY);
            if (pPassword->is())
                continue;
        }
    }
}

bool
getStringRequestArgument(star::uno::Sequence< star::uno::Any > const &
                             rArguments,
                         rtl::OUString const & rKey,
                         rtl::OUString * pValue)
    SAL_THROW(())
{
    for (sal_Int32 i = 0; i < rArguments.getLength(); ++i)
    {
        star::beans::PropertyValue aProperty;
        if ((rArguments[i] >>= aProperty) && aProperty.Name == rKey)
        {
            rtl::OUString aValue;
            if (aProperty.Value >>= aValue)
            {
                if (pValue)
                    *pValue = aValue;
                return true;
            }
        }
    }
    return false;
}

bool
getBoolRequestArgument(star::uno::Sequence< star::uno::Any > const &
                           rArguments,
                       rtl::OUString const & rKey,
                       bool * pValue)
    SAL_THROW(())
{
    for (sal_Int32 i = 0; i < rArguments.getLength(); ++i)
    {
        star::beans::PropertyValue aProperty;
        if ((rArguments[i] >>= aProperty) && aProperty.Name == rKey)
        {
            sal_Bool bValue;
            if (aProperty.Value >>= bValue)
            {
                if (pValue)
                    *pValue = bValue;
                return true;
            }
        }
    }
    return false;
}

bool
getResourceNameRequestArgument(star::uno::Sequence< star::uno::Any > const &
                                   rArguments,
                               rtl::OUString * pValue)
    SAL_THROW(())
{
    if (!getStringRequestArgument(rArguments,
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                      "Uri")),
                                  pValue))
        return false;
    // Use the resource name only for file URLs, to avoid confusion:
    //TODO! work with ucp locality concept instead of hardcoded "file"?
    if (pValue
        && pValue->matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(
                                                  "file:")))
        getStringRequestArgument(rArguments,
                                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "ResourceName")),
                                 pValue);
    return true;
}

}

UUIInteractionHandler::UUIInteractionHandler(
    star::uno::Reference< star::lang::XMultiServiceFactory > const &
        rServiceFactory)
    SAL_THROW(()):
    m_xServiceFactory(rServiceFactory)
{}

UUIInteractionHandler::~UUIInteractionHandler()
{}

rtl::OUString SAL_CALL UUIInteractionHandler::getImplementationName()
    throw (star::uno::RuntimeException)
{
    return rtl::OUString::createFromAscii(m_aImplementationName);
}

sal_Bool SAL_CALL
UUIInteractionHandler::supportsService(rtl::OUString const & rServiceName)
    throw (star::uno::RuntimeException)
{
    star::uno::Sequence< rtl::OUString >
        aNames(getSupportedServiceNames_static());
    for (sal_Int32 i = 0; i < aNames.getLength(); ++i)
        if (aNames[i] == rServiceName)
            return true;
    return false;
}

star::uno::Sequence< rtl::OUString > SAL_CALL
UUIInteractionHandler::getSupportedServiceNames()
    throw (star::uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

void SAL_CALL
UUIInteractionHandler::initialize(
    star::uno::Sequence< star::uno::Any > const & rArguments)
    throw (star::uno::Exception)
{
    osl::MutexGuard aGuard(m_aPropertyMutex);
    m_aProperties = rArguments;
}

void SAL_CALL
UUIInteractionHandler::handle(
    star::uno::Reference< star::task::XInteractionRequest > const & rRequest)
    throw (star::uno::RuntimeException)
{
    try
    {
        if (!rRequest.is())
            return;

        star::uno::Any aAnyRequest(rRequest->getRequest());

        star::task::ErrorCodeRequest aErrorCodeRequest;
        if (aAnyRequest >>= aErrorCodeRequest)
        {
            handleGenericErrorRequest(aErrorCodeRequest,
                                        rRequest->getContinuations());
            return;
        }

        star::ucb::AuthenticationRequest aAuthenticationRequest;
        if (aAnyRequest >>= aAuthenticationRequest)
        {
            handleAuthenticationRequest(aAuthenticationRequest,
                                        rRequest->getContinuations());
            return;
        }

        star::task::MasterPasswordRequest aMasterPasswordRequest;
        if (aAnyRequest >>= aMasterPasswordRequest)
        {
            handleMasterPasswordRequest(aMasterPasswordRequest.Mode,
                                  rRequest->getContinuations());
            return;
        }

        star::task::DocumentPasswordRequest aDocumentPasswordRequest;
        if (aAnyRequest >>= aDocumentPasswordRequest)
        {
            handlePasswordRequest(aDocumentPasswordRequest.Mode,
                                  rRequest->getContinuations(),
                                  aDocumentPasswordRequest.Name);
            return;
        }

        star::task::PasswordRequest aPasswordRequest;
        if (aAnyRequest >>= aPasswordRequest)
        {
            handlePasswordRequest(aPasswordRequest.Mode,
                                  rRequest->getContinuations());
            return;
        }

        star::ucb::HandleCookiesRequest aCookiesRequest;
        if (aAnyRequest >>= aCookiesRequest)
        {
            handleCookiesRequest(aCookiesRequest,
                                 rRequest->getContinuations());
            return;
        }

        star::document::NoSuchFilterRequest aNoSuchFilterRequest;
        if (aAnyRequest >>= aNoSuchFilterRequest)
        {
            handleNoSuchFilterRequest(aNoSuchFilterRequest,
                                      rRequest->getContinuations());
            return;
        }

        star::document::AmbigousFilterRequest aAmbigousFilterRequest;
        if (aAnyRequest >>= aAmbigousFilterRequest)
        {
            handleAmbigousFilterRequest(aAmbigousFilterRequest,
                                        rRequest->getContinuations());
            return;
        }

        star::document::FilterOptionsRequest aFilterOptionsRequest;
        if (aAnyRequest >>= aFilterOptionsRequest)
        {
            handleFilterOptionsRequest(aFilterOptionsRequest,
                                       rRequest->getContinuations());
            return;
        }

        star::ucb::NameClashException aNCException;
        if (aAnyRequest >>= aNCException)
        {
            ErrCode nErrorCode = ERRCODE_UUI_IO_TARGETALREADYEXISTS;
            std::vector< rtl::OUString > aArguments;

            if( aNCException.Name.getLength() )
            {
                nErrorCode = ERRCODE_UUI_IO_ALREADYEXISTS;
                aArguments.push_back( aNCException.Name );
            }

            handleErrorRequest( aNCException.Classification,
                                nErrorCode,
                                aArguments,
                                rRequest->getContinuations());

            return;
        }

        star::ucb::UnsupportedNameClashException aUORequest;
        if (aAnyRequest >>= aUORequest)
        {
            ErrCode nErrorCode = ERRCODE_UUI_IO_UNSUPPORTEDOVERWRITE;
            std::vector< rtl::OUString > aArguments;

            star::uno::Reference< star::task::XInteractionApprove > xApprove;
            star::uno::Reference< star::task::XInteractionDisapprove > xDisapprove;
            getContinuations(
                rRequest->getContinuations(), &xApprove, &xDisapprove, 0, 0, 0, 0);

            if( xApprove.is() && xDisapprove.is() )
            {
                handleErrorRequest( star::task::InteractionClassification_QUERY,
                                    nErrorCode,
                                    aArguments,
                                    rRequest->getContinuations());
            }

            return;
        }

        star::document::BrokenPackageRequest aBrokenPackageRequest;
        if (aAnyRequest >>= aBrokenPackageRequest)
        {
            std::vector< rtl::OUString > aArguments;

            if( aBrokenPackageRequest.aName.getLength() )
                aArguments.push_back( aBrokenPackageRequest.aName );

            handleBrokenPackageRequest( aArguments, rRequest->getContinuations() );

            return;
        }

        star::ucb::InteractiveIOException aIoException;
        if (aAnyRequest >>= aIoException)
        {
            star::uno::Sequence< star::uno::Any > aRequestArguments;
            star::ucb::InteractiveAugmentedIOException aAugmentedIoException;
            if (aAnyRequest >>= aAugmentedIoException)
                aRequestArguments = aAugmentedIoException.Arguments;

            ErrCode nErrorCode;
            std::vector< rtl::OUString > aArguments;
            static ErrCode const
                    aErrorCode[star::ucb::IOErrorCode_WRONG_VERSION + 1][2]
                = { { ERRCODE_IO_ABORT, ERRCODE_UUI_IO_ABORT }, // ABORT
                    { ERRCODE_IO_ACCESSDENIED, ERRCODE_UUI_IO_ACCESSDENIED },
                        // ACCESS_DENIED
                    { ERRCODE_IO_ALREADYEXISTS,
                      ERRCODE_UUI_IO_ALREADYEXISTS }, // ALREADY_EXISTING
                    { ERRCODE_IO_BADCRC, ERRCODE_UUI_IO_BADCRC }, // BAD_CRC
                    { ERRCODE_IO_CANTCREATE, ERRCODE_UUI_IO_CANTCREATE },
                        // CANT_CREATE
                    { ERRCODE_IO_CANTREAD, ERRCODE_UUI_IO_CANTREAD },
                        // CANT_READ
                    { ERRCODE_IO_CANTSEEK, ERRCODE_UUI_IO_CANTSEEK },
                        // CANT_SEEK
                    { ERRCODE_IO_CANTTELL, ERRCODE_UUI_IO_CANTTELL },
                        // CANT_TELL
                    { ERRCODE_IO_CANTWRITE, ERRCODE_UUI_IO_CANTWRITE },
                        // CANT_WRITE
                    { ERRCODE_IO_CURRENTDIR, ERRCODE_UUI_IO_CURRENTDIR },
                        // CURRENT_DIRECTORY
                    { ERRCODE_IO_DEVICENOTREADY, ERRCODE_UUI_IO_NOTREADY },
                        // DEVICE_NOT_READY
                    { ERRCODE_IO_NOTSAMEDEVICE,
                      ERRCODE_UUI_IO_NOTSAMEDEVICE }, // DIFFERENT_DEVICES
                    { ERRCODE_IO_GENERAL, ERRCODE_UUI_IO_GENERAL }, // GENERAL
                    { ERRCODE_IO_INVALIDACCESS,
                      ERRCODE_UUI_IO_INVALIDACCESS }, // INVALID_ACCESS
                    { ERRCODE_IO_INVALIDCHAR, ERRCODE_UUI_IO_INVALIDCHAR },
                        // INVALID_CHARACTER
                    { ERRCODE_IO_INVALIDDEVICE,
                      ERRCODE_UUI_IO_INVALIDDEVICE }, // INVALID_DEVICE
                    { ERRCODE_IO_INVALIDLENGTH,
                      ERRCODE_UUI_IO_INVALIDLENGTH }, // INVALID_LENGTH
                    { ERRCODE_IO_INVALIDPARAMETER,
                      ERRCODE_UUI_IO_INVALIDPARAMETER }, // INVALID_PARAMETER
                    { ERRCODE_IO_ISWILDCARD, ERRCODE_UUI_IO_ISWILDCARD },
                        // IS_WILDCARD
                    { ERRCODE_IO_LOCKVIOLATION,
                      ERRCODE_UUI_IO_LOCKVIOLATION }, // LOCKING_VIOLATION
                    { ERRCODE_IO_MISPLACEDCHAR,
                      ERRCODE_UUI_IO_MISPLACEDCHAR }, // MISPLACED_CHARACTER
                    { ERRCODE_IO_NAMETOOLONG, ERRCODE_UUI_IO_NAMETOOLONG },
                        // NAME_TOO_LONG
                    { ERRCODE_IO_NOTEXISTS, ERRCODE_UUI_IO_NOTEXISTS },
                        // NOT_EXISTING
                    { ERRCODE_IO_NOTEXISTSPATH,
                      ERRCODE_UUI_IO_NOTEXISTSPATH }, // NOT_EXISTING_PATH
                    { ERRCODE_IO_NOTSUPPORTED, ERRCODE_UUI_IO_NOTSUPPORTED },
                        // NOT_SUPPORTED
                    { ERRCODE_IO_NOTADIRECTORY,
                      ERRCODE_UUI_IO_NOTADIRECTORY }, // NO_DIRECTORY
                    { ERRCODE_IO_NOTAFILE, ERRCODE_UUI_IO_NOTAFILE },
                        // NO_FILE
                    { ERRCODE_IO_OUTOFSPACE, ERRCODE_UUI_IO_OUTOFSPACE },
                        // OUT_OF_DISK_SPACE
                    { ERRCODE_IO_TOOMANYOPENFILES,
                      ERRCODE_UUI_IO_TOOMANYOPENFILES },
                        // OUT_OF_FILE_HANDLES
                    { ERRCODE_IO_OUTOFMEMORY, ERRCODE_UUI_IO_OUTOFMEMORY },
                        // OUT_OF_MEMORY
                    { ERRCODE_IO_PENDING, ERRCODE_UUI_IO_PENDING }, // PENDING
                    { ERRCODE_IO_RECURSIVE, ERRCODE_UUI_IO_RECURSIVE },
                        // RECURSIVE
                    { ERRCODE_IO_UNKNOWN, ERRCODE_UUI_IO_UNKNOWN }, // UNKNOWN
                    { ERRCODE_IO_WRITEPROTECTED,
                      ERRCODE_UUI_IO_WRITEPROTECTED }, // WRITE_PROTECTED
                    { ERRCODE_IO_WRONGFORMAT, ERRCODE_UUI_IO_WRONGFORMAT },
                        // WRONG_FORMAT
                    { ERRCODE_IO_WRONGVERSION,
                      ERRCODE_UUI_IO_WRONGVERSION } }; // WRONG_VERSION
            switch (aIoException.Code)
            {
            case star::ucb::IOErrorCode_CANT_CREATE:
                {
                    rtl::OUString aArgFolder;
                    if (getStringRequestArgument(
                            aRequestArguments,
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                              "Folder")),
                            &aArgFolder))
                    {
                        rtl::OUString aArgUri;
                        if (getResourceNameRequestArgument(aRequestArguments,
                                                           &aArgUri))
                        {
                            nErrorCode = ERRCODE_UUI_IO_CANTCREATE;
                            aArguments.reserve(2);
                            aArguments.push_back(aArgUri);
                            aArguments.push_back(aArgFolder);
                        }
                        else
                        {
                            nErrorCode = ERRCODE_UUI_IO_CANTCREATE_NONAME;
                            aArguments.push_back(aArgFolder);
                        }
                    }
                    else
                        nErrorCode = aErrorCode[aIoException.Code][0];
                    break;
                }

            case star::ucb::IOErrorCode_DEVICE_NOT_READY:
                {
                    rtl::OUString aArgUri;
                    if (getResourceNameRequestArgument(aRequestArguments,
                                                       &aArgUri))
                    {
                        rtl::OUString aResourceType;
                        getStringRequestArgument(
                            aRequestArguments,
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                              "ResourceType")),
                            &aResourceType);
                        bool bRemovable = false;
                        getBoolRequestArgument(aRequestArguments,
                                               rtl::OUString(
                                                   RTL_CONSTASCII_USTRINGPARAM(
                                                       "Removable")),
                                               &bRemovable);
                        nErrorCode
                            = aResourceType.equalsAsciiL(
                                RTL_CONSTASCII_STRINGPARAM("volume"))
                            ? (bRemovable
                               ? ERRCODE_UUI_IO_NOTREADY_VOLUME_REMOVABLE
                               : ERRCODE_UUI_IO_NOTREADY_VOLUME)
                            : (bRemovable
                               ? ERRCODE_UUI_IO_NOTREADY_REMOVABLE
                               : ERRCODE_UUI_IO_NOTREADY);
                        aArguments.push_back(aArgUri);
                    }
                    else
                        nErrorCode = aErrorCode[aIoException.Code][0];
                    break;
                }

            case star::ucb::IOErrorCode_DIFFERENT_DEVICES:
                {
                    rtl::OUString aArgVolume;
                    rtl::OUString aArgOtherVolume;
                    if (getStringRequestArgument(
                            aRequestArguments,
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                              "Volume")),
                            &aArgVolume)
                        && getStringRequestArgument(
                            aRequestArguments,
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                              "OtherVolume")),
                            &aArgOtherVolume))
                    {
                        nErrorCode = aErrorCode[aIoException.Code][1];
                        aArguments.reserve(2);
                        aArguments.push_back(aArgVolume);
                        aArguments.push_back(aArgOtherVolume);
                    }
                    else
                        nErrorCode = aErrorCode[aIoException.Code][0];
                    break;
                }

            case star::ucb::IOErrorCode_NOT_EXISTING:
                {
                    rtl::OUString aArgUri;
                    if (getResourceNameRequestArgument(aRequestArguments,
                                                       &aArgUri))
                    {
                        rtl::OUString aResourceType;
                        getStringRequestArgument(
                            aRequestArguments,
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                              "ResourceType")),
                            &aResourceType);
                        nErrorCode
                            = aResourceType.equalsAsciiL(
                                RTL_CONSTASCII_STRINGPARAM("volume"))
                            ? ERRCODE_UUI_IO_NOTEXISTS_VOLUME
                            : (aResourceType.equalsAsciiL(
                                   RTL_CONSTASCII_STRINGPARAM("folder"))
                               ? ERRCODE_UUI_IO_NOTEXISTS_FOLDER
                               : ERRCODE_UUI_IO_NOTEXISTS);
                        aArguments.push_back(aArgUri);
                    }
                    else
                        nErrorCode = aErrorCode[aIoException.Code][0];
                    break;
                }

            default:
                {
                    rtl::OUString aArgUri;
                    if (getResourceNameRequestArgument(aRequestArguments,
                                                       &aArgUri))
                    {
                        nErrorCode = aErrorCode[aIoException.Code][1];
                        aArguments.push_back(aArgUri);
                    }
                    else
                        nErrorCode = aErrorCode[aIoException.Code][0];
                    break;
                }
            }

            handleErrorRequest(aIoException.Classification,
                               nErrorCode,
                               aArguments,
                               rRequest->getContinuations());
            return;
        }

        star::ucb::InteractiveAppException aAppException;
        if (aAnyRequest >>= aAppException)
        {
            std::vector< rtl::OUString > aArguments;
            handleErrorRequest( aAppException.Classification,
                                aAppException.Code,
                                aArguments,
                                rRequest->getContinuations());
        }

        star::ucb::InteractiveNetworkException aNetworkException;
        if (aAnyRequest >>= aNetworkException)
        {
            ErrCode nErrorCode;
            std::vector< rtl::OUString > aArguments;
            star::ucb::InteractiveNetworkOffLineException aOffLineException;
            star::ucb::InteractiveNetworkResolveNameException
                aResolveNameException;
            star::ucb::InteractiveNetworkConnectException aConnectException;
            star::ucb::InteractiveNetworkReadException aReadException;
            star::ucb::InteractiveNetworkWriteException aWriteException;
            if (aAnyRequest >>= aOffLineException)
                nErrorCode = ERRCODE_INET_OFFLINE;
            else if (aAnyRequest >>= aResolveNameException)
            {
                nErrorCode = ERRCODE_INET_NAME_RESOLVE;
                aArguments.push_back(aResolveNameException.Server);
            }
            else if (aAnyRequest >>= aConnectException)
            {
                nErrorCode = ERRCODE_INET_CONNECT;
                aArguments.push_back(aConnectException.Server);
            }
            else if (aAnyRequest >>= aReadException)
            {
                nErrorCode = ERRCODE_INET_READ;
                aArguments.push_back(aReadException.Diagnostic);
            }
            else if (aAnyRequest >>= aWriteException)
            {
                nErrorCode = ERRCODE_INET_WRITE;
                aArguments.push_back(aWriteException.Diagnostic);
            }
            else
                nErrorCode = ERRCODE_INET_GENERAL;
            handleErrorRequest(aNetworkException.Classification,
                               nErrorCode,
                               aArguments,
                               rRequest->getContinuations());
            return;
        }

        star::ucb::InteractiveCHAOSException aChaosException;
        if (aAnyRequest >>= aChaosException)
        {
            std::vector< rtl::OUString > aArguments;
            sal_Int32 nCount
                = std::min< sal_Int32 >(aChaosException.Arguments.getLength(),
                                        2);
            aArguments.
                reserve(
                    static_cast< std::vector< rtl::OUString >::size_type >(
                        nCount));
            for (sal_Int32 i = 0; i < nCount; ++i)
                aArguments.push_back(aChaosException.Arguments[i]);
            handleErrorRequest(aChaosException.Classification,
                               aChaosException.ID,
                               aArguments,
                               rRequest->getContinuations());
            return;
        }

        star::ucb::InteractiveWrongMediumException aWrongMediumException;
        if (aAnyRequest >>= aWrongMediumException)
        {
            sal_Int32 nMedium;
            aWrongMediumException.Medium >>= nMedium;
            std::vector< rtl::OUString > aArguments;
            aArguments.push_back(UniString::CreateFromInt32(nMedium + 1));
            handleErrorRequest(aWrongMediumException.Classification,
                               ERRCODE_UUI_WRONGMEDIUM,
                               aArguments,
                               rRequest->getContinuations());
            return;
        }

        star::java::WrongJavaVersionException aWrongJavaVersionException;
        if (aAnyRequest >>= aWrongJavaVersionException)
        {
            ErrCode nErrorCode;
            std::vector< rtl::OUString > aArguments;
            if (aWrongJavaVersionException.DetectedVersion.getLength() == 0)
                if (aWrongJavaVersionException.LowestSupportedVersion.
                                                   getLength()
                        == 0)
                    nErrorCode = ERRCODE_UUI_WRONGJAVA;
                else
                {
                    nErrorCode = ERRCODE_UUI_WRONGJAVA_MIN;
                    aArguments.push_back(aWrongJavaVersionException.
                                             LowestSupportedVersion);
                }
            else if (aWrongJavaVersionException.LowestSupportedVersion.
                                                    getLength()
                         == 0)
            {
                nErrorCode = ERRCODE_UUI_WRONGJAVA_VERSION;
                aArguments.push_back(aWrongJavaVersionException.
                                         DetectedVersion);
            }
            else
            {
                nErrorCode = ERRCODE_UUI_WRONGJAVA_VERSION_MIN;
                aArguments.reserve(2);
                aArguments.push_back(aWrongJavaVersionException.
                                         DetectedVersion);
                aArguments.push_back(aWrongJavaVersionException.
                                         LowestSupportedVersion);
            }
            handleErrorRequest(star::task::InteractionClassification_ERROR,
                               nErrorCode,
                               aArguments,
                               rRequest->getContinuations());
            return;
        }

        star::sync2::BadPartnershipException aBadPartnershipException;
        if (aAnyRequest >>= aBadPartnershipException)
        {
            ErrCode nErrorCode;
            std::vector< rtl::OUString > aArguments;
            if (aBadPartnershipException.Partnership.getLength() == 0)
                nErrorCode = ERRCODE_UUI_BADPARTNERSHIP;
            else
            {
                nErrorCode = ERRCODE_UUI_BADPARTNERSHIP_NAME;
                aArguments.push_back(aBadPartnershipException.Partnership);
            }
            handleErrorRequest(star::task::InteractionClassification_ERROR,
                               nErrorCode,
                               aArguments,
                               rRequest->getContinuations());
            return;
        }
    }
    catch (std::bad_alloc const &)
    {
        throw com::sun::star::uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  *this);
    }
}

Window * UUIInteractionHandler::getParentProperty() SAL_THROW(())
{
    osl::MutexGuard aGuard(m_aPropertyMutex);
    for (sal_Int32 i = 0; i < m_aProperties.getLength(); ++i)
    {
        star::beans::PropertyValue aProperty;
        if ((m_aProperties[i] >>= aProperty)
            && aProperty.
                   Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Parent")))
        {
            star::uno::Reference< star::awt::XWindow > xWindow;
            aProperty.Value >>= xWindow;
            return VCLUnoHelper::GetWindow(xWindow);
        }
    }
    return 0;
}

rtl::OUString UUIInteractionHandler::getContextProperty() SAL_THROW(())
{
    osl::MutexGuard aGuard(m_aPropertyMutex);
    for (sal_Int32 i = 0; i < m_aProperties.getLength(); ++i)
    {
        star::beans::PropertyValue aProperty;
        if ((m_aProperties[i] >>= aProperty)
            && aProperty.
                   Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Context")))
        {
            rtl::OUString aContext;
            aProperty.Value >>= aContext;
            return aContext;
        }
    }
    return rtl::OUString();
}

bool
UUIInteractionHandler::initPasswordContainer(
    star::uno::Reference< star::task::XPasswordContainer > * pContainer)
    const SAL_THROW(())
{
    OSL_ENSURE(pContainer, "specification violation");
    if (!pContainer->is() && m_xServiceFactory.is())
        try
        {
            *pContainer
                = star::uno::Reference< star::task::XPasswordContainer >(
                      m_xServiceFactory->
                          createInstance(
                              rtl::OUString(
                                  RTL_CONSTASCII_USTRINGPARAM(
                                     "com.sun.star.task.PasswordContainer"))),
                      star::uno::UNO_QUERY);
        }
        catch (star::uno::Exception const &)
        {}
    OSL_ENSURE(pContainer->is(), "unexpected situation");
    return pContainer->is();
}

void UUIInteractionHandler::executeLoginDialog(LoginErrorInfo & rInfo,
                                               rtl::OUString const & rRealm)
    SAL_THROW((star::uno::RuntimeException))
{
    try
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        bool bAccount = (rInfo.GetFlags() & LOGINERROR_FLAG_MODIFY_ACCOUNT)
                            != 0;
        bool bSavePassword = rInfo.GetIsPersistentPassword()
                             || rInfo.GetIsSavePassword();

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

        std::auto_ptr< ResMgr >
            xManager(ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        UniString aRealm(rRealm); // Forte compiler needs it spelled out...
        std::auto_ptr< LoginDialog >
            xDialog(new LoginDialog(getParentProperty(),
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
                                          xManager.get()));
            xDialog->SetSavePassword(rInfo.GetIsSavePassword());
        }

        rInfo.SetResult(xDialog->Execute() == RET_OK ? ERRCODE_BUTTON_OK :
                                                       ERRCODE_BUTTON_CANCEL);
        rInfo.SetUserName(xDialog->GetName());
        rInfo.SetPassword(xDialog->GetPassword());
        rInfo.SetAccount(xDialog->GetAccount());
        rInfo.SetSavePassword(xDialog->IsSavePassword());
    }
    catch (std::bad_alloc const &)
    {
        throw star::uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  *this);
    }
}

void
UUIInteractionHandler::executeMasterPasswordDialog(LoginErrorInfo & rInfo,
                                                   star::task::PasswordRequestMode
                                                   nMode)
    SAL_THROW((star::uno::RuntimeException))
{
    rtl::OString aMaster;
    try
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        std::auto_ptr< ResMgr >
            xManager(ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
    if( nMode == star::task::PasswordRequestMode_PASSWORD_CREATE )
    {
            std::auto_ptr< MasterPasswordCreateDialog >
                    xDialog(new MasterPasswordCreateDialog(
                            getParentProperty(), xManager.get()));
            rInfo.SetResult(xDialog->Execute() == RET_OK ? ERRCODE_BUTTON_OK :
                                                       ERRCODE_BUTTON_CANCEL);
            aMaster = rtl::OUStringToOString(xDialog->GetMasterPassword(),
                                                 RTL_TEXTENCODING_UTF8);
    }
    else
    {
            std::auto_ptr< MasterPasswordDialog >
                    xDialog(new MasterPasswordDialog(
                            getParentProperty(), nMode, xManager.get()));
            rInfo.SetResult(xDialog->Execute() == RET_OK ? ERRCODE_BUTTON_OK :
                                                       ERRCODE_BUTTON_CANCEL);
            aMaster = rtl::OUStringToOString(xDialog->GetMasterPassword(),
                                                 RTL_TEXTENCODING_UTF8);
    }

    }
    catch (std::bad_alloc const &)
    {
        throw star::uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  *this);
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
UUIInteractionHandler::executePasswordDialog(LoginErrorInfo & rInfo,
                                             star::task::PasswordRequestMode nMode,
                                             ::rtl::OUString aDocName)
    SAL_THROW((star::uno::RuntimeException))
{
    try
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        std::auto_ptr< ResMgr >
            xManager(ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        if( nMode == star::task::PasswordRequestMode_PASSWORD_CREATE )
        {
            std::auto_ptr< PasswordCreateDialog >
                    xDialog(new PasswordCreateDialog(
                            getParentProperty(), xManager.get()));

            ::rtl::OUString aTitle( xDialog->GetText() );
            if( aDocName.getLength() )
            {
                aTitle += ::rtl::OUString::createFromAscii( " [" );
                aTitle += aDocName;
                aTitle += ::rtl::OUString::createFromAscii( "]" );
                xDialog->SetText( aTitle );
            }

            rInfo.SetResult(xDialog->Execute() == RET_OK ? ERRCODE_BUTTON_OK :
                                                       ERRCODE_BUTTON_CANCEL);
            rInfo.SetPassword( xDialog->GetPassword() );
        }
        else
        {
            std::auto_ptr< PasswordDialog >
                    xDialog(new PasswordDialog(
                            getParentProperty(), nMode, xManager.get()));

            ::rtl::OUString aTitle( xDialog->GetText() );
            if( aDocName.getLength() )
            {
                aTitle += ::rtl::OUString::createFromAscii( " [" );
                aTitle += aDocName;
                aTitle += ::rtl::OUString::createFromAscii( "]" );
                xDialog->SetText( aTitle );
            }

            rInfo.SetResult(xDialog->Execute() == RET_OK ? ERRCODE_BUTTON_OK :
                                                       ERRCODE_BUTTON_CANCEL);
            rInfo.SetPassword( xDialog->GetPassword() );
        }
    }
    catch (std::bad_alloc const &)
    {
        throw star::uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  *this);
    }
}

void
UUIInteractionHandler::executeCookieDialog(CntHTTPCookieRequest & rRequest)
    SAL_THROW((star::uno::RuntimeException))
{
    try
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        std::auto_ptr< ResMgr >
            xManager(ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        std::auto_ptr< CookiesDialog >
            xDialog(new CookiesDialog(
                            getParentProperty(), &rRequest, xManager.get()));
        xDialog->Execute();
    }
    catch (std::bad_alloc const &)
    {
        throw star::uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  *this);
    }
}

void UUIInteractionHandler::executeFilterDialog(rtl::OUString       const & rURL    ,
                                                uui::FilterNameList const & rFilters,
                                                rtl::OUString             & rFilter )
    SAL_THROW((star::uno::RuntimeException))
{
    try
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        std::auto_ptr< ResMgr >
            xManager(ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));

        std::auto_ptr< uui::FilterDialog >
            xDialog(new uui::FilterDialog(getParentProperty(),
                                     xManager.get()));

        xDialog->SetURL(rURL);
        xDialog->ChangeFilters(&rFilters);

        uui::FilterNameListPtr pSelected = rFilters.end();
        if( xDialog->AskForFilter( pSelected ) )
        {
            rFilter = pSelected->sInternal;
        }
    }
    catch (std::bad_alloc const &)
    {
        throw star::uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  *this);
    }
}

USHORT
UUIInteractionHandler::executeErrorDialog(
    star::task::InteractionClassification eClassification,
    rtl::OUString const & rContext,
    rtl::OUString const & rMessage,
    WinBits nButtonMask)
    SAL_THROW((star::uno::RuntimeException))
{

    vos::OGuard aGuard(Application::GetSolarMutex());

    rtl::OUStringBuffer aText(rContext);
    if (rContext.getLength() != 0 && rMessage.getLength() != 0)
        aText.appendAscii(RTL_CONSTASCII_STRINGPARAM(":\n"));
            //TODO! must be internationalized
    if (rMessage.getLength() != 0)
    {
        aText.append(rMessage);
        aText.append(
                  static_cast< sal_Unicode >(
                      eClassification
                              == star::task::InteractionClassification_QUERY ?
                          '?' : '.'));
            //TODO! must be internationalized
    }

    std::auto_ptr< MessBox > xBox;
    try
    {
        switch (eClassification)
        {
        case star::task::InteractionClassification_ERROR:
            xBox.reset(new ErrorBox(getParentProperty(),
                                    nButtonMask,
                                    aText.makeStringAndClear()));
            break;

        case star::task::InteractionClassification_WARNING:
            xBox.reset(new WarningBox(getParentProperty(),
                                      nButtonMask,
                                      aText.makeStringAndClear()));
            break;

        case star::task::InteractionClassification_INFO:
            if ((nButtonMask & 0x01F00000) == WB_DEF_OK)
                    //TODO! missing win bit button mask define (want to ignore
                    // any default button settings)...
                xBox.reset(new InfoBox(getParentProperty(),
                                       aText.makeStringAndClear()));
            else
                xBox.reset(new ErrorBox(getParentProperty(),
                                        nButtonMask,
                                        aText.makeStringAndClear()));
            break;

        case star::task::InteractionClassification_QUERY:
            xBox.reset(new QueryBox(getParentProperty(),
                                    nButtonMask,
                                    aText.makeStringAndClear()));

            USHORT aResult = xBox->Execute();
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
    catch (std::bad_alloc const &)
    {
        throw star::uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  *this);
    }

    return xBox->Execute();
}

USHORT
UUIInteractionHandler::executeMessageBox(
    rtl::OUString const & rTitle,
    rtl::OUString const & rMessage,
    WinBits nButtonMask )
    SAL_THROW((star::uno::RuntimeException))
{

    vos::OGuard aGuard(Application::GetSolarMutex());

    MessBox xBox( getParentProperty(), nButtonMask, rTitle, rMessage );

    USHORT aResult = xBox.Execute();
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

void
UUIInteractionHandler::handleAuthenticationRequest(
    star::ucb::AuthenticationRequest const & rRequest,
    star::uno::Sequence< star::uno::Reference<
                             star::task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((star::uno::RuntimeException))
{
    star::uno::Reference< star::task::XInteractionRetry > xRetry;
    star::uno::Reference< star::task::XInteractionAbort > xAbort;
    star::uno::Reference< star::ucb::XInteractionSupplyAuthentication >
        xSupplyAuthentication;
    getContinuations(
        rContinuations, 0, 0, &xRetry, &xAbort, &xSupplyAuthentication, 0);
    bool bRemember;
    bool bRememberPersistent;
    if (xSupplyAuthentication.is())
    {
        star::ucb::RememberAuthentication eDefault;
        star::uno::Sequence< star::ucb::RememberAuthentication >
            aModes(xSupplyAuthentication->getRememberPasswordModes(eDefault));
        bRemember = eDefault != star::ucb::RememberAuthentication_NO;
        bRememberPersistent = false;
        for (sal_Int32 i = 0; i < aModes.getLength(); ++i)
            if (aModes[i] == star::ucb::RememberAuthentication_PERSISTENT)
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

    com::sun::star::uno::Reference< com::sun::star::task::XPasswordContainer >
        xContainer;

    // xContainer works with userName passwdSequences pairs:
    if (rRequest.HasUserName
        && rRequest.HasPassword
        && initPasswordContainer(&xContainer))
        try
        {
            if (rRequest.UserName.getLength() == 0)
            {
                star::task::UrlRecord
                    aRec(xContainer->find(rRequest.ServerName, this));
                if (aRec.UserList.getLength() != 0)
                {
                    xSupplyAuthentication->
                        setUserName(aRec.UserList[0].UserName.getStr());
                    OSL_ENSURE(aRec.UserList[0].Passwords.getLength() != 0,
                               "empty password list");
                    xSupplyAuthentication->
                        setPassword(aRec.UserList[0].Passwords[0].getStr());
                    if (aRec.UserList[0].Passwords.getLength() > 1)
                        if (rRequest.HasRealm)
                            xSupplyAuthentication->
                                setRealm(aRec.UserList[0].Passwords[1].
                                                              getStr());
                        else
                            xSupplyAuthentication->
                                setAccount(aRec.UserList[0].Passwords[1].
                                                                getStr());
                    xSupplyAuthentication->select();
                    return;
                }
            }
            else
            {
                star::task::UrlRecord
                    aRec(xContainer->findForName(rRequest.ServerName,
                                                 rRequest.UserName,
                                                 this));
                if (aRec.UserList.getLength() != 0)
                {
                    OSL_ENSURE(aRec.UserList[0].Passwords.getLength() != 0,
                               "empty password list");
                    if (!rRequest.HasPassword
                        || rRequest.Password != aRec.UserList[0].Passwords[0])
                    {
                        xSupplyAuthentication->
                            setUserName(aRec.UserList[0].UserName.getStr());
                        xSupplyAuthentication->
                            setPassword(aRec.UserList[0].Passwords[0].
                                                             getStr());
                        if (aRec.UserList[0].Passwords.getLength() > 1)
                            if (rRequest.HasRealm)
                                xSupplyAuthentication->
                                    setRealm(aRec.UserList[0].Passwords[1].
                                                                  getStr());
                            else
                                xSupplyAuthentication->
                                    setAccount(aRec.UserList[0].Passwords[1].
                                                                    getStr());
                        xSupplyAuthentication->select();
                        return;
                    }
                }
            }
        }
        catch (star::task::NoMasterException const &)
        {} // user did not enter master password

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
    aInfo.SetModifyAccount(rRequest.HasAccount
                           && xSupplyAuthentication.is()
                           && xSupplyAuthentication->canSetAccount());
    aInfo.SetModifyUserName(rRequest.HasUserName
                            && xSupplyAuthentication.is()
                            && xSupplyAuthentication->canSetUserName());
    executeLoginDialog(aInfo,
                       rRequest.HasRealm ? rRequest.Realm : rtl::OUString());
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
                            star::ucb::RememberAuthentication_PERSISTENT :
                            star::ucb::RememberAuthentication_SESSION :
                        star::ucb::RememberAuthentication_NO);
            if (rRequest.HasRealm)
                xSupplyAuthentication->setRealm(aInfo.GetAccount());
            else
                xSupplyAuthentication->setAccount(aInfo.GetAccount());
            xSupplyAuthentication->select();
        }
        // Empty user name can not be valid:
        if (aInfo.GetUserName().Len() != 0
            && initPasswordContainer(&xContainer))
        {
            star::uno::Sequence< rtl::OUString >
                aPassList(aInfo.GetAccount().Len() == 0 ? 1 : 2);
            aPassList[0] = aInfo.GetPassword();
            if (aInfo.GetAccount().Len() != 0)
                aPassList[1] = aInfo.GetAccount();
            try
            {
                if (aInfo.GetIsSavePassword())
                    if (bRememberPersistent)
                        xContainer->addPersistent(rRequest.ServerName,
                                                  aInfo.GetUserName(),
                                                  aPassList,
                                                  this);
                    else
                        xContainer->add(rRequest.ServerName,
                                        aInfo.GetUserName(),
                                        aPassList,
                                        this);
            }
            catch (star::task::NoMasterException const &)
            {} // user did not enter master password
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
UUIInteractionHandler::handleMasterPasswordRequest(
    star::task::PasswordRequestMode nMode,
    star::uno::Sequence< star::uno::Reference<
                             star::task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((star::uno::RuntimeException))
{
    star::uno::Reference< star::task::XInteractionRetry > xRetry;
    star::uno::Reference< star::task::XInteractionAbort > xAbort;
    star::uno::Reference< star::ucb::XInteractionSupplyAuthentication >
        xSupplyAuthentication;
    getContinuations(
        rContinuations, 0, 0, &xRetry, &xAbort, &xSupplyAuthentication, 0);
    LoginErrorInfo aInfo;

    // in case of master password a hash code is returned
    executeMasterPasswordDialog(aInfo, nMode);

    switch (aInfo.GetResult())
    {
    case ERRCODE_BUTTON_OK:
        if (xSupplyAuthentication.is())
        {
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
UUIInteractionHandler::handlePasswordRequest(
    star::task::PasswordRequestMode nMode,
    star::uno::Sequence< star::uno::Reference<
                             star::task::XInteractionContinuation > > const &
        rContinuations,
    ::rtl::OUString aDocumentName )
    SAL_THROW((star::uno::RuntimeException))
{
    star::uno::Reference< star::task::XInteractionRetry > xRetry;
    star::uno::Reference< star::task::XInteractionAbort > xAbort;
    star::uno::Reference< star::task::XInteractionPassword >
        xPassword;
    getContinuations(
        rContinuations, 0, 0, &xRetry, &xAbort, 0, &xPassword);
    LoginErrorInfo aInfo;

       executePasswordDialog(aInfo, nMode, aDocumentName);

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

void
UUIInteractionHandler::handleCookiesRequest(
    star::ucb::HandleCookiesRequest const & rRequest,
    star::uno::Sequence< star::uno::Reference<
                             star::task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((star::uno::RuntimeException))
{
    CookieList aCookies;
    {for (sal_Int32 i = 0; i < rRequest.Cookies.getLength(); ++i)
        try
        {
            std::auto_ptr< CntHTTPCookie > xCookie(new CntHTTPCookie);
            xCookie->m_aName = UniString(rRequest.Cookies[i].Name);
            xCookie->m_aValue = UniString(rRequest.Cookies[i].Value);
            xCookie->m_aDomain = UniString(rRequest.Cookies[i].Domain);
            xCookie->m_aPath = UniString(rRequest.Cookies[i].Path);
            xCookie->m_aExpires
                = DateTime(Date(rRequest.Cookies[i].Expires.Day,
                                rRequest.Cookies[i].Expires.Month,
                                rRequest.Cookies[i].Expires.Year),
                           Time(rRequest.Cookies[i].Expires.Hours,
                                rRequest.Cookies[i].Expires.Minutes,
                                rRequest.Cookies[i].Expires.Seconds,
                                rRequest.Cookies[i].Expires.
                                                        HundredthSeconds));
            xCookie->m_nFlags
                = rRequest.Cookies[i].Secure ? CNTHTTP_COOKIE_FLAG_SECURE : 0;
            switch (rRequest.Cookies[i].Policy)
            {
            case star::ucb::CookiePolicy_CONFIRM:
                xCookie->m_nPolicy = CNTHTTP_COOKIE_POLICY_INTERACTIVE;
                break;

            case star::ucb::CookiePolicy_ACCEPT:
                xCookie->m_nPolicy = CNTHTTP_COOKIE_POLICY_ACCEPTED;
                break;

            case star::ucb::CookiePolicy_IGNORE:
                xCookie->m_nPolicy = CNTHTTP_COOKIE_POLICY_BANNED;
                break;
            }
            aCookies.Insert(xCookie.get(), LIST_APPEND);
            xCookie.release();
        }
        catch (std::bad_alloc const &)
        {
            throw star::uno::RuntimeException(
                      rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                        "out of memory")),
                      *this);
        }
    }
    CntHTTPCookieRequest
        aRequest(rRequest.URL,
                 aCookies,
                 rRequest.Request == star::ucb::CookieRequest_RECEIVE ?
                     CNTHTTP_COOKIE_REQUEST_RECV :
                     CNTHTTP_COOKIE_REQUEST_SEND);
    executeCookieDialog(aRequest);
    for (sal_Int32 i = 0; i < rContinuations.getLength(); ++i)
    {
        star::uno::Reference< star::ucb::XInteractionCookieHandling >
            xCookieHandling(rContinuations[i], star::uno::UNO_QUERY);
        if (xCookieHandling.is())
        {
            switch (aRequest.m_nRet)
            {
            case CNTHTTP_COOKIE_POLICY_INTERACTIVE:
                xCookieHandling->
                    setGeneralPolicy(star::ucb::CookiePolicy_CONFIRM);
                break;

            case CNTHTTP_COOKIE_POLICY_ACCEPTED:
                xCookieHandling->
                    setGeneralPolicy(star::ucb::CookiePolicy_ACCEPT);
                break;

            case CNTHTTP_COOKIE_POLICY_BANNED:
                xCookieHandling->
                    setGeneralPolicy(star::ucb::CookiePolicy_IGNORE);
                break;
            }
            for (sal_Int32 j = 0; j < rRequest.Cookies.getLength(); ++j)
                if (rRequest.Cookies[j].Policy
                        == star::ucb::CookiePolicy_CONFIRM)
                    switch (static_cast< CntHTTPCookie * >(aCookies.
                                                               GetObject(j))->
                                m_nPolicy)
                    {
                    case CNTHTTP_COOKIE_POLICY_ACCEPTED:
                        xCookieHandling->
                            setSpecificPolicy(rRequest.Cookies[j], true);
                        break;

                    case CNTHTTP_COOKIE_POLICY_BANNED:
                        xCookieHandling->
                            setSpecificPolicy(rRequest.Cookies[j], false);
                        break;
                    }
            xCookieHandling->select();
            break;
        }
    }
}

void
UUIInteractionHandler::handleNoSuchFilterRequest( star::document::NoSuchFilterRequest const &                                                 rRequest       ,
                                                  star::uno::Sequence< star::uno::Reference< star::task::XInteractionContinuation > > const & rContinuations ) SAL_THROW((star::uno::RuntimeException))
{
    star::uno::Reference< star::task::XInteractionAbort >            xAbort          ;
    star::uno::Reference< star::document::XInteractionFilterSelect > xFilterTransport;

    sal_Int32 nCount = rContinuations.getLength();
    for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
    {
        if( ! xAbort.is() )
            xAbort = star::uno::Reference< star::task::XInteractionAbort >( rContinuations[nStep], star::uno::UNO_QUERY );

        if( ! xFilterTransport.is() )
            xFilterTransport = star::uno::Reference< star::document::XInteractionFilterSelect >( rContinuations[nStep], star::uno::UNO_QUERY );
    }

    // check neccessary ressources - if they doesn't exist - abort or break this operation
    if (!xAbort.is())
        return;

    if (!xFilterTransport.is() || !m_xServiceFactory.is())
    {
        xAbort->select();
        return;
    }

    star::uno::Reference< star::container::XNameContainer > xFilterContainer( m_xServiceFactory->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.document.FilterFactory") ), star::uno::UNO_QUERY );
    if (!xFilterContainer.is())
    {
        xAbort->select();
        return;
    }

    uui::FilterNameList                                 lNames        ;
    uui::FilterNamePair                                 aPair         ;
    star::uno::Sequence< rtl::OUString >                lFilters      ;
    sal_Int32                                           nFilterCount  = 0;
    sal_Int32                                           nPropCount    = 0;
    star::uno::Sequence< star::beans::PropertyValue >   lProps        ;
    rtl::OUString                                       sInternalName ;
    rtl::OUString                                       sUIName       ;
    sal_Bool                                            bTakeIt       = sal_False;
    sal_Int8                                            nHandledProps = 0;

    // Note: We look for all filters here which match the following criteria:
    //          - they are import filters as minimum (of course they can support export too)
    //          - we don't show any filter which are flaged as "don't show it at the UI" or "they are not installed"
    //          - we ignore filters, which have not set any valid DocumentService (e.g. our pure graphic filters)
    //          - we show it sorted by her UIName's
    //          - We don't use the order flag or prefer default filters. (Because this list shows all filters and the user should find his filter vry easy by his UIName ...)
    //          - We use "_query_all" here ... but we filter graphic filters out by using DocumentService property later!
    if ( !(xFilterContainer->getByName(::rtl::OUString::createFromAscii("_query_all:sort_prop=uiname:iflags=1:eflags=143360")) >>= lFilters) )
    {
        xAbort->select();
        return;
    }

    nFilterCount = lFilters.getLength();
    for (sal_Int32 f=0; f<nFilterCount; ++f)
    {
        // ignore it - if we can't unpack set of properties or we found an empty internal name!
        if ( lFilters[f].getLength()<1 || !(xFilterContainer->getByName(lFilters[f])>>=lProps) )
            continue;

        // step over the properties of the filter
        // search for "UIName" and "DocumentService"
        // break the loop if both values could be found
        bTakeIt       = sal_False;           // indicates if we use this filter after this loop of not
        nHandledProps = 0;                   // used to break loop if two properties could be readed
        nPropCount    = lProps.getLength();
        for(sal_Int32 p=0; p<nPropCount && nHandledProps<2; ++p )
        {
            if( lProps[p].Name.compareToAscii("UIName") == 0 )
            {
                bTakeIt = ( (lProps[p].Value>>=sUIName) && (sUIName.getLength()>0) );
                if (!bTakeIt)
                    break;
                ++nHandledProps;
            }
            else
            if( lProps[p].Name.compareToAscii("DocumentService") == 0 )
            {
                ::rtl::OUString sService;
                bTakeIt = ( (lProps[p].Value>>=sService) && (sService.getLength()>0) );
                if (!bTakeIt)
                    break;
                ++nHandledProps;
            }
        }

        // now we should have a valid filter for showing ... or not :-)
        if (!bTakeIt)
            continue;

        aPair.sInternal = lFilters[f];
        aPair.sUI       = sUIName    ;
        lNames.push_back( aPair );
    }

    // no list available for showing
    // -> abort operation
    if (lNames.size()<1)
    {
        xAbort->select();
        return;
    }

    // let the user select the right filter
    rtl::OUString sSelectedFilter;
    executeFilterDialog( rRequest.URL, lNames, sSelectedFilter );

    // If he doesn't select anyone
    // -> abort operation
    if (sSelectedFilter.getLength()<1)
    {
        xAbort->select();
        return;
    }

    // otherwhise set it for return
    xFilterTransport->setFilter( sSelectedFilter );
    xFilterTransport->select();
}

void
UUIInteractionHandler::handleAmbigousFilterRequest(
    com::sun::star::document::AmbigousFilterRequest const & rRequest,
    com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference<
                com::sun::star::task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((com::sun::star::uno::RuntimeException))
{
    star::uno::Reference< star::task::XInteractionAbort >            xAbort          ;
    star::uno::Reference< star::document::XInteractionFilterSelect > xFilterTransport;

    sal_Int32 nCount = rContinuations.getLength();
    for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
    {
        if( ! xAbort.is() )
            xAbort = star::uno::Reference< star::task::XInteractionAbort >( rContinuations[nStep], star::uno::UNO_QUERY );

        if( ! xFilterTransport.is() )
            xFilterTransport = star::uno::Reference< star::document::XInteractionFilterSelect >( rContinuations[nStep], star::uno::UNO_QUERY );
    }

    uui::FilterNameList lNames;

    if( m_xServiceFactory.is() == sal_True )
    {
        star::uno::Reference< star::container::XNameContainer > xFilterContainer( m_xServiceFactory->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.document.FilterFactory") ), star::uno::UNO_QUERY );
        if( xFilterContainer.is() == sal_True )
        {
            star::uno::Any                                    aPackedSet    ;
            star::uno::Sequence< star::beans::PropertyValue > lProps        ;
            sal_Int32                                         nCount        ;
            sal_Int32                                         nStep         ;
            uui::FilterNamePair                               aPair         ;

            aPackedSet   = xFilterContainer->getByName( rRequest.SelectedFilter );
            aPackedSet >>= lProps;
            nCount       = lProps.getLength();
            for( nStep=0; nStep<nCount; ++nStep )
            {
                if( lProps[nStep].Name.compareToAscii("UIName") == 0 )
                {
                    ::rtl::OUString sTemp;
                    lProps[nStep].Value >>= sTemp;
                    aPair.sUI       = sTemp;
                    aPair.sInternal = rRequest.SelectedFilter;
                    lNames.push_back( aPair );
                    break;
                }
            }

            aPackedSet   = xFilterContainer->getByName( rRequest.DetectedFilter );
            aPackedSet >>= lProps;
            nCount       = lProps.getLength();
            for( nStep=0; nStep<nCount; ++nStep )
            {
                if( lProps[nStep].Name.compareToAscii("UIName") == 0 )
                {
                    ::rtl::OUString sTemp;
                    lProps[nStep].Value >>= sTemp;
                    aPair.sUI       = sTemp;
                    aPair.sInternal = rRequest.DetectedFilter;
                    lNames.push_back( aPair );
                    break;
                }
            }
        }
    }

    if( xAbort.is() && xFilterTransport.is() )
    {
        if( lNames.size() < 1 )
        {
            xAbort->select();
        }
        else
        {
            rtl::OUString sFilter;
            executeFilterDialog( rRequest.URL, lNames, sFilter );

            if( sFilter.getLength() > 0 )
            {
                xFilterTransport->setFilter( sFilter );
                xFilterTransport->select();
            }
            else
                xAbort->select();
        }
    }
}

void
UUIInteractionHandler::handleGenericErrorRequest(
    com::sun::star::task::ErrorCodeRequest const & rRequest,
    com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference<
                com::sun::star::task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((com::sun::star::uno::RuntimeException))
{
    star::uno::Reference< star::task::XInteractionAbort > xAbort;
    star::uno::Reference< star::task::XInteractionApprove > xApprove;

    sal_Int32 nCount = rContinuations.getLength();
    for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
    {
        if( ! xAbort.is() )
            xAbort = star::uno::Reference< star::task::XInteractionAbort >( rContinuations[nStep], star::uno::UNO_QUERY );

        if( ! xApprove.is() )
            xApprove = star::uno::Reference< star::task::XInteractionApprove >( rContinuations[nStep], star::uno::UNO_QUERY );
    }

    // Note: It's important to convert the transported long to the required unsigned long value.
    // Otherwhise using as flag field can fail ...
    ErrCode  nError   = (ErrCode)rRequest.ErrCode;
    sal_Bool bWarning = !ERRCODE_TOERROR(nError);
    ErrorHandler::HandleError(rRequest.ErrCode);

    if (xApprove.is() && bWarning)
        xApprove->select();
    else if (xAbort.is())
        xAbort->select();
}

void
UUIInteractionHandler::handleFilterOptionsRequest(
    com::sun::star::document::FilterOptionsRequest const & rRequest,
    com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference<
                com::sun::star::task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((com::sun::star::uno::RuntimeException))
{
    star::uno::Reference< star::task::XInteractionAbort >            xAbort          ;
    star::uno::Reference< star::document::XInteractionFilterOptions > xFilterOptions;

    sal_Int32 nCount = rContinuations.getLength();
    for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
    {
        if( ! xAbort.is() )
            xAbort = star::uno::Reference< star::task::XInteractionAbort >( rContinuations[nStep], star::uno::UNO_QUERY );

        if( ! xFilterOptions.is() )
            xFilterOptions = star::uno::Reference< star::document::XInteractionFilterOptions >( rContinuations[nStep], star::uno::UNO_QUERY );
    }

    star::uno::Reference< star::container::XNameAccess > xFilterCFG;
    if( m_xServiceFactory.is() )
    {
        xFilterCFG = star::uno::Reference< star::container::XNameAccess >(
            m_xServiceFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.document.FilterFactory" ) ),
            star::uno::UNO_QUERY );
    }

    if( xFilterCFG.is() && rRequest.rProperties.getLength() )
    {
        try {
            ::rtl::OUString aFilterName;
            sal_Int32 nPropCount = rRequest.rProperties.getLength();
               for( sal_Int32 ind = 0; ind < nPropCount; ++ind )
            {
                rtl::OUString tmp = rRequest.rProperties[ind].Name;
                   if( rRequest.rProperties[ind].Name.equals( ::rtl::OUString::createFromAscii("FilterName")) )
                {
                    rRequest.rProperties[ind].Value >>= aFilterName;
                    break;
                }
            }

            star::uno::Sequence < star::beans::PropertyValue > aProps;
            if( xFilterCFG->getByName( aFilterName ) >>= aProps )
               {
                   sal_Int32 nPropertyCount = aProps.getLength();
                   for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
                       if( aProps[nProperty].Name.equals( ::rtl::OUString::createFromAscii("UIComponent")) )
                       {
                        ::rtl::OUString aServiceName;
                           aProps[nProperty].Value >>= aServiceName;
                        if( aServiceName.getLength() )
                        {
                            star::uno::Reference< star::ui::dialogs::XExecutableDialog > xFilterDialog(
                                        m_xServiceFactory->createInstance( aServiceName ),
                                        star::uno::UNO_QUERY );
                            star::uno::Reference< star::beans::XPropertyAccess > xFilterProperties(
                                        xFilterDialog,
                                        star::uno::UNO_QUERY );

                            if( xFilterDialog.is() && xFilterProperties.is() )
                            {
                                star::uno::Reference< star::document::XImporter > xImporter(
                                                xFilterDialog,
                                                star::uno::UNO_QUERY );
                                if( xImporter.is() )
                                    xImporter->setTargetDocument( star::uno::Reference< star::lang::XComponent >(
                                                    rRequest.rModel,
                                                    star::uno::UNO_QUERY ) );

                                xFilterProperties->setPropertyValues( rRequest.rProperties );

                                if( xFilterDialog->execute() )
                                {
                                    xFilterOptions->setFilterOptions( xFilterProperties->getPropertyValues() );
                                    xFilterOptions->select();
                                    return;

                                }
                            }
                        }
                        break;
                    }
            }
        }
        catch( star::container::NoSuchElementException& )
        {
            // the filter name is unknown
        }
        catch( star::uno::Exception& )
        {
        }
    }

    xAbort->select();
}

::rtl::OUString replaceMessageWithArguments(
    ::rtl::OUString aMessage,
    std::vector< rtl::OUString > const & rArguments )
{
    for (sal_Int32 i = 0;;)
    {
        i = aMessage.
                indexOf(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$(ARG")),
                        i);
        if (i == -1)
            break;
        if (aMessage.getLength() - i >= RTL_CONSTASCII_LENGTH("$(ARGx)")
            && aMessage.getStr()[i + RTL_CONSTASCII_LENGTH("$(ARGx")] == ')')
        {
            sal_Unicode c
                = aMessage.getStr()[i + RTL_CONSTASCII_LENGTH("$(ARG")];
            if (c >= '1' && c <= '2')
            {
                std::vector< rtl::OUString >::size_type nIndex
                    = static_cast< std::vector< rtl::OUString >::size_type >(
                          c - '1');
                if (nIndex < rArguments.size())
                {
                    aMessage
                        = aMessage.replaceAt(i,
                                             RTL_CONSTASCII_LENGTH("$(ARGx)"),
                                             rArguments[nIndex]);
                    i += rArguments[nIndex].getLength();
                    continue;
                }
            }
        }
        ++i;
    }

    return aMessage;
}


void
UUIInteractionHandler::handleErrorRequest(
    star::task::InteractionClassification eClassification,
    ErrCode nErrorCode,
    std::vector< rtl::OUString > const & rArguments,
    star::uno::Sequence< star::uno::Reference<
                             star::task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((star::uno::RuntimeException))
{
    //TODO! It can happen that the buttons calculated below do not match the
    // error text from the resource (e.g., some text that is not a question,
    // but YES and NO buttons).  Some error texts have ExtraData that
    // specifies a set of buttons, but that data is not really useful, because
    // a single error text may well make sense both with only an OK button and
    // with RETRY and CANCEL buttons.

    star::uno::Reference< star::task::XInteractionApprove > xApprove;
    star::uno::Reference< star::task::XInteractionDisapprove > xDisapprove;
    star::uno::Reference< star::task::XInteractionRetry > xRetry;
    star::uno::Reference< star::task::XInteractionAbort > xAbort;
    getContinuations(
        rContinuations, &xApprove, &xDisapprove, &xRetry, &xAbort, 0, 0);

    // The following mapping uses the bit mask
    //     Approve = 8,
    //     Disapprove = 4,
    //     Retry = 2,
    //     Abort = 1
    //
    // The mapping has five properties on which the code to select the correct
    // continuation relies:
    // 1  The OK button is mapped to Approve if that is available, otherwise
    //    to Abort if that is available, otherwise to none.
    // 2  The CANCEL button is always mapped to Abort.
    // 3  The RETRY button is always mapped to Retry.
    // 4  The NO button is always mapped to Disapprove.
    // 5  The YES button is always mapped to Approve.
    //
    // Because the WinBits button combinations are quite restricted, not every
    // request can be served here.
    //
    // Finally, it seems to be better to leave default button determination to
    // VCL (the favouring of CANCEL as default button seems to not always be
    // what the user wants)...
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

    rtl::OUString aMessage;
    {
        enum Source { SOURCE_DEFAULT, SOURCE_CNT, SOURCE_UUI };
        static char const * const aManager[3]
            = { 0,
                CREATEVERSIONRESMGR_NAME(cnt),
                CREATEVERSIONRESMGR_NAME(uui) };
        static USHORT const aId[3]
            = { RID_ERRHDL,
                RID_CHAOS_START + 12,
                    // cf. chaos/source/inc/cntrids.hrc, where
                    // #define RID_CHAOS_ERRHDL (RID_CHAOS_START + 12)
                RID_UUI_ERRHDL };
        Source eSource = nErrorCode >= ERRCODE_AREA_TOOLS
                         && nErrorCode < ERRCODE_AREA_LIB1 ?
                             SOURCE_DEFAULT :
                         nErrorCode >= ERRCODE_AREA_CHAOS
                         && nErrorCode < ERRCODE_AREA_CHAOS_END ?
                             SOURCE_CNT :
                             SOURCE_UUI;

        vos::OGuard aGuard(Application::GetSolarMutex());
        std::auto_ptr< ResMgr > xManager;
        if (aManager[eSource])
        {
            xManager.reset(ResMgr::CreateResMgr(aManager[eSource]));
            if (!xManager.get())
                return;
        }
        ResId aResId(aId[eSource], xManager.get());
        if (!ErrorResource(aResId).  getString(nErrorCode, &aMessage))
            return;
    }

    aMessage = replaceMessageWithArguments( aMessage, rArguments );

    switch (executeErrorDialog(
                eClassification, aContext, aMessage, nButtonMask ))
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

void
UUIInteractionHandler::handleBrokenPackageRequest(
    std::vector< rtl::OUString > const & rArguments,
    star::uno::Sequence< star::uno::Reference<
                             star::task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((star::uno::RuntimeException))
{
    star::uno::Reference< star::task::XInteractionApprove > xApprove;
    star::uno::Reference< star::task::XInteractionDisapprove > xDisapprove;
    star::uno::Reference< star::task::XInteractionAbort > xAbort;
    getContinuations(
        rContinuations, &xApprove, &xDisapprove, 0, &xAbort, 0, 0);

    WinBits nButtonMask;
    ErrCode nErrorCode;
    if( xApprove.is() && xDisapprove.is() )
    {
        nErrorCode = ERRCODE_UUI_IO_BROKENPACKAGE;
        nButtonMask = WB_YES_NO | WB_DEF_YES;
    }
    else if ( xAbort.is() )
    {
        nErrorCode = ERRCODE_UUI_IO_BROKENPACKAGE_CANTREPAIR;
        nButtonMask = WB_OK;
    }
    else
        return;

    star::uno::Any aProductNameAny =
                ::utl::ConfigManager::GetConfigManager()->GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTNAME );
    star::uno::Any aProductVersionAny =
                ::utl::ConfigManager::GetConfigManager()->GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTVERSION );
    ::rtl::OUString aProductName, aProductVersion;
    if ( !( aProductNameAny >>= aProductName ) )
        aProductName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("StarOffice") );

    ::rtl::OUString aTitle( aProductName );
    if( aProductVersionAny >>= aProductVersion )
    {
        aTitle += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(" ") );
        aTitle += aProductVersion;
    }

    ::rtl::OUString aMessage;

    {
        vos::OGuard aGuard(Application::GetSolarMutex());
        std::auto_ptr< ResMgr > xManager(ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        if (!xManager.get())
            return;

        ResId aResId( RID_UUI_ERRHDL, xManager.get() );
        if ( !ErrorResource(aResId).getString(nErrorCode, &aMessage) )
            return;
    }

    aMessage = replaceMessageWithArguments( aMessage, rArguments );

    switch ( executeMessageBox( aTitle, aMessage, nButtonMask ))
    {
    case ERRCODE_BUTTON_OK:
        OSL_ENSURE( xAbort.is(), "unexpected situation" );
        if (xAbort.is())
            xAbort->select();
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

char const UUIInteractionHandler::m_aImplementationName[]
    = "com.sun.star.comp.uui.UUIInteractionHandler";

star::uno::Sequence< rtl::OUString >
UUIInteractionHandler::getSupportedServiceNames_static()
{
    star::uno::Sequence< rtl::OUString > aNames(2);
    aNames[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.task.InteractionHandler"));
    aNames[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.uui.InteractionHandler"));
         // for backwards compatibility
    return aNames;
}

star::uno::Reference< star::uno::XInterface > SAL_CALL
UUIInteractionHandler::createInstance(
    star::uno::Reference< star::lang::XMultiServiceFactory > const &
        rServiceFactory)
    SAL_THROW((star::uno::Exception))
{
    try
    {
        return *new UUIInteractionHandler(rServiceFactory);
    }
    catch (std::bad_alloc const &)
    {
        throw star::uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  0);
    }
}
