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
#ifndef OOVBAAPI_VBA_HELPERINTERFACE_HXX
#define OOVBAAPI_VBA_HELPERINTERFACE_HXX

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <ooo/vba/XHelperInterface.hpp>
#include <vbahelper/vbahelper.hxx>
#include <com/sun/star/container/XNameAccess.hpp>

// use this class when you have an a object like
// interface  XAnInterface which contains XHelperInterface in its inheritance hierarchy
// interface XAnInterface
// {
//     interface XHelperInterface;
//     [attribute, string] name;
// }
// or
// interface XAnInterface : XHelperInterface;
// {
//     [attribute, string] name;
// }
//
// then this class can provide a default implementation of XHelperInterface,
// you can use it like this
// typedef InheritedHelperInterfaceImpl< XAnInterface > > AnInterfaceImpl_BASE;
// class AnInterfaceImpl : public AnInterfaceImpl_BASE
// {
// public:
//     AnInterface( const Reference< HelperInterface >& xParent ) : AnInterfaceImpl_BASE( xParent ) {}
//     // implement XAnInterface methods only, no need to implement the XHelperInterface
//     // methods
//     virtual void setName( const OUString& );
//     virtual OUString getName();
// }
//
const ::rtl::OUString sHelperServiceName( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.HelperServiceBase" ) );

template< typename Ifc1 >
class InheritedHelperInterfaceImpl : public Ifc1
{
protected:
    css::uno::WeakReference< ov::XHelperInterface > mxParent;
    css::uno::Reference< css::uno::XComponentContext > mxContext;
public:
    InheritedHelperInterfaceImpl() {}
    InheritedHelperInterfaceImpl( const css::uno::Reference< css::uno::XComponentContext >& xContext ) : mxContext( xContext ) {}
    InheritedHelperInterfaceImpl( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext ) : mxParent( xParent ), mxContext( xContext ) {}
    virtual rtl::OUString& getServiceImplName() = 0;
    virtual css::uno::Sequence<rtl::OUString> getServiceNames() = 0;

    // XHelperInterface Methods
    virtual ::sal_Int32 SAL_CALL getCreator() throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        return 0x53756E4F;
    }
    virtual css::uno::Reference< ov::XHelperInterface > SAL_CALL getParent(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException) { return mxParent; }

    virtual css::uno::Any SAL_CALL Application(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException) {
            // The application could certainly be passed around in the context - seems
            // to make sense
            css::uno::Reference< css::container::XNameAccess > xNameAccess( mxContext, css::uno::UNO_QUERY_THROW );
            return xNameAccess->getByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Application" ) ) );
    }

    // XServiceInfo Methods
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException) { return getServiceImplName(); }
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (css::uno::RuntimeException)
    {
        css::uno::Sequence< rtl::OUString > sServices = getSupportedServiceNames();
        const rtl::OUString* pStart = sServices.getConstArray();
        const rtl::OUString* pEnd = pStart + sServices.getLength();
        for ( ; pStart != pEnd ; ++pStart )
            if ( (*pStart).equals( ServiceName ) )
                return sal_True;
        return sal_False;
    }
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException)
    {
        css::uno::Sequence< rtl::OUString > aNames = getServiceNames();
        return aNames;
    }
 };

template< typename Ifc1 >
class InheritedHelperInterfaceImpl1 : public InheritedHelperInterfaceImpl< ::cppu::WeakImplHelper1< Ifc1 > >
{
    typedef InheritedHelperInterfaceImpl< ::cppu::WeakImplHelper1< Ifc1 > > Base;
public:
    InheritedHelperInterfaceImpl1< Ifc1 >() {}
    InheritedHelperInterfaceImpl1< Ifc1 >( const css::uno::Reference< css::uno::XComponentContext >& xContext ) : Base( xContext ) {}
    InheritedHelperInterfaceImpl1< Ifc1 >( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext ) : Base( xParent, xContext ) {}
};

template< typename Ifc1, typename Ifc2 >
class InheritedHelperInterfaceImpl2 : public InheritedHelperInterfaceImpl< ::cppu::WeakImplHelper2< Ifc1, Ifc2 > >
{
    typedef InheritedHelperInterfaceImpl< ::cppu::WeakImplHelper2< Ifc1, Ifc2 > > Base;
public:
    InheritedHelperInterfaceImpl2< Ifc1, Ifc2 >() {}
    InheritedHelperInterfaceImpl2< Ifc1, Ifc2 >( const css::uno::Reference< css::uno::XComponentContext >& xContext ) : Base( xContext ) {}
    InheritedHelperInterfaceImpl2< Ifc1, Ifc2 >( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext ) : Base( xParent, xContext ) {}
};

template< typename Ifc1, typename Ifc2, typename Ifc3 >
class InheritedHelperInterfaceImpl3 : public InheritedHelperInterfaceImpl< ::cppu::WeakImplHelper3< Ifc1, Ifc2, Ifc3 > >
{
    typedef InheritedHelperInterfaceImpl< ::cppu::WeakImplHelper3< Ifc1, Ifc2, Ifc3 > > Base;
public:
    InheritedHelperInterfaceImpl3< Ifc1, Ifc2, Ifc3 >() {}
    InheritedHelperInterfaceImpl3< Ifc1, Ifc2, Ifc3 >( const css::uno::Reference< css::uno::XComponentContext >& xContext ) : Base( xContext ) {}
    InheritedHelperInterfaceImpl3< Ifc1, Ifc2, Ifc3 >( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext ) : Base( xParent, xContext ) {}
};

// ============================================================================

/** Helper macro to implement the method 'getServiceImplName()' of the
    'ooo.vba.XHelperInterface' interface. Will return the class name as service
    implementation name.
 */
#define VBAHELPER_IMPL_GETSERVICEIMPLNAME( classname ) \
::rtl::OUString& classname::getServiceImplName() \
{ \
    static ::rtl::OUString saImplName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( #classname ) ); \
    return saImplName; \
}

// ----------------------------------------------------------------------------

/** Helper macro to implement the method 'getServiceNames()' for a single
    service name.
 */
#define VBAHELPER_IMPL_GETSERVICENAMES( classname, servicename ) \
css::uno::Sequence< ::rtl::OUString > classname::getServiceNames() \
{ \
    static css::uno::Sequence< ::rtl::OUString > saServiceNames; \
    if( saServiceNames.getLength() == 0 ) \
    { \
        saServiceNames.realloc( 1 ); \
        saServiceNames[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( servicename ) ); \
    } \
    return saServiceNames; \
}

// ----------------------------------------------------------------------------

/** Helper macro to declare the methods 'getServiceImplName()' and
    'getServiceNames()' of the 'ooo.vba.XHelperInterface' interface in a class
    declaration.
 */
#define VBAHELPER_DECL_XHELPERINTERFACE \
    virtual ::rtl::OUString& getServiceImplName(); \
    virtual css::uno::Sequence< ::rtl::OUString > getServiceNames();

// ----------------------------------------------------------------------------

/** Helper macro to implement the methods 'getServiceImplName()' and
    'getServiceNames()' of the 'ooo.vba.XHelperInterface' interface. Will
    return the class name as service implementation name.
 */
#define VBAHELPER_IMPL_XHELPERINTERFACE( classname, servicename ) \
VBAHELPER_IMPL_GETSERVICEIMPLNAME( classname ) \
VBAHELPER_IMPL_GETSERVICENAMES( classname, servicename )

// ============================================================================

#endif
