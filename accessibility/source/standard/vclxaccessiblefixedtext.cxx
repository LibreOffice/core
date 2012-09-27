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

#include <accessibility/standard/vclxaccessiblefixedtext.hxx>

#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <vcl/fixed.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;


// -----------------------------------------------------------------------------
// VCLXAccessibleFixedText
// -----------------------------------------------------------------------------

VCLXAccessibleFixedText::VCLXAccessibleFixedText( VCLXWindow* pVCLWindow )
    :VCLXAccessibleTextComponent( pVCLWindow )
{
}

// -----------------------------------------------------------------------------

VCLXAccessibleFixedText::~VCLXAccessibleFixedText()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleFixedText::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleTextComponent::FillAccessibleStateSet( rStateSet );

    if ( GetWindow() && GetWindow()->GetStyle() & WB_WORDBREAK )
        rStateSet.AddState( AccessibleStateType::MULTI_LINE );
}

// -----------------------------------------------------------------------------

void VCLXAccessibleFixedText::implGetLineBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
{
    // TODO
    OCommonAccessibleText::implGetLineBoundary( rBoundary, nIndex );
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

OUString VCLXAccessibleFixedText::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleFixedText" );
}

// -----------------------------------------------------------------------------

Sequence< OUString > VCLXAccessibleFixedText::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.awt.AccessibleFixedText";
    return aNames;
}

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
