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

#include <memory>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/configuration/backend/MergeRecoveryRequest.hpp>
#include <com/sun/star/configuration/backend/StratumCreationException.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/document/BrokenPackageRequest.hpp>
#include <com/sun/star/document/ExoticFileLoadException.hpp>
#include <com/sun/star/task/DocumentMacroConfirmationRequest.hpp>
#include <com/sun/star/java/WrongJavaVersionException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/script/ModuleSizeExceededRequest.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/task/ErrorCodeRequest2.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/ucb/AuthenticationFallbackRequest.hpp>
#include <com/sun/star/ucb/InteractiveAppException.hpp>
#include <com/sun/star/ucb/InteractiveLockingLockedException.hpp>
#include <com/sun/star/ucb/InteractiveLockingNotLockedException.hpp>
#include <com/sun/star/ucb/InteractiveLockingLockExpiredException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkConnectException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkOffLineException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkReadException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkResolveNameException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkWriteException.hpp>
#include <com/sun/star/ucb/InteractiveWrongMediumException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/NameClashResolveRequest.hpp>
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/XInteractionReplaceExistingData.hpp>
#include <com/sun/star/ucb/XInteractionSupplyName.hpp>
#include <com/sun/star/xforms/InvalidDataOnSubmitException.hpp>
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>

#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/conditn.hxx>
#include <unotools/resmgr.hxx>
#include <utility>
#include <vcl/errinf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/propertysequence.hxx>
#include <svtools/sfxecode.hxx>
#include <unotools/configmgr.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <typelib/typedescription.hxx>
#include <unotools/confignode.hxx>

#include <ids.hxx>
#include <ids.hrc>
#include <strings.hrc>

#include "getcontinuations.hxx"
#include "secmacrowarnings.hxx"

#include "iahndl.hxx"
#include "nameclashdlg.hxx"
#include <comphelper/string.hxx>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::task::XInteractionContinuation;
using ::com::sun::star::task::XInteractionAbort;
using ::com::sun::star::task::XInteractionApprove;
using ::com::sun::star::lang::XInitialization;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::task::InteractionHandler;
using ::com::sun::star::task::XInteractionHandler2;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Any;
using ::com::sun::star::task::XInteractionRequest;

using namespace ::com::sun::star;

namespace {

class HandleData : public osl::Condition
{
public:
    explicit HandleData(
        uno::Reference< task::XInteractionRequest > xRequest)
        : m_rRequest(std::move(xRequest)),
          bHandled( false )
    {
    }
    uno::Reference< task::XInteractionRequest > m_rRequest;
    bool                                        bHandled;
    beans::Optional< OUString >            m_aResult;
};

} /* namespace */

UUIInteractionHelper::UUIInteractionHelper(
    uno::Reference< uno::XComponentContext > xContext)
    :  m_xContext(std::move(xContext))
{
}

UUIInteractionHelper::~UUIInteractionHelper()
{
}

void UUIInteractionHelper::handlerequest(
    void* pHandleData, void* pInteractionHelper)
{
    HandleData* pHND
        = static_cast< HandleData * >(pHandleData);
    UUIInteractionHelper* pUUI
        = static_cast< UUIInteractionHelper * >(pInteractionHelper);
    bool bDummy = false;
    OUString aDummy;
    try
    {
        pHND->bHandled
            = pUUI->handleRequest_impl(pHND->m_rRequest, false, bDummy, aDummy);
        pHND->set();
    }
    catch (css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("uui", "");
    }
}

bool
UUIInteractionHelper::handleRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    if(!Application::IsMainThread() && GetpApp())
    {
        // we are not in the main thread, let it handle that stuff
        HandleData aHD(rRequest);
        Link<void*,void> aLink(&aHD,handlerequest);
        Application::PostUserEvent(aLink,this);
        comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
        sal_uInt32 nLockCount = (rSolarMutex.IsCurrentThread()) ? rSolarMutex.release(true) : 0;
        aHD.wait();
        if (nLockCount)
            rSolarMutex.acquire(nLockCount);
        return aHD.bHandled;
    }
    else
    {
        bool bDummy = false;
        OUString aDummy;
        return handleRequest_impl(rRequest, false, bDummy, aDummy);
    }
}

void UUIInteractionHelper::getstringfromrequest(
    void* pHandleData,void* pInteractionHelper)
{
    HandleData* pHND = static_cast<HandleData*>(pHandleData);
    UUIInteractionHelper* pUUI = static_cast<UUIInteractionHelper*>(pInteractionHelper);
    pHND->m_aResult = pUUI->getStringFromRequest_impl(pHND->m_rRequest);
    pHND->set();
}

