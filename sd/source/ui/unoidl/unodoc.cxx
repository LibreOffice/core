/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            _nCreationFlags, false, DOCUMENT_TYPE_DRAW );
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
            _nCreationFlags, false, DOCUMENT_TYPE_IMPRESS );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}


