/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxaccessibletabpagewindow.cxx,v $
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
#include <accessibility/standard/vclxaccessibletabpagewindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


//  ----------------------------------------------------
//  class VCLXAccessibleTabPageWindow
//  ----------------------------------------------------

VCLXAccessibleTabPageWindow::VCLXAccessibleTabPageWindow( VCLXWindow* pVCLXWindow )
    :VCLXAccessibleComponent( pVCLXWindow )
{
    m_pTabPage = static_cast< TabPage* >( GetWindow() );
    if ( m_pTabPage )
    {
        Window* pParent = m_pTabPage->GetAccessibleParentWindow();
        if ( pParent && pParent->GetType() == WINDOW_TABCONTROL )
        {
            m_pTabControl = static_cast< TabControl* >( pParent );
            if ( m_pTabControl )
            {
                for ( sal_uInt16 i = 0, nCount = m_pTabControl->GetPageCount(); i < nCount; ++i )
                {
                    sal_uInt16 nPageId = m_pTabControl->GetPageId( i );
                    if ( m_pTabControl->GetTabPage( nPageId ) == m_pTabPage )
                        m_nPageId = nPageId;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

VCLXAccessibleTabPageWindow::~VCLXAccessibleTabPageWindow()
{
}

// -----------------------------------------------------------------------------
// OCommonAccessibleComponent
// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void VCLXAccessibleTabPageWindow::disposing()
{
    VCLXAccessibleComponent::disposing();

    m_pTabControl = NULL;
    m_pTabPage = NULL;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleTabPageWindow::getAccessibleParent(  ) throw (RuntimeException)
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
                xParent = xCont->getAccessibleChild( m_pTabControl->GetPagePos( m_nPageId ) );
        }
    }

    return xParent;
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTabPageWindow::getAccessibleIndexInParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 0;
}

// -----------------------------------------------------------------------------
