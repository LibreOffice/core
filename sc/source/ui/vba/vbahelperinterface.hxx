/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbahelperinterface.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:54:00 $
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
#ifndef SC_VBA_HELPERINTERFACE_HXX
#define SC_VBA_HELPERINTERFACE_HXX

#include <cppuhelper/implbase1.hxx>
#include <org/openoffice/vba/XHelperInterface.hpp>
#include "vbahelper.hxx"
#include "vbaglobals.hxx"

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
const ::rtl::OUString sHelperServiceName( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.vba.HelperServiceBase" ) );

template< typename Ifc1 >
class InheritedHelperInterfaceImpl : public Ifc1
{
protected:
    css::uno::WeakReference< oo::vba::XHelperInterface > mxParent;
    css::uno::Reference< css::uno::XComponentContext > mxContext;
public:
    InheritedHelperInterfaceImpl() {}
    InheritedHelperInterfaceImpl( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext ) : mxParent( xParent ), mxContext( xContext ) {}
    virtual rtl::OUString& getServiceImplName() = 0;
    virtual css::uno::Sequence<rtl::OUString> getServiceNames() = 0;

    // XHelperInterface Methods
    virtual ::sal_Int32 SAL_CALL getCreator() throw (css::script::BasicErrorException, css::uno::RuntimeException)
    {
        return 0x53756E4F;
    }
    virtual css::uno::Reference< oo::vba::XHelperInterface > SAL_CALL getParent(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException) { return mxParent; }

    virtual css::uno::Any SAL_CALL Application(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException) { return  css::uno::makeAny( ScVbaGlobals::getGlobalsImpl( mxContext )->getApplication() ); }


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
        css::uno::Sequence< rtl::OUString > aNames = getServiceNames();;
        return aNames;
    }
 };

template< typename Ifc1 >
class InheritedHelperInterfaceImpl1 : public InheritedHelperInterfaceImpl< ::cppu::WeakImplHelper1< Ifc1 > >

{
typedef InheritedHelperInterfaceImpl< ::cppu::WeakImplHelper1< Ifc1 > > Base;
public:
    InheritedHelperInterfaceImpl1< Ifc1 > ( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext ) : Base( xParent, xContext ) {}

};
#endif
