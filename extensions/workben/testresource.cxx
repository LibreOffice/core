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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include <smart/com/sun/star/registry/XImplementationRegistration.hxx>
#include <smart/com/sun/star/script/XInvocation.hxx>

#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <usr/services.hxx>
#include <vcl/svapp.hxx>

using namespace usr;
using ::rtl::StringToOUString;

class MyApp : public Application
{
public:
    void        Main();
};

MyApp aMyApp;

// -----------------------------------------------------------------------

void MyApp::Main()
{
    XMultiServiceFactoryRef xSMgr = createRegistryServiceManager();
    registerUsrServices( xSMgr );
    setProcessServiceManager( xSMgr );

    XInterfaceRef x = xSMgr->createInstance( L"com.sun.star.registry.ImplementationRegistration" );
    XImplementationRegistrationRef xReg( x, USR_QUERY );
    sal_Char szBuf[1024];
    ORealDynamicLoader::computeModuleName( "res", szBuf, 1024 );
    UString aDllName( StringToOUString( szBuf, CHARSET_SYSTEM ) );
    xReg->registerImplementation( L"com.sun.star.loader.SharedLibrary", aDllName, XSimpleRegistryRef() );

    x = xSMgr->createInstance( L"com.sun.star.resource.VclStringResourceLoader" );
    XInvocationRef xResLoader( x, USR_QUERY );
    XIntrospectionAccessRef xIntrospection = xResLoader->getIntrospection();
    UString aFileName( L"TestResource" );
    UsrAny aVal;
    aVal.setString( aFileName );
    xResLoader->setValue( L"FileName", aVal );

    Sequence< UsrAny > Args( 1 );
    Sequence< INT16 > OutPos;
    Sequence< UsrAny > OutArgs;
    Args.getArray()[0].setINT32( 1000 );

    BOOL b = xResLoader->invoke( L"hasString", Args, OutPos, OutArgs ).getBOOL();
    OSL_ENSURE( b, "hasString" );

    UString aStr = xResLoader->invoke( L"getString", Args, OutPos, OutArgs ).getString();
    OSL_ENSURE( aStr == L"Hello", "getString" );

    Args.getArray()[0].setINT32( 1001 );
    b = xResLoader->invoke( L"hasString", Args, OutPos, OutArgs ).getBOOL();
    OSL_ENSURE( !b, "!hasString" );

    xReg->revokeImplementation( aDllName, XSimpleRegistryRef() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
