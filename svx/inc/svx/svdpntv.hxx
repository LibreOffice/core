/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdpntv.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 14:04:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVDPNTV_HXX
#define _SVDPNTV_HXX

#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif

#ifndef _UNDO_HXX //autogen
#include <svtools/undo.hxx>
#endif

#ifndef _SVDDRAG_HXX
#include <svx/svddrag.hxx>
#endif

#ifndef _SVDLAYER_HXX
#include <svx/svdlayer.hxx>  // fuer SetOfByte
#endif

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifndef INCLUDED_SVTOOLS_OPTIONSDRAWINGLAYER_HXX
#include <svtools/optionsdrawinglayer.hxx>
#endif

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
class XOutputDevice;
class SdrOle2Obj;
class SdrModel;
class SdrObject;
class SdrViewUserMarker;
class B2dIAOManager;

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


class SvxViewHint : public SfxHint
{
public:
    enum HintType { SVX_HINT_VIEWCHANGED };
    TYPEINFO();
    SvxViewHint (HintType eType);
    HintType GetHintType (void) const;

private:
    HintType meHintType;
};

// typedefs for a list of SdrPaintWindows
class SdrPaintWindow;
typedef ::std::vector< SdrPaintWindow* > SdrPaintWindowVector;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrPaintView : public SfxListener, public SfxRepeatTarget, public SfxBroadcaster
{
    friend class                SdrPageView;
    friend class                FrameAnimator;
    friend class                SdrGrafObj;

protected:
    // #114409#-2 Migrate Encirclement
    class ImplEncirclementOverlay*              mpEncirclementOverlay;

    SdrModel*                   pMod;
    XOutputDevice*              pXOut;
#ifdef DBG_UTIL
    SdrItemBrowser*             pItemBrowser;
#endif
    const OutputDevice*         pActualOutDev; // Nur zum vergleichen
    OutputDevice*               pDragWin;
    SfxStyleSheet*              pDefaultStyleSheet;

    String                      aAktLayer;     // Aktueller Zeichenlayer
    String                      aMeasureLayer; // Aktueller Layer fuer Bemassung

//  Container                   aPagV;         // Liste von SdrPageViews
    SdrPageView*                mpPageView;

    // All windows this view is displayed on
    SdrPaintWindowVector        maPaintWindows;

    MapMode                     aActualMapMode;
    Size                        aGridBig; // muss dann mal raus
    Size                        aGridFin; // muss dann mal raus
    Size                        aGridSubdiv;
    Fraction                    aGridWdtX;
    Fraction                    aGridWdtY;
    SdrDragStat                 aDragStat;
    Rectangle                   aMaxWorkArea;
    SfxItemSet                  aDefaultAttr;
    Timer                       aComeBackTimer;

    SdrAnimationMode            eAnimationMode;

    USHORT                      nHitTolPix;
    USHORT                      nMinMovPix;
    USHORT                      nHitTolLog;
    USHORT                      nMinMovLog;
    ULONG                       nMasterCacheMode;
    ULONG                       nGraphicManagerDrawMode;

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

    // BOOL fuer die Verwaltung des anzuzeigenden Status
    // Gruppe Betreten/Verlassen. Default ist TRUE, wird aber
    // beispielsweise beim Chart auf FALSE gesetzt, da dort
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
    unsigned                    bBordVisibleOnlyLeftRight : 1;

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

public:
    sal_uInt32 PaintWindowCount() const { return maPaintWindows.size(); }
    SdrPaintWindow* FindPaintWindow(const OutputDevice& rOut) const;
    SdrPaintWindow* GetPaintWindow(sal_uInt32 nIndex) const;
    // replacement for GetWin(0), may return 0L (!)
    OutputDevice* GetFirstOutputDevice() const;

private:
    SVX_DLLPRIVATE void ImpClearVars();
    DECL_LINK(ImpComeBackHdl,Timer*);

protected:
    USHORT ImpGetMinMovLogic(short nMinMov, const OutputDevice* pOut) const;
    USHORT ImpGetHitTolLogic(short nHitTol, const OutputDevice* pOut) const;

    // Wenn man den IdleStatus des Systems nicht abwarten will (auf const geschummelt):
    void FlushComeBackTimer() const;
    void TheresNewMapMode();
    void ImpSetGlueVisible2(bool bOn) { if (bGlueVisible2!=(unsigned)bOn) { bGlueVisible2=bOn; if (!bGlueVisible && !bGlueVisible3 && !bGlueVisible4) GlueInvalidate(); } }
    void ImpSetGlueVisible3(bool bOn) { if (bGlueVisible3!=(unsigned)bOn) { bGlueVisible3=bOn; if (!bGlueVisible && !bGlueVisible2 && !bGlueVisible4) GlueInvalidate(); } }
    void ImpSetGlueVisible4(bool bOn) { if (bGlueVisible4!=(unsigned)bOn) { bGlueVisible4=bOn; if (!bGlueVisible && !bGlueVisible2 && !bGlueVisible3) GlueInvalidate(); } }
    BOOL ImpIsGlueVisible2() const { return bGlueVisible2; }
    BOOL ImpIsGlueVisible3() const { return bGlueVisible3; }
    BOOL ImpIsGlueVisible4() const { return bGlueVisible4; }

public:
    BOOL ImpIsGlueVisible() { return bGlueVisible || bGlueVisible2 || bGlueVisible3 || bGlueVisible4; }
protected:

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
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

    // access to contained XOutDev
    XOutputDevice* GetExtendedOutputDevice() const { return pXOut; }

    virtual void ClearPageView();
//  virtual void ClearAll();
//  virtual void Clear(); // PageViews loeschen, Markierungen weg, ...
    SdrModel* GetModel() const { return pMod; }

    virtual BOOL IsAction() const;
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
    void SetMinMoveDistancePixel(USHORT nVal) { nMinMovPix=nVal; TheresNewMapMode(); }
    USHORT GetMinMoveDistancePixel() const { return (USHORT)nMinMovPix; }
    void SetHitTolerancePixel(USHORT nVal) { nHitTolPix=nVal; TheresNewMapMode(); }
    USHORT GetHitTolerancePixel() const { return (USHORT)nHitTolPix; }

    // Flag zur Visualisierung von Gruppen abfragen/testen
    BOOL DoVisualizeEnteredGroup() const { return bVisualizeEnteredGroup; }
    void SetVisualizeEnteredGroup(BOOL bNew) { bVisualizeEnteredGroup = bNew; }

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
//  USHORT GetPageViewCount() const { return USHORT(aPagV.Count()); }
//  SdrPageView* GetPageViewByIndex(USHORT nPvNum) const { return ((SdrPageView*)aPagV.GetObject(nPvNum)); }
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

    void SetLayerVisible(const String& rName, BOOL bShow=TRUE);
    bool IsLayerVisible(const String& rName) const;
    void SetAllLayersVisible(BOOL bShow=TRUE);

    void SetLayerLocked(const String& rName, BOOL bLock=TRUE);
    bool IsLayerLocked(const String& rName) const;
    void SetAllLayersLocked(BOOL bLock=TRUE);

    void SetLayerPrintable(const String& rName, BOOL bPrn=TRUE);
    bool IsLayerPrintable(const String& rName) const;
    void SetAllLayersPrintable(BOOL bPrn=TRUE);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // used internally for Draw/Impress/sch/chart2
    virtual void CompleteRedraw(OutputDevice* pOut, const Region& rReg, USHORT nPaintMode = 0, ::sdr::contact::ViewObjectContactRedirector* pRedirector = 0);

    // #i72889# used from CompleteRedraw() implementation internally, added to be able to do a complete redraw in single steps
    //
    // BeginCompleteRedraw returns (or even creates) a SdrPaintWindow whcih shall then be used as
    // target for paints. Since paints may be buffered, use it's GetTargetOutputDevice() method which will
    // return the buffer in case of bufered.
    // DoCompleteRedraw draws the DrawingLayer hierarchy then.
    // EndCompleteRedraw does the necessary refreshes, evtl. paints text edit and overlay and evtl destroys the
    // SdrPaintWindow again. This means: the SdrPaintWindow is no longer safe after this closing call.
    SdrPaintWindow* BeginCompleteRedraw(OutputDevice* pOut);
    void DoCompleteRedraw(SdrPaintWindow& rPaintWindow, const Region& rReg, USHORT nPaintMode = 0, ::sdr::contact::ViewObjectContactRedirector* pRedirector = 0);
    void EndCompleteRedraw(SdrPaintWindow& rPaintWindow);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // used for the other applications basctl/sc/sw which call DrawLayer at PageViews
    // #i74769# Interface change to use common BeginCompleteRedraw/EndCompleteRedraw
    // #i76114# bDisableIntersect disables intersecting rReg with the Window's paint region
    SdrPaintWindow* BeginDrawLayers(OutputDevice* pOut, const Region& rReg, bool bDisableIntersect = false);
    void EndDrawLayers(SdrPaintWindow& rPaintWindow);

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // used to call the old PaintOutlinerView at the single PageViews. Will be replaced when the
    // outliner will be displayed on the overlay in edit mode.
    void ImpTextEditDrawing(SdrPaintWindow& rPaintWindow) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // used to paint the form layer after the PreRender device is flushed (painted) to the window.
    void ImpFormLayerDrawing(SdrPaintWindow& rPaintWindow) const;

public:
    BOOL IsPageVisible() const { return bPageVisible; }             // Seite (weisse Flaeche) malen oder nicht
    BOOL IsPageBorderVisible() const { return bPageBorderVisible; } // Seite (weisse Flaeche) malen oder nicht
    BOOL IsBordVisible() const { return bBordVisible; }             // Seitenrandlinie malen oder nicht
    BOOL IsBordVisibleOnlyLeftRight() const { return bBordVisibleOnlyLeftRight; } // draw only left and right border
    BOOL IsGridVisible() const { return bGridVisible; }             // Rastergitter malen oder nicht
    BOOL IsGridFront() const { return bGridFront  ; }               // Rastergitter ueber die Objekte druebermalen oder dahinter
    BOOL IsHlplVisible() const { return bHlplVisible; }             // Hilfslinien der Seiten malen oder nicht
    BOOL IsHlplFront() const { return bHlplFront  ; }               // Hilfslinie ueber die Objekte druebermalen oder dahinter
    BOOL IsGlueVisible() const { return bGlueVisible; }             // Konnektoren der objekte sichtbar oder nicht
    Color GetGridColor() const;
    void SetPageVisible(bool bOn = true) { bPageVisible=bOn; InvalidateAllWin(); }
    void SetPageBorderVisible(bool bOn = true) { bPageBorderVisible=bOn; InvalidateAllWin(); }
    void SetBordVisible(bool bOn = true) { bBordVisible=bOn; InvalidateAllWin(); }
    void SetBordVisibleOnlyLeftRight(bool bOn = true) { bBordVisibleOnlyLeftRight=bOn; InvalidateAllWin(); }
    void SetGridVisible(bool bOn = true) { bGridVisible=bOn; InvalidateAllWin(); }
    void SetGridFront(bool bOn = true) { bGridFront  =bOn; InvalidateAllWin(); }
    void SetHlplVisible(bool bOn = true) { bHlplVisible=bOn; InvalidateAllWin(); }
    void SetHlplFront(bool bOn = true) { bHlplFront  =bOn; InvalidateAllWin(); }
    void SetGlueVisible(bool bOn = true) { if (bGlueVisible!=(unsigned)bOn) { bGlueVisible=bOn; if (!bGlueVisible2 && !bGlueVisible3 && !bGlueVisible4) GlueInvalidate(); } }
    void SetGridColor( Color aColor );

    sal_Bool IsPreviewRenderer() const { return (sal_Bool )mbPreviewRenderer; }
    void SetPreviewRenderer(bool bOn) { if((unsigned)bOn != mbPreviewRenderer) { mbPreviewRenderer=bOn; }}

    /*alt*/void SetGridCoarse(const Size& rSiz) { aGridBig=rSiz; SetGridWidth(Fraction(rSiz.Width(),1),Fraction(rSiz.Height(),1)); }
    /*alt*/void SetGridFine(const Size& rSiz) { aGridFin=rSiz; if (aGridFin.Height()==0) aGridFin.Height()=aGridFin.Width(); if (bGridVisible) InvalidateAllWin(); } // #40479#
    /*alt*/const Size& GetGridCoarse() const { return aGridBig; }
    /*alt*/const Size& GetGridFine() const { return aGridFin; }

    // SetGridSubdivision(): Werte fuer X und Y sind die Anzahl der
    // Zwischenraeume, also = Anzahl der Zwischenpunkte+1
    void SetGridWidth(const Fraction& rX, const Fraction& rY) { aGridWdtX=rX; aGridWdtY=rY; if (bGridVisible) InvalidateAllWin(); }
    void SetGridSubdivision(const Size& rSubdiv) { aGridSubdiv=rSubdiv; if (bGridVisible) InvalidateAllWin(); }
    const Fraction& GetGridWidthX() const { return aGridWdtX; }
    const Fraction& GetGridWidthY() const { return aGridWdtY; }
    const Size& GetGridSubdivision() const { return aGridSubdiv; }

    void InvalidateAllWin();
    void InvalidateAllWin(const Rectangle& rRect, BOOL bPlus1Pix=FALSE);

    // Wenn die View kein Invalidate() an den Fenstern durchfuehren soll, muss
    // man diese beiden folgenden Methoden ueberladen und entsprechend anders
    // reagieren.
    virtual void InvalidateOneWin(Window& rWin);
    virtual void InvalidateOneWin(Window& rWin, const Rectangle& rRect);

    void SetActiveLayer(const String& rName) { aAktLayer=rName; }
    const String&  GetActiveLayer() const { return aAktLayer; }

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
    void SetDefaultAttr(const SfxItemSet& rAttr, BOOL bReplaceAll);
    const SfxItemSet& GetDefaultAttr() const { return aDefaultAttr; }
    void SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr);
    SfxStyleSheet* GetDefaultStyleSheet() const { return pDefaultStyleSheet; }

    void SetNotPersistDefaultAttr(const SfxItemSet& rAttr, BOOL bReplaceAll);
    void MergeNotPersistDefaultAttr(SfxItemSet& rAttr, BOOL bOnlyHardAttr) const;

    // Aufziehen eines animierten Rechtecks fuer Applikationsspeziefische
    // Verwendung. Alle Positionsangaben in logischen View-Koordinaten.
    // pOut bezeichnet das OutputDevice, in das animierte Rechteck dargestellt
    // werden soll. Wird NULL uebergeben, wird es in allen an der View
    // angemeldeten OutputDevices gleichzeitig dargestellt.
    void BegEncirclement(const Point& rPnt);
    void MovEncirclement(const Point& rPnt);
    Rectangle EndEncirclement(sal_Bool bNoJustify = sal_False);
    void BrkEncirclement();
    sal_Bool IsEncirclement() const { return (0L != mpEncirclementOverlay); }

    // use this mode as mode to draw all internal GraphicManager objects with
    ULONG GetGraphicManagerDrawMode() const { return nGraphicManagerDrawMode; }
    void SetGraphicManagerDrawMode( ULONG nMode ) { nGraphicManagerDrawMode = nMode; }

    // SwapIn (z.B. von Grafiken) asynchron durchfuehren. Also nicht
    // beim Paint sofort nachladen, sondern dort das Nachladen anstossen.
    // Nach Abschluss des Nachladens wird das Objekt dann angezeigt.
    // Hat nur z.Zt. Wirkung, wenn SwapGraphics am Model eingeschaltet ist.
    // Default=FALSE. Flag ist nicht persistent.
    BOOL IsSwapAsynchron() const { return bSwapAsynchron; }
    void SetSwapAsynchron(BOOL bJa=TRUE) { bSwapAsynchron=bJa; }
    virtual BOOL KeyInput(const KeyEvent& rKEvt, Window* pWin);

    virtual BOOL MouseButtonDown(const MouseEvent& /*rMEvt*/, Window* /*pWin*/) { return FALSE; }
    virtual BOOL MouseButtonUp(const MouseEvent& /*rMEvt*/, Window* /*pWin*/) { return FALSE; }
    virtual BOOL MouseMove(const MouseEvent& /*rMEvt*/, Window* /*pWin*/) { return FALSE; }
    virtual BOOL Command(const CommandEvent& /*rCEvt*/, Window* /*pWin*/) { return FALSE; }
    BOOL Cut(ULONG /*nFormat*/=SDR_ANYFORMAT) { return FALSE; }
    BOOL Yank(ULONG /*nFormat*/=SDR_ANYFORMAT) { return FALSE; }
    BOOL Paste(Window* /*pWin*/=NULL, ULONG /*nFormat*/=SDR_ANYFORMAT) { return FALSE; }

    /* new interface src537 */
    BOOL GetAttributes(SfxItemSet& rTargetSet, BOOL bOnlyHardAttr=FALSE) const;

    BOOL SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(BOOL& rOk) const;
    BOOL SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr);

    virtual void MakeVisible(const Rectangle& rRect, Window& rWin);

    // Fuer PlugIn. Wird vom Paint des OLE-Obj gerufen.
    virtual void DoConnect(SdrOle2Obj* pOleObj);

    // Animation aktivieren/deaktivieren fuer ::Paint
    // wird z.Zt. ausgewertet von SdrGrafObj, wenn in dem eine Animation steckt
    // Das Unterbinden der automatischen Animation wird z.B. fuer die Dia-Show benoetigt
    BOOL IsAnimationEnabled() const { return ( SDR_ANIMATION_ANIMATE == eAnimationMode ); }
    void SetAnimationEnabled( BOOL bEnable=TRUE );

    // set/unset pause state for animations
    bool IsAnimationPause() const { return bAnimationPause; }
    void SetAnimationPause( bool bSet );

    // Verhalten beim Starten von Animation im Paint-Handler:
    // 1. Animation normal starten( SDR_ANIMATION_ANIMATE ) => DEFAULT
    // 2. Nur die Ersatzdarstellung der Animation zeichnen ( SDR_ANIMATION_DONT_ANIMATE )
    // 3. Nicht starten und nichts ersatzweise ausgeben ( SDR_ANIMATION_DISABLE )
    void SetAnimationMode( const SdrAnimationMode eMode );
    SdrAnimationMode GetAnimationMode() const { return eAnimationMode; }

    // bei bShow=FALSE wird der Browser destruiert
