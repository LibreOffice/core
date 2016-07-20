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
#include <vcl/idle.hxx>
#include <vcl/notebookbar.hxx>
#include <vcl/window.hxx>
#include <o3tl/typed_flags_set.hxx>

class ModalDialog;
class MenuBar;
class TaskPaneList;
class VclContainer;

#define ICON_LO_DEFAULT                 1
#define ICON_TEXT_DOCUMENT              2
#define ICON_SPREADSHEET_DOCUMENT       4
#define ICON_DRAWING_DOCUMENT           6
#define ICON_PRESENTATION_DOCUMENT      8
#define ICON_TEMPLATE                   11
#define ICON_DATABASE_DOCUMENT          12
#define ICON_MATH_DOCUMENT              13
#define ICON_MACROLIBRARY               1

enum class WindowStateMask {
    NONE             = 0x0000,
    X                = 0x0001,
    Y                = 0x0002,
    Width            = 0x0004,
    Height           = 0x0008,
    State            = 0x0010,
    Minimized        = 0x0020,
    MaximizedX       = 0x0100,
    MaximizedY       = 0x0200,
    MaximizedWidth   = 0x0400,
    MaximizedHeight  = 0x0800,
    Pos              = X | Y,
    All              = X | Y | Width | Height | MaximizedX | MaximizedY | MaximizedWidth | MaximizedHeight | State | Minimized
};
namespace o3tl
{
    template<> struct typed_flags<WindowStateMask> : is_typed_flags<WindowStateMask, 0x0f3f> {};
}

enum class WindowStateState {
    NONE           = 0x0000,
    Normal         = 0x0001,
    Minimized      = 0x0002,
    Maximized      = 0x0004,
    Rollup         = 0x0008,
    MaximizedHorz  = 0x0010,
    MaximizedVert  = 0x0020,
    SystemMask     = 0xffff
};
namespace o3tl
{
    template<> struct typed_flags<WindowStateState> : is_typed_flags<WindowStateState, 0xffff> {};
}

class VCL_PLUGIN_PUBLIC WindowStateData
{
private:
    WindowStateMask     mnValidMask;
    int                 mnX;
    int                 mnY;
    unsigned int        mnWidth;
    unsigned int        mnHeight;
    int                 mnMaximizedX;
    int                 mnMaximizedY;
    unsigned int        mnMaximizedWidth;
    unsigned int        mnMaximizedHeight;
    WindowStateState    mnState;

public:
    WindowStateData()
        : mnValidMask(WindowStateMask::NONE)
        , mnX(0)
        , mnY(0)
        , mnWidth(0)
        , mnHeight(0)
        , mnMaximizedX(0)
        , mnMaximizedY(0)
        , mnMaximizedWidth(0)
        , mnMaximizedHeight(0)
        , mnState(WindowStateState::NONE)
    {
    }

    void        SetMask( WindowStateMask nValidMask ) { mnValidMask = nValidMask; }
    WindowStateMask GetMask() const { return mnValidMask; }

    void         SetX( int nX ) { mnX = nX; }
    int          GetX() const { return mnX; }
    void         SetY( int nY ) { mnY = nY; }
    int          GetY() const { return mnY; }
    void         SetWidth( unsigned int nWidth ) { mnWidth = nWidth; }
    unsigned int GetWidth() const { return mnWidth; }
    void         SetHeight( unsigned int nHeight ) { mnHeight = nHeight; }
    unsigned int GetHeight() const { return mnHeight; }
    void         SetState( WindowStateState nState ) { mnState = nState; }
    WindowStateState GetState() const { return mnState; }
    void         SetMaximizedX( int nRX ) { mnMaximizedX = nRX; }
    int          GetMaximizedX() const { return mnMaximizedX; }
    void         SetMaximizedY( int nRY ) { mnMaximizedY = nRY; }
    int          GetMaximizedY() const { return mnMaximizedY; }
    void         SetMaximizedWidth( unsigned int nRWidth ) { mnMaximizedWidth = nRWidth; }
    unsigned int GetMaximizedWidth() const { return mnMaximizedWidth; }
    void         SetMaximizedHeight( unsigned int nRHeight ) { mnMaximizedHeight = nRHeight; }
    unsigned int GetMaximizedHeight() const { return mnMaximizedHeight; }
};


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

