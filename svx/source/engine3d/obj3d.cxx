/*************************************************************************
 *
 *  $RCSfile: obj3d.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2000-11-14 13:34:45 $
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

#define ITEMID_COLOR            SID_ATTR_3D_LIGHTCOLOR

#include "svdstr.hrc"
#include "svdglob.hxx"

#ifndef _SOT_DTRANS_HXX
#include <sot/dtrans.hxx>
#endif

#ifndef _SVDVIEW_HXX
#include "svdview.hxx"
#endif

#ifndef _SVDATTR_HXX
#include "svdattr.hxx"
#endif

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _SVDIO_HXX
#include "svdio.hxx"
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

#ifndef _E3D_POLYOB3D_HXX
#include "polyob3d.hxx"
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

#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif

#ifndef _SV_POLY_HXX
#include <vcl/poly.hxx>
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

#ifndef _EEITEM_HXX
#include "eeitem.hxx"
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
    E3dObject* pOwner = (E3dObject*)GetOwnerObj();

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
    E3dObject* pOwner = (E3dObject*)GetOwnerObj();

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

TYPEINIT1(E3dObject, SdrAttrObj);

E3dObject::E3dObject() :
    nLogicalGroup(0),
    nObjTreeLevel(0),
    eDragDetail(E3DDETAIL_ONEBOX),
    nPartOfParent(0),
    bTfHasChanged(TRUE),
    bBoundVolValid(TRUE),
    bIsSelected(FALSE)
//-/    bAttrUseSubObjects(TRUE)
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

void E3dObject::SetRectsDirty(FASTBOOL bNotMyself)
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
    rInfo.bCanConvToPath        = FALSE;

    // no transparence for 3d objects
    rInfo.bTransparenceAllowed = FALSE;

    // gradient depends on fillstyle
    // BM *** check if SetItem is NULL ***
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetItem(XATTR_FILLSTYLE))).GetValue();
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

SdrLayerID E3dObject::GetLayer() const
{
    FASTBOOL bFirst = TRUE;
    E3dObjList* pOL = pSub;
    ULONG       nObjCnt = pOL->GetObjCount();
    SdrLayerID  nLayer = SdrLayerID(nLayerID);

    for ( ULONG i = 0; i < nObjCnt; i++ )
    {
        SdrLayerID nObjLayer;
        if(pOL->GetObj(i)->ISA(E3dPolyObj))
            nObjLayer = SdrLayerID(nLayerID);
        else
            nObjLayer = pOL->GetObj(i)->GetLayer();

        if (bFirst)
        {
            nLayer = nObjLayer;
            bFirst = FALSE;
        }
        else if ( nObjLayer != nLayer )
            return 0;
    }
    return nLayer;
}

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
    // SdrAttrObj::NbcResize(rRef, xFact, yFact);

    // Bewegung in X,Y im Augkoordinatensystem
    E3dScene* pScene = GetScene();

    if(pScene)
    {
        // pos ermitteln
        B3dTransformationSet& rSet = pScene->GetCameraSet();
        Vector3D aScaleCenter((double)rRef.X(), (double)rRef.Y(), 32768.0);
        aScaleCenter = rSet.ViewToEyeCoor(aScaleCenter);

        // scale-faktoren holen
        double fScaleX = xFact;
        double fScaleY = yFact;

        // build transform
        Matrix4D mFullTransform(GetFullTransform());
        Matrix4D mTrans(mFullTransform);

        mTrans *= rSet.GetOrientation();
        mTrans.Translate(-aScaleCenter);
        mTrans.Scale(fScaleX, fScaleY, 1.0);
        mTrans.Translate(aScaleCenter);
        mTrans *= rSet.GetInvOrientation();
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
}

/*************************************************************************
|*
|* Objekt verschieben in 2D, wird bei Cursortasten benoetigt
|*
\************************************************************************/

void E3dObject::NbcMove(const Size& rSize)
{
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
        B3dTransformationSet& rSet = pScene->GetCameraSet();
        const Volume3D& rVol = pScene->GetBoundVolume();
        Volume3D aEyeVol = rVol.GetTransformVolume(rSet.GetOrientation());

        // relativen Bewegungsvektor in Augkoordinaten bilden
        Vector3D aMove(
            (double)rSize.Width() * aEyeVol.GetWidth() / (double)aRect.GetWidth(),
            (double)-rSize.Height() * aEyeVol.GetHeight() / (double)aRect.GetHeight(),
            0.0);

        // Bewegungsvektor in lokale Koordinaten des Parents des Objektes
        Vector3D aPos;
        aMove = rSet.EyeToWorldCoor(aMove);
        aMove *= mInvDispTransform;
        aPos = rSet.EyeToWorldCoor(aPos);
        aPos *= mInvDispTransform;
        aMove = aMove - aPos;

        // Transformieren
        Translate(aMove);

        // force new camera and SnapRect on scene, geometry may have really
        // changed
        pScene->CorrectSceneDimensions();
    }
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

    ((E3dObject*) this)->ImpCreateWireframePoly(aXPP, E3DDETAIL_ONEBOX);
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

