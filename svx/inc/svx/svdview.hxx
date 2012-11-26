/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVDVIEW_HXX
#define _SVDVIEW_HXX

#include <svx/svdhdl.hxx>
#include <tools/weakbase.hxx>
#include <svtools/accessibilityoptions.hxx>
#include "svx/svxdllapi.h"
#include <svx/svdcrtv.hxx>
#include <unotools/options.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SvxURLField;

////////////////////////////////////////////////////////////////////////////////////////////////////
// defines

enum SdrViewContext
{
    SDRCONTEXT_STANDARD,
                     SDRCONTEXT_POINTEDIT,
                     SDRCONTEXT_GLUEPOINTEDIT,
                     SDRCONTEXT_GRAPHIC,
                     SDRCONTEXT_MEDIA,
    SDRCONTEXT_TABLE
};

enum SdrEventKind
{
    SDREVENT_NONE,
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
    SDREVENT_EXECUTEURL
};

#define SDRMOUSEBUTTONDOWN 1
#define SDRMOUSEMOVE       2
#define SDRMOUSEBUTTONUP   3

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrViewEvent

struct SVX_DLLPUBLIC SdrViewEvent
{
public:
    const SdrHdl*       mpHdl;
    SdrObject*          mpObj;
    SdrObject*          mpRootObj;        // Dieses Markieren bei SdrBeginTextEdit
//  SdrPageView*        mpPV;

    String              maURLField;
    String              maTargetFrame;

    basegfx::B2DPoint   maLogicPos;
    SdrHitKind          meHit;
    SdrEventKind        meEvent;
    SdrCreateCmd        meEndCreateCmd;   // auch fuer EndInsPoint

    sal_uInt16          mnMouseClicks;
    sal_uInt16          mnMouseMode;
    sal_uInt16          mnMouseCode;
    sal_uInt32          mnHlplIdx;
    sal_uInt32          mnGlueId;

    bool                mbMouseDown : 1;
    bool                mbMouseUp : 1;
    bool                mbIsAction : 1;       // Action ist aktiv
    bool                mbIsTextEdit : 1;     // TextEdit laeuft zur Zeit
    bool                mbTextEditHit : 1;    // offene OutlinerView getroffen
    bool                mbAddMark : 1;
    bool                mbUnmark : 1;
    bool                mbPrevNextMark : 1;
    bool                mbMarkPrev : 1;
    bool                mbInsPointNewObj : 1;
    bool                mbDragWithCopy : 1;
    bool                mbCaptureMouse : 1;
    bool                mbReleaseMouse : 1;

    SdrViewEvent();
    ~SdrViewEvent();

    // nEventKind ist SDRMOUSEBUTTONDOWN, SDRMOUSEMOVE oder SDRMOUSEBUTTONUP
    void SetMouseEvent(const MouseEvent& rMEvt, sal_uInt16 nEventKind);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// helper class for all D&D overlays

class SVX_DLLPUBLIC SdrDropMarkerOverlay
{
private:
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
//
// old view hierarchy, but in principle a single class since the lowest incarnated class
// has to be SdrView anyways
//
//  SdrPaintView
//      SdrSnapView
//          SdrMarkView
//              SdrEditView, IPolyPolygonEditorController
//                  SdrPolyEditView
//                      SdrGlueEditView
//                          SdrObjEditView
//                              SdrExchangeView
//                                  SdrDragView
//                                      SdrCreateView
//                                          SdrView
//
////////////////////////////////////////////////////////////////////////////////////////////////////
//
// All current derivations from SdrView
//
//                              HideSdrPage()       implemented
//                              in destructor       and overloaded
//
// SdrView                          *                   SdrarkView, SdrPaintView
//  DlgEdView
//  OSectionView
//  FrameView
//  GraphCtrlView
//  E3dView
//      DrawViewWrapper
//      FmFormView                  *                   *
//          ScDrawView
//          sd::View
//              sd::DrawView        *                   *
//          SwDrawView
//

class SVX_DLLPUBLIC SdrView: public SdrCreateView, public tools::WeakBase< SdrView >
{
private:
    /// bitfield
    bool                mbNoExtendedMouseDispatcher : 1;
    bool                mbNoExtendedKeyDispatcher : 1;
    bool                mbNoExtendedCommandDispatcher : 1;
    bool                mbMasterPagePaintCaching : 1;

protected:
    SvtAccessibilityOptions maAccessibilityOptions;

public:
    SdrView(SdrModel& rModel1, OutputDevice* pOut = 0);
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
    void EnableExtendedMouseEventDispatcher(bool bOn) { mbNoExtendedMouseDispatcher = !bOn; }
    bool IsExtendedMouseEventDispatcherEnabled() const { return mbNoExtendedMouseDispatcher; }

