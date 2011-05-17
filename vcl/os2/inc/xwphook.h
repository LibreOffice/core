/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * xwphook.h:
 *      header for both xwphook.c and xwpdaemon.c. This is also
 *      included from a number of sources for XFLDR.DLL which need
 *      to interface (configure) the daemon.
 */

/*
 *      Copyright (C) 1999-2002 Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef XWPHOOK_HEADER_INCLUDED
    #define XWPHOOK_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   OS2.INI applications and keys
     *
     ********************************************************************/

    #define INIAPP_XWPHOOK              "XWorkplace:Hook"   // added V0.9.0
    #define INIKEY_HOOK_HOTKEYS         "Hotkeys"           // added V0.9.0
    #define INIKEY_HOOK_CONFIG          "Config"            // added V0.9.0
    #define INIKEY_HOOK_PAGERCONFIG     "PagerConfig"       // V0.9.2 (2000-02-25) [umoeller]
                            // changed V0.9.19 (2002-05-07) [umoeller]
    #define INIKEY_HOOK_PAGERWINPOS     "PagerWinPos"
                            // changed V0.9.19 (2002-05-07) [umoeller]
    #define INIKEY_HOOK_FUNCTIONKEYS    "FuncKeys"          // added V0.9.3 (2000-04-19) [umoeller]
    #define INIKEY_HOOK_MOUSEMAPPINGS   "MouseMappings"     // added V0.9.19 (2002-04-20) [lafaix]

    /* ******************************************************************
     *
     *   Structures
     *
     ********************************************************************/

    // do not change the following, or this will break
    // binary compatibility of the XPager OS2.INI data
    #define MAX_STICKIES        64
    #define STICKYLEN           50

#ifndef __NOPAGER__

    // flags for PAGERCONFIG.aulStickyFlags
    #define SF_CONTAINS          0x00000000L // default
    #define SF_BEGINSWITH        0x00000001L
    #define SF_ENDSWITH          0x00000002L
    #define SF_EQUALS            0x00000004L
    #define SF_MATCHES           0x00000008L        // V0.9.19 (2002-04-17) [umoeller]
    #define SF_OPERATOR_MASK     0x0000FFFFL

    #define SF_INCLUDE           0x00000000L // default
    #define SF_EXCLUDE           0x00010000L
    #define SF_CRITERIA_MASK     0x00010000L

    #define SF_TITLE             0x00000000L // default
    #define SF_ATTRIBUTE_MASK    0xFFFE0000L

    #pragma pack(1)

    /*
     *@@ PAGERCONFIG:
     *      XPager configuration data.
     *      This is stored within the HOOKDATA structure
     *      (statically in the hook DLL) so that both
     *      the daemon and the hook have access to this.
     *
     *@@added V0.9.2 (2000-02-25) [umoeller]
     */

    typedef struct _PAGERCONFIG
    {
        BYTE        cDesktopsX,
                    cDesktopsY,
                // no. of virtual Desktops (x and y)
                    bStartX,
                    bStartY;
                // initial desktop at startup

        ULONG       flPager;
                // flags for "XPager" settings page 1
                #define PGRFL_WRAPAROUND            0x0001
                #define PGRFL_HOTKEYS               0x0002
                #define PGRFL_NOFOLLOWFOCUS         0x0004

                #define PGRMASK_PAGE1 (PGRFL_WRAPAROUND | PGRFL_HOTKEYS)
                #define PGRFL_PAGE1_DEFAULTS        0

                // flags for "XPager" settings page 2
                #define PGRFL_PRESERVEPROPS         0x0010
                    // preserve proportions of XPager win when resizing?
                #define PGRFL_STAYONTOP             0x0020
                    // stay on top?
                #define PGRFL_FLASHTOTOP            0x0040
                #define PGRFL_MINIWINDOWS           0x0080
                    // show mini windows in XPager?
                #define PGRFL_MINIWIN_TITLES        0x0100
                    // show mini window titles?
                #define PGRFL_MINIWIN_MOUSE         0x0200
                    // allow activate/lower by mouse clicks?
                #define PGRFL_INCLUDESECONDARY      0x0400
                #define PGRFL_INCLUDESTICKY         0x0800
                #define PGRFL_MINIWIN_ICONS         0x1000
                    // show mini window icons?
                            // V0.9.19 (2002-06-13) [umoeller]

                #define PGRMASK_PAGE2 (   PGRFL_PRESERVEPROPS       \
                                        | PGRFL_STAYONTOP           \
                                        | PGRFL_FLASHTOTOP          \
                                        | PGRFL_MINIWINDOWS         \
                                        | PGRFL_MINIWIN_TITLES      \
                                        | PGRFL_MINIWIN_MOUSE       \
                                        | PGRFL_INCLUDESECONDARY    \
                                        | PGRFL_INCLUDESTICKY       \
                                        | PGRFL_MINIWIN_ICONS )
                #define PGRFL_PAGE2_DEFAULTS (   PGRFL_PRESERVEPROPS  \
                                               | PGRFL_MINIWINDOWS    \
                                               | PGRFL_MINIWIN_TITLES \
                                               | PGRFL_MINIWIN_MOUSE)

                // flags for "XPager" settings page 3
                #define PGRFL_ADDSTICKYTOGGLE       0x2000
                    // add "sticky" option to system menu?
                            // V1.0.0 (2002-09-14) [lafaix]

                #define PGRMASK_PAGE3 ( PGRFL_ADDSTICKYTOGGLE )
                #define PGRFL_PAGE3_DEFAULTS        0
                #define PGRFL_WINDOWS_KEYS          0x00010000
                // V1.0.3 (2004-10-14) [bird]: Use windows keys for left / right

        ULONG       ulFlashDelay;
                // "flash" delay in milliseconds, if PGRFL_FLASHTOTOP
        ULONG       flKeyShift;        // KC_* values

        // paint settings
        ULONG       flPaintMode;        // as in BKGNDINFO (gpih.h)
        LONG        lcolDesktop1,       // as in BKGNDINFO (gpih.h)
                    lcolDesktop2;       // as in BKGNDINFO (gpih.h)

        LONG        lcolActiveDesktop;  // color of hatching for active desktop

        LONG        lcolGrid;           // grid color (separators between desktops)

        LONG        lcolInactiveWindow,
                    lcolActiveWindow,
                    lcolWindowFrame,
                    lcolInactiveText,
                    lcolActiveText;

        // sticky windows
        ULONG       cStickies;
        ULONG       aulStickyFlags[MAX_STICKIES]; // SF_* flags per sticky
        CHAR        aszStickies[MAX_STICKIES][STICKYLEN];

    } PAGERCONFIG, *PPAGERCONFIG;

    #pragma pack()

