/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/polygn3d.hxx>
#include <svx/svdpage.hxx>
#include "svx/globl3d.hxx"
#include <basegfx/point/b3dpoint.hxx>
#include <svx/sdr/contact/viewcontactofe3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>

TYPEINIT1(E3dPolygonObj, E3dCompoundObject);

//////////////////////////////////////////////////////////////////////////////
// #110094# DrawContact section

sdr::contact::ViewContact* E3dPolygonObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfE3dPolygon(*this);
}

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

E3dPolygonObj::E3dPolygonObj(
    E3dDefaultAttributes& rDefault,
    const basegfx::B3DPolyPolygon& rPolyPoly3D,
    sal_Bool bLinOnly)
:   E3dCompoundObject(rDefault),
    bLineOnly(bLinOnly)
{
    // Geometrie setzen
    SetPolyPolygon3D(rPolyPoly3D);

    // Default-Normals erzeugen
    CreateDefaultNormals();

    // Default-Texturkoordinaten erzeugen
    CreateDefaultTexture();
}

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

E3dPolygonObj::E3dPolygonObj(
    E3dDefaultAttributes& rDefault,
    const basegfx::B3DPolyPolygon& rPolyPoly3D,
    const basegfx::B3DPolyPolygon& rPolyNormals3D,
    sal_Bool bLinOnly)
:   E3dCompoundObject(rDefault),
    bLineOnly(bLinOnly)
{
    // Geometrie und Normalen setzen
    SetPolyPolygon3D(rPolyPoly3D);
    SetPolyNormals3D(rPolyNormals3D);

    // Default-Texturkoordinaten erzeugen
    CreateDefaultTexture();
}

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

E3dPolygonObj::E3dPolygonObj(
    E3dDefaultAttributes& rDefault,
    const basegfx::B3DPolyPolygon& rPolyPoly3D,
    const basegfx::B3DPolyPolygon& rPolyNormals3D,
    const basegfx::B2DPolyPolygon& rPolyTexture2D,
    sal_Bool bLinOnly)
:   E3dCompoundObject(rDefault),
    bLineOnly(bLinOnly)
{
    SetPolyPolygon3D(rPolyPoly3D);
    SetPolyNormals3D(rPolyNormals3D);
    SetPolyTexture2D(rPolyTexture2D);
}

/*************************************************************************
|*
|* Leer-Konstruktor
|*
\************************************************************************/

