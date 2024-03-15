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

class BubbleWindow;
class MenuBar;
class SystemWindow;
class VclSimpleEvent;
class VclWindowEvent;
struct MenuBarButtonCallbackArg;

class VCL_DLLPUBLIC MenuBarUpdateIconManager
{
private:
    OUString maBubbleTitle;
    OUString maBubbleText;
    Image maBubbleImage;
    VclPtr<BubbleWindow> mpBubbleWin;
    VclPtr<SystemWindow> mpActiveSysWin;
    VclPtr<MenuBar> mpActiveMBar;
    std::vector<VclPtr<MenuBar>> maIconMBars;
    std::vector<sal_uInt16> maIconIDs;

    Link<VclWindowEvent&, void> maWindowEventHdl;
    Link<VclSimpleEvent&, void> maApplicationEventHdl;
    Link<LinkParamNone*, void> maClickHdl;

    Timer maTimeoutTimer;
    Idle maWaitIdle;

    bool mbShowMenuIcon;
    bool mbShowBubble;
    bool mbBubbleChanged;

    DECL_DLLPRIVATE_LINK(UserEventHdl, void*, void);
    DECL_DLLPRIVATE_LINK(TimeOutHdl, Timer*, void);
    DECL_DLLPRIVATE_LINK(WindowEventHdl, VclWindowEvent&, void);
    DECL_DLLPRIVATE_LINK(ApplicationEventHdl, VclSimpleEvent&, void);
    DECL_DLLPRIVATE_LINK(WaitTimeOutHdl, Timer*, void);
    DECL_DLLPRIVATE_LINK(ClickHdl, MenuBarButtonCallbackArg&, bool);
    DECL_DLLPRIVATE_LINK(HighlightHdl, MenuBarButtonCallbackArg&, bool);

    SAL_DLLPRIVATE VclPtr<BubbleWindow> GetBubbleWindow();
    SAL_DLLPRIVATE void SetBubbleChanged();

    SAL_DLLPRIVATE sal_uInt16 GetIconID(MenuBar* pMenuBar) const;

    SAL_DLLPRIVATE void AddMenuBarIcon(SystemWindow& rSysWin, bool bAddEventHdl);
    SAL_DLLPRIVATE void RemoveMenuBarIcon(MenuBar* pMenuBar);
    SAL_DLLPRIVATE void RemoveMenuBarIcons();

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
    const OUString& GetBubbleTitle() const { return maBubbleTitle; }
    const OUString& GetBubbleText() const { return maBubbleText; }

    void RemoveBubbleWindow();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
