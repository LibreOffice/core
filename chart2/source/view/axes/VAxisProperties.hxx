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
#ifndef _CHART2_VAXIS_PROPERTIES_HXX
#define _CHART2_VAXIS_PROPERTIES_HXX

#include "TickmarkProperties.hxx"
#include "PlottingPositionHelper.hxx"
#include "LabelAlignment.hxx"
#include "ExplicitCategoriesProvider.hxx"

#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <vector>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

//These properties describe how a couple of labels are arranged one to another.
//The couple can contain all labels for all tickmark depth or just the labels for one single depth or
//the labels from an coherent range of tick depths (e.g. the major and first minor tickmarks should be handled together).
//... only allow side by side for different tick depth
enum AxisLabelStaggering
{
      SIDE_BY_SIDE
    , STAGGER_EVEN
    , STAGGER_ODD
    , STAGGER_AUTO
};

struct AxisLabelProperties
{
    AxisLabelProperties();

    ::com::sun::star::awt::Size         m_aFontReferenceSize;//reference size to calculate the font height
    ::com::sun::star::awt::Rectangle    m_aMaximumSpaceForLabels;//Labels need to be clipped in order to fit into this rectangle

    sal_Int32            nNumberFormatKey;

    AxisLabelStaggering  eStaggering;

    bool                 bLineBreakAllowed;
    bool                 bOverlapAllowed;

    bool                 bStackCharacters;
    double               fRotationAngleDegree;

    sal_Int32   nRhythm; //show only each nth label with n==nRhythm
    bool        bRhythmIsFix; //states whether the given rhythm is fix or may be changed

    //methods:
    void init( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XAxis >&  xAxisModel );

    bool getIsStaggered() const;
};

struct AxisProperties
{
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > m_xAxisModel;

    sal_Int32   m_nDimensionIndex;
    bool        m_bIsMainAxis;//not secondary axis
    bool        m_bSwapXAndY;

    ::com::sun::star::chart::ChartAxisPosition      m_eCrossoverType;
    ::com::sun::star::chart::ChartAxisLabelPosition m_eLabelPos;
    ::com::sun::star::chart::ChartAxisMarkPosition  m_eTickmarkPos;

    double*     m_pfMainLinePositionAtOtherAxis;
    double*     m_pfExrtaLinePositionAtOtherAxis;

    bool        m_bCrossingAxisHasReverseDirection;
    bool        m_bCrossingAxisIsCategoryAxes;

    //this direction is used to indicate in which direction the labels are to be drawn
    double          m_fLabelDirectionSign;
    //this direction is used to indicate in which direction inner tickmarks are to be drawn
    double          m_fInnerDirectionSign;
    LabelAlignment  m_aLabelAlignment;
    bool            m_bDisplayLabels;

    sal_Int32       m_nNumberFormatKey;

    /*
    0: no tickmarks         1: inner tickmarks
    2: outer tickmarks      3: inner and outer tickmarks
    */
    sal_Int32                           m_nMajorTickmarks;
    sal_Int32                           m_nMinorTickmarks;
    ::std::vector<TickmarkProperties>   m_aTickmarkPropertiesList;

    VLineProperties                     m_aLineProperties;

    //for category axes ->
    sal_Int32                           m_nAxisType;//REALNUMBER, CATEGORY etc. type ::com::sun::star::chart2::AxisType
    bool                                m_bComplexCategories;
    ExplicitCategoriesProvider* m_pExplicitCategoriesProvider;/*no ownership here*/
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XTextualDataSequence >
                                                    m_xAxisTextProvider; //for categries or series names
    //<- category axes

    //methods:

    AxisProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxisModel
                  , ExplicitCategoriesProvider* pExplicitCategoriesProvider );
    AxisProperties( const AxisProperties& rAxisProperties );
    virtual ~AxisProperties();
    virtual void init(bool bCartesian=false);//init from model data (m_xAxisModel)

    void initAxisPositioning( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xAxisProp );

    static TickmarkProperties getBiggestTickmarkProperties();
    TickmarkProperties makeTickmarkPropertiesForComplexCategories( sal_Int32 nTickLength, sal_Int32 nTickStartDistanceToAxis, sal_Int32 nTextLevel ) const;

private:
    AxisProperties();

protected:
    virtual TickmarkProperties  makeTickmarkProperties( sal_Int32 nDepth ) const;
    VLineProperties      makeLinePropertiesForDepth( sal_Int32 nDepth ) const;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
