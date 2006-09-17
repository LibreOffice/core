/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: camera3d.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:54:26 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _CAMERA3D_HXX
#include "camera3d.hxx"
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

Camera3D::Camera3D(const Vector3D& rPos, const Vector3D& rLookAt,
                   double fFocalLen, double fBankAng) :
    aResetPos(rPos),
    aResetLookAt(rLookAt),
    fResetFocalLength(fFocalLen),
    fResetBankAngle(fBankAng),
    fBankAngle(fBankAng),
    bAutoAdjustProjection(TRUE)
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
    Vector3D aVector3D(0,0,1);
    Camera3D(aVector3D, Vector3D());
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

void Camera3D::SetDefaults(const Vector3D& rPos, const Vector3D& rLookAt,
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

void Camera3D::SetPosition(const Vector3D& rNewPos)
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

void Camera3D::SetLookAt(const Vector3D& rNewLookAt)
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

void Camera3D::SetPosAndLookAt(const Vector3D& rNewPos,
                               const Vector3D& rNewLookAt)
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
    Vector3D aDiff = aPosition - aLookAt;
    Vector3D aPrj = aDiff;

    fBankAngle = fAngle;

    if ( aDiff.Y() == 0 )
        aPrj.Y() = -1;
    else
    {   // aPrj = Projektion von aDiff auf die XZ-Ebene
        aPrj.Y() = 0;
        if ( aDiff.Y() < 0 )
            aPrj = -aPrj;
    }
    // von aDiff nach oben zeigenden View-Up-Vektor berechnen
    aPrj |= aDiff;
    aPrj |= aDiff;
    aDiff.Normalize();

    // auf Z-Achse rotieren, dort um BankAngle drehen und zurueck
    Matrix4D aTf;
    double fV = sqrt(aDiff.Y() * aDiff.Y() + aDiff.Z() * aDiff.Z());

    if ( fV != 0 )
        aTf.RotateX(aDiff.Y() / fV, aDiff.Z() / fV);
    aTf.RotateY(-aDiff.X(), fV);
    aTf.RotateZ(fBankAngle);
    aTf.RotateY(aDiff.X(), fV);
    if ( fV != 0 )
        aTf.RotateX(-aDiff.Y() / fV, aDiff.Z() / fV);

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
    SetPRP(Vector3D(0, 0, fLen / 35.0 * aViewWin.W));
    fFocalLength = fLen;
}

/*************************************************************************
|*
|* Um die Kameraposition drehen, LookAt wird dabei veraendert
|*
\************************************************************************/

void Camera3D::Rotate(double fHAngle, double fVAngle)
{
    Matrix4D aTf;
    Vector3D aDiff = aLookAt - aPosition;
    double fV = sqrt(aDiff.X() * aDiff.X() + aDiff.Z() * aDiff.Z());

    if ( fV != 0 )  aTf.RotateY(aDiff.Z() / fV, aDiff.X() / fV);
    aTf.RotateZ(fVAngle);
    if ( fV != 0 )  aTf.RotateY(-aDiff.Z() / fV, aDiff.X() / fV);
    aTf.RotateY(fHAngle);

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
    Matrix4D aTf;
    Vector3D aDiff = aPosition - aLookAt;
    double fV = sqrt(aDiff.X() * aDiff.X() + aDiff.Z() * aDiff.Z());

    if ( fV != 0 )  aTf.RotateY(aDiff.Z() / fV, aDiff.X() / fV);
    aTf.RotateZ(fVAngle);
    if ( fV != 0 )  aTf.RotateY(-aDiff.Z() / fV, aDiff.X() / fV);
    aTf.RotateY(fHAngle);

    aDiff *= aTf;
    SetPosition(aLookAt + aDiff);
}

/*************************************************************************
|*
|* Stream-Out-Operator fuer Camera3D   fuer die Filerevision 3.1
|*
\************************************************************************/

//BFS01void Camera3D::WriteData31(SvStream& rOut) const
//BFS01{
//BFS01#ifndef SVX_LIGHT
//BFS01 Viewport3D::WriteData(rOut);
//BFS01
//BFS01 rOut << aResetPos;
//BFS01 rOut << aResetLookAt;
//BFS01 rOut << fResetFocalLength;
//BFS01 rOut << fResetBankAngle;
//BFS01 rOut << aPosition;
//BFS01 rOut << aLookAt;
//BFS01 rOut << fFocalLength;
//BFS01 rOut << fBankAngle;
//BFS01 rOut << BOOL(bAutoAdjustProjection);
//BFS01#endif
//BFS01}

/*************************************************************************
|*
|* Stream-Out-Operator fuer Camera3D
|* Zur Version 356 wurde das Fileformat inkompatibel, wenn man das alte
|* Format schreiben will, muss man die Version am Stream abfragen.
|*
\************************************************************************/

