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
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <TableWindowListBox.hxx>
#include <TableConnection.hxx>
#include <JoinController.hxx>

using namespace dbaui;
OTableWindowTitle::OTableWindowTitle(OTableWindow* pParent)
    : InterimItemWindow(pParent, "dbaccess/ui/tabletitle.ui", "TableTitle")
    , m_pTabWin( pParent )
    , m_xLabel(m_xBuilder->weld_label("label"))
    , m_xImage(m_xBuilder->weld_image("image"))
{
    m_xLabel->connect_mouse_press(LINK(this, OTableWindowTitle, MousePressHdl));
}

OTableWindowTitle::~OTableWindowTitle()
{
    disposeOnce();
}

void OTableWindowTitle::dispose()
{
    m_xImage.reset();
    m_xLabel.reset();
    m_pTabWin.clear();
    InterimItemWindow::dispose();
}

IMPL_LINK(OTableWindowTitle, MousePressHdl, const MouseEvent&, rEvt, bool)
{
    if (rEvt.IsLeft())
    {
        if( rEvt.GetClicks() == 2)
        {
            Size aSize(GetTextWidth(GetText()) + 20,
                        m_pTabWin->GetSizePixel().Height() - m_pTabWin->GetListBox()->GetSizePixel().Height());

            weld::TreeView& rTreeView = m_pTabWin->GetListBox()->get_widget();
            aSize.AdjustHeight(rTreeView.get_height_rows(rTreeView.n_children() + 2));
            if (m_pTabWin->GetSizePixel() != aSize)
            {
                m_pTabWin->SetSizePixel(aSize);

                OJoinTableView* pView = m_pTabWin->getTableView();
                OSL_ENSURE(pView,"No OJoinTableView!");
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
            OSL_ENSURE(pView,"No OJoinTableView!");
            pView->NotifyTitleClicked( static_cast<OTableWindow*>(GetParent()), aPos );
        }
    }
    else if (rEvt.IsRight())
    {
        CommandEvent aCEvt(rEvt.GetPosPixel(), CommandEventId::ContextMenu, true);
        // tdf#94709 - protect shutdown code-path.
        VclPtr<OTableWindow> xTabWin(m_pTabWin);
        xTabWin->Command(aCEvt);
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
