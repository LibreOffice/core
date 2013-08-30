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

#ifndef _SVDPNTV_HXX
#define _SVDPNTV_HXX

#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>
#include <svl/smplhint.hxx>
#include <svl/undo.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdlayer.hxx>  // fuer SetOfByte
#include <vcl/window.hxx>
#include <svtools/colorcfg.hxx>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <svl/itemset.hxx>
#include <vcl/timer.hxx>
#include "svx/svxdllapi.h"
#include <svtools/optionsdrawinglayer.hxx>
#include <unotools/options.hxx>

//************************************************************
//   Pre-Defines
//************************************************************

class SdrPageWindow;

namespace com { namespace sun { namespace star { namespace awt {
    class XControlContainer;
}}}}

class SdrPage;
class SdrView;
class SfxItemSet;
class SfxStyleSheet;
class SdrOle2Obj;
class SdrModel;
class SdrObject;

#ifdef DBG_UTIL
class SdrItemBrowser;
#endif

namespace sdr { namespace contact {
    class ViewObjectContactRedirector;
}}

//************************************************************
//   Defines for AnimationMode
//************************************************************

enum SdrAnimationMode
{
    SDR_ANIMATION_ANIMATE,
    SDR_ANIMATION_DONT_ANIMATE,
    SDR_ANIMATION_DISABLE
};

//************************************************************
//   Typedef's und defines
//************************************************************

typedef unsigned char TRISTATE;
#define FUZZY                   (2)
#define SDR_ANYFORMAT           (0xFFFFFFFF)
#define SDR_ANYITEM             (0xFFFF)
#define SDRVIEWWIN_NOTFOUND     (0xFFFF)


////////////////////////////////////////////////////////////////////////////////////////////////////
class SdrPaintView;

