/*************************************************************************
 *
 *  $RCSfile: scene3d.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 10:55:03 $
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
    aLightGroup = rDefault.GetDefaultLightGroup();
    aShadowPlaneDirection = rDefault.GetDefaultShadowPlaneDirection();
    eShadeModel = rDefault.GetDefaultShadeModel();
    bDither = rDefault.GetDefaultDither();
    bForceDraftShadeModel = rDefault.GetDefaultForceDraftShadeModel();

    // Alte Werte initialisieren
    aCamera.SetViewWindow(-2, -2, 4, 4);
    aCameraSet.SetDeviceRectangle(-2, 2, -2, 2);
    aCamera.SetDeviceWindow(Rectangle(0, 0, 10, 10));
    Rectangle aRect(0, 0, 10, 10);
    aCameraSet.SetViewportRectangle(aRect);
    nSortingMode = E3D_SORT_FAST_SORTING | E3D_SORT_IN_PARENTS | E3D_SORT_TEST_LENGTH;

    // Schattenebene normalisieren
    aShadowPlaneDirection.Normalize();
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
//-/            const XFillAttrSetItem* pFillAttr = ((E3dCompoundObject*)pObj)->GetFillAttr();
//-/            if ( pFillAttr)
//-/            {
//-/            const SfxItemSet& rSet = pFillAttr->GetItemSet();
            UINT16 nFillTrans = ITEMVALUE(GetItemSet(), XATTR_FILLTRANSPARENCE, XFillTransparenceItem);
            if(nFillTrans)
                bRetval = TRUE;
//-/            }

            if(!bRetval)
            {
                // Linienattribut testen
                UINT16 nLineTransparence = ITEMVALUE( GetItemSet(), XATTR_LINETRANSPARENCE, XLineTransparenceItem );
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
    rOut << aShadowPlaneDirection;

    // neu ab 383:
    rOut << (BOOL)bDither;

    // neu ab 384:
    rOut << (UINT16)eShadeModel;
    rOut << (BOOL)bForceDraftShadeModel;

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
            rIn >> aShadowPlaneDirection;
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
            rIn >> nTmp; eShadeModel = (Base3DShadeModel)nTmp;
        }
        if (aCompat.GetBytesLeft() >= sizeof(BOOL))
        {
            rIn >> bTmp; bForceDraftShadeModel = bTmp;
        }

        // SnapRects der Objekte ungueltig
        SetRectsDirty();

        // Transformationen initialisieren, damit bei RecalcSnapRect()
        // richtig gerechnet wird
        InitTransformationSet();

        RebuildLists();
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
    aShadowPlaneDirection = r3DObj.aShadowPlaneDirection;
    aCameraSet = r3DObj.aCameraSet;

    // neu ab 383:
    aLightGroup = r3DObj.aLightGroup;
    eShadeModel = r3DObj.eShadeModel;
    bDither = r3DObj.bDither;
    bForceDraftShadeModel = r3DObj.bForceDraftShadeModel;

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
    if(GetLightGroup().IsLightingEnabled())
    {
        // Global Ambient Light
        const Color& rAmbient = GetLightGroup().GetGlobalAmbientLight();
        if(rAmbient != Color(COL_BLACK))
            Insert3DObj(new E3dLight(Vector3D(), rAmbient, 1.0));

        // Andere Lichter
        for(UINT16 a=0;a<BASE3D_MAX_NUMBER_LIGHTS;a++)
        {
            B3dLight& rLight = GetLightGroup().
                GetLightObject((Base3DLightNumber)(Base3DLight0 + a));
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
        GetLightGroup().SetGlobalAmbientLight(Color(COL_BLACK));

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
                        GetLightGroup().SetIntensity(aCol, Base3DMaterialDiffuse, eLight);
                        GetLightGroup().SetIntensity(Color(COL_WHITE), Base3DMaterialSpecular, eLight);
                        Vector3D aPos = pLight->GetPosition();
                        GetLightGroup().SetPosition(aPos, eLight);

                        // Lichtquelle einschalten
                        GetLightGroup().Enable(TRUE, eLight);

                        // Naechstes Licht in Base3D
                        eLight = (Base3DLightNumber)(eLight + 1);
                    }
                    else if(pLight->ISA(E3dDistantLight))
                    {
                        // ist ein E3dDistantLight
                        // Richtung, keine Position
                        B3dColor aCol(pLight->GetColor().GetColor());
                        aCol *= pLight->GetIntensity();
                        GetLightGroup().SetIntensity(aCol, Base3DMaterialDiffuse, eLight);
                        GetLightGroup().SetIntensity(Color(COL_WHITE), Base3DMaterialSpecular, eLight);
                        Vector3D aDir = ((E3dDistantLight *)pLight)->GetDirection();
                        GetLightGroup().SetDirection(aDir, eLight);

                        // Lichtquelle einschalten
                        GetLightGroup().Enable(TRUE, eLight);

                        // Naechstes Licht in Base3D
                        eLight = (Base3DLightNumber)(eLight + 1);
                    }
                    else
                    {
                        // nur ein E3dLight, gibt ein
                        // ambientes licht, auf globales aufaddieren
                        B3dColor aCol(pLight->GetColor().GetColor());
                        aCol *= pLight->GetIntensity();
                        aCol += (B3dColor &)GetLightGroup().GetGlobalAmbientLight();
                        GetLightGroup().SetGlobalAmbientLight(aCol);
                    }
                }
            }
        }

        // Alle anderen Lichter ausschalten
        while(eLight <= Base3DLight7)
        {
            GetLightGroup().Enable(FALSE, eLight);
            eLight = (Base3DLightNumber)(eLight + 1);
        }
    }

    // Beleuchtung einschalten, falls Lampen vorhanden
    GetLightGroup().EnableLighting(bLampFound);
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
|* Attribute setzen
|*
\************************************************************************/

