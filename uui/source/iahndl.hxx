/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: iahndl.hxx,v $
 * $Revision: 1.21.22.1 $
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

#ifndef UUI_IAHNDL_HXX
#define UUI_IAHNDL_HXX

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif
#include "osl/mutex.hxx"
#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/task/InteractionClassification.hpp"
#include "com/sun/star/task/PasswordRequestMode.hpp"
#include "com/sun/star/task/DocumentMacroConfirmationRequest.hpp"
#include "com/sun/star/task/FutureDocumentVersionProductUpdateRequest.hpp"
#include "tools/solar.h"
#include "tools/errcode.hxx"
#include "vcl/wintypes.hxx"
#include "fltdlg.hxx"
#include <com/sun/star/security/XCertificate.hpp>
#ifndef _COM_SUN_STAR_XML_CRYPTO_XXSECURITYENVIRONMENT_HPP_
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#endif

class Window;
class LoginErrorInfo;
struct CntHTTPCookieRequest;

#define DESCRIPTION_1 1
#define DESCRIPTION_2 2
#define TITLE 3

#define UUI_DOC_LOAD_LOCK       0
#define UUI_DOC_OWN_LOAD_LOCK   1
#define UUI_DOC_SAVE_LOCK       2
#define UUI_DOC_OWN_SAVE_LOCK   3

//============================================================================
/** Information about a InteractionHandler
 */
struct InteractionHandlerData
{
    /** The UNO service name to use to instanciate the content provider.
     */
    rtl::OUString ServiceName;

    InteractionHandlerData() {};
    InteractionHandlerData( const rtl::OUString & rService)
    : ServiceName( rService ){}
};

typedef std::vector< InteractionHandlerData > InteractionHandlerDataList;

namespace cssu = com::sun::star::uno;
namespace dcss = ::com::sun::star;

namespace com { namespace sun { namespace star {
    namespace document {
        class AmbigousFilterRequest;
        class FilterOptionsRequest;
        class NoSuchFilterRequest;
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

    bool
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
    bool
    handle_impl(com::sun::star::uno::Reference<
                    com::sun::star::task::XInteractionRequest > const &
                rRequest)
        throw (com::sun::star::uno::RuntimeException);

    void
    GetInteractionHandlerList(InteractionHandlerDataList &rdataList);

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

    void
    executeMSPasswordDialog(LoginErrorInfo & rInfo,
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
    handleMSPasswordRequest(
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
    handleFutureDocumentVersionUpdateRequest(
        const ::com::sun::star::task::FutureDocumentVersionProductUpdateRequest& _rRequest,
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

    bool handleMessageboxRequests(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::task::XInteractionRequest > const &
    rRequest,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    rtl::OUString & rErrorString);

    bool handleDialogRequests(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::task::XInteractionRequest > const &
    rRequest);

    bool handleErrorHandlerRequests(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::task::XInteractionRequest > const &
    rRequest,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    rtl::OUString & rErrorString);

    void handleLockedDocumentRequest(
    const ::rtl::OUString& aDocumentURL,
    const ::rtl::OUString& aInfo,
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::task::XInteractionContinuation > > const &
            rContinuations,
    sal_uInt16 nMode )
        SAL_THROW((::com::sun::star::uno::RuntimeException));

    void handleChangedByOthersRequest(
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::task::XInteractionContinuation > > const &
            rContinuations )
        SAL_THROW((::com::sun::star::uno::RuntimeException));

    void handleLockFileIgnoreRequest(
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::task::XInteractionContinuation > > const &
            rContinuations )
        SAL_THROW((::com::sun::star::uno::RuntimeException));

};

#endif // UUI_IAHNDL_HXX