    void EnableExtendedKeyInputDispatcher(bool bOn) { mbNoExtendedKeyDispatcher=!bOn; }
    bool IsExtendedKeyInputDispatcherEnabled() const { return mbNoExtendedKeyDispatcher; }

    void EnableExtendedCommandEventDispatcher(bool bOn) { mbNoExtendedCommandDispatcher=!bOn; }
    bool IsExtendedCommandEventDispatcherEnabled() const { return mbNoExtendedCommandDispatcher; }

    void SetMasterPagePaintCaching(bool bOn);
    bool IsMasterPagePaintCaching() const { return mbMasterPagePaintCaching; }

    virtual bool KeyInput(const KeyEvent& rKEvt, Window* pWin);
    virtual bool MouseButtonDown(const MouseEvent& rMEvt, Window* pWin);
    virtual bool MouseButtonUp(const MouseEvent& rMEvt, Window* pWin);
    virtual bool MouseMove(const MouseEvent& rMEvt, Window* pWin);
    virtual bool Command(const CommandEvent& rCEvt, Window* pWin);

    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );

    bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll=false) { return SdrCreateView::SetAttributes(rSet,bReplaceAll); }
    bool SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr=false) { return SdrCreateView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr); }
    bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr=false) const;

    SfxStyleSheet* GetStyleSheet() const;

    // unvollstaendige Implementation:
    // Das OutputDevice ist notwendig, damit ich die HandleSize ermitteln kann.
    // Bei NULL wird das 1. angemeldete Win verwendet.
    Pointer GetPreferedPointer(const basegfx::B2DPoint& rMousePos, const OutputDevice* pOut, sal_uInt16 nModifier = 0, bool bLeftDown = false) const;
    SdrHitKind PickAnything(const MouseEvent& rMEvt, sal_uInt16 nMouseDownOrMoveOrUp, SdrViewEvent& rVEvt) const;
    SdrHitKind PickAnything(const basegfx::B2DPoint& rLogicPos, SdrViewEvent& rVEvt) const;
    bool DoMouseEvent(const SdrViewEvent& rVEvt);
    virtual SdrViewContext GetContext() const;

    // Die Methoden beruecksichtigen den jeweiligen Kontex:
    // - Einfaches Zeichnen
    // - Punktbearbeitungs-Mode
    // - Klebepunkt-Editmode
    // - TextEdit
    // - ... to be continued
    bool IsMarkPossible() const;
    void MarkAll();
    void UnmarkAll();
    bool IsAllMarked() const;
    bool IsAllMarkPrevNextPossible() const; // das geht naemlich nicht bei TextEdit!
    bool MarkNext(bool bPrev = false);
    bool MarkNext(const basegfx::B2DPoint& rPnt, bool bPrev = false);

    virtual void DeleteMarked();
    bool IsDeleteMarkedPossible() const;
    bool IsDeletePossible() const { return IsDeleteMarkedPossible(); }

    // Markieren von Objekten, Polygonpunkten oder Klebepunkten (je nach View-
    // Kontext) durch Aufziehen eines Selektionsrahmens.
    //   bAddMark=true: zur bestehenden Selektion hinzumarkieren (->Shift)
    //   bUnmark=true: Bereits selektierte Objekte/Punkte/Klebepunkte die innerhalb
    //                 des aufgezogenen Rahmens liegen werden deselektiert.
    bool BegMark(const basegfx::B2DPoint& rPnt, bool bAddMark = false, bool bUnmark = false);

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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDVIEW_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
