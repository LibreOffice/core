/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VPolarTransformation.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:27:15 $
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
