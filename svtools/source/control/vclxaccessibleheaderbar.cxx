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
#include <vclxaccessibleheaderbar.hxx>
#include <vclxaccessibleheaderbaritem.hxx>

#include <toolkit/awt/vclxwindows.hxx>
#include <svtools/headbar.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
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

VCLXHeaderBar::VCLXHeaderBar(vcl::Window* pHeaderBar)
{
    SetWindow(pHeaderBar);
}

VCLXHeaderBar::~VCLXHeaderBar()
{
}

css::uno::Reference< css::accessibility::XAccessibleContext > VCLXHeaderBar::CreateAccessibleContext()
{
    return new VCLXAccessibleHeaderBar(this);
}


VCLXAccessibleHeaderBar::VCLXAccessibleHeaderBar( VCLXWindow* pVCLWindow )
    :VCLXAccessibleComponent( pVCLWindow )
    ,m_pHeadBar(nullptr)
{
    m_pHeadBar = GetAs< HeaderBar >();
}


VCLXAccessibleHeaderBar::~VCLXAccessibleHeaderBar()
{
}


// XServiceInfo


::rtl::OUString VCLXAccessibleHeaderBar::getImplementationName()
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleHeaderBar" );
}


Sequence< ::rtl::OUString > VCLXAccessibleHeaderBar::getSupportedServiceNames()
{
    Sequence<OUString> aNames { "com.sun.star.awt.AccessibleHeaderBar" };
    return aNames;
}

// =======XAccessibleContext=======

sal_Int32 SAL_CALL VCLXAccessibleHeaderBar::getAccessibleChildCount(  )
{
    SolarMutexGuard g;

    sal_Int32 nCount = 0;
    if ( m_pHeadBar )
        nCount = m_pHeadBar->GetItemCount();

    return nCount;
}
css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        VCLXAccessibleHeaderBar::getAccessibleChild( sal_Int32 i )
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

sal_Int16 SAL_CALL VCLXAccessibleHeaderBar::getAccessibleRole(  )
{
    return css::accessibility::AccessibleRole::LIST;
}

void SAL_CALL VCLXAccessibleHeaderBar::disposing()
{
    SolarMutexGuard g;

    ListItems().swap(m_aAccessibleChildren);
    VCLXAccessibleComponent::disposing();
}

css::uno::Reference< css::accessibility::XAccessible > VCLXAccessibleHeaderBar::CreateChild (sal_Int32 i)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
