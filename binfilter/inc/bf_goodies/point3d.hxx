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

#ifndef _B2D_POINT3D_HXX
#define _B2D_POINT3D_HXX

#include <tools/stream.hxx>

#include <tools/gen.hxx>

#include <tools/vector2d.hxx>

/*************************************************************************
|*
|* homogener 3D-Punkt (x,y,w)
|*
\************************************************************************/

namespace binfilter {
class Point3D
{
protected:
    // 3 Dimensinen, X,Y und W
    double						V[3];

public:
    Point3D(double fX = 0.0, double fY = 0.0, double fW = 1.0)
        { V[0] = fX; V[1] = fY; V[2] = fW; }

    const double& X() const { return V[0]; }
    const double& Y() const { return V[1]; }
    const double& W() const { return V[2]; }

    double& X() { return V[0]; }
    double& Y() { return V[1]; }
    double& W() { return V[2]; }

    const double& operator[] (int nPos) const { return V[nPos]; }
    double& operator[] (int nPos) { return V[nPos]; }

    // Rechnen untereinander
    Point3D&	operator+=	(const Point3D&);
    Point3D&	operator-=	(const Point3D&);
    Point3D		operator+ 	(const Point3D&) const;
    Point3D		operator-	(const Point3D&) const;
    Point3D		operator-	(void) const;
    Point3D&	operator*=	(const Point3D&);
    Point3D&	operator/=	(const Point3D&);
    Point3D		operator* 	(const Point3D&) const;
    Point3D		operator/	(const Point3D&) const;

    // Verschiebung um einen Vektor
    Point3D&	operator+=	(const Vector2D&);
    Point3D&	operator-=	(const Vector2D&);
    Point3D		operator+ 	(const Vector2D&) const;
    Point3D		operator-	(const Vector2D&) const;

    // Multiplikation, Division um einen Faktor
    Point3D&	operator*=	(double);
    Point3D		operator*	(double) const;
    Point3D&	operator/=	(double);
    Point3D		operator/	(double) const;

    BOOL		operator==	(const Point3D&) const;
    BOOL		operator!=	(const Point3D&) const;

    friend SvStream& operator>>(SvStream& rIStream, Point3D&);
    friend SvStream& operator<<(SvStream& rOStream, const Point3D&);
};
}//end of namespace binfilter


#endif          // _B2D_POINT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