//-/void E3dScene::Distribute3DAttributes(const SfxItemSet& rAttr)
//-/{
//-/    // call parent
//-/    E3dObject::Distribute3DAttributes(rAttr);
//-/
//-/    const SfxPoolItem* pPoolItem = NULL;
//-/    B3dLightGroup& rLightGroup = GetLightGroup();
//-/    Camera3D aSceneCam (GetCamera());
//-/    BOOL bSceneCamChanged = FALSE;
//-/
//-/    // ProjectionType
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_PERSPECTIVE, FALSE, &pPoolItem))
//-/    {
//-/        ProjectionType eNew = (ProjectionType)((const SfxUInt16Item*)pPoolItem)->GetValue();
//-/        if(eNew != aSceneCam.GetProjection())
//-/        {
//-/            aSceneCam.SetProjection( eNew );
//-/            bSceneCamChanged = TRUE;
//-/        }
//-/    }
//-/
//-/    // CamPos
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_DISTANCE, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aActualPosition = aSceneCam.GetPosition();
//-/        double fNew = (double)((const SfxUInt32Item*)pPoolItem)->GetValue();
//-/        if(fabs(fNew - aActualPosition.Z()) > 1.0)
//-/        {
//-/            aSceneCam.SetPosition( Vector3D( aActualPosition.X(), aActualPosition.Y(), fNew) );
//-/            bSceneCamChanged = TRUE;
//-/        }
//-/    }
//-/
//-/    // FocalLength
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_FOCAL_LENGTH, FALSE, &pPoolItem))
//-/    {
//-/        double fNew = (double)((const SfxUInt32Item*)pPoolItem)->GetValue() / 100.0;
//-/        if(fNew != aSceneCam.GetFocalLength())
//-/        {
//-/            aSceneCam.SetFocalLength( fNew);
//-/            bSceneCamChanged = TRUE;
//-/        }
//-/    }
//-/
//-/    // TwoSidedLighting
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_TWO_SIDED_LIGHTING, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetModelTwoSide( bNew );
//-/    }
//-/
//-/    // LightColors
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_1, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight0);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_2, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight1);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_3, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight2);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_4, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight3);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_5, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight4);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_6, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight5);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_7, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight6);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_8, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight7);
//-/    }
//-/
//-/    // AmbientColor
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_AMBIENTCOLOR, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetGlobalAmbientLight( aNew );
//-/    }
//-/
//-/    // LightOn
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_1, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight0);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_2, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight1);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_3, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight2);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_4, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight3);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_5, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight4);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_6, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight5);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_7, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight6);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_8, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight7);
//-/    }
//-/
//-/    // LightDirection
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_1, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight0);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_2, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight1);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_3, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight2);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_4, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight3);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_5, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight4);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_6, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight5);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_7, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight6);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_8, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight7);
//-/    }
//-/
//-/    // ShadowSlant
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_SHADOW_SLANT, FALSE, &pPoolItem))
//-/    {
//-/        UINT16 nNew = ((const SfxUInt16Item*)pPoolItem)->GetValue();
//-/        double fWink = (double)nNew * F_PI180;
//-/        Vector3D aVec(0.0, sin(fWink), cos(fWink));
//-/        aVec.Normalize();
//-/        SetShadowPlaneDirection(aVec);
//-/    }
//-/
//-/    // ShadeMode
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_SHADE_MODE, FALSE, &pPoolItem))
//-/    {
//-/        UINT16 nNew = ((const SfxUInt16Item*)pPoolItem)->GetValue();
//-/        if(nNew == 3)
//-/        {
//-/            SetForceDraftShadeModel(TRUE);
//-/            SetShadeModel(Base3DSmooth);
//-/        }
//-/        else
//-/        {
//-/            SetForceDraftShadeModel(FALSE);
//-/            if(nNew == 0)
//-/            {
//-/                SetShadeModel(Base3DFlat);
//-/            }
//-/            else if(nNew == 1)
//-/            {
//-/                SetShadeModel(Base3DPhong);
//-/            }
//-/            else
//-/            {
//-/                // Gouraud
//-/                SetShadeModel(Base3DSmooth);
//-/            }
//-/        }
//-/    }
//-/
//-/    // Nachbehandlung
//-/    if(bSceneCamChanged)
//-/    {
//-/        SetCamera( aSceneCam );
//-/    }
//-/}

