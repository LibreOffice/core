/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <svx/polygn3d.hxx>
#include <svx/svdpage.hxx>
#include "svx/globl3d.hxx"
#include <basegfx/point/b3dpoint.hxx>
#include <svx/sdr/contact/viewcontactofe3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>

TYPEINIT1(E3dPolygonObj, E3dCompoundObject);

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* E3dPolygonObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfE3dPolygon(*this);
}

E3dPolygonObj::E3dPolygonObj(
    E3dDefaultAttributes& rDefault,
    const basegfx::B3DPolyPolygon& rPolyPoly3D,
    sal_Bool bLinOnly)
:   E3dCompoundObject(rDefault),
    bLineOnly(bLinOnly)
{
    // Set geometry
    SetPolyPolygon3D(rPolyPoly3D);

    // Create default normals
    CreateDefaultNormals();

    // Create default texture coordinates
    CreateDefaultTexture();
}

E3dPolygonObj::E3dPolygonObj()
:   E3dCompoundObject(),
    bLineOnly(false) // added missing initialisation
{
    // Create no geometry
}

void E3dPolygonObj::CreateDefaultNormals()
{
    basegfx::B3DPolyPolygon aPolyNormals;

    // Create a complete PolyPolygon with the plane normal
    for(sal_uInt32 a(0L); a < aPolyPoly3D.count(); a++)
    {
        // Find source polygon
        const basegfx::B3DPolygon aPolygon(aPolyPoly3D.getB3DPolygon(a));

        // Creating a new polygon for the normal
        basegfx::B3DPolygon aNormals;

        // Get normal (and invert)
        basegfx::B3DVector aNormal(-basegfx::tools::getNormal(aPolygon));

        // Fill new polygon
        for(sal_uInt32 b(0L); b < aPolygon.count(); b++)
        {
            aNormals.append(aNormal);
        }

        // Insert new polygon into the PolyPolygon
        aPolyNormals.append(aNormals);
    }

    // Set default normal
    SetPolyNormals3D(aPolyNormals);
}

void E3dPolygonObj::CreateDefaultTexture()
{
    basegfx::B2DPolyPolygon aPolyTexture;
    // Create a complete PolyPolygon with the texture coordinates
    // The texture coordinates extend over X,Y and Z
    // on the whole extreme values in the range 0.0 .. 1.0
    for(sal_uInt32 a(0L); a < aPolyPoly3D.count(); a++)
    {
        // Find source polygon
        const basegfx::B3DPolygon& aPolygon(aPolyPoly3D.getB3DPolygon(a));

        // Determine the total size of the object
        basegfx::B3DRange aVolume(basegfx::tools::getRange(aPolygon));

        // Get normal
        basegfx::B3DVector aNormal(basegfx::tools::getNormal(aPolygon));
        aNormal.setX(fabs(aNormal.getX()));
        aNormal.setY(fabs(aNormal.getY()));
        aNormal.setZ(fabs(aNormal.getZ()));

        // Decide which coordinates should be used as a source for the mapping
        sal_uInt16 nSourceMode = 0;

        // Determine the greatest degree of freedom
        if(!(aNormal.getX() > aNormal.getY() && aNormal.getX() > aNormal.getZ()))
        {
            if(aNormal.getY() > aNormal.getZ())
            {
                // Y is the largest, use X,Z as mapping
                nSourceMode = 1;
            }
            else
            {
                // Z is the largest, use X,Y as mapping
                nSourceMode = 2;
            }
        }

        // Create new polygon for texture coordinates
        basegfx::B2DPolygon aTexture;

        // Fill new polygon
        for(sal_uInt32 b(0L); b < aPolygon.count(); b++)
        {
            basegfx::B2DPoint aTex;
            const basegfx::B3DPoint aCandidate(aPolygon.getB3DPoint(b));

            switch(nSourceMode)
            {
                case 0: //Source is Y,Z
                    if(aVolume.getHeight())
                        aTex.setX((aCandidate.getY() - aVolume.getMinY()) / aVolume.getHeight());
                    if(aVolume.getDepth())
                        aTex.setY((aCandidate.getZ() - aVolume.getMinZ()) / aVolume.getDepth());
                    break;

                case 1: // Source is X,Z
                    if(aVolume.getWidth())
                        aTex.setX((aCandidate.getX() - aVolume.getMinX()) / aVolume.getWidth());
                    if(aVolume.getDepth())
                        aTex.setY((aCandidate.getZ() - aVolume.getMinZ()) / aVolume.getDepth());
                    break;

                case 2: // Source is X,Y
                    if(aVolume.getWidth())
                        aTex.setX((aCandidate.getX() - aVolume.getMinX()) / aVolume.getWidth());
                    if(aVolume.getHeight())
                        aTex.setY((aCandidate.getY() - aVolume.getMinY()) / aVolume.getHeight());
                    break;
            }

            aTexture.append(aTex);
        }

        // Insert new polygon into the PolyPolygon
        aPolyTexture.append(aTexture);
    }

    // Set default Texture coordinates
    SetPolyTexture2D(aPolyTexture);
}

E3dPolygonObj::~E3dPolygonObj()
{
}

sal_uInt16 E3dPolygonObj::GetObjIdentifier() const
{
    return E3D_POLYGONOBJ_ID;
}

void E3dPolygonObj::SetPolyPolygon3D(const basegfx::B3DPolyPolygon& rNewPolyPoly3D)
{
    if ( aPolyPoly3D != rNewPolyPoly3D )
    {
        // New PolyPolygon; copying
        aPolyPoly3D = rNewPolyPoly3D;

        // Create new geometry
        ActionChanged();
    }
}

void E3dPolygonObj::SetPolyNormals3D(const basegfx::B3DPolyPolygon& rNewPolyNormals3D)
{
    if ( aPolyNormals3D != rNewPolyNormals3D )
    {
        // New PolyPolygon; copying
        aPolyNormals3D = rNewPolyNormals3D;

        // Create new geometry
        ActionChanged();
    }
}

void E3dPolygonObj::SetPolyTexture2D(const basegfx::B2DPolyPolygon& rNewPolyTexture2D)
{
    if ( aPolyTexture2D != rNewPolyTexture2D )
    {
        // New PolyPolygon; copying
        aPolyTexture2D = rNewPolyTexture2D;

        // Create new geometry
        ActionChanged();
    }
}

// Convert the object into a group object consisting of 6 polygons

SdrObject *E3dPolygonObj::DoConvertToPolyObj(sal_Bool /*bBezier*/) const
{
    return NULL;
}

E3dPolygonObj* E3dPolygonObj::Clone() const
{
    return CloneHelper< E3dPolygonObj >();
}

void E3dPolygonObj::SetLineOnly(sal_Bool bNew)
{
    if(bNew != bLineOnly)
    {
        bLineOnly = bNew;
        ActionChanged();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
