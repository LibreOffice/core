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

#include <standard/vclxaccessibletabpagewindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;




VCLXAccessibleTabPageWindow::VCLXAccessibleTabPageWindow( VCLXWindow* pVCLXWindow )
    :VCLXAccessibleComponent( pVCLXWindow )
{
    m_pTabPage = GetAs<TabPage>();
    m_pTabControl = nullptr;
    m_nPageId = 0;
    if ( !m_pTabPage )
        return;

    vcl::Window* pParent = m_pTabPage->GetAccessibleParentWindow();
    if ( !(pParent && pParent->GetType() == WindowType::TABCONTROL) )
        return;

    m_pTabControl = static_cast< TabControl* >( pParent );
    if ( m_pTabControl )
    {
        for ( sal_uInt16 i = 0, nCount = m_pTabControl->GetPageCount(); i < nCount; ++i )
        {
            sal_uInt16 nPageId = m_pTabControl->GetPageId( i );
            if ( m_pTabControl->GetTabPage( nPageId ) == m_pTabPage.get() )
                m_nPageId = nPageId;
        }
    }
}


// OCommonAccessibleComponent


awt::Rectangle VCLXAccessibleTabPageWindow::implGetBounds()
{
    awt::Rectangle aBounds( 0, 0, 0, 0 );

    if ( m_pTabControl )
    {
        tools::Rectangle aPageRect = m_pTabControl->GetTabBounds( m_nPageId );
        if ( m_pTabPage )
        {
            tools::Rectangle aRect( m_pTabPage->GetPosPixel(), m_pTabPage->GetSizePixel() );
            aRect.Move( -aPageRect.Left(), -aPageRect.Top() );
            aBounds = AWTRectangle( aRect );
        }
    }

    return aBounds;
}


// XComponent


void VCLXAccessibleTabPageWindow::disposing()
{
    VCLXAccessibleComponent::disposing();

    m_pTabControl = nullptr;
    m_pTabPage = nullptr;
}


// XAccessibleContext


Reference< XAccessible > VCLXAccessibleTabPageWindow::getAccessibleParent(  )
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xParent;
    if ( m_pTabControl )
    {
        Reference< XAccessible > xAcc( m_pTabControl->GetAccessible() );
        if ( xAcc.is() )
        {
            Reference< XAccessibleContext > xCont( xAcc->getAccessibleContext() );
            if ( xCont.is() )
            {
                sal_uInt16 const nPagePos(m_pTabControl->GetPagePos(m_nPageId));
                SAL_WARN_IF(nPagePos == TAB_PAGE_NOTFOUND, "accessibility",
                        "getAccessibleParent(): no tab page");
                if (nPagePos != TAB_PAGE_NOTFOUND)
                {
                    xParent = xCont->getAccessibleChild(nPagePos);
                }
            }
        }
    }

    return xParent;
}


sal_Int32 VCLXAccessibleTabPageWindow::getAccessibleIndexInParent(  )
{
    OExternalLockGuard aGuard( this );

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
