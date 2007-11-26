/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: obj3d.cxx,v $
 *
 *  $Revision: 1.46 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 14:50:54 $
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


#include "svdstr.hrc"
#include "svdglob.hxx"

#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif

#ifndef _SVDATTR_HXX
#include <svx/svdattr.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif

#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _CAMERA3D_HXX
#include <svx/camera3d.hxx>
#endif

#ifndef _E3D_SCENE3D_HXX
#include <svx/scene3d.hxx>
#endif

#ifndef _E3D_POLYSC3D_HXX
#include <svx/polysc3d.hxx>
#endif

#ifndef _E3D_CUBE3D_HXX
#include <svx/cube3d.hxx>
#endif

#ifndef _E3D_LATHE3D_HXX
#include <svx/lathe3d.hxx>
#endif

#ifndef _E3D_SPHERE3D_HXX
#include <svx/sphere3d.hxx>
#endif

#ifndef _E3D_EXTRUD3D_HXX
#include <svx/extrud3d.hxx>
#endif

#ifndef _E3D_OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif

#ifndef _XOUTX_HXX
#include <svx/xoutx.hxx>
#endif

#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX
#include <svx/xflclit.hxx>
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
#include <svx/xlnclit.hxx>
#endif

#ifndef _SFXMETRICITEM_HXX
#include <svtools/metitem.hxx>
#endif

#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif

#ifndef _SVX_FILLITEM_HXX
#include <svx/xfillit.hxx>
#endif

#ifndef _SVX_XLNWTIT_HXX
#include <svx/xlnwtit.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif

#ifndef _B3D_B3DTRANS_HXX
#include <goodies/b3dtrans.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif

#ifndef _SVXE3DITEM_HXX
#include <svx/e3ditem.hxx>
#endif

#ifndef _SVX_XLNTRIT_HXX
#include <svx/xlntrit.hxx>
#endif

#ifndef _SVX_XFLTRIT_HXX
#include <svx/xfltrit.hxx>
#endif

#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif

#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
#endif

#ifndef _SVX3DITEMS_HXX
#include <svx/svx3ditems.hxx>
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif

#ifndef _SVX_XGRSCIT_HXX
#include <svx/xgrscit.hxx>
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
#include <svx/xlndsit.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolygontools.hxx>
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
    // Bewegung in X,Y im Augkoordinatensystem
    E3dScene* pScene = GetScene();

    if(pScene)
    {
        // pos ermitteln
        B3dTransformationSet& rTransSet = pScene->GetCameraSet();
        basegfx::B3DPoint aScaleCenter((double)rRef.X(), (double)rRef.Y(), 32768.0);
        aScaleCenter = rTransSet.ViewToEyeCoor(aScaleCenter);

        // scale-faktoren holen
        double fScaleX(xFact);
        double fScaleY(yFact);

        // build transform
        basegfx::B3DHomMatrix mFullTransform(GetFullTransform());
        basegfx::B3DHomMatrix mTrans(mFullTransform);

        mTrans *= rTransSet.GetOrientation();
        mTrans.translate(-aScaleCenter.getX(), -aScaleCenter.getY(), -aScaleCenter.getZ());
        mTrans.scale(fScaleX, fScaleY, 1.0);
        mTrans.translate(aScaleCenter.getX(), aScaleCenter.getY(), aScaleCenter.getZ());
        mTrans *= rTransSet.GetInvOrientation();
        mFullTransform.invert();
        mTrans *= mFullTransform;

        // anwenden
        basegfx::B3DHomMatrix mObjTrans(GetTransform());
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
        basegfx::B3DHomMatrix mInvDispTransform;
        if(GetParentObj())
        {
            mInvDispTransform = GetParentObj()->GetFullTransform();
            mInvDispTransform.invert();
        }

        // BoundVolume von Weltkoordinaten in Eye-Koordinaten
        B3dTransformationSet& rTransSet = pScene->GetCameraSet();
        const Volume3D& rVol = pScene->GetBoundVolume();
        Volume3D aEyeVol = rVol.GetTransformVolume(rTransSet.GetOrientation());

        // relativen Bewegungsvektor in Augkoordinaten bilden
        basegfx::B3DPoint aMove(
            (double)rSize.Width() * aEyeVol.getWidth() / (double)aRect.GetWidth(),
            (double)-rSize.Height() * aEyeVol.getHeight() / (double)aRect.GetHeight(),
            0.0);

        // Bewegungsvektor in lokale Koordinaten des Parents des Objektes
        basegfx::B3DPoint aPos;
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

sal_uInt32 E3dObject::GetHdlCount() const
{
    // 8 Eckpunkte + 1 E3dVolumeMarker (= Wireframe-Darstellung)
    return 9L;
}

/*************************************************************************
|*
|* Handle-Liste fuellen
|*
\************************************************************************/

void E3dObject::AddToHdlList(SdrHdlList& rHdlList) const
{
    const basegfx::B2DPolyPolygon aPolyPoly(ImpCreateWireframePoly());
    const sal_uInt32 nPolyCount(aPolyPoly.count());

    for(sal_uInt32 a(0L); a < nPolyCount; a += 3L)
    {
        const basegfx::B2DPolygon aPoly(aPolyPoly.getB2DPolygon(a));
        const basegfx::B2DPoint aPointA(aPoly.getB2DPoint(0L));
        const basegfx::B2DPoint aPointB(aPoly.getB2DPoint(1L));
        rHdlList.AddHdl(new SdrHdl(Point(FRound(aPointA.getX()), FRound(aPointA.getY())), HDL_BWGT));
        rHdlList.AddHdl(new SdrHdl(Point(FRound(aPointB.getX()), FRound(aPointB.getY())), HDL_BWGT));
    }

    if(nPolyCount)
    {
        E3dVolumeMarker* pVolMarker = new E3dVolumeMarker(aPolyPoly);
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

basegfx::B2DPolyPolygon E3dObject::ImpTakeContour3D() const
{
    basegfx::B2DPolyPolygon aRetval;

    if(pSub && pSub->GetObjCount())
    {
        for (ULONG i = 0; i < pSub->GetObjCount(); i++)
        {
            // #i74258# call ImpTakeContour3D at child objects (!)
            SdrObject* pObj = pSub->GetObj(i);
            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In E3dObject sind nur 3D-Objekte erlaubt!");
            aRetval.append(((E3dObject*)pObj)->ImpTakeContour3D());
        }
    }

    return aRetval;
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
            const basegfx::B3DHomMatrix& rTf  = p3DObj->GetTransform();
            aBoundVol.expand(rVol.GetTransformVolume(rTf));
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
                double fExpand(nLineWidth / 2.0);
                aBoundVol.grow(fExpand);
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

const Volume3D& E3dObject::GetBoundVolume() const
{
    if ( !bBoundVolValid )
    {
        ((E3dObject*)this)->RecalcBoundVolume();
    }

    return aBoundVol;
}

/*************************************************************************
|*
|* Mittelpunkt liefern
|*
\************************************************************************/

basegfx::B3DPoint E3dObject::GetCenter()
{
    return GetBoundVolume().getCenter();
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
|* hierarchische Transformation ueber alle Parents bestimmen, in
|* aFullTfMatrix ablegen und diese zurueckgeben
|*
\************************************************************************/

const basegfx::B3DHomMatrix& E3dObject::GetFullTransform() const
{
    if(bTfHasChanged)
    {
        E3dObject* pThis = (E3dObject*)this;
        pThis->aFullTfMatrix = aTfMatrix;
        if ( GetParentObj() )
            pThis->aFullTfMatrix *= GetParentObj()->GetFullTransform();
        pThis->bTfHasChanged = FALSE;
    }

    return aFullTfMatrix;
}

/*************************************************************************
|*
|* Transformationsmatrix abfragen
|*
\************************************************************************/

const basegfx::B3DHomMatrix& E3dObject::GetTransform() const
{
    return aTfMatrix;
}

/*************************************************************************
|*
|* Transformationsmatrix setzen
|*
\************************************************************************/

void E3dObject::NbcSetTransform(const basegfx::B3DHomMatrix& rMatrix)
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
    aTfMatrix.identity();
    SetTransformChanged();
    StructureChanged(this);
}

/*************************************************************************
|*
|* Transformationsmatrix setzen mit Repaint-Broadcast
|*
\************************************************************************/

void E3dObject::SetTransform(const basegfx::B3DHomMatrix& rMatrix)
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

void E3dObject::NbcTranslate(const basegfx::B3DVector& rTrans)
{
    aTfMatrix.translate(rTrans.getX(), rTrans.getY(), rTrans.getZ());
    SetTransformChanged();
    StructureChanged(this);
}
/*************************************************************************
|*
|* Translation mit Repaint-Broadcast
|*
\************************************************************************/

void E3dObject::Translate(const basegfx::B3DVector& rTrans)
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
    aTfMatrix.scale(fSx, 1.0, 1.0);
    SetTransformChanged();
    StructureChanged(this);
}

/************************************************************************/

void E3dObject::NbcScaleY(double fSy)
{
    aTfMatrix.scale(1.0, fSy, 1.0);
    SetTransformChanged();
    StructureChanged(this);
}

/************************************************************************/

void E3dObject::NbcScaleZ(double fSz)
{
    aTfMatrix.scale(1.0, 1.0, fSz);
    SetTransformChanged();
    StructureChanged(this);
}

/************************************************************************/

void E3dObject::NbcScale(double fSx, double fSy, double fSz)
{
    aTfMatrix.scale(fSx, fSy, fSz);
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
    aTfMatrix.scale(fS, fS, fS);
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
    aTfMatrix.rotate(fAng, 0.0, 0.0);
    SetTransformChanged();
    StructureChanged(this);
}

/************************************************************************/

void E3dObject::NbcRotateY(double fAng)
{
    aTfMatrix.rotate(0.0, fAng, 0.0);
    SetTransformChanged();
    StructureChanged(this);
}

/************************************************************************/

void E3dObject::NbcRotateZ(double fAng)
{
    aTfMatrix.rotate(0.0, 0.0, fAng);
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
|* Linien fuer die Wireframe-Darstellung des Objekts dem uebergebenen
|* basegfx::B3DPolygon hinzufuegen. Als default wird das BoundVolume verwendet.
|*
\************************************************************************/

void E3dObject::CreateWireframe(basegfx::B3DPolygon& rWirePoly, const basegfx::B3DHomMatrix* pTf) const
{
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
|* Wireframe-PolyPolygon erzeugen
|*
\************************************************************************/

basegfx::B2DPolyPolygon E3dObject::ImpCreateWireframePoly() const
{
    basegfx::B2DPolyPolygon aRetval;
    E3dScene* pScene = GetScene();

    if(pScene)
    {
        basegfx::B3DPolygon aPoly3D;
        CreateWireframe(aPoly3D, 0L);
        const sal_uInt32 nPntCnt(aPoly3D.count());

        if(nPntCnt)
        {
            const Volume3D aVolume(pScene->FitInSnapRect());
            pScene->GetCameraSet().SetDeviceVolume(aVolume, sal_False);
            pScene->GetCameraSet().SetObjectTrans(GetFullTransform());

            for(sal_uInt32 a(0L); a < nPntCnt;)
            {
                basegfx::B3DPoint aPointA(aPoly3D.getB3DPoint(a++));
                aPointA = pScene->GetCameraSet().ObjectToViewCoor(aPointA);
                basegfx::B3DPoint aPointB(aPoly3D.getB3DPoint(a++));
                aPointB = pScene->GetCameraSet().ObjectToViewCoor(aPointB);
                basegfx::B2DPolygon aTmpPoly;
                aTmpPoly.append(basegfx::B2DPoint(aPointA.getX(), aPointA.getY()));
                aTmpPoly.append(basegfx::B2DPoint(aPointB.getX(), aPointB.getY()));
                aRetval.append(aTmpPoly);
            }
        }
    }

    return aRetval;
}

/*************************************************************************
|*
|* Drag-Polygon zurueckgeben
|*
\************************************************************************/

basegfx::B2DPolyPolygon E3dObject::TakeXorPoly(sal_Bool /*bDetail*/) const
{
    return ImpCreateWireframePoly();
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
                        SdrObject::Free( pObj );
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

    // Da sich der Parent geaendert haben kann, Gesamttransformation beim
    // naechsten Mal auf jeden Fall neu bestimmen
    SetTransformChanged();

    // Selektionsstatus kopieren
    bIsSelected = r3DObj.bIsSelected;
}

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

    bGeometryValid = FALSE;
    bFullTfIsPositive = TRUE;
}

E3dCompoundObject::E3dCompoundObject(E3dDefaultAttributes& rDefault) : E3dObject()
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

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

        if(!rBoundVol.isEmpty())
        {
            const basegfx::B3DHomMatrix& rTrans = GetFullTransform();
            Vol3DPointIterator aIter(rBoundVol, &rTrans);
            basegfx::B3DPoint aTfVec;
            while ( aIter.Next(aTfVec) )
            {
                aTfVec = pScene->GetCameraSet().WorldToViewCoor(aTfVec);
                Point aPoint((long)(aTfVec.getX() + 0.5), (long)(aTfVec.getY() + 0.5));
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
            basegfx::B3DHomMatrix mTransform = GetFullTransform();
            pScene->GetCameraSet().SetObjectTrans(mTransform);

            // Schattenpolygon holen
            basegfx::B2DPolyPolygon aShadowPoly2D(ImpGetShadowPolygon());

            // invert Y coor cause of GetPolyPolygon() later
            basegfx::B2DHomMatrix aTransMat;
            aTransMat.scale(1.0, -1.0);
            aShadowPoly2D.transform(aTransMat);

            PolyPolygon aShadowPoly(aShadowPoly2D);

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

const Volume3D& E3dCompoundObject::GetBoundVolume() const
{
    // Geometrie aktuell?
    if(!bGeometryValid)
    {
        // Neu erzeugen und eine Neubestimmung des BoundVol erzwingen
        E3dCompoundObject* pThis = (E3dCompoundObject*)this;
        pThis->ReCreateGeometry();
        pThis->bBoundVolValid = FALSE;
    }

    // call parent
    return E3dObject::GetBoundVolume();
}

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

basegfx::B3DPolyPolygon E3dCompoundObject::Get3DLineGeometry() const
{
    basegfx::B3DPolyPolygon aRetval;
    const B3dEntityBucket& rEntityBucket = GetDisplayGeometry().GetEntityBucket();
    const GeometryIndexValueBucket& rIndexBucket = GetDisplayGeometry().GetIndexBucket();
    sal_uInt32 nPolyCounter(0L);
    sal_uInt32 nEntityCounter(0L);

    while(nPolyCounter < rIndexBucket.Count())
    {
        // next primitive
        sal_uInt32 nUpperBound(rIndexBucket[nPolyCounter++].GetIndex());
        basegfx::B3DPoint aLastPoint;
        sal_Bool bLastLineVisible(rEntityBucket[nUpperBound - 1].IsEdgeVisible());

        if(bLastLineVisible)
        {
            aLastPoint = rEntityBucket[nUpperBound - 1].Point();
        }

        while(nEntityCounter < nUpperBound)
        {
            basegfx::B3DPoint aNewPoint(rEntityBucket[nEntityCounter].Point());

            if(bLastLineVisible)
            {
                if(aLastPoint != aNewPoint)
                {
                    // fill polygon
                    basegfx::B3DPolygon aNewPoly;
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
            aDisplayGeometry.CreateDefaultNormalsSphere();
        if(GetNormalsInvert())
            aDisplayGeometry.InvertNormals();
    }

    if(bCreateTexture)
    {
        aDisplayGeometry.CreateDefaultTexture(
            ((GetTextureProjectionX() > 0) ? B3D_CREATE_DEFAULT_X : FALSE)
            |((GetTextureProjectionY() > 0) ? B3D_CREATE_DEFAULT_Y : FALSE),
            GetTextureProjectionX() > 1);
    }

    // Das Ende der Geometrieerzeugung anzeigen
    aDisplayGeometry.EndDescription();
}

void E3dCompoundObject::ReCreateGeometry()
{
    // Geometrie zerstoeren
    DestroyGeometry();

    // ... und neu erzeugen
    CreateGeometry();
}

/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/

void E3dCompoundObject::AddGeometry(
    const basegfx::B3DPolyPolygon& rPolyPolygon,
    BOOL bHintIsComplex, BOOL bOutline)
{
    if(rPolyPolygon.count())
    {
        // neue Geometrie erzeugen
        for(sal_uInt32 a(0L); a < rPolyPolygon.count(); a++)
        {
            const basegfx::B3DPolygon aPoly3D(rPolyPolygon.getB3DPolygon(a));
            aDisplayGeometry.StartObject(bHintIsComplex, bOutline);

            for(sal_uInt32 b(0L); b < aPoly3D.count(); b++ )
            {
                aDisplayGeometry.AddEdge(aPoly3D.getB3DPoint(b));
            }
        }
        aDisplayGeometry.EndObject();

        // LocalBoundVolume pflegen
        aLocalBoundVol.expand(basegfx::tools::getRange(rPolyPolygon));

        // Eigenes BoundVol nicht mehr gueltig
        SetBoundVolInvalid();
        SetRectsDirty();
    }
}

void E3dCompoundObject::AddGeometry(
    const basegfx::B3DPolyPolygon& rPolyPolygon,
    const basegfx::B3DPolyPolygon& rPolyPolygonNormal,
    BOOL bHintIsComplex, BOOL bOutline)
{
    if(rPolyPolygon.count())
    {
        // neue Geometrie erzeugen
        for(sal_uInt32 a(0L); a < rPolyPolygon.count(); a++ )
        {
            const basegfx::B3DPolygon aPoly3D(rPolyPolygon.getB3DPolygon(a));
            const basegfx::B3DPolygon aNormal3D(rPolyPolygonNormal.getB3DPolygon(a));
            aDisplayGeometry.StartObject(bHintIsComplex, bOutline);

            for(sal_uInt32 b(0L); b < aPoly3D.count(); b++ )
            {
                aDisplayGeometry.AddEdge(aPoly3D.getB3DPoint(b), aNormal3D.getB3DPoint(b));
            }
        }
        aDisplayGeometry.EndObject();

        // LocalBoundVolume pflegen
        aLocalBoundVol.expand(basegfx::tools::getRange(rPolyPolygon));

        // Eigenes BoundVol nicht mehr gueltig
        SetBoundVolInvalid();
        SetRectsDirty();
    }
}

void E3dCompoundObject::AddGeometry(
    const basegfx::B3DPolyPolygon& rPolyPolygon,
    const basegfx::B3DPolyPolygon& rPolyPolygonNormal,
    const basegfx::B2DPolyPolygon& rPolyPolygonTexture,
    BOOL bHintIsComplex, BOOL bOutline)
{
    if(rPolyPolygon.count())
    {
        // neue Geometrie erzeugen
        for(sal_uInt32 a(0L); a < rPolyPolygon.count(); a++ )
        {
            const basegfx::B3DPolygon aPoly3D(rPolyPolygon.getB3DPolygon(a));
            const basegfx::B3DPolygon aNormal3D(rPolyPolygonNormal.getB3DPolygon(a));
            const basegfx::B2DPolygon aTexture2D(rPolyPolygonTexture.getB2DPolygon(a));
            aDisplayGeometry.StartObject(bHintIsComplex, bOutline);

            for(sal_uInt32 b(0L); b < aPoly3D.count(); b++ )
            {
                aDisplayGeometry.AddEdge(aPoly3D.getB3DPoint(b), aNormal3D.getB3DPoint(b), aTexture2D.getB2DPoint(b));
            }
        }
        aDisplayGeometry.EndObject();

        // LocalBoundVolume pflegen
        aLocalBoundVol.expand(basegfx::tools::getRange(rPolyPolygon));

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

basegfx::B3DPolyPolygon E3dCompoundObject::ImpGrowPoly(
    const basegfx::B3DPolyPolygon& rPolyPolyGrow,
    const basegfx::B3DPolyPolygon& rPolyPolyNormals,
    double fFactor)
{
    basegfx::B3DPolyPolygon aRetval;
    const sal_uInt32 nPolyCount(rPolyPolyGrow.count());
    const bool bClosed(rPolyPolyGrow.isClosed());

    for(sal_uInt32 a(0L); a < nPolyCount; a++)
    {
        const basegfx::B3DPolygon aPolyGrow(rPolyPolyGrow.getB3DPolygon(a));
        const basegfx::B3DPolygon aPolyNormals(rPolyPolyNormals.getB3DPolygon(a));
        const sal_uInt32 nPointCount(aPolyGrow.count());
        basegfx::B3DPolygon aNewPolyGrow;

        for(sal_uInt32 b(0L); b < nPointCount; b++)
        {
            aNewPolyGrow.append(aPolyGrow.getB3DPoint(b) + (aPolyNormals.getB3DPoint(b) * fFactor));
        }

        aNewPolyGrow.setClosed(bClosed);
        aRetval.append(aNewPolyGrow);
    }

    return aRetval;
}

basegfx::B2VectorOrientation E3dCompoundObject::ImpGetOrientationInPoint(
    const basegfx::B3DPolygon& rPolygon,
    sal_uInt32 nIndex)
{
    sal_uInt32 nPntCnt(rPolygon.count());
    basegfx::B2VectorOrientation eRetval(basegfx::ORIENTATION_NEUTRAL);

    if(nIndex < nPntCnt)
    {
        const basegfx::B3DPoint aMid(rPolygon.getB3DPoint(nIndex));
        const basegfx::B3DPoint aPre(rPolygon.getB3DPoint((nIndex == 0L) ? nPntCnt - 1L : nIndex - 1L));
        const basegfx::B3DPoint aPos(rPolygon.getB3DPoint((nIndex == nPntCnt - 1L) ? 0L : nIndex + 1L));
        const basegfx::B3DVector aVecA(aPre - aMid);
        const basegfx::B3DVector aVecB(aPos - aMid);
        const basegfx::B3DVector aNormal(aVecA.getPerpendicular(aVecB));

        if(aNormal.getZ() > 0.0)
        {
            eRetval = basegfx::ORIENTATION_POSITIVE;
        }
        else if(aNormal.getZ() < 0.0)
        {
            eRetval = basegfx::ORIENTATION_NEGATIVE;
        }
    }

    return eRetval;
}

basegfx::B3DPolyPolygon E3dCompoundObject::ImpCorrectGrownPoly(
    const basegfx::B3DPolyPolygon& aToBeCorrected,
    const basegfx::B3DPolyPolygon& aOriginal)
{
    const sal_uInt32 aOriginalCount(aOriginal.count());
    const sal_uInt32 aToBeCorrectedCount(aToBeCorrected.count());
    const bool bClosed(aToBeCorrected.isClosed());

    if(aOriginalCount == aToBeCorrectedCount)
    {
        basegfx::B3DPolyPolygon aRetval;

        for(sal_uInt32 a(0L); a < aToBeCorrectedCount; a++)
        {
            const basegfx::B3DPolygon aCorr(aToBeCorrected.getB3DPolygon(a));
            const basegfx::B3DPolygon aOrig(aOriginal.getB3DPolygon(a));
            const sal_uInt32 aOrigCount(aOrig.count());
            const sal_uInt32 aCorrCount(aCorr.count());

            if(aOrigCount == aCorrCount && aOrigCount > 2L)
            {
                sal_uInt32 nNumDiff(0L);
                sal_uInt32 nDoneStart(0xffffffff);

                // Testen auf Anzahl Aenderungen
                sal_uInt32 b;

                for(b = 0L; b < aOrigCount; b++)
                {
                    const basegfx::B2VectorOrientation eOrig(ImpGetOrientationInPoint(aOrig, b));
                    const basegfx::B2VectorOrientation eCorr(ImpGetOrientationInPoint(aCorr, b));

                    if(eOrig != eCorr)
                    {
                        nNumDiff++;
                    }
                    else
                    {
                        if(nDoneStart == 0xffffffff)
                        {
                            // eventuellen Startpunkt auf gleiche Orientierung legen
                            nDoneStart = b;
                        }
                    }
                }

                if(nNumDiff == aOrigCount)
                {
                    // Komplett umgedreht, alles auf einen Punkt
                    const basegfx::B3DRange aCorrRange = basegfx::tools::getRange(aCorr);
                    basegfx::B3DPolygon aNew;

                    aNew.append(aCorrRange.getCenter(), aCorrCount);
                    aRetval.append(aNew);
                }
                else if(nNumDiff)
                {
                    // es gibt welche, nDoneStart ist gesetzt. Erzeuge (und
                    // setze) nDoneEnd
                    sal_uInt32 nDoneEnd(nDoneStart);
                    sal_uInt32 nStartLoop(0L);
                    BOOL bInLoop(FALSE);
                    basegfx::B3DPolygon aNew(aCorr);

                    // einen step mehr in der Schleife, um Loops abzuschliessen
                    BOOL bFirstStep(TRUE);

                    while(nDoneEnd != nDoneStart || bFirstStep)
                    {
                        bFirstStep = FALSE;

                        // nCandidate ist Kandidat fuer Test
                        const sal_uInt32 nCandidate((nDoneEnd == aOrigCount - 1L) ? 0L : nDoneEnd + 1L);
                        const basegfx::B2VectorOrientation eOrig(ImpGetOrientationInPoint(aOrig, nCandidate));
                        const basegfx::B2VectorOrientation eCorr(ImpGetOrientationInPoint(aCorr, nCandidate));

                        if(eOrig == eCorr)
                        {
                            // Orientierung ist gleich
                            if(bInLoop)
                            {
                                // Punkte innerhalb bInLoop auf ihr Zentrum setzen
                                basegfx::B3DPoint aMiddle;
                                sal_uInt32 nCounter(0L);
                                sal_uInt32 nStart(nStartLoop);

                                while(nStart != nCandidate)
                                {
                                    aMiddle += aCorr.getB3DPoint(nStart);
                                    nCounter++;
                                    nStart = (nStart == aOrigCount - 1L) ? 0L : nStart + 1L;
                                }

                                // Mittelwert bilden
                                aMiddle /= (double)nCounter;

                                // Punkte umsetzen
                                nStart = nStartLoop;
                                while(nStart != nCandidate)
                                {
                                    aNew.setB3DPoint(nStart, aMiddle);
                                    nStart = (nStart == aOrigCount - 1L) ? 0L : nStart + 1L;
                                }

                                // Loop beenden
                                bInLoop = FALSE;
                            }
                        }
                        else
                        {
                            // Orientierung unterschiedlich
                            if(!bInLoop)
                            {
                                // Start eines Loop mit geaenderter Orientierung
                                nStartLoop = nCandidate;
                                bInLoop = TRUE;
                            }
                        }

                        // Weitergehen
                        nDoneEnd = nCandidate;
                    }

                    aRetval.append(aNew);
                }
                else
                {
                    // no change, append original
                    aRetval.append(aCorr);
                }
            }
            else
            {
                // less than 2 -> no change, append original
                aRetval.append(aCorr);
            }
        }

        aRetval.setClosed(bClosed);
        return aRetval;
    }
    else
    {
        return aToBeCorrected;
    }
}

basegfx::B3DPolyPolygon E3dCompoundObject::ImpScalePoly(
    const basegfx::B3DPolyPolygon& rPolyPolyScale,
    double fFactor)
{
    basegfx::B3DPolyPolygon aRetval(rPolyPolyScale);
    const basegfx::B3DRange aPolyPolyRange(basegfx::tools::getRange(rPolyPolyScale));
    const basegfx::B3DPoint aCenter(aPolyPolyRange.getCenter());
    basegfx::B3DHomMatrix aTransform;

    aTransform.translate(-aCenter.getX(), -aCenter.getY(), -aCenter.getZ());
    aTransform.scale(fFactor, fFactor, fFactor);
    aTransform.translate(aCenter.getX(), aCenter.getY(), aCenter.getZ());
    aRetval.transform(aTransform);

    return aRetval;
}

void E3dCompoundObject::ImpCreateFront(
    const basegfx::B3DPolyPolygon& rPolyPoly3D,
    const basegfx::B3DPolyPolygon& rFrontNormals,
    BOOL bDoCreateNormals,
    BOOL bDoCreateTexture)
{
    // Vorderseite
    if(bDoCreateNormals)
    {
        if(bDoCreateTexture)
        {
            // create default texture polygon
            const basegfx::B3DRange aRange(basegfx::tools::getRange(rPolyPoly3D));
            const double fScaleX(0.0 == aRange.getWidth() ? 1.0 : 1.0 / aRange.getWidth());
            const double fScaleY(0.0 == aRange.getHeight() ? 1.0 : 1.0 / aRange.getHeight());
            const double fScaleZ(0.0 == aRange.getDepth() ? 1.0 : 1.0 / aRange.getDepth());
            basegfx::B3DHomMatrix aTrans3DTo2D;

            aTrans3DTo2D.translate(-aRange.getMinX(), -aRange.getMinY(), -aRange.getMinZ());
            aTrans3DTo2D.scale(fScaleX, -fScaleY, fScaleZ);
            aTrans3DTo2D.translate(0.0, 1.0, 0.0);
            basegfx::B2DPolyPolygon aPolyTexture(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(rPolyPoly3D, aTrans3DTo2D));

            AddGeometry(rPolyPoly3D, rFrontNormals, aPolyTexture, TRUE);
        }
        else
        {
            AddGeometry(rPolyPoly3D, rFrontNormals, TRUE);
        }
    }
    else
    {
        AddGeometry(rPolyPoly3D, TRUE);
    }
}

void E3dCompoundObject::ImpCreateBack(
    const basegfx::B3DPolyPolygon& rPolyPoly3D,
    const basegfx::B3DPolyPolygon& rBackNormals,
    BOOL bDoCreateNormals,
    BOOL bDoCreateTexture)
{
    // flip polys
    basegfx::B3DPolyPolygon aPolyPoly3D(rPolyPoly3D);
    basegfx::B3DPolyPolygon aBackNormals(rBackNormals);
    aPolyPoly3D.flip();
    aBackNormals.flip();

    // use ImpCreateFront with flipped polys
    ImpCreateFront(aPolyPoly3D, aBackNormals, bDoCreateNormals, bDoCreateTexture);
}

basegfx::B3DPolyPolygon E3dCompoundObject::ImpCreateByPattern(const basegfx::B3DPolyPolygon& rPattern)
{
    basegfx::B3DPolyPolygon aRetval;
    const sal_uInt32 nPolyCount(rPattern.count());
    const bool bClosed(rPattern.isClosed());

    for(sal_uInt32 a(0L); a < nPolyCount; a++)
    {
        basegfx::B3DPolygon aNew;
        aNew.append(basegfx::B3DPoint(), rPattern.getB3DPolygon(a).count());
        aNew.setClosed(bClosed);
        aRetval.append(aNew);
    }

    return aRetval;
}

basegfx::B3DPolyPolygon E3dCompoundObject::ImpAddFrontNormals(
    const basegfx::B3DPolyPolygon& rNormalsFront,
    const basegfx::B3DPoint& rOffset)
{
    basegfx::B3DPoint aBackOffset(-rOffset);
    return ImpAddBackNormals(rNormalsFront, aBackOffset);
}

basegfx::B3DPolyPolygon E3dCompoundObject::ImpAddBackNormals(
    const basegfx::B3DPolyPolygon& rNormalsBack,
    const basegfx::B3DPoint& rOffset)
{
    basegfx::B3DPolyPolygon aRetval;
    basegfx::B3DVector aOffset(rOffset);
    aOffset.normalize();
    basegfx::B3DPoint aValue(aOffset);
    const sal_uInt32 nPolyCount(rNormalsBack.count());
    const bool bClosed(rNormalsBack.isClosed());

    for(sal_uInt32 a(0L); a < nPolyCount; a++)
    {
        const basegfx::B3DPolygon aPoly(rNormalsBack.getB3DPolygon(a));
        const sal_uInt32 nPointCount(aPoly.count());
        basegfx::B3DPolygon aNew;

        for(sal_uInt32 b(0L); b < nPointCount; b++)
        {
            aNew.append(aPoly.getB3DPoint(b) + aValue);
        }

        aNew.setClosed(bClosed);
        aRetval.append(aNew);
    }

    return aRetval;
}

basegfx::B3DPolyPolygon E3dCompoundObject::ImpAddInBetweenNormals(
    const basegfx::B3DPolyPolygon& rPolyPolyFront,
    const basegfx::B3DPolyPolygon& rPolyPolyBack,
    const basegfx::B3DPolyPolygon& rPolyPolyNormals,
    BOOL bSmoothed)
{
    basegfx::B3DPolyPolygon aRetval;
    const sal_uInt32 nPolyCnt(rPolyPolyFront.count());
    const bool bClosed(rPolyPolyFront.isClosed());

    // Verbindungsstuecke
    for(sal_uInt32 a(0L); a < nPolyCnt; a++)
    {
        const basegfx::B3DPolygon aPoly3DFront(rPolyPolyFront.getB3DPolygon(a));
        const basegfx::B3DPolygon aPoly3DBack(rPolyPolyBack.getB3DPolygon(a));
        const basegfx::B3DPolygon aNormalPoly(rPolyPolyNormals.getB3DPolygon(a));
        const sal_uInt32 nPntCnt(aPoly3DFront.count());

        if(aPoly3DBack.isClosed())
        {
            const basegfx::B3DVector aVecA(aPoly3DBack.getB3DPoint(nPntCnt - 1L) - aPoly3DFront.getB3DPoint(nPntCnt - 1L));
            const basegfx::B3DVector aVecB(aPoly3DFront.getB3DPoint(0L) - aPoly3DFront.getB3DPoint(nPntCnt - 1L));
            basegfx::B3DVector aNormal(aVecA.getPerpendicular(aVecB));
            aNormal.normalize();
            basegfx::B3DPolygon aNewPoly;

            for(sal_uInt32 i(0L); i < nPntCnt; i++)
            {
                const basegfx::B3DVector aVecC(aPoly3DBack.getB3DPoint(i) - aPoly3DFront.getB3DPoint(i));
                const basegfx::B3DVector aVecD(aPoly3DFront.getB3DPoint((i + 1L == nPntCnt) ? 0L : i + 1L) - aPoly3DFront.getB3DPoint(i));
                basegfx::B3DVector aNextNormal(aVecC.getPerpendicular(aVecD));
                aNextNormal.normalize();
                basegfx::B3DVector aNew;

                if(bSmoothed)
                {
                    basegfx::B3DVector aMidNormal(aNormal + aNextNormal);
                    aMidNormal.normalize();
                    aNew = aNormalPoly.getB3DPoint(i) + aMidNormal;
                }
                else
                {
                    aNew = aNormalPoly.getB3DPoint(i) + aNormal;
                }

                aNew.normalize();
                aNewPoly.append(aNew);
                aNormal = aNextNormal;
            }

            aNewPoly.setClosed(bClosed);
            aRetval.append(aNewPoly);
        }
        else
        {
            basegfx::B3DVector aNormal;

            if(aPoly3DBack.getB3DPoint(0L) == aPoly3DFront.getB3DPoint(0L))
            {
                const basegfx::B3DVector aVecA(aPoly3DBack.getB3DPoint(1L) - aPoly3DFront.getB3DPoint(1L));
                const basegfx::B3DVector aVecB(aPoly3DFront.getB3DPoint(1L) - aPoly3DFront.getB3DPoint(0L));
                aNormal = aVecA.getPerpendicular(aVecB);
            }
            else
            {
                const basegfx::B3DVector aVecA(aPoly3DBack.getB3DPoint(0L) - aPoly3DFront.getB3DPoint(0L));
                const basegfx::B3DVector aVecB(aPoly3DFront.getB3DPoint(1L) - aPoly3DFront.getB3DPoint(0L));
                aNormal = aVecA.getPerpendicular(aVecB);
            }

            aNormal.normalize();
            basegfx::B3DVector aNew(aNormalPoly.getB3DPoint(0L) + aNormal);
            aNew.normalize();
            basegfx::B3DPolygon aNewPoly;
            aNewPoly.append(aNew);

            for(sal_uInt32 i(1L); i < nPntCnt; i++)
            {
                basegfx::B3DVector aNextNormal;

                if(i + 1L == nPntCnt)
                {
                    aNextNormal = aNormal;
                }
                else
                {
                    const basegfx::B3DVector aVecA(aPoly3DBack.getB3DPoint(i) - aPoly3DFront.getB3DPoint(i));
                    const basegfx::B3DVector aVecB(aPoly3DFront.getB3DPoint(i + 1L) - aPoly3DFront.getB3DPoint(i));
                    aNextNormal = aVecA.getPerpendicular(aVecB);
                }

                aNextNormal.normalize();

                if(bSmoothed)
                {
                    basegfx::B3DVector aMidNormal(aNormal + aNextNormal);
                    aMidNormal.normalize();
                    aNew = aNormalPoly.getB3DPoint(i) + aMidNormal;
                }
                else
                {
                    aNew = aNormalPoly.getB3DPoint(i) + aNormal;
                }

                aNew.normalize();
                aNewPoly.append(aNew);
                aNormal = aNextNormal;
            }

            aNewPoly.setClosed(bClosed);
            aRetval.append(aNewPoly);
        }
    }

    return aRetval;
}

void E3dCompoundObject::ImpCreateInBetween(
    const basegfx::B3DPolyPolygon& rPolyPolyFront,
    const basegfx::B3DPolyPolygon& rPolyPolyBack,
    const basegfx::B3DPolyPolygon& rFrontNormals,
    const basegfx::B3DPolyPolygon& rBackNormals,
    BOOL bDoCreateNormals,
    double fSurroundFactor,
    double fTextureStart,
    double fTextureDepth,
    BOOL bRotateTexture90)
{
    const sal_uInt32 nPolyCnt(rPolyPolyFront.count());
    bool bDoCreateTexture(0.0 != fTextureDepth);
    double fPolyLength(0.0), fPolyPos(0.0);
    sal_uInt32 nLastIndex(0L);

    // Verbindungsstuecke
    if(bDoCreateNormals)
    {
        for(sal_uInt32 a(0L); a < nPolyCnt; a++)
        {
            const basegfx::B3DPolygon aPoly3DFront(rPolyPolyFront.getB3DPolygon(a));
            const basegfx::B3DPolygon aPoly3DBack(rPolyPolyBack.getB3DPolygon(a));
            const basegfx::B3DPolygon aPolyNormalsFront(rFrontNormals.getB3DPolygon(a));
            const basegfx::B3DPolygon aPolyNormalsBack(rBackNormals.getB3DPolygon(a));
            const sal_uInt32 nPntCnt(aPoly3DFront.count());
            const sal_uInt32 nPrefillIndex(aPoly3DFront.isClosed() ? nPntCnt - 1L : 0L);
            basegfx::B3DPolygon aRect3D;
            basegfx::B3DPolygon aNormal3D;
            basegfx::B2DPolygon aTexture2D;

            aRect3D.append(basegfx::B3DPoint(), 4L);
            aNormal3D.append(basegfx::B3DPoint(), 4L);
            aTexture2D.append(basegfx::B2DPoint(), 4L);
            aRect3D.setB3DPoint(3L, aPoly3DFront.getB3DPoint(nPrefillIndex));
            aRect3D.setB3DPoint(2L, aPoly3DBack.getB3DPoint(nPrefillIndex));
            aNormal3D.setB3DPoint(3L, aPolyNormalsFront.getB3DPoint(nPrefillIndex));
            aNormal3D.setB3DPoint(2L, aPolyNormalsBack.getB3DPoint(nPrefillIndex));

            if(bDoCreateTexture)
            {
                fPolyLength = basegfx::tools::getLength(aPoly3DFront);
                fPolyPos = 0.0;
                nLastIndex = aPoly3DFront.isClosed() ? nPntCnt - 1L : 0L;

                if(bRotateTexture90)
                {
                    // X,Y vertauschen
                    aTexture2D.setB2DPoint(3L, basegfx::B2DPoint(fTextureStart, (1.0 - fPolyPos) * fSurroundFactor));
                    aTexture2D.setB2DPoint(2L, basegfx::B2DPoint(fTextureStart + fTextureDepth, (1.0 - fPolyPos) * fSurroundFactor));
                }
                else
                {
                    aTexture2D.setB2DPoint(3L, basegfx::B2DPoint(fPolyPos * fSurroundFactor, fTextureStart));
                    aTexture2D.setB2DPoint(2L, basegfx::B2DPoint(fPolyPos * fSurroundFactor, fTextureStart + fTextureDepth));
                }
            }

            for(sal_uInt32 i(aPoly3DFront.isClosed() ? 0L : 1L); i < nPntCnt; i++)
            {
                aRect3D.setB3DPoint(0L, aRect3D.getB3DPoint(3L));
                aRect3D.setB3DPoint(1L, aRect3D.getB3DPoint(2L));

                aRect3D.setB3DPoint(3L, aPoly3DFront.getB3DPoint(i));
                aRect3D.setB3DPoint(2L, aPoly3DBack.getB3DPoint(i));

                aNormal3D.setB3DPoint(0L, aNormal3D.getB3DPoint(3L));
                aNormal3D.setB3DPoint(1L, aNormal3D.getB3DPoint(2L));

                aNormal3D.setB3DPoint(3L, aPolyNormalsFront.getB3DPoint(i));
                aNormal3D.setB3DPoint(2L, aPolyNormalsBack.getB3DPoint(i));

                if(bDoCreateTexture)
                {
                    // Texturkoordinaten ermitteln
                    basegfx::B3DVector aPart(aPoly3DFront.getB3DPoint(i) - aPoly3DFront.getB3DPoint(nLastIndex));
                    fPolyPos += aPart.getLength() / fPolyLength;
                    nLastIndex = i;

                    // Der Abschnitt am Polygon entspricht dem Teil
                    // von fPolyPos bis fPolyPos+fPartLength

                    aTexture2D.setB2DPoint(0L, aTexture2D.getB2DPoint(3L));
                    aTexture2D.setB2DPoint(1L, aTexture2D.getB2DPoint(2L));

                    if(bRotateTexture90)
                    {
                        // X,Y vertauschen
                        aTexture2D.setB2DPoint(3L, basegfx::B2DPoint(fTextureStart, (1.0 - fPolyPos) * fSurroundFactor));
                        aTexture2D.setB2DPoint(2L, basegfx::B2DPoint(fTextureStart + fTextureDepth, (1.0 - fPolyPos) * fSurroundFactor));
                    }
                    else
                    {
                        aTexture2D.setB2DPoint(3L, basegfx::B2DPoint(fPolyPos * fSurroundFactor, fTextureStart));
                        aTexture2D.setB2DPoint(2L, basegfx::B2DPoint(fPolyPos * fSurroundFactor, fTextureStart + fTextureDepth));
                    }

                    AddGeometry(basegfx::B3DPolyPolygon(aRect3D), basegfx::B3DPolyPolygon(aNormal3D), basegfx::B2DPolyPolygon(aTexture2D), FALSE);
                }
                else
                {
                    AddGeometry(basegfx::B3DPolyPolygon(aRect3D), basegfx::B3DPolyPolygon(aNormal3D), FALSE);
                }
            }
        }
    }
    else
    {
        for(sal_uInt32 a(0L); a < nPolyCnt; a++)
        {
            const basegfx::B3DPolygon aPoly3DFront(rPolyPolyFront.getB3DPolygon(a));
            const basegfx::B3DPolygon aPoly3DBack(rPolyPolyBack.getB3DPolygon(a));
            basegfx::B3DPolygon aRect3D;
            const sal_uInt32 nPntCnt(aPoly3DFront.count());
            const sal_uInt32 nPrefillIndex(aPoly3DFront.isClosed() ? nPntCnt - 1L : 0L);

            aRect3D.append(basegfx::B3DPoint(), 4L);
            aRect3D.setB3DPoint(3L, aPoly3DFront.getB3DPoint(nPrefillIndex));
            aRect3D.setB3DPoint(2L, aPoly3DBack.getB3DPoint(nPrefillIndex));

            for(sal_uInt32 i(aPoly3DFront.isClosed() ? 0L : 1L); i < nPntCnt; i++)
            {
                aRect3D.setB3DPoint(0L, aRect3D.getB3DPoint(3L));
                aRect3D.setB3DPoint(1L, aRect3D.getB3DPoint(2L));

                aRect3D.setB3DPoint(3L, aPoly3DFront.getB3DPoint(i));
                aRect3D.setB3DPoint(2L, aPoly3DBack.getB3DPoint(i));

                AddGeometry(basegfx::B3DPolyPolygon(aRect3D), FALSE);
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

sal_Bool E3dCompoundObject::ImpSet3DParForFill(XOutputDevice& /*rOut*/, Base3D* pBase3D, UINT16 nDrawFlags, BOOL bGhosted)
{
    sal_Bool bDrawObject(sal_True);

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
            basegfx::B2DHomMatrix mTexture;

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
                basegfx::B2DPoint aScaleVector(1.0, 1.0);
                basegfx::B2DPoint aTranslateVector(0.0, 0.0);

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
                        (long)((rVol.getWidth() * (double)aSize.Width() / 100.0) + 0.5),
                        (long)((rVol.getHeight() * (double)aSize.Height() / 100.0) + 0.5));
                }

                // Skalieren
                aScaleVector.setX(rVol.getWidth() / (double)aLogicalSize.Width());
                aScaleVector.setY(rVol.getHeight() / (double)aLogicalSize.Height());

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
                        fLeftBound = (rVol.getWidth() / 2.0) - ((double)aLogicalSize.Width() / 2.0);
                    }
                    else
                    {
                        // Rechts aligned starten
                        fLeftBound = rVol.getWidth() - (double)aLogicalSize.Width();
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
                        fTopBound = (rVol.getHeight() / 2.0) - ((double)aLogicalSize.Height() / 2.0);
                    }
                    else
                    {
                        // Bottom aligned starten
                        fTopBound = rVol.getHeight() - (double)aLogicalSize.Height();
                    }

                    // Verschieben
                    aTranslateVector.setX(fLeftBound);
                    aTranslateVector.setY(fTopBound);

                    // Offset beachten
                    if(nOffPosX || nOffPosY)
                    {
                        aTranslateVector.setX(aTranslateVector.getX() + (double)aLogicalSize.Width() * ((double)nOffPosX / 100.0));
                        aTranslateVector.setY(aTranslateVector.getY() + (double)aLogicalSize.Height() * ((double)nOffPosY / 100.0));
                    }
                }
                else
                {
                    if(bStretch)
                    {
                        // 1x drauflegen, alles wie gehabt
                        // fertig
                        aScaleVector.setX(1.0);
                        aScaleVector.setY(1.0);
                    }
                    else
                    {
                        // nur einmal benutzen
                        eWrapX = Base3DTextureSingle;
                        eWrapY = Base3DTextureSingle;

                        // Groesse beachten, zentriert anlegen
                        double fLeftBound = (rVol.getWidth() / 2.0) - ((double)aLogicalSize.Width() / 2.0);
                        double fTopBound = (rVol.getHeight() / 2.0) - ((double)aLogicalSize.Height() / 2.0);

                        // Verschieben
                        aTranslateVector.setX(fLeftBound);
                        aTranslateVector.setY(fTopBound);
                    }
                }

                // TranslateVector anpassen
                if(aTranslateVector.getX())
                    aTranslateVector.setX(aTranslateVector.getX() / -rVol.getWidth());
                if(aTranslateVector.getY())
                    aTranslateVector.setY(aTranslateVector.getY() / -rVol.getHeight());

                // Texturtransformation setzen
                mTexture.translate(aTranslateVector.getX(),aTranslateVector.getY());
                mTexture.scale(aScaleVector.getX(),aScaleVector.getY());
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
                mTexture.scale(20.0, 20.0);
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

    return bDrawObject;
}

sal_Bool E3dCompoundObject::ImpSet3DParForLine(XOutputDevice& rOut, Base3D* pBase3D, UINT16 nDrawFlags, BOOL /*bGhosted*/)
{
    sal_Bool bDrawOutline(sal_True);

    // do drawflags allow line drawing at all?
    const SfxItemSet& rSet = GetObjectItemSet();
    sal_uInt16 nLineTransparence = ((const XLineTransparenceItem&)(rSet.Get(XATTR_LINETRANSPARENCE))).GetValue();
    BOOL bLineTransparence = (nLineTransparence != 0);
    BOOL bDrawTransparence = ((nDrawFlags & E3D_DRAWFLAG_TRANSPARENT) != 0);

    // #b4899532# if not filled but fill draft, avoid object being invisible in using
    // a hair linestyle and COL_LIGHTGRAY
    SfxItemSet aItemSet(rSet);

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

        Point aPnt(nLineWidth, 0);
        aPnt = pBase3D->GetOutputDevice()->LogicToPixel(aPnt) - pBase3D->GetOutputDevice()->LogicToPixel(Point());
        if(aPnt.X() <= 0)
            aPnt.X() = 1;
        pBase3D->SetLineWidth((double)aPnt.X());

        // Material setzen
        pBase3D->SetColor(aColorLine);
    }

    return bDrawOutline;
}

void E3dCompoundObject::SetBase3DParams(XOutputDevice& rOut, Base3D* pBase3D,
    BOOL& bDrawObject, BOOL& bDrawOutline, UINT16 nDrawFlags, BOOL bGhosted)
{
    bDrawObject = ((nDrawFlags & E3D_DRAWFLAG_FILLED) != 0);
    if(bDrawObject)
    {
        bDrawObject = ImpSet3DParForFill(rOut, pBase3D, nDrawFlags, bGhosted);
    }

    bDrawOutline = ((nDrawFlags & E3D_DRAWFLAG_OUTLINE) != 0);
    if(bDrawOutline)
    {
        bDrawOutline = ImpSet3DParForLine(rOut, pBase3D, nDrawFlags, bGhosted);
    }

    // Set ObjectTrans if line or fill is still set (maybe retet by upper calls)
    if(bDrawObject || bDrawOutline)
    {
        basegfx::B3DHomMatrix mTransform = GetFullTransform();
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
sal_Bool E3dCompoundObject::ImpIsInsideBoundVolume(const basegfx::B3DPoint& rFront, const basegfx::B3DPoint& rBack, const Point& /*rPnt*/) const
{
    const Volume3D& rBoundVol = ((E3dCompoundObject*)this)->GetBoundVolume();

    if(!rBoundVol.isEmpty())
    {
        double fXMax = rFront.getX();
        double fXMin = rBack.getX();

        if(fXMax < fXMin)
        {
            fXMax = rBack.getX();
            fXMin = rFront.getX();
        }

        if(rBoundVol.getMinX() <= fXMax && rBoundVol.getMaxX() >= fXMin)
        {
            double fYMax = rFront.getY();
            double fYMin = rBack.getY();

            if(fYMax < fYMin)
            {
                fYMax = rBack.getY();
                fYMin = rFront.getY();
            }

            if(rBoundVol.getMinY() <= fYMax && rBoundVol.getMaxY() >= fYMin)
            {
                double fZMax = rFront.getZ();
                double fZMin = rBack.getZ();

                if(fZMax < fZMin)
                {
                    fZMax = rBack.getZ();
                    fZMin = rFront.getZ();
                }

                if(rBoundVol.getMinZ() <= fZMax && rBoundVol.getMaxZ() >= fZMin)
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
        basegfx::B3DHomMatrix mTransform = GetFullTransform();
        pScene->GetCameraSet().SetObjectTrans(mTransform);

        // create HitPoint Front und Back, transform to object coordinates
        basegfx::B3DPoint aFront(rPnt.X(), rPnt.Y(), 0.0);
        basegfx::B3DPoint aBack(rPnt.X(), rPnt.Y(), ZBUFFER_DEPTH_RANGE);
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
|* Schattenattribute holen
|*
\************************************************************************/

Color E3dCompoundObject::GetShadowColor() const
{
    return ((SdrShadowColorItem&)(GetObjectItem(SDRATTR_SHADOWCOLOR))).GetColorValue();
}

BOOL E3dCompoundObject::DrawShadowAsOutline() const
{
    const SfxItemSet& rSet = GetObjectItemSet();
    XFillStyle eFillStyle = ((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();
    XLineStyle eLineStyle = ((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
    BOOL bFillAttrIsNone = eFillStyle == XFILL_NONE;
    BOOL bLineAttrIsNone = eLineStyle == XLINE_NONE;
    return (bFillAttrIsNone && !bLineAttrIsNone);
}

INT32 E3dCompoundObject::GetShadowXDistance() const
{
    return (long)((SdrShadowXDistItem&)(GetObjectItem(SDRATTR_SHADOWXDIST))).GetValue();
}

INT32 E3dCompoundObject::GetShadowYDistance() const
{
    return (long)((SdrShadowYDistItem&)(GetObjectItem(SDRATTR_SHADOWYDIST))).GetValue();
}

UINT16 E3dCompoundObject::GetShadowTransparence() const
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
    const B3dEntityBucket& rEntityBucket = GetDisplayGeometry().GetEntityBucket();
    const GeometryIndexValueBucket& rIndexBucket = GetDisplayGeometry().GetIndexBucket();
    B3dTransformationSet& rTransSet = GetScene()->GetCameraSet();
    BOOL bDrawLine, bLastDrawLine;
    basegfx::B3DPoint aPoint;

    while(nPolyCounter < rIndexBucket.Count())
    {
        // Naechstes Primitiv
        nUpperBound = rIndexBucket[nPolyCounter++].GetIndex();
        bDrawLine = bLastDrawLine = rEntityBucket[nEntityCounter].IsEdgeVisible();
        aPoint = rTransSet.ObjectToViewCoor(rEntityBucket[nEntityCounter++].Point());
        aFirstPoint.X() = (long)(aPoint.getX() + 0.5);
        aFirstPoint.Y() = (long)(aPoint.getY() + 0.5);
        aLastPoint = aFirstPoint;

        // Polygon fuellen
        while(nEntityCounter < nUpperBound)
        {
            // Punkt holen und auf Weltkoordinaten umrechnen
            bDrawLine = rEntityBucket[nEntityCounter].IsEdgeVisible();
            aPoint = rTransSet.ObjectToViewCoor(rEntityBucket[nEntityCounter++].Point());
            aNewPoint.X() = (long)(aPoint.getX() + 0.5);
            aNewPoint.Y() = (long)(aPoint.getY() + 0.5);
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
basegfx::B3DPolyPolygon E3dCompoundObject::ImpCompleteLinePolygon(const basegfx::B3DPolyPolygon& rLinePolyPoly, sal_uInt32 nPolysPerRun, sal_Bool bClosed)
{
    basegfx::B3DPolyPolygon aRetval;
    const sal_uInt32 nLinePolyPolyCount(rLinePolyPoly.count());

    if(nLinePolyPolyCount && nPolysPerRun)
    {
        // get number of layers
        sal_uInt32 nLayers(nLinePolyPolyCount / nPolysPerRun);

        // add vertical Polygons if at least two horizontal ones exist
        if(nLayers > 1L)
        {
            for(sal_uInt32 a(0L); a < nPolysPerRun; a++)
            {
                const sal_uInt32 nPntCnt(rLinePolyPoly.getB3DPolygon(a).count());

                for(sal_uInt32 b(0L); b < nPntCnt; b++)
                {
                    basegfx::B3DPolygon aNewVerPoly;

                    for(sal_uInt32 c(0L); c < nLayers; c++)
                    {
                        aNewVerPoly.append(rLinePolyPoly.getB3DPolygon((c * nPolysPerRun) + a).getB3DPoint(b));
                    }

                    // evtl. set first point again to close polygon
                    aNewVerPoly.setClosed(bClosed);

                    // insert
                    aRetval.append(aNewVerPoly);
                }
            }
        }
    }

    return aRetval;
}

/*************************************************************************
|*
|* Ein Segment fuer Extrude oder Lathe erzeugen
|*
\************************************************************************/

void E3dCompoundObject::ImpCreateSegment(
    const basegfx::B3DPolyPolygon& rFront,      // vorderes Polygon
    const basegfx::B3DPolyPolygon& rBack,           // hinteres Polygon
    const basegfx::B3DPolyPolygon* pPrev,           // smooth uebergang zu Vorgaenger
    const basegfx::B3DPolyPolygon* pNext,           // smooth uebergang zu Nachfolger
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
    basegfx::B3DPolyPolygon* pLineGeometryFront,    // For creation of line geometry front parts
    basegfx::B3DPolyPolygon* pLineGeometryBack, // For creation of line geometry back parts
    basegfx::B3DPolyPolygon* pLineGeometry      // For creation of line geometry in-betweens
    )
{
    basegfx::B3DPolyPolygon aNormalsLeft(ImpCreateByPattern(rFront));
    basegfx::B3DPolyPolygon aNormalsRight(ImpCreateByPattern(rFront));
    aNormalsLeft = ImpAddInBetweenNormals(rFront, rBack, aNormalsLeft, bSmoothLeft);
    aNormalsRight = ImpAddInBetweenNormals(rFront, rBack, aNormalsRight, bSmoothRight);
    const basegfx::B3DRange aBackRange(basegfx::tools::getRange(rBack));
    const basegfx::B3DRange aFrontRange(basegfx::tools::getRange(rFront));
    basegfx::B3DPoint aOffset(aBackRange.getCenter() - aFrontRange.getCenter());

    // #i28528#
    sal_Bool bTakeCareOfLineGeometry(pLineGeometryFront != 0L || pLineGeometryBack != 0L || pLineGeometry != 0L);

    // Ausnahmen: Nicht geschlossen
    if(!rFront.isClosed())
    {
        bCreateFront = FALSE;
    }
    if(!rBack.isClosed())
    {
        bCreateBack = FALSE;
    }

    // Ausnahmen: Einfache Linie
    if(rFront.getB3DPolygon(0L).count() < 3L || (!bCreateFront && !bCreateBack))
    {
        fPercentDiag = 0.0;
    }

    if(fPercentDiag == 0.0)
    {
        // Ohne Schraegen, Vorderseite
        if(bCreateFront)
        {
            basegfx::B3DPolyPolygon aNormalsFront(ImpCreateByPattern(rFront));
            aNormalsFront = ImpAddFrontNormals(aNormalsFront, aOffset);

            if(!bSmoothFrontBack)
                ImpCreateFront(rFront, aNormalsFront, bDoCreateNormals, bDoCreateTexture);
            if(bSmoothLeft)
                aNormalsLeft = ImpAddFrontNormals(aNormalsLeft, aOffset);
            if(bSmoothFrontBack)
                ImpCreateFront(rFront, aNormalsLeft, bDoCreateNormals, bDoCreateTexture);
        }
        else
        {
            if(pPrev)
                aNormalsLeft = ImpAddInBetweenNormals(*pPrev, rFront, aNormalsLeft, bSmoothLeft);
        }

        // Ohne Schraegen, Rueckseite
        if(bCreateBack)
        {
            basegfx::B3DPolyPolygon aNormalsBack(ImpCreateByPattern(rBack));
            aNormalsBack = ImpAddBackNormals(aNormalsBack, aOffset);

            if(!bSmoothFrontBack)
                ImpCreateBack(rBack, aNormalsBack, bDoCreateNormals, bDoCreateTexture);
            if(bSmoothRight)
                aNormalsRight = ImpAddBackNormals(aNormalsRight, aOffset);
            if(bSmoothFrontBack)
                ImpCreateBack(rBack, aNormalsRight, bDoCreateNormals, bDoCreateTexture);
        }
        else
        {
            if(pNext)
                aNormalsRight = ImpAddInBetweenNormals(rBack, *pNext, aNormalsRight, bSmoothRight);
        }

        // eigentliches Zwischenstueck
        ImpCreateInBetween(rFront, rBack,
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
                if(pLineGeometryFront) pLineGeometryFront->append(rFront);
            }
            else
            {
                if(pLineGeometry) pLineGeometry->append(rFront);
            }

            if(bCreateBack)
            {
                if(pLineGeometryBack) pLineGeometryBack->append(rBack);
            }
        }
    }
    else
    {
        // Mit Scraegen, Vorderseite
        basegfx::B3DPolyPolygon aLocalFront = rFront;
        basegfx::B3DPolyPolygon aLocalBack = rBack;
        double fExtrudeDepth, fDiagLen(0.0);
        double fTexMidStart = fTextureStart;
        double fTexMidDepth = fTextureDepth;

        if(bCreateFront || bCreateBack)
        {
            fExtrudeDepth = basegfx::B3DVector(aOffset).getLength();
            fDiagLen = fPercentDiag * fExtrudeDepth;
        }

        basegfx::B3DPolyPolygon aOuterFront;
        basegfx::B3DPolyPolygon aOuterBack;

        if(bCreateFront)
        {
            basegfx::B3DPolyPolygon aNormalsOuterFront(ImpCreateByPattern(aLocalFront));
            aNormalsOuterFront = ImpAddFrontNormals(aNormalsOuterFront, aOffset);

            if(bCharacterExtrude)
            {
                // Polygon kopieren
                aOuterFront = aLocalFront;

                // notwendige Normalen erzeugen
                basegfx::B3DPolyPolygon aGrowDirection(ImpCreateByPattern(aLocalFront));
                aGrowDirection = ImpAddInBetweenNormals(aLocalFront, aLocalBack, aGrowDirection, bSmoothLeft);

                // Groesse inneres Polygon merken
                Volume3D aOldSize(basegfx::tools::getRange(aLocalFront));

                // Inneres Polygon vergroessern
                aLocalFront = ImpGrowPoly(aLocalFront, aGrowDirection, fDiagLen);

                // Inneres Polygon nach innen verschieben
                //GrowPoly(aLocalFront, aNormalsOuterFront, -fDiagLen);

                // Neue Groesse inneres Polygon feststellen
                basegfx::B3DRange aNewSize(basegfx::tools::getRange(aLocalFront));

                // Skalierung feststellen (nur X,Y)
                basegfx::B3DPoint aScaleVec(
                    (aNewSize.getWidth() != 0.0) ? aOldSize.getWidth() / aNewSize.getWidth() : 1.0,
                    (aNewSize.getHeight() != 0.0) ? aOldSize.getHeight() / aNewSize.getHeight() : 1.0,
                    (aNewSize.getDepth() != 0.0) ? aOldSize.getDepth() / aNewSize.getDepth() : 1.0);

                // Transformation bilden
                basegfx::B3DHomMatrix aTransMat;
                aTransMat.scale(aScaleVec.getX(), aScaleVec.getY(), aScaleVec.getZ());

                // aeusseres und inneres Polygon skalieren
                aLocalFront.transform(aTransMat);
                aOuterFront.transform(aTransMat);

                // Neue Groesse aktualisieren
                aNewSize = basegfx::tools::getRange(aLocalFront);

                // Translation feststellen
                basegfx::B3DPoint aTransVec(aOldSize.getCenter() - aNewSize.getCenter());

                // Transformation bilden
                aTransMat.identity();
                aTransMat.translate(aTransVec.getX(), aTransVec.getY(), aTransVec.getZ());

                // aeusseres und inneres Polygon skalieren
                aLocalFront.transform(aTransMat);
                aOuterFront.transform(aTransMat);

                // move aLocalFront again, scale and translate has moved it back
                aLocalFront = ImpGrowPoly(aLocalFront, aNormalsOuterFront, -fDiagLen);
            }
            else
            {
                // Polygon kopieren
                aOuterFront = aLocalFront;

                // notwendige Normalen erzeugen
                basegfx::B3DPolyPolygon aGrowDirection(ImpCreateByPattern(aLocalFront));
                aGrowDirection = ImpAddInBetweenNormals(aLocalFront, aLocalBack, aGrowDirection, bSmoothLeft);

                // Aeusseres Polygon verkleinern
                aOuterFront = ImpGrowPoly(aOuterFront, aGrowDirection, -fDiagLen);
                aOuterFront = ImpCorrectGrownPoly(aOuterFront, aLocalFront);

                // Inneres Polygon nach innen verschieben
                aLocalFront = ImpGrowPoly(aLocalFront, aNormalsOuterFront, -fDiagLen);
            }

            // eventuell noch glaetten
            if(bSmoothLeft)
            {
                if(bSmoothFrontBack)
                    aNormalsOuterFront = ImpAddInBetweenNormals(aOuterFront, aLocalFront, aNormalsOuterFront, bSmoothLeft);
                aNormalsLeft = ImpAddInBetweenNormals(aOuterFront, aLocalFront, aNormalsLeft, bSmoothLeft);
            }

            // vordere Zwischenstuecke erzeugen
            ImpCreateInBetween(aOuterFront, aLocalFront,
                aNormalsOuterFront, aNormalsLeft,
                bDoCreateNormals,
                fSurroundFactor,
                fTextureStart,
                fTextureDepth * fPercentDiag,
                bRotateTexture90);

            // Vorderseite erzeugen
            ImpCreateFront(aOuterFront, aNormalsOuterFront, bDoCreateNormals, bDoCreateTexture);

            // Weitere Texturwerte setzen
            fTexMidStart += fTextureDepth * fPercentDiag;
            fTexMidDepth -= fTextureDepth * fPercentDiag;
        }
        else
        {
            if(pPrev)
                aNormalsLeft = ImpAddInBetweenNormals(*pPrev, rFront, aNormalsLeft, bSmoothLeft);
        }

        // Mit Scraegen, Rueckseite
        if(bCreateBack)
        {
            basegfx::B3DPolyPolygon aNormalsOuterBack(ImpCreateByPattern(aLocalBack));
            aNormalsOuterBack = ImpAddBackNormals(aNormalsOuterBack, aOffset);

            if(bCharacterExtrude)
            {
                // Polygon kopieren
                aOuterBack = aLocalBack;

                // notwendige Normalen erzeugen
                basegfx::B3DPolyPolygon aGrowDirection(ImpCreateByPattern(aLocalFront));
                aGrowDirection = ImpAddInBetweenNormals(aLocalFront, aLocalBack, aGrowDirection, bSmoothRight);

                // Groesse inneres Polygon merken
                Volume3D aOldSize(basegfx::tools::getRange(aLocalBack));

                // Inneres Polygon vergroessern
                aLocalBack = ImpGrowPoly(aLocalBack, aGrowDirection, fDiagLen);

                // Inneres Polygon nach innen verschieben
                //GrowPoly(aLocalBack, aNormalsOuterBack, -fDiagLen);

                // Neue Groesse inneres Polygon feststellen
                basegfx::B3DRange aNewSize(basegfx::tools::getRange(aLocalBack));

                // Skalierung feststellen (nur X,Y)
                basegfx::B3DPoint aScaleVec(
                    (aNewSize.getWidth() != 0.0) ? aOldSize.getWidth() / aNewSize.getWidth() : 1.0,
                    (aNewSize.getHeight() != 0.0) ? aOldSize.getHeight() / aNewSize.getHeight() : 1.0,
                    (aNewSize.getDepth() != 0.0) ? aOldSize.getDepth() / aNewSize.getDepth() : 1.0);

                // Transformation bilden
                basegfx::B3DHomMatrix aTransMat;
                aTransMat.scale(aScaleVec.getX(), aScaleVec.getY(), aScaleVec.getZ());

                // aeusseres und inneres Polygon skalieren
                aLocalBack.transform(aTransMat);
                aOuterBack.transform(aTransMat);

                // Neue Groesse aktualisieren
                aNewSize = basegfx::tools::getRange(aLocalBack);

                // Translation feststellen
                basegfx::B3DPoint aTransVec(aOldSize.getCenter() - aNewSize.getCenter());

                // Transformation bilden
                aTransMat.identity();
                aTransMat.translate(aTransVec.getX(), aTransVec.getY(), aTransVec.getZ());

                // aeusseres und inneres Polygon skalieren
                aLocalBack.transform(aTransMat);
                aOuterBack.transform(aTransMat);

                // move aLocalBack again, scale and translate has moved it back
                aLocalBack = ImpGrowPoly(aLocalBack, aNormalsOuterBack, -fDiagLen);
            }
            else
            {
                // Polygon kopieren
                aOuterBack = aLocalBack;

                // notwendige Normalen erzeugen
                basegfx::B3DPolyPolygon aGrowDirection(ImpCreateByPattern(aLocalFront));
                aGrowDirection = ImpAddInBetweenNormals(aLocalFront, aLocalBack, aGrowDirection, bSmoothRight);

                // Aeusseres Polygon verkleinern
                aOuterBack = ImpGrowPoly(aOuterBack, aGrowDirection, -fDiagLen);
                aOuterBack = ImpCorrectGrownPoly(aOuterBack, aLocalBack);

                // Inneres Polygon nach innen verschieben
                aLocalBack = ImpGrowPoly(aLocalBack, aNormalsOuterBack, -fDiagLen);
            }

            // eventuell noch glaetten
            if(bSmoothRight)
            {
                if(bSmoothFrontBack)
                    aNormalsOuterBack = ImpAddInBetweenNormals(aLocalBack, aOuterBack, aNormalsOuterBack, bSmoothRight);
                aNormalsRight = ImpAddInBetweenNormals(aLocalBack, aOuterBack, aNormalsRight, bSmoothRight);
            }

            // vordere Zwischenstuecke erzeugen
            // hintere Zwischenstuecke erzeugen
            ImpCreateInBetween(aLocalBack, aOuterBack,
                aNormalsRight, aNormalsOuterBack,
                bDoCreateNormals,
                fSurroundFactor,
                fTextureStart + (fTextureDepth * (1.0 - fPercentDiag)),
                fTextureDepth * fPercentDiag,
                bRotateTexture90);

            // Rueckseite erzeugen
            ImpCreateBack(aOuterBack, aNormalsOuterBack, bDoCreateNormals, bDoCreateTexture);

            // Weitere Texturwerte setzen
            fTexMidDepth -= fTextureDepth * fPercentDiag;
        }
        else
        {
            if(pNext)
                aNormalsRight = ImpAddInBetweenNormals(rBack, *pNext, aNormalsRight, bSmoothRight);
        }

        // eigentliches Zwischenstueck
        ImpCreateInBetween(aLocalFront, aLocalBack,
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
                if(pLineGeometryFront) pLineGeometryFront->append(aOuterFront);
            }

            if(bCreateFront)
            {
                if(pLineGeometryFront) pLineGeometryFront->append(aLocalFront);
            }
            else
            {
                if(pLineGeometry) pLineGeometry->append(aLocalFront);
            }

            if(bCreateBack && pLineGeometryBack)
            {
                pLineGeometryBack->append(aLocalBack);
                pLineGeometryBack->append(aOuterBack);
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

const B3dGeometry& E3dCompoundObject::GetDisplayGeometry() const
{
    // Geometrie herstellen
    if(!bGeometryValid)
    {
        ((E3dCompoundObject*)this)->ReCreateGeometry();
    }

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

void ImplGet3DLineGeometry(const SfxItemSet& rSet, const basegfx::B3DPolyPolygon& rLinePolyPolygon,
    basegfx::B3DPolyPolygon& rAreaPolyPoly, basegfx::B3DPolyPolygon& rNormalPolyPoly)
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
            double fFullDotDashLen = aDash.CreateDotDashArray(aDotDashArray, (double)nLineWidth);

            // convert to new polygon class
            rAreaPolyPoly = rLinePolyPolygon;

            // apply line dashing
            rAreaPolyPoly = basegfx::tools::applyLineDashing(rAreaPolyPoly, aDotDashArray, fFullDotDashLen);
        }

        if(0L != nLineWidth)
        {
            // prepare rAreaPolyPoly for output data
            const basegfx::B3DPolyPolygon aSourcePolyPolygon(rAreaPolyPoly);
            rAreaPolyPoly.clear();
            const double fDistance(nLineWidth / 2.0);

            for(sal_uInt32 nInd(0L); nInd < aSourcePolyPolygon.count(); nInd++)
            {
                basegfx::B3DPolygon aLinePoly(aSourcePolyPolygon.getB3DPolygon(nInd));
                const sal_uInt32 nOrigCount(aLinePoly.count());

                if(nOrigCount)
                {
                    const sal_uInt32 nCount(aLinePoly.isClosed() ? nOrigCount : nOrigCount - 1L);

                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        basegfx::B3DPoint aStart(aLinePoly.getB3DPoint(a));
                        basegfx::B3DPoint aEnd(aLinePoly.getB3DPoint((a + 1L) % nOrigCount));
                        basegfx::B3DVector aVector(aEnd - aStart);

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
                            basegfx::B3DVector aOppositeVector(-aVector);
                            basegfx::B3DVector aStartVec(-aVector.getY(), aVector.getZ(), aVector.getX());
                            basegfx::B3DVector aXAxis = aVector.getPerpendicular(aStartVec);
                            basegfx::B3DVector aYAxis = aVector.getPerpendicular(aXAxis);

                            // prepare angle and angle increment
                            double fAngle(0.0);
                            const double fAngleIncrement(F_PI * (2.0 / 6.0));
                            double fSin(sin(fAngle));
                            double fCos(cos(fAngle));

                            // prepare start vectors
                            basegfx::B3DVector aCurrentVector((aXAxis * fCos) + (aYAxis * fSin));
                            basegfx::B3DVector aOffset(aCurrentVector * fDistance);
                            basegfx::B3DPoint aLeft(aStart + aOffset);
                            basegfx::B3DPoint aRight(aEnd + aOffset);

                            for(sal_uInt32 b(0L); b < 6; b++)
                            {
                                fAngle += fAngleIncrement;
                                fSin = sin(fAngle);
                                fCos = cos(fAngle);

                                basegfx::B3DVector aNextVector((aXAxis * fCos) + (aYAxis * fSin));
                                aOffset = basegfx::B3DVector(aNextVector * fDistance);
                                basegfx::B3DPoint aNextLeft(aStart + aOffset);
                                basegfx::B3DPoint aNextRight(aEnd + aOffset);
                                basegfx::B3DPolygon aLine;
                                basegfx::B3DPolygon aNormal;

                                aLine.append(aStart);
                                aLine.append(aNextLeft);
                                aLine.append(aLeft);
                                aLine.append(aNextRight);
                                aLine.append(aRight);
                                aLine.append(aEnd);

                                aNormal.append(basegfx::B3DPoint(aVector));
                                aNormal.append(basegfx::B3DPoint(aNextVector));
                                aNormal.append(basegfx::B3DPoint(aCurrentVector));
                                aNormal.append(basegfx::B3DPoint(aNextVector));
                                aNormal.append(basegfx::B3DPoint(aCurrentVector));
                                aNormal.append(basegfx::B3DPoint(aOppositeVector));

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
        BOOL bGhosted((rInfoRec.pPV && rInfoRec.pPV->GetView().DoVisualizeEnteredGroup()) ? rInfoRec.bNotActive : FALSE);
        SetBase3DParams(rOut, pBase3D, bDrawObject, bDrawOutline, nDrawFlags, bGhosted);

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
            basegfx::B3DPolyPolygon aLinePolyPolygon(Get3DLineGeometry());
            basegfx::B3DPolyPolygon aAreaPolyPolygon;
            basegfx::B3DPolyPolygon aNormalPolyPolygon;
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
                        basegfx::B3DPolygon aPolygon(aAreaPolyPolygon.getB3DPolygon(a));
                        basegfx::B3DPolygon aNormals(aNormalPolyPolygon.getB3DPolygon(a));
                        pBase3D->StartPrimitive(Base3DTriangleStrip);

                        for(sal_uInt32 b(0); b < aPolygon.count(); b++)
                        {
                            basegfx::B3DVector aVector(aNormals.getB3DPoint(b));
                            basegfx::B3DPoint a3DPoint(aPolygon.getB3DPoint(b));
                            pBase3D->AddVertex(a3DPoint, aVector);
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
                        const basegfx::B3DPolygon aPolygon(aAreaPolyPolygon.getB3DPolygon(a));
                        pBase3D->StartPrimitive(Base3DLineStrip);

                        for(sal_uInt32 b(0); b < aPolygon.count(); b++)
                        {
                            basegfx::B3DPoint a3DPoint(aPolygon.getB3DPoint(b));
                            pBase3D->AddVertex(a3DPoint);
                        }

                        if(aPolygon.count() && aPolygon.isClosed())
                        {
                            basegfx::B3DPoint a3DPoint(aPolygon.getB3DPoint(0L));
                            pBase3D->AddVertex(a3DPoint);
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

basegfx::B2DPolyPolygon E3dCompoundObject::ImpTakeContour3D() const
{
    basegfx::B2DPolyPolygon aRetval;

    // call parent
    aRetval.append(E3dObject::ImpTakeContour3D());

    // Kontur dieses Objektes liefern
    UINT32 nPolyCounter = 0;
    UINT32 nEntityCounter = 0;
    UINT32 nUpperBound;
    const B3dEntityBucket& rEntityBucket = GetDisplayGeometry().GetEntityBucket();
    const GeometryIndexValueBucket& rIndexBucket = GetDisplayGeometry().GetIndexBucket();
    B3dTransformationSet& rTransSet = GetScene()->GetCameraSet();
    basegfx::B3DVector aPoint;
    Point aNewPoint;

    // ObjectTrans setzen
    basegfx::B3DHomMatrix mTransform = GetFullTransform();
    rTransSet.SetObjectTrans(mTransform);

    while(nPolyCounter < rIndexBucket.Count())
    {
        // Naechstes Primitiv
        nUpperBound = rIndexBucket[nPolyCounter++].GetIndex();
        basegfx::B2DPolygon aNew;

        while(nEntityCounter < nUpperBound)
        {
            aPoint = rTransSet.ObjectToViewCoor(rEntityBucket[nEntityCounter++].Point());
            aNew.append(basegfx::B2DPoint(aPoint.getX(), aPoint.getY()));
        }

        // Teilprimitiv einfuegen
        aRetval.append(aNew);
    }

    // add shadow now too (#61279#)
    basegfx::B2DPolyPolygon aShadowPolyPoly(ImpGetShadowPolygon());

    // invert Y coor cause of GetPolygon() later
    basegfx::B2DHomMatrix aTransMat;
    aTransMat.scale(1.0, -1.0);
    aShadowPolyPoly.transform(aTransMat);
    aRetval.append(aShadowPolyPoly);

    return aRetval;
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
        GetScene()->GetCameraSet().SetObjectTrans(GetFullTransform());

        // Schattenpolygon holen
        const basegfx::B2DPolyPolygon aShadowPoly(ImpGetShadowPolygon());

        // invert Y coor cause of GetPolyPolygon() in ImpDrawShadowPolygon() later
        // ##not necessary with B2DPolygons
        //basegfx::B2DHomMatrix aTransMat;
        //aTransMat.scale(1.0, -1.0);
        //aShadowPoly.transform(aTransMat);

        // ...und Zeichnen
        ImpDrawShadowPolygon(aShadowPoly, rXOut);
    }
}

basegfx::B2DPolyPolygon E3dCompoundObject::ImpGetShadowPolygon() const
{
    basegfx::B2DPolyPolygon aRetval;

    // #79585#
    sal_Int32 nXDist(GetShadowXDistance());
    sal_Int32 nYDist(GetShadowYDistance());
    BOOL bDrawAsOutline(DrawShadowAsOutline());
    basegfx::B3DPolyPolygon aLinePolyPolygon;
    B3dTransformationSet& rTransSet = GetScene()->GetCameraSet();
    const SfxItemSet& rSet = GetObjectItemSet();
    XLineStyle aLineStyle = ((const XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
    //sal_Int32 nLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();

    if(!bDrawAsOutline)
    {
        // add basic polygon geometry, generate from 3D bucket
        const B3dEntityBucket& rEntityBucket = GetDisplayGeometry().GetEntityBucket();
        const GeometryIndexValueBucket& rIndexBucket = GetDisplayGeometry().GetIndexBucket();
        sal_uInt32 nPolyCounter(0);
        sal_uInt32 nEntityCounter(0);

        while(nPolyCounter < rIndexBucket.Count())
        {
            // next primitive
            sal_uInt32 nUpperBound(rIndexBucket[nPolyCounter++].GetIndex());
            basegfx::B3DPolygon aNewPolygon;

            while(nEntityCounter < nUpperBound)
            {
                aNewPolygon.append(rEntityBucket[nEntityCounter++].Point());
            }

            aNewPolygon.setClosed(true);
            aLinePolyPolygon.append(aNewPolygon);
        }
    }

    if(bDrawAsOutline || (XLINE_NONE != aLineStyle))
    {
        // get line geometry
        basegfx::B3DPolyPolygon aBasicLinePolyPoly(Get3DLineGeometry());
        basegfx::B3DPolyPolygon aAreaPolyPolygon;
        basegfx::B3DPolyPolygon aNormalPolyPolygon;
        ImplGet3DLineGeometry(rSet, aBasicLinePolyPoly, aAreaPolyPolygon, aNormalPolyPolygon);

        if(aAreaPolyPolygon.count())
        {
            if(aNormalPolyPolygon.count())
            {
                // draw as 3d tubes
                for(sal_uInt32 a(0L); a < aAreaPolyPolygon.count(); a++)
                {
                    // start new primitive
                    basegfx::B3DPolygon aPolygon(aAreaPolyPolygon.getB3DPolygon(a));
                    basegfx::B3DPolygon aNewPolygon;

                    aNewPolygon.append(aPolygon.getB3DPoint(1L));
                    aNewPolygon.append(aPolygon.getB3DPoint(3L));
                    aNewPolygon.append(aPolygon.getB3DPoint(4L));
                    aNewPolygon.append(aPolygon.getB3DPoint(2L));
                    aNewPolygon.setClosed(true);

                    aLinePolyPolygon.append(aNewPolygon);
                }
            }
            else
            {
                // draw as lines
                for(sal_uInt32 a(0L); a < aAreaPolyPolygon.count(); a++)
                {
                    basegfx::B3DPolygon aNewPolygon(aAreaPolyPolygon.getB3DPolygon(a));

                    if(aNewPolygon.isClosed() && aNewPolygon.count())
                    {
                        // use open polygon to force to line draw, close by hand
                        aNewPolygon.setClosed(false);

                        if(aNewPolygon.getB3DPoint(0L) != aNewPolygon.getB3DPoint(aNewPolygon.count() - 1L))
                        {
                            aNewPolygon.append(aNewPolygon.getB3DPoint(0L));
                        }
                    }

                    aLinePolyPolygon.append(aNewPolygon);
                }
            }
        }
    }

    if(aLinePolyPolygon.count())
    {
        if(GetShadow3D())
        {
            // 3D Schatten. Nimm Lichtquelle und Ebene. Projiziere
            // die Punkte und jage sie durch die 3D Darstellung.
            basegfx::B3DVector aLampPositionOrDirection;
            BOOL bDirectionalSource(TRUE);
            basegfx::B3DVector aGroundPosition;
            basegfx::B3DVector aGroundDirection;
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
                    aLampPositionOrDirection.normalize();
                }
                else
                {
                    // Nur Position vorhanden
                    aLampPositionOrDirection = rLightGroup.GetPosition(aLightNumber);
                    bDirectionalSource = FALSE;
                }

                // Ebene holen, Richtung in Augkoordinaten
                aGroundDirection = -GetScene()->GetShadowPlaneDirection();
                aGroundDirection.normalize();

                // Ist die Lampe auch vor der Ebene?
                basegfx::B3DVector aLightNormal(aLampPositionOrDirection);
                if(!bDirectionalSource)
                {
                    // Nur Position vorhanden, berechne einen Lichtvektor
                    const basegfx::B3DPoint aPolyNullPoint(aLinePolyPolygon.getB3DPolygon(0L).getB3DPoint(0L));
                    aLightNormal = aPolyNullPoint - aLampPositionOrDirection;
                    aLightNormal.normalize();
                }

                double fLightAndNormal = aLightNormal.scalar(aGroundDirection);
                basegfx::B3DRange aVolume(rTransSet.GetDeviceVolume());

                // auf Augkoordinaten umstellen
                basegfx::B3DPoint aOldMin(aVolume.getMinimum());
                basegfx::B3DPoint aOldMax(aVolume.getMaximum());
                double fTemp = aOldMin.getZ();
                aOldMin.setZ(-aOldMax.getZ());
                aOldMax.setZ(-fTemp);
                aVolume = basegfx::B3DRange(aOldMin, aOldMax);

                if(fLightAndNormal > 0.0)
                {
                    // Position der Ebene in Augkoordinaten setzen
                    aGroundPosition.setX((aGroundDirection.getX() < 0.0) ? aVolume.getMinX() : aVolume.getMaxX());
                    aGroundPosition.setY((aGroundDirection.getY() < 0.0) ? aVolume.getMinY() : aVolume.getMaxY());
                    aGroundPosition.setZ(aVolume.getMinZ() - ((aVolume.getMaxZ() - aVolume.getMinZ()) / 8.0));

                    // Skalar der Ebenengleichung holen
                    const double fGroundScalar(-aGroundPosition.scalar(aGroundDirection));

                    // ObjectTrans setzen
                    BOOL bPolygonVisible(TRUE);
                    rTransSet.SetObjectTrans(GetFullTransform());

                    for(sal_uInt32 a(0L); a < aLinePolyPolygon.count(); a++)
                    {
                        basegfx::B3DPolygon aLinePoly = aLinePolyPolygon.getB3DPolygon(a);
                        basegfx::B2DPolygon aPoly;

                        for(sal_uInt32 b(0L); b < aLinePoly.count(); b++)
                        {
                            // Naechsten Punkt holen
                            basegfx::B3DPoint aPoint(aLinePoly.getB3DPoint(b));

                            // Auf Augkoordinaten umrechnen
                            aPoint = rTransSet.ObjectToEyeCoor(aPoint);

                            // Richtung bestimmen
                            basegfx::B3DVector aDirection(aLampPositionOrDirection);
                            if(!bDirectionalSource)
                            {
                                aDirection = aPoint - aLampPositionOrDirection;
                                aDirection.normalize();
                            }

                            // Schnittpunkt berechnen (N.D)
                            double fDiv = aGroundDirection.scalar(aDirection);
                            if(fabs(fDiv) < SMALL_DVALUE)
                            {
                                bPolygonVisible = FALSE;
                            }
                            else
                            {
                                fDiv = -((fGroundScalar + aGroundDirection.scalar(aPoint)) / fDiv);
                                aPoint += aDirection * fDiv;
                            }

                            // Punkt normal transformieren
                            if(bPolygonVisible)
                            {
                                // Auf ViewKoordinaten
                                basegfx::B3DPoint aShadowPoint(rTransSet.EyeToViewCoor(aPoint));
                                aPoly.append(basegfx::B2DPoint(
                                    aShadowPoint.getX() + (double)nXDist,
                                    aShadowPoint.getY() + (double)nYDist));
                            }
                        }

                        // Teilpolygon einfuegen
                        if(aPoly.count())
                        {
                            aPoly.setClosed(aLinePoly.isClosed());
                            aRetval.append(aPoly);
                        }
                    }
                }
            }
        }
        else
        {
            // ObjectTrans setzen
            rTransSet.SetObjectTrans(GetFullTransform());

            for(sal_uInt32 a(0L); a < aLinePolyPolygon.count(); a++)
            {
                basegfx::B3DPolygon aLinePoly(aLinePolyPolygon.getB3DPolygon(a));
                basegfx::B2DPolygon aPoly;

                // Polygon fuellen
                for(sal_uInt32 b(0L); b < aLinePoly.count(); b++)
                {
                    // Naechsten Punkt holen
                    basegfx::B3DPoint aPoint(aLinePoly.getB3DPoint(b));
                    aPoint = rTransSet.ObjectToViewCoor(aPoint);
                    aPoly.append(basegfx::B2DPoint(
                        aPoint.getX() + (double)nXDist,
                        aPoint.getY() + (double)nYDist));
                }

                // open/close
                aPoly.setClosed(aLinePoly.isClosed());

                // Teilpolygon einfuegen
                aRetval.append(aPoly);
            }
        }
    }

    return aRetval;
}

void E3dCompoundObject::ImpDrawShadowPolygon(const basegfx::B2DPolyPolygon& rPoly, XOutputDevice& rXOut)
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
            PolyPolygon aPolyPolygon(rPoly);
            Rectangle aBound(aPolyPolygon.GetBoundRect());

            // prepare VDev and MetaFile
            aVDev.EnableOutput(FALSE);
            aVDev.SetMapMode(rXOut.GetOutDev()->GetMapMode());
            aMetaFile.Record(&aVDev);

            aVDev.SetFont(rXOut.GetOutDev()->GetFont());
            aVDev.SetDrawMode(rXOut.GetOutDev()->GetDrawMode());
            aVDev.SetSettings(rXOut.GetOutDev()->GetSettings());
            aVDev.SetRefPoint(rXOut.GetOutDev()->GetRefPoint());

            // create output
            for(UINT16 a(0); a < aPolyPolygon.Count(); a++)
            {
                if(rPoly.getB2DPolygon(a).isClosed())
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
        for(sal_uInt32 a(0L); a < rPoly.count(); a++)
        {
            const basegfx::B2DPolygon aPoly(rPoly.getB2DPolygon(a));

            if(aPoly.isClosed())
            {
                pDevice->SetLineColor();
                pDevice->SetFillColor(aCol);
            }
            else
            {
                pDevice->SetLineColor(aCol);
                pDevice->SetFillColor();
            }

            // #i74631# Polygon constructor with B2DPolygon is now explicit; found a bug here
            const Polygon aPolygon(aPoly);
            pDevice->DrawPolygon(aPolygon);
        }
    }
}

/*************************************************************************
|*
|* convert given basegfx::B3DPolyPolygon to screen coor
|*
\************************************************************************/

basegfx::B2DPolyPolygon E3dCompoundObject::TransformToScreenCoor(const basegfx::B3DPolyPolygon& rCandidate)
{
    basegfx::B2DPolyPolygon aRetval;
    B3dTransformationSet& rTransSet = GetScene()->GetCameraSet();
    const basegfx::B3DHomMatrix mTransform(GetFullTransform() * rTransSet.GetMatFromWorldToView());

    for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
    {
        const basegfx::B3DPolygon aCandidate(rCandidate.getB3DPolygon(a));
        const sal_uInt32 nCount(aCandidate.count());

        if(nCount)
        {
            basegfx::B2DPolygon aTempPoly;

            for(sal_uInt32 b(0L); b < nCount; b++)
            {
                basegfx::B3DPoint aPoint(aCandidate.getB3DPoint(b));
                aPoint *= mTransform;
                aTempPoly.append(basegfx::B2DPoint(aPoint.getX(), aPoint.getY()));
            }

            aRetval.append(aTempPoly);
        }
    }

    return aRetval;
}

// #110988#
double E3dCompoundObject::GetMinimalDepthInViewCoor(E3dScene& rScene) const
{
    double fRetval(DBL_MAX);
    B3dTransformationSet& rTransSet = rScene.GetCameraSet();
    basegfx::B3DHomMatrix mTransform = GetFullTransform();
    rTransSet.SetObjectTrans(mTransform);
    const B3dEntityBucket& rEntityBucket = GetDisplayGeometry().GetEntityBucket();
    const GeometryIndexValueBucket& rIndexBucket = GetDisplayGeometry().GetIndexBucket();
    sal_uInt32 nPolyCounter(0L);
    sal_uInt32 nEntityCounter(0L);

    while(nPolyCounter < rIndexBucket.Count())
    {
        sal_uInt32 nUpperBound(rIndexBucket[nPolyCounter++].GetIndex());

        while(nEntityCounter < nUpperBound)
        {
            basegfx::B3DPoint aNewPoint(rEntityBucket[nEntityCounter++].Point());
            aNewPoint = rTransSet.ObjectToViewCoor(aNewPoint);

            if(aNewPoint.getZ() < fRetval)
            {
                fRetval = aNewPoint.getZ();
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
