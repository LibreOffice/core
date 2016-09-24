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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_VDATASERIES_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_VDATASERIES_HXX

#include "PropertyMapper.hxx"

#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <cppuhelper/weakref.hxx>

#include <memory>
#include <vector>
#include <map>

namespace chart
{

class VDataSequence
{
public:
    void init( const css::uno::Reference<css::chart2::data::XDataSequence>& xModel );
    bool is() const;
    void clear();
    double getValue( sal_Int32 index ) const;
    sal_Int32 detectNumberFormatKey( sal_Int32 index ) const;
    sal_Int32 getLength() const;

    css::uno::Reference<css::chart2::data::XDataSequence> Model;

    mutable css::uno::Sequence<double> Doubles;
};

class VDataSeries final
{
public:
    VDataSeries( const css::uno::Reference<css::chart2::XDataSeries>& xDataSeries );

    ~VDataSeries();

    VDataSeries(const VDataSeries&) = delete;
    const VDataSeries& operator=(const VDataSeries&) = delete;

    const css::uno::Reference<css::chart2::XDataSeries>& getModel() const;

    void setCategoryXAxis();
    void setXValues( const css::uno::Reference<css::chart2::data::XDataSequence>& xValues );
    void setXValuesIfNone( const css::uno::Reference<css::chart2::data::XDataSequence>& xValues );
    void setParticle( const OUString& rSeriesParticle );
    void setGlobalSeriesIndex( sal_Int32 nGlobalSeriesIndex );
    void setPageReferenceSize( const css::awt::Size & rPageRefSize );

    sal_Int32   getTotalPointCount() const { return m_nPointCount;}
    double      getXValue( sal_Int32 index ) const;
    double      getYValue( sal_Int32 index ) const;

    void        getMinMaxXValue( double& fMin, double& fMax ) const;

    double      getY_Min( sal_Int32 index ) const;
    double      getY_Max( sal_Int32 index ) const;
    double      getY_First( sal_Int32 index ) const;
    double      getY_Last( sal_Int32 index ) const;

    double      getBubble_Size( sal_Int32 index ) const;

    double      getMinimumofAllDifferentYValues( sal_Int32 index ) const;
    double      getMaximumofAllDifferentYValues( sal_Int32 index ) const;

    double      getValueByProperty( sal_Int32 index, const OUString& rPropName ) const;

    bool        hasPropertyMapping( const OUString& rPropName ) const;

    css::uno::Sequence< double > const & getAllX() const;
    css::uno::Sequence< double > const & getAllY() const;

    double getXMeanValue() const;
    double getYMeanValue() const;

    bool        hasExplicitNumberFormat( sal_Int32 nPointIndex, bool bForPercentage ) const;
    sal_Int32   getExplicitNumberFormat( sal_Int32 nPointIndex, bool bForPercentage ) const;
    sal_Int32   detectNumberFormatKey( sal_Int32 nPointIndex ) const;
    bool        shouldLabelNumberFormatKeyBeDetectedFromYAxis() const;

    sal_Int32 getLabelPlacement(
        sal_Int32 nPointIndex, const css::uno::Reference<css::chart2::XChartType>& xChartType,
        sal_Int32 nDimensionCount, bool bSwapXAndY ) const;

    css::uno::Reference<css::beans::XPropertySet> getPropertiesOfPoint( sal_Int32 index ) const;

    css::uno::Reference<css::beans::XPropertySet> getPropertiesOfSeries() const;

    css::chart2::Symbol* getSymbolProperties( sal_Int32 index ) const;

    css::uno::Reference<css::beans::XPropertySet> getXErrorBarProperties( sal_Int32 index ) const;

    css::uno::Reference<css::beans::XPropertySet> getYErrorBarProperties( sal_Int32 index ) const;

    bool hasPointOwnColor( sal_Int32 index ) const;

    css::chart2::StackingDirection getStackingDirection() const;
    sal_Int32 getAttachedAxisIndex() const;
    void setAttachedAxisIndex( sal_Int32 nAttachedAxisIndex );

    void doSortByXValues();

    void setConnectBars( bool bConnectBars );
    bool getConnectBars() const;

    void setGroupBarsPerAxis( bool bGroupBarsPerAxis );
    bool getGroupBarsPerAxis() const;

    void setStartingAngle( sal_Int32 nStartingAngle );
    sal_Int32 getStartingAngle() const;

    void setRoleOfSequenceForDataLabelNumberFormatDetection( const OUString& rRole );

    //this is only temporarily here for area chart:
    css::drawing::PolyPolygonShape3D       m_aPolyPolygonShape3D;
    sal_Int32   m_nPolygonIndex;
    double m_fLogicMinX;
    double m_fLogicMaxX;

    //this is here for deep stacking:
    double m_fLogicZPos;//from 0 to series count -1

