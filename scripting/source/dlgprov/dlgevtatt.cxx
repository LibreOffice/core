/*************************************************************************
 *
 *  $RCSfile: dlgevtatt.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 13:27:06 $
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

#ifndef SCRIPTING_DLGEVTATT_HXX
#include "dlgevtatt.hxx"
#endif

#ifndef SCRIPTING_DLGPROV_HXX
#include "dlgprov.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
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


using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;


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
                            Reference< XEventListener > xListener = m_xEventAttacher->attachSingleEventListener(
                                xControlModel, xAllListener, Helper, aDesc.ListenerType,
                                aDesc.AddListenerParam, aDesc.EventMethod );

                            if ( xListener.is() )
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
                                Reference< XEventListener > xListener = m_xEventAttacher->attachSingleEventListener(
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
        aScriptEvent.Arguments      = Event.Arguments;
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
            const Reference< ::com::sun::star::frame::XModel >& rxModel )
        :m_xContext( rxContext )
        ,m_xModel( rxModel )
    {
    }

    // -----------------------------------------------------------------------------

    DialogScriptListenerImpl::~DialogScriptListenerImpl()
    {
    }

    // -----------------------------------------------------------------------------

    void DialogScriptListenerImpl::firing_impl( const ScriptEvent& aScriptEvent, Any* pRet )
    {
        ::rtl::OUString sScriptURL;
        ::rtl::OUString sScriptCode( aScriptEvent.ScriptCode );

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

        OSL_TRACE( "DialogScriptListenerImpl::firing_impl: processing scriptURL %s",
            ::rtl::OUStringToOString( sScriptURL, RTL_TEXTENCODING_ASCII_US ).pData->buffer );

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
