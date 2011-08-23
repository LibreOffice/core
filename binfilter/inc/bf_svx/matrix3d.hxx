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

#ifndef _SVX_MATRIX3D_HXX
#define _SVX_MATRIX3D_HXX

#include <bf_goodies/hmatrix.hxx>

namespace binfilter {

/*************************************************************************
|*
|* 3D-Transformationsmatrix, bestehend aus 3 Zeilenvektoren und
|* einem Spaltenvektor, der die Translation enthaelt:
|*
|*      |V11 V12 V13 T1|
|*  M = |V21 V22 C33 T2|
|*      |V31 V32 V33 T3|
|*
|* Vektortransformation durch Postmultiplikation mit Spaltenvektoren:
|* V' = M * V
|* Matrixverkettung per Postmultiplikation mit Transformation oder
|* anderer Matrix:
|* M' = [T,S,Rxyz] * M oder
|* M' = M2 * M
|*
\************************************************************************/

class Old_Matrix3D
{
protected:
    Vector3D					M[3];
    Vector3D					aTranslation;

 public:
    // default: Einheitsmatrix erstellen (Vektoren sind auf 0 initialisiert)
    Old_Matrix3D() { M[0][0] = M[1][1] = M[2][2] = 1; }

    // Zeilenvektor zurueckgeben
    Vector3D& operator[](int nPos) { return M[nPos]; }
    const Vector3D& operator[](int nPos) const { return M[nPos]; }

    // Spaltenvektor zurueckgeben
    Vector3D GetColumnVector(int nCol) const
        { return Vector3D(M[0][nCol], M[1][nCol], M[2][nCol]); }

    friend SvStream& operator>>(SvStream& rIStream, Old_Matrix3D&);
    friend SvStream& operator<<(SvStream& rOStream, const Old_Matrix3D&);

    operator Matrix4D();
    Old_Matrix3D operator=(const Matrix4D&);
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
