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

#ifndef INCLUDED_SVX_SOURCE_SVDRAW_SVDFMTF_HXX
#define INCLUDED_SVX_SOURCE_SVDRAW_SVDFMTF_HXX

#include <sal/config.h>

#include <memory>

#include <svl/itemset.hxx>
#include <tools/fract.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <svx/svdobj.hxx>
#include <svx/xdash.hxx>

// Forward Declarations


class SdrObjList;
class SdrModel;
class SvdProgressInfo;


// Helper Class ImpSdrGDIMetaFileImport
class ImpSdrGDIMetaFileImport final
{
    ::std::vector< rtl::Reference<SdrObject> > maTmpList;
    ScopedVclPtr<VirtualDevice> mpVD;
    MapMode maPrefMapMode;
    tools::Rectangle            maScaleRect;
    std::unique_ptr<SfxItemSet> mpLineAttr;
    std::unique_ptr<SfxItemSet> mpFillAttr;
    std::unique_ptr<SfxItemSet> mpTextAttr;
    SdrModel*                   mpModel;
    SdrLayerID                  mnLayer;
    Color                       maOldLineColor;
    sal_Int32                   mnLineWidth;
    basegfx::B2DLineJoin        maLineJoin;
    css::drawing::LineCap       maLineCap;
    XDash                       maDash;

    bool                        mbMov;
    bool                        mbSize;
    Point                       maOfs;
    double                      mfScaleX;
    double                      mfScaleY;
    Fraction                    maScaleX;
    Fraction                    maScaleY;

    bool                        mbFntDirty;

    // to optimize (PenNULL,Brush,DrawPoly),(Pen,BrushNULL,DrawPoly) -> two-in-one
    bool                        mbLastObjWasPolyWithoutLine;
    bool                        mbNoLine;
    bool                        mbNoFill;

    // to optimize multiple lines into a Polyline
    bool                        mbLastObjWasLine;

    // clipregion
    basegfx::B2DPolyPolygon     maClip;

    // check for clip and evtl. fill maClip
    void checkClip();
    bool isClip() const;

    // actions
    void DoAction(MetaLineAction const      & rAct);
    void DoAction(MetaRectAction const      & rAct);
    void DoAction(MetaRoundRectAction const & rAct);
    void DoAction(MetaEllipseAction const   & rAct);
    void DoAction(MetaArcAction const       & rAct);
    void DoAction(MetaPieAction const       & rAct);
    void DoAction(MetaChordAction const     & rAct);
    void DoAction(MetaPolyLineAction const  & rAct);
    void DoAction(MetaPolygonAction const   & rAct);
    void DoAction(MetaPolyPolygonAction const & rAct);
    void DoAction(MetaTextAction const      & rAct);
    void DoAction(MetaTextArrayAction const & rAct);
    void DoAction(MetaStretchTextAction const & rAct);
    void DoAction(MetaBmpAction const       & rAct);
    void DoAction(MetaBmpScaleAction const  & rAct);
    void DoAction(MetaBmpExAction const     & rAct);
    void DoAction(MetaBmpExScaleAction const & rAct);
    void DoAction(MetaHatchAction const     & rAct);
    void DoAction(MetaLineColorAction       & rAct);
    void DoAction(MetaMapModeAction         & rAct);
    void DoAction(MetaFillColorAction       & rAct) { rAct.Execute(mpVD); }
    void DoAction(MetaTextColorAction       & rAct) { rAct.Execute(mpVD); }
    void DoAction(MetaTextFillColorAction   & rAct) { rAct.Execute(mpVD); }
    void DoAction(MetaFontAction            & rAct) { rAct.Execute(mpVD); mbFntDirty = true; }
    void DoAction(MetaTextAlignAction       & rAct) { rAct.Execute(mpVD); mbFntDirty = true; }
    void DoAction(MetaClipRegionAction      & rAct) { rAct.Execute(mpVD); checkClip(); }
    void DoAction(MetaRasterOpAction        & rAct) { rAct.Execute(mpVD); }
    void DoAction(MetaPushAction            & rAct) { rAct.Execute(mpVD); checkClip(); }
    void DoAction(MetaPopAction             & rAct) { rAct.Execute(mpVD); mbFntDirty = true; checkClip(); }
    void DoAction(MetaMoveClipRegionAction  & rAct) { rAct.Execute(mpVD); checkClip(); }
    void DoAction(MetaISectRectClipRegionAction& rAct) { rAct.Execute(mpVD); checkClip(); }
    void DoAction(MetaISectRegionClipRegionAction& rAct) { rAct.Execute(mpVD); checkClip(); }

    // #i125211# The MetaCommentAction needs to advance (if used), thus
    // give current metafile and index which may be changed
    void DoAction(MetaCommentAction const & rAct, GDIMetaFile const & rMtf, size_t& a);

    // missing actions added
    void DoAction(MetaTextRectAction const & rAct);
    void DoAction(MetaBmpScalePartAction const & rAct);
    void DoAction(MetaBmpExScalePartAction const & rAct);
    void DoAction(MetaMaskAction const & rAct);
    void DoAction(MetaMaskScaleAction const & rAct);
    void DoAction(MetaMaskScalePartAction const & rAct);
    void DoAction(MetaGradientAction const & rAct);
    void DoAction(MetaTransparentAction const & rAct);
    void DoAction(MetaRefPointAction& rAct)  { rAct.Execute(mpVD); }
    void DoAction(MetaTextLineColorAction& rAct)  { rAct.Execute(mpVD); mbFntDirty = true; }
    void DoAction(MetaFloatTransparentAction const & rAct);
    void DoAction(MetaGradientExAction const & rAct);
    void DoAction(MetaLayoutModeAction& rAct)  { rAct.Execute(mpVD); mbFntDirty = true; }
    void DoAction(MetaTextLanguageAction& rAct)  { rAct.Execute(mpVD); mbFntDirty = true; }
    void DoAction(MetaOverlineColorAction& rAct)  { rAct.Execute(mpVD); mbFntDirty = true; }

    void ImportText(const Point& rPos, const OUString& rStr, const MetaAction& rAct);
    void SetAttributes(SdrObject* pObj, bool bForceTextAttr = false);
    void InsertObj(SdrObject* pObj, bool bScale = true);

    // #i73407# reformulation to use new B2DPolygon classes
    bool CheckLastLineMerge(const basegfx::B2DPolygon& rSrcPoly);
    bool CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon& rPolyPolygon);

    void DoLoopActions(GDIMetaFile const & rMtf, SvdProgressInfo* pProgrInfo, sal_uInt32* pActionsToReport);

    // map from current virtual device mapmode to original mapmode
    Size implMap(const Size& rSz) const;
    Point implMap(const Point& rPt) const;
    basegfx::B2DHomMatrix implMapMatrix() const;

    // Copy assignment is forbidden and not implemented.
    ImpSdrGDIMetaFileImport (const ImpSdrGDIMetaFileImport &) = delete;
    ImpSdrGDIMetaFileImport & operator= (const ImpSdrGDIMetaFileImport &) = delete;

public:
    ImpSdrGDIMetaFileImport(
        SdrModel& rModel,
        SdrLayerID nLay,
        const tools::Rectangle& rRect);

    size_t DoImport(
        const GDIMetaFile& rMtf,
        SdrObjList& rDestList,
        size_t nInsPos,
        SvdProgressInfo* pProgrInfo = nullptr);
};

#endif // INCLUDED_SVX_SOURCE_SVDRAW_SVDFMTF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
