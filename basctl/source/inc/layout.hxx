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

#pragma once

#include "bastypes.hxx"
#include <vcl/split.hxx>
#include <vcl/vclptr.hxx>

#include <vector>

class DockingWindow;
class SfxRequest;
class SfxItemSet;

namespace basctl
{

class DockingWindow;
class BaseWindow;


// Layout -- the common base of ModulLayout and DialogLayout.
// Handles the splitting lines and the dockable windows.

class Layout: public vcl::Window
{
public:
    void ArrangeWindows ();

    virtual void Activating (BaseWindow&);
    virtual void Deactivating ();
    virtual void ExecuteGlobal (SfxRequest&) { }
    virtual void GetState (SfxItemSet&, unsigned nWhich) = 0;
    virtual void UpdateDebug (bool bBasicStopped ) = 0;

    virtual ~Layout() override;
    virtual void dispose() override;

protected:
    explicit Layout(vcl::Window* pParent);

    void AddToLeft   (DockingWindow* pWin, Size const& rSize) { aLeftSide.Add(pWin, rSize); }
    void AddToBottom (DockingWindow* pWin, Size const& rSize) { aBottomSide.Add(pWin, rSize); }
    void Remove (DockingWindow*);
    bool HasSize () const { return !bFirstSize; }

    // Window:
    virtual void Resize () override;
    virtual void DataChanged (DataChangedEvent const& rDCEvt) override;
    // new:
    virtual void OnFirstSize (long nWidth, long nHeight) = 0;

private:
    // the main child window (either ModulWindow or DialogWindow)
    VclPtr<BaseWindow> pChild;

    // when this window has at first (nonempty) size
    bool bFirstSize;

    // horizontal or vertical split strip
    class SplittedSide
    {
    public:
        enum class Side {Left, Bottom};
        SplittedSide (Layout*, Side);
        void Add (DockingWindow*, Size const&);
        void Remove (DockingWindow*);
        bool IsEmpty () const;
        long GetSize () const;
        void ArrangeIn (tools::Rectangle const&);
        void dispose();

    private:
        // the layout window
        Layout& rLayout;
        // horizontal or vertical strip?
        bool bVertical;
        // lower (top or left) or higher (bottom or right) strip?
        bool bLower;
        // rectangle to move in
        tools::Rectangle aRect;
        // size (width or height)
        long nSize;
        // the main splitting line
        VclPtr<Splitter> aSplitter;
        // the dockable windows (and some data)
        struct Item
        {
            // pointer to the dockable window
            VclPtr<DockingWindow> pWin;
            // starting and ending position in the strip
            // They may be different from the actual window position, because
            // the window may fill the space of the adjacent currently
            // non-docking windows, but this change is not stored in these
            // variables. These change only when the splitter lines are moved.
            long nStartPos, nEndPos;
            // splitter line window before the window
            // (the first one is always nullptr)
            VclPtr<Splitter> pSplit;
        };
        std::vector<Item> vItems;

        Point MakePoint (long, long) const;
        Size MakeSize (long, long) const;
        static bool IsDocking (DockingWindow const&);
        DECL_LINK(SplitHdl, Splitter*, void);
        void CheckMarginsFor (Splitter*);
        void InitSplitter (Splitter&);
    } aLeftSide, aBottomSide;
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
