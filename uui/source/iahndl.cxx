/*************************************************************************
 *
 *  $RCSfile: iahndl.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2001-08-20 12:31:10 $
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
#ifndef UUI_PASSWORDDLG_HXX
#include "passworddlg.hxx"
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
#ifndef _COM_SUN_STAR_TASK_CLASSIFIEDINTERACTIONREQUEST_HPP_
#include "com/sun/star/task/ClassifiedInteractionRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_NOMASTEREXCEPTION_HPP_
#include "com/sun/star/task/NoMasterException.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_PASSWORDREQUEST_HPP_
#include "com/sun/star/task/PasswordRequest.hpp"
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
#ifndef _COM_SUN_STAR_UCB_AUTHENTICATIONREQUEST_HPP_
#include "com/sun/star/ucb/AuthenticationRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_HANDLECOOKIESREQUEST_HPP_
#include "com/sun/star/ucb/HandleCookiesRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAUGMENTEDIOEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveAugmentedIOException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEBADTRANSFERURLEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveBadTransferURLException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVECHAOSEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveCHAOSException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEFILEIOEXCEPTION_HPP_
#include "com/sun/star/ucb/InteractiveFileIOException.hpp"
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
#ifndef _COM_SUN_STAR_UCB_XINTERACTIONCOOKIEHANDLING_HPP_
#include "com/sun/star/ucb/XInteractionCookieHandling.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_XINTERACTIONSUPPLYAUTHENTICATION_HPP_
#include "com/sun/star/ucb/XInteractionSupplyAuthentication.hpp"
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
#ifndef _EHDL_HXX
#include "svtools/ehdl.hxx"
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
#ifndef _EINF_HXX
#include "tools/errinf.hxx"
#endif
#ifndef _LIST_HXX
#include "tools/list.hxx"
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
#ifndef _SV_WRKWIN_HXX
#include "vcl/wrkwin.hxx"
#endif
#ifndef _VOS_MUTEX_HXX_
#include "vos/mutex.hxx"
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif
#ifndef INCLUDED_NEW
#include <new>
#define INCLUDED_NEW
#endif

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

class SimpleErrorContext: public ErrorContext
{
public:
    inline SimpleErrorContext(Window * pParent,
                              bool bHasContext,
                              UniString const & rContext)
        SAL_THROW(());

    virtual BOOL GetString(ULONG, UniString & rCtxStr);

private:
    rtl::OUString m_aContext;
    bool m_bHasContext;
};

inline SimpleErrorContext::SimpleErrorContext(Window * pParent,
                                              bool bHasContext,
                                              UniString const & rContext)
    SAL_THROW(()):
    ErrorContext(pParent),
    m_bHasContext(bHasContext),
    m_aContext(rContext)
{}

BOOL SimpleErrorContext::GetString(ULONG, UniString & rCtxStr)
{
    rCtxStr = m_aContext;
    return m_bHasContext;
}

bool
hasAbortContinuation(
    star::uno::Sequence< star::uno::Reference<
                             star::task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((star::uno::RuntimeException))
{
    for (sal_Int32 i = 0; i < rContinuations.getLength(); ++i)
        if (star::uno::Reference< star::task::XInteractionAbort >::query(
                    rContinuations[i]).
                is())
            return true;
    return false;
}

void
getAuthenticationContinuations(
    star::uno::Sequence< star::uno::Reference<
                             star::task::XInteractionContinuation > > const &
        rContinuations,
    star::uno::Reference< star::task::XInteractionAbort > * pAbort,
    star::uno::Reference< star::task::XInteractionRetry > * pRetry,
    star::uno::Reference< star::ucb::XInteractionSupplyAuthentication > *
        pSupplyAuthentication)
    SAL_THROW((star::uno::RuntimeException))
{
    for (sal_Int32 i = 0; i < rContinuations.getLength(); ++i)
    {
        if (pAbort && !pAbort->is())
        {
            *pAbort = star::uno::Reference< star::task::XInteractionAbort >(
                          rContinuations[i], star::uno::UNO_QUERY);
            if (pAbort->is())
                continue;
        }
        if (pRetry && !pRetry->is())
        {
            *pRetry = star::uno::Reference< star::task::XInteractionRetry >(
                          rContinuations[i], star::uno::UNO_QUERY);
            if (pRetry->is())
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
    }
}

bool getArgument(star::uno::Sequence< star::uno::Any > const & rArguments,
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

bool getResourceNameArgument(star::uno::Sequence< star::uno::Any > const &
                                 rArguments,
                             rtl::OUString * pValue)
    SAL_THROW(())
{
    if (!getArgument(rArguments,
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Uri")),
                     pValue))
        return false;
    // Use the resource name only for file URLs, to avoid confusion:
    //TODO! work with ucp locality concept instead of hardcoded "file"?
    if (pValue
        && pValue->matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(
                                                  "file:")))
        getArgument(rArguments,
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
    osl::MutexGuard aGuard(m_aMutex);
    m_aArguments = rArguments;
}

void SAL_CALL
UUIInteractionHandler::handle(
    star::uno::Reference< star::task::XInteractionRequest > const & rRequest)
    throw (star::uno::RuntimeException)
{
    if (!rRequest.is())
        return;

    star::uno::Any aTheRequest(rRequest->getRequest());
    star::uno::Sequence< star::uno::Reference<
                             star::task::XInteractionContinuation > >
        aContinuations(rRequest->getContinuations());

    star::ucb::AuthenticationRequest aAuthenticationRequest;
    if (aTheRequest >>= aAuthenticationRequest)
    {
        handleAuthenticationRequest(aAuthenticationRequest, aContinuations);
        return;
    }

    star::task::PasswordRequest aPasswordRequest;
    if (aTheRequest >>= aPasswordRequest)
    {
        handlePasswordRequest(aPasswordRequest, aContinuations);
        return;
    }

    star::ucb::HandleCookiesRequest aCookiesRequest;
    if (aTheRequest >>= aCookiesRequest)
    {
        handleCookiesRequest(aCookiesRequest, aContinuations);
        return;
    }

    star::task::InteractionClassification eClassification
        = star::task::InteractionClassification_ERROR;
    star::task::ClassifiedInteractionRequest aClassifiedInteractionRequest;
    if (aTheRequest >>= aClassifiedInteractionRequest)
        eClassification = aClassifiedInteractionRequest.Classification;

    enum Execute { EXECUTE_NO, EXECUTE_YES, EXECUTE_IGNORE_RESULT };

    ULONG nErrorID = ERRCODE_NONE;
    sal_uInt16 nErrorFlags = USHRT_MAX;
    bool bOverrideContext = false;
    rtl::OUString aContext;
    Execute eExecute = EXECUTE_YES;
    USHORT nButton = ERRCODE_BUTTON_CANCEL;

    star::ucb::InteractiveIOException aIOException;
    star::ucb::InteractiveNetworkException aNetworkException;
    star::ucb::InteractiveCHAOSException aCHAOSException;
    star::ucb::InteractiveBadTransferURLException aTransferException;
    star::ucb::InteractiveWrongMediumException aWrongMediumException;
    star::java::WrongJavaVersionException aWrongJavaVersionException;
    star::sync2::BadPartnershipException aBadPartnershipException;
    try
    {
        if (aTheRequest >>= aIOException)
        {
            // Due to the implementation of ErrorHandler::HandleError, IO
            // errors only display an OK button, and that button is (per
            // definition) mapped to the XInteractionAbort continuation.  So,
            // if that continuation is missing, do not handle the request:
            if (!hasAbortContinuation(aContinuations))
                return;

            nErrorFlags = ERRCODE_BUTTON_OK;
            switch (eClassification)
            {
            case star::task::InteractionClassification_ERROR:
                nErrorFlags |= ERRCODE_MSG_ERROR;
                break;

            case star::task::InteractionClassification_WARNING:
                nErrorFlags |= ERRCODE_MSG_WARNING;
                break;

            case star::task::InteractionClassification_INFO:
                nErrorFlags |= ERRCODE_MSG_INFO;
                break;

            case star::task::InteractionClassification_QUERY:
                nErrorFlags |= ERRCODE_MSG_QUERY;
                break;
            }

            eExecute = EXECUTE_IGNORE_RESULT;

            static ULONG const aID[star::ucb::IOErrorCode_WRONG_VERSION + 1]
                                  [2]
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
                    { ERRCODE_IO_DEVICENOTREADY,
                      ERRCODE_UUI_IO_DEVICENOTREADY }, // DEVICE_NOT_READY
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

            star::ucb::InteractiveFileIOException aFileIOException;
            if (aTheRequest >>= aFileIOException)
            {
                nErrorID = aID[aIOException.Code][0];
                bOverrideContext = true;

                vos::OGuard aGuard(Application::GetSolarMutex());
                std::auto_ptr< ResMgr >
                    xManager(ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(
                                                      uui)));
                UniString aTemplate(ResId(STR_ERROR_FILEIO, xManager.get()));
                aTemplate.SearchAndReplaceAscii("($URL1)",
                                                aFileIOException.FileName);
                aContext = aTemplate;
            }
            else
            {
                star::uno::Sequence< star::uno::Any > aArguments;
                star::ucb::InteractiveAugmentedIOException
                    aAugmentedIOException;
                if (aTheRequest >>= aAugmentedIOException)
                    aArguments = aAugmentedIOException.Arguments;

                //TODO! remove this backwards compatibility?
                bool bArgUri = false;
                bool bArgFolder = false;
                bool bArgVolumes = false;
                rtl::OUString aArgUri;
                rtl::OUString aArgFolder;
                rtl::OUString aArgVolume;
                rtl::OUString aArgOtherVolume;
                switch (aIOException.Code)
                {
                case star::ucb::IOErrorCode_CANT_CREATE:
                    if (aArguments.getLength() == 2
                        && (aArguments[0] >>= aArgUri)
                        && (aArguments[1] >>= aArgFolder))
                    {
                        bArgUri = true;
                        bArgFolder = true;
                        aArguments.realloc(0);
                    }
                    break;

                case star::ucb::IOErrorCode_DIFFERENT_DEVICES:
                    if (aArguments.getLength() == 2
                        && (aArguments[0] >>= aArgVolume)
                        && (aArguments[1] >>= aArgOtherVolume))
                    {
                        bArgVolumes = true;
                        aArguments.realloc(0);
                    }
                    break;

                default:
                    if (aArguments.getLength() == 1
                        && (aArguments[0] >>= aArgUri))
                    {
                        bArgUri = true;
                        aArguments.realloc(0);
                    }
                    break;
                }

                switch (aIOException.Code)
                {
                case star::ucb::IOErrorCode_CANT_CREATE:
                    nErrorID
                        = (bArgUri
                               || getResourceNameArgument(aArguments,
                                                          &aArgUri))
                          && (bArgFolder
                              || getArgument(aArguments,
                                             rtl::OUString(
                                                 RTL_CONSTASCII_USTRINGPARAM(
                                                     "Folder")),
                                             &aArgFolder)) ?
                              *new TwoStringErrorInfo(
                                       aID[aIOException.Code][1],
                                       aArgUri,
                                       aArgFolder) :
                              aID[aIOException.Code][0];
                    break;

                case star::ucb::IOErrorCode_DIFFERENT_DEVICES:
                    nErrorID
                        = bArgVolumes
                          || getArgument(aArguments,
                                         rtl::OUString(
                                             RTL_CONSTASCII_USTRINGPARAM(
                                                 "Volume")),
                                         &aArgVolume)
                                 && getArgument(
                                        aArguments,
                                        rtl::OUString(
                                            RTL_CONSTASCII_USTRINGPARAM(
                                                "OtherVolume")),
                                        &aArgOtherVolume) ?
                              *new TwoStringErrorInfo(
                                       aID[aIOException.Code][1],
                                       aArgVolume,
                                       aArgOtherVolume) :
                              aID[aIOException.Code][0];
                    break;

                case star::ucb::IOErrorCode_NOT_EXISTING:
                    if (bArgUri
                        || getResourceNameArgument(aArguments, &aArgUri))
                    {
                        rtl::OUString aResourceType;
                        getArgument(aArguments,
                                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                      "ResourceType")),
                                    &aResourceType);
                        nErrorID
                            = *new StringErrorInfo(
                                       aResourceType.
                                               equalsAsciiL(
                                                   RTL_CONSTASCII_STRINGPARAM(
                                                       "volume")) ?
                                           ERRCODE_UUI_IO_NOTEXISTS_VOLUME :
                                       aResourceType.
                                               equalsAsciiL(
                                                   RTL_CONSTASCII_STRINGPARAM(
                                                       "folder")) ?
                                           ERRCODE_UUI_IO_NOTEXISTS_FOLDER :
                                           ERRCODE_UUI_IO_NOTEXISTS,
                                       aArgUri);
                    }
                    else
                        nErrorID = aID[aIOException.Code][0];
                    break;

                default:
                    nErrorID
                        = bArgUri
                          || getResourceNameArgument(aArguments, &aArgUri) ?
                              *new StringErrorInfo(aID[aIOException.Code][1],
                                                   aArgUri) :
                              aID[aIOException.Code][0];
                    break;
                }
            }
        }
        else if (aTheRequest >>= aNetworkException)
        {
            star::ucb::InteractiveNetworkOffLineException aOffLineException;
            star::ucb::InteractiveNetworkResolveNameException
                aResolveNameException;
            star::ucb::InteractiveNetworkConnectException aConnectException;
            star::ucb::InteractiveNetworkReadException aReadException;
            star::ucb::InteractiveNetworkWriteException aWriteException;
            nErrorID
                = (aTheRequest >>= aOffLineException) ?
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
            eExecute = EXECUTE_NO;
        else if (aTheRequest >>= aWrongMediumException)
        {
            vos::OGuard aGuard(Application::GetSolarMutex());

            UniString aText;
            {
                std::auto_ptr< ResMgr >
                    xManager(ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(
                                                      uui)));
                aText = ResId(STR_ERROR_WRONGMEDIUM, xManager.get());
            }

            sal_Int32 nMedium;
            aWrongMediumException.Medium >>= nMedium;
            aText.SearchAndReplaceAscii("$$",
                                        UniString::CreateFromInt32(nMedium
                                                                       + 1));

            nButton = ErrorBox(getParentArgument(), WB_OK_CANCEL, aText).
                          Execute();
            eExecute = EXECUTE_NO;
        }
        else if (aTheRequest >>= aWrongJavaVersionException)
        {
            if (!hasAbortContinuation(aContinuations))
                return;
            eExecute = EXECUTE_IGNORE_RESULT;
            nErrorID
                = aWrongJavaVersionException.DetectedVersion.getLength()
                          == 0 ?
                      aWrongJavaVersionException.LowestSupportedVersion.
                                                     getLength()
                              == 0 ?
                          ERRCODE_UUI_WRONGJAVA :
                          static_cast< ULONG >(
                              *new StringErrorInfo(
                                       ERRCODE_UUI_WRONGJAVA_MIN,
                                       aWrongJavaVersionException.
                                           LowestSupportedVersion)) :
                      aWrongJavaVersionException.LowestSupportedVersion.
                                                     getLength()
                              == 0 ?
                          static_cast< ULONG >(
                              *new StringErrorInfo(
                                       ERRCODE_UUI_WRONGJAVA_VERSION,
                                       aWrongJavaVersionException.
                                           DetectedVersion)) :
                          static_cast< ULONG >(
                              *new TwoStringErrorInfo(
                                       ERRCODE_UUI_WRONGJAVA_VERSION_MIN,
                                       aWrongJavaVersionException.
                                           DetectedVersion,
                                       aWrongJavaVersionException.
                                           LowestSupportedVersion));
        }
        else if (aTheRequest >>= aBadPartnershipException)
        {
            if (!hasAbortContinuation(aContinuations))
                return;
            eExecute = EXECUTE_IGNORE_RESULT;
            nErrorID = aBadPartnershipException.Partnership.getLength() == 0 ?
                           ERRCODE_UUI_BADPARTNERSHIP :
                           *new StringErrorInfo(
                                    ERRCODE_UUI_BADPARTNERSHIP_NAME,
                                    aBadPartnershipException.Partnership);
        }
    }
    catch (std::bad_alloc const &)
    {
        throw star::uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  *this);
    }

    if (eExecute != EXECUTE_NO)
    {
        USHORT nResult = executeErrorDialog(nErrorID,
                                            nErrorFlags,
                                            bOverrideContext,
                                            aContext);
        if (eExecute != EXECUTE_IGNORE_RESULT)
            nButton = nResult;
    }

    switch (nButton)
    {
    case ERRCODE_BUTTON_OK:
    case ERRCODE_BUTTON_YES:
        {for (sal_Int32 i = 0; i < aContinuations.getLength(); ++i)
        {
            star::uno::Reference< star::task::XInteractionApprove >
                xApprove(aContinuations[i], star::uno::UNO_QUERY);
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
            star::uno::Reference< star::task::XInteractionAbort >
                xAbort(aContinuations[i], star::uno::UNO_QUERY);
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
            star::uno::Reference< star::task::XInteractionRetry >
                xRetry(aContinuations[i], star::uno::UNO_QUERY);
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
            star::uno::Reference< star::task::XInteractionDisapprove >
                xDisapprove(aContinuations[i], star::uno::UNO_QUERY);
            if (xDisapprove.is())
            {
                xDisapprove->select();
                break;
            }
        }}
        break;
    }
}

Window * UUIInteractionHandler::getParentArgument() SAL_THROW(())
{
    osl::MutexGuard aGuard(m_aMutex);
    for (sal_Int32 i = 0; i < m_aArguments.getLength(); ++i)
    {
        star::beans::PropertyValue aArgument;
        if ((m_aArguments[i] >>= aArgument)
            && aArgument.
                   Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Parent")))
        {
            star::uno::Reference< star::awt::XWindow > xWindow;
            aArgument.Value >>= xWindow;
            return VCLUnoHelper::GetWindow(xWindow);
        }
    }
    return 0;
}

bool UUIInteractionHandler::getContextArgument(rtl::OUString * pContext)
    SAL_THROW(())
{
    OSL_ENSURE(pContext, "specification violation");
    osl::MutexGuard aGuard(m_aMutex);
    for (sal_Int32 i = 0; i < m_aArguments.getLength(); ++i)
    {
        star::beans::PropertyValue aArgument;
        if ((m_aArguments[i] >>= aArgument)
            && aArgument.
                   Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Context")))
        {
            aArgument.Value >>= *pContext;
            return true;
        }
    }
    return false;
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

USHORT
UUIInteractionHandler::executeErrorDialog(ULONG nID,
                                          USHORT nMask,
                                          bool bOverrideContext,
                                          rtl::OUString const & rContext)
    SAL_THROW((star::uno::RuntimeException))
{
    try
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        std::auto_ptr< ResMgr >
            xManager1(ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(ofa)));
        std::auto_ptr< SfxErrorHandler >
            xHandler1(new SfxErrorHandler(RID_ERRHDL,
                                          ERRCODE_AREA_TOOLS,
                                          ERRCODE_AREA_LIB1 - 1,
                                          xManager1.get()));
        std::auto_ptr< ResMgr >
            xManager2(ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(cnt)));
        std::auto_ptr< SfxErrorHandler >
            xHandler2(new SfxErrorHandler(RID_CHAOS_START + 12,
                                          ERRCODE_AREA_CHAOS,
                                          ERRCODE_AREA_CHAOS_END,
                                          xManager2.get()));
            // cf. chaos/source/inc/cntrids.hrc, where
            // #define RID_CHAOS_ERRHDL (RID_CHAOS_START + 12)
        std::auto_ptr< ResMgr >
            xManager3(ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        std::auto_ptr< SfxErrorHandler >
            xHandler3(new SfxErrorHandler(RID_UUI_ERRHDL,
                                          ERRCODE_AREA_UUI,
                                          ERRCODE_AREA_UUI_END,
                                          xManager3.get()));

        bool bContext;
        rtl::OUString aContext;
        if (bOverrideContext)
        {
            bContext = true;
            aContext = rContext;
        }
        else
            bContext = getContextArgument(&aContext);
        std::auto_ptr< ErrorContext >
            xContext(new SimpleErrorContext(getParentArgument(),
                                            bContext,
                                            aContext));

        // Needed because within ErrorHandler::HandleError() ResIds are
        // created without a ResMgr---they require a default ResMgr:
        ResMgr * pDefaultManager = Resource::GetResManager();
        Resource::SetResManager(xManager1.get());
        USHORT nButton = ErrorHandler::HandleError(nID, nMask);
        Resource::SetResManager(pDefaultManager);
        return nButton;
    }
    catch (std::bad_alloc const &)
    {
        throw star::uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  *this);
    }
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
            xDialog(new LoginDialog(getParentArgument(),
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
UUIInteractionHandler::executePasswordDialog(LoginErrorInfo & rInfo,
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
        std::auto_ptr< MasterPasswordDialog >
            xDialog(new MasterPasswordDialog(
                            getParentArgument(), nMode, xManager.get()));
        rInfo.SetResult(xDialog->Execute() == RET_OK ? ERRCODE_BUTTON_OK :
                                                       ERRCODE_BUTTON_CANCEL);
        aMaster = rtl::OUStringToOString(xDialog->GetMasterPassword(),
                                         RTL_TEXTENCODING_UTF8);
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
                            getParentArgument(), &rRequest, xManager.get()));
        xDialog->Execute();
    }
    catch (std::bad_alloc const &)
    {
        throw star::uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  *this);
    }
}

void
UUIInteractionHandler::handleAuthenticationRequest(
    star::ucb::AuthenticationRequest const & rRequest,
    star::uno::Sequence< star::uno::Reference<
                             star::task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((star::uno::RuntimeException))
{
    star::uno::Reference< star::task::XInteractionAbort > xAbort;
    star::uno::Reference< star::task::XInteractionRetry > xRetry;
    star::uno::Reference< star::ucb::XInteractionSupplyAuthentication >
        xSupplyAuthentication;
    getAuthenticationContinuations(
        rContinuations, &xAbort, &xRetry, &xSupplyAuthentication);
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
UUIInteractionHandler::handlePasswordRequest(
    star::task::PasswordRequest const & rRequest,
    star::uno::Sequence< star::uno::Reference<
                             star::task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((star::uno::RuntimeException))
{
    star::uno::Reference< star::task::XInteractionAbort > xAbort;
    star::uno::Reference< star::task::XInteractionRetry > xRetry;
    star::uno::Reference< star::ucb::XInteractionSupplyAuthentication >
        xSupplyAuthentication;
    getAuthenticationContinuations(
        rContinuations, &xAbort, &xRetry, &xSupplyAuthentication);
    LoginErrorInfo aInfo;
    executePasswordDialog(aInfo, rRequest.Mode);
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

sal_Char const UUIInteractionHandler::m_aImplementationName[]
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
