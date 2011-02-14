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
|* Kamera-Ableitung
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

    FASTBOOL    bAutoAdjustProjection;

 public:
    Camera3D(const basegfx::B3DPoint& rPos, const basegfx::B3DPoint& rLookAt,
             double fFocalLen = 35.0, double fBankAng = 0);
    Camera3D();

    // Anfangswerte wieder herstellen
    void Reset();

    void SetDefaults(const basegfx::B3DPoint& rPos, const basegfx::B3DPoint& rLookAt,
                     double fFocalLen = 35.0, double fBankAng = 0);

    void SetViewWindow(double fX, double fY, double fW, double fH);

    void SetPosition(const basegfx::B3DPoint& rNewPos);
    const basegfx::B3DPoint& GetPosition() const { return aPosition; }
    void SetLookAt(const basegfx::B3DPoint& rNewLookAt);
    const basegfx::B3DPoint& GetLookAt() const { return aLookAt; }
    void SetPosAndLookAt(const basegfx::B3DPoint& rNewPos, const basegfx::B3DPoint& rNewLookAt);

    // Brennweite in mm
    void    SetFocalLength(double fLen);
    void    SetFocalLengthWithCorrect(double fLen);
    double  GetFocalLength() const { return fFocalLength; }

    // Neigung links/rechts
    void    SetBankAngle(double fAngle);
    double  GetBankAngle() const { return fBankAngle; }

    // Um die Kameraposition drehen, LookAt wird dabei veraendert
    void Rotate(double fHAngle, double fVAngle);

    // Um den Blickpunkt drehen, Position wird dabei veraendert
    void RotateAroundLookAt(double fHAngle, double fVAngle);

    void SetAutoAdjustProjection(FASTBOOL bAdjust = sal_True)
        { bAutoAdjustProjection = bAdjust; }
    FASTBOOL IsAutoAdjustProjection() const { return bAutoAdjustProjection; }
};

#endif      // _CAMERA3D_HXX
