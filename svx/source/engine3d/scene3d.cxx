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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include "svx/svditer.hxx"

#if defined( UNX ) || defined( ICC )
#include <stdlib.h>
#endif
#include "svx/globl3d.hxx"
#include <svx/svdpage.hxx>
#include <svl/style.hxx>
#include <svx/scene3d.hxx>
#include <svx/e3dundo.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svxids.hrc>
#include <editeng/colritem.hxx>
#include <svx/e3ditem.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/svx3ditems.hxx>
#include <svl/whiter.hxx>
#include <svx/xflftrit.hxx>
#include <svx/sdr/properties/e3dsceneproperties.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <svx/svddrag.hxx>
#include <helperminimaldepth3d.hxx>
#include <algorithm>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/e3dsceneupdater.hxx>

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

//////////////////////////////////////////////////////////////////////////////
// #110988#

class ImpRemap3DDepth
{
    sal_uInt32                  mnOrdNum;
    double                      mfMinimalDepth;

    // bitfield
    unsigned                    mbIsScene : 1;

public:
    ImpRemap3DDepth(sal_uInt32 nOrdNum, double fMinimalDepth);
    ImpRemap3DDepth(sal_uInt32 nOrdNum);
    ~ImpRemap3DDepth();

    // for ::std::sort
    bool operator<(const ImpRemap3DDepth& rComp) const;

    sal_uInt32 GetOrdNum() const { return mnOrdNum; }
    sal_Bool IsScene() const { return mbIsScene; }
};

ImpRemap3DDepth::ImpRemap3DDepth(sal_uInt32 nOrdNum, double fMinimalDepth)
:   mnOrdNum(nOrdNum),
    mfMinimalDepth(fMinimalDepth),
    mbIsScene(sal_False)
{
}

ImpRemap3DDepth::ImpRemap3DDepth(sal_uInt32 nOrdNum)
:   mnOrdNum(nOrdNum),
    mfMinimalDepth(0.0),
    mbIsScene(sal_True)
{
}

ImpRemap3DDepth::~ImpRemap3DDepth()
{
}

bool ImpRemap3DDepth::operator<(const ImpRemap3DDepth& rComp) const
{
    if(IsScene())
    {
        return sal_False;
    }
    else
    {
        if(rComp.IsScene())
        {
            return sal_True;
        }
        else
        {
            return mfMinimalDepth < rComp.mfMinimalDepth;
        }
    }
}

// typedefs for a vector of ImpRemap3DDepths
typedef ::std::vector< ImpRemap3DDepth > ImpRemap3DDepthVector;

//////////////////////////////////////////////////////////////////////////////
// #110988#

class Imp3DDepthRemapper
{
    ImpRemap3DDepthVector       maVector;

public:
    Imp3DDepthRemapper(E3dScene& rScene);
    ~Imp3DDepthRemapper();

    sal_uInt32 RemapOrdNum(sal_uInt32 nOrdNum) const;
};

Imp3DDepthRemapper::Imp3DDepthRemapper(E3dScene& rScene)
{
    // only called when rScene.GetSubList() and nObjCount > 1L
    SdrObjList* pList = rScene.GetSubList();
    const sal_uInt32 nObjCount(pList->GetObjCount());

    for(sal_uInt32 a(0L); a < nObjCount; a++)
    {
        SdrObject* pCandidate = pList->GetObj(a);

        if(pCandidate)
        {
            if(pCandidate->ISA(E3dCompoundObject))
            {
                // single 3d object, calc depth
                const double fMinimalDepth(getMinimalDepthInViewCoordinates(static_cast< const E3dCompoundObject& >(*pCandidate)));
                ImpRemap3DDepth aEntry(a, fMinimalDepth);
                maVector.push_back(aEntry);
            }
            else
            {
                // scene, use standard entry for scene
                ImpRemap3DDepth aEntry(a);
                maVector.push_back(aEntry);
            }
        }
    }

    // now, we need to sort the maVector by it's members minimal depth. The
    // smaller, the nearer to the viewer.
    ::std::sort(maVector.begin(), maVector.end());
}

Imp3DDepthRemapper::~Imp3DDepthRemapper()
{
}

