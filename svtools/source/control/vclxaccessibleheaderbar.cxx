/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2010.
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

//IAccessibility2 Implementation 2009-----

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#ifndef _SVTOOLS_VCLXACCESSIBLEHEADERBAR_HXX_
#include <svtools/vclxaccessibleheaderbar.hxx>
#endif
#ifndef _SVTOOLS_VCLACCESSIBLEHEADBARITEM_HXX_
#include <svtools/vclxaccessibleheaderbaritem.hxx>
#endif

#ifndef _TOOLKIT_AWT_VCLXWINDOWS_HXX_
#include <toolkit/awt/vclxwindows.hxx>
#endif
#ifndef _HEADBAR_HXX
#include <headbar.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;

VCLXHeaderBar::VCLXHeaderBar(Window* pHeaderBar)
{
    SetWindow(pHeaderBar);
}

VCLXHeaderBar::~VCLXHeaderBar()
{
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXHeaderBar::CreateAccessibleContext()
{
    return new VCLXAccessibleHeaderBar(this);
}


VCLXAccessibleHeaderBar::VCLXAccessibleHeaderBar( VCLXWindow* pVCLWindow )
    :VCLXAccessibleComponent( pVCLWindow )
    ,m_pHeadBar(NULL)
{
    m_pHeadBar = static_cast< HeaderBar* >( GetWindow() );
}

// -----------------------------------------------------------------------------

VCLXAccessibleHeaderBar::~VCLXAccessibleHeaderBar()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleHeaderBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
}

// -----------------------------------------------------------------------------

void VCLXAccessibleHeaderBar::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleHeaderBar::getImplementationName() throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleHeaderBar" );
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > VCLXAccessibleHeaderBar::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleHeaderBar" );
    return aNames;
}

// =======XAccessibleContext=======

sal_Int32 SAL_CALL VCLXAccessibleHeaderBar::getAccessibleChildCount(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nCount = 0;
    if ( m_pHeadBar )
        nCount = m_pHeadBar->GetItemCount();

    return nCount;
}
::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        VCLXAccessibleHeaderBar::getAccessibleChild( sal_Int32 i )  throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;
    // search for the child
    if ( static_cast<sal_uInt16>(i) >= m_aAccessibleChildren.size() )
        xChild = CreateChild (i);
    else
    {
        xChild = m_aAccessibleChildren[i];
        if ( !xChild.is() )
            xChild = CreateChild (i);
    }
    return xChild;
}

sal_Int16 SAL_CALL VCLXAccessibleHeaderBar::getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return com::sun::star::accessibility::AccessibleRole::LIST;
}

void SAL_CALL VCLXAccessibleHeaderBar::disposing (void)
{
    ListItems().swap(m_aAccessibleChildren);
    VCLXAccessibleComponent::disposing();
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > VCLXAccessibleHeaderBar::CreateChild (sal_Int32 i)
{
    Reference<XAccessible> xChild;

    sal_uInt16 nPos = static_cast<sal_uInt16>(i);
    if ( nPos >= m_aAccessibleChildren.size() )
    {
        m_aAccessibleChildren.resize(nPos + 1);

        // insert into the container
        xChild = new VCLXAccessibleHeaderBarItem(m_pHeadBar, i);
        m_aAccessibleChildren[nPos] = xChild;
    }
    else
    {
        xChild = m_aAccessibleChildren[nPos];
        // check if position is empty and can be used else we have to adjust all entries behind this
        if ( !xChild.is() )
        {
            xChild = new VCLXAccessibleHeaderBarItem(m_pHeadBar, i);
            m_aAccessibleChildren[nPos] = xChild;
        }
    }
    return xChild;
}
//-----IAccessibility2 Implementation 2009
