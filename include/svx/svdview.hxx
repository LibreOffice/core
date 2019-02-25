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

#ifndef INCLUDED_SVX_SVDVIEW_HXX
#define INCLUDED_SVX_SVDVIEW_HXX

// HACK to avoid too deep includes and to have some
// levels free in svdmark itself (MS compiler include depth limit)
#include <svx/svdhdl.hxx>
#include <tools/weakbase.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svx/svxdllapi.h>
#include <svx/svdcrtv.hxx>
#include <vcl/event.hxx>
#include <unotools/options.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

//  class hierarchy of View:
//         SfxListener
//         SdrPaintView    PntV   Action            ModChg   Attr   Notify
//         SdrSnapView     SnpV   Action
//
//         SdrMarkView     MrkV   Action   MrkChg   ModChg          Notify
//
//         SdrEditView     EdtV            MrkChg   ModChg   Attr
//         SdrPolyEditView PoEV
//         SdrGlueEditView GlEV
//         SdrObjEditView  EdxV   Action            ModChg   Attr   Notify
//
//         SdrExchangeView XcgV
//         SdrDragView     DrgV   Action
//
//         SdrCreateView   CrtV   Action
//         SdrView         View

class SvxURLField;
namespace sdr { namespace contact { class ObjectContact; }}

enum class SdrViewContext {
    Standard,
    PointEdit,
    GluePointEdit,
    Graphic,
    Media,
    Table
};

enum class SdrEventKind {
    NONE,
    TextEdit,
    MoveAction,
    EndAction,
    BackAction,
    EndCreate,
    EndDrag,
    MarkObj,
    MarkPoint,
    MarkGluePoint,
    BeginMark,
    BeginInsertObjPoint,
    EndInsertObjPoint,
    BeginInsertGluePoint,
    BeginDragHelpline,
    BeginDragObj,
    BeginCreateObj,
    BeginMacroObj,
    BeginTextEdit,
    EndMark,
    BrkMark,
    ExecuteUrl
};

/* for PickAnything() */
enum class SdrMouseEventKind
{
    BUTTONDOWN = 1,
    MOVE       = 2,
    BUTTONUP   = 3,
};

// helper class SdrViewEvent
struct SVX_DLLPUBLIC SdrViewEvent
{
    SdrHdl*                     pHdl;
    SdrObject*                  pObj;
    SdrObject*                  pRootObj;        // mark this when SdrBeginTextEdit is executed
    SdrPageView*                pPV;
    const SvxURLField*          pURLField;

    Point                       aLogicPos;
    SdrHitKind                  eHit;
    SdrEventKind                eEvent;

    sal_uInt16                  nMouseClicks;
    MouseEventModifiers         nMouseMode;
    sal_uInt16                  nMouseCode;
    sal_uInt16                  nHlplIdx;
    sal_uInt16                  nGlueId;

    bool                        bMouseDown : 1;
    bool                        bMouseUp : 1;
    bool                        bIsAction : 1;       // Action is active
    bool                        bIsTextEdit : 1;     // TextEdit runs currently
    bool                        bAddMark : 1;
    bool                        bUnmark : 1;
    bool                        bPrevNextMark : 1;
    bool                        bMarkPrev : 1;

public:
    SdrViewEvent();
    ~SdrViewEvent();
};

// helper class for all D&D overlays
class SVX_DLLPUBLIC SdrDropMarkerOverlay
{
    // The OverlayObjects
    sdr::overlay::OverlayObjectList               maObjects;

    void ImplCreateOverlays(
        const SdrView& rView,
        const basegfx::B2DPolyPolygon& rLinePolyPolygon);

public:
    SdrDropMarkerOverlay(const SdrView& rView, const SdrObject& rObject);
    SdrDropMarkerOverlay(const SdrView& rView, const tools::Rectangle& rRectangle);
    SdrDropMarkerOverlay(const SdrView& rView, const Point& rStart, const Point& rEnd);
    ~SdrDropMarkerOverlay();
};


class SVX_DLLPUBLIC SdrView : public SdrCreateView, public virtual tools::WeakBase
{
    friend class                SdrPageView;

    bool                        bNoExtendedMouseDispatcher : 1;
    bool                        bNoExtendedKeyDispatcher : 1;
    bool                        mbMasterPagePaintCaching : 1;

    SvtAccessibilityOptions maAccessibilityOptions;

public:
    explicit SdrView(
        SdrModel& rSdrModel,
        OutputDevice* pOut = nullptr);

    virtual ~SdrView() override;