sal_uInt32 Imp3DDepthRemapper::RemapOrdNum(sal_uInt32 nOrdNum) const
{
    if(nOrdNum < maVector.size())
    {
        nOrdNum = maVector[(maVector.size() - 1) - nOrdNum].GetOrdNum();
    }

    return nOrdNum;
}

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* E3dScene::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dSceneProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// #110094# DrawContact section

sdr::contact::ViewContact* E3dScene::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfE3dScene(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dScene, E3dObject);

/*************************************************************************
|*
|* E3dScene-Konstruktor
|*
\************************************************************************/

E3dScene::E3dScene()
:   E3dObject(),
    aCamera(basegfx::B3DPoint(0.0, 0.0, 4.0), basegfx::B3DPoint()),
    mp3DDepthRemapper(0L),
    bDrawOnlySelected(false)
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

E3dScene::E3dScene(E3dDefaultAttributes& rDefault)
:   E3dObject(),
    aCamera(basegfx::B3DPoint(0.0, 0.0, 4.0), basegfx::B3DPoint()),
    mp3DDepthRemapper(0L),
    bDrawOnlySelected(false)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);
}

void E3dScene::SetDefaultAttributes(E3dDefaultAttributes& /*rDefault*/)
{
    // Fuer OS/2 die FP-Exceptions abschalten
#if defined(OS2)
#define SC_FPEXCEPTIONS_ON()    _control87( MCW_EM, 0 )
#define SC_FPEXCEPTIONS_OFF()   _control87( MCW_EM, MCW_EM )
    SC_FPEXCEPTIONS_OFF();
#endif

    // Fuer WIN95/NT die FP-Exceptions abschalten
#if defined(WNT)
#define SC_FPEXCEPTIONS_ON()    _control87( _MCW_EM, 0 )
#define SC_FPEXCEPTIONS_OFF()   _control87( _MCW_EM, _MCW_EM )
    SC_FPEXCEPTIONS_OFF();
#endif

    // Defaults setzen
    aCamera.SetViewWindow(-2, -2, 4, 4);
    aCameraSet.SetDeviceRectangle(-2, 2, -2, 2);
    aCamera.SetDeviceWindow(Rectangle(0, 0, 10, 10));
    Rectangle aRect(0, 0, 10, 10);
    aCameraSet.SetViewportRectangle(aRect);

    // set defaults for Camera from ItemPool
    aCamera.SetProjection(GetPerspective());
    basegfx::B3DPoint aActualPosition(aCamera.GetPosition());
    double fNew = GetDistance();

    if(fabs(fNew - aActualPosition.getZ()) > 1.0)
    {
        aCamera.SetPosition( basegfx::B3DPoint( aActualPosition.getX(), aActualPosition.getY(), fNew) );
    }

    fNew = GetFocalLength() / 100.0;
    aCamera.SetFocalLength(fNew);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

E3dScene::~E3dScene()
{
    // #110988#
    ImpCleanup3DDepthMapper();
}

basegfx::B2DPolyPolygon E3dScene::TakeXorPoly() const
{
    const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(GetViewContact());
    const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());
    const basegfx::B3DPolyPolygon aCubePolyPolygon(CreateWireframe());

    basegfx::B2DPolyPolygon aRetval(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(aCubePolyPolygon,
        aViewInfo3D.getObjectToView()));
    aRetval.transform(rVCScene.getObjectTransformation());

    return aRetval;
}

// #110988#
void E3dScene::ImpCleanup3DDepthMapper()
{
    if(mp3DDepthRemapper)
    {
        delete mp3DDepthRemapper;
        mp3DDepthRemapper = 0L;
    }
}

