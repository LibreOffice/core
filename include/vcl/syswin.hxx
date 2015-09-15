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

#ifndef INCLUDED_VCL_SYSWIN_HXX
#define INCLUDED_VCL_SYSWIN_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/builder.hxx>
#include <vcl/window.hxx>
#include <vcl/idle.hxx>

class ModalDialog;
class MenuBar;
class TaskPaneList;
class VclContainer;

// - Icon-Types -
#define ICON_LO_DEFAULT                 1
#define ICON_TEXT_DOCUMENT              2
#define ICON_SPREADSHEET_DOCUMENT       4
#define ICON_DRAWING_DOCUMENT           6
#define ICON_PRESENTATION_DOCUMENT      8
#define ICON_TEMPLATE                   11
#define ICON_DATABASE_DOCUMENT          12
#define ICON_MATH_DOCUMENT              13
#define ICON_MACROLIBRARY               1


// - WindowStateData -


#define WINDOWSTATE_MASK_X                  ((sal_uInt32)0x00000001)
#define WINDOWSTATE_MASK_Y                  ((sal_uInt32)0x00000002)
#define WINDOWSTATE_MASK_WIDTH              ((sal_uInt32)0x00000004)
#define WINDOWSTATE_MASK_HEIGHT             ((sal_uInt32)0x00000008)
#define WINDOWSTATE_MASK_STATE              ((sal_uInt32)0x00000010)
#define WINDOWSTATE_MASK_MINIMIZED          ((sal_uInt32)0x00000020)
#define WINDOWSTATE_MASK_MAXIMIZED_X        ((sal_uInt32)0x00000100)
#define WINDOWSTATE_MASK_MAXIMIZED_Y        ((sal_uInt32)0x00000200)
#define WINDOWSTATE_MASK_MAXIMIZED_WIDTH    ((sal_uInt32)0x00000400)
#define WINDOWSTATE_MASK_MAXIMIZED_HEIGHT   ((sal_uInt32)0x00000800)
#define WINDOWSTATE_MASK_POS  (WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y)
#define WINDOWSTATE_MASK_ALL  (WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y | WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT | WINDOWSTATE_MASK_MAXIMIZED_X | WINDOWSTATE_MASK_MAXIMIZED_Y | WINDOWSTATE_MASK_MAXIMIZED_WIDTH | WINDOWSTATE_MASK_MAXIMIZED_HEIGHT | WINDOWSTATE_MASK_STATE | WINDOWSTATE_MASK_MINIMIZED)

#define WINDOWSTATE_STATE_NORMAL         ((sal_uInt32)0x00000001)
#define WINDOWSTATE_STATE_MINIMIZED      ((sal_uInt32)0x00000002)
#define WINDOWSTATE_STATE_MAXIMIZED      ((sal_uInt32)0x00000004)
#define WINDOWSTATE_STATE_ROLLUP         ((sal_uInt32)0x00000008)
#define WINDOWSTATE_STATE_MAXIMIZED_HORZ ((sal_uInt32)0x00000010)
#define WINDOWSTATE_STATE_MAXIMIZED_VERT ((sal_uInt32)0x00000020)
#define WINDOWSTATE_STATE_SYSTEMMASK     ((sal_uInt32)0x0000FFFF)

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
        : mnValidMask(0)
        , mnX(0)
        , mnY(0)
        , mnWidth(0)
        , mnHeight(0)
        , mnMaximizedX(0)
        , mnMaximizedY(0)
        , mnMaximizedWidth(0)
        , mnMaximizedHeight(0)
        , mnState(0)
    {
    }

    void        SetMask( sal_uInt32 nValidMask ) { mnValidMask = nValidMask; }
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


// - SystemWindow-Types -


enum class MenuBarMode
{
    Normal, Hide
};

enum class TitleButton
{
    Docking        = 1,
    Hide           = 2,
    Menu           = 4,
};

