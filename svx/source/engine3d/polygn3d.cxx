/*************************************************************************
 *
 *  $RCSfile: polygn3d.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:15 $
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

#ifndef _E3D_POLYGON3D_HXX
#include "polygn3d.hxx"
#endif

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif


#ifndef _E3D_E3DIOCMPT_HXX
#include "e3dcmpt.hxx"
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

void E3dPolygonObj::CreateWireframe(Polygon3D& rWirePoly,
    const Matrix4D* pTf, E3dDragDetail eDetail)
{
    if ( eDetail == E3DDETAIL_ALLLINES ||
        (eDetail == E3DDETAIL_DEFAULT && GetDragDetail() == E3DDETAIL_ALLLINES) )
    {
        // Detailliert erzeugen
    }
    else
    {
        // call parent
        E3dObject::CreateWireframe(rWirePoly, pTf, eDetail);
    }
}

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

SdrObject *E3dPolygonObj::DoConvertToPolyObj(BOOL bBezier) const
{
    return NULL;
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

void E3dPolygonObj::WriteData(SvStream& rOut) const
{
    long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
    if(nVersion < 3800)
    {
        // Alte Geometrie erzeugen, um die E3dPolyObj's zu haben
        ((E3dCompoundObject*)this)->ReCreateGeometry(TRUE);
    }

    // call parent
    E3dCompoundObject::WriteData(rOut);

    E3dIOCompat aCompat(rOut, STREAM_WRITE, 1);
    rOut << BOOL(bLineOnly);

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

void E3dPolygonObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    // call parent
    E3dCompoundObject::ReadData(rHead, rIn);

    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    if(AreBytesLeft())
    {
        E3dIOCompat aIoCompat(rIn, STREAM_READ);
        if(aIoCompat.GetVersion() >= 1)
        {
            rIn >> bLineOnly;
        }
    }

    // Geometrie neu erzeugen, mit oder ohne E3dPolyObj's
    ReCreateGeometry();
}

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
|* uebergebene Transformationsmatrix anwenden
|*
\************************************************************************/

void E3dPolygonObj::ApplyTransform(const Matrix4D& rMatrix)
{
    // call parent
    E3dCompoundObject::ApplyTransform(rMatrix);

    // Anwenden auf lokale geometrie
    aPolyPoly3D.Transform(rMatrix);
}

/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

void E3dPolygonObj::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
{
    // call parent
    E3dCompoundObject::NbcSetAttributes(rAttr, bReplaceAll);

    // special Attr for E3dPolygonObj
}

/*************************************************************************
|*
|* Attribute lesen
|*
\************************************************************************/

void E3dPolygonObj::TakeAttributes(SfxItemSet& rAttr, FASTBOOL bMerge, FASTBOOL bOnlyHardAttr) const
{
    // call parent
    E3dCompoundObject::TakeAttributes(rAttr, bMerge, bOnlyHardAttr);

    // special Attr for E3dPolygonObj
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

