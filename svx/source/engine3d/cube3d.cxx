/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cube3d.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:18:22 $
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

E3dCubeObj::E3dCubeObj(E3dDefaultAttributes& rDefault, basegfx::B3DPoint aPos, const basegfx::B3DVector& r3DSize)
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

basegfx::B3DPolyPolygon E3dCubeObj::Get3DLineGeometry() const
{
    basegfx::B3DPolyPolygon aRetval;

    // add geometry describing polygons to rLinePolyPolygon
    basegfx::B3DPolygon aNewUpper;
    aNewUpper.append(basegfx::B3DPoint(aCubePos.getX(), aCubePos.getY(), aCubePos.getZ() + aCubeSize.getZ()));
    aNewUpper.append(basegfx::B3DPoint(aCubePos.getX(), aCubePos.getY() + aCubeSize.getY(), aCubePos.getZ() + aCubeSize.getZ()));
    aNewUpper.append(basegfx::B3DPoint(aCubePos.getX() + aCubeSize.getX(), aCubePos.getY() + aCubeSize.getY(), aCubePos.getZ() + aCubeSize.getZ()));
    aNewUpper.append(basegfx::B3DPoint(aCubePos.getX() + aCubeSize.getX(), aCubePos.getY(), aCubePos.getZ() + aCubeSize.getZ()));
    aNewUpper.append(aNewUpper.getB3DPoint(0));
    aRetval.append(aNewUpper);

    basegfx::B3DPolygon aNewLower;
    aNewLower.append(basegfx::B3DPoint(aCubePos.getX(), aCubePos.getY(), aCubePos.getZ()));
    aNewLower.append(basegfx::B3DPoint(aCubePos.getX(), aCubePos.getY() + aCubeSize.getY(), aCubePos.getZ()));
    aNewLower.append(basegfx::B3DPoint(aCubePos.getX() + aCubeSize.getX(), aCubePos.getY() + aCubeSize.getY(), aCubePos.getZ()));
    aNewLower.append(basegfx::B3DPoint(aCubePos.getX() + aCubeSize.getX(), aCubePos.getY(), aCubePos.getZ()));
    aNewLower.append(aNewLower.getB3DPoint(0));
    aRetval.append(aNewLower);

    basegfx::B3DPolygon aNewVertical;
    aNewVertical.append(basegfx::B3DPoint(aCubePos.getX(), aCubePos.getY(), aCubePos.getZ()));
    aNewVertical.append(basegfx::B3DPoint(aCubePos.getX(), aCubePos.getY(), aCubePos.getZ() + aCubeSize.getZ()));
    aRetval.append(aNewVertical);

    aNewVertical.clear();
    aNewVertical.append(basegfx::B3DPoint(aCubePos.getX(), aCubePos.getY() + aCubeSize.getY(), aCubePos.getZ()));
    aNewVertical.append(basegfx::B3DPoint(aCubePos.getX(), aCubePos.getY() + aCubeSize.getY(), aCubePos.getZ() + aCubeSize.getZ()));
    aRetval.append(aNewVertical);

    aNewVertical.clear();
    aNewVertical.append(basegfx::B3DPoint(aCubePos.getX() + aCubeSize.getX(), aCubePos.getY() + aCubeSize.getY(), aCubePos.getZ()));
    aNewVertical.append(basegfx::B3DPoint(aCubePos.getX() + aCubeSize.getX(), aCubePos.getY() + aCubeSize.getY(), aCubePos.getZ() + aCubeSize.getZ()));
    aRetval.append(aNewVertical);

    aNewVertical.clear();
    aNewVertical.append(basegfx::B3DPoint(aCubePos.getX() + aCubeSize.getX(), aCubePos.getY(), aCubePos.getZ()));
    aNewVertical.append(basegfx::B3DPoint(aCubePos.getX() + aCubeSize.getX(), aCubePos.getY(), aCubePos.getZ() + aCubeSize.getZ()));
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
    basegfx::B3DPoint   aPos(aCubePos);
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
            basegfx::B3DPolygon aRect3D;

            aRect3D.append(aPos); aPos[nV1] += aCubeSize[nV1];
            aRect3D.append(aPos); aPos[nV2] += aCubeSize[nV2];
            aRect3D.append(aPos); aPos[nV1] -= aCubeSize[nV1];
            aRect3D.append(aPos); aPos[nV2] -= aCubeSize[nV2];

            if(GetCreateNormals())
            {
                basegfx::B3DPolygon aNormals3D;
                basegfx::B3DVector aVecTmp;

                aVecTmp = aRect3D.getB3DPoint(0L); aVecTmp.normalize(); aNormals3D.append(basegfx::B3DPoint(aVecTmp));
                aVecTmp = aRect3D.getB3DPoint(1L); aVecTmp.normalize(); aNormals3D.append(basegfx::B3DPoint(aVecTmp));
                aVecTmp = aRect3D.getB3DPoint(2L); aVecTmp.normalize(); aNormals3D.append(basegfx::B3DPoint(aVecTmp));
                aVecTmp = aRect3D.getB3DPoint(3L); aVecTmp.normalize(); aNormals3D.append(basegfx::B3DPoint(aVecTmp));

                if(GetCreateTexture())
                {
                    basegfx::B2DPolygon aTexture2D;

                    aTexture2D.append(basegfx::B2DPoint(1.0, 0.0));
                    aTexture2D.append(basegfx::B2DPoint(0.0, 0.0));
                    aTexture2D.append(basegfx::B2DPoint(0.0, 1.0));
                    aTexture2D.append(basegfx::B2DPoint(1.0, 1.0));

                    AddGeometry(basegfx::B3DPolyPolygon(aRect3D), basegfx::B3DPolyPolygon(aNormals3D), basegfx::B2DPolyPolygon(aTexture2D), FALSE);
                }
                else
                {
                    AddGeometry(basegfx::B3DPolyPolygon(aRect3D), basegfx::B3DPolyPolygon(aNormals3D), FALSE);
                }
            }
            else
            {
                AddGeometry(basegfx::B3DPolyPolygon(aRect3D), FALSE);
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
            basegfx::B3DPolygon aRect3D;
            basegfx::B2DPolygon aTexture2D;

            aRect3D.append(aPos); aPos[nV1] -= aCubeSize[nV1];
            aRect3D.append(aPos); aPos[nV2] -= aCubeSize[nV2];
            aRect3D.append(aPos); aPos[nV1] += aCubeSize[nV1];
            aRect3D.append(aPos); aPos[nV2] += aCubeSize[nV2];

            if(GetCreateTexture())
            {
                aTexture2D.append(basegfx::B2DPoint(1.0, 0.0));
                aTexture2D.append(basegfx::B2DPoint(0.0, 0.0));
                aTexture2D.append(basegfx::B2DPoint(0.0, 1.0));
                aTexture2D.append(basegfx::B2DPoint(1.0, 1.0));
            }

            if(GetCreateNormals())
            {
                basegfx::B3DPolygon aNormals3D;
                basegfx::B3DVector aVecTmp;

                aVecTmp = aRect3D.getB3DPoint(0L); aVecTmp.normalize(); aNormals3D.append(basegfx::B3DPoint(aVecTmp));
                aVecTmp = aRect3D.getB3DPoint(1L); aVecTmp.normalize(); aNormals3D.append(basegfx::B3DPoint(aVecTmp));
                aVecTmp = aRect3D.getB3DPoint(2L); aVecTmp.normalize(); aNormals3D.append(basegfx::B3DPoint(aVecTmp));
                aVecTmp = aRect3D.getB3DPoint(3L); aVecTmp.normalize(); aNormals3D.append(basegfx::B3DPoint(aVecTmp));

                if(GetCreateTexture())
                {
                    AddGeometry(basegfx::B3DPolyPolygon(aRect3D), basegfx::B3DPolyPolygon(aNormals3D), basegfx::B2DPolyPolygon(aTexture2D), FALSE);
                }
                else
                {
                    AddGeometry(basegfx::B3DPolyPolygon(aRect3D), basegfx::B3DPolyPolygon(aNormals3D), FALSE);
                }
            }
            else
            {
                AddGeometry(basegfx::B3DPolyPolygon(aRect3D), FALSE);
            }
        }
        nSideBit <<= 1;
    }

    // call parent
    E3dCompoundObject::CreateGeometry();
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
|* Lokale Parameter setzen mit Geometrieneuerzeugung
|*
\************************************************************************/

void E3dCubeObj::SetCubePos(const basegfx::B3DPoint& rNew)
{
    if(aCubePos != rNew)
    {
        aCubePos = rNew;
        bGeometryValid = FALSE;
    }
}

void E3dCubeObj::SetCubeSize(const basegfx::B3DVector& rNew)
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
