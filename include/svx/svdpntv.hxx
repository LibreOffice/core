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

#ifndef INCLUDED_SVX_SVDPNTV_HXX
#define INCLUDED_SVX_SVDPNTV_HXX

#include <svl/SfxBroadcaster.hxx>
#include <svl/lstner.hxx>
#include <svl/undo.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdlayer.hxx>
#include <vcl/window.hxx>
#include <svtools/colorcfg.hxx>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <svl/itemset.hxx>
#include <vcl/timer.hxx>
#include <svx/svxdllapi.h>
#include <svtools/optionsdrawinglayer.hxx>
#include <unotools/options.hxx>
#include <vcl/idle.hxx>
#include <memory>


// Pre defines
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
enum class GraphicManagerDrawFlags;

#ifdef DBG_UTIL
class SdrItemBrowser;
#endif

namespace sdr { namespace contact {
    class ViewObjectContactRedirector;
}}


// Defines for AnimationMode
enum class SdrAnimationMode
{
    Animate,
    Disable
};


class SdrPaintView;

namespace sdr
{
    namespace contact
    {
        class ViewObjectContactRedirector;
    }
}


class SVX_DLLPUBLIC SvxViewChangedHint : public SfxHint
{
public:
    explicit SvxViewChangedHint();
};

class SdrPaintWindow;

/**
 * Helper to convert any GDIMetaFile to a good quality BitmapEx,
 * using default parameters and graphic::XPrimitive2DRenderer
 */
BitmapEx SVX_DLLPUBLIC convertMetafileToBitmapEx(
    const GDIMetaFile& rMtf,
    const basegfx::B2DRange& rTargetRange,
    const sal_uInt32 nMaximumQuadraticPixels);

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SdrPaintView
//      SdrSnapView
//          SdrMarkView
//              SdrEditView
//                  SdrPolyEditView
//                      SdrGlueEditView
//                          SdrObjEditView
//                              SdrExchangeView
//                                  SdrDragView
//                                      SdrCreateView
//                                          SdrView
//                                              DlgEdView
//                                              GraphCtrlView
//                                              E3dView
//                                                  DrawViewWrapper
//                                                  FmFormView
//                                                      ScDrawView
//                                                      sd::View (may have more?)
//                                                          sd::DrawView
//                                                      SwDrawView
//                                              OSectionView

class SVX_DLLPUBLIC SdrPaintView : public SfxListener, public SfxRepeatTarget, public SfxBroadcaster, public ::utl::ConfigurationListener
{
private:
    friend class                SdrPageView;
    friend class                SdrGrafObj;

    // the SdrModel this view was created with, unchanged during lifetime
    SdrModel&                   mrSdrModelFromSdrView;

    std::unique_ptr<SdrPageView> mpPageView;
protected:
    SdrModel*                   mpModel;
#ifdef DBG_UTIL
    VclPtr<SdrItemBrowser>      mpItemBrowser;
#endif
    VclPtr<OutputDevice>        mpActualOutDev; // Only for comparison
    VclPtr<OutputDevice>        mpDragWin;
    SfxStyleSheet*              mpDefaultStyleSheet;

    OUString                    maActualLayer;     // Current drawing layer
    OUString                    maMeasureLayer; // Current layer for measurements

//  Container                   aPagV;         // List of SdrPageViews

    // All windows this view is displayed on
    std::vector< std::unique_ptr<SdrPaintWindow> >  maPaintWindows;

    Size                        maGridBig;   // FIXME: We need to get rid of this eventually
    Size                        maGridFin;   // FIXME: We need to get rid of this eventually
    SdrDragStat                 maDragStat;
    tools::Rectangle            maMaxWorkArea;
    SfxItemSet                  maDefaultAttr;
    Idle                        maComeBackIdle;

    SdrAnimationMode            meAnimationMode;

    sal_uInt16                  mnHitTolPix;
    sal_uInt16                  mnMinMovPix;
    sal_uInt16                  mnHitTolLog;
    sal_uInt16                  mnMinMovLog;

    // Hold an incarnation of Drawinglayer configuration options
    SvtOptionsDrawinglayer const maDrawinglayerOpt;

