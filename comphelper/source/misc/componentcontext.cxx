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
#include "precompiled_comphelper.hxx"
#include <comphelper/componentcontext.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

//........................................................................
namespace comphelper
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::lang::ServiceNotRegisteredException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    /** === end UNO using === **/

    //====================================================================
    //= ComponentContext
    //====================================================================
    //--------------------------------------------------------------------
    ComponentContext::ComponentContext( const Reference< XComponentContext >& _rxContext )
        :m_xContext( _rxContext )
    {
        if ( m_xContext.is() )
            m_xORB = m_xContext->getServiceManager();
        if ( !m_xORB.is() )
            throw NullPointerException();
    }

    //------------------------------------------------------------------------
    ComponentContext::ComponentContext( const Reference< XMultiServiceFactory >& _rxLegacyFactory )
    {
        if ( !_rxLegacyFactory.is() )
            throw NullPointerException();

        try
        {
            Reference< XPropertySet > xFactoryProperties( _rxLegacyFactory, UNO_QUERY_THROW );
            m_xContext = Reference< XComponentContext >(
                xFactoryProperties->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) ),
                UNO_QUERY );
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            throw RuntimeException();
        }

        if ( m_xContext.is() )
            m_xORB = m_xContext->getServiceManager();
        if ( !m_xORB.is() )
            throw NullPointerException();
    }

    //------------------------------------------------------------------------
    Any ComponentContext::getContextValueByName( const ::rtl::OUString& _rName ) const
    {
        Any aReturn;
        try
        {
            aReturn = m_xContext->getValueByName( _rName );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "ComponentContext::getContextValueByName: caught an exception!" );
        }
        return aReturn;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > ComponentContext::createComponent( const ::rtl::OUString& _rServiceName ) const
    {
        Reference< XInterface > xComponent(
            m_xORB->createInstanceWithContext( _rServiceName, m_xContext )
        );
        if ( !xComponent.is() )
            throw ServiceNotRegisteredException( _rServiceName, NULL );
        return xComponent;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > ComponentContext::createComponentWithArguments( const ::rtl::OUString& _rServiceName, const Sequence< Any >& _rArguments ) const
    {
        Reference< XInterface > xComponent(
            m_xORB->createInstanceWithArgumentsAndContext( _rServiceName, _rArguments, m_xContext )
        );
        if ( !xComponent.is() )
            throw ServiceNotRegisteredException( _rServiceName, NULL );
        return xComponent;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > ComponentContext::getSingleton( const ::rtl::OUString& _rInstanceName ) const
    {
        ::rtl::OUString sKey( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/singletons/" ) ) );
        sKey += _rInstanceName;
        return Reference< XInterface >( getContextValueByName( sKey ), UNO_QUERY );
    }

    //------------------------------------------------------------------------
    Reference< XMultiServiceFactory > ComponentContext::getLegacyServiceFactory() const
    {
        return Reference< XMultiServiceFactory >( m_xORB, UNO_QUERY_THROW );
    }

//........................................................................
} // namespace comphelper
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
