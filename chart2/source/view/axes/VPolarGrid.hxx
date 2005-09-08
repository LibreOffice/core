/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VPolarGrid.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:41:17 $
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
#ifndef _CHART2_VPOLARGRID_HXX
#define _CHART2_VPOLARGRID_HXX

#include "VMeterBase.hxx"
#include "TickmarkHelper.hxx"
#include "VLineProperties.hxx"

#ifndef _COM_SUN_STAR_CHART2_XGRID_HPP_
#include <com/sun/star/chart2/XGrid.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class PolarPlottingPositionHelper;

class VPolarGrid : public VMeterBase
{
//-------------------------------------------------------------------------
// public methods
//-------------------------------------------------------------------------
public:
    VPolarGrid( const ::com::sun::star::uno::Reference<
         ::com::sun::star::chart2::XGrid >& xGrid
         , sal_Int32 nDimensionCount );
    virtual ~VPolarGrid();

    virtual void SAL_CALL createShapes();

    void setIncrements( const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::chart2::ExplicitIncrementData >& rIncrements );

    static void createLinePointSequence_ForAngleAxis(
                    ::com::sun::star::drawing::PointSequenceSequence& rPoints
                    , ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos
                    , const ::com::sun::star::chart2::ExplicitIncrementData& rIncrement
                    , const ::com::sun::star::chart2::ExplicitScaleData& rScale
                    , PolarPlottingPositionHelper* pPosHelper
                    , double fLogicRadius, double fLogicZ );

private: //member
    PolarPlottingPositionHelper* m_pPosHelper;
    ::com::sun::star::uno::Sequence<
            ::com::sun::star::chart2::ExplicitIncrementData >   m_aIncrements;

    void    getAllTickInfos( sal_Int32 nDimensionIndex, ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;

    void    create2DRadiusGrid( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , ::std::vector< ::std::vector< TickInfo > >& rRadiusTickInfos
                    , ::std::vector< ::std::vector< TickInfo > >& rAngleTickInfos
                    , const ::std::vector<VLineProperties>& rLinePropertiesList );
    void    create2DAngleGrid( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , ::std::vector< ::std::vector< TickInfo > >& rRadiusTickInfos
                    , ::std::vector< ::std::vector< TickInfo > >& rAngleTickInfos
                    , const ::std::vector<VLineProperties>& rLinePropertiesList );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
