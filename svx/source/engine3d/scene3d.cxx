/*************************************************************************
 *
 *  $RCSfile: scene3d.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: dl $ $Date: 2001-03-28 08:08:15 $
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

#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif

#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif

#if defined( UNX ) || defined( ICC )
#include <stdlib.h>
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

#ifndef _E3D_SCENE3D_HXX
#include "scene3d.hxx"
#endif

#ifndef _E3D_UNDO_HXX
#include "e3dundo.hxx"
#endif

#ifndef _B3D_BASE3D_HXX
#include <goodies/base3d.hxx>
#endif

#ifndef _E3D_PLIGHT3D_HXX
#include "plight3d.hxx"
#endif

#ifndef _E3D_DLIGHT3D_HXX
#include "dlight3d.hxx"
#endif

#ifndef _SVDTRANS_HXX
#include "svdtrans.hxx"
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

#ifndef _SVX3DITEMS_HXX
#include "svx3ditems.hxx"
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

TYPEINIT1(E3dScene, E3dObject);

/*************************************************************************
|*
|* E3dScene-Konstruktor
|*
\************************************************************************/

E3dScene::E3dScene()
:   E3dObject(),
    aCamera(Vector3D(0,0,4), Vector3D()),
    bDoubleBuffered(FALSE),
    bClipping(FALSE),
    nSaveStatus (0),
    nRestStatus (0),
    bFitInSnapRect(TRUE),
    aPaintTime(),
    nDisplayQuality(255),
    bDrawOnlySelected(FALSE)
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

E3dScene::E3dScene(E3dDefaultAttributes& rDefault)
:   E3dObject(),
    aCamera(Vector3D(0,0,4), Vector3D()),
    bDoubleBuffered(FALSE),
    bClipping(FALSE),
    nSaveStatus (0),
    nRestStatus (0),
    bFitInSnapRect(TRUE),
    aPaintTime(),
    nDisplayQuality(255),
    bDrawOnlySelected(FALSE)
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
    nSortingMode = E3D_SORT_FAST_SORTING | E3D_SORT_IN_PARENTS | E3D_SORT_TEST_LENGTH;

    // set defaults for Camera from ItemPool
    aCamera.SetProjection(GetPerspective());
    Vector3D aActualPosition = aCamera.GetPosition();
    double fNew = GetDistance();
    if(fabs(fNew - aActualPosition.Z()) > 1.0)
        aCamera.SetPosition( Vector3D( aActualPosition.X(), aActualPosition.Y(), fNew) );
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
}

/*************************************************************************
|*
|* Feststellen, ob die Szene transparente Teile enthaelt
|*
\************************************************************************/

