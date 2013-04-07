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

#include "pcrcomponentcontext.hxx"

#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

//........................................................................
namespace pcr
{
//........................................................................

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
    Any ComponentContext::getContextValueByName( const OUString& _rName ) const
    {
        Any aReturn;
        try
        {
            aReturn = m_xContext->getValueByName( _rName );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "PropertyHandler::getContextValueByName: caught an exception!" );
        }
        return aReturn;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > ComponentContext::createComponent( const OUString& _rServiceName ) const
    {
        Reference< XInterface > xComponent(
            m_xORB->createInstanceWithContext( _rServiceName, m_xContext )
        );
        if ( !xComponent.is() )
            throw ServiceNotRegisteredException( _rServiceName, NULL );
        return xComponent;
    }

    //------------------------------------------------------------------------
    Reference< XMultiServiceFactory > ComponentContext::getLegacyServiceFactory() const
    {
        return Reference< XMultiServiceFactory >( m_xORB, UNO_QUERY_THROW );
    }

//........................................................................
} // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