beans::Optional< OUString >
UUIInteractionHelper::getStringFromRequest_impl(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    bool bSuccess = false;
    OUString aMessage;
    handleRequest_impl(rRequest, true, bSuccess, aMessage);

    OSL_ENSURE(bSuccess ||
               !isInformationalErrorMessageRequest(
                   rRequest->getContinuations()),
               "Interaction request is a candidate for a string representation."
               "Please implement!");

    return beans::Optional< OUString >(bSuccess, aMessage);
}

beans::Optional< OUString >
UUIInteractionHelper::getStringFromRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    if(!Application::IsMainThread() && GetpApp())
    {
        // we are not in the main thread, let it handle that stuff
        HandleData aHD(rRequest);
        Link<void*,void> aLink(&aHD,getstringfromrequest);
        Application::PostUserEvent(aLink,this);
        comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
        sal_uInt32 nLockCount = (rSolarMutex.IsCurrentThread()) ? rSolarMutex.release(true) : 0;
        aHD.wait();
        if (nLockCount)
            rSolarMutex.acquire(nLockCount);
        return aHD.m_aResult;
    }
    else
        return getStringFromRequest_impl(rRequest);
}

OUString
UUIInteractionHelper::replaceMessageWithArguments(
    const OUString& _aMessage,
    std::vector< OUString > const & rArguments )
{
    OUString aMessage = _aMessage;

    SAL_WARN_IF(rArguments.empty(), "uui", "replaceMessageWithArguments: No arguments passed!");
    for (size_t i = 0; i < rArguments.size(); ++i)
    {
        const OUString sReplaceTemplate = "$(ARG" + OUString::number(i+1) + ")";
        aMessage = aMessage.replaceAll(sReplaceTemplate, rArguments[i]);
    }

    return aMessage;
}

bool
UUIInteractionHelper::isInformationalErrorMessageRequest(
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations)
{
    // Only requests with a single continuation (user has no choice, request
    // is just informational)
    if (rContinuations.getLength() != 1 )
        return false;

    // user can only abort or approve, all other continuations are not
    // considered to be informational.
    uno::Reference< task::XInteractionApprove > xApprove(
        rContinuations[0], uno::UNO_QUERY);
    if (xApprove.is())
        return true;

    uno::Reference< task::XInteractionAbort > xAbort(
        rContinuations[0], uno::UNO_QUERY);
    return xAbort.is();
}

namespace
{

    bool lcl_matchesRequest( const Any& i_rRequest, const OUString& i_rTypeName, std::u16string_view i_rPropagation )
    {
        const css::uno::TypeDescription aTypeDesc( i_rTypeName );
        const typelib_TypeDescription* pTypeDesc = aTypeDesc.get();
        if ( !pTypeDesc || !pTypeDesc->pWeakRef )
        {
            SAL_WARN( "uui","no type found for '" << i_rTypeName << "'" );
            return false;
        }
        const css::uno::Type aType( pTypeDesc->pWeakRef );

        const bool bExactMatch = i_rPropagation == u"named-only";
        if ( bExactMatch )
            return i_rRequest.getValueType().equals( aType );

        return i_rRequest.isExtractableTo( aType );
    }
}


bool UUIInteractionHelper::handleCustomRequest( const Reference< XInteractionRequest >& i_rRequest, const OUString& i_rServiceName ) const
{
    try
    {
        Reference< XInteractionHandler2 > xHandler( m_xContext->getServiceManager()->createInstanceWithContext( i_rServiceName, m_xContext ), UNO_QUERY_THROW );

        Reference< XInitialization > xHandlerInit( xHandler, UNO_QUERY );
        if ( xHandlerInit.is() )
        {
            ::comphelper::NamedValueCollection aInitArgs;
            aInitArgs.put( "Parent", getParentXWindow() );
            xHandlerInit->initialize( aInitArgs.getWrappedPropertyValues() );
        }

        if ( xHandler->handleInteractionRequest( i_rRequest ) )
            return true;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("uui");
    }
    return false;
}


