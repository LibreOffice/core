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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdstr.hrc>
#include <svx/svdglob.hxx>
#include <svx/svditer.hxx>
#if defined( UNX ) || defined( ICC )
#include <stdlib.h>
#endif
#include <svx/globl3d.hxx>
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
#include <svx/svdlegacy.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/svdmodel.hxx>

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

//////////////////////////////////////////////////////////////////////////////
// #110988#

class ImpRemap3DDepth
{
    sal_uInt32                  mnOrdNum;
    double                      mfMinimalDepth;

    // bitfield
    bool                        mbIsScene : 1;

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
    SdrObjList* pList = rScene.getChildrenOfSdrObject();

    if(pList)
    {
        const sal_uInt32 nObjCount(pList->GetObjCount());

        for(sal_uInt32 a(0L); a < nObjCount; a++)
        {
            SdrObject* pCandidate = pList->GetObj(a);

            if(pCandidate)
            {
                    E3dCompoundObject* pE3dCompoundObject = dynamic_cast< E3dCompoundObject* >(pCandidate);

                    if(pE3dCompoundObject)
                {
                    // single 3d object, calc depth
                        const double fMinimalDepth(getMinimalDepthInViewCoordinates(*pE3dCompoundObject));
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

E3dScene::E3dScene(
    SdrModel& rSdrModel,
    const E3dDefaultAttributes& rDefault)
:   E3dObject(rSdrModel),
    SdrObjList(),
    aCamera(basegfx::B3DPoint(0.0, 0.0, 4.0), basegfx::B3DPoint()),
    mp3DDepthRemapper(0L),
    bDrawOnlySelected(false)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);
}

void E3dScene::SetDefaultAttributes(const E3dDefaultAttributes& /*rDefault*/)
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
    aCameraSet.SetViewportRectangle(Rectangle(0, 0, 10, 10));

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

E3dScene::~E3dScene()
{
    // #110988#
    ImpCleanup3DDepthMapper();

    if(GetObjCount())
    {
        // cannot be called in SdrObjList::ClearSdrObjList() where it originally was
        // since there it would be a pure virtual function call. Needs to be called
        // in all destructors of classes derived from SdrObjList
        getSdrModelFromSdrObjList().SetChanged();
    }
}

void E3dScene::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const E3dScene* pSource = dynamic_cast< const E3dScene* >(&rSource);

        if(pSource)
        {
            // call parent
            E3dObject::copyDataFromSdrObject(rSource);

            // copy local data
            copyDataFromSdrObjList(*pSource->getChildrenOfSdrObject());

            aCamera = pSource->aCamera;
            aCameraSet = pSource->aCameraSet;
            ((sdr::properties::E3dSceneProperties&)GetProperties()).SetSceneItemsFromCamera();
            InvalidateBoundVolume();
            RebuildLists();
            ImpCleanup3DDepthMapper();
            GetViewContact().ActionChanged();
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* E3dScene::CloneSdrObject(SdrModel* pTargetModel) const
{
    E3dScene* pClone = new E3dScene(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject(),
        E3dDefaultAttributes());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

SdrPage* E3dScene::getSdrPageFromSdrObjList() const
{
    return getSdrPageFromSdrObject();
}

SdrObject* E3dScene::getSdrObjectFromSdrObjList() const
{
    return const_cast< E3dScene* >(this);
}

SdrModel& E3dScene::getSdrModelFromSdrObjList() const
{
    return getSdrModelFromSdrObject();
}

void E3dScene::handleContentChange(const SfxHint& rHint)
{
    // call parent
    SdrObjList::handleContentChange(rHint);

    // react self if outmost scene
    E3dScene* pScene = GetScene();

    if(pScene == this)
    {
        // depth mapper needs to change
        ImpCleanup3DDepthMapper();
    }
    else
    {
        // reset local transformation to allow on-demand recalculation
        maSdrObjectTransformation.setB2DHomMatrix(basegfx::B2DHomMatrix());
    }
}

SdrObjList* E3dScene::getChildrenOfSdrObject() const
{
    return const_cast< E3dScene* >(this);
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
        const sal_uInt32 nObjCount(GetObjCount());

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

    ActionChanged();

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
    GetCameraSet().SetViewportRectangle(rCam.GetDeviceWindow());

    // #110988#
    ImpCleanup3DDepthMapper();
}

/*************************************************************************
|*
|* 3D-Objekt einfuegen
|*
\************************************************************************/

void E3dScene::NewObjectInserted(const E3dObject& r3DObj)
{
    E3dObject::NewObjectInserted(r3DObj);

    if(&r3DObj == this)
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

    if(getSdrModelFromSdrObject().isLocked())
    {
        // #123539# optimization for 3D chart object generation: do not reset
        // already calculated scene projection data every time an object gets
        // initialized
        // TTTT: Check this if it works, the idea was to buffer chart 3D object
        // creation... (see task)
        maSdrObjectTransformation.setB2DHomMatrix(basegfx::B2DHomMatrix());
    }

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

    for(sal_uInt32 a(0); a < GetObjCount(); a++)
    {
        SdrObject* pObj = GetObj(a);

        if(pObj)
        {
            bool bRemoveObject(false);
            E3dScene* pScene = dynamic_cast< E3dScene* >(pObj);
            E3dCompoundObject* pCompound = dynamic_cast< E3dCompoundObject* >(pObj);

            if(pScene)
            {
                // iterate over this sub-scene
                pScene->removeAllNonSelectedObjects();

                // check object count. Empty scenes can be deleted
                const sal_uInt32 nObjCount(pScene->GetObjCount());

                if(!nObjCount)
                {
                    // all objects removed, scene can be removed, too
                    bRemoveObject = true;
                }
            }
            else if(pCompound)
            {
                if(!pCompound->GetSelected())
                {
                    bRemoveObject = true;
                }
            }

            if(bRemoveObject)
            {
                RemoveObjectFromSdrObjList(pObj->GetNavigationPosition());
                a--;
                deleteSdrObjectSafeAndClearPointer(pObj);
            }
        }
    }
}

/*************************************************************************
|*
|* Licht- und Labelobjektlisten neu aufbauen (nach Laden, Zuweisung)
|*
\************************************************************************/

void E3dScene::RebuildLists()
{
    // zuerst loeschen
    const SdrLayerID nCurrLayerID(GetLayer());

    for(sal_uInt32 a(0); a < GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetLayer(nCurrLayerID);
            NewObjectInserted(*pCandidate);
        }
        else
        {
            OSL_ENSURE(false, "E3dScene::RebuildLists error in 3D SdrObjList (!)");
        }
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
    ActionChanged();
    E3dObject::Notify(rBC, rHint);
}

/*************************************************************************
|*
\************************************************************************/

void E3dScene::RotateScene (const Point& rRef, long /*nWink*/, double sn, double cs)
{
    Point UpperLeft, LowerRight, Center, NewCenter;
    const Rectangle aSnapRect(sdr::legacy::GetSnapRect(*this));

    UpperLeft = aSnapRect.TopLeft();
    LowerRight = aSnapRect.BottomRight();

    long dxOutRectHalf = labs(UpperLeft.X() - LowerRight.X());
    dxOutRectHalf /= 2;
    long dyOutRectHalf = labs(UpperLeft.Y() - LowerRight.Y());
    dyOutRectHalf /= 2;

    Rectangle RectQuelle(aSnapRect), RectZiel(aSnapRect);

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
    else
    {
        // Hier wird um einen beliebigen Winkel in mathematisch positiver Richtung gedreht!
        // xneu = x * cos(alpha) - y * sin(alpha)
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

    sdr::legacy::MoveSdrObject(*this, Differenz);  // fuehrt die eigentliche Koordinatentransformation durch.
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

void E3dScene::SetB3DTransform(const basegfx::B3DHomMatrix& rMatrix)
{
    if(maTransformation != rMatrix)
    {
        // call parent
        E3dObject::SetB3DTransform(rMatrix);
    }
}

/*************************************************************************
|*
|* Aufbrechen
|*
\************************************************************************/

sal_Bool E3dScene::IsBreakObjPossible()
{
    for(sal_uInt32 a(0); a < GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(GetObj(a));

        if(pCandidate)
        {
            if(!pCandidate->IsBreakObjPossible())
            {
                return false;
            }
        }
        else
        {
            OSL_ENSURE(false, "E3dScene::IsBreakObjPossible error in 3D SdrObjList (!)");
        }
    }

    return true;
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
    GetProperties().SetObjectItemDirect(SfxUInt16Item(SDRATTR_3DSCENE_SHADOW_SLANT, nSceneShadowSlant));
}

basegfx::B2DPolyPolygon E3dScene::TakeCreatePoly(const SdrDragStat& /*rDrag*/) const
{
    return TakeXorPoly();
}

bool E3dScene::MovCreate(SdrDragStat& rStat)
{
    const basegfx::B2DRange aRange(rStat.TakeCreateRange());
    rStat.SetActionRange(aRange);
    sdr::legacy::SetSnapRange(*this, aRange);

    return true;
}

bool E3dScene::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    const basegfx::B2DRange aRange(rStat.TakeCreateRange());
    sdr::legacy::SetSnapRange(*this, aRange);

    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

bool E3dScene::BckCreate(SdrDragStat& /*rStat*/)
{
    return false;
}

void E3dScene::BrkCreate(SdrDragStat& /*rStat*/)
{
}

void E3dScene::SetSelected(bool bNew)
{
    // call parent
    E3dObject::SetSelected(bNew);

    for(sal_uInt32 a(0); a < GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetSelected(bNew);
        }
    }
}

/*************************************************************************
|*
|* Layer setzen
|*
\************************************************************************/

void E3dScene::SetLayer(SdrLayerID nLayer)
{
    // call parent
    E3dObject::SetLayer(nLayer);

    for(sal_uInt32 a(0); a < GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetLayer(nLayer);
        }
    }
}

void E3dScene::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
    if(pOldPage != pNewPage)
    {
        // call parent
        E3dObject::handlePageChange(pOldPage, pNewPage);

        for(sal_uInt32 a(0); a < GetObjCount(); a++)
        {
            E3dObject* pCandidate = dynamic_cast< E3dObject* >(GetObj(a));

            if(pCandidate)
            {
                pCandidate->handlePageChange(pOldPage, pNewPage);
    }
    else
    {
                OSL_ENSURE(false, "E3dScene::handlePageChange invalid object list (!)");
            }
        }
    }
}

/*************************************************************************
|*
|* SnapRect berechnen
|*
\************************************************************************/

const basegfx::B2DHomMatrix& E3dScene::getSdrObjectTransformation() const
{
    const E3dScene* pScene = GetScene();

    if(pScene == this)
    {
        // scene is the outmost one and thus a 2D object. Nothing to do,
        // 2D transformation is where it belongs
    }
    else
    {
        // scene is member of another 3D scene and used as 3D grouping object.
        // Transformation of the group is it's size (scale) and position (translation)
        // of all sub-objects combined. To not always create this, use isIdentity() as
        // hint for recalculation
        // TTTT: look for correct place to set to identity
        // TTTT: should be done by E3dScene::handleContentChange, needs check
        if(maSdrObjectTransformation.getB2DHomMatrix().isIdentity())
        {
            basegfx::B2DRange aRange;

            for(sal_uInt32 a(0); a < GetObjCount(); a++)
            {
                E3dObject* pCandidate = dynamic_cast< E3dObject* >(GetObj(a));

                if(pCandidate)
                {
                    aRange.expand(sdr::legacy::GetLogicRange(*pCandidate));
                }
            }

            const_cast< E3dScene* >(this)->maSdrObjectTransformation.setB2DHomMatrix(
                basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aRange.getRange(),
                    aRange.getMinimum()));
        }
    }

    // call parent
    return E3dObject::getSdrObjectTransformation();
}

void E3dScene::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    const E3dScene* pScene = GetScene();

    if(pScene == this)
    {
        // scene is the outmost one and thus also a 2D object
        E3dObject::setSdrObjectTransformation(rTransformation);

        // also set data at camera as needed
        const basegfx::B2DVector aAbsScale(basegfx::absolute(getSdrObjectScale()));
        const basegfx::B2DPoint& rTranslate(getSdrObjectTranslate());
        const Rectangle aCameraRectangle(
            basegfx::fround(rTranslate.getX()),
            basegfx::fround(rTranslate.getY()),
            basegfx::fround(rTranslate.getX() + aAbsScale.getX()),
            basegfx::fround(rTranslate.getY() + aAbsScale.getY()));

        aCamera.SetDeviceWindow(aCameraRectangle);
        aCameraSet.SetViewportRectangle(aCameraRectangle);
        ImpCleanup3DDepthMapper();
    }
    else
    {
        // scene is member of another 3D scene and used as 3D grouping
        // object. Apply to all sub-objects
        for(sal_uInt32 a(0); a < GetObjCount(); a++)
        {
            E3dObject* pCandidate = dynamic_cast< E3dObject* >(GetObj(a));

            if(pCandidate)
            {
                pCandidate->setSdrObjectTransformation(rTransformation);
            }
        }

        // reset local transformation
        maSdrObjectTransformation.setB2DHomMatrix(basegfx::B2DHomMatrix());
    }
}

/*************************************************************************
|*
|* 3D-Objekt einfuegen
|*
\************************************************************************/

void E3dScene::Insert3DObj(E3dObject& r3DObj)
{
    InsertObjectToSdrObjList(r3DObj);
    InvalidateBoundVolume();
    NewObjectInserted(r3DObj);
    StructureChanged();
}

void E3dScene::Remove3DObj(E3dObject& r3DObj)
{
    if(r3DObj.GetParentObj() == this)
    {
        RemoveObjectFromSdrObjList(r3DObj.GetNavigationPosition());
        InvalidateBoundVolume();
        StructureChanged();
    }
}

/*************************************************************************
|*
|* umschliessendes Volumen inklusive aller Kindobjekte berechnen
|*
\************************************************************************/

basegfx::B3DRange E3dScene::RecalcBoundVolume() const
{
    basegfx::B3DRange aRetval;
    const sal_uInt32 nObjCnt(GetObjCount());

    if(nObjCnt)
    {
        for(sal_uInt32 a(0); a < nObjCnt; a++)
        {
            const E3dObject* p3DObject = dynamic_cast< const E3dObject* >(GetObj(a));

            if(p3DObject)
            {
                basegfx::B3DRange aLocalRange(p3DObject->GetBoundVolume());
                aLocalRange.transform(p3DObject->GetB3DTransform());
                aRetval.expand(aLocalRange);
            }
        }
    }

    return aRetval;
}

/*************************************************************************
|*
|* Aederung des BoundVolumes an alle Kindobjekte weitergeben
|*
\************************************************************************/

void E3dScene::SetBoundVolInvalid()
{
    // call parent
    E3dObject::SetBoundVolInvalid();

    for(sal_uInt32 a(0); a < GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetBoundVolInvalid();
        }
    }
}

/*************************************************************************
|*
|* Aederung der Transformation an alle Kindobjekte weitergeben
|*
\************************************************************************/

void E3dScene::SetTransformChanged()
{
    // call parent
    E3dObject::SetTransformChanged();

    for(sal_uInt32 a(0); a < GetObjCount(); a++)
{
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetTransformChanged();
        }
    }
}

// eof
