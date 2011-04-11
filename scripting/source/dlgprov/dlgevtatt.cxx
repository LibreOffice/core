/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"
#include "dlgevtatt.hxx"

#include "dlgprov.hxx"

#include <sfx2/sfx.hrc>
#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XDialogEventHandler.hpp>
#include <com/sun/star/awt/XContainerWindowEventHandler.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/lang/NoSuchMethodException.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>

#include <ooo/vba/XVBAToOOEventDescGen.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <vbahelper/vbaaccesshelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::reflection;


//.........................................................................
namespace dlgprov
{

  class DialogSFScriptListenerImpl : public DialogScriptListenerImpl
    {
        protected:
        Reference< frame::XModel >  m_xModel;
        virtual void firing_impl( const script::ScriptEvent& aScriptEvent, uno::Any* pRet );
        public:
        DialogSFScriptListenerImpl( const Reference< XComponentContext >& rxContext, const Reference< frame::XModel >& rxModel ) : DialogScriptListenerImpl( rxContext ), m_xModel( rxModel ) {}
    };

  class DialogLegacyScriptListenerImpl : public DialogSFScriptListenerImpl
    {
        protected:
        virtual void firing_impl( const script::ScriptEvent& aScriptEvent, uno::Any* pRet );
        public:
        DialogLegacyScriptListenerImpl( const Reference< XComponentContext >& rxContext, const Reference< frame::XModel >& rxModel ) : DialogSFScriptListenerImpl( rxContext, rxModel ){}
    };

  class DialogUnoScriptListenerImpl : public DialogSFScriptListenerImpl
    {
    Reference< awt::XControl > m_xControl;
        Reference< XInterface > m_xHandler;
    Reference< beans::XIntrospectionAccess > m_xIntrospectionAccess;
    bool m_bDialogProviderMode;

        virtual void firing_impl( const script::ScriptEvent& aScriptEvent, uno::Any* pRet );

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
        rtl::OUString msDialogCodeName;
        rtl::OUString msDialogLibName;
        Reference<  script::XScriptListener > mxListener;
        virtual void firing_impl( const script::ScriptEvent& aScriptEvent, uno::Any* pRet );
        public:
        DialogVBAScriptListenerImpl( const Reference< XComponentContext >& rxContext, const Reference< awt::XControl >& rxControl, const Reference< frame::XModel >& xModel, const rtl::OUString& sDialogLibName );
    };

