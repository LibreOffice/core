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
#ifndef INCLUDED_CHART2_SOURCE_INC_OBJECTIDENTIFIER_HXX
#define INCLUDED_CHART2_SOURCE_INC_OBJECTIDENTIFIER_HXX

#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include "TitleHelper.hxx"
#include "charttoolsdllapi.hxx"
#include <ChartModel.hxx>

#include <rtl/ustring.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/XShape.hpp>

namespace chart
{

enum ObjectType
{
    OBJECTTYPE_PAGE,
    OBJECTTYPE_TITLE,
    OBJECTTYPE_LEGEND,
    OBJECTTYPE_LEGEND_ENTRY,
    OBJECTTYPE_DIAGRAM,
    OBJECTTYPE_DIAGRAM_WALL,
    OBJECTTYPE_DIAGRAM_FLOOR,
    OBJECTTYPE_AXIS,
    OBJECTTYPE_AXIS_UNITLABEL,
    OBJECTTYPE_GRID,
    OBJECTTYPE_SUBGRID,
    OBJECTTYPE_DATA_SERIES,
    OBJECTTYPE_DATA_POINT,
    OBJECTTYPE_DATA_LABELS,
    OBJECTTYPE_DATA_LABEL,
    OBJECTTYPE_DATA_ERRORS_X,
    OBJECTTYPE_DATA_ERRORS_Y,
    OBJECTTYPE_DATA_ERRORS_Z,
    OBJECTTYPE_DATA_CURVE,//e.g. a statistical method printed as line
    OBJECTTYPE_DATA_AVERAGE_LINE,
    OBJECTTYPE_DATA_CURVE_EQUATION,
    OBJECTTYPE_DATA_STOCK_RANGE,
    OBJECTTYPE_DATA_STOCK_LOSS,
    OBJECTTYPE_DATA_STOCK_GAIN,
    OBJECTTYPE_SHAPE,
    OBJECTTYPE_UNKNOWN
};

class OOO_DLLPUBLIC_CHARTTOOLS ObjectIdentifier
{
    //CID == ClassifiedIdentifier <--> name of shape
    //semicolon, colon, equal sign and slash have special meanings in a CID
    //and are therefore not allowed in its components

    //syntax of a CID:  CID:/classification/ObjectID

    //where classification: nothing or "MultiClick" or "DragMethod=DragMethodServiceName" and "DragParameter=DragParameterString"
    //                      or a combination of these separated with a colon
    //where DragMethodServiceName can be a selfdefined servicename for special actions //todo define standard service for this purpose
    //where DragParameterString is any string you like to transport information to your special drag service
    //                            only semicolon, colon, equal sign and slash are not allowed characters
    //                            also the keywords used in the ObjectIdentifiers are not allowed

    //where ObjectID: Parent-Particle:Particle //e.g. Series=2:Point=22
    //where Particle: Type=ParticleID //e.g. Point=22
    //where Type: getStringForType( ObjectType eType ) or other string

public:
    ObjectIdentifier();
    ObjectIdentifier( const OUString& rObjectCID );
    ObjectIdentifier( const css::uno::Reference< css::drawing::XShape >& rxShape );
    ObjectIdentifier( const css::uno::Any& rAny );

    bool operator==( const ObjectIdentifier& rOID ) const;
    bool operator!=( const ObjectIdentifier& rOID ) const;
    bool operator<( const ObjectIdentifier& rOID ) const;

    static OUString createClassifiedIdentifierForObject(
          const css::uno::Reference< css::uno::XInterface >& xObject
        , ChartModel& rModel);

    static OUString createClassifiedIdentifierForObject(
          const css::uno::Reference< css::uno::XInterface >& xObject
        , const css::uno::Reference< css::frame::XModel >& xChartModel );

    static OUString createClassifiedIdentifierForParticle(
        const OUString& rParticle );

    static OUString createClassifiedIdentifierForParticles(
            const OUString& rParentParticle
          , const OUString& rChildParticle
          , const OUString& rDragMethodServiceName = OUString()
          , const OUString& rDragParameterString = OUString() );

    static OUString createClassifiedIdentifierForGrid(
          const css::uno::Reference< css::chart2::XAxis >& xAxis
        , const css::uno::Reference< css::frame::XModel >& xChartModel
        , sal_Int32 nSubIndex = -1 );//-1: main grid, 0: first subgrid etc

    SAL_DLLPRIVATE static OUString createParticleForDiagram();

    static OUString createParticleForCoordinateSystem(
          const css::uno::Reference< css::chart2::XCoordinateSystem >& xCooSys
        , ChartModel& rModel );

    static OUString createParticleForCoordinateSystem(
          const css::uno::Reference< css::chart2::XCoordinateSystem >& xCooSys
        , const css::uno::Reference< css::frame::XModel >& xChartModel );