namespace sdr
{
    namespace contact
    {
        class ViewObjectContactRedirector;
    } // end of namespace contact
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////


class SVX_DLLPUBLIC SvxViewHint : public SfxHint
{
public:
    enum HintType { SVX_HINT_VIEWCHANGED };
    TYPEINFO();
    explicit SvxViewHint (HintType eType);
    HintType GetHintType (void) const;

private:
    HintType meHintType;
};

// typedefs for a list of SdrPaintWindows
class SdrPaintWindow;
typedef ::std::vector< SdrPaintWindow* > SdrPaintWindowVector;

//////////////////////////////////////////////////////////////////////////////
// helper to convert any GDIMetaFile to a good quality BitmapEx,
// using default parameters and graphic::XPrimitive2DRenderer

BitmapEx SVX_DLLPUBLIC convertMetafileToBitmapEx(
    const GDIMetaFile& rMtf,
    const basegfx::B2DRange& rTargetRange,
    const sal_uInt32 nMaximumQuadraticPixels = 500000);

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrPaintView : public SfxListener, public SfxRepeatTarget, public SfxBroadcaster, public ::utl::ConfigurationListener
{
    friend class                SdrPageView;
    friend class                SdrGrafObj;

    SdrPageView*                mpPageView;
protected:
    SdrModel*                   pMod;
#ifdef DBG_UTIL
    SdrItemBrowser*             pItemBrowser;
#endif
    const OutputDevice*         pActualOutDev; // Nur zum vergleichen
    OutputDevice*               pDragWin;
    SfxStyleSheet*              pDefaultStyleSheet;

    OUString                    aAktLayer;     // Aktueller Zeichenlayer
    OUString                    aMeasureLayer; // Aktueller Layer fuer Bemassung

//  Container                   aPagV;         // Liste von SdrPageViews

    // All windows this view is displayed on
    SdrPaintWindowVector        maPaintWindows;

    MapMode                     aActualMapMode;
    Size                        aGridBig; // muss dann mal raus
    Size                        aGridFin; // muss dann mal raus
    SdrDragStat                 aDragStat;
    Rectangle                   aMaxWorkArea;
    SfxItemSet                  aDefaultAttr;
    Timer                       aComeBackTimer;

    SdrAnimationMode            eAnimationMode;

    sal_uInt16                      nHitTolPix;
    sal_uInt16                      nMinMovPix;
    sal_uInt16                      nHitTolLog;
    sal_uInt16                      nMinMovLog;
    sal_uIntPtr                     nMasterCacheMode;
    sal_uIntPtr                       nGraphicManagerDrawMode;

    // hold an incarnation of Drawinglayer configuration options
    SvtOptionsDrawinglayer      maDrawinglayerOpt;

    unsigned                    bPageVisible : 1;
    unsigned                    bPageBorderVisible : 1;
    unsigned                    bBordVisible : 1;
    unsigned                    bGridVisible : 1;
    unsigned                    bGridFront : 1;
    unsigned                    bHlplVisible : 1;
    unsigned                    bHlplFront : 1;
    unsigned                    bGlueVisible : 1;    // Persistent. Klebepunkte anzeigen
    unsigned                    bGlueVisible2 : 1;   // Klebepunkte auch bei GluePointEdit anzeigen
    unsigned                    bGlueVisible3 : 1;   // Klebepunkte auch bei EdgeTool anzeigen
    unsigned                    bGlueVisible4 : 1;   // Klebepunkte anzeigen, wenn 1 Edge markiert
    unsigned                    bRestoreColors : 1;   // Pens und Brushes werden zurueckgesetzt.
    unsigned                    bSomeObjChgdFlag : 1;
    unsigned                    bSwapAsynchron : 1;
    unsigned                    bPrintPreview : 1;

    // sal_Bool fuer die Verwaltung des anzuzeigenden Status
    // Gruppe Betreten/Verlassen. Default ist sal_True, wird aber
    // beispielsweise beim Chart auf sal_False gesetzt, da dort
    // die Ghosted-Effekte zur Darstellug unerwuenscht sind.
    unsigned                    bVisualizeEnteredGroup : 1;
    unsigned                    bAnimationPause : 1;

    // #114898#
    // Flag which decides if buffered output for this view is allowed. When
    // set, PreRendering for PageView rendering will be used. Default is sal_False
    unsigned                    mbBufferedOutputAllowed : 1;

    // #114898#
    // Flag which decides if buffered overlay for this view is allowed. When
    // set, the output will be buffered in an overlay vdev. When not, overlay is
    // directly painted to OutDev. Default is sal_False.
    unsigned                    mbBufferedOverlayAllowed : 1;

    // allow page painting at all?
    unsigned                    mbPagePaintingAllowed : 1;

    // is this a preview renderer?
    unsigned                    mbPreviewRenderer : 1;

    // flags for calc and sw for suppressing OLE, CHART or DRAW objects
    unsigned                    mbHideOle : 1;
    unsigned                    mbHideChart : 1;
    unsigned                    mbHideDraw : 1;             // hide draw objects other than form controls
    unsigned                    mbHideFormControl : 1;      // hide form controls only

public:
    // #114898#
    // interface for PagePaintingAllowed flag
    bool IsBufferedOutputAllowed() const;
    void SetBufferedOutputAllowed(bool bNew);

    // interface for BufferedOverlayAllowed flag
    bool IsBufferedOverlayAllowed() const;
    void SetBufferedOverlayAllowed(bool bNew);

    // allow page painting at all?
    sal_Bool IsPagePaintingAllowed() const;
    void SetPagePaintingAllowed(bool bNew);

protected:
    svtools::ColorConfig            maColorConfig;
    Color                           maGridColor;

    // interface to SdrPaintWindow
protected:
    void AppendPaintWindow(SdrPaintWindow& rNew);
    SdrPaintWindow* RemovePaintWindow(SdrPaintWindow& rOld);
    void ConfigurationChanged( ::utl::ConfigurationBroadcaster*, sal_uInt32 );

public:
    sal_uInt32 PaintWindowCount() const { return maPaintWindows.size(); }
    SdrPaintWindow* FindPaintWindow(const OutputDevice& rOut) const;
    SdrPaintWindow* GetPaintWindow(sal_uInt32 nIndex) const;
    // replacement for GetWin(0), may return 0L (!)
    OutputDevice* GetFirstOutputDevice() const;

private:
    SVX_DLLPRIVATE void ImpClearVars();
    DECL_LINK(ImpComeBackHdl,void*);

protected:
    sal_uInt16 ImpGetMinMovLogic(short nMinMov, const OutputDevice* pOut) const;
    sal_uInt16 ImpGetHitTolLogic(short nHitTol, const OutputDevice* pOut) const;

    // Wenn man den IdleStatus des Systems nicht abwarten will (auf const geschummelt):
    void FlushComeBackTimer() const;
    void TheresNewMapMode();
    void ImpSetGlueVisible2(bool bOn) { if (bGlueVisible2!=(unsigned)bOn) { bGlueVisible2=bOn; if (!bGlueVisible && !bGlueVisible3 && !bGlueVisible4) GlueInvalidate(); } }
    void ImpSetGlueVisible3(bool bOn) { if (bGlueVisible3!=(unsigned)bOn) { bGlueVisible3=bOn; if (!bGlueVisible && !bGlueVisible2 && !bGlueVisible4) GlueInvalidate(); } }
    void ImpSetGlueVisible4(bool bOn) { if (bGlueVisible4!=(unsigned)bOn) { bGlueVisible4=bOn; if (!bGlueVisible && !bGlueVisible2 && !bGlueVisible3) GlueInvalidate(); } }
    sal_Bool ImpIsGlueVisible2() const { return bGlueVisible2; }
    sal_Bool ImpIsGlueVisible3() const { return bGlueVisible3; }
    sal_Bool ImpIsGlueVisible4() const { return bGlueVisible4; }

public:
    sal_Bool ImpIsGlueVisible() { return bGlueVisible || bGlueVisible2 || bGlueVisible3 || bGlueVisible4; }
protected:

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);
    void GlueInvalidate() const;

