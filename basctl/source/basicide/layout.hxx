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

#ifndef BASCTL_LAYOUT_HXX
#define BASCTL_LAYOUT_HXX

#include <vcl/window.hxx>
#include <vcl/split.hxx>
#include <unotools/options.hxx>

#include <vector>
#include <boost/shared_ptr.hpp>

class DockingWindow;
class BasicDockingWindow;
class IDEBaseWindow;
class SfxRequest;
class SfxItemSet;

namespace basctl
{

//
// Layout -- the common base of ModulLayout and DialogLayout.
// Handles the splitting lines and the dockable windows.
//
class Layout: public Window
{
public:
    void DockaWindow (DockingWindow*);
    void ArrangeWindows ();

    virtual void Activating (IDEBaseWindow&);
    virtual void Deactivating ();
    virtual void GetState (SfxItemSet&, unsigned nWhich) = 0;
    virtual void UpdateDebug (bool bBasicStopped ) = 0;

protected:
    Layout (Window* pParent);
    virtual ~Layout ();

    void AddToLeft   (BasicDockingWindow* pWin, Size const& rSize) { aLeftSide.Add(pWin, rSize); }
    void AddToBottom (BasicDockingWindow* pWin, Size const& rSize) { aBottomSide.Add(pWin, rSize); }

protected:
    // Window:
    virtual void Resize ();
    virtual void DataChanged (DataChangedEvent const& rDCEvt);
    // new:
    virtual void OnFirstSize (int nWidth, int nHeight) = 0;

private:
    // the main child window (either ModulWindow or DialogWindow)
    IDEBaseWindow* pChild;

    // when this window has at first (nonempty) size
    bool bFirstSize;

    // horizontal or vertical splitted strip
    class SplittedSide
    {
    public:
        enum Side {Right, Top, Left, Bottom};
        SplittedSide (Layout*, Side);
        void Add (BasicDockingWindow*, Size const&);
        bool IsEmpty () const;
        int  GetSize () const;
        void ArrangeIn (Rectangle const&);

    private:
        // the layout window
        Layout& rLayout;
        // ArrangeIn() is called at first time?
        bool bFirstArrange;
        // horizontal or vertical strip?
        bool bVertical;
        // lower (top or left) or higher (bottom or right) strip?
        bool bLower;
        // rectangle to move in
        Rectangle aRect;
        // size (width or height)
        int nSize;
        // last position (between Add()s)
        int nLastPos;
        // the main splitting line
        Splitter aSplitter;
        // the dockable windows
        std::vector<BasicDockingWindow*> vWindows;
        // splitting lines between the docking windows (vWindows.size() - 1)
        std::vector<boost::shared_ptr<Splitter> > vSplitters;

    private:
        Point MakePoint (int, int) const;
        Size MakeSize (int, int) const;
    private:
        DECL_LINK(SplitHdl, Splitter*);
        void CheckMarginsFor (Splitter*);
        void InitSplitter (Splitter&);
    } aLeftSide, aBottomSide;
};

} // namespace basctl

#endif // BASCTL_LAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
