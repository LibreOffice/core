/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sphere3d.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:58:52 $
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

#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _E3D_SPHERE3D_HXX
#include "sphere3d.hxx"
#endif

#ifndef _POLY3D_HXX
#include "poly3d.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SVX3DITEMS_HXX
#include "svx3ditems.hxx"
#endif

#ifndef _SDR_PROPERTIES_E3DSPHEREPROPERTIES_HXX
#include <svx/sdr/properties/e3dsphereproperties.hxx>
#endif

#ifndef _BGFX_VECTOR_B3DVECTOR_HXX
#include <basegfx/vector/b3dvector.hxx>
#endif

#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dSphereObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dSphereProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dSphereObj, E3dCompoundObject);

/*************************************************************************
|*
|* Kugel aus Polygonfacetten nach Laengen und Breitengrad aufbauen
|*
\************************************************************************/

E3dSphereObj::E3dSphereObj(E3dDefaultAttributes& rDefault, const Vector3D& rCenter, const Vector3D& r3DSize)
:   E3dCompoundObject(rDefault)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // Uebergebene drueberbuegeln
    aCenter = rCenter;
    aSize = r3DSize;

    // Geometrie erzeugen
    CreateGeometry();
}

E3dSphereObj::E3dSphereObj()
:   E3dCompoundObject()
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

/*************************************************************************
|*
|* Kugel erzeugen ohne die Polygone darin zu erzeugen
|*
\************************************************************************/

// FG: Dieser Aufruf erfolgt von der 3D-Object Factory (objfac3d.cxx) und zwar ausschliesslich beim
//     laden von Dokumenten. Hier braucht man keinen CreateSphere-Aufruf, denn die wirkliche
//     Anzahl Segmente ist ja noch nicht bekannt. Dies war bis zum 10.2.97 ein (kleines)
//     Speicherleck.
E3dSphereObj::E3dSphereObj(int /*dummy*/) // den Parameter braucht es um unterscheiden zu koennen, welcher
{                                     // der beiden Konstruktoren gemeint ist. Der obige halt per Default
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

void E3dSphereObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    // Defaults setzen
    aCenter = rDefault.GetDefaultSphereCenter();
    aSize = rDefault.GetDefaultSphereSize();
}

/*************************************************************************
|*
|* Give out simple line geometry
|*
\************************************************************************/

::basegfx::B3DPolyPolygon E3dSphereObj::Get3DLineGeometry() const
{
    ::basegfx::B3DPolyPolygon aRetval;

    // add geometry describing polygons to rLinePolyPolygon
    const sal_uInt32 nCntHor((sal_uInt32)GetHorizontalSegments());
    const sal_uInt32 nCntVer((sal_uInt32)GetVerticalSegments());
    const sal_Bool bCreateHorizontal(sal_True);
    const sal_Bool bCreateVertical(sal_True);

    if(nCntHor && nCntVer && (bCreateHorizontal || bCreateVertical))
    {
        const double fHInc((double)DEG2RAD(360) / nCntHor);
        const double fVInc((double)DEG2RAD(180) / nCntVer);
        const ::basegfx::B3DVector aRadius(aSize.X() / 2.0, aSize.Y() / 2.0, aSize.Z() / 2.0);
        ::basegfx::B3DPoint aCenterPos(aCenter.X(), aCenter.Y(), aCenter.Z());
        double fHAng(0.0);
        ::basegfx::B3DPolygon aAllPoints;

        // create all sphere points
        for(sal_uInt32 nH(0L); nH < nCntHor; nH++)
        {
            const double fHSin(sin(fHAng));
            const double fHCos(cos(fHAng));
            fHAng += fHInc;
            double fVAng(DEG2RAD(90) - fVInc);

            for(sal_uInt32 nV(1L); nV < nCntVer; nV++)
            {
                const double fVSin(sin(fVAng));
                const double fVCos(cos(fVAng));
                fVAng -= fVInc;

                ::basegfx::B3DPoint aNewPos(
                    aCenterPos.getX() + (aRadius.getX() * fVCos) * fHCos,
                    aCenterPos.getY() + aRadius.getY() * fVSin,
                    aCenterPos.getZ() + (aRadius.getZ() * fVCos) * fHSin);

                aAllPoints.append(aNewPos);
            }
        }

        // create horizontal lines
        if(bCreateHorizontal)
        {
            for(sal_uInt32 a(1L); a < nCntVer; a++)
            {
                ::basegfx::B3DPolygon aNewHor;
                sal_uInt32 nStartIndex(a - 1L);

                for(sal_uInt32 b(0L); b < nCntHor; b++)
                {
                    aNewHor.append(aAllPoints.getB3DPoint(nStartIndex));
                    nStartIndex += (nCntVer - 1L);
                }

                aNewHor.append(aNewHor.getB3DPoint(0L));
                aRetval.append(aNewHor);
            }
        }

        // create vertical lines
        if(bCreateVertical)
        {
            const ::basegfx::B3DPoint aTopPos(aCenterPos.getX(), aCenterPos.getY() + aRadius.getY(), aCenterPos.getZ());
            const ::basegfx::B3DPoint aBottomPos(aCenterPos.getX(), aCenterPos.getY() - aRadius.getY(), aCenterPos.getZ());

            for(sal_uInt32 a(0L); a < nCntHor; a++)
            {
                ::basegfx::B3DPolygon aNewVer;
                aNewVer.append(aTopPos);
                sal_uInt32 nStartIndex(a * (nCntVer - 1L));

                for(sal_uInt32 b(1L); b < nCntVer; b++, nStartIndex++)
                {
                    aNewVer.append(aAllPoints.getB3DPoint(nStartIndex));
                }

                aNewVer.append(aBottomPos);
                aRetval.append(aNewVer);
            }
        }
    }

    return aRetval;
}