bool UUIInteractionHelper::handleTypedHandlerImplementations( Reference< XInteractionRequest > const & rRequest )
{
    // the request
    const Any aRequest( rRequest->getRequest() );

    const StringHashMap::const_iterator aCacheHitTest = m_aTypedCustomHandlers.find( aRequest.getValueTypeName() );
    if ( aCacheHitTest != m_aTypedCustomHandlers.end() )
        return handleCustomRequest( rRequest, aCacheHitTest->second );

    // the base registration node for "typed" interaction handlers
    const ::utl::OConfigurationTreeRoot aConfigRoot( ::utl::OConfigurationTreeRoot::createWithComponentContext(
        m_xContext,
        "/org.openoffice.Interaction/InteractionHandlers",
        -1,
        ::utl::OConfigurationTreeRoot::CM_READONLY
    ) );

    // loop through all registered implementations
    const Sequence< OUString > aRegisteredHandlers( aConfigRoot.getNodeNames() );
    for ( auto const & handlerName : aRegisteredHandlers )
    {
        const ::utl::OConfigurationNode aHandlerNode( aConfigRoot.openNode( handlerName ) );
        const ::utl::OConfigurationNode aTypesNode( aHandlerNode.openNode( u"HandledRequestTypes"_ustr ) );

        // loop through all the types which the current handler is registered for
        const Sequence< OUString > aHandledTypes( aTypesNode.getNodeNames() );
        for ( auto const & type : aHandledTypes )
        {
            // the UNO type is the node name
            ::utl::OConfigurationNode aType( aTypesNode.openNode( type ) );
            // and there's a child denoting how the responsibility propagates
            OUString sPropagation;
            OSL_VERIFY( aType.getNodeValue( u"Propagation"_ustr ) >>= sPropagation );
            if ( lcl_matchesRequest( aRequest, type, sPropagation ) )
            {
                // retrieve the service/implementation name of the handler
                OUString sServiceName;
                OSL_VERIFY( aHandlerNode.getNodeValue( u"ServiceName"_ustr ) >>= sServiceName );
                // cache the information who feels responsible for requests of this type
                m_aTypedCustomHandlers[ aRequest.getValueTypeName() ] = sServiceName;
                // actually handle the request
                return handleCustomRequest( rRequest, sServiceName );
            }
        }
    }

    return false;
}