BOOL E3dScene::AreThereTransparentParts() const
{
    BOOL bRetval(FALSE);

    SdrObjListIter a3DIterator(*pSub, IM_DEEPWITHGROUPS);
    while ( !bRetval && a3DIterator.IsMore() )
    {
        SdrObject* pObj = a3DIterator.Next();

        // Nur darstellbare Objekte bewerten
        if(pObj->ISA(E3dCompoundObject))
        {
            // Flaechenattribut testen
            UINT16 nFillTrans = ITEMVALUE( pObj->GetItemSet(), XATTR_FILLTRANSPARENCE, XFillTransparenceItem);
            if(nFillTrans)
                bRetval = TRUE;

            if(!bRetval)
            {
                // Linienattribut testen
                UINT16 nLineTransparence = ITEMVALUE( pObj->GetItemSet(), XATTR_LINETRANSPARENCE, XLineTransparenceItem );
                if(nLineTransparence)
                    bRetval = TRUE;
            }
        }
    }
    return bRetval;
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

USHORT E3dScene::GetHdlCount() const
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
    ImpSetSceneItemsFromCamera();
    SetRectsDirty();

    // Neue Kamera aus alter fuellen
    Camera3D& rCam = (Camera3D&)GetCamera();

    // Ratio abschalten
    if(rCam.GetAspectMapping() == AS_NO_MAPPING)
        GetCameraSet().SetRatio(0.0);

    // Abbildungsgeometrie setzen
    Vector3D aVRP = rCam.GetViewPoint();
    Vector3D aVPN = aVRP - rCam.GetVRP();
    Vector3D aVUV = rCam.GetVUV();
    GetCameraSet().SetOrientation(aVRP, aVPN, aVUV);

    // Perspektive setzen
    GetCameraSet().SetPerspective(rCam.GetProjection() == PR_PERSPECTIVE);
    GetCameraSet().SetViewportRectangle((Rectangle&)rCam.GetDeviceWindow());

    // E3dLabel-Objekte muessen neu an die Projektion angepasst werden
    if ( aLabelList.Count() > 0 )
    {
        SetBoundVolInvalid();
        SetRectsDirty();
    }
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

    if ( p3DObj->ISA(E3dLabelObj) )
    {
        aLabelList.Insert((E3dLabelObj*) p3DObj, LIST_APPEND);
    }

    // falls Unterobjekte vorhanden sind, auch diese pruefen
    if ( p3DObj->IsGroupObject() )
    {
        SdrObjListIter a3DIterator(*p3DObj, IM_DEEPWITHGROUPS);

        while ( a3DIterator.IsMore() )
        {
            SdrObject* pObj = a3DIterator.Next();

            if ( pObj->ISA(E3dLabelObj) )
            {
                aLabelList.Insert((E3dLabelObj*) pObj, LIST_APPEND);
            }
        }
    }
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

Volume3D E3dScene::FitInSnapRect()
{
    // Alter Kram
    Matrix4D aFullTrans = GetFullTransform();
    aCamera.FitViewToVolume(GetBoundVolume(), aFullTrans);

    // Neuer Kram
    // Maximas holen in Augkoordinaten zwecks Z-Werten
    Volume3D aNewVol;
    Vector3D aTfVec;
    Vol3DPointIterator aIter(GetBoundVolume());

    GetCameraSet().SetObjectTrans(aFullTrans);
    while ( aIter.Next(aTfVec) )
    {
        aTfVec = GetCameraSet().ObjectToEyeCoor(aTfVec);
        aNewVol.Union(aTfVec);
    }

    // ... und merken
    double fZMin = -aNewVol.MaxVec().Z();
    double fZMax = -aNewVol.MinVec().Z();

    // Jetzt XY-Werte projizieren auf Projektionsflaeche
    // in Device-Koordinaten
    Matrix4D aWorldToDevice = GetCameraSet().GetOrientation();
    if(aCamera.GetProjection() == PR_PERSPECTIVE)
        aWorldToDevice.Frustum(-1.0, 1.0, -1.0, 1.0, fZMin, fZMax);
    else
        aWorldToDevice.Ortho(-1.0, 1.0, -1.0, 1.0, fZMin, fZMax);
    aNewVol.Reset();
    aIter.Reset();
    while ( aIter.Next(aTfVec) )
    {
        aTfVec = GetCameraSet().ObjectToWorldCoor(aTfVec);
        aTfVec *= aWorldToDevice;
        aNewVol.Union(aTfVec);
    }

    // Labels behandeln
    ULONG nLabelCnt = aLabelList.Count();
    if ( nLabelCnt > 0 )
    {
        // Vorlaeufige Projektion bestimmen und Transformation in
        // ViewKoordinaten bestimmen
        Matrix4D aMatWorldToView = GetCameraSet().GetOrientation();
        if(aCamera.GetProjection() == PR_PERSPECTIVE)
            aMatWorldToView.Frustum(aNewVol.MinVec().X(), aNewVol.MaxVec().X(),
            aNewVol.MinVec().Y(), aNewVol.MaxVec().Y(), fZMin, fZMax);
        else
            aMatWorldToView.Ortho(aNewVol.MinVec().X(), aNewVol.MaxVec().X(),
            aNewVol.MinVec().Y(), aNewVol.MaxVec().Y(), fZMin, fZMax);

        // Logische Abmessungen der Szene holen
        Rectangle aSceneRect = GetSnapRect();

        // Matrix DeviceToView aufbauen
        Vector3D aTranslate, aScale;

        aTranslate[0] = (double)aSceneRect.Left() + (aSceneRect.GetWidth() / 2.0);
        aTranslate[1] = (double)aSceneRect.Top() + (aSceneRect.GetHeight() / 2.0);
        aTranslate[2] = ZBUFFER_DEPTH_RANGE / 2.0;

        // Skalierung
        aScale[0] = (aSceneRect.GetWidth() - 1) / 2.0;
        aScale[1] = (aSceneRect.GetHeight() - 1) / -2.0;
        aScale[2] = ZBUFFER_DEPTH_RANGE / 2.0;

        aMatWorldToView.Scale(aScale);
        aMatWorldToView.Translate(aTranslate);

        // Inverse Matrix ViewToDevice aufbauen
        Matrix4D aMatViewToWorld(aMatWorldToView);
        aMatViewToWorld.Invert();

        for (ULONG i = 0; i < nLabelCnt; i++)
        {
            E3dLabelObj* p3DObj = aLabelList.GetObject(i);
            const SdrObject* pObj = p3DObj->Get2DLabelObj();

            // View- Abmessungen des Labels holen
            const Rectangle& rObjRect = pObj->GetLogicRect();

            // Position des Objektes in Weltkoordinaten ermitteln
            Matrix4D aObjTrans = p3DObj->GetFullTransform();
            Vector3D aObjPos = aObjTrans * p3DObj->GetPosition();

            // View-Position des Objektes feststellen
            // nach ViewKoordinaten
            aObjPos *= aMatWorldToView;

            // Relative Position des Labels in View-Koordinaten
            Vector3D aRelPosOne(pObj->GetRelativePos(), aObjPos.Z());
            aRelPosOne.X() += aObjPos.X();
            aRelPosOne.Y() += aObjPos.Y();
            Vector3D aRelPosTwo(aRelPosOne);
            aRelPosTwo.X() += rObjRect.GetWidth();
            aRelPosTwo.Y() += rObjRect.GetHeight();

            // Jetzt Eckpunkte in DeviceKoordinaten bestimmen und
            // den Abmessungen hinzufuegen
            aRelPosOne *= aMatViewToWorld;
            aRelPosOne *= aWorldToDevice;
            aNewVol.Union(aRelPosOne);

            aRelPosTwo *= aMatViewToWorld;
            aRelPosTwo *= aWorldToDevice;
            aNewVol.Union(aRelPosTwo);
        }
    }

    // Z-Werte eintragen
    aNewVol.MinVec().Z() = fZMin;
    aNewVol.MaxVec().Z() = fZMax;

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
    Matrix4D mTransform = GetFullTransform();
    rSet.SetObjectTrans(mTransform);

    // 3D Ausgabe vorbereiten, Maximas holen in DeviceKoordinaten
    Volume3D aVolume = FitInSnapRect();

    // Maximas fuer Abbildung verwenden
    rSet.SetDeviceVolume(aVolume, FALSE);
    rSet.SetViewportRectangle(aBound);
}

/*************************************************************************
|*
|* sichern mit neuer Methode und zukunftskompatibilitaet
|* Die Zahl 3560 ist die Major-Update-Nummer * 10 zu der die Umstellung
|* erfolgte. Dies ist leider das korrekte Verhalten, die 3d-Engine hat keine
|* eigene Versionsnummer sondern ist an die der Drawing-Engine gekoppelt.
|* Probleme gibt es immer dann wenn einen neue Version ein altes Format
|* schreiben soll: Hier wird von der Drawing-Engine trotzdem die neue Nummer
|* verwendet.
|*
\************************************************************************/

void E3dScene::WriteData(SvStream& rOut) const
{
    long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
    if(nVersion < 3830)
    {
        // Hier die Lichtobjekte erzeugen, um im alten Format schreiben zu koennen
        ((E3dScene*)(this))->CreateLightObjectsFromLightGroup();
    }

    // Schreiben
    E3dObject::WriteData(rOut);

    if(nVersion < 3830)
    {
        // Lichtobjekte wieder wegnehmen
        ((E3dScene*)(this))->RemoveLightObjects();
    }
    else
    {
#ifdef E3D_STREAMING
        SdrDownCompat aCompat(rOut, STREAM_WRITE);
#ifdef DBG_UTIL
        aCompat.SetID("B3dLightGroup");
#endif
        // LightGroup schreiben
        aLightGroup.WriteData(rOut);

#endif
    }

#ifdef E3D_STREAMING
    SdrDownCompat aCompat(rOut, STREAM_WRITE);
#ifdef DBG_UTIL
    aCompat.SetID("E3dScene");
#endif

    DBG_ASSERT (rOut.GetVersion(),"3d-Engine: Keine Version am Stream gesetzt!");
    if (rOut.GetVersion() < 3560) // FG: Das ist der Zeitpunkt der Umstellung
    {
        rOut << aCamera;
    }
    if (rOut.GetVersion() >= 3560)
    {
        aCamera.WriteData(rOut);
    }

    rOut << BOOL(bDoubleBuffered);
    rOut << BOOL(bClipping);
    rOut << BOOL(bFitInSnapRect);
    rOut << nSortingMode;

    // neu ab 377:
    Vector3D aPlaneDirection = GetShadowPlaneDirection();
    rOut << aPlaneDirection;

    // neu ab 383:
    rOut << (BOOL)bDither;

    // neu ab 384:
    sal_uInt16 nShadeMode = GetShadeMode();
    if(nShadeMode == 0)
        rOut << (sal_uInt16)Base3DFlat;
    else if(nShadeMode == 1)
        rOut << (sal_uInt16)Base3DPhong;
    else
        rOut << (sal_uInt16)Base3DSmooth;
    rOut << (BOOL)(nShadeMode > 2);

#endif
}

/*************************************************************************
|*
|* laden
|*
\************************************************************************/

void E3dScene::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if (ImpCheckSubRecords (rHead, rIn))
    {
        E3dObject::ReadData(rHead, rIn);

        if(CountNumberOfLights())
        {
            // An dieser Stelle die gelesenen Lampen ausmerzen
            // und in die neue Struktur ueberfuehren
            FillLightGroup();
            RemoveLightObjects();
        }
        long nVersion = rIn.GetVersion(); // Build_Nr * 10 z.B. 3810
        if(nVersion >= 3830)
        {
            SdrDownCompat aCompat(rIn, STREAM_READ);
#ifdef DBG_UTIL
            aCompat.SetID("B3dLightGroup");
#endif
            if(aCompat.GetBytesLeft())
            {
                // LightGroup lesen
                aLightGroup.ReadData(rIn);
            }
        }

        SdrDownCompat aCompat(rIn, STREAM_READ);
#ifdef DBG_UTIL
        aCompat.SetID("E3dScene");
#endif
        BOOL bTmp;

        DBG_ASSERT (rIn.GetVersion(),"3d-Engine: Keine Version am Stream gesetzt!");

        if ((rIn.GetVersion() < 3560) || (rHead.GetVersion() <= 12))
        {
            rIn >> aCamera;
        }
        if ((rIn.GetVersion() >= 3560) && (rHead.GetVersion() >= 13))
        {
            aCamera.ReadData(rHead, rIn);
        }

        // Neue Kamera aus alter fuellen
        Camera3D& rCam = (Camera3D&)GetCamera();

        // Ratio abschalten
        if(rCam.GetAspectMapping() == AS_NO_MAPPING)
            GetCameraSet().SetRatio(0.0);

        // Abbildungsgeometrie setzen
        Vector3D aVRP = rCam.GetViewPoint();
        Vector3D aVPN = aVRP - rCam.GetVRP();
        Vector3D aVUV = rCam.GetVUV();
        GetCameraSet().SetOrientation(aVRP, aVPN, aVUV);

        // Perspektive setzen
        GetCameraSet().SetPerspective(rCam.GetProjection() == PR_PERSPECTIVE);
        GetCameraSet().SetViewportRectangle((Rectangle&)rCam.GetDeviceWindow());

        rIn >> bTmp; bDoubleBuffered = bTmp;
        rIn >> bTmp; bClipping = bTmp;
        rIn >> bTmp; bFitInSnapRect = bTmp;

        if (aCompat.GetBytesLeft() >= sizeof(UINT32))
        {
            rIn >> nSortingMode;
        }

        // neu ab 377:
        if (aCompat.GetBytesLeft() >= sizeof(Vector3D))
        {
            Vector3D aShadowVec;
            rIn >> aShadowVec;
            SetShadowPlaneDirection(aShadowVec);
        }

        // neu ab 383:
        if (aCompat.GetBytesLeft() >= sizeof(BOOL))
        {
            rIn >> bTmp; bDither = bTmp;
        }

        // neu ab 384:
        if (aCompat.GetBytesLeft() >= sizeof(UINT16))
        {
            UINT16 nTmp;
            rIn >> nTmp;
            if(nTmp == (Base3DShadeModel)Base3DFlat)
                mpObjectItemSet->Put(Svx3DShadeModeItem(0));
            else if(nTmp == (Base3DShadeModel)Base3DPhong)
                mpObjectItemSet->Put(Svx3DShadeModeItem(1));
            else
                mpObjectItemSet->Put(Svx3DShadeModeItem(2));
        }
        if (aCompat.GetBytesLeft() >= sizeof(BOOL))
        {
            rIn >> bTmp;
            if(bTmp)
                mpObjectItemSet->Put(Svx3DShadeModeItem(3));
        }

        // SnapRects der Objekte ungueltig
        SetRectsDirty();

        // Transformationen initialisieren, damit bei RecalcSnapRect()
        // richtig gerechnet wird
        InitTransformationSet();

        RebuildLists();

        // set items from combined read objects like lightgroup and camera
        ImpSetLightItemsFromLightGroup();
        ImpSetSceneItemsFromCamera();
    }
}