#endif

    // flags for HOOKCONFIG.usScrollMode
    #define SM_LINEWISE         0
    #define SM_AMPLIFIED        1

    // flags for HOOKCONFIG.ulAutoHideFlags
    #define AHF_IGNOREMENUS     0x00000001L
    #define AHF_IGNOREBUTTONS   0x00000002L

    // flags for HOOKCONFIG.ulAutoMoveFlags
    #define AMF_ALWAYSMOVE      0x00000001L
    #define AMF_IGNORENOBUTTON  0x00000002L
    #define AMF_ANIMATE         0x00000004L

    /*
     *@@ HOOKCONFIG:
     *      configuration data for the hook and the daemon.
     *
     *      This is stored within the HOOKDATA structure
     *      (statically in the hook DLL) so that both
     *      the daemon and the hook have access to this.
     *
     *      A mirror of this structure is put into OS2.INI
     *      which gets loaded by the XWorkplace settings
     *      objects in XFLDR.DLL to configure the hook.
     *      This gets (re)loaded by the daemon when XFLDR.DLL
     *      posts XDM_HOOKCONFIG to fnwpDaemonObject.
     *
     *      So this is seen by the hook and the daemon;
     *      XFLDR.DLL only writes this back to OS2.INI and
     *      notifies the daemon to reload this.
     *
     *      For every item, the safe default value is null
     *      so the structure can be zeroed to disable
     *      everything.
     *
     *      If settings are added to this structure, they
     *      must be added to the bottom in order not to
     *      break binary compatibility between XWP versions.
     *
     *      Note that the object hotkey _definitions_ are
     *      not part of this structure. Those are set using
     *      XDM_HOTKEYSCHANGED instead. However, object
     *      hotkeys are globally enabled in here (fGlobalHotkeys).
     */
