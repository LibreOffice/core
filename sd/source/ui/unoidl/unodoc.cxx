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
#include <com/sun/star/frame/XModel.hpp>

#include <sddll.hxx>
#include <DrawDocShell.hxx>
#include <GraphicDocShell.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

// com.sun.star.comp.Draw.DrawingDocument

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
sd_DrawingDocument_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const& args)
{
    SolarMutexGuard aGuard;

    SdDLL::Init();

    css::uno::Reference<css::uno::XInterface> xInterface = sfx2::createSfxModelInstance(args,
        [](SfxModelFlags _nCreationFlags)
        {
            SfxObjectShell* pShell = new ::sd::GraphicDocShell( _nCreationFlags );
            return uno::Reference< uno::XInterface >( pShell->GetModel() );
        });
    xInterface->acquire();
    return xInterface.get();
}


// com.sun.star.comp.Draw.PresentationDocument

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
sd_PresentationDocument_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const& args)
{
    SolarMutexGuard aGuard;

    SdDLL::Init();

    css::uno::Reference<css::uno::XInterface> xInterface = sfx2::createSfxModelInstance(args,
        [](SfxModelFlags _nCreationFlags)
        {
            SfxObjectShell* pShell =
                new ::sd::DrawDocShell(
                    _nCreationFlags, false, DocumentType::Impress );
            return pShell->GetModel();
        });
    xInterface->acquire();
    return xInterface.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