/*************************************************************************
|*
|* Einpassen der Objekte in umschliessendes Rechteck aus-/einschalten
|*
\************************************************************************/

void E3dScene::FitSnapRectToBoundVol()
{
    Vector3D aTfVec;
    Volume3D aFitVol;

    SetBoundVolInvalid();
    Matrix4D aTransform = GetFullTransform() * aCamera.GetViewTransform();
    Vol3DPointIterator aIter(GetBoundVolume(), &aTransform);
    Rectangle aRect;

    while ( aIter.Next(aTfVec) )
    {
        aCamera.DoProjection(aTfVec);
        aFitVol.Union(aTfVec);
        Vector3D aZwi = aCamera.MapToDevice(aTfVec);
        Point aP((long)aZwi.X(), (long)aZwi.Y());
        aRect.Union(Rectangle(aP, aP));
    }
    aCamera.SetViewWindow(aFitVol.MinVec().X(), aFitVol.MinVec().Y(),
        aFitVol.GetWidth(), aFitVol.GetHeight());
    SetSnapRect(aRect);

    // Die SnapRects aller beteiligten Objekte muessen auf dieser
    // veraenderten Basis aufgebaut werden, invalidiere diese. Das
    // eigene kann auch invalidiert werden, da ein RecalcSnapRect
    // an einer Szene nur aus der Kamera liest
    SetRectsDirty();
}

