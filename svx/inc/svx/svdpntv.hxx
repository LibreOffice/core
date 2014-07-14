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
#include <svl/itemset.hxx>
#include <vcl/timer.hxx>
#include "svx/svxdllapi.h"
#include <svtools/optionsdrawinglayer.hxx>
#include <unotools/options.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocirc.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrPageWindow;
class SdrPage;
class SfxItemSet;
class SfxStyleSheet;
class SdrOle2Obj;
class SdrModel;
namespace sdr { namespace contact { class ViewObjectContactRedirector; }}
class SdrPageView;
class SdrView;

#ifdef DBG_UTIL
class SdrItemBrowser;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//   Defines for AnimationMode

enum SdrAnimationMode
{
    SDR_ANIMATION_ANIMATE,
    SDR_ANIMATION_DONT_ANIMATE,
    SDR_ANIMATION_DISABLE
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   Typedef's und defines

typedef unsigned char TRISTATE;
#define FUZZY                   (2)
#define SDR_ANYFORMAT           (0xFFFFFFFF)
#define SDR_ANYITEM             (0xFFFF)

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SvxViewHint : public SfxHint
{
public:
    enum HintType
    {
        SVX_HINT_VIEWCHANGED
    };

    SvxViewHint (HintType eType);
    HintType GetHintType (void) const;

private:
    HintType meHintType;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// typedefs for a list of SdrPaintWindows

class SdrPaintWindow;
typedef ::std::vector< SdrPaintWindow* > SdrPaintWindowVector;
class LazyObjectChangeReactor;

//////////////////////////////////////////////////////////////////////////////
// helper to convert any GDIMetaFile to a good quality BitmapEx,
// using default parameters and graphic::XPrimitive2DRenderer

BitmapEx SVX_DLLPUBLIC convertMetafileToBitmapEx(
    const GDIMetaFile& rMtf,
    const basegfx::B2DRange& rTargetRange,
    const sal_uInt32 nMaximumQuadraticPixels = 500000);

////////////////////////////////////////////////////////////////////////////////////////////////////
// tooling

SVX_DLLPUBLIC basegfx::B2DPoint OrthoDistance8(const basegfx::B2DPoint& rReference, const basegfx::B2DPoint& rCandidate, bool bBigOrtho);
SVX_DLLPUBLIC basegfx::B2DPoint OrthoDistance4(const basegfx::B2DPoint& rReference, const basegfx::B2DPoint& rCandidate, bool bBigOrtho);

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrPaintView : public SfxListener, public SfxRepeatTarget, public SfxBroadcaster, public ::utl::ConfigurationListener
{
private:
    // allow LazyObjectChangeReactor to callback into LazyReactOnObjectChanges()
    friend class LazyObjectChangeReactor;
    LazyObjectChangeReactor*    mpLazyObjectChangeReactor;

protected:
    SdrModel&                   mrModelFromSdrView;
#ifdef DBG_UTIL
    SdrItemBrowser*             mpItemBrowser;
#endif
    const OutputDevice*         mpActualOutDev; // Nur zum vergleichen
    SfxStyleSheet*              mpDefaultStyleSheet;

    String                      maAktLayer;     // Aktueller Zeichenlayer
    String                      maMeasureLayer; // Aktueller Layer fuer Bemassung
    SdrPageView*                mpPageView;

    // All windows this view is displayed on
    SdrPaintWindowVector        maPaintWindows;

    Size                        maGridBig; // muss dann mal raus
    Size                        maGridFin; // muss dann mal raus

    SdrDragStat                 maDragStat;
    basegfx::B2DRange           maMaxWorkArea;
    SfxItemSet                  maDefaultAttr;

    SdrAnimationMode            meAnimationMode;

    double                      mfHitTolPix;
    double                      mfMinMovPix;

    // hold an incarnation of Drawinglayer configuration options
    SvtOptionsDrawinglayer      maDrawinglayerOpt;

    bool                        mbPageVisible : 1;
    bool                        mbPageBorderVisible : 1;
    bool                        mbBordVisible : 1;
    bool                        mbGridVisible : 1;
    bool                        mbGridFront : 1;
    bool                        mbHlplVisible : 1;
    bool                        mbHlplFront : 1;
    bool                        mbGlueVisible : 1;    // Persistent. Klebepunkte anzeigen
    bool                        mbGlueVisible2 : 1;   // Klebepunkte auch bei GluePointEdit anzeigen
    bool                        mbGlueVisible3 : 1;   // Klebepunkte auch bei EdgeTool anzeigen
    bool                        mbGlueVisible4 : 1;   // Klebepunkte anzeigen, wenn 1 Edge markiert
    bool                        mbSwapAsynchron : 1;
    bool                        mbPrintPreview : 1;

    // bool fuer die Verwaltung des anzuzeigenden Status
    // Gruppe Betreten/Verlassen. Default ist true, wird aber
    // beispielsweise beim Chart auf false gesetzt, da dort
    // die Ghosted-Effekte zur Darstellug unerwuenscht sind.
    bool                        mbVisualizeEnteredGroup : 1;
    bool                        mbAnimationPause : 1;

    // #114898#
    // Flag which decides if buffered output for this view is allowed. When
    // set, PreRendering for PageView rendering will be used. Default is false
    bool                        mbBufferedOutputAllowed : 1;

    // #114898#
    // Flag which decides if buffered overlay for this view is allowed. When
    // set, the output will be buffered in an overlay vdev. When not, overlay is
    // directly painted to OutDev. Default is false.
    bool                        mbBufferedOverlayAllowed : 1;

    // allow page painting at all?
    bool                        mbPagePaintingAllowed : 1;

    // is this a preview renderer?
    bool                        mbPreviewRenderer : 1;

    // flags for calc and sw for suppressing OLE, CHART or DRAW objects
    bool                        mbHideOle : 1;
    bool                        mbHideChart : 1;
    bool                        mbHideDraw : 1;             // hide draw objects other than form controls
    bool                        mbHideFormControl : 1;      // hide form controls only

    svtools::ColorConfig            maColorConfig;
    Color                           maGridColor;

    // interface to SdrPaintWindow
    void AppendPaintWindow(SdrPaintWindow& rNew);
    SdrPaintWindow* RemovePaintWindow(SdrPaintWindow& rOld);
    void ConfigurationChanged( ::utl::ConfigurationBroadcaster*, sal_uInt32 );

    // Wenn man den IdleStatus des Systems nicht abwarten will (auf const geschummelt):
    void ForceLazyReactOnObjectChanges() const;
    void ImpSetGlueVisible2(bool bOn) { if(mbGlueVisible2 != bOn) { mbGlueVisible2 = bOn; if (!IsGlueVisible() && !ImpIsGlueVisible3() && !ImpIsGlueVisible4()) GlueInvalidate(); } }
    void ImpSetGlueVisible3(bool bOn) { if(mbGlueVisible3 != bOn) { mbGlueVisible3 = bOn; if (!IsGlueVisible() && !ImpIsGlueVisible2() && !ImpIsGlueVisible4()) GlueInvalidate(); } }
    void ImpSetGlueVisible4(bool bOn) { if(mbGlueVisible4 != bOn) { mbGlueVisible4 = bOn; if (!IsGlueVisible() && !ImpIsGlueVisible2() && !ImpIsGlueVisible3()) GlueInvalidate(); } }
    bool ImpIsGlueVisible2() const { return mbGlueVisible2; }
    bool ImpIsGlueVisible3() const { return mbGlueVisible3; }
    bool ImpIsGlueVisible4() const { return mbGlueVisible4; }

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    void ShowEncirclement(OutputDevice* pOut);
    void HideEncirclement(OutputDevice* pOut);
    void DrawEncirclement(OutputDevice* pOut) const;

    // LazyReactOnObjectChanges wird gerufen, sobald nach beliebig vielen HINT_OBJCHG_*
    // das System wieder idle ist (StarView-Timer). Wer diese Methode ueberlaed,
    // muss unbedingt LazyReactOnObjectChanges() der Basisklasse rufen.
    virtual void LazyReactOnObjectChanges();

    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrPaintView(SdrModel& rModel1, OutputDevice* pOut = 0);
    virtual ~SdrPaintView();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // used to paint the form layer after the PreRender device is flushed (painted) to the window.
    void ImpFormLayerDrawing(SdrPaintWindow& rPaintWindow) const;

    SVX_DLLPRIVATE const OutputDevice* GetActualOutDev() const { return mpActualOutDev; }

public:
    void GlueInvalidate() const;

    /// helper as long as the SdrView hierarchy is in different classes. Return
    /// this as SdrView to have access to upper functionalities
    SdrView* getAsSdrView() const { return (SdrView*)this; }

    // interface for PagePaintingAllowed flag
    bool IsBufferedOutputAllowed() const;
    void SetBufferedOutputAllowed(bool bNew);

    // interface for BufferedOverlayAllowed flag
    bool IsBufferedOverlayAllowed() const;
    void SetBufferedOverlayAllowed(bool bNew);

    // allow page painting at all?
    bool IsPagePaintingAllowed() const;
    void SetPagePaintingAllowed(bool bNew);

    sal_uInt32 PaintWindowCount() const { return maPaintWindows.size(); }
    SdrPaintWindow* FindPaintWindow(const OutputDevice& rOut) const;
    SdrPaintWindow* GetPaintWindow(sal_uInt32 nIndex) const;
    // replacement for GetWin(0), may return 0L (!)
    OutputDevice* GetFirstOutputDevice() const;

    bool ImpIsGlueVisible() { return IsGlueVisible() || ImpIsGlueVisible2() || ImpIsGlueVisible3() || ImpIsGlueVisible4(); }

    virtual void HideSdrPage();
    SdrModel& getSdrModelFromSdrView() const { return mrModelFromSdrView; }

    virtual bool IsTextEdit() const;

    // Muss dann bei jedem Fensterwechsel (wenn die SdrView in mehreren
    // Fenstern gleichzeitig dargestellt wird (->z.B. Splitter)) und bei
    // jedem MapMode(Scaling)-wechsel gerufen werden, damit ich aus meinen
    // Pixelwerten logische Werte berechnen kann.
    void SetActualOutDev(const OutputDevice* pWin) { mpActualOutDev = pWin; }

    void SetMinMoveDistancePixel(double fVal) { if(mfMinMovPix != fVal) mfMinMovPix = fVal; }
    double GetMinMoveDistancePixel() const { return mfMinMovPix; }

    void SetHitTolerancePixel(double fVal) { if(mfHitTolPix != fVal) mfHitTolPix = fVal; }
    double GetHitTolerancePixel() const { return mfHitTolPix; }

    // access on logic HitTolerance and MinMoveTolerance, derived from mpActualOutDev
    // and mfMinMovPix/mfHitTolPix
    double getHitTolLog() const;
    double getMinMovLog() const;

    // Flag zur Visualisierung von Gruppen abfragen/testen
    bool DoVisualizeEnteredGroup() const { return mbVisualizeEnteredGroup; }
    void SetVisualizeEnteredGroup(bool bNew) { if(mbVisualizeEnteredGroup != bNew) { mbVisualizeEnteredGroup = bNew; }}

    // Am DragStatus laesst sich beispielsweise erfragen, welche
    // entfernung bereits gedraggd wurde, etc.
    SdrDragStat& GetDragStat() { return maDragStat; }
    const SdrDragStat& GetDragStat() const { return maDragStat; }

    // Anmelden/Abmelden einer PageView an der View.
    // Dieselbe // Seite kann nicht mehrfach angemeldet werden.
    // Methoden mit dem Suffix PgNum erwarten als numerischen Parameter
    // eine Seitennummer (0...). Methoden mit dem Suffix PvNum erwarten
    // degagen als numerischen Parameter die Nummer der PageView an der
    // SdrView (Iterieren ueber alle angemeldeten Pages).
    virtual void ShowSdrPage(SdrPage& rPage);
    SdrPageView* GetSdrPageView() const { return mpPageView; }

    // Eine SdrView kann auf mehreren Fenstern gleichzeitig abgebiltet sein:
    virtual void AddWindowToPaintView(OutputDevice* pNewWin);
    virtual void DeleteWindowFromPaintView(OutputDevice* pOldWin);

    void SetLayerVisible(const String& rName, bool bShow=true);
    bool IsLayerVisible(const String& rName) const;
    void SetAllLayersVisible(bool bShow=true);

    void SetLayerLocked(const String& rName, bool bLock=true);
    bool IsLayerLocked(const String& rName) const;
    void SetAllLayersLocked(bool bLock=true);

    void SetLayerPrintable(const String& rName, bool bPrn=true);
    bool IsLayerPrintable(const String& rName) const;
    void SetAllLayersPrintable(bool bPrn=true);

    // PrePaint call forwarded from app windows
    void PrePaint();

    // PostPaint call forwarded from app windows
    void PostPaint();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // used internally for Draw/Impress/sch/chart2
    virtual void CompleteRedraw(OutputDevice* pOut, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0) const;

    // #i72889# used from CompleteRedraw() implementation internally, added to be able to do a complete redraw in single steps
    //
    // BeginCompleteRedraw returns (or even creates) a SdrPaintWindow which shall then be used as
    // target for paints. Since paints may be buffered, use it's GetTargetOutputDevice() method which will
    // return the buffer in case of bufered.
    // DoCompleteRedraw draws the DrawingLayer hierarchy then.
    // EndCompleteRedraw does the necessary refreshes, evtl. paints text edit and overlay and evtl destroys the
    // SdrPaintWindow again. This means: the SdrPaintWindow is no longer safe after this closing call.
    virtual SdrPaintWindow* BeginCompleteRedraw(OutputDevice* pOut) const;
    virtual void DoCompleteRedraw(SdrPaintWindow& rPaintWindow, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0) const;
    virtual void EndCompleteRedraw(SdrPaintWindow& rPaintWindow, bool bPaintFormLayer) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // used for the other applications basctl/sc/sw which call DrawLayer at PageViews
    // #i74769# Interface change to use common BeginCompleteRedraw/EndCompleteRedraw
    // #i76114# bDisableIntersect disables intersecting rReg with the Window's paint region
    SdrPaintWindow* BeginDrawLayers(OutputDevice* pOut, const Region& rReg, bool bDisableIntersect = false) const;
    void EndDrawLayers(SdrPaintWindow& rPaintWindow, bool bPaintFormLayer) const;

    bool IsPageVisible() const { return mbPageVisible; }             // Seite (weisse Flaeche) malen oder nicht
    bool IsPageBorderVisible() const { return mbPageBorderVisible; } // Seite (weisse Flaeche) malen oder nicht
    bool IsBordVisible() const { return mbBordVisible; }             // Seitenrandlinie malen oder nicht
    bool IsGridVisible() const { return mbGridVisible; }             // Rastergitter malen oder nicht
    bool IsGridFront() const { return mbGridFront; }               // Rastergitter ueber die Objekte druebermalen oder dahinter
    bool IsHlplVisible() const { return mbHlplVisible; }             // Hilfslinien der Seiten malen oder nicht
    bool IsHlplFront() const { return mbHlplFront; }               // Hilfslinie ueber die Objekte druebermalen oder dahinter
    bool IsGlueVisible() const { return mbGlueVisible; }             // Konnektoren der objekte sichtbar oder nicht

    Color GetGridColor() const;

    void SetPageVisible(bool bOn = true) { if(mbPageVisible != bOn) { mbPageVisible = bOn; InvalidateAllWin(); }}
    void SetPageBorderVisible(bool bOn = true) { if(mbPageBorderVisible != bOn) { mbPageBorderVisible = bOn; InvalidateAllWin(); }}
    void SetBordVisible(bool bOn = true) { if(mbBordVisible != bOn) { mbBordVisible = bOn; InvalidateAllWin(); }}
    void SetGridVisible(bool bOn = true) { if(mbGridVisible != bOn) { mbGridVisible = bOn; InvalidateAllWin(); }}
    void SetGridFront(bool bOn = true) { if(mbGridFront != bOn) { mbGridFront  =bOn; InvalidateAllWin(); }}
    void SetHlplVisible(bool bOn = true) { if(mbHlplVisible != bOn) { mbHlplVisible = bOn; InvalidateAllWin(); }}
    void SetHlplFront(bool bOn = true) { if(mbHlplFront != bOn) { mbHlplFront = bOn; InvalidateAllWin(); }}
    void SetGlueVisible(bool bOn = true) { if(mbGlueVisible != bOn) { mbGlueVisible = bOn; if (!ImpIsGlueVisible2() && !ImpIsGlueVisible3() && !ImpIsGlueVisible4()) GlueInvalidate(); } }
    void SetGridColor( Color aColor );

    bool IsPreviewRenderer() const { return mbPreviewRenderer; }
    void SetPreviewRenderer(bool bOn) { if(bOn != mbPreviewRenderer) mbPreviewRenderer = bOn; }

    // access methods for calc and sw hide object modes
    bool getHideOle() const { return mbHideOle; }
    bool getHideChart() const { return mbHideChart; }
    bool getHideDraw() const { return mbHideDraw; }
    bool getHideFormControl() const { return mbHideFormControl; }
    void setHideOle(bool bNew) { if(bNew != mbHideOle) mbHideOle = bNew; }
    void setHideChart(bool bNew) { if(bNew != mbHideChart) mbHideChart = bNew; }
    void setHideDraw(bool bNew) { if(bNew != mbHideDraw) mbHideDraw = bNew; }
    void setHideFormControl(bool bNew) { if(bNew != mbHideFormControl) mbHideFormControl = bNew; }

    void SetGridCoarse(const Size& rSiz) { maGridBig = rSiz; }
    void SetGridFine(const Size& rSiz) { maGridFin = rSiz; if (!maGridFin.Height()) maGridFin.Height() = maGridFin.Width(); if (IsGridVisible()) InvalidateAllWin(); } // #40479#
    const Size& GetGridCoarse() const { return maGridBig; }
    const Size& GetGridFine() const { return maGridFin; }

    void InvalidateAllWin() const;
    void InvalidateAllWin(const basegfx::B2DRange& rRange, bool bPlus1Pix = false) const;

    // Wenn die View kein Invalidate() an den Fenstern durchfuehren soll, muss
    // man diese beiden folgenden Methoden ueberladen und entsprechend anders
    // reagieren.
    virtual void InvalidateOneWin(Window& rWin) const;
    virtual void InvalidateOneWin(Window& rWin, const basegfx::B2DRange& rRange) const;

    void SetActiveLayer(const String& rName) { maAktLayer = rName; }
    const String&  GetActiveLayer() const { return maAktLayer; }

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
    void SetDefaultAttr(const SfxItemSet& rAttr, bool bReplaceAll);
    const SfxItemSet& GetDefaultAttr() const { return maDefaultAttr; }
    void SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);
    SfxStyleSheet* GetDefaultStyleSheet() const { return mpDefaultStyleSheet; }

    void SetNotPersistDefaultAttr(const SfxItemSet& rAttr, bool bReplaceAll);
    void MergeNotPersistDefaultAttr(SfxItemSet& rAttr, bool bOnlyHardAttr) const;

    // SwapIn (z.B. von Grafiken) asynchron durchfuehren. Also nicht
    // beim Paint sofort nachladen, sondern dort das Nachladen anstossen.
    // Nach Abschluss des Nachladens wird das Objekt dann angezeigt.
    // Hat nur z.Zt. Wirkung, wenn SwapGraphics am Model eingeschaltet ist.
    // Default=false. Flag ist nicht persistent.
    bool IsSwapAsynchron() const { return mbSwapAsynchron; }
    void SetSwapAsynchron(bool bJa = true) { if(mbSwapAsynchron != bJa) { mbSwapAsynchron = bJa; }}
    virtual bool KeyInput(const KeyEvent& rKEvt, Window* pWin);

    virtual bool MouseButtonDown(const MouseEvent& /*rMEvt*/, Window* /*pWin*/) { return false; }
    virtual bool MouseButtonUp(const MouseEvent& /*rMEvt*/, Window* /*pWin*/) { return false; }
    virtual bool MouseMove(const MouseEvent& /*rMEvt*/, Window* /*pWin*/) { return false; }
    virtual bool Command(const CommandEvent& /*rCEvt*/, Window* /*pWin*/) { return false; }
    bool Cut(sal_uInt32 /*nFormat*/=SDR_ANYFORMAT) { return false; }
    bool Yank(sal_uInt32 /*nFormat*/=SDR_ANYFORMAT) { return false; }
    bool Paste(Window* /*pWin*/=NULL, sal_uInt32 /*nFormat*/=SDR_ANYFORMAT) { return false; }

    /* new interface src537 */
    bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr = false) const;

    bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(bool& rOk) const;
    bool SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);

    virtual void MakeVisibleAtView(const basegfx::B2DRange& rRange, Window& rWin);

    // Fuer PlugIn. Wird vom Paint des OLE-Obj gerufen.
    virtual void DoConnect(SdrOle2Obj* pOleObj);

    // Animation aktivieren/deaktivieren fuer ::Paint
    // wird z.Zt. ausgewertet von SdrGrafObj, wenn in dem eine Animation steckt
    // Das Unterbinden der automatischen Animation wird z.B. fuer die Dia-Show benoetigt
    bool IsAnimationEnabled() const { return ( SDR_ANIMATION_ANIMATE == meAnimationMode ); }
    void SetAnimationEnabled( bool bEnable=true );

    // set/unset pause state for animations
    bool IsAnimationPause() const { return mbAnimationPause; }
    void SetAnimationPause( bool bSet );

    // Verhalten beim Starten von Animation im Paint-Handler:
    // 1. Animation normal starten( SDR_ANIMATION_ANIMATE ) => DEFAULT
    // 2. Nur die Ersatzdarstellung der Animation zeichnen ( SDR_ANIMATION_DONT_ANIMATE )
    // 3. Nicht starten und nichts ersatzweise ausgeben ( SDR_ANIMATION_DISABLE )
    void SetAnimationMode( const SdrAnimationMode eMode );
    SdrAnimationMode GetAnimationMode() const { return meAnimationMode; }

    // bei bShow=false wird der Browser destruiert
