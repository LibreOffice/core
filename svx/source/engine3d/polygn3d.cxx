/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polygn3d.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:58:10 $
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

#ifndef _E3D_POLYGON3D_HXX
#include "polygn3d.hxx"
#endif

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif

TYPEINIT1(E3dPolygonObj, E3dCompoundObject);

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

E3dPolygonObj::E3dPolygonObj(
    E3dDefaultAttributes& rDefault,
    const PolyPolygon3D& rPolyPoly3D,
    BOOL bLinOnly)
:   E3dCompoundObject(rDefault),
    bLineOnly(bLinOnly)
{
    // Geometrie setzen
    SetPolyPolygon3D(rPolyPoly3D);

    // Default-Normals erzeugen
    CreateDefaultNormals();

    // Default-Texturkoordinaten erzeugen
    CreateDefaultTexture();

    // Geometrie erzeugen
    CreateGeometry();
}

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

E3dPolygonObj::E3dPolygonObj(
    E3dDefaultAttributes& rDefault,
    const PolyPolygon3D& rPolyPoly3D,
    const PolyPolygon3D& rPolyNormals3D,
    BOOL bLinOnly)
:   E3dCompoundObject(rDefault),
    bLineOnly(bLinOnly)
{
    // Geometrie und Normalen setzen
    SetPolyPolygon3D(rPolyPoly3D);
    SetPolyNormals3D(rPolyNormals3D);

    // Default-Texturkoordinaten erzeugen
    CreateDefaultTexture();

    // Geometrie erzeugen
    CreateGeometry();
}

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

E3dPolygonObj::E3dPolygonObj(
    E3dDefaultAttributes& rDefault,
    const PolyPolygon3D& rPolyPoly3D,
    const PolyPolygon3D& rPolyNormals3D,
    const PolyPolygon3D& rPolyTexture3D,
    BOOL bLinOnly)
:   E3dCompoundObject(rDefault),
    bLineOnly(bLinOnly)
{
    SetPolyPolygon3D(rPolyPoly3D);
    SetPolyNormals3D(rPolyNormals3D);
    SetPolyTexture3D(rPolyTexture3D);

    // Geometrie erzeugen
    CreateGeometry();
}

/*************************************************************************
|*
|* Linien-Konstruktor
|*
\************************************************************************/

E3dPolygonObj::E3dPolygonObj(
    E3dDefaultAttributes& rDefault,
    const Vector3D& rP1, const Vector3D& rP2,
    BOOL bLinOnly)
:   E3dCompoundObject(rDefault),
    aPolyPoly3D(1),
    bLineOnly(bLinOnly)
{
    Polygon3D aPoly3D(2);
    aPoly3D[0] = rP1;
    aPoly3D[1] = rP2;
    aPolyPoly3D.Insert(aPoly3D);

    // Geometrie erzeugen
    CreateGeometry();
}

/*************************************************************************
|*
|* Leer-Konstruktor
|*
\************************************************************************/

E3dPolygonObj::E3dPolygonObj()
:   E3dCompoundObject()
{
    // Keine Geometrie erzeugen
}

/*************************************************************************
|*
|* Default-Normalen erzeugen
|*
\************************************************************************/

void E3dPolygonObj::CreateDefaultNormals()
{
    PolyPolygon3D aPolyNormals(aPolyPoly3D.Count());

    // Komplettes PolyPolygon mit den Ebenennormalen anlegen
    for(UINT16 a=0;a<aPolyPoly3D.Count();a++)
    {
        // Quellpolygon finden
        const Polygon3D& rPolygon = aPolyPoly3D[a];

        // Neues Polygon fuer Normalen anlegen
        Polygon3D aNormals(rPolygon.GetPointCount());

        // Normale holen (und umdrehen)
        Vector3D aNormal = -rPolygon.GetNormal();

        // Neues Polygon fuellen
        for(UINT16 b=0;b<rPolygon.GetPointCount();b++)
            aNormals[b] = aNormal;

        // Neues Polygon in PolyPolygon einfuegen
        aPolyNormals.Insert(aNormals);
    }

    // Default-Normalen setzen
    SetPolyNormals3D(aPolyNormals);
}