/*************************************************************************
|*
|* Falls die Geometrie einer Szene sich ausgedehnt/vermindert hat,
|* muss das Volume und das SnapRect angepasst werden
|*
\************************************************************************/

void E3dScene::CorrectSceneDimensions()
{
    // SnapRects der Objekte ungueltig
    SetRectsDirty();

    // SnapRect anpassen, invalidiert auch die SnapRects
    // der enthaltenen Objekte
    FitSnapRectToBoundVol();

    // Neues BoundVolume der Kamera holen
    Volume3D aVolume = FitInSnapRect();

    // Neues BoundVolume an der Kamera setzen
    GetCameraSet().SetDeviceVolume(aVolume, FALSE);

    // Danach noch die SnapRects der enthaltenen Objekte
    // invalidieren, um diese auf der neuen Grundlage berechnen
    // zu lassen (falls diese von FitInSnapRect() berechnet wurden)
    SetRectsDirty();
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
    nSortingMode     = r3DObj.nSortingMode;

    // neu ab 377:
    aCameraSet = r3DObj.aCameraSet;
    ImpSetSceneItemsFromCamera();

    // neu ab 383:
    aLightGroup = r3DObj.aLightGroup;
    ImpSetLightItemsFromLightGroup();
    bDither = r3DObj.bDither;

    bBoundVolValid = FALSE;
    RebuildLists();

    SetRectsDirty();
}

/*************************************************************************
|*
|* Licht- und Labelobjektlisten neu aufbauen (nach Laden, Zuweisung)
|*
\************************************************************************/

