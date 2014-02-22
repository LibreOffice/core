/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    DockingWindow(pParent),
    aTitle(this),
    aTree(this, IDEResId(RID_TLB_MACROS))
{
    SetHelpId("basctl:FloatingWindow:RID_BASICIDE_OBJCAT");
    SetText(IDEResId(RID_BASICIDE_OBJCAT).toString());

    
    aTitle.SetText(IDEResId(RID_BASICIDE_OBJCAT).toString());
    aTitle.SetStyle(WB_CENTER);

    
    aTree.Hide();
    aTree.SetStyle(
        WB_BORDER | WB_TABSTOP | WB_HSCROLL |
        WB_HASLINES | WB_HASLINESATROOT |
        WB_HASBUTTONS | WB_HASBUTTONSATROOT
    );
    aTree.SetAccessibleName(IDEResId(RID_STR_TLB_MACROS).toString());
    aTree.SetHelpId(HID_BASICIDE_OBJECTCAT);
    aTree.ScanAllEntries();
    aTree.GrabFocus();

    {
        
        Window const& rParent = *GetParent();
        Point aPos = rParent.OutputToScreenPixel(Point(0, 0));
        Size const aParentSize = rParent.GetSizePixel();
        Size const aSize = GetSizePixel();
        aPos.X() += (aParentSize.Width() - aSize.Width()) / 2;
        aPos.Y() += (aParentSize.Height() - aSize.Height()) / 2;
        SetPosPixel(aPos);
    }

    
    GetParent()->GetSystemWindow()->GetTaskPaneList()->AddWindow(this);
}

ObjectCatalog::~ObjectCatalog ()
{
    GetParent()->GetSystemWindow()->GetTaskPaneList()->RemoveWindow(this);
}


void ObjectCatalog::Resize ()
{
    
    ArrangeWindows();
}


void ObjectCatalog::ToggleFloatingMode ()
{
    
    DockingWindow::ToggleFloatingMode();
    
    ArrangeWindows();
}


void ObjectCatalog::ArrangeWindows ()
{
    Size const aSize = GetOutputSizePixel();
    bool const bFloating = IsFloatingMode();

    
    
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

void ObjectCatalog::SetCurrentEntry (BaseWindow* pCurWin)
{
    EntryDescriptor aDescriptor;
    if (pCurWin)
        aDescriptor = pCurWin->CreateEntryDescriptor();
    aTree.SetCurrentEntry(aDescriptor);
}


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