bool
UUIInteractionHelper::handleRequest_impl(
    uno::Reference< task::XInteractionRequest > const & rRequest,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    OUString & rErrorString)
{
    try
    {
        if (!rRequest.is())
            return false;

        uno::Any aAnyRequest(rRequest->getRequest());

        script::ModuleSizeExceededRequest aModSizeException;
        if (aAnyRequest >>= aModSizeException )
        {
            std::vector< OUString > aArguments;
            aArguments.push_back(
                comphelper::string::convertCommaSeparated(aModSizeException.Names));
            handleErrorHandlerRequest( task::InteractionClassification_WARNING,
                                       ERRCODE_UUI_IO_MODULESIZEEXCEEDED,
                                       aArguments,
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString);
            return true;
        }

        document::ExoticFileLoadException aExoticFileLoadException;
        if (aAnyRequest >>= aExoticFileLoadException)
        {
            std::vector< OUString > aArguments;

            if( !aExoticFileLoadException.URL.isEmpty() )
            {
                aArguments.push_back( aExoticFileLoadException.URL );
            }
            if( !aExoticFileLoadException.FilterUIName.isEmpty() )
            {
                aArguments.push_back( aExoticFileLoadException.FilterUIName );
            }

            handleErrorHandlerRequest( task::InteractionClassification_WARNING,
                                       ERRCODE_UUI_IO_EXOTICFILEFORMAT,
                                       aArguments,
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString);
            return true;
        }

        ucb::NameClashException aNCException;
        if (aAnyRequest >>= aNCException)
        {
            ErrCode nErrorCode = ERRCODE_UUI_IO_TARGETALREADYEXISTS;
            std::vector< OUString > aArguments;

            if( !aNCException.Name.isEmpty() )
            {
                nErrorCode = ERRCODE_UUI_IO_ALREADYEXISTS;
                aArguments.push_back( aNCException.Name );
            }

            handleErrorHandlerRequest( aNCException.Classification,
                                       nErrorCode,
                                       aArguments,
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString);
            return true;
        }

        ucb::UnsupportedNameClashException aUORequest;
        if (aAnyRequest >>= aUORequest)
        {

            uno::Reference< task::XInteractionApprove > xApprove;
            uno::Reference< task::XInteractionDisapprove > xDisapprove;
            getContinuations(
                rRequest->getContinuations(), &xApprove, &xDisapprove);

            if ( xApprove.is() && xDisapprove.is() )
            {
                std::vector< OUString > aArguments;
                handleErrorHandlerRequest( task::InteractionClassification_QUERY,
                                           ERRCODE_UUI_IO_UNSUPPORTEDOVERWRITE,
                                           aArguments,
                                           rRequest->getContinuations(),
                                           bObtainErrorStringOnly,
                                           bHasErrorString,
                                           rErrorString);
            }
            return true;
        }

        if ( handleInteractiveIOException( rRequest,
                                           bObtainErrorStringOnly,
                                           bHasErrorString,
                                           rErrorString ) )
            return true;

        ucb::InteractiveAppException aAppException;
        if (aAnyRequest >>= aAppException)
        {
            std::vector< OUString > aArguments;
            handleErrorHandlerRequest( aAppException.Classification,
                                       ErrCode(aAppException.Code),
                                       aArguments,
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString);
            return true;
        }

        ucb::InteractiveNetworkException aNetworkException;
        if (aAnyRequest >>= aNetworkException)
        {
            ErrCode nErrorCode;
            std::vector< OUString > aArguments;
            ucb::InteractiveNetworkOffLineException aOffLineException;
            ucb::InteractiveNetworkResolveNameException aResolveNameException;
            ucb::InteractiveNetworkConnectException aConnectException;
            ucb::InteractiveNetworkReadException aReadException;
            ucb::InteractiveNetworkWriteException aWriteException;
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

            handleErrorHandlerRequest(aNetworkException.Classification,
                                      nErrorCode,
                                      aArguments,
                                      rRequest->getContinuations(),
                                      bObtainErrorStringOnly,
                                      bHasErrorString,
                                      rErrorString);
            return true;
        }

        ucb::InteractiveWrongMediumException aWrongMediumException;
        if (aAnyRequest >>= aWrongMediumException)
        {
            sal_Int32 nMedium = 0;
            aWrongMediumException.Medium >>= nMedium;
            std::vector< OUString > aArguments { OUString::number(nMedium + 1) };
            handleErrorHandlerRequest(aWrongMediumException.Classification,
                                      ERRCODE_UUI_WRONGMEDIUM,
                                      aArguments,
                                      rRequest->getContinuations(),
                                      bObtainErrorStringOnly,
                                      bHasErrorString,
                                      rErrorString);
            return true;
        }

        java::WrongJavaVersionException aWrongJavaVersionException;
        if (aAnyRequest >>= aWrongJavaVersionException)
        {
            ErrCode nErrorCode;
            std::vector< OUString > aArguments;
            if (aWrongJavaVersionException.DetectedVersion.isEmpty())
                if (aWrongJavaVersionException.LowestSupportedVersion.isEmpty())
                    nErrorCode = ERRCODE_UUI_WRONGJAVA;
                else
                {
                    nErrorCode = ERRCODE_UUI_WRONGJAVA_MIN;
                    aArguments.push_back(aWrongJavaVersionException.LowestSupportedVersion);
                }
            else if (aWrongJavaVersionException.LowestSupportedVersion.isEmpty())
            {
                nErrorCode = ERRCODE_UUI_WRONGJAVA_VERSION;
                aArguments.push_back(aWrongJavaVersionException.DetectedVersion);
            }
            else
            {
                nErrorCode = ERRCODE_UUI_WRONGJAVA_VERSION_MIN;
                aArguments.reserve(2);
                aArguments.push_back(aWrongJavaVersionException.DetectedVersion);
                aArguments.push_back(aWrongJavaVersionException.LowestSupportedVersion);
            }
            handleErrorHandlerRequest(task::InteractionClassification_ERROR,
                                      nErrorCode,
                                      aArguments,
                                      rRequest->getContinuations(),
                                      bObtainErrorStringOnly,
                                      bHasErrorString,
                                      rErrorString);
            return true;
        }

        configuration::backend::MergeRecoveryRequest aMergeRecoveryRequest;
        if (aAnyRequest >>= aMergeRecoveryRequest)
        {
            ErrCode nErrorCode = aMergeRecoveryRequest.IsRemovalRequest
                ? ERRCODE_UUI_CONFIGURATION_BROKENDATA_WITHREMOVE
                : ERRCODE_UUI_CONFIGURATION_BROKENDATA_NOREMOVE;

            std::vector< OUString > aArguments { aMergeRecoveryRequest.ErrorLayerId };

            handleErrorHandlerRequest(task::InteractionClassification_ERROR,
                                      nErrorCode,
                                      aArguments,
                                      rRequest->getContinuations(),
                                      bObtainErrorStringOnly,
                                      bHasErrorString,
                                      rErrorString);
            return true;
        }

        configuration::backend::StratumCreationException
            aStratumCreationException;

        if (aAnyRequest >>= aStratumCreationException)
        {
            const ErrCode nErrorCode = ERRCODE_UUI_CONFIGURATION_BACKENDMISSING;

            OUString aStratum = aStratumCreationException.StratumData;
            if (aStratum.isEmpty())
                aStratum = aStratumCreationException.StratumService;

            std::vector< OUString > aArguments { aStratum };

            handleErrorHandlerRequest(task::InteractionClassification_ERROR,
                                      nErrorCode,
                                      aArguments,
                                      rRequest->getContinuations(),
                                      bObtainErrorStringOnly,
                                      bHasErrorString,
                                      rErrorString);
            return true;
        }

        xforms::InvalidDataOnSubmitException aInvalidDataOnSubmitException;
        if (aAnyRequest >>= aInvalidDataOnSubmitException)
        {
            const ErrCode nErrorCode =
                ERRCODE_UUI_INVALID_XFORMS_SUBMISSION_DATA;

            std::vector< OUString > aArguments;

            handleErrorHandlerRequest(task::InteractionClassification_QUERY,
                                      nErrorCode,
                                      aArguments,
                                      rRequest->getContinuations(),
                                      bObtainErrorStringOnly,
                                      bHasErrorString,
                                      rErrorString);
            return true;
        }

        ucb::InteractiveLockingLockedException aLLException;
        if (aAnyRequest >>= aLLException)
        {
            ErrCode nErrorCode = aLLException.SelfOwned
                ? ERRCODE_UUI_LOCKING_LOCKED_SELF : ERRCODE_UUI_LOCKING_LOCKED;
            std::vector< OUString > aArguments { aLLException.Url };

            handleErrorHandlerRequest( aLLException.Classification,
                                       nErrorCode,
                                       aArguments,
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString );
            return true;
        }

        ucb::InteractiveLockingNotLockedException aLNLException;
        if (aAnyRequest >>= aLNLException)
        {
            std::vector< OUString > aArguments { aLNLException.Url };

            handleErrorHandlerRequest( aLNLException.Classification,
                                       ERRCODE_UUI_LOCKING_NOT_LOCKED,
                                       aArguments,
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString );
            return true;
        }

        ucb::InteractiveLockingLockExpiredException aLLEException;
        if (aAnyRequest >>= aLLEException)
        {
            std::vector< OUString > aArguments { aLLEException.Url };

            handleErrorHandlerRequest( aLLEException.Classification,
                                       ERRCODE_UUI_LOCKING_LOCK_EXPIRED,
                                       aArguments,
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString );
            return true;
        }

        document::BrokenPackageRequest aBrokenPackageRequest;
        if (aAnyRequest >>= aBrokenPackageRequest)
        {
            std::vector< OUString > aArguments;

            if( !aBrokenPackageRequest.aName.isEmpty() )
                aArguments.push_back( aBrokenPackageRequest.aName );

            handleBrokenPackageRequest( aArguments,
                                        rRequest->getContinuations(),
                                        bObtainErrorStringOnly,
                                        bHasErrorString,
                                        rErrorString );
            return true;
        }

        task::ErrorCodeRequest2 aErrorCodeRequest2;
        if (aAnyRequest >>= aErrorCodeRequest2)
        {
            handleGenericErrorRequest(
                    ErrCodeMsg(ErrCode(aErrorCodeRequest2.ErrCode), aErrorCodeRequest2.Arg1, aErrorCodeRequest2.Arg2, static_cast<DialogMask>(aErrorCodeRequest2.DialogMask)),
                    rRequest->getContinuations(),
                    bObtainErrorStringOnly,
                    bHasErrorString,
                    rErrorString);
            return true;
        }
        task::ErrorCodeRequest aErrorCodeRequest;
        if (aAnyRequest >>= aErrorCodeRequest)
        {
            handleGenericErrorRequest( ErrCode(aErrorCodeRequest.ErrCode),
                    rRequest->getContinuations(),
                    bObtainErrorStringOnly,
                    bHasErrorString,
                    rErrorString);
            return true;
        }

        task::ErrorCodeIOException aErrorCodeIOException;
        if (aAnyRequest >>= aErrorCodeIOException)
        {
            handleGenericErrorRequest( ErrCode(aErrorCodeIOException.ErrCode),
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString);
            return true;
        }

        loader::CannotActivateFactoryException aCannotActivateFactoryException;
        if (aAnyRequest >>= aCannotActivateFactoryException)
        {
            std::vector< OUString > aArguments { aCannotActivateFactoryException.Message };

            handleErrorHandlerRequest( task::InteractionClassification_ERROR,
                                       ERRCODE_UUI_CANNOT_ACTIVATE_FACTORY,
                                       aArguments,
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString );
            return true;
        }


        // Handle requests which do not have a plain string representation.

        if (!bObtainErrorStringOnly)
        {
            ucb::AuthenticationFallbackRequest anAuthFallbackRequest;
            if ( aAnyRequest >>= anAuthFallbackRequest )
            {
                handleAuthFallbackRequest( anAuthFallbackRequest.instructions,
                        anAuthFallbackRequest.url, rRequest->getContinuations() );
                return true;
            }

            if ( handleAuthenticationRequest( rRequest ) )
                return true;

            if ( handleCertificateValidationRequest( rRequest ) )
                return true;

            ucb::NameClashResolveRequest aNameClashResolveRequest;
            if (aAnyRequest >>= aNameClashResolveRequest)
            {
                handleNameClashResolveRequest(aNameClashResolveRequest,
                                              rRequest->getContinuations());
                return true;
            }

            if ( handleMasterPasswordRequest( rRequest ) )
                return true;

            if ( handlePasswordRequest( rRequest ) )
                return true;

            if ( handleNoSuchFilterRequest( rRequest ) )
                return true;

            if ( handleFilterOptionsRequest( rRequest ) )
                return true;

            if ( handleLockedDocumentRequest( rRequest ) )
                return true;

            if ( handleChangedByOthersRequest( rRequest ) )
                return true;

            if ( handleLockFileProblemRequest( rRequest ) )
                return true;

            if ( handleReloadEditableRequest( rRequest ) )
                return true;

            task::DocumentMacroConfirmationRequest aMacroConfirmRequest;
            if (aAnyRequest >>= aMacroConfirmRequest)
            {
                handleMacroConfirmRequest(
                    aMacroConfirmRequest.DocumentURL,
                    aMacroConfirmRequest.DocumentStorage,
                    !aMacroConfirmRequest.DocumentVersion.isEmpty() ? aMacroConfirmRequest.DocumentVersion : ODFVER_013_TEXT,
                    aMacroConfirmRequest.DocumentSignatureInformation,
                    rRequest->getContinuations());
                return true;
            }

            OUString aFileName;
            beans::NamedValue aLoadReadOnlyRequest;
            if ((aAnyRequest >>= aLoadReadOnlyRequest) &&
                aLoadReadOnlyRequest.Name == "LoadReadOnlyRequest" &&
                (aLoadReadOnlyRequest.Value >>= aFileName))
            {
                handleLoadReadOnlyRequest(aFileName,
                                          rRequest->getContinuations());
                return true;
            }

            // Last chance: interaction handlers registered in the configuration


            // typed InteractionHandlers (ooo.Interactions)
            if ( handleTypedHandlerImplementations( rRequest ) )
                return true;
        }

        // Not handled.
        return false;
    }
    catch( const uno::RuntimeException& )
    {
        throw;  // allowed to leave here
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("uui");
    }
    return false;
}