    // The default value for all dispatchers is activated. If the app for example
    // wants to intervene in MouseDispatcher for special treatment, you have to
    // deactivate the MouseDispatcher with the help of the method below and you have
    // to implement it yourself. Example for MouseButtonDown:
    //      SdrViewEvent aVEvt;
    //      SdrHitKind eHit=pSdrView->PickAnything(rMEvt,SdrMouseEventKind::BUTTONDOWN,aVEvt);
    //      ... here application-specific intervention ...
    //      pSdrView->DoMouseEvent(aVEvt);
    //      SetPointer(GetPreferredPointer(...))
    //      CaptureMouse(...)
    void EnableExtendedMouseEventDispatcher(bool bOn) { bNoExtendedMouseDispatcher = !bOn; }
    bool IsExtendedMouseEventDispatcherEnabled() const { return bNoExtendedMouseDispatcher; }

    void EnableExtendedKeyInputDispatcher(bool bOn) { bNoExtendedKeyDispatcher=!bOn; }
    bool IsExtendedKeyInputDispatcherEnabled() const { return bNoExtendedKeyDispatcher; }

    void SetMasterPagePaintCaching(bool bOn);
    bool IsMasterPagePaintCaching() const { return mbMasterPagePaintCaching; }

    bool KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt, vcl::Window* pWin) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt, vcl::Window* pWin) override;
    virtual bool MouseMove(const MouseEvent& rMEvt, vcl::Window* pWin) override;
    using SdrCreateView::RequestHelp;
    virtual bool Command(const CommandEvent& rCEvt, vcl::Window* pWin) override;

    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints ) override;

    bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll=false) { return SdrCreateView::SetAttributes(rSet,bReplaceAll); }

    /* new interface src537 */
    void GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr=false) const;

    // incomplete implementation:
    // OutputDevice is necessary to determine HandleSize.
    // If NULL the first signed on Win is used.
    PointerStyle GetPreferredPointer(const Point& rMousePos, const OutputDevice* pOut, sal_uInt16 nModifier=0, bool bLeftDown=false) const;
    SdrHitKind PickAnything(const MouseEvent& rMEvt, SdrMouseEventKind nMouseDownOrMoveOrUp, SdrViewEvent& rVEvt) const;
    SdrHitKind PickAnything(const Point& rLogicPos, SdrViewEvent& rVEvt) const;
    bool DoMouseEvent(const SdrViewEvent& rVEvt);
    virtual SdrViewContext GetContext() const;

    // The methods consider the particular context:
    // - simple drawing
    // - mode for editing points
    // - mode for editing glue points
    // - TextEdit
    // - ... to be continued
    void MarkAll();
    void UnmarkAll();

    const tools::Rectangle& GetMarkedRect() const;

    virtual void DeleteMarked();

    // Marking objects, traverse stations or glue points (depending on view-
    // context) by enveloping the selection frame.
    //   bAddMark=TRUE: add to existing selection (->Shift)
    //   bUnmark=TRUE: remove objects from selection which are inside of
    //                 the enveloped frame.
    bool BegMark(const Point& rPnt, bool bAddMark, bool bUnmark);

    // The following actions are possible:
    //   - ObjectCreating
    //   - ObjectMarking
    //   - Object-specific dragging
    //   - General dragging
    // and more...
    OUString GetStatusText();

    SvtAccessibilityOptions& getAccessibilityOptions() { return maAccessibilityOptions;}

    virtual void onAccessibilityOptionsChanged();

    // Do not create ObjectContact locally, but offer a call to allow override
    // and to create own derivations of ObjectContact
    virtual sdr::contact::ObjectContact* createViewSpecificObjectContact(
        SdrPageWindow& rPageWindow,
        const sal_Char* pDebugName) const;
};

// First of all the app creates a SdrModel.
// Then it opens a Win and creates a SdrView.
// ShowSdrPage() announces a page at SdrView.
// It's possible to show SdrView in any Wins at once.
// internal:
// SdrView can show as many Wins as it wants at once. Pages are announced
// or checked out with the help of ShowSdrPage()/HideSdrPage(). For every announced
// page there is a SdrPageView instance in container aPages. If more than one page
// is showed, you have to pay attention that the offset parameter of ShowSdrPage()
// is conformed to the size of the page (to prevent overlapping of two pages).
//
// elementary methods:
// ~~~~~~~~~~~~~~~~~~~~
//   simple events:
//   ~~~~~~~~~~~~~~~~
//     bool KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin);
//     bool MouseButtonDown(const MouseEvent& rMEvt, vcl::Window* pWin);
//     bool MouseButtonUp(const MouseEvent& rMEvt, vcl::Window* pWin);
//     bool MouseMove(const MouseEvent& rMEvt, vcl::Window* pWin);
//     bool Command(const CommandEvent& rCEvt, vcl::Window* pWin);
//
//   SfxItems:
//   ~~~~~~~~~
//     sal_Bool GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr=sal_False) const;
//     sal_Bool SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll);
//     SfxStyleSheet* GetStyleSheet() const;
//     sal_Bool SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr);
//
//   others:
//   ~~~~~~~~~~
//     Pointer GetPreferredPointer(const Point& rMousePos, const OutputDevice* pOut, sal_uInt16 nTol=0) const;
//     OUString GetStatusText();

#endif // INCLUDED_SVX_SVDVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
