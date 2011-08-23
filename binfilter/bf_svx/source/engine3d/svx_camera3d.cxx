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

#include "camera3d.hxx"


#include <svdio.hxx>
namespace binfilter {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ Camera3D::Camera3D(const Vector3D& rPos, const Vector3D& rLookAt,
/*N*/ 				   double fFocalLen, double fBankAng) :
/*N*/ 	aResetPos(rPos),
/*N*/ 	aResetLookAt(rLookAt),
/*N*/ 	fResetFocalLength(fFocalLen),
/*N*/ 	fResetBankAngle(fBankAng),
/*N*/ 	fBankAngle(fBankAng),
/*N*/ 	bAutoAdjustProjection(TRUE)
/*N*/ {
/*N*/ 	SetVPD(0);
/*N*/ 	SetPosition(rPos);
/*N*/ 	SetLookAt(rLookAt);
/*N*/ 	SetFocalLength(fFocalLen);
/*N*/ }

/*************************************************************************
|*
|* Default-Konstruktor
|*
\************************************************************************/

/*?*/ Camera3D::Camera3D()
/*?*/ {
/*?*/ 	Vector3D aVector3D(0,0,1);
/*?*/ 	Camera3D(aVector3D, Vector3D());
/*?*/ }

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ void Camera3D::Reset()
/*N*/ {
/*N*/ 	SetVPD(0);
/*N*/ 	fBankAngle = fResetBankAngle;
/*N*/ 	SetPosition(aResetPos);
/*N*/ 	SetLookAt(aResetLookAt);
/*N*/ 	SetFocalLength(fResetFocalLength);
/*N*/ }

/*************************************************************************
|*
|* Defaultwerte fuer Reset setzen
|*
\************************************************************************/

/*N*/ void Camera3D::SetDefaults(const Vector3D& rPos, const Vector3D& rLookAt,
/*N*/ 							double fFocalLen, double fBankAng)
/*N*/ {
/*N*/ 	aResetPos			= rPos;
/*N*/ 	aResetLookAt		= rLookAt;
/*N*/ 	fResetFocalLength	= fFocalLen;
/*N*/ 	fResetBankAngle 	= fBankAng;
/*N*/ }

/*************************************************************************
|*
|* ViewWindow setzen und PRP anpassen
|*
\************************************************************************/

/*N*/ void Camera3D::SetViewWindow(double fX, double fY, double fW, double fH)
/*N*/ {
/*N*/ 	Viewport3D::SetViewWindow(fX, fY, fW, fH);
/*N*/ 	if ( bAutoAdjustProjection )
/*N*/ 		SetFocalLength(fFocalLength);
/*N*/ }

/*************************************************************************
|*
|* Kameraposition setzen
|*
\************************************************************************/

/*N*/ void Camera3D::SetPosition(const Vector3D& rNewPos)
/*N*/ {
/*N*/ 	if ( rNewPos != aPosition )
/*N*/ 	{
/*N*/ 		aPosition = rNewPos;
/*N*/ 		SetVRP(aPosition);
/*N*/ 		SetVPN(aPosition - aLookAt);
/*N*/ 		SetBankAngle(fBankAngle);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Blickpunkt setzen
|*
\************************************************************************/

/*N*/ void Camera3D::SetLookAt(const Vector3D& rNewLookAt)
/*N*/ {
/*N*/ 	if ( rNewLookAt != aLookAt )
/*N*/ 	{
/*N*/ 		aLookAt = rNewLookAt;
/*N*/ 		SetVPN(aPosition - aLookAt);
/*N*/ 		SetBankAngle(fBankAngle);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Position und Blickpunkt setzen
|*
\************************************************************************/

/*N*/ void Camera3D::SetPosAndLookAt(const Vector3D& rNewPos,
/*N*/ 							   const Vector3D& rNewLookAt)
/*N*/ {
/*N*/ 	if ( rNewPos != aPosition || rNewLookAt != aLookAt )
/*N*/ 	{
/*N*/ 		aPosition = rNewPos;
/*N*/ 		aLookAt = rNewLookAt;
/*N*/ 
/*N*/ 		SetVRP(aPosition);
/*N*/ 		SetVPN(aPosition - aLookAt);
/*N*/ 		SetBankAngle(fBankAngle);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* seitlichen Neigungswinkel setzen
|*
\************************************************************************/

/*N*/ void Camera3D::SetBankAngle(double fAngle)
/*N*/ {
/*N*/ 	Vector3D aDiff = aPosition - aLookAt;
/*N*/ 	Vector3D aPrj = aDiff;
/*N*/ 
/*N*/ 	fBankAngle = fAngle;
/*N*/ 
/*N*/ 	if ( aDiff.Y() == 0 )
/*N*/ 		aPrj.Y() = -1;
/*N*/ 	else
/*N*/ 	{	// aPrj = Projektion von aDiff auf die XZ-Ebene
/*N*/ 		aPrj.Y() = 0;
/*N*/ 		if ( aDiff.Y() < 0 )
/*N*/ 			aPrj = -aPrj;
/*N*/ 	}
/*N*/ 	// von aDiff nach oben zeigenden View-Up-Vektor berechnen
/*N*/ 	aPrj |= aDiff;
/*N*/ 	aPrj |= aDiff;
/*N*/ 	aDiff.Normalize();
/*N*/ 
/*N*/ 	// auf Z-Achse rotieren, dort um BankAngle drehen und zurueck
/*N*/ 	Matrix4D aTf;
/*N*/ 	double fV = sqrt(aDiff.Y() * aDiff.Y() + aDiff.Z() * aDiff.Z());
/*N*/ 
/*N*/ 	if ( fV != 0 )
/*N*/ 		aTf.RotateX(aDiff.Y() / fV, aDiff.Z() / fV);
/*N*/ 	aTf.RotateY(-aDiff.X(), fV);
/*N*/ 	aTf.RotateZ(fBankAngle);
/*N*/ 	aTf.RotateY(aDiff.X(), fV);
/*N*/ 	if ( fV != 0 )
/*N*/ 		aTf.RotateX(-aDiff.Y() / fV, aDiff.Z() / fV);
/*N*/ 
/*N*/ 	SetVUV(aTf * aPrj);
/*N*/ }

/*************************************************************************
|*
|* Brennweite setzen
|*
\************************************************************************/

/*N*/ void Camera3D::SetFocalLength(double fLen)
/*N*/ {
/*N*/ 	if ( fLen < 5 )
/*N*/ 		fLen = 5;
/*N*/ 	SetPRP(Vector3D(0, 0, fLen / 35.0 * aViewWin.W));
/*N*/ 	fFocalLength = fLen;
/*N*/ }

/*************************************************************************
|*
|* Um den Blickpunkt drehen, Position wird dabei veraendert
|*
\************************************************************************/

/*N*/ void Camera3D::RotateAroundLookAt(double fHAngle, double fVAngle)
/*N*/ {
/*N*/ 	Matrix4D aTf;
/*N*/ 	Vector3D aDiff = aPosition - aLookAt;
/*N*/ 	double fV = sqrt(aDiff.X() * aDiff.X() + aDiff.Z() * aDiff.Z());
/*N*/ 
/*N*/ 	if ( fV != 0 )	aTf.RotateY(aDiff.Z() / fV, aDiff.X() / fV);
/*N*/ 	aTf.RotateZ(fVAngle);
/*N*/ 	if ( fV != 0 )	aTf.RotateY(-aDiff.Z() / fV, aDiff.X() / fV);
/*N*/ 	aTf.RotateY(fHAngle);
/*N*/ 
/*N*/ 	aDiff *= aTf;
/*N*/ 	SetPosition(aLookAt + aDiff);
/*N*/ }

/*************************************************************************
|*
|* Stream-Out-Operator fuer Camera3D   fuer die Filerevision 3.1
|*
\************************************************************************/

/*N*/ void Camera3D::WriteData31(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	Viewport3D::WriteData(rOut);
/*N*/ 
/*N*/ 	rOut << aResetPos;
/*N*/ 	rOut << aResetLookAt;
/*N*/ 	rOut << fResetFocalLength;
/*N*/ 	rOut << fResetBankAngle;
/*N*/ 	rOut << aPosition;
/*N*/ 	rOut << aLookAt;
/*N*/ 	rOut << fFocalLength;
/*N*/ 	rOut << fBankAngle;
/*N*/ 	rOut << BOOL(bAutoAdjustProjection);
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* Stream-Out-Operator fuer Camera3D
|* Zur Version 356 wurde das Fileformat inkompatibel, wenn man das alte
|* Format schreiben will, muss man die Version am Stream abfragen.
|*
\************************************************************************/

/*N*/ void Camera3D::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 
/*N*/ 	if (rOut.GetVersion() < 3560)  // FG: Ab der Release 356 wurde das Fileformat geaendert
/*N*/ 	{                              //     Falls das Format der Version 31 geschrieben werden soll
/*N*/ 		WriteData31(rOut);         //     muss am Stream die Version der 3.1 gesetzt sein.
/*N*/ 		return;                    //     Im Prinzip kann man auf diese Art auch Zwischenversionen
/*N*/ 	}                              //     erzeugen.
/*N*/ 
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("Camera3D");
/*N*/ #endif
/*N*/ 	Viewport3D::WriteData(rOut);
/*N*/ 
/*N*/ 	rOut << aResetPos;              // Alles Vektoren 3*double
/*N*/ 	rOut << aResetLookAt;
/*N*/ 	rOut << fResetFocalLength;
/*N*/ 	rOut << fResetBankAngle;
/*N*/ 	rOut << aPosition;
/*N*/ 	rOut << aLookAt;
/*N*/ 	rOut << fFocalLength;
/*N*/ 	rOut << fBankAngle;
/*N*/ 	rOut << BOOL(bAutoAdjustProjection);
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* Stream-In-Operator fuer Camera3D
|*
\************************************************************************/

/*N*/ void Camera3D::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if ( rIn.GetError() != SVSTREAM_OK )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if ((rHead.GetVersion() < 13) || (rIn.GetVersion() < 3560))
/*N*/ 	{
/*N*/ 		ReadData31(rIn);
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("Camera3D");
/*N*/ #endif
/*N*/ 
/*N*/ 	Viewport3D::ReadData(rHead, rIn);
/*N*/ 
/*N*/ 	BOOL bTmp;
/*N*/ 
/*N*/ 	rIn >> aResetPos;
/*N*/ 	rIn >> aResetLookAt;
/*N*/ 	rIn >> fResetFocalLength;
/*N*/ 	rIn >> fResetBankAngle;
/*N*/ 	rIn >> aPosition;
/*N*/ 	rIn >> aLookAt;
/*N*/ 	rIn >> fFocalLength;
/*N*/ 	rIn >> fBankAngle;
/*N*/ 	rIn >> bTmp; bAutoAdjustProjection = bTmp;
/*N*/ 
/*N*/ 	SetVPD(0);
/*N*/ 	SetPosAndLookAt(aPosition, aLookAt);
/*N*/ }

/*************************************************************************
|*
|* Stream-In-Operator fuer Camera3D
|*
\************************************************************************/

/*N*/ void Camera3D::ReadData31(SvStream& rIn)
/*N*/ {
/*N*/ 	if ( rIn.GetError() != SVSTREAM_OK )
/*N*/ 		return;
/*N*/ 
/*N*/ 	Viewport3D::ReadData31 (rIn);
/*N*/ 
/*N*/ 	BOOL bTmp;
/*N*/ 
/*N*/ 	rIn >> aResetPos;
/*N*/ 	rIn >> aResetLookAt;
/*N*/ 	rIn >> fResetFocalLength;
/*N*/ 	rIn >> fResetBankAngle;
/*N*/ 	rIn >> aPosition;
/*N*/ 	rIn >> aLookAt;
/*N*/ 	rIn >> fFocalLength;
/*N*/ 	rIn >> fBankAngle;
/*N*/ 	rIn >> bTmp; bAutoAdjustProjection = bTmp;
/*N*/ 
/*N*/ 	SetVPD(0);
/*N*/ 	SetPosAndLookAt(aPosition, aLookAt);
/*N*/ }

/*************************************************************************
|*
|* Stream-Out-Operator fuer Camera3D
|*
\************************************************************************/

/*N*/ SvStream& operator<<(SvStream& rOStream, const Camera3D& rCam)
/*N*/ {
/*N*/ 	rCam.WriteData31(rOStream);
/*N*/ 	return rOStream;
/*N*/ }

/*************************************************************************
|*
|* Stream-In-Operator fuer Camera3D
|*
\************************************************************************/

/*N*/ SvStream& operator>>(SvStream& rIStream, Camera3D& rCam)
/*N*/ {
/*N*/ 	rCam.ReadData31(rIStream);
/*N*/ 	return rIStream;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
