/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cube3d.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:54:53 $
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

#ifndef _E3D_CUBE3D_HXX
#include "cube3d.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _POLY3D_HXX
#include "poly3d.hxx"
#endif

#ifndef _SVX_VECTOR3D_HXX
#include <goodies/vector3d.hxx>
#endif

#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

TYPEINIT1(E3dCubeObj, E3dCompoundObject);

/*************************************************************************
|*
|* Konstruktor:                                                 |
|* 3D-Quader erzeugen; aPos: Zentrum oder links, unten, hinten  |__
|*                           (abhaengig von bPosIsCenter)      /
|*
\************************************************************************/

E3dCubeObj::E3dCubeObj(E3dDefaultAttributes& rDefault, Vector3D aPos, const Vector3D& r3DSize)
:   E3dCompoundObject(rDefault)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // uebergebene drueberbuegeln
    aCubePos = aPos;
    aCubeSize = r3DSize;

    // Geometrie erzeugen
    CreateGeometry();
}

E3dCubeObj::E3dCubeObj()
:   E3dCompoundObject()
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

void E3dCubeObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    aCubePos = rDefault.GetDefaultCubePos();
    aCubeSize = rDefault.GetDefaultCubeSize();
    nSideFlags = rDefault.GetDefaultCubeSideFlags();
    bPosIsCenter = rDefault.GetDefaultCubePosIsCenter();
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

UINT16 E3dCubeObj::GetObjIdentifier() const
{
    return E3D_CUBEOBJ_ID;
}

/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus 6 Polygonen
|*
\************************************************************************/

SdrObject *E3dCubeObj::DoConvertToPolyObj(BOOL /*bBezier*/) const
{
    return NULL;
}

/*************************************************************************
|*
|* Give out simple line geometry
|*
\************************************************************************/

::basegfx::B3DPolyPolygon E3dCubeObj::Get3DLineGeometry() const
{
    ::basegfx::B3DPolyPolygon aRetval;

    // add geometry describing polygons to rLinePolyPolygon
    ::basegfx::B3DPolygon aNewUpper;
    aNewUpper.append(::basegfx::B3DPoint(aCubePos.X(), aCubePos.Y(), aCubePos.Z() + aCubeSize.Z()));
    aNewUpper.append(::basegfx::B3DPoint(aCubePos.X(), aCubePos.Y() + aCubeSize.Y(), aCubePos.Z() + aCubeSize.Z()));
    aNewUpper.append(::basegfx::B3DPoint(aCubePos.X() + aCubeSize.X(), aCubePos.Y() + aCubeSize.Y(), aCubePos.Z() + aCubeSize.Z()));
    aNewUpper.append(::basegfx::B3DPoint(aCubePos.X() + aCubeSize.X(), aCubePos.Y(), aCubePos.Z() + aCubeSize.Z()));
    aNewUpper.append(aNewUpper.getB3DPoint(0));
    aRetval.append(aNewUpper);

    ::basegfx::B3DPolygon aNewLower;
    aNewLower.append(::basegfx::B3DPoint(aCubePos.X(), aCubePos.Y(), aCubePos.Z()));
    aNewLower.append(::basegfx::B3DPoint(aCubePos.X(), aCubePos.Y() + aCubeSize.Y(), aCubePos.Z()));
    aNewLower.append(::basegfx::B3DPoint(aCubePos.X() + aCubeSize.X(), aCubePos.Y() + aCubeSize.Y(), aCubePos.Z()));
    aNewLower.append(::basegfx::B3DPoint(aCubePos.X() + aCubeSize.X(), aCubePos.Y(), aCubePos.Z()));
    aNewLower.append(aNewLower.getB3DPoint(0));
    aRetval.append(aNewLower);

    ::basegfx::B3DPolygon aNewVertical;
    aNewVertical.append(::basegfx::B3DPoint(aCubePos.X(), aCubePos.Y(), aCubePos.Z()));
    aNewVertical.append(::basegfx::B3DPoint(aCubePos.X(), aCubePos.Y(), aCubePos.Z() + aCubeSize.Z()));
    aRetval.append(aNewVertical);

    aNewVertical.clear();
    aNewVertical.append(::basegfx::B3DPoint(aCubePos.X(), aCubePos.Y() + aCubeSize.Y(), aCubePos.Z()));
    aNewVertical.append(::basegfx::B3DPoint(aCubePos.X(), aCubePos.Y() + aCubeSize.Y(), aCubePos.Z() + aCubeSize.Z()));
    aRetval.append(aNewVertical);

    aNewVertical.clear();
    aNewVertical.append(::basegfx::B3DPoint(aCubePos.X() + aCubeSize.X(), aCubePos.Y() + aCubeSize.Y(), aCubePos.Z()));
    aNewVertical.append(::basegfx::B3DPoint(aCubePos.X() + aCubeSize.X(), aCubePos.Y() + aCubeSize.Y(), aCubePos.Z() + aCubeSize.Z()));
    aRetval.append(aNewVertical);

    aNewVertical.clear();
    aNewVertical.append(::basegfx::B3DPoint(aCubePos.X() + aCubeSize.X(), aCubePos.Y(), aCubePos.Z()));
    aNewVertical.append(::basegfx::B3DPoint(aCubePos.X() + aCubeSize.X(), aCubePos.Y(), aCubePos.Z() + aCubeSize.Z()));
    aRetval.append(aNewVertical);

    return aRetval;
}

