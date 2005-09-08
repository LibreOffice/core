/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gradtrns.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:21:16 $
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

#ifndef _GRADTRANS_HXX
#define _GRADTRANS_HXX

#include "xgrad.hxx"

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _VECTOR2D_HXX
#include <tools/vector2d.hxx>
#endif

class SdrObject;

class GradTransVector
{
public:
    Point                       aPos1;
    Point                       aPos2;
    Color                       aCol1;
    Color                       aCol2;
};

class GradTransGradient
{
public:
    XGradient                   aGradient;
};

class GradTransformer
{
public:
    GradTransformer() {}

    void GradToVec(GradTransGradient& rG, GradTransVector& rV,
        const SdrObject* pObj);
    void VecToGrad(GradTransVector& rV, GradTransGradient& rG,
        GradTransGradient& rGOld, const SdrObject* pObj, BOOL bMoveSingle, BOOL bMoveFirst);
};

#endif // _GRADTRANS_HXX

