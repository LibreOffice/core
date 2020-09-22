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

#include <TableWindowTitle.hxx>
#include <TableWindow.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <TableWindowListBox.hxx>
#include <TableConnection.hxx>
#include <JoinController.hxx>

using namespace dbaui;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

OTableWindowTitle::OTableWindowTitle(OTableWindow* pParent)
    : InterimItemWindow(pParent, "dbaccess/ui/windowtitle.ui", "WindowTitle")
    , m_pTabWin(pParent)
    , m_xLabel(m_xBuilder->weld_label("label"))
{
    m_xLabel->connect_mouse_press(LINK(this, OTableWindowTitle, MouseButtonDownHdl));
#if 0
    // set background- and text colour
    StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
    SetBackground(Wallpaper(aSystemStyle.GetFaceColor()));
    SetTextColor(aSystemStyle.GetButtonTextColor());

    vcl::Font aFont( GetFont() );
    aFont.SetTransparent( true );
    SetFont( aFont );
#endif
}

OTableWindowTitle::~OTableWindowTitle()
{
    disposeOnce();
}

void OTableWindowTitle::dispose()
{
    m_xLabel.reset();
    m_pTabWin.clear();
    InterimItemWindow::dispose();
}

#if 0
void OTableWindowTitle::GetFocus()
{
    if(m_pTabWin)
        m_pTabWin->GetFocus();
    else
        FixedText::GetFocus();
}

void OTableWindowTitle::LoseFocus()
{
    if (m_pTabWin)
        m_pTabWin->LoseFocus();
    else
        FixedText::LoseFocus();
}

void OTableWindowTitle::Command( const CommandEvent& rEvt )
{
    if ( rEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        GrabFocus();
        if ( m_pTabWin )
        {
            // tdf#94709 - protect shutdown code-path.
            VclPtr<OTableWindow> xTabWin(m_pTabWin);
            xTabWin->Command( rEvt );
        }
        else
            Control::Command(rEvt);
    }
}

void OTableWindowTitle::KeyInput( const KeyEvent& rEvt )
{
    if ( m_pTabWin )
        m_pTabWin->KeyInput( rEvt );
}

#endif

IMPL_LINK(OTableWindowTitle, MouseButtonDownHdl, const MouseEvent&, rEvt, bool)
{
    if (rEvt.IsLeft())
    {
        if( rEvt.GetClicks() == 2)
        {
            Size aSize(m_xLabel->get_preferred_size().Width() + 20,
                        m_pTabWin->GetSizePixel().Height() - m_pTabWin->GetListBox()->GetSizePixel().Height());

            weld::TreeView& rTreeView = m_pTabWin->GetListBox()->get_widget();
            aSize.AdjustHeight(rTreeView.get_height_rows(rTreeView.n_children() + 2));
            if (m_pTabWin->GetSizePixel() != aSize)
            {
                m_pTabWin->SetSizePixel(aSize);

                OJoinTableView* pView = m_pTabWin->getTableView();
                assert(pView && "No OJoinTableView!");
                for (auto& conn : pView->getTableConnections())
                    conn->RecalcLines();

                pView->InvalidateConnections();
                pView->getDesignView()->getController().setModified(true);
                pView->Invalidate(InvalidateFlags::NoChildren);
            }
        }
        else
        {
            Point aPos = rEvt.GetPosPixel();
            aPos = OutputToScreenPixel( aPos );
            OJoinTableView* pView = m_pTabWin->getTableView();
            assert(pView && "No OJoinTableView!");
            pView->NotifyTitleClicked(m_pTabWin, aPos);
        }
        m_pTabWin->GrabFocus();
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