const uno::Reference< awt::XWindow>&
UUIInteractionHelper::getParentXWindow() const
{
    return m_xWindowParam;
}

uno::Reference< task::XInteractionHandler2 >
UUIInteractionHelper::getInteractionHandler() const
{
    return InteractionHandler::createWithParentAndContext(
        m_xContext, m_xWindowParam,
        m_aContextParam);
}

namespace {

DialogMask
executeMessageBox(
    weld::Window * pParent,
    OUString const & rTitle,
    OUString const & rMessage,
    VclMessageType eMessageType)
{
    SolarMutexGuard aGuard;

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent, eMessageType,
        eMessageType == VclMessageType::Question ? VclButtonsType::YesNo : VclButtonsType::Ok, rMessage, GetpApp()));
    xBox->set_title(rTitle);

    short nMessResult = xBox->run();
    DialogMask aResult = DialogMask::NONE;
    switch (nMessResult)
    {
        case RET_OK:
            aResult = DialogMask::ButtonsOk;
            break;
        case RET_CANCEL:
            aResult = DialogMask::ButtonsCancel;
            break;
        case RET_YES:
            aResult = DialogMask::ButtonsYes;
            break;
        case RET_NO:
            aResult = DialogMask::ButtonsNo;
            break;
        default:
            SAL_WARN("uui", "executeMessageBox: nMessResult is " << nMessResult);
            assert(false);
    }

    return aResult;
}

