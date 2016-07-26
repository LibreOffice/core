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

#include "dlgevtatt.hxx"

#include "dlgprov.hxx"

#include <sfx2/sfx.hrc>
#include <sfx2/app.hxx>
#include <vcl/layout.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XDialogEventHandler.hpp>
#include <com/sun/star/awt/XContainerWindowEventHandler.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/provider/theMasterScriptProviderFactory.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/lang/NoSuchMethodException.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>

#include <ooo/vba/XVBAToOOEventDescGen.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::reflection;


namespace dlgprov
{

  class DialogSFScriptListenerImpl : public DialogScriptListenerImpl
    {
        protected:
        Reference< frame::XModel >  m_xModel;
        virtual void firing_impl( const script::ScriptEvent& aScriptEvent, uno::Any* pRet ) override;
        public:
        DialogSFScriptListenerImpl( const Reference< XComponentContext >& rxContext, const Reference< frame::XModel >& rxModel ) : DialogScriptListenerImpl( rxContext ), m_xModel( rxModel ) {}
    };

  class DialogLegacyScriptListenerImpl : public DialogSFScriptListenerImpl
    {
        protected:
        virtual void firing_impl( const script::ScriptEvent& aScriptEvent, uno::Any* pRet ) override;
        public:
        DialogLegacyScriptListenerImpl( const Reference< XComponentContext >& rxContext, const Reference< frame::XModel >& rxModel ) : DialogSFScriptListenerImpl( rxContext, rxModel ){}
    };

  class DialogUnoScriptListenerImpl : public DialogSFScriptListenerImpl
    {
    Reference< awt::XControl > m_xControl;
        Reference< XInterface > m_xHandler;
    Reference< beans::XIntrospectionAccess > m_xIntrospectionAccess;
    bool m_bDialogProviderMode;

        virtual void firing_impl( const script::ScriptEvent& aScriptEvent, uno::Any* pRet ) override;

    public:
        DialogUnoScriptListenerImpl( const Reference< XComponentContext >& rxContext,
            const Reference< frame::XModel >& rxModel,
            const Reference< awt::XControl >& rxControl,
            const Reference< XInterface >& rxHandler,
            const Reference< beans::XIntrospectionAccess >& rxIntrospectionAccess,
            bool bDialogProviderMode );     // false: ContainerWindowProvider mode

    };

  class DialogVBAScriptListenerImpl : public DialogScriptListenerImpl
    {
        protected:
        OUString msDialogCodeName;
        OUString msDialogLibName;
        Reference<  script::XScriptListener > mxListener;
        virtual void firing_impl( const script::ScriptEvent& aScriptEvent, uno::Any* pRet ) override;
        public:
        DialogVBAScriptListenerImpl( const Reference< XComponentContext >& rxContext, const Reference< awt::XControl >& rxControl, const Reference< frame::XModel >& xModel, const OUString& sDialogLibName );
    };

