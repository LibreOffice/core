/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgevtatt.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-14 07:09:20 $
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

#ifndef SCRIPTING_DLGEVTATT_HXX
#include "dlgevtatt.hxx"
#endif

#ifndef SCRIPTING_DLGPROV_HXX
#include "dlgprov.hxx"
#endif
#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDIALOGEVENTHANDLER_HPP_
#include <com/sun/star/awt/XDialogEventHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSCRIPTEVENTSSUPPLIER_HPP_
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_PROVIDER_XSCRIPTPROVIDER_HPP_
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#endif
#ifndef  _COM_SUN_STAR_SCRIPT_PROVIDER_XSCRIPTPROVIDERFACTORY_HPP_
#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_PROVIDER_XSCRIPTPROVIDERSUPPLIER_HPP_
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_NOSUCHMETHODEXCEPTION_HPP_
#include <com/sun/star/lang/NoSuchMethodException.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XIDLMETHOD_HPP_
#include <com/sun/star/reflection/XIdlMethod.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_METHODCONCEPT_HPP_
#include <com/sun/star/beans/MethodConcept.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMATERIALHOLDER_HPP_
#include <com/sun/star/beans/XMaterialHolder.hpp>
#endif


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
//.........................................................................

    // =============================================================================
    // DialogEventsAttacherImpl
    // =============================================================================

    DialogEventsAttacherImpl::DialogEventsAttacherImpl( const Reference< XComponentContext >& rxContext )
        :m_xContext( rxContext )
    {
    }

    // -----------------------------------------------------------------------------

    DialogEventsAttacherImpl::~DialogEventsAttacherImpl()
    {
    }

    // -----------------------------------------------------------------------------
    // XScriptEventsAttacher
    // -----------------------------------------------------------------------------

    void SAL_CALL DialogEventsAttacherImpl::attachEvents( const Sequence< Reference< XInterface > >& Objects,
        const Reference< XScriptListener >& xListener, const Any& Helper )
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

        // go over all objects
        const Reference< XInterface >* pObjects = Objects.getConstArray();
        sal_Int32 nObjCount = Objects.getLength();
        for ( sal_Int32 i = 0; i < nObjCount; ++i )
        {
            // We know that we have to do with instances of XControl.
            // Otherwise this is not the right implementation for
            // XScriptEventsAttacher and we have to give up.
            Reference< XControl > xControl( pObjects[ i ], UNO_QUERY );
            if ( !xControl.is() )
                throw IllegalArgumentException();

            // get XEventsSupplier from control model
            Reference< XControlModel > xControlModel = xControl->getModel();
            Reference< XScriptEventsSupplier > xEventsSupplier( xControlModel, UNO_QUERY );
            if ( xEventsSupplier.is() )
            {
                Reference< container::XNameContainer > xEventCont = xEventsSupplier->getEvents();
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
                        Reference< XAllListener > xAllListener =
                            new DialogAllListenerImpl( xListener, aDesc.ScriptType, aDesc.ScriptCode );

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
                        catch ( IllegalArgumentException& )
                        {
                        }
                        catch ( IntrospectionException& )
                        {
                        }
                        catch ( CannotCreateAdapterException& )
                        {
                        }
                        catch ( ServiceNotRegisteredException& )
                        {
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
                        catch( IllegalArgumentException& )
                        {
                        }
                        catch( IntrospectionException& )
                        {
                        }
                        catch( CannotCreateAdapterException& )
                        {
                        }
                        catch( ServiceNotRegisteredException& )
                        {
                        }
                    }
                }
            }
        }
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
        ::osl::MutexGuard aGuard( getMutex() );

        firing_impl( Event, NULL );
    }

    // -----------------------------------------------------------------------------

    Any DialogAllListenerImpl::approveFiring( const AllEventObject& Event )
        throw ( reflection::InvocationTargetException, RuntimeException )
    {
        ::osl::MutexGuard aGuard( getMutex() );

        Any aReturn;
        firing_impl( Event, &aReturn );
        return aReturn;
    }


    // =============================================================================
    // DialogScriptListenerImpl
    // =============================================================================

    DialogScriptListenerImpl::DialogScriptListenerImpl( const Reference< XComponentContext >& rxContext,
            const Reference< ::com::sun::star::frame::XModel >& rxModel,
            const Reference< ::com::sun::star::awt::XDialog >& rxDialog,
            const Reference< ::com::sun::star::uno::XInterface >& rxHandler,
            const Reference< ::com::sun::star::beans::XIntrospectionAccess >& rxIntrospectionAccess )
        :m_xContext( rxContext )
        ,m_xModel( rxModel )
        ,m_xDialog( rxDialog )
        ,m_xHandler( rxHandler )
        ,m_xIntrospectionAccess( rxIntrospectionAccess )
    {
    }

    // -----------------------------------------------------------------------------

    DialogScriptListenerImpl::~DialogScriptListenerImpl()
    {
    }

    // -----------------------------------------------------------------------------

    void DialogScriptListenerImpl::firing_impl( const ScriptEvent& aScriptEvent, Any* pRet )
    {
        static ::rtl::OUString aVndSunStarUNO =
            ::rtl::OUString::createFromAscii( "vnd.sun.star.UNO:" );

        ::rtl::OUString sScriptURL;
        ::rtl::OUString sScriptCode( aScriptEvent.ScriptCode );

        bool bUNO = (sScriptCode.indexOf( aVndSunStarUNO ) == 0);
        if( bUNO )
        {
            handleUnoScript( aScriptEvent, pRet );
            return;
        }
        else
        {
            if ( aScriptEvent.ScriptType.compareToAscii( "StarBasic" ) == 0 )
            {
                // StarBasic script: convert ScriptCode to scriptURL
                sal_Int32 nIndex = sScriptCode.indexOf( ':' );
                if ( nIndex >= 0 && nIndex < sScriptCode.getLength() )
                {
                    sScriptURL = ::rtl::OUString::createFromAscii( "vnd.sun.star.script:" );
                    sScriptURL += sScriptCode.copy( nIndex + 1 );
                    sScriptURL += ::rtl::OUString::createFromAscii( "?language=Basic&location=" );
                    sScriptURL += sScriptCode.copy( 0, nIndex );
                }
            }
            else
            {
                // scripting framework script: ScriptCode contains scriptURL
                sScriptURL = sScriptCode;
            }
        }

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
                        ::rtl::OUString::createFromAscii( "/singletons/com.sun.star.script.provider.theMasterScriptProviderFactory" ) ),
                        UNO_QUERY );
                    OSL_ENSURE( xFactory.is(), "SFURL_firing_impl: failed to get master script provider factory" );
                    if ( xFactory.is() )
                    {
                        Any aCtx;
                        aCtx <<= ::rtl::OUString::createFromAscii( "user" );
                        xScriptProvider.set( xFactory->createScriptProvider( aCtx ), UNO_QUERY );
                    }
                }
            }

            OSL_ENSURE( xScriptProvider.is(), "DialogScriptListenerImpl::firing_impl: failed to get script provider" );

            if ( xScriptProvider.is() )
            {
                Reference< provider::XScript > xScript = xScriptProvider->getScript( sScriptURL );
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
        catch ( RuntimeException& e )
        {
            OSL_TRACE( "DialogScriptListenerImpl::firing_impl: caught RuntimeException reason %s",
                ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
        catch ( Exception& e )
        {
            OSL_TRACE( "DialogScriptListenerImpl::firing_impl: caught Exception reason %s",
                ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
    }

    void DialogScriptListenerImpl::handleUnoScript( const ScriptEvent& aScriptEvent, Any* pRet )
    {
        static ::rtl::OUString sUnoURLScheme = ::rtl::OUString::createFromAscii( "vnd.sun.star.UNO:" );

        ::rtl::OUString sScriptCode( aScriptEvent.ScriptCode );
        ::rtl::OUString aMethodName = aScriptEvent.ScriptCode.copy( sUnoURLScheme.getLength() );

        const Any* pArguments = aScriptEvent.Arguments.getConstArray();
        Any aEventObject = pArguments[0];

        Reference< XDialogEventHandler > xDialogEventHandler( m_xHandler, UNO_QUERY );
        bool bHandled = false;
        if( xDialogEventHandler.is() )
        {
            bHandled = xDialogEventHandler->callHandlerMethod( m_xDialog, aEventObject, aMethodName );
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
                    pArgs[0] <<= m_xDialog;
                    pArgs[1] = aEventObject;
                    aRet = rxMethod->invoke( aHandlerObject, Args );
                    bHandled = true;
                }
            }
            catch( com::sun::star::lang::IllegalArgumentException& e )
            {}
            catch( com::sun::star::lang::NoSuchMethodException& e )
            {}
            catch( com::sun::star::reflection::InvocationTargetException& e )
            {}
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
                String aRes( ResId(STR_ERRUNOEVENTBINDUNG, pResMgr) );
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
        ::osl::MutexGuard aGuard( getMutex() );

        firing_impl( aScriptEvent, NULL );
    }

    // -----------------------------------------------------------------------------

    Any DialogScriptListenerImpl::approveFiring( const ScriptEvent& aScriptEvent )
        throw ( reflection::InvocationTargetException, RuntimeException )
    {
        ::osl::MutexGuard aGuard( getMutex() );

        Any aReturn;
        firing_impl( aScriptEvent, &aReturn );
        return aReturn;
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace dlgprov
//.........................................................................
