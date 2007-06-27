/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessibletextfield.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:41:48 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTFIELD_HXX
#include <accessibility/standard/vclxaccessibletextfield.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef ACCESSIBILITY_HELPER_LISTBOXHELPER_HXX
#include <accessibility/helper/listboxhelper.hxx>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;




VCLXAccessibleTextField::VCLXAccessibleTextField (VCLXWindow* pVCLWindow, const Reference< XAccessible >& _xParent) :

    VCLXAccessibleTextComponent (pVCLWindow),

    m_xParent( _xParent )

{
}




VCLXAccessibleTextField::~VCLXAccessibleTextField (void)
{
}




::rtl::OUString VCLXAccessibleTextField::implGetText (void)
{
    ::rtl::OUString aText;
    ListBox* pListBox = static_cast<ListBox*>(GetWindow());
    if (pListBox!=NULL && !pListBox->IsInDropDown())
        aText = pListBox->GetSelectEntry();

    return aText;
}




IMPLEMENT_FORWARD_XINTERFACE2(VCLXAccessibleTextField, VCLXAccessibleTextComponent, VCLXAccessible_BASE)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(VCLXAccessibleTextField, VCLXAccessibleTextComponent, VCLXAccessible_BASE)


//=====  XAccessible  =========================================================

Reference<XAccessibleContext> SAL_CALL
    VCLXAccessibleTextField::getAccessibleContext (void)
    throw (RuntimeException)
{
    return this;
}


//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL VCLXAccessibleTextField::getAccessibleChildCount (void)
    throw (RuntimeException)
{
    return 0;
}




Reference<XAccessible> SAL_CALL VCLXAccessibleTextField::getAccessibleChild (sal_Int32)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    throw IndexOutOfBoundsException();
}




sal_Int16 SAL_CALL VCLXAccessibleTextField::getAccessibleRole (void)
    throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return AccessibleRole::TEXT;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleTextField::getAccessibleParent(  )
    throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return m_xParent;
}



//===== XServiceInfo ==========================================================

::rtl::OUString VCLXAccessibleTextField::getImplementationName (void)
    throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii ("com.sun.star.comp.toolkit.AccessibleTextField");
}




Sequence< ::rtl::OUString > VCLXAccessibleTextField::getSupportedServiceNames (void)
    throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames = VCLXAccessibleTextComponent::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = ::rtl::OUString::createFromAscii(
        "com.sun.star.accessibility.AccessibleTextField");
    return aNames;
}
