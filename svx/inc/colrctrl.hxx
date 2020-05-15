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

#include <sal/types.h>
#include <sfx2/dockwin.hxx>
#include <svl/lstner.hxx>
#include <svx/SvxColorValueSet.hxx>
#include <svx/xtable.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/vclptr.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>

namespace vcl { class Window; }

class SfxBindings;
class SfxBroadcaster;
class SfxChildWindow;
class SfxHint;
class SvxColorValueSetData;

/*************************************************************************
|*
|* SvxColorValueSet_docking
|*
\************************************************************************/

class SAL_WARN_UNUSED SvxColorValueSet_docking final : public SvxColorValueSet
{
    rtl::Reference<SvxColorValueSetData> m_xHelper;
    bool            mbLeftButton;

    // ValueSet
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    virtual bool StartDrag() override;

public:
    SvxColorValueSet_docking(std::unique_ptr<weld::ScrolledWindow> pWindow);

    bool IsLeftButton() const { return mbLeftButton; }
};

/*************************************************************************
|*
|* SvxColorDockingWindow
|*
\************************************************************************/

class SAL_WARN_UNUSED SvxColorDockingWindow final : public SfxDockingWindow, public SfxListener
{
friend class SvxColorChildWindow;

private:
    XColorListRef       pColorList;
    std::unique_ptr<SvxColorValueSet_docking> xColorSet;
    std::unique_ptr<weld::CustomWeld> xColorSetWin;

    void                FillValueSet();

    DECL_LINK(SelectHdl, ValueSet*, void);

    /** This function is called when the window gets the focus.  It grabs
        the focus to the color value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus() override;

    virtual bool    Close() override;

public:
    SvxColorDockingWindow(SfxBindings* pBindings,
                          SfxChildWindow *pCW,
                          vcl::Window* pParent);
    virtual ~SvxColorDockingWindow() override;
    virtual void    dispose() override;

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