void E3dObject::Paint3D(ExtOutputDevice& rOut, Base3D* pBase3D,
    const SdrPaintInfoRec& rInfoRec, UINT16 nDrawFlags)
{
    if(pSub && pSub->GetObjCount())
    {
        BOOL bWasNotActive = rInfoRec.bNotActive;
        BOOL bIsEnteredGroup(FALSE);

        if((rInfoRec.pPV && GetSubList() && rInfoRec.pPV->GetObjList() == GetSubList())
            || (rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE))
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
    ExtOutputDevice& rXOut,
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
            Rectangle aSubRect = ((E3dObject*)pObj)->GetBoundRect();
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
    p3DObj->SetObjTreeLevel(nObjTreeLevel + 1);
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
|* Linienattribute abfragen, ggf. vom Parent holen
|*
\************************************************************************/

//-/const XLineAttrSetItem* E3dObject::GetLineAttr() const
//-/{
//-/    const XLineAttrSetItem* pSetItem = pLineAttr;
//-/
//-/    if ( pSetItem == NULL && GetParentObj() )
//-/        pSetItem = GetParentObj()->GetLineAttr();
//-/
//-/    return pSetItem;
//-/}

/*************************************************************************
|*
|* Flaechenattribute abfragen, ggf. vom Parent holen
|*
\************************************************************************/

//-/const XFillAttrSetItem* E3dObject::GetFillAttr() const
//-/{
//-/    const XFillAttrSetItem* pSetItem = pFillAttr;
//-/
//-/    if ( pSetItem == NULL && GetParentObj() )
//-/        pSetItem = GetParentObj()->GetFillAttr();
//-/
//-/    return pSetItem;
//-/}

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
        aBoundVol = aLocalBoundVol;
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
    SendRepaintBroadcast();
    NbcSetTransform(rMatrix);
    SetChanged();
    SendRepaintBroadcast();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/*************************************************************************
|*
|* Transformationsmatrix zuruecksetzen mit Repaint-Broadcast
|*
\************************************************************************/

void E3dObject::ResetTransform()
{
    SendRepaintBroadcast();
    NbcResetTransform();
    SetChanged();
    SendRepaintBroadcast();
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
    SendRepaintBroadcast();
    NbcTranslate(rTrans);
    SetChanged();
    SendRepaintBroadcast();
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
    SendRepaintBroadcast();
    NbcScaleX(fSx);
    SetChanged();
    SendRepaintBroadcast();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/************************************************************************/

void E3dObject::ScaleY(double fSy)
{
    SendRepaintBroadcast();
    NbcScaleY(fSy);
    SetChanged();
    SendRepaintBroadcast();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/************************************************************************/

void E3dObject::ScaleZ(double fSz)
{
    SendRepaintBroadcast();
    NbcScaleZ(fSz);
    SetChanged();
    SendRepaintBroadcast();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/************************************************************************/

void E3dObject::Scale(double fSx, double fSy, double fSz)
{
    SendRepaintBroadcast();
    NbcScale(fSx, fSy, fSz);
    SetChanged();
    SendRepaintBroadcast();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/************************************************************************/

void E3dObject::Scale(double fS)
{
    SendRepaintBroadcast();
    NbcScale(fS);
    SetChanged();
    SendRepaintBroadcast();
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
    SendRepaintBroadcast();
    NbcRotateX(fAng);
    SetChanged();
    SendRepaintBroadcast();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/************************************************************************/

void E3dObject::RotateY(double fAng)
{
    SendRepaintBroadcast();
    NbcRotateY(fAng);
    SetChanged();
    SendRepaintBroadcast();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/************************************************************************/

void E3dObject::RotateZ(double fAng)
{
    SendRepaintBroadcast();
    NbcRotateZ(fAng);
    SetChanged();
    SendRepaintBroadcast();
    if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
}

/*************************************************************************
|*
|* uebergebene Transformationsmatrix auf die Objektkoordinaten (bei
|* abgeleiteten Objekten), d.h. nicht auf die lokale Matrix, und auf
|* alle Childs anwenden
|*
\************************************************************************/

void E3dObject::ApplyTransform(const Matrix4D& rMatrix)
{
    E3dObjList* pOL = pSub;
    ULONG nObjCnt = pOL->GetObjCount();

    for (ULONG i = 0; i < nObjCnt; i++)
    {
        SdrObject* pObj = pOL->GetObj(i);
        DBG_ASSERT(pObj->ISA(E3dObject), "In E3dObject sind nur 3D-Objekte erlaubt!");

        ((E3dObject*) pObj)->ApplyTransform(rMatrix);
    }
}

/*************************************************************************
|*
|* Objektbaum-Ebene des Objekts und aller Children setzen
|*
\************************************************************************/

void E3dObject::SetObjTreeLevel(USHORT nNewLevel)
{
    nObjTreeLevel = nNewLevel;
    nNewLevel++;

    E3dObjList* pOL = pSub;
    ULONG nObjCnt = pOL->GetObjCount();

    for (ULONG i = 0; i < nObjCnt; i++)
    {
        SdrObject* pObj = pOL->GetObj(i);
        DBG_ASSERT(pObj->ISA(E3dObject), "In E3dObject sind nur 3D-Objekte erlaubt!");

        ((E3dObject*) pObj)->SetObjTreeLevel(nNewLevel);
    }
}

/*************************************************************************
|*
|* logische Gruppe setzen
|*
\************************************************************************/

void E3dObject::SetLogicalGroup(USHORT nGroup)
{
    nLogicalGroup = nGroup;
}

/*************************************************************************
|*
|* Linien fuer die Wireframe-Darstellung des Objekts dem uebergebenen
|* Polygon3D hinzufuegen. Als default wird das BoundVolume verwendet.
|*
\************************************************************************/

void E3dObject::CreateWireframe(Polygon3D& rWirePoly, const Matrix4D* pTf,
    E3dDragDetail eDetail)
{
    if ( eDetail == E3DDETAIL_DEFAULT )
        eDetail = eDragDetail;

    if ( eDetail == E3DDETAIL_ALLBOXES || eDetail == E3DDETAIL_ALLLINES )
    {
        E3dObjList* pOL = pSub;
        ULONG nObjCnt = pOL->GetObjCount();

        for (ULONG i = 0; i < nObjCnt; i++)
        {
            E3dObject* pObj = (E3dObject*)pOL->GetObj(i);
            DBG_ASSERT(pObj->ISA(E3dObject), "In E3dObject sind nur 3D-Objekte erlaubt!");

            Matrix4D aLocalTf(pObj->GetTransform());
            if(pTf)
                aLocalTf *= *pTf;
            pObj->CreateWireframe(rWirePoly, &aLocalTf, eDetail);
        }

        if(eDetail == E3DDETAIL_ALLBOXES && nObjCnt != 1)
            GetBoundVolume().CreateWireframe(rWirePoly, pTf);
    }
    else
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

void E3dObject::ImpCreateWireframePoly(XPolyPolygon& rXPP,
    E3dDragDetail eDetail)
{
    // Neue Methode
    E3dScene* pScene = GetScene();
    Polygon3D aPoly3D(24, 240);
    XPolygon aLine(2);
    USHORT nPntCnt;

    // WireFrame herstellen
    CreateWireframe(aPoly3D, NULL, eDetail);
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

void E3dObject::TakeXorPoly(XPolyPolygon& rXPP, FASTBOOL bDetail) const
{
    rXPP.Clear();
    // Const mal wieder weg, da evtl. das BoundVolume neu generiert wird
    static E3dDragDetail eDetail = E3DDETAIL_DEFAULT;
    ((E3dObject*) this)->ImpCreateWireframePoly(rXPP, eDetail);
}

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

void E3dObject::operator=(const SdrObject& rObj)
{
    // erstmal alle Childs kopieren
    SdrAttrObj::operator=(rObj);

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
    nLogicalGroup   = r3DObj.nLogicalGroup;
    nObjTreeLevel   = r3DObj.nObjTreeLevel;
    nPartOfParent   = r3DObj.nPartOfParent;
    eDragDetail     = r3DObj.eDragDetail;

    // Da sich der Parent geaendert haben kann, Gesamttransformation beim
    // naechsten Mal auf jeden Fall neu bestimmen
    SetTransformChanged();

    // Selektionsstatus kopieren
    bIsSelected = r3DObj.bIsSelected;
}

/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

//-/void E3dObject::Distribute3DAttributes(const SfxItemSet& rAttr)
//-/{
//-/}

//-/void E3dObject::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/    // call parent
//-/    if(!ISA(E3dScene))
//-/        SdrAttrObj::NbcSetAttributes(rAttr, bReplaceAll);
//-/
//-/    if(bAttrUseSubObjects)
//-/    {
//-/        E3dObjList* pOL = pSub;
//-/        ULONG nObjCnt = pOL->GetObjCount();
//-/        for ( ULONG i = 0; i < nObjCnt; i++ )
//-/            pOL->GetObj(i)->NbcSetAttributes(rAttr, bReplaceAll);
//-/    }
//-/    else
//-/    {
//-/        bAttrUseSubObjects = TRUE;
//-/    }
//-/    StructureChanged(this);
//-/}

/*************************************************************************
|*
|* Attribute abfragen
|*
\************************************************************************/

//-/void E3dObject::TakeAttributes(SfxItemSet& rAttr, FASTBOOL bMerge,
//-/    FASTBOOL bOnlyHardAttr) const
//-/{
//-/    // call parent (only with real 3d objects, not with groups)
//-/    if(!ISA(E3dScene))
//-/        SdrAttrObj::TakeAttributes(rAttr, bMerge, bOnlyHardAttr);
//-/
//-/    if(bAttrUseSubObjects)
//-/    {
//-/        E3dObjList* pOL = pSub;
//-/        ULONG nObjCnt = pOL->GetObjCount();
//-/        for ( ULONG i = 0; i < nObjCnt; i++ )
//-/            pOL->GetObj(i)->TakeAttributes(rAttr, TRUE, bOnlyHardAttr);
//-/    }
//-/    else
//-/    {
//-/        ((E3dObject*)this)->bAttrUseSubObjects = TRUE;
//-/    }
//-/}

//-/void E3dObject::Collect3DAttributes(SfxItemSet& rAttr) const
//-/{
//-/}

// ItemSet access
//-/SfxItemSet* E3dObject::CreateNewItemSet(SfxItemPool& rPool)
//-/{
//-/    return new SfxItemSet(rPool,
//-/        SDRATTR_START,  SDRATTR_END,
//-/        SID_ATTR_3D_START, SID_ATTR_3D_END,
//-/        0, 0);
//-/}

const SfxItemSet& E3dObject::GetItemSet() const
{
    E3dScene* pScene = GetScene();
    if(pScene && pScene != this)
    {
        SfxItemSet& rSet = (SfxItemSet&)SdrAttrObj::GetItemSet();
        SfxItemSet aSet(*rSet.GetPool(), SDRATTR_3DSCENE_FIRST, SDRATTR_3DSCENE_LAST);
        aSet.Put(pScene->E3dObject::GetItemSet());
//-/        rSet.MergeValues(aSet, TRUE);
        SfxWhichIter aIter(aSet);
        sal_uInt16 nWhich(aIter.FirstWhich());
        while(nWhich)
        {
            const SfxPoolItem* pItem = NULL;
            aSet.GetItemState(nWhich, TRUE, &pItem);

            if(pItem)
            {
                if(pItem == (SfxPoolItem *)-1)
                    rSet.InvalidateItem(nWhich);
                else
                    rSet.MergeValue(*pItem, TRUE);
            }
            nWhich = aIter.NextWhich();
        }
    }

    return SdrAttrObj::GetItemSet();
}
//-/const SfxItemSet& E3dObject::GetItemSet() const
//-/{
//-/    // get base items from SdrAttrObj
//-/    SfxItemSet& rSet = (SfxItemSet&)SdrAttrObj::GetItemSet();
//-/
//-/    // add local 3D items of this object
//-/    Collect3DAttributes(rSet);
//-/
//-/    // add outmost scene (if available)
//-/    E3dScene* pScene = GetScene();
//-/    if(pScene)
//-/        pScene->Collect3DAttributes(rSet);
//-/
//-/    // return completed ItemSet
//-/    return rSet;
//-/}

void E3dObject::SetItem( const SfxPoolItem& rItem )
{
    if(rItem.Which() >= SDRATTR_3DSCENE_FIRST && rItem.Which() <= SDRATTR_3DSCENE_LAST)
    {
        E3dScene* pScene = GetScene();
        if(pScene && pScene != this)
        {
            pScene->E3dObject::SetItem(rItem);
        }
    }

    SdrAttrObj::SetItem(rItem);
    StructureChanged(this);
}
//-/void E3dObject::SetItem( const SfxPoolItem& rItem )
//-/{
//-/    // set base items
//-/    SdrAttrObj::SetItem(rItem);
//-/
//-/    // set local 3D attributes
//-/    const SfxItemSet& rSet = SdrAttrObj::GetItemSet();
//-/    Distribute3DAttributes(rSet);
//-/
//-/    // set at outmost scene (if available)
//-/    E3dScene* pScene = GetScene();
//-/    if(pScene)
//-/        pScene->Distribute3DAttributes(rSet);
//-/}

void E3dObject::ClearItem( USHORT nWhich )
{
    if(nWhich >= SDRATTR_3DSCENE_FIRST && nWhich <= SDRATTR_3DSCENE_LAST)
    {
        E3dScene* pScene = GetScene();
        if(pScene && pScene != this)
        {
            pScene->E3dObject::ClearItem(nWhich);
        }
    }

    if(mpObjectItemSet)
    {
        SdrAttrObj::ClearItem(nWhich);
        StructureChanged(this);
    }
}
//-/void E3dObject::ClearItem( USHORT nWhich )
//-/{
//-/    if(mpObjectItemSet)
//-/    {
//-/        // clear base item
//-/        SdrAttrObj::ClearItem(nWhich);
//-/
//-/        // clear local 3D attributes
//-/        const SfxItemSet& rSet = SdrAttrObj::GetItemSet();
//-/        Distribute3DAttributes(rSet);
//-/
//-/        // clear at outmost scene (if available)
//-/        E3dScene* pScene = GetScene();
//-/        if(pScene)
//-/            pScene->Distribute3DAttributes(rSet);
//-/    }
//-/}

void E3dObject::SetItemSet( const SfxItemSet& rSet )
{
    E3dScene* pScene = GetScene();
    if(pScene && pScene != this)
    {
        pScene->E3dObject::SetItemSet(rSet);
    }

    // set base items
    SdrAttrObj::SetItemSet(rSet);
    StructureChanged(this);
}
//-/void E3dObject::SetItemSet( const SfxItemSet& rSet )
//-/{
//-/    // set base items
//-/    SdrAttrObj::SetItemSet(rSet);
//-/
//-/    // set local 3D attributes
//-/    Distribute3DAttributes(rSet);
//-/
//-/    // set at outmost scene (if available)
//-/    E3dScene* pScene = GetScene();
//-/    if(pScene)
//-/        pScene->Distribute3DAttributes(rSet);
//-/}

SfxItemSet* E3dObject::CreateNewItemSet(SfxItemPool& rPool)
{
    // include ALL items, 2D and 3D
    return new SfxItemSet(rPool,
        // ranges from SdrAttrObj
        SDRATTR_START, SDRATTRSET_SHADOW,
        SDRATTRSET_OUTLINER, SDRATTRSET_MISC,

        // ranges for 3D (object and scene)
        SDRATTR_3D_FIRST, SDRATTR_3D_LAST,

        // outliner and end
        EE_ITEMS_START, EE_ITEMS_END,
        0, 0);
}

/*************************************************************************
|*
|* StyleSheet setzen
|*
\************************************************************************/

void E3dObject::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet,
    FASTBOOL bDontRemoveHardAttr)
{
    // call parent
    SdrAttrObj::NbcSetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

    E3dObjList* pOL = pSub;
    ULONG nObjCnt = pOL->GetObjCount();
    for ( ULONG i = 0; i < nObjCnt; i++ )
        pOL->GetObj(i)->NbcSetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
}

/*************************************************************************
|*
|* Nur die Member des E3dObjekts in den Stream speichern
|* Dies wird direkt auch von E3dSphere gerufen um zu verhindern dass die
|* Subliste weggeschrieben wird. (FG)
|*
\************************************************************************/

void E3dObject::WriteOnlyOwnMembers(SvStream& rOut) const
{
    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rOut, STREAM_WRITE);
#ifdef DBG_UTIL
    aCompat.SetID("E3dObjectOwnMembers");
#endif

    rOut << aLocalBoundVol;

    Old_Matrix3D aMat3D;
    aMat3D = aTfMatrix;
    rOut << aMat3D;

    rOut << nLogicalGroup;
    rOut << nObjTreeLevel;
    rOut << nPartOfParent;
    rOut << UINT16(eDragDetail);
}

/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

void E3dObject::WriteData(SvStream& rOut) const
{
    long position = rOut.Tell();
    SdrAttrObj::WriteData(rOut);
    position = rOut.Tell();

    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rOut, STREAM_WRITE);
#ifdef DBG_UTIL
    aCompat.SetID("E3dObject");
#endif

    position = rOut.Tell();
    pSub->Save(rOut);
    position = rOut.Tell();

    if (rOut.GetVersion() < 3560)
    {
        rOut << aLocalBoundVol;

        Old_Matrix3D aMat3D;
        aMat3D = aTfMatrix;
        rOut << aMat3D;

        rOut << nLogicalGroup;
        rOut << nObjTreeLevel;
        rOut << nPartOfParent;
        rOut << UINT16(eDragDetail);
    }
    else
    {
        WriteOnlyOwnMembers(rOut);
    }
    position = rOut.Tell();
}

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

void E3dObject::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    long position = rIn.Tell();
    if (ImpCheckSubRecords (rHead, rIn))
    {
        position = rIn.Tell();
        SdrAttrObj::ReadData(rHead, rIn);
        position = rIn.Tell();
        // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
        SdrDownCompat aCompat(rIn, STREAM_READ);
#ifdef DBG_UTIL
        aCompat.SetID("E3dObject");
#endif
        pSub->Load(rIn, *pPage);

        position = rIn.Tell();
        if ((rIn.GetVersion() < 3560) || (rHead.GetVersion() <= 12))
        {
            UINT16  nTmp16;

            rIn >> aLocalBoundVol;

            Old_Matrix3D aMat3D;
            rIn >> aMat3D;
            aTfMatrix = Matrix4D(aMat3D);

            rIn >> nLogicalGroup;
            rIn >> nObjTreeLevel;
            rIn >> nPartOfParent;
            rIn >> nTmp16; eDragDetail = E3dDragDetail(nTmp16);
        }
        else
        {
            ReadOnlyOwnMembers(rHead, rIn);
        }
        position = rIn.Tell();

        // Wie ein veraendertes Objekt behandeln
        SetTransformChanged();
        StructureChanged(this);

        // BoundVolume muss neu berechnet werden
        bBoundVolValid = FALSE;

        // SnapRect auch
        bSnapRectDirty = TRUE;
    }
}

/*************************************************************************
|*
|* Nur die Daten des E3dObject aus Stream laden (nicht der Sublisten und
|* der Basisklassen). Wird von E3dSphere auch genutzt. (FileFormat-Optimierung)
|*
\************************************************************************/

void E3dObject::ReadOnlyOwnMembers(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rIn, STREAM_READ);
#ifdef DBG_UTIL
    aCompat.SetID("E3dObjectOwnMembers");
#endif
    UINT16  nTmp16;

    rIn >> aLocalBoundVol;

    Old_Matrix3D aMat3D;
    rIn >> aMat3D;
    aTfMatrix = Matrix4D(aMat3D);

    rIn >> nLogicalGroup;
    rIn >> nObjTreeLevel;
    rIn >> nPartOfParent;
    rIn >> nTmp16; eDragDetail = E3dDragDetail(nTmp16);

    bBoundVolValid = FALSE;
}


/*************************************************************************
|*
|* nach dem Laden...
|*
\************************************************************************/

void E3dObject::AfterRead()
{
    SdrAttrObj::AfterRead();
    if (pSub)
        pSub->AfterRead();

    // put loaded items to ItemSet
//-/    Collect3DAttributes((SfxItemSet&)GetItemSet());
}

//-/void E3dObject::Collect3DAttributes(SfxItemSet& rAttr) const
//-/{
//-/}

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

BOOL E3dObject::ImpCheckSubRecords (const SdrObjIOHeader& rHead,
                                    SvStream&             rIn)
{
    BOOL bDoRead = FALSE;

    if ( rIn.GetError() == SVSTREAM_OK )
    {
        if (rHead.GetVersion () <= 12)
        {
            ULONG nPos0 = rIn.Tell();
            // Einen SubRecord ueberspringen (SdrObject)
            { SdrDownCompat aCompat(rIn,STREAM_READ); }
            // Nocheinen SubRecord ueberspringen (SdrAttrObj)
            { SdrDownCompat aCompat(rIn,STREAM_READ); }
            // Und nun muesste meiner kommen
            bDoRead = rHead.GetBytesLeft() != 0;
            rIn.Seek (nPos0); // FilePos wieder restaurieren
        }
        else
        {
            bDoRead = TRUE;
        }
    }

    return bDoRead;
}

/*************************************************************************
|*
|* Keine DefaultAttr, zu langsam
|*
\************************************************************************/

void E3dObject::ForceDefaultAttr()
{
}

/*************************************************************************
|*
|* Falls doch noch DefaultAttrs benoetigt werden
|*
\************************************************************************/

void E3dObject::ForceDefaultAttrAgain()
{
    SdrAttrObj::ForceDefaultAttr();
}

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

    bBytesLeft = FALSE;
    bCreateE3dPolyObj = FALSE;
    bGeometryValid = FALSE;
}

E3dCompoundObject::E3dCompoundObject(E3dDefaultAttributes& rDefault) : E3dObject()
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    bBytesLeft = FALSE;
    bCreateE3dPolyObj = FALSE;
    bGeometryValid = FALSE;
}

void E3dCompoundObject::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    // Defaults setzen
//-/    SetFrontMaterial(rDefault.GetDefaultFrontMaterial());
    aMaterialAmbientColor = rDefault.GetDefaultAmbientColor();

    aBackMaterial = rDefault.GetDefaultBackMaterial();
//-/    eTextureKind = rDefault.GetDefaultTextureKind();
//-/    eTextureMode = rDefault.GetDefaultTextureMode();
//-/    bDoubleSided = rDefault.GetDefaultDoubleSided();
    bCreateNormals = rDefault.GetDefaultCreateNormals();
    bCreateTexture = rDefault.GetDefaultCreateTexture();
//-/    bUseStdNormals = rDefault.GetDefaultUseStdNormals();
//-/    bUseStdNormalsUseSphere = rDefault.GetDefaultUseStdNormalsUseSphere();
//-/    bInvertNormals = rDefault.GetDefaultInvertNormals();
//-/    bUseStdTextureX = rDefault.GetDefaultUseStdTextureX();
//-/    bUseStdTextureXUseSphere = rDefault.GetDefaultUseStdTextureXUseSphere();
//-/    bUseStdTextureY = rDefault.GetDefaultUseStdTextureY();
//-/    bUseStdTextureYUseSphere = rDefault.GetDefaultUseStdTextureYUseSphere();
//-/    bShadow3D = rDefault.GetDefaultShadow3D();
//-/    bFilterTexture = rDefault.GetDefaultFilterTexture();
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
|* Compounds brauchen Defaults
|*
\************************************************************************/

void E3dCompoundObject::ForceDefaultAttr()
{
    SdrAttrObj::ForceDefaultAttr();
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
            PolyPolygon aShadowPoly;
            GetShadowPolygon(aShadowPoly);

            // Hinzufuegen
            aOutRect.Union(aShadowPoly.GetBoundRect());
        }

        // Linienbreite beruecksichtigen
//-/        const XLineAttrSetItem* pLineAttr = GetLineAttr();
        INT32 nLineWidth = ((const XLineWidthItem&)(GetItem(XATTR_LINEWIDTH))).GetValue();
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

void E3dCompoundObject::WriteData(SvStream& rOut) const
{
#ifdef E3D_STREAMING

    if (!aLocalBoundVol.IsValid() && aBoundVol.IsValid())
    {
        // Das aLocalBoundVol wird gespeichert.
        // Ist dieses ungueltig, so wird das aBoundVol genommen
        // (sollten beim E3dCompoundObject sowieso gleich sein)
        ((E3dCompoundObject*) this)->aLocalBoundVol = aBoundVol;
    }

    E3dObject::WriteData(rOut);
    if (rOut.GetVersion() < 3560)
    {
        // In diesem Fall passiert nichts, da vor der Version 4.0
        // also im Falle der Revision 3.1
    }
    else
    {
        SdrDownCompat aCompat(rOut, STREAM_WRITE);
#ifdef DBG_UTIL
        aCompat.SetID("E3dCompoundObject");
#endif
//-/        rOut << BOOL(bDoubleSided);
        rOut << BOOL(GetDoubleSided());
#endif

        // neue Parameter zur Geometrieerzeugung
        rOut << BOOL(bCreateNormals);
        rOut << BOOL(bCreateTexture);

//-/        rOut << BOOL(bUseStdNormals);
//-/        rOut << BOOL(bUseStdNormalsUseSphere);
        sal_uInt16 nVal = GetNormalsKind();
        rOut << BOOL(nVal > 0);
        rOut << BOOL(nVal > 1);

//-/        rOut << BOOL(bUseStdTextureX);
//-/        rOut << BOOL(bUseStdTextureXUseSphere);
        nVal = GetTextureProjectionX();
        rOut << BOOL(nVal > 0);
        rOut << BOOL(nVal > 1);

//-/        rOut << BOOL(bUseStdTextureY);
//-/        rOut << BOOL(bUseStdTextureYUseSphere);
        nVal = GetTextureProjectionY();
        rOut << BOOL(nVal > 0);
        rOut << BOOL(nVal > 1);

//-/        rOut << BOOL(bShadow3D);
        rOut << BOOL(GetShadow3D());

        // neu al 384:
//-/        aFrontMaterial.WriteData(rOut);
        rOut << GetMaterialAmbientColor();
        rOut << GetMaterialColor();
        rOut << GetMaterialSpecular();
        rOut << GetMaterialEmission();
        rOut << GetMaterialSpecularIntensity();

        aBackMaterial.WriteData(rOut);

//-/        rOut << (UINT16)eTextureKind;
        rOut << (UINT16)GetTextureKind();

//-/        rOut << (UINT16)eTextureMode;
        rOut << (UINT16)GetTextureMode();

//-/        rOut << BOOL(bInvertNormals);
        rOut << BOOL(GetNormalsInvert());

        // neu ab 534: (hat noch gefehlt)
//-/        rOut << BOOL(bFilterTexture);
        rOut << BOOL(GetTextureFilter());
    }
}

/*************************************************************************
|*
|* Einlesen der Datenmember eines E3dCompounds
|*
\************************************************************************/

void E3dCompoundObject::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if ( rIn.GetError() != SVSTREAM_OK )
        return;

    E3dObject::ReadData(rHead, rIn);

      // Vor der Filerevision 13 wurde das Objekt nie geschrieben.
      // auch kein Kompatibilitaetsrecord.
    if ((rHead.GetVersion() < 13) || (rIn.GetVersion() < 3560))
    {
        return;
    }

    SdrDownCompat aCompat(rIn, STREAM_READ);
#ifdef DBG_UTIL
    aCompat.SetID("E3dCompoundObject");
#endif

    bBytesLeft = FALSE;
    if (aCompat.GetBytesLeft () >= sizeof (BOOL))
    {
        BOOL bTmp, bTmp2;
        sal_uInt16 nTmp;

        rIn >> bTmp;
        mpObjectItemSet->Put(Svx3DDoubleSidedItem(bTmp));
//-/        bDoubleSided = bTmp;

        // neue Parameter zur Geometrieerzeugung
        if (aCompat.GetBytesLeft () >= sizeof (BOOL))
        {
            rIn >> bTmp;
            bCreateNormals = bTmp;

            rIn >> bTmp;
            bCreateTexture = bTmp;

//-/            rIn >> bTmp;
//-/            bUseStdNormals = bTmp;
//-/            rIn >> bTmp;
//-/            bUseStdNormalsUseSphere = bTmp;
            rIn >> bTmp;
            rIn >> bTmp2;
            if(bTmp == FALSE && bTmp2 == FALSE)
                nTmp = 0;
            else if(bTmp == TRUE && bTmp2 == FALSE)
                nTmp = 1;
            else
                nTmp = 2;
            mpObjectItemSet->Put(Svx3DNormalsKindItem(nTmp));

//-/            rIn >> bTmp;
//-/            bUseStdTextureX = bTmp;
//-/            rIn >> bTmp;
//-/            bUseStdTextureXUseSphere = bTmp;
            rIn >> bTmp;
            rIn >> bTmp2;
            if(bTmp == FALSE && bTmp2 == FALSE)
                nTmp = 0;
            else if(bTmp == TRUE && bTmp2 == FALSE)
                nTmp = 1;
            else
                nTmp = 2;
            mpObjectItemSet->Put(Svx3DTextureProjectionXItem(nTmp));

//-/            rIn >> bTmp;
//-/            bUseStdTextureY = bTmp;
//-/            rIn >> bTmp;
//-/            bUseStdTextureYUseSphere = bTmp;
            rIn >> bTmp;
            rIn >> bTmp2;
            if(bTmp == FALSE && bTmp2 == FALSE)
                nTmp = 0;
            else if(bTmp == TRUE && bTmp2 == FALSE)
                nTmp = 1;
            else
                nTmp = 2;
            mpObjectItemSet->Put(Svx3DTextureProjectionYItem(nTmp));

            rIn >> bTmp;
//-/            bShadow3D = bTmp;
            mpObjectItemSet->Put(Svx3DShadow3DItem(bTmp));

            // Setze ein Flag fuer den Aufrufer, dass neues Format
            // zu lesen ist
            bBytesLeft = TRUE;
        }

        // neu al 384:
        if (aCompat.GetBytesLeft () >= sizeof (B3dMaterial))
        {
            UINT16 nTmp;

//-/            aFrontMaterial.ReadData(rIn);
            Color aCol;

            rIn >> aCol;
            SetMaterialAmbientColor(aCol);

            rIn >> aCol;
//-/            SetItem(Svx3DMaterialColorItem(aCol));
            // do NOT use, this is the old 3D-Color(!)
            // SetItem(XFillColorItem(String(), aCol));

            rIn >> aCol;
            mpObjectItemSet->Put(Svx3DMaterialSpecularItem(aCol));

            rIn >> aCol;
            mpObjectItemSet->Put(Svx3DMaterialEmissionItem(aCol));

            rIn >> nTmp;
            mpObjectItemSet->Put(Svx3DMaterialSpecularIntensityItem(nTmp));

            aBackMaterial.ReadData(rIn);

            rIn >> nTmp;
//-/            eTextureKind = (Base3DTextureKind)nTmp;
            mpObjectItemSet->Put(Svx3DTextureKindItem(nTmp));

            rIn >> nTmp;
//-/            eTextureMode = (Base3DTextureMode)nTmp;
            mpObjectItemSet->Put(Svx3DTextureModeItem(nTmp));

            rIn >> bTmp;
//-/            bInvertNormals = bTmp;
            mpObjectItemSet->Put(Svx3DNormalsInvertItem(bTmp));
        }

        // neu ab 534: (hat noch gefehlt)
        if (aCompat.GetBytesLeft () >= sizeof (BOOL))
        {
            rIn >> bTmp;
//-/            bFilterTexture = bTmp;
            mpObjectItemSet->Put(Svx3DTextureFilterItem(bTmp));
        }
    }
}

/*************************************************************************
|*
|* Bitmaps fuer 3D-Darstellung von Gradients und Hatches holen
|*
\************************************************************************/

Bitmap E3dCompoundObject::GetGradientBitmap(const SfxItemSet& rSet)
{
    VirtualDevice *pVD = new VirtualDevice();
    Size aVDSize(256, 256);
    pVD->SetOutputSizePixel( aVDSize );
    XOutputDevice *pXOut = new XOutputDevice( pVD );
//-/    XFillAttrSetItem *pXFSet = new XFillAttrSetItem( rSet.GetPool() );
    SfxItemSet aFillSet(*rSet.GetPool());

    aFillSet.Put( XFillStyleItem( XFILL_GRADIENT ) );
    aFillSet.Put( rSet.Get(XATTR_FILLGRADIENT) );
    aFillSet.Put( rSet.Get(XATTR_GRADIENTSTEPCOUNT) );
    pXOut->SetFillAttr( aFillSet );

//-/    XLineAttrSetItem *pXLSet = new XLineAttrSetItem( rSet.GetPool() );
    aFillSet.Put( XLineStyleItem( XLINE_NONE ) );
    pXOut->SetLineAttr( aFillSet );

    pXOut->DrawRect( Rectangle( Point(), aVDSize ) );
    Bitmap aGradientBitmap = pVD->GetBitmap( Point(), aVDSize );

    if( pVD )
        delete pVD;
    if( pXOut )
        delete pXOut;
//-/    if( pXFSet )
//-/        delete pXFSet;
//-/    if( pXLSet )
//-/        delete pXLSet;

    return aGradientBitmap;
}

Bitmap E3dCompoundObject::GetHatchBitmap(const SfxItemSet& rSet)
{
    VirtualDevice *pVD = new VirtualDevice();
    const XFillHatchItem* pFillHatchItem = (XFillHatchItem*)&rSet.Get(XATTR_FILLHATCH);
    const XHatch& rHatch = pFillHatchItem->GetValue();
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
//-/    XFillAttrSetItem *pXFSet = new XFillAttrSetItem( rSet.GetPool() );
//-/    XLineAttrSetItem *pXLSet = new XLineAttrSetItem( rSet.GetPool() );
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
//-/    if( pXFSet )
//-/        delete pXFSet;
//-/    if( pXLSet )
//-/        delete pXLSet;

    return aHatchBitmap;
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
//-/        if(bUseStdNormals && bUseStdNormalsUseSphere)
        if(GetNormalsKind() > 1)
            GetDisplayGeometry().CreateDefaultNormalsSphere();
        if(GetNormalsInvert())
            GetDisplayGeometry().InvertNormals();
    }

    if(bCreateTexture)
    {
//-/        GetDisplayGeometry().CreateDefaultTexture(
//-/            ((bUseStdTextureX) ? B3D_CREATE_DEFAULT_X : FALSE)
//-/            |((bUseStdTextureY) ? B3D_CREATE_DEFAULT_Y : FALSE),
//-/            bUseStdTextureXUseSphere);
        GetDisplayGeometry().CreateDefaultTexture(
            ((GetTextureProjectionX() > 0) ? B3D_CREATE_DEFAULT_X : FALSE)
            |((GetTextureProjectionY() > 0) ? B3D_CREATE_DEFAULT_Y : FALSE),
            GetTextureProjectionX() > 1);
    }

    // Am Ende der Geometrieerzeugung das model an den erzeugten
    // PolyObj's setzen, d.h. beim ueberladen dieser Funktion
    // den parent am Ende rufen.
    if(bCreateE3dPolyObj)
        SetModel(pModel);

    // Das Ende der Geometrieerzeugung anzeigen
    aDisplayGeometry.EndDescription();
}

void E3dCompoundObject::ReCreateGeometry(BOOL bCreateOldGeometry)
{
    // Geometrie zerstoeren
    DestroyGeometry();

    // Flag fuer Geometrieerzeugung setzen
    bCreateE3dPolyObj = bCreateOldGeometry;

    // ... und neu erzeugen
    CreateGeometry();
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
        if(bCreateE3dPolyObj)
        {
            E3dPolyObj* pObj = new E3dPolyObj(
                rPolyPolygon3D, GetDoubleSided(), TRUE);
            pObj->SetPartOfParent();
            Insert3DObj(pObj);
        }

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
        if(bCreateE3dPolyObj)
        {
            E3dPolyObj* pObj = new E3dPolyObj(
                rPolyPolygon3D, rPolyNormal3D, GetDoubleSided(), TRUE);
            pObj->SetPartOfParent();
            Insert3DObj(pObj);
        }

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
        if(bCreateE3dPolyObj)
        {
            E3dPolyObj* pObj = new E3dPolyObj(
                rPolyPolygon3D, rPolyNormal3D,
                rPolyTexture3D, GetDoubleSided(), TRUE);
            pObj->SetPartOfParent();
            Insert3DObj(pObj);
        }

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
    BOOL bCreateNormals,
    BOOL bCreateTexture)
{
    // Vorderseite
    if(bCreateNormals)
    {
        if(bCreateTexture)
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
    BOOL bCreateNormals,
    BOOL bCreateTexture)
{
    // PolyPolygon umdrehen
    PolyPolygon3D aLocalPoly = rPolyPoly3D;
    aLocalPoly.FlipDirections();

    // Rueckseite
    if(bCreateNormals)
    {
        PolyPolygon3D aLocalNormals = rBackNormals;
        aLocalNormals.FlipDirections();
        if(bCreateTexture)
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
    BOOL bCreateNormals,
    double fSurroundFactor,
    double fTextureStart,
    double fTextureDepth,
    BOOL bRotateTexture90)
{
    USHORT nPolyCnt = rPolyPolyFront.Count();
    BOOL bCreateTexture = (fTextureDepth == 0.0) ? FALSE : TRUE;
    double fPolyLength, fPolyPos;
    USHORT nLastIndex;

    // Verbindungsstuecke
    if(bCreateNormals)
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

            if(bCreateTexture)
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

                if(bCreateTexture)
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
//-/    bDoubleSided = r3DObj.bDoubleSided;
    bCreateNormals = r3DObj.bCreateNormals;
    bCreateTexture = r3DObj.bCreateTexture;
//-/    bUseStdNormals = r3DObj.bUseStdNormals;
//-/    bUseStdNormalsUseSphere = r3DObj.bUseStdNormalsUseSphere;
//-/    bUseStdTextureX = r3DObj.bUseStdTextureX;
//-/    bUseStdTextureXUseSphere = r3DObj.bUseStdTextureXUseSphere;
//-/    bUseStdTextureY = r3DObj.bUseStdTextureY;
//-/    bUseStdTextureYUseSphere = r3DObj.bUseStdTextureYUseSphere;
    bGeometryValid = r3DObj.bGeometryValid;
//-/    bShadow3D = r3DObj.bShadow3D;
    bBytesLeft = r3DObj.bBytesLeft;
    bCreateE3dPolyObj = r3DObj.bCreateE3dPolyObj;

    // neu ab 383:
//-/    aFrontMaterial = r3DObj.aFrontMaterial;
    aMaterialAmbientColor = r3DObj.aMaterialAmbientColor;

    aBackMaterial = r3DObj.aBackMaterial;
//-/    eTextureKind = r3DObj.eTextureKind;
//-/    eTextureMode = r3DObj.eTextureMode;
//-/    bInvertNormals = r3DObj.bInvertNormals;
//-/    bFilterTexture = r3DObj.bFilterTexture;
    bUseDifferentBackMaterial = r3DObj.bUseDifferentBackMaterial;
}

/*************************************************************************
|*
|* Ausgabeparameter an 3D-Kontext setzen
|*
\************************************************************************/

void E3dCompoundObject::SetBase3DParams(ExtOutputDevice& rOut, Base3D* pBase3D,
    BOOL& bDrawObject, BOOL& bDrawOutline, UINT16 nDrawFlags, BOOL bGhosted,
    BOOL bIsLineDraft, BOOL bIsFillDraft)
{
    bDrawObject = (nDrawFlags & E3D_DRAWFLAG_FILLED);
    if(bDrawObject)
    {
        // Attribute aus dem Objekt holen
//-/        const XFillAttrSetItem* pFillAttr = GetFillAttr();
        if(!bIsFillDraft)
        {
            const SfxItemSet& rSet = GetItemSet();
            UINT16 nFillTrans = ITEMVALUE(rSet, XATTR_FILLTRANSPARENCE, XFillTransparenceItem);

            if(
                (nFillTrans && (nDrawFlags & E3D_DRAWFLAG_TRANSPARENT))
                || (!nFillTrans && !(nDrawFlags & E3D_DRAWFLAG_TRANSPARENT)))
            {
                XFillStyle eFillStyle = ITEMVALUE(rSet, XATTR_FILLSTYLE, XFillStyleItem );
                Color aColorSolid = ((const XFillColorItem&) (rSet.
                    Get(XATTR_FILLCOLOR))).GetValue();

                if(bGhosted)
                {
                    aColorSolid = Color((aColorSolid.GetRed() >> 1) + 0x80,
                                        (aColorSolid.GetGreen() >> 1) + 0x80,
                                        (aColorSolid.GetBlue() >> 1) + 0x80);
                }

                Color aColorSolidWithTransparency = aColorSolid;
                aColorSolidWithTransparency.SetTransparency((UINT8)(nFillTrans * 255 / 100));
                Color aColorWhite(COL_WHITE);
                Color aColorWhiteWithTransparency(COL_WHITE);
                aColorWhiteWithTransparency.SetTransparency((UINT8)(nFillTrans * 255 / 100));
                B3dTexture* pTexture = NULL;

                // Material setzen
                pBase3D->SetMaterial(aColorWhite, Base3DMaterialAmbient);
                pBase3D->SetMaterial(aColorWhiteWithTransparency, Base3DMaterialDiffuse);

//-/                pBase3D->SetMaterial(aFrontMaterial.GetMaterial(Base3DMaterialSpecular), Base3DMaterialSpecular);
                pBase3D->SetMaterial(GetMaterialSpecular(), Base3DMaterialSpecular);

//-/                pBase3D->SetMaterial(aFrontMaterial.GetMaterial(Base3DMaterialEmission), Base3DMaterialEmission);
                pBase3D->SetMaterial(GetMaterialEmission(), Base3DMaterialEmission);

//-/                pBase3D->SetShininess(aFrontMaterial.GetShininess());
                pBase3D->SetShininess(GetMaterialSpecularIntensity());

                if(GetUseDifferentBackMaterial())
                {
                    pBase3D->SetMaterial(aColorWhite, Base3DMaterialAmbient, Base3DMaterialBack);
                    pBase3D->SetMaterial(aColorWhiteWithTransparency, Base3DMaterialDiffuse, Base3DMaterialBack);
                    pBase3D->SetMaterial(aBackMaterial.GetMaterial(Base3DMaterialSpecular), Base3DMaterialSpecular, Base3DMaterialBack);
                    pBase3D->SetMaterial(aBackMaterial.GetMaterial(Base3DMaterialEmission), Base3DMaterialEmission, Base3DMaterialBack);
                    pBase3D->SetShininess(aBackMaterial.GetShininess(), Base3DMaterialBack);
                }

                Base3DTextureWrap eWrapX = Base3DTextureRepeat;
                Base3DTextureWrap eWrapY = Base3DTextureRepeat;
                Matrix4D mTexture;

                if(pBase3D->GetOutputDevice()->GetDrawMode() & DRAWMODE_WHITEFILL)
                {
                    // Farbe bleibt auf Weiss, nur noch den RenderMode setzen
                    pBase3D->SetRenderMode(Base3DRenderFill);
                }
                else if(eFillStyle == XFILL_NONE)
                {
                    bDrawObject = FALSE;
                }
                else if(eFillStyle == XFILL_BITMAP)
                {
                    Bitmap aBmp;

                    if(SFX_ITEM_SET == rSet.GetItemState(XATTR_FILLBITMAP, TRUE))
                    {
                        // EIndeutige Bitmap, benutze diese
                        aBmp = ITEMVALUE( rSet, XATTR_FILLBITMAP, XFillBitmapItem ).GetBitmap();
                    }
                    else
                    {
                        // Keine eindeutige Bitmap. benutze default
                        //
                        // DIES IST EINE NOTLOESUNG, BIS MAN IRGENDWO AN DIE
                        // DEAULT-BITMAP RANKOMMT (IST VON KA IN VORBEREITUNG)
                        //
                        aBmp = Bitmap(Size(4,4), 8);
                    }

                    // Texturattribute bilden
                    TextureAttributesBitmap aTexAttr(aBmp, bGhosted);

                    pTexture = pBase3D->ObtainTexture(aTexAttr);
                    if(!pTexture)
                    {
                        if(bGhosted)
                        {
                            aBmp.Adjust( 50 );
                        }

                        pTexture = pBase3D->ObtainTexture(aTexAttr, aBmp);
                    }

                    USHORT nOffX = ITEMVALUE( rSet, XATTR_FILLBMP_TILEOFFSETX, SfxUInt16Item );
                    USHORT nOffY = ITEMVALUE( rSet, XATTR_FILLBMP_TILEOFFSETY, SfxUInt16Item );
                    USHORT nOffPosX = ITEMVALUE( rSet, XATTR_FILLBMP_POSOFFSETX, SfxUInt16Item );
                    USHORT nOffPosY = ITEMVALUE( rSet, XATTR_FILLBMP_POSOFFSETY, SfxUInt16Item );
                    RECT_POINT eRectPoint = (RECT_POINT) ITEMVALUE( rSet, XATTR_FILLBMP_POS, SfxEnumItem );
                    BOOL bTile = ITEMVALUE( rSet, XATTR_FILLBMP_TILE, SfxBoolItem );
                    BOOL bStretch = ITEMVALUE( rSet, XATTR_FILLBMP_STRETCH, SfxBoolItem );
                    BOOL bLogSize = ITEMVALUE( rSet, XATTR_FILLBMP_SIZELOG, SfxBoolItem );

                    Size aSize;
                    aSize.Width() = labs( ITEMVALUE( rSet, XATTR_FILLBMP_SIZEX, SfxMetricItem ) );
                    aSize.Height() = labs( ITEMVALUE( rSet, XATTR_FILLBMP_SIZEY, SfxMetricItem ) );

                    Vector3D aScaleVector(1.0, 1.0, 1.0);
                    Vector3D aTranslateVector(0.0, 0.0, 0.0);

                    // Groesse beachten, logische Groesse einer Kachel bestimmen
                    // erst mal in 1/100 mm
                    Size aLogicalSize = aBmp.GetPrefSize();
                    const Volume3D& rVol = GetBoundVolume();
                    if(aLogicalSize.Width() == 0 || aLogicalSize.Height() == 0)
                    {
                        // Keine logische Groesse, nimm Pixelgroesse
                        // und wandle diese um
                        aLogicalSize = Application::GetDefaultDevice()->PixelToLogic(aBmp.GetSizePixel(), MAP_100TH_MM);
                    }
                    else
                    {
                        aLogicalSize =
                            OutputDevice::LogicToLogic(aLogicalSize, aBmp.GetPrefMapMode(), MAP_100TH_MM);
                    }

                    if(bLogSize)
                    {
                        // logische Groesse
                        if(aSize.Width() == 0 && aSize.Height() == 0)
                        {
                            // Originalgroesse benutzen, Original flag

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
                            double fLeftBound = (rVol.GetWidth() / 2.0)
                                - ((double)aLogicalSize.Width() / 2.0);
                            double fTopBound = (rVol.GetHeight() / 2.0)
                                - ((double)aLogicalSize.Height() / 2.0);

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
                    TextureAttributesGradient aTexAttr(
                        (void*)&rSet.Get(XATTR_FILLGRADIENT),
                        (void*)&rSet.Get(XATTR_GRADIENTSTEPCOUNT),
                        bGhosted);

                    pTexture = pBase3D->ObtainTexture(aTexAttr);
                    if(!pTexture)
                    {
                        Bitmap aBmp = GetGradientBitmap(rSet);

                        if(bGhosted)
                        {
                            aBmp.Adjust( 50 );
                        }

                        pTexture = pBase3D->ObtainTexture(aTexAttr, aBmp);
                    }
                }
                else if(eFillStyle == XFILL_HATCH)
                {
                    TextureAttributesHatch aTexAttr(
                        (void*)&rSet.Get(XATTR_FILLHATCH),
                        bGhosted);

                    pTexture = pBase3D->ObtainTexture(aTexAttr);
                    if(!pTexture)
                    {
                        Bitmap aBmp = GetHatchBitmap(rSet);

                        if(bGhosted)
                        {
                            aBmp.Adjust( 50 );
                        }

                        pTexture = pBase3D->ObtainTexture(aTexAttr, aBmp);
                    }

                    // Texturtransformation setzen
                    mTexture.Scale(Vector3D(20.0, 20.0, 20.0));
                }
                else // if(eFillStyle == XFILL_SOLID)
                {
                    // Material setzen
                    pBase3D->SetMaterial(aColorSolid, Base3DMaterialAmbient);
                    pBase3D->SetMaterial(aColorSolidWithTransparency, Base3DMaterialDiffuse);
                    if(GetUseDifferentBackMaterial())
                    {
                        pBase3D->SetMaterial(aBackMaterial.GetMaterial(Base3DMaterialAmbient), Base3DMaterialAmbient, Base3DMaterialBack);
                        pBase3D->SetMaterial(aBackMaterial.GetMaterial(Base3DMaterialDiffuse), Base3DMaterialDiffuse, Base3DMaterialBack);
                    }
                    pBase3D->SetRenderMode(Base3DRenderFill);
                }

                // Textur verwenden?
                if(pTexture)
                {
                    // Einige Modi einstellen
//-/                    pTexture->SetTextureKind(eTextureKind);
                    pTexture->SetTextureKind(GetTextureKind());

//-/                    pTexture->SetTextureMode(eTextureMode);
                    pTexture->SetTextureMode(GetTextureMode());

//-/                    pTexture->SetTextureFilter(bFilterTexture ? Base3DTextureLinear : Base3DTextureNearest);
                    pTexture->SetTextureFilter(GetTextureFilter() ?
                        Base3DTextureLinear : Base3DTextureNearest);

                    pTexture->SetTextureWrapS(eWrapX);
                    pTexture->SetTextureWrapT(eWrapY);

                    pTexture->SetBlendColor(aColorSolid);
                    pTexture->SetTextureColor(aColorSolid);

                    // Textur aktivieren
                    pBase3D->SetActiveTexture(pTexture);
                    pBase3D->SetRenderMode(Base3DRenderFill);

                    // Texturtransformation setzen
                    GetScene()->GetCameraSet().SetTexture(mTexture);
                }
                else
                {
                    // Reset auf Standardwerte
                    pBase3D->SetActiveTexture();
                }
            }
            else
            {
                bDrawObject = FALSE;
            }
        }
        else
        {
            bDrawObject = FALSE;
        }
    }

    bDrawOutline = (nDrawFlags & E3D_DRAWFLAG_OUTLINE);
    if(bDrawOutline)
    {
        XLineStyle aLineStyle(XLINE_NONE);
//-/        const XLineAttrSetItem* pLineAttr = GetLineAttr();
        if(!rOut.GetIgnoreLineStyle())
        {
            Color aColorLine(COL_WHITE);
            UINT16 nLineTransparence = ITEMVALUE(GetItemSet(), XATTR_LINETRANSPARENCE, XLineTransparenceItem);

            if(
                (nLineTransparence && (nDrawFlags & E3D_DRAWFLAG_TRANSPARENT))
                || (!nLineTransparence && !(nDrawFlags & E3D_DRAWFLAG_TRANSPARENT)))
            {
                aColorLine = ((const XLineColorItem&)(GetItem(XATTR_LINECOLOR))).GetValue();

                if(bGhosted)
                {
                    aColorLine = Color( (aColorLine.GetRed() >> 1) + 0x80,
                                        (aColorLine.GetGreen() >> 1) + 0x80,
                                        (aColorLine.GetBlue() >> 1) + 0x80);
                }

                aColorLine.SetTransparency((UINT8)(nLineTransparence * 255 / 100));
                aLineStyle = ((const XLineStyleItem&)(GetItem(XATTR_LINESTYLE))).GetValue();
                INT32 nLineWidth = ((const XLineWidthItem&)(GetItem(XATTR_LINEWIDTH))).GetValue();
                if(nLineWidth && !bIsLineDraft)
                {
                    Point aPnt(nLineWidth, 0);
                    aPnt = pBase3D->GetOutputDevice()->LogicToPixel(aPnt)
                        - pBase3D->GetOutputDevice()->LogicToPixel(Point());
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
            else
            {
                bDrawOutline = FALSE;
            }
        }

        // Muss auch eine Outline des Objektes generiert werden?
        if(bDrawOutline)
            bDrawOutline = (aLineStyle != XLINE_NONE);

        // Spezieller Modus, falls in Schwarz/Weiss gezeichnet werden soll.
        // Die Linienfarbe ist bereits auf Schwarz gesetzt (Base3d::SetColor())
        if(!bDrawOutline && pBase3D->GetOutputDevice()->GetDrawMode() & DRAWMODE_WHITEFILL)
            bDrawOutline = TRUE;
    }

    if(bDrawObject || bDrawOutline)
    {
        // ObjectTrans setzen
        Matrix4D mTransform = GetFullTransform();
        GetScene()->GetCameraSet().SetObjectTrans(mTransform);
        pBase3D->SetTransformationSet(&(GetScene()->GetCameraSet()));
    }
}

/*************************************************************************
|*
|* Transformation auf die Geometrie anwenden
|*
\************************************************************************/

void E3dCompoundObject::ApplyTransform(const Matrix4D& rMatrix)
{
    // call parent
    E3dObject::ApplyTransform(rMatrix);

    // Anwenden auf subobjekte (alte Geometrie)
    ULONG nObjCnt = pSub->GetObjCount();
    aLocalBoundVol = Volume3D();

    for (ULONG i = 0; i < nObjCnt; i++)
    {
        E3dObject *p3DObj = (E3dObject*) pSub->GetObj(i);

        if ( p3DObj->IsPartOfParent() )
            aLocalBoundVol.Union(p3DObj->GetLocalBoundVolume());
    }

    // Geometrie herstellen
    if(!bGeometryValid)
        ReCreateGeometry();

    // Matrix auch auf die neue Geometrie anwenden
    aDisplayGeometry.Transform(rMatrix);

    // LocalBoundVol neu aufbauen
    aLocalBoundVol = aDisplayGeometry.GetBoundVolume();
}

/*************************************************************************
|*
|* Hittest fuer 3D-Objekte, wird an Geometrie weitergegeben
|*
\************************************************************************/

SdrObject* E3dCompoundObject::CheckHit(const Point& rPnt, USHORT nTol,
    const SetOfByte* pVisiLayer) const
{
    E3dPolyScene* pScene = (E3dPolyScene*)GetScene();
    if(pScene)
    {
        // HitLine holen in ObjektKoordinaten
        // ObjectTrans setzen
        Matrix4D mTransform = ((E3dCompoundObject*)this)->GetFullTransform();
        pScene->GetCameraSet().SetObjectTrans(mTransform);

        // HitPoint Front und Back erzeugen und umrechnen
        Vector3D aFront(rPnt.X(), rPnt.Y(), 0.0);
        Vector3D aBack(rPnt.X(), rPnt.Y(), ZBUFFER_DEPTH_RANGE);
        aFront = pScene->GetCameraSet().ViewToObjectCoor(aFront);
        aBack = pScene->GetCameraSet().ViewToObjectCoor(aBack);

        const Volume3D& rBoundVol = ((E3dCompoundObject*)this)->GetBoundVolume();
        if(rBoundVol.IsValid())
        {
            double fXMax = aFront.X();
            double fXMin = aBack.X();

            if(fXMax < fXMin)
            {
                fXMax = aBack.X();
                fXMin = aFront.X();
            }

            if(rBoundVol.MinVec().X() <= fXMax && rBoundVol.MaxVec().X() >= fXMin)
            {
                double fYMax = aFront.Y();
                double fYMin = aBack.Y();

                if(fYMax < fYMin)
                {
                    fYMax = aBack.Y();
                    fYMin = aFront.Y();
                }

                if(rBoundVol.MinVec().Y() <= fYMax && rBoundVol.MaxVec().Y() >= fYMin)
                {
                    double fZMax = aFront.Z();
                    double fZMin = aBack.Z();

                    if(fZMax < fZMin)
                    {
                        fZMax = aBack.Z();
                        fZMin = aFront.Z();
                    }

                    if(rBoundVol.MinVec().Z() <= fZMax && rBoundVol.MaxVec().Z() >= fZMin)
                    {
                        // Geometrie herstellen
                        if(!bGeometryValid)
                            ((E3dCompoundObject*)this)->ReCreateGeometry();

                        // 3D Volumes schneiden sich, teste in der Geometrie
                        // auf Basis der Projektion weiter
                        if(((E3dCompoundObject*)this)->aDisplayGeometry.CheckHit(aFront, aBack, nTol) != -1L)
                            return ((E3dCompoundObject*)this);
                    }
                }
            }
        }
    }
    return NULL;
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
    ApplyTransform(aTransMat);
}

/*************************************************************************
|*
|* Schattenattribute holen
|*
\************************************************************************/

Color E3dCompoundObject::GetShadowColor()
{
    return ((SdrShadowColorItem&)(GetItem(SDRATTR_SHADOWCOLOR))).GetValue();
//-/    Color aShadCol = rShadColItem.GetValue();
//-/    return aShadCol;
//-/    Color aShadCol;
//-/
//-/    if (pShadAttr!=NULL)
//-/    {
//-/        const SdrShadowColorItem& rShadColItem=((SdrShadowColorItem&)(pShadAttr->GetItem(SDRATTR_SHADOWCOLOR)));
//-/        aShadCol = rShadColItem.GetValue();
//-/    }
//-/
//-/    return aShadCol;
}

BOOL E3dCompoundObject::DrawShadowAsOutline()
{
    const SfxItemSet& rSet = GetItemSet();
    XFillStyle eFillStyle = ((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();
    XLineStyle eLineStyle = ((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
    BOOL bFillAttrIsNone = eFillStyle == XFILL_NONE;
    BOOL bLineAttrIsNone = eLineStyle == XLINE_NONE;
    return (bFillAttrIsNone && !bLineAttrIsNone);
//-/    BOOL bRetval = FALSE;
//-/
//-/    if (pShadAttr!=NULL)
//-/    {
//-/        BOOL bFillAttrIsNone = TRUE;
//-/        if (pFillAttr!=NULL)
//-/        {
//-/            XFillStyle eFillStyle=((XFillStyleItem&)(pFillAttr->GetItem(XATTR_FILLSTYLE))).GetValue();
//-/            if (eFillStyle!=XFILL_NONE)
//-/                bFillAttrIsNone = FALSE;
//-/        }
//-/
//-/        BOOL bLineAttrIsNone = TRUE;
//-/        if (pLineAttr!=NULL)
//-/        {
//-/            XLineStyle eLineStyle=((XLineStyleItem&)(pLineAttr->GetItem(XATTR_LINESTYLE))).GetValue();
//-/            if (eLineStyle!=XLINE_NONE)
//-/                bLineAttrIsNone = FALSE;
//-/        }
//-/
//-/        bRetval = bFillAttrIsNone && !bLineAttrIsNone;
//-/    }
//-/
//-/    return bRetval;
}

INT32 E3dCompoundObject::GetShadowXDistance()
{
    return (long)((SdrShadowXDistItem&)(GetItem(SDRATTR_SHADOWXDIST))).GetValue();
}

INT32 E3dCompoundObject::GetShadowYDistance()
{
    return (long)((SdrShadowYDistItem&)(GetItem(SDRATTR_SHADOWYDIST))).GetValue();
}

UINT16 E3dCompoundObject::GetShadowTransparence()
{
    return (UINT16)((SdrShadowTransparenceItem&)(GetItem(SDRATTR_SHADOWTRANSPARENCE))).GetValue();
}

BOOL E3dCompoundObject::DoDrawShadow()
{
    const SfxItemSet& rSet = GetItemSet();
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
//-/    BOOL bRetval = FALSE;
//-/
//-/    if (pShadAttr!=NULL)
//-/    {
//-/        FASTBOOL bShadOn=((SdrShadowItem&)(pShadAttr->GetItem(SDRATTR_SHADOW))).GetValue();
//-/        if (bShadOn) {
//-/            bRetval = TRUE;
//-/
//-/            // Eventuell bRetval auf FALSE falls nicht gefuellt und
//-/            // kein Linienstil!
//-/            if(!pFillAttr
//-/                || ((XFillStyleItem&)(pFillAttr->GetItem(XATTR_FILLSTYLE))).GetValue() == XFILL_NONE)
//-/            {
//-/                if(!pLineAttr
//-/                    || ((XLineStyleItem&)(pLineAttr->GetItem(XATTR_LINESTYLE))).GetValue() == XLINE_NONE)
//-/                {
//-/                    bRetval = FALSE;
//-/                }
//-/            }
//-/        }
//-/    }
    return bRetval;
}

/*************************************************************************
|*
|* Objekt als WireFrame zeichnen
|*
\************************************************************************/

void E3dCompoundObject::DrawObjectWireframe(ExtOutputDevice& rXOut)
{
    UINT32 nPolyCounter = 0;
    UINT32 nEntityCounter = 0;
    UINT32 nUpperBound;
    Point aFirstPoint, aLastPoint, aNewPoint;
    B3dEntityBucket& rEntityBucket = GetDisplayGeometry().GetEntityBucket();
    GeometryIndexValueBucket& rIndexBucket = GetDisplayGeometry().GetIndexBucket();
    B3dTransformationSet& rSet = GetScene()->GetCameraSet();
    BOOL bDrawLine, bLastDrawLine;
    Vector3D aPoint;

    while(nPolyCounter < rIndexBucket.Count())
    {
        // Naechstes Primitiv
        nUpperBound = rIndexBucket[nPolyCounter++].GetIndex();
        bDrawLine = bLastDrawLine = rEntityBucket[nEntityCounter].IsEdgeVisible();
        aPoint = rSet.ObjectToViewCoor(rEntityBucket[nEntityCounter++].Point().GetVector3D());
        aFirstPoint.X() = (long)(aPoint.X() + 0.5);
        aFirstPoint.Y() = (long)(aPoint.Y() + 0.5);
        aLastPoint = aFirstPoint;

        // Polygon fuellen
        while(nEntityCounter < nUpperBound)
        {
            // Punkt holen und auf Weltkoordinaten umrechnen
            bDrawLine = rEntityBucket[nEntityCounter].IsEdgeVisible();
            aPoint = rSet.ObjectToViewCoor(rEntityBucket[nEntityCounter++].Point().GetVector3D());
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
|* Ein Segment fuer Extrude oder Lathe erzeugen
|*
\************************************************************************/

void E3dCompoundObject::CreateSegment(
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
    BOOL bCreateTexture,
    BOOL bCreateNormals,
    BOOL bCharacterExtrude,             // FALSE=exakt, TRUE=ohne Ueberschneidungen
    BOOL bRotateTexture90               // Textur der Seitenflaechen um 90 Grad kippen
    )
{
    PolyPolygon3D aNormalsLeft, aNormalsRight;
    AddInBetweenNormals(rFront, rBack, aNormalsLeft, bSmoothLeft);
    AddInBetweenNormals(rFront, rBack, aNormalsRight, bSmoothRight);
    Vector3D aOffset = rBack.GetMiddle() - rFront.GetMiddle();

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
                CreateFront(rFront, aNormalsFront, bCreateNormals, bCreateTexture);
            if(bSmoothLeft)
                AddFrontNormals(rFront, aNormalsLeft, aOffset);
            if(bSmoothFrontBack)
                CreateFront(rFront, aNormalsLeft, bCreateNormals, bCreateTexture);
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
                CreateBack(rBack, aNormalsBack, bCreateNormals, bCreateTexture);
            if(bSmoothRight)
                AddBackNormals(rBack, aNormalsRight, aOffset);
            if(bSmoothFrontBack)
                CreateBack(rBack, aNormalsRight, bCreateNormals, bCreateTexture);
        }
        else
        {
            if(pNext)
                AddInBetweenNormals(rBack, *pNext, aNormalsRight, bSmoothRight);
        }

        // eigentliches Zwischenstueck
        CreateInBetween(rFront, rBack,
            aNormalsLeft, aNormalsRight,
            bCreateNormals,
            fSurroundFactor,
            fTextureStart,
            fTextureDepth,
            bRotateTexture90);
    }
    else
    {
        // Mit Scraegen, Vorderseite
        PolyPolygon3D aLocalFront = rFront;
        PolyPolygon3D aLocalBack = rBack;
        double fExtrudeDepth, fDiagLen;
        double fTexMidStart = fTextureStart;
        double fTexMidDepth = fTextureDepth;

        if(bCreateFront || bCreateBack)
        {
            fExtrudeDepth = aOffset.GetLength();
            fDiagLen = fPercentDiag * fExtrudeDepth;
        }

        if(bCreateFront)
        {
            PolyPolygon3D aOuterFront;
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
                GrowPoly(aLocalFront, aNormalsOuterFront, -fDiagLen);

                // Neue Groesse inneres Polygon feststellen
                Volume3D aNewSize(aLocalFront.GetPolySize());

                // Skalierung feststellen (nur X,Y)
                Vector3D aScaleVec(aOldSize.GetWidth() / aNewSize.GetWidth(),
                    aOldSize.GetHeight() / aNewSize.GetHeight(),
                    1.0);

                // Transformation bilden
                Matrix4D aTransMat;
                aTransMat.Scale(aScaleVec);

                // aeusseres und inneres Polygon skalieren
                aLocalFront.Transform(aTransMat);
                aOuterFront.Transform(aTransMat);

                // Neue Groesse aktualisieren
                aNewSize = aLocalFront.GetPolySize();

                // Translation feststellen
                Vector3D aTransVec(aOldSize.MinVec().X() - aNewSize.MinVec().X(),
                    aOldSize.MinVec().Y() - aNewSize.MinVec().Y(),
                    0.0);

                // Transformation bilden
                aTransMat.Identity();
                aTransMat.Translate(aTransVec);

                // aeusseres und inneres Polygon skalieren
                aLocalFront.Transform(aTransMat);
                aOuterFront.Transform(aTransMat);
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
                bCreateNormals,
                fSurroundFactor,
                fTextureStart,
                fTextureDepth * fPercentDiag,
                bRotateTexture90);

            // Vorderseite erzeugen
            CreateFront(aOuterFront, aNormalsOuterFront, bCreateNormals, bCreateTexture);

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
            PolyPolygon3D aOuterBack;
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
                GrowPoly(aLocalBack, aNormalsOuterBack, -fDiagLen);

                // Neue Groesse inneres Polygon feststellen
                Volume3D aNewSize(aLocalBack.GetPolySize());

                // Skalierung feststellen (nur X,Y)
                Vector3D aScaleVec(aOldSize.GetWidth() / aNewSize.GetWidth(),
                    aOldSize.GetHeight() / aNewSize.GetHeight(),
                    1.0);

                // Transformation bilden
                Matrix4D aTransMat;
                aTransMat.Scale(aScaleVec);

                // aeusseres und inneres Polygon skalieren
                aLocalBack.Transform(aTransMat);
                aOuterBack.Transform(aTransMat);

                // Neue Groesse aktualisieren
                aNewSize = aLocalBack.GetPolySize();

                // Translation feststellen
                Vector3D aTransVec(aOldSize.MinVec().X() - aNewSize.MinVec().X(),
                    aOldSize.MinVec().Y() - aNewSize.MinVec().Y(),
                    0.0);

                // Transformation bilden
                aTransMat.Identity();
                aTransMat.Translate(aTransVec);

                // aeusseres und inneres Polygon skalieren
                aLocalBack.Transform(aTransMat);
                aOuterBack.Transform(aTransMat);
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
                bCreateNormals,
                fSurroundFactor,
                fTextureStart + (fTextureDepth * (1.0 - fPercentDiag)),
                fTextureDepth * fPercentDiag,
                bRotateTexture90);

            // Rueckseite erzeugen
            CreateBack(aOuterBack, aNormalsOuterBack, bCreateNormals, bCreateTexture);

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
            bCreateNormals,
            fSurroundFactor,
            fTexMidStart,
            fTexMidDepth,
            bRotateTexture90);
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

//-/void E3dCompoundObject::SetFrontMaterial(const B3dMaterial& rNew)
//-/{
//-/    if(aFrontMaterial != rNew)
//-/    {
//-/        aFrontMaterial = rNew;
//-/    }
//-/}

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

void E3dCompoundObject::Paint3D(ExtOutputDevice& rOut, Base3D* pBase3D,
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
        BOOL bIsLineDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTLINE));
        BOOL bIsFillDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTFILL));
        SetBase3DParams(rOut, pBase3D, bDrawObject, bDrawOutline, nDrawFlags,
            (rInfoRec.pPV && rInfoRec.pPV->GetView().DoVisualizeEnteredGroup()) ? rInfoRec.bNotActive : FALSE,
            bIsLineDraft, bIsFillDraft);

        // Culling?
        pBase3D->SetCullMode(GetDoubleSided() ? Base3DCullNone : Base3DCullBack);

        // Objekt flat darstellen?
//-/        BOOL bForceFlat = (bUseStdNormals && !bUseStdNormalsUseSphere);
        BOOL bForceFlat = ((GetNormalsKind() > 0) && !(GetNormalsKind() > 1));
        pBase3D->SetForceFlat(bForceFlat);

        // Geometrie ausgeben
        if(bDrawObject)
            pBase3D->DrawPolygonGeometry(GetDisplayGeometry());

        // Outline ausgeben
        if(bDrawOutline && pBase3D->GetLightGroup())
        {
            BOOL bLightingWasEnabled = pBase3D->GetLightGroup()->IsLightingEnabled();
            pBase3D->GetLightGroup()->EnableLighting(FALSE);
            pBase3D->SetLightGroup(pBase3D->GetLightGroup());

            // #79585#
            pBase3D->SetActiveTexture();

            pBase3D->DrawPolygonGeometry(GetDisplayGeometry(), TRUE);
            pBase3D->GetLightGroup()->EnableLighting(bLightingWasEnabled);
            pBase3D->SetLightGroup(pBase3D->GetLightGroup());
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
        aTempRect = GetBoundRect();
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
    B3dTransformationSet& rSet = GetScene()->GetCameraSet();
    Vector3D aPoint;
    Point aNewPoint;

    // ObjectTrans setzen
    Matrix4D mTransform = GetFullTransform();
    rSet.SetObjectTrans(mTransform);

    while(nPolyCounter < rIndexBucket.Count())
    {
        // Naechstes Primitiv
        nUpperBound = rIndexBucket[nPolyCounter++].GetIndex();
        XPolygon aNewPart(UINT16(nUpperBound - nEntityCounter));
        UINT16 nIndex = 0;

        while(nEntityCounter < nUpperBound)
        {
            aPoint = rSet.ObjectToViewCoor(rEntityBucket[nEntityCounter++].Point().GetVector3D());
            aNewPart[nIndex  ].X() = (long)(aPoint.X() + 0.5);
            aNewPart[nIndex++].Y() = (long)(aPoint.Y() + 0.5);
        }

        // Teilprimitiv einfuegen
        rPoly.Insert(aNewPart);
    }

    // add shadow now too (#61279#)
    PolyPolygon aShadowPolyPoly;
    GetShadowPolygon(aShadowPolyPoly);

    for(UINT16 a = 0; a < aShadowPolyPoly.Count(); a++)
    {
        XPolygon aNewPart(aShadowPolyPoly[a]);
        rPoly.Insert(aNewPart);
    }
}

/*************************************************************************
|*
|* Schatten fuer 3D-Objekte zeichnen
|*
\************************************************************************/

void E3dCompoundObject::DrawShadows(Base3D *pBase3D, ExtOutputDevice& rXOut,
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
        PolyPolygon aShadowPoly;
        GetShadowPolygon(aShadowPoly);

        // ...und Zeichnen
        DrawShadowPolygon(aShadowPoly, rXOut);
    }
}

void E3dCompoundObject::GetShadowPolygon(PolyPolygon& rPoly)
{
    INT32 nXDist = GetShadowXDistance();
    INT32 nYDist = GetShadowYDistance();
    UINT32 nPolyCounter = 0;
    UINT32 nEntityCounter = 0;
    UINT32 nUpperBound;
    UINT16 nPolyPos = 0;
    Point aPolyPoint;

    // Buckets der Geometrie holen
    B3dTransformationSet& rSet = GetScene()->GetCameraSet();
    B3dEntityBucket& rEntityBucket = GetDisplayGeometry().GetEntityBucket();
    GeometryIndexValueBucket& rIndexBucket = GetDisplayGeometry().GetIndexBucket();

//-/    if(bShadow3D)
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
                aLightNormal = GetDisplayGeometry().GetEntityBucket()[0].Point().GetVector3D()
                    - aLampPositionOrDirection;
                aLightNormal.Normalize();
            }

            double fLightAndNormal = aLightNormal.Scalar(aGroundDirection);
            B3dVolume aVolume = rSet.GetDeviceVolume();

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
                B3dTransformationSet& rSet = GetScene()->GetCameraSet();
                Matrix4D mTransform = GetFullTransform();
                rSet.SetObjectTrans(mTransform);

                while(nPolyCounter < rIndexBucket.Count())
                {
                    // Naechstes Primitiv
                    nUpperBound = rIndexBucket[nPolyCounter++].GetIndex();
                    nPolyPos = 0;
                    bPolygonVisible = TRUE;
                    Polygon aPoly((UINT16)(nUpperBound - nEntityCounter));

                    // Polygon fuellen
                    while(nEntityCounter < nUpperBound)
                    {
                        // Naechsten Punkt holen
                        Vector3D aPoint = rEntityBucket[nEntityCounter++].Point().GetVector3D();

                        // Auf Augkoordinaten umrechnen
                        aPoint = rSet.ObjectToEyeCoor(aPoint);

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
                            Vector3D aShadowPoint = rSet.EyeToViewCoor(aPoint);
                            aPolyPoint.X() = (long)(aShadowPoint.X() + 0.5) + nXDist;
                            aPolyPoint.Y() = (long)(aShadowPoint.Y() + 0.5) + nYDist;
                            aPoly[nPolyPos++] = aPolyPoint;
                        }
                    }

                    // Teilpolygon einfuegen
                    rPoly.Insert(aPoly);
                }
            }
        }
    }
    else
    {
        // Normaler 2D Schatten
        Vector3D aPoint;

        // ObjectTrans setzen
        Matrix4D mTransform = GetFullTransform();
        rSet.SetObjectTrans(mTransform);

        if(DrawShadowAsOutline())
        {
            //SubPolygon mit Punktpaaren bilden
            BOOL bLastLineVisible;
            UINT16 nPolySize = (UINT16)(rEntityBucket.Count() * 2);
            Polygon aPoly(nPolySize);
            Vector3D aLast;
            Point aLastPolyPoint;
            nPolyPos = 0;

            while(nPolyCounter < rIndexBucket.Count())
            {
                // Naechstes Primitiv
                nUpperBound = rIndexBucket[nPolyCounter++].GetIndex();

                // Polygon bilden
                if(bLastLineVisible = rEntityBucket[nUpperBound - 1].IsEdgeVisible())
                {
                    aLast = rEntityBucket[nUpperBound - 1].Point().GetVector3D();
                    aLast = rSet.ObjectToViewCoor(aLast);
                    aLastPolyPoint.X() = (long)(aLast.X() + 0.5) + nXDist;
                    aLastPolyPoint.Y() = (long)(aLast.Y() + 0.5) + nYDist;
                }

                while(nEntityCounter < nUpperBound)
                {
                    aPoint = rEntityBucket[nEntityCounter].Point().GetVector3D();
                    aPoint = rSet.ObjectToViewCoor(aPoint);
                    aPolyPoint.X() = (long)(aPoint.X() + 0.5) + nXDist;
                    aPolyPoint.Y() = (long)(aPoint.Y() + 0.5) + nYDist;

                    // Linie aLast, aPoint erfassen
                    if(bLastLineVisible)
                    {
                        if(nPolyPos + 2 > nPolySize)
                        {
                            // Polygon muss groesser
                            nPolySize *= 2;
                            aPoly.SetSize(nPolySize);
                        }
                        aPoly[nPolyPos++] = aLastPolyPoint;
                        aPoly[nPolyPos++] = aPolyPoint;
                    }

                    // naechster Punkt
                    aLastPolyPoint = aPolyPoint;
                    aLast = aPoint;
                    bLastLineVisible = rEntityBucket[nEntityCounter++].IsEdgeVisible();
                }
            }

            // Wahre Groesse setzen
            aPoly.SetSize(nPolyPos);

            // Teilpolygon einfuegen
            rPoly.Insert(aPoly);
        }
        else
        {
            while(nPolyCounter < rIndexBucket.Count())
            {
                // Naechstes Primitiv
                nUpperBound = rIndexBucket[nPolyCounter++].GetIndex();
                Polygon aPoly((UINT16)(nUpperBound - nEntityCounter));
                nPolyPos = 0;

                // Polygon fuellen
                while(nEntityCounter < nUpperBound)
                {
                    aPoint = rEntityBucket[nEntityCounter++].Point().GetVector3D();
                    aPoint = rSet.ObjectToViewCoor(aPoint);
                    aPolyPoint.X() = (long)(aPoint.X() + 0.5) + nXDist;
                    aPolyPoint.Y() = (long)(aPoint.Y() + 0.5) + nYDist;
                    aPoly[nPolyPos++] = aPolyPoint;
                }

                // Teilpolygon einfuegen
                rPoly.Insert(aPoly);
            }
        }
    }
}

void E3dCompoundObject::DrawShadowPolygon(PolyPolygon& rPoly, ExtOutputDevice& rXOut)
{
    Color aCol = GetShadowColor();
    OutputDevice *pDevice = rXOut.GetOutDev();
    BOOL bDrawAsOutline(DrawShadowAsOutline());

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
            Rectangle aBound(rPoly.GetBoundRect());

            // prepare VDev and MetaFile
            aVDev.EnableOutput(FALSE);
            aVDev.SetMapMode(rXOut.GetOutDev()->GetMapMode());
            aMetaFile.Record(&aVDev);

            if(bDrawAsOutline)
            {
                aVDev.SetLineColor(aCol);
                aVDev.SetFillColor();
            }
            else
            {
                aVDev.SetLineColor();
                aVDev.SetFillColor(aCol);
            }

            aVDev.SetFont(rXOut.GetOutDev()->GetFont());
            aVDev.SetDrawMode(rXOut.GetOutDev()->GetDrawMode());
            aVDev.SetRefPoint(rXOut.GetOutDev()->GetRefPoint());

            // create output
            for(UINT16 a(0); a < rPoly.Count(); a++)
                aMetaFile.AddAction(new MetaPolygonAction(rPoly[a]));

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
        if(bDrawAsOutline)
        {
            pDevice->SetLineColor(aCol);
            pDevice->SetFillColor();
        }
        else
        {
            pDevice->SetLineColor();
            pDevice->SetFillColor(aCol);
        }

        for(UINT16 a(0); a < rPoly.Count(); a++)
            pDevice->DrawPolygon(rPoly[a]);
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
    B3dTransformationSet& rSet = GetScene()->GetCameraSet();

    // set ObjectTrans
    Matrix4D mTransform = GetFullTransform();
    rSet.SetObjectTrans(mTransform);

    // transform base polygon to screen coor
    for(UINT16 a=0;a<rExtrudePoly.Count();a++)
    {
        const Polygon3D &rExtPoly = rExtrudePoly[a];
        BOOL bClosed = rExtPoly.IsClosed();
        XPolygon aNewPoly(rExtPoly.GetPointCount() + (bClosed ? 1 : 0));

        UINT16 b;
        for(b=0;b<rExtPoly.GetPointCount();b++)
        {
            Vector3D aPoint = rSet.ObjectToViewCoor(rExtPoly[b]);
            aNewPoly[b].X() = (long)(aPoint.X() + 0.5);
            aNewPoly[b].Y() = (long)(aPoint.Y() + 0.5);
        }

        if(bClosed)
            aNewPoly[b] = aNewPoly[0];

        aNewPolyPolygon.Insert(aNewPoly);
    }

    return aNewPolyPolygon;
}

/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

//-/void E3dCompoundObject::Distribute3DAttributes(const SfxItemSet& rAttr)
//-/{
//-/    // call parent
//-/    E3dObject::Distribute3DAttributes(rAttr);
//-/
//-/    // special Attr for E3dCompoundObject
//-/    const SfxPoolItem* pPoolItem = NULL;
//-/    B3dMaterial aNewMat = GetFrontMaterial();
//-/    BOOL bNewMatUsed = FALSE;
//-/
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_DOUBLE_SIDED, TRUE, &pPoolItem ) )
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        SetDoubleSided(bNew);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_NORMALS_KIND, TRUE, &pPoolItem ) )
//-/    {
//-/        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
//-/        if(nNew == 0)
//-/        {
//-/            SetUseStdNormals(FALSE);
//-/            SetUseStdNormalsUseSphere(FALSE);
//-/        }
//-/        else if(nNew == 1)
//-/        {
//-/            SetUseStdNormals(TRUE);
//-/            SetUseStdNormalsUseSphere(FALSE);
//-/        }
//-/        else
//-/        {
//-/            SetUseStdNormals(TRUE);
//-/            SetUseStdNormalsUseSphere(TRUE);
//-/        }
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_NORMALS_INVERT, TRUE, &pPoolItem ) )
//-/    {
//-/        BOOL bNew = ( ( const SfxBoolItem* ) pPoolItem )->GetValue();
//-/        SetInvertNormals(bNew);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_PROJ_X, TRUE, &pPoolItem ) )
//-/    {
//-/        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
//-/        if(nNew == 0)
//-/        {
//-/            SetUseStdTextureX(FALSE);
//-/            SetUseStdTextureXUseSphere(FALSE);
//-/        }
//-/        else if(nNew == 1)
//-/        {
//-/            SetUseStdTextureX(TRUE);
//-/            SetUseStdTextureXUseSphere(FALSE);
//-/        }
//-/        else
//-/        {
//-/            SetUseStdTextureX(TRUE);
//-/            SetUseStdTextureXUseSphere(TRUE);
//-/        }
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_PROJ_Y, TRUE, &pPoolItem ) )
//-/    {
//-/        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
//-/        if(nNew == 0)
//-/        {
//-/            SetUseStdTextureY(FALSE);
//-/            SetUseStdTextureYUseSphere(FALSE);
//-/        }
//-/        else if(nNew == 1)
//-/        {
//-/            SetUseStdTextureY(TRUE);
//-/            SetUseStdTextureYUseSphere(FALSE);
//-/        }
//-/        else
//-/        {
//-/            SetUseStdTextureY(TRUE);
//-/            SetUseStdTextureYUseSphere(TRUE);
//-/        }
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_SHADOW_3D, TRUE, &pPoolItem ) )
//-/    {
//-/        BOOL bNew = ( ( const SfxBoolItem* ) pPoolItem )->GetValue();
//-/        SetDrawShadow3D(bNew);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_MAT_COLOR, TRUE, &pPoolItem ) )
//-/    {
//-/        Color aNew = ( ( const SvxColorItem* ) pPoolItem )->GetValue();
//-/        aNewMat.SetMaterial(aNew, Base3DMaterialDiffuse);
//-/        bNewMatUsed = TRUE;
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_MAT_EMISSION, TRUE, &pPoolItem ) )
//-/    {
//-/        Color aNew = ( ( const SvxColorItem* ) pPoolItem )->GetValue();
//-/        aNewMat.SetMaterial(aNew, Base3DMaterialEmission);
//-/        bNewMatUsed = TRUE;
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_MAT_SPECULAR, TRUE, &pPoolItem ) )
//-/    {
//-/        Color aNew = ( ( const SvxColorItem* ) pPoolItem )->GetValue();
//-/        aNewMat.SetMaterial(aNew, Base3DMaterialSpecular);
//-/        bNewMatUsed = TRUE;
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_MAT_SPECULAR_INTENSITY, TRUE, &pPoolItem ) )
//-/    {
//-/        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
//-/        aNewMat.SetShininess(nNew);
//-/        bNewMatUsed = TRUE;
//-/    }
//-/    if(bNewMatUsed)
//-/        SetFrontMaterial(aNewMat);
//-/
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_KIND, TRUE, &pPoolItem ) )
//-/    {
//-/        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
//-/        if(nNew == 0)
//-/        {
//-/            SetTextureKind(Base3DTextureLuminance);
//-/        }
//-/        else
//-/        {
//-/            SetTextureKind(Base3DTextureColor);
//-/        }
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_MODE, TRUE, &pPoolItem ) )
//-/    {
//-/        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
//-/        if(nNew == 0)
//-/        {
//-/            SetTextureMode(Base3DTextureReplace);
//-/        }
//-/        else if(nNew == 1)
//-/        {
//-/            SetTextureMode(Base3DTextureModulate);
//-/        }
//-/        else
//-/        {
//-/            SetTextureMode(Base3DTextureBlend);
//-/        }
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_FILTER, TRUE, &pPoolItem ) )
//-/    {
//-/        BOOL bNew = ( ( const SfxBoolItem* ) pPoolItem )->GetValue();
//-/        SetFilterTexture(bNew);
//-/    }
//-/}

