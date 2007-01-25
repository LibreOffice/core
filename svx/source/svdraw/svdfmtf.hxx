/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdfmtf.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-25 11:07:11 $
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

#ifndef _SVDFMTF_HXX
#define _SVDFMTF_HXX

#ifndef _SV_METAACT_HXX //autogen
#include <vcl/metaact.hxx>
#endif

#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include "svdobj.hxx"
#endif

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
//   Hilfsklasse SdrObjRefList
//************************************************************

class SdrObjRefList
{
    Container                   aList;
public:

    SdrObjRefList()
    :   aList(1024,64,64)
    {}

    void Clear() { aList.Clear(); }
    ULONG GetObjCount() const { return aList.Count(); }
    SdrObject* GetObj(ULONG nNum) const { return (SdrObject*)aList.GetObject(nNum); }
    SdrObject* operator[](ULONG nNum) const { return (SdrObject*)aList.GetObject(nNum); }
    void InsertObject(SdrObject* pObj, ULONG nPos=CONTAINER_APPEND) { aList.Insert(pObj,nPos); }
    void RemoveObject(ULONG nPos) { aList.Remove(nPos); }
};

//************************************************************
//   Hilfsklasse ImpSdrGDIMetaFileImport
//************************************************************

class ImpSdrGDIMetaFileImport
{
protected:
    SdrObjRefList               aTmpList;
    VirtualDevice               aVD;
    Rectangle                   aScaleRect;
    ULONG                       nMapScalingOfs; // ab hier nocht nicht mit MapScaling bearbeitet
    SfxItemSet*                 pLineAttr;
    SfxItemSet*                 pFillAttr;
    SfxItemSet*                 pTextAttr;
    SdrPage*                    pPage;
    SdrModel*                   pModel;
    SdrLayerID                  nLayer;
    Color                       aOldLineColor;
    sal_Int32                   nLineWidth;

    sal_Bool                    bMov;
    sal_Bool                    bSize;
    Point                       aOfs;
    double                      fScaleX;
    double                      fScaleY;
    Fraction                    aScaleX;
    Fraction                    aScaleY;

    sal_Bool                    bFntDirty;

    // fuer Optimierung von (PenNULL,Brush,DrawPoly),(Pen,BrushNULL,DrawPoly) -> aus 2 mach ein
    sal_Bool                    bLastObjWasPolyWithoutLine;
    sal_Bool                    bNoLine;
    sal_Bool                    bNoFill;

    // fuer Optimierung mehrerer Linien zu einer Polyline
    sal_Bool                    bLastObjWasLine;

protected:
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
    void DoAction(MetaFillColorAction       & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaTextColorAction       & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaTextFillColorAction   & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaFontAction            & rAct) { rAct.Execute(&aVD); bFntDirty=TRUE; }
    void DoAction(MetaTextAlignAction       & rAct) { rAct.Execute(&aVD); bFntDirty=TRUE; }
    void DoAction(MetaClipRegionAction      & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaRasterOpAction        & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaPushAction            & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaPopAction             & rAct) { rAct.Execute(&aVD); bFntDirty=TRUE; }
    void DoAction(MetaMoveClipRegionAction  & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaISectRectClipRegionAction& rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaISectRegionClipRegionAction& rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaCommentAction& rAct, GDIMetaFile* pMtf);

    void ImportText( const Point& rPos, const XubString& rStr, const MetaAction& rAct );
    void SetAttributes(SdrObject* pObj, FASTBOOL bForceTextAttr=FALSE);
    void InsertObj( SdrObject* pObj, sal_Bool bScale = sal_True );
    void MapScaling();

    // #i73407# reformulation to use new B2DPolygon classes
    bool CheckLastLineMerge(const basegfx::B2DPolygon& rSrcPoly);
    bool CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon& rPolyPolygon);

public:
    ImpSdrGDIMetaFileImport(SdrModel& rModel);
    ~ImpSdrGDIMetaFileImport();
    ULONG DoImport(const GDIMetaFile& rMtf, SdrObjList& rDestList, ULONG nInsPos=CONTAINER_APPEND, SvdProgressInfo *pProgrInfo = NULL);
    void SetLayer(SdrLayerID nLay) { nLayer=nLay; }
    SdrLayerID GetLayer() const { return nLayer; }
    void SetScaleRect(const Rectangle& rRect) { aScaleRect=rRect; }
    const Rectangle& GetScaleRect() const { return aScaleRect; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDFMTF_HXX
// eof
