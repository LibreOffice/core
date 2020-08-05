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
#pragma once

#include "VAxisOrGridBase.hxx"
#include "VAxisProperties.hxx"
#include "Tickmarks.hxx"

namespace com::sun::star::util { class XNumberFormatsSupplier; }

namespace chart
{

class VAxisBase : public VAxisOrGridBase
{
public:
    VAxisBase( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
           , const AxisProperties& rAxisProperties
           , const css::uno::Reference< css::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );
    virtual ~VAxisBase() override;

    /**
     * Return the number of dimensions the diagram has.  2 for x and y, and 3
     * for x, y, and z.
     */
    sal_Int32 getDimensionCount() const;

    virtual void createMaximumLabels()=0;
    virtual void createLabels()=0;
    virtual void updatePositions()=0;

    virtual bool isAnythingToDraw();
    virtual void initAxisLabelProperties(
                    const css::awt::Size& rFontReferenceSize
                  , const css::awt::Rectangle& rMaximumSpaceForLabels );

    virtual void setExplicitScaleAndIncrement(
            const ExplicitScaleData& rScale
          , const ExplicitIncrementData& rIncrement ) override;

    virtual sal_Int32 estimateMaximumAutoMainIncrementCount();
    virtual void createAllTickInfos( TickInfoArraysType& rAllTickInfos );

    void setExtraLinePositionAtOtherAxis( double fCrossingAt );

protected: //methods
    static size_t getIndexOfLongestLabel( const css::uno::Sequence<OUString>& rLabels );
    void removeTextShapesFromTicks();
    void updateUnscaledValuesAtTicks( TickIter& rIter );

    virtual bool prepareShapeCreation();
    void recordMaximumTextSize( const css::uno::Reference< css::drawing::XShape >& xShape
                    , double fRotationAngleDegree );

    bool isDateAxis() const;
    bool isComplexCategoryAxis() const;

protected: //member
    css::uno::Reference< css::util::XNumberFormatsSupplier >  m_xNumberFormatsSupplier;
    AxisProperties                                            m_aAxisProperties;
    AxisLabelProperties                                       m_aAxisLabelProperties;
    css::uno::Sequence< OUString >                            m_aTextLabels;
    bool                                                      m_bUseTextLabels;

    css::uno::Reference< css::drawing::XShapes > m_xGroupShape_Shapes;
    css::uno::Reference< css::drawing::XShapes > m_xTextTarget;

    /**
     * This typically consists of 2 TickInfo vectors (i.e. the outer vector
     * has 2 child vector elements) for normal axis.  The first vector
     * corresponds with the major ticks while the second corresponds with the
     * minor ticks.
     *
     * It may have more than 2 TickInfo vectors for complex category axis
     * which has multi-level axis labels.
     */
    TickInfoArraysType m_aAllTickInfos;
    bool m_bReCreateAllTickInfos;

    bool m_bRecordMaximumTextSize;
    sal_Int32 m_nMaximumTextWidthSoFar;
    sal_Int32 m_nMaximumTextHeightSoFar;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