void E3dCompoundObject::ImpLocalItemValueChange(const SfxPoolItem& rNew)
{
    switch(rNew.Which())
    {
        case SDRATTR_3DOBJ_DOUBLE_SIDED:
        {
//-/            BOOL bNew = ((const Svx3DDoubleSidedItem&)rNew).GetValue();
//-/            if(GetDoubleSided() != bNew)
                bGeometryValid = FALSE;
            break;
        }
        case SDRATTR_3DOBJ_NORMALS_KIND:
        {
//-/            UINT16 nNew = ((const Svx3DNormalsKindItem&)rNew).GetValue();
//-/            if(GetNormalsKind() != nNew)
                bGeometryValid = FALSE;
//-/            if(nNew == 0)
//-/            {
//-/                ImpSetUseStdNormals(FALSE);
//-/                ImpSetUseStdNormalsUseSphere(FALSE);
//-/            }
//-/            else if(nNew == 1)
//-/            {
//-/                ImpSetUseStdNormals(TRUE);
//-/                ImpSetUseStdNormalsUseSphere(FALSE);
//-/            }
//-/            else
//-/            {
//-/                ImpSetUseStdNormals(TRUE);
//-/                ImpSetUseStdNormalsUseSphere(TRUE);
//-/            }
            break;
        }
        case SDRATTR_3DOBJ_NORMALS_INVERT:
        {
//-/            BOOL bNew = ((const Svx3DNormalsInvertItem&)rNew).GetValue();
//-/            if(GetNormalsInvert() != bNew)
//-/                GetDisplayGeometry().InvertNormals();
                bGeometryValid = FALSE;
//-/            ImpSetInvertNormals(bNew);
            break;
        }
        case SDRATTR_3DOBJ_TEXTURE_PROJ_X:
        {
//-/            UINT16 nNew = ((const Svx3DTextureProjectionXItem&)rNew).GetValue();
//-/            if(GetTextureProjectionX() != nNew)
                bGeometryValid = FALSE;
//-/            if(nNew == 0)
//-/            {
//-/                ImpSetUseStdTextureX(FALSE);
//-/                ImpSetUseStdTextureXUseSphere(FALSE);
//-/            }
//-/            else if(nNew == 1)
//-/            {
//-/                ImpSetUseStdTextureX(TRUE);
//-/                ImpSetUseStdTextureXUseSphere(FALSE);
//-/            }
//-/            else
//-/            {
//-/                ImpSetUseStdTextureX(TRUE);
//-/                ImpSetUseStdTextureXUseSphere(TRUE);
//-/            }
            break;
        }
        case SDRATTR_3DOBJ_TEXTURE_PROJ_Y:
        {
//-/            UINT16 nNew = ((const Svx3DTextureProjectionYItem&)rNew).GetValue();
//-/            if(GetTextureProjectionY() != nNew)
                bGeometryValid = FALSE;
//-/            if(nNew == 0)
//-/            {
//-/                ImpSetUseStdTextureY(FALSE);
//-/                ImpSetUseStdTextureYUseSphere(FALSE);
//-/            }
//-/            else if(nNew == 1)
//-/            {
//-/                ImpSetUseStdTextureY(TRUE);
//-/                ImpSetUseStdTextureYUseSphere(FALSE);
//-/            }
//-/            else
//-/            {
//-/                ImpSetUseStdTextureY(TRUE);
//-/                ImpSetUseStdTextureYUseSphere(TRUE);
//-/            }
            break;
        }
//-/        case SDRATTR_3DOBJ_SHADOW_3D:
//-/        {
//-/            BOOL bNew = ((const Svx3DShadow3DItem&)rNew).GetValue();
//-/            if(GetShadow3D() != bNew)
//-/                ;
//-///-/            ImpSetDrawShadow3D(bNew);
//-/            break;
//-/        }
//-/        case SDRATTR_3DOBJ_MAT_COLOR:
//-/        {
//-/            // set 3d object color as fill color
//-/            Color aNew = ((const Svx3DMaterialColorItem&)rNew).GetValue();
//-/            ImpForceItemSet();
//-/            mpObjectItemSet->Put(XFillColorItem(String(), aNew));
//-///-/            if(GetMaterialColor() != aNew)
//-///-/            {
//-///-/                B3dMaterial aNewMat = GetFrontMaterial();
//-///-/                aNewMat.SetMaterial(aNew, Base3DMaterialDiffuse);
//-///-/                SetFrontMaterial(aNewMat);
//-///-/            }
//-/            break;
//-/        }
//-/        case SDRATTR_3DOBJ_MAT_EMISSION:
//-/        {
//-/            Color aNew = ((const Svx3DMaterialEmissionItem&)rNew).GetValue();
//-/            if(GetMaterialEmission() != aNew)
//-/            {
//-/                B3dMaterial aNewMat = GetFrontMaterial();
//-/                aNewMat.SetMaterial(aNew, Base3DMaterialEmission);
//-/                SetFrontMaterial(aNewMat);
//-/            }
//-/            break;
//-/        }
//-/        case SDRATTR_3DOBJ_MAT_SPECULAR:
//-/        {
//-/            Color aNew = ((const Svx3DMaterialSpecularItem&)rNew).GetValue();
//-/            if(GetMaterialSpecular() != aNew)
//-/            {
//-/                B3dMaterial aNewMat = GetFrontMaterial();
//-/                aNewMat.SetMaterial(aNew, Base3DMaterialSpecular);
//-/                SetFrontMaterial(aNewMat);
//-/            }
//-/            break;
//-/        }
//-/        case SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY:
//-/        {
//-/            UINT16 nNew = ((const Svx3DMaterialSpecularIntensityItem&)rNew).GetValue();
//-/            if(GetMaterialSpecularIntensity() != nNew)
//-/            {
//-/                B3dMaterial aNewMat = GetFrontMaterial();
//-/                aNewMat.SetShininess(nNew);
//-/                SetFrontMaterial(aNewMat);
//-/            }
//-/            break;
//-/        }
//-/        case SDRATTR_3DOBJ_TEXTURE_KIND:
//-/        {
//-/            Base3DTextureKind eNew = ((const Svx3DTextureKindItem&)rNew).GetValue();
//-/            if(GetTextureKind() != eNew)
//-/                ;
//-/            if(nNew == 0)
//-/            {
//-/                ImpSetTextureKind(Base3DTextureLuminance);
//-/            }
//-/            else
//-/            {
//-/                ImpSetTextureKind(Base3DTextureColor);
//-/            }
//-/            break;
//-/        }
//-/        case SDRATTR_3DOBJ_TEXTURE_MODE:
//-/        {
//-/            Base3DTextureMode eNew = ((const Svx3DTextureModeItem&)rNew).GetValue();
//-/            if(GetTextureMode() != eNew)
//-/                ;
//-/            if(nNew == 0)
//-/            {
//-/                ImpSetTextureMode(Base3DTextureReplace);
//-/            }
//-/            else if(nNew == 1)
//-/            {
//-/                ImpSetTextureMode(Base3DTextureModulate);
//-/            }
//-/            else
//-/            {
//-/                ImpSetTextureMode(Base3DTextureBlend);
//-/            }
//-/            break;
//-/        }
//-/        case SDRATTR_3DOBJ_TEXTURE_FILTER:
//-/        {
//-/            BOOL bNew = ((const Svx3DTextureFilterItem&)rNew).GetValue();
//-/            if(GetTextureFilter() != bNew)
//-/                ;
//-/            ImpSetFilterTexture(bNew);
//-/            break;
//-/        }
    }
}

