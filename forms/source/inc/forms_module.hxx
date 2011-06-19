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

#ifndef FORMS_MODULE_INCLUDE_CONTEXT
    #error "not to be included directly! use 'foo_module.hxx instead'!"
#endif

#ifndef FORMS_MODULE_NAMESPACE
    #error "set FORMS_MODULE_NAMESPACE to your namespace identifier!"
#endif

#include <osl/mutex.hxx>
#include <tools/resid.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/factory.hxx>
#include <rtl/string.hxx>

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

    #define IMPLEMENT_SERVICE_REGISTRATION_8( classname, baseclass, service1, service2, service3, service4 , service5, service6, service7, service8 ) \
        IMPLEMENT_SERVICE_REGISTRATION_BASE( classname, baseclass ) \
        \
           Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames_Static() \
           { \
                   Sequence< ::rtl::OUString > aOwnNames( 8 ); \
                   aOwnNames[ 0 ] = service1; \
                   aOwnNames[ 1 ] = service2; \
                   aOwnNames[ 2 ] = service3; \
                   aOwnNames[ 3 ] = service4; \
                   aOwnNames[ 4 ] = service5; \
                   aOwnNames[ 5 ] = service6; \
                   aOwnNames[ 6 ] = service7; \
                   aOwnNames[ 6 ] = service8; \
            \
            return ::comphelper::concatSequences( \
                baseclass::getSupportedServiceNames_Static(), \
                aOwnNames \
            ); \
           } \

//.........................................................................
}   // namespace FORMS_MODULE_NAMESPACE
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
