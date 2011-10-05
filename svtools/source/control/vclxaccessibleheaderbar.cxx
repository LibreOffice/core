/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <headbar.hxx>
#include <svtools/vclxaccessibleheaderbar.hxx>
#include <svtools/vclxaccessibleheaderbaritem.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <unotools/accessiblestatesethelper.hxx>

namespace css = com::sun::star;
using namespace ::comphelper;

VCLXHeaderBar::VCLXHeaderBar(Window* pHeaderBar)
{
    SetWindow(pHeaderBar);
}

VCLXHeaderBar::~VCLXHeaderBar()
{
}

::css::uno::Reference< ::css::accessibility::XAccessibleContext > VCLXHeaderBar::CreateAccessibleContext()
{
    return new VCLXAccessibleHeaderBar(this);
}


VCLXAccessibleHeaderBar::VCLXAccessibleHeaderBar( VCLXWindow* pVCLWindow )
    :VCLXAccessibleComponent( pVCLWindow )
    ,m_pHeadBar(NULL)
{
    m_pHeadBar = static_cast< HeaderBar* >( GetWindow() );
}

VCLXAccessibleHeaderBar::~VCLXAccessibleHeaderBar()
{
}

void VCLXAccessibleHeaderBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
}

void VCLXAccessibleHeaderBar::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleHeaderBar::getImplementationName() throw ( ::css::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleHeaderBar" );
}

// -----------------------------------------------------------------------------

::css::uno::Sequence< ::rtl::OUString > VCLXAccessibleHeaderBar::getSupportedServiceNames() throw (::css::uno::RuntimeException)
{
    ::css::uno::Sequence< ::rtl::OUString > aNames(1);
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
::css::uno::Reference< ::css::accessibility::XAccessible > SAL_CALL VCLXAccessibleHeaderBar::getAccessibleChild( sal_Int32 i )
    throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    if ( i < 0 || i >= getAccessibleChildCount() )
    {
        throw ::css::lang::IndexOutOfBoundsException();
    }
    ::css::uno::Reference< ::css::accessibility::XAccessible > xChild;
    // search for the child
    if ( (size_t)i >= m_aAccessibleChildren.size() )
    {
        xChild = CreateChild (i);
    }
    else
    {
        xChild = m_aAccessibleChildren[i];
        if ( !xChild.is() )
        {
            xChild = CreateChild (i);
        }
    }
    return xChild;
}

sal_Int16 SAL_CALL VCLXAccessibleHeaderBar::getAccessibleRole(  ) throw (::css::uno::RuntimeException)
{
    return css::accessibility::AccessibleRole::LIST;
}

void SAL_CALL VCLXAccessibleHeaderBar::disposing (void)
{
    ListItems().swap(m_aAccessibleChildren);
    VCLXAccessibleComponent::disposing();
}

::css::uno::Reference< ::css::accessibility::XAccessible > VCLXAccessibleHeaderBar::CreateChild (sal_Int32 i)
{
    ::css::uno::Reference< ::css::accessibility::XAccessible> xChild;

if ( (size_t)i >= m_aAccessibleChildren.size() )
    {
        m_aAccessibleChildren.resize(i + 1);

        // insert into the container
        xChild = new VCLXAccessibleHeaderBarItem(m_pHeadBar, i);
        m_aAccessibleChildren[i] = xChild;
    }
    else
    {
        xChild = m_aAccessibleChildren[i];
        // check if position is empty and can be used else we have to adjust all entries behind this
        if ( !xChild.is() )
        {
            xChild = new VCLXAccessibleHeaderBarItem(m_pHeadBar, i);
            m_aAccessibleChildren[i] = xChild;
        }
    }
    return xChild;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
