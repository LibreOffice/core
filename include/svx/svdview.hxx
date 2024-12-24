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

#pragma once

// HACK to avoid too deep includes and to have some
// levels free in svdmark itself (MS compiler include depth limit)
#include <svx/svdhdl.hxx>
#include <tools/weakbase.h>
#include <svx/svxdllapi.h>
#include <svx/svdcrtv.hxx>
#include <vcl/event.hxx>
#include <unotools/options.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

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
class SdrPageWindow;
namespace sdr::contact { class ObjectContact; }

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
struct SVXCORE_DLLPUBLIC SdrViewEvent
{
    SdrHdl* mpHdl;
    SdrObject* mpObj;
    SdrObject* mpRootObj; // mark this when SdrBeginTextEdit is executed
    SdrPageView* mpPV;
    const SvxURLField* mpURLField;

    Point maLogicPos;
    SdrHitKind meHit;
    SdrEventKind meEvent;

    sal_uInt16 mnMouseClicks;
    MouseEventModifiers mnMouseMode;
    sal_uInt16 mnMouseCode;
    sal_uInt16 mnHlplIdx;
    sal_uInt16 mnGlueId;

    bool mbMouseDown : 1;
    bool mbMouseUp : 1;
    bool mbIsAction : 1;       // Action is active
    bool mbIsTextEdit : 1;     // TextEdit runs currently
    bool mbAddMark : 1;
    bool mbUnmark : 1;
    bool mbPrevNextMark : 1;
    bool mbMarkPrev : 1;

public:
    SdrViewEvent();
};

// helper class for all D&D overlays
class SVXCORE_DLLPUBLIC SdrDropMarkerOverlay
{
    // The OverlayObjects
    sdr::overlay::OverlayObjectList maObjects;

    void ImplCreateOverlays(
        const SdrView& rView,
        const basegfx::B2DPolyPolygon& rLinePolyPolygon);

public:
    SdrDropMarkerOverlay(const SdrView& rView, const SdrObject& rObject);
    SdrDropMarkerOverlay(const SdrView& rView, const tools::Rectangle& rRectangle);
    SdrDropMarkerOverlay(const SdrView& rView, const Point& rStart, const Point& rEnd);
    ~SdrDropMarkerOverlay();
};


class SVXCORE_DLLPUBLIC SdrView : public SdrCreateView, public tools::WeakBase
{
    friend class SdrPageView;

    bool mbNoExtendedMouseDispatcher : 1;
    bool mbNoExtendedKeyDispatcher : 1;
    bool mbMasterPagePaintCaching : 1;

public:
    explicit SdrView(
        SdrModel& rSdrModel,
        OutputDevice* pOut = nullptr);

    virtual ~SdrView() override;

    virtual bool IsSdrView() const final { return true; }

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
    void EnableExtendedMouseEventDispatcher(bool bOn) { mbNoExtendedMouseDispatcher = !bOn; }
    bool IsExtendedMouseEventDispatcherEnabled() const { return mbNoExtendedMouseDispatcher; }

    void EnableExtendedKeyInputDispatcher(bool bOn) { mbNoExtendedKeyDispatcher=!bOn; }
    bool IsExtendedKeyInputDispatcherEnabled() const { return mbNoExtendedKeyDispatcher; }

    void SetMasterPagePaintCaching(bool bOn);
    bool IsMasterPagePaintCaching() const { return mbMasterPagePaintCaching; }

    bool KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt, OutputDevice* pWin) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt, OutputDevice* pWin) override;
    virtual bool MouseMove(const MouseEvent& rMEvt, OutputDevice* pWin) override;
    using SdrCreateView::RequestHelp;
    virtual bool Command(const CommandEvent& rCEvt, vcl::Window* pWin) override;

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
    // - mode for editing gluepoints
    // - TextEdit
    // - ... to be continued
    void MarkAll();
    void UnmarkAll();

    const tools::Rectangle& GetMarkedRect() const;

    virtual void DeleteMarked();

    // Marking objects, traverse stations or gluepoints (depending on view-
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

    // Do not create ObjectContact locally, but offer a call to allow override
    // and to create own derivations of ObjectContact
    virtual sdr::contact::ObjectContact* createViewSpecificObjectContact(
        SdrPageWindow& rPageWindow,
        const char* pDebugName) const;

    // Interactive Move Action programmatically
    bool MoveShapeHandle(const sal_uInt32 handleNum, const Point& aEndPoint, const sal_Int32 aObjectOrdNum = -1);
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
