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

#ifndef _SVX_DEF3D_HXX
#define _SVX_DEF3D_HXX

class Vector3D;

#ifndef _INC_MATH
#include <math.h>
#endif

#include <tools/solar.h>
namespace binfilter {

const double fPiDiv180 = 0.01745329251994;
const double EPSILON = 1e-06;

#define DEG2RAD(fAngle) (fPiDiv180 * (fAngle))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ 3D-Hilfsfunktionen +++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FASTBOOL Do3DEdgesIntersect(const Vector3D& rV1a, const Vector3D& rV1b,
                            const Vector3D& rV2a, const Vector3D& rV2b);

enum E3dDragDetail { E3DDETAIL_DEFAULT = 0,
                     E3DDETAIL_ONEBOX,
                     E3DDETAIL_ALLBOXES,
                     E3DDETAIL_ALLLINES
};

enum E3dDragConstraint { E3DDRAG_CONSTR_X	= 0x0001,
                         E3DDRAG_CONSTR_Y	= 0x0002,
                         E3DDRAG_CONSTR_Z	= 0x0004,
                         E3DDRAG_CONSTR_XY	= 0x0003,
                         E3DDRAG_CONSTR_XZ	= 0x0005,
                         E3DDRAG_CONSTR_YZ	= 0x0006,
                         E3DDRAG_CONSTR_XYZ	= 0x0007
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
