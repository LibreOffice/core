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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PCROMPONENTCONTEXT_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_PCROMPONENTCONTEXT_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= ComponentContext
    //====================================================================
    /** a helper class for working with a component context
    */
    class ComponentContext
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiComponentFactory >  m_xORB;

    public:
        /** constructs an instance
            @param _rxContext
                the component context to manage
            @throws ::com::sun::star::lang::NullPointerException
                if the given context, or its component factory, are <NULL/>
        */
        ComponentContext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );

        /** returns the ->XComponentContext interface
        */
        inline ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
            getUNOContext() const { return m_xContext; }

        /** determines whether the context is not <NULL/>
        */
        inline sal_Bool is() const
        {
            return m_xContext.is();
        }

        /** creates a component using our component factory/context
            @throws ::com::sun::star::uno::Exception
            @return
                <TRUE/> if and only if the component could be successfully created
        */
        template < class INTERFACE >
        bool createComponent( const OUString& _rServiceName, ::com::sun::star::uno::Reference< INTERFACE >& _out_rxComponent ) const
        {
            _out_rxComponent.clear();
            _out_rxComponent = _out_rxComponent.query(
                m_xORB->createInstanceWithContext( _rServiceName, m_xContext )
            );
            return _out_rxComponent.is();
        }

        /** creates a component using our component factory/context
            @throws ::com::sun::star::uno::Exception
            @return
                <TRUE/> if and only if the component could be successfully created
        */
        template < class INTERFACE >
        bool createComponent( const sal_Char* _pAsciiServiceName, ::com::sun::star::uno::Reference< INTERFACE >& _out_rxComponent ) const
        {
            return createComponent( OUString::createFromAscii( _pAsciiServiceName ), _out_rxComponent );
        }

        /** creates a component using our component factory/context

            @throws ::com::sun::star::lang::ServiceNotRegisteredException
                if the given service is not registered
            @throws Exception
                if an exception occurred during creating the component
            @return
                the newly created component. Is never <NULL/>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createComponent( const OUString& _rServiceName ) const;

        /** creates a component using our component factory/context

            @throws ::com::sun::star::lang::ServiceNotRegisteredException
                if the given service is not registered
            @throws Exception
                if an exception occurred during creating the component
            @return
                the newly created component. Is never <NULL/>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createComponent( const sal_Char* _pAsciiServiceName ) const
        {
            return createComponent( OUString::createFromAscii( _pAsciiServiceName ) );
        }

        /** returns the ->XMultiServiceFactory interface of ->m_xORB, for passing to
            older code which does not yet support ->XMultiComponentFactory
            @throws ::com::sun::star::uno::RuntimeException
                if our our component factory does not support this interface
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
            getLegacyServiceFactory() const;

        /** retrieves a value from our component context
            @param _rName
                the name of the value to retrieve
            @return
                the context value with the given name
            @seealso XComponentContext::getValueByName
            @seealso getContextValueByAsciiName
        */
        ::com::sun::star::uno::Any
                getContextValueByName( const OUString& _rName ) const;

        /** retrieves a value from our component context, specified by 8-bit ASCII string
            @param _rName
                the name of the value to retrieve, as ASCII character string
            @return
                the context value with the given name
            @seealso XComponentContext::getValueByName
            @seealso getContextValueByName
        */
        inline ::com::sun::star::uno::Any
                getContextValueByAsciiName( const sal_Char* _pAsciiName ) const
        {
            return getContextValueByName( OUString::createFromAscii( _pAsciiName ) );
        }

        /** retrieve context to create interfaces by the ctors
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getContext() const { return        m_xContext;}

    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_PCROMPONENTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