/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/

void E3dCubeObj::CreateGeometry()
{
    Vector3D    aPos(aCubePos);
    Polygon3D   aRect3D(4);
    Polygon3D   aNormals3D(4);
    Polygon3D   aTexture3D(4);
    short       nV1, nV2;
    UINT16      nSideBit = 0x0001;

    // Start der Geometrieerzeugung ankuendigen
    StartCreateGeometry();

    if ( bPosIsCenter )
        aCubePos -= aCubeSize / 2;

    for (nV1 = 0; nV1 < 3; nV1++)
    {
        if ( nV1 == 0 ) nV2 = 2;
        else            nV2 = nV1 - 1;

        // Nur die Flaechen erzeugen, fuer die ein Bit
        if ( nSideFlags & nSideBit )
        {   // Flaechenpunkte entgegen dem Uhrzeigersinn generieren
            aRect3D[0] = aPos; aPos[nV1] += aCubeSize[nV1];
            aRect3D[1] = aPos; aPos[nV2] += aCubeSize[nV2];
            aRect3D[2] = aPos; aPos[nV1] -= aCubeSize[nV1];
            aRect3D[3] = aPos; aPos[nV2] -= aCubeSize[nV2];

            if(GetCreateNormals())
            {
                aNormals3D = aRect3D;
                aNormals3D[0].Normalize();
                aNormals3D[1].Normalize();
                aNormals3D[2].Normalize();
                aNormals3D[3].Normalize();

                if(GetCreateTexture())
                {
                    aTexture3D[1].X() = 0.0;
                    aTexture3D[1].Y() = 0.0;

                    aTexture3D[0].X() = 1.0;
                    aTexture3D[0].Y() = 0.0;

                    aTexture3D[2].X() = 0.0;
                    aTexture3D[2].Y() = 1.0;

                    aTexture3D[3].X() = 1.0;
                    aTexture3D[3].Y() = 1.0;

                    AddGeometry(aRect3D, aNormals3D, aTexture3D, FALSE);
                }
                else
                    AddGeometry(aRect3D, aNormals3D, FALSE);
            }
            else
            {
                AddGeometry(aRect3D, FALSE);
            }
        }
        nSideBit <<= 1;
    }
    aPos += aCubeSize;

    for (nV1 = 2; nV1 >= 0; nV1--)
    {
        if ( nV1 == 2 ) nV2 = 0;
        else            nV2 = nV1 + 1;

        if ( nSideFlags & nSideBit )
        {   // Flaechenpunkte entgegen dem Uhrzeigersinn generieren
            aRect3D[0] = aPos; aPos[nV1] -= aCubeSize[nV1];
            aRect3D[1] = aPos; aPos[nV2] -= aCubeSize[nV2];
            aRect3D[2] = aPos; aPos[nV1] += aCubeSize[nV1];
            aRect3D[3] = aPos; aPos[nV2] += aCubeSize[nV2];

            if(GetCreateTexture())
            {
                aTexture3D[1].X() = 0.0;
                aTexture3D[1].Y() = 0.0;

                aTexture3D[0].X() = 1.0;
                aTexture3D[0].Y() = 0.0;

                aTexture3D[2].X() = 0.0;
                aTexture3D[2].Y() = 1.0;

                aTexture3D[3].X() = 1.0;
                aTexture3D[3].Y() = 1.0;
            }

            if(GetCreateNormals())
            {
                aNormals3D = aRect3D;
                aNormals3D[0].Normalize();
                aNormals3D[1].Normalize();
                aNormals3D[2].Normalize();
                aNormals3D[3].Normalize();

                if(GetCreateTexture())
                    AddGeometry(aRect3D, aNormals3D, aTexture3D, FALSE);
                else
                    AddGeometry(aRect3D, aNormals3D, FALSE);
            }
            else
            {
                AddGeometry(aRect3D, FALSE);
            }
        }
        nSideBit <<= 1;
    }

    // call parent
    E3dCompoundObject::CreateGeometry();
}

