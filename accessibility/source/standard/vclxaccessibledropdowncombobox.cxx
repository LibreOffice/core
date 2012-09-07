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

#include <accessibility/standard/vclxaccessibledropdowncombobox.hxx>
#include <accessibility/standard/vclxaccessiblecombobox.hxx>
#include <accessibility/standard/vclxaccessibletextfield.hxx>
#include <accessibility/standard/vclxaccessiblelist.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <vcl/svapp.hxx>
#include <vcl/combobox.hxx>
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


VCLXAccessibleDropDownComboBox::VCLXAccessibleDropDownComboBox (VCLXWindow* pVCLWindow)
    : VCLXAccessibleBox (pVCLWindow, VCLXAccessibleBox::COMBOBOX, true)
{
}




VCLXAccessibleDropDownComboBox::~VCLXAccessibleDropDownComboBox (void)
{
}



bool VCLXAccessibleDropDownComboBox::IsValid (void) const
{
    return static_cast<ComboBox*>(GetWindow()) != NULL;

}




void VCLXAccessibleDropDownComboBox::ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent)
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_DROPDOWN_OPEN:
        case VCLEVENT_DROPDOWN_CLOSE:
        {
            break;
        }

        default:
            VCLXAccessibleBox::ProcessWindowEvent( rVclWindowEvent );
    }
}




//=====  XServiceInfo  ========================================================

OUString VCLXAccessibleDropDownComboBox::getImplementationName()
    throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleDropDownComboBox" );
}




Sequence< OUString > VCLXAccessibleDropDownComboBox::getSupportedServiceNames (void)
    throw (RuntimeException)
{
    Sequence< OUString > aNames = VCLXAccessibleBox::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = "com.sun.star.accessibility.AccessibleDropDownComboBox";
    return aNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