// #110988#
sal_uInt32 E3dScene::RemapOrdNum(sal_uInt32 nNewOrdNum) const
{
    if(!mp3DDepthRemapper)
    {
        const sal_uInt32 nObjCount(GetSubList() ? GetSubList()->GetObjCount() : 0L);

        if(nObjCount > 1L)
        {
            ((E3dScene*)this)->mp3DDepthRemapper = new Imp3DDepthRemapper((E3dScene&)(*this));
        }
    }

    if(mp3DDepthRemapper)
    {
        return mp3DDepthRemapper->RemapOrdNum(nNewOrdNum);
    }

    return nNewOrdNum;
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

sal_uInt16 E3dScene::GetObjIdentifier() const
{
    return E3D_SCENE_ID;
}

void E3dScene::SetBoundRectDirty()
{
    E3dScene* pScene = GetScene();

    if(pScene == this)
    {
        // avoid resetting aOutRect which in case of a 3D scene used as 2d object
        // is model data,not re-creatable view data
    }
    else
    {
        // if not the outmost scene it is used as group in 3d, call parent
        E3dObject::SetBoundRectDirty();
    }
}

/*************************************************************************
|*
|* SetSnapRect
|*
\************************************************************************/

void E3dScene::NbcSetSnapRect(const Rectangle& rRect)
{
    SetRectsDirty();
    E3dObject::NbcSetSnapRect(rRect);
    aCamera.SetDeviceWindow(rRect);
    aCameraSet.SetViewportRectangle((Rectangle&)rRect);

    // #110988#
    ImpCleanup3DDepthMapper();
}

/*************************************************************************
|*
|* Objekt verschieben
|*
\************************************************************************/

void E3dScene::NbcMove(const Size& rSize)
{
    Rectangle aNewSnapRect = GetSnapRect();
    MoveRect(aNewSnapRect, rSize);
    NbcSetSnapRect(aNewSnapRect);
}

/*************************************************************************
|*
|* Objekt Resizen
|*
\************************************************************************/

void E3dScene::NbcResize(const Point& rRef, const Fraction& rXFact,
                                            const Fraction& rYFact)
{
    Rectangle aNewSnapRect = GetSnapRect();
    ResizeRect(aNewSnapRect, rRef, rXFact, rYFact);
    NbcSetSnapRect(aNewSnapRect);
}

/*************************************************************************
|*
|* Neue Kamera setzen, und dabei die Szene und ggf. das BoundVolume
|* als geaendert markieren
|*
\************************************************************************/

void E3dScene::SetCamera(const Camera3D& rNewCamera)
{
    // Alte Kamera setzen
    aCamera = rNewCamera;
    ((sdr::properties::E3dSceneProperties&)GetProperties()).SetSceneItemsFromCamera();

    SetRectsDirty();

    // Neue Kamera aus alter fuellen
    Camera3D& rCam = (Camera3D&)GetCamera();

    // Ratio abschalten
    if(rCam.GetAspectMapping() == AS_NO_MAPPING)
        GetCameraSet().SetRatio(0.0);

    // Abbildungsgeometrie setzen
    basegfx::B3DPoint aVRP(rCam.GetViewPoint());
    basegfx::B3DVector aVPN(aVRP - rCam.GetVRP());
    basegfx::B3DVector aVUV(rCam.GetVUV());

    // #91047# use SetViewportValues() to set VRP, VPN and VUV as vectors, too.
    // Else these values would not be exported/imported correctly.
    GetCameraSet().SetViewportValues(aVRP, aVPN, aVUV);

    // Perspektive setzen
    GetCameraSet().SetPerspective(rCam.GetProjection() == PR_PERSPECTIVE);
    GetCameraSet().SetViewportRectangle((Rectangle&)rCam.GetDeviceWindow());

    // #110988#
    ImpCleanup3DDepthMapper();
}

/*************************************************************************
|*
|* 3D-Objekt einfuegen
|*
\************************************************************************/

void E3dScene::NewObjectInserted(const E3dObject* p3DObj)
{
    E3dObject::NewObjectInserted(p3DObj);

    if ( p3DObj == this )
        return;

    // #110988#
    ImpCleanup3DDepthMapper();
}

/*************************************************************************
|*
|* Parent ueber Aenderung eines Childs informieren
|*
\************************************************************************/

void E3dScene::StructureChanged()
{
    E3dObject::StructureChanged();
    SetRectsDirty();

    // #110988#
    ImpCleanup3DDepthMapper();
}

/*************************************************************************
|*
|* Uebergeordnetes Szenenobjekt bestimmen
|*
\************************************************************************/

E3dScene* E3dScene::GetScene() const
{
    if(GetParentObj())
        return GetParentObj()->GetScene();
    else
        return (E3dScene*)this;
}

void E3dScene::removeAllNonSelectedObjects()
{
    E3DModifySceneSnapRectUpdater aUpdater(this);

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        SdrObject* pObj = maSubList.GetObj(a);

        if(pObj)
        {
            bool bRemoveObject(false);

            if(pObj->ISA(E3dScene))
            {
                E3dScene* pScene = (E3dScene*)pObj;

                // iterate over this sub-scene
                pScene->removeAllNonSelectedObjects();

                // check object count. Empty scenes can be deleted
                const sal_uInt32 nObjCount(pScene->GetSubList() ? pScene->GetSubList()->GetObjCount() : 0);

                if(!nObjCount)
                {
                    // all objects removed, scene can be removed, too
                    bRemoveObject = true;
                }
            }
            else if(pObj->ISA(E3dCompoundObject))
            {
                E3dCompoundObject* pCompound = (E3dCompoundObject*)pObj;

                if(!pCompound->GetSelected())
                {
                    bRemoveObject = true;
                }
            }

            if(bRemoveObject)
            {
                maSubList.NbcRemoveObject(pObj->GetOrdNum());
                a--;
                SdrObject::Free(pObj);
            }
        }
    }
}

