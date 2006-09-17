/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: obj3d.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:57:03 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define ITEMID_COLOR            SID_ATTR_3D_LIGHTCOLOR

#include "svdstr.hrc"
#include "svdglob.hxx"

#ifndef _SVDVIEW_HXX
#include "svdview.hxx"
#endif

#ifndef _SVDATTR_HXX
#include "svdattr.hxx"
#endif

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif

#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _CAMERA3D_HXX
#include "camera3d.hxx"
#endif

#ifndef _E3D_VOLMRK3D_HXX
#include "volmrk3d.hxx"
#endif

#ifndef _E3D_SCENE3D_HXX
#include "scene3d.hxx"
#endif

#ifndef _E3D_POLYSC3D_HXX
#include "polysc3d.hxx"
#endif

#ifndef _E3D_CUBE3D_HXX
#include "cube3d.hxx"
#endif

#ifndef _E3D_LATHE3D_HXX
#include "lathe3d.hxx"
#endif

#ifndef _E3D_SPHERE3D_HXX
#include "sphere3d.hxx"
#endif

#ifndef _E3D_EXTRUD3D_HXX
#include "extrud3d.hxx"
#endif

#ifndef _E3D_OBJ3D_HXX
#include "obj3d.hxx"
#endif

#ifndef _XOUTX_HXX
#include "xoutx.hxx"
#endif

#ifndef _XTABLE_HXX
#include "xtable.hxx"
#endif

#ifndef _SVX_XFLCLIT_HXX
#include "xflclit.hxx"
#endif

#ifndef _SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SETTINGS_HXX
#include <vcl/settings.hxx>
#endif

#ifndef _B3D_BASE3D_HXX
#include <goodies/base3d.hxx>
#endif

#ifndef _B3D_B3DTEX_HXX
#include <goodies/b3dtex.hxx>
#endif

#ifndef _SVX_XLNCLIT_HXX
#include "xlnclit.hxx"
#endif

#ifndef _SFXMETRICITEM_HXX
#include <svtools/metitem.hxx>
#endif

#ifndef _XTABLE_HXX
#include "xtable.hxx"
#endif

#ifndef _SVX_FILLITEM_HXX
#include "xfillit.hxx"
#endif

#ifndef _SVX_XLNWTIT_HXX
#include "xlnwtit.hxx"
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif

#ifndef _B3D_B3DTRANS_HXX
#include "b3dtrans.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SVX_COLRITEM_HXX
#include "colritem.hxx"
#endif

#ifndef _SVXE3DITEM_HXX
#include "e3ditem.hxx"
#endif

#ifndef _SVX_XLNTRIT_HXX
#include "xlntrit.hxx"
#endif

#ifndef _SVX_XFLTRIT_HXX
#include "xfltrit.hxx"
#endif

#ifndef _SVDPAGV_HXX
#include "svdpagv.hxx"
#endif

#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif

#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
#endif

#ifndef _SVX3DITEMS_HXX
#include "svx3ditems.hxx"
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#ifndef _EEITEM_HXX
#include "eeitem.hxx"
#endif

#ifndef _SVX_XGRSCIT_HXX
#include "xgrscit.hxx"
#endif

#ifndef _SVX_SVDOIMP_HXX
#include "svdoimp.hxx"
#endif

#ifndef _SDR_PROPERTIES_E3DPROPERTIES_HXX
#include <svx/sdr/properties/e3dproperties.hxx>
#endif

#ifndef _SDR_PROPERTIES_E3DCOMPOUNDPROPERTIES_HXX
#include <svx/sdr/properties/e3dcompoundproperties.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif

#ifndef _BGFX_VECTOR_B3DVECTOR_HXX
#include <basegfx/vector/b3dvector.hxx>
#endif

#ifndef _SVX_XLNDSIT_HXX
#include <xlndsit.hxx>
#endif

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

/*************************************************************************
|*
|* Liste fuer 3D-Objekte
|*
\************************************************************************/

TYPEINIT1(E3dObjList, SdrObjList);

E3dObjList::E3dObjList(SdrModel* pNewModel, SdrPage* pNewPage, E3dObjList* pNewUpList)
:   SdrObjList(pNewModel, pNewPage, pNewUpList)
{
}

E3dObjList::E3dObjList(const E3dObjList& rSrcList)
:   SdrObjList(rSrcList)
{
}

E3dObjList::~E3dObjList()
{
}

void E3dObjList::NbcInsertObject(SdrObject* pObj, ULONG nPos, const SdrInsertReason* pReason)
{
    // Owner holen
    DBG_ASSERT(GetOwnerObj()->ISA(E3dObject), "AW: Einfuegen 3DObject in Parent != 3DObject");

    // Ist es ueberhaupt ein 3D-Objekt?
    if(pObj && pObj->ISA(E3dObject))
    {
        // Normales 3D Objekt, einfuegen mittels
        // call parent
        SdrObjList::NbcInsertObject(pObj, nPos, pReason);
    }
    else
    {
        // Kein 3D Objekt, fuege in Seite statt in Szene ein...
        GetOwnerObj()->GetPage()->InsertObject(pObj, nPos);
    }
}

SdrObject* E3dObjList::NbcRemoveObject(ULONG nObjNum)
{
    // Owner holen
    DBG_ASSERT(GetOwnerObj()->ISA(E3dObject), "AW: Entfernen 3DObject aus Parent != 3DObject");
    //E3dObject* pOwner = (E3dObject*)GetOwnerObj();

    // call parent
    SdrObject* pRetval = SdrObjList::NbcRemoveObject(nObjNum);

    // FitSnapRectToBoundVol vorbereiten
    if(GetOwnerObj() && GetOwnerObj()->ISA(E3dScene))
        ((E3dScene*)GetOwnerObj())->CorrectSceneDimensions();

    return pRetval;
}

SdrObject* E3dObjList::RemoveObject(ULONG nObjNum)
{
    // Owner holen
    DBG_ASSERT(GetOwnerObj()->ISA(E3dObject), "AW: Entfernen 3DObject aus Parent != 3DObject");
    //E3dObject* pOwner = (E3dObject*)GetOwnerObj();

    // call parent
    SdrObject* pRetval = SdrObjList::RemoveObject(nObjNum);

    // FitSnapRectToBoundVol vorbereiten
    if(GetOwnerObj() && GetOwnerObj()->ISA(E3dScene))
        ((E3dScene*)GetOwnerObj())->CorrectSceneDimensions();

    return pRetval;
}

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dObject::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dProperties(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dObject, SdrAttrObj);

E3dObject::E3dObject() :
    //BFS01nLogicalGroup(0),
    //BFS01nObjTreeLevel(0),
    //BFS01eDragDetail(E3DDETAIL_ONEBOX),
    //BFS01nPartOfParent(0),
    bTfHasChanged(TRUE),
    bBoundVolValid(TRUE),
    bIsSelected(FALSE)
{
    bIs3DObj = TRUE;
    pSub = new E3dObjList(NULL, NULL);
    pSub->SetOwnerObj(this);
    pSub->SetListKind(SDROBJLIST_GROUPOBJ);
    bClosedObj = TRUE;
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

E3dObject::~E3dObject()
{
    delete pSub;
    pSub = NULL;
}

/*************************************************************************
|*
|* Selektions-Flag setzen
|*
\************************************************************************/

void E3dObject::SetSelected(BOOL bNew)
{
    // selbst setzen
    bIsSelected = bNew;

    // bei SubObjekten setzen
    for ( ULONG i = 0; i < pSub->GetObjCount(); i++ )
    {
        if(pSub->GetObj(i) && pSub->GetObj(i)->ISA(E3dObject))
            ((E3dObject*)pSub->GetObj(i))->SetSelected(bNew);
    }
}

/*************************************************************************
|*
|* Aufbrechen, default-Implementierungen
|*
\************************************************************************/

BOOL E3dObject::IsBreakObjPossible()
{
    return FALSE;
}

SdrAttrObj* E3dObject::GetBreakObj()
{
    return 0L;
}

/*************************************************************************
|*
|* SetRectsDirty muss ueber die lokale SdrSubList gehen
|*
\************************************************************************/

void E3dObject::SetRectsDirty(sal_Bool bNotMyself)
{
    // call parent
    SdrAttrObj::SetRectsDirty(bNotMyself);

    // Eigene SubListe AUCH behandeln
    if(pSub && pSub->GetObjCount())
    {
        for (ULONG i = 0; i < pSub->GetObjCount(); i++)
        {
            SdrObject* pObj = pSub->GetObj(i);
            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In E3dObject sind nur 3D-Objekte erlaubt!");
            ((E3dObject*)pObj)->SetRectsDirty(bNotMyself);
        }
    }
}

/*************************************************************************
|*
|* Inventor zurueckgeben
|*
\************************************************************************/

UINT32 E3dObject::GetObjInventor() const
{
    return E3dInventor;
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

UINT16 E3dObject::GetObjIdentifier() const
{
    return E3D_OBJECT_ID;
}

/*************************************************************************
|*
|* Faehigkeiten des Objektes feststellen
|*
\************************************************************************/

void E3dObject::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bResizeFreeAllowed    = TRUE;
    rInfo.bResizePropAllowed    = TRUE;
    rInfo.bRotateFreeAllowed    = TRUE;
    rInfo.bRotate90Allowed      = TRUE;
    rInfo.bMirrorFreeAllowed    = FALSE;
    rInfo.bMirror45Allowed      = FALSE;
    rInfo.bMirror90Allowed      = FALSE;
    rInfo.bShearAllowed         = FALSE;
    rInfo.bEdgeRadiusAllowed    = FALSE;
    rInfo.bCanConvToPath        = FALSE;

    // no transparence for 3d objects
    rInfo.bTransparenceAllowed = FALSE;

    // gradient depends on fillstyle
    // BM *** check if SetItem is NULL ***
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetMergedItem(XATTR_FILLSTYLE))).GetValue();
    rInfo.bGradientAllowed = (eFillStyle == XFILL_GRADIENT);

    // Umwandeln von 3D-Koerpern in Gruppe von Polygonen:
    //
    // Erst mal nicht moeglich, da die Erzeugung einer Gruppe von
    // 2D-Polygonen notwendig waere, die tiefensortiert werden muessten,
    // also bei Durchdringugnen auch gegeneinander geschnitten werden
    // muessten. Auch die Texturkoorinaten waeren ein ungeloestes
    // Problem.
    rInfo.bCanConvToPoly = FALSE;
    rInfo.bCanConvToContour = FALSE;

    rInfo.bCanConvToPathLineToArea = FALSE;
    rInfo.bCanConvToPolyLineToArea = FALSE;
}

/*************************************************************************
|*
|* Layer abfragen
|*
\************************************************************************/

//BFS01SdrLayerID E3dObject::GetLayer() const
//BFS01{
    //BFS01FASTBOOL bFirst = TRUE;
    //BFS01E3dObjList* pOL = pSub;
    //BFS01ULONG       nObjCnt = pOL->GetObjCount();
//BFS01 SdrLayerID  nLayer = SdrLayerID(nLayerID);

    //BFS01for ( ULONG i = 0; i < nObjCnt; i++ )
    //BFS01{
    //BFS01 SdrLayerID nObjLayer;
    //BFS01 if(pOL->GetObj(i)->ISA(E3dPolyObj))
    //BFS01     nObjLayer = SdrLayerID(nLayerID);
    //BFS01 else
    //BFS01     nObjLayer = pOL->GetObj(i)->GetLayer();

    //BFS01 if (bFirst)
    //BFS01 {
    //BFS01     nLayer = nObjLayer;
    //BFS01     bFirst = FALSE;
    //BFS01 }
    //BFS01 else if ( nObjLayer != nLayer )
    //BFS01     return 0;
    //BFS01}
//BFS01 return nLayer;
//BFS01}

/*************************************************************************
|*
|* Layer setzen
|*
\************************************************************************/

void E3dObject::NbcSetLayer(SdrLayerID nLayer)
{
    SdrAttrObj::NbcSetLayer(nLayer);

    E3dObjList* pOL = pSub;
    ULONG nObjCnt = pOL->GetObjCount();
    ULONG i;
    for ( i = 0; i < nObjCnt; i++ )
        pOL->GetObj(i)->NbcSetLayer(nLayer);
}

/*************************************************************************
|*
|* ObjList auch an SubList setzen
|*
\************************************************************************/

void E3dObject::SetObjList(SdrObjList* pNewObjList)
{
    SdrObject::SetObjList(pNewObjList);
    pSub->SetUpList(pNewObjList);
}

/*************************************************************************
|*
|* Layer setzen
|*
\************************************************************************/

void E3dObject::SetPage(SdrPage* pNewPage)
{
    SdrAttrObj::SetPage(pNewPage);
    pSub->SetPage(pNewPage);
}

/*************************************************************************
|*
|* Layer setzen
|*
\************************************************************************/

void E3dObject::SetModel(SdrModel* pNewModel)
{
    SdrAttrObj::SetModel(pNewModel);
    pSub->SetModel(pNewModel);
}

/*************************************************************************
|*
|* resize object, used from old 2d interfaces, e.g. in Move/Scale dialog
|* (F4)
|*
\************************************************************************/
void E3dObject::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
#ifndef SVX_LIGHT
    // SdrAttrObj::NbcResize(rRef, xFact, yFact);

    // Bewegung in X,Y im Augkoordinatensystem
    E3dScene* pScene = GetScene();

    if(pScene)
    {
        // pos ermitteln
        B3dTransformationSet& rTransSet = pScene->GetCameraSet();
        Vector3D aScaleCenter((double)rRef.X(), (double)rRef.Y(), 32768.0);
        aScaleCenter = rTransSet.ViewToEyeCoor(aScaleCenter);

        // scale-faktoren holen
        double fScaleX = xFact;
        double fScaleY = yFact;

        // build transform
        Matrix4D mFullTransform(GetFullTransform());
        Matrix4D mTrans(mFullTransform);

        mTrans *= rTransSet.GetOrientation();
        mTrans.Translate(-aScaleCenter);
        mTrans.Scale(fScaleX, fScaleY, 1.0);
        mTrans.Translate(aScaleCenter);
        mTrans *= rTransSet.GetInvOrientation();
        mFullTransform.Invert();
        mTrans *= mFullTransform;

        // anwenden
        Matrix4D mObjTrans(GetTransform());
        mObjTrans *= mTrans;
        SetTransform(mObjTrans);

        // force new camera and SnapRect on scene, geometry may have really
        // changed
        pScene->CorrectSceneDimensions();
    }
#endif
}

/*************************************************************************
|*
|* Objekt verschieben in 2D, wird bei Cursortasten benoetigt
|*
\************************************************************************/
void E3dObject::NbcMove(const Size& rSize)
{
#ifndef SVX_LIGHT
    // Bewegung in X,Y im Augkoordinatensystem
    E3dScene* pScene = GetScene();

    if(pScene)
    {
        // Abmessungen der Szene in 3D und 2D als Vergleich
        Rectangle aRect = pScene->GetSnapRect();

        // Transformation Weltkoordinaten bis eine VOR Objektkoordinaten holen
        Matrix4D mInvDispTransform;
        if(GetParentObj())
        {
            mInvDispTransform = GetParentObj()->GetFullTransform();
            mInvDispTransform.Invert();
        }

        // BoundVolume von Weltkoordinaten in Eye-Koordinaten
        B3dTransformationSet& rTransSet = pScene->GetCameraSet();
        const Volume3D& rVol = pScene->GetBoundVolume();
        Volume3D aEyeVol = rVol.GetTransformVolume(rTransSet.GetOrientation());

        // relativen Bewegungsvektor in Augkoordinaten bilden
        Vector3D aMove(
            (double)rSize.Width() * aEyeVol.GetWidth() / (double)aRect.GetWidth(),
            (double)-rSize.Height() * aEyeVol.GetHeight() / (double)aRect.GetHeight(),
            0.0);

        // Bewegungsvektor in lokale Koordinaten des Parents des Objektes
        Vector3D aPos;
        aMove = rTransSet.EyeToWorldCoor(aMove);
        aMove *= mInvDispTransform;
        aPos = rTransSet.EyeToWorldCoor(aPos);
        aPos *= mInvDispTransform;
        aMove = aMove - aPos;

        // Transformieren
        Translate(aMove);

        // force new camera and SnapRect on scene, geometry may have really
        // changed
        pScene->CorrectSceneDimensions();
    }
#endif
}

/*************************************************************************
|*
|* liefere die Sublist, aber nur dann, wenn darin Objekte enthalten sind !
|*
\************************************************************************/

SdrObjList* E3dObject::GetSubList() const
{
    return pSub;
}

/*************************************************************************
|*
|* Anzahl der Handles zurueckgeben
|*
\************************************************************************/

USHORT E3dObject::GetHdlCount() const
{
    // 8 Eckpunkte + 1 E3dVolumeMarker (= Wireframe-Darstellung)
    return 9;
}

/*************************************************************************
|*
|* Handle-Liste fuellen
|*
\************************************************************************/

void E3dObject::AddToHdlList(SdrHdlList& rHdlList) const
{
    XPolyPolygon     aXPP(12);
    XPolygon         aLine(2);
    E3dVolumeMarker* pVolMarker;
    USHORT           nPolyCnt;

    ((E3dObject*) this)->ImpCreateWireframePoly(aXPP/*BFS01, E3DDETAIL_ONEBOX*/);
    nPolyCnt = aXPP.Count();

    for ( USHORT i = 0; i < nPolyCnt; i += 3 )
    {
        rHdlList.AddHdl(new SdrHdl(aXPP[i][0], HDL_BWGT));
        rHdlList.AddHdl(new SdrHdl(aXPP[i][1], HDL_BWGT));
    }

    if ( nPolyCnt > 0 )
    {
        pVolMarker = new E3dVolumeMarker(aXPP);
        rHdlList.AddHdl(pVolMarker);
    }
}

/*************************************************************************
|*
\************************************************************************/

FASTBOOL E3dObject::HasSpecialDrag() const
{
    return TRUE;
}

/*************************************************************************
|*
|* Paint; wird z.Z. nicht benutzt, da das Paint ueber die
|* (2D-)Displayliste der Szene laeuft
|*
\************************************************************************/

void E3dObject::Paint3D(XOutputDevice& rOut, Base3D* pBase3D,
    const SdrPaintInfoRec& rInfoRec, UINT16 nDrawFlags)
{
    if(pSub && pSub->GetObjCount())
    {
        BOOL bWasNotActive = rInfoRec.bNotActive;
        BOOL bIsEnteredGroup(FALSE);

        if((rInfoRec.pPV && GetSubList() && rInfoRec.pPV->GetObjList() == GetSubList())
            || ((rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE) != 0))
        {
            bIsEnteredGroup = TRUE;
        }

        if(bIsEnteredGroup && bWasNotActive)
        {
            // auf aktive Elemente schalten
            ((SdrPaintInfoRec&)rInfoRec).bNotActive = FALSE;
        }

        for (ULONG i = 0; i < pSub->GetObjCount(); i++)
        {
            SdrObject* pObj = pSub->GetObj(i);
            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In E3dObject sind nur 3D-Objekte erlaubt!");
            ((E3dObject*)pObj)->Paint3D(rOut, pBase3D, rInfoRec, nDrawFlags);
        }

        if(bIsEnteredGroup && bWasNotActive)
        {
            // Zurueck auf Ursprung, Zustand wieder verlassen
            ((SdrPaintInfoRec&)rInfoRec).bNotActive = TRUE;
        }
    }
}

/*************************************************************************
|*
|* Objekt als Kontur in das Polygon einfuegen
|*
\************************************************************************/

void E3dObject::TakeContour3D(XPolyPolygon& rPoly)
{
    if(pSub && pSub->GetObjCount())
    {
        for (ULONG i = 0; i < pSub->GetObjCount(); i++)
        {
            SdrObject* pObj = pSub->GetObj(i);
            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In E3dObject sind nur 3D-Objekte erlaubt!");
            ((E3dObject*)pObj)->TakeContour3D(rPoly);
        }
    }
}

/*************************************************************************
|*
|* Schatten fuer 3D-Objekte zeichnen
|*
\************************************************************************/

void E3dObject::DrawShadows(Base3D *pBase3D,
    XOutputDevice& rXOut,
    const Rectangle& rBound, const Volume3D& rVolume,
    const SdrPaintInfoRec& rInfoRec)
{
    if(pSub && pSub->GetObjCount())
    {
        for (ULONG i = 0; i < pSub->GetObjCount(); i++)
        {
            SdrObject* pObj = pSub->GetObj(i);
            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In E3dObject sind nur 3D-Objekte erlaubt!");
            ((E3dObject*)pObj)->DrawShadows(pBase3D, rXOut, rBound, rVolume, rInfoRec);
        }
    }
}

/*************************************************************************
|*
|* SnapRect berechnen
|*
\************************************************************************/

void E3dObject::RecalcSnapRect()
{
    maSnapRect = Rectangle();
    if(pSub && pSub->GetObjCount())
    {
        for (ULONG i = 0; i < pSub->GetObjCount(); i++)
        {
            SdrObject* pObj = pSub->GetObj(i);
            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In E3dObject sind nur 3D-Objekte erlaubt!");
            Rectangle aSubRect = ((E3dObject*)pObj)->GetSnapRect();
            maSnapRect.Union(aSubRect);
        }
    }
}

/*************************************************************************
|*
|* BoundRect berechnen
|*
\************************************************************************/

void E3dObject::RecalcBoundRect()
{
    // BoundRect aus SnapRect berechnen
    aOutRect = GetSnapRect();

    if(pSub && pSub->GetObjCount())
    {
        for (ULONG i = 0; i < pSub->GetObjCount(); i++)
        {
            SdrObject* pObj = pSub->GetObj(i);
            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In E3dObject sind nur 3D-Objekte erlaubt!");
            Rectangle aSubRect = ((E3dObject*)pObj)->GetCurrentBoundRect();
            aOutRect.Union(aSubRect);
        }
    }
}

/*************************************************************************
|*
|* Einfuegen eines 3D-Objekts an den Parent weitermelden, damit dieser
|* ggf. eine Sonderbehandlung fuer spezielle Objekte durchfuehren kann
|* (z.B. Light/Label in E3dScene)
|*
\************************************************************************/

void E3dObject::NewObjectInserted(const E3dObject* p3DObj)
{
    if(GetParentObj())
        GetParentObj()->NewObjectInserted(p3DObj);
}

/*************************************************************************
|*
|* Parent ueber Aenderung der Struktur (z.B. durch Transformation)
|* informieren; dabei wird das Objekt, in welchem die Aenderung
|* aufgetreten ist, uebergeben
|*
\************************************************************************/

void E3dObject::StructureChanged(const E3dObject* p3DObj)
{
    if ( GetParentObj() )
    {
        // Wenn sich im Child das BoundVolume geaendert hat, muessen
        // auch die der Parents angepasst werden
        if ( !p3DObj->bBoundVolValid )
            GetParentObj()->bBoundVolValid = FALSE;

        GetParentObj()->StructureChanged(p3DObj);
    }
}

