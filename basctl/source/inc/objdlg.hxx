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

#ifndef INCLUDED_BASCTL_SOURCE_INC_OBJDLG_HXX
#define INCLUDED_BASCTL_SOURCE_INC_OBJDLG_HXX

#include "bastype2.hxx"
#include "bastypes.hxx"

#include <vcl/fixed.hxx>

namespace basctl
{


// ObjectCatalog -- a docking window that contains the currently loaded macros
// in a tree structure.

class ObjectCatalog : public DockingWindow
{
public:
    explicit ObjectCatalog(vcl::Window* pParent);
    virtual ~ObjectCatalog();
    virtual void dispose() override;
public:
    void UpdateEntries () { aTree->UpdateEntries(); }
    void SetCurrentEntry (BaseWindow* pCurWin);

private:
    // title: "Object Catalog"
    VclPtr<FixedText> aTitle;
    // the tree-list of the objects
    VclPtr<TreeListBox> aTree;

private:
    virtual void Resize () override; // Window
    virtual void ToggleFloatingMode () override; // DockingWindow
    void ArrangeWindows ();
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_OBJDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
