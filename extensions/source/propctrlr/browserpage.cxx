/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


