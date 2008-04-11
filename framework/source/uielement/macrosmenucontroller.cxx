/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: macrosmenucontroller.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_framework.hxx"

#include <uielement/macrosmenucontroller.hxx>
#include <threadhelp/resetableguard.hxx>
#include "services.h"
#include <classes/resource.hrc>
#include <classes/fwkresid.hxx>
#include <helper/imageproducer.hxx>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::style;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;

namespace framework
{
class
DEFINE_XSERVICEINFO_MULTISERVICE        (   MacrosMenuController                    ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_MACROSMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   MacrosMenuController, {} )

MacrosMenuController::MacrosMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    PopupMenuControllerBase( xServiceManager ),
    m_xServiceManager( xServiceManager)
{
}

MacrosMenuController::~MacrosMenuController()
{
    OSL_TRACE("calling dtor");
}

// private function
void MacrosMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    VCLXPopupMenu* pVCLPopupMenu = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*     pPopupMenu    = 0;

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    resetPopupMenu( rPopupMenu );
    if ( pVCLPopupMenu )
        pPopupMenu = (PopupMenu *)pVCLPopupMenu->GetMenu();

    // insert basic
    String aCommand = String::CreateFromAscii( ".uno:MacroDialog" );
    String aDisplayName = RetrieveLabelFromCommand( aCommand );
    pPopupMenu->InsertItem( 2, aDisplayName );
    pPopupMenu->SetItemCommand( 2, aCommand );
    //pPopupMenu->SetHelpId( 2, HID_SVX_BASIC_MACRO_ORGANIZER );
    pPopupMenu->SetHelpId( 2, 40012 );

    // insert providers but not basic or java
    addScriptItems( pPopupMenu, 4);
}

// XEventListener
void SAL_CALL MacrosMenuController::disposing( const EventObject& ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    ResetableGuard aLock( m_aLock );
    OSL_TRACE("disposing");
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xServiceManager.clear();

    if ( m_xPopupMenu.is() )
    {
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
        OSL_TRACE("removed listener");
    }
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL MacrosMenuController::statusChanged( const FeatureStateEvent& ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );
    if ( m_xPopupMenu.is() )
    {
        fillPopupMenu( m_xPopupMenu );
    }
}

// XMenuListener
void SAL_CALL MacrosMenuController::highlight( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

void SAL_CALL MacrosMenuController::select( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    Reference< css::awt::XPopupMenu >   xPopupMenu;
    Reference< XDispatch >              xDispatch;
    Reference< XMultiServiceFactory >   xServiceManager;

    ResetableGuard aLock( m_aLock );
    xPopupMenu      = m_xPopupMenu;
    xDispatch       = m_xDispatch;
    xServiceManager = m_xServiceManager;
    aLock.unlock();

    if ( xPopupMenu.is() && xDispatch.is() )
    {
        VCLXPopupMenu* pVCLPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( xPopupMenu );
        if ( pVCLPopupMenu )
        {
            css::util::URL               aTargetURL;
            Sequence<PropertyValue>      aArgs;
            Reference< XURLTransformer > xURLTransformer( xServiceManager->createInstance(
                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                        UNO_QUERY );

            {
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
                PopupMenu* pPopupMenu = (PopupMenu *)pVCLPopupMenu->GetMenu();

                aTargetURL.Complete = pPopupMenu->GetItemCommand( rEvent.MenuId );
            }

            xURLTransformer->parseStrict( aTargetURL );

            // need to requery, since we handle more than one type of Command
            // if we don't do this only .uno:ScriptOrganizer commands are executed
            xDispatch = m_xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
            if( xDispatch.is() )
            {
                ExecuteInfo* pExecuteInfo = new ExecuteInfo;
                pExecuteInfo->xDispatch     = xDispatch;
                pExecuteInfo->aTargetURL    = aTargetURL;
                pExecuteInfo->aArgs         = aArgs;
//                xDispatch->dispatch( aTargetURL, aArgs );
                Application::PostUserEvent( STATIC_LINK(0, MacrosMenuController , ExecuteHdl_Impl), pExecuteInfo );
            }
            else
            {
            }
        }
    }
}


IMPL_STATIC_LINK_NOINSTANCE( MacrosMenuController, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
{
   try
   {
       // Asynchronous execution as this can lead to our own destruction!
       // Framework can recycle our current frame and the layout manager disposes all user interface
       // elements if a component gets detached from its frame!
       pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
   }
   catch ( Exception& )
   {
   }
   delete pExecuteInfo;
   return 0;
}

void SAL_CALL MacrosMenuController::activate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

void SAL_CALL MacrosMenuController::deactivate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

// XPopupMenuController
void SAL_CALL MacrosMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >( (OWeakObject*)this, UNO_QUERY ));

        Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                    UNO_QUERY );
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        m_xDispatchProvider = xDispatchProvider;
        com::sun::star::util::URL aTargetURL;
        aTargetURL.Complete = m_aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );
        m_xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        updatePopupMenu();
    }
}