NameClashResolveDialogResult executeSimpleNameClashResolveDialog(weld::Window *pParent,
                                                                 OUString const & rTargetFolderURL,
                                                                 OUString const & rClashingName,
                                                                 OUString & rProposedNewName,
                                                                 bool bAllowOverwrite)
{
    std::locale aResLocale = Translate::Create("uui");
    NameClashDialog aDialog(pParent, aResLocale, rTargetFolderURL,
                            rClashingName, rProposedNewName, bAllowOverwrite);

    NameClashResolveDialogResult eResult = static_cast<NameClashResolveDialogResult>(aDialog.run());
    rProposedNewName = aDialog.getNewName();
    return eResult;
}

} // namespace

void
UUIInteractionHelper::handleNameClashResolveRequest(
    ucb::NameClashResolveRequest const & rRequest,
    uno::Sequence< uno::Reference<
        task::XInteractionContinuation > > const & rContinuations)
{
    OSL_ENSURE(!rRequest.TargetFolderURL.isEmpty(),
        "NameClashResolveRequest must not contain empty TargetFolderURL" );

    OSL_ENSURE(!rRequest.ClashingName.isEmpty(),
        "NameClashResolveRequest must not contain empty ClashingName" );

    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< ucb::XInteractionSupplyName > xSupplyName;
    uno::Reference< ucb::XInteractionReplaceExistingData > xReplaceExistingData;
    getContinuations(
        rContinuations, &xAbort, &xSupplyName, &xReplaceExistingData);

    OSL_ENSURE( xAbort.is(),
        "NameClashResolveRequest must contain Abort continuation" );

    OSL_ENSURE( xSupplyName.is(),
        "NameClashResolveRequest must contain SupplyName continuation" );

    NameClashResolveDialogResult eResult = ABORT;
    OUString aProposedNewName( rRequest.ProposedNewName );

    uno::Reference<awt::XWindow> xParent = getParentXWindow();
    eResult = executeSimpleNameClashResolveDialog(Application::GetFrameWeld(xParent),
                    rRequest.TargetFolderURL,
                    rRequest.ClashingName,
                    aProposedNewName,
                    xReplaceExistingData.is());

    switch ( eResult )
    {
    case ABORT:
        xAbort->select();
        break;

    case RENAME:
        xSupplyName->setName( aProposedNewName );
        xSupplyName->select();
        break;

    case OVERWRITE:
        OSL_ENSURE(
            xReplaceExistingData.is(),
            "Invalid NameClashResolveDialogResult: OVERWRITE - "
            "No ReplaceExistingData continuation available!" );
        xReplaceExistingData->select();
        break;
    }
}

