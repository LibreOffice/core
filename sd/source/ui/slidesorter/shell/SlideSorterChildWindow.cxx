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

#include "SlideSorterChildWindow.hxx"

#include "app.hrc"
#include "sfx2/app.hxx"
#include <sfx2/dockwin.hxx>

#include "SlideSorter.hxx"

namespace sd { namespace slidesorter {

SlideSorterChildWindow::SlideSorterChildWindow (
    ::Window* pParentWindow,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : SfxChildWindow (pParentWindow, nId)
{
    pWindow = new SlideSorter (
        pBindings,
        this,
        pParentWindow);
    eChildAlignment = SFX_ALIGN_LEFT;
    static_cast<SfxDockingWindow*>(pWindow)->Initialize (pInfo);
    //  SetHideNotDelete (sal_True);
}

SFX_IMPL_DOCKINGWINDOW(SlideSorterChildWindow, SID_SLIDE_BROWSER)

} } // end of namespace ::sd::slidesorter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
