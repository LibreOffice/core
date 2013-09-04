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

#ifndef _SVDVIEW_HXX
#define _SVDVIEW_HXX

// HACK to avoid too deep includes and to have some
// levels free in svdmark itself (MS compiler include depth limit)
#include <svx/svdhdl.hxx>
#include <tools/weakbase.hxx>
#include <svtools/accessibilityoptions.hxx>
#include "svx/svxdllapi.h"
#include <svx/svdcrtv.hxx>
#include <unotools/options.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Klassenhierarchie der View:
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
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class SvxURLField;

enum SdrViewContext {SDRCONTEXT_STANDARD,
                     SDRCONTEXT_POINTEDIT,
                     SDRCONTEXT_GLUEPOINTEDIT,
                     SDRCONTEXT_GRAPHIC,
                     SDRCONTEXT_MEDIA,
                     SDRCONTEXT_TABLE};

enum SdrEventKind  {SDREVENT_NONE,
                    SDREVENT_TEXTEDIT,
                    SDREVENT_MOVACTION,
                    SDREVENT_ENDACTION,
                    SDREVENT_BCKACTION,
                    SDREVENT_BRKACTION,
                    SDREVENT_ENDCREATE,
                    SDREVENT_ENDDRAG,
                    SDREVENT_MARKOBJ,
                    SDREVENT_MARKPOINT,
                    SDREVENT_MARKGLUEPOINT,
                    SDREVENT_BEGMARK,
                    SDREVENT_BEGINSOBJPOINT,
                    SDREVENT_ENDINSOBJPOINT,
                    SDREVENT_BEGINSGLUEPOINT,
                    SDREVENT_BEGDRAGHELPLINE,
                    SDREVENT_BEGDRAGOBJ,
                    SDREVENT_BEGCREATEOBJ,
                    SDREVENT_BEGMACROOBJ,
                    SDREVENT_BEGTEXTEDIT,
                    SDREVENT_ENDMARK,
                    SDREVENT_BRKMARK,
                    SDREVENT_EXECUTEURL};

#define SDRMOUSEBUTTONDOWN 1
#define SDRMOUSEMOVE       2
#define SDRMOUSEBUTTONUP   3

// helper class SdrViewEvent
struct SVX_DLLPUBLIC SdrViewEvent
{
    SdrHdl*                     pHdl;
    SdrObject*                  pObj;
    SdrObject*                  pRootObj;        // Dieses Markieren bei SdrBeginTextEdit
    SdrPageView*                pPV;
    const SvxURLField*          pURLField;

    Point                       aLogicPos;
    SdrHitKind                  eHit;
    SdrEventKind                eEvent;
    SdrHdlKind                  eHdlKind;
    SdrCreateCmd                eEndCreateCmd;   // auch fuer EndInsPoint

    sal_uInt16                      nMouseClicks;
    sal_uInt16                      nMouseMode;
    sal_uInt16                      nMouseCode;
    sal_uInt16                      nHlplIdx;
    sal_uInt16                      nGlueId;

    unsigned                    bMouseDown : 1;
    unsigned                    bMouseUp : 1;
    unsigned                    bDoubleHdlSize : 1;  // Doppelte Handlegroesse wg. TextEdit
    unsigned                    bIsAction : 1;       // Action ist aktiv
    unsigned                    bIsTextEdit : 1;     // TextEdit laeuft zur Zeit
    unsigned                    bTextEditHit : 1;    // offene OutlinerView getroffen
    unsigned                    bAddMark : 1;
    unsigned                    bUnmark : 1;
    unsigned                    bPrevNextMark : 1;
    unsigned                    bMarkPrev : 1;
    unsigned                    bInsPointNewObj : 1;
    unsigned                    bDragWithCopy : 1;
    unsigned                    bCaptureMouse : 1;
    unsigned                    bReleaseMouse : 1;

public:
    SdrViewEvent();
    ~SdrViewEvent();

    // nEventKind ist SDRMOUSEBUTTONDOWN, SDRMOUSEMOVE oder SDRMOUSEBUTTONUP
    void SetMouseEvent(const MouseEvent& rMEvt, sal_uInt16 nEventKind);
};