    void ShowEncirclement(OutputDevice* pOut);
    void HideEncirclement(OutputDevice* pOut);
    void DrawEncirclement(OutputDevice* pOut) const;

    // ModelHasChanged wird gerufen, sobald nach beliebig vielen HINT_OBJCHG
    // das System wieder idle ist (StarView-Timer). Wer diese Methode ueberlaed,
    // muss unbedingt ModelHasChanged() der Basisklasse rufen.
    virtual void ModelHasChanged();

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrPaintView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrPaintView();

public:
    TYPEINFO();

    virtual void ClearPageView();
    SdrModel* GetModel() const { return pMod; }

    virtual sal_Bool IsAction() const;
    virtual void MovAction(const Point& rPnt);
    virtual void EndAction();
    virtual void BckAction();
    virtual void BrkAction(); // Alle Actions z.B. Draggen abbrechen.
    virtual void TakeActionRect(Rectangle& rRect) const;

    // info about TextEdit. Default is sal_False.
    virtual bool IsTextEdit() const;

    // info about TextEditPageView. Default is 0L.
    virtual SdrPageView* GetTextEditPageView() const;

    // Muss dann bei jedem Fensterwechsel (wenn die SdrView in mehreren
    // Fenstern gleichzeitig dargestellt wird (->z.B. Splitter)) und bei
    // jedem MapMode(Scaling)-wechsel gerufen werden, damit ich aus meinen
    // Pixelwerten logische Werte berechnen kann.
    void SetActualWin(const OutputDevice* pWin);
    void SetMinMoveDistancePixel(sal_uInt16 nVal) { nMinMovPix=nVal; TheresNewMapMode(); }
    sal_uInt16 GetMinMoveDistancePixel() const { return (sal_uInt16)nMinMovPix; }
    void SetHitTolerancePixel(sal_uInt16 nVal) { nHitTolPix=nVal; TheresNewMapMode(); }
    sal_uInt16 GetHitTolerancePixel() const { return (sal_uInt16)nHitTolPix; }