#pragma pack(4)
    typedef struct _HOOKCONFIG
    {
        // Sliding focus:

        PM_BOOL            __fSlidingFocus;
                // enabled?
        ULONG           __ulSlidingFocusDelay;
                // delay in ms; 0 = off

        PM_BOOL            __fSlidingBring2Top;
                // bring windows to top or preserve Z-order
        PM_BOOL            __fSlidingIgnoreDesktop;
                // ignore Desktop windows
        PM_BOOL            __fSlidingIgnoreSeamless;
                // TRUE: ignore seamless Win-OS/2 windows
                // FALSE: always bring them to top

        // Screen corner objects:
        HOBJECT         ahobjDummy[4];      // was four screen corner objects;
                                            // we extended the array to 8 items
                                            // so the array had to be moved to the
                                            // bottom in order not to break binary
                                            // compatibility

        BYTE            bMonitorDrives[30];     // array of 1-byte BOOLs; if any of these
                // is "1", the corresponding drive letter
                // will be monitored for media change
                // (index 1 = A, index 2 = B, index 3 = C, ...).
                // Index 0 is unused to match logical drive numbers.

        // More mouse mappings: V0.9.1 (99-12-03)

        PM_BOOL            fChordWinList;       // deprecated V0.9.19 (2002-04-20) [lafaix]
                // show window list on mb1+2 chord
        PM_BOOL            fSysMenuMB2TitleBar; // deprecated V0.9.19 (2002-04-20) [lafaix]
                // show system menu on mb2 title-bar click

        // Mouse-button-3 scrolling: V0.9.1 (99-12-03)

        PM_BOOL            fMB3Scroll;          // deprecated V0.9.19 (2002-04-20) [lafaix]
                // scroll window contents on MB3Drag
        PM_BOOL            fMB3ScrollReverse;   // deprecated V0.9.19 (2002-04-20) [lafaix]
                // reverse scrolling
        USHORT          usScrollMode;        // deprecated V0.9.19 (2002-04-20) [lafaix]
                // one of the following:
                //  -- SM_LINEWISE (0): scroll fixed, line-wise
                //  -- SM_AMPLIFIED (1): scroll amplified, relative to window size
        USHORT          usMB3ScrollMin;
                // minimum pixels that mouse must be moved;
                // 0 means 1, 1 means 2, ...
        SHORT           sAmplification;
                // amplification (-9 thru +10)
                // the amplification in percent is calculated like this:
                //      percent = 100 + (sAmplification * 10)
                // so we get:
                //      0       -->  100%
                //      2       -->  120%
                //     10       -->  200%
                //     -2       -->  80%
                //     -9       -->  10%

        // Auto-hide mouse pointer: V0.9.1 (99-12-03)
        PM_BOOL            __fAutoHideMouse;
        ULONG           __ulAutoHideDelay;
                // delay in seconds; 0 means 1 second, 2 means 3, ...

        // Global object hotkeys enabled:
        // this can be disabled even if any hotkeys are defined
        // because the hotkeys themselves are stored separately
        // in shared memory
        PM_BOOL            __fGlobalHotkeys;

        // XPager configuration
        PM_BOOL            fRemoved1, // _fXPagerStayOnTop,
                        fRemoved2; //__fSlidingIgnoreXPager;
                                // removed V0.9.19 (2002-05-07) [umoeller]

        // Sliding menus
        PM_BOOL            fSlidingMenus;
                // enabled?
        ULONG           ulSubmenuDelay;
                // delay in ms; 0 = off
        PM_BOOL            fMenuImmediateHilite;

        // Mouse-button-3 single-clicks to MB1 double-clicks
        // V0.9.4 (2000-06-12) [umoeller]
        PM_BOOL            fMB3Click2MB1DblClk; // deprecated V0.9.19 (2002-04-20) [lafaix]

        // Screen corner objects:
        // moved the array down here (there's a dummy above)
        // V0.9.4 (2000-06-12) [umoeller]
        HOBJECT         ahobjHotCornerObjects[8];
                            // Indices:
                            //      0 = lower left corner,
                            //      1 = top left corner,
                            //      2 = lower right corner,
                            //      3 = top right corner;
                            //   borders added V0.9.4 (2000-06-12) [umoeller]:
                            //      4 = top border,
                            //      5 = left border,
                            //      6 = right border,
                            //      7 = bottom border.
                  // V0.9.18 (2002-02-12) [pr]
                  #define SCREENCORNER_MIN            0
                  #define SCREENCORNER_BOTTOMLEFT     0
                  #define SCREENCORNER_TOPLEFT        1
                  #define SCREENCORNER_BOTTOMRIGHT    2
                  #define SCREENCORNER_TOPRIGHT       3
                  #define SCREENCORNER_TOP            4
                  #define SCREENCORNER_LEFT           5
                  #define SCREENCORNER_RIGHT          6
                  #define SCREENCORNER_BOTTOM         7
                  #define SCREENCORNER_MAX            7
                            // If any item is NULLHANDLE, it means the
                            // corner is inactive (no function defined).
                            // If the hiword of the item is 0xFFFF, this
                            // means a special function has been defined:
                            // (#define's added V0.9.19 (2002-04-17) [umoeller]):
                  #define SPECIALOBJ_FIRST                  0xFFFF0000
                  #define SPECIALOBJ_SHOWWINDOWLIST         0xFFFF0000
                  #define SPECIALOBJ_DESKTOPCONTEXTMENU     0xFFFF0001
                  #define SPECIALOBJ_PAGER_SHOW             0xFFFF0002
                  // the following added (2001-01-26) [lafaix]
                  #define SPECIALOBJ_PAGER_UP               0xFFFF0003
                  #define SPECIALOBJ_PAGER_RIGHT            0xFFFF0004
                  #define SPECIALOBJ_PAGER_DOWN             0xFFFF0005
                  #define SPECIALOBJ_PAGER_LEFT             0xFFFF0006
                  // the following added V0.9.18 [pr]
                  #define SPECIALOBJ_SCREENWRAP             0xFFFF0007
                            // Otherwise (> 0 and < 0xFFFF0000), we have
                            // a "real" object handle, and a regular WPS
                            // object is to be opened.

        // special treatment for conditional-cascade submenus when
        // using sliding menus (V0.9.6 (2000-10-27) [umoeller])
        PM_BOOL            fConditionalCascadeSensitive;

        // more XPager configuration V0.9.7 (2000-12-08) [umoeller]
        PM_BOOL            fRemoved3; // __fSlidingIgnoreXCenter;
                            // removed V0.9.19 (2002-05-07) [umoeller]

        // screen corner objects sensitivity; in percent of the
        // adjacents borders.  0 = off, 50 = borders objects disabled
        // V0.9.9 (2001-03-15) [lafaix]
        ULONG           ulCornerSensitivity;

        // Mouse-button-3 autoscroll and push to bottom features
        PM_BOOL            fMB3AutoScroll;      // deprecated V0.9.19 (2002-04-20) [lafaix]
        PM_BOOL            fMB3Push2Bottom;     // deprecated V0.9.19 (2002-04-20) [lafaix]

        // Auto hide and automatic pointer movement options
        // V0.9.14 (2001-08-02) [lafaix]
        ULONG           __ulAutoHideFlags;
        PM_BOOL            __fAutoMoveMouse;
        ULONG           __ulAutoMoveFlags;
        ULONG           __ulAutoMoveDelay;            // V0.9.14 (2001-08-21) [umoeller]
        ULONG           __ulMouseMappingsCount;       // V0.9.19 (2002-04-20) [lafaix]
    } HOOKCONFIG, *PHOOKCONFIG;