//-/void E3dScene::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/    // call parent
//-/    E3dObject::NbcSetAttributes(rAttr, bReplaceAll);
//-/    StructureChanged(this);
//-/
//-/    // special Attr for E3dScene
//-/    const SfxPoolItem* pPoolItem = NULL;
//-/    B3dLightGroup& rLightGroup = GetLightGroup();
//-/    Camera3D aSceneCam (GetCamera());
//-/    BOOL bSceneCamChanged = FALSE;
//-/
//-/    // ProjectionType
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_PERSPECTIVE, FALSE, &pPoolItem))
//-/    {
//-/        ProjectionType eNew = (ProjectionType)((const SfxUInt16Item*)pPoolItem)->GetValue();
//-/        if(eNew != aSceneCam.GetProjection())
//-/        {
//-/            aSceneCam.SetProjection( eNew );
//-/            bSceneCamChanged = TRUE;
//-/        }
//-/    }
//-/
//-/    // CamPos
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_DISTANCE, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aActualPosition = aSceneCam.GetPosition();
//-/        double fNew = (double)((const SfxUInt32Item*)pPoolItem)->GetValue();
//-/        if(fabs(fNew - aActualPosition.Z()) > 1.0)
//-/        {
//-/            aSceneCam.SetPosition( Vector3D( aActualPosition.X(), aActualPosition.Y(), fNew) );
//-/            bSceneCamChanged = TRUE;
//-/        }
//-/    }
//-/
//-/    // FocalLength
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_FOCAL_LENGTH, FALSE, &pPoolItem))
//-/    {
//-/        double fNew = (double)((const SfxUInt32Item*)pPoolItem)->GetValue() / 100.0;
//-/        if(fNew != aSceneCam.GetFocalLength())
//-/        {
//-/            aSceneCam.SetFocalLength( fNew);
//-/            bSceneCamChanged = TRUE;
//-/        }
//-/    }
//-/
//-/    // TwoSidedLighting
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_TWO_SIDED_LIGHTING, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetModelTwoSide( bNew );
//-/    }
//-/
//-/    // LightColors
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_1, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight0);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_2, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight1);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_3, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight2);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_4, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight3);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_5, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight4);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_6, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight5);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_7, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight6);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_8, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight7);
//-/    }
//-/
//-/    // AmbientColor
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_AMBIENTCOLOR, FALSE, &pPoolItem))
//-/    {
//-/        Color aNew = ((const SvxColorItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetGlobalAmbientLight( aNew );
//-/    }
//-/
//-/    // LightOn
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_1, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight0);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_2, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight1);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_3, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight2);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_4, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight3);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_5, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight4);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_6, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight5);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_7, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight6);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTON_8, FALSE, &pPoolItem))
//-/    {
//-/        BOOL bNew = ((const SfxBoolItem*)pPoolItem)->GetValue();
//-/        rLightGroup.Enable( bNew, Base3DLight7);
//-/    }
//-/
//-/    // LightDirection
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_1, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight0);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_2, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight1);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_3, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight2);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_4, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight3);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_5, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight4);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_6, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight5);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_7, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight6);
//-/    }
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_8, FALSE, &pPoolItem))
//-/    {
//-/        Vector3D aNew = ((const SvxVector3DItem*)pPoolItem)->GetValue();
//-/        rLightGroup.SetDirection( aNew, Base3DLight7);
//-/    }
//-/
//-/    // ShadowSlant
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_SHADOW_SLANT, FALSE, &pPoolItem))
//-/    {
//-/        UINT16 nNew = ((const SfxUInt16Item*)pPoolItem)->GetValue();
//-/        double fWink = (double)nNew * F_PI180;
//-/        Vector3D aVec(0.0, sin(fWink), cos(fWink));
//-/        aVec.Normalize();
//-/        SetShadowPlaneDirection(aVec);
//-/    }
//-/
//-/    // ShadeMode
//-/    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_SHADE_MODE, FALSE, &pPoolItem))
//-/    {
//-/        UINT16 nNew = ((const SfxUInt16Item*)pPoolItem)->GetValue();
//-/        if(nNew == 3)
//-/        {
//-/            SetForceDraftShadeModel(TRUE);
//-/            SetShadeModel(Base3DSmooth);
//-/        }
//-/        else
//-/        {
//-/            SetForceDraftShadeModel(FALSE);
//-/            if(nNew == 0)
//-/            {
//-/                SetShadeModel(Base3DFlat);
//-/            }
//-/            else if(nNew == 1)
//-/            {
//-/                SetShadeModel(Base3DPhong);
//-/            }
//-/            else
//-/            {
//-/                // Gouraud
//-/                SetShadeModel(Base3DSmooth);
//-/            }
//-/        }
//-/    }
//-/
//-/    // Nachbehandlung
//-/    if(bSceneCamChanged)
//-/    {
//-/        SetCamera( aSceneCam );
//-/    }
//-/}

/*************************************************************************
|*
|* Attribute abfragen
|*
\************************************************************************/

