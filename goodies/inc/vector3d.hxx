/*************************************************************************
 *
 *  $RCSfile: vector3d.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#endif // _SVX_VECTOR3D_HXX
