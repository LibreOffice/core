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

#include "scripthandler.hxx"

#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/script/provider/ScriptFrameworkErrorException.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/script/provider/theMasterScriptProviderFactory.hpp>
#include <com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>

#include <sfx2/objsh.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxdlg.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <framework/documentundoguard.hxx>
#include <officecfg/Office/Common.hxx>

#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrlReference.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::script::provider;
using namespace ::com::sun::star::document;

namespace scripting_protocolhandler
{

void SAL_CALL ScriptProtocolHandler::initialize(
    const css::uno::Sequence < css::uno::Any >& aArguments )
{
    if ( m_bInitialised )
    {
        return ;
    }

    // first argument contains a reference to the frame (may be empty or the desktop,
    // but usually it's a "real" frame)
    if ( aArguments.hasElements() && !( aArguments[ 0 ] >>= m_xFrame ) )
    {
        throw RuntimeException( u"ScriptProtocolHandler::initialize: could not extract reference to the frame"_ustr );
    }

    ENSURE_OR_THROW( m_xContext.is(), "ScriptProtocolHandler::initialize: No Service Manager available" );
    m_bInitialised = true;
}

Reference< XDispatch > SAL_CALL ScriptProtocolHandler::queryDispatch(
    const URL& aURL, const OUString&, sal_Int32 )
{
    Reference< XDispatch > xDispatcher;
    // get scheme of url

    Reference< uri::XUriReferenceFactory > xFac = uri::UriReferenceFactory::create( m_xContext );
    Reference<  uri::XUriReference > uriRef = xFac->parse( aURL.Complete );
    if ( uriRef.is() )
    {
        if ( uriRef->getScheme() == "vnd.sun.star.script" )
        {
            xDispatcher = this;
        }
    }

    return xDispatcher;
}

Sequence< Reference< XDispatch > > SAL_CALL
ScriptProtocolHandler::queryDispatches(
const Sequence < DispatchDescriptor >& seqDescriptor )
{
    sal_Int32 nCount = seqDescriptor.getLength();
    Sequence< Reference< XDispatch > > lDispatcher( nCount );
    std::transform(seqDescriptor.begin(), seqDescriptor.end(), lDispatcher.getArray(),
        [this](const DispatchDescriptor& rDescr) -> Reference<XDispatch> {
            return queryDispatch(rDescr.FeatureURL, rDescr.FrameName, rDescr.SearchFlags); });
    return lDispatcher;
}

void SAL_CALL ScriptProtocolHandler::dispatchWithNotification(
    const URL& aURL, const Sequence < PropertyValue >& lArgs,
    const Reference< XDispatchResultListener >& xListener )
{
    if (officecfg::Office::Common::Security::Scripting::DisableMacrosExecution::get())
        return;

    bool bSuccess = false;
    Any invokeResult;
    bool bCaughtException = false;
    Any aException;

    if ( m_bInitialised )
    {
        try
        {
            css::uno::Reference<css::uri::XUriReferenceFactory> urifac(
                css::uri::UriReferenceFactory::create(m_xContext));
            css::uno::Reference<css::uri::XVndSunStarScriptUrlReference> uri(
                urifac->parse(aURL.Complete), css::uno::UNO_QUERY_THROW);
            auto const loc = uri->getParameter(u"location"_ustr);
            bool bIsDocumentScript = loc == "document";

            if ( bIsDocumentScript )
            {
                // obtain the component for our security check
                Reference< XEmbeddedScripts > xDocumentScripts;
                if ( getScriptInvocation() )
                    xDocumentScripts.set( m_xScriptInvocation->getScriptContainer(), UNO_SET_THROW );

                OSL_ENSURE( xDocumentScripts.is(), "ScriptProtocolHandler::dispatchWithNotification: can't do the security check!" );
                if ( !xDocumentScripts.is() || !xDocumentScripts->getAllowMacroExecution() )
                {
                    if ( xListener.is() )
                    {
                        css::frame::DispatchResultEvent aEvent(
                                getXWeak(),
                                css::frame::DispatchResultState::FAILURE,
                                invokeResult );
                        try
                        {
                            xListener->dispatchFinished( aEvent ) ;
                        }
                        catch(const RuntimeException &)
                        {
                            TOOLS_WARN_EXCEPTION("scripting",
                                "ScriptProtocolHandler::dispatchWithNotification: caught RuntimeException"
                                "while dispatchFinished with failure of the execution");
                        }
                    }
                    return;
                }
            }

            // Creates a ScriptProvider ( if one is not created already )
            createScriptProvider();

            Reference< provider::XScript > xFunc =
                m_xScriptProvider->getScript( aURL.Complete );
            ENSURE_OR_THROW( xFunc.is(),
                "ScriptProtocolHandler::dispatchWithNotification: validate xFunc - unable to obtain XScript interface" );


            Sequence< Any > inArgs;
            Sequence< Any > outArgs;
            Sequence< sal_Int16 > outIndex;

            if ( lArgs.hasElements() )
            {
               int argCount = 0;
               for ( const auto& rArg : lArgs )
               {
                   // Sometimes we get a propertyval with name = "Referer" or "SynchronMode". These
                   // are not actual arguments to be passed to script, but flags describing the
                   // call, so ignore. Who thought that passing such "meta-arguments" mixed in with
                   // real arguments was a good idea?
                   if ( (rArg.Name != "Referer" &&
                         rArg.Name != "SynchronMode") ||
                        rArg.Name.isEmpty() ) //TODO:???
                   {
                       inArgs.realloc( ++argCount );
                       inArgs.getArray()[ argCount - 1 ] = rArg.Value;
                   }
               }
            }

            // attempt to protect the document against the script tampering with its Undo Context
            std::unique_ptr< ::framework::DocumentUndoGuard > pUndoGuard;
            if ( bIsDocumentScript )
                pUndoGuard.reset( new ::framework::DocumentUndoGuard( m_xScriptInvocation ) );

            bSuccess = false;
            while ( !bSuccess )
            {
                std::exception_ptr aFirstCaughtException;
                try
                {
                    invokeResult = xFunc->invoke( inArgs, outIndex, outArgs );
                    bSuccess = true;
                }
                catch( const provider::ScriptFrameworkErrorException& se )
                {
                    if  (!aFirstCaughtException)
                        aFirstCaughtException = std::current_exception();

                    if ( se.errorType != provider::ScriptFrameworkErrorType::NO_SUCH_SCRIPT )
                        // the only condition which allows us to retry is if there is no method with the
                        // given name/signature
                        std::rethrow_exception(aFirstCaughtException);

                    if ( !inArgs.hasElements() )
                        // no chance to retry if we can't strip more in-args
                        std::rethrow_exception(aFirstCaughtException);

                    // strip one argument, then retry
                    inArgs.realloc( inArgs.getLength() - 1 );
                }
            }
        }
        // Office doesn't handle exceptions rethrown here very well, it cores,
        // all we can is log them and then set fail for the dispatch event!
        // (if there is a listener of course)
        catch ( const Exception & e )
        {
            aException = ::cppu::getCaughtException();

            invokeResult <<= "ScriptProtocolHandler::dispatch: caught "
                        + aException.getValueTypeName() + ": " + e.Message;

            bCaughtException = true;
        }
    }
    else
    {
        invokeResult <<= u"ScriptProtocolHandler::dispatchWithNotification failed, ScriptProtocolHandler not initialised"_ustr;
    }

    if ( bCaughtException )
    {
        SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
        pFact->ShowAsyncScriptErrorDialog( nullptr, aException );
    }

    if ( !xListener.is() )
        return;

    // always call dispatchFinished(), because we didn't load a document but
    // executed a macro instead!
    css::frame::DispatchResultEvent aEvent;

    aEvent.Source = getXWeak();
    aEvent.Result = invokeResult;
    if ( bSuccess )
    {
        aEvent.State = css::frame::DispatchResultState::SUCCESS;
    }
    else
    {
        aEvent.State = css::frame::DispatchResultState::FAILURE;
    }

    try
    {
        xListener->dispatchFinished( aEvent ) ;
    }
    catch(const RuntimeException &)
    {
        TOOLS_WARN_EXCEPTION("scripting",
            "ScriptProtocolHandler::dispatchWithNotification: caught RuntimeException"
            "while dispatchFinished" );
    }
}

void SAL_CALL ScriptProtocolHandler::dispatch(
const URL& aURL, const Sequence< PropertyValue >& lArgs )
{
    dispatchWithNotification( aURL, lArgs, Reference< XDispatchResultListener >() );
}

void SAL_CALL ScriptProtocolHandler::addStatusListener(
const Reference< XStatusListener >&, const URL& )
{
    // implement if status is supported
}

void SAL_CALL ScriptProtocolHandler::removeStatusListener(
const Reference< XStatusListener >&, const URL& )
{}

bool
ScriptProtocolHandler::getScriptInvocation()
{
    if ( !m_xScriptInvocation.is() && m_xFrame.is() )
    {
        Reference< XController > xController = m_xFrame->getController();
        if ( xController .is() )
        {
            // try to obtain an XScriptInvocationContext interface, preferred from the
            // mode, then from the controller
            if ( !m_xScriptInvocation.set( xController->getModel(), UNO_QUERY ) )
                m_xScriptInvocation.set( xController, UNO_QUERY );
        }
        else
        {
            if ( m_xFrame.is() )
            {
                SfxFrame* pFrame = nullptr;
                for ( pFrame = SfxFrame::GetFirst(); pFrame; pFrame = SfxFrame::GetNext( *pFrame ) )
                {
                    if ( pFrame->GetFrameInterface() == m_xFrame )
                        break;
                }
                if (SfxObjectShell* pDocShell = pFrame ? pFrame->GetCurrentDocument() : SfxObjectShell::Current())
                {
                    Reference< XModel > xModel( pDocShell->GetModel() );
                    m_xScriptInvocation.set( xModel, UNO_QUERY );
                }
            }
        }
    }
    return m_xScriptInvocation.is();
}

void ScriptProtocolHandler::createScriptProvider()
{
    if ( m_xScriptProvider.is() )
        return;

    try
    {
        // first, ask the component supporting the XScriptInvocationContext interface
        // (if there is one) for a script provider
        if ( getScriptInvocation() )
        {
            Reference< XScriptProviderSupplier > xSPS( m_xScriptInvocation, UNO_QUERY );
            if ( xSPS.is() )
                m_xScriptProvider = xSPS->getScriptProvider();
        }

        // second, ask the model in our frame
        if ( !m_xScriptProvider.is() && m_xFrame.is() )
        {
            Reference< XController > xController = m_xFrame->getController();
            if ( xController .is() )
            {
                Reference< XScriptProviderSupplier > xSPS( xController->getModel(), UNO_QUERY );
                if ( xSPS.is() )
                    m_xScriptProvider = xSPS->getScriptProvider();
            }
        }


        // as a fallback, ask the controller
        if ( !m_xScriptProvider.is() && m_xFrame.is() )
        {
            Reference< XScriptProviderSupplier > xSPS( m_xFrame->getController(), UNO_QUERY );
            if ( xSPS.is() )
                m_xScriptProvider = xSPS->getScriptProvider();
        }

        // if nothing of this is successful, use the master script provider
        if ( !m_xScriptProvider.is() )
        {
            Reference< provider::XScriptProviderFactory > xFac =
                provider::theMasterScriptProviderFactory::get( m_xContext );

            Any aContext;
            if ( getScriptInvocation() )
                aContext <<= m_xScriptInvocation;
            m_xScriptProvider.set( xFac->createScriptProvider( aContext ), UNO_SET_THROW );
        }
    }
    catch ( const Exception & e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
            "ScriptProtocolHandler::createScriptProvider: " + e.Message,
            nullptr, anyEx );
    }
}

ScriptProtocolHandler::ScriptProtocolHandler( const Reference< css::uno::XComponentContext > & xContext )
  : m_bInitialised( false ), m_xContext( xContext )
{
}

ScriptProtocolHandler::~ScriptProtocolHandler()
{
}

/* XServiceInfo */
OUString SAL_CALL ScriptProtocolHandler::getImplementationName( )
{
    return u"com.sun.star.comp.ScriptProtocolHandler"_ustr;
}

/* XServiceInfo */
sal_Bool SAL_CALL ScriptProtocolHandler::supportsService(const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL ScriptProtocolHandler::getSupportedServiceNames()
{
    return {u"com.sun.star.frame.ProtocolHandler"_ustr};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
scripting_ScriptProtocolHandler_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ScriptProtocolHandler(context));
}

} // namespace scripting_protocolhandler


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
