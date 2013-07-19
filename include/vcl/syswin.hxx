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

#ifndef _SV_SYSWIN_HXX
#define _SV_SYSWIN_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/window.hxx>

class ModalDialog;
class MenuBar;
class TaskPaneList;

// --------------
// - Icon-Types -
// --------------

#define ICON_LO_DEFAULT                 1
#define ICON_TEXT_DOCUMENT              2
#define ICON_TEXT_TEMPLATE              3
#define ICON_SPREADSHEET_DOCUMENT       4
#define ICON_SPREADSHEET_TEMPLATE       5
#define ICON_DRAWING_DOCUMENT           6
#define ICON_DRAWING_TEMPLATE           7
#define ICON_PRESENTATION_DOCUMENT      8
#define ICON_PRESENTATION_TEMPLATE      9
#define ICON_MASTER_DOCUMENT            10
#define ICON_TEMPLATE                   11
#define ICON_DATABASE_DOCUMENT          12
#define ICON_MATH_DOCUMENT              13
#define ICON_MACROLIBRARY               1
#define ICON_SETUP                      500

// -------------------
// - WindowStateData -
// -------------------

#define WINDOWSTATE_MASK_X                  ((sal_uLong)0x00000001)
#define WINDOWSTATE_MASK_Y                  ((sal_uLong)0x00000002)
#define WINDOWSTATE_MASK_WIDTH              ((sal_uLong)0x00000004)
#define WINDOWSTATE_MASK_HEIGHT             ((sal_uLong)0x00000008)
#define WINDOWSTATE_MASK_STATE              ((sal_uLong)0x00000010)
#define WINDOWSTATE_MASK_MINIMIZED          ((sal_uLong)0x00000020)
#define WINDOWSTATE_MASK_MAXIMIZED_X        ((sal_uLong)0x00000100)
#define WINDOWSTATE_MASK_MAXIMIZED_Y        ((sal_uLong)0x00000200)
#define WINDOWSTATE_MASK_MAXIMIZED_WIDTH    ((sal_uLong)0x00000400)
#define WINDOWSTATE_MASK_MAXIMIZED_HEIGHT   ((sal_uLong)0x00000800)
#define WINDOWSTATE_MASK_POS  (WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y)
#define WINDOWSTATE_MASK_ALL  (WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y | WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT | WINDOWSTATE_MASK_MAXIMIZED_X | WINDOWSTATE_MASK_MAXIMIZED_Y | WINDOWSTATE_MASK_MAXIMIZED_WIDTH | WINDOWSTATE_MASK_MAXIMIZED_HEIGHT | WINDOWSTATE_MASK_STATE | WINDOWSTATE_MASK_MINIMIZED)

#define WINDOWSTATE_STATE_NORMAL         ((sal_uLong)0x00000001)
#define WINDOWSTATE_STATE_MINIMIZED      ((sal_uLong)0x00000002)
#define WINDOWSTATE_STATE_MAXIMIZED      ((sal_uLong)0x00000004)
#define WINDOWSTATE_STATE_ROLLUP         ((sal_uLong)0x00000008)
#define WINDOWSTATE_STATE_MAXIMIZED_HORZ ((sal_uLong)0x00000010)
#define WINDOWSTATE_STATE_MAXIMIZED_VERT ((sal_uLong)0x00000020)
#define WINDOWSTATE_STATE_SYSTEMMASK     ((sal_uLong)0x0000FFFF)

class VCL_PLUGIN_PUBLIC WindowStateData
{
private:
    sal_uInt32          mnValidMask;
    int                 mnX;
    int                 mnY;
    unsigned int        mnWidth;
    unsigned int        mnHeight;
    int                 mnMaximizedX;
    int                 mnMaximizedY;
    unsigned int        mnMaximizedWidth;
    unsigned int        mnMaximizedHeight;
    sal_uInt32          mnState;

public:
                WindowStateData()
                {
                    mnValidMask = mnX = mnY = mnWidth = mnHeight = mnState = 0;
                    mnMaximizedX = mnMaximizedY = mnMaximizedWidth = mnMaximizedHeight = 0;
                }

    void        SetMask( sal_uLong nValidMask ) { mnValidMask = nValidMask; }
    sal_uInt32  GetMask() const { return mnValidMask; }

    void         SetX( int nX ) { mnX = nX; }
    int          GetX() const { return mnX; }
    void         SetY( int nY ) { mnY = nY; }
    int          GetY() const { return mnY; }
    void         SetWidth( unsigned int nWidth ) { mnWidth = nWidth; }
    unsigned int GetWidth() const { return mnWidth; }
    void         SetHeight( unsigned int nHeight ) { mnHeight = nHeight; }
    unsigned int GetHeight() const { return mnHeight; }
    void         SetState( sal_uInt32 nState ) { mnState = nState; }
    sal_uInt32   GetState() const { return mnState; }
    void         SetMaximizedX( int nRX ) { mnMaximizedX = nRX; }
    int          GetMaximizedX() const { return mnMaximizedX; }
    void         SetMaximizedY( int nRY ) { mnMaximizedY = nRY; }
    int          GetMaximizedY() const { return mnMaximizedY; }
    void         SetMaximizedWidth( unsigned int nRWidth ) { mnMaximizedWidth = nRWidth; }
    unsigned int GetMaximizedWidth() const { return mnMaximizedWidth; }
    void         SetMaximizedHeight( unsigned int nRHeight ) { mnMaximizedHeight = nRHeight; }
    unsigned int GetMaximizedHeight() const { return mnMaximizedHeight; }
};