void
UUIInteractionHelper::handleGenericErrorRequest(
    ErrCodeMsg nErrorCode,
    uno::Sequence< uno::Reference<
        task::XInteractionContinuation > > const & rContinuations,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    OUString & rErrorString)
{
    if (bObtainErrorStringOnly)
    {
        bHasErrorString = isInformationalErrorMessageRequest(rContinuations);
        if (bHasErrorString)
        {
            OUString aErrorString;
            ErrorHandler::GetErrorString(nErrorCode, aErrorString);
            rErrorString = aErrorString;
        }
    }
    else
    {
        uno::Reference< task::XInteractionAbort > xAbort;
        uno::Reference< task::XInteractionApprove > xApprove;
        getContinuations(rContinuations, &xApprove, &xAbort);

        // Note: It's important to convert the transported long to the
        // required  unsigned long value. Otherwise using as flag field
        // can fail ...
        bool bWarning = !nErrorCode.IgnoreWarning();

        if ( nErrorCode == ERRCODE_SFX_INCOMPLETE_ENCRYPTION )
        {
            // the security warning box needs a special title
            OUString aErrorString;
            ErrorHandler::GetErrorString( nErrorCode, aErrorString );

            std::locale aResLocale = Translate::Create("uui");
            OUString aTitle( utl::ConfigManager::getProductName() );

            OUString aErrTitle = Translate::get(STR_WARNING_INCOMPLETE_ENCRYPTION_TITLE, aResLocale);

            if ( !aTitle.isEmpty() && !aErrTitle.isEmpty() )
                aTitle += " - " ;
            aTitle += aErrTitle;

            uno::Reference<awt::XWindow> xParent = getParentXWindow();
            executeMessageBox(Application::GetFrameWeld(xParent), aTitle, aErrorString, VclMessageType::Error);
        }
        else
        {
            uno::Reference<awt::XWindow> xParent = getParentXWindow();
            ErrorHandler::HandleError(nErrorCode, Application::GetFrameWeld(xParent));
        }

        if (xApprove.is() && bWarning)
            xApprove->select();
        else if (xAbort.is())
            xAbort->select();
    }
}

