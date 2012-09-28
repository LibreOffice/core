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

#ifndef BASCTL_OBJDLG_HXX
#define BASCTL_OBJDLG_HXX

#include "bastype2.hxx"
#include "bastypes.hxx"

#include <svheader.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/image.hxx>

namespace basctl
{

//
// ObjectCatalog -- a docking window that contains the currently loaded macros
// in a tree structure.
//
class ObjectCatalog : public DockingWindow
{
public:
    ObjectCatalog (Window* pParent);
    virtual ~ObjectCatalog ();
public:
    void UpdateEntries () { aTree.UpdateEntries(); }
    void SetCurrentEntry (BaseWindow* pCurWin);

private:
    // title: "Object Catalog"
    FixedText aTitle;
    // the tree-list of the objects
    TreeListBox aTree;

private:
    virtual void Resize (); // Window
    virtual void ToggleFloatingMode (); // DockingWindow
    void ArrangeWindows ();
};

} // namespace basctl

#endif // BASCTL_OBJDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