E3dScene* E3dScene::Clone() const
{
    return CloneHelper< E3dScene >();
}

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

E3dScene& E3dScene::operator=(const E3dScene& rObj)
{
    if( this == &rObj )
        return *this;
    E3dObject::operator=(rObj);

    const E3dScene& r3DObj = (const E3dScene&) rObj;
    aCamera          = r3DObj.aCamera;

    // neu ab 377:
    aCameraSet = r3DObj.aCameraSet;
    ((sdr::properties::E3dSceneProperties&)GetProperties()).SetSceneItemsFromCamera();

    // SetSnapRect(r3DObj.GetSnapRect());
    InvalidateBoundVolume();
    RebuildLists();
    SetRectsDirty();

    // #110988#
    ImpCleanup3DDepthMapper();

    // #i101941#
    // After a Scene as model object is cloned, the used
    // ViewContactOfE3dScene is created and partially used
    // to calculate Bound/SnapRects, but - since quite some
    // values are buffered at the VC - not really well
    // initialized. It would be possible to always watch for
    // preconditions of buffered data, but this would be expensive
    // and would create a lot of short living data structures.
    // It is currently better to flush that data, e.g. by using
    // ActionChanged at the VC which will for this class
    // flush that cached data and initalize it's valid reconstruction
    GetViewContact().ActionChanged();
    return *this;
}

/*************************************************************************
|*
|* Licht- und Labelobjektlisten neu aufbauen (nach Laden, Zuweisung)
|*
\************************************************************************/

void E3dScene::RebuildLists()
{
    // zuerst loeschen
    SdrLayerID nCurrLayerID = GetLayer();

    SdrObjListIter a3DIterator(maSubList, IM_FLAT);

    // dann alle Objekte in der Szene pruefen
    while ( a3DIterator.IsMore() )
    {
        E3dObject* p3DObj = (E3dObject*) a3DIterator.Next();
        p3DObj->NbcSetLayer(nCurrLayerID);
        NewObjectInserted(p3DObj);
    }
}

/*************************************************************************
|*
|* erstelle neues GeoData-Objekt
|*
\************************************************************************/

SdrObjGeoData *E3dScene::NewGeoData() const
{
    return new E3DSceneGeoData;
}

/*************************************************************************
|*
|* uebergebe aktuelle werte an das GeoData-Objekt
|*
\************************************************************************/

void E3dScene::SaveGeoData(SdrObjGeoData& rGeo) const
{
    E3dObject::SaveGeoData (rGeo);

    ((E3DSceneGeoData &) rGeo).aCamera = aCamera;
}

/*************************************************************************
|*
|* uebernehme werte aus dem GeoData-Objekt
|*
\************************************************************************/

void E3dScene::RestGeoData(const SdrObjGeoData& rGeo)
{
    // #i94832# removed E3DModifySceneSnapRectUpdater here.
    // It should not be needed, is already part of E3dObject::RestGeoData
    E3dObject::RestGeoData (rGeo);
    SetCamera (((E3DSceneGeoData &) rGeo).aCamera);
}

/*************************************************************************
|*
|* Am StyleSheet wurde etwas geaendert, also Scene aendern
|*
\************************************************************************/

