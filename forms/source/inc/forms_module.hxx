/*************************************************************************
 *
 *  $RCSfile: forms_module.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 16:09:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef FORMS_MODULE_INCLUDE_CONTEXT
    #error "not to be included directly! use 'foo_module.hxx instead'!"
#endif

#ifndef FORMS_MODULE_NAMESPACE
    #error "set FORMS_MODULE_NAMESPACE to your namespace identifier!"
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

//.........................................................................
namespace FORMS_MODULE_NAMESPACE
{
//.........................................................................

    typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > (SAL_CALL *FactoryInstantiation)
        (
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rServiceManager,
            const ::rtl::OUString & _rComponentName,
            ::cppu::ComponentInstantiation _pCreateFunction,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString > & _rServiceNames,
            rtl_ModuleCount* _pModuleCounter
        );

    //=========================================================================
    //= OFormsModule
    //=========================================================================
    class OFormsModule
    {
    private:
        OFormsModule();
            // not implemented. OFormsModule is a static class

    protected:
        // auto registration administration
        static  ::com::sun::star::uno::Sequence< ::rtl::OUString >*
            s_pImplementationNames;
        static  ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > >*
            s_pSupportedServices;
        static  ::com::sun::star::uno::Sequence< sal_Int64 >*
            s_pCreationFunctionPointers;
        static  ::com::sun::star::uno::Sequence< sal_Int64 >*
            s_pFactoryFunctionPointers;

    public:
        /** register a component implementing a service with the given data.
            @param  _rImplementationName
                        the implementation name of the component
            @param  _rServiceNames
                        the services the component supports
            @param  _pCreateFunction
                        a function for creating an instance of the component
            @param  _pFactoryFunction
                        a function for creating a factory for that component
            @see revokeComponent
        */
        static void registerComponent(
            const ::rtl::OUString& _rImplementationName,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rServiceNames,
            ::cppu::ComponentInstantiation _pCreateFunction,
            FactoryInstantiation _pFactoryFunction);

        /** revoke the registration for the specified component
            @param  _rImplementationName
                the implementation name of the component
        */
        static void revokeComponent(
            const ::rtl::OUString& _rImplementationName);

        /** write the registration information of all known components
            <p>writes the registration information of all components which are currently registered into the
            specified registry.<p/>
            <p>Usually used from within component_writeInfo.<p/>
            @param  _rxServiceManager
                        the service manager
            @param  _rRootKey
                        the registry key under which the information will be stored
            @return
                        sal_True if the registration of all implementations was successfull, sal_False otherwise
        */
        static sal_Bool writeComponentInfos(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceManager,
            const ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >& _rRootKey);

        /** creates a Factory for the component with the given implementation name.
            <p>Usually used from within component_getFactory.<p/>
            @param  _rxServiceManager
                        a pointer to an XMultiServiceFactory interface as got in component_getFactory
            @param  _pImplementationName
                        the implementation name of the component
            @return
                        the XInterface access to a factory for the component
        */
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getComponentFactory(
            const ::rtl::OUString& _rImplementationName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceManager
            );

    private:
        /** ensure that the impl class exists
            @precond m_aMutex is guarded when this method gets called
        */
        static void ensureImpl();
    };

    //==========================================================================
    //= OMultiInstanceAutoRegistration
    //==========================================================================
    template <class TYPE>
    class OMultiInstanceAutoRegistration
    {
    public:
        /** automatically registeres a multi instance component
            <p>Assumed that the template argument has the three methods
                <ul>
                    <li><code>static ::rtl::OUString getImplementationName_Static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&)</code>
                        </li>
                <ul/>
            the instantiation of this object will automatically register the class via <method>OFormsModule::registerComponent</method>.
            <p/>
            <p>The factory creation function used is <code>::cppu::createSingleFactory</code>.</p>

            @see OOneInstanceAutoRegistration
        */
        OMultiInstanceAutoRegistration();
        ~OMultiInstanceAutoRegistration();
    };

    template <class TYPE>
    OMultiInstanceAutoRegistration<TYPE>::OMultiInstanceAutoRegistration()
    {
        OFormsModule::registerComponent(
            TYPE::getImplementationName_Static(),
            TYPE::getSupportedServiceNames_Static(),
            TYPE::Create,
            ::cppu::createSingleFactory
            );
    }

    template <class TYPE>
    OMultiInstanceAutoRegistration<TYPE>::~OMultiInstanceAutoRegistration()
    {
        OFormsModule::revokeComponent(TYPE::getImplementationName_Static());
    }

    //==========================================================================
    //= OOneInstanceAutoRegistration
    //==========================================================================
    template <class TYPE>
    class OOneInstanceAutoRegistration
    {
    public:
        /** automatically registeres a single instance component
            <p>Assumed that the template argument has the three methods
                <ul>
                    <li><code>static ::rtl::OUString getImplementationName_Static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&)</code>
                        </li>
                <ul/>
            the instantiation of this object will automatically register the class via <method>OFormsModule::registerComponent</method>.
            <p/>
            The factory creation function used is <code>::cppu::createOneInstanceFactory</code>.
            @see OOneInstanceAutoRegistration
        */
        OOneInstanceAutoRegistration();
        ~OOneInstanceAutoRegistration();
    };

    template <class TYPE>
    OOneInstanceAutoRegistration<TYPE>::OOneInstanceAutoRegistration()
    {
        OFormsModule::registerComponent(
            TYPE::getImplementationName_Static(),
            TYPE::getSupportedServiceNames_Static(),
            TYPE::Create,
            ::cppu::createOneInstanceFactory
            );
    }

    template <class TYPE>
    OOneInstanceAutoRegistration<TYPE>::~OOneInstanceAutoRegistration()
    {
        OFormsModule::revokeComponent(TYPE::getImplementationName_Static());
    }

    //==========================================================================
    //= helper for classes implementing the service handling via
    //= OMultiInstanceAutoRegistration or OOneInstanceAutoRegistration
    //==========================================================================
    #define DECLARE_SERVICE_REGISTRATION( classname ) \
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException); \
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException); \
        \
        static  ::rtl::OUString SAL_CALL getImplementationName_Static(); \
        static  ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static(); \
        static  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory ); \
        \
        friend class OOneInstanceAutoRegistration< classname >; \
        friend class OMultiInstanceAutoRegistration< classname >; \

    #define IMPLEMENT_SERVICE_REGISTRATION_BASE( classname, baseclass ) \
        \
        ::rtl::OUString SAL_CALL classname::getImplementationName(  ) throw ( RuntimeException ) \
        { return getImplementationName_Static(); } \
        \
        Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames(  ) throw (RuntimeException) \
        { \
            return ::comphelper::concatSequences( \
                getAggregateServiceNames(), \
                getSupportedServiceNames_Static() \
            ); \
        } \
        \
        ::rtl::OUString SAL_CALL classname::getImplementationName_Static() \
        { return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.forms."#classname ) ); } \
        \
        Reference< XInterface > SAL_CALL classname::Create( const Reference< XMultiServiceFactory >& _rxFactory ) \
        { return static_cast< XServiceInfo* >( new classname( _rxFactory ) ); } \
        \

    #define IMPLEMENT_SERVICE_REGISTRATION_1( classname, baseclass, service1 ) \
        IMPLEMENT_SERVICE_REGISTRATION_BASE( classname, baseclass ) \
        \
        Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames_Static() \
        { \
            Sequence< ::rtl::OUString > aOwnNames( 1 ); \
            aOwnNames[ 0 ] = service1; \
            \
            return ::comphelper::concatSequences( \
                baseclass::getSupportedServiceNames_Static(), \
                aOwnNames \
            ); \
        } \

    #define IMPLEMENT_SERVICE_REGISTRATION_2( classname, baseclass, service1, service2 ) \
        IMPLEMENT_SERVICE_REGISTRATION_BASE( classname, baseclass ) \
        \
        Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames_Static() \
        { \
            Sequence< ::rtl::OUString > aOwnNames( 2 ); \
            aOwnNames[ 0 ] = service1; \
            aOwnNames[ 1 ] = service2; \
            \
            return ::comphelper::concatSequences( \
                baseclass::getSupportedServiceNames_Static(), \
                aOwnNames \
            ); \
        } \

    #define IMPLEMENT_SERVICE_REGISTRATION_7( classname, baseclass, service1, service2, service3, service4 , service5, service6, service7 ) \
        IMPLEMENT_SERVICE_REGISTRATION_BASE( classname, baseclass ) \
        \
           Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames_Static() \
           { \
                   Sequence< ::rtl::OUString > aOwnNames( 7 ); \
                   aOwnNames[ 0 ] = service1; \
                   aOwnNames[ 1 ] = service2; \
                   aOwnNames[ 2 ] = service3; \
                   aOwnNames[ 3 ] = service4; \
                   aOwnNames[ 4 ] = service5; \
                   aOwnNames[ 5 ] = service6; \
                   aOwnNames[ 6 ] = service7; \
            \
            return ::comphelper::concatSequences( \
                baseclass::getSupportedServiceNames_Static(), \
                aOwnNames \
            ); \
           } \

//.........................................................................
}   // namespace FORMS_MODULE_NAMESPACE
//.........................................................................
