/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: def3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:44:12 $
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

#ifndef _SVX_DEF3D_HXX
#define _SVX_DEF3D_HXX

class Vector3D;

#ifndef _INC_MATH
#include <math.h>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

const double fPiDiv180 = 0.01745329251994;
const double EPSILON = 1e-06;

#define DEG2RAD(fAngle) (fPiDiv180 * (fAngle))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ 3D-Hilfsfunktionen +++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FASTBOOL Do3DEdgesIntersect(const Vector3D& rV1a, const Vector3D& rV1b,
                            const Vector3D& rV2a, const Vector3D& rV2b);

enum E3dDragConstraint { E3DDRAG_CONSTR_X   = 0x0001,
                         E3DDRAG_CONSTR_Y   = 0x0002,
                         E3DDRAG_CONSTR_Z   = 0x0004,
                         E3DDRAG_CONSTR_XY  = 0x0003,
                         E3DDRAG_CONSTR_XZ  = 0x0005,
                         E3DDRAG_CONSTR_YZ  = 0x0006,
                         E3DDRAG_CONSTR_XYZ = 0x0007
};

#endif
