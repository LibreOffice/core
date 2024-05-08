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
#include <services.h>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <officecfg/Office/Common.hxx>
#include <vcl/svapp.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <osl/mutex.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::container;

namespace framework
{

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL MacrosMenuController::getImplementationName()
{
    return u"com.sun.star.comp.framework.MacrosMenuController"_ustr;
}

sal_Bool SAL_CALL MacrosMenuController::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL MacrosMenuController::getSupportedServiceNames()
{
    return { SERVICENAME_POPUPMENUCONTROLLER };
}

MacrosMenuController::MacrosMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext ),
    m_xContext( xContext)
{
}

MacrosMenuController::~MacrosMenuController()
{
}

// private function
void MacrosMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    bool bMacrosDisabled = officecfg::Office::Common::Security::Scripting::DisableMacrosExecution::get();
    if (bMacrosDisabled)
        return;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu(rPopupMenu);
    assert(rPopupMenu->getItemCount() == 0);

    // insert basic
    OUString aCommand(u".uno:MacroDialog"_ustr);
    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(aCommand, m_aModuleName);
    OUString aDisplayName = vcl::CommandInfoProvider::GetMenuLabelForCommand(aProperties);
    rPopupMenu->insertItem(2, aDisplayName, 0, 0);
    rPopupMenu->setCommand(2, aCommand);

    // insert providers but not basic or java
    addScriptItems(rPopupMenu, 4);
}

// XEventListener
void SAL_CALL MacrosMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(this);

    std::unique_lock aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xContext.clear();

    if ( m_xPopupMenu.is() )
    {
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(this) );
    }
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL MacrosMenuController::statusChanged( const FeatureStateEvent& )
{
    std::unique_lock aLock( m_aMutex );
    if ( m_xPopupMenu.is() )
    {
        fillPopupMenu( m_xPopupMenu );
    }
}

void MacrosMenuController::addScriptItems(const Reference<css::awt::XPopupMenu>& rPopupMenu, sal_uInt16 startItemId)
{
    static constexpr OUStringLiteral aCmdBase(u".uno:ScriptOrganizer?ScriptOrganizer.Language:string=");
    static constexpr OUStringLiteral ellipsis( u"..." );
    static constexpr OUString providerKey(u"com.sun.star.script.provider.ScriptProviderFor"_ustr);
    sal_uInt16 itemId = startItemId;
    Reference< XContentEnumerationAccess > xEnumAccess( m_xContext->getServiceManager(), UNO_QUERY_THROW );
    Reference< XEnumeration > xEnum = xEnumAccess->createContentEnumeration ( u"com.sun.star.script.provider.LanguageScriptProvider"_ustr );

    sal_Int16 nPos = rPopupMenu->getItemCount();

    while ( xEnum->hasMoreElements() )
    {
        Reference< XServiceInfo > xServiceInfo;
        if ( !( xEnum->nextElement() >>= xServiceInfo ) )
        {
            break;
        }
        const Sequence< OUString > serviceNames = xServiceInfo->getSupportedServiceNames();

        for ( OUString const & serviceName : serviceNames )
        {
            if ( serviceName.startsWith( providerKey ) )
            {
                OUString aCommand = aCmdBase;
                OUString aDisplayName = serviceName.copy( providerKey.getLength() );
                if( aDisplayName == "Java" || aDisplayName == "Basic" )
                {
                    // no entries for Java & Basic added elsewhere
                    break;
                }
                aCommand += aDisplayName;
                aDisplayName += ellipsis;
                rPopupMenu->insertItem(itemId, aDisplayName, 0, nPos++);
                rPopupMenu->setCommand(itemId, aCommand);
                itemId++;
                break;
            }
        }
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_MacrosMenuController_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::MacrosMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