/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/

void E3dSphereObj::CreateGeometry()
{
    Polygon3D   aRect3D(4);
    Polygon3D   aNormal3D(4);
    Polygon3D   aTexture3D(4);

    if ( GetHorizontalSegments() <  3  )
    {
        GetProperties().SetObjectItemDirect(Svx3DHorizontalSegmentsItem(3));
    }

    if ( GetHorizontalSegments() > 100 )
    {
        GetProperties().SetObjectItemDirect(Svx3DHorizontalSegmentsItem(100));
    }

    if ( GetVerticalSegments() <  2  )
    {
        GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(2));
    }

    if ( GetVerticalSegments() > 100 )
    {
        GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(100));
    }

    // Start der Geometrieerzeugung ankuendigen
    StartCreateGeometry();

    Vector3D aRadius = aSize / 2;
    double fHSin1, fHSin2, fHCos1, fHCos2;
    double fHAng = 0;
    double fHInc = DEG2RAD(360) / GetHorizontalSegments();
    double fVInc = DEG2RAD(180) / GetVerticalSegments();

    fHSin2 = 0.0; // sin(0)
    fHCos2 = 1.0; // cos(0)
    sal_uInt16 nUpperBound = (sal_uInt16)GetHorizontalSegments();

    for (USHORT nH = 0; nH < nUpperBound; nH++)
    {
        fHAng += fHInc;
        fHSin1 = fHSin2;
        fHSin2 = sin(fHAng);
        fHCos1 = fHCos2;
        fHCos2 = cos(fHAng);

        double fRx, fRz;
        double fVAng = DEG2RAD(90);
        double fVSin1, fVSin2, fVCos1, fVCos2;
        fVSin2 = 1.0; // sin(90)
        fVCos2 = 0.0; // cos(90)
        sal_uInt16 nUpperVert = (sal_uInt16)GetVerticalSegments();

        for (USHORT nV = 0; nV < nUpperVert; nV++)
        {
            Vector3D aPos;

            fVAng -= fVInc;
            fVSin1 = fVSin2;
            fVSin2 = sin(fVAng);
            fVCos1 = fVCos2;
            fVCos2 = cos(fVAng);

            aPos = aCenter;
            fRx = aRadius.X() * fVCos1;
            fRz = aRadius.Z() * fVCos1;
            aPos.X() += fRx * fHCos1;
            aPos.Y() += aRadius.Y() * fVSin1;
            aPos.Z() += fRz * fHSin1;
            aRect3D[1] = aPos;

            aPos.X() = aCenter.X() + fRx * fHCos2;
            aPos.Z() = aCenter.Z() + fRz * fHSin2;
            aRect3D[2] = aPos;

            aPos = aCenter;
            fRx = aRadius.X() * fVCos2;
            fRz = aRadius.Z() * fVCos2;
            aPos.X() += fRx * fHCos1;
            aPos.Y() += aRadius.Y() * fVSin2;
            aPos.Z() += fRz * fHSin1;
            aRect3D[0] = aPos;

            aPos.X() = aCenter.X() + fRx * fHCos2;
            aPos.Z() = aCenter.Z() + fRz * fHSin2;
            aRect3D[3] = aPos;

            if(GetCreateTexture())
            {
                aTexture3D[1].X() =
                    (double)(nUpperBound - (nH + 1)) / (double)nUpperBound;
                aTexture3D[1].Y() =
                    (double)nV / (double)nUpperVert;

                aTexture3D[2].X() =
                    (double)((nUpperBound - (nH + 1)) - 1) / (double)nUpperBound;
                aTexture3D[2].Y() = aTexture3D[1].Y();

                aTexture3D[3].X() = aTexture3D[2].X();
                aTexture3D[3].Y() =
                    (double)(nV+1) / (double)nUpperVert;

                aTexture3D[0].X() = aTexture3D[1].X();
                aTexture3D[0].Y() = aTexture3D[3].Y();
            }

            if(GetCreateNormals())
            {
                aNormal3D = aRect3D;
                aNormal3D[0].Normalize();
                aNormal3D[1].Normalize();
                aNormal3D[2].Normalize();
                aNormal3D[3].Normalize();

                if(GetCreateTexture())
                    AddGeometry(aRect3D, aNormal3D, aTexture3D, FALSE);
                else
                    AddGeometry(aRect3D, aNormal3D, FALSE);
            }
            else
            {
                AddGeometry(aRect3D, FALSE);
            }
        }
    }

    // call parent
    E3dCompoundObject::CreateGeometry();
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

