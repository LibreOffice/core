/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "OutlineView.hxx"
#include <Outliner.hxx>

class SdrTextObj;

namespace sd::tools
{
class EventMultiplexerEvent;
}

namespace sd
{
class DrawDocShell;
class NotesPanelViewShell;

/**
 * Derivative of ::sd::SimpleOutlinerView for the notes panel
|*
\************************************************************************/

class NotesPanelView final : public ::sd::SimpleOutlinerView
{
    NotesPanelViewShell& mrNotesPanelViewShell;
    SdOutliner maOutliner;
    OutlinerView maOutlinerView;

    Idle aModifyIdle;

    SdrTextObj* mpTextObj = nullptr;
    bool mbIgnoreNotifications = false;
    bool mbInFocus = false;

    /** stores the last used document color.
        this is changed in onUpdateStyleSettings()
    */
    Color maDocColor = COL_WHITE;

    void removeListener();
    void addListener();

    void setListenerIgnored(bool bIgnore);
    bool isListenerIgnored();

    void getNotesFromDoc();
    void setNotesToDoc();

public:
    NotesPanelView(DrawDocShell& rDocSh, vcl::Window* pWindow,
                   NotesPanelViewShell& rNotesPanelViewSh);
    virtual ~NotesPanelView() override;

    void Paint(const ::tools::Rectangle& rRect, ::sd::Window const* pWin);
    void onResize();
    void onGrabFocus();
    void onLoseFocus();

    OutlinerView* GetOutlinerView();
    OutlinerView* GetViewByWindow(vcl::Window const* pWin) const override;

    SdOutliner& GetOutliner() { return maOutliner; }

    void FillOutliner();
    void onUpdateStyleSettings();
    virtual SvtScriptType GetScriptType() const override;

    void SetLinks();
    void ResetLinks();
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    virtual void GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr = false) const override;
    virtual bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll = false,
                               bool bSlide = false, bool bMaster = false) override;

    // SdrObjEditView's Outliner access overrides to use TextObjectBar implementations.
    virtual const SdrOutliner* GetTextEditOutliner() const override { return &maOutliner; }
    virtual SdrOutliner* GetTextEditOutliner() override { return &maOutliner; }
    virtual const OutlinerView* GetTextEditOutlinerView() const override { return &maOutlinerView; }
    virtual OutlinerView* GetTextEditOutlinerView() override { return &maOutlinerView; }

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                SdrLayerID nLayer) override;
    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt, ::sd::Window* pTargetWindow,
                                 sal_uInt16 nPage, SdrLayerID nLayer) override;

    DECL_LINK(StatusEventHdl, EditStatus&, void);
    DECL_LINK(EditModifiedHdl, LinkParamNone*, void);
    DECL_LINK(ModifyTimerHdl, Timer*, void);
    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent&, void);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
