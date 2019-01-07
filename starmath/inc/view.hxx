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
#ifndef INCLUDED_STARMATH_INC_VIEW_HXX
#define INCLUDED_STARMATH_INC_VIEW_HXX

#include <sal/config.h>

#include <memory>

#include <rtl/ref.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/viewsh.hxx>
#include <svtools/scrwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/timer.hxx>
#include "document.hxx"
#include "edit.hxx"

class SmViewShell;
class SmPrintUIOptions;
class SmGraphicAccessible;
class SmNode;

namespace svtools { class ColorConfig; }

class SmGraphicWindow final : public ScrollableWindow
{
public:
    bool IsCursorVisible() const
    {
        return bIsCursorVisible;
    }
    void ShowCursor(bool bShow);
    bool IsLineVisible() const
    {
        return bIsLineVisible;
    }
    void ShowLine(bool bShow);
    const SmNode * SetCursorPos(sal_uInt16 nRow, sal_uInt16 nCol);

    explicit SmGraphicWindow(SmViewShell* pShell);
    virtual ~SmGraphicWindow() override;
    virtual void dispose() override;

    // Window
    virtual void MouseButtonDown(const MouseEvent &rMEvt) override;
    virtual void MouseMove(const MouseEvent &rMEvt) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;

    SmViewShell* GetView()
    {
        return pViewShell;
    }

    using Window::SetZoom;
    void SetZoom(sal_uInt16 Factor);
    using Window::GetZoom;
    sal_uInt16 GetZoom() const
    {
        return nZoom;
    }

    const Point& GetFormulaDrawPos() const
    {
        return aFormulaDrawPos;
    }

    void ZoomToFitInWindow();
    using ScrollableWindow::SetTotalSize;
    void SetTotalSize();

    void ApplyColorConfigValues(const svtools::ColorConfig &rColorCfg);

    // for Accessibility
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

    using Window::GetAccessible;
    SmGraphicAccessible* GetAccessible_Impl()
    {
        return mxAccessible.get();
    }

private:
    void SetIsCursorVisible(bool bVis)
    {
        bIsCursorVisible = bVis;
    }
    using Window::SetCursor;
    void SetCursor(const SmNode *pNode);
    void SetCursor(const tools::Rectangle &rRect);
    bool IsInlineEditEnabled() const;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual void KeyInput(const KeyEvent& rKEvt) override;
    virtual void Command(const CommandEvent& rCEvt) override;
    virtual void StateChanged( StateChangedType eChanged ) override;

    void RepaintViewShellDoc();
    DECL_LINK(CaretBlinkTimerHdl, Timer *, void);
    void CaretBlinkInit();
    void CaretBlinkStart();
    void CaretBlinkStop();

    Point aFormulaDrawPos;
    // old style editing pieces
    tools::Rectangle aCursorRect;
    bool bIsCursorVisible;
    bool bIsLineVisible;
    AutoTimer aCaretBlinkTimer;
    rtl::Reference<SmGraphicAccessible> mxAccessible;
    SmViewShell* pViewShell;
    sal_uInt16 nZoom;
};

class SmGraphicController final : public SfxControllerItem
{
    SmGraphicWindow &rGraphic;
public:
    SmGraphicController(SmGraphicWindow &, sal_uInt16, SfxBindings & );
    virtual void StateChanged(sal_uInt16             nSID,
                              SfxItemState       eState,
                              const SfxPoolItem* pState) override;
};

class SmEditController final : public SfxControllerItem
{
    SmEditWindow &rEdit;

public:
    SmEditController(SmEditWindow &, sal_uInt16, SfxBindings  & );

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState) override;
};

class SmCmdBoxWindow : public SfxDockingWindow
{
    VclPtr<SmEditWindow>        aEdit;
    SmEditController    aController;
    bool                bExiting;

    Timer               aInitialFocusTimer;

    DECL_LINK(InitialFocusTimerHdl, Timer *, void);

protected:

    // Window
    virtual void    GetFocus() override;
    virtual void Resize() override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void StateChanged( StateChangedType nStateChange ) override;

    virtual Size CalcDockingSize(SfxChildAlignment eAlign) override;
    virtual SfxChildAlignment CheckAlignment(SfxChildAlignment eActual,
                                             SfxChildAlignment eWish) override;

    virtual void    ToggleFloatingMode() override;

public:
    SmCmdBoxWindow(SfxBindings    *pBindings,
                   SfxChildWindow *pChildWindow,
                   Window         *pParent);

    virtual ~SmCmdBoxWindow () override;
    virtual void dispose() override;