void E3dScene::RebuildLists()
{
    // zuerst loeschen
    aLabelList.Clear();
    SdrLayerID nLayerID = GetLayer();

    SdrObjListIter a3DIterator(*pSub, IM_FLAT);

    // dann alle Objekte in der Szene pruefen
    while ( a3DIterator.IsMore() )
    {
        E3dObject* p3DObj = (E3dObject*) a3DIterator.Next();
        p3DObj->NbcSetLayer(nLayerID);
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
    ((E3DSceneGeoData &) rGeo).aLabelList             = aLabelList;
}

/*************************************************************************
|*
|* uebernehme werte aus dem GeoData-Objekt
|*
\************************************************************************/

void E3dScene::RestGeoData(const SdrObjGeoData& rGeo)
{
    E3dObject::RestGeoData (rGeo);

    aLabelList = ((E3DSceneGeoData &) rGeo).aLabelList;
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
|* Compounds brauchen Defaults
|*
\************************************************************************/

void E3dScene::ForceDefaultAttr()
{
    SdrAttrObj::ForceDefaultAttr();
}

/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

void E3dScene::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    E3dObjList* pOL = pSub;
    ULONG nObjCnt = pOL->GetObjCount();

    for ( ULONG i = 0; i < nObjCnt; i++ )
        pOL->GetObj(i)->NbcSetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

    StructureChanged(this);
}

/*************************************************************************
|*
|* Attribute abfragen
|*
\************************************************************************/

SfxStyleSheet* E3dScene::GetStyleSheet() const
{
    E3dObjList    *pOL          = pSub;
    ULONG         nObjCnt       = pOL->GetObjCount();
    SfxStyleSheet *pRet         = 0;

    for ( ULONG i = 0; i < nObjCnt; i++ )
    {
        SfxStyleSheet *pSheet = pOL->GetObj(i)->GetStyleSheet();

        if (!pRet)
            pRet = pSheet;
        else if (pSheet)
        {
            if(!pSheet->GetName().Equals(pRet->GetName()))
                return 0;
        }
    }
    return pRet;
}

/*************************************************************************
|*
\************************************************************************/

void E3dScene::RotateScene (const Point& rRef, long nWink, double sn, double cs)
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
    if (sn==1.0 && cs==0.0) { // 90ø
        NewCenter.X() = -Center.Y();
        NewCenter.Y() = -Center.X();
    } else if (sn==0.0 && cs==-1.0) { // 180ø
        NewCenter.X() = -Center.X();
        NewCenter.Y() = -Center.Y();
    } else if (sn==-1.0 && cs==0.0) { // 270ø
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
|* Die NbcRotate-Routine überlädt die des SdrObject. Die Idee ist die Scene
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
        // 3D-Objekte gibt es nur ein einziges das kann zwar mehrere Flächen haben aber die Flaechen
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
|* Licht-Objekte rauswerfen
|*
\************************************************************************/

void E3dScene::RemoveLightObjects()
{
    SdrObjList* pSubList = GetSubList();
    if(pSubList)
    {
        SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
        while ( a3DIterator.IsMore() )
        {
            E3dObject* pObj = (E3dObject*) a3DIterator.Next();
            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
            if(pObj->ISA(E3dLight))
            {
                // Weg damit
                Remove3DObj(pObj);
            }
        }
    }
}

/*************************************************************************
|*
|* Licht-Objekte erzeugen, um kompatibel zur 4.0
|* speichern zu koennen
|*
\************************************************************************/

void E3dScene::CreateLightObjectsFromLightGroup()
{
    if(aLightGroup.IsLightingEnabled())
    {
        // Global Ambient Light
        const Color& rAmbient = aLightGroup.GetGlobalAmbientLight();
        if(rAmbient != Color(COL_BLACK))
            Insert3DObj(new E3dLight(Vector3D(), rAmbient, 1.0));

        // Andere Lichter
        for(UINT16 a=0;a<BASE3D_MAX_NUMBER_LIGHTS;a++)
        {
            B3dLight& rLight = aLightGroup.GetLightObject((Base3DLightNumber)(Base3DLight0 + a));
            if(rLight.IsEnabled())
            {
                if(rLight.IsDirectionalSource())
                {
                    // erzeuge E3dDistantLight
                    Insert3DObj(new E3dDistantLight(Vector3D(),
                        rLight.GetPosition(),
                        rLight.GetIntensity(Base3DMaterialDiffuse), 1.0));
                }
                else
                {
                    // erzeuge E3dPointLight
                    Insert3DObj(new E3dPointLight(rLight.GetPosition(),
                        rLight.GetIntensity(Base3DMaterialDiffuse), 1.0));
                }
            }
        }
    }
}

/*************************************************************************
|*
|* Beleuchtung aus dem alten Beleuchtungsmodell uebernehmen
|*
\************************************************************************/

void E3dScene::FillLightGroup()
{
    SdrObjList* pSubList = GetSubList();
    BOOL bLampFound = FALSE;

    if(pSubList)
    {
        SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
        Base3DLightNumber eLight = Base3DLight0;

        // AmbientLight aus
        aLightGroup.SetGlobalAmbientLight(Color(COL_BLACK));

        while ( a3DIterator.IsMore() )
        {
            E3dObject* pObj = (E3dObject*) a3DIterator.Next();
            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
            if(pObj->ISA(E3dLight) && eLight <= Base3DLight7)
            {
                E3dLight* pLight = (E3dLight*)pObj;
                bLampFound = TRUE;

                // pLight in Base3D Konvention aktivieren
                if(pLight->IsOn())
                {
                    if(pLight->ISA(E3dPointLight))
                    {
                        // ist ein E3dPointLight
                        // Position, keine Richtung
                        B3dColor aCol(pLight->GetColor().GetColor());
                        aCol *= pLight->GetIntensity();
                        aLightGroup.SetIntensity(aCol, Base3DMaterialDiffuse, eLight);
                        aLightGroup.SetIntensity(Color(COL_WHITE), Base3DMaterialSpecular, eLight);
                        Vector3D aPos = pLight->GetPosition();
                        aLightGroup.SetPosition(aPos, eLight);

                        // Lichtquelle einschalten
                        aLightGroup.Enable(TRUE, eLight);

                        // Naechstes Licht in Base3D
                        eLight = (Base3DLightNumber)(eLight + 1);
                    }
                    else if(pLight->ISA(E3dDistantLight))
                    {
                        // ist ein E3dDistantLight
                        // Richtung, keine Position
                        B3dColor aCol(pLight->GetColor().GetColor());
                        aCol *= pLight->GetIntensity();
                        aLightGroup.SetIntensity(aCol, Base3DMaterialDiffuse, eLight);
                        aLightGroup.SetIntensity(Color(COL_WHITE), Base3DMaterialSpecular, eLight);
                        Vector3D aDir = ((E3dDistantLight *)pLight)->GetDirection();
                        aLightGroup.SetDirection(aDir, eLight);

                        // Lichtquelle einschalten
                        aLightGroup.Enable(TRUE, eLight);

                        // Naechstes Licht in Base3D
                        eLight = (Base3DLightNumber)(eLight + 1);
                    }
                    else
                    {
                        // nur ein E3dLight, gibt ein
                        // ambientes licht, auf globales aufaddieren
                        B3dColor aCol(pLight->GetColor().GetColor());
                        aCol *= pLight->GetIntensity();
                        aCol += (B3dColor &)aLightGroup.GetGlobalAmbientLight();
                        aLightGroup.SetGlobalAmbientLight(aCol);
                    }
                }
            }
        }

        // Alle anderen Lichter ausschalten
        while(eLight <= Base3DLight7)
        {
            aLightGroup.Enable(FALSE, eLight);
            eLight = (Base3DLightNumber)(eLight + 1);
        }
    }

    // Beleuchtung einschalten, falls Lampen vorhanden
    aLightGroup.EnableLighting(bLampFound);
}

/*************************************************************************
|*
|* Lichter zaehlen
|*
\************************************************************************/

UINT16 E3dScene::CountNumberOfLights()
{
    UINT16 nNumLights = 0;

    SdrObjList* pSubList = GetSubList();
    if(pSubList)
    {
        SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
        while ( a3DIterator.IsMore() )
        {
            E3dObject* pObj = (E3dObject*) a3DIterator.Next();
            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
            if(pObj->ISA(E3dLight))
            {
                // Zaehlen...
                nNumLights++;
            }
        }
    }
    return nNumLights;
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
|* Attribute abfragen
|*
\************************************************************************/

void E3dScene::ImpSetLightItemsFromLightGroup()
{
    ImpForceItemSet();

    // TwoSidedLighting
    mpObjectItemSet->Put(Svx3DTwoSidedLightingItem(aLightGroup.GetModelTwoSide()));

    // LightColors
    mpObjectItemSet->Put(Svx3DLightcolor1Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight0)));
    mpObjectItemSet->Put(Svx3DLightcolor2Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight1)));
    mpObjectItemSet->Put(Svx3DLightcolor3Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight2)));
    mpObjectItemSet->Put(Svx3DLightcolor4Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight3)));
    mpObjectItemSet->Put(Svx3DLightcolor5Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight4)));
    mpObjectItemSet->Put(Svx3DLightcolor6Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight5)));
    mpObjectItemSet->Put(Svx3DLightcolor7Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight6)));
    mpObjectItemSet->Put(Svx3DLightcolor8Item(aLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight7)));

    // AmbientColor
    mpObjectItemSet->Put(Svx3DAmbientcolorItem(aLightGroup.GetGlobalAmbientLight()));

    // LightOn
    mpObjectItemSet->Put(Svx3DLightOnOff1Item(aLightGroup.IsEnabled(Base3DLight0)));
    mpObjectItemSet->Put(Svx3DLightOnOff2Item(aLightGroup.IsEnabled(Base3DLight1)));
    mpObjectItemSet->Put(Svx3DLightOnOff3Item(aLightGroup.IsEnabled(Base3DLight2)));
    mpObjectItemSet->Put(Svx3DLightOnOff4Item(aLightGroup.IsEnabled(Base3DLight3)));
    mpObjectItemSet->Put(Svx3DLightOnOff5Item(aLightGroup.IsEnabled(Base3DLight4)));
    mpObjectItemSet->Put(Svx3DLightOnOff6Item(aLightGroup.IsEnabled(Base3DLight5)));
    mpObjectItemSet->Put(Svx3DLightOnOff7Item(aLightGroup.IsEnabled(Base3DLight6)));
    mpObjectItemSet->Put(Svx3DLightOnOff8Item(aLightGroup.IsEnabled(Base3DLight7)));

    // LightDirection
    mpObjectItemSet->Put(Svx3DLightDirection1Item(aLightGroup.GetDirection( Base3DLight0 )));
    mpObjectItemSet->Put(Svx3DLightDirection2Item(aLightGroup.GetDirection( Base3DLight1 )));
    mpObjectItemSet->Put(Svx3DLightDirection3Item(aLightGroup.GetDirection( Base3DLight2 )));
    mpObjectItemSet->Put(Svx3DLightDirection4Item(aLightGroup.GetDirection( Base3DLight3 )));
    mpObjectItemSet->Put(Svx3DLightDirection5Item(aLightGroup.GetDirection( Base3DLight4 )));
    mpObjectItemSet->Put(Svx3DLightDirection6Item(aLightGroup.GetDirection( Base3DLight5 )));
    mpObjectItemSet->Put(Svx3DLightDirection7Item(aLightGroup.GetDirection( Base3DLight6 )));
    mpObjectItemSet->Put(Svx3DLightDirection8Item(aLightGroup.GetDirection( Base3DLight7 )));
}

