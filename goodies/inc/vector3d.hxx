/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vector3d.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:11:13 $
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

#ifndef _SVX_VECTOR3D_HXX
#define _SVX_VECTOR3D_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#include <math.h>
#include <vector>

class Point;
class SvStream;

/*************************************************************************
|*
|* 3D-Vektor
|*
\************************************************************************/

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

    double  GetLength(void) const;
    double  GetXYLength(void) const;
    double  GetXZLength(void) const;
    double  GetYZLength(void) const;

    void    Normalize();
    void    Min(const Vector3D& rVec);
    void    Max(const Vector3D& rVec);
    void    Abs();

    void CalcInBetween(const Vector3D& rOld1, const Vector3D& rOld2, double t);
    void CalcMiddle(const Vector3D& rOld1, const Vector3D& rOld2);
    void CalcMiddle(const Vector3D& rOld1, const Vector3D& rOld2, Vector3D& rOld3);

    Vector3D GetPerpendicular2D() const;
    Vector3D GetPerpendicular(const Vector3D& rNormalizedVec) const;
    Vector3D GetProjectionOnPlane(const Vector3D& rNormalizedPlane) const;

    Vector3D&   operator+=  (const Vector3D&);
    Vector3D&   operator-=  (const Vector3D&);
    Vector3D    operator+   (const Vector3D&) const;
    Vector3D    operator-   (const Vector3D&) const;
    Vector3D    operator-   (void) const;

    // Kreuzprodukt mittels operator| (wie: Senkrecht)
    Vector3D&   operator|=  (const Vector3D&);
    Vector3D    operator|   (const Vector3D&) const;

    // Skalarprodukt als Methode, um Verwechslungen zu vermeiden
    double Scalar(const Vector3D&) const;

    Vector3D&   operator/=  (const Vector3D&);
    Vector3D    operator/   (const Vector3D&) const;
    Vector3D&   operator*=  (const Vector3D&);
    Vector3D    operator*   (const Vector3D&) const;

    Vector3D&   operator*=  (double);
    Vector3D    operator*   (double) const;
    Vector3D&   operator/=  (double);
    Vector3D    operator/   (double) const;

    BOOL        operator==  (const Vector3D&) const;
    BOOL        operator!=  (const Vector3D&) const;

    friend SvStream& operator>>(SvStream& rIStream, Vector3D&);
    friend SvStream& operator<<(SvStream& rOStream, const Vector3D&);
};

// #110988# typedefs for a vector of Vector3Ds
typedef ::std::vector< Vector3D > Vector3DVector;

#endif // _SVX_VECTOR3D_HXX