#pragma pack()

    /*
     *@@ GLOBALHOTKEY:
     *      single XWorkplace object hotkey definition.
     *      Arrays of this are allocated in shared memory and
     *      used by xwphook.c, xwpdaemn.c, and also XFldObject
     *      for hotkey manipulation and detection.
     */

    typedef struct _GLOBALHOTKEY
    {
        USHORT  usFlags;
                        // Keyboard control codes:
                        // SHORT1FROMMP(mp1) of WM_CHAR, filtered.
                        // Only the following flags will be set:
                        // --   KC_CTRL
                        // --   KC_ALT
                        // --   KC_SHIFT
                        // --   KC_VIRTUALKEY
                        // --   KC_INVALIDCOMP: special flag used if the
                        //      scan code represents one of the user-defined
                        //      function keys in the XWPKeyboard object.
                        // KC_CTRL, KC_ALT, KC_SHIFT work always,
                        // no matter if we're in a PM or VIO session.
                        // However, for some reason, KC_VIRTUALKEY is
                        // never set in VIO sessions. We still store it
                        // in this structure though to be able to display
                        // the hotkey in the configuration pages.
                        // The hook will filter that out since the scan
                        // code is good enough to identify the key.
        UCHAR   ucScanCode;
                        // Hardware scan code:
                        // CHAR4FROMMP(mp1) of WM_CHAR.
                        // As opposed to what we do with folder hotkeys,
                        // this must be stored also, because we must use
                        // the scan code for WM_CHAR processing in the hook
                        // to identify hotkeys. We cannot use usKeyCode
                        // because that's different in VIO sessions, while
                        // this one is always the same.
                        // Even if any of Ctrl, Alt, Shift are pressed, this
                        // has the scan code of the additional key which was
                        // pressed.
        USHORT  usKeyCode;
                        // key code:
                        // if KC_VIRTUALKEY is set in usFlags, this has usvk,
                        // otherwise usch from WM_CHAR.
                        // This is only used to be able to display the hotkey
                        // in the hotkey configuration dialogs; we do _not_ use
                        // this to check WM_CHAR messages in the hook, because
                        // this is different between PM and VIO sessions.
        ULONG   ulHandle;
                        // handle to post to thread-1 object window (kernel.c);
                        // this is normally the HOBJECT of the object to be
                        // opened.
    } GLOBALHOTKEY, *PGLOBALHOTKEY;

    /*
     *@@ MOUSEMAPPING:
     *      single XWorkplace mouse mapping definition.
     *      Arrays of this are allocated in shared memory and
     *      used by xwphook.c, xwpdaemn.c, and also XWPMouse
     *      for mouse mappings manipulation and detection.
     *
     *@@added V0.9.19 (2002-04-20) [lafaix]
     */

    typedef struct _MOUSEMAPPING
    {
        USHORT  usEvent;
                        // One of the MME_* values (or MME_XBUTTON_FIRST+n
                        // to denote extra button n,  0 <= n < 32).
                  // standard mouse events
                  #define MME_BUTTON1CLICK   0x0001
                  #define MME_BUTTON2CLICK   0x0002
                  #define MME_BUTTON3CLICK   0x0003
                  #define MME_BUTTON1DRAG    0x0004
                  #define MME_BUTTON2DRAG    0x0005
                  #define MME_BUTTON3DRAG    0x0006
                  #define MME_CHORD          0x0007
                  // wheel/stick events
                  #define MME_UP             0x1001
                  #define MME_DOWN           0x1002
                  #define MME_LEFT           0x1003
                  #define MME_RIGHT          0x1004
                  // the 2X and 3X variants are handy when mapping an action
                  // to a wheel/stick movement.  They map at least 2 (or 3)
                  // consecutive events, and hence disambiguate an unexpected
                  // wheel/stick event.
                  #define MME_UP2X           0x2001
                  #define MME_DOWN2X         0x2002
                  #define MME_LEFT2X         0x2003
                  #define MME_RIGHT2X        0x2004
                  #define MME_UP3X           0x3001
                  #define MME_DOWN3X         0x3002
                  #define MME_LEFT3X         0x3003
                  #define MME_RIGHT3X        0x3004
                  // extra buttons
                  #define MME_XBUTTON_FIRST  0x4000
                  // type helpers
                  #define MME_TYPE_MASK      0xF000
                  #define MME_TYPE_STANDARD  0x0000
                  #define MME_TYPE_WHEEL     0x1000
                  #define MME_TYPE_WHEEL2X   0x2000
                  #define MME_TYPE_WHEEL3X   0x3000
                  #define MME_TYPE_XBUTTON   0x4000
                  #define MME_TYPE_DISABLED  0x8000

        USHORT  usModifiers;
                        // A possibly empty combination of KC_SHIFT, KC_CTRL,
                        // and KC_ALT.
        CHAR    achLocation[32];
                        // The class name upon which this mapping applies.
                        // An empty location (i.e., achLocation[0] = 0) means
                        // a global mapping.
        CHAR    achPluginName[8];
                        // The plugin library providing the action defined for
                        // this mapping.  An empty location (i.e.,
                        // achPluginName[0] = 0) means a buildin action.
        USHORT  usAction;
                        // The action (relative to the plugin) defined for
                        // this event.
        BYTE    abSetup[16];
                        // A private area possibly refining the action.
    } MOUSEMAPPING, *PMOUSEMAPPING;

    /*
     *@@ FUNCTIONKEY:
     *      XWorkplace function key description.
     *      An array of these is returned by
     *      hifQueryFunctionKeys().
     *
     *@@added V0.9.3 (2000-04-19) [umoeller]
     */

    typedef struct _FUNCTIONKEY
    {
        UCHAR       ucScanCode;         // hardware scan code;
                                        // CHAR4FROMMP(mp1) of WM_CHAR
        CHAR        szDescription[30];  // key description (e.g. "Win left")
        PM_BOOL        fModifier;          // TRUE if the scan code represents
                                        // a modifier key which can be pressed
                                        // together with another key, similar
                                        // to Ctrl or Alt or Del; this will
                                        // allow us to do things like "WinLeft + C"
    } FUNCTIONKEY, *PFUNCTIONKEY;

    /* ******************************************************************
     *
     *   Messages
     *
     ********************************************************************/

    #define XDM_HOOKINSTALL         (WM_USER + 400)

    #define XDM_DESKTOPREADY        (WM_USER + 401)

    #define XDM_HOOKCONFIG          (WM_USER + 402)

