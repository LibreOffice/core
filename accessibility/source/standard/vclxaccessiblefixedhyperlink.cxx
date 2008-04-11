/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxaccessiblefixedhyperlink.cxx,v $
 * $Revision: 1.3 $
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
#include "precompiled_accessibility.hxx"

// includes --------------------------------------------------------------
#include <accessibility/standard/vclxaccessiblefixedhyperlink.hxx>

using namespace ::com::sun::star;

// -----------------------------------------------------------------------------
// VCLXAccessibleFixedHyperlink
// -----------------------------------------------------------------------------

VCLXAccessibleFixedHyperlink::VCLXAccessibleFixedHyperlink( VCLXWindow* pVCLWindow )
    :VCLXAccessibleTextComponent( pVCLWindow )
{
}

// -----------------------------------------------------------------------------

VCLXAccessibleFixedHyperlink::~VCLXAccessibleFixedHyperlink()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleFixedHyperlink::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleTextComponent::FillAccessibleStateSet( rStateSet );
}

// -----------------------------------------------------------------------------

void VCLXAccessibleFixedHyperlink::implGetLineBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
{
    // TODO
    OCommonAccessibleText::implGetLineBoundary( rBoundary, nIndex );
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleFixedHyperlink::getImplementationName() throw (uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleFixedHyperlink" );
}

// -----------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > VCLXAccessibleFixedHyperlink::getSupportedServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleFixedHyperlink" );
    return aNames;
}

// -----------------------------------------------------------------------------