void E3dCompoundObject::SetItem( const SfxPoolItem& rItem )
{
    // set item
    E3dObject::SetItem(rItem);

    // handle value change
    if(rItem.Which() >= SDRATTR_3DOBJ_FIRST && rItem.Which() <= SDRATTR_3DOBJ_LAST)
        ImpLocalItemValueChange(rItem);

//-/    // set fill color as 3d object color
//-/    if(rItem.Which() == XATTR_FILLCOLOR)
//-/    {
//-/        Color aNew = ((const XFillColorItem&)rItem).GetValue();
//-/        mpObjectItemSet->Put(Svx3DMaterialColorItem(aNew));
//-/    }
}

void E3dCompoundObject::ClearItem( USHORT nWhich )
{
    if(mpObjectItemSet)
    {
        // clear base items at SdrAttrObj, NOT at E3dObject(!)
        E3dObject::ClearItem(nWhich);

        // handle value change
        if(nWhich >= SDRATTR_3DOBJ_FIRST && nWhich <= SDRATTR_3DOBJ_LAST)
            ImpLocalItemValueChange(mpObjectItemSet->Get(nWhich));

//-/        // clear fill color when 3d object color is cleared
//-/        if(nWhich == XATTR_FILLCOLOR)
//-/            mpObjectItemSet->ClearItem(SDRATTR_3DOBJ_MAT_COLOR);
    }
}

