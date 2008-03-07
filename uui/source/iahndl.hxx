/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: iahndl.hxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 12:25:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef UUI_IAHNDL_HXX
#define UUI_IAHNDL_HXX

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

#ifndef _OSL_MUTEX_HXX_
#include "osl/mutex.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_OPTIONAL_HPP_
#include "com/sun/star/beans/Optional.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif
#ifndef _COM_SUN_STAR_TASK_INTERACTIONCLASSIFICATION_HPP_
#include "com/sun/star/task/InteractionClassification.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_PASSWORDREQUESTMODE_HPP_
#include "com/sun/star/task/PasswordRequestMode.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_DOCUMENTMACROCONFIRMATIONREQUEST_HPP_
#include "com/sun/star/task/DocumentMacroConfirmationRequest.hpp"
#endif
#ifndef _SOLAR_H
#include "tools/solar.h"
#endif
#ifndef _ERRCODE_HXX
#include "tools/errcode.hxx"
#endif
#ifndef _SV_WINTYPES_HXX
#include "vcl/wintypes.hxx"
#endif

#ifndef UUI_FLTDLG_HXX
#include "fltdlg.hxx"
#endif

#ifndef _COM_SUN_STAR_SECURITY_XCERTIFICATE_HPP_
#include <com/sun/star/security/XCertificate.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_XXSECURITYENVIRONMENT_HPP_
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#endif

class Window;
class LoginErrorInfo;
struct CntHTTPCookieRequest;

#define DESCRIPTION_1 1
#define DESCRIPTION_2 2
#define TITLE 3

namespace cssu = com::sun::star::uno;
namespace dcss = ::com::sun::star;

namespace com { namespace sun { namespace star {
    namespace document {
        class AmbigousFilterRequest;
        class FilterOptionsRequest;
        class NoSuchFilterRequest;
        class LockedDocumentRequest;
    }
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace task {
        class XInteractionContinuation;
        class XInteractionHandler;
        class XInteractionRequest;
        class XPasswordContainer;
    }
    namespace ucb {
        class AuthenticationRequest;
        class HandleCookiesRequest;
        class NameClashResolveRequest;
        class CertificateValidationRequest;
    }
    namespace uno {
        class RuntimeException;
    }
} } }

class UUIInteractionHelper
{
private:
    osl::Mutex m_aPropertyMutex;
    com::sun::star::uno::Reference<
    com::sun::star::lang::XMultiServiceFactory >
    m_xServiceFactory;
    com::sun::star::uno::Sequence< com::sun::star::uno::Any > m_aProperties;

    UUIInteractionHelper(UUIInteractionHelper &); // not implemented
    void operator =(UUIInteractionHelper); // not implemented

public:
    UUIInteractionHelper(com::sun::star::uno::Reference<
                 com::sun::star::lang::XMultiServiceFactory >
                         const & rServiceFactory,
                         com::sun::star::uno::Sequence<
                 com::sun::star::uno::Any > const & rArguments)
        SAL_THROW(());
    UUIInteractionHelper(com::sun::star::uno::Reference<
                 com::sun::star::lang::XMultiServiceFactory >
                         const & rServiceFactory)
        SAL_THROW(());

    ~UUIInteractionHelper() SAL_THROW(());

    void
    handleRequest(com::sun::star::uno::Reference<
              com::sun::star::task::XInteractionRequest > const &
                  rRequest)
        throw (com::sun::star::uno::RuntimeException);

    com::sun::star::beans::Optional< rtl::OUString >
    getStringFromRequest(com::sun::star::uno::Reference<
                 com::sun::star::task::XInteractionRequest > const &
                         rRequest)
        throw (com::sun::star::uno::RuntimeException);

private:
    void
    handle_impl(com::sun::star::uno::Reference<
                    com::sun::star::task::XInteractionRequest > const &
                rRequest)
        throw (com::sun::star::uno::RuntimeException);


    sal_Bool
    isDomainMatch( rtl::OUString hostName, rtl::OUString certHostName);
    static long
    handlerequest(void* pHandleData, void* pInteractionHandler);

    com::sun::star::beans::Optional< rtl::OUString >
    getStringFromRequest_impl(com::sun::star::uno::Reference<
                    com::sun::star::task::XInteractionRequest > const &
                rRequest)
        throw (com::sun::star::uno::RuntimeException);

    static long
    getstringfromrequest(void* pHandleData, void* pInteractionHandler);

    Window * getParentProperty() SAL_THROW(());

    rtl::OUString getContextProperty() SAL_THROW(());

    bool
    initPasswordContainer(com::sun::star::uno::Reference<
                  com::sun::star::task::XPasswordContainer > *
              pContainer)
        const SAL_THROW(());

