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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "VPolarTransformation.hxx"
#include "ViewDefines.hxx"
#include "CommonConverters.hxx"
#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;

namespace chart
{


VPolarTransformation::VPolarTransformation( const PolarPlottingPositionHelper& rPositionHelper )
        : m_aPositionHelper(rPositionHelper)
        , m_aUnitCartesianToScene( rPositionHelper.getUnitCartesianToScene() )
{
}

VPolarTransformation::~VPolarTransformation()
{
}

// ____ XTransformation ____
Sequence< double > SAL_CALL VPolarTransformation::transform(
                        const Sequence< double >& rSourceValues )
    throw (RuntimeException,
           lang::IllegalArgumentException)
{
    double fScaledLogicAngle  = rSourceValues[0];
    double fScaledLogicRadius = rSourceValues[1];

    if( m_aPositionHelper.isSwapXAndY() )
        std::swap(fScaledLogicAngle,fScaledLogicRadius);

    double fAngleDegree = m_aPositionHelper.transformToAngleDegree( fScaledLogicAngle, false );
    double fAnglePi     = fAngleDegree*F_PI/180.0;
    double fRadius      = m_aPositionHelper.transformToRadius( fScaledLogicRadius, false);

    double fX=fRadius*cos(fAnglePi);
    double fY=fRadius*sin(fAnglePi);
    double fZ=rSourceValues[2];

    //!! applying matrix to vector does ignore translation, so it is important to use a B3DPoint here instead of B3DVector
    ::basegfx::B3DPoint aPoint(fX,fY,fZ);
    ::basegfx::B3DPoint aRet = m_aUnitCartesianToScene * aPoint;
    return B3DPointToSequence(aRet);
}

sal_Int32 SAL_CALL VPolarTransformation::getSourceDimension()
    throw (RuntimeException)
{
    return 3;
}

sal_Int32 SAL_CALL VPolarTransformation::getTargetDimension()
    throw (RuntimeException)
{
    return 3;
}


}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
