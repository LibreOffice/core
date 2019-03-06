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

#include <sfx2/sfxmodelfactory.hxx>

#include "register.hxx"
#include <smdll.hxx>
#include <document.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

OUString SmDocument_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Math.FormulaDocument" );
}

uno::Sequence< OUString > SmDocument_getSupportedServiceNames() throw()
{
    return uno::Sequence<OUString>{ "com.sun.star.formula.FormulaProperties" };
}

uno::Reference< uno::XInterface > SmDocument_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & /*rSMgr*/, SfxModelFlags _nCreationFlags )
{
    SolarMutexGuard aGuard;
    SmGlobals::ensure();
    SfxObjectShell* pShell = new SmDocShell( _nCreationFlags );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