    bool                        mbPageVisible : 1;
    bool                        mbPageShadowVisible : 1;
    bool                        mbPageBorderVisible : 1;
    bool                        mbBordVisible : 1;
    bool                        mbGridVisible : 1;
    bool                        mbGridFront : 1;
    bool                        mbHlplVisible : 1;
    bool                        mbHlplFront : 1;
    bool                        mbGlueVisible : 1;    // Persistent; show glue points
    bool                        mbGlueVisible2 : 1;   // Also show glue points for GluePointEdit
    bool                        mbGlueVisible3 : 1;   // Also show glue points for EdgeTool
    bool                        mbGlueVisible4 : 1;   // Show glue points, if one edge is selected
    bool                        mbSomeObjChgdFlag : 1;
    bool                        mbSwapAsynchron : 1;
    bool                        mbPrintPreview : 1;

    // These bools manage, the status that is displayed
    //
    bool                        mbAnimationPause : 1;

    // Flag which decides if buffered output for this view is allowed. When
    // set, PreRendering for PageView rendering will be used. Default is sal_False
    bool                        mbBufferedOutputAllowed : 1;

    // Flag which decides if buffered overlay for this view is allowed. When
    // set, the output will be buffered in an overlay vdev. When not, overlay is
    // directly painted to OutDev. Default is sal_False.
    bool                        mbBufferedOverlayAllowed : 1;

    // Allow page painting at all?
    bool                        mbPagePaintingAllowed : 1;

    // Is this a preview renderer?
    bool                        mbPreviewRenderer : 1;

    // Flags for calc and sw for suppressing OLE, CHART or DRAW objects
    bool                        mbHideOle : 1;
    bool                        mbHideChart : 1;
    bool                        mbHideDraw : 1;             // hide draw objects other than form controls
    bool                        mbHideFormControl : 1;      // hide form controls only

public:
    // Interface for PagePaintingAllowed flag
    bool IsBufferedOutputAllowed() const;
    void SetBufferedOutputAllowed(bool bNew);

    // Interface for BufferedOverlayAllowed flag
    bool IsBufferedOverlayAllowed() const;
    void SetBufferedOverlayAllowed(bool bNew);

    // Allow page painting at all?
    bool IsPagePaintingAllowed() const { return mbPagePaintingAllowed;}
    void SetPagePaintingAllowed(bool bNew);

protected:
    svtools::ColorConfig            maColorConfig;
    Color                           maGridColor;

    // Interface to SdrPaintWindow
    void DeletePaintWindow(SdrPaintWindow& rOld);
    void ConfigurationChanged( ::utl::ConfigurationBroadcaster*, ConfigurationHints ) override;

public:
    sal_uInt32 PaintWindowCount() const { return maPaintWindows.size(); }
    SdrPaintWindow* FindPaintWindow(const OutputDevice& rOut) const;
    SdrPaintWindow* GetPaintWindow(sal_uInt32 nIndex) const;
    // Replacement for GetWin(0), may return 0L (!)
    OutputDevice* GetFirstOutputDevice() const;

private:
    SVX_DLLPRIVATE void ImpClearVars();
    DECL_LINK(ImpComeBackHdl, Timer*, void);

protected:
    sal_uInt16 ImpGetMinMovLogic(short nMinMov, const OutputDevice* pOut) const;
    sal_uInt16 ImpGetHitTolLogic(short nHitTol, const OutputDevice* pOut) const;

    // If one does not want to wait for the IdleState of the system (cheated as const)
    void FlushComeBackTimer() const;
    void TheresNewMapMode();
    void ImpSetGlueVisible2(bool bOn) { if (mbGlueVisible2!=bOn) { mbGlueVisible2=bOn; if (!mbGlueVisible && !mbGlueVisible3 && !mbGlueVisible4) GlueInvalidate(); } }
    void ImpSetGlueVisible3(bool bOn) { if (mbGlueVisible3!=bOn) { mbGlueVisible3=bOn; if (!mbGlueVisible && !mbGlueVisible2 && !mbGlueVisible4) GlueInvalidate(); } }
    void ImpSetGlueVisible4(bool bOn) { if (mbGlueVisible4!=bOn) { mbGlueVisible4=bOn; if (!mbGlueVisible && !mbGlueVisible2 && !mbGlueVisible3) GlueInvalidate(); } }

public:
    bool ImpIsGlueVisible() { return mbGlueVisible || mbGlueVisible2 || mbGlueVisible3 || mbGlueVisible4; }

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;
    void GlueInvalidate() const;