#ifdef DBG_UTIL
    void ShowItemBrowser(BOOL bShow=TRUE);
    BOOL IsItemBrowserVisible() const { return pItemBrowser!=NULL && ((Window*)pItemBrowser)->IsVisible(); }
    Window* GetItemBrowser() const { return (Window*)pItemBrowser; }
#endif

    // Muss von App beim Scrollen usw. gerufen werden, damit ein u.U.
    // aktives FormularControl mitverschoben werden kann
    void VisAreaChanged(const OutputDevice* pOut=NULL);
    void VisAreaChanged(const SdrPageWindow& rWindow);

    BOOL IsPrintPreview() const { return bPrintPreview; }
    void SetPrintPreview(bool bOn = true) { bPrintPreview=bOn; }

    const svtools::ColorConfig& getColorConfig() const;

    virtual void onChangeColorConfig();

    // #103834# Set background color for svx at SdrPageViews
    void SetApplicationBackgroundColor(Color aBackgroundColor);

    // #103911# Set document color for svx at SdrPageViews
    void SetApplicationDocumentColor(Color aDocumentColor);

    /** Determine typical background color for given area on given page

        This method calculates a single representative color for the
        given rectangular area. This is accomplished by sampling from
        various points within that area, and calculating a weighted
        mean value of those samples. High contrast settings are
        respected by this method, i.e. in high contrast mode,
        svtools::DOCCOLOR is used as the background color.

        @attention For some cases, such as highly structured
        background or huge color ranges spanned by the background,
        this method cannot yield meaningful results, as the background
        simply cannot be represented by a single color. Beware.

        @param rArea
        Rectangular area to determine representative background color from

        @param rVisibleLayers
        Set of layers visible when calculating the background color

        @param rCurrPage
        Current page for which to determine background color on

        @return a representative background color.
     */
    Color CalcBackgroundColor( const Rectangle&     rArea,
                               const SetOfByte&     rVisibleLayers,
                               const SdrPage&   rCurrPage ) const;

    // #i38135#
    // Sets the timer for Object animations and restarts.
    void SetAnimationTimer(sal_uInt32 nTime);

    // access to Drawinglayer configuration options
    const SvtOptionsDrawinglayer& getOptionsDrawinglayer() const { return maDrawinglayerOpt; }
};

#endif //_SVDPNTV_HXX

