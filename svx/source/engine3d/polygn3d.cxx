/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/polygn3d.hxx>
#include <svx/svdpage.hxx>
#include <svx/globl3d.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <sdr/contact/viewcontactofe3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>

// DrawContact section
std::unique_ptr<sdr::contact::ViewContact> E3dPolygonObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfE3dPolygon>(*this);
}

E3dPolygonObj::E3dPolygonObj(
    SdrModel& rSdrModel,
    const basegfx::B3DPolyPolygon& rPolyPoly3D)
:   E3dCompoundObject(rSdrModel),
    bLineOnly(true)
{
    // Set geometry
    SetPolyPolygon3D(rPolyPoly3D);

    // Create default normals
    CreateDefaultNormals();

    // Create default texture coordinates
    CreateDefaultTexture();
}

E3dPolygonObj::E3dPolygonObj(SdrModel& rSdrModel)
:   E3dCompoundObject(rSdrModel),
    bLineOnly(false)
{
    // Create no geometry
}

void E3dPolygonObj::CreateDefaultNormals()
{
    basegfx::B3DPolyPolygon aPolyNormals;

    // Create a complete tools::PolyPolygon with the plane normal
    for(sal_uInt32 a(0); a < aPolyPoly3D.count(); a++)
    {
        // Find source polygon
        const basegfx::B3DPolygon aPolygon(aPolyPoly3D.getB3DPolygon(a));

        // Creating a new polygon for the normal
        basegfx::B3DPolygon aNormals;

        // Get normal (and invert)
        basegfx::B3DVector aNormal(-aPolygon.getNormal());

        // Fill new polygon
        for(sal_uInt32 b(0); b < aPolygon.count(); b++)
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
    // Create a complete tools::PolyPolygon with the texture coordinates
    // The texture coordinates extend over X,Y and Z
    // on the whole extreme values in the range 0.0 .. 1.0
    for(sal_uInt32 a(0); a < aPolyPoly3D.count(); a++)
    {
        // Find source polygon
        const basegfx::B3DPolygon& aPolygon(aPolyPoly3D.getB3DPolygon(a));

        // Determine the total size of the object
        basegfx::B3DRange aVolume(basegfx::utils::getRange(aPolygon));

        // Get normal
        basegfx::B3DVector aNormal(aPolygon.getNormal());
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
        for(sal_uInt32 b(0); b < aPolygon.count(); b++)
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

SdrObject *E3dPolygonObj::DoConvertToPolyObj(bool /*bBezier*/, bool /*bAddText*/) const
{
    return nullptr;
}

E3dPolygonObj* E3dPolygonObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< E3dPolygonObj >(rTargetModel);
}

E3dPolygonObj& E3dPolygonObj::operator=(const E3dPolygonObj& rObj)
{
    if( this == &rObj )
        return *this;
    E3dCompoundObject::operator=(rObj);

    aPolyPoly3D = rObj.aPolyPoly3D;
    aPolyNormals3D = rObj.aPolyNormals3D;
    aPolyTexture2D = rObj.aPolyTexture2D;
    bLineOnly = rObj.bLineOnly;

    return *this;
}

void E3dPolygonObj::SetLineOnly(bool bNew)
{
    if(bNew != bLineOnly)
    {
        bLineOnly = bNew;
        ActionChanged();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
