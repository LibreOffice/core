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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "ObjectIdentifier.hxx"
#include "macros.hxx"
#include "TitleHelper.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "DiagramHelper.hxx"
#include "AxisIndexDefines.hxx"
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>

// header for define DBG_ASSERT
#include <tools/debug.hxx>
#include <comphelper/InlineContainer.hxx>

#include <rtl/ustrbuf.hxx>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using rtl::OUString;
using rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;

static OUString m_aMultiClick( C2U("MultiClick") );
static OUString m_aDragMethodEquals( C2U("DragMethod=") );
static OUString m_aDragParameterEquals( C2U("DragParameter=") );
static OUString m_aProtocol( C2U("CID/") );
static OUString m_aEmptyString;
static OUString m_aPieSegmentDragMethodServiceName( C2U("PieSegmentDraging") );

namespace
{

OUString lcl_createClassificationStringForType( ObjectType eObjectType
            , const OUString& rDragMethodServiceName
            , const OUString& rDragParameterString
            )
{
    OUStringBuffer aRet;
    switch( eObjectType )
    {
        //these object types are all selected only after their parents was selected before
        case OBJECTTYPE_LEGEND_ENTRY: //parent is intended to be OBJECTTYPE_LEGEND
        case OBJECTTYPE_DATA_POINT: //parent is intended to be OBJECTTYPE_DATA_SERIES
        case OBJECTTYPE_DATA_LABEL: //parent is intended to be OBJECTTYPE_DATA_LABELS
        case OBJECTTYPE_DATA_ERRORS_X: //parent is intended to be OBJECTTYPE_DATA_ERRORS
        case OBJECTTYPE_DATA_ERRORS_Y: //parent is intended to be OBJECTTYPE_DATA_ERRORS
        case OBJECTTYPE_DATA_ERRORS_Z: //parent is intended to be OBJECTTYPE_DATA_ERRORS
            aRet=m_aMultiClick;
        default:
            ;//empty string
    }
    if( rDragMethodServiceName.getLength() )
    {
        if( aRet.getLength() )
            aRet.appendAscii(":");
        aRet.append( m_aDragMethodEquals );
        aRet.append( rDragMethodServiceName );

        if( rDragParameterString.getLength() )
        {
            if( aRet.getLength() )
                aRet.appendAscii(":");
            aRet.append( m_aDragParameterEquals );
            aRet.append( rDragParameterString );
        }
    }
    return aRet.makeStringAndClear();
}

typedef ::comphelper::MakeMap< TitleHelper::eTitleType, OUString > tTitleMap;
const tTitleMap& lcl_getTitleMap()
{
    //maps the title type to the ParentParticle for that title
    static tTitleMap m_aTitleMap = tTitleMap
        ( TitleHelper::MAIN_TITLE, C2U("") )
        ( TitleHelper::SUB_TITLE, C2U("D=0") )
        ( TitleHelper::X_AXIS_TITLE, C2U("D=0:CS=0:Axis=0,0") )
        ( TitleHelper::Y_AXIS_TITLE, C2U("D=0:CS=0:Axis=1,0") )
        ( TitleHelper::Z_AXIS_TITLE, C2U("D=0:CS=0:Axis=2,0") )
        ( TitleHelper::SECONDARY_X_AXIS_TITLE, C2U("D=0:CS=0:Axis=0,1") )
        ( TitleHelper::SECONDARY_Y_AXIS_TITLE, C2U("D=0:CS=0:Axis=1,1") )
        ;
    return m_aTitleMap;
}

OUString lcl_getTitleParentParticle( TitleHelper::eTitleType aTitleType )
{
    OUString aRet;

    const tTitleMap& rMap = lcl_getTitleMap();
    tTitleMap::const_iterator aIt( rMap.find( aTitleType ) );
    if( aIt != rMap.end())
        aRet = (*aIt).second;

    return aRet;
}

Reference<XChartType> lcl_getFirstStockChartType( const Reference< frame::XModel >& xChartModel )
{
    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    if(!xDiagram.is())
        return 0;

    //iterate through all coordinate systems
    Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is())
        return 0;

    uno::Sequence< Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
    {
        //iterate through all chart types in the current coordinate system
        Reference< XChartTypeContainer > xChartTypeContainer( aCooSysList[nCS], uno::UNO_QUERY );
        if( !xChartTypeContainer.is() )
            continue;

        uno::Sequence< Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
        {
            Reference< XChartType > xChartType( aChartTypeList[nT] );
            if(!xChartType.is())
                continue;
            OUString aChartType = xChartType->getChartType();
            if( aChartType.equalsIgnoreAsciiCase(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
                return xChartType;
        }
    }
    return 0;
}

OUString lcl_getIndexStringAfterString( const OUString& rString, const OUString& rSearchString )
{
    OUStringBuffer aRet;

    sal_Int32 nIndexStart = rString.lastIndexOf( rSearchString );
    if( nIndexStart != -1 )
    {
        nIndexStart += rSearchString.getLength();
        sal_Int32 nIndexEnd = rString.getLength();
        sal_Int32 nNextColon = rString.indexOf( ':', nIndexStart );
        if( nNextColon != -1 )
            nIndexEnd = nNextColon;
        aRet = rString.copy(nIndexStart,nIndexEnd-nIndexStart);
    }

    return aRet.makeStringAndClear();
}

sal_Int32 lcl_StringToIndex( const OUString& rIndexString )
{
    sal_Int32 nRet = -1;
    if( rIndexString.getLength() )
    {
        nRet = rIndexString.toInt32();
        if( nRet < -1 )
            nRet = -1;
    }
    return nRet;
}

void lcl_parseCooSysIndices( sal_Int32& rnDiagram, sal_Int32& rnCooSys, const OUString& rString )
{
    rnDiagram = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, C2U("D=") ) );
    rnCooSys = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, C2U("CS=") ) );
}

void lcl_parseAxisIndices( sal_Int32& rnDimensionIndex, sal_Int32& rnAxisIndex, const OUString& rString )
{
    OUString aAxisIndexString = lcl_getIndexStringAfterString( rString, C2U(":Axis=") );
    sal_Int32 nCharacterIndex=0;
    rnDimensionIndex = lcl_StringToIndex( aAxisIndexString.getToken( 0, ',', nCharacterIndex ) );
    rnAxisIndex = lcl_StringToIndex( aAxisIndexString.getToken( 0, ',', nCharacterIndex ) );
}

void lcl_parseGridIndices( sal_Int32& rnSubGridIndex, const OUString& rString )
{
    rnSubGridIndex = -1;
    rnSubGridIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, C2U(":SubGrid=") ) );
}