// ----------------------
// - SystemWindow-Types -
// ----------------------

#define MENUBAR_MODE_NORMAL         ((sal_uInt16)0)
#define MENUBAR_MODE_HIDE           ((sal_uInt16)1)

#define TITLE_BUTTON_DOCKING        ((sal_uInt16)1)
#define TITLE_BUTTON_HIDE           ((sal_uInt16)2)
#define TITLE_BUTTON_MENU           ((sal_uInt16)4)

// ----------------
// - SystemWindow -
// ----------------


class VCL_DLLPUBLIC SystemWindow : public Window
{
    friend class WorkWindow;
    class ImplData;

private:
    MenuBar*        mpMenuBar;
    Size            maOrgSize;
    Size            maRollUpOutSize;
    Size            maMinOutSize;
    sal_Bool            mbPined;
    sal_Bool            mbRollUp;
    sal_Bool            mbRollFunc;
    sal_Bool            mbDockBtn;
    sal_Bool            mbHideBtn;
    sal_Bool            mbSysChild;
    sal_uInt16          mnMenuBarMode;
    sal_uInt16          mnIcon;
    ImplData*       mpImplData;

public:
    using Window::ImplIsInTaskPaneList;
    SAL_DLLPRIVATE sal_Bool ImplIsInTaskPaneList( Window* pWin );

private:
    // Default construction is forbidden and not implemented.
    SystemWindow();

    // Copy assignment is forbidden and not implemented.
    SystemWindow (const SystemWindow &);
    SystemWindow & operator= (const SystemWindow &);

    SAL_DLLPRIVATE void ImplMoveToScreen( long& io_rX, long& io_rY, long i_nWidth, long i_nHeight, Window* i_pConfigureWin );

protected:
    // Single argument ctors shall be explicit.
    explicit        SystemWindow( WindowType nType );

    void            SetWindowStateData( const WindowStateData& rData );

public:
                    ~SystemWindow();
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

    virtual sal_Bool    Close();
    virtual void    TitleButtonClick( sal_uInt16 nButton );
    virtual void    Pin();
    virtual void    Roll();
    virtual void    Resizing( Size& rSize );

    void            SetIcon( sal_uInt16 nIcon );
    sal_uInt16          GetIcon() const { return mnIcon; }
    // for systems like MacOSX which can display the URL a document is loaded from
    // separately from the window title
    void            SetRepresentedURL( const OUString& );

    void            EnableSaveBackground( sal_Bool bSave = sal_True );
    sal_Bool            IsSaveBackgroundEnabled() const;

    void            ShowTitleButton( sal_uInt16 nButton, sal_Bool bVisible = sal_True );
    sal_Bool            IsTitleButtonVisible( sal_uInt16 nButton ) const;

    void            SetPin( sal_Bool bPin );
    sal_Bool            IsPined() const { return mbPined; }

    void            RollUp();
    void            RollDown();
    sal_Bool            IsRollUp() const { return mbRollUp; }

    void            SetRollUpOutputSizePixel( const Size& rSize ) { maRollUpOutSize = rSize; }
    Size            GetRollUpOutputSizePixel() const { return maRollUpOutSize; }

    void            SetMinOutputSizePixel( const Size& rSize );
    const Size&     GetMinOutputSizePixel() const { return maMinOutSize; }
    void            SetMaxOutputSizePixel( const Size& rSize );
    const Size&     GetMaxOutputSizePixel() const;
    Size            GetResizeOutputSizePixel() const;

    void            SetWindowState(const OString& rStr);
    OString    GetWindowState(sal_uLong nMask = WINDOWSTATE_MASK_ALL) const;

    void            SetMenuBar( MenuBar* pMenuBar );
    MenuBar*        GetMenuBar() const { return mpMenuBar; }
    void            SetMenuBarMode( sal_uInt16 nMode );
    sal_uInt16          GetMenuBarMode() const { return mnMenuBarMode; }

    TaskPaneList*   GetTaskPaneList();
    void            GetWindowStateData( WindowStateData& rData ) const;

    /**
    Returns the screen number the window is on

    The Display Screen number is counted the same way that
    <code>Application::GetScreenPosSizePixel</code>,
    <code>Application::GetDisplayScreenName</code>
    and of course <code>SystemWindow::SetScreenNumber</code>
    are counted in.

    In case the window is positioned on multiple screens the
    screen number returned will be of the screen containing the
    upper left pixel of the frame area (that is of the client
    area on system decorated windows, or the frame area of
    undecorated resp. owner decorated windows.

    @returns the screen number

    @see SystemWindow::SetScreenNumber
    */
    unsigned int    GetScreenNumber() const;
    /**
    Move the Window to a new screen. The same rules for
    positioning apply as in <code>SystemWindow::GetScreenNumber</code>

    The Display Screen number is counted the same way that
    <code>Application::GetScreenPosSizePixel</code>,
    <code>Application::GetDisplayScreenName</code>
    and of course <code>SystemWindow::GetScreenNumber</code>
    are counted in.

    @see GetScreenNumber
    */
    void            SetScreenNumber( unsigned int nNewScreen );

    void            SetApplicationID( const OUString &rApplicationID );

    void SetCloseHdl(const Link& rLink);
    const Link& GetCloseHdl() const;
};

#endif // _SV_SYSWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