// helper class for all D&D overlays
class SVX_DLLPUBLIC SdrDropMarkerOverlay
{
    // The OverlayObjects
    ::sdr::overlay::OverlayObjectList               maObjects;

    void ImplCreateOverlays(
        const SdrView& rView,
        const basegfx::B2DPolyPolygon& rLinePolyPolygon);

public:
    SdrDropMarkerOverlay(const SdrView& rView, const SdrObject& rObject);
    SdrDropMarkerOverlay(const SdrView& rView, const Rectangle& rRectangle);
    SdrDropMarkerOverlay(const SdrView& rView, const Point& rStart, const Point& rEnd);
    ~SdrDropMarkerOverlay();
};


/*
 * View
 */
class SVX_DLLPUBLIC SdrView: public SdrCreateView, public tools::WeakBase< SdrView >
{
    friend class                SdrPageView;

    unsigned                    bNoExtendedMouseDispatcher : 1;
    unsigned                    bNoExtendedKeyDispatcher : 1;
    unsigned                    bNoExtendedCommandDispatcher : 1;
    unsigned                    bTextEditOnObjectsWithoutTextIfTextTool : 1;
    unsigned                    mbMasterPagePaintCaching : 1;

protected:
    SvtAccessibilityOptions maAccessibilityOptions;

public:
    TYPEINFO();
    explicit SdrView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrView();

    // Default sind alle Dispatcher aktiviert. Will die App z.B. fuer
    // Sonderbehandlungen im MouseDispatcher eingreifen, so muss sie
    // den erweiterten MouseDispather mit unten stehender Methode deaktivieren
    // und selbst nachimplementieren. Beispiel fuer MouseButtonDown:
    //      SdrViewEvent aVEvt;
    //      SdrHitKind eHit=pSdrView->PickAnything(rMEvt,SDRMOUSEBUTTONDOWN,aVEvt);
    //      ... hier Applikationsspezifischer Eingriff ...
    //      pSdrView->DoMouseEvent(aVEvt);
    //      SetPointer(GetPreferedPointer(...))
    //      CaptureMouse(...)
    void EnableExtendedMouseEventDispatcher(sal_Bool bOn) { bNoExtendedMouseDispatcher = !bOn; }
    sal_Bool IsExtendedMouseEventDispatcherEnabled() const { return bNoExtendedMouseDispatcher; }

    void EnableExtendedKeyInputDispatcher(sal_Bool bOn) { bNoExtendedKeyDispatcher=!bOn; }
    sal_Bool IsExtendedKeyInputDispatcherEnabled() const { return bNoExtendedKeyDispatcher; }

    void EnableExtendedCommandEventDispatcher(sal_Bool bOn) { bNoExtendedCommandDispatcher=!bOn; }
    sal_Bool IsExtendedCommandEventDispatcherEnabled() const { return bNoExtendedCommandDispatcher; }

    void EnableTextEditOnObjectsWithoutTextIfTextTool(sal_Bool bOn) { bTextEditOnObjectsWithoutTextIfTextTool=bOn; }
    sal_Bool IsEnableTextEditOnObjectsWithoutTextIfTextToolEnabled() const { return bTextEditOnObjectsWithoutTextIfTextTool; }

    void SetMasterPagePaintCaching(sal_Bool bOn);
    sal_Bool IsMasterPagePaintCaching() const { return mbMasterPagePaintCaching; }

