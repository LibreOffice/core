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

#include "browserpage.hxx"

namespace pcr
{
    #define LAYOUT_BORDER_LEFT      3
    #define LAYOUT_BORDER_TOP       3
    #define LAYOUT_BORDER_RIGHT     3
    #define LAYOUT_BORDER_BOTTOM    3

    OBrowserPage::OBrowserPage(vcl::Window* pParent,WinBits nWinStyle)
            :TabPage(pParent,nWinStyle)
            ,m_aListBox(VclPtr<OBrowserListBox>::Create(this))
    {
        m_aListBox->SetBackground(GetBackground());
        m_aListBox->SetPaintTransparent( true );
        m_aListBox->Show();
    }

    OBrowserPage::~OBrowserPage()
    {
        disposeOnce();
    }

    void OBrowserPage::dispose()
    {
        m_aListBox.disposeAndClear();
        TabPage::dispose();
    }

    void OBrowserPage::Resize()
    {
        Size aSize( GetOutputSizePixel() );
        aSize.Width() -= LAYOUT_BORDER_LEFT + LAYOUT_BORDER_RIGHT;
        aSize.Height() -= LAYOUT_BORDER_TOP + LAYOUT_BORDER_BOTTOM;
        m_aListBox->SetPosSizePixel( Point( LAYOUT_BORDER_LEFT, LAYOUT_BORDER_TOP ), aSize );
    }

    void OBrowserPage::StateChanged(StateChangedType nType)
    {
        Window::StateChanged( nType);
        if (StateChangedType::Visible == nType && m_aListBox)
            m_aListBox->ActivateListBox(IsVisible());
    }

    sal_Int32 OBrowserPage::getMinimumWidth()
    {
        return m_aListBox->GetMinimumWidth() + LAYOUT_BORDER_LEFT + LAYOUT_BORDER_RIGHT;
    }

    sal_Int32 OBrowserPage::getMinimumHeight()
    {
        return m_aListBox->GetMinimumHeight() + LAYOUT_BORDER_TOP + LAYOUT_BORDER_BOTTOM;
    }

} // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