    // data read access on logic HitTolerance and MinMoveTolerance
    sal_uInt16 getHitTolLog() const { return nHitTolLog; }
    sal_uInt16 getMinMovLog() const { return nMinMovLog; }

    // Flag zur Visualisierung von Gruppen abfragen/testen
    sal_Bool DoVisualizeEnteredGroup() const { return bVisualizeEnteredGroup; }
    void SetVisualizeEnteredGroup(sal_Bool bNew) { bVisualizeEnteredGroup = bNew; }

    // Am DragStatus laesst sich beispielsweise erfragen, welche
    // entfernung bereits gedraggd wurde, etc.
    const SdrDragStat& GetDragStat() const { return aDragStat; }

    // Anmelden/Abmelden einer PageView an der View.
    // Dieselbe // Seite kann nicht mehrfach angemeldet werden.
    // Methoden mit dem Suffix PgNum erwarten als numerischen Parameter
    // eine Seitennummer (0...). Methoden mit dem Suffix PvNum erwarten
    // degagen als numerischen Parameter die Nummer der PageView an der
    // SdrView (Iterieren ueber alle angemeldeten Pages).
    virtual SdrPageView* ShowSdrPage(SdrPage* pPage);
    virtual void HideSdrPage();

    // Iterieren ueber alle angemeldeten PageViews
//  sal_uInt16 GetPageViewCount() const { return sal_uInt16(aPagV.Count()); }
//  SdrPageView* GetPageViewByIndex(sal_uInt16 nPvNum) const { return ((SdrPageView*)aPagV.GetObject(nPvNum)); }
    SdrPageView* GetSdrPageView() const { return mpPageView; }

    // Pageview einer bestimmten Seite ermitteln
//  SdrPageView* GetPageViewByPage(const SdrPage* pPage) const;
//  sal_uInt16 GetIndexByPageView(const SdrPageView* pPV) const;

    // Test, ob eine Seite getroffen
//  SdrPageView* HitPage(const Point& rPnt) const;

    // Die Seite, die dem Punkt am naechsten ist. Liefert nur NULL,
    // wenn absolut keine Seite angemeldet ist.
//  SdrPageView* GetPageViewByPosition(const Point& rPnt) const;

    // Eine SdrView kann auf mehreren Fenstern gleichzeitig abgebiltet sein:
    virtual void AddWindowToPaintView(OutputDevice* pNewWin);
    virtual void DeleteWindowFromPaintView(OutputDevice* pOldWin);

    void SetLayerVisible(const OUString& rName, bool bShow=true);
    bool IsLayerVisible(const OUString& rName) const;

    void SetLayerLocked(const OUString& rName, sal_Bool bLock=sal_True);
    bool IsLayerLocked(const OUString& rName) const;

    void SetLayerPrintable(const OUString& rName, sal_Bool bPrn=sal_True);
    bool IsLayerPrintable(const OUString& rName) const;

    // PrePaint call forwarded from app windows
    void PrePaint();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // used internally for Draw/Impress/sch/chart2
    virtual void CompleteRedraw(OutputDevice* pOut, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0);

