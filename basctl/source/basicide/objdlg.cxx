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

#include "objdlg.hrc"

#include "objdlg.hxx"
#include "helpid.hrc"

#include <svl/itemset.hxx>
#include <vcl/taskpanelist.hxx>

namespace basctl
{

ObjectCatalog::ObjectCatalog (vcl::Window* pParent) :
    DockingWindow(pParent),
    aTitle(VclPtr<FixedText>::Create(this)),
    aTree(VclPtr<TreeListBox>::Create(this, IDEResId(RID_TLB_MACROS)))
{
    SetHelpId("basctl:FloatingWindow:RID_BASICIDE_OBJCAT");
    SetText(IDEResId(RID_BASICIDE_OBJCAT).toString());

    // title
    aTitle->SetText(IDEResId(RID_BASICIDE_OBJCAT).toString());
    aTitle->SetStyle(WB_CENTER);

    // tree list
    aTree->Hide();
    aTree->SetStyle(
        WB_BORDER | WB_TABSTOP | WB_HSCROLL |
        WB_HASLINES | WB_HASLINESATROOT |
        WB_HASBUTTONS | WB_HASBUTTONSATROOT
    );
    aTree->SetAccessibleName(IDEResId(RID_STR_TLB_MACROS).toString());
    aTree->SetHelpId(HID_BASICIDE_OBJECTCAT);
    aTree->ScanAllEntries();
    aTree->GrabFocus();

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

ObjectCatalog::~ObjectCatalog()
{
    disposeOnce();
}

void ObjectCatalog::dispose()
{
    if (!IsDisposed())
        GetParent()->GetSystemWindow()->GetTaskPaneList()->RemoveWindow(this);
    aTitle.disposeAndClear();
    aTree.disposeAndClear();
    DockingWindow::dispose();
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
    DockingWindow::ToggleFloatingMode();
    // rearranging the controls (title)
    ArrangeWindows();
}

// ArrangeWindows() -- arranges the controls to the size of the ObjectCatalog
void ObjectCatalog::ArrangeWindows()
{
    if (!aTitle || !aTree)
        return;

    Size const aSize = GetOutputSizePixel();
    bool const bFloating = IsFloatingMode();

    // title
    // (showing only if no title bar)
    if (bFloating)
        aTitle->Hide();
    else
    {
        Size aTitleSize = LogicToPixel(Size(3, 10), MAP_APPFONT);
        aTitleSize.Width() = aSize.Width() - 2*aTitleSize.Width();
        aTitle->SetPosPixel(LogicToPixel(Point(3, 3), MAP_APPFONT));
        aTitle->SetSizePixel(aTitleSize);
        aTitle->Show();
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
        aTree->SetPosSizePixel(aTreePos, aTreeSize);
        aTree->Show();
    }
    else
        aTree->Hide();
}

void ObjectCatalog::SetCurrentEntry (BaseWindow* pCurWin)
{
    EntryDescriptor aDescriptor;
    if (pCurWin)
        aDescriptor = pCurWin->CreateEntryDescriptor();
    aTree->SetCurrentEntry(aDescriptor);
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