#ifdef DBG_UTIL
    void ShowItemBrowser(bool bShow = true);
    Window* GetItemBrowser() const { return (Window*)mpItemBrowser; }
    bool IsItemBrowserVisible() const { return GetItemBrowser() && GetItemBrowser()->IsVisible(); }
#endif

    // Muss von App beim Scrollen usw. gerufen werden, damit ein u.U.
    // aktives FormularControl mitverschoben werden kann
    void VisAreaChanged(const OutputDevice* pOut = 0);

    bool IsPrintPreview() const { return mbPrintPreview; }
    void SetPrintPreview(bool bOn = true) { if(mbPrintPreview != bOn) { mbPrintPreview = bOn; }}

    const svtools::ColorConfig& getColorConfig() const;

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

    // Begrenzung des Arbeitsbereichs. Die Begrenzung bezieht sich auf die
    // View, nicht auf die einzelnen PageViews. Von der View wird diese
    // Begrenzung nur bei Interaktionen wie Dragging und Create ausgewertet.
    // Bei von der App algorithmisch oder UI-gesteuerte Aktionen (SetGeoAttr,
    // MoveMarkedObj, ...) muss die App dieses Limit selbst beruecksichtigen.
    // Ferner ist dieses Limit als Grob-Limit zu sehen. U.U. koennen Objekte
    // (z.B. beim Drehen) nicht exakt bis an dieses Limit herangedraggt werden,
    // koennen Objekte durch Rundungsfehler doch etwas ueberstehen, ... .
    // Default=EmptyRect=keine Begrenzung.
    // erst z.T. impl.
    // (besser in die DragView?)
    void SetWorkArea(const basegfx::B2DRange& rRange) { maMaxWorkArea = rRange; }
    const basegfx::B2DRange& GetWorkArea() const { return maMaxWorkArea; }

    // react on selection changes. Called on every selection change, triggered from
    // sdr::selection::Selection::selectionChanged(), thus indirectly by calling
    // one of the above non-const methods of the selection interface. Also called
    // with timer delay when a selected SdrObject triggers HINT_OBJCHG_MOVE,
    // HINT_OBJCHG_RESIZE, HINT_OBJCHG_ATTR, HINT_OBJINSERTED or HINT_OBJREMOVED
    // which also represent a selection change.
    virtual void handleSelectionChange();
};

#endif //_SVDPNTV_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
