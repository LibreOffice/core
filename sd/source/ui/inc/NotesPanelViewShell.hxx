/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ViewShell.hxx"
#include <glob.hxx>

class SdPage;

namespace sd
{
class NotesPanelView;

class NotesPanelViewShell final : public ViewShell
{
public:
    SFX_DECL_VIEWFACTORY(NotesPanelViewShell);
    SFX_DECL_INTERFACE(SD_IF_SDNOTESPANELVIEWSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    /** Create a new view shell for the notes panel.
        @param rViewShellBase
            The new object will be stacked on this view shell base.
        @param pFrameView
            The frame view that makes it possible to pass information from
            one view shell to the next.
    */
    NotesPanelViewShell(SfxViewFrame* pFrame, ViewShellBase& rViewShellBase,
                        vcl::Window* pParentWindow, FrameView* pFrameView);

    virtual ~NotesPanelViewShell() override;

    virtual void Paint(const ::tools::Rectangle& rRect, ::sd::Window* pWin) override;
    virtual bool PrepareClose(bool bUI = true) override;
    virtual void UpdateScrollBars() override;
    virtual void VirtHScrollHdl(ScrollAdaptor* pHScroll) override;
    virtual void VirtVScrollHdl(ScrollAdaptor* pVScroll) override;
    virtual void Activate(bool IsMDIActivate) override;
    /** this method is called when the visible area of the view from this viewshell is changed */
    virtual void VisAreaChanged(const ::tools::Rectangle& rRect) override;

    virtual void ArrangeGUIElements() override;
    virtual SdPage* GetActualPage() override;
    virtual SdPage* getCurrentPage() const override;
    virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController() override;

    void ExecCtrl(SfxRequest& rReq);
    void GetCtrlState(SfxItemSet& rSet);
    void GetAttrState(SfxItemSet& rSet);
    void GetState(SfxItemSet& rSet);
    void GetCharState(SfxItemSet& rSet);

    static void ExecStatusBar(SfxRequest& rReq);
    void GetStatusBarState(SfxItemSet& rSet);

    void FuTemporary(SfxRequest& rReq);
    void FuTemporaryModify(SfxRequest& rReq);
    void FuPermanent(SfxRequest& rReq);
    void FuSupport(SfxRequest& rReq);
    void Execute(SfxRequest& rReq);
    void ExecChar(SfxRequest& rReq);

    virtual void Command(const CommandEvent& rCEvt, ::sd::Window* pWin) override;
    virtual bool KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin) override;
    virtual void MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin) override;

    virtual void SetZoom(::tools::Long nZoom) override;
    virtual void SetZoomRect(const ::tools::Rectangle& rZoomRect) override;

    virtual void ReadFrameViewData(FrameView* pView) override;
    virtual void WriteFrameViewData() override;
    virtual css::uno::Reference<css::accessibility::XAccessible>
    CreateAccessibleDocumentView(::sd::Window* /*pWindow*/) override
    {
        // TODO
        return {};
    }

private:
    std::unique_ptr<NotesPanelView> mpNotesPanelView;
    bool mbInitialized = false;

    /** Initiates the shell with it's NotesPanelView instance
    */
    void Construct();
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