/*************************************************************************
|*
|* Default-Texturkoordinaten erzeugen
|*
\************************************************************************/

void E3dPolygonObj::CreateDefaultTexture()
{
    PolyPolygon3D aPolyTexture(aPolyPoly3D.Count());

    // Komplettes PolyPolygon mit den Texturkoordinaten anlegen
    // Die Texturkoordinaten erstrecken sich ueber X,Y und Z
    // ueber die gesamten Extremwerte im Bereich 0.0 .. 1.0
    for(UINT16 a=0;a<aPolyPoly3D.Count();a++)
    {
        // Quellpolygon finden
        const Polygon3D& rPolygon = aPolyPoly3D[a];

        // Gesamtgroesse des Objektes feststellen
        Volume3D aVolume = rPolygon.GetPolySize();

        // Neues Polygon fuer Texturkoordinaten anlegen
        Polygon3D aTexture(rPolygon.GetPointCount());

        // Normale holen
        Vector3D aNormal = rPolygon.GetNormal();
        aNormal.Abs();

        // Entscheiden, welche Koordinaten als Source fuer das
        // Mapping benutzt werden sollen
        UINT16 nSourceMode = 0;

        // Groessten Freiheitsgrad ermitteln
        if(!(aNormal.X() > aNormal.Y() && aNormal.X() > aNormal.Z()))
        {
            if(aNormal.Y() > aNormal.Z())
            {
                // Y ist am groessten, benutze X,Z als mapping
                nSourceMode = 1;
            }
            else
            {
                // Z ist am groessten, benutze X,Y als mapping
                nSourceMode = 2;
            }
        }

        // Neues Polygon fuellen
        for(UINT16 b=0;b<rPolygon.GetPointCount();b++)
        {
            Vector3D& aTex = aTexture[b];
            const Vector3D& aPoly = rPolygon[b];

            switch(nSourceMode)
            {
                case 0: // Quelle ist Y,Z
                    if(aVolume.GetHeight())
                        aTex.X() = (aPoly.Y() - aVolume.MinVec().Y()) / aVolume.GetHeight();
                    else
                        aTex.X() = 0.0;

                    if(aVolume.GetDepth())
                        aTex.Y() = (aPoly.Z() - aVolume.MinVec().Z()) / aVolume.GetDepth();
                    else
                        aTex.Y() = 0.0;
                    break;

                case 1: // Quelle ist X,Z
                    if(aVolume.GetWidth())
                        aTex.X() = (aPoly.X() - aVolume.MinVec().X()) / aVolume.GetWidth();
                    else
                        aTex.X() = 0.0;

                    if(aVolume.GetDepth())
                        aTex.Y() = (aPoly.Z() - aVolume.MinVec().Z()) / aVolume.GetDepth();
                    else
                        aTex.Y() = 0.0;
                    break;

                case 2: // Quelle ist X,Y
                    if(aVolume.GetWidth())
                        aTex.X() = (aPoly.X() - aVolume.MinVec().X()) / aVolume.GetWidth();
                    else
                        aTex.X() = 0.0;

                    if(aVolume.GetHeight())
                        aTex.Y() = (aPoly.Y() - aVolume.MinVec().Y()) / aVolume.GetHeight();
                    else
                        aTex.Y() = 0.0;
                    break;
            }
        }

        // Neues Polygon in PolyPolygon einfuegen
        aPolyTexture.Insert(aTexture);
    }

    // Default-Texturkoordinaten setzen
    SetPolyTexture3D(aPolyTexture);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

E3dPolygonObj::~E3dPolygonObj()
{
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

UINT16 E3dPolygonObj::GetObjIdentifier() const
{
    return E3D_POLYGONOBJ_ID;
}

/*************************************************************************
|*
|* Wireframe erzeugen
|*
\************************************************************************/

//BFS01void E3dPolygonObj::CreateWireframe(Polygon3D& rWirePoly,
//BFS01 const Matrix4D* pTf, E3dDragDetail eDetail)
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
|* Polygon setzen
|*
\************************************************************************/

void E3dPolygonObj::SetPolyPolygon3D(const PolyPolygon3D& rNewPolyPoly3D)
{
    if ( aPolyPoly3D != rNewPolyPoly3D )
    {
        // Neues PolyPolygon; kopieren
        aPolyPoly3D = rNewPolyPoly3D;

        // Geometrie neu erzeugen
        bGeometryValid = FALSE;
    }
}

void E3dPolygonObj::SetPolyNormals3D(const PolyPolygon3D& rNewPolyNormals3D)
{
    if ( aPolyNormals3D != rNewPolyNormals3D )
    {
        // Neue Normalen; kopieren
        aPolyNormals3D = rNewPolyNormals3D;

        // Geometrie neu erzeugen
        bGeometryValid = FALSE;
    }
}

void E3dPolygonObj::SetPolyTexture3D(const PolyPolygon3D& rNewPolyTexture3D)
{
    if ( aPolyTexture3D != rNewPolyTexture3D )
    {
        // Neue Texturkoordinaten; kopieren
        aPolyTexture3D = rNewPolyTexture3D;

        // Geometrie neu erzeugen
        bGeometryValid = FALSE;
    }
}

/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus 6 Polygonen
|*
\************************************************************************/

SdrObject *E3dPolygonObj::DoConvertToPolyObj(BOOL /*bBezier*/) const
{
    return NULL;
}

/*************************************************************************
|*
|* Give out simple line geometry
|*
\************************************************************************/

::basegfx::B3DPolyPolygon E3dPolygonObj::Get3DLineGeometry() const
{
    ::basegfx::B3DPolyPolygon aRetval;

    for(sal_uInt16 a(0); a < aPolyPoly3D.Count(); a++)
    {
        ::basegfx::B3DPolygon aNew(aPolyPoly3D[a].getB3DPolygon());

        if(aNew.count() && aNew.isClosed())
        {
            aNew.append(aNew.getB3DPoint(0));
            aNew.setClosed(sal_False);
        }

        aRetval.append(aNew);
    }

    return aRetval;
}

/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/

void E3dPolygonObj::CreateGeometry()
{
    // Start der Geometrieerzeugung ankuendigen
    StartCreateGeometry();

    if(aPolyNormals3D.Count())
    {
        if(aPolyTexture3D.Count())
        {
            AddGeometry(aPolyPoly3D, aPolyNormals3D, aPolyTexture3D, TRUE, bLineOnly);
        }
        else
        {
            AddGeometry(aPolyPoly3D, aPolyNormals3D, TRUE, bLineOnly);
        }
    }
    else
    {
        AddGeometry(aPolyPoly3D, TRUE, bLineOnly);
    }

    // call parent
    E3dCompoundObject::CreateGeometry();
}

/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

//BFS01void E3dPolygonObj::WriteData(SvStream& rOut) const
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
//BFS01 E3dCompoundObject::WriteData(rOut);
//BFS01
//BFS01 E3dIOCompat aCompat(rOut, STREAM_WRITE, 1);
//BFS01 rOut << BOOL(bLineOnly);
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

//BFS01void E3dPolygonObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 // call parent
//BFS01 E3dCompoundObject::ReadData(rHead, rIn);
//BFS01
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 if(AreBytesLeft())
//BFS01 {
//BFS01     E3dIOCompat aIoCompat(rIn, STREAM_READ);
//BFS01     if(aIoCompat.GetVersion() >= 1)
//BFS01     {
//BFS01         rIn >> bLineOnly;
//BFS01     }
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

void E3dPolygonObj::operator=(const SdrObject& rObj)
{
    // erstmal alle Childs kopieren
    E3dCompoundObject::operator=(rObj);

    // weitere Parameter kopieren
    const E3dPolygonObj& r3DObj = (const E3dPolygonObj&)rObj;

    aPolyPoly3D      = r3DObj.aPolyPoly3D;
    aPolyNormals3D   = r3DObj.aPolyNormals3D;
    aPolyTexture3D   = r3DObj.aPolyTexture3D;
    bLineOnly        = r3DObj.bLineOnly;
}

/*************************************************************************
|*
|* LineOnly setzen
|*
\************************************************************************/

void E3dPolygonObj::SetLineOnly(BOOL bNew)
{
    if(bNew != bLineOnly)
    {
        bLineOnly = bNew;
        bGeometryValid = FALSE;
    }
}

// eof
