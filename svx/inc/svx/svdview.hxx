/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SvxURLField;

//************************************************************
//   Defines
//************************************************************

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

//************************************************************
//   Hilfsklasse SdrViewEvent
//************************************************************

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

    UINT16                      nMouseClicks;
    UINT16                      nMouseMode;
    UINT16                      nMouseCode;
    UINT16                      nHlplIdx;
    UINT16                      nGlueId;

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
    void SetMouseEvent(const MouseEvent& rMEvt, USHORT nEventKind);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// helper class for all D&D overlays

class SVX_DLLPUBLIC SdrDropMarkerOverlay
{
    // The OverlayObjects
    ::sdr::overlay::OverlayObjectList               maObjects;

    void ImplCreateOverlays(const SdrView& rView, const basegfx::B2DPolyPolygon& rPolyPolygon);

public:
    SdrDropMarkerOverlay(const SdrView& rView, const SdrObject& rObject);
    SdrDropMarkerOverlay(const SdrView& rView, const Rectangle& rRectangle);
    SdrDropMarkerOverlay(const SdrView& rView, const Point& rStart, const Point& rEnd);
    ~SdrDropMarkerOverlay();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@ @@ @@ @@@@@ @@   @@
//  @@ @@ @@ @@    @@   @@
//  @@ @@ @@ @@    @@ @ @@
//  @@@@@ @@ @@@@  @@@@@@@
//   @@@  @@ @@    @@@@@@@
//   @@@  @@ @@    @@@ @@@
//    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

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
    SdrView(SdrModel* pModel1, OutputDevice* pOut = 0L);
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
    void EnableExtendedMouseEventDispatcher(BOOL bOn) { bNoExtendedMouseDispatcher = !bOn; }
    BOOL IsExtendedMouseEventDispatcherEnabled() const { return bNoExtendedMouseDispatcher; }

    void EnableExtendedKeyInputDispatcher(BOOL bOn) { bNoExtendedKeyDispatcher=!bOn; }
    BOOL IsExtendedKeyInputDispatcherEnabled() const { return bNoExtendedKeyDispatcher; }

    void EnableExtendedCommandEventDispatcher(BOOL bOn) { bNoExtendedCommandDispatcher=!bOn; }
    BOOL IsExtendedCommandEventDispatcherEnabled() const { return bNoExtendedCommandDispatcher; }

    void EnableTextEditOnObjectsWithoutTextIfTextTool(BOOL bOn) { bTextEditOnObjectsWithoutTextIfTextTool=bOn; }
    BOOL IsEnableTextEditOnObjectsWithoutTextIfTextToolEnabled() const { return bTextEditOnObjectsWithoutTextIfTextTool; }

    void SetMasterPagePaintCaching(sal_Bool bOn);
    sal_Bool IsMasterPagePaintCaching() const { return mbMasterPagePaintCaching; }

    BOOL KeyInput(const KeyEvent& rKEvt, Window* pWin);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt, Window* pWin);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt, Window* pWin);
    virtual BOOL MouseMove(const MouseEvent& rMEvt, Window* pWin);
    virtual BOOL Command(const CommandEvent& rCEvt, Window* pWin);

    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );

    BOOL SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll=FALSE) { return SdrCreateView::SetAttributes(rSet,bReplaceAll); }
    BOOL SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr=FALSE) { return SdrCreateView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr); }

    /* new interface src537 */
    BOOL GetAttributes(SfxItemSet& rTargetSet, BOOL bOnlyHardAttr=FALSE) const;

    SfxStyleSheet* GetStyleSheet() const;

    // unvollstaendige Implementation:
    // Das OutputDevice ist notwendig, damit ich die HandleSize ermitteln kann.
    // Bei NULL wird das 1. angemeldete Win verwendet.
    Pointer GetPreferedPointer(const Point& rMousePos, const OutputDevice* pOut, USHORT nModifier=0, BOOL bLeftDown=FALSE) const;
    SdrHitKind PickAnything(const MouseEvent& rMEvt, USHORT nMouseDownOrMoveOrUp, SdrViewEvent& rVEvt) const;
    SdrHitKind PickAnything(const Point& rLogicPos, SdrViewEvent& rVEvt) const;
    BOOL DoMouseEvent(const SdrViewEvent& rVEvt);
    virtual SdrViewContext GetContext() const;

    // Die Methoden beruecksichtigen den jeweiligen Kontex:
    // - Einfaches Zeichnen
    // - Punktbearbeitungs-Mode
    // - Klebepunkt-Editmode
    // - TextEdit
    // - ... to be continued
    BOOL IsMarkPossible() const;
    void MarkAll();
    void UnmarkAll();
    BOOL IsAllMarked() const;
    BOOL IsAllMarkPrevNextPossible() const; // das geht naemlich nicht bei TextEdit!
    BOOL MarkNext(BOOL bPrev=FALSE);
    BOOL MarkNext(const Point& rPnt, BOOL bPrev=FALSE);

    const Rectangle& GetMarkedRect() const;
    void SetMarkedRect(const Rectangle& rRect);

    virtual void DeleteMarked();
    BOOL IsDeleteMarkedPossible() const;
    BOOL IsDeletePossible() const { return IsDeleteMarkedPossible(); }

    // Markieren von Objekten, Polygonpunkten oder Klebepunkten (je nach View-
    // Kontext) durch Aufziehen eines Selektionsrahmens.
    //   bAddMark=TRUE: zur bestehenden Selektion hinzumarkieren (->Shift)
    //   bUnmark=TRUE: Bereits selektierte Objekte/Punkte/Klebepunkte die innerhalb
    //                 des aufgezogenen Rahmens liegen werden deselektiert.
    BOOL BegMark(const Point& rPnt, BOOL bAddMark=FALSE, BOOL bUnmark=FALSE);

    // Folgende Actions sind moeglich:
    //   - ObjectCreating
    //   - ObjectMarking
    //   - Object-specific dragging
    //   - General dragging
    // und mehr...
    String GetStatusText();

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
//     BOOL KeyInput(const KeyEvent& rKEvt, Window* pWin);
//     BOOL MouseButtonDown(const MouseEvent& rMEvt, Window* pWin);
//     BOOL MouseButtonUp(const MouseEvent& rMEvt, Window* pWin);
//     BOOL MouseMove(const MouseEvent& rMEvt, Window* pWin);
//     BOOL Command(const CommandEvent& rCEvt, Window* pWin);
//
//   Exchange (Clipboard derzeit noch ohne SdrPrivateData):
//   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//     BOOL Cut(ULONG nFormat=SDR_ANYFORMAT);
//     BOOL Yank(ULONG nFormat=SDR_ANYFORMAT);
//     BOOL Paste(Window* pWin=NULL, ULONG nFormat=SDR_ANYFORMAT);
//
//   SfxItems:
//   ~~~~~~~~~
//     BOOL GetAttributes(SfxItemSet& rTargetSet, BOOL bOnlyHardAttr=FALSE) const;
//     BOOL SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll);
//     SfxStyleSheet* GetStyleSheet() const;
//     BOOL SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr);
//
//   Sonstiges:
//   ~~~~~~~~~~
//     Pointer GetPreferedPointer(const Point& rMousePos, const OutputDevice* pOut, USHORT nTol=0) const;
//     String  GetStatusText();
//
///////////////////////////////////////////////////////////////////////////////////////////////// */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
