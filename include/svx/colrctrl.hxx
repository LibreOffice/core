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
#ifndef INCLUDED_SVX_COLRCTRL_HXX
#define INCLUDED_SVX_COLRCTRL_HXX

#include <sal/types.h>
#include <sfx2/dockwin.hxx>
#include <svl/lstner.hxx>
#include <svtools/transfer.hxx>
#include <svtools/valueset.hxx>
#include <svx/SvxColorValueSet.hxx>
#include <svx/xtable.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <tools/wintypes.hxx>
#include <vcl/event.hxx>
#include <vcl/vclptr.hxx>

namespace vcl { class Window; }

class CommandEvent;
class SfxBindings;
class SfxBroadcaster;
class SfxChildWindow;
class SfxHint;

/*************************************************************************
|*
|* SvxColorValueSet_docking
|*
\************************************************************************/

class SAL_WARN_UNUSED SvxColorValueSet_docking : public SvxColorValueSet, public DragSourceHelper
{
private:
    using SvxColorValueSet::StartDrag;

    bool            mbLeftButton;
    Point           aDragPosPixel;

protected:

    void            DoDrag();

    // ValueSet
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;

    // DragSourceHelper
    virtual void    StartDrag( sal_Int8 nAction, const Point& rPtPixel ) override;

                    DECL_LINK(ExecDragHdl, void*, void);

public:
    SvxColorValueSet_docking( vcl::Window* pParent );

    bool IsLeftButton() const { return mbLeftButton; }
};

/*************************************************************************
|*
|* SvxColorDockingWindow
|*
\************************************************************************/

class SAL_WARN_UNUSED SvxColorDockingWindow : public SfxDockingWindow, public SfxListener
{
friend class SvxColorChildWindow;

private:
    XColorListRef       pColorList;
    VclPtr<SvxColorValueSet_docking> aColorSet;
    sal_uInt16          nLeftSlot;
    sal_uInt16          nRightSlot;
    sal_uInt16          nCols;
    sal_uInt16          nLines;
    long                nCount;
    Size                aItemSize;

    void                FillValueSet();
    void                SetSize();
       DECL_LINK( SelectHdl, ValueSet*, void );

    /** This function is called when the window gets the focus.  It grabs
        the focus to the color value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus() override;

protected:
    virtual bool    Close() override;
    virtual void    Resize() override;
    virtual void    Resizing( Size& rSize ) override;

public:
    SvxColorDockingWindow(SfxBindings* pBindings,
                          SfxChildWindow *pCW,
                          vcl::Window* pParent);
    virtual ~SvxColorDockingWindow() override;
    virtual void    dispose() override;

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
