/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxaccessibledropdowncombobox.cxx,v $
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
#include <accessibility/standard/vclxaccessibledropdowncombobox.hxx>
#include <accessibility/standard/vclxaccessiblecombobox.hxx>
#include <accessibility/standard/vclxaccessibletextfield.hxx>
#include <accessibility/standard/vclxaccessiblelist.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <tools/debug.hxx>
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
            /*          // child count changed
            Any aOldValue, aNewValue;
            // get the listbox child
            Reference< XAccessible > xChild;
            if ( !xChild.is() )
            {
                try
                {
                    // the listbox is the second child
                    xChild = getAccessibleChild(1);
                }
                catch ( IndexOutOfBoundsException& ) {}
                catch ( RuntimeException& ) {}
            }
            if ( rVclWindowEvent.GetId() == VCLEVENT_DROPDOWN_OPEN )
                aNewValue <<= xChild;
            else
                aOldValue <<= xChild;
            NotifyAccessibleEvent(
            AccessibleEventId::CHILD, aOldValue, aNewValue
            );
            */
            break;
        }

        default:
            VCLXAccessibleBox::ProcessWindowEvent( rVclWindowEvent );
    }
}




//=====  XServiceInfo  ========================================================

::rtl::OUString VCLXAccessibleDropDownComboBox::getImplementationName()
    throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.toolkit.AccessibleDropDownComboBox");
}




Sequence< ::rtl::OUString > VCLXAccessibleDropDownComboBox::getSupportedServiceNames (void)
    throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames = VCLXAccessibleBox::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = ::rtl::OUString::createFromAscii(
        "com.sun.star.accessibility.AccessibleDropDownComboBox" );
    return aNames;
}