void
UUIInteractionHelper::handleMacroConfirmRequest(
    const OUString& aDocumentURL,
    const uno::Reference< embed::XStorage >& xZipStorage,
    const OUString& aDocumentVersion,
    const uno::Sequence< security::DocumentSignatureInformation >& aSignInfo,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations )
{
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< task::XInteractionApprove > xApprove;
    getContinuations( rContinuations, &xApprove, &xAbort );

    bool bApprove = false;

    bool bShowSignatures = aSignInfo.hasElements();
    uno::Reference<awt::XWindow> xParent = getParentXWindow();
    MacroWarning aWarning(Application::GetFrameWeld(xParent), bShowSignatures);

    aWarning.SetDocumentURL(aDocumentURL);
    if ( aSignInfo.getLength() > 1 )
    {
        aWarning.SetStorage(xZipStorage, aDocumentVersion, aSignInfo);
    }
    else if ( aSignInfo.getLength() == 1 )
    {
        aWarning.SetCertificate(aSignInfo[0].Signer);
    }

    bApprove = aWarning.run() == RET_OK;

    if ( bApprove && xApprove.is() )
        xApprove->select();
    else if ( xAbort.is() )
        xAbort->select();
}

void
UUIInteractionHelper::handleBrokenPackageRequest(
    std::vector< OUString > const & rArguments,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    OUString & rErrorString)
{
    if (bObtainErrorStringOnly)
    {
        bHasErrorString = isInformationalErrorMessageRequest(rContinuations);
        if (!bHasErrorString)
            return;
    }

    uno::Reference< task::XInteractionApprove > xApprove;
    uno::Reference< task::XInteractionDisapprove > xDisapprove;
    uno::Reference< task::XInteractionAbort > xAbort;
    getContinuations(rContinuations, &xApprove, &xDisapprove, &xAbort);

    ErrCode nErrorCode;
    if( xApprove.is() && xDisapprove.is() )
    {
        nErrorCode = ERRCODE_UUI_IO_BROKENPACKAGE;
    }
    else if ( xAbort.is() )
    {
        nErrorCode = ERRCODE_UUI_IO_BROKENPACKAGE_CANTREPAIR;
    }
    else
        return;

    OUString aMessage;
    {
        std::locale aResLocale = Translate::Create("uui");
        ErrorResource aErrorResource(RID_UUI_ERRHDL, aResLocale);
        if (!aErrorResource.getString(nErrorCode, aMessage))
            return;
    }

    aMessage = replaceMessageWithArguments( aMessage, rArguments );

    if (bObtainErrorStringOnly)
    {
        rErrorString = aMessage;
        return;
    }

    VclMessageType eMessageType;
    if( xApprove.is() && xDisapprove.is() )
        eMessageType = VclMessageType::Question;
    else if ( xAbort.is() )
        eMessageType = VclMessageType::Warning;
    else
        return;

    OUString title(
        utl::ConfigManager::getProductName() +
        " " +
        utl::ConfigManager::getProductVersion() );

    uno::Reference<awt::XWindow> xParent = getParentXWindow();
    switch (executeMessageBox(Application::GetFrameWeld(xParent), title, aMessage, eMessageType))
    {
    case DialogMask::ButtonsOk:
        OSL_ENSURE( xAbort.is(), "unexpected situation" );
        if (xAbort.is())
            xAbort->select();
        break;

    case DialogMask::ButtonsNo:
        OSL_ENSURE(xDisapprove.is(), "unexpected situation");
        if (xDisapprove.is())
            xDisapprove->select();
        break;

    case DialogMask::ButtonsYes:
        OSL_ENSURE(xApprove.is(), "unexpected situation");
        if (xApprove.is())
            xApprove->select();
        break;

    default: break;
    }
}

// ErrorResource Implementation
bool ErrorResource::getString(ErrCode nErrorCode, OUString &rString) const
{
    for (const std::pair<TranslateId, ErrCode>* pStringArray = m_pStringArray; pStringArray->first; ++pStringArray)
    {
        if (nErrorCode.StripWarning() == pStringArray->second.StripWarning())
        {
            rString = Translate::get(pStringArray->first, m_rResLocale);
            return true;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