UINT16 E3dSphereObj::GetObjIdentifier() const
{
    return E3D_SPHEREOBJ_ID;
}

/*************************************************************************
|*
|* Wireframe erzeugen
|*
\************************************************************************/

//BFS01void E3dSphereObj::CreateWireframe(Polygon3D& rWirePoly, const Matrix4D* pTf,
//BFS01 E3dDragDetail eDetail)
//BFS01{
//BFS01 if ( eDetail == E3DDETAIL_ALLLINES ||
//BFS01     (eDetail == E3DDETAIL_DEFAULT && GetDragDetail() == E3DDETAIL_ALLLINES) )
//BFS01 {
//BFS01     // Detailliert erzeugen
//BFS01 }
//BFS01 else
//BFS01 {
//BFS01     // call parent
//BFS01     E3dObject::CreateWireframe(rWirePoly, pTf, eDetail);
//BFS01 }
//BFS01}

/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus n Polygonen
|*
\************************************************************************/

SdrObject *E3dSphereObj::DoConvertToPolyObj(BOOL /*bBezier*/) const
{
    return NULL;
}

/*************************************************************************
|*
|* Leer-Konstruktor
|*
\************************************************************************/

void E3dSphereObj::ReSegment(long nHSegs, long nVSegs)
{
    if((nHSegs != GetHorizontalSegments() || nVSegs != GetVerticalSegments()) &&
        (nHSegs != 0 || nVSegs != 0))
    {
        GetProperties().SetObjectItemDirect(Svx3DHorizontalSegmentsItem(nHSegs));
        GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nVSegs));

        bGeometryValid = FALSE;
    }
}

/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

