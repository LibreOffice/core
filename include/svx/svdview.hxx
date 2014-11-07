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
#include <unotools/options.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

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

    sal_uInt16                  nMouseClicks;
    MouseEventModifiers         nMouseMode;
    sal_uInt16                  nMouseCode;
    sal_uInt16                  nHlplIdx;
    sal_uInt16                  nGlueId;

    bool                        bMouseDown : 1;
    bool                        bMouseUp : 1;
    bool                        bDoubleHdlSize : 1;  // Doppelte Handlegroesse wg. TextEdit
    bool                        bIsAction : 1;       // Action ist aktiv
    bool                        bIsTextEdit : 1;     // TextEdit laeuft zur Zeit
    bool                        bTextEditHit : 1;    // offene OutlinerView getroffen
    bool                        bAddMark : 1;
    bool                        bUnmark : 1;
    bool                        bPrevNextMark : 1;
    bool                        bMarkPrev : 1;
    bool                        bInsPointNewObj : 1;
    bool                        bDragWithCopy : 1;
    bool                        bCaptureMouse : 1;
    bool                        bReleaseMouse : 1;

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

    bool                        bNoExtendedMouseDispatcher : 1;
    bool                        bNoExtendedKeyDispatcher : 1;
    bool                        bNoExtendedCommandDispatcher : 1;
    bool                        bTextEditOnObjectsWithoutTextIfTextTool : 1;
    bool                        mbMasterPagePaintCaching : 1;

protected:
    SvtAccessibilityOptions maAccessibilityOptions;

public:
    TYPEINFO_OVERRIDE();
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
    //      SetPointer(GetPreferredPointer(...))
    //      CaptureMouse(...)
    void EnableExtendedMouseEventDispatcher(bool bOn) { bNoExtendedMouseDispatcher = !bOn; }
    bool IsExtendedMouseEventDispatcherEnabled() const { return bNoExtendedMouseDispatcher; }

    void EnableExtendedKeyInputDispatcher(bool bOn) { bNoExtendedKeyDispatcher=!bOn; }
    bool IsExtendedKeyInputDispatcherEnabled() const { return bNoExtendedKeyDispatcher; }

    void EnableExtendedCommandEventDispatcher(bool bOn) { bNoExtendedCommandDispatcher=!bOn; }
    bool IsExtendedCommandEventDispatcherEnabled() const { return bNoExtendedCommandDispatcher; }

    void EnableTextEditOnObjectsWithoutTextIfTextTool(bool bOn) { bTextEditOnObjectsWithoutTextIfTextTool=bOn; }
    bool IsEnableTextEditOnObjectsWithoutTextIfTextToolEnabled() const { return bTextEditOnObjectsWithoutTextIfTextTool; }

    void SetMasterPagePaintCaching(bool bOn);
    bool IsMasterPagePaintCaching() const { return mbMasterPagePaintCaching; }

    bool KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin) SAL_OVERRIDE;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt, vcl::Window* pWin) SAL_OVERRIDE;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt, vcl::Window* pWin) SAL_OVERRIDE;
    virtual bool MouseMove(const MouseEvent& rMEvt, vcl::Window* pWin) SAL_OVERRIDE;
    virtual bool Command(const CommandEvent& rCEvt, vcl::Window* pWin) SAL_OVERRIDE;

    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 ) SAL_OVERRIDE;

    bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll=false) { return SdrCreateView::SetAttributes(rSet,bReplaceAll); }
    bool SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr=false) { return SdrCreateView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr); }

    /* new interface src537 */
    bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr=false) const;

    SfxStyleSheet* GetStyleSheet() const;

    // unvollstaendige Implementation:
    // Das OutputDevice ist notwendig, damit ich die HandleSize ermitteln kann.
    // Bei NULL wird das 1. angemeldete Win verwendet.
    Pointer GetPreferredPointer(const Point& rMousePos, const OutputDevice* pOut, sal_uInt16 nModifier=0, bool bLeftDown=false) const;
    SdrHitKind PickAnything(const MouseEvent& rMEvt, sal_uInt16 nMouseDownOrMoveOrUp, SdrViewEvent& rVEvt) const;
    SdrHitKind PickAnything(const Point& rLogicPos, SdrViewEvent& rVEvt) const;
    bool DoMouseEvent(const SdrViewEvent& rVEvt);
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
    bool BegMark(const Point& rPnt, bool bAddMark=false, bool bUnmark=false);

    // Folgende Actions sind moeglich:
    //   - ObjectCreating
    //   - ObjectMarking
    //   - Object-specific dragging
    //   - General dragging
    // und mehr...
    OUString GetStatusText();

    SvtAccessibilityOptions& getAccessibilityOptions() { return maAccessibilityOptions;}

    virtual void onAccessibilityOptionsChanged();
};

#endif // INCLUDED_SVX_SVDVIEW_HXX

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
//     sal_Bool KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin);
//     sal_Bool MouseButtonDown(const MouseEvent& rMEvt, vcl::Window* pWin);
//     sal_Bool MouseButtonUp(const MouseEvent& rMEvt, vcl::Window* pWin);
//     sal_Bool MouseMove(const MouseEvent& rMEvt, vcl::Window* pWin);
//     sal_Bool Command(const CommandEvent& rCEvt, vcl::Window* pWin);
//
//   Exchange (Clipboard derzeit noch ohne SdrPrivateData):
//   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//     sal_Bool Cut(sal_uIntPtr nFormat=SDR_ANYFORMAT);
//     sal_Bool Yank(sal_uIntPtr nFormat=SDR_ANYFORMAT);
//     sal_Bool Paste(vcl::Window* pWin=NULL, sal_uIntPtr nFormat=SDR_ANYFORMAT);
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
//     Pointer GetPreferredPointer(const Point& rMousePos, const OutputDevice* pOut, sal_uInt16 nTol=0) const;
//     OUString GetStatusText();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
