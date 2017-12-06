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

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tvfactory.hxx>
#include <tvread.hxx>

using namespace treeview;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

TVFactory::TVFactory( const uno::Reference< XComponentContext >& xContext )
    : m_xContext( xContext )
{
}

TVFactory::~TVFactory()
{
}

// XServiceInfo methods.

OUString SAL_CALL
TVFactory::getImplementationName()
{
    return TVFactory::getImplementationName_static();
}

sal_Bool SAL_CALL TVFactory::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL
TVFactory::getSupportedServiceNames()
{
    return TVFactory::getSupportedServiceNames_static();
}

// XMultiServiceFactory

Reference< XInterface > SAL_CALL
TVFactory::createInstance(
    const OUString& aServiceSpecifier )
{
    uno::Sequence<uno::Any> seq(comphelper::InitAnyPropertySequence(
    {
        {"nodepath", uno::Any(OUString())}
    }));

    return createInstanceWithArguments( aServiceSpecifier, seq );
}

Reference< XInterface > SAL_CALL
TVFactory::createInstanceWithArguments(
    const OUString& /*ServiceSpecifier*/,
    const Sequence< Any >& Arguments )
{
    if( ! m_xHDS.is() )
    {
        cppu::OWeakObject* p = new TVChildTarget( m_xContext );
        m_xHDS.set( p );
    }

    OUString hierview;
    for( int i = 0; i < Arguments.getLength(); ++i )
    {
        PropertyValue pV;
        if( ! ( Arguments[i] >>= pV ) )
            continue;

        if( pV.Name != "nodepath" )
            continue;

        if( ! ( pV.Value >>= hierview ) )
            continue;

        break;
    }

    if( !hierview.isEmpty() )
    {
        Reference< XHierarchicalNameAccess > xhieraccess( m_xHDS,UNO_QUERY );
        Any aAny = xhieraccess->getByHierarchicalName( hierview );
        Reference< XInterface > xInterface;
        aAny >>= xInterface;
        return xInterface;
    }
    else
        return m_xHDS;
}

Sequence< OUString > SAL_CALL
TVFactory::getAvailableServiceNames( )
{
    Sequence<OUString> seq { "com.sun.star.ucb.HierarchyDataReadAccess" };
    return seq;
}

// static

OUString SAL_CALL
TVFactory::getImplementationName_static()
{
    return OUString( "com.sun.star.help.TreeViewImpl" );
}

Sequence< OUString > SAL_CALL
TVFactory::getSupportedServiceNames_static()
{
    Sequence< OUString > seq( 2 );
    seq[0] = "com.sun.star.help.TreeView";
    seq[1] = "com.sun.star.ucb.HiearchyDataSource";
    return seq;
}

Reference< XSingleServiceFactory >
TVFactory::createServiceFactory(
    const Reference< XMultiServiceFactory >& rxServiceMgr )
{
    return Reference< XSingleServiceFactory > (
        cppu::createSingleFactory(
            rxServiceMgr,
            TVFactory::getImplementationName_static(),
            TVFactory::CreateInstance,
            TVFactory::getSupportedServiceNames_static() ) );
}

Reference< XInterface > SAL_CALL
TVFactory::CreateInstance(
    const Reference< XMultiServiceFactory >& xMultiServiceFactory )
{
    XServiceInfo* xP = static_cast<XServiceInfo*>(new TVFactory( comphelper::getComponentContext(xMultiServiceFactory) ));
    return Reference< XInterface >::query( xP );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