    // ModelHasChanged is called, as soon as the system is idle again after many SdrHintKind::ObjectChange.
    //
    // Any sub-class override this method, MUST call the base class' ModelHasChanged() method
    virtual void ModelHasChanged();

    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    // A SdrView always needs a SdrModel for lifetime (Pool, ...)
    SdrPaintView(SdrModel& rSdrModel, OutputDevice* pOut);
    virtual ~SdrPaintView() override;

public:
    // SdrModel access on SdrView level
    SdrModel& getSdrModelFromSdrView() const { return mrSdrModelFromSdrView; }

    virtual void ClearPageView();
    SdrModel* GetModel() const { return mpModel; }

    virtual bool IsAction() const;
    virtual void MovAction(const Point& rPnt);
    virtual void EndAction();
    virtual void BckAction();
    virtual void BrkAction(); // Cancel all Actions (e.g. cancel dragging)
    virtual void TakeActionRect(tools::Rectangle& rRect) const;

    // Info about TextEdit. Default is sal_False.
    virtual bool IsTextEdit() const;

    // Must be called for every Window change as well as MapMode (Scaling) change:
    // If the SdrView is shown in multiple windows at the same time (e.g.
    // using the split pane), so that I can convert my pixel values to logical
    // values.
    void SetActualWin(const OutputDevice* pWin);
    void SetMinMoveDistancePixel(sal_uInt16 nVal) { mnMinMovPix=nVal; TheresNewMapMode(); }
    void SetHitTolerancePixel(sal_uInt16 nVal) { mnHitTolPix=nVal; TheresNewMapMode(); }
    sal_uInt16 GetHitTolerancePixel() const { return mnHitTolPix; }

    // Data read access on logic HitTolerance and MinMoveTolerance
    sal_uInt16 getHitTolLog() const { return mnHitTolLog; }

    // Using the DragState we can tell e.g. which distance was
    // already dragged
    const SdrDragStat& GetDragStat() const { return maDragStat; }

    // Registering/de-registering a PageView at a View
    //
    // The same Page cannot be registered multiple times.
    //
    // Methods ending in PgNum expect being passed a Page number.
    // Methods ending in PvNum, instead, expect the number of the
    // PageView at the SdrView (iterating over all registered Pages).
    virtual SdrPageView* ShowSdrPage(SdrPage* pPage);
    virtual void HideSdrPage();

    // Iterate over all registered PageViews
    SdrPageView* GetSdrPageView() const { return mpPageView.get(); }

    // A SdrView can be displayed on multiple Windows at the same time
    virtual void AddWindowToPaintView(OutputDevice* pNewWin, vcl::Window* pWindow);
    virtual void DeleteWindowFromPaintView(OutputDevice* pOldWin);

    void SetLayerVisible(const OUString& rName, bool bShow);
    bool IsLayerVisible(const OUString& rName) const;

    void SetLayerLocked(const OUString& rName, bool bLock=true);
    bool IsLayerLocked(const OUString& rName) const;

    void SetLayerPrintable(const OUString& rName, bool bPrn);
    bool IsLayerPrintable(const OUString& rName) const;

    // PrePaint call forwarded from app windows
    void PrePaint();


    // Used internally for Draw/Impress/sch/chart2
    virtual void CompleteRedraw(OutputDevice* pOut, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = nullptr);

    // #i72889# used from CompleteRedraw() implementation internally, added to be able to do a complete redraw in single steps

    // BeginCompleteRedraw returns (or even creates) a SdrPaintWindow which will then be used as the
    // target for paints. Since paints may be buffered, use its GetTargetOutputDevice() method which will
    // return the buffer in case it's buffered.
    //
    // DoCompleteRedraw then draws the DrawingLayer hierarchy
    // EndCompleteRedraw does the necessary refreshes, paints text edit and overlay as well as destroys the
    // SdrPaintWindow again, if needed.
    // This means: the SdrPaintWindow is no longer safe after this closing call.
    virtual SdrPaintWindow* BeginCompleteRedraw(OutputDevice* pOut);
    void DoCompleteRedraw(SdrPaintWindow& rPaintWindow, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = nullptr);
    virtual void EndCompleteRedraw(SdrPaintWindow& rPaintWindow, bool bPaintFormLayer);


    // Used for the other applications basctl/sc/sw which call DrawLayer at PageViews
    // #i74769# Interface change to use common BeginCompleteRedraw/EndCompleteRedraw
    // #i76114# bDisableIntersect disables intersecting rReg with the Window's paint region
    SdrPaintWindow* BeginDrawLayers(OutputDevice* pOut, const vcl::Region& rReg, bool bDisableIntersect = false);