class VCL_DLLPUBLIC SystemWindow
    : public vcl::Window
    , public VclBuilderContainer
{
    friend class WorkWindow;
    class ImplData;

private:
    VclPtr<MenuBar> mpMenuBar;
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
    MenuBarMode     mnMenuBarMode;
    sal_uInt16      mnIcon;
    ImplData*       mpImplData;
    Idle            maLayoutIdle;
    OUString        maNotebookBarUIFile;
protected:
    bool            mbIsDefferedInit;
    VclPtr<vcl::Window> mpDialogParent;
public:
    using Window::ImplIsInTaskPaneList;
    SAL_DLLPRIVATE bool ImplIsInTaskPaneList( vcl::Window* pWin );
    SAL_DLLPRIVATE bool isDeferredInit() const { return mbIsDefferedInit; }

private:
    SAL_DLLPRIVATE void ImplMoveToScreen( long& io_rX, long& io_rY, long i_nWidth, long i_nHeight, vcl::Window* i_pConfigureWin );
    SAL_DLLPRIVATE void setPosSizeOnContainee(Size aSize, Window &rBox);
    DECL_DLLPRIVATE_LINK( ImplHandleLayoutTimerHdl, Idle*, void );

protected:
    // Single argument ctors shall be explicit.
    explicit SystemWindow(WindowType nType);
    void loadUI(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());

    void     SetWindowStateData( const WindowStateData& rData );

    virtual void settingOptimalLayoutSize(Window *pBox);

    SAL_DLLPRIVATE void DoInitialLayout();

    SAL_DLLPRIVATE void SetIdleDebugName( const sal_Char *pDebugName );
public:
    virtual         ~SystemWindow() override;
    virtual void    dispose() override;

    virtual bool    Notify( NotifyEvent& rNEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;

    virtual bool    Close();
    virtual void    TitleButtonClick( TitleButton nButton );
    virtual void    Resizing( Size& rSize );
    virtual void    Resize() override;
    virtual Size    GetOptimalSize() const override;
    virtual void    queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
    bool            isLayoutEnabled() const;
    void            setOptimalLayoutSize();
    bool            isCalculatingInitialLayoutSize() const { return mbIsCalculatingInitialLayoutSize; }

    void            SetIcon( sal_uInt16 nIcon );
    sal_uInt16          GetIcon() const { return mnIcon; }
    // for systems like MacOSX which can display the URL a document is loaded from
    // separately from the window title
    void            SetRepresentedURL( const OUString& );

    void            ShowTitleButton( TitleButton nButton, bool bVisible );
    bool            IsTitleButtonVisible( TitleButton nButton ) const;

    void            SetPin( bool bPin );
    bool            IsPinned() const { return mbPinned; }

    void            RollUp();
    void            RollDown();
    bool            IsRollUp() const { return mbRollUp; }

    void            SetRollUpOutputSizePixel( const Size& rSize ) { maRollUpOutSize = rSize; }
    const Size&     GetRollUpOutputSizePixel() const { return maRollUpOutSize; }

    void            SetMinOutputSizePixel( const Size& rSize );
    const Size&     GetMinOutputSizePixel() const { return maMinOutSize; }
    void            SetMaxOutputSizePixel( const Size& rSize );
    const Size&     GetMaxOutputSizePixel() const;

    void            SetWindowState(const OString& rStr);
    OString         GetWindowState(WindowStateMask nMask = WindowStateMask::All) const;

    void            SetMenuBar(MenuBar* pMenuBar);
    MenuBar*        GetMenuBar() const { return mpMenuBar; }
    void            SetMenuBarMode( MenuBarMode nMode );

    void            SetNotebookBar(const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame>& rFrame);
    void            CloseNotebookBar();
    VclPtr<NotebookBar> GetNotebookBar() const;

    TaskPaneList*   GetTaskPaneList();
    void            GetWindowStateData( WindowStateData& rData ) const;

    virtual void     SetText( const OUString& rStr ) override;
    virtual OUString GetText() const override;

    /**
    Returns the screen number the window is on

    The Display Screen number is counted the same way that
    <code>Application::GetScreenPosSizePixel</code>
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
    <code>Application::GetScreenPosSizePixel</code>
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

inline void SystemWindow::SetIdleDebugName( const sal_Char *pDebugName )
{
    maLayoutIdle.SetDebugName( pDebugName );
}

#endif // INCLUDED_VCL_SYSWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