void E3dScene::Collect3DAttributes(SfxItemSet& rAttr) const
{
    // call parent
    E3dObject::Collect3DAttributes(rAttr);

    // special Attr for E3dCompoundObject
    B3dLightGroup& rLightGroup = ((E3dScene*)this)->GetLightGroup();
    Camera3D aSceneCam (GetCamera());
    double   fSceneCamPosZ = aSceneCam.GetPosition().Z();
    double   fSceneFocal = aSceneCam.GetFocalLength() * 100.0;
    BOOL     bSceneTwoSidedLighting = rLightGroup.GetModelTwoSide();
    Color    aSceneLightColor1 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight0);
    Color    aSceneLightColor2 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight1);
    Color    aSceneLightColor3 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight2);
    Color    aSceneLightColor4 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight3);
    Color    aSceneLightColor5 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight4);
    Color    aSceneLightColor6 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight5);
    Color    aSceneLightColor7 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight6);
    Color    aSceneLightColor8 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight7);
    Color    aSceneAmbientColor = rLightGroup.GetGlobalAmbientLight();
    BOOL     bSceneLightOn1 = rLightGroup.IsEnabled(Base3DLight0);
    BOOL     bSceneLightOn2 = rLightGroup.IsEnabled(Base3DLight1);
    BOOL     bSceneLightOn3 = rLightGroup.IsEnabled(Base3DLight2);
    BOOL     bSceneLightOn4 = rLightGroup.IsEnabled(Base3DLight3);
    BOOL     bSceneLightOn5 = rLightGroup.IsEnabled(Base3DLight4);
    BOOL     bSceneLightOn6 = rLightGroup.IsEnabled(Base3DLight5);
    BOOL     bSceneLightOn7 = rLightGroup.IsEnabled(Base3DLight6);
    BOOL     bSceneLightOn8 = rLightGroup.IsEnabled(Base3DLight7);
    Vector3D aSceneLightDirection1 = rLightGroup.GetDirection( Base3DLight0 );
    Vector3D aSceneLightDirection2 = rLightGroup.GetDirection( Base3DLight1 );
    Vector3D aSceneLightDirection3 = rLightGroup.GetDirection( Base3DLight2 );
    Vector3D aSceneLightDirection4 = rLightGroup.GetDirection( Base3DLight3 );
    Vector3D aSceneLightDirection5 = rLightGroup.GetDirection( Base3DLight4 );
    Vector3D aSceneLightDirection6 = rLightGroup.GetDirection( Base3DLight5 );
    Vector3D aSceneLightDirection7 = rLightGroup.GetDirection( Base3DLight6 );
    Vector3D aSceneLightDirection8 = rLightGroup.GetDirection( Base3DLight7 );
    ProjectionType eScenePT = aSceneCam.GetProjection();
    UINT16   nSceneShadeMode;
    const Vector3D& rShadowVec = ((E3dScene*)this)->GetShadowPlaneDirection();
    UINT16 nSceneShadowSlant = (UINT16)((atan2(rShadowVec.Y(), rShadowVec.Z()) / F_PI180) + 0.5);

    if(bForceDraftShadeModel)
    {
        nSceneShadeMode = 3; // Draft-Modus
    }
    else
    {
        if(eShadeModel == Base3DSmooth)
        {
            nSceneShadeMode = 2; // Gouraud
        }
        else if(eShadeModel == Base3DFlat)
        {
            nSceneShadeMode = 0; // Flat
        }
        else // Base3DPhong
        {
            nSceneShadeMode = 1; // Phong
        }
    }

    // ProjectionType
    rAttr.Put(SfxUInt16Item(SDRATTR_3DSCENE_PERSPECTIVE, (UINT16)eScenePT));

    // CamPos
    rAttr.Put(SfxUInt32Item(SDRATTR_3DSCENE_DISTANCE, (UINT32)(fSceneCamPosZ + 0.5)));

    // FocalLength
    rAttr.Put(SfxUInt32Item(SDRATTR_3DSCENE_FOCAL_LENGTH, (UINT32)(fSceneFocal + 0.5)));

    // TwoSidedLighting
    rAttr.Put(SfxBoolItem(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING, bSceneTwoSidedLighting));

    // LightColors
    rAttr.Put(SvxColorItem(aSceneLightColor1, SDRATTR_3DSCENE_LIGHTCOLOR_1));
    rAttr.Put(SvxColorItem(aSceneLightColor2, SDRATTR_3DSCENE_LIGHTCOLOR_2));
    rAttr.Put(SvxColorItem(aSceneLightColor3, SDRATTR_3DSCENE_LIGHTCOLOR_3));
    rAttr.Put(SvxColorItem(aSceneLightColor4, SDRATTR_3DSCENE_LIGHTCOLOR_4));
    rAttr.Put(SvxColorItem(aSceneLightColor5, SDRATTR_3DSCENE_LIGHTCOLOR_5));
    rAttr.Put(SvxColorItem(aSceneLightColor6, SDRATTR_3DSCENE_LIGHTCOLOR_6));
    rAttr.Put(SvxColorItem(aSceneLightColor7, SDRATTR_3DSCENE_LIGHTCOLOR_7));
    rAttr.Put(SvxColorItem(aSceneLightColor8, SDRATTR_3DSCENE_LIGHTCOLOR_8));

    // AmbientColor
    rAttr.Put(SvxColorItem(aSceneAmbientColor, SDRATTR_3DSCENE_AMBIENTCOLOR));

    // LightOn
    rAttr.Put(SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_1, bSceneLightOn1));
    rAttr.Put(SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_2, bSceneLightOn2));
    rAttr.Put(SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_3, bSceneLightOn3));
    rAttr.Put(SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_4, bSceneLightOn4));
    rAttr.Put(SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_5, bSceneLightOn5));
    rAttr.Put(SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_6, bSceneLightOn6));
    rAttr.Put(SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_7, bSceneLightOn7));
    rAttr.Put(SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_8, bSceneLightOn8));

    // LightDirection
    rAttr.Put(SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_1, aSceneLightDirection1));
    rAttr.Put(SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_2, aSceneLightDirection2));
    rAttr.Put(SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_3, aSceneLightDirection3));
    rAttr.Put(SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_4, aSceneLightDirection4));
    rAttr.Put(SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_5, aSceneLightDirection5));
    rAttr.Put(SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_6, aSceneLightDirection6));
    rAttr.Put(SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_7, aSceneLightDirection7));
    rAttr.Put(SvxVector3DItem(SDRATTR_3DSCENE_LIGHTDIRECTION_8, aSceneLightDirection8));

    // ShadowSlant
    rAttr.Put(SfxUInt16Item(SDRATTR_3DSCENE_SHADOW_SLANT, nSceneShadowSlant));

    // ShadeMode
    rAttr.Put(SfxUInt16Item(SDRATTR_3DSCENE_SHADE_MODE, nSceneShadeMode));
}

// ItemSet access
const sal_uInt16 E3dScene::mnSceneRangeData[4] = { SDRATTR_3DSCENE_FIRST, SDRATTR_3DSCENE_LAST, 0, 0 };
const sal_uInt16 E3dScene::mnAllRangeData[4] = { SDRATTR_START, SDRATTR_END, 0, 0 };

void E3dScene::ImpResetToSceneItems()
{
    ImpForceItemSet();
    mpObjectItemSet->SetRanges(mnSceneRangeData);
    mpObjectItemSet->SetRanges(mnAllRangeData);
}

const SfxItemSet& E3dScene::GetItemSet() const
{
    // collect all ItemSets in mpGroupItemSet
    ((E3dScene*)this)->ImpResetToSceneItems();

    sal_uInt32 nCount(pSub->GetObjCount());
    for(sal_uInt32 a(0); a < nCount; a++)
    {
//-/        mpObjectItemSet->MergeValues(pSub->GetObj(a)->GetItemSet(), TRUE);
        const SfxItemSet& rSet = pSub->GetObj(a)->GetItemSet();
        SfxWhichIter aIter(rSet);
        sal_uInt16 nWhich(aIter.FirstWhich());

        while(nWhich)
        {
            const SfxPoolItem* pItem = NULL;
            rSet.GetItemState(nWhich, TRUE, &pItem);

            if(pItem)
            {
                if(pItem == (SfxPoolItem *)-1)
                    mpObjectItemSet->InvalidateItem(nWhich);
                else
                    mpObjectItemSet->MergeValue(*pItem, TRUE);
            }
            nWhich = aIter.NextWhich();
        }
    }

    return *mpObjectItemSet;
}

