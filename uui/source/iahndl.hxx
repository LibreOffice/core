/*************************************************************************
 *
 *  $RCSfile: iahndl.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:44:57 $
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
#define UUI_IAHNDL_HXX

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include "com/sun/star/lang/XInitialization.hpp"
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include "com/sun/star/lang/XServiceInfo.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_INTERACTIONCLASSIFICATION_HPP_
#include "com/sun/star/task/InteractionClassification.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_PASSWORDREQUESTMODE_HPP_
#include "com/sun/star/task/PasswordRequestMode.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include "com/sun/star/task/XInteractionHandler.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include "com/sun/star/uno/Exception.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include "com/sun/star/uno/RuntimeException.hpp"
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_NOSUCHFILTERREQUEST_HPP_
#include "com/sun/star/document/NoSuchFilterRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_AMBIGOUSFILTERREQUEST_HPP_
#include "com/sun/star/document/AmbigousFilterRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_ERRORCODEREQUEST_HPP_
#include "com/sun/star/task/ErrorCodeRequest.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include "cppuhelper/implbase3.hxx"
#endif
#ifndef _OSL_MUTEX_HXX_
#include "osl/mutex.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _ERRCODE_HXX
#include "tools/errcode.hxx"
#endif
#ifndef _SOLAR_H
#include "tools/solar.h"
#endif
#ifndef _SV_WINTYPES_HXX
#include "vcl/wintypes.hxx"
#endif
#ifndef UUI_FLTDLG_HXX
#include "fltdlg.hxx"
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace com { namespace sun { namespace star {
    namespace lang { class XMulitServiceFactory; }
    namespace task {
        class PasswordRequest;
        class XInteractionContinuation;
        class XPasswordContainer;
    }
    namespace ucb {
        class AuthenticationRequest;
        class HandleCookiesRequest;
    }
    namespace uno {
        class Any;
        class XInterface;
    }
    namespace document {
        class FilterOptionsRequest;
    }
} } }
namespace rtl { class OUString; }
struct CntHTTPCookieRequest;
class LoginErrorInfo;
class Window;

class UUIInteractionHandler:
    public cppu::WeakImplHelper3< com::sun::star::lang::XServiceInfo,
                                  com::sun::star::lang::XInitialization,
                                  com::sun::star::task::XInteractionHandler >
{
public:
    static char const m_aImplementationName[];

    static com::sun::star::uno::Sequence< rtl::OUString >
    getSupportedServiceNames_static();

    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL
    createInstance(
        com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory > const &
            rServiceFactory)
        SAL_THROW((com::sun::star::uno::Exception));

private:
    osl::Mutex m_aPropertyMutex;
    com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory >
        m_xServiceFactory;
    com::sun::star::uno::Sequence< com::sun::star::uno::Any > m_aProperties;

    UUIInteractionHandler(UUIInteractionHandler &); // not implemented
    void operator =(UUIInteractionHandler); // not implemented

    UUIInteractionHandler(com::sun::star::uno::Reference<
                                  com::sun::star::lang::XMultiServiceFactory >
                              const & rServiceFactory)
        SAL_THROW(());

    virtual ~UUIInteractionHandler() SAL_THROW(());

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const &
                                                  rServiceName)
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    initialize(
        com::sun::star::uno::Sequence< com::sun::star::uno::Any > const &
            rArguments)
        throw (com::sun::star::uno::Exception);

    virtual void SAL_CALL
    handle(com::sun::star::uno::Reference<
                   com::sun::star::task::XInteractionRequest > const &
               rRequest)
        throw (com::sun::star::uno::RuntimeException);

    Window * getParentProperty() SAL_THROW(());

    rtl::OUString getContextProperty() SAL_THROW(());

    bool
    initPasswordContainer(com::sun::star::uno::Reference<
                                  com::sun::star::task::XPasswordContainer > *
                              pContainer)
        const SAL_THROW(());

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
            rContinuations )
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleGenericErrorRequest(
        com::sun::star::task::ErrorCodeRequest const & rRequest,
        com::sun::star::uno::Sequence<
                com::sun::star::uno::Reference<
                    com::sun::star::task::XInteractionContinuation > > const &
            rContinuations)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleBrokenPackageRequest(
        std::vector< rtl::OUString > const & rArguments,
        ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::task::XInteractionContinuation > > const &
            rContinuations)
        SAL_THROW((::com::sun::star::uno::RuntimeException));

};

#endif // UUI_IAHNDL_HXX
