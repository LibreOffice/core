/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
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
#include "precompiled_extensions.hxx"
#include "browserpage.hxx"

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
        m_aListBox.SetPaintTransparent( sal_True );
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
