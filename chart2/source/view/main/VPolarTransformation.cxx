/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
