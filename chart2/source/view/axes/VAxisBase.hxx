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
#ifndef _CHART2_VAXISBASE_HXX
#define _CHART2_VAXISBASE_HXX

#include "VAxisOrGridBase.hxx"
#include "VAxisProperties.hxx"
#include "Tickmarks.hxx"
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

namespace chart
{

/**
*/

class VAxisBase : public VAxisOrGridBase
{
public:
    VAxisBase( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
           , const AxisProperties& rAxisProperties
           , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );
    virtual ~VAxisBase();

    sal_Int32 getDimensionCount();

    virtual void createMaximumLabels()=0;
    virtual void createLabels()=0;
    virtual void updatePositions()=0;

    virtual sal_Bool isAnythingToDraw();
    virtual void initAxisLabelProperties(
                    const ::com::sun::star::awt::Size& rFontReferenceSize
                  , const ::com::sun::star::awt::Rectangle& rMaximumSpaceForLabels );

    virtual void setExplicitScaleAndIncrement(
            const ExplicitScaleData& rScale
          , const ExplicitIncrementData& rIncrement )
                throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 estimateMaximumAutoMainIncrementCount();
    virtual void createAllTickInfos( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos );

    void setExrtaLinePositionAtOtherAxis( const double& fCrossingAt );

protected: //methods
    sal_Int32 getIndexOfLongestLabel( const ::com::sun::star::uno::Sequence< OUString >& rLabels );
    void removeTextShapesFromTicks();
    void updateUnscaledValuesAtTicks( TickIter& rIter );

    virtual bool prepareShapeCreation();
    void recordMaximumTextSize( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShape >& xShape
                    , double fRotationAngleDegree );

    bool isDateAxis() const;
    bool isComplexCategoryAxis() const;

protected: //member
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  m_xNumberFormatsSupplier;
    AxisProperties                                                                      m_aAxisProperties;
    AxisLabelProperties                                                                 m_aAxisLabelProperties;
    ::com::sun::star::uno::Sequence< OUString >                                    m_aTextLabels;
    bool                                                                                m_bUseTextLabels;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > m_xGroupShape_Shapes;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > m_xTextTarget;

    ::std::vector< ::std::vector< TickInfo > >                                          m_aAllTickInfos;
    bool m_bReCreateAllTickInfos;

    bool m_bRecordMaximumTextSize;
    sal_Int32 m_nMaximumTextWidthSoFar;
    sal_Int32 m_nMaximumTextHeightSoFar;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
