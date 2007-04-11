/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: camera3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:39:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CAMERA3D_HXX
#define _CAMERA3D_HXX

#ifndef _VIEWPT3D_HXX
#include <svx/viewpt3d.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

    void SetAutoAdjustProjection(FASTBOOL bAdjust = TRUE)
        { bAutoAdjustProjection = bAdjust; }
    FASTBOOL IsAutoAdjustProjection() const { return bAutoAdjustProjection; }
};

#endif      // _CAMERA3D_HXX
