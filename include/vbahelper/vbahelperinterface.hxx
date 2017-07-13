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
#ifndef INCLUDED_VBAHELPER_VBAHELPERINTERFACE_HXX
#define INCLUDED_VBAHELPER_VBAHELPERINTERFACE_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <ooo/vba/XHelperInterface.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbahelper.hxx>

namespace com { namespace sun { namespace star {
    namespace uno { class XComponentContext; } }
} }

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

template< typename... Ifc >
class SAL_DLLPUBLIC_TEMPLATE InheritedHelperInterfaceImpl : public Ifc...
{
protected:
    css::uno::WeakReference< ov::XHelperInterface > mxParent;
    css::uno::Reference< css::uno::XComponentContext > mxContext;
public:
    InheritedHelperInterfaceImpl( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext ) : mxParent( xParent ), mxContext( xContext ) {}
    virtual OUString getServiceImplName() = 0;
    virtual css::uno::Sequence<OUString> getServiceNames() = 0;

    // XHelperInterface Methods
    virtual ::sal_Int32 SAL_CALL getCreator() override
    {
        return 0x53756E4F;
    }
    virtual css::uno::Reference< ov::XHelperInterface > SAL_CALL getParent(  ) override { return mxParent; }

    virtual css::uno::Any SAL_CALL Application(  ) override {
            // The application could certainly be passed around in the context - seems
            // to make sense
            css::uno::Reference< css::container::XNameAccess > xNameAccess( mxContext, css::uno::UNO_QUERY_THROW );
            return xNameAccess->getByName( "Application" );
    }

    // XServiceInfo Methods
    virtual OUString SAL_CALL getImplementationName(  ) override { return getServiceImplName(); }
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override
    {
        css::uno::Sequence< OUString > sServices = getSupportedServiceNames();
        const OUString* pStart = sServices.getConstArray();
        const OUString* pEnd = pStart + sServices.getLength();
        for ( ; pStart != pEnd ; ++pStart )
            if ( *pStart == ServiceName )
                return true;
        return false;
    }
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override
    {
        css::uno::Sequence< OUString > aNames = getServiceNames();
        return aNames;
    }
 };

template <typename... Ifc >
class SAL_DLLPUBLIC_TEMPLATE InheritedHelperInterfaceWeakImpl : public InheritedHelperInterfaceImpl< ::cppu::WeakImplHelper< Ifc... > >
{
    typedef InheritedHelperInterfaceImpl< ::cppu::WeakImplHelper< Ifc... > > Base;
public:
    InheritedHelperInterfaceWeakImpl< Ifc... >( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext ) : Base( xParent, xContext ) {}
};


/** Helper macro to declare the methods 'getServiceImplName()' and
    'getServiceNames()' of the 'ooo.vba.XHelperInterface' interface in a class
    declaration.
 */
#define VBAHELPER_DECL_XHELPERINTERFACE \
    virtual OUString getServiceImplName() override; \
    virtual css::uno::Sequence< OUString > getServiceNames() override;


/** Helper macro to implement the methods 'getServiceImplName()' and
    'getServiceNames()' of the 'ooo.vba.XHelperInterface' interface. Will
    return the class name as service implementation name.
 */
#define VBAHELPER_IMPL_XHELPERINTERFACE( classname, servicename ) \
OUString classname::getServiceImplName() \
{ \
    return OUString( #classname ); \
} \
css::uno::Sequence< OUString > classname::getServiceNames() \
{ \
    static css::uno::Sequence< OUString > saServiceNames; \
    if( saServiceNames.getLength() == 0 ) \
    { \
        saServiceNames.realloc( 1 ); \
        saServiceNames[ 0 ] = servicename; \
    } \
    return saServiceNames; \
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
