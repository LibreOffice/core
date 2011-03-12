/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_PANE_DOCKING_WINDOW_HXX
#define SD_PANE_DOCKING_WINDOW_HXX

#include <sfx2/titledockwin.hxx>
#include <sfx2/viewfrm.hxx>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class ToolBox;
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
        @param rResId
            The resource is used to determine initial size and attributes.
        @param rsTitle
            the initial title
    */
    PaneDockingWindow (
        SfxBindings *pBindings,
        SfxChildWindow *pChildWindow,
        ::Window* pParent,
        const ResId& rResId,
        const ::rtl::OUString& rsTitle);

    virtual ~PaneDockingWindow (void);
    virtual void StateChanged( StateChangedType nType );
    virtual void MouseButtonDown (const MouseEvent& rEvent);
    /** When docked the given range is passed to the parent SplitWindow.
    */
    void SetValidSizeRange (const Range aValidSizeRange);

    enum Orientation { HorizontalOrientation, VerticalOrientation, UnknownOrientation };
    /** When the PaneDockingWindow is docked and managed by a split window
        it can derive its orientation from the orientation of the split
        window and return either HorizontalOrientation or
        VerticalOrientation.
        Otherwise UnknownOrientation is returned.
    */
    Orientation GetOrientation (void) const;

    /** The current height of the title bar.
    */
    sal_Int32 mnTitleBarHeight;

};

} // end of namespace ::sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
