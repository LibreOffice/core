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

#include <accessibility/standard/vclxaccessibletabpagewindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;



//  class VCLXAccessibleTabPageWindow


VCLXAccessibleTabPageWindow::VCLXAccessibleTabPageWindow( VCLXWindow* pVCLXWindow )
    :VCLXAccessibleComponent( pVCLXWindow )
{
    m_pTabPage = static_cast< TabPage* >( GetWindow().get() );
    m_pTabControl = nullptr;
    m_nPageId = 0;
    if ( m_pTabPage )
    {
        vcl::Window* pParent = m_pTabPage->GetAccessibleParentWindow();
        if ( pParent && pParent->GetType() == WINDOW_TABCONTROL )
        {
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
    }
}



VCLXAccessibleTabPageWindow::~VCLXAccessibleTabPageWindow()
{
}


// OCommonAccessibleComponent


awt::Rectangle VCLXAccessibleTabPageWindow::implGetBounds() throw (RuntimeException)
{
    awt::Rectangle aBounds( 0, 0, 0, 0 );

    if ( m_pTabControl )
    {
        Rectangle aPageRect = m_pTabControl->GetTabBounds( m_nPageId );
        if ( m_pTabPage )
        {
            Rectangle aRect = Rectangle( m_pTabPage->GetPosPixel(), m_pTabPage->GetSizePixel() );
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


Reference< XAccessible > VCLXAccessibleTabPageWindow::getAccessibleParent(  ) throw (RuntimeException, std::exception)
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
                SAL_WARN_IF(TAB_PAGE_NOTFOUND == nPagePos, "accessibility",
                        "getAccessibleParent(): no tab page");
                if (TAB_PAGE_NOTFOUND != nPagePos)
                {
                    xParent = xCont->getAccessibleChild(nPagePos);
                }
            }
        }
    }

    return xParent;
}



sal_Int32 VCLXAccessibleTabPageWindow::getAccessibleIndexInParent(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
