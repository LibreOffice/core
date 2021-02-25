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

#include <vcl/dllapi.h>
#include <vcl/idle.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/window.hxx>
#include <vcl/windowstate.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <memory>

class SalInstanceBuilder;
class ScreenshotTest;
class MenuBar;
class MnemonicGenerator;
class NotebookBar;
class TaskPaneList;
class VclBuilder;
struct NotebookBarAddonsItem;

#define ICON_LO_DEFAULT                 1
#define ICON_TEXT_DOCUMENT              2
#define ICON_SPREADSHEET_DOCUMENT       4
#define ICON_DRAWING_DOCUMENT           6
#define ICON_PRESENTATION_DOCUMENT      8
#define ICON_TEMPLATE                   11
#define ICON_DATABASE_DOCUMENT          12
#define ICON_MATH_DOCUMENT              13
#define ICON_MACROLIBRARY               1

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

//helper baseclass to ease retro fitting dialogs/tabpages that load a resource
//to load a .ui file instead
//
//vcl requires the Window Children of a Parent Window to be destroyed before
//the Parent Window.  VclBuilderContainer owns the VclBuilder which owns the
//Children Window. So the VclBuilderContainer dtor must be called before
//the Parent Window dtor.
//
//i.e.  class Dialog : public SystemWindow, public VclBuilderContainer
//not   class Dialog : public VclBuilderContainer, public SystemWindow
//
//With the new 'dispose' framework, it is necessary to force the builder
//dispose before the Window dispose; so a Dialog::dispose() method would
//finish: disposeBuilder(); SystemWindow::dispose() to capture this ordering.

class VCL_DLLPUBLIC VclBuilderContainer
{
public:
                    VclBuilderContainer();
    virtual         ~VclBuilderContainer();
    void            disposeBuilder();

    void setDeferredProperties();

protected:
    std::unique_ptr<VclBuilder> m_pUIBuilder;

    friend class ::SalInstanceBuilder;
    friend class ::ScreenshotTest;
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
    Size            maMinOutSize;
    bool            mbDockBtn;
    bool            mbHideBtn;
    bool            mbSysChild;
    bool            mbIsCalculatingInitialLayoutSize;
    bool            mbPaintComplete;
    MenuBarMode     mnMenuBarMode;
    sal_uInt16      mnIcon;
    std::unique_ptr<ImplData> mpImplData;
    Idle            maLayoutIdle;
    OUString        maNotebookBarUIFile;
protected:
    bool            mbIsDeferredInit;
    VclPtr<vcl::Window> mpDialogParent;
public:
    using Window::ImplIsInTaskPaneList;
    SAL_DLLPRIVATE bool ImplIsInTaskPaneList( vcl::Window* pWin );
    SAL_DLLPRIVATE bool isDeferredInit() const { return mbIsDeferredInit; }

private:
    SAL_DLLPRIVATE void ImplMoveToScreen( tools::Long& io_rX, tools::Long& io_rY, tools::Long i_nWidth, tools::Long i_nHeight, vcl::Window const * i_pConfigureWin );
    SAL_DLLPRIVATE void setPosSizeOnContainee(Size aSize, Window &rBox);
    DECL_DLLPRIVATE_LINK( ImplHandleLayoutTimerHdl, Timer*, void );

    // try to extract content and return as Bitmap. To do that reliably, a Yield-loop
    // like in Execute() has to be executed and it is necessary to detect when the
    // paint is finished
    virtual void PrePaint(vcl::RenderContext& rRenderContext) override;
    virtual void PostPaint(vcl::RenderContext& rRenderContext) override;

    // ensureRepaint - triggers Application::Yield until the dialog is
    // completely repainted. Sometimes needed for dialogs showing progress
    // during actions
    SAL_DLLPRIVATE void ensureRepaint();

protected:
    // Single argument ctors shall be explicit.
    explicit SystemWindow(WindowType nType);
    void loadUI(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());

    void     SetWindowStateData( const WindowStateData& rData );

    virtual void settingOptimalLayoutSize(Window *pBox);

    SAL_DLLPRIVATE void DoInitialLayout();

    SAL_DLLPRIVATE void SetIdleDebugName( const char *pDebugName );

public:
    virtual         ~SystemWindow() override;
    virtual void    dispose() override;

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
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

    void            SetMinOutputSizePixel( const Size& rSize );
    const Size&     GetMinOutputSizePixel() const { return maMinOutSize; }
    void            SetMaxOutputSizePixel( const Size& rSize );
    const Size&     GetMaxOutputSizePixel() const;

    void            SetWindowState(const OString& rStr);
    OString         GetWindowState(WindowStateMask nMask = WindowStateMask::All) const;

    void            SetMenuBar(MenuBar* pMenuBar);
    MenuBar*        GetMenuBar() const { return mpMenuBar; }
    void            SetMenuBarMode( MenuBarMode nMode );
    void            CollectMenuBarMnemonics(MnemonicGenerator& rMnemonicGenerator) const;
    int             GetMenuBarHeight() const;

    void SetNotebookBar(const OUString& rUIXMLDescription,
                        const css::uno::Reference<css::frame::XFrame>& rFrame,
                        const NotebookBarAddonsItem& aNotebookBarAddonsItem,
                        bool bReloadNotebookbar = false);

    void            CloseNotebookBar();
    VclPtr<NotebookBar> const & GetNotebookBar() const;

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

    // Screenshot interface
    VclPtr<VirtualDevice> createScreenshot();
};

inline void SystemWindow::SetIdleDebugName( const char *pDebugName )
{
    maLayoutIdle.SetDebugName( pDebugName );
}

#endif // INCLUDED_VCL_SYSWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