void E3dScene::ImpSetSceneItemsFromCamera()
{
    ImpForceItemSet();
    Camera3D aSceneCam (GetCamera());

    // ProjectionType
    mpObjectItemSet->Put(Svx3DPerspectiveItem((UINT16)aSceneCam.GetProjection()));

    // CamPos
    mpObjectItemSet->Put(Svx3DDistanceItem((UINT32)(aSceneCam.GetPosition().Z() + 0.5)));

    // FocalLength
    mpObjectItemSet->Put(Svx3DFocalLengthItem((UINT32)((aSceneCam.GetFocalLength() * 100.0) + 0.5)));
}

//////////////////////////////////////////////////////////////////////////////
// ItemSet access

const SfxItemSet& E3dScene::GetItemSet() const
{
    // prepare ItemSet
    if(mpObjectItemSet)
    {
        SfxItemSet aNew(*mpObjectItemSet->GetPool(), SDRATTR_3DSCENE_FIRST, SDRATTR_3DSCENE_LAST);
        aNew.Put(*mpObjectItemSet);
        mpObjectItemSet->ClearItem();
        mpObjectItemSet->Put(aNew);
    }
    else
        ((E3dScene*)this)->ImpForceItemSet();

    // collect all ItemSets in mpGroupItemSet
    sal_uInt32 nCount(pSub->GetObjCount());
    for(sal_uInt32 a(0); a < nCount; a++)
    {
        const SfxItemSet& rSet = pSub->GetObj(a)->GetItemSet();
        SfxWhichIter aIter(rSet);
        sal_uInt16 nWhich(aIter.FirstWhich());

        while(nWhich)
        {
            if(SFX_ITEM_DONTCARE == rSet.GetItemState(nWhich, FALSE))
                mpObjectItemSet->InvalidateItem(nWhich);
            else
                mpObjectItemSet->MergeValue(rSet.Get(nWhich), TRUE);

            nWhich = aIter.NextWhich();
        }
    }

    return *mpObjectItemSet;
}