    DialogVBAScriptListenerImpl::DialogVBAScriptListenerImpl( const Reference< XComponentContext >& rxContext, const Reference< awt::XControl >& rxControl, const Reference< frame::XModel >& xModel, const OUString& sDialogLibName ) : DialogScriptListenerImpl( rxContext ), msDialogLibName( sDialogLibName )
    {
        Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );
        Sequence< Any > args(1);
        if ( xSMgr.is() )
        {
            args[0] <<= xModel;
            mxListener.set( xSMgr->createInstanceWithArgumentsAndContext( "ooo.vba.EventListener", args, m_xContext ), UNO_QUERY );
        }
        if ( rxControl.is() )
        {
            try
            {
                Reference< XPropertySet > xProps( rxControl->getModel(), UNO_QUERY_THROW );
                xProps->getPropertyValue("Name") >>= msDialogCodeName;
                xProps.set( mxListener, UNO_QUERY_THROW );
                xProps->setPropertyValue("Model", args[ 0 ] );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

    }

    void DialogVBAScriptListenerImpl::firing_impl( const script::ScriptEvent& aScriptEvent, uno::Any* )
    {
        if ( aScriptEvent.ScriptType == "VBAInterop" && mxListener.is() )
        {
            ScriptEvent aScriptEventCopy( aScriptEvent );
            aScriptEventCopy.ScriptCode = msDialogLibName.concat( "." ).concat( msDialogCodeName );
            try
            {
                mxListener->firing( aScriptEventCopy );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }


    // DialogEventsAttacherImpl


    DialogEventsAttacherImpl::DialogEventsAttacherImpl( const Reference< XComponentContext >& rxContext, const Reference< frame::XModel >& rxModel, const Reference< awt::XControl >& rxControl, const Reference< XInterface >& rxHandler, const Reference< beans::XIntrospectionAccess >& rxIntrospect, bool bProviderMode, const Reference< script::XScriptListener >& rxRTLListener, const OUString& sDialogLibName )
        :mbUseFakeVBAEvents( false ), m_xContext( rxContext )
    {
        // key listeners by protocol when ScriptType = 'Script'
        // otherwise key is the ScriptType e.g. StarBasic
        if ( rxRTLListener.is() ) // set up handler for RTL_BASIC
            listernersForTypes[ OUString("StarBasic") ] = rxRTLListener;
        else
            listernersForTypes[ OUString("StarBasic") ] = new DialogLegacyScriptListenerImpl( rxContext, rxModel );
        // handler for Script & OUString("vnd.sun.star.UNO:")
        listernersForTypes[ OUString("vnd.sun.star.UNO") ] = new DialogUnoScriptListenerImpl( rxContext, rxModel, rxControl, rxHandler, rxIntrospect, bProviderMode );
        listernersForTypes[ OUString("vnd.sun.star.script") ] = new DialogSFScriptListenerImpl( rxContext, rxModel );

        // determine the VBA compatibility mode from the Basic library container
        try
        {
            uno::Reference< beans::XPropertySet > xModelProps( rxModel, uno::UNO_QUERY_THROW );
            uno::Reference< script::vba::XVBACompatibility > xVBACompat(
                xModelProps->getPropertyValue("BasicLibraries"), uno::UNO_QUERY_THROW );
            mbUseFakeVBAEvents = xVBACompat->getVBACompatibilityMode();
        }
        catch( uno::Exception& )
        {
        }
        if ( mbUseFakeVBAEvents )
            listernersForTypes[ OUString("VBAInterop") ] = new DialogVBAScriptListenerImpl( rxContext, rxControl, rxModel, sDialogLibName );
    }


    DialogEventsAttacherImpl::~DialogEventsAttacherImpl()
    {
    }


    Reference< script::XScriptListener >
    DialogEventsAttacherImpl::getScriptListenerForKey( const OUString& sKey ) throw ( RuntimeException )
    {
        ListenerHash::iterator it = listernersForTypes.find( sKey );
        if ( it == listernersForTypes.end() )
            throw RuntimeException(); // more text info here please
        return it->second;
    }
    Reference< XScriptEventsSupplier > DialogEventsAttacherImpl::getFakeVbaEventsSupplier( const Reference< XControl >& xControl, OUString& sControlName )
    {
        Reference< XScriptEventsSupplier > xEventsSupplier;
        Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );
        if ( xSMgr.is() )
        {
            Reference< ooo::vba::XVBAToOOEventDescGen > xVBAToOOEvtDesc( xSMgr->createInstanceWithContext("ooo.vba.VBAToOOEventDesc", m_xContext ), UNO_QUERY );
            if ( xVBAToOOEvtDesc.is() )
                xEventsSupplier.set( xVBAToOOEvtDesc->getEventSupplier( xControl, sControlName ), UNO_QUERY );

        }
        return xEventsSupplier;
    }


    void DialogEventsAttacherImpl::attachEventsToControl( const Reference< XControl>& xControl, const Reference< XScriptEventsSupplier >& xEventsSupplier, const Any& Helper )
    {
        if ( xEventsSupplier.is() )
        {
            Reference< container::XNameContainer > xEventCont = xEventsSupplier->getEvents();

            Reference< XControlModel > xControlModel = xControl->getModel();
            Reference< XPropertySet > xProps( xControlModel, uno::UNO_QUERY );
            OUString sName;
            xProps->getPropertyValue("Name") >>= sName;
            if ( xEventCont.is() )
            {
                Sequence< OUString > aNames = xEventCont->getElementNames();
                const OUString* pNames = aNames.getConstArray();
                sal_Int32 nNameCount = aNames.getLength();

                for ( sal_Int32 j = 0; j < nNameCount; ++j )
                {
                    ScriptEventDescriptor aDesc;

                    Any aElement = xEventCont->getByName( pNames[ j ] );
                    aElement >>= aDesc;
                    OUString sKey = aDesc.ScriptType;
                    if ( aDesc.ScriptType == "Script" || aDesc.ScriptType == "UNO" )
                    {
                        sal_Int32 nIndex = aDesc.ScriptCode.indexOf( ':' );
                        sKey = aDesc.ScriptCode.copy( 0, nIndex );
                    }
                    Reference< XAllListener > xAllListener =
                        new DialogAllListenerImpl( getScriptListenerForKey( sKey ), aDesc.ScriptType, aDesc.ScriptCode );

                    // try first to attach event to the ControlModel
                    bool bSuccess = false;
                    try
                    {
                        Reference< XEventListener > xListener_ = m_xEventAttacher->attachSingleEventListener(
                            xControlModel, xAllListener, Helper, aDesc.ListenerType,
                            aDesc.AddListenerParam, aDesc.EventMethod );

                        if ( xListener_.is() )
                            bSuccess = true;
                    }
                    catch ( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }

                    try
                    {
                        // if we had no success, try to attach to the control
                        if ( !bSuccess )
                        {
                            Reference< XEventListener > xListener_ = m_xEventAttacher->attachSingleEventListener(
                                xControl, xAllListener, Helper, aDesc.ListenerType,
                                aDesc.AddListenerParam, aDesc.EventMethod );
                        }
                    }
                    catch ( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
        }
    }


    void DialogEventsAttacherImpl::nestedAttachEvents( const Sequence< Reference< XInterface > >& Objects, const Any& Helper, OUString& sDialogCodeName )
    {
        const Reference< XInterface >* pObjects = Objects.getConstArray();
        sal_Int32 nObjCount = Objects.getLength();

        for ( sal_Int32 i = 0; i < nObjCount; ++i )
        {
            // We know that we have to do with instances of XControl.
            // Otherwise this is not the right implementation for
            // XScriptEventsAttacher and we have to give up.
            Reference< XControl > xControl( pObjects[ i ], UNO_QUERY );
            Reference< XControlContainer > xControlContainer( xControl, UNO_QUERY );
            Reference< XDialog > xDialog( xControl, UNO_QUERY );
            if ( !xControl.is() )
                throw IllegalArgumentException();

            // get XEventsSupplier from control model
            Reference< XControlModel > xControlModel = xControl->getModel();
            Reference< XScriptEventsSupplier > xEventsSupplier( xControlModel, UNO_QUERY );
            attachEventsToControl( xControl, xEventsSupplier, Helper );
            if ( mbUseFakeVBAEvents )
            {
                xEventsSupplier.set( getFakeVbaEventsSupplier( xControl, sDialogCodeName ) );
                Any newHelper(xControl );
                attachEventsToControl( xControl, xEventsSupplier, newHelper );
            }
            if ( xControlContainer.is() && !xDialog.is() )
            {
                Sequence< Reference< XControl > > aControls = xControlContainer->getControls();
                sal_Int32 nControlCount = aControls.getLength();

                Sequence< Reference< XInterface > > aObjects( nControlCount );
                Reference< XInterface >* pObjects2 = aObjects.getArray();
                const Reference< XControl >* pControls = aControls.getConstArray();

                for ( sal_Int32 i2 = 0; i2 < nControlCount; ++i2 )
                {
                    pObjects2[i2].set( pControls[i2], UNO_QUERY );
                }
                nestedAttachEvents( aObjects, Helper, sDialogCodeName );
            }
        }
    }


    // XScriptEventsAttacher


    void SAL_CALL DialogEventsAttacherImpl::attachEvents( const Sequence< Reference< XInterface > >& Objects,
        const css::uno::Reference<css::script::XScriptListener>&,
        const Any& Helper )
        throw (IllegalArgumentException, IntrospectionException, CannotCreateAdapterException,
               ServiceNotRegisteredException, RuntimeException, std::exception)
    {
        // get EventAttacher
        {
            ::osl::MutexGuard aGuard( getMutex() );

            if ( !m_xEventAttacher.is() )
            {
                Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );
                if ( xSMgr.is() )
                {
                    m_xEventAttacher.set( xSMgr->createInstanceWithContext(
                        "com.sun.star.script.EventAttacher", m_xContext ), UNO_QUERY );

                    if ( !m_xEventAttacher.is() )
                        throw ServiceNotRegisteredException();
                }
                else
                {
                    throw RuntimeException();
                }

            }
        }
        OUString sDialogCodeName;
        sal_Int32 nObjCount = Objects.getLength();
        Reference< awt::XControl > xDlgControl( Objects[ nObjCount - 1 ], uno::UNO_QUERY ); // last object is the dialog
        if ( xDlgControl.is() )
        {
            Reference< XPropertySet > xProps( xDlgControl->getModel(), UNO_QUERY );
            try
            {
                xProps->getPropertyValue("Name") >>= sDialogCodeName;
            }
            catch( Exception& ){}
        }
        // go over all objects
        nestedAttachEvents( Objects, Helper, sDialogCodeName );
    }


    // DialogAllListenerImpl


    DialogAllListenerImpl::DialogAllListenerImpl( const Reference< XScriptListener >& rxListener,
        const OUString& rScriptType, const OUString& rScriptCode )
        :m_xScriptListener( rxListener )
        ,m_sScriptType( rScriptType )
        ,m_sScriptCode( rScriptCode )
    {
    }


    DialogAllListenerImpl::~DialogAllListenerImpl()
    {
    }


    void DialogAllListenerImpl::firing_impl( const AllEventObject& Event, Any* pRet )
    {
        ScriptEvent aScriptEvent;
        aScriptEvent.Source         = static_cast<OWeakObject *>(this);  // get correct XInterface
        aScriptEvent.ListenerType   = Event.ListenerType;
        aScriptEvent.MethodName     = Event.MethodName;
        aScriptEvent.Arguments      = Event.Arguments;
        aScriptEvent.Helper         = Event.Helper;
        aScriptEvent.ScriptType     = m_sScriptType;
        aScriptEvent.ScriptCode     = m_sScriptCode;

        if ( m_xScriptListener.is() )
        {
            if ( pRet )
                *pRet = m_xScriptListener->approveFiring( aScriptEvent );
            else
                m_xScriptListener->firing( aScriptEvent );
        }
    }


    // XEventListener


    void DialogAllListenerImpl::disposing(const EventObject& ) throw ( RuntimeException, std::exception )
    {
    }


    // XAllListener


    void DialogAllListenerImpl::firing( const AllEventObject& Event ) throw ( RuntimeException, std::exception )
    {
        //::osl::MutexGuard aGuard( getMutex() );

        firing_impl( Event, nullptr );
    }


    Any DialogAllListenerImpl::approveFiring( const AllEventObject& Event )
        throw ( reflection::InvocationTargetException, RuntimeException, std::exception )
    {
        //::osl::MutexGuard aGuard( getMutex() );

        Any aReturn;
        firing_impl( Event, &aReturn );
        return aReturn;
    }


    // DialogScriptListenerImpl


    DialogUnoScriptListenerImpl::DialogUnoScriptListenerImpl( const Reference< XComponentContext >& rxContext,
            const Reference< css::frame::XModel >& rxModel,
            const Reference< css::awt::XControl >& rxControl,
            const Reference< css::uno::XInterface >& rxHandler,
            const Reference< css::beans::XIntrospectionAccess >& rxIntrospectionAccess,
            bool bDialogProviderMode )
        : DialogSFScriptListenerImpl( rxContext, rxModel )
        ,m_xControl( rxControl )
        ,m_xHandler( rxHandler )
        ,m_xIntrospectionAccess( rxIntrospectionAccess )
        ,m_bDialogProviderMode( bDialogProviderMode )
    {
    }


    DialogScriptListenerImpl::~DialogScriptListenerImpl()
    {
    }


    void DialogSFScriptListenerImpl::firing_impl( const ScriptEvent& aScriptEvent, Any* pRet )
    {
        try
        {
            Reference< provider::XScriptProvider > xScriptProvider;
            if ( m_xModel.is() )
            {
                Reference< provider::XScriptProviderSupplier > xSupplier( m_xModel, UNO_QUERY );
                OSL_ENSURE( xSupplier.is(), "DialogScriptListenerImpl::firing_impl: failed to get script provider supplier" );
                if ( xSupplier.is() )
                    xScriptProvider.set( xSupplier->getScriptProvider() );
            }
            else
            {
                OSL_ASSERT( m_xContext.is() );
                if ( m_xContext.is() )
                {
                    Reference< provider::XScriptProviderFactory > xFactory =
                        provider::theMasterScriptProviderFactory::get( m_xContext );

                    Any aCtx;
                    aCtx <<= OUString("user");
                    xScriptProvider.set( xFactory->createScriptProvider( aCtx ), UNO_QUERY );
                }
            }

            OSL_ENSURE( xScriptProvider.is(), "DialogScriptListenerImpl::firing_impl: failed to get script provider" );

            if ( xScriptProvider.is() )
            {
                Reference< provider::XScript > xScript = xScriptProvider->getScript( aScriptEvent.ScriptCode );
                OSL_ENSURE( xScript.is(), "DialogScriptListenerImpl::firing_impl: failed to get script" );

                if ( xScript.is() )
                {
                    Sequence< Any > aInParams;
                    Sequence< sal_Int16 > aOutParamsIndex;
                    Sequence< Any > aOutParams;

                    // get arguments for script
                    aInParams = aScriptEvent.Arguments;

                    Any aResult = xScript->invoke( aInParams, aOutParamsIndex, aOutParams );
                    if ( pRet )
                        *pRet = aResult;
                }
            }
        }
        catch ( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    void DialogLegacyScriptListenerImpl::firing_impl( const ScriptEvent& aScriptEvent, Any* pRet )
    {
        OUString sScriptURL;
        OUString sScriptCode( aScriptEvent.ScriptCode );

        if ( aScriptEvent.ScriptType == "StarBasic" )
        {
            // StarBasic script: convert ScriptCode to scriptURL
            sal_Int32 nIndex = sScriptCode.indexOf( ':' );
            if ( nIndex >= 0 && nIndex < sScriptCode.getLength() )
            {
                sScriptURL = "vnd.sun.star.script:";
                sScriptURL += sScriptCode.copy( nIndex + 1 );
                sScriptURL += "?language=Basic&location=";
                sScriptURL += sScriptCode.copy( 0, nIndex );
            }
            ScriptEvent aSFScriptEvent( aScriptEvent );
            aSFScriptEvent.ScriptCode = sScriptURL;
            DialogSFScriptListenerImpl::firing_impl( aSFScriptEvent, pRet );
        }
    }

    void DialogUnoScriptListenerImpl::firing_impl( const ScriptEvent& aScriptEvent, Any* pRet )
    {
        static const char sUnoURLScheme[] = "vnd.sun.star.UNO:";

        OUString aMethodName = aScriptEvent.ScriptCode.copy( strlen(sUnoURLScheme) );

        const Any* pArguments = aScriptEvent.Arguments.getConstArray();
        Any aEventObject = pArguments[0];

        bool bHandled = false;
        if( m_xHandler.is() )
        {
            if( m_bDialogProviderMode )
            {
                Reference< XDialogEventHandler > xDialogEventHandler( m_xHandler, UNO_QUERY );
                if( xDialogEventHandler.is() )
                {
                    Reference< XDialog > xDialog( m_xControl, UNO_QUERY );
                    bHandled = xDialogEventHandler->callHandlerMethod( xDialog, aEventObject, aMethodName );
                }
            }
            else
            {
                Reference< XContainerWindowEventHandler > xContainerWindowEventHandler( m_xHandler, UNO_QUERY );
                if( xContainerWindowEventHandler.is() )
                {
                    Reference< XWindow > xWindow( m_xControl, UNO_QUERY );
                    bHandled = xContainerWindowEventHandler->callHandlerMethod( xWindow, aEventObject, aMethodName );
                }
            }
        }

        Any aRet;
        if( !bHandled && m_xIntrospectionAccess.is() )
        {
            try
            {
                // Methode ansprechen
                const Reference< XIdlMethod >& rxMethod = m_xIntrospectionAccess->
                    getMethod( aMethodName, MethodConcept::ALL - MethodConcept::DANGEROUS );

                Reference< XMaterialHolder > xMaterialHolder =
                    Reference< XMaterialHolder >::query( m_xIntrospectionAccess );
                Any aHandlerObject = xMaterialHolder->getMaterial();

                Sequence< Reference< XIdlClass > > aParamTypeSeq = rxMethod->getParameterTypes();
                sal_Int32 nParamCount = aParamTypeSeq.getLength();
                if( nParamCount == 0 )
                {
                    Sequence<Any> args;
                    rxMethod->invoke( aHandlerObject, args );
                    bHandled = true;
                }
                else if( nParamCount == 2 )
                {
                    // Signature check automatically done by reflection
                    Sequence<Any> Args(2);
                    Any* pArgs = Args.getArray();
                    if( m_bDialogProviderMode )
                    {
                        Reference< XDialog > xDialog( m_xControl, UNO_QUERY );
                        pArgs[0] <<= xDialog;
                    }
                    else
                    {
                        Reference< XWindow > xWindow( m_xControl, UNO_QUERY );
                        pArgs[0] <<= xWindow;
                    }
                    pArgs[1] = aEventObject;
                    aRet = rxMethod->invoke( aHandlerObject, Args );
                    bHandled = true;
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        if( bHandled )
        {
            if( pRet )
                *pRet = aRet;
        }
        else
        {
            ResMgr* pResMgr = SfxApplication::GetSfxResManager();
            if( pResMgr )
            {
                OUString aRes( ResId(STR_ERRUNOEVENTBINDUNG, *pResMgr) );
                OUString aQuoteChar( "\"" );

                sal_Int32 nIndex = aRes.indexOf( '%' );

                OUString aOUFinal;
                aOUFinal += aRes.copy( 0, nIndex );
                aOUFinal += aQuoteChar;
                aOUFinal += aMethodName;
                aOUFinal += aQuoteChar;
                aOUFinal += aRes.copy( nIndex + 2 );

                ScopedVclPtrInstance<MessageDialog>(nullptr, aOUFinal)->Execute();
            }
        }
    }


    // XEventListener


    void DialogScriptListenerImpl::disposing(const EventObject& ) throw ( RuntimeException, std::exception )
    {
    }


    // XScriptListener


    void DialogScriptListenerImpl::firing( const ScriptEvent& aScriptEvent ) throw ( RuntimeException, std::exception )
    {
        //::osl::MutexGuard aGuard( getMutex() );

        firing_impl( aScriptEvent, nullptr );
    }


    Any DialogScriptListenerImpl::approveFiring( const ScriptEvent& aScriptEvent )
        throw ( reflection::InvocationTargetException, RuntimeException, std::exception )
    {
        //::osl::MutexGuard aGuard( getMutex() );

        Any aReturn;
        firing_impl( aScriptEvent, &aReturn );
        return aReturn;
    }


}   // namespace dlgprov


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