/*************************************************************************
|*
|* 3D-Objekt einfuegen
|*
\************************************************************************/

void E3dObject::Insert3DObj(E3dObject* p3DObj)
{
    DBG_ASSERT(p3DObj, "Insert3DObj mit NULL-Zeiger!");
//BFS01 p3DObj->SetObjTreeLevel(nObjTreeLevel + 1);
    SdrPage* pPg = pPage;
    pSub->InsertObject(p3DObj);
    pPage = pPg;
    bBoundVolValid = FALSE;
    NewObjectInserted(p3DObj);
    StructureChanged(this);
}

void E3dObject::Remove3DObj(E3dObject* p3DObj)
{
    DBG_ASSERT(p3DObj, "Remove3DObj mit NULL-Zeiger!");

    if(p3DObj->GetParentObj() == this)
    {
        SdrPage* pPg = pPage;
        pSub->RemoveObject(p3DObj->GetOrdNum());
        pPage = pPg;

        bBoundVolValid = FALSE;
        StructureChanged(this);
    }
}

/*************************************************************************
|*
|* Parent holen
|*
\************************************************************************/

E3dObject* E3dObject::GetParentObj() const
{
    E3dObject* pRetval = NULL;

    if(GetObjList()
        && GetObjList()->GetOwnerObj()
        && GetObjList()->GetOwnerObj()->ISA(E3dObject))
        pRetval = ((E3dObject*)GetObjList()->GetOwnerObj());
    return pRetval;
}

/*************************************************************************
|*
|* Uebergeordnetes Szenenobjekt bestimmen
|*
\************************************************************************/

E3dScene* E3dObject::GetScene() const
{
    if(GetParentObj())
        return GetParentObj()->GetScene();
    return NULL;
}

/*************************************************************************
|*
|* umschliessendes Volumen inklusive aller Kindobjekte berechnen
|*
\************************************************************************/

