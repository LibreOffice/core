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

#include <accessibility/standard/vclxaccessiblecombobox.hxx>
#include <accessibility/standard/vclxaccessiblelist.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <vcl/svapp.hxx>
#include <vcl/combobox.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;


VCLXAccessibleComboBox::VCLXAccessibleComboBox (VCLXWindow* pVCLWindow)
    : VCLXAccessibleBox (pVCLWindow, VCLXAccessibleBox::COMBOBOX, false)
{
}




VCLXAccessibleComboBox::~VCLXAccessibleComboBox (void)
{
}




bool VCLXAccessibleComboBox::IsValid (void) const
{
    return static_cast<ComboBox*>(GetWindow()) != NULL;

}




void VCLXAccessibleComboBox::ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent)
{
    VCLXAccessibleBox::ProcessWindowEvent( rVclWindowEvent );
}




//=====  XServiceInfo  ========================================================

OUString VCLXAccessibleComboBox::getImplementationName (void)
    throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleComboBox" );
}




Sequence< OUString > VCLXAccessibleComboBox::getSupportedServiceNames (void)
    throw (RuntimeException)
{
    Sequence< OUString > aNames = VCLXAccessibleBox::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = OUString( "com.sun.star.accessibility.AccessibleComboBox" );
    return aNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
