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

#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/configuration/theDefaultProvider.hpp"
#include "com/sun/star/configuration/backend/MergeRecoveryRequest.hpp"
#include "com/sun/star/configuration/backend/StratumCreationException.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/document/BrokenPackageRequest.hpp"
#include "com/sun/star/task/DocumentMacroConfirmationRequest.hpp"
#include "com/sun/star/java/WrongJavaVersionException.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/script/ModuleSizeExceededRequest.hpp"
#include "com/sun/star/task/ErrorCodeIOException.hpp"
#include "com/sun/star/task/ErrorCodeRequest.hpp"
#include "com/sun/star/task/FutureDocumentVersionProductUpdateRequest.hpp"
#include "com/sun/star/task/InteractionHandler.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/task/XInteractionAskLater.hpp"
#include "com/sun/star/task/XInteractionDisapprove.hpp"
#include "com/sun/star/task/XInteractionHandler2.hpp"
#include "com/sun/star/task/XInteractionRequest.hpp"
#include "com/sun/star/task/XInteractionRetry.hpp"
#include "com/sun/star/ucb/InteractiveAppException.hpp"
#include "com/sun/star/ucb/InteractiveLockingLockedException.hpp"
#include "com/sun/star/ucb/InteractiveLockingNotLockedException.hpp"
#include "com/sun/star/ucb/InteractiveLockingLockExpiredException.hpp"
#include "com/sun/star/ucb/InteractiveNetworkConnectException.hpp"
#include "com/sun/star/ucb/InteractiveNetworkOffLineException.hpp"
#include "com/sun/star/ucb/InteractiveNetworkReadException.hpp"
#include "com/sun/star/ucb/InteractiveNetworkResolveNameException.hpp"
#include "com/sun/star/ucb/InteractiveNetworkWriteException.hpp"
#include "com/sun/star/ucb/InteractiveWrongMediumException.hpp"
#include "com/sun/star/ucb/NameClashException.hpp"
#include "com/sun/star/ucb/NameClashResolveRequest.hpp"
#include "com/sun/star/ucb/UnsupportedNameClashException.hpp"
#include "com/sun/star/ucb/XInteractionReplaceExistingData.hpp"
#include "com/sun/star/ucb/XInteractionSupplyName.hpp"
#include "com/sun/star/xforms/InvalidDataOnSubmitException.hpp"
#include "com/sun/star/loader/CannotActivateFactoryException.hpp"

#include <rtl/strbuf.hxx>
#include "osl/conditn.hxx"
#include "tools/rcid.h" // RSC_STRING
#include "tools/errinf.hxx" // ErrorHandler, ErrorContext, ...
#include "osl/mutex.hxx"
#include "osl/thread.hxx"
#include "tools/diagnose_ex.h"
#include "comphelper/documentconstants.hxx" // ODFVER_012_TEXT
#include "svtools/sfxecode.hxx" // ERRCODE_SFX_*
#include "vcl/msgbox.hxx"
#include "vcl/svapp.hxx"
#include "unotools/configmgr.hxx"
#include "toolkit/helper/vclunohelper.hxx"
#include "comphelper/processfactory.hxx"
#include "comphelper/namedvaluecollection.hxx"
#include "typelib/typedescription.hxx"
#include "unotools/confignode.hxx"

#include "ids.hrc"

#include "getcontinuations.hxx"
#include "secmacrowarnings.hxx"
#include "newerverwarn.hxx"

#include "iahndl.hxx"
#include "nameclashdlg.hxx"

#include <boost/scoped_ptr.hpp>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::task::XInteractionContinuation;
using ::com::sun::star::task::XInteractionAbort;
using ::com::sun::star::task::XInteractionApprove;
using ::com::sun::star::task::XInteractionAskLater;
using ::com::sun::star::task::FutureDocumentVersionProductUpdateRequest;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::lang::XInitialization;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::task::InteractionHandler;
using ::com::sun::star::task::XInteractionHandler2;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Any;
using ::com::sun::star::task::XInteractionRequest;
using ::com::sun::star::lang::XMultiServiceFactory;

