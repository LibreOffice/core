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
// Helper Class SdrObjRefList
//************************************************************

class SdrObjRefList
{
    Container                   aList;
public:

    SdrObjRefList()
    :   aList(1024,64,64)
    {}

    void Clear() { aList.Clear(); }
    sal_uLong GetObjCount() const { return aList.Count(); }
    SdrObject* GetObj(sal_uLong nNum) const { return (SdrObject*)aList.GetObject(nNum); }
    SdrObject* operator[](sal_uLong nNum) const { return (SdrObject*)aList.GetObject(nNum); }
    void InsertObject(SdrObject* pObj, sal_uLong nPos=CONTAINER_APPEND) { aList.Insert(pObj,nPos); }
    void RemoveObject(sal_uLong nPos) { aList.Remove(nPos); }
};

//************************************************************
// Helper Class ImpSdrGDIMetaFileImport
//************************************************************

class ImpSdrGDIMetaFileImport
{
protected:
    SdrObjRefList               aTmpList;
    VirtualDevice               aVD;
    Rectangle                   aScaleRect;
    sal_uLong                       nMapScalingOfs; // from here on, not edited with MapScaling
    SfxItemSet*                 pLineAttr;
    SfxItemSet*                 pFillAttr;
    SfxItemSet*                 pTextAttr;
    SdrPage*                    pPage;
    SdrModel*                   pModel;
    SdrLayerID                  nLayer;
    Color                       aOldLineColor;
    sal_Int32                   nLineWidth;
    basegfx::B2DLineJoin        maLineJoin;
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

protected:
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
    void DoAction(MetaClipRegionAction      & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaRasterOpAction        & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaPushAction            & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaPopAction             & rAct) { rAct.Execute(&aVD); bFntDirty=sal_True; }
    void DoAction(MetaMoveClipRegionAction  & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaISectRectClipRegionAction& rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaISectRegionClipRegionAction& rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaCommentAction& rAct, GDIMetaFile* pMtf);
    void DoAction(MetaRenderGraphicAction& rAct);

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
