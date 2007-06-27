/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessibledropdowncombobox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:37:41 $
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
#include "precompiled_accessibility.hxx"

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEDROPDOWNCOMBOBOX_HXX
#include <accessibility/standard/vclxaccessibledropdowncombobox.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLECOMBOBOX_HXX
#include <accessibility/standard/vclxaccessiblecombobox.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTFIELD_HXX
#include <accessibility/standard/vclxaccessibletextfield.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLELIST_HXX
#include <accessibility/standard/vclxaccessiblelist.hxx>
#endif
#ifndef ACCESSIBILITY_HELPER_TKARESMGR_HXX
#include <accessibility/helper/accresmgr.hxx>
#endif
#ifndef ACCESSIBILITY_HELPER_ACCESSIBLESTRINGS_HRC_
#include <accessibility/helper/accessiblestrings.hrc>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _VCL_UNOHELP_HXX
#include <vcl/unohelp.hxx>
#endif

#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif

#ifndef _COMPHELPER_SEQUENCE_HXX
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif


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
