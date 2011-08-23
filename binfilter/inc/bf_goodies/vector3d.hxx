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

#ifndef _SVX_VECTOR3D_HXX
#define _SVX_VECTOR3D_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _INC_MATH
#include <math.h>
#endif

#include <vector>

class Point;
class SvStream;

/*************************************************************************
|*
|* 3D-Vektor
|*
\************************************************************************/

namespace binfilter {
class Vector3D
{
 protected:
    double V[3];

 public:
    Vector3D(double fX = 0, double fY = 0, double fZ = 0)
        { V[0] = fX; V[1] = fY; V[2] = fZ; }
    Vector3D(const Point& rPnt, double fZ = 0);
    const double& X() const { return V[0]; }
    const double& Y() const { return V[1]; }
    const double& Z() const { return V[2]; }
    double& X() { return V[0]; }
    double& Y() { return V[1]; }
    double& Z() { return V[2]; }
    const double& operator[] (int nPos) const { return V[nPos]; }
    double& operator[] (int nPos) { return V[nPos]; }

    double	GetLength(void) const;
    double	GetXZLength(void) const;
    double	GetYZLength(void) const;

    void	Normalize();
    void	Min(const Vector3D& rVec);
    void	Max(const Vector3D& rVec);
    void	Abs();

    void CalcInBetween(const Vector3D& rOld1, const Vector3D& rOld2, double t);
    void CalcMiddle(const Vector3D& rOld1, const Vector3D& rOld2);
    
    Vector3D&	operator+=	(const Vector3D&);
    Vector3D&	operator-=	(const Vector3D&);
    Vector3D	operator+ 	(const Vector3D&) const;
    Vector3D	operator-	(const Vector3D&) const;
    Vector3D	operator-	(void) const;

    // Kreuzprodukt mittels operator| (wie: Senkrecht)
    Vector3D&	operator|=	(const Vector3D&);
    Vector3D	operator|	(const Vector3D&) const;

    // Skalarprodukt als Methode, um Verwechslungen zu vermeiden
    double Scalar(const Vector3D&) const;

    Vector3D&	operator/=	(const Vector3D&);
    Vector3D	operator/	(const Vector3D&) const;
    Vector3D&	operator*=	(const Vector3D&);
    Vector3D	operator*	(const Vector3D&) const;

    Vector3D&	operator*=	(double);
    Vector3D	operator*	(double) const;
    Vector3D&	operator/=	(double);
    Vector3D	operator/	(double) const;

    BOOL		operator==	(const Vector3D&) const;
    BOOL		operator!=	(const Vector3D&) const;

    friend SvStream& operator>>(SvStream& rIStream, Vector3D&);
    friend SvStream& operator<<(SvStream& rOStream, const Vector3D&);
};

// #110988# typedefs for a vector of Vector3Ds
typedef ::std::vector< Vector3D > Vector3DVector;

}//end of namespace binfilter
#endif // _SVX_VECTOR3D_HXX
