/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scene3d.cxx,v $
 * $Revision: 1.34 $
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


#include "svdstr.hrc"
#include "svdglob.hxx"
#include "svditer.hxx"

#if defined( UNX ) || defined( ICC )
#include <stdlib.h>
#endif
#include "globl3d.hxx"
#include <svx/svdpage.hxx>
#include <svtools/style.hxx>
#include <svx/scene3d.hxx>
#include <svx/e3dundo.hxx>
#include <goodies/base3d.hxx>
#include <svx/svdtrans.hxx>

#include <svx/svxids.hrc>
#include <svx/colritem.hxx>
#include <svx/e3ditem.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/svx3ditems.hxx>
#include <svtools/whiter.hxx>
#include <svx/xflftrit.hxx>
#include <svx/sdr/properties/e3dsceneproperties.hxx>

// #110094#
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <svx/svddrag.hxx>

// for ::std::sort
#include <algorithm>

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
                const double fMinimalDepth(((E3dCompoundObject*)pCandidate)->GetMinimalDepthInViewCoor(rScene));
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
    aPaintTime(),
    nDisplayQuality(255),
    mp3DDepthRemapper(0L),
    bDoubleBuffered(FALSE),
    bClipping(FALSE),
    bFitInSnapRect(TRUE),
    bDither(false),
    bWasSelectedWhenCopy(false),
    bDrawOnlySelected(false),
    mfPolygonOffset(0.005) // #i71618#
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

E3dScene::E3dScene(E3dDefaultAttributes& rDefault)
:   E3dObject(),
    aCamera(basegfx::B3DPoint(0.0, 0.0, 4.0), basegfx::B3DPoint()),
    aPaintTime(),
    nDisplayQuality(255),
    mp3DDepthRemapper(0L),
    bDoubleBuffered(FALSE),
    bClipping(FALSE),
    bFitInSnapRect(TRUE),
    bDither(false),
    bWasSelectedWhenCopy(false),
    bDrawOnlySelected(false),
    mfPolygonOffset(0.005) // #i71618#
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);
}

void E3dScene::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    // Fuer OS/2 die FP-Exceptions abschalten
#if defined(OS2)
#define SC_FPEXCEPTIONS_ON()    _control87( MCW_EM, 0 )
#define SC_FPEXCEPTIONS_OFF()   _control87( MCW_EM, MCW_EM )
    SC_FPEXCEPTIONS_OFF();
#endif

    // Fuer WIN95/NT die FP-Exceptions abschalten
#if defined(WNT) || defined(WIN)
#define SC_FPEXCEPTIONS_ON()    _control87( _MCW_EM, 0 )
#define SC_FPEXCEPTIONS_OFF()   _control87( _MCW_EM, _MCW_EM )
    SC_FPEXCEPTIONS_OFF();
#endif

    // Defaults setzen

    // set defaults for LightGroup from ItemPool
    aLightGroup.SetModelTwoSide(GetTwoSidedLighting());
    aLightGroup.SetIntensity( GetLightColor1(), Base3DMaterialDiffuse, Base3DLight0);
    aLightGroup.SetIntensity( GetLightColor2(), Base3DMaterialDiffuse, Base3DLight1);
    aLightGroup.SetIntensity( GetLightColor3(), Base3DMaterialDiffuse, Base3DLight2);
    aLightGroup.SetIntensity( GetLightColor4(), Base3DMaterialDiffuse, Base3DLight3);
    aLightGroup.SetIntensity( GetLightColor5(), Base3DMaterialDiffuse, Base3DLight4);
    aLightGroup.SetIntensity( GetLightColor6(), Base3DMaterialDiffuse, Base3DLight5);
    aLightGroup.SetIntensity( GetLightColor7(), Base3DMaterialDiffuse, Base3DLight6);
    aLightGroup.SetIntensity( GetLightColor8(), Base3DMaterialDiffuse, Base3DLight7);
    aLightGroup.SetGlobalAmbientLight(GetGlobalAmbientColor());
    aLightGroup.Enable( GetLightOnOff1(), Base3DLight0);
    aLightGroup.Enable( GetLightOnOff2(), Base3DLight1);
    aLightGroup.Enable( GetLightOnOff3(), Base3DLight2);
    aLightGroup.Enable( GetLightOnOff4(), Base3DLight3);
    aLightGroup.Enable( GetLightOnOff5(), Base3DLight4);
    aLightGroup.Enable( GetLightOnOff6(), Base3DLight5);
    aLightGroup.Enable( GetLightOnOff7(), Base3DLight6);
    aLightGroup.Enable( GetLightOnOff8(), Base3DLight7);
    aLightGroup.SetDirection( GetLightDirection1(), Base3DLight0);
    aLightGroup.SetDirection( GetLightDirection2(), Base3DLight1);
    aLightGroup.SetDirection( GetLightDirection3(), Base3DLight2);
    aLightGroup.SetDirection( GetLightDirection4(), Base3DLight3);
    aLightGroup.SetDirection( GetLightDirection5(), Base3DLight4);
    aLightGroup.SetDirection( GetLightDirection6(), Base3DLight5);
    aLightGroup.SetDirection( GetLightDirection7(), Base3DLight6);
    aLightGroup.SetDirection( GetLightDirection8(), Base3DLight7);

    bDither = rDefault.GetDefaultDither();

    // Alte Werte initialisieren
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

