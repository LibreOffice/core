/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unodoc.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:08:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

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

