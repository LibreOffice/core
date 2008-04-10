/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unodoc.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_sw.hxx"
// System - Includes -----------------------------------------------------

#include <tools/string.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include "swdll.hxx"
#include "docsh.hxx"
#include "globdoc.hxx"
#include "wdocsh.hxx"
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

//============================================================
// com.sun.star.comp.Writer.TextDocument

uno::Sequence< ::rtl::OUString > SAL_CALL SwTextDocument_getSupportedServiceNames() throw()
{
    // return only top level services here! All others must be
    // resolved by rtti!
    uno::Sequence< ::rtl::OUString > aRet ( 1 );
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.text.TextDocument" ) );

    return aRet;
}

::rtl::OUString SAL_CALL SwTextDocument_getImplementationName() throw()
{
    return ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.TextDocument" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SwTextDocument_createInstance(
        const uno::Reference< lang::XMultiServiceFactory >&, const sal_uInt64 _nCreationFlags )
    throw( uno::Exception )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SwDLL::Init();
    const SfxObjectCreateMode eCreateMode = ( _nCreationFlags & SFXMODEL_EMBEDDED_OBJECT ) ? SFX_CREATE_MODE_EMBEDDED : SFX_CREATE_MODE_STANDARD;
    const bool bScriptSupport = ( _nCreationFlags & SFXMODEL_DISABLE_EMBEDDED_SCRIPTS ) == 0;
    SfxObjectShell* pShell = new SwDocShell( eCreateMode, bScriptSupport );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

//============================================================
// com.sun.star.comp.Writer.WebDocument

uno::Sequence< ::rtl::OUString > SAL_CALL SwWebDocument_getSupportedServiceNames() throw()
{
    // return only top level services here! All others must be
    // resolved by rtti!
    uno::Sequence< ::rtl::OUString > aRet ( 1 );
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.text.WebDocument" ) );

    return aRet;
}

::rtl::OUString SAL_CALL SwWebDocument_getImplementationName() throw()
{
    return ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.WebDocument" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SwWebDocument_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & )
        throw( uno::Exception )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SwDLL::Init();
    SfxObjectShell* pShell = new SwWebDocShell( SFX_CREATE_MODE_STANDARD );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

//============================================================
// com.sun.star.comp.Writer.GlobalDocument

uno::Sequence< ::rtl::OUString > SAL_CALL SwGlobalDocument_getSupportedServiceNames() throw()
{
    uno::Sequence< ::rtl::OUString > aRet ( 1 );
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.text.GlobalDocument" ) );

    return aRet;
}

::rtl::OUString SAL_CALL SwGlobalDocument_getImplementationName() throw()
{
    return ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.GlobalDocument" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SwGlobalDocument_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > &)
        throw( uno::Exception )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SwDLL::Init();
    SfxObjectShell* pShell = new SwGlobalDocShell( SFX_CREATE_MODE_STANDARD );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

