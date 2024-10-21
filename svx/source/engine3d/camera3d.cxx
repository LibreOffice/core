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

#include <svx/camera3d.hxx>

Camera3D::Camera3D(const basegfx::B3DPoint& rPos, const basegfx::B3DPoint& rLookAt,
                   double fFocalLen)
    : m_fBankAngle(0)
    , m_bAutoAdjustProjection(true)
{
    SetPosition(rPos);
    SetLookAt(rLookAt);
    SetFocalLength(fFocalLen);
}

Camera3D::Camera3D()
    : m_fFocalLength(35.0)
    , m_fBankAngle(0.0)
    , m_bAutoAdjustProjection(false)
{
}

// Set ViewWindow and adjust PRP

void Camera3D::SetViewWindow(double fX, double fY, double fW, double fH)
{
    Viewport3D::SetViewWindow(fX, fY, fW, fH);
    if (m_bAutoAdjustProjection)
        SetFocalLength(m_fFocalLength);
}

void Camera3D::SetPosition(const basegfx::B3DPoint& rNewPos)
{
    if (rNewPos != m_aPosition)
    {
        m_aPosition = rNewPos;
        SetVRP(m_aPosition);
        SetVPN(m_aPosition - m_aLookAt);
        SetBankAngle(m_fBankAngle);
    }
}

void Camera3D::SetLookAt(const basegfx::B3DPoint& rNewLookAt)
{
    if (rNewLookAt != m_aLookAt)
    {
        m_aLookAt = rNewLookAt;
        SetVPN(m_aPosition - m_aLookAt);
        SetBankAngle(m_fBankAngle);
    }
}

void Camera3D::SetPosAndLookAt(const basegfx::B3DPoint& rNewPos,
                               const basegfx::B3DPoint& rNewLookAt)
{
    if (rNewPos != m_aPosition || rNewLookAt != m_aLookAt)
    {
        m_aPosition = rNewPos;
        m_aLookAt = rNewLookAt;

        SetVRP(m_aPosition);
        SetVPN(m_aPosition - m_aLookAt);
        SetBankAngle(m_fBankAngle);
    }
}

void Camera3D::SetBankAngle(double fAngle)
{
    basegfx::B3DVector aDiff(m_aPosition - m_aLookAt);
    basegfx::B3DVector aPrj(aDiff);
    m_fBankAngle = fAngle;

    if (aDiff.getY() == 0)
    {
        aPrj.setY(-1.0);
    }
    else
    { // aPrj = Projection from aDiff on the XZ-plane
        aPrj.setY(0.0);

        if (aDiff.getY() < 0.0)
        {
            aPrj = -aPrj;
        }
    }

    // Calculate from aDiff to upwards pointing View-Up-Vector
    // duplicated line is intentional!
    aPrj = aPrj.getPerpendicular(aDiff);
    aPrj = aPrj.getPerpendicular(aDiff);
    aDiff.normalize();

    // Rotate on Z axis, to rotate the BankAngle and back
    basegfx::B3DHomMatrix aTf;
    const double fV(std::hypot(aDiff.getY(), aDiff.getZ()));

    if (fV != 0.0)
    {
        basegfx::B3DHomMatrix aTemp;
        const double fSin(aDiff.getY() / fV);
        const double fCos(aDiff.getZ() / fV);

        aTemp.set(1, 1, fCos);
        aTemp.set(2, 2, fCos);
        aTemp.set(2, 1, fSin);
        aTemp.set(1, 2, -fSin);

        aTf *= aTemp;
    }

    {
        basegfx::B3DHomMatrix aTemp;
        const double fSin(-aDiff.getX());
        const double fCos(fV);

        aTemp.set(0, 0, fCos);
        aTemp.set(2, 2, fCos);
        aTemp.set(0, 2, fSin);
        aTemp.set(2, 0, -fSin);

        aTf *= aTemp;
    }

    aTf.rotate(0.0, 0.0, m_fBankAngle);

    {
        basegfx::B3DHomMatrix aTemp;
        const double fSin(aDiff.getX());
        const double fCos(fV);

        aTemp.set(0, 0, fCos);
        aTemp.set(2, 2, fCos);
        aTemp.set(0, 2, fSin);
        aTemp.set(2, 0, -fSin);

        aTf *= aTemp;
    }

    if (fV != 0.0)
    {
        basegfx::B3DHomMatrix aTemp;
        const double fSin(-aDiff.getY() / fV);
        const double fCos(aDiff.getZ() / fV);

        aTemp.set(1, 1, fCos);
        aTemp.set(2, 2, fCos);
        aTemp.set(2, 1, fSin);
        aTemp.set(1, 2, -fSin);

        aTf *= aTemp;
    }

    SetVUV(aTf * aPrj);
}

void Camera3D::SetFocalLength(double fLen)
{
    if (fLen < 5)
        fLen = 5;
    SetPRP(basegfx::B3DPoint(0.0, 0.0, fLen / 35.0 * m_aViewWin.W));
    m_fFocalLength = fLen;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