UINT16 E3dScene::GetObjIdentifier() const
{
    return E3D_SCENE_ID;
}

/*************************************************************************
|*
|* Anzahl der Handles zurueckgeben
|*
\************************************************************************/

sal_uInt32 E3dScene::GetHdlCount() const
{
    // Ueberladung aus E3dObject rueckgaengig machen
    return SdrAttrObj::GetHdlCount();
}

/*************************************************************************
|*
|* Handle-Liste fuellen
|*
\************************************************************************/

void E3dScene::AddToHdlList(SdrHdlList& rHdlList) const
{
    // Ueberladung aus E3dObject rueckgaengig machen
    SdrAttrObj::AddToHdlList(rHdlList);
}

/*************************************************************************
|*
\************************************************************************/

FASTBOOL E3dScene::HasSpecialDrag() const
{
    return FALSE;
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

void E3dScene::StructureChanged(const E3dObject* p3DObj)
{
    E3dObject::StructureChanged(p3DObj);
    SetRectsDirty();

    // #110988#
    ImpCleanup3DDepthMapper();
}

/*************************************************************************
|*
|* Double Buffering aus-/einschalten
|*
\************************************************************************/

void E3dScene::SetDoubleBuffered(FASTBOOL bBuff)
{
    if ( bDoubleBuffered != (BOOL)bBuff )
    {
        bDoubleBuffered = bBuff;
        SetRectsDirty();
    }
}

/*************************************************************************
|*
|* Clipping auf umschliessendes Rechteck der Szene aus-/einschalten
|*
\************************************************************************/

void E3dScene::SetClipping(FASTBOOL bClip)
{
    if ( bClipping != (BOOL)bClip )
    {
        bClipping = bClip;
        SetRectsDirty();
    }
}

/*************************************************************************
|*
|* Einpassen der Objekte in umschliessendes Rechteck aus-/einschalten
|*
\************************************************************************/

void E3dScene::SetFitInSnapRect(FASTBOOL bFit)
{
    if ( bFitInSnapRect != (BOOL)bFit )
    {
        bFitInSnapRect = bFit;
        SetRectsDirty();
    }
}

/*************************************************************************
|*
|* Einpassen der Projektion aller Szenenobjekte in das
|* umschliessende Rechteck
|*
\************************************************************************/

basegfx::B3DRange E3dScene::FitInSnapRect()
{
    basegfx::B3DRange aNewVol;
    const sal_uInt32 nObjCount(GetSubList() ? GetSubList()->GetObjCount() : 0L);

    if(nObjCount)
    {
        // Alter Kram
        basegfx::B3DHomMatrix aFullTrans(GetFullTransform());
        aCamera.FitViewToVolume(GetBoundVolume(), aFullTrans);

        // Neuer Kram
        // Maximas holen in Augkoordinaten zwecks Z-Werten
        basegfx::B3DPoint aTfVec;
        Vol3DPointIterator aIter(GetBoundVolume());

        GetCameraSet().SetObjectTrans(aFullTrans);

        while ( aIter.Next(aTfVec) )
        {
            aTfVec = GetCameraSet().ObjectToEyeCoor(aTfVec);
            aNewVol.expand(aTfVec);
        }

        // ... und merken
        double fZMin(-aNewVol.getMaxZ());
        double fZMax(-aNewVol.getMinZ());

        // Jetzt XY-Werte projizieren auf Projektionsflaeche
        // in Device-Koordinaten
        basegfx::B3DHomMatrix aWorldToDevice(GetCameraSet().GetOrientation());

        if(aCamera.GetProjection() == PR_PERSPECTIVE)
        {
            aWorldToDevice.frustum(-1.0, 1.0, -1.0, 1.0, fZMin, fZMax);
        }
        else
        {
            aWorldToDevice.ortho(-1.0, 1.0, -1.0, 1.0, fZMin, fZMax);
        }

        aNewVol.reset();
        aIter.Reset();

        while ( aIter.Next(aTfVec) )
        {
            aTfVec = GetCameraSet().ObjectToWorldCoor(aTfVec);
            aTfVec *= aWorldToDevice;
            aNewVol.expand(aTfVec);
        }

        // Z-Werte eintragen
        aNewVol = basegfx::B3DRange(aNewVol.getMinX(), aNewVol.getMinY(), fZMin, aNewVol.getMaxX(), aNewVol.getMaxY(), fZMax);
    }

    // #110988#
    ImpCleanup3DDepthMapper();

    // Rueckgabewert setzen
    return aNewVol;
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

/*************************************************************************
|*
|* TransformationSet vorbereiten
|*
\************************************************************************/

void E3dScene::InitTransformationSet()
{
    Rectangle aBound(GetSnapRect());

    // GeometricSet reset und mit pBase3D assoziieren
    B3dCamera& rSet = GetCameraSet();

    // Transformation auf Weltkoordinaten holen
    basegfx::B3DHomMatrix mTransform = GetFullTransform();
    rSet.SetObjectTrans(mTransform);

    // 3D Ausgabe vorbereiten, Maximas holen in DeviceKoordinaten
    basegfx::B3DRange aVolume(FitInSnapRect());

    // #i85887#
    static basegfx::B3DRange aLastVolume;
    if(aVolume != aLastVolume)
    {
        // The BoundRects for the contained 3D SdrObjects depend on the
        // calculated BoundVolume. If the BoundVolume changes, those rects
        // need to be invalidated. Since the first inits when importing a ODF
        // work with wrong 3D Volumes, the initially calculated BoundRects
        // tend to be wrong and need to be invalidated on 3D Volume change.
        SetRectsDirty();
        aLastVolume = aVolume;
    }

    // Maximas fuer Abbildung verwenden
    rSet.SetDeviceVolume(aVolume, FALSE);
    rSet.SetViewportRectangle(aBound);

    // #110988#
    ImpCleanup3DDepthMapper();
}

/*************************************************************************
|*
|* Einpassen der Objekte in umschliessendes Rechteck aus-/einschalten
|*
\************************************************************************/

void E3dScene::FitSnapRectToBoundVol()
{
    basegfx::B3DPoint aTfVec;
    Volume3D aFitVol;

    SetBoundVolInvalid();
    basegfx::B3DHomMatrix aTransform = aCamera.GetViewTransform() * GetFullTransform(); // #112587#
    Vol3DPointIterator aIter(GetBoundVolume(), &aTransform);
    Rectangle aRect;

    while ( aIter.Next(aTfVec) )
    {
        aTfVec = aCamera.DoProjection(aTfVec);
        aFitVol.expand(aTfVec);
        basegfx::B3DPoint aZwi(aCamera.MapToDevice(aTfVec));
        Point aP((long)aZwi.getX(), (long)aZwi.getY());
        aRect.Union(Rectangle(aP, aP));
    }
    aCamera.SetViewWindow(aFitVol.getMinX(), aFitVol.getMinY(), aFitVol.getWidth(), aFitVol.getHeight());
    SetSnapRect(aRect);

    // Die SnapRects aller beteiligten Objekte muessen auf dieser
    // veraenderten Basis aufgebaut werden, invalidiere diese. Das
    // eigene kann auch invalidiert werden, da ein RecalcSnapRect
    // an einer Szene nur aus der Kamera liest
    SetRectsDirty();

    // #110988#
    ImpCleanup3DDepthMapper();
}

/*************************************************************************
|*
|* Falls die Geometrie einer Szene sich ausgedehnt/vermindert hat,
|* muss das Volume und das SnapRect angepasst werden
|*
\************************************************************************/

void E3dScene::CorrectSceneDimensions()
{
    const sal_uInt32 nObjCount(GetSubList() ? GetSubList()->GetObjCount() : 0L);

    if(nObjCount)
    {
        // SnapRects der Objekte ungueltig
        SetRectsDirty();

        // SnapRect anpassen, invalidiert auch die SnapRects
        // der enthaltenen Objekte
        FitSnapRectToBoundVol();

        // Neues BoundVolume der Kamera holen
        basegfx::B3DRange aVolume(FitInSnapRect());

        // Neues BoundVolume an der Kamera setzen
        GetCameraSet().SetDeviceVolume(aVolume, FALSE);

        // Danach noch die SnapRects der enthaltenen Objekte
        // invalidieren, um diese auf der neuen Grundlage berechnen
        // zu lassen (falls diese von FitInSnapRect() berechnet wurden)
        SetRectsDirty();
    }

    // #110988#
    ImpCleanup3DDepthMapper();
}

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

void E3dScene::operator=(const SdrObject& rObj)
{
    E3dObject::operator=(rObj);

    const E3dScene& r3DObj = (const E3dScene&) rObj;
    aCamera          = r3DObj.aCamera;
    bDoubleBuffered  = r3DObj.bDoubleBuffered;
    bClipping        = r3DObj.bClipping;
    bFitInSnapRect   = r3DObj.bFitInSnapRect;

    // neu ab 377:
    aCameraSet = r3DObj.aCameraSet;
    ((sdr::properties::E3dSceneProperties&)GetProperties()).SetSceneItemsFromCamera();

    // neu ab 383:
    aLightGroup = r3DObj.aLightGroup;
    ((sdr::properties::E3dSceneProperties&)GetProperties()).SetLightItemsFromLightGroup(aLightGroup);

    bDither = r3DObj.bDither;

    bBoundVolValid = FALSE;
    RebuildLists();

    SetRectsDirty();

    // #110988#
    ImpCleanup3DDepthMapper();
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

    SdrObjListIter a3DIterator(*pSub, IM_FLAT);

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

    ((E3DSceneGeoData &) rGeo).aCamera                = aCamera;
}

/*************************************************************************
|*
|* uebernehme werte aus dem GeoData-Objekt
|*
\************************************************************************/

void E3dScene::RestGeoData(const SdrObjGeoData& rGeo)
{
    E3dObject::RestGeoData (rGeo);

    SetCamera (((E3DSceneGeoData &) rGeo).aCamera);
    FitSnapRectToBoundVol();
}

/*************************************************************************
|*
|* Am StyleSheet wurde etwas geaendert, also Scene aendern
|*
\************************************************************************/

void E3dScene::SFX_NOTIFY(SfxBroadcaster &rBC,
                          const TypeId   &rBCType,
                          const SfxHint  &rHint,
                          const TypeId   &rHintType)
{
    SetRectsDirty();
    E3dObject::SFX_NOTIFY(rBC, rBCType, rHint, rHintType);
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

void E3dScene::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
        // Also derzeit sind die Klebepunkte relativ zum aOutRect der Szene definiert. Vor dem Drehen
        // werden die Klebepunkte relativ zur Seite definiert. Sie nehmen an der Drehung der Szene noch nicht Teil
        // dafuer gibt es den
    SetGlueReallyAbsolute(TRUE);

        // So dass war die Szene, ab jetzt kommen die Objekte in der Szene
        // 3D-Objekte gibt es nur ein einziges das kann zwar mehrere Flaechen haben aber die Flaechen
        // muessen ja nicht zusammenhaengend sein
        // es ermoeglicht den Zugriff auf Kindobjekte
        // Ich gehe also die gesamte Liste durch und rotiere um die Z-Achse die durch den
        // Mittelpunkt von aOutRect geht (Satz von Steiner), also RotateZ

    RotateScene (rRef, nWink, sn, cs);  // Rotiert die Szene
    double fWinkelInRad = nWink/100 * F_PI180;
    NbcRotateZ(fWinkelInRad);
    FitSnapRectToBoundVol();
    SetRectsDirty();    // Veranlasst eine Neuberechnung aller BoundRects
    NbcRotateGluePoints(rRef,nWink,sn,cs);  // Rotiert die Klebepunkte (die haben noch Koordinaten relativ
                                            // zum Urpsung des Blattes
    SetGlueReallyAbsolute(FALSE);  // ab jetzt sind sie wieder relativ zum BoundRect (also dem aOutRect definiert)
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

BOOL E3dScene::IsBreakObjPossible()
{
    // Szene ist aufzubrechen, wenn alle Mitglieder aufzubrechen sind
    SdrObjList* pSubList = GetSubList();
    if(pSubList)
    {
        SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
        while ( a3DIterator.IsMore() )
        {
            E3dObject* pObj = (E3dObject*) a3DIterator.Next();
            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
            if(!pObj->IsBreakObjPossible())
                return FALSE;
        }
    }
    return TRUE;
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
    UINT16 nSceneShadowSlant = (UINT16)((atan2(rVec.getY(), rVec.getZ()) / F_PI180) + 0.5);
    GetProperties().SetObjectItemDirect(Svx3DShadowSlantItem(nSceneShadowSlant));
}


// #115662#
// helper class for in-between results from E3dScene::HitTest
class ImplPairDephAndObject
{
public:
    SdrObject* pObject;
    double fDepth;

    // for ::std::sort
    bool operator<(const ImplPairDephAndObject& rComp) const;
};

bool ImplPairDephAndObject::operator<(const ImplPairDephAndObject& rComp) const
{
    if(fDepth < rComp.fDepth)
        return true;
    return false;
}

// #115662#
// For new chart, calculate the number of hit contained 3D objects at given point,
// give back the count and a depth-sorted list of SdrObjects (a Vector). The vector will be
// changed, at least cleared.
sal_uInt32 E3dScene::HitTest(const Point& rHitTestPosition, ::std::vector< SdrObject* >& o_rResult)
{
    // prepare output variables
    sal_uInt32 nRetval(0L);
    o_rResult.clear();
    SdrObjList* pList = GetSubList();

    if(pList && pList->GetObjCount())
    {
        SdrObjListIter aIterator(*pList, IM_DEEPNOGROUPS);
        ::std::vector< ImplPairDephAndObject > aDepthAndObjectResults;

        while(aIterator.IsMore())
        {
            SdrObject* pObj = aIterator.Next();

            if(pObj->ISA(E3dCompoundObject))
            {
                E3dCompoundObject* pCompoundObj = (E3dCompoundObject*)pObj;

                // get HitLine in local 3D ObjectKoordinates
                basegfx::B3DHomMatrix mTransform = pCompoundObj->GetFullTransform();
                GetCameraSet().SetObjectTrans(mTransform);

                // create HitPoint Front und Back, transform to local object coordinates
                basegfx::B3DPoint aFront(rHitTestPosition.X(), rHitTestPosition.Y(), 0.0);
                basegfx::B3DPoint aBack(rHitTestPosition.X(), rHitTestPosition.Y(), ZBUFFER_DEPTH_RANGE);
                aFront = GetCameraSet().ViewToObjectCoor(aFront);
                aBack = GetCameraSet().ViewToObjectCoor(aBack);

                // make BoundVolume HitTest for speedup first
                const Volume3D& rBoundVol = pCompoundObj->GetBoundVolume();

                if(!rBoundVol.isEmpty())
                {
                    double fXMax(aFront.getX());
                    double fXMin(aBack.getX());

                    if(fXMax < fXMin)
                    {
                        fXMax = aBack.getX();
                        fXMin = aFront.getX();
                    }

                    if(rBoundVol.getMinX() <= fXMax && rBoundVol.getMaxX() >= fXMin)
                    {
                        double fYMax(aFront.getY());
                        double fYMin(aBack.getY());

                        if(fYMax < fYMin)
                        {
                            fYMax = aBack.getY();
                            fYMin = aFront.getY();
                        }

                        if(rBoundVol.getMinY() <= fYMax && rBoundVol.getMaxY() >= fYMin)
                        {
                            double fZMax(aFront.getZ());
                            double fZMin(aBack.getZ());

                            if(fZMax < fZMin)
                            {
                                fZMax = aBack.getZ();
                                fZMin = aFront.getZ();
                            }

                            if(rBoundVol.getMinZ() <= fZMax && rBoundVol.getMaxZ() >= fZMin)
                            {
                                // BoundVol is hit, get geometry cuts now
                                ::std::vector< basegfx::B3DPoint > aParameter;
                                const B3dGeometry& rGeometry = pCompoundObj->GetDisplayGeometry();
                                rGeometry.GetAllCuts(aParameter, aFront, aBack);

                                if(aParameter.size())
                                {
                                    // take first cut as base, use Z-Coor in ViewCoor (0 ..ZBUFFER_DEPTH_RANGE)
                                    ImplPairDephAndObject aTempResult;
                                    basegfx::B3DPoint aTempVector(aParameter[0]);
                                    aTempVector = GetCameraSet().ObjectToViewCoor(aTempVector);

                                    aTempResult.pObject = pCompoundObj;
                                    aTempResult.fDepth = aTempVector.getZ();

                                    // look for cut points in front of the first one
                                    ::std::vector< basegfx::B3DPoint >::iterator aIterator2(aParameter.begin());
                                    aIterator2++;

                                    for(;aIterator2 != aParameter.end(); aIterator2++)
                                    {
                                        basegfx::B3DPoint aTempVector2(*aIterator2);
                                        aTempVector2 = GetCameraSet().ObjectToViewCoor(aTempVector2);

                                        // use the smallest one
                                        if(aTempVector2.getZ() < aTempResult.fDepth)
                                        {
                                            aTempResult.fDepth = aTempVector2.getZ();
                                        }
                                    }

                                    // remember smallest cut with this object
                                    aDepthAndObjectResults.push_back(aTempResult);
                                }
                            }
                        }
                    }
                }
            }
        }

        // fill nRetval
        nRetval = aDepthAndObjectResults.size();

        if(nRetval)
        {
            // sort aDepthAndObjectResults by depth
            ::std::sort(aDepthAndObjectResults.begin(), aDepthAndObjectResults.end());

            // copy SdrObject pointers to return result set
            ::std::vector< ImplPairDephAndObject >::iterator aIterator2(aDepthAndObjectResults.begin());

            for(;aIterator2 != aDepthAndObjectResults.end(); aIterator2++)
            {
                o_rResult.push_back(aIterator2->pObject);
            }
        }
    }

    return nRetval;
}

basegfx::B2DPolyPolygon E3dScene::TakeCreatePoly(const SdrDragStat& /*rDrag*/) const
{
    return TakeXorPoly(sal_True);
}

FASTBOOL E3dScene::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    NbcSetSnapRect(aRect1);
    return TRUE;
}

FASTBOOL E3dScene::MovCreate(SdrDragStat& rStat)
{
    Rectangle aRect1;
    rStat.TakeCreateRect(aRect1);
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    NbcSetSnapRect(aRect1);
    SetBoundRectDirty();
    bSnapRectDirty=TRUE;
    return TRUE;
}

FASTBOOL E3dScene::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    Rectangle aRect1;
    rStat.TakeCreateRect(aRect1);
    aRect1.Justify();
    NbcSetSnapRect(aRect1);
    SetRectsDirty();
    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

FASTBOOL E3dScene::BckCreate(SdrDragStat& /*rStat*/)
{
    return FALSE;
}

void E3dScene::BrkCreate(SdrDragStat& /*rStat*/)
{
}

// eof