//-/const SfxItemSet& E3dScene::GetItemSet() const
//-/{
//-/    if(mpItemSet)
//-/        mpItemSet->ClearItem();
//-/    else
//-/        ((SdrObjGroup*)this)->mpItemSet =
//-/        ((SdrObjGroup*)this)->CreateNewItemSet((SfxItemPool&)(*GetItemPool()));
//-/    DBG_ASSERT(mpItemSet, "Could not create an SfxItemSet(!)");
//-/
//-/    // collect all ItemSets in mpItemSet
//-/    SdrObjList* pSubList = GetSubList();
//-/    if(pSubList)
//-/    {
//-/        SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
//-/        while ( a3DIterator.IsMore() )
//-/        {
//-/            E3dObject* pObj = (E3dObject*) a3DIterator.Next();
//-/            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
//-/            mpItemSet->Put(pObj->GetItemSet());
//-/        }
//-/    }
//-/
//-/    return *mpItemSet;
//-/    // get base items from SdrAttrObj, NOT from E3dObject(!)
//-/    SfxItemSet& rSet = (SfxItemSet&)SdrAttrObj::GetItemSet();
//-/
//-/    // add local 3D items of this object
//-/    Collect3DAttributes(rSet);
//-/
//-/    // add all contained objects
//-/    SdrObjList* pSubList = GetSubList();
//-/    if(pSubList)
//-/    {
//-/        SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
//-/        while ( a3DIterator.IsMore() )
//-/        {
//-/            E3dObject* pObj = (E3dObject*) a3DIterator.Next();
//-/            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
//-/            pObj->Collect3DAttributes(rSet);
//-/        }
//-/    }
//-/
//-/    // return completed ItemSet
//-/    return rSet;
//-/}

