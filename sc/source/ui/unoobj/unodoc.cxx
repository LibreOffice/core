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

#include <rtl/ustring.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "appluno.hxx"
#include "scmod.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include "docsh.hxx"

using namespace ::com::sun::star;

OUString SAL_CALL ScDocument_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.SpreadsheetDocument" );
}

uno::Sequence< OUString > SAL_CALL ScDocument_getSupportedServiceNames() throw()
{
    uno::Sequence< OUString > aSeq( 1 );
    aSeq[0] = "com.sun.star.sheet.SpreadsheetDocument";
    return aSeq;
}

extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface* SAL_CALL
com_sun_star_comp_Calc_SpreadsheetDocument_get_implementation(::com::sun::star::uno::XComponentContext*,
                                                              ::com::sun::star::uno::Sequence<css::uno::Any> const &)
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    SfxObjectShell* pShell = new ScDocShell( SFX_CREATE_MODE_STANDARD );
    uno::Reference< uno::XInterface > model( pShell->GetModel() );
    model->acquire();
    return model.get();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
