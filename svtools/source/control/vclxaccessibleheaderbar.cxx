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
#include <svtools/vclxaccessibleheaderbar.hxx>
#include <svtools/vclxaccessibleheaderbaritem.hxx>

#include <toolkit/awt/vclxwindows.hxx>
#include <svtools/headbar.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <vcl/svapp.hxx>

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


// XServiceInfo


::rtl::OUString VCLXAccessibleHeaderBar::getImplementationName() throw (RuntimeException, std::exception)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleHeaderBar" );
}



Sequence< ::rtl::OUString > VCLXAccessibleHeaderBar::getSupportedServiceNames() throw (RuntimeException, std::exception)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleHeaderBar" );
    return aNames;
}

// =======XAccessibleContext=======

sal_Int32 SAL_CALL VCLXAccessibleHeaderBar::getAccessibleChildCount(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    sal_Int32 nCount = 0;
    if ( m_pHeadBar )
        nCount = m_pHeadBar->GetItemCount();

    return nCount;
}
::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        VCLXAccessibleHeaderBar::getAccessibleChild( sal_Int32 i )  throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

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

sal_Int16 SAL_CALL VCLXAccessibleHeaderBar::getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return com::sun::star::accessibility::AccessibleRole::LIST;
}

void SAL_CALL VCLXAccessibleHeaderBar::disposing()
{
    SolarMutexGuard g;

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
