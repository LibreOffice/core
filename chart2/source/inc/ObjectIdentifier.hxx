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

#include <sal/config.h>

#include <string_view>

#include "TitleHelper.hxx"

#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>

namespace chart { class ChartModel; }
namespace com::sun::star::awt { struct Point; }
namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::chart2 { class XAxis; }
namespace com::sun::star::drawing { class XShape; }
namespace com::sun::star::uno { class XInterface; }

namespace chart
{
class Axis;
class BaseCoordinateSystem;
class DataSeries;
class Diagram;
class Legend;

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
    OBJECTTYPE_DATA_TABLE,
    OBJECTTYPE_SHAPE,
    OBJECTTYPE_UNKNOWN
};

class ObjectIdentifier
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
    ObjectIdentifier( OUString aObjectCID );
    ObjectIdentifier( const css::uno::Reference< css::drawing::XShape >& rxShape );
    ObjectIdentifier( const css::uno::Any& rAny );

    bool operator==( const ObjectIdentifier& rOID ) const;
    bool operator<( const ObjectIdentifier& rOID ) const;

    static OUString createClassifiedIdentifierForObject(
          const css::uno::Reference< css::uno::XInterface >& xObject
        , const rtl::Reference<::chart::ChartModel>& xChartModel );
    static OUString createClassifiedIdentifierForObject(
          const rtl::Reference< ::chart::Title >& xObject
        , const rtl::Reference<::chart::ChartModel>& xChartModel );
    static OUString createClassifiedIdentifierForObject(
          const rtl::Reference< ::chart::Legend >& xObject
        , const rtl::Reference<::chart::ChartModel>& xChartModel );
    static OUString createClassifiedIdentifierForObject(
          const rtl::Reference< ::chart::Axis >& xObject
        , const rtl::Reference<::chart::ChartModel>& xChartModel );

    static OUString createClassifiedIdentifierForParticle(
        std::u16string_view rParticle );

    static OUString createClassifiedIdentifierForParticles(
            std::u16string_view rParentParticle
          , std::u16string_view rChildParticle
          , std::u16string_view rDragMethodServiceName = std::u16string_view()
          , std::u16string_view rDragParameterString = std::u16string_view() );

    static OUString createClassifiedIdentifierForGrid(
          const css::uno::Reference< css::chart2::XAxis >& xAxis
        , const rtl::Reference<::chart::ChartModel>& xChartModel
        , sal_Int32 nSubIndex = -1 );//-1: main grid, 0: first subgrid etc

    static OUString createParticleForDiagram();

    static OUString createParticleForCoordinateSystem(
          const rtl::Reference< ::chart::BaseCoordinateSystem >& xCooSys
        , const rtl::Reference<::chart::ChartModel>& xChartModel );

    static OUString createParticleForAxis(
                      sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

    static OUString createParticleForGrid(
                      sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

    static OUString createParticleForSeries( sal_Int32 nDiagramIndex, sal_Int32 nCooSysIndex
            , sal_Int32 nChartTypeIndex, sal_Int32 nSeriesIndex );

    static OUString createParticleForLegend(
        const rtl::Reference<::chart::ChartModel>& xChartModel );

    /** Creates an identifier for the data table */
    static OUString createParticleForDataTable(
        const rtl::Reference<::chart::ChartModel>& xChartModel );

    static OUString addChildParticle( std::u16string_view rParticle, std::u16string_view rChildParticle );
    static OUString createChildParticleWithIndex( ObjectType eObjectType, sal_Int32 nIndex );
    static sal_Int32 getIndexFromParticleOrCID( std::u16string_view rParticleOrCID );

    static OUString createClassifiedIdentifier(
        enum ObjectType eObjectType //e.g. OBJECTTYPE_DATA_SERIES
        , std::u16string_view rParticleID );//e.g. SeriesID

    static OUString createClassifiedIdentifierWithParent(
        enum ObjectType //e.g. OBJECTTYPE_DATA_POINT or OBJECTTYPE_GRID
        , std::u16string_view rParticleID //for points or subgrids this is an Index or otherwise an identifier from the model object
        , std::u16string_view rParentPartical //e.g. "Series=SeriesID" or "Grid=GridId"
        , std::u16string_view rDragMethodServiceName = std::u16string_view()
        , std::u16string_view rDragParameterString = std::u16string_view()
        );

    static bool isCID( std::u16string_view rName );
    static std::u16string_view getDragMethodServiceName( std::u16string_view rClassifiedIdentifier );
    static std::u16string_view getDragParameterString( std::u16string_view rCID );
    static bool isDragableObject( std::u16string_view rClassifiedIdentifier );
    bool isDragableObject() const;
    static bool isRotateableObject( std::u16string_view rClassifiedIdentifier );
    static bool isMultiClickObject( std::u16string_view rClassifiedIdentifier );
    static bool areSiblings( std::u16string_view rCID1, std::u16string_view rCID2 );//identical object is no sibling
    static bool areIdenticalObjects( std::u16string_view rCID1, std::u16string_view rCID2 );

    static OUString getStringForType( ObjectType eObjectType );
    static ObjectType getObjectType( std::u16string_view rCID );
    ObjectType getObjectType() const;

    static OUString createSeriesSubObjectStub( ObjectType eSubObjectType
                    , std::u16string_view rSeriesParticle
                    , std::u16string_view rDragMethodServiceName = std::u16string_view()
                    , std::u16string_view rDragParameterString = std::u16string_view() );
    static OUString createPointCID( std::u16string_view rPointCID_Stub, sal_Int32 nIndex  );

    static OUString createDataCurveCID( std::u16string_view rSeriesParticle, sal_Int32 nCurveIndex, bool bAverageLine );
    static OUString createDataCurveEquationCID( std::u16string_view rSeriesParticle, sal_Int32 nCurveIndex );

    static OUString getObjectID( std::u16string_view rCID );
    static std::u16string_view getParticleID( std::u16string_view rCID );
    static std::u16string_view getFullParentParticle( std::u16string_view rCID );

    //returns the series particle of a CID when the CID is a child of the series
    static OUString getSeriesParticleFromCID( std::u16string_view rCID );

    //return the model object that is indicated by rObjectCID
    static css::uno::Reference< css::beans::XPropertySet >
            getObjectPropertySet(
                  std::u16string_view rObjectCID
                , const rtl::Reference< ::chart::ChartModel >& xChartDocument );

    //return the axis object that belongs to rObjectCID if any
    static rtl::Reference< ::chart::Axis >
            getAxisForCID(
                  std::u16string_view rObjectCID
                , const rtl::Reference<::chart::ChartModel>& xChartModel );

    //return the series object that belongs to rObjectCID if any
    static rtl::Reference< ::chart::DataSeries >
            getDataSeriesForCID(
                  std::u16string_view rObjectCID
                , const rtl::Reference<::chart::ChartModel>& xChartModel );

    static rtl::Reference< ::chart::Diagram >
            getDiagramForCID(
                  std::u16string_view rObjectCID
                , const rtl::Reference<::chart::ChartModel>& xChartModel );

    static const OUString& getPieSegmentDragMethodServiceName();
    static OUString createPieSegmentDragParameterString(
          sal_Int32 nOffsetPercent
        , const css::awt::Point& rMinimumPosition
        , const css::awt::Point& rMaximumPosition );
    static bool parsePieSegmentDragParameterString( std::u16string_view rDragParameterString
        , sal_Int32& rOffsetPercent
        , css::awt::Point& rMinimumPosition
        , css::awt::Point& rMaximumPosition );

    static TitleHelper::eTitleType getTitleTypeForCID( std::u16string_view rCID );

    static OUString getMovedSeriesCID( std::u16string_view rObjectCID, bool bForward );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
