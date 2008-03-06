/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unodoc.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:45:45 $
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
#include "precompiled_starmath.hxx"

// System - Includes -----------------------------------------------------

#include <tools/string.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#include "smdll.hxx"
#include "document.hxx"

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace ::com::sun::star;

::rtl::OUString SAL_CALL SmDocument_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Math.FormulaDocument" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL SmDocument_getSupportedServiceNames() throw()
{
    uno::Sequence< rtl::OUString > aSeq( 1 );
    aSeq[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.formula.FormulaProperties" ));
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SmDocument_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & /*rSMgr*/, const sal_uInt64 _nCreationFlags ) throw( uno::Exception )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( !SM_MOD() )
        SmDLL::Init();

    const SfxObjectCreateMode eCreateMode = ( _nCreationFlags & SFXMODEL_EMBEDDED_OBJECT ) ? SFX_CREATE_MODE_EMBEDDED : SFX_CREATE_MODE_STANDARD;
    const bool bScriptSupport = ( _nCreationFlags & SFXMODEL_DISABLE_EMBEDDED_SCRIPTS ) == 0;

    SfxObjectShell* pShell = new SmDocShell( eCreateMode, bScriptSupport );
    if( pShell )
        return uno::Reference< uno::XInterface >( pShell->GetModel() );

    return uno::Reference< uno::XInterface >();
}


