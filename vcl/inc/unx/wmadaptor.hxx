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

#ifndef _VCL_WMADAPTOR_HXX_
#define _VCL_WMADAPTOR_HXX_

#include <tools/string.hxx>
#include <tools/gen.hxx>
#ifndef _PREX_H
#include <prex.h>
#include <X11/Xlib.h>
#include <postx.h>
#endif
#include <vclpluginapi.h>
#include <vector>

class SalDisplay;
class X11SalFrame;

namespace vcl_sal {

class VCLPLUG_GEN_PUBLIC WMAdaptor
{
public:
    enum WMAtom {
        // atoms for types
        UTF8_STRING,

        // atoms for extended WM hints
        NET_SUPPORTED,
        NET_SUPPORTING_WM_CHECK,
        NET_WM_NAME,
        NET_WM_DESKTOP,
        NET_WM_ICON_NAME,
        NET_WM_PID,
        NET_WM_PING,
        NET_WM_STATE,
        NET_WM_STATE_MAXIMIZED_HORZ,
        NET_WM_STATE_MAXIMIZED_VERT,
        NET_WM_STATE_MODAL,
        NET_WM_STATE_SHADED,
        NET_WM_STATE_SKIP_PAGER,
        NET_WM_STATE_SKIP_TASKBAR,
        NET_WM_STATE_STAYS_ON_TOP,
        NET_WM_STATE_STICKY,
        NET_WM_STATE_FULLSCREEN,
        NET_WM_STRUT,
        NET_WM_STRUT_PARTIAL,
        NET_WM_USER_TIME,
        NET_WM_WINDOW_TYPE,
        NET_WM_WINDOW_TYPE_DESKTOP,
        NET_WM_WINDOW_TYPE_DIALOG,
        NET_WM_WINDOW_TYPE_DOCK,
        NET_WM_WINDOW_TYPE_MENU,
        NET_WM_WINDOW_TYPE_NORMAL,
        NET_WM_WINDOW_TYPE_TOOLBAR,
        KDE_NET_WM_WINDOW_TYPE_OVERRIDE,
        NET_WM_WINDOW_TYPE_SPLASH,
        NET_WM_WINDOW_TYPE_UTILITY,
        NET_NUMBER_OF_DESKTOPS,
        NET_CURRENT_DESKTOP,
        NET_WORKAREA,

        // atoms for Gnome WM hints
        WIN_SUPPORTING_WM_CHECK,
        WIN_PROTOCOLS,
        WIN_WORKSPACE_COUNT,
        WIN_WORKSPACE,
        WIN_LAYER,
        WIN_STATE,
        WIN_HINTS,
        WIN_APP_STATE,
        WIN_EXPANDED_SIZE,
        WIN_ICONS,
        WIN_WORKSPACE_NAMES,
        WIN_CLIENT_LIST,

        // atoms for general WM hints
        WM_STATE,
        MOTIF_WM_HINTS,
        WM_PROTOCOLS,
        WM_DELETE_WINDOW,
        WM_TAKE_FOCUS,
        WM_CLIENT_LEADER,
        WM_COMMAND,
        WM_LOCALE_NAME,
        WM_TRANSIENT_FOR,

        // special atoms
        SAL_QUITEVENT,
        SAL_USEREVENT,
        SAL_EXTTEXTEVENT,
        SAL_GETTIMEEVENT,
        DTWM_IS_RUNNING,
        VCL_SYSTEM_SETTINGS,
        XSETTINGS,
        XEMBED,
        XEMBED_INFO,
        NetAtomMax
    };

    /*
     *  flags for frame decoration
     */
    static const int decoration_Title           = 0x00000001;
    static const int decoration_Border      = 0x00000002;
    static const int decoration_Resize      = 0x00000004;
    static const int decoration_MinimizeBtn = 0x00000008;
    static const int decoration_MaximizeBtn = 0x00000010;
    static const int decoration_CloseBtn        = 0x00000020;
    static const int decoration_All         = 0x10000000;

    /*
     *  window type
     */
    enum WMWindowType
    {
        windowType_Normal,
        windowType_ModalDialogue,
        windowType_ModelessDialogue,
        windowType_Utility,
        windowType_Splash,
        windowType_Toolbar,
        windowType_Dock
    };

protected:
    SalDisplay*             m_pSalDisplay;      // Display to use
    Display*                m_pDisplay;         // X Display of SalDisplay
    String                  m_aWMName;
    Atom                    m_aWMAtoms[ NetAtomMax];
    int                     m_nDesktops;
    bool                    m_bEqualWorkAreas;
    ::std::vector< Rectangle >
                            m_aWMWorkAreas;
    bool                    m_bTransientBehaviour;
    bool                    m_bEnableAlwaysOnTopWorks;
    bool                    m_bLegacyPartialFullscreen;
    int                     m_nWinGravity;
    int                     m_nInitWinGravity;
    bool                    m_bWMshouldSwitchWorkspace;
    bool                    m_bWMshouldSwitchWorkspaceInit;

    WMAdaptor( SalDisplay * )
;
    void initAtoms();
    bool getNetWmName();

    /*
     *  returns whether this instance is useful
     *  only useful for createWMAdaptor
     */
    virtual bool isValid() const;

    bool getWMshouldSwitchWorkspace() const;
public:
    virtual ~WMAdaptor();

    /*
     *  creates a vaild WMAdaptor instance for the SalDisplay
     */
    static WMAdaptor* createWMAdaptor( SalDisplay* );