//////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access

void E3dScene::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
{
    // handle local value change
    if(!nWhich || (nWhich >= SDRATTR_3DSCENE_FIRST && nWhich <= SDRATTR_3DSCENE_LAST))
        SdrAttrObj::ItemChange(nWhich, pNewItem);

    // ItemChange at all contained objects
    List aPostItemChangeList;
    sal_uInt32 nCount(pSub->GetObjCount());

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        SdrObject* pObj = pSub->GetObj(a);
        if(pObj->AllowItemChange(nWhich, pNewItem))
        {
            pObj->ItemChange(nWhich, pNewItem);
            aPostItemChangeList.Insert((void*)pObj, LIST_APPEND);
        }
    }

    for(a = 0; a < aPostItemChangeList.Count(); a++)
    {
        SdrObject* pObj = (SdrObject*)aPostItemChangeList.GetObject(a);
        pObj->PostItemChange(nWhich);
    }
}

void E3dScene::PostItemChange(const sal_uInt16 nWhich)
{
    // call parent
    if(!nWhich || (nWhich >= SDRATTR_3DSCENE_FIRST && nWhich <= SDRATTR_3DSCENE_LAST))
        SdrAttrObj::PostItemChange(nWhich);

    // local changes
    StructureChanged(this);

    switch(nWhich)
    {
        case SDRATTR_3DSCENE_PERSPECTIVE            :
        case SDRATTR_3DSCENE_DISTANCE               :
        case SDRATTR_3DSCENE_FOCAL_LENGTH           :
        {
            // #83387#, #83391#
            // one common function for the camera attributes
            // since SetCamera() sets all three back to the ItemSet
            Camera3D aSceneCam(GetCamera());
            BOOL bChange(FALSE);

            // for SDRATTR_3DSCENE_PERSPECTIVE:
            if(aSceneCam.GetProjection() != GetPerspective())
            {
                aSceneCam.SetProjection(GetPerspective());
                bChange = TRUE;
            }

            // for SDRATTR_3DSCENE_DISTANCE:
            Vector3D aActualPosition = aSceneCam.GetPosition();
            double fNew = GetDistance();
            if(fNew != aActualPosition.Z())
            {
                aSceneCam.SetPosition( Vector3D( aActualPosition.X(), aActualPosition.Y(), fNew) );
                bChange = TRUE;
            }

            // for SDRATTR_3DSCENE_FOCAL_LENGTH:
            fNew = GetFocalLength() / 100.0;
            if(aSceneCam.GetFocalLength() != fNew)
            {
                aSceneCam.SetFocalLength(fNew);
                bChange = TRUE;
            }

            // for all
            if(bChange)
                SetCamera(aSceneCam);

            break;
        }
        case SDRATTR_3DSCENE_TWO_SIDED_LIGHTING     :
        {
            aLightGroup.SetModelTwoSide(GetTwoSidedLighting());
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_1           :
        {
            aLightGroup.SetIntensity( GetLightColor1(), Base3DMaterialDiffuse, Base3DLight0);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_2           :
        {
            aLightGroup.SetIntensity( GetLightColor2(), Base3DMaterialDiffuse, Base3DLight1);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_3           :
        {
            aLightGroup.SetIntensity( GetLightColor3(), Base3DMaterialDiffuse, Base3DLight2);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_4           :
        {
            aLightGroup.SetIntensity( GetLightColor4(), Base3DMaterialDiffuse, Base3DLight3);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_5           :
        {
            aLightGroup.SetIntensity( GetLightColor5(), Base3DMaterialDiffuse, Base3DLight4);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_6           :
        {
            aLightGroup.SetIntensity( GetLightColor6(), Base3DMaterialDiffuse, Base3DLight5);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_7           :
        {
            aLightGroup.SetIntensity( GetLightColor7(), Base3DMaterialDiffuse, Base3DLight6);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_8           :
        {
            aLightGroup.SetIntensity( GetLightColor8(), Base3DMaterialDiffuse, Base3DLight7);
            break;
        }
        case SDRATTR_3DSCENE_AMBIENTCOLOR           :
        {
            aLightGroup.SetGlobalAmbientLight(GetGlobalAmbientColor());
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_1              :
        {
            aLightGroup.Enable( GetLightOnOff1(), Base3DLight0);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_2              :
        {
            aLightGroup.Enable( GetLightOnOff2(), Base3DLight1);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_3              :
        {
            aLightGroup.Enable( GetLightOnOff3(), Base3DLight2);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_4              :
        {
            aLightGroup.Enable( GetLightOnOff4(), Base3DLight3);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_5              :
        {
            aLightGroup.Enable( GetLightOnOff5(), Base3DLight4);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_6              :
        {
            aLightGroup.Enable( GetLightOnOff6(), Base3DLight5);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_7              :
        {
            aLightGroup.Enable( GetLightOnOff7(), Base3DLight6);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_8              :
        {
            aLightGroup.Enable( GetLightOnOff8(), Base3DLight7);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_1       :
        {
            aLightGroup.SetDirection( GetLightDirection1(), Base3DLight0);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_2       :
        {
            aLightGroup.SetDirection( GetLightDirection2(), Base3DLight1);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_3       :
        {
            aLightGroup.SetDirection( GetLightDirection3(), Base3DLight2);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_4       :
        {
            aLightGroup.SetDirection( GetLightDirection4(), Base3DLight3);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_5       :
        {
            aLightGroup.SetDirection( GetLightDirection5(), Base3DLight4);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_6       :
        {
            aLightGroup.SetDirection( GetLightDirection6(), Base3DLight5);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_7       :
        {
            aLightGroup.SetDirection( GetLightDirection7(), Base3DLight6);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_8       :
        {
            aLightGroup.SetDirection( GetLightDirection8(), Base3DLight7);
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet was changed, maybe user wants to react

void E3dScene::ItemSetChanged( const SfxItemSet& rSet )
{
    // call parent
    E3dObject::ItemSetChanged( rSet );

    // set at all contained objects
    sal_uInt32 nCount(pSub->GetObjCount());
    for(sal_uInt32 a(0); a < nCount; a++)
        pSub->GetObj(a)->ItemSetChanged( rSet );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

void E3dScene::PreSave()
{
    // call parent
    E3dObject::PreSave();

    // set at all contained objects
    sal_uInt32 nCount(pSub->GetObjCount());
    for(sal_uInt32 a(0); a < nCount; a++)
        pSub->GetObj(a)->PreSave();
}

void E3dScene::PostSave()
{
    // call parent
    E3dObject::PostSave();

    // set at all contained objects
    sal_uInt32 nCount(pSub->GetObjCount());
    for(sal_uInt32 a(0); a < nCount; a++)
        pSub->GetObj(a)->PostSave();
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

/*************************************************************************
|*
|* ItemPool fuer dieses Objekt wechseln
|*
\************************************************************************/

void E3dScene::MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool)
{
    if(pSrcPool && pDestPool && (pSrcPool != pDestPool))
    {
        // call parent
        E3dObject::MigrateItemPool(pSrcPool, pDestPool);

        // own reaction, but only with outmost scene
        SdrObjList* pSubList = GetSubList();
        if(pSubList && GetScene() == this)
        {
            SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
            while ( a3DIterator.IsMore() )
            {
                E3dObject* pObj = (E3dObject*) a3DIterator.Next();
                DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
                pObj->MigrateItemPool(pSrcPool, pDestPool);
            }
        }
    }
}

Vector3D E3dScene::GetShadowPlaneDirection() const
{
    double fWink = (double)GetShadowSlant() * F_PI180;
    Vector3D aShadowPlaneDir(0.0, sin(fWink), cos(fWink));
    aShadowPlaneDir.Normalize();
    return aShadowPlaneDir;
}

void E3dScene::SetShadowPlaneDirection(const Vector3D& rVec)
{
    UINT16 nSceneShadowSlant = (UINT16)((atan2(rVec.Y(), rVec.Z()) / F_PI180) + 0.5);
    ImpForceItemSet();
    mpObjectItemSet->Put(Svx3DShadowSlantItem(nSceneShadowSlant));
}

// EOF