    // Used when the region passed to BeginDrawLayers needs to be changed
    void UpdateDrawLayersRegion(OutputDevice* pOut, const vcl::Region& rReg);
    void EndDrawLayers(SdrPaintWindow& rPaintWindow, bool bPaintFormLayer);

protected:

    // Used to paint the form layer after the PreRender device is flushed (painted) to the window.
    void ImpFormLayerDrawing( SdrPaintWindow& rPaintWindow );

    static vcl::Region OptimizeDrawLayersRegion(OutputDevice* pOut, const vcl::Region& rReg, bool bDisableIntersect);

public:
    /// Draw Page as a white area or not
    bool IsPageVisible() const { return mbPageVisible; }

    /// Draw Page shadow or not
    bool IsPageShadowVisible() const { return mbPageShadowVisible; }

    /// Draw Page as a white area or not
    bool IsPageBorderVisible() const { return mbPageBorderVisible; }

    /// Draw Border line or not
    bool IsBordVisible() const { return mbBordVisible; }

    /// Draw Grid or not
    bool IsGridVisible() const { return mbGridVisible; }

    /// Draw Grid in front of objects or behind them
    bool IsGridFront() const { return mbGridFront  ; }

    /// Draw Help line of the Page or not
    bool IsHlplVisible() const { return mbHlplVisible; }

    /// Draw Help line in front of the objects or behind them
    bool IsHlplFront() const { return mbHlplFront  ; }

    const Color& GetGridColor() const { return maGridColor;}
    void SetPageVisible(bool bOn = true) { mbPageVisible=bOn; InvalidateAllWin(); }
    void SetPageShadowVisible(bool bOn) { mbPageShadowVisible=bOn; InvalidateAllWin(); }
    void SetPageBorderVisible(bool bOn = true) { mbPageBorderVisible=bOn; InvalidateAllWin(); }
    void SetBordVisible(bool bOn = true) { mbBordVisible=bOn; InvalidateAllWin(); }
    void SetGridVisible(bool bOn) { mbGridVisible=bOn; InvalidateAllWin(); }
    void SetGridFront(bool bOn) { mbGridFront  =bOn; InvalidateAllWin(); }
    void SetHlplVisible(bool bOn = true) { mbHlplVisible=bOn; InvalidateAllWin(); }
    void SetHlplFront(bool bOn) { mbHlplFront  =bOn; InvalidateAllWin(); }
    void SetGlueVisible(bool bOn = true) { if (mbGlueVisible!=bOn) { mbGlueVisible=bOn; if (!mbGlueVisible2 && !mbGlueVisible3 && !mbGlueVisible4) GlueInvalidate(); } }

    bool IsPreviewRenderer() const { return mbPreviewRenderer; }
    void SetPreviewRenderer(bool bOn) { mbPreviewRenderer=bOn; }

    // Access methods for calc and sw hide object modes
    bool getHideOle() const { return mbHideOle; }
    bool getHideChart() const { return mbHideChart; }
    bool getHideDraw() const { return mbHideDraw; }
    bool getHideFormControl() const { return mbHideFormControl; }
    void setHideOle(bool bNew) { if(bNew != mbHideOle) mbHideOle = bNew; }
    void setHideChart(bool bNew) { if(bNew != mbHideChart) mbHideChart = bNew; }
    void setHideDraw(bool bNew) { if(bNew != mbHideDraw) mbHideDraw = bNew; }
    void setHideFormControl(bool bNew) { if(bNew != mbHideFormControl) mbHideFormControl = bNew; }

    void SetGridCoarse(const Size& rSiz) { maGridBig=rSiz; }
    void SetGridFine(const Size& rSiz) {
        maGridFin=rSiz;
        if (maGridFin.Height()==0) maGridFin.setHeight(maGridFin.Width());
        if (mbGridVisible) InvalidateAllWin();
    }
    const Size& GetGridCoarse() const { return maGridBig; }
    const Size& GetGridFine() const { return maGridFin; }

    void InvalidateAllWin();
    void InvalidateAllWin(const tools::Rectangle& rRect);

    /// If the View should not call Invalidate() on the windows, override
    /// the following 2 methods and do something else.
    virtual void InvalidateOneWin(vcl::Window& rWin);
    virtual void InvalidateOneWin(vcl::Window& rWin, const tools::Rectangle& rRect);

    void SetActiveLayer(const OUString& rName) { maActualLayer=rName; }
    const OUString&  GetActiveLayer() const { return maActualLayer; }