// - SystemWindow -
class VCL_DLLPUBLIC SystemWindow
    : public vcl::Window
    , public VclBuilderContainer
{
    friend class WorkWindow;
    class ImplData;

private:
    MenuBar*        mpMenuBar;
    Size            maOrgSize;
    Size            maRollUpOutSize;
    Size            maMinOutSize;
    bool            mbPinned;
    bool            mbRollUp;
    bool            mbRollFunc;
    bool            mbDockBtn;
    bool            mbHideBtn;
    bool            mbSysChild;
    bool            mbIsCalculatingInitialLayoutSize;
    bool            mbInitialLayoutDone;
    MenuBarMode     mnMenuBarMode;
    sal_uInt16      mnIcon;
    ImplData*       mpImplData;
    Idle            maLayoutIdle;
protected:
    bool            mbIsDefferedInit;
    VclPtr<vcl::Window> mpDialogParent;
public:
    using Window::ImplIsInTaskPaneList;
    SAL_DLLPRIVATE bool ImplIsInTaskPaneList( vcl::Window* pWin );
    SAL_DLLPRIVATE bool isDeferredInit() const { return mbIsDefferedInit; }

private:
    SAL_DLLPRIVATE void Init();
    SAL_DLLPRIVATE void ImplMoveToScreen( long& io_rX, long& io_rY, long i_nWidth, long i_nHeight, vcl::Window* i_pConfigureWin );
    virtual void setPosSizeOnContainee(Size aSize, Window &rBox);
    DECL_DLLPRIVATE_LINK_TYPED( ImplHandleLayoutTimerHdl, Idle*, void );

protected:
    // Single argument ctors shall be explicit.
    explicit SystemWindow(WindowType nType);
    void loadUI(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());

    void     SetWindowStateData( const WindowStateData& rData );

    virtual void settingOptimalLayoutSize(Window *pBox);

    SAL_DLLPRIVATE void DoInitialLayout();
public:
    virtual         ~SystemWindow();
    virtual void    dispose() SAL_OVERRIDE;

    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    virtual bool    Close();
    virtual void    TitleButtonClick( TitleButton nButton );
    virtual void    Pin();
    virtual void    Roll();
    virtual void    Resizing( Size& rSize );
    virtual void    Resize() SAL_OVERRIDE;
    virtual Size    GetOptimalSize() const SAL_OVERRIDE;
    virtual void    queue_resize(StateChangedType eReason = StateChangedType::Layout) SAL_OVERRIDE;
    bool            isLayoutEnabled() const;
    void            setOptimalLayoutSize();
    bool            isCalculatingInitialLayoutSize() const { return mbIsCalculatingInitialLayoutSize; }

    void            SetIcon( sal_uInt16 nIcon );
    sal_uInt16          GetIcon() const { return mnIcon; }
    // for systems like MacOSX which can display the URL a document is loaded from
    // separately from the window title
    void            SetRepresentedURL( const OUString& );

    void            EnableSaveBackground( bool bSave = true );
    bool            IsSaveBackgroundEnabled() const;

    void            ShowTitleButton( TitleButton nButton, bool bVisible = true );
    bool            IsTitleButtonVisible( TitleButton nButton ) const;

    void            SetPin( bool bPin );
    bool            IsPinned() const { return mbPinned; }

    void            RollUp();
    void            RollDown();
    bool            IsRollUp() const { return mbRollUp; }

    void            SetRollUpOutputSizePixel( const Size& rSize ) { maRollUpOutSize = rSize; }
    Size            GetRollUpOutputSizePixel() const { return maRollUpOutSize; }

    void            SetMinOutputSizePixel( const Size& rSize );
    const Size&     GetMinOutputSizePixel() const { return maMinOutSize; }
    void            SetMaxOutputSizePixel( const Size& rSize );
    const Size&     GetMaxOutputSizePixel() const;

    void            SetWindowState(const OString& rStr);
    OString         GetWindowState(sal_uInt32 nMask = WINDOWSTATE_MASK_ALL) const;

    void            SetMenuBar(MenuBar* pMenuBar, const css::uno::Reference<css::frame::XFrame>& rFrame = css::uno::Reference<css::frame::XFrame>());
    MenuBar*        GetMenuBar() const { return mpMenuBar; }
    void            SetMenuBarMode( MenuBarMode nMode );

    TaskPaneList*   GetTaskPaneList();
    void            GetWindowStateData( WindowStateData& rData ) const;

    virtual void     SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString GetText() const SAL_OVERRIDE;

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

    void SetCloseHdl(const Link<SystemWindow&,void>& rLink);
    const Link<SystemWindow&,void>& GetCloseHdl() const;

    SAL_DLLPRIVATE bool hasPendingLayout() const { return maLayoutIdle.IsActive(); }

    virtual        void    doDeferredInit(WinBits nBits);
};

#endif // INCLUDED_VCL_SYSWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