void E3dScene::Notify(SfxBroadcaster &rBC, const SfxHint  &rHint)
{
    SetRectsDirty();
    E3dObject::Notify(rBC, rHint);
}

/*************************************************************************
|*
\************************************************************************/

void E3dScene::RotateScene (const Point& rRef, long /*nWink*/, double sn, double cs)
{
    Point UpperLeft, LowerRight, Center, NewCenter;

    UpperLeft = aOutRect.TopLeft();
    LowerRight = aOutRect.BottomRight();

    long dxOutRectHalf = labs(UpperLeft.X() - LowerRight.X());
    dxOutRectHalf /= 2;
    long dyOutRectHalf = labs(UpperLeft.Y() - LowerRight.Y());
    dyOutRectHalf /= 2;

    Rectangle RectQuelle(aOutRect), RectZiel(aOutRect);

       // Nur der Mittelpunkt wird bewegt. Die Ecken werden von NbcMove bewegt.
       // Fuer das Drehen wird von mir ein kartesisches Koordinatensystem verwendet in dem der Drehpunkt
       // der Nullpunkt ist und die Y- Achse nach oben ansteigt, die X-Achse nach rechts.
       // Dies muss bei den Y-Werten beachtet werden. (Auf dem Blatt zeigt die Y-Achse nach unten
    Center.X() = (UpperLeft.X() + dxOutRectHalf) - rRef.X();
    Center.Y() = -((UpperLeft.Y() + dyOutRectHalf) - rRef.Y());
                  // Ein paar Spezialfaelle zuerst abhandeln (n*90 Grad n ganzzahlig)
    if (sn==1.0 && cs==0.0) { // 90deg
        NewCenter.X() = -Center.Y();
        NewCenter.Y() = -Center.X();
    } else if (sn==0.0 && cs==-1.0) { // 180deg
        NewCenter.X() = -Center.X();
        NewCenter.Y() = -Center.Y();
    } else if (sn==-1.0 && cs==0.0) { // 270deg
        NewCenter.X() =  Center.Y();
        NewCenter.Y() = -Center.X();
    }
    else          // Hier wird um einen beliebigen Winkel in mathematisch positiver Richtung gedreht!
    {             // xneu = x * cos(alpha) - y * sin(alpha)
                  // yneu = x * sin(alpha) + y * cos(alpha)
                  // Unten Rechts wird nicht gedreht: die Seiten von RectQuelle muessen parallel
                  // zu den Koordinatenachsen bleiben.
        NewCenter.X() = (long) (Center.X() * cs - Center.Y() * sn);
        NewCenter.Y() = (long) (Center.X() * sn + Center.Y() * cs);
    }

    Size Differenz;
    Point DiffPoint = (NewCenter - Center);
    Differenz.Width() = DiffPoint.X();
    Differenz.Height() = -DiffPoint.Y();  // Man beachte dass die Y-Achse nach unten positiv gezaehlt wird.
    NbcMove (Differenz);  // fuehrt die eigentliche Koordinatentransformation durch.
}

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

void E3dScene::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulScene3d);

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

void E3dScene::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralScene3d);
}

/*************************************************************************
|*
|* Die NbcRotate-Routine ueberlaedt die des SdrObject. Die Idee ist die Scene
|* drehen zu koennen und relativ zur Lage der Scene dann auch die Objekte
|* in der Scene
|*
\************************************************************************/

void E3dScene::NbcSetTransform(const basegfx::B3DHomMatrix& rMatrix)
{
    if(maTransformation != rMatrix)
    {
        // call parent
        E3dObject::NbcSetTransform(rMatrix);
    }
}

void E3dScene::SetTransform(const basegfx::B3DHomMatrix& rMatrix)
{
    if(rMatrix != maTransformation)
    {
        // call parent
        E3dObject::SetTransform(rMatrix);
    }
}