//BFS01void E3dSphereObj::WriteData(SvStream& rOut) const
//BFS01{
//BFS01#ifndef SVX_LIGHT
//BFS01 long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
//BFS01 if(nVersion < 3800)
//BFS01 {
//BFS01     // Alte Geometrie erzeugen, um die E3dPolyObj's zu haben
//BFS01     ((E3dCompoundObject*)this)->ReCreateGeometry(TRUE);
//BFS01 }
//BFS01
//BFS01 // call parent
//BFS01 SdrAttrObj::WriteData(rOut);
//BFS01
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 SdrDownCompat aCompat(rOut, STREAM_WRITE);
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("E3dSphereObj");
//BFS01#endif
//BFS01
//BFS01 if (rOut.GetVersion() < 3560) // FG: kleiner als die Final Beta der Version 4.0
//BFS01 {
//BFS01     pSub->Save(rOut);
//BFS01 }
//BFS01 else
//BFS01 {
//BFS01     // [FG] Jetzt wird die Kindliste abgeklapptert, allerdings weiss ich im Gegensatz zu
//BFS01     // Joe dass es nur E3dPolyObj - Kindobjekte sein koennen.
//BFS01     // Jedes dieser Objekte frage ich ob es eigene Attribute enthaelt.  Falls OwnStyle ()
//BFS01     // true liefert, werde ich das Polygon nicht wegspeichern.
//BFS01
//BFS01     SdrObjListIter aIter(*pSub,IM_FLAT);
//BFS01     while (aIter.IsMore()) {
//BFS01         E3dPolyObj* pObj=(E3dPolyObj *) aIter.Next();
//BFS01         if ((!pObj->IsNotPersistent()) && (pObj->OwnAttrs() || pObj->OwnStyle()))
//BFS01         {
//BFS01             rOut<<*pObj;
//BFS01         }
//BFS01         if (pSub->GetModel()!=NULL) pSub->GetModel()->IncProgress();
//BFS01     }
//BFS01     SdrIOHeader(rOut,STREAM_WRITE,SdrIOEndeID); // Endemarke
//BFS01 }
//BFS01
//BFS01 // Daß hier gehört zum E3dObject (ohne Basisklassen);
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
//BFS01     E3dObject::WriteOnlyOwnMembers(rOut);
//BFS01 }
//BFS01 // Das gehört zu E3dSphere
//BFS01 rOut << GetHorizontalSegments();
//BFS01
//BFS01 rOut << GetVerticalSegments();
//BFS01
//BFS01 rOut << aCenter;
//BFS01 rOut << aSize;
//BFS01
//BFS01 // Das hier ist ein Merkmal eines Compound-Objektes
//BFS01 rOut << GetDoubleSided();
//BFS01
//BFS01 // Ab Version 395 (8.6.98): Parameter aus dem Objekt
//BFS01 // E3dCompoundObject. Da irgendwann mal jemand die Ableitungs-
//BFS01 // hierarchie beim FileFormat unterbrochen hat, wurden diese Attribute
//BFS01 // bisher NOCH NIE gespeichert (Grrr). Diese Stelle muss nun natuerlich
//BFS01 // auch IMMER MITGEPFLEGT werden, wenn sich Parameter in
//BFS01 // E3dCompoundObject oder E3dObject aendern.
//BFS01 rOut << GetDoubleSided();
//BFS01
//BFS01 rOut << BOOL(bCreateNormals);
//BFS01 rOut << BOOL(bCreateTexture);
//BFS01
//BFS01 sal_uInt16 nVal = GetNormalsKind();
//BFS01 rOut << BOOL(nVal > 0);
//BFS01 rOut << BOOL(nVal > 1);
//BFS01
//BFS01 nVal = GetTextureProjectionX();
//BFS01 rOut << BOOL(nVal > 0);
//BFS01 rOut << BOOL(nVal > 1);
//BFS01
//BFS01 nVal = GetTextureProjectionY();
//BFS01 rOut << BOOL(nVal > 0);
//BFS01 rOut << BOOL(nVal > 1);
//BFS01
//BFS01 rOut << BOOL(GetShadow3D());
//BFS01
//BFS01 rOut << GetMaterialAmbientColor();
//BFS01 rOut << GetMaterialColor();
//BFS01 rOut << GetMaterialSpecular();
//BFS01 rOut << GetMaterialEmission();
//BFS01 rOut << GetMaterialSpecularIntensity();
//BFS01
//BFS01 aBackMaterial.WriteData(rOut);
//BFS01
//BFS01 rOut << (UINT16)GetTextureKind();
//BFS01
//BFS01 rOut << (UINT16)GetTextureMode();
//BFS01
//BFS01 rOut << BOOL(GetNormalsInvert());
//BFS01
//BFS01 // neu ab 534: (hat noch gefehlt)
//BFS01 rOut << BOOL(GetTextureFilter());
//BFS01
//BFS01 if(nVersion < 3800)
//BFS01 {
//BFS01     // Geometrie neu erzeugen, um E3dPolyObj's wieder loszuwerden
//BFS01     ((E3dCompoundObject*)this)->ReCreateGeometry();
//BFS01 }
//BFS01#endif
//BFS01}

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

//BFS01void E3dSphereObj::ReadData31(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 SdrDownCompat aCompat(rIn, STREAM_READ);
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("E3dSphereObj");
//BFS01#endif
//BFS01 // dann die Member
//BFS01 UINT16  nTmp16;
//BFS01 sal_Int32 nTmp32;
//BFS01
//BFS01 pSub->Load(rIn, *pPage);
//BFS01 // FG: Die Daten des 3D-Objektes
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
//BFS01 // FG: Die Daten der Kugel
//BFS01
//BFS01 rIn >> nTmp32;
//BFS01 GetProperties().SetObjectItemDirect(Svx3DHorizontalSegmentsItem(nTmp32));
//BFS01
//BFS01 rIn >> nTmp32;
//BFS01 GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nTmp32));
//BFS01
//BFS01 rIn >> aCenter;
//BFS01 rIn >> aSize;
//BFS01
//BFS01 bBoundVolValid = FALSE;
//BFS01
//BFS01 // Geometrie neu erzeugen
//BFS01 ReCreateGeometry();
//BFS01}

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

