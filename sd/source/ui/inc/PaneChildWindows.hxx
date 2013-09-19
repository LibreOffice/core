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

#ifndef SD_PANE_CHILD_WINDOWS_HXX
#define SD_PANE_CHILD_WINDOWS_HXX

#include <sfx2/childwin.hxx>
#include <sfx2/taskpane.hxx>

namespace sd {

class PaneChildWindow
    : public SfxChildWindow
{
public:
    PaneChildWindow (
        ::Window* pParentWindow,
        sal_uInt16 nId,
        SfxBindings* pBindings,
        SfxChildWinInfo* pInfo,
        const sal_uInt16 nDockWinTitleResId,
        const sal_uInt16 nTitleBarResId,
        SfxChildAlignment eAlignment);
    virtual ~PaneChildWindow (void);
};




class LeftPaneImpressChildWindow
    : public PaneChildWindow
{
public:
    LeftPaneImpressChildWindow (::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo*);

    SFX_DECL_CHILDWINDOW_WITHID(LeftPaneImpressChildWindow);
};




class LeftPaneDrawChildWindow
    : public PaneChildWindow
{
public:
    LeftPaneDrawChildWindow (::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo*);

    SFX_DECL_CHILDWINDOW_WITHID(LeftPaneDrawChildWindow);
};


} // end of namespace ::sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