    // #i72889# used from CompleteRedraw() implementation internally, added to be able to do a complete redraw in single steps
    //
    // BeginCompleteRedraw returns (or even creates) a SdrPaintWindow which shall then be used as
    // target for paints. Since paints may be buffered, use it's GetTargetOutputDevice() method which will
    // return the buffer in case of bufered.
    // DoCompleteRedraw draws the DrawingLayer hierarchy then.
    // EndCompleteRedraw does the necessary refreshes, evtl. paints text edit and overlay and evtl destroys the
    // SdrPaintWindow again. This means: the SdrPaintWindow is no longer safe after this closing call.
    virtual SdrPaintWindow* BeginCompleteRedraw(OutputDevice* pOut);
    virtual void DoCompleteRedraw(SdrPaintWindow& rPaintWindow, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0);
    virtual void EndCompleteRedraw(SdrPaintWindow& rPaintWindow, bool bPaintFormLayer);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // used for the other applications basctl/sc/sw which call DrawLayer at PageViews
    // #i74769# Interface change to use common BeginCompleteRedraw/EndCompleteRedraw
    // #i76114# bDisableIntersect disables intersecting rReg with the Window's paint region
    SdrPaintWindow* BeginDrawLayers(OutputDevice* pOut, const Region& rReg, bool bDisableIntersect = false);
    // used when the region passed to BeginDrawLayers needs to be changed
    void UpdateDrawLayersRegion(OutputDevice* pOut, const Region& rReg, bool bDisableIntersect = false);
    void EndDrawLayers(SdrPaintWindow& rPaintWindow, bool bPaintFormLayer);

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // used to paint the form layer after the PreRender device is flushed (painted) to the window.
    void ImpFormLayerDrawing(SdrPaintWindow& rPaintWindow) const;

    Region OptimizeDrawLayersRegion(OutputDevice* pOut, const Region& rReg, bool bDisableIntersect);

public:
    sal_Bool IsPageVisible() const { return bPageVisible; }             // Seite (weisse Flaeche) malen oder nicht
    sal_Bool IsPageBorderVisible() const { return bPageBorderVisible; } // Seite (weisse Flaeche) malen oder nicht
    sal_Bool IsBordVisible() const { return bBordVisible; }             // Seitenrandlinie malen oder nicht
    sal_Bool IsGridVisible() const { return bGridVisible; }             // Rastergitter malen oder nicht
    sal_Bool IsGridFront() const { return bGridFront  ; }               // Rastergitter ueber die Objekte druebermalen oder dahinter
    sal_Bool IsHlplVisible() const { return bHlplVisible; }             // Hilfslinien der Seiten malen oder nicht
    sal_Bool IsHlplFront() const { return bHlplFront  ; }               // Hilfslinie ueber die Objekte druebermalen oder dahinter
    sal_Bool IsGlueVisible() const { return bGlueVisible; }             // Konnektoren der objekte sichtbar oder nicht
    Color GetGridColor() const;
    void SetPageVisible(bool bOn = true) { bPageVisible=bOn; InvalidateAllWin(); }
    void SetPageBorderVisible(bool bOn = true) { bPageBorderVisible=bOn; InvalidateAllWin(); }
    void SetBordVisible(bool bOn = true) { bBordVisible=bOn; InvalidateAllWin(); }
    void SetGridVisible(bool bOn = true) { bGridVisible=bOn; InvalidateAllWin(); }
    void SetGridFront(bool bOn = true) { bGridFront  =bOn; InvalidateAllWin(); }
    void SetHlplVisible(bool bOn = true) { bHlplVisible=bOn; InvalidateAllWin(); }
    void SetHlplFront(bool bOn = true) { bHlplFront  =bOn; InvalidateAllWin(); }
    void SetGlueVisible(bool bOn = true) { if (bGlueVisible!=(unsigned)bOn) { bGlueVisible=bOn; if (!bGlueVisible2 && !bGlueVisible3 && !bGlueVisible4) GlueInvalidate(); } }
    void SetGridColor( Color aColor );

    sal_Bool IsPreviewRenderer() const { return (sal_Bool )mbPreviewRenderer; }
    void SetPreviewRenderer(bool bOn) { if((unsigned)bOn != mbPreviewRenderer) { mbPreviewRenderer=bOn; }}

