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

#include <sfx2/docfac.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include "swdll.hxx"
#include "docsh.hxx"
#include "globdoc.hxx"
#include "wdocsh.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

// com.sun.star.comp.Writer.TextDocument

uno::Sequence< OUString > SAL_CALL SwTextDocument_getSupportedServiceNames() throw()
{
    // return only top level services here! All others must be
    // resolved by rtti!
    uno::Sequence< OUString > aRet ( 1 );
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextDocument";

    return aRet;
}

OUString SAL_CALL SwTextDocument_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Writer.TextDocument" );
}

uno::Reference< uno::XInterface > SAL_CALL SwTextDocument_createInstance(
        const uno::Reference< lang::XMultiServiceFactory >&, const sal_uInt64 _nCreationFlags )
    throw( uno::Exception )
{
    SolarMutexGuard aGuard;
    SwGlobals::ensure();
    SfxObjectShell* pShell = new SwDocShell( _nCreationFlags );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

// com.sun.star.comp.Writer.WebDocument

uno::Sequence< OUString > SAL_CALL SwWebDocument_getSupportedServiceNames() throw()
{
    // return only top level services here! All others must be
    // resolved by rtti!
    uno::Sequence< OUString > aRet ( 1 );
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.WebDocument";

    return aRet;
}

OUString SAL_CALL SwWebDocument_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Writer.WebDocument" );
}

uno::Reference< uno::XInterface > SAL_CALL SwWebDocument_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & )
        throw( uno::Exception )
{
    SolarMutexGuard aGuard;
    SwGlobals::ensure();
    SfxObjectShell* pShell = new SwWebDocShell( SFX_CREATE_MODE_STANDARD );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

// com.sun.star.comp.Writer.GlobalDocument

uno::Sequence< OUString > SAL_CALL SwGlobalDocument_getSupportedServiceNames() throw()
{
    uno::Sequence< OUString > aRet ( 1 );
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.GlobalDocument";

    return aRet;
}

OUString SAL_CALL SwGlobalDocument_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Writer.GlobalDocument" );
}

uno::Reference< uno::XInterface > SAL_CALL SwGlobalDocument_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > &)
        throw( uno::Exception )
{
    SolarMutexGuard aGuard;
    SwGlobals::ensure();
    SfxObjectShell* pShell = new SwGlobalDocShell( SFX_CREATE_MODE_STANDARD );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