//BFS01void Camera3D::WriteData(SvStream& rOut) const
//BFS01{
//BFS01#ifndef SVX_LIGHT
//BFS01
//BFS01 if (rOut.GetVersion() < 3560)  // FG: Ab der Release 356 wurde das Fileformat geaendert
//BFS01 {                              //     Falls das Format der Version 31 geschrieben werden soll
//BFS01     WriteData31(rOut);         //     muss am Stream die Version der 3.1 gesetzt sein.
//BFS01     return;                    //     Im Prinzip kann man auf diese Art auch Zwischenversionen
//BFS01 }                              //     erzeugen.
//BFS01
//BFS01 SdrDownCompat aCompat(rOut, STREAM_WRITE);
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("Camera3D");
//BFS01#endif
//BFS01 Viewport3D::WriteData(rOut);
//BFS01
//BFS01 rOut << aResetPos;              // Alles Vektoren 3*double
//BFS01 rOut << aResetLookAt;
//BFS01 rOut << fResetFocalLength;
//BFS01 rOut << fResetBankAngle;
//BFS01 rOut << aPosition;
//BFS01 rOut << aLookAt;
//BFS01 rOut << fFocalLength;
//BFS01 rOut << fBankAngle;
//BFS01 rOut << BOOL(bAutoAdjustProjection);
//BFS01#endif
//BFS01}

/*************************************************************************
|*
|* Stream-In-Operator fuer Camera3D
|*
\************************************************************************/

//BFS01void Camera3D::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 if ( rIn.GetError() != SVSTREAM_OK )
//BFS01     return;
//BFS01
//BFS01 if ((rHead.GetVersion() < 13) || (rIn.GetVersion() < 3560))
//BFS01 {
//BFS01     ReadData31(rIn);
//BFS01     return;
//BFS01 }
//BFS01
//BFS01 SdrDownCompat aCompat(rIn, STREAM_READ);
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("Camera3D");
//BFS01#endif
//BFS01
//BFS01 Viewport3D::ReadData(rHead, rIn);
//BFS01
//BFS01 BOOL bTmp;
//BFS01
//BFS01 rIn >> aResetPos;
//BFS01 rIn >> aResetLookAt;
//BFS01 rIn >> fResetFocalLength;
//BFS01 rIn >> fResetBankAngle;
//BFS01 rIn >> aPosition;
//BFS01 rIn >> aLookAt;
//BFS01 rIn >> fFocalLength;
//BFS01 rIn >> fBankAngle;
//BFS01 rIn >> bTmp; bAutoAdjustProjection = bTmp;
//BFS01
//BFS01 SetVPD(0);
//BFS01 SetPosAndLookAt(aPosition, aLookAt);
//BFS01}

/*************************************************************************
|*
|* Stream-In-Operator fuer Camera3D
|*
\************************************************************************/

//BFS01void Camera3D::ReadData31(SvStream& rIn)
//BFS01{
//BFS01 if ( rIn.GetError() != SVSTREAM_OK )
//BFS01     return;
//BFS01
//BFS01 Viewport3D::ReadData31 (rIn);
//BFS01
//BFS01 BOOL bTmp;
//BFS01
//BFS01 rIn >> aResetPos;
//BFS01 rIn >> aResetLookAt;
//BFS01 rIn >> fResetFocalLength;
//BFS01 rIn >> fResetBankAngle;
//BFS01 rIn >> aPosition;
//BFS01 rIn >> aLookAt;
//BFS01 rIn >> fFocalLength;
//BFS01 rIn >> fBankAngle;
//BFS01 rIn >> bTmp; bAutoAdjustProjection = bTmp;
//BFS01
//BFS01 SetVPD(0);
//BFS01 SetPosAndLookAt(aPosition, aLookAt);
//BFS01}

/*************************************************************************
|*
|* Stream-Out-Operator fuer Camera3D
|*
\************************************************************************/

//BFS01SvStream& operator<<(SvStream& rOStream, const Camera3D& rCam)
//BFS01{
//BFS01 rCam.WriteData31(rOStream);
//BFS01 return rOStream;
//BFS01}

/*************************************************************************
|*
|* Stream-In-Operator fuer Camera3D
|*
\************************************************************************/

//BFS01SvStream& operator>>(SvStream& rIStream, Camera3D& rCam)
//BFS01{
//BFS01 rCam.ReadData31(rIStream);
//BFS01 return rIStream;
//BFS01}


/*************************************************************************
|*
|* FG: ??? Setzt wohl die Projektionsebene in eine bestimmte Tiefe
|*
\************************************************************************/

void Camera3D::SetFocalLengthWithCorrect(double fLen)
{
    if ( fLen < 5 )
        fLen = 5;
    SetPRP(Vector3D(0, 0, aPRP.Z () * fLen / fFocalLength));
    fFocalLength = fLen;
}

// eof
