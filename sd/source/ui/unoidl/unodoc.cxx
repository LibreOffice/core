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
#include "facreg.hxx"
#include "DrawDocShell.hxx"
#include "GraphicDocShell.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

// com.sun.star.comp.Draw.DrawingDocument

OUString SAL_CALL SdDrawingDocument_getImplementationName() throw( uno::RuntimeException )
{
    return OUString( "com.sun.star.comp.Draw.DrawingDocument" );
}

uno::Sequence< OUString > SAL_CALL SdDrawingDocument_getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSeq( 2 );
    aSeq[0] = "com.sun.star.drawing.DrawingDocument";
    aSeq[1] = "com.sun.star.drawing.DrawingDocumentFactory";

    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SdDrawingDocument_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > &, SfxModelFlags _nCreationFlags )
{
    SolarMutexGuard aGuard;

    SdDLL::Init();

    SfxObjectShell* pShell =
        new ::sd::GraphicDocShell(
            _nCreationFlags, false, DocumentType::Draw );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

// com.sun.star.comp.Draw.PresentationDocument

OUString SAL_CALL SdPresentationDocument_getImplementationName() throw( uno::RuntimeException )
{
    return OUString( "com.sun.star.comp.Draw.PresentationDocument" );
}

uno::Sequence< OUString > SAL_CALL SdPresentationDocument_getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSeq( 2 );
    aSeq[0] = "com.sun.star.drawing.DrawingDocumentFactory";
    aSeq[1] = "com.sun.star.presentation.PresentationDocument";

    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SdPresentationDocument_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > &, SfxModelFlags _nCreationFlags )
{
    SolarMutexGuard aGuard;

    SdDLL::Init();

    SfxObjectShell* pShell =
        new ::sd::DrawDocShell(
            _nCreationFlags, false, DocumentType::Impress );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