    // access methods for calc and sw hide object modes
    bool getHideOle() const { return mbHideOle; }
    bool getHideChart() const { return mbHideChart; }
    bool getHideDraw() const { return mbHideDraw; }
    bool getHideFormControl() const { return mbHideFormControl; }
    void setHideOle(bool bNew) { if(bNew != (bool)mbHideOle) mbHideOle = bNew; }
    void setHideChart(bool bNew) { if(bNew != (bool)mbHideChart) mbHideChart = bNew; }
    void setHideDraw(bool bNew) { if(bNew != (bool)mbHideDraw) mbHideDraw = bNew; }
    void setHideFormControl(bool bNew) { if(bNew != (bool)mbHideFormControl) mbHideFormControl = bNew; }

    void SetGridCoarse(const Size& rSiz) { aGridBig=rSiz; }
    void SetGridFine(const Size& rSiz) { aGridFin=rSiz; if (aGridFin.Height()==0) aGridFin.Height()=aGridFin.Width(); if (bGridVisible) InvalidateAllWin(); } // #40479#
    const Size& GetGridCoarse() const { return aGridBig; }
    const Size& GetGridFine() const { return aGridFin; }

    void InvalidateAllWin();
    void InvalidateAllWin(const Rectangle& rRect, sal_Bool bPlus1Pix=sal_False);

    // Wenn die View kein Invalidate() an den Fenstern durchfuehren soll, muss
    // man diese beiden folgenden Methoden ueberladen und entsprechend anders
    // reagieren.
    virtual void InvalidateOneWin(Window& rWin);
    virtual void InvalidateOneWin(Window& rWin, const Rectangle& rRect);

    void SetActiveLayer(const OUString& rName) { aAktLayer=rName; }
    const OUString&  GetActiveLayer() const { return aAktLayer; }

    // Verlassen einer betretenen Objektgruppe aller sichtbaren Seiten.
    // (wie MsDos chdir ..)
    void LeaveOneGroup();

    // Verlassen aller betretenen Objektgruppen aller sichtbaren Seiten.
    // (wie MsDos chdir \)
    void LeaveAllGroup();

    // Feststellen, ob Leave sinnvoll ist.
    bool IsGroupEntered() const;