using namespace ::com::sun::star;

namespace {

class HandleData : public osl::Condition
{
public:
    HandleData(
        uno::Reference< task::XInteractionRequest > const & rRequest)
        : osl::Condition(),
          m_rRequest(rRequest),
          bHandled( false )
    {
    }
    uno::Reference< task::XInteractionRequest > m_rRequest;
    bool                                        bHandled;
    beans::Optional< OUString >            m_aResult;
};

} /* namespace */

UUIInteractionHelper::UUIInteractionHelper(
    uno::Reference< uno::XComponentContext > const & rxContext,
    uno::Reference< awt::XWindow > const & rxWindowParam,
    const OUString & rContextParam)
    SAL_THROW(()):
        m_xContext(rxContext),
        m_xWindowParam(rxWindowParam),
        m_aContextParam(rContextParam)
{
}

UUIInteractionHelper::UUIInteractionHelper(
    uno::Reference< uno::XComponentContext > const & rxContext)
    SAL_THROW(()):
        m_xContext(rxContext)
{
}

UUIInteractionHelper::~UUIInteractionHelper()
{
}

long
UUIInteractionHelper::handlerequest(
    void* pHandleData, void* pInteractionHelper)
{
    HandleData* pHND
        = static_cast< HandleData * >(pHandleData);
    UUIInteractionHelper* pUUI
        = static_cast< UUIInteractionHelper * >(pInteractionHelper);
    bool bDummy = false;
    OUString aDummy;
    pHND->bHandled
        = pUUI->handleRequest_impl(pHND->m_rRequest, false, bDummy, aDummy);
    pHND->set();
    return 0;
}

bool
UUIInteractionHelper::handleRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    SAL_THROW((uno::RuntimeException))
{
    Application* pApp = 0;
    if(
        // be aware,it is the same type
        static_cast< oslThreadIdentifier >(
            Application::GetMainThreadIdentifier())
        != osl::Thread::getCurrentIdentifier()
        &&
        (pApp = GetpApp())
        != 0
    ) {
        // we are not in the main thread, let it handle that stuff
        HandleData aHD(rRequest);
        Link aLink(&aHD,handlerequest);
        pApp->PostUserEvent(aLink,this);
        sal_uLong locks = Application::ReleaseSolarMutex();
        aHD.wait();
        Application::AcquireSolarMutex(locks);
        return aHD.bHandled;
    }
    else
    {
        bool bDummy = false;
        OUString aDummy;
        return handleRequest_impl(rRequest, false, bDummy, aDummy);
    }
}

long
UUIInteractionHelper::getstringfromrequest(
    void* pHandleData,void* pInteractionHelper)
{
    HandleData* pHND = (HandleData*) pHandleData;
    UUIInteractionHelper* pUUI = (UUIInteractionHelper*) pInteractionHelper;
    pHND->m_aResult = pUUI->getStringFromRequest_impl(pHND->m_rRequest);
    pHND->set();
    return 0;
}

