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


#include "ctp_factory.hxx"
#include "ctp_panel.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppuhelper/supportsservice.hxx>

namespace sd { namespace colortoolpanel
{
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::ui::XUIElement;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::awt::XWindow;

    ToolPanelFactory::ToolPanelFactory( const Reference< XComponentContext >& i_rContext )
        :m_xContext( i_rContext )
    {
    }


    ToolPanelFactory::~ToolPanelFactory()
    {
    }


    Reference< XUIElement > SAL_CALL ToolPanelFactory::createUIElement( const OUString& i_rResourceURL, const Sequence< PropertyValue >& i_rArgs )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !i_rResourceURL.startsWith( "private:resource/toolpanel/org.openoffice.example.colorpanel/" ) )
            throw NoSuchElementException( i_rResourceURL, *this );

        const OUString sColor( i_rResourceURL.copy( i_rResourceURL.lastIndexOf( '/' ) + 1 ) );
        const sal_Int32 nPanelColor = static_cast< sal_Int32 >( sColor.toUInt32( 16 ) );

        // retrieve the parent window
        Reference< XWindow > xParentWindow;
        const PropertyValue* pArg = i_rArgs.getConstArray();
        const PropertyValue* pArgEnd = i_rArgs.getConstArray() + i_rArgs.getLength();
        for ( ; pArg != pArgEnd; ++pArg )
        {
            if ( pArg->Name == "ParentWindow" )
            {
                xParentWindow.set( pArg->Value, UNO_QUERY );
                break;
            }
        }
        if ( !xParentWindow.is() )
        {
            OSL_FAIL( "ToolPanelFactory::createUIElement: no parent window in the args!" );
            throw IllegalArgumentException(
                "No parent window provided in the creation arguments. Cannot create tool panel.",
                *this,
                2
            );
        }

        /// create the panel
        Reference< XUIElement > xUIElement( new PanelUIElement( m_xContext, xParentWindow, i_rResourceURL, nPanelColor ) );
        return xUIElement;
    }

    OUString SAL_CALL ToolPanelFactory::getImplementationName(  )
    {
        return getImplementationName_static();
    }

    OUString SAL_CALL ToolPanelFactory::getImplementationName_static(  )
    {
        return OUString( "org.openoffice.comp.example.custompanel.ToolPanelFactory" );
    }

    sal_Bool SAL_CALL ToolPanelFactory::supportsService( const OUString& i_rServiceName )
    {
        return cppu::supportsService(this, i_rServiceName);
    }

    Sequence< OUString > SAL_CALL ToolPanelFactory::getSupportedServiceNames()
    {
        return getSupportedServiceNames_static();
    }

    Sequence< OUString > SAL_CALL ToolPanelFactory::getSupportedServiceNames_static()
    {
        Sequence< OUString > aServiceNames(1);
        aServiceNames[0] = "org.openoffice.example.colorpanel.ToolPanelFactory";
        return aServiceNames;
    }


    Reference< XInterface > SAL_CALL ToolPanelFactory::Create( const Reference< XComponentContext >& i_rContext )
    {
        return *( new ToolPanelFactory( i_rContext ) );
    }


} } // namespace sd::colortoolpanel


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
