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

OUString VCLXAccessibleFixedHyperlink::getImplementationName() throw (uno::RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleFixedHyperlink" );
}

// -----------------------------------------------------------------------------

uno::Sequence< OUString > VCLXAccessibleFixedHyperlink::getSupportedServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.awt.AccessibleFixedHyperlink";
    return aNames;
}

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
