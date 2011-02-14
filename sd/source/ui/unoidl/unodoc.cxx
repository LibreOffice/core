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
#include "precompiled_sd.hxx"

// System - Includes -----------------------------------------------------

#include <tools/string.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "sdmod.hxx"
#include "DrawDocShell.hxx"
#include "GraphicDocShell.hxx"
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

// com.sun.star.comp.Draw.DrawingDocument

::rtl::OUString SAL_CALL SdDrawingDocument_getImplementationName() throw( uno::RuntimeException )
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.DrawingDocument" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL SdDrawingDocument_getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSeq( 2 );
    aSeq[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocument"));
    aSeq[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocumentFactory"));

    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SdDrawingDocument_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > &, const sal_uInt64 _nCreationFlags )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    SdDLL::Init();

    SfxObjectShell* pShell =
        new ::sd::GraphicDocShell(
            _nCreationFlags, sal_False, DOCUMENT_TYPE_DRAW );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

// com.sun.star.comp.Draw.PresentationDocument

::rtl::OUString SAL_CALL SdPresentationDocument_getImplementationName() throw( uno::RuntimeException )
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.PresentationDocument" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL SdPresentationDocument_getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSeq( 2 );
    aSeq[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocumentFactory"));
    aSeq[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PresentationDocument"));

    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SdPresentationDocument_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > &, const sal_uInt64 _nCreationFlags )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    SdDLL::Init();

    SfxObjectShell* pShell =
        new ::sd::DrawDocShell(
            _nCreationFlags, sal_False, DOCUMENT_TYPE_IMPRESS );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}


