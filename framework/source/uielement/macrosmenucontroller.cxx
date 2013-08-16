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


#include <uielement/macrosmenucontroller.hxx>
#include <uielement/menubarmanager.hxx>
#include <threadhelp/resetableguard.hxx>
#include "services.h"
#include <classes/resource.hrc>
#include <classes/fwkresid.hxx>
#include <framework/imageproducer.hxx>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <rtl/ustrbuf.hxx>
#include "helper/mischelper.hxx"
#include "helpid.hrc"
#include <osl/mutex.hxx>

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
DEFINE_XSERVICEINFO_MULTISERVICE_2      (   MacrosMenuController                    ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_MACROSMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   MacrosMenuController, {} )

MacrosMenuController::MacrosMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext ),
    m_xContext( xContext)
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

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pVCLPopupMenu )
        pPopupMenu = (PopupMenu *)pVCLPopupMenu->GetMenu();

    if (!pPopupMenu)
        return;

    // insert basic
    OUString aCommand(".uno:MacroDialog");
    String aDisplayName = RetrieveLabelFromCommand( aCommand );
    pPopupMenu->InsertItem( 2, aDisplayName );
    pPopupMenu->SetItemCommand( 2, aCommand );

    // insert providers but not basic or java
    addScriptItems( pPopupMenu, 4);
}

// XEventListener
void SAL_CALL MacrosMenuController::disposing( const EventObject& ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    OSL_TRACE("disposing");
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xContext.clear();

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
    osl::MutexGuard aLock( m_aMutex );
    if ( m_xPopupMenu.is() )
    {
        fillPopupMenu( m_xPopupMenu );
    }
}

// XMenuListener
void MacrosMenuController::impl_select(const Reference< XDispatch >& /*_xDispatch*/,const ::com::sun::star::util::URL& aTargetURL)
{
    // need to requery, since we handle more than one type of Command
    // if we don't do this only .uno:ScriptOrganizer commands are executed
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
    if( xDispatch.is() )
    {
        ExecuteInfo* pExecuteInfo = new ExecuteInfo;
        pExecuteInfo->xDispatch     = xDispatch;
        pExecuteInfo->aTargetURL    = aTargetURL;
        Application::PostUserEvent( STATIC_LINK(0, MacrosMenuController , ExecuteHdl_Impl), pExecuteInfo );
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
   catch ( const Exception& )
   {
   }
   delete pExecuteInfo;
   return 0;
}

OUString MacrosMenuController::RetrieveLabelFromCommand(const OUString& rCmdURL)
{
    sal_Bool bModuleIdentified = !m_aModuleIdentifier.isEmpty();
    return framework::RetrieveLabelFromCommand(rCmdURL, m_xContext, m_xUICommandLabels,m_xFrame,m_aModuleIdentifier,bModuleIdentified,"Label");
}

void MacrosMenuController::addScriptItems( PopupMenu* pPopupMenu, sal_uInt16 startItemId )
{
    const OUString aCmdBase(".uno:ScriptOrganizer?ScriptOrganizer.Language:string=");
    const OUString ellipsis( "..." );
    const OUString providerKey("com.sun.star.script.provider.ScriptProviderFor");
    const OUString languageProviderName("com.sun.star.script.provider.LanguageScriptProvider");
    sal_uInt16 itemId = startItemId;
    Reference< XContentEnumerationAccess > xEnumAccess = Reference< XContentEnumerationAccess >( m_xContext->getServiceManager(), UNO_QUERY_THROW );
    Reference< XEnumeration > xEnum = xEnumAccess->createContentEnumeration ( languageProviderName );

    while ( xEnum->hasMoreElements() )
    {
        Reference< XServiceInfo > xServiceInfo;
        if ( sal_False == ( xEnum->nextElement() >>= xServiceInfo ) )
        {
            break;
        }
        Sequence< OUString > serviceNames = xServiceInfo->getSupportedServiceNames();

        if ( serviceNames.getLength() > 0 )
        {
            for ( sal_Int32 index = 0; index < serviceNames.getLength(); index++ )
            {
                if ( serviceNames[ index ].indexOf( providerKey ) == 0 )
                {
                    OUString serviceName = serviceNames[ index ];
                    String aCommand =  aCmdBase;
                    String aDisplayName = String( serviceName.copy( providerKey.getLength() ) );
                    if( aDisplayName.Equals( OUString("Java") ) || aDisplayName.Equals( OUString("Basic") ) )
                    {
                        // no entries for Java & Basic added elsewhere
                        break;
                    }
                    aCommand.Append( aDisplayName );
                    aDisplayName.Append( ellipsis );
                    pPopupMenu->InsertItem( itemId, aDisplayName );
                    pPopupMenu->SetItemCommand( itemId, aCommand );
                    itemId++;
                    break;
                }
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