    /// Leave an object group of all visible Pages (like `chdir ..` in MSDOS)
    void LeaveOneGroup();

    /// Leave all entered object groups of all visible Pages (like `chdir \` in MSDOS)
    void LeaveAllGroup();

    /// Determine, whether Leave is useful or not
    bool IsGroupEntered() const;

    /// Default attributes at the View
    /// Newly created objects are assigned these attributes by default when they are created.
    void SetDefaultAttr(const SfxItemSet& rAttr, bool bReplaceAll);
    const SfxItemSet& GetDefaultAttr() const { return maDefaultAttr; }
    void SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);

    void SetNotPersistDefaultAttr(const SfxItemSet& rAttr);
    void MergeNotPersistDefaultAttr(SfxItemSet& rAttr) const;

    /// Execute a swap-in of e.g. graphics asynchronously.
    /// This does not reload all graphics like Paint does, but kicks off
    /// the loading there. When such an object is done loading, it is displayed.
    /// TODO: Only works at the moment, if SwapGraphics is enabled in the model.
    /// The default = false flag is non-persistent
    bool IsSwapAsynchron() const { return mbSwapAsynchron; }
    void SetSwapAsynchron(bool bJa=true) { mbSwapAsynchron=bJa; }
    virtual bool KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin);

    virtual bool MouseButtonDown(const MouseEvent& /*rMEvt*/, vcl::Window* /*pWin*/) { return false; }
    virtual bool MouseButtonUp(const MouseEvent& /*rMEvt*/, vcl::Window* /*pWin*/) { return false; }
    virtual bool MouseMove(const MouseEvent& /*rMEvt*/, vcl::Window* /*pWin*/) { return false; }
    virtual bool RequestHelp(const HelpEvent& /*rHEvt*/) { return false; }
    virtual bool Command(const CommandEvent& /*rCEvt*/, vcl::Window* /*pWin*/) { return false; }

    void GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const;

    void SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(bool& rOk) const;
    void SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);

    virtual void MakeVisible(const tools::Rectangle& rRect, vcl::Window& rWin);

    /// For Plugins
    /// Is called by the Paint of the OLE object
    virtual void DoConnect(SdrOle2Obj* pOleObj);

    /// Enable/disable animations for ::Paint
    /// Is used by e.g. SdrGrafObj, if it contains an animation
    /// Preventing automatic animation is needed for e.g. the presentation view
    bool IsAnimationEnabled() const { return ( SdrAnimationMode::Animate == meAnimationMode ); }
    void SetAnimationEnabled( bool bEnable=true );

    /// Set/unset pause state for animations
    void SetAnimationPause( bool bSet );

    /// Mode when starting an animation in the Paint Handler:
    /// 1. SdrAnimationMode::Animate (default): start animation normally
    /// 2. SDR_ANIMATION_DONT_ANIMATE: only show the replacement picture
    /// 3. SdrAnimationMode::Disable: don't start and don't show any replacement
    void SetAnimationMode( const SdrAnimationMode eMode );

    /// The Browser is destroyed for bShow=false
#ifdef DBG_UTIL
    void ShowItemBrowser(bool bShow);
    bool IsItemBrowserVisible() const { return mpItemBrowser!=nullptr && GetItemBrowser()->IsVisible(); }
    vcl::Window* GetItemBrowser() const;
#endif

    /// Must be called by the App when scrolling etc. in order for
    /// an active FormControl to be moved too
    void VisAreaChanged(const OutputDevice* pOut);
    void VisAreaChanged();

    bool IsPrintPreview() const { return mbPrintPreview; }
    void SetPrintPreview(bool bOn = true) { mbPrintPreview=bOn; }

    const svtools::ColorConfig& getColorConfig() const { return maColorConfig;}

    void onChangeColorConfig();

    // #103834# Set background color for svx at SdrPageViews
    void SetApplicationBackgroundColor(Color aBackgroundColor);

    // #103911# Set document color for svx at SdrPageViews
    void SetApplicationDocumentColor(Color aDocumentColor);

    // #i38135#
    // Sets the timer for Object animations and restarts.
    void SetAnimationTimer(sal_uInt32 nTime);

    // Access to Drawinglayer configuration options
    const SvtOptionsDrawinglayer& getOptionsDrawinglayer() const { return maDrawinglayerOpt; }
};

#endif // INCLUDED_SVX_SVDPNTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
