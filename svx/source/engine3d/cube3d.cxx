/*************************************************************************
 *
 *  $RCSfile: cube3d.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:14 $
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

#ifndef _E3D_E3DIOCMPT_HXX
#include "e3dcmpt.hxx"
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

SdrObject *E3dCubeObj::DoConvertToPolyObj(BOOL bBezier) const
{
    return NULL;
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

void E3dCubeObj::WriteData(SvStream& rOut) const
{
    long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
    if(nVersion < 3800)
    {
        // Alte Geometrie erzeugen, um die E3dPolyObj's zu haben
        ((E3dCompoundObject*)this)->ReCreateGeometry(TRUE);
    }

    // call parent, schreibt die SubList (alte Geometrie) raus
    E3dCompoundObject::WriteData(rOut);

    E3dIOCompat aCompat(rOut, STREAM_WRITE, 1);
    rOut << aCubePos;
    rOut << aCubeSize;
    rOut << BOOL(bPosIsCenter);
    rOut << nSideFlags;

    if(nVersion < 3800)
    {
        // Geometrie neu erzeugen, um E3dPolyObj's wieder loszuwerden
        ((E3dCompoundObject*)this)->ReCreateGeometry();
    }
}

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

void E3dCubeObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    // call parent
    E3dCompoundObject::ReadData(rHead, rIn);

    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    BOOL bAllDone(FALSE);
    if(AreBytesLeft())
    {
        E3dIOCompat aIoCompat(rIn, STREAM_READ);
        if(aIoCompat.GetVersion() >= 1)
        {
            BOOL bTmp;
            rIn >> aCubePos;
            rIn >> aCubeSize;
            rIn >> bTmp; bPosIsCenter = bTmp;
            rIn >> nSideFlags;
            bAllDone = TRUE;
        }
    }

    if(!bAllDone)
    {
        // Geometrie aus erzeugten PolyObj's rekonstruieren
        const Volume3D& rVolume = GetLocalBoundVolume();
        aCubeSize = rVolume.MaxVec() - rVolume.MinVec();
        aCubePos = rVolume.MinVec();
        bPosIsCenter = FALSE;
        nSideFlags = CUBE_FULL;
    }

    // Geometrie neu erzeugen, mit oder ohne E3dPolyObj's
    ReCreateGeometry();
}

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
|* uebergebene Transformationsmatrix anwenden
|*
\************************************************************************/

void E3dCubeObj::ApplyTransform(const Matrix4D& rMatrix)
{
    // call parent
    E3dCompoundObject::ApplyTransform(rMatrix);

    // Anwenden auf lokale geometrie
    Vector3D aOuterEdge = aCubePos + aCubeSize;
    aCubePos *= rMatrix;
    aOuterEdge *= rMatrix;
    aCubeSize = aOuterEdge - aCubePos;
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
|* Attribute setzen
|*
\************************************************************************/

void E3dCubeObj::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
{
    // call parent
    E3dCompoundObject::NbcSetAttributes(rAttr, bReplaceAll);

    // special Attr for E3dCubeObj
}

/*************************************************************************
|*
|* Attribute lesen
|*
\************************************************************************/

void E3dCubeObj::TakeAttributes(SfxItemSet& rAttr, FASTBOOL bMerge, FASTBOOL bOnlyHardAttr) const
{
    // call parent
    E3dCompoundObject::TakeAttributes(rAttr, bMerge, bOnlyHardAttr);

    // special Attr for E3dCubeObj
}

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

void E3dCubeObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulCube3d);
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


