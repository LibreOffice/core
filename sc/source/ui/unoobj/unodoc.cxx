/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sc.hxx"

// System - Includes -----------------------------------------------------

#include <tools/string.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "scmod.hxx"
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

#include "docsh.hxx"

using namespace ::com::sun::star;

::rtl::OUString SAL_CALL ScDocument_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.SpreadsheetDocument" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScDocument_getSupportedServiceNames() throw()
{
    uno::Sequence< rtl::OUString > aSeq( 1 );
    aSeq[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.SpreadsheetDocument" ));
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScDocument_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & /* rSMgr */, const sal_uInt64 _nCreationFlags ) throw( uno::Exception )
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    SfxObjectShell* pShell = new ScDocShell( _nCreationFlags );
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
