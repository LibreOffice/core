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
#ifndef _CHART2_VPOLARANGLEAXIS_HXX
#define _CHART2_VPOLARANGLEAXIS_HXX

#include "VPolarAxis.hxx"
#include "Tickmarks_Equidistant.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class VPolarAngleAxis : public VPolarAxis
{
public:
    VPolarAngleAxis( const AxisProperties& rAxisProperties
           , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier
           , sal_Int32 nDimensionCount );
    virtual ~VPolarAngleAxis();

    virtual void createMaximumLabels();
    virtual void createLabels();
    virtual void updatePositions();

    virtual void createShapes();

private: //methods
    bool createTextShapes_ForAngleAxis(
                       const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                     , EquidistantTickIter& rTickIter
                     , AxisLabelProperties& rAxisLabelProperties
                     , double fLogicRadius, double fLogicZ );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
