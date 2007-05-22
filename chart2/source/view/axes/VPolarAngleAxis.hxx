/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VPolarAngleAxis.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:12:29 $
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
#ifndef _CHART2_VPOLARANGLEAXIS_HXX
#define _CHART2_VPOLARANGLEAXIS_HXX

#include "VPolarAxis.hxx"

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

    virtual void SAL_CALL createMaximumLabels();
    virtual void SAL_CALL createLabels();
    virtual void SAL_CALL updatePositions();

    virtual void SAL_CALL createShapes();

private: //methods
    bool createTextShapes_ForAngleAxis(
                       const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                     , TickIter& rTickIter
                     , AxisLabelProperties& rAxisLabelProperties
                     , double fLogicRadius, double fLogicZ );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
