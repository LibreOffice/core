/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: syswin.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _SV_SYSWIN_HXX
#define _SV_SYSWIN_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/window.hxx>

class ModalDialog;
class MenuBar;
class TaskPaneList;

// --------------
// - Icon-Types -
// --------------

#define ICON_DEFAULT                    0
#define ICON_SO_DEFAULT                 1
#define ICON_TEXT_DOCUMENT              2
#define ICON_TEXT_TEMPLATE              3
#define ICON_SPREADSHEET_DOCUMENT       4
#define ICON_SPREADSHEET_TEMPLATE       5
#define ICON_DRAWING_DOCUMENT           6
#define ICON_DRAWING_TEMPLATE           7
#define ICON_PRESENTATION_DOCUMENT      8
#define ICON_PRESENTATION_TEMPLATE      9
#define ICON_PRESENTATION_COMPRESSED    10
#define ICON_GLOBAL_DOCUMENT            11
#define ICON_HTML_DOCUMENT              12
#define ICON_CHART_DOCUMENT             13
#define ICON_DATABASE_DOCUMENT          14
#define ICON_MATH_DOCUMENT              15
#define ICON_TEMPLATE                   16
#define ICON_MACROLIBRARY               17
#define ICON_PLAYER                     100
#define ICON_SETUP                      500

// -------------------
// - WindowStateData -
// -------------------

#define WINDOWSTATE_MASK_X                  ((ULONG)0x00000001)
#define WINDOWSTATE_MASK_Y                  ((ULONG)0x00000002)
#define WINDOWSTATE_MASK_WIDTH              ((ULONG)0x00000004)
#define WINDOWSTATE_MASK_HEIGHT             ((ULONG)0x00000008)
#define WINDOWSTATE_MASK_STATE              ((ULONG)0x00000010)
#define WINDOWSTATE_MASK_MINIMIZED          ((ULONG)0x00000020)
#define WINDOWSTATE_MASK_MAXIMIZED_X        ((ULONG)0x00000100)
#define WINDOWSTATE_MASK_MAXIMIZED_Y        ((ULONG)0x00000200)
#define WINDOWSTATE_MASK_MAXIMIZED_WIDTH    ((ULONG)0x00000400)
#define WINDOWSTATE_MASK_MAXIMIZED_HEIGHT   ((ULONG)0x00000800)
#define WINDOWSTATE_MASK_POS  (WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y)
#define WINDOWSTATE_MASK_ALL  (WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y | WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT | WINDOWSTATE_MASK_MAXIMIZED_X | WINDOWSTATE_MASK_MAXIMIZED_Y | WINDOWSTATE_MASK_MAXIMIZED_WIDTH | WINDOWSTATE_MASK_MAXIMIZED_HEIGHT | WINDOWSTATE_MASK_STATE | WINDOWSTATE_MASK_MINIMIZED)

#define WINDOWSTATE_STATE_NORMAL         ((ULONG)0x00000001)
#define WINDOWSTATE_STATE_MINIMIZED      ((ULONG)0x00000002)
#define WINDOWSTATE_STATE_MAXIMIZED      ((ULONG)0x00000004)
#define WINDOWSTATE_STATE_ROLLUP         ((ULONG)0x00000008)
#define WINDOWSTATE_STATE_MAXIMIZED_HORZ ((ULONG)0x00000010)
#define WINDOWSTATE_STATE_MAXIMIZED_VERT ((ULONG)0x00000020)

class VCL_DLLPUBLIC WindowStateData
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

    void        SetMask( ULONG nValidMask ) { mnValidMask = nValidMask; }
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

#define MENUBAR_MODE_NORMAL         ((USHORT)0)
#define MENUBAR_MODE_HIDE           ((USHORT)1)

#define TITLE_BUTTON_DOCKING        ((USHORT)1)
#define TITLE_BUTTON_HIDE           ((USHORT)2)
#define TITLE_BUTTON_MENU           ((USHORT)4)

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
    BOOL            mbPined;
    BOOL            mbRollUp;
    BOOL            mbRollFunc;
    BOOL            mbDockBtn;
    BOOL            mbHideBtn;
    BOOL            mbSysChild;
    USHORT          mnMenuBarMode;
    USHORT          mnIcon;
    ImplData*       mpImplData;

#if _SOLAR__PRIVATE
public:
    using Window::ImplIsInTaskPaneList;
    SAL_DLLPRIVATE BOOL ImplIsInTaskPaneList( Window* pWin );
#endif

private:
    // Default construction is forbidden and not implemented.
    SystemWindow();

    // Copy assignment is forbidden and not implemented.
    SystemWindow (const SystemWindow &);
    SystemWindow & operator= (const SystemWindow &);

protected:
    // Single argument ctors shall be explicit.
    explicit        SystemWindow( WindowType nType );

    void            SetWindowStateData( const WindowStateData& rData );

public:
                    ~SystemWindow();
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

    virtual BOOL    Close();
    virtual void    TitleButtonClick( USHORT nButton );
    virtual void    Pin();
    virtual void    Roll();
    virtual void    Resizing( Size& rSize );

    void            SetIcon( USHORT nIcon );
    USHORT          GetIcon() const { return mnIcon; }

    void            SetZLevel( BYTE nLevel );
    BYTE            GetZLevel() const;

    void            EnableSaveBackground( BOOL bSave = TRUE );
    BOOL            IsSaveBackgroundEnabled() const;

    void            ShowTitleButton( USHORT nButton, BOOL bVisible = TRUE );
    BOOL            IsTitleButtonVisible( USHORT nButton ) const;

    void            SetPin( BOOL bPin );
    BOOL            IsPined() const { return mbPined; }

    void            RollUp();
    void            RollDown();
    BOOL            IsRollUp() const { return mbRollUp; }

    void            SetRollUpOutputSizePixel( const Size& rSize ) { maRollUpOutSize = rSize; }
    Size            GetRollUpOutputSizePixel() const { return maRollUpOutSize; }

    void            SetMinOutputSizePixel( const Size& rSize );
    const Size&     GetMinOutputSizePixel() const { return maMinOutSize; }
    void            SetMaxOutputSizePixel( const Size& rSize );
    const Size&     GetMaxOutputSizePixel() const;
    Size            GetResizeOutputSizePixel() const;

    void            SetWindowState( const ByteString& rStr );
    ByteString      GetWindowState( ULONG nMask = WINDOWSTATE_MASK_ALL ) const;

    void            SetMenuBar( MenuBar* pMenuBar );
    MenuBar*        GetMenuBar() const { return mpMenuBar; }
    void            SetMenuBarMode( USHORT nMode );
    USHORT          GetMenuBarMode() const { return mnMenuBarMode; }

    TaskPaneList*   GetTaskPaneList();
    void            GetWindowStateData( WindowStateData& rData ) const;

    /**
    Returns the screen number the window is on

    The screen number is counted the same way that
    <code>Application::GetScreenPosSizePixel</code>,
    <code>Application::GetWorkAreaPosSizePixel</code>,
    <code>Application::GetScreenName</code>
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

    The screen number is counted the same way that
    <code>Application::GetScreenPosSizePixel</code>,
    <code>Application::GetWorkAreaPosSizePixel</code>,
    <code>Application::GetScreenName</code>
    and of course <code>SystemWindow::GetScreenNumber</code>
    are counted in.

    @see GetScreenNumber
    */
    void            SetScreenNumber( unsigned int nNewScreen );
};

#endif // _SV_SYSWIN_HXX
