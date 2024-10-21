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

Viewport3D::Viewport3D() :
    m_aVRP(0, 0, 5),
    m_aVPN(0, 0, 1),
    m_aVUV(0, 1, 1),
    m_aPRP(0, 0, 2),
    m_eProjection(ProjectionType::Perspective),
    m_aDeviceRect(Point(0,0), Size(-1,-1)),
    m_aViewPoint (0, 0, 5000),
    m_bTfValid(false)
{
    m_aViewWin.X = -1; m_aViewWin.Y = -1;
    m_aViewWin.W =  2; m_aViewWin.H = 2;
}

// Set ViewWindow (in View coordinates)

void Viewport3D::SetViewWindow(double fX, double fY, double fW, double fH)
{
    m_aViewWin.X = fX;
    m_aViewWin.Y = fY;
    if ( fW > 0 )   m_aViewWin.W = fW;
    else            m_aViewWin.W = 1.0;
    if ( fH > 0 )   m_aViewWin.H = fH;
    else            m_aViewWin.H = 1.0;
}

// Returns observer position (PRP) in world coordinates

const basegfx::B3DPoint& Viewport3D::GetViewPoint()
{
    // Calculate View transformations matrix
    if ( !m_bTfValid )
    {
        double fV, fXupVp, fYupVp;
        m_aViewPoint = m_aVRP + m_aVPN * m_aPRP.getZ();

        // Reset to Identity matrix
        m_aViewTf.identity();

        // shift in the origin
        m_aViewTf.translate(-m_aVRP.getX(), -m_aVRP.getY(), -m_aVRP.getZ());

        // fV = Length of the projection of aVPN on the yz plane:
        fV = m_aVPN.getYZLength();

        if ( fV != 0 )
        {
            basegfx::B3DHomMatrix aTemp;
            const double fSin(m_aVPN.getY() / fV);
            const double fCos(m_aVPN.getZ() / fV);
            aTemp.set(2, 2, fCos);
            aTemp.set(1, 1, fCos);
            aTemp.set(2, 1, fSin);
            aTemp.set(1, 2, -fSin);
            m_aViewTf *= aTemp;
        }

        {
            basegfx::B3DHomMatrix aTemp;
            const double fSin(-m_aVPN.getX());
            const double fCos(fV);
            aTemp.set(2, 2, fCos);
            aTemp.set(0, 0, fCos);
            aTemp.set(0, 2, fSin);
            aTemp.set(2, 0, -fSin);
            m_aViewTf *= aTemp;
        }

        // Convert X- and Y- coordinates of the view up vector to the
        // (preliminary) view coordinate system.
        fXupVp = m_aViewTf.get(0, 0) * m_aVUV.getX() + m_aViewTf.get(0, 1) * m_aVUV.getY() + m_aViewTf.get(0, 2) * m_aVUV.getZ();
        fYupVp = m_aViewTf.get(1, 0) * m_aVUV.getX() + m_aViewTf.get(1, 1) * m_aVUV.getY() + m_aViewTf.get(1, 2) * m_aVUV.getZ();
        fV = std::hypot(fXupVp, fYupVp);

        if ( fV != 0 )
        {
            basegfx::B3DHomMatrix aTemp;
            const double fSin(fXupVp / fV);
            const double fCos(fYupVp / fV);
            aTemp.set(1, 1, fCos);
            aTemp.set(0, 0, fCos);
            aTemp.set(1, 0, fSin);
            aTemp.set(0, 1, -fSin);
            m_aViewTf *= aTemp;
        }

        m_bTfValid = true;
    }
    return m_aViewPoint;
}

void Viewport3D::SetDeviceWindow(const tools::Rectangle& rRect)
{
    m_aDeviceRect = rRect;
}

// Set View Reference Point

void Viewport3D::SetVRP(const basegfx::B3DPoint& rNewVRP)
{
    m_aVRP = rNewVRP;
    m_bTfValid = false;
}

// Set View Plane Normal

void Viewport3D::SetVPN(const basegfx::B3DVector& rNewVPN)
{
    m_aVPN = rNewVPN;
    m_aVPN.normalize();
    m_bTfValid = false;
}

// Set View Up Vector

void Viewport3D::SetVUV(const basegfx::B3DVector& rNewVUV)
{
    m_aVUV = rNewVUV;
    m_bTfValid = false;
}

// Set Center Of Projection

void Viewport3D::SetPRP(const basegfx::B3DPoint& rNewPRP)
{
    m_aPRP = rNewPRP;
    m_aPRP.setX(0.0);
    m_aPRP.setY(0.0);
    m_bTfValid = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
