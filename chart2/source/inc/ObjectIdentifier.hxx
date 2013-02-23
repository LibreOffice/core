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
#ifndef _CHART2_OBJECTIDENTIFIER_HXX
#define _CHART2_OBJECTIDENTIFIER_HXX

#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include "TitleHelper.hxx"
#include "charttoolsdllapi.hxx"

// header for class OUString
#include <rtl/ustring.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/XShape.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
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
    //                            also the keywors used in the ObjectIdentifiers are not allowed

    //where ObjectID: Parent-Particle:Particle //e.g. Series=2:Point=22
    //where Particle: Type=ParticleID //e.g. Point=22
    //where Type: getStringForType( ObjectType eType ) or other string

public:
    ObjectIdentifier();
    ObjectIdentifier( const ::rtl::OUString& rObjectCID );
    ObjectIdentifier( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape );
    ObjectIdentifier( const ::com::sun::star::uno::Any& rAny );
    virtual ~ObjectIdentifier();

    ObjectIdentifier( const ObjectIdentifier& rOID );
    ObjectIdentifier& operator=( const ObjectIdentifier& rOID );
    bool operator==( const ObjectIdentifier& rOID ) const;
    bool operator!=( const ObjectIdentifier& rOID ) const;
    bool operator<( const ObjectIdentifier& rOID ) const;

    static rtl::OUString createClassifiedIdentifierForObject(
          const ::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XInterface >& xObject
        , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xChartModel );

    static rtl::OUString createClassifiedIdentifierForParticle(
        const rtl::OUString& rParticle );

    static rtl::OUString createClassifiedIdentifierForParticles(
            const rtl::OUString& rParentParticle
          , const rtl::OUString& rChildParticle
          , const rtl::OUString& rDragMethodServiceName = rtl::OUString()
          , const rtl::OUString& rDragParameterString = rtl::OUString() );

    static rtl::OUString createClassifiedIdentifierForGrid(
          const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XAxis >& xAxis
        , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xChartModel
        , sal_Int32 nSubIndex = -1 );//-1: main grid, 0: first subgrid etc

    SAL_DLLPRIVATE static rtl::OUString createParticleForDiagram(
          const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram >& xDiagram
        , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xChartModel );


    static rtl::OUString createParticleForCoordinateSystem(
          const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XCoordinateSystem >& xCooSys
        , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xChartModel );

    static rtl::OUString createParticleForAxis(
                      sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

    static rtl::OUString createParticleForGrid(
                      sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

    static rtl::OUString createParticleForSeries( sal_Int32 nDiagramIndex, sal_Int32 nCooSysIndex
            , sal_Int32 nChartTypeIndex, sal_Int32 nSeriesIndex );

    static rtl::OUString createParticleForLegend(
          const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XLegend >& xLegend
        , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xChartModel );

    static rtl::OUString addChildParticle( const rtl::OUString& rParticle, const rtl::OUString& rChildParticle );
    static rtl::OUString createChildParticleWithIndex( ObjectType eObjectType, sal_Int32 nIndex );
    static sal_Int32 getIndexFromParticleOrCID( const rtl::OUString& rParticleOrCID );

    static rtl::OUString createClassifiedIdentifier(
        enum ObjectType eObjectType //e.g. OBJECTTYPE_DATA_SERIES
        , const rtl::OUString& rParticleID );//e.g. SeriesID

    static rtl::OUString createClassifiedIdentifierWithParent(
        enum ObjectType //e.g. OBJECTTYPE_DATA_POINT or OBJECTTYPE_GRID
        , const rtl::OUString& rParticleID //for points or subgrids this is an Index or otherwise an identifier from the model object
        , const rtl::OUString& rParentPartical //e.g. "Series=SeriesID" or "Grid=GridId"
        , const rtl::OUString& rDragMethodServiceName = rtl::OUString()
        , const rtl::OUString& rDragParameterString = rtl::OUString()
        );

    static bool isCID( const rtl::OUString& rName );
    static rtl::OUString getDragMethodServiceName( const rtl::OUString& rClassifiedIdentifier );
    static rtl::OUString getDragParameterString( const rtl::OUString& rCID );
    static bool isDragableObject( const rtl::OUString& rClassifiedIdentifier );
    bool isDragableObject();
    static bool isRotateableObject( const rtl::OUString& rClassifiedIdentifier );
    static bool isMultiClickObject( const rtl::OUString& rClassifiedIdentifier );
    static bool areSiblings( const rtl::OUString& rCID1, const rtl::OUString& rCID2 );//identical object is no sibling
    static bool areIdenticalObjects( const ::rtl::OUString& rCID1, const ::rtl::OUString& rCID2 );

    static rtl::OUString getStringForType( ObjectType eObjectType );
    static ObjectType    getObjectType( const rtl::OUString& rCID );
    ObjectType getObjectType();

    static rtl::OUString createSeriesSubObjectStub( ObjectType eSubObjectType
                    , const rtl::OUString& rSeriesParticle
                    , const rtl::OUString& rDragMethodServiceName = rtl::OUString()
                    , const rtl::OUString& rDragParameterString = rtl::OUString() );
    static rtl::OUString createPointCID( const rtl::OUString& rPointCID_Stub, sal_Int32 nIndex  );

    static rtl::OUString createDataCurveCID( const rtl::OUString& rSeriesParticle, sal_Int32 nCurveIndex, bool bAverageLine );
    static rtl::OUString createDataCurveEquationCID( const rtl::OUString& rSeriesParticle, sal_Int32 nCurveIndex );

    SAL_DLLPRIVATE static rtl::OUString getObjectID( const rtl::OUString& rCID );
    static rtl::OUString getParticleID( const rtl::OUString& rCID );
    static rtl::OUString getFullParentParticle( const rtl::OUString& rCID );

    //returns the series particle of a CID when the CID is a child of the series
    static rtl::OUString getSeriesParticleFromCID( const rtl::OUString& rCID );

    //return the model object that is indicated by rObjectCID
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
            getObjectPropertySet(
                  const rtl::OUString& rObjectCID
                , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xChartModel );
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
            getObjectPropertySet(
                  const rtl::OUString& rObjectCID
                , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XChartDocument >& xChartDocument );

    //return the axis object that belongs to rObjectCID if any
    static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >
            getAxisForCID(
                  const rtl::OUString& rObjectCID
                , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xChartModel );

    //return the series object that belongs to rObjectCID if any
    static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >
            getDataSeriesForCID(
                  const rtl::OUString& rObjectCID
                , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xChartModel );

    static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >
            getDiagramForCID(
                  const rtl::OUString& rObjectCID
                , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xChartModel );

    static const ::rtl::OUString& getPieSegmentDragMethodServiceName();
    static ::rtl::OUString createPieSegmentDragParameterString(
          sal_Int32 nOffsetPercent
        , const ::com::sun::star::awt::Point& rMinimumPosition
        , const ::com::sun::star::awt::Point& rMaximumPosition );
    static bool parsePieSegmentDragParameterString( const rtl::OUString& rDragParameterString
        , sal_Int32& rOffsetPercent
        , ::com::sun::star::awt::Point& rMinimumPosition
        , ::com::sun::star::awt::Point& rMaximumPosition );

    static TitleHelper::eTitleType getTitleTypeForCID( const ::rtl::OUString& rCID );

    static ::rtl::OUString getMovedSeriesCID( const ::rtl::OUString& rObjectCID, sal_Bool bForward );

    bool isValid() const;
    bool isAutoGeneratedObject() const;
    bool isAdditionalShape() const;
    ::rtl::OUString getObjectCID() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > getAdditionalShape() const;
    ::com::sun::star::uno::Any getAny() const;

private:
    // #i12587# support for shapes in chart
    // For autogenerated chart objects a CID is specified in m_aObjectCID,
    // for all other objects m_xAdditionalShape is set.
    // Note, that if m_aObjectCID is set, m_xAdditionalShape must be empty
    // and vice versa.
    ::rtl::OUString m_aObjectCID;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > m_xAdditionalShape;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
