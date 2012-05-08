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

#ifndef _SVDFMTF_HXX
#define _SVDFMTF_HXX

#include <tools/contnr.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <svx/svdobj.hxx>

//************************************************************
// Forward Declarations
//************************************************************

class SfxItemSet;
class SdrObjList;
class SdrModel;
class SdrPage;
class SdrObject;
class SvdProgressInfo;

//************************************************************
// Helper Class ImpSdrGDIMetaFileImport
//************************************************************

class ImpSdrGDIMetaFileImport
{
protected:
    std::vector<SdrObject*>     aTmpList;
    VirtualDevice               aVD;
    Rectangle                   aScaleRect;
    size_t                      nMapScalingOfs; // from here on, not edited with MapScaling
    SfxItemSet*                 pLineAttr;
    SfxItemSet*                 pFillAttr;
    SfxItemSet*                 pTextAttr;
    SdrPage*                    pPage;
    SdrModel*                   pModel;
    SdrLayerID                  nLayer;
    Color                       aOldLineColor;
    sal_Int32                   nLineWidth;
    basegfx::B2DLineJoin        maLineJoin;
    com::sun::star::drawing::LineCap    maLineCap;
    XDash                       maDash;

    sal_Bool                    bMov;
    sal_Bool                    bSize;
    Point                       aOfs;
    double                      fScaleX;
    double                      fScaleY;
    Fraction                    aScaleX;
    Fraction                    aScaleY;

    sal_Bool                    bFntDirty;

    // to optimize (PenNULL,Brush,DrawPoly),(Pen,BrushNULL,DrawPoly) -> two-in-one
    sal_Bool                    bLastObjWasPolyWithoutLine;
    sal_Bool                    bNoLine;
    sal_Bool                    bNoFill;

    // to optimize multiple lines into a Polyline
    sal_Bool                    bLastObjWasLine;

    // clipregion
    basegfx::B2DPolyPolygon     maClip;

protected:
    // check for clip and evtl. fill maClip
    void checkClip();
    bool isClip() const;

    // actions
    void DoAction(MetaPixelAction           & rAct) const;
    void DoAction(MetaPointAction           & rAct) const;
    void DoAction(MetaLineAction            & rAct);
    void DoAction(MetaRectAction            & rAct);
    void DoAction(MetaRoundRectAction       & rAct);
    void DoAction(MetaEllipseAction         & rAct);
    void DoAction(MetaArcAction             & rAct);
    void DoAction(MetaPieAction             & rAct);
    void DoAction(MetaChordAction           & rAct);
    void DoAction(MetaPolyLineAction        & rAct);
    void DoAction(MetaPolygonAction         & rAct);
    void DoAction(MetaPolyPolygonAction     & rAct);
    void DoAction(MetaTextAction            & rAct);
    void DoAction(MetaTextArrayAction       & rAct);
    void DoAction(MetaStretchTextAction     & rAct);
    void DoAction(MetaBmpAction             & rAct);
    void DoAction(MetaBmpScaleAction        & rAct);
    void DoAction(MetaBmpExAction           & rAct);
    void DoAction(MetaBmpExScaleAction      & rAct);
    void DoAction(MetaHatchAction           & rAct);
    void DoAction(MetaLineColorAction       & rAct);
    void DoAction(MetaMapModeAction         & rAct);
    void DoAction(MetaFillColorAction       & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaTextColorAction       & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaTextFillColorAction   & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaFontAction            & rAct) { rAct.Execute(&aVD); bFntDirty=sal_True; }
    void DoAction(MetaTextAlignAction       & rAct) { rAct.Execute(&aVD); bFntDirty=sal_True; }
    void DoAction(MetaClipRegionAction      & rAct) { rAct.Execute(&aVD); checkClip(); }
    void DoAction(MetaRasterOpAction        & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaPushAction            & rAct) { rAct.Execute(&aVD); checkClip(); }
    void DoAction(MetaPopAction             & rAct) { rAct.Execute(&aVD); bFntDirty=sal_True; checkClip(); }
    void DoAction(MetaMoveClipRegionAction  & rAct) { rAct.Execute(&aVD); checkClip(); }
    void DoAction(MetaISectRectClipRegionAction& rAct) { rAct.Execute(&aVD); checkClip(); }
    void DoAction(MetaISectRegionClipRegionAction& rAct) { rAct.Execute(&aVD); checkClip(); }
    void DoAction(MetaCommentAction& rAct, GDIMetaFile* pMtf);

    void ImportText( const Point& rPos, const XubString& rStr, const MetaAction& rAct );
    void SetAttributes(SdrObject* pObj, bool bForceTextAttr = false);
    void InsertObj( SdrObject* pObj, sal_Bool bScale = sal_True );
    void MapScaling();

    // #i73407# reformulation to use new B2DPolygon classes
    bool CheckLastLineMerge(const basegfx::B2DPolygon& rSrcPoly);
    bool CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon& rPolyPolygon);

public:
    ImpSdrGDIMetaFileImport(SdrModel& rModel);
    ~ImpSdrGDIMetaFileImport();
    sal_uLong DoImport(const GDIMetaFile& rMtf, SdrObjList& rDestList, sal_uLong nInsPos=CONTAINER_APPEND, SvdProgressInfo *pProgrInfo = NULL);
    void SetLayer(SdrLayerID nLay) { nLayer=nLay; }
    SdrLayerID GetLayer() const { return nLayer; }
    void SetScaleRect(const Rectangle& rRect) { aScaleRect=rRect; }
    const Rectangle& GetScaleRect() const { return aScaleRect; }
};

#endif //_SVDFMTF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
