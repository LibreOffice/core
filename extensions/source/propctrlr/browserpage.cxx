/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: browserpage.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 11:56:09 $
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
#include "precompiled_extensions.hxx"

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERPAGE_HXX_
#include "browserpage.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................

    #define LAYOUT_BORDER_LEFT      3
    #define LAYOUT_BORDER_TOP       3
    #define LAYOUT_BORDER_RIGHT     3
    #define LAYOUT_BORDER_BOTTOM    3

    //==================================================================
    // class OBrowserPage
    //==================================================================
    //------------------------------------------------------------------
    OBrowserPage::OBrowserPage(Window* pParent,WinBits nWinStyle)
            :TabPage(pParent,nWinStyle)
            ,m_aListBox(this)
    {
        m_aListBox.SetBackground(GetBackground());
        m_aListBox.SetPaintTransparent( TRUE );
        m_aListBox.Show();
    }

    //------------------------------------------------------------------
    OBrowserPage::~OBrowserPage()
    {
    }

    //------------------------------------------------------------------
    void OBrowserPage::Resize()
    {
        Size aSize( GetOutputSizePixel() );
        aSize.Width() -= LAYOUT_BORDER_LEFT + LAYOUT_BORDER_RIGHT;
        aSize.Height() -= LAYOUT_BORDER_TOP + LAYOUT_BORDER_BOTTOM;
        m_aListBox.SetPosSizePixel( Point( LAYOUT_BORDER_LEFT, LAYOUT_BORDER_TOP ), aSize );
    }

    //------------------------------------------------------------------
    OBrowserListBox& OBrowserPage::getListBox()
    {
        return m_aListBox;
    }

    //------------------------------------------------------------------
    const OBrowserListBox& OBrowserPage::getListBox() const
    {
        return m_aListBox;
    }

    //------------------------------------------------------------------
    void OBrowserPage::StateChanged(StateChangedType nType)
    {
        Window::StateChanged( nType);
        if (STATE_CHANGE_VISIBLE == nType)
            m_aListBox.ActivateListBox(IsVisible());
    }

    //------------------------------------------------------------------
    sal_Int32 OBrowserPage::getMinimumWidth()
    {
        return m_aListBox.GetMinimumWidth() + LAYOUT_BORDER_LEFT + LAYOUT_BORDER_RIGHT;
    }

    //------------------------------------------------------------------
    sal_Int32 OBrowserPage::getMinimumHeight()
    {
        return m_aListBox.GetMinimumHeight() + LAYOUT_BORDER_TOP + LAYOUT_BORDER_BOTTOM;
    }

//............................................................................
} // namespace pcr
//............................................................................