    com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >
    getInteractionHandler() const
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void executeLoginDialog(LoginErrorInfo & rInfo,
                            rtl::OUString const & rRealm)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    executeMasterPasswordDialog(LoginErrorInfo & rInfo,
                                com::sun::star::task::PasswordRequestMode nMode)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    executePasswordDialog(LoginErrorInfo & rInfo,
                          com::sun::star::task::PasswordRequestMode nMode,
              ::rtl::OUString aDocumentName)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void executeCookieDialog(CntHTTPCookieRequest & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void executeFilterDialog(rtl::OUString       const & rURL    ,
                             uui::FilterNameList const & rFilters,
                             rtl::OUString             & rFilter )
        SAL_THROW((com::sun::star::uno::RuntimeException));

    sal_Bool executeUnknownAuthDialog( const cssu::Reference< dcss::security::XCertificate >& rXCert )
        SAL_THROW((com::sun::star::uno::RuntimeException));

    sal_Bool executeSSLWarnDialog( const cssu::Reference< dcss::security::XCertificate >& rXCert,
                                   sal_Int32 const & failures,
                                   const rtl::OUString & hostName)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    rtl::OUString
    getLocalizedDatTimeStr( ::com::sun::star::util::DateTime aDateTime );

    USHORT
    executeErrorDialog(com::sun::star::task::InteractionClassification
                   eClassification,
                       rtl::OUString const & rContext,
                       rtl::OUString const & rMessage,
                       WinBits nButtonMask )
        SAL_THROW((com::sun::star::uno::RuntimeException));

    USHORT
    executeMessageBox( rtl::OUString const & rTitle,
                       rtl::OUString const & rMessage,
                       WinBits nButtonMask )
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleAuthenticationRequest(
        com::sun::star::ucb::AuthenticationRequest const & rRequest,
        com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
    rContinuations)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleCertificateValidationRequest(
        com::sun::star::ucb::CertificateValidationRequest const & rRequest,
        com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
    rContinuations)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleNameClashResolveRequest(
        com::sun::star::ucb::NameClashResolveRequest const & rRequest,
        com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
        rContinuations)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleMasterPasswordRequest(
        com::sun::star::task::PasswordRequestMode nMode,
        com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
    rContinuations)
        SAL_THROW((com::sun::star::uno::RuntimeException));


    void
    handlePasswordRequest(
        com::sun::star::task::PasswordRequestMode nMode,
        com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
    rContinuations,
    ::rtl::OUString aDocumentName = ::rtl::OUString())
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleCookiesRequest(
        com::sun::star::ucb::HandleCookiesRequest const & rRequest,
        com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
    rContinuations)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleNoSuchFilterRequest(
        com::sun::star::document::NoSuchFilterRequest const & rRequest,
        com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
    rContinuations)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleAmbigousFilterRequest(
        com::sun::star::document::AmbigousFilterRequest const & rRequest,
        com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
    rContinuations)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleFilterOptionsRequest(
        com::sun::star::document::FilterOptionsRequest const & rRequest,
        com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
    rContinuations)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleErrorRequest(
        com::sun::star::task::InteractionClassification eClassification,
        ErrCode nErrorCode,
        std::vector< rtl::OUString > const & rArguments,
        com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
    rContinuations,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    rtl::OUString & rErrorString)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleGenericErrorRequest(
    sal_Int32 nErrorCode,
        com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionContinuation > > const &
        rContinuations,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    rtl::OUString & rErrorString)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleMacroConfirmRequest(
        const ::com::sun::star::task::DocumentMacroConfirmationRequest& _rRequest,
        com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference<
                com::sun::star::task::XInteractionContinuation > > const &
                    rContinuations
    )
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleBrokenPackageRequest(
    std::vector< rtl::OUString > const & rArguments,
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::task::XInteractionContinuation > > const &
    rContinuations,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    rtl::OUString & rErrorString)
        SAL_THROW((::com::sun::star::uno::RuntimeException));

    void handleMessageboxRequests(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::task::XInteractionRequest > const &
    rRequest,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    rtl::OUString & rErrorString);

    void handleDialogRequests(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::task::XInteractionRequest > const &
    rRequest);

    void handleErrorHandlerRequests(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::task::XInteractionRequest > const &
    rRequest,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    rtl::OUString & rErrorString);

    void handleLockedDocumentRequest(
    ::com::sun::star::document::LockedDocumentRequest const & aRequest,
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::task::XInteractionContinuation > > const &
            rContinuations )
        SAL_THROW((::com::sun::star::uno::RuntimeException));
};

#endif // UUI_IAHNDL_HXX
