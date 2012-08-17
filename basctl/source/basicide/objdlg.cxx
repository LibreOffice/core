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

#include "basidesh.hrc"
#include "objdlg.hrc"

#include "basidesh.hxx"
#include "iderdll.hxx"
#include "iderdll2.hxx"
#include "objdlg.hxx"
#include "helpid.hrc"

#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/itemset.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/taskpanelist.hxx>

namespace basctl
{

ObjectCatalog::ObjectCatalog (Window* pParent) :
    BasicDockingWindow(pParent),
    aTitle(this),
    aTree(this, IDEResId(RID_TLB_MACROS))
{
    SetHelpId("basctl:FloatingWindow:RID_BASICIDE_OBJCAT");
    SetText(String(IDEResId(RID_BASICIDE_OBJCAT)));

    // title
    aTitle.SetText(String(IDEResId(RID_BASICIDE_OBJCAT)));
    aTitle.SetStyle(WB_CENTER);

    // tree list
    aTree.Hide();
    aTree.SetStyle(
        WB_BORDER | WB_TABSTOP | WB_HSCROLL |
        WB_HASLINES | WB_HASLINESATROOT |
        WB_HASBUTTONS | WB_HASBUTTONSATROOT
    );
    aTree.SetAccessibleName(String(IDEResId(RID_STR_TLB_MACROS)));
    aTree.SetHelpId(HID_BASICIDE_OBJECTCAT);
    aTree.ScanAllEntries();
    aTree.GrabFocus();

    {
        // centered after AppWin:
        Window const& rParent = *GetParent();
        Point aPos = rParent.OutputToScreenPixel(Point(0, 0));
        Size const aParentSize = rParent.GetSizePixel();
        Size const aSize = GetSizePixel();
        aPos.X() += (aParentSize.Width() - aSize.Width()) / 2;
        aPos.Y() += (aParentSize.Height() - aSize.Height()) / 2;
        SetPosPixel(aPos);
    }

    // make object catalog keyboard accessible
    GetParent()->GetSystemWindow()->GetTaskPaneList()->AddWindow(this);
}

ObjectCatalog::~ObjectCatalog ()
{
    GetParent()->GetSystemWindow()->GetTaskPaneList()->RemoveWindow(this);
}

// Resize() -- called by Window
void ObjectCatalog::Resize ()
{
    // arranging the controls
    ArrangeWindows();
}

// ToggleFloatingMode() -- called by DockingWindow when IsFloatingMode() changes
void ObjectCatalog::ToggleFloatingMode ()
{
    // base class version
    BasicDockingWindow::ToggleFloatingMode();
    // rearranging the controls (title)
    ArrangeWindows();
}

// ArrangeWindows() -- arranges the controls to the size of the ObjectCatalog
void ObjectCatalog::ArrangeWindows ()
{
    Size const aSize = GetOutputSizePixel();
    bool const bFloating = IsFloatingMode();

    // title
    // (showing only if no title bar)
    if (bFloating)
        aTitle.Hide();
    else
    {
        Size aTitleSize = LogicToPixel(Size(3, 10), MAP_APPFONT);
        aTitleSize.Width() = aSize.Width() - 2*aTitleSize.Width();
        aTitle.SetPosPixel(LogicToPixel(Point(3, 3), MAP_APPFONT));
        aTitle.SetSizePixel(aTitleSize);
        aTitle.Show();
    }

    // tree
    Point const aTreePos = LogicToPixel(Point(3, bFloating ? 3 : 16), MAP_APPFONT);
    long const nMargin = aTreePos.X();
    Size const aTreeSize(
        aSize.Width() - 2*nMargin,
        aSize.Height() - aTreePos.Y() - nMargin
    );
    if (aTreeSize.Height() > 0)
    {
        aTree.SetPosSizePixel(aTreePos, aTreeSize);
        aTree.Show();
    }
    else
        aTree.Hide();
}

void ObjectCatalog::SetCurrentEntry (IDEBaseWindow* pCurWin)
{
    BasicEntryDescriptor aDescriptor;
    if (pCurWin)
        aDescriptor = pCurWin->CreateEntryDescriptor();
    aTree.SetCurrentEntry(aDescriptor);
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