void lcl_parseSeriesIndices( sal_Int32& rnChartTypeIndex, sal_Int32& rnSeriesIndex, sal_Int32& rnPointIndex, const OUString& rString )
{
    rnChartTypeIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, C2U("CT=") ) );
    rnSeriesIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, C2U("Series=") ) );
    rnPointIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, C2U("Point=") ) );
}

void lcl_getDiagramAndCooSys( const OUString& rObjectCID
                , const Reference< frame::XModel >& xChartModel
                , Reference< XDiagram >& xDiagram
                , Reference< XCoordinateSystem >& xCooSys )
{
    sal_Int32 nDiagramIndex = -1;
    sal_Int32 nCooSysIndex = -1;
    lcl_parseCooSysIndices( nDiagramIndex, nCooSysIndex, rObjectCID );
    xDiagram = ChartModelHelper::findDiagram( xChartModel );//todo use nDiagramIndex when more than one diagram is possible in future
    if( !xDiagram.is() )
        return;

    if( nCooSysIndex > -1 )
    {
        Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
        if( xCooSysContainer.is() )
        {
            uno::Sequence< Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
            if( nCooSysIndex < aCooSysList.getLength() )
                xCooSys = aCooSysList[nCooSysIndex];
        }
    }
}

} //anonymous namespace

ObjectIdentifier::ObjectIdentifier()
    :m_aObjectCID( OUString() )
    ,m_xAdditionalShape( 0 )
{
}

ObjectIdentifier::ObjectIdentifier( const OUString& rObjectCID )
    :m_aObjectCID( rObjectCID )
    ,m_xAdditionalShape( 0 )
{
}

ObjectIdentifier::ObjectIdentifier( const Reference< drawing::XShape >& rxShape )
    :m_aObjectCID( OUString() )
    ,m_xAdditionalShape( rxShape )
{
}

ObjectIdentifier::ObjectIdentifier( const Any& rAny )
    :m_aObjectCID( OUString() )
    ,m_xAdditionalShape( 0 )
{
    const uno::Type& rType = rAny.getValueType();
    if ( rType == ::getCppuType( static_cast< const OUString* >( 0 ) ) )
    {
        rAny >>= m_aObjectCID;
    }
    else if ( rType == ::getCppuType( static_cast< const Reference< drawing::XShape >* >( 0 ) ) )
    {
        rAny >>= m_xAdditionalShape;
    }
}

ObjectIdentifier::~ObjectIdentifier()
{
}

ObjectIdentifier::ObjectIdentifier( const ObjectIdentifier& rOID )
    :m_aObjectCID( rOID.m_aObjectCID )
    ,m_xAdditionalShape( rOID.m_xAdditionalShape )
{

}

ObjectIdentifier& ObjectIdentifier::operator=( const ObjectIdentifier& rOID )
{
    m_aObjectCID = rOID.m_aObjectCID;
    m_xAdditionalShape = rOID.m_xAdditionalShape;
    return *this;
}

bool ObjectIdentifier::operator==( const ObjectIdentifier& rOID ) const
{
    if ( areIdenticalObjects( m_aObjectCID, rOID.m_aObjectCID ) &&
         ( m_xAdditionalShape == rOID.m_xAdditionalShape ) )
    {
        return true;
    }
    return false;
}

bool ObjectIdentifier::operator!=( const ObjectIdentifier& rOID ) const
{
    return !operator==( rOID );
}

bool ObjectIdentifier::operator<( const ObjectIdentifier& rOID ) const
{
    bool bReturn = false;
    if ( m_aObjectCID.getLength() && rOID.m_aObjectCID.getLength() )
    {
        bReturn = ( m_aObjectCID.compareTo( rOID.m_aObjectCID ) < 0 );
    }
    else if ( m_aObjectCID.getLength() )
    {
        bReturn = true;
    }
    else if ( rOID.m_aObjectCID.getLength() )
    {
        bReturn = false;
    }
    else if ( m_xAdditionalShape.is() && rOID.m_xAdditionalShape.is() )
    {
        bReturn = ( m_xAdditionalShape < rOID.m_xAdditionalShape );
    }
    return bReturn;
}

OUString ObjectIdentifier::createClassifiedIdentifierForObject(
          const Reference< uno::XInterface >& xObject
        , const Reference< frame::XModel >& xChartModel )
{
    OUString aRet;

    enum ObjectType eObjectType = OBJECTTYPE_UNKNOWN;
    OUString aObjectID;
    OUString aParentParticle;
    OUString aDragMethodServiceName;
    OUString aDragParameterString;


    try
    {
        //title
        Reference< XTitle > xTitle( xObject, uno::UNO_QUERY );
        if( xTitle.is() )
        {
            TitleHelper::eTitleType aTitleType;
            if( TitleHelper::getTitleType( aTitleType, xTitle, xChartModel ) )
            {
                eObjectType = OBJECTTYPE_TITLE;
                aParentParticle = lcl_getTitleParentParticle( aTitleType );
                aRet = ObjectIdentifier::createClassifiedIdentifierWithParent(
                    eObjectType, aObjectID, aParentParticle, aDragMethodServiceName, aDragParameterString );
            }
            return aRet;

        }

        //axis
        Reference< XAxis > xAxis( xObject, uno::UNO_QUERY );
        if( xAxis.is() )
        {
            Reference< XCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemOfAxis( xAxis, ChartModelHelper::findDiagram( xChartModel ) ) );
            rtl::OUString aCooSysParticle( createParticleForCoordinateSystem( xCooSys, xChartModel ) );
            sal_Int32 nDimensionIndex=-1;
            sal_Int32 nAxisIndex=-1;
            AxisHelper::getIndicesForAxis( xAxis, xCooSys, nDimensionIndex, nAxisIndex );
            rtl::OUString aAxisParticle( createParticleForAxis( nDimensionIndex, nAxisIndex ) );
            return createClassifiedIdentifierForParticles( aCooSysParticle, aAxisParticle );
        }

        //legend
        Reference< XLegend > xLegend( xObject, uno::UNO_QUERY );
        if( xLegend.is() )
        {
            return createClassifiedIdentifierForParticle( createParticleForLegend( xLegend, xChartModel ) );
        }

        //diagram
        Reference< XDiagram > xDiagram( xObject, uno::UNO_QUERY );
        if( xDiagram.is() )
        {
            return createClassifiedIdentifierForParticle( createParticleForDiagram( xDiagram, xChartModel ) );
        }

        //todo
        //XDataSeries
        //CooSys
        //charttype
        //datapoint?
        //Gridproperties
    }
    catch( uno::Exception& ex)
    {
        ASSERT_EXCEPTION( ex );
    }

    if( eObjectType != OBJECTTYPE_UNKNOWN )
    {
        aRet = ObjectIdentifier::createClassifiedIdentifierWithParent(
            eObjectType, aObjectID, aParentParticle, aDragMethodServiceName, aDragParameterString );
    }
    else
    {
        DBG_ASSERT(false,"give object could not be identifed in createClassifiedIdentifierForObject");
    }

    return aRet;
}