    sal_Bool KeyInput(const KeyEvent& rKEvt, Window* pWin);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt, Window* pWin);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt, Window* pWin);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt, Window* pWin);
    virtual sal_Bool Command(const CommandEvent& rCEvt, Window* pWin);

    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );

    sal_Bool SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll=sal_False) { return SdrCreateView::SetAttributes(rSet,bReplaceAll); }
    sal_Bool SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr=sal_False) { return SdrCreateView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr); }

    /* new interface src537 */
    sal_Bool GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr=sal_False) const;

    SfxStyleSheet* GetStyleSheet() const;

    // unvollstaendige Implementation:
    // Das OutputDevice ist notwendig, damit ich die HandleSize ermitteln kann.
    // Bei NULL wird das 1. angemeldete Win verwendet.
    Pointer GetPreferedPointer(const Point& rMousePos, const OutputDevice* pOut, sal_uInt16 nModifier=0, sal_Bool bLeftDown=sal_False) const;
    SdrHitKind PickAnything(const MouseEvent& rMEvt, sal_uInt16 nMouseDownOrMoveOrUp, SdrViewEvent& rVEvt) const;
    SdrHitKind PickAnything(const Point& rLogicPos, SdrViewEvent& rVEvt) const;
    sal_Bool DoMouseEvent(const SdrViewEvent& rVEvt);
    virtual SdrViewContext GetContext() const;

    // Die Methoden beruecksichtigen den jeweiligen Kontex:
    // - Einfaches Zeichnen
    // - Punktbearbeitungs-Mode
    // - Klebepunkt-Editmode
    // - TextEdit
    // - ... to be continued
    void MarkAll();
    void UnmarkAll();

    const Rectangle& GetMarkedRect() const;

    virtual void DeleteMarked();

    // Markieren von Objekten, Polygonpunkten oder Klebepunkten (je nach View-
    // Kontext) durch Aufziehen eines Selektionsrahmens.
    //   bAddMark=TRUE: zur bestehenden Selektion hinzumarkieren (->Shift)
    //   bUnmark=TRUE: Bereits selektierte Objekte/Punkte/Klebepunkte die innerhalb
    //                 des aufgezogenen Rahmens liegen werden deselektiert.
    sal_Bool BegMark(const Point& rPnt, sal_Bool bAddMark=sal_False, sal_Bool bUnmark=sal_False);

    // Folgende Actions sind moeglich:
    //   - ObjectCreating
    //   - ObjectMarking
    //   - Object-specific dragging
    //   - General dragging
    // und mehr...
    OUString GetStatusText();

    SvtAccessibilityOptions& getAccessibilityOptions();

    virtual void onAccessibilityOptionsChanged();
};

#endif //_SVDVIEW_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Die App macht sich zunaechst ein SdrModel.
// Anschliessend oeffnet sie ein Win und erzeugt dann eine SdrView.
// An der SdrView meldet sie dann mit der Methode ShowSdrPage() eine Seite an.
// Eine SdrView kann in beliebig vielen Fenstern gleichzeitig angezeigt werden.
// Intern:
// Eine SdrView kann beliebig viele Seiten gleichzeitig anzeigen. Seiten
// werden an- und abgemeldet mit ShowSdrPage()/HideSdrPage(). Fuer jede angemeldete
// Seite wird eine SdrPageView-Instanz im Container aPages angelegt. Bei
// gleichzeitiger Anzeige mehrerer Seiten ist darauf zu achten, dass der Offset-
// Parameter von ShowSdrPage() der Seitengroesse angepasst ist, da sich sonst die
// Seiten ueberlappen koennten.
//
// Elementare Methoden:
// ~~~~~~~~~~~~~~~~~~~~
//   Einfache Events:
//   ~~~~~~~~~~~~~~~~
//     sal_Bool KeyInput(const KeyEvent& rKEvt, Window* pWin);
//     sal_Bool MouseButtonDown(const MouseEvent& rMEvt, Window* pWin);
//     sal_Bool MouseButtonUp(const MouseEvent& rMEvt, Window* pWin);
//     sal_Bool MouseMove(const MouseEvent& rMEvt, Window* pWin);
//     sal_Bool Command(const CommandEvent& rCEvt, Window* pWin);
//
//   Exchange (Clipboard derzeit noch ohne SdrPrivateData):
//   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//     sal_Bool Cut(sal_uIntPtr nFormat=SDR_ANYFORMAT);
//     sal_Bool Yank(sal_uIntPtr nFormat=SDR_ANYFORMAT);
//     sal_Bool Paste(Window* pWin=NULL, sal_uIntPtr nFormat=SDR_ANYFORMAT);
//
//   SfxItems:
//   ~~~~~~~~~
//     sal_Bool GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr=sal_False) const;
//     sal_Bool SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll);
//     SfxStyleSheet* GetStyleSheet() const;
//     sal_Bool SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr);
//
//   Sonstiges:
//   ~~~~~~~~~~
//     Pointer GetPreferedPointer(const Point& rMousePos, const OutputDevice* pOut, sal_uInt16 nTol=0) const;
//     OUString GetStatusText();
//
///////////////////////////////////////////////////////////////////////////////////////////////// */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
