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

#include <svx/viewpt3d.hxx>
#include <basegfx/range/b3drange.hxx>
#include <o3tl/numeric.hxx>

Viewport3D::Viewport3D() :
    aVRP(0, 0, 5),
    aVPN(0, 0, 1),
    aVUV(0, 1, 1),
    aPRP(0, 0, 2),
    eProjection(ProjectionType::Perspective),
    aDeviceRect(Point(0,0), Size(-1,-1)),
    aViewPoint (0, 0, 5000),
    bTfValid(false)
{
    aViewWin.X = -1; aViewWin.Y = -1;
    aViewWin.W =  2; aViewWin.H = 2;
}

// Set ViewWindow (in View coordinates)

void Viewport3D::SetViewWindow(double fX, double fY, double fW, double fH)
{
    aViewWin.X = fX;
    aViewWin.Y = fY;
    if ( fW > 0 )   aViewWin.W = fW;
    else            aViewWin.W = 1.0;
    if ( fH > 0 )   aViewWin.H = fH;
    else            aViewWin.H = 1.0;
}

// Returns observer position (PRP) in world coordinates

const basegfx::B3DPoint& Viewport3D::GetViewPoint()
{
    // Calculate View transformations matrix
    if ( !bTfValid )
    {
        double fV, fXupVp, fYupVp;
        aViewPoint = aVRP + aVPN * aPRP.getZ();

        // Reset to Identity matrix
        aViewTf.identity();

        // shift in the origin
        aViewTf.translate(-aVRP.getX(), -aVRP.getY(), -aVRP.getZ());

        // fV = Length of the projection of aVPN on the yz plane:
        fV = aVPN.getYZLength();

        if ( fV != 0 )
        {
            basegfx::B3DHomMatrix aTemp;
            const double fSin(aVPN.getY() / fV);
            const double fCos(aVPN.getZ() / fV);
            aTemp.set(2, 2, fCos);
            aTemp.set(1, 1, fCos);
            aTemp.set(2, 1, fSin);
            aTemp.set(1, 2, -fSin);
            aViewTf *= aTemp;
        }

        {
            basegfx::B3DHomMatrix aTemp;
            const double fSin(-aVPN.getX());
            const double fCos(fV);
            aTemp.set(2, 2, fCos);
            aTemp.set(0, 0, fCos);
            aTemp.set(0, 2, fSin);
            aTemp.set(2, 0, -fSin);
            aViewTf *= aTemp;
        }

        // Convert X- and Y- coordinates of the view up vector to the
        // (preliminary) view coordinate system.
        fXupVp = aViewTf.get(0, 0) * aVUV.getX() + aViewTf.get(0, 1) * aVUV.getY() + aViewTf.get(0, 2) * aVUV.getZ();
        fYupVp = aViewTf.get(1, 0) * aVUV.getX() + aViewTf.get(1, 1) * aVUV.getY() + aViewTf.get(1, 2) * aVUV.getZ();
        fV = sqrt(fXupVp * fXupVp + fYupVp * fYupVp);

        if ( fV != 0 )
        {
            basegfx::B3DHomMatrix aTemp;
            const double fSin(fXupVp / fV);
            const double fCos(fYupVp / fV);
            aTemp.set(1, 1, fCos);
            aTemp.set(0, 0, fCos);
            aTemp.set(1, 0, fSin);
            aTemp.set(0, 1, -fSin);
            aViewTf *= aTemp;
        }

        bTfValid = true;
    }
    return aViewPoint;
}

void Viewport3D::SetDeviceWindow(const tools::Rectangle& rRect)
{
    aDeviceRect = rRect;
}

// Set View Reference Point

void Viewport3D::SetVRP(const basegfx::B3DPoint& rNewVRP)
{
    aVRP = rNewVRP;
    bTfValid = false;
}

// Set View Plane Normal

void Viewport3D::SetVPN(const basegfx::B3DVector& rNewVPN)
{
    aVPN = rNewVPN;
    aVPN.normalize();
    bTfValid = false;
}

// Set View Up Vector

void Viewport3D::SetVUV(const basegfx::B3DVector& rNewVUV)
{
    aVUV = rNewVUV;
    bTfValid = false;
}

// Set Center Of Projection

void Viewport3D::SetPRP(const basegfx::B3DPoint& rNewPRP)
{
    aPRP = rNewPRP;
    aPRP.setX(0.0);
    aPRP.setY(0.0);
    bTfValid = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
