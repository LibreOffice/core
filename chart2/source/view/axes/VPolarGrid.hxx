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
#ifndef _CHART2_VPOLARGRID_HXX
#define _CHART2_VPOLARGRID_HXX

#include "VAxisOrGridBase.hxx"
#include "Tickmarks.hxx"
#include "VLineProperties.hxx"
#include <com/sun/star/drawing/PointSequenceSequence.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class PolarPlottingPositionHelper;

class VPolarGrid : public VAxisOrGridBase
{
//-------------------------------------------------------------------------
// public methods
//-------------------------------------------------------------------------
public:
    VPolarGrid( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
        , const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > > & rGridPropertiesList //main grid, subgrid, subsubgrid etc
        );
    virtual ~VPolarGrid();

    virtual void createShapes();

    void setIncrements( const std::vector< ExplicitIncrementData >& rIncrements );

    static void createLinePointSequence_ForAngleAxis(
                    ::com::sun::star::drawing::PointSequenceSequence& rPoints
                    , ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos
                    , const ExplicitIncrementData& rIncrement
                    , const ExplicitScaleData& rScale
                    , PolarPlottingPositionHelper* pPosHelper
                    , double fLogicRadius, double fLogicZ );

private: //member
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > > m_aGridPropertiesList;//main grid, subgrid, subsubgrid etc
    PolarPlottingPositionHelper* m_pPosHelper;
    ::std::vector< ExplicitIncrementData >   m_aIncrements;

    void    getAllTickInfos( sal_Int32 nDimensionIndex, ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;

    void    create2DRadiusGrid( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xLogicTarget
                    , ::std::vector< ::std::vector< TickInfo > >& rRadiusTickInfos
                    , ::std::vector< ::std::vector< TickInfo > >& rAngleTickInfos
                    , const ::std::vector<VLineProperties>& rLinePropertiesList );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
