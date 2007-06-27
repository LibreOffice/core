/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessibletabpagewindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:41:25 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETABPAGEWINDOW_HXX
#include <accessibility/standard/vclxaccessibletabpagewindow.hxx>
#endif

#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif

#ifndef _SV_TABCTRL_HXX
#include <vcl/tabctrl.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif


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
