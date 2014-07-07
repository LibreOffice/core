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



#ifndef _SVDFMTF_HXX
#define _SVDFMTF_HXX

#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <svx/svdobj.hxx>

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SfxItemSet;
class SdrObjList;
class SdrModel;
class SdrPage;
class SdrObject;
class SvdProgressInfo;

//************************************************************
//   Hilfsklasse ImpSdrGDIMetaFileImport
//************************************************************

class ImpSdrGDIMetaFileImport
{
protected:
    ::std::vector< SdrObject* > maTmpList;
    VirtualDevice               maVD;
    Rectangle                   maScaleRect;
    sal_uLong                   mnMapScalingOfs; // ab hier nocht nicht mit MapScaling bearbeitet
    SfxItemSet*                 mpLineAttr;
    SfxItemSet*                 mpFillAttr;
    SfxItemSet*                 mpTextAttr;
    SdrModel*                   mpModel;
    SdrLayerID                  mnLayer;
    Color                       maOldLineColor;
    sal_Int32                   mnLineWidth;
    basegfx::B2DLineJoin        maLineJoin;
    com::sun::star::drawing::LineCap    maLineCap;
    XDash                       maDash;

    bool                        mbMov;
    bool                        mbSize;
    Point                       maOfs;
    double                      mfScaleX;
    double                      mfScaleY;
    Fraction                    maScaleX;
    Fraction                    maScaleY;

    bool                        mbFntDirty;

    // fuer Optimierung von (PenNULL,Brush,DrawPoly),(Pen,BrushNULL,DrawPoly) -> aus 2 mach ein
    bool                        mbLastObjWasPolyWithoutLine;
    bool                        mbNoLine;
    bool                        mbNoFill;

    // fuer Optimierung mehrerer Linien zu einer Polyline
    bool                        mbLastObjWasLine;

    // clipregion
    basegfx::B2DPolyPolygon     maClip;

protected:
    // ckeck for clip and evtl. fill maClip
    void checkClip();
    bool isClip() const;

    // actions
    void DoAction(MetaPixelAction           & rAct);
    void DoAction(MetaPointAction           & rAct);
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
    void DoAction(MetaFillColorAction       & rAct) { rAct.Execute(&maVD); }
    void DoAction(MetaTextColorAction       & rAct) { rAct.Execute(&maVD); }
    void DoAction(MetaTextFillColorAction   & rAct) { rAct.Execute(&maVD); }
    void DoAction(MetaFontAction            & rAct) { rAct.Execute(&maVD); mbFntDirty = true; }
    void DoAction(MetaTextAlignAction       & rAct) { rAct.Execute(&maVD); mbFntDirty = true; }
    void DoAction(MetaClipRegionAction      & rAct) { rAct.Execute(&maVD); checkClip(); }
    void DoAction(MetaRasterOpAction        & rAct) { rAct.Execute(&maVD); }
    void DoAction(MetaPushAction            & rAct) { rAct.Execute(&maVD); checkClip(); }
    void DoAction(MetaPopAction             & rAct) { rAct.Execute(&maVD); mbFntDirty = true; checkClip(); }
    void DoAction(MetaMoveClipRegionAction  & rAct) { rAct.Execute(&maVD); checkClip(); }
    void DoAction(MetaISectRectClipRegionAction& rAct) { rAct.Execute(&maVD); checkClip(); }
    void DoAction(MetaISectRegionClipRegionAction& rAct) { rAct.Execute(&maVD); checkClip(); }

    // #i125211# The MetaCommentAction needs to advance (if used), thus
    // give current metafile and index which may be changed
    void DoAction(MetaCommentAction& rAct, GDIMetaFile& rMtf, sal_uLong& a);

    // missing actions added
    void DoAction(MetaTextRectAction& rAct);
    void DoAction(MetaBmpScalePartAction& rAct);
    void DoAction(MetaBmpExScalePartAction& rAct);
    void DoAction(MetaMaskAction& rAct);
    void DoAction(MetaMaskScaleAction& rAct);
    void DoAction(MetaMaskScalePartAction& rAct);
    void DoAction(MetaGradientAction& rAct);
    void DoAction(MetaWallpaperAction& rAct);
    void DoAction(MetaTransparentAction& rAct);
    void DoAction(MetaEPSAction& rAct);
    void DoAction(MetaRefPointAction& rAct)  { rAct.Execute(&maVD); }
    void DoAction(MetaTextLineColorAction& rAct)  { rAct.Execute(&maVD); mbFntDirty = true; }
    void DoAction(MetaTextLineAction& rAct);
    void DoAction(MetaFloatTransparentAction& rAct);
    void DoAction(MetaGradientExAction& rAct);
    void DoAction(MetaLayoutModeAction& rAct)  { rAct.Execute(&maVD); mbFntDirty = true; }
    void DoAction(MetaTextLanguageAction& rAct)  { rAct.Execute(&maVD); mbFntDirty = true; }
    void DoAction(MetaOverlineColorAction& rAct)  { rAct.Execute(&maVD); mbFntDirty = true; }

    void ImportText(const Point& rPos, const XubString& rStr, const MetaAction& rAct);
    void SetAttributes(SdrObject* pObj, bool bForceTextAttr = false);
    void InsertObj(SdrObject* pObj, bool bScale = true);
    void MapScaling();

    // #i73407# reformulation to use new B2DPolygon classes
    bool CheckLastLineMerge(const basegfx::B2DPolygon& rSrcPoly);
    bool CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon& rPolyPolygon);

    void DoLoopActions(GDIMetaFile& rMtf, SvdProgressInfo* pProgrInfo, sal_uInt32* pActionsToReport);

public:
    ImpSdrGDIMetaFileImport(
        SdrModel& rModel,
        SdrLayerID nLay,
        const Rectangle& rRect);
    ~ImpSdrGDIMetaFileImport();

    sal_uInt32 DoImport(
        const GDIMetaFile& rMtf,
        SdrObjList& rDestList,
        sal_uInt32 nInsPos = CONTAINER_APPEND,
        SvdProgressInfo* pProgrInfo = 0);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDFMTF_HXX
// eof