E3dPolygonObj::E3dPolygonObj()
:   E3dCompoundObject(),
    bLineOnly(false) // added missing initialisation
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
    basegfx::B3DPolyPolygon aPolyNormals;

    // Komplettes PolyPolygon mit den Ebenennormalen anlegen
    for(sal_uInt32 a(0L); a < aPolyPoly3D.count(); a++)
    {
        // Quellpolygon finden
        const basegfx::B3DPolygon aPolygon(aPolyPoly3D.getB3DPolygon(a));

        // Neues Polygon fuer Normalen anlegen
        basegfx::B3DPolygon aNormals;

        // Normale holen (und umdrehen)
        basegfx::B3DVector aNormal(-basegfx::tools::getNormal(aPolygon));

        // Neues Polygon fuellen
        for(sal_uInt32 b(0L); b < aPolygon.count(); b++)
        {
            aNormals.append(aNormal);
        }

        // Neues Polygon in PolyPolygon einfuegen
        aPolyNormals.append(aNormals);
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
    basegfx::B2DPolyPolygon aPolyTexture;

    // Komplettes PolyPolygon mit den Texturkoordinaten anlegen
    // Die Texturkoordinaten erstrecken sich ueber X,Y und Z
    // ueber die gesamten Extremwerte im Bereich 0.0 .. 1.0
    for(sal_uInt32 a(0L); a < aPolyPoly3D.count(); a++)
    {
        // Quellpolygon finden
        const basegfx::B3DPolygon& aPolygon(aPolyPoly3D.getB3DPolygon(a));

        // Gesamtgroesse des Objektes feststellen
        basegfx::B3DRange aVolume(basegfx::tools::getRange(aPolygon));

        // Normale holen
        basegfx::B3DVector aNormal(basegfx::tools::getNormal(aPolygon));
        aNormal.setX(fabs(aNormal.getX()));
        aNormal.setY(fabs(aNormal.getY()));
        aNormal.setZ(fabs(aNormal.getZ()));

        // Entscheiden, welche Koordinaten als Source fuer das
        // Mapping benutzt werden sollen
        sal_uInt16 nSourceMode = 0;

        // Groessten Freiheitsgrad ermitteln
        if(!(aNormal.getX() > aNormal.getY() && aNormal.getX() > aNormal.getZ()))
        {
            if(aNormal.getY() > aNormal.getZ())
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

        // Neues Polygon fuer Texturkoordinaten anlegen
        basegfx::B2DPolygon aTexture;

        // Neues Polygon fuellen
        for(sal_uInt32 b(0L); b < aPolygon.count(); b++)
        {
            basegfx::B2DPoint aTex;
            const basegfx::B3DPoint aCandidate(aPolygon.getB3DPoint(b));

            switch(nSourceMode)
            {
                case 0: // Quelle ist Y,Z
                    if(aVolume.getHeight())
                        aTex.setX((aCandidate.getY() - aVolume.getMinY()) / aVolume.getHeight());
                    if(aVolume.getDepth())
                        aTex.setY((aCandidate.getZ() - aVolume.getMinZ()) / aVolume.getDepth());
                    break;

                case 1: // Quelle ist X,Z
                    if(aVolume.getWidth())
                        aTex.setX((aCandidate.getX() - aVolume.getMinX()) / aVolume.getWidth());
                    if(aVolume.getDepth())
                        aTex.setY((aCandidate.getZ() - aVolume.getMinZ()) / aVolume.getDepth());
                    break;

                case 2: // Quelle ist X,Y
                    if(aVolume.getWidth())
                        aTex.setX((aCandidate.getX() - aVolume.getMinX()) / aVolume.getWidth());
                    if(aVolume.getHeight())
                        aTex.setY((aCandidate.getY() - aVolume.getMinY()) / aVolume.getHeight());
                    break;
            }

            aTexture.append(aTex);
        }

        // Neues Polygon in PolyPolygon einfuegen
        aPolyTexture.append(aTexture);
    }

    // Default-Texturkoordinaten setzen
    SetPolyTexture2D(aPolyTexture);
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

sal_uInt16 E3dPolygonObj::GetObjIdentifier() const
{
    return E3D_POLYGONOBJ_ID;
}

/*************************************************************************
|*
|* Polygon setzen
|*
\************************************************************************/

void E3dPolygonObj::SetPolyPolygon3D(const basegfx::B3DPolyPolygon& rNewPolyPoly3D)
{
    if ( aPolyPoly3D != rNewPolyPoly3D )
    {
        // Neues PolyPolygon; kopieren
        aPolyPoly3D = rNewPolyPoly3D;

        // Geometrie neu erzeugen
        ActionChanged();
    }
}

void E3dPolygonObj::SetPolyNormals3D(const basegfx::B3DPolyPolygon& rNewPolyNormals3D)
{
    if ( aPolyNormals3D != rNewPolyNormals3D )
    {
        // Neue Normalen; kopieren
        aPolyNormals3D = rNewPolyNormals3D;

        // Geometrie neu erzeugen
        ActionChanged();
    }
}

void E3dPolygonObj::SetPolyTexture2D(const basegfx::B2DPolyPolygon& rNewPolyTexture2D)
{
    if ( aPolyTexture2D != rNewPolyTexture2D )
    {
        // Neue Texturkoordinaten; kopieren
        aPolyTexture2D = rNewPolyTexture2D;

        // Geometrie neu erzeugen
        ActionChanged();
    }
}

/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus 6 Polygonen
|*
\************************************************************************/

SdrObject *E3dPolygonObj::DoConvertToPolyObj(sal_Bool /*bBezier*/, bool /*bAddText*/) const
{
    return NULL;
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
    aPolyTexture2D   = r3DObj.aPolyTexture2D;
    bLineOnly        = r3DObj.bLineOnly;
}

/*************************************************************************
|*
|* LineOnly setzen
|*
\************************************************************************/

void E3dPolygonObj::SetLineOnly(sal_Bool bNew)
{
    if(bNew != bLineOnly)
    {
        bLineOnly = bNew;
        ActionChanged();
    }
}

// eof
