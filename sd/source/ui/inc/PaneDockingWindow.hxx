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

#ifndef INCLUDED_SD_SOURCE_UI_INC_PANEDOCKINGWINDOW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_PANEDOCKINGWINDOW_HXX

#include <sfx2/titledockwin.hxx>
#include <sfx2/viewfrm.hxx>

#include <memory>

class SplitWindow;

namespace sd {

    class PaneDockingWindow : public ::sfx2::TitledDockingWindow
{
public:
    /** Create a new docking window.
        @param pBindings
            Used, among others, to determine the ViewShellBase and
            PaneManager that manage the new docking window.
        @param pChildWindow
            This child window is the logical container for the new docking
            window.
        @param pParent
            The parent window of the new docking window.
        @param rsTitle
            the initial title
    */
    PaneDockingWindow (
        SfxBindings *pBindings,
        SfxChildWindow *pChildWindow,
        vcl::Window* pParent,
        const OUString& rsTitle);

    virtual ~PaneDockingWindow();
    virtual void StateChanged( StateChangedType nType ) override;
    virtual void MouseButtonDown (const MouseEvent& rEvent) override;
    /** When docked the given range is passed to the parent SplitWindow.
    */
    void SetValidSizeRange (const Range& rValidSizeRange);

    enum Orientation { HorizontalOrientation, VerticalOrientation, UnknownOrientation };
    /** When the PaneDockingWindow is docked and managed by a split window
        it can derive its orientation from the orientation of the split
        window and return either HorizontalOrientation or
        VerticalOrientation.
        Otherwise UnknownOrientation is returned.
    */
    Orientation GetOrientation() const;
};

} // end of namespace ::sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
