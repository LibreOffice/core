/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef UUI_IAHNDL_HXX
#define UUI_IAHNDL_HXX

#include <vector>

#include "osl/mutex.hxx"
#include "rtl/ustring.hxx"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"

#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/task/InteractionClassification.hpp"

#include "tools/solar.h" // sal_uInt16
#include "tools/errcode.hxx" // ErrCode
#include "tools/rc.hxx" // Resource
#include "tools/wintypes.hxx" // WinBits

namespace com { namespace sun { namespace star {
    namespace awt {
        class XWindow;
    }
    namespace embed {
        class XStorage;
    }
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace security {
        struct DocumentSignatureInformation;
    }
    namespace task {
        class FutureDocumentVersionProductUpdateRequest;
        class XInteractionContinuation;
        class XInteractionHandler;
        class XInteractionRequest;
    }
    namespace ucb {
        class NameClashResolveRequest;
    }
} } }

#include <hash_map>

class Window;

//============================================================================
struct InteractionHandlerData
{
    /** The UNO service name to use to instanciate the content provider.
     */
    rtl::OUString ServiceName;

    InteractionHandlerData() {};
    InteractionHandlerData(const rtl::OUString & rService)
    : ServiceName( rService ){}
};

typedef std::vector< InteractionHandlerData > InteractionHandlerDataList;

typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > Continuations;

typedef ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash >    StringHashMap;

//============================================================================
class UUIInteractionHelper
{
private:
    mutable osl::Mutex                                                                      m_aPropertyMutex;
            ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >  m_xServiceFactory;
            ::com::sun::star::uno::Sequence< com::sun::star::uno::Any >                     m_aProperties;
            StringHashMap                                                                   m_aTypedCustomHandlers;
    UUIInteractionHelper(UUIInteractionHelper &); // not implemented
    void operator =(UUIInteractionHelper); // not implemented

public:
    UUIInteractionHelper(
        com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > const & rServiceFactory,
        com::sun::star::uno::Sequence<
            com::sun::star::uno::Any > const & rArguments)
        SAL_THROW(());
    UUIInteractionHelper(
        com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > const & rServiceFactory)
        SAL_THROW(());

    ~UUIInteractionHelper() SAL_THROW(());

    bool
    handleRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    com::sun::star::beans::Optional< rtl::OUString >
    getStringFromRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    // Helper.
    static ::rtl::OUString
    replaceMessageWithArguments(
        ::rtl::OUString aMessage,
            std::vector< rtl::OUString > const & rArguments );

private:
    bool
    handleRequest_impl(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest,
        bool bObtainErrorStringOnly,
        bool & bHasErrorString,
        rtl::OUString & rErrorString)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    static long
    handlerequest(void* pHandleData, void* pInteractionHandler);

    com::sun::star::beans::Optional< rtl::OUString >
    getStringFromRequest_impl(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    static long
    getstringfromrequest(void* pHandleData, void* pInteractionHandler);

    Window *
    getParentProperty()
        SAL_THROW(());

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>
    getParentXWindow() const
        SAL_THROW(());

    rtl::OUString
    getContextProperty()
        SAL_THROW(());

    com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >
    getInteractionHandler()
        SAL_THROW((com::sun::star::uno::RuntimeException));

    bool    handleTypedHandlerImplementations(
                ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest > const &  rRequest
            );

    bool
    tryOtherInteractionHandler(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    getInteractionHandlerList(InteractionHandlerDataList &rdataList)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    static bool
    isInformationalErrorMessageRequest(
        com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference<
                com::sun::star::task::XInteractionContinuation > > const &
            rContinuations);

    //=====================================================================

    bool
    handleInteractiveIOException(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest,
        bool bObtainErrorStringOnly,
        bool & bHasErrorString,
        rtl::OUString & rErrorString)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    bool
    handleAuthenticationRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    bool
    handleCertificateValidationRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleNameClashResolveRequest(
        com::sun::star::ucb::NameClashResolveRequest const & rRequest,
        com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference<
                com::sun::star::task::XInteractionContinuation > > const &
                    rContinuations)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    bool
    handleMasterPasswordRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    bool
    handlePasswordRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    bool
    handleCookiesRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    bool
    handleNoSuchFilterRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    bool
    handleAmbigousFilterRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    bool
    handleFilterOptionsRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleErrorHandlerRequest(
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
        const ::rtl::OUString& aDocumentURL,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XStorage >& xZipStorage,
        const ::rtl::OUString& aDocumentVersion,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::security::DocumentSignatureInformation >
                aSignInfo,
        com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference<
                com::sun::star::task::XInteractionContinuation > > const &
                    rContinuations)
        SAL_THROW((com::sun::star::uno::RuntimeException));

    void
    handleFutureDocumentVersionUpdateRequest(
        const ::com::sun::star::task::FutureDocumentVersionProductUpdateRequest&
            _rRequest,
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
                    rContinuations,
        bool bObtainErrorStringOnly,
        bool & bHasErrorString,
        rtl::OUString & rErrorString)
        SAL_THROW((::com::sun::star::uno::RuntimeException));

    bool handleLockedDocumentRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((::com::sun::star::uno::RuntimeException));

    bool handleChangedByOthersRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((::com::sun::star::uno::RuntimeException));

    bool handleLockFileIgnoreRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
        SAL_THROW((::com::sun::star::uno::RuntimeException));

    bool    handleCustomRequest(
                const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& i_rRequest,
                const ::rtl::OUString& i_rServiceName
            ) const;
};

class ErrorResource: private Resource
{
public:
    inline ErrorResource(ResId & rResId) SAL_THROW(()): Resource(rResId) {}

    inline ~ErrorResource() SAL_THROW(()) { FreeResource(); }

    bool getString(ErrCode nErrorCode, rtl::OUString * pString) const
        SAL_THROW(());
};

/*
class InteractionRequest
{
public:
    InteractionRequest(
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest > const & rRequest)
    : m_aRequest( rRequest ) {}

    virtual bool toString( rtl::OUString & rString ) = 0;
    virtual bool handle( rtl::OUString & rString ) = 0;

private:
    com::sun::star::uno::Reference<
        com::sun::star::task::XInteractionRequest > m_aRequest;
};
*/

#endif // UUI_IAHNDL_HXX
