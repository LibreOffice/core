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


#include <accessibility/standard/vclxaccessibledropdownlistbox.hxx>
#include <accessibility/standard/vclxaccessiblelistbox.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <vcl/svapp.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/unohelp.hxx>

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>

#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <unotools/accessiblestatesethelper.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;


VCLXAccessibleDropDownListBox::VCLXAccessibleDropDownListBox (VCLXWindow* pVCLWindow)
    : VCLXAccessibleBox (pVCLWindow,  VCLXAccessibleBox::LISTBOX, true)
{
}


VCLXAccessibleDropDownListBox::~VCLXAccessibleDropDownListBox()
{
}

bool VCLXAccessibleDropDownListBox::IsValid() const
{
    return GetWindow().get() != nullptr;
}

void VCLXAccessibleDropDownListBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    VCLXAccessibleBox::ProcessWindowEvent (rVclWindowEvent);
}


// XServiceInfo

OUString VCLXAccessibleDropDownListBox::getImplementationName()
    throw (RuntimeException, std::exception)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleDropDownListBox" );
}


Sequence< OUString > VCLXAccessibleDropDownListBox::getSupportedServiceNames()
    throw (RuntimeException, std::exception)
{
    Sequence< OUString > aNames = VCLXAccessibleBox::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = "com.sun.star.accessibility.AccessibleDropDownListBox";
    return aNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