    void AdjustPosition();

    SmEditWindow& GetEditWindow()
    {
        return *aEdit;
    }
    SmViewShell* GetView();
};

class SmCmdBoxWrapper final : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW_WITHID(SmCmdBoxWrapper);

    SmCmdBoxWrapper(vcl::Window* pParentWindow, sal_uInt16 nId, SfxBindings* pBindings, SfxChildWinInfo* pInfo);

public:

    SmEditWindow& GetEditWindow()
    {
        return static_cast<SmCmdBoxWindow *>(GetWindow())->GetEditWindow();
    }
};

namespace sfx2 { class FileDialogHelper; }
struct SmViewShell_Impl;

class SmViewShell: public SfxViewShell
{
    std::unique_ptr<SmViewShell_Impl> mpImpl;

    VclPtr<SmGraphicWindow> mpGraphic;
    SmGraphicController const maGraphicController;
    OUString maStatusText;

    bool mbPasteState;

    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper*, void );
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    /** Used to determine whether insertions using SID_INSERTSPECIAL and SID_INSERTCOMMANDTEXT
     * should be inserted into SmEditWindow or directly into the SmDocShell as done if the
     * visual editor was last to have focus.
     */
    bool mbInsertIntoEditWindow;
protected:

    static Size GetTextLineSize(OutputDevice const & rDevice,
                         const OUString& rLine);
    static Size GetTextSize(OutputDevice const & rDevice,
                     const OUString& rText,
                     long          MaxWidth);
    static void DrawTextLine(OutputDevice& rDevice,
                      const Point&  rPosition,
                      const OUString& rLine);
    static void DrawText(OutputDevice& rDevice,
                  const Point&  rPosition,
                  const OUString& rText,
                  sal_uInt16        MaxWidth);

    virtual SfxPrinter *GetPrinter(bool bCreate = false) override;
    virtual sal_uInt16 SetPrinter(SfxPrinter *pNewPrinter,
                              SfxPrinterChangeFlags nDiffFlags = SFX_PRINTER_ALL) override;

    void Insert( SfxMedium& rMedium );
    void InsertFrom(SfxMedium &rMedium);

    virtual bool HasPrintOptionsPage() const override;
    virtual VclPtr<SfxTabPage> CreatePrintOptionsPage(TabPageParent pParent,
                                                      const SfxItemSet &rOptions) override;
    virtual void Deactivate(bool IsMDIActivate) override;
    virtual void Activate(bool IsMDIActivate) override;
    virtual void InnerResizePixel(const Point &rOfs, const Size  &rSize, bool inplaceEditModeChange) override;
    virtual void OuterResizePixel(const Point &rOfs, const Size  &rSize) override;
    virtual void QueryObjAreaPixel( tools::Rectangle& rRect ) const override;
    virtual void SetZoomFactor( const Fraction &rX, const Fraction &rY ) override;

public:

    SmViewShell(SfxViewFrame *pFrame, SfxViewShell *pOldSh);
    virtual ~SmViewShell() override;

    SmDocShell * GetDoc()
    {
        return static_cast<SmDocShell *>( GetViewFrame()->GetObjectShell() );
    }

    SmEditWindow * GetEditWindow();

    SmGraphicWindow& GetGraphicWindow()
    {
        return *mpGraphic;
    }
    const SmGraphicWindow& GetGraphicWindow() const
    {
        return *mpGraphic;
    }

    void        SetStatusText(const OUString& rText);

    void        ShowError( const SmErrorDesc *pErrorDesc );
    void        NextError();
    void        PrevError();

    SFX_DECL_INTERFACE(SFX_INTERFACE_SMA_START+SfxInterfaceId(2))
    SFX_DECL_VIEWFACTORY(SmViewShell);

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    void Execute( SfxRequest& rReq );
    void GetState(SfxItemSet &);

    void Impl_Print( OutputDevice &rOutDev, const SmPrintUIOptions &rPrintUIOptions,
            tools::Rectangle aOutRect );

    /** Set bInsertIntoEditWindow so we know where to insert
     *
     * This method is called whenever SmGraphicWindow or SmEditWindow gets focus,
     * so that when text is inserted from catalog or elsewhere we know whether to
     * insert for the visual editor, or the text editor.
     */
    void SetInsertIntoEditWindow(bool bEditWindowHadFocusLast){
        mbInsertIntoEditWindow = bEditWindowHadFocusLast;
    }
    bool IsInlineEditEnabled() const;

private:
    void ZoomByItemSet(const SfxItemSet *pSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