void E3dObject::RecalcBoundVolume()
{
    E3dObjList* pOL = pSub;
    ULONG nObjCnt = pOL->GetObjCount();

    if(nObjCnt)
    {
        aBoundVol = Volume3D();

        for (ULONG i = 0; i < nObjCnt; i++)
        {
            SdrObject* pObj = pOL->GetObj(i);

            DBG_ASSERT(pObj->ISA(E3dObject), "In E3dObject sind nur 3D-Objekte erlaubt!");
            // Bei den Kindobjekten auch die lokalen Transformationen
            // beruecksichtigen
            E3dObject* p3DObj = (E3dObject*) pObj;
            const Volume3D& rVol = p3DObj->GetBoundVolume();
            const Matrix4D& rTf  = p3DObj->GetTransform();
            aBoundVol.Union(rVol.GetTransformVolume(rTf));
        }

        aLocalBoundVol = aBoundVol;
    }
    else
    {
        // use local value
        aBoundVol = aLocalBoundVol;

        // detect if lines are displayed
        const SfxItemSet& rSet = GetMergedItemSet();
        XLineStyle aLineStyle = ((const XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();

        if(aLineStyle != XLINE_NONE)
        {
            // expand BoundVolume with 1/2 line width
            sal_Int32 nLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();

            if(nLineWidth)
            {
                double fExpand = nLineWidth / 2.0;

                Vector3D aExpand(fExpand, fExpand, fExpand);
                Vector3D aMinVec(aBoundVol.MinVec() - aExpand);
                Vector3D aMaxVec(aBoundVol.MaxVec() + aExpand);

                aBoundVol.Union(aMinVec);
                aBoundVol.Union(aMaxVec);
            }
        }
    }

    bBoundVolValid = TRUE;
}

/*************************************************************************
|*
|* umschliessendes Volumen zurueckgeben und ggf. neu berechnen
|*
\************************************************************************/

const Volume3D& E3dObject::GetBoundVolume()
{
    if ( !bBoundVolValid )
        RecalcBoundVolume();

    if(!aBoundVol.IsValid())
        aBoundVol = Volume3D(Vector3D(), Vector3D());

    return aBoundVol;
}

/*************************************************************************
|*
|* Mittelpunkt liefern
|*
\************************************************************************/

Vector3D E3dObject::GetCenter()
{
    Volume3D aVolume = GetBoundVolume();
    return (aVolume.MaxVec() + aVolume.MinVec()) / 2.0;
}

/*************************************************************************
|*
|* Aederung des BoundVolumes an alle Kindobjekte weitergeben
|*
\************************************************************************/

void E3dObject::SetBoundVolInvalid()
{
    bBoundVolValid = FALSE;

    E3dObjList* pOL = pSub;
    ULONG nObjCnt = pOL->GetObjCount();

    for (ULONG i = 0; i < nObjCnt; i++)
    {
        SdrObject* pObj = pOL->GetObj(i);
        DBG_ASSERT(pObj->ISA(E3dObject), "In E3dObject sind nur 3D-Objekte erlaubt!");

        ((E3dObject*) pObj)->SetBoundVolInvalid();
    }
}

/*************************************************************************
|*
|* Aederung der Transformation an alle Kindobjekte weitergeben
|*
\************************************************************************/

void E3dObject::SetTransformChanged()
{
    bTfHasChanged = TRUE;
    bBoundVolValid = FALSE;

    E3dObjList* pOL = pSub;
    ULONG nObjCnt = pOL->GetObjCount();

    for (ULONG i = 0; i < nObjCnt; i++)
    {
        SdrObject* pObj = pOL->GetObj(i);
        DBG_ASSERT(pObj->ISA(E3dObject), "In E3dObject sind nur 3D-Objekte erlaubt!");

        ((E3dObject*) pObj)->SetTransformChanged();
    }
}

/*************************************************************************
|*
|* hierarchische Transformation ueber alle Parents bestimmen und mit
|* der uebergebenen Matrix verketten
|*
\************************************************************************/

void E3dObject::GetFullTransform(Matrix4D& rMatrix) const
{
    if ( bTfHasChanged )
    {
        rMatrix *= aTfMatrix;
        if ( GetParentObj() )
            GetParentObj()->GetFullTransform(rMatrix);
    }
    else
        rMatrix *= aFullTfMatrix;
}

/*************************************************************************
|*
|* hierarchische Transformation ueber alle Parents bestimmen, in
|* aFullTfMatrix ablegen und diese zurueckgeben
|*
\************************************************************************/

const Matrix4D& E3dObject::GetFullTransform()
{
    if ( bTfHasChanged )
    {
        aFullTfMatrix = aTfMatrix;

        if ( GetParentObj() )
            aFullTfMatrix *= GetParentObj()->GetFullTransform();

        bTfHasChanged = FALSE;
    }
    return aFullTfMatrix;
}

/*************************************************************************
|*
|* Transformationsmatrix abfragen
|*
\************************************************************************/

const Matrix4D& E3dObject::GetTransform() const
{
    return aTfMatrix;
}

/*************************************************************************
|*
|* Transformationsmatrix setzen
|*
\************************************************************************/

void E3dObject::NbcSetTransform(const Matrix4D& rMatrix)
{
    aTfMatrix = rMatrix;
    SetTransformChanged();
    StructureChanged(this);
}

/*************************************************************************
|*
|* Transformationsmatrix auf Einheitsmatrix zuruecksetzen
|*
\************************************************************************/

void E3dObject::NbcResetTransform()
{
    aTfMatrix.Identity();
    SetTransformChanged();
    StructureChanged(this);
}

/*************************************************************************
|*
|* Transformationsmatrix setzen mit Repaint-Broadcast
|*
\************************************************************************/

void E3dObject::SetTransform(const Matrix4D& rMatrix)
{
    // #110094#-14 SendRepaintBroadcast();
    NbcSetTransform(rMatrix);
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/*************************************************************************
|*
|* Transformationsmatrix zuruecksetzen mit Repaint-Broadcast
|*
\************************************************************************/

void E3dObject::ResetTransform()
{
    // #110094#-14 SendRepaintBroadcast();
    NbcResetTransform();
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/*************************************************************************
|*
|* Translation
|*
\************************************************************************/

void E3dObject::NbcTranslate(const Vector3D& rTrans)
{
    aTfMatrix.Translate(rTrans);
    SetTransformChanged();
    StructureChanged(this);
}
/*************************************************************************
|*
|* Translation mit Repaint-Broadcast
|*
\************************************************************************/

void E3dObject::Translate(const Vector3D& rTrans)
{
    // #110094#-14 SendRepaintBroadcast();
    NbcTranslate(rTrans);
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/*************************************************************************
|*
|* Skalierungen
|*
\************************************************************************/

void E3dObject::NbcScaleX(double fSx)
{
    aTfMatrix.ScaleX(fSx);
    SetTransformChanged();
    StructureChanged(this);
}

/************************************************************************/

void E3dObject::NbcScaleY(double fSy)
{
    aTfMatrix.ScaleY(fSy);
    SetTransformChanged();
    StructureChanged(this);
}

/************************************************************************/

void E3dObject::NbcScaleZ(double fSz)
{
    aTfMatrix.ScaleZ(fSz);
    SetTransformChanged();
    StructureChanged(this);
}

/************************************************************************/

void E3dObject::NbcScale(double fSx, double fSy, double fSz)
{
    aTfMatrix.Scale(fSx, fSy, fSz);
    SetTransformChanged();
    StructureChanged(this);
}

/*************************************************************************
|*
|* gleichmaessige Skalierung
|*
\************************************************************************/

void E3dObject::NbcScale(double fS)
{
    aTfMatrix.Scale(fS, fS, fS);
    SetTransformChanged();
    StructureChanged(this);
}

/*************************************************************************
|*
|* Skalierungen mit mit Repaint-Broadcast
|*
\************************************************************************/

void E3dObject::ScaleX(double fSx)
{
    // #110094#-14 SendRepaintBroadcast();
    NbcScaleX(fSx);
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/************************************************************************/

void E3dObject::ScaleY(double fSy)
{
    // #110094#-14 SendRepaintBroadcast();
    NbcScaleY(fSy);
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/************************************************************************/

void E3dObject::ScaleZ(double fSz)
{
    // #110094#-14 SendRepaintBroadcast();
    NbcScaleZ(fSz);
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/************************************************************************/

void E3dObject::Scale(double fSx, double fSy, double fSz)
{
    // #110094#-14 SendRepaintBroadcast();
    NbcScale(fSx, fSy, fSz);
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/************************************************************************/

void E3dObject::Scale(double fS)
{
    // #110094#-14 SendRepaintBroadcast();
    NbcScale(fS);
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/*************************************************************************
|*
|* Rotationen mit Winkel in Radiant
|*
\************************************************************************/

void E3dObject::NbcRotateX(double fAng)
{
    aTfMatrix.RotateX(fAng);
    SetTransformChanged();
    StructureChanged(this);
}

/************************************************************************/

void E3dObject::NbcRotateY(double fAng)
{
    aTfMatrix.RotateY(fAng);
    SetTransformChanged();
    StructureChanged(this);
}

/************************************************************************/

void E3dObject::NbcRotateZ(double fAng)
{
    aTfMatrix.RotateZ(fAng);
    SetTransformChanged();
    StructureChanged(this);
}

/*************************************************************************
|*
|* Rotationen mit Repaint-Broadcast
|*
\************************************************************************/

void E3dObject::RotateX(double fAng)
{
    // #110094#-14 SendRepaintBroadcast();
    NbcRotateX(fAng);
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/************************************************************************/

void E3dObject::RotateY(double fAng)
{
    // #110094#-14 SendRepaintBroadcast();
    NbcRotateY(fAng);
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/************************************************************************/

void E3dObject::RotateZ(double fAng)
{
    // #110094#-14 SendRepaintBroadcast();
    NbcRotateZ(fAng);
    SetChanged();
    BroadcastObjectChange();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/*************************************************************************
|*
|* Objektbaum-Ebene des Objekts und aller Children setzen
|*
\************************************************************************/

//BFS01void E3dObject::SetObjTreeLevel(USHORT nNewLevel)
//BFS01{
//BFS01 nObjTreeLevel = nNewLevel;
//BFS01 nNewLevel++;
//BFS01
//BFS01 E3dObjList* pOL = pSub;
//BFS01 ULONG nObjCnt = pOL->GetObjCount();
//BFS01
//BFS01 for (ULONG i = 0; i < nObjCnt; i++)
//BFS01 {
//BFS01     SdrObject* pObj = pOL->GetObj(i);
//BFS01     DBG_ASSERT(pObj->ISA(E3dObject), "In E3dObject sind nur 3D-Objekte erlaubt!");
//BFS01
//BFS01     ((E3dObject*) pObj)->SetObjTreeLevel(nNewLevel);
//BFS01 }
//BFS01}

/*************************************************************************
|*
|* logische Gruppe setzen
|*
\************************************************************************/

//BFS01void E3dObject::SetLogicalGroup(USHORT nGroup)
//BFS01{
//BFS01 nLogicalGroup = nGroup;
//BFS01}

/*************************************************************************
|*
|* Linien fuer die Wireframe-Darstellung des Objekts dem uebergebenen
|* Polygon3D hinzufuegen. Als default wird das BoundVolume verwendet.
|*
\************************************************************************/

void E3dObject::CreateWireframe(Polygon3D& rWirePoly, const Matrix4D* pTf /*BFS01 , E3dDragDetail eDetail*/)
{
//BFS01 if ( eDetail == E3DDETAIL_DEFAULT )
//BFS01     eDetail = eDragDetail;

//BFS01 if ( eDetail == E3DDETAIL_ALLBOXES || eDetail == E3DDETAIL_ALLLINES )
//BFS01 {
//BFS01     E3dObjList* pOL = pSub;
//BFS01     ULONG nObjCnt = pOL->GetObjCount();
//BFS01
//BFS01     for (ULONG i = 0; i < nObjCnt; i++)
//BFS01     {
//BFS01         E3dObject* pObj = (E3dObject*)pOL->GetObj(i);
//BFS01         DBG_ASSERT(pObj->ISA(E3dObject), "In E3dObject sind nur 3D-Objekte erlaubt!");
//BFS01
//BFS01         Matrix4D aLocalTf(pObj->GetTransform());
//BFS01         if(pTf)
//BFS01             aLocalTf *= *pTf;
//BFS01         pObj->CreateWireframe(rWirePoly, &aLocalTf, eDetail);
//BFS01     }

//BFS01     if(eDetail == E3DDETAIL_ALLBOXES && nObjCnt != 1)
//BFS01         GetBoundVolume().CreateWireframe(rWirePoly, pTf);
//BFS01 }
//BFS01 else
        GetBoundVolume().CreateWireframe(rWirePoly, pTf);
}

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

void E3dObject::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulObj3d);

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

/*************************************************************************
|*
|* Get the name of the object (plural)
|*
\************************************************************************/

void E3dObject::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralObj3d);
}

/*************************************************************************
|*
|* Wireframe-XPolyPolygon erzeugen
|*
\************************************************************************/

void E3dObject::ImpCreateWireframePoly(XPolyPolygon& rXPP/*BFS01, E3dDragDetail eDetail*/)
{
    // Neue Methode
    E3dScene* pScene = GetScene();
    Polygon3D aPoly3D(24, 240);
    XPolygon aLine(2);
    USHORT nPntCnt;

    // WireFrame herstellen
    CreateWireframe(aPoly3D, NULL/*BFS01, eDetail*/);
    nPntCnt = aPoly3D.GetPointCount();

    if(pScene)
    {
        // Maximas holen in DeviceKoordinaten
        Volume3D aVolume = pScene->FitInSnapRect();

        // Maximas fuer Abbildung verwenden
        pScene->GetCameraSet().SetDeviceVolume(aVolume, FALSE);
        Matrix4D mTransform = GetFullTransform();
        pScene->GetCameraSet().SetObjectTrans(mTransform);

        if ( nPntCnt > 1 )
        {
            Vector3D aVec;
            for ( USHORT i = 0; i < nPntCnt; i += 2 )
            {
                aVec = pScene->GetCameraSet().ObjectToViewCoor(aPoly3D[i]);
                aLine[0] = Point((long)(aVec.X() + 0.5), (long)(aVec.Y() + 0.5));

                aVec = pScene->GetCameraSet().ObjectToViewCoor(aPoly3D[i+1]);
                aLine[1] = Point((long)(aVec.X() + 0.5), (long)(aVec.Y() + 0.5));

                rXPP.Insert(aLine);
            }
        }
    }
}

/*************************************************************************
|*
|* Drag-Polygon zurueckgeben
|*
\************************************************************************/

void E3dObject::TakeXorPoly(XPolyPolygon& rXPP, FASTBOOL /*bDetail*/) const
{
    rXPP.Clear();
    // Const mal wieder weg, da evtl. das BoundVolume neu generiert wird
    //BFS01static E3dDragDetail eDetail = E3DDETAIL_DEFAULT;
    ((E3dObject*) this)->ImpCreateWireframePoly(rXPP/*BFS01, eDetail*/);
}

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

void E3dObject::operator=(const SdrObject& rObj)
{
    SdrObject::operator=(rObj);

    const E3dObject& r3DObj = (const E3dObject&) rObj;
    if (r3DObj.GetSubList())
    {
        // feststellen, ob alle SubObjekte selektiert oder
        // deselektiert sind
        BOOL bAllSelected = TRUE;
        BOOL bNoneSelected = TRUE;
        UINT32 nObjCnt = r3DObj.GetSubList()->GetObjCount();

        ULONG i;
        for (i = 0; i < nObjCnt; i++)
        {
            SdrObject* pObj = r3DObj.GetSubList()->GetObj(i);
            if(pObj && pObj->ISA(E3dObject))
            {
                E3dObject* p3DObj = (E3dObject*)pObj;
                if(p3DObj->GetSelected())
                    bNoneSelected = FALSE;
                else
                    bAllSelected = FALSE;
            }
        }

        if(bAllSelected || bNoneSelected)
        {
            // Normales verhalten
            pSub->CopyObjects(*r3DObj.GetSubList());
        }
        else
        {
            // Spezielle SubListe aufstellen, kopieren
            SdrObjList aOwnSubList(*r3DObj.GetSubList());

            // Alle nicht selektierten Objekte rausschmeissen
            for(i = 0;i < aOwnSubList.GetObjCount();i++)
            {
                SdrObject* pObj = aOwnSubList.GetObj(i);
                if(pObj && pObj->ISA(E3dObject))
                {
                    E3dObject* p3DObj = (E3dObject*)pObj;
                    if(!p3DObj->GetSelected())
                    {
                        aOwnSubList.NbcRemoveObject(pObj->GetOrdNum());
                        i--;
                        delete pObj;
                    }
                }
            }

            // jetzt diese Liste kopieren
            pSub->CopyObjects(aOwnSubList);

            // Hier noch ein FitSnapRect einleiten
            if(ISA(E3dScene))
                ((E3dScene&)r3DObj).FitSnapRectToBoundVol();
        }
    }

    // BoundVol kann uebernommen werden, da die Childs auch kopiert werden
    bBoundVolValid  = r3DObj.bBoundVolValid;
    aBoundVol       = r3DObj.aBoundVol;
    aLocalBoundVol  = r3DObj.aLocalBoundVol;

    aTfMatrix       = r3DObj.aTfMatrix;
    //BFS01nLogicalGroup   = r3DObj.nLogicalGroup;
    //BFS01nObjTreeLevel   = r3DObj.nObjTreeLevel;
    //BFS01nPartOfParent   = r3DObj.nPartOfParent;
    //BFS01eDragDetail     = r3DObj.eDragDetail;

    // Da sich der Parent geaendert haben kann, Gesamttransformation beim
    // naechsten Mal auf jeden Fall neu bestimmen
    SetTransformChanged();

    // Selektionsstatus kopieren
    bIsSelected = r3DObj.bIsSelected;
}

/*************************************************************************
|*
|* Nur die Member des E3dObjekts in den Stream speichern
|* Dies wird direkt auch von E3dSphere gerufen um zu verhindern dass die
|* Subliste weggeschrieben wird. (FG)
|*
\************************************************************************/

//BFS01#ifndef SVX_LIGHT
//BFS01void E3dObject::WriteOnlyOwnMembers(SvStream& rOut) const
//BFS01{
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 SdrDownCompat aCompat(rOut, STREAM_WRITE);
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("E3dObjectOwnMembers");
//BFS01#endif
//BFS01
//BFS01 rOut << aLocalBoundVol;
//BFS01
//BFS01 Old_Matrix3D aMat3D;
//BFS01 aMat3D = aTfMatrix;
//BFS01 rOut << aMat3D;
//BFS01
//BFS01 rOut << nLogicalGroup;
//BFS01 rOut << nObjTreeLevel;
//BFS01 rOut << nPartOfParent;
//BFS01 rOut << UINT16(eDragDetail);
//BFS01}
//BFS01#endif

/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

//BFS01void E3dObject::WriteData(SvStream& rOut) const
//BFS01{
//BFS01#ifndef SVX_LIGHT
//BFS01 long position = rOut.Tell();
//BFS01 SdrAttrObj::WriteData(rOut);
//BFS01 position = rOut.Tell();
//BFS01
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 SdrDownCompat aCompat(rOut, STREAM_WRITE);
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("E3dObject");
//BFS01#endif
//BFS01
//BFS01 position = rOut.Tell();
//BFS01 pSub->Save(rOut);
//BFS01 position = rOut.Tell();
//BFS01
//BFS01 if (rOut.GetVersion() < 3560)
//BFS01 {
//BFS01     rOut << aLocalBoundVol;
//BFS01
//BFS01     Old_Matrix3D aMat3D;
//BFS01     aMat3D = aTfMatrix;
//BFS01     rOut << aMat3D;
//BFS01
//BFS01     rOut << nLogicalGroup;
//BFS01     rOut << nObjTreeLevel;
//BFS01     rOut << nPartOfParent;
//BFS01     rOut << UINT16(eDragDetail);
//BFS01 }
//BFS01 else
//BFS01 {
//BFS01     WriteOnlyOwnMembers(rOut);
//BFS01 }
//BFS01 position = rOut.Tell();
//BFS01#endif
//BFS01}

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

//BFS01void E3dObject::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 long position = rIn.Tell();
//BFS01 if (ImpCheckSubRecords (rHead, rIn))
//BFS01 {
//BFS01     position = rIn.Tell();
//BFS01     SdrAttrObj::ReadData(rHead, rIn);
//BFS01     position = rIn.Tell();
//BFS01     // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01     SdrDownCompat aCompat(rIn, STREAM_READ);
//BFS01#ifdef DBG_UTIL
//BFS01     aCompat.SetID("E3dObject");
//BFS01#endif
//BFS01     pSub->Load(rIn, *pPage);
//BFS01
//BFS01     position = rIn.Tell();
//BFS01     if ((rIn.GetVersion() < 3560) || (rHead.GetVersion() <= 12))
//BFS01     {
//BFS01         UINT16  nTmp16;
//BFS01
//BFS01         rIn >> aLocalBoundVol;
//BFS01
//BFS01         Old_Matrix3D aMat3D;
//BFS01         rIn >> aMat3D;
//BFS01         aTfMatrix = Matrix4D(aMat3D);
//BFS01
//BFS01         rIn >> nLogicalGroup;
//BFS01         rIn >> nObjTreeLevel;
//BFS01         rIn >> nPartOfParent;
//BFS01         rIn >> nTmp16; eDragDetail = E3dDragDetail(nTmp16);
//BFS01     }
//BFS01     else
//BFS01     {
//BFS01         ReadOnlyOwnMembers(rHead, rIn);
//BFS01     }
//BFS01     position = rIn.Tell();
//BFS01
//BFS01     // Wie ein veraendertes Objekt behandeln
//BFS01     SetTransformChanged();
//BFS01     StructureChanged(this);
//BFS01
//BFS01     // BoundVolume muss neu berechnet werden
//BFS01     bBoundVolValid = FALSE;
//BFS01
//BFS01     // SnapRect auch
//BFS01     bSnapRectDirty = TRUE;
//BFS01 }
//BFS01}

/*************************************************************************
|*
|* Nur die Daten des E3dObject aus Stream laden (nicht der Sublisten und
|* der Basisklassen). Wird von E3dSphere auch genutzt. (FileFormat-Optimierung)
|*
\************************************************************************/

//BFS01void E3dObject::ReadOnlyOwnMembers(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 SdrDownCompat aCompat(rIn, STREAM_READ);
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("E3dObjectOwnMembers");
//BFS01#endif
//BFS01 UINT16  nTmp16;
//BFS01
//BFS01 rIn >> aLocalBoundVol;
//BFS01
//BFS01 Old_Matrix3D aMat3D;
//BFS01 rIn >> aMat3D;
//BFS01 aTfMatrix = Matrix4D(aMat3D);
//BFS01
//BFS01 rIn >> nLogicalGroup;
//BFS01 rIn >> nObjTreeLevel;
//BFS01 rIn >> nPartOfParent;
//BFS01 rIn >> nTmp16; eDragDetail = E3dDragDetail(nTmp16);
//BFS01
//BFS01 bBoundVolValid = FALSE;
//BFS01}


/*************************************************************************
|*
|* nach dem Laden...
|*
\************************************************************************/

//BFS01void E3dObject::AfterRead()
//BFS01{
//BFS01 SdrAttrObj::AfterRead();
//BFS01 if (pSub)
//BFS01     pSub->AfterRead();
//BFS01}

/*************************************************************************
|*
|* erstelle neues GeoData-Objekt
|*
\************************************************************************/

SdrObjGeoData *E3dObject::NewGeoData() const
{
    // Theoretisch duerfen auch nur Szenen ihre GeoDatas erstellen und verwalten !!
    // AW: Dies stimmt nicht mehr, diese Stelle ist mit der neuen Engine OK!
    return new E3DObjGeoData;
}

/*************************************************************************
|*
|* uebergebe aktuelle werte an das GeoData-Objekt
|*
\************************************************************************/

void E3dObject::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrAttrObj::SaveGeoData (rGeo);

    ((E3DObjGeoData &) rGeo).aLocalBoundVol  = aLocalBoundVol;
    ((E3DObjGeoData &) rGeo).aTfMatrix       = aTfMatrix;
}

/*************************************************************************
|*
|* uebernehme werte aus dem GeoData-Objekt
|*
\************************************************************************/

void E3dObject::RestGeoData(const SdrObjGeoData& rGeo)
{
    aLocalBoundVol = ((E3DObjGeoData &) rGeo).aLocalBoundVol;
    NbcSetTransform (((E3DObjGeoData &) rGeo).aTfMatrix);

    SdrAttrObj::RestGeoData (rGeo);
    GetScene()->FitSnapRectToBoundVol();
}

/*************************************************************************
|*
|* Pruefe, ob die SubRecords ok sind und mit der Factory gelesen werden
|* koennen.
|*
\************************************************************************/

//BFS01BOOL E3dObject::ImpCheckSubRecords (const SdrObjIOHeader& rHead,
//BFS01                                 SvStream&             rIn)
//BFS01{
//BFS01 BOOL bDoRead = FALSE;
//BFS01
//BFS01 if ( rIn.GetError() == SVSTREAM_OK )
//BFS01 {
//BFS01     if (rHead.GetVersion () <= 12)
//BFS01     {
//BFS01         ULONG nPos0 = rIn.Tell();
//BFS01         // Einen SubRecord ueberspringen (SdrObject)
//BFS01         { SdrDownCompat aCompat(rIn,STREAM_READ); }
//BFS01         // Nocheinen SubRecord ueberspringen (SdrAttrObj)
//BFS01         { SdrDownCompat aCompat(rIn,STREAM_READ); }
//BFS01         // Und nun muesste meiner kommen
//BFS01         bDoRead = rHead.GetBytesLeft() != 0;
//BFS01         rIn.Seek (nPos0); // FilePos wieder restaurieren
//BFS01     }
//BFS01     else
//BFS01     {
//BFS01         bDoRead = TRUE;
//BFS01     }
//BFS01 }
//BFS01
//BFS01 return bDoRead;
//BFS01}

/*************************************************************************
|*
|* Rotation eines 3d-Koerpers
|*
\************************************************************************/
// 2D-rotation eines 3D-Koerpers, normalerweise macht das die Szene selbst
// Ist aber eine korrekte Implementierung, denn alles was passiert ist eine
// Rotation um die Achse die senkrecht auf dem Bildschirm steht und zwar
// unabhaengig davon, wie die Szene bisher gedreht worden ist.

void E3dObject::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
        // Also derzeit sind die Klebepunkte relativ zum aOutRect der Szene definiert. Vor dem Drehen
        // werden die Klebepunkte relativ zur Seite definiert. Sie nehmen an der Drehung der Szene noch nicht Teil
        // dafuer gibt es den
    SetGlueReallyAbsolute(TRUE);

    // SendRepaintBroadcast();
    double fWinkelInRad = nWink/100 * F_PI180;
    NbcRotateZ(fWinkelInRad);
    SetRectsDirty();    // Veranlasst eine Neuberechnung aller BoundRects
    NbcRotateGluePoints(rRef,nWink,sn,cs);  // Rotiert die Klebepunkte (die haben noch Koordinaten relativ
                                            // zum Urpsung des Blattes
    SetGlueReallyAbsolute(FALSE);  // ab jetzt sind sie wieder relativ zum BoundRect (also dem aOutRect definiert)
}

/*************************************************************************/

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dCompoundObject::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dCompoundProperties(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dCompoundObject, E3dObject);

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

E3dCompoundObject::E3dCompoundObject() : E3dObject()
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);

    //BFS01bBytesLeft = FALSE;
    //BFS01bCreateE3dPolyObj = FALSE;
    bGeometryValid = FALSE;
    bFullTfIsPositive = TRUE;
}

E3dCompoundObject::E3dCompoundObject(E3dDefaultAttributes& rDefault) : E3dObject()
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    //BFS01bBytesLeft = FALSE;
    //BFS01bCreateE3dPolyObj = FALSE;
    bGeometryValid = FALSE;
}

void E3dCompoundObject::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    // Defaults setzen
    aMaterialAmbientColor = rDefault.GetDefaultAmbientColor();

    aBackMaterial = rDefault.GetDefaultBackMaterial();
    bCreateNormals = rDefault.GetDefaultCreateNormals();
    bCreateTexture = rDefault.GetDefaultCreateTexture();
    bUseDifferentBackMaterial = rDefault.GetDefaultUseDifferentBackMaterial();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

E3dCompoundObject::~E3dCompoundObject ()
{
}

/*************************************************************************
|*
|* Start der Geometrieerzeugung ankuendigen
|*
\************************************************************************/

void E3dCompoundObject::StartCreateGeometry()
{
    // Geometriestart mitteilen
    aDisplayGeometry.StartDescription();

    // Lokales Volumen reset
    aLocalBoundVol = Volume3D();

    // Geometrie ist ab jetzt gueltig, um ein rekursives weiteres
    // Erzeugen zu verhindern
    bGeometryValid = TRUE;
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

UINT16 E3dCompoundObject::GetObjIdentifier() const
{
    return E3D_COMPOUNDOBJ_ID;
}

/*************************************************************************
|*
|* SnapRect berechnen
|*
\************************************************************************/

void E3dCompoundObject::RecalcSnapRect()
{
    E3dScene* pScene = GetScene();
    if(pScene)
    {
        // Objekttransformation uebernehmen
        const Volume3D& rBoundVol = GetBoundVolume();
        maSnapRect = Rectangle();

        if(rBoundVol.IsValid())
        {
            const Matrix4D& rTrans = GetFullTransform();
            Vol3DPointIterator aIter(rBoundVol, &rTrans);
            Vector3D aTfVec;
            while ( aIter.Next(aTfVec) )
            {
                aTfVec = pScene->GetCameraSet().WorldToViewCoor(aTfVec);
                Point aPoint((long)(aTfVec.X() + 0.5), (long)(aTfVec.Y() + 0.5));
                maSnapRect.Union(Rectangle(aPoint, aPoint));
            }
        }
        bSnapRectDirty = FALSE;
    }
}

/*************************************************************************
|*
|* BoundRect berechnen und evtl. Schatten einbeziehen
|* Dazu muss ein eventueller Schatten des einzelnen 3D-Objektes
|* beruecksichtigt werden
|*
\************************************************************************/

void E3dCompoundObject::RecalcBoundRect()
{
    // BoundRect aus SnapRect berechnen
    aOutRect = GetSnapRect();

    E3dScene* pScene = GetScene();
    if(pScene)
    {
        // Schatten beruecksichtigen
        if(DoDrawShadow())
        {
            // ObjectTrans setzen
            Matrix4D mTransform = GetFullTransform();
            pScene->GetCameraSet().SetObjectTrans(mTransform);

            // Schattenpolygon holen
            PolyPolygon3D aShadowPoly3D;
            ImpGetShadowPolygon(aShadowPoly3D);

            // invert Y coor cause of GetPolyPolygon() later
            Matrix4D aTransMat;
            aTransMat.Scale(1.0, -1.0, 1.0);
            aShadowPoly3D.Transform(aTransMat);

            PolyPolygon aShadowPoly(aShadowPoly3D.GetPolyPolygon());

            // Hinzufuegen
            aOutRect.Union(aShadowPoly.GetBoundRect());
        }

        // Linienbreite beruecksichtigen
        INT32 nLineWidth = ((const XLineWidthItem&)(GetObjectItem(XATTR_LINEWIDTH))).GetValue();
        if(nLineWidth)
        {
            Rectangle aShadowRect = aOutRect;
            aShadowRect.Left() -= nLineWidth;
            aShadowRect.Right() += nLineWidth;
            aShadowRect.Top() -= nLineWidth;
            aShadowRect.Bottom() += nLineWidth;
            aOutRect.Union(aShadowRect);
        }
    }
}

/*************************************************************************
|*
|* BoundVolume holen. Falls die Geometrie ungueltig ist, diese neu
|* erzeugen und das BoundVol neu berechnen
|*
\************************************************************************/

const Volume3D& E3dCompoundObject::GetBoundVolume()
{
    // Geometrie aktuell?
    if(!bGeometryValid)
    {
        // Neu erzeugen und eine Neubestimmung des BoundVol erzwingen
        ReCreateGeometry();
        bBoundVolValid = FALSE;
    }

    // call parent
    return E3dObject::GetBoundVolume();
}

/*************************************************************************
|*
|* Rausschreiben der Datenmember eines E3dCompounds
|*
\************************************************************************/

//BFS01void E3dCompoundObject::WriteData(SvStream& rOut) const
//BFS01{
//BFS01#ifndef SVX_LIGHT
//BFS01#ifdef E3D_STREAMING
//BFS01
//BFS01 if (!aLocalBoundVol.IsValid() && aBoundVol.IsValid())
//BFS01 {
//BFS01     // Das aLocalBoundVol wird gespeichert.
//BFS01     // Ist dieses ungueltig, so wird das aBoundVol genommen
//BFS01     // (sollten beim E3dCompoundObject sowieso gleich sein)
//BFS01     ((E3dCompoundObject*) this)->aLocalBoundVol = aBoundVol;
//BFS01 }
//BFS01
//BFS01 E3dObject::WriteData(rOut);
//BFS01 if (rOut.GetVersion() < 3560)
//BFS01 {
//BFS01     // In diesem Fall passiert nichts, da vor der Version 4.0
//BFS01     // also im Falle der Revision 3.1
//BFS01 }
//BFS01 else
//BFS01 {
//BFS01     SdrDownCompat aCompat(rOut, STREAM_WRITE);
//BFS01#ifdef DBG_UTIL
//BFS01     aCompat.SetID("E3dCompoundObject");
//BFS01#endif
//BFS01     rOut << BOOL(GetDoubleSided());
//BFS01#endif
//BFS01
//BFS01     // neue Parameter zur Geometrieerzeugung
//BFS01     rOut << BOOL(bCreateNormals);
//BFS01     rOut << BOOL(bCreateTexture);
//BFS01
//BFS01     sal_uInt16 nVal = GetNormalsKind();
//BFS01     rOut << BOOL(nVal > 0);
//BFS01     rOut << BOOL(nVal > 1);
//BFS01
//BFS01     nVal = GetTextureProjectionX();
//BFS01     rOut << BOOL(nVal > 0);
//BFS01     rOut << BOOL(nVal > 1);
//BFS01
//BFS01     nVal = GetTextureProjectionY();
//BFS01     rOut << BOOL(nVal > 0);
//BFS01     rOut << BOOL(nVal > 1);
//BFS01
//BFS01     rOut << BOOL(GetShadow3D());
//BFS01
//BFS01     // neu al 384:
//BFS01     rOut << GetMaterialAmbientColor();
//BFS01     rOut << GetMaterialColor();
//BFS01     rOut << GetMaterialSpecular();
//BFS01     rOut << GetMaterialEmission();
//BFS01     rOut << GetMaterialSpecularIntensity();
//BFS01
//BFS01     aBackMaterial.WriteData(rOut);
//BFS01
//BFS01     rOut << (UINT16)GetTextureKind();
//BFS01
//BFS01     rOut << (UINT16)GetTextureMode();
//BFS01
//BFS01     rOut << BOOL(GetNormalsInvert());
//BFS01
//BFS01     // neu ab 534: (hat noch gefehlt)
//BFS01     rOut << BOOL(GetTextureFilter());
//BFS01 }
//BFS01#endif
//BFS01}

/*************************************************************************
|*
|* Einlesen der Datenmember eines E3dCompounds
|*
\************************************************************************/

//BFS01void E3dCompoundObject::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 if ( rIn.GetError() != SVSTREAM_OK )
//BFS01     return;
//BFS01
//BFS01 E3dObject::ReadData(rHead, rIn);
//BFS01
//BFS01   // Vor der Filerevision 13 wurde das Objekt nie geschrieben.
//BFS01   // auch kein Kompatibilitaetsrecord.
//BFS01 if ((rHead.GetVersion() < 13) || (rIn.GetVersion() < 3560))
//BFS01 {
//BFS01     return;
//BFS01 }
//BFS01
//BFS01 SdrDownCompat aCompat(rIn, STREAM_READ);
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("E3dCompoundObject");
//BFS01#endif
//BFS01
//BFS01 bBytesLeft = FALSE;
//BFS01 if (aCompat.GetBytesLeft () >= sizeof (BOOL))
//BFS01 {
//BFS01     BOOL bTmp, bTmp2;
//BFS01     sal_uInt16 nTmp;
//BFS01
//BFS01     rIn >> bTmp;
//BFS01     GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(bTmp));
//BFS01
//BFS01     // neue Parameter zur Geometrieerzeugung
//BFS01     if (aCompat.GetBytesLeft () >= sizeof (BOOL))
//BFS01     {
//BFS01         rIn >> bTmp;
//BFS01         bCreateNormals = bTmp;
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         bCreateTexture = bTmp;
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         rIn >> bTmp2;
//BFS01         if(bTmp == FALSE && bTmp2 == FALSE)
//BFS01             nTmp = 0;
//BFS01         else if(bTmp == TRUE && bTmp2 == FALSE)
//BFS01             nTmp = 1;
//BFS01         else
//BFS01             nTmp = 2;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DNormalsKindItem(nTmp));
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         rIn >> bTmp2;
//BFS01         if(bTmp == FALSE && bTmp2 == FALSE)
//BFS01             nTmp = 0;
//BFS01         else if(bTmp == TRUE && bTmp2 == FALSE)
//BFS01             nTmp = 1;
//BFS01         else
//BFS01             nTmp = 2;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DTextureProjectionXItem(nTmp));
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         rIn >> bTmp2;
//BFS01         if(bTmp == FALSE && bTmp2 == FALSE)
//BFS01             nTmp = 0;
//BFS01         else if(bTmp == TRUE && bTmp2 == FALSE)
//BFS01             nTmp = 1;
//BFS01         else
//BFS01             nTmp = 2;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DTextureProjectionYItem(nTmp));
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DShadow3DItem(bTmp));
//BFS01
//BFS01         // Setze ein Flag fuer den Aufrufer, dass neues Format
//BFS01         // zu lesen ist
//BFS01         bBytesLeft = TRUE;
//BFS01     }
//BFS01
//BFS01     // neu al 384:
//BFS01     if (aCompat.GetBytesLeft () >= sizeof (B3dMaterial))
//BFS01     {
//BFS01         UINT16 nTmp;
//BFS01
//BFS01         Color aCol;
//BFS01
//BFS01         rIn >> aCol;
//BFS01         SetMaterialAmbientColor(aCol);
//BFS01
//BFS01         rIn >> aCol;
//BFS01         // do NOT use, this is the old 3D-Color(!)
//BFS01         // SetItem(XFillColorItem(String(), aCol));
//BFS01
//BFS01         rIn >> aCol;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DMaterialSpecularItem(aCol));
//BFS01
//BFS01         rIn >> aCol;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DMaterialEmissionItem(aCol));
//BFS01
//BFS01         rIn >> nTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DMaterialSpecularIntensityItem(nTmp));
//BFS01
//BFS01         aBackMaterial.ReadData(rIn);
//BFS01
//BFS01         rIn >> nTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DTextureKindItem(nTmp));
//BFS01
//BFS01         rIn >> nTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DTextureModeItem(nTmp));
//BFS01
//BFS01         rIn >> bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DNormalsInvertItem(bTmp));
//BFS01     }
//BFS01
//BFS01     // neu ab 534: (hat noch gefehlt)
//BFS01     if (aCompat.GetBytesLeft () >= sizeof (BOOL))
//BFS01     {
//BFS01         rIn >> bTmp;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DTextureFilterItem(bTmp));
//BFS01     }
//BFS01 }
//BFS01}

/*************************************************************************
|*
|* Bitmaps fuer 3D-Darstellung von Gradients und Hatches holen
|*
\************************************************************************/

AlphaMask E3dCompoundObject::GetAlphaMask(const SfxItemSet& rSet, const Size& rSizePixel)
{
    const XFillFloatTransparenceItem& rFloatTrans = ((const XFillFloatTransparenceItem&)(rSet.Get(XATTR_FILLFLOATTRANSPARENCE)));
    VirtualDevice *pVD = new VirtualDevice();
    pVD->SetOutputSizePixel( rSizePixel );
    XOutputDevice *pXOut = new XOutputDevice( pVD );
    SfxItemSet aFillSet(*rSet.GetPool());

    XGradient aNewGradient(rFloatTrans.GetGradientValue());

    Color aStartCol(aNewGradient.GetStartColor());
    if(aNewGradient.GetStartIntens() != 100)
    {
        double fFact = (double)aNewGradient.GetStartIntens() / 100.0;
        aStartCol = (B3dColor)aStartCol * fFact;
    }
    aNewGradient.SetStartColor(aStartCol);
    aNewGradient.SetStartIntens(100);

    Color aEndCol(aNewGradient.GetEndColor());
    if(aNewGradient.GetEndIntens() != 100)
    {
        double fFact = (double)aNewGradient.GetEndIntens() / 100.0;
        aEndCol = (B3dColor)aEndCol * fFact;
    }
    aNewGradient.SetEndColor(aEndCol);
    aNewGradient.SetEndIntens(100);

    aFillSet.Put( XFillStyleItem( XFILL_GRADIENT ) );
    aFillSet.Put( XFillGradientItem( String(), aNewGradient ) );
    aFillSet.Put( XGradientStepCountItem( aNewGradient.GetSteps() ) );
    pXOut->SetFillAttr( aFillSet );

    aFillSet.Put( XLineStyleItem( XLINE_NONE ) );
    pXOut->SetLineAttr( aFillSet );

    pXOut->DrawRect( Rectangle( Point(), rSizePixel ) );
    Bitmap aGradientBitmap = pVD->GetBitmap( Point(), rSizePixel );

    if( pVD )
        delete pVD;
    if( pXOut )
        delete pXOut;

    return AlphaMask(aGradientBitmap);
}

Bitmap E3dCompoundObject::GetGradientBitmap(const SfxItemSet& rSet)
{
    VirtualDevice *pVD = new VirtualDevice();
    Size aVDSize(256, 256);
    pVD->SetOutputSizePixel( aVDSize );
    XOutputDevice *pXOut = new XOutputDevice( pVD );
    SfxItemSet aFillSet(*rSet.GetPool());

    aFillSet.Put( XFillStyleItem( XFILL_GRADIENT ) );
    aFillSet.Put( rSet.Get(XATTR_FILLGRADIENT) );
    aFillSet.Put( rSet.Get(XATTR_GRADIENTSTEPCOUNT) );
    pXOut->SetFillAttr( aFillSet );

    aFillSet.Put( XLineStyleItem( XLINE_NONE ) );
    pXOut->SetLineAttr( aFillSet );

    pXOut->DrawRect( Rectangle( Point(), aVDSize ) );
    Bitmap aGradientBitmap = pVD->GetBitmap( Point(), aVDSize );

    if( pVD )
        delete pVD;
    if( pXOut )
        delete pXOut;

    return aGradientBitmap;
}

Bitmap E3dCompoundObject::GetHatchBitmap(const SfxItemSet& rSet)
{
    VirtualDevice *pVD = new VirtualDevice();
    const XFillHatchItem* pFillHatchItem = (XFillHatchItem*)&rSet.Get(XATTR_FILLHATCH);
    const XHatch& rHatch = pFillHatchItem->GetHatchValue();
    long nDistance = rHatch.GetDistance(); // in 100stel mm
    double fAngle = double(rHatch.GetAngle()) * (F_PI180 / 10.0);

    double fMinX(0.0);
    double fMaxX(0.0);
    double fMinY(0.0);
    double fMaxY(0.0);

    // nDistance in X-Richtung
    double fX = cos(fAngle) * double(nDistance);
    double fY = sin(fAngle) * double(nDistance);

    if(fX < fMinX)
        fMinX = fX;
    if(fX > fMaxX)
        fMaxX = fX;

    if(fY < fMinY)
        fMinY = fY;
    if(fY > fMaxY)
        fMaxY = fY;

    // nDistance in Y-Richtung
    fX = cos(fAngle + F_PI2) * double(nDistance);
    fY = sin(fAngle + F_PI2) * double(nDistance);

    if(fX < fMinX)
        fMinX = fX;
    if(fX > fMaxX)
        fMaxX = fX;

    if(fY < fMinY)
        fMinY = fY;
    if(fY > fMaxY)
        fMaxY = fY;

    // nDistance in -X-Richtung
    fX = cos(fAngle + F_PI) * double(nDistance);
    fY = sin(fAngle + F_PI) * double(nDistance);

    if(fX < fMinX)
        fMinX = fX;
    if(fX > fMaxX)
        fMaxX = fX;

    if(fY < fMinY)
        fMinY = fY;
    if(fY > fMaxY)
        fMaxY = fY;

    // nDistance in -Y-Richtung
    fX = cos(fAngle + (F_PI + F_PI2)) * double(nDistance);
    fY = sin(fAngle + (F_PI + F_PI2)) * double(nDistance);

    if(fX < fMinX)
        fMinX = fX;
    if(fX > fMaxX)
        fMaxX = fX;

    if(fY < fMinY)
        fMinY = fY;
    if(fY > fMaxY)
        fMaxY = fY;

    long nDistanceX = long(fMaxX - fMinX);
    long nDistanceY = long(fMaxY - fMinY);

    // Bei Schraffuren in eine der 4 Himmelsrichtungen Ausdehnung halbieren
    if(rHatch.GetAngle() % 900 == 0)
    {
        nDistanceX /= 2;
        nDistanceY /= 2;
    }

    Size aVDSize(64, 64);
    pVD->SetOutputSizePixel( aVDSize );
    MapMode aMapMode(MAP_100TH_MM);
    pVD->SetMapMode(aMapMode);

    XOutputDevice *pXOut = new XOutputDevice( pVD );
    SfxItemSet aFillSet(*rSet.GetPool());

    aFillSet.Put( XFillStyleItem( XFILL_SOLID ) );
    aFillSet.Put( XFillColorItem( String(), RGB_Color( COL_WHITE ) ) );
    aFillSet.Put( XLineStyleItem( XLINE_NONE ) );
    pXOut->SetLineAttr( aFillSet );
    pXOut->SetFillAttr( aFillSet );
    pXOut->DrawRect( Rectangle( Point(), pVD->PixelToLogic(aVDSize) ) );

    aFillSet.Put( XFillStyleItem( XFILL_HATCH ) );
    aFillSet.Put( rSet.Get(XATTR_FILLHATCH) );
    pXOut->SetFillAttr( aFillSet );

    Size aLogicalSize = pVD->PixelToLogic(aVDSize);

    // #109483#
    // If nDistance was 0 (the init value from the API), nDistanceX/Y
    // may be zero, too, which is not a valid value for a fraction. The
    // best value then is 1 since this simply takes the logical size as
    // scaling. A distance of 0 in a hatch makes no sense anyways.
    if(!nDistanceX)
    {
        nDistanceX = 1;
    }

    if(!nDistanceY)
    {
        nDistanceY = 1;
    }

    Fraction aFractionX(aLogicalSize.Width(), nDistanceX);
    Fraction aFractionY(aLogicalSize.Height(), nDistanceY);
    aMapMode.SetScaleX(aFractionX);
    aMapMode.SetScaleY(aFractionY);
    pVD->SetMapMode(aMapMode);
    pXOut->DrawRect( Rectangle( Point(), pVD->PixelToLogic(aVDSize) ) );

    Bitmap aHatchBitmap = pVD->GetBitmap( Point(), pVD->PixelToLogic(aVDSize) );

    if( pVD )
        delete pVD;
    if( pXOut )
        delete pXOut;

    return aHatchBitmap;
}

/*************************************************************************
|*
|* Give out simple line geometry
|*
\************************************************************************/

::basegfx::B3DPolyPolygon E3dCompoundObject::Get3DLineGeometry() const
{
    ::basegfx::B3DPolyPolygon aRetval;
    B3dEntityBucket& rEntityBucket = ((E3dCompoundObject*)this)->GetDisplayGeometry().GetEntityBucket();
    GeometryIndexValueBucket& rIndexBucket = ((E3dCompoundObject*)this)->GetDisplayGeometry().GetIndexBucket();
    sal_uInt32 nPolyCounter(0L);
    sal_uInt32 nEntityCounter(0L);

    while(nPolyCounter < rIndexBucket.Count())
    {
        // next primitive
        sal_uInt32 nUpperBound(rIndexBucket[nPolyCounter++].GetIndex());
        ::basegfx::B3DPoint aLastPoint;

        sal_Bool bLastLineVisible(rEntityBucket[nUpperBound - 1].IsEdgeVisible());

        if(bLastLineVisible)
        {
            Vector3D aVector(rEntityBucket[nUpperBound - 1].Point().GetVector3D());
            aLastPoint = ::basegfx::B3DPoint(aVector.X(), aVector.Y(), aVector.Z());
        }

        while(nEntityCounter < nUpperBound)
        {
            Vector3D aVector(rEntityBucket[nEntityCounter].Point().GetVector3D());
            ::basegfx::B3DPoint aNewPoint(aVector.X(), aVector.Y(), aVector.Z());

            if(bLastLineVisible)
            {
                if(aLastPoint != aNewPoint)
                {
                    // fill polygon
                    ::basegfx::B3DPolygon aNewPoly;
                    aNewPoly.append(aLastPoint);
                    aNewPoly.append(aNewPoint);

                    // create line geometry for polygon in eye coor to
                    // have it always orthogonal to camera plane
                    aRetval.append(aNewPoly);
                }
            }

            bLastLineVisible = rEntityBucket[nEntityCounter++].IsEdgeVisible();
            aLastPoint = aNewPoint;
        }
    }

    return aRetval;
}

/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/

void E3dCompoundObject::DestroyGeometry()
{
    // Alle Objekte in der Sub-Liste zerstoeren. Dies sind die
    // zur Visualisierung des Objektes verwendeten Hilfsobjekte
    pSub->Clear();
    delete pSub;
    pSub = new E3dObjList(NULL, NULL);
    pSub->SetOwnerObj(this);
    pSub->SetListKind(SDROBJLIST_GROUPOBJ);

    // Neue Geometrie zerstoeren
    aDisplayGeometry.Erase();

    // BoundVols resetten
    aLocalBoundVol  = Volume3D();
    bBoundVolValid = FALSE;
    StructureChanged(this);

    // Geometrie ist ungueltig
    bGeometryValid = FALSE;
}

void E3dCompoundObject::CreateGeometry()
{
    // Geometrie ist gueltig, um rekursion zu verhindern
    bGeometryValid = TRUE;

    // Eventuell entstandene Geometrie noch korrigieren
    // und Default -Normalen oder -Texturkoordinaten erzeugen
    if(bCreateNormals)
    {
        if(GetNormalsKind() > 1)
            GetDisplayGeometry().CreateDefaultNormalsSphere();
        if(GetNormalsInvert())
            GetDisplayGeometry().InvertNormals();
    }

    if(bCreateTexture)
    {
        GetDisplayGeometry().CreateDefaultTexture(
            ((GetTextureProjectionX() > 0) ? B3D_CREATE_DEFAULT_X : FALSE)
            |((GetTextureProjectionY() > 0) ? B3D_CREATE_DEFAULT_Y : FALSE),
            GetTextureProjectionX() > 1);
    }

    // Am Ende der Geometrieerzeugung das model an den erzeugten
    // PolyObj's setzen, d.h. beim ueberladen dieser Funktion
    // den parent am Ende rufen.
    //BFS01if(bCreateE3dPolyObj)
    //BFS01 SetModel(pModel);

    // Das Ende der Geometrieerzeugung anzeigen
    aDisplayGeometry.EndDescription();
}

void E3dCompoundObject::ReCreateGeometry(/*BFS01 BOOL bCreateOldGeometry*/)
{
    // Geometrie zerstoeren
    DestroyGeometry();

    // Flag fuer Geometrieerzeugung setzen
    //BFS01bCreateE3dPolyObj = bCreateOldGeometry;

    // ... und neu erzeugen
    CreateGeometry();
}

void E3dCompoundObject::GetFullTransform(Matrix4D& rMatrix) const
{
    E3dObject::GetFullTransform( rMatrix );
}
const Matrix4D& E3dCompoundObject::GetFullTransform()
{
    if ( bTfHasChanged )
    {
        aFullTfMatrix = aTfMatrix;

        if ( GetParentObj() )
            aFullTfMatrix *= GetParentObj()->GetFullTransform();

        bTfHasChanged = FALSE;
    }
    return aFullTfMatrix;
}

/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/

void E3dCompoundObject::AddGeometry(const PolyPolygon3D& rPolyPolygon3D,
    BOOL bHintIsComplex, BOOL bOutline)
{
    if(rPolyPolygon3D.Count())
    {
        // eventuell alte Geometrie erzeugen (z.B. zum speichern)
        //BFS01if(bCreateE3dPolyObj)
        //BFS01{
        //BFS01 E3dPolyObj* pObj = new E3dPolyObj(
        //BFS01     rPolyPolygon3D, GetDoubleSided(), TRUE);
        //BFS01 pObj->SetPartOfParent();
        //BFS01 Insert3DObj(pObj);
        //BFS01}

        // neue Geometrie erzeugen
        for(USHORT a = 0; a < rPolyPolygon3D.Count(); a++ )
        {
            const Polygon3D& rPoly3D = rPolyPolygon3D[a];
            aDisplayGeometry.StartObject(bHintIsComplex, bOutline);
            for(USHORT b = 0; b < rPoly3D.GetPointCount(); b++ )
                aDisplayGeometry.AddEdge(rPoly3D[b]);
        }
        aDisplayGeometry.EndObject();

        // LocalBoundVolume pflegen
        aLocalBoundVol.Union(rPolyPolygon3D.GetPolySize());

        // Eigenes BoundVol nicht mehr gueltig
        SetBoundVolInvalid();
        SetRectsDirty();
    }
}

void E3dCompoundObject::AddGeometry(
    const PolyPolygon3D& rPolyPolygon3D,
    const PolyPolygon3D& rPolyNormal3D,
    BOOL bHintIsComplex, BOOL bOutline)
{
    if(rPolyPolygon3D.Count())
    {
        // eventuell alte Geometrie erzeugen (z.B. zum speichern)
        //BFS01if(bCreateE3dPolyObj)
        //BFS01{
        //BFS01 E3dPolyObj* pObj = new E3dPolyObj(
        //BFS01     rPolyPolygon3D, rPolyNormal3D, GetDoubleSided(), TRUE);
        //BFS01 pObj->SetPartOfParent();
        //BFS01 Insert3DObj(pObj);
        //BFS01}

        // neue Geometrie erzeugen
        for(USHORT a = 0; a < rPolyPolygon3D.Count(); a++ )
        {
            const Polygon3D& rPoly3D = rPolyPolygon3D[a];
            const Polygon3D& rNormal3D = rPolyNormal3D[a];
            aDisplayGeometry.StartObject(bHintIsComplex, bOutline);
            for(USHORT b = 0; b < rPoly3D.GetPointCount(); b++ )
                aDisplayGeometry.AddEdge(rPoly3D[b], rNormal3D[b]);
        }
        aDisplayGeometry.EndObject();

        // LocalBoundVolume pflegen
        aLocalBoundVol.Union(rPolyPolygon3D.GetPolySize());

        // Eigenes BoundVol nicht mehr gueltig
        SetBoundVolInvalid();
        SetRectsDirty();
    }
}

void E3dCompoundObject::AddGeometry(
    const PolyPolygon3D& rPolyPolygon3D,
    const PolyPolygon3D& rPolyNormal3D,
    const PolyPolygon3D& rPolyTexture3D,
    BOOL bHintIsComplex, BOOL bOutline)
{
    if(rPolyPolygon3D.Count())
    {
        // eventuell alte Geometrie erzeugen (z.B. zum speichern)
        //BFS01if(bCreateE3dPolyObj)
        //BFS01{
        //BFS01 E3dPolyObj* pObj = new E3dPolyObj(
        //BFS01     rPolyPolygon3D, rPolyNormal3D,
        //BFS01     rPolyTexture3D, GetDoubleSided(), TRUE);
        //BFS01 pObj->SetPartOfParent();
        //BFS01 Insert3DObj(pObj);
        //BFS01}

        // neue Geometrie erzeugen
        for(USHORT a = 0; a < rPolyPolygon3D.Count(); a++ )
        {
            const Polygon3D& rPoly3D = rPolyPolygon3D[a];
            const Polygon3D& rNormal3D = rPolyNormal3D[a];
            const Polygon3D& rTexture3D = rPolyTexture3D[a];
            aDisplayGeometry.StartObject(bHintIsComplex, bOutline);
            for(USHORT b = 0; b < rPoly3D.GetPointCount(); b++ )
                aDisplayGeometry.AddEdge(rPoly3D[b], rNormal3D[b], rTexture3D[b]);
        }
        aDisplayGeometry.EndObject();

        // LocalBoundVolume pflegen
        aLocalBoundVol.Union(rPolyPolygon3D.GetPolySize());

        // Eigenes BoundVol nicht mehr gueltig
        SetBoundVolInvalid();
        SetRectsDirty();
    }
}

/*************************************************************************
|*
|* Hilfsfunktionen zur Geometrieerzeugung
|*
\************************************************************************/

void E3dCompoundObject::RotatePoly(
    PolyPolygon3D& rPolyPolyRotate,
    Matrix4D& rRotMat)
{
    USHORT nPolyCnt = rPolyPolyRotate.Count();

    for(UINT16 a=0;a<nPolyCnt;a++)
    {
        Polygon3D& rPolyRotate = rPolyPolyRotate[a];
        USHORT nPntCnt = rPolyRotate.GetPointCount();

        for(UINT16 b=0;b<nPntCnt;b++)
            rPolyRotate[b] *= rRotMat;
    }
}

void E3dCompoundObject::GrowPoly(
    PolyPolygon3D& rPolyPolyGrow,
    PolyPolygon3D& rPolyPolyNormals,
    double fFactor)
{
    USHORT nPolyCnt = rPolyPolyGrow.Count();

    for(UINT16 a=0;a<nPolyCnt;a++)
    {
        Polygon3D& rPolyGrow = rPolyPolyGrow[a];
        const Polygon3D& rPolyNormals = rPolyPolyNormals[a];
        USHORT nPntCnt = rPolyGrow.GetPointCount();

        for(UINT16 b=0;b<nPntCnt;b++)
            rPolyGrow[b] += rPolyNormals[b] * fFactor;
    }
}

void E3dCompoundObject::AddNormals(
    PolyPolygon3D& rPolyPolyDest,
    const PolyPolygon3D& rPolyPolySource)
{
    USHORT nPolyCnt = rPolyPolyDest.Count();

    for(UINT16 a=0;a<nPolyCnt;a++)
    {
        Polygon3D& rPolyDest = rPolyPolyDest[a];
        const Polygon3D& rPolySource = rPolyPolySource[a];
        USHORT nPntCnt = rPolyDest.GetPointCount();

        for(UINT16 b=0;b<nPntCnt;b++)
        {
            rPolyDest[b] += rPolySource[b];
            rPolyDest[b].Normalize();
        }
    }
}

void E3dCompoundObject::ScalePoly(
    PolyPolygon3D& rPolyPolyScale,
    double fFactor)
{
    USHORT nPolyCnt = rPolyPolyScale.Count();
    Vector3D aMiddle = rPolyPolyScale.GetMiddle();

    for(UINT16 a=0;a<nPolyCnt;a++)
    {
        Polygon3D& rPolyScale = rPolyPolyScale[a];
        USHORT nPntCnt = rPolyScale.GetPointCount();

        for(UINT16 b=0;b<nPntCnt;b++)
            rPolyScale[b] = ((rPolyScale[b] - aMiddle) * fFactor) + aMiddle;
    }
}

void E3dCompoundObject::CreateFront(
    const PolyPolygon3D& rPolyPoly3D,
    const PolyPolygon3D& rFrontNormals,
    BOOL bDoCreateNormals,
    BOOL bDoCreateTexture)
{
    // Vorderseite
    if(bDoCreateNormals)
    {
        if(bDoCreateTexture)
        {
            // Polygon fuer die Textur erzeugen
            PolyPolygon3D aPolyTexture = rPolyPoly3D;
            Volume3D aSize = aPolyTexture.GetPolySize();
            Matrix4D aTrans;

            aTrans.Identity();
            aTrans.Translate(-aSize.MinVec());
            aPolyTexture.Transform(aTrans);

            double fFactorX(1.0), fFactorY(1.0), fFactorZ(1.0);

            if(aSize.GetWidth() != 0.0)
                fFactorX = 1.0 / aSize.GetWidth();

            if(aSize.GetHeight() != 0.0)
                fFactorY = 1.0 / aSize.GetHeight();

            if(aSize.GetDepth() != 0.0)
                fFactorZ = 1.0 / aSize.GetDepth();

            aTrans.Identity();
            aTrans.Scale(fFactorX, -fFactorY, fFactorZ);
            aTrans.Translate(Vector3D(0.0, 1.0, 0.0));
            aPolyTexture.Transform(aTrans);

            AddGeometry(rPolyPoly3D, rFrontNormals, aPolyTexture, TRUE);
        }
        else
            AddGeometry(rPolyPoly3D, rFrontNormals, TRUE);
    }
    else
        AddGeometry(rPolyPoly3D, TRUE);
}

void E3dCompoundObject::AddFrontNormals(
    const PolyPolygon3D& rPolyPoly3D,
    PolyPolygon3D& rNormalsFront,
    Vector3D &rOffset)
{
    Vector3D aFrontNormal = -rOffset;
    aFrontNormal.Normalize();
    USHORT nPolyCnt = rPolyPoly3D.Count();

    for(UINT16 a=0;a<nPolyCnt;a++)
    {
        const Polygon3D& rPoly3D = rPolyPoly3D[a];
        Polygon3D& rNormalPoly = rNormalsFront[a];
        USHORT nPntCnt = rPoly3D.GetPointCount();

        for(UINT16 b=0;b<nPntCnt;b++)
        {
            rNormalPoly[b] += aFrontNormal;
            rNormalPoly[b].Normalize();
        }
    }
}

void E3dCompoundObject::CreateBack(
    const PolyPolygon3D& rPolyPoly3D,
    const PolyPolygon3D& rBackNormals,
    BOOL bDoCreateNormals,
    BOOL bDoCreateTexture)
{
    // PolyPolygon umdrehen
    PolyPolygon3D aLocalPoly = rPolyPoly3D;
    aLocalPoly.FlipDirections();

    // Rueckseite
    if(bDoCreateNormals)
    {
        PolyPolygon3D aLocalNormals = rBackNormals;
        aLocalNormals.FlipDirections();
        if(bDoCreateTexture)
        {
            // Polygon fuer die Textur erzeugen
            PolyPolygon3D aPolyTexture(aLocalPoly);
            Volume3D aSize = aPolyTexture.GetPolySize();
            Matrix4D aTrans;

            aTrans.Identity();
            aTrans.Translate(-aSize.MinVec());
            aPolyTexture.Transform(aTrans);

            double fFactorX(1.0), fFactorY(1.0), fFactorZ(1.0);

            if(aSize.GetWidth() != 0.0)
                fFactorX = 1.0 / aSize.GetWidth();

            if(aSize.GetHeight() != 0.0)
                fFactorY = 1.0 / aSize.GetHeight();

            if(aSize.GetDepth() != 0.0)
                fFactorZ = 1.0 / aSize.GetDepth();

            aTrans.Identity();
            aTrans.Scale(fFactorX, -fFactorY, fFactorZ);
            aTrans.Translate(Vector3D(0.0, 1.0, 0.0));
            aPolyTexture.Transform(aTrans);

            AddGeometry(aLocalPoly, aLocalNormals, aPolyTexture, TRUE);
        }
        else
            AddGeometry(aLocalPoly, aLocalNormals, TRUE);
    }
    else
    {
        AddGeometry(aLocalPoly, TRUE);
    }
}

void E3dCompoundObject::AddBackNormals(
    const PolyPolygon3D& rPolyPoly3D,
    PolyPolygon3D& rNormalsBack,
    Vector3D& rOffset)
{
    Vector3D aBackNormal = rOffset;
    aBackNormal.Normalize();
    USHORT nPolyCnt = rPolyPoly3D.Count();

    for(UINT16 a=0;a<nPolyCnt;a++)
    {
        const Polygon3D& rPoly3D = rPolyPoly3D[a];
        Polygon3D& rNormalPoly = rNormalsBack[a];
        USHORT nPntCnt = rPoly3D.GetPointCount();

        for(UINT16 b=0;b<nPntCnt;b++)
        {
            rNormalPoly[b] += aBackNormal;
            rNormalPoly[b].Normalize();
        }
    }
}

void E3dCompoundObject::CreateInBetween(
    const PolyPolygon3D& rPolyPolyFront,
    const PolyPolygon3D& rPolyPolyBack,
    const PolyPolygon3D& rFrontNormals,
    const PolyPolygon3D& rBackNormals,
    BOOL bDoCreateNormals,
    double fSurroundFactor,
    double fTextureStart,
    double fTextureDepth,
    BOOL bRotateTexture90)
{
    USHORT nPolyCnt = rPolyPolyFront.Count();
    BOOL bDoCreateTexture = (fTextureDepth == 0.0) ? FALSE : TRUE;
    double fPolyLength(0.0), fPolyPos(0.0);
    USHORT nLastIndex(0);

    // Verbindungsstuecke
    if(bDoCreateNormals)
    {
        for(UINT16 a=0;a<nPolyCnt;a++)
        {
            const Polygon3D& rPoly3DFront = rPolyPolyFront[a];
            const Polygon3D& rPoly3DBack = rPolyPolyBack[a];

            const Polygon3D& rPolyNormalsFront = rFrontNormals[a];
            const Polygon3D& rPolyNormalsBack = rBackNormals[a];

            Polygon3D   aRect3D(4);
            Polygon3D   aNormal3D(4);
            Polygon3D   aTexture3D(4);
            USHORT nPntCnt = rPoly3DFront.GetPointCount();
            USHORT nPrefillIndex = rPoly3DFront.IsClosed() ? nPntCnt - 1 : 0;

            aRect3D[3] = rPoly3DFront[nPrefillIndex];
            aRect3D[2] = rPoly3DBack[nPrefillIndex];
            aNormal3D[3] = rPolyNormalsFront[nPrefillIndex];
            aNormal3D[2] = rPolyNormalsBack[nPrefillIndex];

            if(bDoCreateTexture)
            {
                fPolyLength = rPoly3DFront.GetLength();
                fPolyPos = 0.0;
                nLastIndex = rPoly3DFront.IsClosed() ? nPntCnt - 1 : 0;

                if(bRotateTexture90)
                {
                    // X,Y vertauschen
                    aTexture3D[3].X() = fTextureStart;
                    aTexture3D[3].Y() = (1.0 - fPolyPos) * fSurroundFactor;

                    aTexture3D[2].X() = fTextureStart + fTextureDepth;
                    aTexture3D[2].Y() = (1.0 - fPolyPos) * fSurroundFactor;
                }
                else
                {
                    aTexture3D[3].X() = fPolyPos * fSurroundFactor;
                    aTexture3D[3].Y() = fTextureStart;

                    aTexture3D[2].X() = fPolyPos * fSurroundFactor;
                    aTexture3D[2].Y() = fTextureStart + fTextureDepth;
                }
            }

            for (USHORT i = rPoly3DFront.IsClosed() ? 0 : 1; i < nPntCnt; i++)
            {
                aRect3D[0] = aRect3D[3];
                aRect3D[1] = aRect3D[2];

                aRect3D[3] = rPoly3DFront[i];
                aRect3D[2] = rPoly3DBack[i];

                aNormal3D[0] = aNormal3D[3];
                aNormal3D[1] = aNormal3D[2];

                aNormal3D[3] = rPolyNormalsFront[i];
                aNormal3D[2] = rPolyNormalsBack[i];

                if(bDoCreateTexture)
                {
                    // Texturkoordinaten ermitteln
                    Vector3D aPart = rPoly3DFront[i] - rPoly3DFront[nLastIndex];
                    fPolyPos += aPart.GetLength() / fPolyLength;
                    nLastIndex = i;

                    // Der Abschnitt am Polygon entspricht dem Teil
                    // von fPolyPos bis fPolyPos+fPartLength

                    aTexture3D[0] = aTexture3D[3];
                    aTexture3D[1] = aTexture3D[2];

                    if(bRotateTexture90)
                    {
                        // X,Y vertauschen
                        aTexture3D[3].X() = fTextureStart;
                        aTexture3D[3].Y() = (1.0 - fPolyPos) * fSurroundFactor;

                        aTexture3D[2].X() = fTextureStart + fTextureDepth;
                        aTexture3D[2].Y() = (1.0 - fPolyPos) * fSurroundFactor;
                    }
                    else
                    {
                        aTexture3D[3].X() = fPolyPos * fSurroundFactor;
                        aTexture3D[3].Y() = fTextureStart;

                        aTexture3D[2].X() = fPolyPos * fSurroundFactor;
                        aTexture3D[2].Y() = fTextureStart + fTextureDepth;
                    }

                    AddGeometry(aRect3D, aNormal3D, aTexture3D, FALSE);
                }
                else
                    AddGeometry(aRect3D, aNormal3D, FALSE);
            }
        }
    }
    else
    {
        for(UINT16 a=0;a<nPolyCnt;a++)
        {
            const Polygon3D& rPoly3DFront = rPolyPolyFront[a];
            const Polygon3D& rPoly3DBack = rPolyPolyBack[a];
            Polygon3D   aRect3D(4);
            USHORT nPntCnt = rPoly3DFront.GetPointCount();
            USHORT nPrefillIndex = rPoly3DFront.IsClosed() ? nPntCnt - 1 : 0;

            aRect3D[3] = rPoly3DFront[nPrefillIndex];
            aRect3D[2] = rPoly3DBack[nPrefillIndex];

            for (USHORT i = rPoly3DFront.IsClosed() ? 0 : 1; i < nPntCnt; i++)
            {
                aRect3D[0] = aRect3D[3];
                aRect3D[1] = aRect3D[2];

                aRect3D[3] = rPoly3DFront[i];
                aRect3D[2] = rPoly3DBack[i];

                AddGeometry(aRect3D, FALSE);
            }
        }
    }
}

void E3dCompoundObject::AddInBetweenNormals(
    const PolyPolygon3D& rPolyPolyFront,
    const PolyPolygon3D& rPolyPolyBack,
    PolyPolygon3D& rNormals,
    BOOL bSmoothed)
{
    USHORT nPolyCnt = rPolyPolyFront.Count();

    // Verbindungsstuecke
    for(UINT16 a=0;a<nPolyCnt;a++)
    {
        const Polygon3D& rPoly3DFront = rPolyPolyFront[a];
        const Polygon3D& rPoly3DBack = rPolyPolyBack[a];
        Polygon3D& rNormalPoly = rNormals[a];
        USHORT nPntCnt = rPoly3DFront.GetPointCount();

        if(rPoly3DBack.IsClosed())
        {
            Vector3D aNormal = (rPoly3DBack[nPntCnt - 1] - rPoly3DFront[nPntCnt - 1])
                      |(rPoly3DFront[0] - rPoly3DFront[nPntCnt - 1]);
            aNormal.Normalize();
            for (USHORT i = 0; i < nPntCnt; i++)
            {
                Vector3D aNextNormal = (rPoly3DBack[i] - rPoly3DFront[i])
                    |(rPoly3DFront[(i+1 == nPntCnt) ? 0 : i+1] - rPoly3DFront[i]);
                aNextNormal.Normalize();
                if(bSmoothed)
                {
                    Vector3D aMidNormal = aNormal + aNextNormal;
                    aMidNormal.Normalize();
                    rNormalPoly[i] += aMidNormal;
                }
                else
                    rNormalPoly[i] += aNormal;
                rNormalPoly[i].Normalize();
                aNormal = aNextNormal;
            }
        }
        else
        {
            Vector3D aNormal;
            if(rPoly3DBack[0] == rPoly3DFront[0])
            {
                aNormal = (rPoly3DBack[1] - rPoly3DFront[1])
                      |(rPoly3DFront[1] - rPoly3DFront[0]);
            }
            else
            {
                aNormal = (rPoly3DBack[0] - rPoly3DFront[0])
                      |(rPoly3DFront[1] - rPoly3DFront[0]);
            }
            aNormal.Normalize();
            rNormalPoly[0] += aNormal; rNormalPoly[0].Normalize();
            for (USHORT i = 1; i < nPntCnt; i++)
            {
                Vector3D aNextNormal;
                if(i+1 == nPntCnt)
                {
                    aNextNormal = aNormal;
                }
                else
                {
                    aNextNormal = (rPoly3DBack[i] - rPoly3DFront[i])
                        |(rPoly3DFront[i+1] - rPoly3DFront[i]);
                }
                aNextNormal.Normalize();
                if(bSmoothed)
                {
                    Vector3D aMidNormal = aNormal + aNextNormal;
                    aMidNormal.Normalize();
                    rNormalPoly[i] += aMidNormal;
                }
                else
                    rNormalPoly[i] += aNormal;
                rNormalPoly[i].Normalize();
                aNormal = aNextNormal;
            }
        }
    }
}

/*************************************************************************
|*
|* Copy-Operator
|*
\************************************************************************/

void E3dCompoundObject::operator=(const SdrObject& rObj)
{
    // erstmal alle Childs kopieren
    E3dObject::operator=(rObj);

    // weitere Parameter kopieren
    const E3dCompoundObject& r3DObj = (const E3dCompoundObject&) rObj;

    aDisplayGeometry = r3DObj.aDisplayGeometry;
    bCreateNormals = r3DObj.bCreateNormals;
    bCreateTexture = r3DObj.bCreateTexture;
    bGeometryValid = r3DObj.bGeometryValid;
    //BFS01bBytesLeft = r3DObj.bBytesLeft;
    //BFS01bCreateE3dPolyObj = r3DObj.bCreateE3dPolyObj;

    // neu ab 383:
    aMaterialAmbientColor = r3DObj.aMaterialAmbientColor;

    aBackMaterial = r3DObj.aBackMaterial;
    bUseDifferentBackMaterial = r3DObj.bUseDifferentBackMaterial;
}

/*************************************************************************
|*
|* Ausgabeparameter an 3D-Kontext setzen
|*
\************************************************************************/

void E3dCompoundObject::ImpSet3DParForFill(XOutputDevice& /*rOut*/, Base3D* pBase3D,
    BOOL& bDrawObject, UINT16 nDrawFlags, BOOL bGhosted, BOOL bIsFillDraft)
{
    if(bIsFillDraft)
    {
        bDrawObject = FALSE;
    }
    else
    {
        const SfxItemSet& rSet = GetObjectItemSet();
        const XFillStyle eFillStyle = ((const XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();

        if(eFillStyle == XFILL_NONE)
        {
            bDrawObject = FALSE;
        }
        else
        {
            sal_uInt16 nFillTrans = ((const XFillTransparenceItem&)(rSet.Get(XATTR_FILLTRANSPARENCE))).GetValue();
            const XFillFloatTransparenceItem& rFloatTrans = ((const XFillFloatTransparenceItem&)(rSet.Get(XATTR_FILLFLOATTRANSPARENCE)));
            BOOL bFillTransparence = (nFillTrans != 0);
            BOOL bFloatTransparence = rFloatTrans.IsEnabled();
            BOOL bAnyTransparence = (bFillTransparence || bFloatTransparence);
            BOOL bDrawTransparence = ((nDrawFlags & E3D_DRAWFLAG_TRANSPARENT) != 0);

            // force no fill transparence when float transparence
            if(bFloatTransparence)
            {
                bFillTransparence = FALSE;
                nFillTrans = 0;
            }

            if(bAnyTransparence != bDrawTransparence)
            {
                bDrawObject = FALSE;
            }
            else
            {
                // get base color
                Color aColorSolid = ((const XFillColorItem&) (rSet.Get(XATTR_FILLCOLOR))).GetColorValue();
                if(bGhosted)
                {
                    aColorSolid = Color(
                        (aColorSolid.GetRed() >> 1) + 0x80,
                        (aColorSolid.GetGreen() >> 1) + 0x80,
                        (aColorSolid.GetBlue() >> 1) + 0x80);
                }

                // prepare custom colors for linear transparency and black/white mode
                Color aColorSolidWithTransparency(aColorSolid);
                aColorSolidWithTransparency.SetTransparency((UINT8)(nFillTrans * 255 / 100));
                Color aColorWhite(COL_WHITE);
                Color aColorWhiteWithTransparency(COL_WHITE);
                aColorWhiteWithTransparency.SetTransparency((UINT8)(nFillTrans * 255 / 100));

                // set base materials (if no drawmode is set)
                pBase3D->SetMaterial(aColorWhite, Base3DMaterialAmbient);
                pBase3D->SetMaterial(aColorWhiteWithTransparency, Base3DMaterialDiffuse);
                pBase3D->SetMaterial(GetMaterialSpecular(), Base3DMaterialSpecular);
                pBase3D->SetMaterial(GetMaterialEmission(), Base3DMaterialEmission);
                pBase3D->SetShininess(GetMaterialSpecularIntensity());
                if(GetUseDifferentBackMaterial())
                {
                    pBase3D->SetMaterial(aColorWhite, Base3DMaterialAmbient, Base3DMaterialBack);
                    pBase3D->SetMaterial(aColorWhiteWithTransparency, Base3DMaterialDiffuse, Base3DMaterialBack);
                    pBase3D->SetMaterial(aBackMaterial.GetMaterial(Base3DMaterialSpecular), Base3DMaterialSpecular, Base3DMaterialBack);
                    pBase3D->SetMaterial(aBackMaterial.GetMaterial(Base3DMaterialEmission), Base3DMaterialEmission, Base3DMaterialBack);
                    pBase3D->SetShininess(aBackMaterial.GetShininess(), Base3DMaterialBack);
                }

                // prepare some more later used texture parameters
                B3dTexture* pTexture = NULL;
                Base3DTextureWrap eWrapX(Base3DTextureRepeat);
                Base3DTextureWrap eWrapY(Base3DTextureRepeat);
                Matrix4D mTexture;

                // now test the different draw modes and cases
                if((pBase3D->GetOutputDevice()->GetDrawMode() & DRAWMODE_WHITEFILL) != 0)
                {
                    // set material to black and white mode
                    pBase3D->SetMaterial(aColorWhite, Base3DMaterialAmbient);
                    pBase3D->SetMaterial(aColorWhiteWithTransparency, Base3DMaterialDiffuse);
                    if(GetUseDifferentBackMaterial())
                    {
                        pBase3D->SetMaterial(aColorWhite, Base3DMaterialAmbient, Base3DMaterialBack);
                        pBase3D->SetMaterial(aColorWhiteWithTransparency, Base3DMaterialDiffuse, Base3DMaterialBack);
                    }

                    // Color stays white, just set render mode
                    pBase3D->SetRenderMode(Base3DRenderFill);
                }
                else if((pBase3D->GetOutputDevice()->GetDrawMode() & DRAWMODE_SETTINGSFILL) != 0)
                {
                    Color aColorFill(Application::GetSettings().GetStyleSettings().GetWindowColor());
                    Color aColorFillWithTransparency(aColorFill);
                    aColorFillWithTransparency.SetTransparency((UINT8)(nFillTrans * 255 / 100));

                    // set material to black and white mode
                    pBase3D->SetMaterial(aColorFill, Base3DMaterialAmbient);
                    pBase3D->SetMaterial(aColorFillWithTransparency, Base3DMaterialDiffuse);
                    if(GetUseDifferentBackMaterial())
                    {
                        pBase3D->SetMaterial(aColorFill, Base3DMaterialAmbient, Base3DMaterialBack);
                        pBase3D->SetMaterial(aColorFillWithTransparency, Base3DMaterialDiffuse, Base3DMaterialBack);
                    }

                    // Color stays solid, just set render mode
                    pBase3D->SetRenderMode(Base3DRenderFill);
                }
                else if(eFillStyle == XFILL_BITMAP)
                {
                    // bitmap fill, use bitmap texture from 2D defines
                    BitmapEx aBmpEx;

                    if(SFX_ITEM_SET == rSet.GetItemState(XATTR_FILLBITMAP, TRUE))
                    {
                        // EIndeutige Bitmap, benutze diese
                        aBmpEx = BitmapEx((((const XFillBitmapItem&) (rSet.Get(XATTR_FILLBITMAP))).GetBitmapValue()).GetBitmap());
                    }

                    // #i29168#
                    // The received Bitmap may still be empty (see bug), so the fix needs
                    // his own if, it is not enough to use the else-tree
                    if(aBmpEx.IsEmpty())
                    {
                        // Keine eindeutige Bitmap. benutze default
                        //
                        // DIES IST EINE NOTLOESUNG, BIS MAN IRGENDWO AN DIE
                        // DEAULT-BITMAP RANKOMMT (IST VON KA IN VORBEREITUNG)
                        //
                        aBmpEx = BitmapEx(Bitmap(Size(4,4), 8));
                    }

                    // Texturattribute bilden
                    TextureAttributesBitmap aTexAttr(
                        bGhosted,
                        (void*)&rSet.Get(XATTR_FILLFLOATTRANSPARENCE),
                        aBmpEx.GetBitmap());
                    pTexture = pBase3D->ObtainTexture(aTexAttr);
                    if(!pTexture)
                    {
                        if(bGhosted)
                            aBmpEx.Adjust( 50 );

                        if(bFloatTransparence)
                            // add alpha channel to bitmap
                            aBmpEx = BitmapEx(aBmpEx.GetBitmap(), GetAlphaMask(rSet, aBmpEx.GetSizePixel()));

                        pTexture = pBase3D->ObtainTexture(aTexAttr, aBmpEx);
                    }

                    //sal_uInt16 nOffX = ((const SfxUInt16Item&) (rSet.Get(XATTR_FILLBMP_TILEOFFSETX))).GetValue();
                    //sal_uInt16 nOffY = ((const SfxUInt16Item&) (rSet.Get(XATTR_FILLBMP_TILEOFFSETY))).GetValue();
                    sal_uInt16 nOffPosX = ((const SfxUInt16Item&) (rSet.Get(XATTR_FILLBMP_POSOFFSETX))).GetValue();
                    sal_uInt16 nOffPosY = ((const SfxUInt16Item&) (rSet.Get(XATTR_FILLBMP_POSOFFSETY))).GetValue();
                    RECT_POINT eRectPoint = (RECT_POINT)((const SfxEnumItem&) (rSet.Get(XATTR_FILLBMP_POS))).GetValue();
                    BOOL bTile = ((const SfxBoolItem&) (rSet.Get(XATTR_FILLBMP_TILE))).GetValue();
                    BOOL bStretch = ((const SfxBoolItem&) (rSet.Get(XATTR_FILLBMP_STRETCH))).GetValue();
                    BOOL bLogSize = ((const SfxBoolItem&) (rSet.Get(XATTR_FILLBMP_SIZELOG))).GetValue();
                    Size aSize(
                        labs(((const SfxMetricItem&)(rSet.Get(XATTR_FILLBMP_SIZEX))).GetValue()),
                        labs(((const SfxMetricItem&)(rSet.Get(XATTR_FILLBMP_SIZEY))).GetValue()));
                    Vector3D aScaleVector(1.0, 1.0, 1.0);
                    Vector3D aTranslateVector(0.0, 0.0, 0.0);

                    // Groesse beachten, logische Groesse einer Kachel bestimmen
                    // erst mal in 1/100 mm
                    Size aLogicalSize = aBmpEx.GetPrefSize();
                    const Volume3D& rVol = GetBoundVolume();
                    if(aLogicalSize.Width() == 0 || aLogicalSize.Height() == 0)
                    {
                        // Keine logische Groesse, nimm Pixelgroesse
                        // und wandle diese um
                        aLogicalSize = Application::GetDefaultDevice()->PixelToLogic(aBmpEx.GetSizePixel(), MAP_100TH_MM);
                    }
                    else
                    {
                        if ( aBmpEx.GetPrefMapMode() == MAP_PIXEL )
                            aLogicalSize = Application::GetDefaultDevice()->PixelToLogic( aLogicalSize, MAP_100TH_MM );
                        else
                            aLogicalSize = OutputDevice::LogicToLogic( aLogicalSize, aBmpEx.GetPrefMapMode(), MAP_100TH_MM );
                    }

                    if(bLogSize)
                    {
                        // logische Groesse
                        if(aSize.Width() == 0 && aSize.Height() == 0)
                        {
                            // Originalgroesse benutzen, Original flagy

                            // Um ein vernuenftiges Mapping bei defaults auch
                            // fuer 3D-Objekte zu erreichen, nimm die logische
                            // groesse einfach als groesser an
                            aLogicalSize.Width() /= 5; //10;
                            aLogicalSize.Height() /= 5; //10;
                        }
                        else
                        {
                            // Groesse in 100TH_MM in aSize, keine Flags
                            aLogicalSize = aSize;

                            // Um ein vernuenftiges Mapping bei defaults auch
                            // fuer 3D-Objekte zu erreichen, nimm die logische
                            // groesse einfach als groesser an
                            aLogicalSize.Width() /= 5; //10;
                            aLogicalSize.Height() /= 5; //10;
                        }
                    }
                    else
                    {
                        // relative Groesse
                        // 0..100 Prozent in aSize, relativ flag
                        aLogicalSize = Size(
                            (long)((rVol.GetWidth() * (double)aSize.Width() / 100.0) + 0.5),
                            (long)((rVol.GetHeight() * (double)aSize.Height() / 100.0) + 0.5));
                    }

                    // Skalieren
                    aScaleVector.X() = rVol.GetWidth() / (double)aLogicalSize.Width();
                    aScaleVector.Y() = rVol.GetHeight() / (double)aLogicalSize.Height();

                    if(bTile)
                    {
                        // Aneinandergefuegt drauflegen
                        double fLeftBound, fTopBound;

                        // Vertikal
                        if(eRectPoint == RP_LT || eRectPoint == RP_LM || eRectPoint == RP_LB)
                        {
                            // Links aligned starten
                            fLeftBound = 0.0;
                        }
                        else if(eRectPoint == RP_MT || eRectPoint == RP_MM || eRectPoint == RP_MB)
                        {
                            // Mittig
                            fLeftBound = (rVol.GetWidth() / 2.0)
                                - ((double)aLogicalSize.Width() / 2.0);
                        }
                        else
                        {
                            // Rechts aligned starten
                            fLeftBound = rVol.GetWidth()
                                - (double)aLogicalSize.Width();
                        }

                        // Horizontal
                        if(eRectPoint == RP_LT || eRectPoint == RP_MT || eRectPoint == RP_RT)
                        {
                            // Top aligned starten
                            fTopBound = 0.0;
                        }
                        else if(eRectPoint == RP_LM || eRectPoint == RP_MM || eRectPoint == RP_RM)
                        {
                            // Mittig
                            fTopBound = (rVol.GetHeight() / 2.0)
                                - ((double)aLogicalSize.Height() / 2.0);
                        }
                        else
                        {
                            // Bottom aligned starten
                            fTopBound = rVol.GetHeight()
                                - (double)aLogicalSize.Height();
                        }

                        // Verschieben
                        aTranslateVector.X() = fLeftBound;
                        aTranslateVector.Y() = fTopBound;

                        // Offset beachten
                        if(nOffPosX || nOffPosY)
                        {
                            aTranslateVector.X() += (double)aLogicalSize.Width() * ((double)nOffPosX / 100.0);
                            aTranslateVector.Y() += (double)aLogicalSize.Height() * ((double)nOffPosY / 100.0);
                        }
                    }
                    else
                    {
                        if(bStretch)
                        {
                            // 1x drauflegen, alles wie gehabt
                            // fertig
                            aScaleVector.X() = 1.0;
                            aScaleVector.Y() = 1.0;
                        }
                        else
                        {
                            // nur einmal benutzen
                            eWrapX = Base3DTextureSingle;
                            eWrapY = Base3DTextureSingle;

                            // Groesse beachten, zentriert anlegen
                            double fLeftBound = (rVol.GetWidth() / 2.0) - ((double)aLogicalSize.Width() / 2.0);
                            double fTopBound = (rVol.GetHeight() / 2.0) - ((double)aLogicalSize.Height() / 2.0);

                            // Verschieben
                            aTranslateVector.X() = fLeftBound;
                            aTranslateVector.Y() = fTopBound;
                        }
                    }

                    // TranslateVector anpassen
                    if(aTranslateVector.X())
                        aTranslateVector.X() /= -rVol.GetWidth();
                    if(aTranslateVector.Y())
                        aTranslateVector.Y() /= -rVol.GetHeight();

                    // Texturtransformation setzen
                    mTexture.Translate(aTranslateVector);
                    mTexture.Scale(aScaleVector);
                }
                else if(eFillStyle == XFILL_GRADIENT)
                {
                    // gradient fill. Create texture and set.
                    TextureAttributesGradient aTexAttr(
                        bGhosted,
                        (void*)&rSet.Get(XATTR_FILLFLOATTRANSPARENCE),
                        (void*)&rSet.Get(XATTR_FILLGRADIENT),
                        (void*)&rSet.Get(XATTR_GRADIENTSTEPCOUNT));

                    pTexture = pBase3D->ObtainTexture(aTexAttr);
                    if(!pTexture)
                    {
                        BitmapEx aBmpEx = BitmapEx(GetGradientBitmap(rSet));

                        if(bFloatTransparence)
                            // add alpha channel to bitmap
                            aBmpEx = BitmapEx(aBmpEx.GetBitmap(), GetAlphaMask(rSet, aBmpEx.GetSizePixel()));

                        if(bGhosted)
                            aBmpEx.Adjust( 50 );
                        pTexture = pBase3D->ObtainTexture(aTexAttr, aBmpEx);
                    }
                }
                else if(eFillStyle == XFILL_HATCH)
                {
                    // hatch fill. Create texture and set.
                    TextureAttributesHatch aTexAttr(
                        bGhosted,
                        (void*)&rSet.Get(XATTR_FILLFLOATTRANSPARENCE),
                        (void*)&rSet.Get(XATTR_FILLHATCH));

                    pTexture = pBase3D->ObtainTexture(aTexAttr);
                    if(!pTexture)
                    {
                        BitmapEx aBmpEx = GetHatchBitmap(rSet);

                        if(bFloatTransparence)
                            // add alpha channel to bitmap
                            aBmpEx = BitmapEx(aBmpEx.GetBitmap(), GetAlphaMask(rSet, aBmpEx.GetSizePixel()));

                        if(bGhosted)
                            aBmpEx.Adjust( 50 );
                        pTexture = pBase3D->ObtainTexture(aTexAttr, aBmpEx);
                    }

                    // set different texture transformation
                    mTexture.Scale(Vector3D(20.0, 20.0, 20.0));
                }
                else if(eFillStyle == XFILL_SOLID)
                {
                    if(bFloatTransparence)
                    {
                        // Texturattribute bilden
                        TextureAttributesColor aTexAttr(
                            bGhosted,
                            (void*)&rSet.Get(XATTR_FILLFLOATTRANSPARENCE),
                            aColorSolid);

                        pTexture = pBase3D->ObtainTexture(aTexAttr);
                        if(!pTexture)
                        {
                            // build single colored bitmap with draw color and add transparence bitmap
                            Size aSizeBitmap(128, 128);
                            Bitmap aForeground(aSizeBitmap, 24);
                            aForeground.Erase(aColorSolid);

                            if(bGhosted)
                                aForeground.Adjust( 50 );

                            // add alpha channel to bitmap
                            BitmapEx aBmpEx(aForeground, GetAlphaMask(rSet, aSizeBitmap));

                            pTexture = pBase3D->ObtainTexture(aTexAttr, aBmpEx);
                        }
                    }
                    else
                    {
                        // set material to base color
                        pBase3D->SetMaterial(aColorSolid, Base3DMaterialAmbient);
                        pBase3D->SetMaterial(aColorSolidWithTransparency, Base3DMaterialDiffuse);
                        if(GetUseDifferentBackMaterial())
                        {
                            pBase3D->SetMaterial(aBackMaterial.GetMaterial(Base3DMaterialAmbient), Base3DMaterialAmbient, Base3DMaterialBack);
                            pBase3D->SetMaterial(aBackMaterial.GetMaterial(Base3DMaterialDiffuse), Base3DMaterialDiffuse, Base3DMaterialBack);
                        }

                        // and at last, the render mode.
                        pBase3D->SetRenderMode(Base3DRenderFill);
                    }
                }
                else
                {
                    DBG_ERROR("unknown drawing mode (!)");
                }

                // use texture?
                if(pTexture)
                {
                    // set values for texture modes
                    pTexture->SetTextureKind(GetTextureKind());
                    pTexture->SetTextureMode(GetTextureMode());
                    pTexture->SetTextureFilter(GetTextureFilter() ? Base3DTextureLinear : Base3DTextureNearest);
                    pTexture->SetTextureWrapS(eWrapX);
                    pTexture->SetTextureWrapT(eWrapY);
                    pTexture->SetBlendColor(aColorSolid);
                    pTexture->SetTextureColor(aColorSolid);

                    // activate texture
                    pBase3D->SetActiveTexture(pTexture);
                    pBase3D->SetRenderMode(Base3DRenderFill);

                    // set texture transformation
                    GetScene()->GetCameraSet().SetTexture(mTexture);
                }
                else
                {
                    // switch it off.texture usage
                    pBase3D->SetActiveTexture();
                }
            }
        }
    }
}

void E3dCompoundObject::ImpSet3DParForLine(XOutputDevice& rOut, Base3D* pBase3D,
    BOOL& bDrawOutline, UINT16 nDrawFlags, BOOL /*bGhosted*/, BOOL bIsLineDraft, BOOL bIsFillDraft)
{
    // do drawflags allow line drawing at all?
    const SfxItemSet& rSet = GetObjectItemSet();
    sal_uInt16 nLineTransparence = ((const XLineTransparenceItem&)(rSet.Get(XATTR_LINETRANSPARENCE))).GetValue();
    BOOL bLineTransparence = (nLineTransparence != 0);
    BOOL bDrawTransparence = ((nDrawFlags & E3D_DRAWFLAG_TRANSPARENT) != 0);

    // #b4899532# if not filled but fill draft, avoid object being invisible in using
    // a hair linestyle and COL_LIGHTGRAY
    SfxItemSet aItemSet(rSet);
    if(bIsFillDraft && XLINE_NONE == ((const XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue())
    {
        ImpPrepareLocalItemSetForDraftLine(aItemSet);
    }

    if(bLineTransparence != bDrawTransparence)
    {
        bDrawOutline = FALSE;
    }

    // if no linestyle, draw no outline
    XLineStyle aLineStyle(XLINE_NONE);
    if(bDrawOutline)
    {
        aLineStyle = ((const XLineStyleItem&)(aItemSet.Get(XATTR_LINESTYLE))).GetValue();
        bDrawOutline = (aLineStyle != XLINE_NONE);
    }

    // special mode for black/white drawing or high contrast mode
    // Linecolor is set to black before (Base3d::SetColor())
    if((!bDrawOutline) && ((pBase3D->GetOutputDevice()->GetDrawMode() & (DRAWMODE_WHITEFILL|DRAWMODE_SETTINGSLINE)) != 0))
    {
        bDrawOutline = TRUE;
    }

    // does the outdev use linestyle?
    if(bDrawOutline && !rOut.GetIgnoreLineStyle())
    {
        Color aColorLine = ((const XLineColorItem&)(aItemSet.Get(XATTR_LINECOLOR))).GetColorValue();
        sal_Int32 nLineWidth = ((const XLineWidthItem&)(aItemSet.Get(XATTR_LINEWIDTH))).GetValue();

        if(pBase3D->GetOutputDevice()->GetDrawMode() & DRAWMODE_SETTINGSLINE)
        {
            svtools::ColorConfig aColorConfig;
            aColorLine = Color( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
        }

        if(nLineWidth && !bIsLineDraft)
        {
            Point aPnt(nLineWidth, 0);
            aPnt = pBase3D->GetOutputDevice()->LogicToPixel(aPnt) - pBase3D->GetOutputDevice()->LogicToPixel(Point());
            if(aPnt.X() <= 0)
                aPnt.X() = 1;
            pBase3D->SetLineWidth((double)aPnt.X());
        }
        else
        {
            pBase3D->SetLineWidth(1.0);
        }

        // Material setzen
        pBase3D->SetColor(aColorLine);
    }
}

void E3dCompoundObject::SetBase3DParams(XOutputDevice& rOut, Base3D* pBase3D,
    BOOL& bDrawObject, BOOL& bDrawOutline, UINT16 nDrawFlags, BOOL bGhosted,
    BOOL bIsLineDraft, BOOL bIsFillDraft)
{
    bDrawObject = ((nDrawFlags & E3D_DRAWFLAG_FILLED) != 0);
    if(bDrawObject)
        ImpSet3DParForFill(rOut, pBase3D, bDrawObject, nDrawFlags, bGhosted, bIsFillDraft);

    bDrawOutline = ((nDrawFlags & E3D_DRAWFLAG_OUTLINE) != 0);
    if(bDrawOutline)
        ImpSet3DParForLine(rOut, pBase3D, bDrawOutline, nDrawFlags, bGhosted, bIsLineDraft, bIsFillDraft);

    // Set ObjectTrans if line or fill is still set (maybe retet by upper calls)
    if(bDrawObject || bDrawOutline)
    {
        Matrix4D mTransform = GetFullTransform();
        GetScene()->GetCameraSet().SetObjectTrans(mTransform);
        pBase3D->SetTransformationSet(&(GetScene()->GetCameraSet()));
    }
}

/*************************************************************************
|*
|* Hittest fuer 3D-Objekte, wird an Geometrie weitergegeben
|*
\************************************************************************/

// #110988# test if given hit candidate point is inside bound volume of object
sal_Bool E3dCompoundObject::ImpIsInsideBoundVolume(const Vector3D& rFront, const Vector3D& rBack, const Point& /*rPnt*/) const
{
    const Volume3D& rBoundVol = ((E3dCompoundObject*)this)->GetBoundVolume();

    if(rBoundVol.IsValid())
    {
        double fXMax = rFront.X();
        double fXMin = rBack.X();

        if(fXMax < fXMin)
        {
            fXMax = rBack.X();
            fXMin = rFront.X();
        }

        if(rBoundVol.MinVec().X() <= fXMax && rBoundVol.MaxVec().X() >= fXMin)
        {
            double fYMax = rFront.Y();
            double fYMin = rBack.Y();

            if(fYMax < fYMin)
            {
                fYMax = rBack.Y();
                fYMin = rFront.Y();
            }

            if(rBoundVol.MinVec().Y() <= fYMax && rBoundVol.MaxVec().Y() >= fYMin)
            {
                double fZMax = rFront.Z();
                double fZMin = rBack.Z();

                if(fZMax < fZMin)
                {
                    fZMax = rBack.Z();
                    fZMin = rFront.Z();
                }

                if(rBoundVol.MinVec().Z() <= fZMax && rBoundVol.MaxVec().Z() >= fZMin)
                {
                    return sal_True;
                }
            }
        }
    }

    return sal_False;
}

SdrObject* E3dCompoundObject::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* /*pVisiLayer*/) const
{
    E3dPolyScene* pScene = (E3dPolyScene*)GetScene();

    if(pScene)
    {
        // get HitLine in ObjectKoordinates
        // set ObjectTrans
        Matrix4D mTransform = ((E3dCompoundObject*)this)->GetFullTransform();
        pScene->GetCameraSet().SetObjectTrans(mTransform);

        // create HitPoint Front und Back, transform to object coordinates
        Vector3D aFront(rPnt.X(), rPnt.Y(), 0.0);
        Vector3D aBack(rPnt.X(), rPnt.Y(), ZBUFFER_DEPTH_RANGE);
        aFront = pScene->GetCameraSet().ViewToObjectCoor(aFront);
        aBack = pScene->GetCameraSet().ViewToObjectCoor(aBack);

        if(ImpIsInsideBoundVolume(aFront, aBack, rPnt))
        {
            // Geometrie herstellen
            if(!bGeometryValid)
                ((E3dCompoundObject*)this)->ReCreateGeometry();

            // 3D Volumes schneiden sich, teste in der Geometrie
            // auf Basis der Projektion weiter
            if(((E3dCompoundObject*)this)->aDisplayGeometry.CheckHit(aFront, aBack, nTol))
            {
                return ((E3dCompoundObject*)this);
            }
        }
    }

    return 0L;
}

/*************************************************************************
|*
|* Geometrie des Objektes auf angegebenen Punkt zentrieren
|*
\************************************************************************/

void E3dCompoundObject::CenterObject(const Vector3D& rCenter)
{
    // Geometrie herstellen
    if(!bGeometryValid)
        ReCreateGeometry();

    Vector3D aOldCenter = aDisplayGeometry.GetCenter();
    Vector3D aMoveVector = rCenter - aOldCenter;
    Matrix4D aTransMat;

    aTransMat.Translate(aMoveVector);
    SetTransform(aTransMat * GetTransform()); // #112587#
}

/*************************************************************************
|*
|* Schattenattribute holen
|*
\************************************************************************/

Color E3dCompoundObject::GetShadowColor()
{
    return ((SdrShadowColorItem&)(GetObjectItem(SDRATTR_SHADOWCOLOR))).GetColorValue();
}

BOOL E3dCompoundObject::DrawShadowAsOutline()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    XFillStyle eFillStyle = ((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();
    XLineStyle eLineStyle = ((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
    BOOL bFillAttrIsNone = eFillStyle == XFILL_NONE;
    BOOL bLineAttrIsNone = eLineStyle == XLINE_NONE;
    return (bFillAttrIsNone && !bLineAttrIsNone);
}

INT32 E3dCompoundObject::GetShadowXDistance()
{
    return (long)((SdrShadowXDistItem&)(GetObjectItem(SDRATTR_SHADOWXDIST))).GetValue();
}

INT32 E3dCompoundObject::GetShadowYDistance()
{
    return (long)((SdrShadowYDistItem&)(GetObjectItem(SDRATTR_SHADOWYDIST))).GetValue();
}

UINT16 E3dCompoundObject::GetShadowTransparence()
{
    return (UINT16)((SdrShadowTransparenceItem&)(GetObjectItem(SDRATTR_SHADOWTRANSPARENCE))).GetValue();
}

BOOL E3dCompoundObject::DoDrawShadow()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    BOOL bRetval(FALSE);
    BOOL bShadOn = ((SdrShadowItem&)(rSet.Get(SDRATTR_SHADOW))).GetValue();

    if(bShadOn)
    {
        bRetval = TRUE;

        if(((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue() == XFILL_NONE)
        {
            if(((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue() == XLINE_NONE)
            {
                bRetval = FALSE;
            }
        }
    }
    return bRetval;
}

/*************************************************************************
|*
|* Objekt als WireFrame zeichnen
|*
\************************************************************************/

void E3dCompoundObject::DrawObjectWireframe(XOutputDevice& rXOut)
{
    UINT32 nPolyCounter = 0;
    UINT32 nEntityCounter = 0;
    UINT32 nUpperBound;
    Point aFirstPoint, aLastPoint, aNewPoint;
    B3dEntityBucket& rEntityBucket = GetDisplayGeometry().GetEntityBucket();
    GeometryIndexValueBucket& rIndexBucket = GetDisplayGeometry().GetIndexBucket();
    B3dTransformationSet& rTransSet = GetScene()->GetCameraSet();
    BOOL bDrawLine, bLastDrawLine;
    Vector3D aPoint;

    while(nPolyCounter < rIndexBucket.Count())
    {
        // Naechstes Primitiv
        nUpperBound = rIndexBucket[nPolyCounter++].GetIndex();
        bDrawLine = bLastDrawLine = rEntityBucket[nEntityCounter].IsEdgeVisible();
        aPoint = rTransSet.ObjectToViewCoor(rEntityBucket[nEntityCounter++].Point().GetVector3D());
        aFirstPoint.X() = (long)(aPoint.X() + 0.5);
        aFirstPoint.Y() = (long)(aPoint.Y() + 0.5);
        aLastPoint = aFirstPoint;

        // Polygon fuellen
        while(nEntityCounter < nUpperBound)
        {
            // Punkt holen und auf Weltkoordinaten umrechnen
            bDrawLine = rEntityBucket[nEntityCounter].IsEdgeVisible();
            aPoint = rTransSet.ObjectToViewCoor(rEntityBucket[nEntityCounter++].Point().GetVector3D());
            aNewPoint.X() = (long)(aPoint.X() + 0.5);
            aNewPoint.Y() = (long)(aPoint.Y() + 0.5);
            if(bLastDrawLine)
                rXOut.GetOutDev()->DrawLine(aLastPoint, aNewPoint);
            aLastPoint = aNewPoint;
            bLastDrawLine = bDrawLine;
        }

        // Polygon scliessen
        if(bLastDrawLine)
            rXOut.GetOutDev()->DrawLine(aLastPoint, aFirstPoint);
    }
}

/*************************************************************************
|*
|* Create vertical polygons for line polygon
|*
\************************************************************************/

// #i28528#
PolyPolygon3D E3dCompoundObject::ImpCompleteLinePolygon(const PolyPolygon3D& rLinePolyPoly, sal_uInt16 nPolysPerRun, sal_Bool bClosed)
{
    PolyPolygon3D aRetval;

    if(rLinePolyPoly.Count() && nPolysPerRun)
    {
        // get number of layers
        sal_uInt16 nLayers(rLinePolyPoly.Count() / nPolysPerRun);

        // add vertical Polygons if at least two horizontal ones exist
        if(nLayers > 1)
        {
            for(sal_uInt16 a(0); a < nPolysPerRun; a++)
            {
                const sal_uInt16 nPntCnt = rLinePolyPoly[a].GetPointCount();

                for(sal_uInt16 b(0); b < nPntCnt; b++)
                {
                    Polygon3D aNewVerPoly(bClosed ? nLayers + 1 : nLayers);

                    for(sal_uInt16 c(0); c < nLayers; c++)
                    {
                        aNewVerPoly[c] = rLinePolyPoly[(c * nPolysPerRun) + a][b];
                    }

                    // evtl. set first point again to close polygon
                    if(bClosed)
                        aNewVerPoly[aNewVerPoly.GetPointCount()] = aNewVerPoly[0];

                    // insert
                    aRetval.Insert(aNewVerPoly);
                }
            }
        }
    }

    return aRetval;
}

// #i28528#
void E3dCompoundObject::ImpCorrectLinePolygon(PolyPolygon3D& rLinePolyPoly, sal_uInt16 nPolysPerRun)
{
    if(rLinePolyPoly.Count() && nPolysPerRun)
    {
        // open closed polygons
        for(sal_uInt16 a(0); a < rLinePolyPoly.Count(); a++)
        {
            if(rLinePolyPoly[a].IsClosed())
            {
                rLinePolyPoly[a][rLinePolyPoly[a].GetPointCount()] = rLinePolyPoly[a][0];
                rLinePolyPoly[a].SetClosed(FALSE);
            }
        }
    }
}

/*************************************************************************
|*
|* Ein Segment fuer Extrude oder Lathe erzeugen
|*
\************************************************************************/

void E3dCompoundObject::ImpCreateSegment(
    const PolyPolygon3D& rFront,        // vorderes Polygon
    const PolyPolygon3D& rBack,         // hinteres Polygon
    const PolyPolygon3D* pPrev,         // smooth uebergang zu Vorgaenger
    const PolyPolygon3D* pNext,         // smooth uebergang zu Nachfolger
    BOOL bCreateFront,                  // vorderen Deckel erzeugen
    BOOL bCreateBack,                   // hinteren Deckel erzeugen
    double fPercentDiag,                // Anteil des Deckels an der Tiefe
    BOOL bSmoothLeft,                   // Glaetten der umlaufenden Normalen links
    BOOL bSmoothRight,                  // Glaetten der umlaufenden Normalen rechts
    BOOL bSmoothFrontBack,              // Glaetten der Abschlussflaechen
    double fSurroundFactor,             // Wertebereich der Texturkoordinaten im Umlauf
    double fTextureStart,               // TexCoor ueber Extrude-Tiefe
    double fTextureDepth,               // TexCoor ueber Extrude-Tiefe
    BOOL bDoCreateTexture,
    BOOL bDoCreateNormals,
    BOOL bCharacterExtrude,             // FALSE=exakt, TRUE=ohne Ueberschneidungen
    BOOL bRotateTexture90,              // Textur der Seitenflaechen um 90 Grad kippen
    // #i28528#
    PolyPolygon3D* pLineGeometryFront,  // For creation of line geometry front parts
    PolyPolygon3D* pLineGeometryBack,   // For creation of line geometry back parts
    PolyPolygon3D* pLineGeometry        // For creation of line geometry in-betweens
    )
{
    PolyPolygon3D aNormalsLeft, aNormalsRight;
    AddInBetweenNormals(rFront, rBack, aNormalsLeft, bSmoothLeft);
    AddInBetweenNormals(rFront, rBack, aNormalsRight, bSmoothRight);
    Vector3D aOffset = rBack.GetMiddle() - rFront.GetMiddle();

    // #i28528#
    sal_Bool bTakeCareOfLineGeometry(pLineGeometryFront != 0L || pLineGeometryBack != 0L || pLineGeometry != 0L);

    // Ausnahmen: Nicht geschlossen
    if(!rFront.IsClosed())
    {
        bCreateFront = FALSE;
    }
    if(!rBack.IsClosed())
    {
        bCreateBack = FALSE;
    }

    // Ausnahmen: Einfache Linie
    if(rFront[0].GetPointCount() < 3 || (!bCreateFront && !bCreateBack))
    {
        fPercentDiag = 0.0;
    }

    if(fPercentDiag == 0.0)
    {
        // Ohne Schraegen, Vorderseite
        if(bCreateFront)
        {
            PolyPolygon3D aNormalsFront;
            AddFrontNormals(rFront, aNormalsFront, aOffset);

            if(!bSmoothFrontBack)
                CreateFront(rFront, aNormalsFront, bDoCreateNormals, bDoCreateTexture);
            if(bSmoothLeft)
                AddFrontNormals(rFront, aNormalsLeft, aOffset);
            if(bSmoothFrontBack)
                CreateFront(rFront, aNormalsLeft, bDoCreateNormals, bDoCreateTexture);
        }
        else
        {
            if(pPrev)
                AddInBetweenNormals(*pPrev, rFront, aNormalsLeft, bSmoothLeft);
        }

        // Ohne Schraegen, Rueckseite
        if(bCreateBack)
        {
            PolyPolygon3D aNormalsBack;
            AddBackNormals(rBack, aNormalsBack, aOffset);

            if(!bSmoothFrontBack)
                CreateBack(rBack, aNormalsBack, bDoCreateNormals, bDoCreateTexture);
            if(bSmoothRight)
                AddBackNormals(rBack, aNormalsRight, aOffset);
            if(bSmoothFrontBack)
                CreateBack(rBack, aNormalsRight, bDoCreateNormals, bDoCreateTexture);
        }
        else
        {
            if(pNext)
                AddInBetweenNormals(rBack, *pNext, aNormalsRight, bSmoothRight);
        }

        // eigentliches Zwischenstueck
        CreateInBetween(rFront, rBack,
            aNormalsLeft, aNormalsRight,
            bDoCreateNormals,
            fSurroundFactor,
            fTextureStart,
            fTextureDepth,
            bRotateTexture90);

        // #78972#
        if(bTakeCareOfLineGeometry)
        {
            if(bCreateFront)
            {
                if(pLineGeometryFront) pLineGeometryFront->Insert(rFront);
            }
            else
            {
                if(pLineGeometry) pLineGeometry->Insert(rFront);
            }

            if(bCreateBack)
            {
                if(pLineGeometryBack) pLineGeometryBack->Insert(rBack);
            }
        }
    }
    else
    {
        // Mit Scraegen, Vorderseite
        PolyPolygon3D aLocalFront = rFront;
        PolyPolygon3D aLocalBack = rBack;
        double fExtrudeDepth, fDiagLen(0.0);
        double fTexMidStart = fTextureStart;
        double fTexMidDepth = fTextureDepth;

        if(bCreateFront || bCreateBack)
        {
            fExtrudeDepth = aOffset.GetLength();
            fDiagLen = fPercentDiag * fExtrudeDepth;
        }

        PolyPolygon3D aOuterFront;
        PolyPolygon3D aOuterBack;

        if(bCreateFront)
        {
            PolyPolygon3D aNormalsOuterFront;
            AddFrontNormals(aLocalFront, aNormalsOuterFront, aOffset);

            if(bCharacterExtrude)
            {
                // Polygon kopieren
                aOuterFront = aLocalFront;

                // notwendige Normalen erzeugen
                PolyPolygon3D aGrowDirection;
                AddInBetweenNormals(aLocalFront, aLocalBack, aGrowDirection, bSmoothLeft);

                // Groesse inneres Polygon merken
                Volume3D aOldSize(aLocalFront.GetPolySize());

                // Inneres Polygon vergroessern
                GrowPoly(aLocalFront, aGrowDirection, fDiagLen);

                // Inneres Polygon nach innen verschieben
                //GrowPoly(aLocalFront, aNormalsOuterFront, -fDiagLen);

                // Neue Groesse inneres Polygon feststellen
                Volume3D aNewSize(aLocalFront.GetPolySize());

                // Skalierung feststellen (nur X,Y)
                Vector3D aScaleVec(
                    (aNewSize.GetWidth() != 0.0) ? aOldSize.GetWidth() / aNewSize.GetWidth() : 1.0,
                    (aNewSize.GetHeight() != 0.0) ? aOldSize.GetHeight() / aNewSize.GetHeight() : 1.0,
                    (aNewSize.GetDepth() != 0.0) ? aOldSize.GetDepth() / aNewSize.GetDepth() : 1.0);

                // Transformation bilden
                Matrix4D aTransMat;
                aTransMat.Scale(aScaleVec);

                // aeusseres und inneres Polygon skalieren
                aLocalFront.Transform(aTransMat);
                aOuterFront.Transform(aTransMat);

                // Neue Groesse aktualisieren
                aNewSize = aLocalFront.GetPolySize();

                // Translation feststellen
                Vector3D aTransVec(
                    aOldSize.MinVec().X() - aNewSize.MinVec().X(),
                    aOldSize.MinVec().Y() - aNewSize.MinVec().Y(),
                    aOldSize.MinVec().Z() - aNewSize.MinVec().Z());

                // Transformation bilden
                aTransMat.Identity();
                aTransMat.Translate(aTransVec);

                // aeusseres und inneres Polygon skalieren
                aLocalFront.Transform(aTransMat);
                aOuterFront.Transform(aTransMat);

                // move aLocalFront again, scale and translate has moved it back
                GrowPoly(aLocalFront, aNormalsOuterFront, -fDiagLen);
            }
            else
            {
                // Polygon kopieren
                aOuterFront = aLocalFront;

                // notwendige Normalen erzeugen
                PolyPolygon3D aGrowDirection;
                AddInBetweenNormals(aLocalFront, aLocalBack, aGrowDirection, bSmoothLeft);

                // Aeusseres Polygon verkleinern
                GrowPoly(aOuterFront, aGrowDirection, -fDiagLen);
                aOuterFront.CorrectGrownPoly(aLocalFront);

                // Inneres Polygon nach innen verschieben
                GrowPoly(aLocalFront, aNormalsOuterFront, -fDiagLen);
            }

            // eventuell noch glaetten
            if(bSmoothLeft)
            {
                if(bSmoothFrontBack)
                    AddInBetweenNormals(aOuterFront, aLocalFront, aNormalsOuterFront, bSmoothLeft);
                AddInBetweenNormals(aOuterFront, aLocalFront, aNormalsLeft, bSmoothLeft);
            }

            // vordere Zwischenstuecke erzeugen
            CreateInBetween(aOuterFront, aLocalFront,
                aNormalsOuterFront, aNormalsLeft,
                bDoCreateNormals,
                fSurroundFactor,
                fTextureStart,
                fTextureDepth * fPercentDiag,
                bRotateTexture90);

            // Vorderseite erzeugen
            CreateFront(aOuterFront, aNormalsOuterFront, bDoCreateNormals, bDoCreateTexture);

            // Weitere Texturwerte setzen
            fTexMidStart += fTextureDepth * fPercentDiag;
            fTexMidDepth -= fTextureDepth * fPercentDiag;
        }
        else
        {
            if(pPrev)
                AddInBetweenNormals(*pPrev, rFront, aNormalsLeft, bSmoothLeft);
        }

        // Mit Scraegen, Rueckseite
        if(bCreateBack)
        {
            PolyPolygon3D aNormalsOuterBack;
            AddBackNormals(aLocalBack, aNormalsOuterBack, aOffset);

            if(bCharacterExtrude)
            {
                // Polygon kopieren
                aOuterBack = aLocalBack;

                // notwendige Normalen erzeugen
                PolyPolygon3D aGrowDirection;
                AddInBetweenNormals(aLocalFront, aLocalBack, aGrowDirection, bSmoothRight);

                // Groesse inneres Polygon merken
                Volume3D aOldSize(aLocalBack.GetPolySize());

                // Inneres Polygon vergroessern
                GrowPoly(aLocalBack, aGrowDirection, fDiagLen);

                // Inneres Polygon nach innen verschieben
                //GrowPoly(aLocalBack, aNormalsOuterBack, -fDiagLen);

                // Neue Groesse inneres Polygon feststellen
                Volume3D aNewSize(aLocalBack.GetPolySize());

                // Skalierung feststellen (nur X,Y)
                Vector3D aScaleVec(
                    (aNewSize.GetWidth() != 0.0) ? aOldSize.GetWidth() / aNewSize.GetWidth() : 1.0,
                    (aNewSize.GetHeight() != 0.0) ? aOldSize.GetHeight() / aNewSize.GetHeight() : 1.0,
                    (aNewSize.GetDepth() != 0.0) ? aOldSize.GetDepth() / aNewSize.GetDepth() : 1.0);

                // Transformation bilden
                Matrix4D aTransMat;
                aTransMat.Scale(aScaleVec);

                // aeusseres und inneres Polygon skalieren
                aLocalBack.Transform(aTransMat);
                aOuterBack.Transform(aTransMat);

                // Neue Groesse aktualisieren
                aNewSize = aLocalBack.GetPolySize();

                // Translation feststellen
                Vector3D aTransVec(
                    aOldSize.MinVec().X() - aNewSize.MinVec().X(),
                    aOldSize.MinVec().Y() - aNewSize.MinVec().Y(),
                    aOldSize.MinVec().Z() - aNewSize.MinVec().Z());

                // Transformation bilden
                aTransMat.Identity();
                aTransMat.Translate(aTransVec);

                // aeusseres und inneres Polygon skalieren
                aLocalBack.Transform(aTransMat);
                aOuterBack.Transform(aTransMat);

                // move aLocalBack again, scale and translate has moved it back
                GrowPoly(aLocalBack, aNormalsOuterBack, -fDiagLen);
            }
            else
            {
                // Polygon kopieren
                aOuterBack = aLocalBack;

                // notwendige Normalen erzeugen
                PolyPolygon3D aGrowDirection;
                AddInBetweenNormals(aLocalFront, aLocalBack, aGrowDirection, bSmoothRight);

                // Aeusseres Polygon verkleinern
                GrowPoly(aOuterBack, aGrowDirection, -fDiagLen);
                aOuterBack.CorrectGrownPoly(aLocalBack);

                // Inneres Polygon nach innen verschieben
                GrowPoly(aLocalBack, aNormalsOuterBack, -fDiagLen);
            }

            // eventuell noch glaetten
            if(bSmoothRight)
            {
                if(bSmoothFrontBack)
                    AddInBetweenNormals(aLocalBack, aOuterBack, aNormalsOuterBack, bSmoothRight);
                AddInBetweenNormals(aLocalBack, aOuterBack, aNormalsRight, bSmoothRight);
            }

            // vordere Zwischenstuecke erzeugen
            // hintere Zwischenstuecke erzeugen
            CreateInBetween(aLocalBack, aOuterBack,
                aNormalsRight, aNormalsOuterBack,
                bDoCreateNormals,
                fSurroundFactor,
                fTextureStart + (fTextureDepth * (1.0 - fPercentDiag)),
                fTextureDepth * fPercentDiag,
                bRotateTexture90);

            // Rueckseite erzeugen
            CreateBack(aOuterBack, aNormalsOuterBack, bDoCreateNormals, bDoCreateTexture);

            // Weitere Texturwerte setzen
            fTexMidDepth -= fTextureDepth * fPercentDiag;
        }
        else
        {
            if(pNext)
                AddInBetweenNormals(rBack, *pNext, aNormalsRight, bSmoothRight);
        }

        // eigentliches Zwischenstueck
        CreateInBetween(aLocalFront, aLocalBack,
            aNormalsLeft, aNormalsRight,
            bDoCreateNormals,
            fSurroundFactor,
            fTexMidStart,
            fTexMidDepth,
            bRotateTexture90);

        // #78972#
        if(bTakeCareOfLineGeometry)
        {
            if(bCreateFront)
            {
                if(pLineGeometryFront) pLineGeometryFront->Insert(aOuterFront);
            }

            if(bCreateFront)
            {
                if(pLineGeometryFront) pLineGeometryFront->Insert(aLocalFront);
            }
            else
            {
                if(pLineGeometry) pLineGeometry->Insert(aLocalFront);
            }

            if(bCreateBack && pLineGeometryBack)
            {
                pLineGeometryBack->Insert(aLocalBack);
                pLineGeometryBack->Insert(aOuterBack);
            }
        }
    }
}

/*************************************************************************
|*
|* Parameter Geometrieerzeugung setzen
|*
\************************************************************************/

void E3dCompoundObject::SetCreateNormals(BOOL bNew)
{
    if(bCreateNormals != bNew)
    {
        bCreateNormals = bNew;
        bGeometryValid = FALSE;
    }
}

void E3dCompoundObject::SetCreateTexture(BOOL bNew)
{
    if(bCreateTexture != bNew)
    {
        bCreateTexture = bNew;
        bGeometryValid = FALSE;
    }
}

/*************************************************************************
|*
|* DisplayGeometry rausruecken
|*
\************************************************************************/

B3dGeometry& E3dCompoundObject::GetDisplayGeometry()
{
    // Geometrie herstellen
    if(!bGeometryValid)
        ReCreateGeometry();

    return aDisplayGeometry;
}

/*************************************************************************
|*
|* Material des Objektes
|*
\************************************************************************/

void E3dCompoundObject::SetMaterialAmbientColor(const Color& rColor)
{
    if(aMaterialAmbientColor != rColor)
    {
        aMaterialAmbientColor = rColor;
    }
}

void E3dCompoundObject::SetBackMaterial(const B3dMaterial& rNew)
{
    if(aBackMaterial != rNew)
    {
        aBackMaterial = rNew;
    }
}

void E3dCompoundObject::SetUseDifferentBackMaterial(BOOL bNew)
{
    if(bUseDifferentBackMaterial != bNew)
    {
        bUseDifferentBackMaterial = bNew;
    }
}

/*************************************************************************
|*
|* 3D Ausgabe
|*
\************************************************************************/

void ImplGet3DLineGeometry(const SfxItemSet& rSet, const ::basegfx::B3DPolyPolygon& rLinePolyPolygon,
    ::basegfx::B3DPolyPolygon& rAreaPolyPoly, ::basegfx::B3DPolyPolygon& rNormalPolyPoly)
{
    if(rLinePolyPolygon.count())
    {
        // detect if lines need to be drawn specifically
        sal_Int32 nLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
        XLineStyle aLineStyle = ((const XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();

        if(aLineStyle == XLINE_SOLID)
        {
            // add line segments without change
            rAreaPolyPoly = rLinePolyPolygon;
        }
        else
        {
            // create dashed line segments
            ::std::vector<double> aDotDashArray;
            XDash aDash = ((const XLineDashItem&)(rSet.Get(XATTR_LINEDASH))).GetDashValue();
            double fFullDashDotLen = ImpCreateDotDashArray(aDotDashArray, aDash, nLineWidth);

            // convert to new polygon class
            rAreaPolyPoly = rLinePolyPolygon;

            // apply line dashing
            rAreaPolyPoly = ::basegfx::tools::applyLineDashing(rAreaPolyPoly, aDotDashArray, fFullDashDotLen);
        }

        if(0L != nLineWidth)
        {
            // prepare rAreaPolyPoly for output data
            const ::basegfx::B3DPolyPolygon aSourcePolyPolygon(rAreaPolyPoly);
            rAreaPolyPoly.clear();
            const double fDistance(nLineWidth / 2.0);

            for(sal_uInt32 nInd(0L); nInd < aSourcePolyPolygon.count(); nInd++)
            {
                ::basegfx::B3DPolygon aLinePoly(aSourcePolyPolygon.getB3DPolygon(nInd));
                const sal_uInt32 nOrigCount(aLinePoly.count());

                if(nOrigCount)
                {
                    const sal_uInt32 nCount(aLinePoly.isClosed() ? nOrigCount : nOrigCount - 1L);

                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        ::basegfx::B3DPoint aStart(aLinePoly.getB3DPoint(a));
                        ::basegfx::B3DPoint aEnd(aLinePoly.getB3DPoint((a + 1L) % nOrigCount));
                        ::basegfx::B3DVector aVector(aEnd - aStart);

                        // test length using scalar with itself, gives the quadrat of the length
                        const double fScalar(aVector.scalar(aVector));

                        if(0.0 != fScalar)
                        {
                            // normalize vector, use known scalar
                            if(1.0 != fScalar)
                            {
                                const double fLen(sqrt(fScalar));
                                aVector.setX(aVector.getX() / fLen);
                                aVector.setY(aVector.getY() / fLen);
                                aVector.setZ(aVector.getZ() / fLen);
                            }

                            // calculate opposite vector and the two axes perpendicular
                            // to it
                            ::basegfx::B3DVector aOppositeVector(-aVector);
                            ::basegfx::B3DVector aStartVec(-aVector.getY(), aVector.getZ(), aVector.getX());
                            ::basegfx::B3DVector aXAxis = aVector.getPerpendicular(aStartVec);
                            ::basegfx::B3DVector aYAxis = aVector.getPerpendicular(aXAxis);

                            // prepare angle and angle increment
                            double fAngle(0.0);
                            const double fAngleIncrement(F_PI * (2.0 / 6.0));
                            double fSin(sin(fAngle));
                            double fCos(cos(fAngle));

                            // prepare start vectors
                            ::basegfx::B3DVector aCurrentVector((aXAxis * fCos) + (aYAxis * fSin));
                            ::basegfx::B3DVector aOffset(aCurrentVector * fDistance);
                            ::basegfx::B3DPoint aLeft(aStart + aOffset);
                            ::basegfx::B3DPoint aRight(aEnd + aOffset);

                            for(sal_uInt32 b(0L); b < 6; b++)
                            {
                                fAngle += fAngleIncrement;
                                fSin = sin(fAngle);
                                fCos = cos(fAngle);

                                ::basegfx::B3DVector aNextVector((aXAxis * fCos) + (aYAxis * fSin));
                                aOffset = ::basegfx::B3DVector(aNextVector * fDistance);
                                ::basegfx::B3DPoint aNextLeft(aStart + aOffset);
                                ::basegfx::B3DPoint aNextRight(aEnd + aOffset);
                                ::basegfx::B3DPolygon aLine;
                                ::basegfx::B3DPolygon aNormal;

                                aLine.append(aStart);
                                aLine.append(aNextLeft);
                                aLine.append(aLeft);
                                aLine.append(aNextRight);
                                aLine.append(aRight);
                                aLine.append(aEnd);

                                aNormal.append(::basegfx::B3DPoint(aVector));
                                aNormal.append(::basegfx::B3DPoint(aNextVector));
                                aNormal.append(::basegfx::B3DPoint(aCurrentVector));
                                aNormal.append(::basegfx::B3DPoint(aNextVector));
                                aNormal.append(::basegfx::B3DPoint(aCurrentVector));
                                aNormal.append(::basegfx::B3DPoint(aOppositeVector));

                                rAreaPolyPoly.append(aLine);
                                rNormalPolyPoly.append(aNormal);

                                aLeft = aNextLeft;
                                aRight = aNextRight;
                                aCurrentVector = aNextVector;
                            }
                        }
                    }
                }
            }
        }
    }
}

void E3dCompoundObject::Paint3D(XOutputDevice& rOut, Base3D* pBase3D,
    const SdrPaintInfoRec& rInfoRec, UINT16 nDrawFlags)
{
    // call parent, draw all subobjects
    E3dObject::Paint3D(rOut, pBase3D, rInfoRec, nDrawFlags);

    // Feststellen, ob das Objekt dargestellt werden muss, was die Layer angeht
    BOOL bPrinter = (pBase3D->GetOutputDevice()->GetOutDevType()==OUTDEV_PRINTER);
    const SetOfByte* pVisiLayer=&rInfoRec.aPaintLayer;
    E3dScene* pScene = GetScene();
    BOOL bOnlySelectedCriteria = (pScene && (!pScene->DoDrawOnlySelected() || GetSelected()));

    if((!bPrinter || IsPrintable()) && pVisiLayer->IsSet(GetLayer()) && bOnlySelectedCriteria)
    {
        // Ausgabeparameter setzen
        BOOL bDrawOutline;
        BOOL bDrawObject;
        BOOL bIsLineDraft((rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTLINE) != 0);
        BOOL bIsFillDraft((rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTFILL) != 0);
        BOOL bGhosted((rInfoRec.pPV && rInfoRec.pPV->GetView().DoVisualizeEnteredGroup()) ? rInfoRec.bNotActive : FALSE);
        SetBase3DParams(rOut, pBase3D, bDrawObject, bDrawOutline, nDrawFlags,
            bGhosted, bIsLineDraft, bIsFillDraft);

        // Culling?
        pBase3D->SetCullMode(GetDoubleSided() ? Base3DCullNone : Base3DCullBack);

        // Objekt flat darstellen?
        BOOL bForceFlat = ((GetNormalsKind() > 0) && !(GetNormalsKind() > 1));
        pBase3D->SetForceFlat(bForceFlat);

        // Geometrie ausgeben
        if(bDrawObject)
        {
            // #92030# for E3dPolygonObj, take flag at created DisplayGeometry into account
            // which may not allow this object to be drawn filled
            if(!GetDisplayGeometry().IsOutline())
            {
                pBase3D->DrawPolygonGeometry(GetDisplayGeometry());
            }
        }

        // Outline ausgeben
        if(bDrawOutline)
        {
            // #79585#
            pBase3D->SetActiveTexture();
            const SfxItemSet& rSet = GetMergedItemSet();

            // get line geometry
            ::basegfx::B3DPolyPolygon aLinePolyPolygon(Get3DLineGeometry());
            ::basegfx::B3DPolyPolygon aAreaPolyPolygon;
            ::basegfx::B3DPolyPolygon aNormalPolyPolygon;
            ImplGet3DLineGeometry(rSet, aLinePolyPolygon, aAreaPolyPolygon, aNormalPolyPolygon);

            if(aAreaPolyPolygon.count())
            {
                if(aNormalPolyPolygon.count())
                {
                    // draw as 3d tubes
                    pBase3D->SetRenderMode(Base3DRenderFill);

                    // set line color as color
                    {
                        Color aColorLine = ((const XLineColorItem&)(rSet.Get(XATTR_LINECOLOR))).GetColorValue();
                        sal_uInt16 nLineTransparence = ((const XLineTransparenceItem&)(rSet.Get(XATTR_LINETRANSPARENCE))).GetValue();

                        if(bGhosted)
                        {
                            aColorLine = Color(
                                (aColorLine.GetRed() >> 1) + 0x80,
                                (aColorLine.GetGreen() >> 1) + 0x80,
                                (aColorLine.GetBlue() >> 1) + 0x80);
                        }

                        // prepare custom colors for linear transparency and black/white mode
                        Color aColorLineWithTransparency(aColorLine);
                        aColorLineWithTransparency.SetTransparency((UINT8)(nLineTransparence * 255 / 100));

                        // set base materials (if no drawmode is set)
                        pBase3D->SetMaterial(GetMaterialSpecular(), Base3DMaterialSpecular);
                        pBase3D->SetMaterial(GetMaterialEmission(), Base3DMaterialEmission);
                        pBase3D->SetShininess(GetMaterialSpecularIntensity());

                        // now test the different draw modes and cases
                        if((pBase3D->GetOutputDevice()->GetDrawMode() & DRAWMODE_WHITEFILL) != 0)
                        {
                            Color aColorWhite(COL_WHITE);
                            Color aColorWhiteWithTransparency(COL_WHITE);
                            aColorWhiteWithTransparency.SetTransparency((UINT8)(nLineTransparence * 255 / 100));

                            // set material to black and white mode
                            pBase3D->SetMaterial(aColorWhite, Base3DMaterialAmbient);
                            pBase3D->SetMaterial(aColorWhiteWithTransparency, Base3DMaterialDiffuse);
                        }
                        else if((pBase3D->GetOutputDevice()->GetDrawMode() & DRAWMODE_SETTINGSFILL) != 0)
                        {
                            Color aColorFill(Application::GetSettings().GetStyleSettings().GetWindowColor());
                            Color aColorFillWithTransparency(aColorFill);
                            aColorFillWithTransparency.SetTransparency((UINT8)(nLineTransparence * 255 / 100));

                            // set material to black and white mode
                            pBase3D->SetMaterial(aColorFill, Base3DMaterialAmbient);
                            pBase3D->SetMaterial(aColorFillWithTransparency, Base3DMaterialDiffuse);
                        }
                        else
                        {
                            // set material to base color
                            pBase3D->SetMaterial(aColorLine, Base3DMaterialAmbient);
                            pBase3D->SetMaterial(aColorLineWithTransparency, Base3DMaterialDiffuse);
                        }
                    }

                    for(sal_uInt32 a(0L); a < aAreaPolyPolygon.count(); a++)
                    {
                        // start new primitive
                        ::basegfx::B3DPolygon aPolygon(aAreaPolyPolygon.getB3DPolygon(a));
                        ::basegfx::B3DPolygon aNormals(aNormalPolyPolygon.getB3DPolygon(a));
                        pBase3D->StartPrimitive(Base3DTriangleStrip);

                        for(sal_uInt32 b(0); b < aPolygon.count(); b++)
                        {
                            ::basegfx::B3DPoint aPoint(aPolygon.getB3DPoint(b));
                            ::basegfx::B3DPoint aNormal(aNormals.getB3DPoint(b));

                            Vector3D aVec(aPoint.getX(), aPoint.getY(), aPoint.getZ());
                            Vector3D aNorm(aNormal.getX(), aNormal.getY(), aNormal.getZ());

                            pBase3D->AddVertex(aVec, aNorm);
                        }

                        // draw primitive
                        pBase3D->EndPrimitive();
                    }
                }
                else
                {
                    // draw as lines
                    BOOL bLightingWasEnabled = pBase3D->GetLightGroup()->IsLightingEnabled();
                    pBase3D->GetLightGroup()->EnableLighting(FALSE);
                    pBase3D->SetLightGroup(pBase3D->GetLightGroup());
                    pBase3D->SetRenderMode(Base3DRenderLine);
                    pBase3D->SetPolygonOffset(Base3DPolygonOffsetLine, TRUE);

                    for(sal_uInt32 a(0L); a < aAreaPolyPolygon.count(); a++)
                    {
                        // start new primitive
                        ::basegfx::B3DPolygon aPolygon(aAreaPolyPolygon.getB3DPolygon(a));
                        pBase3D->StartPrimitive(Base3DLineStrip);

                        for(sal_uInt32 b(0); b < aPolygon.count(); b++)
                        {
                            ::basegfx::B3DPoint aPoint(aPolygon.getB3DPoint(b));
                            Vector3D aVec(aPoint.getX(), aPoint.getY(), aPoint.getZ());
                            pBase3D->AddVertex(aVec);
                        }

                        // draw primitive
                        pBase3D->EndPrimitive();
                    }

                    pBase3D->SetPolygonOffset(Base3DPolygonOffsetLine, FALSE);
                    pBase3D->GetLightGroup()->EnableLighting(bLightingWasEnabled);
                    pBase3D->SetLightGroup(pBase3D->GetLightGroup());
                }
            }
        }
    }

#ifdef DBG_UTIL     // SnapRect und BoundRect zum testen zeichnen
    static BOOL bDoDrawSnapBoundToMakeThemVisible = FALSE;
    if(bDoDrawSnapBoundToMakeThemVisible)
    {
        OutputDevice* pOut = rOut.GetOutDev();

        // SnapRect in Rot
        Rectangle aTempRect = GetSnapRect();
        pOut->SetLineColor(Color(COL_RED));
        pOut->SetFillColor();
        pOut->DrawRect(aTempRect);

        // BoundRect in Gruen
        aTempRect = GetCurrentBoundRect();
        pOut->SetLineColor(Color(COL_GREEN));
        pOut->SetFillColor();
        pOut->DrawRect(aTempRect);
    }
#endif
}

/*************************************************************************
|*
|* Objekt als Kontur in das Polygon einfuegen
|*
\************************************************************************/

void E3dCompoundObject::TakeContour3D(XPolyPolygon& rPoly)
{
    // call parent
    E3dObject::TakeContour3D(rPoly);

    // Kontur dieses Objektes liefern
    UINT32 nPolyCounter = 0;
    UINT32 nEntityCounter = 0;
    UINT32 nUpperBound;
    B3dEntityBucket& rEntityBucket = GetDisplayGeometry().GetEntityBucket();
    GeometryIndexValueBucket& rIndexBucket = GetDisplayGeometry().GetIndexBucket();
    B3dTransformationSet& rTransSet = GetScene()->GetCameraSet();
    Vector3D aPoint;
    Point aNewPoint;

    // ObjectTrans setzen
    Matrix4D mTransform = GetFullTransform();
    rTransSet.SetObjectTrans(mTransform);

    while(nPolyCounter < rIndexBucket.Count())
    {
        // Naechstes Primitiv
        nUpperBound = rIndexBucket[nPolyCounter++].GetIndex();
        XPolygon aNewPart(UINT16(nUpperBound - nEntityCounter));
        UINT16 nIndex = 0;

        while(nEntityCounter < nUpperBound)
        {
            aPoint = rTransSet.ObjectToViewCoor(rEntityBucket[nEntityCounter++].Point().GetVector3D());
            aNewPart[nIndex  ].X() = (long)(aPoint.X() + 0.5);
            aNewPart[nIndex++].Y() = (long)(aPoint.Y() + 0.5);
        }

        // Teilprimitiv einfuegen
        rPoly.Insert(aNewPart);
    }

    // add shadow now too (#61279#)
    PolyPolygon3D aShadowPolyPoly;
    ImpGetShadowPolygon(aShadowPolyPoly);

    // invert Y coor cause of GetPolygon() later
    Matrix4D aTransMat;
    aTransMat.Scale(1.0, -1.0, 1.0);
    aShadowPolyPoly.Transform(aTransMat);

    for(UINT16 a = 0; a < aShadowPolyPoly.Count(); a++)
    {
        XPolygon aNewPart(aShadowPolyPoly[a].GetPolygon());
        rPoly.Insert(aNewPart);
    }
}

/*************************************************************************
|*
|* Schatten fuer 3D-Objekte zeichnen
|*
\************************************************************************/

void E3dCompoundObject::DrawShadows(Base3D *pBase3D, XOutputDevice& rXOut,
    const Rectangle& rBound, const Volume3D& rVolume,
    const SdrPaintInfoRec& rInfoRec)
{
    // call parent
    E3dObject::DrawShadows(pBase3D, rXOut, rBound, rVolume, rInfoRec);

    // Schatten fuer dieses Objekt zeichnen
    // Feststellen, ob das Objekt dargestellt werden muss, was die Layer angeht
    BOOL bPrinter = (pBase3D->GetOutputDevice()->GetOutDevType()==OUTDEV_PRINTER);
    const SetOfByte* pVisiLayer=&rInfoRec.aPaintLayer;
    if(DoDrawShadow()
        && (!bPrinter || IsPrintable())
        && pVisiLayer->IsSet(GetLayer()))
    {
        // ObjectTrans setzen
        Matrix4D mTransform = GetFullTransform();
        GetScene()->GetCameraSet().SetObjectTrans(mTransform);

        // Schattenpolygon holen
        PolyPolygon3D aShadowPoly;
        ImpGetShadowPolygon(aShadowPoly);

        // invert Y coor cause of GetPolyPolygon() in ImpDrawShadowPolygon() later
        Matrix4D aTransMat;
        aTransMat.Scale(1.0, -1.0, 1.0);
        aShadowPoly.Transform(aTransMat);

        // ...und Zeichnen
        ImpDrawShadowPolygon(aShadowPoly, rXOut);
    }
}

void E3dCompoundObject::ImpGetShadowPolygon(PolyPolygon3D& rPoly)
{
    // #79585#
    sal_Int32 nXDist(GetShadowXDistance());
    sal_Int32 nYDist(GetShadowYDistance());
    BOOL bDrawAsOutline(DrawShadowAsOutline());
    PolyPolygon3D aLinePolyPolygon;
    B3dTransformationSet& rTransSet = GetScene()->GetCameraSet();
    const SfxItemSet& rSet = GetObjectItemSet();
    XLineStyle aLineStyle = ((const XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
    //sal_Int32 nLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();

    if(!bDrawAsOutline)
    {
        // add basic polygon geometry, generate from 3D bucket
        B3dEntityBucket& rEntityBucket = ((E3dCompoundObject*)this)->GetDisplayGeometry().GetEntityBucket();
        GeometryIndexValueBucket& rIndexBucket = ((E3dCompoundObject*)this)->GetDisplayGeometry().GetIndexBucket();
        sal_uInt32 nPolyCounter(0);
        sal_uInt32 nEntityCounter(0);

        while(nPolyCounter < rIndexBucket.Count())
        {
            // next primitive
            sal_uInt32 nUpperBound(rIndexBucket[nPolyCounter++].GetIndex());
            Polygon3D aNewPolygon((sal_uInt16)(nUpperBound - nEntityCounter));
            sal_uInt16 nIndex(0);

            while(nEntityCounter < nUpperBound)
                aNewPolygon[nIndex++] = rEntityBucket[nEntityCounter++].Point().GetVector3D();

            aNewPolygon.SetClosed(TRUE);
            aLinePolyPolygon.Insert(aNewPolygon);
        }
    }

    if(bDrawAsOutline || (XLINE_NONE != aLineStyle))
    {
        // get line geometry
        ::basegfx::B3DPolyPolygon aBasicLinePolyPoly(Get3DLineGeometry());
        ::basegfx::B3DPolyPolygon aAreaPolyPolygon;
        ::basegfx::B3DPolyPolygon aNormalPolyPolygon;
        ImplGet3DLineGeometry(rSet, aBasicLinePolyPoly, aAreaPolyPolygon, aNormalPolyPolygon);

        if(aAreaPolyPolygon.count())
        {
            if(aNormalPolyPolygon.count())
            {
                // draw as 3d tubes
                for(sal_uInt32 a(0L); a < aAreaPolyPolygon.count(); a++)
                {
                    // start new primitive
                    ::basegfx::B3DPolygon aPolygon(aAreaPolyPolygon.getB3DPolygon(a));
                    Polygon3D aNewPolygon(4);

                    ::basegfx::B3DPoint aPointA(aPolygon.getB3DPoint(1));
                    Vector3D aVecA(aPointA.getX(), aPointA.getY(), aPointA.getZ());

                    ::basegfx::B3DPoint aPointB(aPolygon.getB3DPoint(3));
                    Vector3D aVecB(aPointB.getX(), aPointB.getY(), aPointB.getZ());

                    ::basegfx::B3DPoint aPointC(aPolygon.getB3DPoint(4));
                    Vector3D aVecC(aPointC.getX(), aPointC.getY(), aPointC.getZ());

                    ::basegfx::B3DPoint aPointD(aPolygon.getB3DPoint(2));
                    Vector3D aVecD(aPointD.getX(), aPointD.getY(), aPointD.getZ());

                    aNewPolygon[0] = aVecA;
                    aNewPolygon[1] = aVecB;
                    aNewPolygon[2] = aVecC;
                    aNewPolygon[3] = aVecD;
                    aNewPolygon.SetClosed(sal_True);

                    aLinePolyPolygon.Insert(aNewPolygon);
                }
            }
            else
            {
                // draw as lines
                aBasicLinePolyPoly = aAreaPolyPolygon;
                aLinePolyPolygon.Insert(aBasicLinePolyPoly);
            }
        }
    }

    if(aLinePolyPolygon.Count())
    {
        if(GetShadow3D())
        {
            // 3D Schatten. Nimm Lichtquelle und Ebene. Projiziere
            // die Punkte und jage sie durch die 3D Darstellung.
            Vector3D aLampPositionOrDirection;
            BOOL bDirectionalSource(TRUE);
            Vector3D aGroundPosition;
            Vector3D aGroundDirection;
            B3dLightGroup& rLightGroup = GetScene()->GetLightGroup();

            // Lampe waehlen
            Base3DLightNumber aLightNumber = Base3DLight0;
            BOOL bLightNumberValid(FALSE);

            while(!bLightNumberValid && aLightNumber <= Base3DLight7)
            {
                if(rLightGroup.IsEnabled(aLightNumber))
                    bLightNumberValid = TRUE;
                else
                    aLightNumber = (Base3DLightNumber)((UINT16)aLightNumber + 1);
            }

            if(bLightNumberValid)
            {
                // Position oder Vektor aus der Lampe extrahieren
                if(rLightGroup.IsDirectionalSource(aLightNumber))
                {
                    // Nur Richtung vorhanden
                    aLampPositionOrDirection = -rLightGroup.GetDirection(aLightNumber);
                    aLampPositionOrDirection.Normalize();
                }
                else
                {
                    // Nur Position vorhanden
                    aLampPositionOrDirection = rLightGroup.GetPosition(aLightNumber);
                    bDirectionalSource = FALSE;
                }

                // Ebene holen, Richtung in Augkoordinaten
                aGroundDirection = -GetScene()->GetShadowPlaneDirection();
                aGroundDirection.Normalize();

                // Ist die Lampe auch vor der Ebene?
                Vector3D aLightNormal = aLampPositionOrDirection;
                if(!bDirectionalSource)
                {
                    // Nur Position vorhanden, berechne einen Lichtvektor
                    aLightNormal = aLinePolyPolygon[0][0] - aLampPositionOrDirection;
                    aLightNormal.Normalize();
                }

                double fLightAndNormal = aLightNormal.Scalar(aGroundDirection);
                B3dVolume aVolume = rTransSet.GetDeviceVolume();

                // auf Augkoordinaten umstellen
                double fTemp = aVolume.MinVec().Z();
                aVolume.MinVec().Z() = -aVolume.MaxVec().Z();
                aVolume.MaxVec().Z() = -fTemp;

                if(fLightAndNormal > 0.0)
                {
                    // Position der Ebene in Augkoordinaten setzen
                    aGroundPosition.X() = (aGroundDirection.X() < 0.0) ? aVolume.MinVec().X() : aVolume.MaxVec().X();
                    aGroundPosition.Y() = (aGroundDirection.Y() < 0.0) ? aVolume.MinVec().Y() : aVolume.MaxVec().Y();
                    aGroundPosition.Z() = aVolume.MinVec().Z() - ((aVolume.MaxVec().Z() - aVolume.MinVec().Z()) / 8.0);

                    // Skalar der Ebenengleichung holen
                    double fGroundScalar = -aGroundPosition.Scalar(aGroundDirection);

                    // ObjectTrans setzen
                    BOOL bPolygonVisible(TRUE);
                    Matrix4D mTransform = GetFullTransform();
                    rTransSet.SetObjectTrans(mTransform);

                    for(sal_uInt16 a(0); a < aLinePolyPolygon.Count(); a++)
                    {
                        Polygon3D& rLinePoly = aLinePolyPolygon[a];
                        Polygon3D aPoly(rLinePoly.GetPointCount());
                        sal_uInt16 nPolyPos(0);

                        for(sal_uInt16 b(0); b < rLinePoly.GetPointCount(); b++)
                        {
                            // Naechsten Punkt holen
                            Vector3D aPoint = rLinePoly[b];

                            // Auf Augkoordinaten umrechnen
                            aPoint = rTransSet.ObjectToEyeCoor(aPoint);

                            // Richtung bestimmen
                            Vector3D aDirection = aLampPositionOrDirection;
                            if(!bDirectionalSource)
                            {
                                aDirection = aPoint - aLampPositionOrDirection;
                                aDirection.Normalize();
                            }

                            // Schnittpunkt berechnen (N.D)
                            double fDiv = aGroundDirection.Scalar(aDirection);
                            if(fabs(fDiv) < SMALL_DVALUE)
                            {
                                bPolygonVisible = FALSE;
                            }
                            else
                            {
                                fDiv = -((fGroundScalar + aGroundDirection.Scalar(aPoint)) / fDiv);
                                aPoint += aDirection * fDiv;
                            }

                            // Punkt normal transformieren
                            if(bPolygonVisible)
                            {
                                // Auf ViewKoordinaten
                                Vector3D aShadowPoint = rTransSet.EyeToViewCoor(aPoint);
                                aPoly[nPolyPos++] = Vector3D(
                                    aShadowPoint.X() + (double)nXDist,
                                    aShadowPoint.Y() + (double)nYDist,
                                    0.0);
                            }
                        }

                        // Teilpolygon einfuegen
                        if(nPolyPos)
                        {
                            aPoly.SetClosed(rLinePoly.IsClosed());
                            rPoly.Insert(aPoly);
                        }
                    }
                }
            }
        }
        else
        {
            // ObjectTrans setzen
            Matrix4D mTransform = GetFullTransform();
            rTransSet.SetObjectTrans(mTransform);

            for(sal_uInt16 a(0); a < aLinePolyPolygon.Count(); a++)
            {
                Polygon3D& rLinePoly = aLinePolyPolygon[a];
                Polygon3D aPoly(rLinePoly.GetPointCount());
                sal_uInt16 nPolyPos(0);

                // Polygon fuellen
                for(sal_uInt16 b(0); b < rLinePoly.GetPointCount(); b++)
                {
                    // Naechsten Punkt holen
                    Vector3D aPoint = rLinePoly[b];
                    aPoint = rTransSet.ObjectToViewCoor(aPoint);
                    aPoly[nPolyPos++] = Vector3D(
                        aPoint.X() + (double)nXDist,
                        aPoint.Y() + (double)nYDist,
                        0.0);
                }

                // open/close
                aPoly.SetClosed(rLinePoly.IsClosed());

                // Teilpolygon einfuegen
                rPoly.Insert(aPoly);
            }
        }
    }
}

void E3dCompoundObject::ImpDrawShadowPolygon(PolyPolygon3D& rPoly, XOutputDevice& rXOut)
{
    Color aCol = GetShadowColor();
    OutputDevice *pDevice = rXOut.GetOutDev();
    //BOOL bDrawAsOutline(DrawShadowAsOutline());
    UINT16 nTransparence = GetShadowTransparence();

    if(nTransparence)
    {
        if(nTransparence != 100)
        {
            // transparence, draw to metafile and then transparent to
            // outdev
            UINT8 nScaledTrans((UINT8)((nTransparence * 255)/100));
            Color aTransColor(nScaledTrans, nScaledTrans, nScaledTrans);
            Gradient aGradient(GRADIENT_LINEAR, aTransColor, aTransColor);
            GDIMetaFile aMetaFile;
            VirtualDevice aVDev;
            MapMode aMap(rXOut.GetOutDev()->GetMapMode());

            // StepCount to someting small
            aGradient.SetSteps(3);

            // create BoundRectangle
            PolyPolygon aPolyPolygon(rPoly.GetPolyPolygon());
            Rectangle aBound(aPolyPolygon.GetBoundRect());

            // prepare VDev and MetaFile
            aVDev.EnableOutput(FALSE);
            aVDev.SetMapMode(rXOut.GetOutDev()->GetMapMode());
            aMetaFile.Record(&aVDev);

            aVDev.SetFont(rXOut.GetOutDev()->GetFont());
            aVDev.SetDrawMode(rXOut.GetOutDev()->GetDrawMode());
            aVDev.SetRefPoint(rXOut.GetOutDev()->GetRefPoint());

            // create output
            for(UINT16 a(0); a < aPolyPolygon.Count(); a++)
            {
                if(rPoly[a].IsClosed())
                {
                    aVDev.SetLineColor();
                    aVDev.SetFillColor(aCol);
                }
                else
                {
                    aVDev.SetLineColor(aCol);
                    aVDev.SetFillColor();
                }

                aMetaFile.AddAction(new MetaPolygonAction(aPolyPolygon[a]));
            }

            // draw metafile
            aMetaFile.Stop();
            aMetaFile.WindStart();
            aMap.SetOrigin(aBound.TopLeft());
            aMetaFile.SetPrefMapMode(aMap);
            aMetaFile.SetPrefSize(aBound.GetSize());
            rXOut.GetOutDev()->DrawTransparent(aMetaFile, aBound.TopLeft(), aBound.GetSize(), aGradient);
        }
    }
    else
    {
         // no transparence, draw all single polys directly
        for(UINT16 a(0); a < rPoly.Count(); a++)
        {
            if(rPoly[a].IsClosed())
            {
                pDevice->SetLineColor();
                pDevice->SetFillColor(aCol);
            }
            else
            {
                pDevice->SetLineColor(aCol);
                pDevice->SetFillColor();
            }

            pDevice->DrawPolygon(rPoly[a].GetPolygon());
        }
    }
}

/*************************************************************************
|*
|* convert given PolyPolygon3D to screen coor
|*
\************************************************************************/

XPolyPolygon E3dCompoundObject::TransformToScreenCoor(const PolyPolygon3D &rExtrudePoly)
{
    XPolyPolygon aNewPolyPolygon;
    B3dTransformationSet& rTransSet = GetScene()->GetCameraSet();

    // set ObjectTrans
    Matrix4D mTransform = GetFullTransform();
    rTransSet.SetObjectTrans(mTransform);

    // transform base polygon to screen coor
    for(UINT16 a=0;a<rExtrudePoly.Count();a++)
    {
        const Polygon3D &rExtPoly = rExtrudePoly[a];
        BOOL bClosed = rExtPoly.IsClosed();
        XPolygon aNewPoly(rExtPoly.GetPointCount() + (bClosed ? 1 : 0));

        UINT16 b;
        for(b=0;b<rExtPoly.GetPointCount();b++)
        {
            Vector3D aPoint = rTransSet.ObjectToViewCoor(rExtPoly[b]);
            aNewPoly[b].X() = (long)(aPoint.X() + 0.5);
            aNewPoly[b].Y() = (long)(aPoint.Y() + 0.5);
        }

        if(bClosed)
            aNewPoly[b] = aNewPoly[0];

        aNewPolyPolygon.Insert(aNewPoly);
    }

    return aNewPolyPolygon;
}

// #110988#
double E3dCompoundObject::GetMinimalDepthInViewCoor(E3dScene& rScene) const
{
    double fRetval(DBL_MAX);
    B3dTransformationSet& rTransSet = rScene.GetCameraSet();
    Matrix4D mTransform = ((E3dCompoundObject*)this)->GetFullTransform();
    rTransSet.SetObjectTrans(mTransform);
    B3dEntityBucket& rEntityBucket = ((E3dCompoundObject*)this)->GetDisplayGeometry().GetEntityBucket();
    GeometryIndexValueBucket& rIndexBucket = ((E3dCompoundObject*)this)->GetDisplayGeometry().GetIndexBucket();
    sal_uInt32 nPolyCounter(0L);
    sal_uInt32 nEntityCounter(0L);

    while(nPolyCounter < rIndexBucket.Count())
    {
        sal_uInt32 nUpperBound(rIndexBucket[nPolyCounter++].GetIndex());

        while(nEntityCounter < nUpperBound)
        {
            Vector3D aNewPoint = rEntityBucket[nEntityCounter++].Point().GetVector3D();
            aNewPoint = rTransSet.ObjectToViewCoor(aNewPoint);

            if(aNewPoint.Z() < fRetval)
            {
                fRetval = aNewPoint.Z();
            }
        }
    }

    return fRetval;
}

// #110988#
sal_Bool E3dCompoundObject::IsAOrdNumRemapCandidate(E3dScene*& prScene) const
{
    if(GetObjList()
        && GetObjList()->GetOwnerObj()
        && GetObjList()->GetOwnerObj()->ISA(E3dScene))
    {
        prScene = (E3dScene*)GetObjList()->GetOwnerObj();
        return sal_True;
    }

    return sal_False;
}

// eof
