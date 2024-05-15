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
#include <cppuhelper/factory.hxx>
#include <tvfactory.hxx>
#include <tvread.hxx>
#include <utility>

using namespace treeview;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

TVFactory::TVFactory( uno::Reference< XComponentContext > xContext )
    : m_xContext(std::move( xContext ))
{
}

TVFactory::~TVFactory()
{
}

// XServiceInfo methods.

OUString SAL_CALL
TVFactory::getImplementationName()
{
    return u"com.sun.star.help.TreeViewImpl"_ustr;
}

sal_Bool SAL_CALL TVFactory::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL
TVFactory::getSupportedServiceNames()
{
    return { u"com.sun.star.help.TreeView"_ustr, u"com.sun.star.ucb.HiearchyDataSource"_ustr };
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
        m_xHDS = cppu::getXWeak(new TVChildTarget());
    }

    OUString hierview;
    for( const auto& rArgument : Arguments )
    {
        PropertyValue pV;
        if( ! ( rArgument >>= pV ) )
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
    return { u"com.sun.star.ucb.HierarchyDataReadAccess"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_help_TreeViewImpl_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new TVFactory(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
