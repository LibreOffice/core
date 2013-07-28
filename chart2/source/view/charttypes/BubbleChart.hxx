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
#ifndef _CHART2_BUBBLECHART_HXX
#define _CHART2_BUBBLECHART_HXX

#include "VSeriesPlotter.hxx"
#include <com/sun/star/drawing/Direction3D.hpp>

namespace chart
{

class BubbleChart : public VSeriesPlotter
{
    // public methods
public:
    BubbleChart( const ::com::sun::star::uno::Reference<
             ::com::sun::star::chart2::XChartType >& xChartTypeModel
             , sal_Int32 nDimensionCount );
    virtual ~BubbleChart();

    virtual void createShapes();

    virtual void addSeries( VDataSeries* pSeries, sal_Int32 zSlot = -1, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 );

    virtual ::com::sun::star::drawing::Direction3D  getPreferredDiagramAspectRatio() const;

    // MinimumAndMaximumSupplier
    virtual bool isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex );
    virtual bool isSeparateStackingForDifferentSigns( sal_Int32 nDimensionIndex );

    virtual LegendSymbolStyle getLegendSymbolStyle();

private: //methods
    //no default constructor
    BubbleChart();

    void    calculateMaximumLogicBubbleSize();
    void    calculateBubbleSizeScalingFactor();

    com::sun::star::drawing::Direction3D transformToScreenBubbleSize( double fLogicSize );

private: //member

    bool   m_bShowNegativeValues;//input parameter
    bool   m_bBubbleSizeAsArea;//input parameter
    double m_fBubbleSizeScaling;//input parameter

    double m_fMaxLogicBubbleSize;//calculated values
    double m_fBubbleSizeFactorToScreen;//calculated values
};
} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
