/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sphere3d.cxx,v $
 * $Revision: 1.19 $
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
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include "globl3d.hxx"
#include <svx/sphere3d.hxx>

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svx/svx3ditems.hxx>
#include <svx/sdr/properties/e3dsphereproperties.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>

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

E3dSphereObj::E3dSphereObj(E3dDefaultAttributes& rDefault, const basegfx::B3DPoint& rCenter, const basegfx::B3DVector& r3DSize)
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

basegfx::B3DPolyPolygon E3dSphereObj::Get3DLineGeometry() const
{
    basegfx::B3DPolyPolygon aRetval;

    // add geometry describing polygons to rLinePolyPolygon
    const sal_uInt32 nCntHor((sal_uInt32)GetHorizontalSegments());
    const sal_uInt32 nCntVer((sal_uInt32)GetVerticalSegments());
    const sal_Bool bCreateHorizontal(sal_True);
    const sal_Bool bCreateVertical(sal_True);

    if(nCntHor && nCntVer && (bCreateHorizontal || bCreateVertical))
    {
        const double fHInc((double)DEG2RAD(360) / nCntHor);
        const double fVInc((double)DEG2RAD(180) / nCntVer);
        const basegfx::B3DVector aRadius(aSize / 2.0);
        const basegfx::B3DPoint aCenterPos(aCenter);
        double fHAng(0.0);
        basegfx::B3DPolygon aAllPoints;

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

                basegfx::B3DPoint aNewPos(
                    aCenterPos.getX() + (aRadius.getX() * fVCos) * fHCos,
                    aCenterPos.getY() + (aRadius.getY() * fVSin),
                    aCenterPos.getZ() + (aRadius.getZ() * fVCos) * fHSin);

                aAllPoints.append(aNewPos);
            }
        }

        // create horizontal lines
        if(bCreateHorizontal)
        {
            for(sal_uInt32 a(1L); a < nCntVer; a++)
            {
                basegfx::B3DPolygon aNewHor;
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
            const basegfx::B3DPoint aTopPos(aCenterPos.getX(), aCenterPos.getY() + aRadius.getY(), aCenterPos.getZ());
            const basegfx::B3DPoint aBottomPos(aCenterPos.getX(), aCenterPos.getY() - aRadius.getY(), aCenterPos.getZ());

            for(sal_uInt32 a(0L); a < nCntHor; a++)
            {
                basegfx::B3DPolygon aNewVer;
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
    if ( GetHorizontalSegments() <  3L  )
    {
        GetProperties().SetObjectItemDirect(Svx3DHorizontalSegmentsItem(3L));
    }

    if ( GetHorizontalSegments() > 100L )
    {
        GetProperties().SetObjectItemDirect(Svx3DHorizontalSegmentsItem(100L));
    }

    if ( GetVerticalSegments() <  2L  )
    {
        GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(2L));
    }

    if ( GetVerticalSegments() > 100L )
    {
        GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(100L));
    }

    // Start der Geometrieerzeugung ankuendigen
    StartCreateGeometry();

    basegfx::B3DVector aRadius(aSize / 2.0);
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
            fVAng -= fVInc;
            fVSin1 = fVSin2;
            fVSin2 = sin(fVAng);
            fVCos1 = fVCos2;
            fVCos2 = cos(fVAng);

            basegfx::B3DPoint aPos1(aCenter);
            fRx = aRadius.getX() * fVCos1;
            fRz = aRadius.getZ() * fVCos1;
            aPos1.setX(aPos1.getX() + (fRx * fHCos1));
            aPos1.setY(aPos1.getY() + (aRadius.getY() * fVSin1));
            aPos1.setZ(aPos1.getZ() + (fRz * fHSin1));

            basegfx::B3DPoint aPos2(aPos1);
            aPos2.setX(aCenter.getX() + fRx * fHCos2);
            aPos2.setZ(aCenter.getZ() + fRz * fHSin2);

            basegfx::B3DPoint aPos0(aCenter);
            fRx = aRadius.getX() * fVCos2;
            fRz = aRadius.getZ() * fVCos2;
            aPos0.setX(aPos0.getX() + (fRx * fHCos1));
            aPos0.setY(aPos0.getY() + (aRadius.getY() * fVSin2));
            aPos0.setZ(aPos0.getZ() + (fRz * fHSin1));

            basegfx::B3DPoint aPos3(aPos0);
            aPos3.setX(aCenter.getX() + (fRx * fHCos2));
            aPos3.setZ(aCenter.getZ() + (fRz * fHSin2));

            basegfx::B3DPolygon aRect3D;
            basegfx::B2DPolygon aTexture2D;

            aRect3D.append(aPos0);
            aRect3D.append(aPos1);
            aRect3D.append(aPos2);
            aRect3D.append(aPos3);

            if(GetCreateTexture())
            {
                basegfx::B2DPoint aTex1;
                aTex1.setX((double)(nUpperBound - (nH + 1)) / (double)nUpperBound);
                aTex1.setY((double)nV / (double)nUpperVert);

                basegfx::B2DPoint aTex2;
                aTex2.setX((double)((nUpperBound - (nH + 1)) - 1) / (double)nUpperBound);
                aTex2.setY(aTex1.getY());

                basegfx::B2DPoint aTex3;
                aTex3.setX(aTex2.getX());
                aTex3.setY((double)(nV+1) / (double)nUpperVert);

                basegfx::B2DPoint aTex0;
                aTex0.setX(aTex1.getX());
                aTex0.setY(aTex3.getY());

                aTexture2D.append(aTex0);
                aTexture2D.append(aTex1);
                aTexture2D.append(aTex2);
                aTexture2D.append(aTex3);
            }

            if(GetCreateNormals())
            {
                basegfx::B3DPolygon aNormal3D;
                basegfx::B3DVector aNorZwi;

                aNorZwi = aRect3D.getB3DPoint(0L); aNorZwi.normalize(); aNormal3D.append(aNorZwi);
                aNorZwi = aRect3D.getB3DPoint(1L); aNorZwi.normalize(); aNormal3D.append(aNorZwi);
                aNorZwi = aRect3D.getB3DPoint(2L); aNorZwi.normalize(); aNormal3D.append(aNorZwi);
                aNorZwi = aRect3D.getB3DPoint(3L); aNorZwi.normalize(); aNormal3D.append(aNorZwi);

                if(GetCreateTexture())
                {
                    AddGeometry(basegfx::B3DPolyPolygon(aRect3D), basegfx::B3DPolyPolygon(aNormal3D), basegfx::B2DPolyPolygon(aTexture2D), FALSE);
                }
                else
                {
                    AddGeometry(basegfx::B3DPolyPolygon(aRect3D), basegfx::B3DPolyPolygon(aNormal3D), FALSE);
                }
            }
            else
            {
                AddGeometry(basegfx::B3DPolyPolygon(aRect3D), FALSE);
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

void E3dSphereObj::ReSegment(sal_uInt32 nHSegs, sal_uInt32 nVSegs)
{
    if((nHSegs != GetHorizontalSegments() || nVSegs != GetVerticalSegments()) && (nHSegs != 0 || nVSegs != 0))
    {
        GetProperties().SetObjectItemDirect(Svx3DHorizontalSegmentsItem(nHSegs));
        GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nVSegs));

        bGeometryValid = FALSE;
    }
}

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

void E3dSphereObj::SetCenter(const basegfx::B3DPoint& rNew)
{
    if(aCenter != rNew)
    {
        aCenter = rNew;
        bGeometryValid = FALSE;
    }
}

void E3dSphereObj::SetSize(const basegfx::B3DVector& rNew)
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
