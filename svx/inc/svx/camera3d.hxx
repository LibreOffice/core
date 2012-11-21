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
    double  GetFocalLength() const { return fFocalLength; }

    // Bank angle links/rechts
    void    SetBankAngle(double fAngle);
    double  GetBankAngle() const { return fBankAngle; }

    void SetAutoAdjustProjection(bool bAdjust = true)
        { bAutoAdjustProjection = bAdjust; }
    bool IsAutoAdjustProjection() const { return bAutoAdjustProjection; }
};

#endif      // _CAMERA3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
