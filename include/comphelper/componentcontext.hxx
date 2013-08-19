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

#ifndef COMPHELPER_COMPONENTCONTEXT_HXX
#define COMPHELPER_COMPONENTCONTEXT_HXX

#include <comphelper/comphelperdllapi.h>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//........................................................................
namespace comphelper
{
//........................................................................

    //====================================================================
    //= ComponentContext
    //====================================================================
    /** a helper class for working with a component context
    */
    class COMPHELPER_DLLPUBLIC ComponentContext
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

        /** constructs an instance
            @param _rxLegacyFactory
                the legacy service factor to obtain the com::sun::star::uno::XComponentContext from
            @throws ::com::sun::star::uno::RuntimeException
                if the given factory or does not have a DefaultContext property to obtain
                a component context
            @throws ::com::sun::star::lang::NullPointerException
                if the given factory is <NULL/>, or provides a component context being <NULL/>, or provides
                a component context whose component factory is <NULL/>
        */
        ComponentContext( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxLegacyFactory );

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
        template < typename INTERFACE >
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
        template < typename INTERFACE >
        bool createComponent( const sal_Char* _pAsciiServiceName, ::com::sun::star::uno::Reference< INTERFACE >& _out_rxComponent ) const
        {
            return createComponent( OUString::createFromAscii( _pAsciiServiceName ), _out_rxComponent );
        }

        /** creates a component using our component factory/context, passing creation arguments
            @throws ::com::sun::star::uno::Exception
            @return
                <TRUE/> if and only if the component could be successfully created
        */
        template < typename INTERFACE >
        bool createComponentWithArguments( const OUString& _rServiceName, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments, ::com::sun::star::uno::Reference< INTERFACE >& _out_rxComponent ) const
        {
            _out_rxComponent.clear();
            _out_rxComponent = _out_rxComponent.query(
                m_xORB->createInstanceWithArgumentsAndContext( _rServiceName, _rArguments, m_xContext )
            );
            return _out_rxComponent.is();
        }

        /** creates a component using our component factory/context, passing creation arguments
            @throws ::com::sun::star::uno::Exception
            @return
                <TRUE/> if and only if the component could be successfully created
        */
        template < typename INTERFACE >
        bool createComponentWithArguments( const sal_Char* _pAsciiServiceName, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments, ::com::sun::star::uno::Reference< INTERFACE >& _out_rxComponent ) const
        {
            return createComponentWithArguments( OUString::createFromAscii( _pAsciiServiceName ), _rArguments, _out_rxComponent );
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

        /** creates a component using our component factory/context, passing creation arguments

            @throws ::com::sun::star::lang::ServiceNotRegisteredException
                if the given service is not registered
            @throws Exception
                if an exception occurred during creating the component
            @return
                the newly created component. Is never <NULL/>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createComponentWithArguments(
            const OUString& _rServiceName,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments
        ) const;

        /** creates a component using our component factory/context, passing creation arguments

            @throws ::com::sun::star::lang::ServiceNotRegisteredException
                if the given service is not registered
            @throws Exception
                if an exception occurred during creating the component
            @return
                the newly created component. Is never <NULL/>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createComponentWithArguments(
                const sal_Char* _pAsciiServiceName,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments
            ) const
        {
            return createComponentWithArguments( OUString::createFromAscii( _pAsciiServiceName ), _rArguments );
        }

        /** retrieves a singleton instance from the context

            Singletons are collected below the <code>/singletons</code> key in a component context,
            so accessing them means retrieving the value under <code>/singletons/&lt;instance_name&gt;</code>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getSingleton( const OUString& _rInstanceName ) const;

        /** retrieves a singleton instance from the context

            Singletons are collected below the <code>/singletons</code> key in a component context,
            so accessing them means retrieving the value under <code>/singletons/&lt;instance_name&gt;</code>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getSingleton( const sal_Char* _pAsciiInstanceName ) const
        {
            return getSingleton( OUString::createFromAscii( _pAsciiInstanceName ) );
        }

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

    };

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_COMPONENTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