void E3dCompoundObject::SetItemSet( const SfxItemSet& rSet )
{
    // set base items at SdrAttrObj, NOT at E3dObject(!)
    E3dObject::SetItemSet(rSet);

    // handle value change
    for(sal_uInt16 nWhich(SDRATTR_3DOBJ_FIRST); nWhich <= SDRATTR_3DOBJ_TEXTURE_FILTER; nWhich++)
        if(SFX_ITEM_SET == rSet.GetItemState(nWhich, FALSE))
            ImpLocalItemValueChange(rSet.Get(nWhich));

//-/    // set fill color as 3d object color
//-/    if(SFX_ITEM_SET == rSet.GetItemState(XATTR_FILLCOLOR, FALSE))
//-/    {
//-/        Color aNew = ((const XFillColorItem&)rSet.Get(XATTR_FILLCOLOR)).GetValue();
//-/        mpObjectItemSet->Put(Svx3DMaterialColorItem(aNew));
//-/    }
}

//-/void E3dCompoundObject::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/    // call parent
//-/    E3dObject::NbcSetAttributes(rAttr, bReplaceAll);
//-/
//-/    // special Attr for E3dCompoundObject
//-/    const SfxPoolItem* pPoolItem = NULL;
//-/    B3dMaterial aNewMat = GetFrontMaterial();
//-/    BOOL bNewMatUsed = FALSE;
//-/
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_DOUBLE_SIDED, TRUE, &pPoolItem ) )
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        SetDoubleSided(bNew);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_NORMALS_KIND, TRUE, &pPoolItem ) )
//-/    {
//-/        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
//-/        if(nNew == 0)
//-/        {
//-/            SetUseStdNormals(FALSE);
//-/            SetUseStdNormalsUseSphere(FALSE);
//-/        }
//-/        else if(nNew == 1)
//-/        {
//-/            SetUseStdNormals(TRUE);
//-/            SetUseStdNormalsUseSphere(FALSE);
//-/        }
//-/        else
//-/        {
//-/            SetUseStdNormals(TRUE);
//-/            SetUseStdNormalsUseSphere(TRUE);
//-/        }
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_NORMALS_INVERT, TRUE, &pPoolItem ) )
//-/    {
//-/        BOOL bNew = ( ( const SfxBoolItem* ) pPoolItem )->GetValue();
//-/        SetInvertNormals(bNew);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_PROJ_X, TRUE, &pPoolItem ) )
//-/    {
//-/        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
//-/        if(nNew == 0)
//-/        {
//-/            SetUseStdTextureX(FALSE);
//-/            SetUseStdTextureXUseSphere(FALSE);
//-/        }
//-/        else if(nNew == 1)
//-/        {
//-/            SetUseStdTextureX(TRUE);
//-/            SetUseStdTextureXUseSphere(FALSE);
//-/        }
//-/        else
//-/        {
//-/            SetUseStdTextureX(TRUE);
//-/            SetUseStdTextureXUseSphere(TRUE);
//-/        }
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_PROJ_Y, TRUE, &pPoolItem ) )
//-/    {
//-/        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
//-/        if(nNew == 0)
//-/        {
//-/            SetUseStdTextureY(FALSE);
//-/            SetUseStdTextureYUseSphere(FALSE);
//-/        }
//-/        else if(nNew == 1)
//-/        {
//-/            SetUseStdTextureY(TRUE);
//-/            SetUseStdTextureYUseSphere(FALSE);
//-/        }
//-/        else
//-/        {
//-/            SetUseStdTextureY(TRUE);
//-/            SetUseStdTextureYUseSphere(TRUE);
//-/        }
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_SHADOW_3D, TRUE, &pPoolItem ) )
//-/    {
//-/        BOOL bNew = ( ( const SfxBoolItem* ) pPoolItem )->GetValue();
//-/        SetDrawShadow3D(bNew);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_MAT_COLOR, TRUE, &pPoolItem ) )
//-/    {
//-/        Color aNew = ( ( const SvxColorItem* ) pPoolItem )->GetValue();
//-/
//-/        // Im Objekt setzen
//-/        SfxItemSet aNewSet(GetModel()->GetItemPool());
//-/        aNewSet.Put(XFillColorItem(String(), aNew));
//-/        SdrAttrObj::NbcSetAttributes(aNewSet, FALSE);
//-/
//-/        // ...und im Material setzen
//-/        aNewMat.SetMaterial(aNew, Base3DMaterialDiffuse);
//-/        bNewMatUsed = TRUE;
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_MAT_EMISSION, TRUE, &pPoolItem ) )
//-/    {
//-/        Color aNew = ( ( const SvxColorItem* ) pPoolItem )->GetValue();
//-/        aNewMat.SetMaterial(aNew, Base3DMaterialEmission);
//-/        bNewMatUsed = TRUE;
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_MAT_SPECULAR, TRUE, &pPoolItem ) )
//-/    {
//-/        Color aNew = ( ( const SvxColorItem* ) pPoolItem )->GetValue();
//-/        aNewMat.SetMaterial(aNew, Base3DMaterialSpecular);
//-/        bNewMatUsed = TRUE;
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_MAT_SPECULAR_INTENSITY, TRUE, &pPoolItem ) )
//-/    {
//-/        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
//-/        aNewMat.SetShininess(nNew);
//-/        bNewMatUsed = TRUE;
//-/    }
//-/    if(bNewMatUsed)
//-/        SetFrontMaterial(aNewMat);
//-/
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_KIND, TRUE, &pPoolItem ) )
//-/    {
//-/        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
//-/        if(nNew == 0)
//-/        {
//-/            SetTextureKind(Base3DTextureLuminance);
//-/        }
//-/        else
//-/        {
//-/            SetTextureKind(Base3DTextureColor);
//-/        }
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_MODE, TRUE, &pPoolItem ) )
//-/    {
//-/        UINT16 nNew = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();
//-/        if(nNew == 0)
//-/        {
//-/            SetTextureMode(Base3DTextureReplace);
//-/        }
//-/        else if(nNew == 1)
//-/        {
//-/            SetTextureMode(Base3DTextureModulate);
//-/        }
//-/        else
//-/        {
//-/            SetTextureMode(Base3DTextureBlend);
//-/        }
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState( SID_ATTR_3D_TEXTURE_FILTER, TRUE, &pPoolItem ) )
//-/    {
//-/        BOOL bNew = ( ( const SfxBoolItem* ) pPoolItem )->GetValue();
//-/        SetFilterTexture(bNew);
//-/    }
//-/}

