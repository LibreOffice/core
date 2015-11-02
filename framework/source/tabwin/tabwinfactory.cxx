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

#include <tabwin/tabwinfactory.hxx>
#include <tabwin/tabwindow.hxx>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

//  Defines

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

namespace framework
{

//  XInterface, XTypeProvider, XServiceInfo

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE_2(   TabWinFactory                                   ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_TABWINFACTORY                       ,
                                            IMPLEMENTATIONNAME_TABWINFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   TabWinFactory, {} )

TabWinFactory::TabWinFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    m_xContext( xContext )
{
}

TabWinFactory::~TabWinFactory()
{
}

css::uno::Reference< css::uno::XInterface > SAL_CALL TabWinFactory::createInstanceWithContext(
    const css::uno::Reference< css::uno::XComponentContext >& xContext )
throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence< css::uno::Any > aArgs;

    return createInstanceWithArgumentsAndContext( aArgs, xContext );
}

css::uno::Reference< css::uno::XInterface > SAL_CALL TabWinFactory::createInstanceWithArgumentsAndContext(
    const css::uno::Sequence< css::uno::Any >& Arguments, const css::uno::Reference< css::uno::XComponentContext >& )
throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    const OUString aTopWindowArgName( "TopWindow");

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexResettableGuard aLock;
    css::uno::Reference< css::awt::XToolkit2 > xToolkit = m_xToolkit;
    css::uno::Reference< css::uno::XComponentContext > xContext( m_xContext );
    aLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    css::uno::Reference< css::uno::XInterface > xReturn;
    css::uno::Reference< css::awt::XTopWindow > xTopWindow;
    css::beans::PropertyValue                   aPropValue;

    for ( sal_Int32 i = 0; i < Arguments.getLength(); i++ )
    {
        if ( Arguments[i] >>= aPropValue )
        {
            if ( aPropValue.Name == aTopWindowArgName )
                aPropValue.Value >>= xTopWindow;
        }
    }

    if ( !xToolkit.is() && xContext.is() )
    {
        xToolkit = css::awt::Toolkit::create( xContext );
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        aLock.reset();
        m_xToolkit = xToolkit;
        aLock.clear();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    }

    if ( !xTopWindow.is() )
    {
        // describe window properties.
        css::awt::WindowDescriptor aDescriptor;
        aDescriptor.Type                =   css::awt::WindowClass_TOP;
        aDescriptor.ParentIndex         =   -1;
        aDescriptor.Parent.clear();
        aDescriptor.Bounds              =   css::awt::Rectangle(0,0,0,0);
        aDescriptor.WindowAttributes    =   css::awt::WindowAttribute::BORDER|
                                            css::awt::WindowAttribute::SIZEABLE|
                                            css::awt::WindowAttribute::MOVEABLE|
                                            css::awt::WindowAttribute::CLOSEABLE|
                                            css::awt::WindowAttribute::MINSIZE;

        // create a parent window
        xTopWindow.set( xToolkit->createWindow( aDescriptor ), css::uno::UNO_QUERY );
    }

    if ( xTopWindow.is() )
    {
        TabWindow* pTabWindow = new TabWindow( xContext );

        css::uno::Sequence< css::uno::Any > aArgs( 1 );

        aPropValue.Name  = aTopWindowArgName;
        aPropValue.Value = css::uno::makeAny( xTopWindow );
        aArgs[0] = css::uno::makeAny( aPropValue );
        pTabWindow->initialize( aArgs );

        xReturn.set( static_cast< OWeakObject* >( pTabWindow ), css::uno::UNO_QUERY );
    }

    return xReturn;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
