/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <svx/weldeditview.hxx>
#include <sfx2/dockwin.hxx>

class Outliner;
class OutlinerView;
class SdrTextObj;

namespace sd::tools
{
class EventMultiplexerEvent;
}

namespace sd
{
class ViewShellBase;
class DrawViewShell;
class NotesChildWindow;
class NotesEditWindow;
class NotesChildDockingWindow;

class NotesEditWindow : public WeldEditView, public SfxListener
{
    NotesChildDockingWindow& mrParentWindow;
    std::unique_ptr<weld::ScrolledWindow> m_xScrolledWindow;
    Idle aModifyIdle;

    SdrTextObj* mpTextObj = nullptr;
    bool mbIgnoreNotifications = false;

    void doScroll();
    void setScrollBarRange();
    void showContextMenu(const Point& rPos);

    DECL_LINK(ScrollHdl, weld::ScrolledWindow&, void);
    DECL_LINK(EditStatusHdl, EditStatus&, void);
    DECL_LINK(EditModifiedHdl, LinkParamNone*, void);
    DECL_LINK(ModifyTimerHdl, Timer*, void);
    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent&, void);

    void removeListener();
    void addListener();

    void setListenerIgnored(bool bIgnore);
    bool isListenerIgnored();

    void getNotesFromDoc();
    void setNotesToDoc();

public:
    NotesEditWindow(NotesChildDockingWindow& rParentWindow,
                    std::unique_ptr<weld::ScrolledWindow> pScrolledWindow);
    virtual ~NotesEditWindow() override;

    void provideNoteText();
    bool HasNotesPlaceholder() { return mpTextObj; }

    virtual void EditViewScrollStateChange() override;
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual EditView* GetEditView() const override;
    virtual EditEngine* GetEditEngine() const override;
    virtual void GetFocus() override;
    virtual bool CanFocus() const override;
    virtual void LoseFocus() override;
    virtual bool Command(const CommandEvent& rCEvt) override;
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;
};

class NotesChildDockingWindow final : public SfxDockingWindow
{
    ViewShellBase* mpViewShellBase;
    std::unique_ptr<NotesEditWindow> m_xEditWindow;
    std::unique_ptr<weld::CustomWeld> m_xEditWindowWeld;

    std::unique_ptr<OutlinerView> mpOutlinerView;
    std::unique_ptr<Outliner> mpOutliner;

public:
    NotesChildDockingWindow(SfxBindings* pBindings, SfxChildWindow* pChildWindow, Window* pParent);
    virtual ~NotesChildDockingWindow() override;
    void dispose() override;

    OutlinerView* GetOutlinerView() { return mpOutlinerView.get(); }
    ::Outliner* GetOutliner() { return mpOutliner.get(); }
    ViewShellBase* GetViewShellBase() { return mpViewShellBase; }
    DrawViewShell* GetDrawViewShell();
};

class NotesChildWindow final : public SfxChildWindow
{
public:
    SFX_DECL_CHILDWINDOW_WITHID(NotesChildWindow);
    NotesChildWindow(vcl::Window* pParentWindow, sal_uInt16 nId, SfxBindings* pBindings,
                     SfxChildWinInfo* pInfo);
};

} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