    // DefaultAttribute an der View: Neu erzeugte Objekte bekommen diese
    // Attribute direkt nach dem Erzeugen erstmal zugewiesen.
    void SetDefaultAttr(const SfxItemSet& rAttr, sal_Bool bReplaceAll);
    const SfxItemSet& GetDefaultAttr() const { return aDefaultAttr; }
    void SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr);
    SfxStyleSheet* GetDefaultStyleSheet() const { return pDefaultStyleSheet; }

    void SetNotPersistDefaultAttr(const SfxItemSet& rAttr, sal_Bool bReplaceAll);
    void MergeNotPersistDefaultAttr(SfxItemSet& rAttr, sal_Bool bOnlyHardAttr) const;

    // use this mode as mode to draw all internal GraphicManager objects with
    sal_uIntPtr GetGraphicManagerDrawMode() const { return nGraphicManagerDrawMode; }
    void SetGraphicManagerDrawMode( sal_uIntPtr nMode ) { nGraphicManagerDrawMode = nMode; }

    // SwapIn (z.B. von Grafiken) asynchron durchfuehren. Also nicht
    // beim Paint sofort nachladen, sondern dort das Nachladen anstossen.
    // Nach Abschluss des Nachladens wird das Objekt dann angezeigt.
    // Hat nur z.Zt. Wirkung, wenn SwapGraphics am Model eingeschaltet ist.
    // Default=FALSE. Flag ist nicht persistent.
    sal_Bool IsSwapAsynchron() const { return bSwapAsynchron; }
    void SetSwapAsynchron(sal_Bool bJa=sal_True) { bSwapAsynchron=bJa; }
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt, Window* pWin);

    virtual sal_Bool MouseButtonDown(const MouseEvent& /*rMEvt*/, Window* /*pWin*/) { return sal_False; }
    virtual sal_Bool MouseButtonUp(const MouseEvent& /*rMEvt*/, Window* /*pWin*/) { return sal_False; }
    virtual sal_Bool MouseMove(const MouseEvent& /*rMEvt*/, Window* /*pWin*/) { return sal_False; }
    virtual sal_Bool Command(const CommandEvent& /*rCEvt*/, Window* /*pWin*/) { return sal_False; }
    sal_Bool Cut(sal_uIntPtr /*nFormat*/=SDR_ANYFORMAT) { return sal_False; }
    sal_Bool Yank(sal_uIntPtr /*nFormat*/=SDR_ANYFORMAT) { return sal_False; }
    sal_Bool Paste(Window* /*pWin*/=NULL, sal_uIntPtr /*nFormat*/=SDR_ANYFORMAT) { return sal_False; }

    /* new interface src537 */
    sal_Bool GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr=sal_False) const;

    sal_Bool SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(sal_Bool& rOk) const;
    sal_Bool SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr);

    virtual void MakeVisible(const Rectangle& rRect, Window& rWin);

    // Fuer PlugIn. Wird vom Paint des OLE-Obj gerufen.
    virtual void DoConnect(SdrOle2Obj* pOleObj);

    // Animation aktivieren/deaktivieren fuer ::Paint
    // wird z.Zt. ausgewertet von SdrGrafObj, wenn in dem eine Animation steckt
    // Das Unterbinden der automatischen Animation wird z.B. fuer die Dia-Show benoetigt
    sal_Bool IsAnimationEnabled() const { return ( SDR_ANIMATION_ANIMATE == eAnimationMode ); }
    void SetAnimationEnabled( sal_Bool bEnable=sal_True );

    // set/unset pause state for animations
    bool IsAnimationPause() const { return bAnimationPause; }
    void SetAnimationPause( bool bSet );

    // Verhalten beim Starten von Animation im Paint-Handler:
    // 1. Animation normal starten( SDR_ANIMATION_ANIMATE ) => DEFAULT
    // 2. Nur die Ersatzdarstellung der Animation zeichnen ( SDR_ANIMATION_DONT_ANIMATE )
    // 3. Nicht starten und nichts ersatzweise ausgeben ( SDR_ANIMATION_DISABLE )
    void SetAnimationMode( const SdrAnimationMode eMode );
    SdrAnimationMode GetAnimationMode() const { return eAnimationMode; }

    // bei bShow=sal_False wird der Browser destruiert
#ifdef DBG_UTIL
    void ShowItemBrowser(sal_Bool bShow=sal_True);
    sal_Bool IsItemBrowserVisible() const { return pItemBrowser!=NULL && ((Window*)pItemBrowser)->IsVisible(); }
    Window* GetItemBrowser() const { return (Window*)pItemBrowser; }
#endif

    // Muss von App beim Scrollen usw. gerufen werden, damit ein u.U.
    // aktives FormularControl mitverschoben werden kann
    void VisAreaChanged(const OutputDevice* pOut=NULL);
    void VisAreaChanged(const SdrPageWindow& rWindow);

    sal_Bool IsPrintPreview() const { return bPrintPreview; }
    void SetPrintPreview(bool bOn = true) { bPrintPreview=bOn; }

    virtual void onChangeColorConfig();

    // #103834# Set background color for svx at SdrPageViews
    void SetApplicationBackgroundColor(Color aBackgroundColor);

    // #103911# Set document color for svx at SdrPageViews
    void SetApplicationDocumentColor(Color aDocumentColor);

    // #i38135#
    // Sets the timer for Object animations and restarts.
    void SetAnimationTimer(sal_uInt32 nTime);

    // access to Drawinglayer configuration options
    const SvtOptionsDrawinglayer& getOptionsDrawinglayer() const { return maDrawinglayerOpt; }
};

#endif //_SVDPNTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