    static OUString createParticleForAxis(
                      sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

    static OUString createParticleForGrid(
                      sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

    static OUString createParticleForSeries( sal_Int32 nDiagramIndex, sal_Int32 nCooSysIndex
            , sal_Int32 nChartTypeIndex, sal_Int32 nSeriesIndex );

    static OUString createParticleForLegend( ChartModel& rModel );

    static OUString createParticleForLegend(
        const css::uno::Reference< css::frame::XModel >& xChartModel );

    static OUString addChildParticle( const OUString& rParticle, const OUString& rChildParticle );
    static OUString createChildParticleWithIndex( ObjectType eObjectType, sal_Int32 nIndex );
    static sal_Int32 getIndexFromParticleOrCID( const OUString& rParticleOrCID );

    static OUString createClassifiedIdentifier(
        enum ObjectType eObjectType //e.g. OBJECTTYPE_DATA_SERIES
        , const OUString& rParticleID );//e.g. SeriesID

    static OUString createClassifiedIdentifierWithParent(
        enum ObjectType //e.g. OBJECTTYPE_DATA_POINT or OBJECTTYPE_GRID
        , const OUString& rParticleID //for points or subgrids this is an Index or otherwise an identifier from the model object
        , const OUString& rParentPartical //e.g. "Series=SeriesID" or "Grid=GridId"
        , const OUString& rDragMethodServiceName = OUString()
        , const OUString& rDragParameterString = OUString()
        );

    static bool isCID( const OUString& rName );
    static OUString getDragMethodServiceName( const OUString& rClassifiedIdentifier );
    static OUString getDragParameterString( const OUString& rCID );
    static bool isDragableObject( const OUString& rClassifiedIdentifier );
    bool isDragableObject();
    static bool isRotateableObject( const OUString& rClassifiedIdentifier );
    static bool isMultiClickObject( const OUString& rClassifiedIdentifier );
    static bool areSiblings( const OUString& rCID1, const OUString& rCID2 );//identical object is no sibling
    static bool areIdenticalObjects( const OUString& rCID1, const OUString& rCID2 );

    static OUString getStringForType( ObjectType eObjectType );
    static ObjectType    getObjectType( const OUString& rCID );
    ObjectType getObjectType();

    static OUString createSeriesSubObjectStub( ObjectType eSubObjectType
                    , const OUString& rSeriesParticle
                    , const OUString& rDragMethodServiceName = OUString()
                    , const OUString& rDragParameterString = OUString() );
    static OUString createPointCID( const OUString& rPointCID_Stub, sal_Int32 nIndex  );

    static OUString createDataCurveCID( const OUString& rSeriesParticle, sal_Int32 nCurveIndex, bool bAverageLine );
    static OUString createDataCurveEquationCID( const OUString& rSeriesParticle, sal_Int32 nCurveIndex );

    SAL_DLLPRIVATE static OUString getObjectID( const OUString& rCID );
    static OUString getParticleID( const OUString& rCID );
    static OUString getFullParentParticle( const OUString& rCID );

    //returns the series particle of a CID when the CID is a child of the series
    static OUString getSeriesParticleFromCID( const OUString& rCID );

    //return the model object that is indicated by rObjectCID
    static css::uno::Reference< css::beans::XPropertySet >
            getObjectPropertySet(
                  const OUString& rObjectCID
                , const css::uno::Reference< css::frame::XModel >& xChartModel );
    static css::uno::Reference< css::beans::XPropertySet >
            getObjectPropertySet(
                  const OUString& rObjectCID
                , const css::uno::Reference< css::chart2::XChartDocument >& xChartDocument );

    //return the axis object that belongs to rObjectCID if any
    static css::uno::Reference< css::chart2::XAxis >
            getAxisForCID(
                  const OUString& rObjectCID
                , const css::uno::Reference< css::frame::XModel >& xChartModel );

    //return the series object that belongs to rObjectCID if any
    static css::uno::Reference< css::chart2::XDataSeries >
            getDataSeriesForCID(
                  const OUString& rObjectCID
                , const css::uno::Reference< css::frame::XModel >& xChartModel );

    static css::uno::Reference< css::chart2::XDiagram >
            getDiagramForCID(
                  const OUString& rObjectCID
                , const css::uno::Reference< css::frame::XModel >& xChartModel );

    static const OUString& getPieSegmentDragMethodServiceName();
    static OUString createPieSegmentDragParameterString(
          sal_Int32 nOffsetPercent
        , const css::awt::Point& rMinimumPosition
        , const css::awt::Point& rMaximumPosition );
    static bool parsePieSegmentDragParameterString( const OUString& rDragParameterString
        , sal_Int32& rOffsetPercent
        , css::awt::Point& rMinimumPosition
        , css::awt::Point& rMaximumPosition );

    static TitleHelper::eTitleType getTitleTypeForCID( const OUString& rCID );

    static OUString getMovedSeriesCID( const OUString& rObjectCID, bool bForward );

    bool isValid() const;
    bool isAutoGeneratedObject() const;
    bool isAdditionalShape() const;
    const OUString& getObjectCID() const { return m_aObjectCID;}
    const css::uno::Reference< css::drawing::XShape >& getAdditionalShape() const { return m_xAdditionalShape;}
    css::uno::Any getAny() const;

private:
    // #i12587# support for shapes in chart
    // For autogenerated chart objects a CID is specified in m_aObjectCID,
    // for all other objects m_xAdditionalShape is set.
    // Note, that if m_aObjectCID is set, m_xAdditionalShape must be empty
    // and vice versa.
    OUString m_aObjectCID;
    css::uno::Reference< css::drawing::XShape > m_xAdditionalShape;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