void E3dScene::ImpLocalItemValueChange(const SfxPoolItem& rNew)
{
    switch(rNew.Which())
    {
        case SDRATTR_3DSCENE_PERSPECTIVE            :
        {
            Camera3D aSceneCam(GetCamera());
            ProjectionType eNew = (ProjectionType)((const Svx3DPerspectiveItem&)rNew).GetValue();
            aSceneCam.SetProjection( eNew );
            SetCamera( aSceneCam );
            break;
        }
        case SDRATTR_3DSCENE_DISTANCE               :
        {
            Camera3D aSceneCam(GetCamera());
            Vector3D aActualPosition = aSceneCam.GetPosition();
            double fNew = (double)((const Svx3DDistanceItem&)rNew).GetValue();
            if(fabs(fNew - aActualPosition.Z()) > 1.0)
            {
                aSceneCam.SetPosition( Vector3D( aActualPosition.X(), aActualPosition.Y(), fNew) );
                SetCamera( aSceneCam );
            }
            break;
        }
        case SDRATTR_3DSCENE_FOCAL_LENGTH           :
        {
            Camera3D aSceneCam(GetCamera());
            double fNew = (double)((const Svx3DFocalLengthItem&)rNew).GetValue() / 100.0;
            aSceneCam.SetFocalLength( fNew);
            SetCamera( aSceneCam );
            break;
        }
        case SDRATTR_3DSCENE_TWO_SIDED_LIGHTING     :
        {
            BOOL bNew = ((const Svx3DTwoSidedLightingItem&)rNew).GetValue();
            GetLightGroup().SetModelTwoSide( bNew );
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_1           :
        {
            Color aNew = ((const Svx3DLightcolor1Item&)rNew).GetValue();
            GetLightGroup().SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight0);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_2           :
        {
            Color aNew = ((const Svx3DLightcolor2Item&)rNew).GetValue();
            GetLightGroup().SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight1);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_3           :
        {
            Color aNew = ((const Svx3DLightcolor3Item&)rNew).GetValue();
            GetLightGroup().SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight2);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_4           :
        {
            Color aNew = ((const Svx3DLightcolor4Item&)rNew).GetValue();
            GetLightGroup().SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight3);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_5           :
        {
            Color aNew = ((const Svx3DLightcolor5Item&)rNew).GetValue();
            GetLightGroup().SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight4);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_6           :
        {
            Color aNew = ((const Svx3DLightcolor6Item&)rNew).GetValue();
            GetLightGroup().SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight5);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_7           :
        {
            Color aNew = ((const Svx3DLightcolor7Item&)rNew).GetValue();
            GetLightGroup().SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight6);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTCOLOR_8           :
        {
            Color aNew = ((const Svx3DLightcolor8Item&)rNew).GetValue();
            GetLightGroup().SetIntensity( aNew, Base3DMaterialDiffuse, Base3DLight7);
            break;
        }
        case SDRATTR_3DSCENE_AMBIENTCOLOR           :
        {
            Color aNew = ((const Svx3DAmbientcolorItem&)rNew).GetValue();
            GetLightGroup().SetGlobalAmbientLight( aNew );
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_1              :
        {
            BOOL bNew = ((const Svx3DLightOnOff1Item&)rNew).GetValue();
            GetLightGroup().Enable( bNew, Base3DLight0);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_2              :
        {
            BOOL bNew = ((const Svx3DLightOnOff2Item&)rNew).GetValue();
            GetLightGroup().Enable( bNew, Base3DLight1);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_3              :
        {
            BOOL bNew = ((const Svx3DLightOnOff3Item&)rNew).GetValue();
            GetLightGroup().Enable( bNew, Base3DLight2);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_4              :
        {
            BOOL bNew = ((const Svx3DLightOnOff4Item&)rNew).GetValue();
            GetLightGroup().Enable( bNew, Base3DLight3);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_5              :
        {
            BOOL bNew = ((const Svx3DLightOnOff5Item&)rNew).GetValue();
            GetLightGroup().Enable( bNew, Base3DLight4);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_6              :
        {
            BOOL bNew = ((const Svx3DLightOnOff6Item&)rNew).GetValue();
            GetLightGroup().Enable( bNew, Base3DLight5);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_7              :
        {
            BOOL bNew = ((const Svx3DLightOnOff7Item&)rNew).GetValue();
            GetLightGroup().Enable( bNew, Base3DLight6);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTON_8              :
        {
            BOOL bNew = ((const Svx3DLightOnOff8Item&)rNew).GetValue();
            GetLightGroup().Enable( bNew, Base3DLight7);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_1       :
        {
            Vector3D aNew = ((const Svx3DLightDirection1Item&)rNew).GetValue();
            GetLightGroup().SetDirection( aNew, Base3DLight0);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_2       :
        {
            Vector3D aNew = ((const Svx3DLightDirection2Item&)rNew).GetValue();
            GetLightGroup().SetDirection( aNew, Base3DLight1);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_3       :
        {
            Vector3D aNew = ((const Svx3DLightDirection3Item&)rNew).GetValue();
            GetLightGroup().SetDirection( aNew, Base3DLight2);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_4       :
        {
            Vector3D aNew = ((const Svx3DLightDirection4Item&)rNew).GetValue();
            GetLightGroup().SetDirection( aNew, Base3DLight3);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_5       :
        {
            Vector3D aNew = ((const Svx3DLightDirection5Item&)rNew).GetValue();
            GetLightGroup().SetDirection( aNew, Base3DLight4);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_6       :
        {
            Vector3D aNew = ((const Svx3DLightDirection6Item&)rNew).GetValue();
            GetLightGroup().SetDirection( aNew, Base3DLight5);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_7       :
        {
            Vector3D aNew = ((const Svx3DLightDirection7Item&)rNew).GetValue();
            GetLightGroup().SetDirection( aNew, Base3DLight6);
            break;
        }
        case SDRATTR_3DSCENE_LIGHTDIRECTION_8       :
        {
            Vector3D aNew = ((const Svx3DLightDirection8Item&)rNew).GetValue();
            GetLightGroup().SetDirection( aNew, Base3DLight7);
            break;
        }
        case SDRATTR_3DSCENE_SHADOW_SLANT           :
        {
            UINT16 nNew = ((const Svx3DShadowSlantItem&)rNew).GetValue();
            double fWink = (double)nNew * F_PI180;
            Vector3D aVec(0.0, sin(fWink), cos(fWink));
            aVec.Normalize();
            ImpSetShadowPlaneDirection(aVec);
            break;
        }
        case SDRATTR_3DSCENE_SHADE_MODE             :
        {
            UINT16 nNew = ((const Svx3DShadeModeItem&)rNew).GetValue();
            if(nNew == 3)
            {
                ImpSetForceDraftShadeModel(TRUE);
                ImpSetShadeModel(Base3DSmooth);
            }
            else
            {
                ImpSetForceDraftShadeModel(FALSE);

                if(nNew == 0)
                {
                    ImpSetShadeModel(Base3DFlat);
                }
                else if(nNew == 1)
                {
                    ImpSetShadeModel(Base3DPhong);
                }
                else
                {
                    // Gouraud
                    ImpSetShadeModel(Base3DSmooth);
                }
            }
            break;
        }
    }
}

void E3dScene::SetItem( const SfxPoolItem& rItem )
{
    // handle value change
    if(rItem.Which() >= SDRATTR_3DSCENE_FIRST && rItem.Which() <= SDRATTR_3DSCENE_LAST)
    {
        SdrAttrObj::SetItem(rItem);
        ImpLocalItemValueChange(rItem);
    }

    // set at all contained objects
    sal_uInt32 nCount(pSub->GetObjCount());
    for(sal_uInt32 a(0); a < nCount; a++)
        pSub->GetObj(a)->SetItem(rItem);

    StructureChanged(this);
//-/    // set local 3D attributes
//-/    const SfxItemSet& rSet = SdrAttrObj::GetItemSet();
//-/    Distribute3DAttributes(rSet);
//-/
//-/    // set at all contained objects
//-/    SdrObjList* pSubList = GetSubList();
//-/    if(pSubList)
//-/    {
//-/        SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
//-/        while ( a3DIterator.IsMore() )
//-/        {
//-/            E3dObject* pObj = (E3dObject*) a3DIterator.Next();
//-/            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
//-/            pObj->Distribute3DAttributes(rSet);
//-/        }
//-/    }
}

void E3dScene::ClearItem( USHORT nWhich )
{
    if(mpObjectItemSet)
    {
        // handle value change
        if(nWhich >= SDRATTR_3DSCENE_FIRST && nWhich <= SDRATTR_3DSCENE_LAST)
        {
            SdrAttrObj::ClearItem(nWhich);
            ImpLocalItemValueChange(mpObjectItemSet->Get(nWhich));
        }

        // clear at all contained objects
        sal_uInt32 nCount(pSub->GetObjCount());
        for(sal_uInt32 a(0); a < nCount; a++)
            pSub->GetObj(a)->ClearItem(nWhich);

        StructureChanged(this);
//-/        // clear local 3D attributes
//-/        const SfxItemSet& rSet = SdrAttrObj::GetItemSet();
//-/        Distribute3DAttributes(rSet);
//-/
//-/        // clear at all contained objects
//-/        SdrObjList* pSubList = GetSubList();
//-/        if(pSubList)
//-/        {
//-/            SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
//-/            while ( a3DIterator.IsMore() )
//-/            {
//-/                E3dObject* pObj = (E3dObject*) a3DIterator.Next();
//-/                DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
//-/                pObj->Distribute3DAttributes(rSet);
//-/            }
//-/        }
    }
}

void E3dScene::SetItemSet( const SfxItemSet& rSet )
{
    // set base items at SdrAttrObj, NOT at E3dObject(!)
    SdrAttrObj::SetItemSet(rSet);

    // handle value change
    for(sal_uInt16 nWhich(SDRATTR_3DSCENE_FIRST); nWhich <= SDRATTR_3DSCENE_LAST; nWhich++)
        ImpLocalItemValueChange(rSet.Get(nWhich));

    // set at all contained objects
    sal_uInt32 nCount(pSub->GetObjCount());
    for(sal_uInt32 a(0); a < nCount; a++)
        pSub->GetObj(a)->SetItemSet(rSet);

    StructureChanged(this);


//-/    // set local 3D attributes
//-/    Distribute3DAttributes(rSet);
//-/
//-/    // set at all contained objects
//-/    SdrObjList* pSubList = GetSubList();
//-/    if(pSubList)
//-/    {
//-/        SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
//-/        while ( a3DIterator.IsMore() )
//-/        {
//-/            E3dObject* pObj = (E3dObject*) a3DIterator.Next();
//-/            DBG_ASSERT(pObj->ISA(E3dObject), "AW: In Szenen sind nur 3D-Objekte erlaubt!");
//-/            pObj->Distribute3DAttributes(rSet);
//-/        }
//-/    }
}

//-/void E3dScene::TakeAttributes(SfxItemSet& rAttr, FASTBOOL bMerge, FASTBOOL bOnlyHardAttr) const
//-/{
//-/    // call parent
//-/    E3dObject::TakeAttributes(rAttr, bMerge, bOnlyHardAttr);
//-/
//-/    // special Attr for E3dCompoundObject
//-/    const SfxPoolItem* pPoolItem = NULL;
//-/    SfxItemState eState;
//-/
//-/    B3dLightGroup& rLightGroup = ((E3dScene*)this)->GetLightGroup();
//-/    Camera3D aSceneCam (GetCamera());
//-/    double   fSceneCamPosZ = aSceneCam.GetPosition().Z();
//-/    double   fSceneFocal = aSceneCam.GetFocalLength() * 100.0;
//-/    BOOL     bSceneTwoSidedLighting = rLightGroup.GetModelTwoSide();
//-/    Color    aSceneLightColor1 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight0);
//-/    Color    aSceneLightColor2 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight1);
//-/    Color    aSceneLightColor3 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight2);
//-/    Color    aSceneLightColor4 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight3);
//-/    Color    aSceneLightColor5 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight4);
//-/    Color    aSceneLightColor6 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight5);
//-/    Color    aSceneLightColor7 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight6);
//-/    Color    aSceneLightColor8 = rLightGroup.GetIntensity(Base3DMaterialDiffuse, Base3DLight7);
//-/    Color    aSceneAmbientColor = rLightGroup.GetGlobalAmbientLight();
//-/    BOOL     bSceneLightOn1 = rLightGroup.IsEnabled(Base3DLight0);
//-/    BOOL     bSceneLightOn2 = rLightGroup.IsEnabled(Base3DLight1);
//-/    BOOL     bSceneLightOn3 = rLightGroup.IsEnabled(Base3DLight2);
//-/    BOOL     bSceneLightOn4 = rLightGroup.IsEnabled(Base3DLight3);
//-/    BOOL     bSceneLightOn5 = rLightGroup.IsEnabled(Base3DLight4);
//-/    BOOL     bSceneLightOn6 = rLightGroup.IsEnabled(Base3DLight5);
//-/    BOOL     bSceneLightOn7 = rLightGroup.IsEnabled(Base3DLight6);
//-/    BOOL     bSceneLightOn8 = rLightGroup.IsEnabled(Base3DLight7);
//-/    Vector3D aSceneLightDirection1 = rLightGroup.GetDirection( Base3DLight0 );
//-/    Vector3D aSceneLightDirection2 = rLightGroup.GetDirection( Base3DLight1 );
//-/    Vector3D aSceneLightDirection3 = rLightGroup.GetDirection( Base3DLight2 );
//-/    Vector3D aSceneLightDirection4 = rLightGroup.GetDirection( Base3DLight3 );
//-/    Vector3D aSceneLightDirection5 = rLightGroup.GetDirection( Base3DLight4 );
//-/    Vector3D aSceneLightDirection6 = rLightGroup.GetDirection( Base3DLight5 );
//-/    Vector3D aSceneLightDirection7 = rLightGroup.GetDirection( Base3DLight6 );
//-/    Vector3D aSceneLightDirection8 = rLightGroup.GetDirection( Base3DLight7 );
//-/    ProjectionType eScenePT = aSceneCam.GetProjection();
//-/    UINT16   nSceneShadeMode;
//-/
//-/    const Vector3D& rShadowVec = ((E3dScene*)this)->GetShadowPlaneDirection();
//-/    UINT16 nSceneShadowSlant = (UINT16)((atan2(rShadowVec.Y(), rShadowVec.Z()) / F_PI180) + 0.5);
//-/
//-/    if(((E3dScene*)this)->GetForceDraftShadeModel())
//-/    {
//-/        nSceneShadeMode = 3; // Draft-Modus
//-/    }
//-/    else
//-/    {
//-/        if(((E3dScene*)this)->GetShadeModel() == Base3DSmooth)
//-/        {
//-/            nSceneShadeMode = 2; // Gouraud
//-/        }
//-/        else if(((E3dScene*)this)->GetShadeModel() == Base3DFlat)
//-/        {
//-/            nSceneShadeMode = 0; // Flat
//-/        }
//-/        else // Base3DPhong
//-/        {
//-/            nSceneShadeMode = 1; // Phong
//-/        }
//-/    }
//-/
//-/    // ProjectionType
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_PERSPECTIVE, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if((UINT16)eScenePT != ((const SfxUInt16Item*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_PERSPECTIVE);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxUInt16Item(SID_ATTR_3D_PERSPECTIVE, (UINT16)eScenePT));
//-/        }
//-/    }
//-/
//-/    // CamPos
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_DISTANCE, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if((UINT32)(fSceneCamPosZ + 0.5) != ((const SfxUInt32Item*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_DISTANCE);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxUInt32Item(SID_ATTR_3D_DISTANCE, (UINT32)(fSceneCamPosZ + 0.5)));
//-/        }
//-/    }
//-/
//-/    // FocalLength
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_FOCAL_LENGTH, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if((UINT32)(fSceneFocal + 0.5) != ((const SfxUInt32Item*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_FOCAL_LENGTH);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxUInt32Item(SID_ATTR_3D_FOCAL_LENGTH, (UINT32)(fSceneFocal + 0.5)));
//-/        }
//-/    }
//-/
//-/    // TwoSidedLighting
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_TWO_SIDED_LIGHTING, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bSceneTwoSidedLighting != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_TWO_SIDED_LIGHTING);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_TWO_SIDED_LIGHTING, bSceneTwoSidedLighting));
//-/        }
//-/    }
//-/
//-/    // LightColors
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_1, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightColor1 != ((const SvxColorItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTCOLOR_1);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxColorItem(aSceneLightColor1, SID_ATTR_3D_LIGHTCOLOR_1));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_2, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightColor2 != ((const SvxColorItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTCOLOR_2);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxColorItem(aSceneLightColor2, SID_ATTR_3D_LIGHTCOLOR_2));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_3, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightColor3 != ((const SvxColorItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTCOLOR_3);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxColorItem(aSceneLightColor3, SID_ATTR_3D_LIGHTCOLOR_3));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_4, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightColor4 != ((const SvxColorItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTCOLOR_4);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxColorItem(aSceneLightColor4, SID_ATTR_3D_LIGHTCOLOR_4));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_5, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightColor5 != ((const SvxColorItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTCOLOR_5);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxColorItem(aSceneLightColor5, SID_ATTR_3D_LIGHTCOLOR_5));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_6, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightColor6 != ((const SvxColorItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTCOLOR_6);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxColorItem(aSceneLightColor6, SID_ATTR_3D_LIGHTCOLOR_6));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_7, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightColor7 != ((const SvxColorItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTCOLOR_7);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxColorItem(aSceneLightColor7, SID_ATTR_3D_LIGHTCOLOR_7));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTCOLOR_8, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightColor8 != ((const SvxColorItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTCOLOR_8);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxColorItem(aSceneLightColor8, SID_ATTR_3D_LIGHTCOLOR_8));
//-/        }
//-/    }
//-/
//-/    // AmbientColor
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_AMBIENTCOLOR, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneAmbientColor != ((const SvxColorItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_AMBIENTCOLOR);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxColorItem(aSceneAmbientColor, SID_ATTR_3D_AMBIENTCOLOR));
//-/        }
//-/    }
//-/
//-/    // LightOn
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTON_1, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bSceneLightOn1 != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTON_1);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_1, bSceneLightOn1));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTON_2, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bSceneLightOn2 != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTON_2);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_2, bSceneLightOn2));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTON_3, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bSceneLightOn3 != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTON_3);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_3, bSceneLightOn3));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTON_4, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bSceneLightOn4 != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTON_4);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_4, bSceneLightOn4));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTON_5, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bSceneLightOn5 != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTON_5);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_5, bSceneLightOn5));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTON_6, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bSceneLightOn6 != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTON_6);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_6, bSceneLightOn6));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTON_7, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bSceneLightOn7 != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTON_7);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_7, bSceneLightOn7));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTON_8, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(bSceneLightOn8 != ((const SfxBoolItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTON_8);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxBoolItem(SID_ATTR_3D_LIGHTON_8, bSceneLightOn8));
//-/        }
//-/    }
//-/
//-/    // LightDirection
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_1, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightDirection1 != ((const SvxVector3DItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTDIRECTION_1);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_1, aSceneLightDirection1));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_2, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightDirection2 != ((const SvxVector3DItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTDIRECTION_2);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_2, aSceneLightDirection2));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_3, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightDirection3 != ((const SvxVector3DItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTDIRECTION_3);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_3, aSceneLightDirection3));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_4, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightDirection4 != ((const SvxVector3DItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTDIRECTION_4);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_4, aSceneLightDirection4));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_5, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightDirection5 != ((const SvxVector3DItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTDIRECTION_5);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_5, aSceneLightDirection5));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_6, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightDirection6 != ((const SvxVector3DItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTDIRECTION_6);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_6, aSceneLightDirection6));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_7, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightDirection7 != ((const SvxVector3DItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTDIRECTION_7);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_7, aSceneLightDirection7));
//-/        }
//-/    }
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_LIGHTDIRECTION_8, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(aSceneLightDirection8 != ((const SvxVector3DItem*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_LIGHTDIRECTION_8);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SvxVector3DItem(SID_ATTR_3D_LIGHTDIRECTION_8, aSceneLightDirection8));
//-/        }
//-/    }
//-/
//-/    // ShadowSlant
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_SHADOW_SLANT, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(nSceneShadowSlant != ((const SfxUInt16Item*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_SHADOW_SLANT);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxUInt16Item(SID_ATTR_3D_SHADOW_SLANT, nSceneShadowSlant));
//-/        }
//-/    }
//-/
//-/    // ShadeMode
//-/    eState = rAttr.GetItemState(SID_ATTR_3D_SHADE_MODE, FALSE, &pPoolItem);
//-/    if(eState == SFX_ITEM_SET)
//-/    {
//-/        // Ist gesetzt
//-/        if(nSceneShadeMode != ((const SfxUInt16Item*)pPoolItem)->GetValue())
//-/        {
//-/            // SfxPoolItem muss invalidiert werden
//-/            rAttr.InvalidateItem(SID_ATTR_3D_SHADE_MODE);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        if(!(eState == SFX_ITEM_DONTCARE))
//-/        {
//-/            // Item gab es noch nicht, setze es
//-/            rAttr.Put(SfxUInt16Item(SID_ATTR_3D_SHADE_MODE, nSceneShadeMode));
//-/        }
//-/    }
//-/}

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

void E3dScene::ImpSetShadowPlaneDirection(const Vector3D& rNew)
{
    aShadowPlaneDirection = rNew;
    aShadowPlaneDirection.Normalize();
}

void E3dScene::ImpSetShadeModel(Base3DShadeModel eNew)
{
    if(eShadeModel != eNew)
    {
        eShadeModel = eNew;
        SetRectsDirty();
    }
}

void E3dScene::ImpSetForceDraftShadeModel(BOOL bNew)
{
    if(bNew != bForceDraftShadeModel)
    {
        bForceDraftShadeModel = bNew;
        SetRectsDirty();
    }
}

// EOF