    /*
     *  may return an empty string if the window manager could
     *  not be identified.
     */
    const String& getWindowManagerName() const
    { return m_aWMName; }

    /*
     *  gets the number of workareas (virtual desktops)
     */
    int getWorkAreaCount() const
    { return m_aWMWorkAreas.size(); }

    /*
     * gets the current work area/desktop number: [0,m_nDesktops[ or -1 if unknown
     */
    int getCurrentWorkArea() const;
    /*
     * gets the workarea the specified window is on (or -1)
     */
    int getWindowWorkArea( XLIB_Window aWindow ) const;
    /*
     *  gets the specified workarea
     */
    const Rectangle& getWorkArea( int n ) const
    { return m_aWMWorkAreas[n]; }

    /*
     * attempt to switch the desktop to a certain workarea (ie. virtual desktops)
     * if bConsiderWM is true, then on some WMs the call will not result in any action
     */
    void switchToWorkArea( int nWorkArea, bool bConsiderWM = true ) const;

    /*
     *  sets window title
     */
    virtual void setWMName( X11SalFrame* pFrame, const String& rWMName ) const;

    /*
     * set NET_WM_PID
     */
    virtual void setPID( X11SalFrame* pFrame ) const;

    /*
     * set WM_CLIENT_MACHINE
     */
    virtual void setClientMachine( X11SalFrame* pFrame ) const;

    virtual void answerPing( X11SalFrame*, XClientMessageEvent* ) const;

    /*
     *  maximizes frame
     *  maximization can be toggled in either direction
     *  to get the original position and size
     *  use maximizeFrame( pFrame, false, false )
     */
    virtual void maximizeFrame( X11SalFrame* pFrame, bool bHorizontal = true, bool bVertical = true ) const;
    /*
     *  start/stop fullscreen mode on a frame
     */
    virtual void showFullScreen( X11SalFrame* pFrame, bool bFullScreen ) const;
    /*
     *  tell whether legacy partial full screen handling is necessary
     *  see #i107249#: NET_WM_STATE_FULLSCREEN is not well defined, but de facto
     *  modern WM's interpret it the "right" way, namely they make "full screen"
     *  taking twin view or Xinerama into accound and honor the positioning hints
     *  to see which screen actually was meant to use for fullscreen.
     */
    bool isLegacyPartialFullscreen() const
    { return m_bLegacyPartialFullscreen; }
    /*
     * set window struts
     */
    virtual void setFrameStruts( X11SalFrame*pFrame,
                                 int left, int right, int top, int bottom,
                                 int left_start_y, int left_end_y,
                                 int right_start_y, int right_end_y,
                                 int top_start_x, int top_end_x,
                                 int bottom_start_x, int bottom_end_x ) const;
    /*
     * set _NET_WM_USER_TIME property, if NetWM
     */
    virtual void setUserTime( X11SalFrame* i_pFrame, long i_nUserTime ) const;

    /*
     *  tells whether fullscreen mode is supported by WM
     */
    bool supportsFullScreen() const { return m_aWMAtoms[ NET_WM_STATE_FULLSCREEN ] != 0; }

    /*
     *  shade/unshade frame
     */
    virtual void shade( X11SalFrame* pFrame, bool bToShaded ) const;

    /*
     *  set hints what decoration is needed;
     *  must be called before showing the frame
     */
    virtual void setFrameTypeAndDecoration( X11SalFrame* pFrame, WMWindowType eType, int nDecorationFlags, X11SalFrame* pTransientFrame = NULL ) const;

    /*
     *  tells whether there is WM support for splash screens
     */
    bool supportsSplash() const { return m_aWMAtoms[ NET_WM_WINDOW_TYPE_SPLASH ] != 0; }

    /*
     *  tells whteher there is WM support for NET_WM_WINDOW_TYPE_TOOLBAR
     */
    bool supportsToolbar() const { return m_aWMAtoms[ NET_WM_WINDOW_TYPE_TOOLBAR ] != 0; }

    /*
     *  enables always on top or equivalent if possible
     */
    virtual void enableAlwaysOnTop( X11SalFrame* pFrame, bool bEnable ) const;

    /*
     *  tells whether enableAlwaysOnTop actually works with this WM
     */
    bool isAlwaysOnTopOK() const { return m_bEnableAlwaysOnTopWorks; }

    /*
     *  handle WM messages (especially WM state changes)
     */
    virtual int handlePropertyNotify( X11SalFrame* pFrame, XPropertyEvent* pEvent ) const;

    /*
     * called by SalFrame::Show: time to update state properties
     */
    virtual void frameIsMapping( X11SalFrame* ) const;

    /*
     *  gets a WM atom
     */
    Atom getAtom( WMAtom eAtom ) const
    { return m_aWMAtoms[ eAtom ]; }

    int getPositionWinGravity () const
    { return m_nWinGravity; }
    int getInitWinGravity() const
    { return m_nInitWinGravity; }

    /*
     *  expected behaviour is that the WM will not allow transient
     *  windows to get stacked behind the windows they are transient for
     */
    bool isTransientBehaviourAsExpected() const
    { return m_bTransientBehaviour; }

    /*
     *  changes the transient hint of a window to reference frame
     *  if reference frame is NULL the root window is used instead
     */
    void changeReferenceFrame( X11SalFrame* pFrame, X11SalFrame* pReferenceFrame ) const;
};

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
