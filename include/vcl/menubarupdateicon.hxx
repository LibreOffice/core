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

#include <tools/link.hxx>
#include <vcl/idle.hxx>
#include <vcl/image.hxx>
#include <vcl/menu.hxx>

class BubbleWindow;
class SystemWindow;
class VclSimpleEvent;
class VclWindowEvent;

class VCL_DLLPUBLIC MenuBarUpdateIconManager
{
private:
    OUString maBubbleTitle;
    OUString maBubbleText;
    OUString maBubbleImageURL;
    Image maBubbleImage;
    VclPtr<BubbleWindow> mpBubbleWin;
    VclPtr<SystemWindow> mpIconSysWin;
    VclPtr<MenuBar> mpIconMBar;

    Link<VclWindowEvent&, void> maWindowEventHdl;
    Link<VclSimpleEvent&, void> maApplicationEventHdl;
    Link<LinkParamNone*, void> maClickHdl;

    Timer maTimeoutTimer;
    Idle maWaitIdle;

    sal_uInt16 mnIconID;

    bool mbShowMenuIcon;
    bool mbShowBubble;
    bool mbBubbleChanged;

    DECL_LINK(UserEventHdl, void*, void);
    DECL_LINK(TimeOutHdl, Timer*, void);
    DECL_LINK(WindowEventHdl, VclWindowEvent&, void);
    DECL_LINK(ApplicationEventHdl, VclSimpleEvent&, void);
    DECL_LINK(WaitTimeOutHdl, Timer*, void);
    DECL_LINK(ClickHdl, MenuBar::MenuBarButtonCallbackArg&, bool);
    DECL_LINK(HighlightHdl, MenuBar::MenuBarButtonCallbackArg&, bool);

    VclPtr<BubbleWindow> GetBubbleWindow();
    void SetBubbleChanged();

public:
    MenuBarUpdateIconManager();
    ~MenuBarUpdateIconManager();

    void SetShowMenuIcon(bool bShowMenuIcon);
    void SetShowBubble(bool bShowBubble);
    void SetBubbleImage(const Image& rImage);
    void SetBubbleTitle(const OUString& rTitle);
    void SetBubbleText(const OUString& rText);

    void SetClickHdl(const Link<LinkParamNone*, void>& rHdl) { maClickHdl = rHdl; }

    bool GetShowMenuIcon() const { return mbShowMenuIcon; }
    bool GetShowBubble() const { return mbShowBubble; }
    OUString GetBubbleTitle() const { return maBubbleTitle; }
    OUString GetBubbleText() const { return maBubbleText; }

    void RemoveBubbleWindow(bool bRemoveIcon);

    void AddMenuBarIcon(SystemWindow* pSysWin, bool bAddEventHdl);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
