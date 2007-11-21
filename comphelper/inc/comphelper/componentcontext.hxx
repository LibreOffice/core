/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: componentcontext.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 16:51:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef COMPHELPER_COMPONENTCONTEXT_HXX
#define COMPHELPER_COMPONENTCONTEXT_HXX

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include <comphelper/comphelperdllapi.h>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
/** === end UNO includes === **/

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
        template < typename INTERFACE >
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
        template < typename INTERFACE >
        bool createComponent( const sal_Char* _pAsciiServiceName, ::com::sun::star::uno::Reference< INTERFACE >& _out_rxComponent ) const
        {
            return createComponent( ::rtl::OUString::createFromAscii( _pAsciiServiceName ), _out_rxComponent );
        }

        /** creates a component using our component factory/context, passing creation arguments
            @throws ::com::sun::star::uno::Exception
            @return
                <TRUE/> if and only if the component could be successfully created
        */
        template < typename INTERFACE >
        bool createComponentWithArguments( const ::rtl::OUString& _rServiceName, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments, ::com::sun::star::uno::Reference< INTERFACE >& _out_rxComponent ) const
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
            return createComponentWithArguments( ::rtl::OUString::createFromAscii( _pAsciiServiceName ), _rArguments, _out_rxComponent );
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

        /** creates a component using our component factory/context, passing creation arguments

            @throws ::com::sun::star::lang::ServiceNotRegisteredException
                if the given service is not registered
            @throws Exception
                if an exception occured during creating the component
            @return
                the newly created component. Is never <NULL/>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createComponentWithArguments(
            const ::rtl::OUString& _rServiceName,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments
        ) const;

        /** creates a component using our component factory/context, passing creation arguments

            @throws ::com::sun::star::lang::ServiceNotRegisteredException
                if the given service is not registered
            @throws Exception
                if an exception occured during creating the component
            @return
                the newly created component. Is never <NULL/>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createComponentWithArguments(
                const sal_Char* _pAsciiServiceName,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments
            ) const
        {
            return createComponentWithArguments( ::rtl::OUString::createFromAscii( _pAsciiServiceName ), _rArguments );
        }

        /** retrieves a singleton instance from the context

            Singletons are collected below the <code>/singletons</code> key in a component context,
            so accessing them means retrieving the value under <code>/singletons/&lt;instance_name&gt;</code>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getSingleton( const ::rtl::OUString& _rInstanceName ) const;

        /** retrieves a singleton instance from the context

            Singletons are collected below the <code>/singletons</code> key in a component context,
            so accessing them means retrieving the value under <code>/singletons/&lt;instance_name&gt;</code>.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getSingleton( const sal_Char* _pAsciiInstanceName ) const
        {
            return getSingleton( ::rtl::OUString::createFromAscii( _pAsciiInstanceName ) );
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

    };

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_COMPONENTCONTEXT_HXX