    const OUString& getCID() const { return m_aCID;}
    const OUString& getSeriesParticle() const { return m_aSeriesParticle;}
    const OUString& getPointCID_Stub() const { return m_aPointCID_Stub;}
    OUString       getErrorBarsCID( bool bYError ) const;
    OUString       getLabelsCID() const;
    const OUString& getLabelCID_Stub() const { return m_aLabelCID_Stub;}
    OUString       getDataCurveCID( sal_Int32 nCurveIndex, bool bAverageLine ) const;

    css::chart2::DataPointLabel* getDataPointLabelIfLabel( sal_Int32 index ) const;
    bool    getTextLabelMultiPropertyLists( sal_Int32 index, tNameSequence*& pPropNames, tAnySequence*& pPropValues ) const;

    OUString       getDataCurveEquationCID( sal_Int32 nCurveIndex ) const;
    bool    isAttributedDataPoint( sal_Int32 index ) const;

    bool    isVaryColorsByPoint() const;

    void releaseShapes();

    void setMissingValueTreatment( sal_Int32 nMissingValueTreatment );
    sal_Int32 getMissingValueTreatment() const;

    void setOldTimeBased( VDataSeries* pOldSeries, double nPercent );
    VDataSeries* createCopyForTimeBased() const;

private: //methods
    css::chart2::DataPointLabel* getDataPointLabel( sal_Int32 index ) const;
    void adaptPointCache( sal_Int32 nNewPointIndex ) const;

    // for copies for time based charting
    VDataSeries();

public: //member
    css::uno::Reference<css::drawing::XShapes> m_xGroupShape;
    css::uno::Reference<css::drawing::XShapes> m_xLabelsGroupShape;
    css::uno::Reference<css::drawing::XShapes> m_xErrorXBarsGroupShape;
    css::uno::Reference<css::drawing::XShapes> m_xErrorYBarsGroupShape;

    //the following group shapes will be created as children of m_xGroupShape on demand
    //they can be used to assure that some parts of a series shape are always in front of others (e.g. symbols in front of lines)
    css::uno::Reference<css::drawing::XShapes> m_xFrontSubGroupShape;
    css::uno::Reference<css::drawing::XShapes> m_xBackSubGroupShape;

private: //member
    css::uno::Reference<css::chart2::XDataSeries> m_xDataSeries;

    //all points given by the model data (here are not only the visible points meant)
    sal_Int32       m_nPointCount;

    VDataSequence   m_aValues_X;
    VDataSequence   m_aValues_Y;
    VDataSequence   m_aValues_Z;

    VDataSequence   m_aValues_Y_Min;
    VDataSequence   m_aValues_Y_Max;
    VDataSequence   m_aValues_Y_First;
    VDataSequence   m_aValues_Y_Last;

    VDataSequence   m_aValues_Bubble_Size;

    VDataSequence*  m_pValueSequenceForDataLabelNumberFormatDetection;

    std::map<OUString, VDataSequence> m_PropertyMap;

    mutable double m_fXMeanValue;
    mutable double m_fYMeanValue;

    css::uno::Sequence<sal_Int32>    m_aAttributedDataPointIndexList;

    css::chart2::StackingDirection     m_eStackingDirection;

    sal_Int32               m_nAxisIndex;//indicates whether this is attached to a main or secondary axis

    bool                m_bConnectBars;

    bool                m_bGroupBarsPerAxis;

    sal_Int32               m_nStartingAngle;

    OUString           m_aSeriesParticle;
    OUString           m_aCID;
    OUString           m_aPointCID_Stub;
    OUString           m_aLabelCID_Stub;

    sal_Int32               m_nGlobalSeriesIndex;

    //some cached values for data labels as they are very expensive
    mutable std::unique_ptr<css::chart2::DataPointLabel>
                                                    m_apLabel_Series;
    mutable std::unique_ptr<tNameSequence>        m_apLabelPropNames_Series;
    mutable std::unique_ptr<tAnySequence>         m_apLabelPropValues_Series;
    mutable std::unique_ptr<css::chart2::Symbol>  m_apSymbolProperties_Series;

    mutable std::unique_ptr<css::chart2::DataPointLabel>
                                                    m_apLabel_AttributedPoint;
    mutable std::unique_ptr<tNameSequence>        m_apLabelPropNames_AttributedPoint;
    mutable std::unique_ptr<tAnySequence>         m_apLabelPropValues_AttributedPoint;
    mutable std::unique_ptr<css::chart2::Symbol>  m_apSymbolProperties_AttributedPoint;
    mutable std::unique_ptr<css::chart2::Symbol>
                                                    m_apSymbolProperties_InvisibleSymbolForSelection;
    mutable sal_Int32                               m_nCurrentAttributedPoint;
    css::awt::Size                     m_aReferenceSize;

    sal_Int32   m_nMissingValueTreatment;
    bool        m_bAllowPercentValueInDataLabel;

    // for time based charting
    VDataSeries* mpOldSeries;
    double mnPercent;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
