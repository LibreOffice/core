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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/camera3d.hxx>
#include <tools/stream.hxx>

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

Camera3D::Camera3D(const basegfx::B3DPoint& rPos, const basegfx::B3DPoint& rLookAt,
                   double fFocalLen, double fBankAng) :
    aResetPos(rPos),
    aResetLookAt(rLookAt),
    fResetFocalLength(fFocalLen),
    fResetBankAngle(fBankAng),
    fBankAngle(fBankAng),
    bAutoAdjustProjection(sal_True)
{
    SetVPD(0);
    SetPosition(rPos);
    SetLookAt(rLookAt);
    SetFocalLength(fFocalLen);
}

/*************************************************************************
|*
|* Default-Konstruktor
|*
\************************************************************************/

Camera3D::Camera3D()
{
    basegfx::B3DPoint aVector3D(0.0 ,0.0 ,1.0);
    Camera3D(aVector3D, basegfx::B3DPoint());
}

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

void Camera3D::Reset()
{
    SetVPD(0);
    fBankAngle = fResetBankAngle;
    SetPosition(aResetPos);
    SetLookAt(aResetLookAt);
    SetFocalLength(fResetFocalLength);
}

/*************************************************************************
|*
|* Defaultwerte fuer Reset setzen
|*
\************************************************************************/

void Camera3D::SetDefaults(const basegfx::B3DPoint& rPos, const basegfx::B3DPoint& rLookAt,
                            double fFocalLen, double fBankAng)
{
    aResetPos           = rPos;
    aResetLookAt        = rLookAt;
    fResetFocalLength   = fFocalLen;
    fResetBankAngle     = fBankAng;
}

/*************************************************************************
|*
|* ViewWindow setzen und PRP anpassen
|*
\************************************************************************/

void Camera3D::SetViewWindow(double fX, double fY, double fW, double fH)
{
    Viewport3D::SetViewWindow(fX, fY, fW, fH);
    if ( bAutoAdjustProjection )
        SetFocalLength(fFocalLength);
}

/*************************************************************************
|*
|* Kameraposition setzen
|*
\************************************************************************/

void Camera3D::SetPosition(const basegfx::B3DPoint& rNewPos)
{
    if ( rNewPos != aPosition )
    {
        aPosition = rNewPos;
        SetVRP(aPosition);
        SetVPN(aPosition - aLookAt);
        SetBankAngle(fBankAngle);
    }
}

/*************************************************************************
|*
|* Blickpunkt setzen
|*
\************************************************************************/

void Camera3D::SetLookAt(const basegfx::B3DPoint& rNewLookAt)
{
    if ( rNewLookAt != aLookAt )
    {
        aLookAt = rNewLookAt;
        SetVPN(aPosition - aLookAt);
        SetBankAngle(fBankAngle);
    }
}

/*************************************************************************
|*
|* Position und Blickpunkt setzen
|*
\************************************************************************/

void Camera3D::SetPosAndLookAt(const basegfx::B3DPoint& rNewPos,
                               const basegfx::B3DPoint& rNewLookAt)
{
    if ( rNewPos != aPosition || rNewLookAt != aLookAt )
    {
        aPosition = rNewPos;
        aLookAt = rNewLookAt;

        SetVRP(aPosition);
        SetVPN(aPosition - aLookAt);
        SetBankAngle(fBankAngle);
    }
}

/*************************************************************************
|*
|* seitlichen Neigungswinkel setzen
|*
\************************************************************************/