/*************************************************************************
|*
|* Attribute lesen
|*
\************************************************************************/

//-/void E3dCompoundObject::Collect3DAttributes(SfxItemSet& rAttr) const
//-/{
//-/    // call parent
//-/    E3dObject::Collect3DAttributes(rAttr);
//-/
//-/    // special Attr for E3dCompoundObject
//-/    BOOL bObjDoubleSided = bDoubleSided;
//-/    UINT16 nObjNormalsKind;
//-/    if(!bUseStdNormals)
//-/    {
//-/        nObjNormalsKind = 0;
//-/    }
//-/    else
//-/    {
//-/        if(bUseStdNormalsUseSphere)
//-/        {
//-/            nObjNormalsKind = 2;
//-/        }
//-/        else
//-/        {
//-/            nObjNormalsKind = 1;
//-/        }
//-/    }
//-/    BOOL bObjNormalsInvert = bInvertNormals;
//-/    UINT16 nObjTextureProjX;
//-/    if(!bUseStdTextureX)
//-/    {
//-/        nObjTextureProjX = 0;
//-/    }
//-/    else
//-/    {
//-/        if(bUseStdTextureXUseSphere)
//-/        {
//-/            nObjTextureProjX = 2;
//-/        }
//-/        else
//-/        {
//-/            nObjTextureProjX = 1;
//-/        }
//-/    }
//-/    UINT16 nObjTextureProjY;
//-/    if(!bUseStdTextureY)
//-/    {
//-/        nObjTextureProjY = 0;
//-/    }
//-/    else
//-/    {
//-/        if(bUseStdTextureYUseSphere)
//-/        {
//-/            nObjTextureProjY = 2;
//-/        }
//-/        else
//-/        {
//-/            nObjTextureProjY = 1;
//-/        }
//-/    }
//-/    BOOL bObjShadow3D = bShadow3D;
//-/    const B3dMaterial& rMat = GetFrontMaterial();
//-/    Color aObjMaterialColor = ((const XFillColorItem&)(GetItem(XATTR_FILLCOLOR))).GetValue();
//-/    Color aObjMaterialEmission = rMat.GetMaterial(Base3DMaterialEmission);
//-/    Color aObjMaterialSpecular = rMat.GetMaterial(Base3DMaterialSpecular);
//-/    UINT16 nObjMaterialIntensity = rMat.GetShininess();
//-/    UINT16 nObjTextureKind;
//-/    if(eTextureKind == Base3DTextureColor)
//-/    {
//-/        nObjTextureKind = 1;
//-/    }
//-/    else
//-/    {
//-/        nObjTextureKind = 0;
//-/    }
//-/    UINT16 nObjTextureMode;
//-/    if(eTextureMode == Base3DTextureReplace)
//-/    {
//-/        nObjTextureMode = 0;
//-/    }
//-/    else if(eTextureMode == Base3DTextureModulate)
//-/    {
//-/        nObjTextureMode = 1;
//-/    }
//-/    else
//-/    {
//-/        nObjTextureMode = 2;
//-/    }
//-/    BOOL bObjTextureFilter = bFilterTexture;
//-/
//-/    // DoubleSided
//-/    rAttr.Put(SfxBoolItem(SDRATTR_3DOBJ_DOUBLE_SIDED, bObjDoubleSided));
//-/
//-/    // NormalsKind
//-/    rAttr.Put(SfxUInt16Item(SDRATTR_3DOBJ_NORMALS_KIND, nObjNormalsKind));
//-/
//-/    // NormalsInvert
//-/    rAttr.Put(SfxBoolItem(SDRATTR_3DOBJ_NORMALS_INVERT, bObjNormalsInvert));
//-/
//-/    // TextureProjectionX
//-/    rAttr.Put(SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_PROJ_X, nObjTextureProjX));
//-/
//-/    // TextureProjectionY
//-/    rAttr.Put(SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_PROJ_Y, nObjTextureProjY));
//-/
//-/    // Shadow3D
//-/    rAttr.Put(SfxBoolItem(SDRATTR_3DOBJ_SHADOW_3D, bObjShadow3D));
//-/
//-/    // Material
//-/    rAttr.Put(SvxColorItem(aObjMaterialColor, SDRATTR_3DOBJ_MAT_COLOR));
//-/    rAttr.Put(SvxColorItem(aObjMaterialEmission, SDRATTR_3DOBJ_MAT_EMISSION));
//-/    rAttr.Put(SvxColorItem(aObjMaterialSpecular, SDRATTR_3DOBJ_MAT_SPECULAR));
//-/    rAttr.Put(SfxUInt16Item(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY, nObjMaterialIntensity));
//-/
//-/    // TextureKind
//-/    rAttr.Put(SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_KIND, nObjTextureKind));
//-/
//-/    // TextureMode
//-/    rAttr.Put(SfxUInt16Item(SDRATTR_3DOBJ_TEXTURE_MODE, nObjTextureMode));
//-/
//-/    // TextureFilter
//-/    rAttr.Put(SfxBoolItem(SDRATTR_3DOBJ_TEXTURE_FILTER, bObjTextureFilter));
//-/}

