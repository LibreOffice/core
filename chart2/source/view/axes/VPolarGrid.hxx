/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _CHART2_VPOLARGRID_HXX
#define _CHART2_VPOLARGRID_HXX

#include "VAxisOrGridBase.hxx"
#include "Tickmarks.hxx"
#include "VLineProperties.hxx"
#include <com/sun/star/drawing/PointSequenceSequence.hpp>

namespace chart
{

/**
*/
class PolarPlottingPositionHelper;

class VPolarGrid : public VAxisOrGridBase
{
// public methods
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

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