#ifndef __NOPAGER__
    #define XDM_STARTSTOPPAGER   (WM_USER + 403)

    #define XDM_PAGERCONFIG      (WM_USER + 404)
        // flags for XDM_PAGERCONFIG:
        #define PGRCFG_REPAINT         0x0001  // invalidates mini windows
        #define PGRCFG_REFORMAT        0x0004  // causes PGRM_POSITIONFRAME,
                                                // repaints background too
        #define PGRCFG_STICKIES        0x0008  // sticky windows have changed
#endif

    #define XDM_HOTKEYPRESSED       (WM_USER + 405)

    #define XDM_HOTKEYSCHANGED      (WM_USER + 406)

    #define XDM_FUNCTIONKEYSCHANGED (WM_USER + 407)

#ifndef __NOSLIDINGFOCUS__
    #define XDM_SLIDINGFOCUS        (WM_USER + 408)
#endif

    #define XDM_SLIDINGMENU         (WM_USER + 409)

    #define XDM_HOTCORNER           (WM_USER + 410)

    #define XDM_WMCHORDWINLIST      (WM_USER + 411)

    // added V0.9.9 (2001-03-18) [lafaix]
    #define XDM_BEGINSCROLL         (WM_USER + 413)
    #define XDM_SETPOINTER          (WM_USER + 414)
    #define XDM_ENDSCROLL           (WM_USER + 415)

    // added V0.9.12 (2001-05-12) [umoeller]
    #define XDM_RECOVERWINDOWS      (WM_USER + 416)

    #define XDM_ADDDISKWATCH        (WM_USER + 417)

    /*
     *@@ ADDDISKWATCH:
     *      struct used with XDM_ADDDISKWATCH.
     *
     *@@added V0.9.14 (2001-08-01) [umoeller]
     */

    typedef struct _ADDDISKWATCH
    {
        ULONG           ulLogicalDrive;         // disk to be monitored
        HWND            hwndNotify;             // window to be notified on change
        ULONG           ulMessage;              // message to be posted to window
    } ADDDISKWATCH, *PADDDISKWATCH;

    #define XDM_REMOVEDISKWATCH     (WM_USER + 418)

    #define XDM_QUERYDISKS          (WM_USER + 419)

    #define XDM_ADDCLICKWATCH       (WM_USER + 420)

    #define XDM_MOUSECLICKED        (WM_USER + 421)

#ifndef __NOMOVEMENT2FEATURES__
    #define XDM_MOVEPTRTOBUTTON     (WM_USER + 422)
#endif

    #define XDM_DISABLEHOTKEYSTEMP  (WM_USER + 423)

    #define XDM_STARTAPP            (WM_USER + 424)

    #define XDM_ADDWINLISTWATCH     (WM_USER + 425)

    #define XDM_WINDOWCHANGE        (WM_USER + 426)

    #define XDM_ICONCHANGE          (WM_USER + 427)

    #define XDM_QUERYWINLIST        (WM_USER + 428)

    #define XDM_REMOVEWINLISTWATCH  (WM_USER + 429) // V0.9.19 (2002-06-14) [lafaix]

    #define XDM_REMOVECLICKWATCH    (WM_USER + 430) // V0.9.19 (2002-06-14) [lafaix]

#ifndef __NOPAGER__
    #define XDM_TOGGLETRANSIENTSTICKY (WM_USER + 431) // V1.0.0 (2002-07-26) [lafaix]
    #define XDM_ISTRANSIENTSTICKY   (WM_USER + 432)
#endif

    #define XDM_NLSCHANGED          (WM_USER + 433) // V1.0.0 (2002-09-15) [lafaix]
#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