//-/void E3dCompoundObject::TakeAttributes(SfxItemSet& rAttr, FASTBOOL bMerge, FASTBOOL bOnlyHardAttr) const
//-/{
//-/    // call parent
//-/    E3dObject::TakeAttributes(rAttr, bMerge, bOnlyHardAttr);
//-/
//-/    // special Attr for E3dCompoundObject
//-/    const SfxPoolItem* pPoolItem = NULL;
//-/    SfxItemState eState;
//-/
//-/    BOOL bObjDoubleSided = GetDoubleSided();
//-/    UINT16 nObjNormalsKind;
//-/    if(!GetUseStdNormals())
//-/    {
//-/        nObjNormalsKind = 0;
//-/    }
//-/    else
//-/    {
//-/        if(GetUseStdNormalsUseSphere())
//-/        {
//-/            nObjNormalsKind = 2;
//-/        }
//-/        else
//-/        {
//-/            nObjNormalsKind = 1;
//-/        }
//-/    }
//-/    BOOL bObjNormalsInvert = GetInvertNormals();
//-/    UINT16 nObjTextureProjX;
//-/    if(!GetUseStdTextureX())
//-/    {
//-/        nObjTextureProjX = 0;
//-/    }
//-/    else
//-/    {
//-/        if(GetUseStdTextureXUseSphere())
//-/        {
//-/            nObjTextureProjX = 2;
//-/        }
//-/        else
//-/        {
//-/            nObjTextureProjX = 1;
//-/        }
//-/    }
//-/    UINT16 nObjTextureProjY;
//-/    if(!GetUseStdTextureY())
//-/    {
//-/        nObjTextureProjY = 0;
//-/    }
//-/    else
//-/    {
//-/        if(GetUseStdTextureYUseSphere())
//-/        {
//-/            nObjTextureProjY = 2;
//-/        }
//-/        else
//-/        {
//-/            nObjTextureProjY = 1;
//-/        }
//-/    }
//-/    BOOL bObjShadow3D = GetDrawShadow3D();
//-///-/    const XFillAttrSetItem* pFillAttr = GetFillAttr();
//-/    const B3dMaterial& rMat = GetFrontMaterial();
//-///-/    Color aObjMaterialColor;
//-///-/    if(pFillAttr)
//-///-/    {
//-///-/        const SfxItemSet& rSet = pFillAttr->GetItemSet();
//-/    Color aObjMaterialColor = ((const XFillColorItem&) (GetItemSet().Get(XATTR_FILLCOLOR))).GetValue();
//-///-/    }
//-///-/    else
//-///-/    {
//-///-/        aObjMaterialColor = rMat.GetMaterial(Base3DMaterialDiffuse);
//-///-/    }
//-/    Color aObjMaterialEmission = rMat.GetMaterial(Base3DMaterialEmission);
//-/    Color aObjMaterialSpecular = rMat.GetMaterial(Base3DMaterialSpecular);
//-/    UINT16 nObjMaterialIntensity = rMat.GetShininess();
//-/    UINT16 nObjTextureKind;
//-/    if(GetTextureKind() == Base3DTextureColor)
//-/    {
//-/        nObjTextureKind = 1;
//-/    }
//-/    else
//-/    {
//-/        nObjTextureKind = 0;
//-/    }
//-/    UINT16 nObjTextureMode;
//-/    if(GetTextureMode() == Base3DTextureReplace)
//-/    {
//-/        nObjTextureMode = 0;
//-/    }
//-/    else if(GetTextureMode() == Base3DTextureModulate)
//-/    {
//-/        nObjTextureMode = 1;
//-/    }
//-/    else
//-/    {
//-/        nObjTextureMode = 2;
//-/    }
//-/    BOOL bObjTextureFilter = GetFilterTexture();
//-/
//-/    // DoubleSided
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_DOUBLE_SIDED, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bObjDoubleSided != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_DOUBLE_SIDED);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_DOUBLE_SIDED, bObjDoubleSided));
//-/        }
//-/    }
//-/
//-/    // NormalsKind
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_NORMALS_KIND, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(nObjNormalsKind != ((const SfxUInt16Item*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_NORMALS_KIND);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxUInt16Item(SID_ATTR_3D_NORMALS_KIND, nObjNormalsKind));
//-/        }
//-/    }
//-/
//-/    // NormalsInvert
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_NORMALS_INVERT, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bObjNormalsInvert != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_NORMALS_INVERT);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_NORMALS_INVERT, bObjNormalsInvert));
//-/        }
//-/    }
//-/
//-/    // TextureProjectionX
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_TEXTURE_PROJ_X, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(nObjTextureProjX != ((const SfxUInt16Item*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_TEXTURE_PROJ_X);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxUInt16Item(SID_ATTR_3D_TEXTURE_PROJ_X, nObjTextureProjX));
//-/        }
//-/    }
//-/
//-/    // TextureProjectionY
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_TEXTURE_PROJ_Y, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(nObjTextureProjY != ((const SfxUInt16Item*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_TEXTURE_PROJ_Y);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxUInt16Item(SID_ATTR_3D_TEXTURE_PROJ_Y, nObjTextureProjY));
//-/        }
//-/    }
//-/
//-/    // Shadow3D
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_SHADOW_3D, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bObjShadow3D != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_SHADOW_3D);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_SHADOW_3D, bObjShadow3D));
//-/        }
//-/    }
//-/
//-/    // Material
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_MAT_COLOR, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aObjMaterialColor != ((const SvxColorItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_MAT_COLOR);
//-/            rAttr.InvalidateItem(XATTR_FILLCOLOR);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxColorItem(aObjMaterialColor, SID_ATTR_3D_MAT_COLOR));
//-/        }
//-/        else
//-/        {
//-/            // kann nur invalidate sein, setze auch die farbe wieder so
//-/            rAttr.InvalidateItem(XATTR_FILLCOLOR);
//-/        }
//-/    }
//-/
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_MAT_EMISSION, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aObjMaterialEmission != ((const SvxColorItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_MAT_EMISSION);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxColorItem(aObjMaterialEmission, SID_ATTR_3D_MAT_EMISSION));
//-/        }
//-/    }
//-/
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_MAT_SPECULAR, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aObjMaterialSpecular != ((const SvxColorItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_MAT_SPECULAR);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxColorItem(aObjMaterialSpecular, SID_ATTR_3D_MAT_SPECULAR));
//-/        }
//-/    }
//-/
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_MAT_SPECULAR_INTENSITY, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(nObjMaterialIntensity != ((const SfxUInt16Item*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_MAT_SPECULAR_INTENSITY);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxUInt16Item(SID_ATTR_3D_MAT_SPECULAR_INTENSITY, nObjMaterialIntensity));
//-/        }
//-/    }
//-/
//-/    // TextureKind
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_TEXTURE_KIND, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(nObjTextureKind != ((const SfxUInt16Item*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_TEXTURE_KIND);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxUInt16Item(SID_ATTR_3D_TEXTURE_KIND, nObjTextureKind));
//-/        }
//-/    }
//-/
//-/    // TextureMode
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_TEXTURE_MODE, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(nObjTextureMode != ((const SfxUInt16Item*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_TEXTURE_MODE);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxUInt16Item(SID_ATTR_3D_TEXTURE_MODE, nObjTextureMode));
//-/        }
//-/    }
//-/
//-/    // TextureFilter
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_TEXTURE_FILTER, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bObjTextureFilter != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_TEXTURE_FILTER);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_TEXTURE_FILTER, bObjTextureFilter));
//-/        }
//-/    }
//-/}

//-/void E3dCompoundObject::ImpSetDoubleSided(BOOL bNew)
//-/{
//-/    if(bDoubleSided != bNew)
//-/    {
//-/        bDoubleSided = bNew;
//-/        bGeometryValid = FALSE;
//-/    }
//-/}

//-/void E3dCompoundObject::ImpSetUseStdNormals(BOOL bNew)
//-/{
//-/    if(bUseStdNormals != bNew)
//-/    {
//-/        bUseStdNormals = bNew;
//-/        bGeometryValid = FALSE;
//-/    }
//-/}

//-/void E3dCompoundObject::ImpSetUseStdNormalsUseSphere(BOOL bNew)
//-/{
//-/    if(bUseStdNormalsUseSphere != bNew)
//-/    {
//-/        bUseStdNormalsUseSphere = bNew;
//-/        bGeometryValid = FALSE;
//-/    }
//-/}

//-/void E3dCompoundObject::ImpSetInvertNormals(BOOL bNew)
//-/{
//-/    if(bInvertNormals != bNew)
//-/    {
//-/        bInvertNormals = bNew;
//-/        GetDisplayGeometry().InvertNormals();
//-/    }
//-/}

//-/void E3dCompoundObject::ImpSetUseStdTextureX(BOOL bNew)
//-/{
//-/    if(bUseStdTextureX != bNew)
//-/    {
//-/        bUseStdTextureX = bNew;
//-/        bGeometryValid = FALSE;
//-/    }
//-/}

//-/void E3dCompoundObject::ImpSetUseStdTextureXUseSphere(BOOL bNew)
//-/{
//-/    if(bUseStdTextureXUseSphere != bNew)
//-/    {
//-/        bUseStdTextureXUseSphere = bNew;
//-/        bGeometryValid = FALSE;
//-/    }
//-/}

//-/void E3dCompoundObject::ImpSetUseStdTextureY(BOOL bNew)
//-/{
//-/    if(bUseStdTextureY != bNew)
//-/    {
//-/        bUseStdTextureY = bNew;
//-/        bGeometryValid = FALSE;
//-/    }
//-/}

//-/void E3dCompoundObject::ImpSetUseStdTextureYUseSphere(BOOL bNew)
//-/{
//-/    if(bUseStdTextureYUseSphere != bNew)
//-/    {
//-/        bUseStdTextureYUseSphere = bNew;
//-/        bGeometryValid = FALSE;
//-/    }
//-/}

//-/void E3dCompoundObject::ImpSetTextureKind(Base3DTextureKind eNew)
//-/{
//-/    if(eTextureKind != eNew)
//-/    {
//-/        eTextureKind = eNew;
//-/    }
//-/}

//-/void E3dCompoundObject::ImpSetTextureMode(Base3DTextureMode eNew)
//-/{
//-/    if(eTextureMode != eNew)
//-/    {
//-/        eTextureMode = eNew;
//-/    }
//-/}

//-/void E3dCompoundObject::ImpSetFilterTexture(BOOL bNew)
//-/{
//-/    if(bFilterTexture != bNew)
//-/    {
//-/        bFilterTexture = bNew;
//-/    }
//-/}

// EOF
