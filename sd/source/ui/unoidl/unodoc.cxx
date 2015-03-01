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

#include "sddll.hxx"
#include "DrawDocShell.hxx"
#include "GraphicDocShell.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface* SAL_CALL
com_sun_star_comp_Draw_DrawingDocument_get_implementation(::com::sun::star::uno::XComponentContext* context,
                                                          ::com::sun::star::uno::Sequence<css::uno::Any>)
{
    SolarMutexGuard aGuard;

    SdDLL::Init();

    uno::Reference< lang::XMultiServiceFactory> xSM(context->getServiceManager(), uno::UNO_QUERY_THROW);

    SfxObjectShell* pShell = new ::sd::GraphicDocShell( SFXMODEL_STANDARD, false, DOCUMENT_TYPE_DRAW );
    uno::Reference< uno::XInterface > xModel( pShell->GetModel() );
    xModel->acquire();
    return xModel.get();
}


extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface* SAL_CALL
com_sun_star_comp_Draw_PresentationDocument_get_implementation(::com::sun::star::uno::XComponentContext* context,
                                                          ::com::sun::star::uno::Sequence<css::uno::Any>)
{
    SolarMutexGuard aGuard;

    SdDLL::Init();

    uno::Reference< lang::XMultiServiceFactory> xSM(context->getServiceManager(), uno::UNO_QUERY_THROW);

    SfxObjectShell* pShell = new ::sd::DrawDocShell( SFXMODEL_STANDARD, false, DOCUMENT_TYPE_IMPRESS );
    uno::Reference< uno::XInterface > xModel( pShell->GetModel() );
    xModel->acquire();
    return xModel.get();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