//BFS01void E3dSphereObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 // FG (10.2.97) Der Pointer ist ein Kunstgriff er wird in der ReadData-Routine erzeugt dort werden
//BFS01 //    die abgespeicherten attributierten Flaechen abgelegt. Danach wird eine Kugel mit Default-Paramtern
//BFS01 //    erzeugt und dann werden die per Default erzeugten einfach, falls noetig mit denen aus dieser Liste
//BFS01 //    ausgetauscht. Am Ende von ReadData wird die Liste wieder zerstoert.
//BFS01 E3dObjList *pLoadedE3dPolyObjs;
//BFS01
//BFS01 if (ImpCheckSubRecords (rHead, rIn))
//BFS01 {
//BFS01     // leider kann das E3dLatheObj nicht auf E3dObject abgestuetzt werden,
//BFS01     // da neue Member hinzugekommen sind und die Kompatibilitaet erhalten
//BFS01     // bleiben muss.
//BFS01     SdrAttrObj::ReadData(rHead, rIn);
//BFS01     if ((rIn.GetVersion() < 3560) || (rHead.GetVersion() <= 12))
//BFS01     {
//BFS01         ReadData31(rHead, rIn);
//BFS01         return;
//BFS01     }
//BFS01
//BFS01
//BFS01     // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01     SdrDownCompat aCompat(rIn, STREAM_READ);
//BFS01#ifdef DBG_UTIL
//BFS01     aCompat.SetID("E3dSphereObj");
//BFS01#endif
//BFS01     // dann die Member
//BFS01     BOOL    bMyDoubleSided;
//BFS01
//BFS01     // [FG]: Es wird erstmal gelesen was da ist, ob es vollstaendig ist zeigt sich bei
//BFS01     // dem Aufruf von CreateSphere, dort werden die fehlenden Flaechen einfach
//BFS01     // erzeugt.
//BFS01     pLoadedE3dPolyObjs = new E3dObjList(NULL, NULL);
//BFS01     pLoadedE3dPolyObjs->SetOwnerObj(this);
//BFS01     pLoadedE3dPolyObjs->SetListKind(SDROBJLIST_GROUPOBJ);
//BFS01     pLoadedE3dPolyObjs->Load(rIn, *pPage);
//BFS01     E3dObject::ReadOnlyOwnMembers(rHead, rIn);
//BFS01
//BFS01     if (aCompat.GetBytesLeft ())
//BFS01     {
//BFS01         // neue Member
//BFS01         sal_Int32 nTmp32;
//BFS01
//BFS01         rIn >> nTmp32;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DHorizontalSegmentsItem(nTmp32));
//BFS01
//BFS01         rIn >> nTmp32;
//BFS01         GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nTmp32));
//BFS01
//BFS01         rIn >> aCenter;
//BFS01         rIn >> aSize;
//BFS01         if (aCompat.GetBytesLeft ())
//BFS01         {
//BFS01             rIn >> bMyDoubleSided;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(bMyDoubleSided));
//BFS01         }
//BFS01
//BFS01         if (aCompat.GetBytesLeft())
//BFS01         {
//BFS01             // Ab Version 395 (8.6.98): Parameter aus dem Objekt
//BFS01             // E3dCompoundObject. Da irgendwann mal jemand die Ableitungs-
//BFS01             // hierarchie beim FileFormat unterbrochen hat, wurden diese Attribute
//BFS01             // bisher NOCH NIE gespeichert (Grrr). Diese Stelle muss nun natuerlich
//BFS01             // auch IMMER MITGEPFLEGT werden, wenn sich Parameter in
//BFS01             // E3dCompoundObject oder E3dObject aendern.
//BFS01             BOOL bTmp, bTmp2;
//BFS01             sal_uInt16 nTmp;
//BFS01
//BFS01             rIn >> bTmp;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DDoubleSidedItem(bTmp));
//BFS01
//BFS01             rIn >> bTmp; bCreateNormals = bTmp;
//BFS01             rIn >> bTmp; bCreateTexture = bTmp;
//BFS01
//BFS01             rIn >> bTmp;
//BFS01             rIn >> bTmp2;
//BFS01             if(bTmp == FALSE && bTmp2 == FALSE)
//BFS01                 nTmp = 0;
//BFS01             else if(bTmp == TRUE && bTmp2 == FALSE)
//BFS01                 nTmp = 1;
//BFS01             else
//BFS01                 nTmp = 2;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DNormalsKindItem(nTmp));
//BFS01
//BFS01             rIn >> bTmp;
//BFS01             rIn >> bTmp2;
//BFS01             if(bTmp == FALSE && bTmp2 == FALSE)
//BFS01                 nTmp = 0;
//BFS01             else if(bTmp == TRUE && bTmp2 == FALSE)
//BFS01                 nTmp = 1;
//BFS01             else
//BFS01                 nTmp = 2;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DTextureProjectionXItem(nTmp));
//BFS01
//BFS01             rIn >> bTmp;
//BFS01             rIn >> bTmp2;
//BFS01             if(bTmp == FALSE && bTmp2 == FALSE)
//BFS01                 nTmp = 0;
//BFS01             else if(bTmp == TRUE && bTmp2 == FALSE)
//BFS01                 nTmp = 1;
//BFS01             else
//BFS01                 nTmp = 2;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DTextureProjectionYItem(nTmp));
//BFS01
//BFS01             rIn >> bTmp;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DShadow3DItem(bTmp));
//BFS01
//BFS01             Color aCol;
//BFS01
//BFS01             rIn >> aCol;
//BFS01             SetMaterialAmbientColor(aCol);
//BFS01
//BFS01             rIn >> aCol;
//BFS01             // do NOT use, this is the old 3D-Color(!)
//BFS01             // SetItem(XFillColorItem(String(), aCol));
//BFS01
//BFS01             rIn >> aCol;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DMaterialSpecularItem(aCol));
//BFS01
//BFS01             rIn >> aCol;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DMaterialEmissionItem(aCol));
//BFS01
//BFS01             rIn >> nTmp;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DMaterialSpecularIntensityItem(nTmp));
//BFS01
//BFS01             aBackMaterial.ReadData(rIn);
//BFS01
//BFS01             rIn >> nTmp;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DTextureKindItem(nTmp));
//BFS01
//BFS01             rIn >> nTmp;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DTextureModeItem(nTmp));
//BFS01
//BFS01             rIn >> bTmp;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DNormalsInvertItem(bTmp));
//BFS01         }
//BFS01
//BFS01         // neu ab 534: (hat noch gefehlt)
//BFS01         if (aCompat.GetBytesLeft () >= sizeof (BOOL))
//BFS01         {
//BFS01             BOOL bTmp;
//BFS01             rIn >> bTmp;
//BFS01             GetProperties().SetObjectItemDirect(Svx3DTextureFilterItem(bTmp));
//BFS01         }
//BFS01     }
//BFS01     else
//BFS01     {
//BFS01         DBG_ERROR("AW: Kugel laden: nicht vorgesehener Fall");
//BFS01     }
//BFS01     pLoadedE3dPolyObjs->Clear();
//BFS01     delete pLoadedE3dPolyObjs;
//BFS01 }
//BFS01
//BFS01 // Geometrie neu erzeugen
//BFS01 ReCreateGeometry();
//BFS01}

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

void E3dSphereObj::operator=(const SdrObject& rObj)
{
    // erstmal alle Childs kopieren
    E3dCompoundObject::operator=(rObj);

    // weitere Parameter kopieren
    const E3dSphereObj& r3DObj = (const E3dSphereObj&) rObj;

    aCenter       = r3DObj.aCenter;
    aSize         = r3DObj.aSize;
}

/*************************************************************************
|*
|* Lokale Parameter setzen mit Geometrieneuerzeugung
|*
\************************************************************************/

void E3dSphereObj::SetCenter(const Vector3D& rNew)
{
    if(aCenter != rNew)
    {
        aCenter = rNew;
        bGeometryValid = FALSE;
    }
}

void E3dSphereObj::SetSize(const Vector3D& rNew)
{
    if(aSize != rNew)
    {
        aSize = rNew;
        bGeometryValid = FALSE;
    }
}

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

void E3dSphereObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulSphere3d);

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

void E3dSphereObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralSphere3d);
}

// eof
