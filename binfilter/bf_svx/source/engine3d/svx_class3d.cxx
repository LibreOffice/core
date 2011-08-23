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




#include "matrix3d.hxx"
namespace binfilter {

/*************************************************************************
|*
|* Cast-Operator auf homogene 4x4 Matrix
|*
\************************************************************************/

/*N*/ Old_Matrix3D::operator Matrix4D()
/*N*/ {
/*N*/ 	Matrix4D aNewMat;
/*N*/ 
/*N*/ 	aNewMat[0] = Point4D(M[0], aTranslation[0]);
/*N*/ 	aNewMat[1] = Point4D(M[1], aTranslation[1]);
/*N*/ 	aNewMat[2] = Point4D(M[2], aTranslation[2]);
/*N*/ 
/*N*/ 	return aNewMat;
/*N*/ }

/*************************************************************************
|*
|* Zuweisungs-Operator mit homogener 4x4 Matrix
|*
\************************************************************************/

/*N*/ Old_Matrix3D Old_Matrix3D::operator=(const Matrix4D& rMat)
/*N*/ {
/*N*/ 	M[0] = Vector3D(rMat[0][0], rMat[0][1], rMat[0][2]);
/*N*/ 	M[1] = Vector3D(rMat[1][0], rMat[1][1], rMat[1][2]);
/*N*/ 	M[2] = Vector3D(rMat[2][0], rMat[2][1], rMat[2][2]);
/*N*/ 	aTranslation = Vector3D(rMat[0][3], rMat[1][3], rMat[2][3]);
/*N*/ 
/*N*/ 	return *this;
/*N*/ }

/*************************************************************************
|*
|* Stream-In-Operator fuer Matrix3D
|*
\************************************************************************/

/*N*/ SvStream& operator>>(SvStream& rIStream, Old_Matrix3D& rMatrix3D)
/*N*/ {
/*N*/ 	for (int i = 0; i < 3; i++)
/*N*/ 		rIStream >> rMatrix3D.M[i];
/*N*/ 
/*N*/ 	rIStream >> rMatrix3D.aTranslation;
/*N*/ 
/*N*/ 	return rIStream;
/*N*/ }

/*************************************************************************
|*
|* Stream-Out-Operator fuer Matrix3D
|*
\************************************************************************/

/*N*/ SvStream& operator<<(SvStream& rOStream, const Old_Matrix3D& rMatrix3D)
/*N*/ {
/*N*/ 	for (int i = 0; i < 3; i++)
/*N*/ 		rOStream << rMatrix3D.M[i];
/*N*/ 
/*N*/ 	rOStream << rMatrix3D.aTranslation;
/*N*/ 
/*N*/ 	return rOStream;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
