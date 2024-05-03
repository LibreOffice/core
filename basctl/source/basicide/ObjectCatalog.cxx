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

#include <strings.hrc>
#include <iderid.hxx>

#include <ObjectCatalog.hxx>
#include <helpids.h>

#include <vcl/taskpanelist.hxx>

namespace basctl
{
ObjectCatalog::ObjectCatalog(vcl::Window* pParent)
    : DockingWindow(pParent, u"modules/BasicIDE/ui/dockingorganizer.ui"_ustr,
                    u"DockingOrganizer"_ustr)
{
    m_xTitle = m_xBuilder->weld_label(u"title"_ustr);
    m_xTree.reset(new SbTreeListBox(m_xBuilder->weld_tree_view(u"libraries"_ustr), GetFrameWeld()));

    SetHelpId(u"basctl:FloatingWindow:RID_BASICIDE_OBJCAT"_ustr);
    SetText(IDEResId(RID_BASICIDE_OBJCAT));

    // title
    m_xTitle->set_label(IDEResId(RID_BASICIDE_OBJCAT));

    // tree list
    weld::TreeView& rWidget = m_xTree->get_widget();

    rWidget.set_help_id(HID_BASICIDE_OBJECTCAT);
    m_xTree->ScanAllEntries();
    rWidget.grab_focus();

    // make object catalog keyboard accessible
    GetParent()->GetSystemWindow()->GetTaskPaneList()->AddWindow(this);
}

ObjectCatalog::~ObjectCatalog() { disposeOnce(); }

void ObjectCatalog::dispose()
{
    GetParent()->GetSystemWindow()->GetTaskPaneList()->RemoveWindow(this);
    m_xTitle.reset();
    m_xTree.reset();
    DockingWindow::dispose();
}

// ToggleFloatingMode() -- called by DockingWindow when IsFloatingMode() changes
void ObjectCatalog::ToggleFloatingMode()
{
    // base class version
    DockingWindow::ToggleFloatingMode();

    bool const bFloating = IsFloatingMode();
    // tdf#152154: m_xTitle will be null during disposing
    if (m_xTitle)
        m_xTitle->set_visible(!bFloating);
}

void ObjectCatalog::SetCurrentEntry(BaseWindow* pCurWin)
{
    EntryDescriptor aDescriptor;
    if (pCurWin)
        aDescriptor = pCurWin->CreateEntryDescriptor();
    m_xTree->SetCurrentEntry(aDescriptor);
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
