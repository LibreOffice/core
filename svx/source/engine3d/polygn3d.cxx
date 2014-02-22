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
#include "svx/globl3d.hxx"
#include <basegfx/point/b3dpoint.hxx>
#include <svx/sdr/contact/viewcontactofe3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>

TYPEINIT1(E3dPolygonObj, E3dCompoundObject);

//////////////////////////////////////////////////////////////////////////////


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
    
    SetPolyPolygon3D(rPolyPoly3D);

    
    CreateDefaultNormals();

    
    CreateDefaultTexture();
}

E3dPolygonObj::E3dPolygonObj()
:   E3dCompoundObject(),
    bLineOnly(false) 
{
    
}

void E3dPolygonObj::CreateDefaultNormals()
{
    basegfx::B3DPolyPolygon aPolyNormals;

    
    for(sal_uInt32 a(0L); a < aPolyPoly3D.count(); a++)
    {
        
        const basegfx::B3DPolygon aPolygon(aPolyPoly3D.getB3DPolygon(a));

        
        basegfx::B3DPolygon aNormals;

        
        basegfx::B3DVector aNormal(-basegfx::tools::getNormal(aPolygon));

        
        for(sal_uInt32 b(0L); b < aPolygon.count(); b++)
        {
            aNormals.append(aNormal);
        }

        
        aPolyNormals.append(aNormals);
    }

    
    SetPolyNormals3D(aPolyNormals);
}

void E3dPolygonObj::CreateDefaultTexture()
{
    basegfx::B2DPolyPolygon aPolyTexture;
    
    
    
    for(sal_uInt32 a(0L); a < aPolyPoly3D.count(); a++)
    {
        
        const basegfx::B3DPolygon& aPolygon(aPolyPoly3D.getB3DPolygon(a));

        
        basegfx::B3DRange aVolume(basegfx::tools::getRange(aPolygon));

        
        basegfx::B3DVector aNormal(basegfx::tools::getNormal(aPolygon));
        aNormal.setX(fabs(aNormal.getX()));
        aNormal.setY(fabs(aNormal.getY()));
        aNormal.setZ(fabs(aNormal.getZ()));

        
        sal_uInt16 nSourceMode = 0;

        
        if(!(aNormal.getX() > aNormal.getY() && aNormal.getX() > aNormal.getZ()))
        {
            if(aNormal.getY() > aNormal.getZ())
            {
                
                nSourceMode = 1;
            }
            else
            {
                
                nSourceMode = 2;
            }
        }

        
        basegfx::B2DPolygon aTexture;

        
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

                case 1: 
                    if(aVolume.getWidth())
                        aTex.setX((aCandidate.getX() - aVolume.getMinX()) / aVolume.getWidth());
                    if(aVolume.getDepth())
                        aTex.setY((aCandidate.getZ() - aVolume.getMinZ()) / aVolume.getDepth());
                    break;

                case 2: 
                    if(aVolume.getWidth())
                        aTex.setX((aCandidate.getX() - aVolume.getMinX()) / aVolume.getWidth());
                    if(aVolume.getHeight())
                        aTex.setY((aCandidate.getY() - aVolume.getMinY()) / aVolume.getHeight());
                    break;
            }

            aTexture.append(aTex);
        }

        
        aPolyTexture.append(aTexture);
    }

    
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
        
        aPolyPoly3D = rNewPolyPoly3D;

        
        ActionChanged();
    }
}

void E3dPolygonObj::SetPolyNormals3D(const basegfx::B3DPolyPolygon& rNewPolyNormals3D)
{
    if ( aPolyNormals3D != rNewPolyNormals3D )
    {
        
        aPolyNormals3D = rNewPolyNormals3D;

        
        ActionChanged();
    }
}

void E3dPolygonObj::SetPolyTexture2D(const basegfx::B2DPolyPolygon& rNewPolyTexture2D)
{
    if ( aPolyTexture2D != rNewPolyTexture2D )
    {
        
        aPolyTexture2D = rNewPolyTexture2D;

        
        ActionChanged();
    }
}



SdrObject *E3dPolygonObj::DoConvertToPolyObj(sal_Bool /*bBezier*/, bool /*bAddText*/) const
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
