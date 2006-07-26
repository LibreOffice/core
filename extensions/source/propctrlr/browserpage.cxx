/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: browserpage.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:53:00 $
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

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERPAGE_HXX_
#include "browserpage.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................
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
        Point aPos(3,3);
        m_aListBox.SetPosPixel(aPos);
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
        aSize.Width() -= 6;
        aSize.Height() -= 6;
        m_aListBox.SetPosSizePixel( Point( 3, 3 ), aSize );
    }

    //------------------------------------------------------------------
    OBrowserListBox* OBrowserPage::getListBox()
    {
        return &m_aListBox;
    }

    //------------------------------------------------------------------
    const OBrowserListBox* OBrowserPage::getListBox() const
    {
        return &m_aListBox;
    }

    //------------------------------------------------------------------
    void OBrowserPage::StateChanged(StateChangedType nType)
    {
        Window::StateChanged( nType);
        if (STATE_CHANGE_VISIBLE == nType)
            m_aListBox.ActivateListBox(IsVisible());
    }

    // #95343# ---------------------------------------------------------
    sal_Int32 OBrowserPage::getMinimumWidth()
    {
        return m_aListBox.GetMinimumWidth()+6;
    }

//............................................................................
} // namespace pcr
//............................................................................