beans::Optional< OUString >
UUIInteractionHelper::getStringFromRequest_impl(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    SAL_THROW((uno::RuntimeException))
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
    SAL_THROW((uno::RuntimeException))
{
    Application* pApp = 0;
    if(
        // be aware,it is the same type
        static_cast< oslThreadIdentifier >(
            Application::GetMainThreadIdentifier())
        != osl::Thread::getCurrentIdentifier()
        &&
        (pApp = GetpApp())
        != 0
    ) {
        // we are not in the main thread, let it handle that stuff
        HandleData aHD(rRequest);
        Link aLink(&aHD,getstringfromrequest);
        pApp->PostUserEvent(aLink,this);
        sal_uLong locks = Application::ReleaseSolarMutex();
        aHD.wait();
        Application::AcquireSolarMutex(locks);
        return aHD.m_aResult;
    }
    else
        return getStringFromRequest_impl(rRequest);
}

OUString
UUIInteractionHelper::replaceMessageWithArguments(
    OUString aMessage,
    std::vector< OUString > const & rArguments )
{
    for (sal_Int32 i = 0;;)
    {
        i = aMessage.indexOf("$(ARG", i);
        if (i == -1)
            break;
        if (aMessage.getLength() - i >= RTL_CONSTASCII_LENGTH("$(ARGx)")
            && aMessage.getStr()[i + RTL_CONSTASCII_LENGTH("$(ARGx")] == ')')
        {
            sal_Unicode c
                = aMessage.getStr()[i + RTL_CONSTASCII_LENGTH("$(ARG")];
            if (c >= '1' && c <= '2')
            {
                std::vector< OUString >::size_type nIndex
                    = static_cast< std::vector< OUString >::size_type >(
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
    if (xAbort.is())
        return true;

    return false;
}

bool
UUIInteractionHelper::tryOtherInteractionHandler(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    SAL_THROW((uno::RuntimeException))
{
    InteractionHandlerDataList dataList;
    getInteractionHandlerList(dataList);

    InteractionHandlerDataList::const_iterator aEnd(dataList.end());
    for (InteractionHandlerDataList::const_iterator aIt(dataList.begin());
         aIt != aEnd;
         ++aIt)
    {
        if ( handleCustomRequest( rRequest, aIt->ServiceName ) )
            return true;
    }
    return false;
}

namespace
{
    // .................................................................................................................
    static bool lcl_matchesRequest( const Any& i_rRequest, const OUString& i_rTypeName, const OUString& i_rPropagation )
    {
        const ::com::sun::star::uno::TypeDescription aTypeDesc( i_rTypeName );
        const typelib_TypeDescription* pTypeDesc = aTypeDesc.get();
        if ( !pTypeDesc || !pTypeDesc->pWeakRef )
        {
#if OSL_DEBUG_LEVEL > 0
            OStringBuffer aMessage;
            aMessage.append( "no type found for '" );
            aMessage.append( OUStringToOString( i_rTypeName, RTL_TEXTENCODING_UTF8 ) );
            aMessage.append( "'" );
            OSL_FAIL( aMessage.makeStringAndClear().getStr() );
#endif
            return false;
        }
        const ::com::sun::star::uno::Type aType( pTypeDesc->pWeakRef );

        const bool bExactMatch = ( i_rPropagation.compareToAscii( "named-only" ) == 0 );
        if ( bExactMatch )
            return i_rRequest.getValueType().equals( aType );

        return i_rRequest.isExtractableTo( aType );
    }
}

// ---------------------------------------------------------------------------------------------------------------------
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
        DBG_UNHANDLED_EXCEPTION();
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
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
        OUString( "/org.openoffice.Interaction/InteractionHandlers" ),
        -1,
        ::utl::OConfigurationTreeRoot::CM_READONLY
    ) );

    // loop through all registered implementations
    const Sequence< OUString > aRegisteredHandlers( aConfigRoot.getNodeNames() );
    const OUString* pHandlerName = aRegisteredHandlers.getConstArray();
    const OUString* pHandlersEnd = aRegisteredHandlers.getConstArray() + aRegisteredHandlers.getLength();
    for ( ; pHandlerName != pHandlersEnd; ++pHandlerName )
    {
        const ::utl::OConfigurationNode aHandlerNode( aConfigRoot.openNode( *pHandlerName ) );
        const ::utl::OConfigurationNode aTypesNode( aHandlerNode.openNode( "HandledRequestTypes" ) );

        // loop through all the types which the current handler is registered for
        const Sequence< OUString > aHandledTypes( aTypesNode.getNodeNames() );
        const OUString* pType = aHandledTypes.getConstArray();
        const OUString* pTypesEnd = aHandledTypes.getConstArray() + aHandledTypes.getLength();
        for ( ; pType != pTypesEnd; ++pType )
        {
            // the UNO type is the node name
            ::utl::OConfigurationNode aType( aTypesNode.openNode( *pType ) );
            // and there's a child denoting how the responsibility propagates
            OUString sPropagation;
            OSL_VERIFY( aType.getNodeValue( "Propagation" ) >>= sPropagation );
            if ( lcl_matchesRequest( aRequest, *pType, sPropagation ) )
            {
                // retrieve the service/implementation name of the handler
                OUString sServiceName;
                OSL_VERIFY( aHandlerNode.getNodeValue( "ServiceName" ) >>= sServiceName );
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
    SAL_THROW((uno::RuntimeException))
{
    try
    {
        if (!rRequest.is())
            return false;

        uno::Any aAnyRequest(rRequest->getRequest());

        script::ModuleSizeExceededRequest aModSizeException;
        if (aAnyRequest >>= aModSizeException )
        {
            ErrCode nErrorCode = ERRCODE_UUI_IO_MODULESIZEEXCEEDED;
            std::vector< OUString > aArguments;
            uno::Sequence< OUString > sModules
                = aModSizeException.Names;
            if ( sModules.getLength() )
            {
                OUString aName;
                for ( sal_Int32 index=0; index< sModules.getLength(); ++index )
                {
                    if ( index )
                        aName = aName + OUString( ',' ) + sModules[index];
                    else
                        aName = sModules[index]; // 1st name
                }
                aArguments.push_back( aName );
            }
            handleErrorHandlerRequest( task::InteractionClassification_WARNING,
                                       nErrorCode,
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
            ErrCode nErrorCode = ERRCODE_UUI_IO_UNSUPPORTEDOVERWRITE;
            std::vector< OUString > aArguments;

            uno::Reference< task::XInteractionApprove > xApprove;
            uno::Reference< task::XInteractionDisapprove > xDisapprove;
            getContinuations(
                rRequest->getContinuations(), &xApprove, &xDisapprove);

            if ( xApprove.is() && xDisapprove.is() )
            {
                handleErrorHandlerRequest( task::InteractionClassification_QUERY,
                                           nErrorCode,
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
                                       aAppException.Code,
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
            std::vector< OUString > aArguments;
            aArguments.push_back(OUString::valueOf(nMedium + 1));
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

            std::vector< OUString > aArguments;
            aArguments.push_back(aMergeRecoveryRequest.ErrorLayerId);

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

            std::vector< OUString > aArguments;
            aArguments.push_back(aStratum);

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
            std::vector< OUString > aArguments;
            aArguments.push_back( aLLException.Url );

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
            ErrCode nErrorCode = ERRCODE_UUI_LOCKING_NOT_LOCKED;
            std::vector< OUString > aArguments;
            aArguments.push_back( aLNLException.Url );

            handleErrorHandlerRequest( aLNLException.Classification,
                                       nErrorCode,
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
            ErrCode nErrorCode = ERRCODE_UUI_LOCKING_LOCK_EXPIRED;
            std::vector< OUString > aArguments;
            aArguments.push_back( aLLEException.Url );

            handleErrorHandlerRequest( aLLEException.Classification,
                                       nErrorCode,
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

        task::ErrorCodeRequest aErrorCodeRequest;
        if (aAnyRequest >>= aErrorCodeRequest)
        {
            // Sucky special handling for xlsx macro filter warning
            if ( (sal_uInt32)ERRCODE_SFX_VBASIC_CANTSAVE_STORAGE == (sal_uInt32)aErrorCodeRequest.ErrCode)
            {
                std::vector< OUString > aArguments;
                handleErrorHandlerRequest( task::InteractionClassification_WARNING,
                                       ERRCODE_UUI_IO_WARN_CANTSAVE_MACROS,
                                       aArguments,
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString);
            }
            else
            {
                handleGenericErrorRequest( aErrorCodeRequest.ErrCode,
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString);
            }
            return true;
        }

        task::ErrorCodeIOException aErrorCodeIOException;
        if (aAnyRequest >>= aErrorCodeIOException)
        {
            handleGenericErrorRequest( aErrorCodeIOException.ErrCode,
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString);
            return true;
        }

        loader::CannotActivateFactoryException aCannotActivateFactoryException;
        if (aAnyRequest >>= aCannotActivateFactoryException)
        {
            ErrCode nErrorCode = ERRCODE_UUI_CANNOT_ACTIVATE_FACTORY;
            std::vector< OUString > aArguments;
            aArguments.push_back( aCannotActivateFactoryException.Message );

            handleErrorHandlerRequest( task::InteractionClassification_ERROR,
                                       nErrorCode,
                                       aArguments,
                                       rRequest->getContinuations(),
                                       bObtainErrorStringOnly,
                                       bHasErrorString,
                                       rErrorString );
            return true;
        }


        ///////////////////////////////////////////////////////////////////
        // Handle requests which do not have a plain string representation.
        ///////////////////////////////////////////////////////////////////
        if (!bObtainErrorStringOnly)
        {
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

            if ( handleAmbigousFilterRequest( rRequest ) )
                return true;

            if ( handleFilterOptionsRequest( rRequest ) )
                return true;

            if ( handleLockedDocumentRequest( rRequest ) )
                return true;

            if ( handleChangedByOthersRequest( rRequest ) )
                return true;

            if ( handleLockFileIgnoreRequest( rRequest ) )
                return true;

            task::DocumentMacroConfirmationRequest aMacroConfirmRequest;
            if (aAnyRequest >>= aMacroConfirmRequest)
            {
                handleMacroConfirmRequest(
                    aMacroConfirmRequest.DocumentURL,
                    aMacroConfirmRequest.DocumentStorage,
                    !aMacroConfirmRequest.DocumentVersion.isEmpty() ? aMacroConfirmRequest.DocumentVersion : ODFVER_012_TEXT,
                    aMacroConfirmRequest.DocumentSignatureInformation,
                    rRequest->getContinuations());
                return true;
            }

            task::FutureDocumentVersionProductUpdateRequest
                aProductUpdateRequest;
            if (aAnyRequest >>= aProductUpdateRequest)
            {
                handleFutureDocumentVersionUpdateRequest(
                    aProductUpdateRequest,
                    rRequest->getContinuations());
                return true;
            }

            ///////////////////////////////////////////////////////////////
            // Last chance: interaction handlers registered in the configuration
            ///////////////////////////////////////////////////////////////

            // typed InteractionHandlers (ooo.Interactions)
            if ( handleTypedHandlerImplementations( rRequest ) )
                return true;

            // legacy configuration (ooo.ucb.InteractionHandlers)
            if (tryOtherInteractionHandler( rRequest ))
                return true;
        }

        // Not handled.
        return false;
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException("out of memory",
            uno::Reference< uno::XInterface >());
    }
    catch( const uno::RuntimeException& )
    {
        throw;  // allowed to leave here
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return false;
}

void
UUIInteractionHelper::getInteractionHandlerList(
    InteractionHandlerDataList &rdataList)
    SAL_THROW((uno::RuntimeException))
{
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xConfigProv =
            configuration::theDefaultProvider::get( m_xContext );

        OUStringBuffer aFullPath;
        aFullPath.appendAscii(
            "/org.openoffice.ucb.InteractionHandler/InteractionHandlers" );

        uno::Sequence< uno::Any > aArguments( 1 );
        beans::PropertyValue      aProperty;
        aProperty.Name = OUString( "nodepath" );
        aProperty.Value <<= aFullPath.makeStringAndClear();
        aArguments[ 0 ] <<= aProperty;

        uno::Reference< uno::XInterface > xInterface(
                xConfigProv->createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationAccess" , aArguments ) );

        if ( !xInterface.is() )
            throw uno::RuntimeException("unable to instanciate config access",
                uno::Reference< uno::XInterface >());

        uno::Reference< container::XNameAccess > xNameAccess(
            xInterface, uno::UNO_QUERY );
        if ( !xNameAccess.is() )
            throw uno::RuntimeException(OUString(
                    "config access does not implement XNameAccess"),
                uno::Reference< uno::XInterface >());

        uno::Sequence< OUString > aElems = xNameAccess->getElementNames();
        const OUString* pElems = aElems.getConstArray();
        sal_Int32 nCount = aElems.getLength();

        if ( nCount > 0 )
        {
            uno::Reference< container::XHierarchicalNameAccess >
                                xHierNameAccess( xInterface, uno::UNO_QUERY );

            if ( !xHierNameAccess.is() )
                throw uno::RuntimeException(OUString(
                    "config access does not implement XHierarchicalNameAccess"),
                    uno::Reference< uno::XInterface >());

            // Iterate over children.
            for ( sal_Int32 n = 0; n < nCount; ++n )
            {
                OUStringBuffer aElemBuffer;
                aElemBuffer.appendAscii( "['" );
                aElemBuffer.append( pElems[ n ] );

                try
                {
                    InteractionHandlerData aInfo;

                    // Obtain service name.
                    OUStringBuffer aKeyBuffer = aElemBuffer;
                    aKeyBuffer.appendAscii( "']/ServiceName" );

                    OUString aValue;
                    if ( !( xHierNameAccess->getByHierarchicalName(
                                aKeyBuffer.makeStringAndClear() ) >>= aValue ) )
                    {
                        OSL_FAIL( "GetInteractionHandlerList - "
                                    "Error getting item value!" );
                        continue;
                    }

                    aInfo.ServiceName = aValue;

                    // Append info to list.
                    rdataList.push_back( aInfo );
                }
                catch ( container::NoSuchElementException& )
                {
                    // getByHierarchicalName

                    OSL_FAIL( "GetInteractionHandlerList - "
                                "caught NoSuchElementException!" );
                }
            }
        }
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & )
    {
        OSL_FAIL( "GetInteractionHandlerList - Caught Exception!" );
    }
}

Window *
UUIInteractionHelper::getParentProperty()
    SAL_THROW(())
{
    uno::Reference< awt::XWindow > xWindow = getParentXWindow();
    if ( xWindow.is() )
        return VCLUnoHelper::GetWindow(xWindow);

    return 0;
}

uno::Reference< awt::XWindow>
UUIInteractionHelper::getParentXWindow() const
    SAL_THROW(())
{
    return m_xWindowParam;
}

OUString
UUIInteractionHelper::getContextProperty()
    SAL_THROW(())
{
    return m_aContextParam;
}

uno::Reference< task::XInteractionHandler2 >
UUIInteractionHelper::getInteractionHandler()
    SAL_THROW((uno::RuntimeException))
{
    return InteractionHandler::createWithParentAndContext(
        m_xContext, m_xWindowParam,
        m_aContextParam);
}

namespace {

sal_uInt16
executeMessageBox(
    Window * pParent,
    OUString const & rTitle,
    OUString const & rMessage,
    WinBits nButtonMask )
    SAL_THROW((uno::RuntimeException))
{
    SolarMutexGuard aGuard;

    MessBox xBox( pParent, nButtonMask, rTitle, rMessage );

    sal_uInt16 aResult = xBox.Execute();
    switch( aResult )
    {
    case RET_OK:
        aResult = ERRCODE_BUTTON_OK;
        break;
    case RET_CANCEL:
        aResult = ERRCODE_BUTTON_CANCEL;
        break;
    case RET_YES:
        aResult = ERRCODE_BUTTON_YES;
        break;
    case RET_NO:
        aResult = ERRCODE_BUTTON_NO;
        break;
    case RET_RETRY:
        aResult = ERRCODE_BUTTON_RETRY;
        break;
    }

    return aResult;
}

NameClashResolveDialogResult executeSimpleNameClashResolveDialog( Window *pParent,
                                                                  OUString const & rTargetFolderURL,
                                                                  OUString const & rClashingName,
                                                                  OUString & rProposedNewName,
                                                                  bool bAllowOverwrite )
{
    boost::scoped_ptr< ResMgr > xManager( ResMgr::CreateResMgr( "uui" ) );
    if ( !xManager.get() )
        return ABORT;

    NameClashDialog aDialog( pParent, xManager.get(), rTargetFolderURL,
                             rClashingName, rProposedNewName, bAllowOverwrite );

    NameClashResolveDialogResult eResult = (NameClashResolveDialogResult) aDialog.Execute();
    rProposedNewName = aDialog.getNewName();
    return eResult;
}

} // namespace

void
UUIInteractionHelper::handleNameClashResolveRequest(
    ucb::NameClashResolveRequest const & rRequest,
    uno::Sequence< uno::Reference<
        task::XInteractionContinuation > > const & rContinuations)
  SAL_THROW((uno::RuntimeException))
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

    eResult = executeSimpleNameClashResolveDialog( getParentProperty(),
                    rRequest.TargetFolderURL,
                    rRequest.ClashingName,
                    aProposedNewName,
                    xReplaceExistingData.is() );

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
    sal_Int32 nErrorCode,
    uno::Sequence< uno::Reference<
        task::XInteractionContinuation > > const & rContinuations,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    OUString & rErrorString)
    SAL_THROW((uno::RuntimeException))
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
        // required  unsigned long value. Otherwhise using as flag field
        // can fail ...
        ErrCode  nError   = static_cast< ErrCode >(nErrorCode);
        sal_Bool bWarning = !ERRCODE_TOERROR(nError);

        if ( nError == ERRCODE_SFX_BROKENSIGNATURE
             || nError == ERRCODE_SFX_INCOMPLETE_ENCRYPTION )
        {
            // the security warning box needs a special title
            OUString aErrorString;
            ErrorHandler::GetErrorString( nErrorCode, aErrorString );

            boost::scoped_ptr< ResMgr > xManager(
                ResMgr::CreateResMgr( "uui" ) );
            OUString aTitle( utl::ConfigManager::getProductName() );

            OUString aErrTitle
                  = ResId( nError == ERRCODE_SFX_BROKENSIGNATURE
                                       ? STR_WARNING_BROKENSIGNATURE_TITLE
                                       : STR_WARNING_INCOMPLETE_ENCRYPTION_TITLE,
                                   *xManager.get() ).toString();

            if ( !aTitle.isEmpty() && !aErrTitle.isEmpty() )
                aTitle += " - " ;
            aTitle += aErrTitle;

            executeMessageBox(
                getParentProperty(), aTitle, aErrorString, WB_OK );
        }
        else
            ErrorHandler::HandleError(nErrorCode);

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
    const uno::Sequence< security::DocumentSignatureInformation > aSignInfo,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations )
    SAL_THROW((uno::RuntimeException))
{
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< task::XInteractionApprove > xApprove;
    getContinuations( rContinuations, &xApprove, &xAbort );

    bool bApprove = false;

    boost::scoped_ptr< ResMgr > pResMgr( ResMgr::CreateResMgr( "uui" ) );
    if ( pResMgr.get() )
    {
        bool bShowSignatures = aSignInfo.getLength() > 0;
        MacroWarning aWarning(
            getParentProperty(), bShowSignatures, *pResMgr.get() );

        aWarning.SetDocumentURL( aDocumentURL );
        if ( aSignInfo.getLength() > 1 )
        {
            aWarning.SetStorage( xZipStorage, aDocumentVersion, aSignInfo );
        }
        else if ( aSignInfo.getLength() == 1 )
        {
            aWarning.SetCertificate( aSignInfo[ 0 ].Signer );
        }

        bApprove = aWarning.Execute() == RET_OK;
    }

    if ( bApprove && xApprove.is() )
        xApprove->select();
    else if ( xAbort.is() )
        xAbort->select();
}

void
UUIInteractionHelper::handleFutureDocumentVersionUpdateRequest(
    const task::FutureDocumentVersionProductUpdateRequest& _rRequest,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations )
    SAL_THROW((uno::RuntimeException))
{
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< task::XInteractionApprove > xApprove;
    uno::Reference< task::XInteractionAskLater > xAskLater;
    getContinuations( rContinuations, &xApprove, &xAbort, &xAskLater );

    short nResult = RET_CANCEL;

    static bool s_bDeferredToNextSession = false;
    // TODO: this static variable is somewhat hacky. Formerly (before the dialog was moved from SFX2 to the
    // interaction handler implementation), this was stored in SFX_APP()'s impl structure, in member
    // bODFVersionWarningLater. Of course, we do not have access to it here.
    //
    // A proper solution which I would envision would be:
    // - There's a central implementation (this one here) of css.task.InteractionHandler
    // - There's a configuration which maps UNO names to service names
    // - If the handler is confronted with a request, it tries to find the name of the UNO structure describing
    //   the request in the said configuration.
    //   - If an entry is found, then
    //     - the respective service is instantiated
    //     - the component is queried for css.task.XInteractionHandler, and the request is delegated
    //   - if no entry is found, then the request is silenced (with calling the AbortContinuation, if possible)
    // This way, the FutureDocumentVersionProductUpdateRequest could be handled in SFX (or any other
    // suitable place), again, and we would only have one place where we remember the s_bDeferredToNextSession
    // flag.
    //
    // Note: The above pattern has been implemented in CWS autorecovery. Now the remaining task is to move the
    // handling of this interaction to SFX, again.

    if ( !s_bDeferredToNextSession )
    {
        boost::scoped_ptr< ResMgr > pResMgr( ResMgr::CreateResMgr( "uui" ) );
        if ( pResMgr.get() )
        {
            ::uui::NewerVersionWarningDialog aDialog(
                getParentProperty(),
                _rRequest.DocumentODFVersion,
                *pResMgr.get() );
            nResult = aDialog.Execute();
        }
    }

    switch ( nResult )
    {
    case RET_OK:
        if ( xApprove.is() )
            xApprove->select();
        break;
    case RET_CANCEL:
        if ( xAbort.is() )
            xAbort->select();
        break;
    case RET_ASK_LATER:
        if ( xAskLater.is() )
            xAskLater->select();
        s_bDeferredToNextSession = true;
        break;
    default:
        OSL_FAIL( "UUIInteractionHelper::handleFutureDocumentVersionUpdateRequest: "
                    "unexpected dialog return value!" );
        break;
    }
}

void
UUIInteractionHelper::handleBrokenPackageRequest(
    std::vector< OUString > const & rArguments,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations,
    bool bObtainErrorStringOnly,
    bool & bHasErrorString,
    OUString & rErrorString)
    SAL_THROW((uno::RuntimeException))
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
        SolarMutexGuard aGuard;
        boost::scoped_ptr< ResMgr > xManager(ResMgr::CreateResMgr("uui"));
        if (!xManager.get())
            return;

        ResId aResId( RID_UUI_ERRHDL, *xManager.get() );
        if ( !ErrorResource(aResId).getString(nErrorCode, aMessage) )
            return;
    }

    aMessage = replaceMessageWithArguments( aMessage, rArguments );

    if (bObtainErrorStringOnly)
    {
        rErrorString = aMessage;
        return;
    }

    WinBits nButtonMask;
    if( xApprove.is() && xDisapprove.is() )
    {
        nButtonMask = WB_YES_NO | WB_DEF_YES;
    }
    else if ( xAbort.is() )
    {
        nButtonMask = WB_OK;
    }
    else
        return;

    OUString title(
        utl::ConfigManager::getProductName() +
        OUString( " " ) +
        utl::ConfigManager::getProductVersion() );

    switch (
        executeMessageBox( getParentProperty(), title, aMessage, nButtonMask ) )
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

//=========================================================================
// ErrorResource Implementation
//=========================================================================

bool
ErrorResource::getString(ErrCode nErrorCode, OUString &rString)
    const SAL_THROW(())
{
    ResId aResId(static_cast< sal_uInt16 >(nErrorCode & ERRCODE_RES_MASK),
                 *m_pResMgr);
    aResId.SetRT(RSC_STRING);
    if (!IsAvailableRes(aResId))
        return false;
    aResId.SetAutoRelease(false);
    rString = aResId.toString();
    m_pResMgr->PopContext();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