OUString ObjectIdentifier::createClassifiedIdentifierForParticle(
        const OUString& rParticle )
{
    return ObjectIdentifier::createClassifiedIdentifierForParticles( rParticle, OUString() );
}

OUString ObjectIdentifier::createClassifiedIdentifierForParticles(
            const OUString& rParentParticle
          , const OUString& rChildParticle
          , const OUString& rDragMethodServiceName
          , const OUString& rDragParameterString )
{
    ObjectType eObjectType( ObjectIdentifier::getObjectType( rChildParticle ) );
    if( eObjectType == OBJECTTYPE_UNKNOWN )
        eObjectType = ObjectIdentifier::getObjectType( rParentParticle );

    OUStringBuffer aRet( m_aProtocol );
    aRet.append( lcl_createClassificationStringForType( eObjectType, rDragMethodServiceName, rDragParameterString ));
    if(aRet.getLength()>m_aProtocol.getLength())
        aRet.appendAscii("/");

    if(rParentParticle.getLength())
    {
        aRet.append(rParentParticle);
        if( rChildParticle.getLength() )
            aRet.appendAscii(":");
    }
    aRet.append(rChildParticle);

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createParticleForDiagram(
          const Reference< XDiagram >& /*xDiagram*/
        , const Reference< frame::XModel >& /*xChartModel*/ )
{
    static OUString aRet(C2U("D=0"));
    //todo: if more than one diagram is implemeted, add the correct diagram index here
    return aRet;
}

OUString ObjectIdentifier::createParticleForCoordinateSystem(
          const Reference< XCoordinateSystem >& xCooSys
        , const Reference< frame::XModel >& xChartModel )
{
    OUStringBuffer aRet;

    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( xCooSysContainer.is() )
    {
        sal_Int32 nCooSysIndex = 0;
        uno::Sequence< Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
        for( ; nCooSysIndex < aCooSysList.getLength(); ++nCooSysIndex )
        {
            Reference< XCoordinateSystem > xCurrentCooSys( aCooSysList[nCooSysIndex] );
            if( xCooSys == xCurrentCooSys )
            {
                aRet = ObjectIdentifier::createParticleForDiagram( xDiagram, xChartModel );
                aRet.appendAscii(":CS=");
                aRet.append( OUString::valueOf( nCooSysIndex ) );
                break;
            }
        }
    }

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createParticleForAxis(
            sal_Int32 nDimensionIndex
          , sal_Int32 nAxisIndex )
{
    OUStringBuffer aRet(C2U("Axis="));

    aRet.append( OUString::valueOf( nDimensionIndex ) );
    aRet.appendAscii(",");
    aRet.append( OUString::valueOf( nAxisIndex ) );

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createParticleForGrid(
            sal_Int32 nDimensionIndex
          , sal_Int32 nAxisIndex )
{
    OUStringBuffer aRet(C2U("Axis="));
    aRet.append( OUString::valueOf( nDimensionIndex ) );
    aRet.appendAscii(",");
    aRet.append( OUString::valueOf( nAxisIndex ) );
    aRet.append( C2U(":Grid=0") );

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createClassifiedIdentifierForGrid(
          const Reference< XAxis >& xAxis
        , const Reference< frame::XModel >& xChartModel
        , sal_Int32 nSubGridIndex )
{
    //-1: main grid, 0: first subgrid etc

    rtl::OUString aAxisCID( createClassifiedIdentifierForObject( xAxis, xChartModel ) );
    rtl::OUString aGridCID( addChildParticle( aAxisCID
        , createChildParticleWithIndex( OBJECTTYPE_GRID, 0 ) ) );
    if( nSubGridIndex >= 0 )
    {
        aGridCID = addChildParticle( aGridCID
            , createChildParticleWithIndex( OBJECTTYPE_SUBGRID, 0 ) );
    }
    return aGridCID;
}

OUString ObjectIdentifier::createParticleForSeries(
              sal_Int32 nDiagramIndex, sal_Int32 nCooSysIndex
            , sal_Int32 nChartTypeIndex, sal_Int32 nSeriesIndex )
{
    OUStringBuffer aRet;

    aRet.appendAscii("D=");
    aRet.append( OUString::valueOf( nDiagramIndex ) );
    aRet.appendAscii(":CS=");
    aRet.append( OUString::valueOf( nCooSysIndex ) );
    aRet.appendAscii(":CT=");
    aRet.append( OUString::valueOf( nChartTypeIndex ) );
    aRet.appendAscii(":");
    aRet.append(getStringForType( OBJECTTYPE_DATA_SERIES ));
    aRet.appendAscii("=");
    aRet.append( OUString::valueOf( nSeriesIndex ) );

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createParticleForLegend(
          const Reference< XLegend >& /*xLegend*/
        , const Reference< frame::XModel >& xChartModel )
{
    OUStringBuffer aRet;

    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    //todo: if more than one diagram is implemeted, find the correct diagram wich is owner of the given legend

    aRet.append( ObjectIdentifier::createParticleForDiagram( xDiagram, xChartModel ) );
    aRet.appendAscii(":");
    aRet.append(getStringForType( OBJECTTYPE_LEGEND ));
    aRet.appendAscii("=");

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createClassifiedIdentifier(
        enum ObjectType eObjectType //e.g. OBJECTTYPE_DATA_SERIES
        , const OUString& rParticleID )//e.g. SeriesID
{
    return createClassifiedIdentifierWithParent(
        eObjectType, rParticleID, m_aEmptyString );
}

OUString ObjectIdentifier::createClassifiedIdentifierWithParent(
        enum ObjectType eObjectType //e.g. OBJECTTYPE_DATA_POINT or OBJECTTYPE_GRID
        , const OUString& rParticleID //e.g. Point Index or SubGrid Index
        , const OUString& rParentPartical //e.g. "Series=SeriesID" or "Grid=GridId"
        , const OUString& rDragMethodServiceName
        , const OUString& rDragParameterString
        )
        //, bool bIsMultiClickObject ) //e.g. true
{
    //e.g. "MultiClick/Series=2:Point=34"

    OUStringBuffer aRet( m_aProtocol );
    aRet.append( lcl_createClassificationStringForType( eObjectType, rDragMethodServiceName, rDragParameterString ));
    if(aRet.getLength()>m_aProtocol.getLength())
        aRet.appendAscii("/");
    aRet.append(rParentPartical);
    if(rParentPartical.getLength())
        aRet.appendAscii(":");

    aRet.append(getStringForType( eObjectType ));
    aRet.appendAscii("=");
    aRet.append(rParticleID);

    return aRet.makeStringAndClear();
}

const OUString& ObjectIdentifier::getPieSegmentDragMethodServiceName()
{
    return m_aPieSegmentDragMethodServiceName;
}

OUString ObjectIdentifier::createPieSegmentDragParameterString(
          sal_Int32 nOffsetPercent
        , const awt::Point& rMinimumPosition
        , const awt::Point& rMaximumPosition )
{
    OUStringBuffer aRet( OUString::valueOf( nOffsetPercent ) );
    aRet.append( sal_Unicode( ',' ));
    aRet.append( OUString::valueOf( rMinimumPosition.X ) );
    aRet.append( sal_Unicode( ',' ));
    aRet.append( OUString::valueOf( rMinimumPosition.Y ) );
    aRet.append( sal_Unicode( ',' ));
    aRet.append( OUString::valueOf( rMaximumPosition.X ) );
    aRet.append( sal_Unicode( ',' ));
    aRet.append( OUString::valueOf( rMaximumPosition.Y ) );
    return aRet.makeStringAndClear();
}

bool ObjectIdentifier::parsePieSegmentDragParameterString(
          const OUString& rDragParameterString
        , sal_Int32& rOffsetPercent
        , awt::Point& rMinimumPosition
        , awt::Point& rMaximumPosition )
{
    OUString aValue;
    sal_Int32 nCharacterIndex = 0;

    OUString aValueString( rDragParameterString.getToken( 0, ',', nCharacterIndex ) );
    rOffsetPercent = aValueString.toInt32();
    if( nCharacterIndex < 0 )
        return false;

    aValueString = rDragParameterString.getToken( 0, ',', nCharacterIndex );
    rMinimumPosition.X = aValueString.toInt32();
    if( nCharacterIndex < 0 )
        return false;

    aValueString = rDragParameterString.getToken( 0, ',', nCharacterIndex );
    rMinimumPosition.Y = aValueString.toInt32();
    if( nCharacterIndex < 0 )
        return false;

    aValueString = rDragParameterString.getToken( 0, ',', nCharacterIndex );
    rMaximumPosition.X = aValueString.toInt32();
    if( nCharacterIndex < 0 )
        return false;

    aValueString = rDragParameterString.getToken( 0, ',', nCharacterIndex );
    rMaximumPosition.Y = aValueString.toInt32();
    if( nCharacterIndex < 0 )
        return false;

    return true;
}

OUString ObjectIdentifier::getDragMethodServiceName( const OUString& rCID )
{
    OUString aRet;

    sal_Int32 nIndexStart = rCID.indexOf( m_aDragMethodEquals );
    if( nIndexStart != -1 )
    {
        nIndexStart = rCID.indexOf( '=', nIndexStart );
        if( nIndexStart != -1 )
        {
            nIndexStart++;
            sal_Int32 nNextSlash = rCID.indexOf( '/', nIndexStart );
            if( nNextSlash != -1 )
            {
                sal_Int32 nIndexEnd = nNextSlash;
                sal_Int32 nNextColon = rCID.indexOf( ':', nIndexStart );
                if( nNextColon < nNextSlash )
                    nIndexEnd = nNextColon;
                aRet = rCID.copy(nIndexStart,nIndexEnd-nIndexStart);
            }
        }
    }
    return aRet;
}

OUString ObjectIdentifier::getDragParameterString( const OUString& rCID )
{
    OUString aRet;

    sal_Int32 nIndexStart = rCID.indexOf( m_aDragParameterEquals );
    if( nIndexStart != -1 )
    {
        nIndexStart = rCID.indexOf( '=', nIndexStart );
        if( nIndexStart != -1 )
        {
            nIndexStart++;
            sal_Int32 nNextSlash = rCID.indexOf( '/', nIndexStart );
            if( nNextSlash != -1 )
            {
                sal_Int32 nIndexEnd = nNextSlash;
                sal_Int32 nNextColon = rCID.indexOf( ':', nIndexStart );
                if( nNextColon < nNextSlash )
                    nIndexEnd = nNextColon;
                aRet = rCID.copy(nIndexStart,nIndexEnd-nIndexStart);
            }
        }
    }
    return aRet;
}

bool ObjectIdentifier::isDragableObject( const OUString& rClassifiedIdentifier )
{
    ObjectType eObjectType = ObjectIdentifier::getObjectType( rClassifiedIdentifier );
    switch( eObjectType )
    {
        case OBJECTTYPE_TITLE:
        case OBJECTTYPE_LEGEND:
        case OBJECTTYPE_DIAGRAM:
        case OBJECTTYPE_DATA_CURVE_EQUATION:
        //case OBJECTTYPE_DIAGRAM_WALL:
            return true;
        default:
            OUString aDragMethodServiceName( ObjectIdentifier::getDragMethodServiceName( rClassifiedIdentifier ) );
            if( aDragMethodServiceName.getLength() )
                return true;
            return false;
    }
    return false;
}

bool ObjectIdentifier::isDragableObject()
{
    bool bReturn = false;
    if ( isAutoGeneratedObject() )
    {
        bReturn = isDragableObject( m_aObjectCID );
    }
    else if ( isAdditionalShape() )
    {
        bReturn = true;
    }
    return bReturn;
}

bool ObjectIdentifier::isRotateableObject( const OUString& rClassifiedIdentifier )
{
    ObjectType eObjectType = ObjectIdentifier::getObjectType( rClassifiedIdentifier );
    switch( eObjectType )
    {
        case OBJECTTYPE_DIAGRAM:
        //case OBJECTTYPE_DIAGRAM_WALL:
            return true;
        default:
            return false;
    }
    return false;
}

bool ObjectIdentifier::isMultiClickObject( const OUString& rClassifiedIdentifier )
{
    //the name of a shape is it's ClassifiedIdentifier

    //a MultiClickObject is an object that is selectable by more than one click only ;
    //before a MultiClickObject can be selected it is necessary that a named parent group object
    //was selected before;

    //!!!!! by definition the name of a MultiClickObject starts with "CID/MultiClick:"
    bool bRet = false;
    bRet = rClassifiedIdentifier.match( m_aMultiClick, m_aProtocol.getLength() );
    return bRet;
}

bool ObjectIdentifier::areSiblings( const OUString& rCID1, const OUString& rCID2 )
{
    bool bRet=false;
    sal_Int32 nLastSign1 = rCID1.lastIndexOf( '=' );
    sal_Int32 nLastSign2 = rCID2.lastIndexOf( '=' );
    if( nLastSign1 == rCID1.indexOf( '=' ) )//CID cannot be sibling if only one "=" occurs
        bRet=false;
    else if( nLastSign2 == rCID2.indexOf( '=' ) )//CID cannot be sibling if only one "=" occurs
        bRet=false;
    else if( ObjectIdentifier::areIdenticalObjects( rCID1, rCID2 ) )
        bRet=false;
    else
    {
        OUString aParent1( ObjectIdentifier::getFullParentParticle( rCID1 ) );
        if( aParent1.getLength() )
        {
            OUString aParent2( ObjectIdentifier::getFullParentParticle( rCID2 ) );
            bRet=aParent1.equals(aParent2);
        }
        //legend entries are special:
        if(!bRet)
        {
            if( OBJECTTYPE_LEGEND_ENTRY == getObjectType(rCID1)
                && OBJECTTYPE_LEGEND_ENTRY == getObjectType(rCID2) )
                bRet = true;
        }
    }
    return bRet;
}

bool ObjectIdentifier::areIdenticalObjects( const OUString& rCID1, const OUString& rCID2 )
{
    if( rCID1.equals( rCID2 ) )
        return true;
    //draggable pie or donut segments need special treatment, as their CIDs do change with offset
    {
        if( rCID1.indexOf( m_aPieSegmentDragMethodServiceName ) < 0
            || rCID2.indexOf( m_aPieSegmentDragMethodServiceName ) < 0 )
            return false;

        OUString aID1( ObjectIdentifier::getObjectID( rCID1 ) );
        OUString aID2( ObjectIdentifier::getObjectID( rCID2 ) );
        if( aID1.getLength() &&  aID1.equals( aID2 ) )
            return true;
        }
    return false;
}

OUString ObjectIdentifier::getStringForType( ObjectType eObjectType )
{
    OUString aRet;
    switch( eObjectType )
    {
        case OBJECTTYPE_PAGE:
                aRet=C2U("Page");
                break;
        case OBJECTTYPE_TITLE:
                aRet=C2U("Title");
                break;
        case OBJECTTYPE_LEGEND:
                aRet=C2U("Legend");
                break;
        case OBJECTTYPE_LEGEND_ENTRY:
                aRet=C2U("LegendEntry");
                break;
        case OBJECTTYPE_DIAGRAM:
                aRet=C2U("D");
                break;
        case OBJECTTYPE_DIAGRAM_WALL:
                aRet=C2U("DiagramWall");
                break;
        case OBJECTTYPE_DIAGRAM_FLOOR:
                aRet=C2U("DiagramFloor");
                break;
        case OBJECTTYPE_AXIS:
                aRet=C2U("Axis");
                break;
        case OBJECTTYPE_AXIS_UNITLABEL:
                aRet=C2U("AxisUnitLabel");
                break;
        case OBJECTTYPE_GRID:
                aRet=C2U("Grid");
                break;
        case OBJECTTYPE_SUBGRID:
                aRet=C2U("SubGrid");
                break;
        case OBJECTTYPE_DATA_SERIES:
                aRet=C2U("Series");
                break;
        case OBJECTTYPE_DATA_POINT:
                aRet=C2U("Point");
                break;
        case OBJECTTYPE_DATA_LABELS:
                aRet=C2U("DataLabels");
                break;
        case OBJECTTYPE_DATA_LABEL:
                aRet=C2U("DataLabel");
                break;
        case OBJECTTYPE_DATA_ERRORS:
                aRet=C2U("Errors");
                break;
        case OBJECTTYPE_DATA_ERRORS_X:
                aRet=C2U("ErrorsX");
                break;
        case OBJECTTYPE_DATA_ERRORS_Y:
                aRet=C2U("ErrorsY");
                break;
        case OBJECTTYPE_DATA_ERRORS_Z:
                aRet=C2U("ErrorsZ");
                break;
        case OBJECTTYPE_DATA_CURVE:
                aRet=C2U("Curve");
                break;
        case OBJECTTYPE_DATA_CURVE_EQUATION:
                aRet=C2U("Equation");
                break;
        case OBJECTTYPE_DATA_AVERAGE_LINE:
                aRet=C2U("Average");
                break;
        case OBJECTTYPE_DATA_STOCK_RANGE:
                aRet=C2U("StockRange");
                break;
        case OBJECTTYPE_DATA_STOCK_LOSS:
                aRet=C2U("StockLoss");
                break;
        case OBJECTTYPE_DATA_STOCK_GAIN:
                aRet=C2U("StockGain");
                break;
        default: //OBJECTTYPE_UNKNOWN
            ;
    }
    return aRet;
}

ObjectType ObjectIdentifier::getObjectType( const OUString& rCID )
{
    ObjectType eRet;
    sal_Int32 nLastSign = rCID.lastIndexOf( ':' );//last sign before the type string
    if(nLastSign==-1)
        nLastSign = rCID.lastIndexOf( '/' );
    if(nLastSign==-1)
    {
        sal_Int32 nEndIndex = rCID.lastIndexOf( '=' );
        if(nEndIndex==-1)
            return OBJECTTYPE_UNKNOWN;
        nLastSign = 0;
    }
    if( nLastSign>0 )
        nLastSign++;

    if( rCID.match(C2U("Page"),nLastSign) )
        eRet = OBJECTTYPE_PAGE;
    else if( rCID.match(C2U("Title"),nLastSign) )
        eRet = OBJECTTYPE_TITLE;
    else if( rCID.match(C2U("LegendEntry"),nLastSign) )
        eRet = OBJECTTYPE_LEGEND_ENTRY;
    else if( rCID.match(C2U("Legend"),nLastSign) )
        eRet = OBJECTTYPE_LEGEND;
    else if( rCID.match(C2U("DiagramWall"),nLastSign) )
        eRet = OBJECTTYPE_DIAGRAM_WALL;
    else if( rCID.match(C2U("DiagramFloor"),nLastSign) )
        eRet = OBJECTTYPE_DIAGRAM_FLOOR;
    else if( rCID.match(C2U("D="),nLastSign) )
        eRet = OBJECTTYPE_DIAGRAM;
    else if( rCID.match(C2U("AxisUnitLabel"),nLastSign) )
        eRet = OBJECTTYPE_AXIS_UNITLABEL;
    else if( rCID.match(C2U("Axis"),nLastSign) )
        eRet = OBJECTTYPE_AXIS;
    else if( rCID.match(C2U("Grid"),nLastSign) )
        eRet = OBJECTTYPE_GRID;
    else if( rCID.match(C2U("SubGrid"),nLastSign) )
        eRet = OBJECTTYPE_SUBGRID;
    else if( rCID.match(C2U("Series"),nLastSign) )
        eRet = OBJECTTYPE_DATA_SERIES;
    else if( rCID.match(C2U("Point"),nLastSign) )
        eRet = OBJECTTYPE_DATA_POINT;
    else if( rCID.match(C2U("DataLabels"),nLastSign) )
        eRet = OBJECTTYPE_DATA_LABELS;
    else if( rCID.match(C2U("DataLabel"),nLastSign) )
        eRet = OBJECTTYPE_DATA_LABEL;
    else if( rCID.match(C2U("ErrorsX"),nLastSign) )
        eRet = OBJECTTYPE_DATA_ERRORS_X;
    else if( rCID.match(C2U("ErrorsY"),nLastSign) )
        eRet = OBJECTTYPE_DATA_ERRORS_Y;
    else if( rCID.match(C2U("ErrorsZ"),nLastSign) )
        eRet = OBJECTTYPE_DATA_ERRORS_Z;
    else if( rCID.match(C2U("Errors"),nLastSign) )
        eRet = OBJECTTYPE_DATA_ERRORS;
    else if( rCID.match(C2U("Curve"),nLastSign) )
        eRet = OBJECTTYPE_DATA_CURVE;
    else if( rCID.match(C2U("Equation"),nLastSign) )
        eRet = OBJECTTYPE_DATA_CURVE_EQUATION;
    else if( rCID.match(C2U("Average"),nLastSign) )
        eRet = OBJECTTYPE_DATA_AVERAGE_LINE;
    else if( rCID.match(C2U("StockRange"),nLastSign) )
        eRet = OBJECTTYPE_DATA_STOCK_RANGE;
    else if( rCID.match(C2U("StockLoss"),nLastSign) )
        eRet = OBJECTTYPE_DATA_STOCK_LOSS;
    else if( rCID.match(C2U("StockGain"),nLastSign) )
        eRet = OBJECTTYPE_DATA_STOCK_GAIN;
    else
        eRet = OBJECTTYPE_UNKNOWN;

    return eRet;
}

ObjectType ObjectIdentifier::getObjectType()
{
    ObjectType eObjectType( OBJECTTYPE_UNKNOWN );
    if ( isAutoGeneratedObject() )
    {
        eObjectType = getObjectType( m_aObjectCID );
    }
    else if ( isAdditionalShape() )
    {
        eObjectType = OBJECTTYPE_SHAPE;
    }
    return eObjectType;
}

OUString ObjectIdentifier::createDataCurveCID(
                                const OUString& rSeriesParticle
                                , sal_Int32 nCurveIndex
                                , bool bAverageLine )
{
    OUString aParticleID( OUString::valueOf( nCurveIndex ) );
    ObjectType eType = bAverageLine ? OBJECTTYPE_DATA_AVERAGE_LINE : OBJECTTYPE_DATA_CURVE;
    return createClassifiedIdentifierWithParent( eType, aParticleID, rSeriesParticle );
}

OUString ObjectIdentifier::createDataCurveEquationCID(
                                const OUString& rSeriesParticle
                                , sal_Int32 nCurveIndex )
{
    OUString aParticleID( OUString::valueOf( nCurveIndex ) );
    return createClassifiedIdentifierWithParent( OBJECTTYPE_DATA_CURVE_EQUATION, aParticleID, rSeriesParticle );
}

OUString ObjectIdentifier::addChildParticle( const rtl::OUString& rParticle, const rtl::OUString& rChildParticle )
{
    OUStringBuffer aRet(rParticle);

    if( aRet.getLength() && rChildParticle.getLength() )
        aRet.appendAscii(":");
    if( rChildParticle.getLength() )
        aRet.append(rChildParticle);

    return aRet.makeStringAndClear();
}

rtl::OUString ObjectIdentifier::createChildParticleWithIndex( ObjectType eObjectType, sal_Int32 nIndex )
{
    OUStringBuffer aRet( getStringForType( eObjectType ) );
    if( aRet.getLength() )
    {
        aRet.appendAscii("=");
        aRet.append(OUString::valueOf(nIndex));
    }
    return aRet.makeStringAndClear();
}

sal_Int32 ObjectIdentifier::getIndexFromParticleOrCID( const rtl::OUString& rParticleOrCID )
{
    sal_Int32 nRet = -1;

    OUString aIndexString = lcl_getIndexStringAfterString( rParticleOrCID, C2U("=") );
    sal_Int32 nCharacterIndex=0;
    nRet = lcl_StringToIndex( aIndexString.getToken( 0, ',', nCharacterIndex ) );

    return nRet;
}

OUString ObjectIdentifier::createSeriesSubObjectStub( ObjectType eSubObjectType
                    , const rtl::OUString& rSeriesParticle
                    , const rtl::OUString& rDragMethodServiceName
                    , const rtl::OUString& rDragParameterString )
{
    OUString aChildParticle( getStringForType( eSubObjectType ) );
    aChildParticle+=(C2U("="));

    return createClassifiedIdentifierForParticles(
            rSeriesParticle, aChildParticle
          , rDragMethodServiceName, rDragParameterString );
}

OUString ObjectIdentifier::createPointCID( const OUString& rPointCID_Stub, sal_Int32 nIndex  )
{
    OUString aRet(rPointCID_Stub);
    return aRet+=OUString::valueOf( nIndex );
}

OUString ObjectIdentifier::getParticleID( const OUString& rCID )
{
    OUString aRet;
    sal_Int32 nLast = rCID.lastIndexOf('=');
    if(nLast>=0)
        aRet = rCID.copy(++nLast);
    return aRet;
}

OUString ObjectIdentifier::getFullParentParticle( const OUString& rCID )
{
    OUString aRet;

    sal_Int32 nStartPos = rCID.lastIndexOf('/');
    if( nStartPos>=0 )
    {
        nStartPos++;
        sal_Int32 nEndPos = rCID.lastIndexOf(':');
        if( nEndPos>=0 && nStartPos < nEndPos )
        {
            aRet = rCID.copy(nStartPos,nEndPos-nStartPos);
        }
    }

    return aRet;
}

OUString ObjectIdentifier::getObjectID( const rtl::OUString& rCID )
{
    OUString aRet;

    sal_Int32 nStartPos = rCID.lastIndexOf('/');
    if( nStartPos>=0 )
    {
        nStartPos++;
        sal_Int32 nEndPos = rCID.getLength();
        aRet = rCID.copy(nStartPos,nEndPos-nStartPos);
    }

    return aRet;
}

bool ObjectIdentifier::isCID( const OUString& rName )
{
    return rName.getLength() && rName.match( m_aProtocol );
}

Reference< beans::XPropertySet > ObjectIdentifier::getObjectPropertySet(
    const OUString& rObjectCID,
    const Reference< chart2::XChartDocument >& xChartDocument )
{
    return ObjectIdentifier::getObjectPropertySet(
        rObjectCID, Reference< frame::XModel >( xChartDocument, uno::UNO_QUERY ));
}

Reference< beans::XPropertySet > ObjectIdentifier::getObjectPropertySet(
                const OUString& rObjectCID
                , const Reference< frame::XModel >& xChartModel )
{
    //return the model object that is indicated by rObjectCID
    if(!rObjectCID.getLength())
        return NULL;
    if(!xChartModel.is())
        return NULL;

    Reference< beans::XPropertySet > xObjectProperties = NULL;
    try
    {
        ObjectType eObjectType = ObjectIdentifier::getObjectType( rObjectCID );
        OUString aParticleID = ObjectIdentifier::getParticleID( rObjectCID );

        Reference< XDiagram > xDiagram;
        Reference< XCoordinateSystem > xCooSys;
        lcl_getDiagramAndCooSys( rObjectCID, xChartModel, xDiagram, xCooSys );

        switch(eObjectType)
        {
            case OBJECTTYPE_PAGE:
                {
                    Reference< XChartDocument > xChartDocument( xChartModel, uno::UNO_QUERY );
                    if( xChartDocument.is())
                        xObjectProperties.set( xChartDocument->getPageBackground() );
                }
                break;
            case OBJECTTYPE_TITLE:
                {
                    TitleHelper::eTitleType aTitleType = getTitleTypeForCID( rObjectCID );
                    Reference< XTitle > xTitle( TitleHelper::getTitle( aTitleType, xChartModel ) );
                    xObjectProperties.set( xTitle, uno::UNO_QUERY );
                }
                break;
            case OBJECTTYPE_LEGEND:
                {
                    if( xDiagram.is() )
                        xObjectProperties.set( xDiagram->getLegend(), uno::UNO_QUERY );
                }
                break;
            case OBJECTTYPE_LEGEND_ENTRY:
                    break;
            case OBJECTTYPE_DIAGRAM:
                {
                    xObjectProperties.set( xDiagram, uno::UNO_QUERY );
                }
                break;
            case OBJECTTYPE_DIAGRAM_WALL:
                {
                    if( xDiagram.is() )
                        xObjectProperties.set( xDiagram->getWall() );
                }
                break;
            case OBJECTTYPE_DIAGRAM_FLOOR:
                {
                    if( xDiagram.is() )
                        xObjectProperties.set( xDiagram->getFloor() );
                }
                break;
            case OBJECTTYPE_AXIS:
                {
                    sal_Int32 nDimensionIndex = -1;
                    sal_Int32 nAxisIndex = -1;
                    lcl_parseAxisIndices( nDimensionIndex, nAxisIndex, rObjectCID );

                    Reference< chart2::XAxis > xAxis(
                        AxisHelper::getAxis( nDimensionIndex, nAxisIndex, xCooSys ) );
                    if( xAxis.is() )
                        xObjectProperties.set( xAxis, uno::UNO_QUERY );
                }
                break;
            case OBJECTTYPE_AXIS_UNITLABEL:
                    break;
            case OBJECTTYPE_GRID:
            case OBJECTTYPE_SUBGRID:
                {
                    sal_Int32 nDimensionIndex = -1;
                    sal_Int32 nAxisIndex = -1;
                    lcl_parseAxisIndices( nDimensionIndex, nAxisIndex, rObjectCID );

                    sal_Int32 nSubGridIndex = -1;
                    lcl_parseGridIndices( nSubGridIndex, rObjectCID );

                    xObjectProperties.set( AxisHelper::getGridProperties( xCooSys , nDimensionIndex, nAxisIndex, nSubGridIndex ) );
                }
                break;
            case OBJECTTYPE_DATA_LABELS:
            case OBJECTTYPE_DATA_SERIES:
                {
                    Reference< XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID(
                        rObjectCID, xChartModel ) );
                    if( xSeries.is() )
                        xObjectProperties = Reference< beans::XPropertySet >( xSeries, uno::UNO_QUERY );

                    break;
                }
            case OBJECTTYPE_DATA_LABEL:
            case OBJECTTYPE_DATA_POINT:
                {
                    Reference< XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID(
                        rObjectCID, xChartModel ) );
                    if(xSeries.is())
                    {
                        sal_Int32 nIndex = aParticleID.toInt32();
                        xObjectProperties = xSeries->getDataPointByIndex( nIndex );
                    }
                    break;
                }
            case OBJECTTYPE_DATA_ERRORS:
                {
                    Reference< XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID(
                        rObjectCID, xChartModel ) );
                    if(xSeries.is())
                    {
                        Reference< beans::XPropertySet > xSeriesProp( xSeries, uno::UNO_QUERY );
                        Reference< beans::XPropertySet > xErrorBarProp;
                        if( xSeriesProp.is() )
                        {
                            xSeriesProp->getPropertyValue( C2U( "ErrorBarY" )) >>= xErrorBarProp;
                            xObjectProperties = Reference< beans::XPropertySet >( xErrorBarProp, uno::UNO_QUERY );
                        }
                    }
                    break;
                }
            case OBJECTTYPE_DATA_ERRORS_X:
                    break;
            case OBJECTTYPE_DATA_ERRORS_Y:
                    break;
            case OBJECTTYPE_DATA_ERRORS_Z:
                    break;
            case OBJECTTYPE_DATA_AVERAGE_LINE:
            case OBJECTTYPE_DATA_CURVE:
            case OBJECTTYPE_DATA_CURVE_EQUATION:
                {
                    Reference< XRegressionCurveContainer > xRegressionContainer( ObjectIdentifier::getDataSeriesForCID(
                        rObjectCID, xChartModel ), uno::UNO_QUERY );
                    if(xRegressionContainer.is())
                    {
                        sal_Int32 nIndex = aParticleID.toInt32();
                        uno::Sequence< Reference< XRegressionCurve > > aCurveList =
                            xRegressionContainer->getRegressionCurves();
                        if( nIndex >= 0 && nIndex <aCurveList.getLength() )
                        {
                            if( eObjectType == OBJECTTYPE_DATA_CURVE_EQUATION )
                                xObjectProperties.set( aCurveList[nIndex]->getEquationProperties());
                            else
                                xObjectProperties.set( aCurveList[nIndex], uno::UNO_QUERY );
                        }
                    }
                    break;
                }
            case OBJECTTYPE_DATA_STOCK_RANGE:
                    break;
            case OBJECTTYPE_DATA_STOCK_LOSS:
                    {
                        Reference<XChartType> xChartType( lcl_getFirstStockChartType( xChartModel ) );
                        Reference< beans::XPropertySet > xChartTypeProps( xChartType, uno::UNO_QUERY );
                        if(xChartTypeProps.is())
                            xChartTypeProps->getPropertyValue( C2U( "BlackDay" ) ) >>= xObjectProperties;
                    }
                    break;
            case OBJECTTYPE_DATA_STOCK_GAIN:
                    {
                        Reference<XChartType> xChartType( lcl_getFirstStockChartType( xChartModel ) );
                        Reference< beans::XPropertySet > xChartTypeProps( xChartType, uno::UNO_QUERY );
                        if(xChartTypeProps.is())
                            xChartTypeProps->getPropertyValue( C2U( "WhiteDay" ) ) >>= xObjectProperties;
                    }
                    break;
            default: //OBJECTTYPE_UNKNOWN
                    break;
        }
    }
    catch( uno::Exception& ex)
    {
        ASSERT_EXCEPTION( ex );
    }
    return xObjectProperties;
}

Reference< XAxis > ObjectIdentifier::getAxisForCID(
                const OUString& rObjectCID
                , const Reference< frame::XModel >& xChartModel )
{
    Reference< XDiagram > xDiagram;
    Reference< XCoordinateSystem > xCooSys;
    lcl_getDiagramAndCooSys( rObjectCID, xChartModel, xDiagram, xCooSys );

    sal_Int32 nDimensionIndex = -1;
    sal_Int32 nAxisIndex = -1;
    lcl_parseAxisIndices( nDimensionIndex, nAxisIndex, rObjectCID );

    return AxisHelper::getAxis( nDimensionIndex, nAxisIndex, xCooSys );
}

Reference< XDataSeries > ObjectIdentifier::getDataSeriesForCID(
                const OUString& rObjectCID
                , const Reference< frame::XModel >& xChartModel )
{
    Reference< XDataSeries > xSeries(NULL);

    Reference< XDiagram > xDiagram;
    Reference< XCoordinateSystem > xCooSys;
    lcl_getDiagramAndCooSys( rObjectCID, xChartModel, xDiagram, xCooSys );

    sal_Int32 nChartTypeIndex = -1;
    sal_Int32 nSeriesIndex = -1;
    sal_Int32 nPointIndex = -1;
    lcl_parseSeriesIndices( nChartTypeIndex, nSeriesIndex, nPointIndex, rObjectCID );

    Reference< XDataSeriesContainer > xDataSeriesContainer( DiagramHelper::getChartTypeByIndex( xDiagram, nChartTypeIndex ), uno::UNO_QUERY );
    if( xDataSeriesContainer.is() )
    {
        uno::Sequence< uno::Reference< XDataSeries > > aDataSeriesSeq( xDataSeriesContainer->getDataSeries() );
        if( nSeriesIndex >= 0 && nSeriesIndex < aDataSeriesSeq.getLength() )
            xSeries.set( aDataSeriesSeq[nSeriesIndex] );
    }

    return xSeries;
}

Reference< XDiagram > ObjectIdentifier::getDiagramForCID(
                  const rtl::OUString& rObjectCID
                , const uno::Reference< frame::XModel >& xChartModel )
{
    Reference< XDiagram > xDiagram;

    Reference< XCoordinateSystem > xCooSys;
    lcl_getDiagramAndCooSys( rObjectCID, xChartModel, xDiagram, xCooSys );

    return xDiagram;
}

TitleHelper::eTitleType ObjectIdentifier::getTitleTypeForCID( const OUString& rCID )
{
    TitleHelper::eTitleType eRet( TitleHelper::MAIN_TITLE );

    OUString aParentParticle = ObjectIdentifier::getFullParentParticle( rCID );
    const tTitleMap& rMap = lcl_getTitleMap();
    tTitleMap::const_iterator aIt( rMap.begin() );
    for( ;aIt != rMap.end(); ++aIt )
    {
        if( aParentParticle.equals( (*aIt).second ) )
        {
            eRet = (*aIt).first;
            break;
        }
    }

    return eRet;
}

OUString ObjectIdentifier::getSeriesParticleFromCID( const OUString& rCID )
{
    sal_Int32 nDiagramIndex = -1;
    sal_Int32 nCooSysIndex = -1;
    lcl_parseCooSysIndices( nDiagramIndex, nCooSysIndex, rCID );

    sal_Int32 nChartTypeIndex = -1;
    sal_Int32 nSeriesIndex = -1;
    sal_Int32 nPointIndex = -1;
    lcl_parseSeriesIndices( nChartTypeIndex, nSeriesIndex, nPointIndex, rCID );

    return ObjectIdentifier::createParticleForSeries( nDiagramIndex, nCooSysIndex, nChartTypeIndex, nSeriesIndex );
}

OUString ObjectIdentifier::getMovedSeriesCID( const ::rtl::OUString& rObjectCID, sal_Bool bForward )
{
    sal_Int32 nDiagramIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rObjectCID, C2U("CID/D=") ) );
    sal_Int32 nCooSysIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rObjectCID, C2U("CS=") ) );
    sal_Int32 nChartTypeIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rObjectCID, C2U("CT=") ) );
    sal_Int32 nSeriesIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rObjectCID, C2U("Series=") ) );

    if( bForward )
        nSeriesIndex--;
    else
        nSeriesIndex++;

    OUString aRet = ObjectIdentifier::createParticleForSeries( nDiagramIndex, nCooSysIndex, nChartTypeIndex, nSeriesIndex );
    return ObjectIdentifier::createClassifiedIdentifierForParticle( aRet );
}

bool ObjectIdentifier::isValid() const
{
    return ( isAutoGeneratedObject() || isAdditionalShape() );
}

bool ObjectIdentifier::isAutoGeneratedObject() const
{
    return ( m_aObjectCID.getLength() > 0 );
}

bool ObjectIdentifier::isAdditionalShape() const
{
    return m_xAdditionalShape.is();
}

OUString ObjectIdentifier::getObjectCID() const
{
    return m_aObjectCID;
}

Reference< drawing::XShape > ObjectIdentifier::getAdditionalShape() const
{
    return m_xAdditionalShape;
}

Any ObjectIdentifier::getAny() const
{
    Any aAny;
    if ( isAutoGeneratedObject() )
    {
        aAny = uno::makeAny( getObjectCID() );
    }
    else if ( isAdditionalShape() )
    {
        aAny = uno::makeAny( getAdditionalShape() );
    }
    return aAny;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