void Camera3D::SetBankAngle(double fAngle)
{
    basegfx::B3DVector aDiff(aPosition - aLookAt);
    basegfx::B3DVector aPrj(aDiff);
    fBankAngle = fAngle;

    if ( aDiff.getY() == 0 )
    {
        aPrj.setY(-1.0);
    }
    else
    {   // aPrj = Projektion von aDiff auf die XZ-Ebene
        aPrj.setY(0.0);

        if ( aDiff.getY() < 0.0 )
        {
            aPrj = -aPrj;
        }
    }

    // von aDiff nach oben zeigenden View-Up-Vektor berechnen
    aPrj = aPrj.getPerpendicular(aDiff);
    aPrj = aPrj.getPerpendicular(aDiff);
    aDiff.normalize();

    // auf Z-Achse rotieren, dort um BankAngle drehen und zurueck
    basegfx::B3DHomMatrix aTf;
    const double fV(sqrt(aDiff.getY() * aDiff.getY() + aDiff.getZ() * aDiff.getZ()));

    if ( fV != 0.0 )
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

    aTf.rotate(0.0, 0.0, fBankAngle);

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

    if ( fV != 0.0 )
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

/*************************************************************************
|*
|* Brennweite setzen
|*
\************************************************************************/

void Camera3D::SetFocalLength(double fLen)
{
    if ( fLen < 5 )
        fLen = 5;
    SetPRP(basegfx::B3DPoint(0.0, 0.0, fLen / 35.0 * aViewWin.W));
    fFocalLength = fLen;
}

/*************************************************************************
|*
|* Um die Kameraposition drehen, LookAt wird dabei veraendert
|*
\************************************************************************/

void Camera3D::Rotate(double fHAngle, double fVAngle)
{
    basegfx::B3DHomMatrix aTf;
    basegfx::B3DVector aDiff(aLookAt - aPosition);
    const double fV(sqrt(aDiff.getX() * aDiff.getX() + aDiff.getZ() * aDiff.getZ()));

    if ( fV != 0.0 )
    {
        basegfx::B3DHomMatrix aTemp;
        const double fSin(aDiff.getZ() / fV);
        const double fCos(aDiff.getX() / fV);

        aTemp.set(0, 0, fCos);
        aTemp.set(2, 2, fCos);
        aTemp.set(0, 2, fSin);
        aTemp.set(2, 0, -fSin);

        aTf *= aTemp;
    }

    {
        aTf.rotate(0.0, 0.0, fVAngle);
    }

    if ( fV != 0.0 )
    {
        basegfx::B3DHomMatrix aTemp;
        const double fSin(-aDiff.getZ() / fV);
        const double fCos(aDiff.getX() / fV);

        aTemp.set(0, 0, fCos);
        aTemp.set(2, 2, fCos);
        aTemp.set(0, 2, fSin);
        aTemp.set(2, 0, -fSin);

        aTf *= aTemp;
    }

    {
        aTf.rotate(0.0, fHAngle, 0.0);
    }

    aDiff *= aTf;
    SetLookAt(aPosition + aDiff);
}


/*************************************************************************
|*
|* Um den Blickpunkt drehen, Position wird dabei veraendert
|*
\************************************************************************/

void Camera3D::RotateAroundLookAt(double fHAngle, double fVAngle)
{
    basegfx::B3DHomMatrix aTf;
    basegfx::B3DVector aDiff(aPosition - aLookAt);
    const double fV(sqrt(aDiff.getX() * aDiff.getX() + aDiff.getZ() * aDiff.getZ()));

    if ( fV != 0.0 )
    {
        basegfx::B3DHomMatrix aTemp;
        const double fSin(aDiff.getZ() / fV);
        const double fCos(aDiff.getX() / fV);

        aTemp.set(0, 0, fCos);
        aTemp.set(2, 2, fCos);
        aTemp.set(0, 2, fSin);
        aTemp.set(2, 0, -fSin);

        aTf *= aTemp;
    }

    {
        aTf.rotate(0.0, 0.0, fVAngle);
    }

    if ( fV != 0.0 )
    {
        basegfx::B3DHomMatrix aTemp;
        const double fSin(-aDiff.getZ() / fV);
        const double fCos(aDiff.getX() / fV);

        aTemp.set(0, 0, fCos);
        aTemp.set(2, 2, fCos);
        aTemp.set(0, 2, fSin);
        aTemp.set(2, 0, -fSin);

        aTf *= aTemp;
    }

    {
        aTf.rotate(0.0, fHAngle, 0.0);
    }

    aDiff *= aTf;
    SetPosition(aLookAt + aDiff);
}

/*************************************************************************
|*
|* FG: ??? Setzt wohl die Projektionsebene in eine bestimmte Tiefe
|*
\************************************************************************/

void Camera3D::SetFocalLengthWithCorrect(double fLen)
{
    if ( fLen < 5.0 )
    {
        fLen = 5.0;
    }

    SetPRP(basegfx::B3DPoint(0.0, 0.0, aPRP.getZ() * fLen / fFocalLength));
    fFocalLength = fLen;
}

// eof