    DialogVBAScriptListenerImpl::DialogVBAScriptListenerImpl( const Reference< XComponentContext >& rxContext, const Reference< awt::XControl >& rxControl, const Reference< frame::XModel >& xModel, const rtl::OUString& sDialogLibName ) : DialogScriptListenerImpl( rxContext ), msDialogLibName( sDialogLibName )
    {
        Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );
        Sequence< Any > args(1);
        if ( xSMgr.is() )
        {
            args[0] <<= xModel;
            mxListener = Reference< XScriptListener >( xSMgr->createInstanceWithArgumentsAndContext( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.EventListener" ) ), args, m_xContext ), UNO_QUERY );
        }
        if ( rxControl.is() )
        {
            try
            {
                Reference< XPropertySet > xProps( rxControl->getModel(), UNO_QUERY_THROW );
                xProps->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name") ) ) >>= msDialogCodeName;
                xProps.set( mxListener, UNO_QUERY_THROW );
                xProps->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Model") ), args[ 0 ] );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

    }

    void DialogVBAScriptListenerImpl::firing_impl( const script::ScriptEvent& aScriptEvent, uno::Any* )
    {
        if ( aScriptEvent.ScriptType.equals( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VBAInterop")) ) && mxListener.is() )
        {
            ScriptEvent aScriptEventCopy( aScriptEvent );
            aScriptEventCopy.ScriptCode = msDialogLibName.concat( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "." ) ) ).concat( msDialogCodeName );
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

//.........................................................................

    // =============================================================================
    // DialogEventsAttacherImpl
    // =============================================================================

    DialogEventsAttacherImpl::DialogEventsAttacherImpl( const Reference< XComponentContext >& rxContext, const Reference< frame::XModel >& rxModel, const Reference< awt::XControl >& rxControl, const Reference< XInterface >& rxHandler, const Reference< beans::XIntrospectionAccess >& rxIntrospect, bool bProviderMode, const Reference< script::XScriptListener >& rxRTLListener, const rtl::OUString& sDialogLibName )
        :mbUseFakeVBAEvents( false ), m_xContext( rxContext )
    {
        // key listeners by protocol when ScriptType = 'Script'
        // otherwise key is the ScriptType e.g. StarBasic
        if ( rxRTLListener.is() ) // set up handler for RTL_BASIC
            listernersForTypes[ rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StarBasic")) ] = rxRTLListener;
        else
            listernersForTypes[ rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StarBasic")) ] = new DialogLegacyScriptListenerImpl( rxContext, rxModel );
        // handler for Script & ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.UNO:"))
        listernersForTypes[ rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.UNO")) ] = new DialogUnoScriptListenerImpl( rxContext, rxModel, rxControl, rxHandler, rxIntrospect, bProviderMode );
        listernersForTypes[ rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.script")) ] = new DialogSFScriptListenerImpl( rxContext, rxModel );
        // Note: in a future cws ( npower13_ObjectModule ) it will be possible
        // to determine the vba mode from the basiclibrary container, the tunnel hack
        // below can then be replaced
        SfxObjectShell* pFoundShell = NULL;
        if ( rxModel.is() )
        {
            uno::Reference< lang::XUnoTunnel >  xObjShellTunnel( rxModel, uno::UNO_QUERY );
            if ( xObjShellTunnel.is() )
            {
                pFoundShell = reinterpret_cast<SfxObjectShell*>( xObjShellTunnel->getSomething(SfxObjectShell::getUnoTunnelId()));
                if ( pFoundShell )
                    mbUseFakeVBAEvents = ooo::vba::isAlienExcelDoc( *pFoundShell ) || ooo::vba::isAlienWordDoc( *pFoundShell ) ;
            }
        }
        if ( mbUseFakeVBAEvents )
            listernersForTypes[ rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VBAInterop")) ] = new DialogVBAScriptListenerImpl( rxContext, rxControl, rxModel, sDialogLibName );
    }

    // -----------------------------------------------------------------------------

    DialogEventsAttacherImpl::~DialogEventsAttacherImpl()
    {
    }

    // -----------------------------------------------------------------------------
    Reference< script::XScriptListener >
    DialogEventsAttacherImpl::getScriptListenerForKey( const rtl::OUString& sKey ) throw ( RuntimeException )
    {
        ListenerHash::iterator it = listernersForTypes.find( sKey );
        if ( it == listernersForTypes.end() )
            throw RuntimeException(); // more text info here please
        return it->second;
    }
    Reference< XScriptEventsSupplier > DialogEventsAttacherImpl::getFakeVbaEventsSupplier( const Reference< XControl >& xControl, rtl::OUString& sControlName )
    {
        Reference< XScriptEventsSupplier > xEventsSupplier;
        Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );
        if ( xSMgr.is() )
        {
            Reference< ooo::vba::XVBAToOOEventDescGen > xVBAToOOEvtDesc( xSMgr->createInstanceWithContext( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.VBAToOOEventDesc" ) ), m_xContext ), UNO_QUERY );
            if ( xVBAToOOEvtDesc.is() )
                xEventsSupplier.set( xVBAToOOEvtDesc->getEventSupplier( xControl, sControlName ), UNO_QUERY );

        }
        return xEventsSupplier;
    }

    // -----------------------------------------------------------------------------
    void SAL_CALL DialogEventsAttacherImpl::attachEventsToControl( const Reference< XControl>& xControl, const Reference< XScriptEventsSupplier >& xEventsSupplier, const Any& Helper )
    {
        if ( xEventsSupplier.is() )
        {
            Reference< container::XNameContainer > xEventCont = xEventsSupplier->getEvents();

            Reference< XControlModel > xControlModel = xControl->getModel();
            Reference< XPropertySet > xProps( xControlModel, uno::UNO_QUERY );
            rtl::OUString sName;
            xProps->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name")) ) >>= sName;
            if ( xEventCont.is() )
            {
                Sequence< ::rtl::OUString > aNames = xEventCont->getElementNames();
                const ::rtl::OUString* pNames = aNames.getConstArray();
                sal_Int32 nNameCount = aNames.getLength();

                for ( sal_Int32 j = 0; j < nNameCount; ++j )
                {
                    ScriptEventDescriptor aDesc;

                    Any aElement = xEventCont->getByName( pNames[ j ] );
                    aElement >>= aDesc;
                    rtl::OUString sKey = aDesc.ScriptType;
                    if ( aDesc.ScriptType.equals( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Script")) ) || aDesc.ScriptType.equals( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UNO")) ) )
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


    void DialogEventsAttacherImpl::nestedAttachEvents( const Sequence< Reference< XInterface > >& Objects, const Any& Helper, rtl::OUString& sDialogCodeName )
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
                Reference< XInterface >* pObjectsModify = aObjects.getArray();
                const Reference< XControl >* pControls = aControls.getConstArray();

                for ( sal_Int32 j = 0; j < nControlCount; ++j )
                {
                    pObjectsModify[j] = Reference< XInterface >( pControls[j], UNO_QUERY );
                }
                nestedAttachEvents( aObjects, Helper, sDialogCodeName );
            }
        }
    }

    // -----------------------------------------------------------------------------
    // XScriptEventsAttacher
    // -----------------------------------------------------------------------------

    void SAL_CALL DialogEventsAttacherImpl::attachEvents( const Sequence< Reference< XInterface > >& Objects,
        const com::sun::star::uno::Reference<com::sun::star::script::XScriptListener>&,
        const Any& Helper )
        throw (IllegalArgumentException, IntrospectionException, CannotCreateAdapterException,
               ServiceNotRegisteredException, RuntimeException)
    {
        // get EventAttacher
        {
            ::osl::MutexGuard aGuard( getMutex() );

            if ( !m_xEventAttacher.is() )
            {
                Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );
                if ( xSMgr.is() )
                {
                    m_xEventAttacher = Reference< XEventAttacher >( xSMgr->createInstanceWithContext(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.EventAttacher" ) ), m_xContext ), UNO_QUERY );

                    if ( !m_xEventAttacher.is() )
                        throw ServiceNotRegisteredException();
                }
                else
                {
                    throw RuntimeException();
                }

            }
        }
        rtl::OUString sDialogCodeName;
        sal_Int32 nObjCount = Objects.getLength();
        Reference< awt::XControl > xDlgControl( Objects[ nObjCount - 1 ], uno::UNO_QUERY ); // last object is the dialog
        if ( xDlgControl.is() )
        {
            Reference< XPropertySet > xProps( xDlgControl->getModel(), UNO_QUERY );
            try
            {
                xProps->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name") ) ) >>= sDialogCodeName;
            }
            catch( Exception& ){}
        }
        // go over all objects
        nestedAttachEvents( Objects, Helper, sDialogCodeName );
    }


    // =============================================================================
    // DialogAllListenerImpl
    // =============================================================================

    DialogAllListenerImpl::DialogAllListenerImpl( const Reference< XScriptListener >& rxListener,
        const ::rtl::OUString& rScriptType, const ::rtl::OUString& rScriptCode )
        :m_xScriptListener( rxListener )
        ,m_sScriptType( rScriptType )
        ,m_sScriptCode( rScriptCode )
    {
    }

    // -----------------------------------------------------------------------------

    DialogAllListenerImpl::~DialogAllListenerImpl()
    {
    }

    // -----------------------------------------------------------------------------

    void DialogAllListenerImpl::firing_impl( const AllEventObject& Event, Any* pRet )
    {
        ScriptEvent aScriptEvent;
        aScriptEvent.Source         = (OWeakObject *)this;  // get correct XInterface
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

    // -----------------------------------------------------------------------------
    // XEventListener
    // -----------------------------------------------------------------------------

    void DialogAllListenerImpl::disposing(const EventObject& ) throw ( RuntimeException )
    {
    }

    // -----------------------------------------------------------------------------
    // XAllListener
    // -----------------------------------------------------------------------------

    void DialogAllListenerImpl::firing( const AllEventObject& Event ) throw ( RuntimeException )
    {
        //::osl::MutexGuard aGuard( getMutex() );

        firing_impl( Event, NULL );
    }

    // -----------------------------------------------------------------------------

    Any DialogAllListenerImpl::approveFiring( const AllEventObject& Event )
        throw ( reflection::InvocationTargetException, RuntimeException )
    {
        //::osl::MutexGuard aGuard( getMutex() );

        Any aReturn;
        firing_impl( Event, &aReturn );
        return aReturn;
    }


    // =============================================================================
    // DialogScriptListenerImpl
    // =============================================================================

    DialogUnoScriptListenerImpl::DialogUnoScriptListenerImpl( const Reference< XComponentContext >& rxContext,
            const Reference< ::com::sun::star::frame::XModel >& rxModel,
            const Reference< ::com::sun::star::awt::XControl >& rxControl,
            const Reference< ::com::sun::star::uno::XInterface >& rxHandler,
            const Reference< ::com::sun::star::beans::XIntrospectionAccess >& rxIntrospectionAccess,
            bool bDialogProviderMode )
        : DialogSFScriptListenerImpl( rxContext, rxModel )
        ,m_xControl( rxControl )
        ,m_xHandler( rxHandler )
        ,m_xIntrospectionAccess( rxIntrospectionAccess )
        ,m_bDialogProviderMode( bDialogProviderMode )
    {
    }

    // -----------------------------------------------------------------------------

    DialogScriptListenerImpl::~DialogScriptListenerImpl()
    {
    }

    // -----------------------------------------------------------------------------
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
                    Reference< provider::XScriptProviderFactory > xFactory(
                        m_xContext->getValueByName(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.script.provider.theMasterScriptProviderFactory")) ),
                        UNO_QUERY );
                    OSL_ENSURE( xFactory.is(), "SFURL_firing_impl: failed to get master script provider factory" );
                    if ( xFactory.is() )
                    {
                        Any aCtx;
                        aCtx <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("user"));
                        xScriptProvider.set( xFactory->createScriptProvider( aCtx ), UNO_QUERY );
                    }
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
        ::rtl::OUString sScriptURL;
        ::rtl::OUString sScriptCode( aScriptEvent.ScriptCode );

        if ( aScriptEvent.ScriptType.compareToAscii( "StarBasic" ) == 0 )
        {
            // StarBasic script: convert ScriptCode to scriptURL
            sal_Int32 nIndex = sScriptCode.indexOf( ':' );
            if ( nIndex >= 0 && nIndex < sScriptCode.getLength() )
            {
                sScriptURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.script:"));
                sScriptURL += sScriptCode.copy( nIndex + 1 );
                sScriptURL += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("?language=Basic&location="));
                sScriptURL += sScriptCode.copy( 0, nIndex );
            }
            ScriptEvent aSFScriptEvent( aScriptEvent );
            aSFScriptEvent.ScriptCode = sScriptURL;
            DialogSFScriptListenerImpl::firing_impl( aSFScriptEvent, pRet );
        }
    }

    void DialogUnoScriptListenerImpl::firing_impl( const ScriptEvent& aScriptEvent, Any* pRet )
    {
        static ::rtl::OUString sUnoURLScheme(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.UNO:"));

        ::rtl::OUString sScriptCode( aScriptEvent.ScriptCode );
        ::rtl::OUString aMethodName = aScriptEvent.ScriptCode.copy( sUnoURLScheme.getLength() );

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
            ResMgr* pResMgr = SFX_APP()->GetSfxResManager();
            if( pResMgr )
            {
                String aRes( ResId(STR_ERRUNOEVENTBINDUNG, *pResMgr) );
                ::rtl::OUString aQuoteChar( RTL_CONSTASCII_USTRINGPARAM( "\"" ) );

                ::rtl::OUString aOURes = aRes;
                sal_Int32 nIndex = aOURes.indexOf( '%' );

                ::rtl::OUString aOUFinal;
                aOUFinal += aOURes.copy( 0, nIndex );
                aOUFinal += aQuoteChar;
                aOUFinal += aMethodName;
                aOUFinal += aQuoteChar;
                aOUFinal += aOURes.copy( nIndex + 2 );

                ErrorBox( NULL, WinBits( WB_OK ), aOUFinal ).Execute();
            }
        }
    }

    // -----------------------------------------------------------------------------
    // XEventListener
    // -----------------------------------------------------------------------------

    void DialogScriptListenerImpl::disposing(const EventObject& ) throw ( RuntimeException )
    {
    }

    // -----------------------------------------------------------------------------
    // XScriptListener
    // -----------------------------------------------------------------------------

    void DialogScriptListenerImpl::firing( const ScriptEvent& aScriptEvent ) throw ( RuntimeException )
    {
        //::osl::MutexGuard aGuard( getMutex() );

        firing_impl( aScriptEvent, NULL );
    }

    // -----------------------------------------------------------------------------

    Any DialogScriptListenerImpl::approveFiring( const ScriptEvent& aScriptEvent )
        throw ( reflection::InvocationTargetException, RuntimeException )
    {
        //::osl::MutexGuard aGuard( getMutex() );

        Any aReturn;
        firing_impl( aScriptEvent, &aReturn );
        return aReturn;
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace dlgprov
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