void E3dScene::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    // Also derzeit sind die Klebepunkte relativ zum aOutRect der Szene definiert. Vor dem Drehen
    // werden die Klebepunkte relativ zur Seite definiert. Sie nehmen an der Drehung der Szene noch nicht Teil
    // dafuer gibt es den
    SetGlueReallyAbsolute(sal_True);

    // So dass war die Szene, ab jetzt kommen die Objekte in der Szene
    // 3D-Objekte gibt es nur ein einziges das kann zwar mehrere Flaechen haben aber die Flaechen
    // muessen ja nicht zusammenhaengend sein
    // es ermoeglicht den Zugriff auf Kindobjekte
    // Ich gehe also die gesamte Liste durch und rotiere um die Z-Achse die durch den
    // Mittelpunkt von aOutRect geht (Satz von Steiner), also RotateZ

    RotateScene (rRef, nWink, sn, cs);  // Rotiert die Szene
    double fWinkelInRad = nWink/100 * F_PI180;

    basegfx::B3DHomMatrix aRotation;
    aRotation.rotate(0.0, 0.0, fWinkelInRad);
    NbcSetTransform(aRotation * GetTransform());

    SetRectsDirty();    // Veranlasst eine Neuberechnung aller BoundRects
    NbcRotateGluePoints(rRef,nWink,sn,cs);  // Rotiert die Klebepunkte (die haben noch Koordinaten relativ
                                            // zum Urpsung des Blattes
    SetGlueReallyAbsolute(sal_False);  // ab jetzt sind sie wieder relativ zum BoundRect (also dem aOutRect definiert)
    SetRectsDirty();
}

/*************************************************************************
|*
|* SnapRect berechnen
|*
\************************************************************************/

void E3dScene::RecalcSnapRect()
{
    E3dScene* pScene = GetScene();

    if(pScene == this)
    {
        // Szene wird als 2D-Objekt benutzt, nimm SnapRect aus der
        // 2D Bildschrimdarstellung
        Camera3D& rCam = (Camera3D&)pScene->GetCamera();
        maSnapRect = rCam.GetDeviceWindow();
    }
    else
    {
        // Szene ist selbst Mitglied einer anderen Szene, hole das
        // SnapRect als zusammengesetztes Objekt
        E3dObject::RecalcSnapRect();
    }
}

/*************************************************************************
|*
|* Aufbrechen
|*
\************************************************************************/

sal_Bool E3dScene::IsBreakObjPossible()
{
    // Szene ist aufzubrechen, wenn alle Mitglieder aufzubrechen sind
    SdrObjListIter a3DIterator(maSubList, IM_DEEPWITHGROUPS);

    while ( a3DIterator.IsMore() )
    {
        E3dObject* pObj = (E3dObject*) a3DIterator.Next();
        DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
        if(!pObj->IsBreakObjPossible())
            return sal_False;
    }

    return sal_True;
}

basegfx::B3DVector E3dScene::GetShadowPlaneDirection() const
{
    double fWink = (double)GetShadowSlant() * F_PI180;
    basegfx::B3DVector aShadowPlaneDir(0.0, sin(fWink), cos(fWink));
    aShadowPlaneDir.normalize();
    return aShadowPlaneDir;
}

void E3dScene::SetShadowPlaneDirection(const basegfx::B3DVector& rVec)
{
    sal_uInt16 nSceneShadowSlant = (sal_uInt16)((atan2(rVec.getY(), rVec.getZ()) / F_PI180) + 0.5);
    GetProperties().SetObjectItemDirect(Svx3DShadowSlantItem(nSceneShadowSlant));
}

basegfx::B2DPolyPolygon E3dScene::TakeCreatePoly(const SdrDragStat& /*rDrag*/) const
{
    return TakeXorPoly();
}

bool E3dScene::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    NbcSetSnapRect(aRect1);
    return sal_True;
}

bool E3dScene::MovCreate(SdrDragStat& rStat)
{
    Rectangle aRect1;
    rStat.TakeCreateRect(aRect1);
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    NbcSetSnapRect(aRect1);
    SetBoundRectDirty();
    bSnapRectDirty=sal_True;
    return sal_True;
}

bool E3dScene::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    Rectangle aRect1;
    rStat.TakeCreateRect(aRect1);
    aRect1.Justify();
    NbcSetSnapRect(aRect1);
    SetRectsDirty();
    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

bool E3dScene::BckCreate(SdrDragStat& /*rStat*/)
{
    return sal_False;
}

void E3dScene::BrkCreate(SdrDragStat& /*rStat*/)
{
}

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