// XInitialization
void SAL_CALL MacrosMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    PopupMenuControllerBase::initialize( aArguments );
}

String MacrosMenuController::RetrieveLabelFromCommand( const String& aCmdURL )
{
    String aLabel;

    // Retrieve popup menu labels
    if ( !m_aModuleIdentifier.getLength() )
    {
        Reference< XModuleManager > xModuleManager( ::comphelper::getProcessServiceFactory()->createInstance(
                                                        SERVICENAME_MODULEMANAGER ), UNO_QUERY_THROW );
        Reference< XInterface > xIfac( m_xFrame, UNO_QUERY );
        m_aModuleIdentifier = xModuleManager->identify( xIfac );

        if ( m_aModuleIdentifier.getLength() > 0 )
        {
            Reference< XNameAccess > xNameAccess( ::comphelper::getProcessServiceFactory()->createInstance(
                                                        SERVICENAME_UICOMMANDDESCRIPTION ), UNO_QUERY );
            if ( xNameAccess.is() )
            {
                Any a = xNameAccess->getByName( m_aModuleIdentifier );
                Reference< XNameAccess > xUICommands;
                a >>= m_xUICommandLabels;
            }
        }
    }

    if ( m_xUICommandLabels.is() )
    {
        try
        {
            if ( aCmdURL.Len() > 0 )
            {
                rtl::OUString aStr;
                Sequence< PropertyValue > aPropSeq;
                Any a( m_xUICommandLabels->getByName( aCmdURL ));
                if ( a >>= aPropSeq )
                {
                    for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                    {
                        if ( aPropSeq[i].Name.equalsAscii( "Label" ))
                        {
                            aPropSeq[i].Value >>= aStr;
                            break;
                        }
                    }
                }
                aLabel = aStr;
            }
        }
        catch ( com::sun::star::uno::Exception& )
        {
        }
    }

    return aLabel;
}

void MacrosMenuController::addScriptItems( PopupMenu* pPopupMenu, USHORT startItemId )
{
    const String aCmdBase = String::CreateFromAscii( ".uno:ScriptOrganizer?ScriptOrganizer.Language:string=" );
    const String ellipsis = String::CreateFromAscii( "..." );
    const ::rtl::OUString providerKey =
    ::rtl::OUString::createFromAscii("com.sun.star.script.provider.ScriptProviderFor" );
    const ::rtl::OUString languageProviderName =
        ::rtl::OUString::createFromAscii("com.sun.star.script.provider.LanguageScriptProvider" );
    USHORT itemId = startItemId;
    Reference< XContentEnumerationAccess > xEnumAccess = Reference< XContentEnumerationAccess >( m_xServiceManager, UNO_QUERY_THROW );
    Reference< XEnumeration > xEnum = xEnumAccess->createContentEnumeration ( languageProviderName );

    while ( xEnum->hasMoreElements() )
    {
        Reference< XServiceInfo > xServiceInfo;
        if ( sal_False == ( xEnum->nextElement() >>= xServiceInfo ) )
        {
            break;
        }
        Sequence< ::rtl::OUString > serviceNames = xServiceInfo->getSupportedServiceNames();

        if ( serviceNames.getLength() > 0 )
        {
            for ( sal_Int32 index = 0; index < serviceNames.getLength(); index++ )
            {
                if ( serviceNames[ index ].indexOf( providerKey ) == 0 )
                {
                    ::rtl::OUString serviceName = serviceNames[ index ];
                    String aCommand =  aCmdBase;
                    String aDisplayName = String( serviceName.copy( providerKey.getLength() ) );
                    if( aDisplayName.Equals( String::CreateFromAscii( "Java" ) ) || aDisplayName.Equals( String::CreateFromAscii( "Basic" ) ) )
                    {
                        // no entries for Java & Basic added elsewhere
                        break;
                    }
                    aCommand.Append( aDisplayName );
                    aDisplayName.Append( ellipsis );
                    pPopupMenu->InsertItem( itemId, aDisplayName );
                    pPopupMenu->SetItemCommand( itemId, aCommand );
                    //pPopupMenu->SetHelpId( itemId, HID_SVX_COMMON_MACRO_ORGANIZER );
                    pPopupMenu->SetHelpId( itemId, 40014 );
                    itemId++;
                    break;
                }
            }
        }
    }
}

}
