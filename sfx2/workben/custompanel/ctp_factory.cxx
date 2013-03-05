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

#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XComponent.hpp>

//......................................................................................................................
namespace sd { namespace colortoolpanel
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::lang::NotInitializedException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::ui::XUIElement;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::awt::XWindow;
    /** === end UNO using === **/

    //==================================================================================================================
    //= ToolPanelFactory
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ToolPanelFactory::ToolPanelFactory( const Reference< XComponentContext >& i_rContext )
        :m_xContext( i_rContext )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ToolPanelFactory::~ToolPanelFactory()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XUIElement > SAL_CALL ToolPanelFactory::createUIElement( const ::rtl::OUString& i_rResourceURL, const Sequence< PropertyValue >& i_rArgs ) throw (NoSuchElementException, IllegalArgumentException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !i_rResourceURL.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "private:resource/toolpanel/org.openoffice.example.colorpanel/" ) ) )
            throw NoSuchElementException( i_rResourceURL, *this );

        const ::rtl::OUString sColor( i_rResourceURL.copy( i_rResourceURL.lastIndexOf( '/' ) + 1 ) );
        const sal_Int32 nPanelColor = sColor.toInt32( 16 );

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
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "No parent window provided in the creation arguments. Cannot create tool panel." ) ),
                *this,
                2
            );
        }

        /// create the panel
        Reference< XUIElement > xUIElement( new PanelUIElement( m_xContext, xParentWindow, i_rResourceURL, nPanelColor ) );
        return xUIElement;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ToolPanelFactory::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ToolPanelFactory::getImplementationName_static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.example.custompanel.ToolPanelFactory" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL ToolPanelFactory::supportsService( const ::rtl::OUString& i_rServiceName ) throw (RuntimeException)
    {
        const Sequence< ::rtl::OUString > aServiceNames( getSupportedServiceNames() );
        for (   const ::rtl::OUString* serviceName = aServiceNames.getConstArray();
                serviceName != aServiceNames.getConstArray() + aServiceNames.getLength();
                ++serviceName
            )
        {
            if ( i_rServiceName == *serviceName )
                return  sal_True;
        }
        return sal_False;
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ToolPanelFactory::getSupportedServiceNames() throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ToolPanelFactory::getSupportedServiceNames_static() throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServiceNames(1);
        aServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.example.colorpanel.ToolPanelFactory" ) );
        return aServiceNames;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ToolPanelFactory::Create( const Reference< XComponentContext >& i_rContext ) throw (RuntimeException)
    {
        return *( new ToolPanelFactory( i_rContext ) );
    }

//......................................................................................................................
} } // namespace sd::colortoolpanel
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
