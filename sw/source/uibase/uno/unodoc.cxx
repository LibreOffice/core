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

#include <sal/config.h>
#include <config_features.h>
#include <config_fuzzers.h>

#include <sfx2/sfxmodelfactory.hxx>
#include <swdll.hxx>
#include <docsh.hxx>
#include <globdoc.hxx>
#include <wdocsh.hxx>
#include <vcl/svapp.hxx>
#include <unomailmerge.hxx>

using namespace ::com::sun::star;

// com.sun.star.comp.Writer.TextDocument

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Writer_SwTextDocument_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const& args)
{
    SolarMutexGuard aGuard;
    SwGlobals::ensure();
    css::uno::Reference<css::uno::XInterface> xInterface = sfx2::createSfxModelInstance(args,
        [](SfxModelFlags _nCreationFlags)
        {
            rtl::Reference<SfxObjectShell> pShell = new SwDocShell(_nCreationFlags);
            return pShell->GetModel();
        });
    xInterface->acquire();
    return xInterface.get();
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_WebDocument_get_implementation(css::uno::XComponentContext*,
                                                        css::uno::Sequence<css::uno::Any> const &)
{
    SolarMutexGuard aGuard;
    SwGlobals::ensure();
    rtl::Reference<SfxObjectShell> pShell = new SwWebDocShell;
    uno::Reference< uno::XInterface > model( pShell->GetModel() );
    model->acquire();
    return model.get();
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_GlobalDocument_get_implementation(css::uno::XComponentContext*,
                                                           css::uno::Sequence<css::uno::Any> const &)
{
    SolarMutexGuard aGuard;
    SwGlobals::ensure();
    rtl::Reference<SfxObjectShell> pShell = new SwGlobalDocShell(SfxObjectCreateMode::STANDARD);
    uno::Reference< uno::XInterface > model( pShell->GetModel() );
    model->acquire();
    return model.get();
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
SwXMailMerge_get_implementation(css::uno::XComponentContext*,
                                css::uno::Sequence<css::uno::Any> const &)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    SolarMutexGuard aGuard;

    //the module may not be loaded
    SwGlobals::ensure();
    return cppu::acquire(new SwXMailMerge());
#else
    return nullptr;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
