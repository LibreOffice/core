/*************************************************************************
 *
 *  $RCSfile: svdfmtf.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVDFMTF_HXX
#define _SVDFMTF_HXX

#ifdef VCL
#ifndef _SV_METAACT_HXX //autogen
#include <vcl/metaact.hxx>
#endif
#else // VCL
#ifndef _SVMTFACT_HXX
#include <svmtfact.hxx>
#endif
#endif // VCL

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
    FASTBOOL                    bPenDirty;
    FASTBOOL                    bBrsDirty;
    FASTBOOL                    bFntDirty;

    // fuer Optimierung von (PenNULL,Brush,DrawPoly),(Pen,BrushNULL,DrawPoly) -> aus 2 mach ein
    FASTBOOL                    bLastObjWasPolyWithoutLine;
    FASTBOOL                    bNoLine;
    FASTBOOL                    bNoFill;

    // fuer Optimierung mehrerer Linien zu einer Polyline
    FASTBOOL                    bLastObjWasLine;

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
    void DoAction(MetaFillColorAction       & rAct) { rAct.Execute(&aVD); bBrsDirty=TRUE; }
    void DoAction(MetaTextColorAction       & rAct) { rAct.Execute(&aVD); bBrsDirty=TRUE; }
    void DoAction(MetaTextFillColorAction   & rAct) { rAct.Execute(&aVD); bBrsDirty=TRUE; }
    void DoAction(MetaFontAction            & rAct) { rAct.Execute(&aVD); bFntDirty=TRUE; }
    void DoAction(MetaClipRegionAction      & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaRasterOpAction        & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaPushAction            & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaPopAction             & rAct) { rAct.Execute(&aVD); bPenDirty=TRUE; bBrsDirty=TRUE; bFntDirty=TRUE; }
    void DoAction(MetaMoveClipRegionAction  & rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaISectRectClipRegionAction& rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaISectRegionClipRegionAction& rAct) { rAct.Execute(&aVD); }
    void DoAction(MetaCommentAction& rAct, GDIMetaFile* pMtf);

    void ImportText(const Point& rPos, const XubString& rStr);
    void SetAttributes(SdrObject* pObj, FASTBOOL bForceTextAttr=FALSE);
    void InsertObj(SdrObject* pObj);
    void MapScaling();

    // Optimierung:
    FASTBOOL CheckLastLineMerge(const XPolygon& rSrcPoly);
    FASTBOOL CheckLastPolyLineAndFillMerge(const XPolyPolygon& rPoly);

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

