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

#ifndef _CAMERA3D_HXX
#define _CAMERA3D_HXX

#include <svx/viewpt3d.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* Camera derivative
|*
\************************************************************************/

class SVX_DLLPUBLIC Camera3D : public Viewport3D
{
 protected:
    basegfx::B3DPoint   aResetPos;
    basegfx::B3DPoint   aResetLookAt;
    double      fResetFocalLength;
    double      fResetBankAngle;

    basegfx::B3DPoint   aPosition;
    basegfx::B3DPoint   aLookAt;
    double      fFocalLength;
    double      fBankAngle;

    bool bAutoAdjustProjection;

 public:
    Camera3D(const basegfx::B3DPoint& rPos, const basegfx::B3DPoint& rLookAt,
             double fFocalLen = 35.0, double fBankAng = 0);
    Camera3D();

    // Reset to default values
    void Reset();

    void SetDefaults(const basegfx::B3DPoint& rPos, const basegfx::B3DPoint& rLookAt,
                     double fFocalLen = 35.0, double fBankAng = 0);

    void SetViewWindow(double fX, double fY, double fW, double fH);

    void SetPosition(const basegfx::B3DPoint& rNewPos);
    const basegfx::B3DPoint& GetPosition() const { return aPosition; }
    void SetLookAt(const basegfx::B3DPoint& rNewLookAt);
    const basegfx::B3DPoint& GetLookAt() const { return aLookAt; }
    void SetPosAndLookAt(const basegfx::B3DPoint& rNewPos, const basegfx::B3DPoint& rNewLookAt);

    // Focal length in mm
    void    SetFocalLength(double fLen);
    void    SetFocalLengthWithCorrect(double fLen);
    double  GetFocalLength() const { return fFocalLength; }

    // Bank angle links/rechts
    void    SetBankAngle(double fAngle);
    double  GetBankAngle() const { return fBankAngle; }

    // For rotating the camera position. Changes LookAt.
    void Rotate(double fHAngle, double fVAngle);

    // For changing the point of view. Changes the position.
    void RotateAroundLookAt(double fHAngle, double fVAngle);

    void SetAutoAdjustProjection(bool bAdjust = true)
        { bAutoAdjustProjection = bAdjust; }
    bool IsAutoAdjustProjection() const { return bAutoAdjustProjection; }
};

#endif      // _CAMERA3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