/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

//BFS01void E3dCubeObj::WriteData(SvStream& rOut) const
//BFS01{
//BFS01#ifndef SVX_LIGHT
//BFS01 long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
//BFS01 if(nVersion < 3800)
//BFS01 {
//BFS01     // Alte Geometrie erzeugen, um die E3dPolyObj's zu haben
//BFS01     ((E3dCompoundObject*)this)->ReCreateGeometry(TRUE);
//BFS01 }
//BFS01
//BFS01 // call parent, schreibt die SubList (alte Geometrie) raus
//BFS01 E3dCompoundObject::WriteData(rOut);
//BFS01
//BFS01 E3dIOCompat aCompat(rOut, STREAM_WRITE, 1);
//BFS01 rOut << aCubePos;
//BFS01 rOut << aCubeSize;
//BFS01 rOut << BOOL(bPosIsCenter);
//BFS01 rOut << nSideFlags;
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

//BFS01void E3dCubeObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 // call parent
//BFS01 E3dCompoundObject::ReadData(rHead, rIn);
//BFS01
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 BOOL bAllDone(FALSE);
//BFS01 if(AreBytesLeft())
//BFS01 {
//BFS01     E3dIOCompat aIoCompat(rIn, STREAM_READ);
//BFS01     if(aIoCompat.GetVersion() >= 1)
//BFS01     {
//BFS01         BOOL bTmp;
//BFS01         rIn >> aCubePos;
//BFS01         rIn >> aCubeSize;
//BFS01         rIn >> bTmp; bPosIsCenter = bTmp;
//BFS01         rIn >> nSideFlags;
//BFS01         bAllDone = TRUE;
//BFS01     }
//BFS01 }
//BFS01
//BFS01 if(!bAllDone)
//BFS01 {
//BFS01     // Geometrie aus erzeugten PolyObj's rekonstruieren
//BFS01     const Volume3D& rVolume = GetLocalBoundVolume();
//BFS01     aCubeSize = rVolume.MaxVec() - rVolume.MinVec();
//BFS01     aCubePos = rVolume.MinVec();
//BFS01     bPosIsCenter = FALSE;
//BFS01     nSideFlags = CUBE_FULL;
//BFS01 }
//BFS01
//BFS01 // Geometrie neu erzeugen, mit oder ohne E3dPolyObj's
//BFS01 ReCreateGeometry();
//BFS01}

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

void E3dCubeObj::operator=(const SdrObject& rObj)
{
    // erstmal alle Childs kopieren
    E3dCompoundObject::operator=(rObj);

    // weitere Parameter kopieren
    const E3dCubeObj& r3DObj = (const E3dCubeObj&)rObj;

    aCubePos = r3DObj.aCubePos;
    aCubeSize = r3DObj.aCubeSize;
    bPosIsCenter = r3DObj.bPosIsCenter;
    nSideFlags = r3DObj.nSideFlags;
}

/*************************************************************************
|*
|* Lokale Parameter setzen mit Geometrieneuerzeugung
|*
\************************************************************************/

void E3dCubeObj::SetCubePos(const Vector3D& rNew)
{
    if(aCubePos != rNew)
    {
        aCubePos = rNew;
        bGeometryValid = FALSE;
    }
}

void E3dCubeObj::SetCubeSize(const Vector3D& rNew)
{
    if(aCubeSize != rNew)
    {
        aCubeSize = rNew;
        bGeometryValid = FALSE;
    }
}

void E3dCubeObj::SetPosIsCenter(BOOL bNew)
{
    if(bPosIsCenter != bNew)
    {
        bPosIsCenter = bNew;
        bGeometryValid = FALSE;
    }
}

void E3dCubeObj::SetSideFlags(UINT16 nNew)
{
    if(nSideFlags != nNew)
    {
        nSideFlags = nNew;
        bGeometryValid = FALSE;
    }
}

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

void E3dCubeObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulCube3d);

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

void E3dCubeObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralCube3d);
}

// eof
