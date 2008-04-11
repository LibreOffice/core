/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pcrcomponentcontext.hxx,v $
 * $Revision: 1.4 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PCROMPONENTCONTEXT_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_PCROMPONENTCONTEXT_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
/** === end UNO includes === **/

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

        /** constructs an instance
            @param _rxLegacyFactory
                the legacy service factor to obtain the <type scope="com::sun::star::uno">XComponentContext</type> from
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
        template < class INTERFACE >
        bool createComponent( const ::rtl::OUString& _rServiceName, ::com::sun::star::uno::Reference< INTERFACE >& _out_rxComponent ) const
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
            return createComponent( ::rtl::OUString::createFromAscii( _pAsciiServiceName ), _out_rxComponent );
        }

        /** creates a component using our component factory/context

            @throws ::com::sun::star::lang::ServiceNotRegisteredException
                if the given service is not registered
            @throws Exception
                if an exception occured during creating the component
            @return
                the newly created component. Is never <NULL/>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createComponent( const ::rtl::OUString& _rServiceName ) const;

        /** creates a component using our component factory/context

            @throws ::com::sun::star::lang::ServiceNotRegisteredException
                if the given service is not registered
            @throws Exception
                if an exception occured during creating the component
            @return
                the newly created component. Is never <NULL/>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createComponent( const sal_Char* _pAsciiServiceName ) const
        {
            return createComponent( ::rtl::OUString::createFromAscii( _pAsciiServiceName ) );
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
                getContextValueByName( const ::rtl::OUString& _rName ) const;

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
            return getContextValueByName( ::rtl::OUString::createFromAscii( _pAsciiName ) );
        }

        /** retrieve context to create interfaces by the ctors
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getContext() const { return        m_xContext;}

    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_PCROMPONENTCONTEXT_HXX

