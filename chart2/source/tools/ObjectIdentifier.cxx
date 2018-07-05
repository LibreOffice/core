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

#include <sal/config.h>

#include <map>

#include <ObjectIdentifier.hxx>
#include <TitleHelper.hxx>
#include <ChartModelHelper.hxx>
#include <AxisHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <DiagramHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <unonames.hxx>

#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>

#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;

static const char m_aMultiClick[] = "MultiClick";
static const char m_aDragMethodEquals[] = "DragMethod=";
static const char m_aDragParameterEquals[] = "DragParameter=";
static const char m_aProtocol[] = "CID/";
static const OUString m_aPieSegmentDragMethodServiceName("PieSegmentDraging");

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
            break;
        default:
            break;//empty string
    }
    if( !rDragMethodServiceName.isEmpty() )
    {
        if( !aRet.isEmpty() )
            aRet.append(":");
        aRet.append( m_aDragMethodEquals );
        aRet.append( rDragMethodServiceName );

        if( !rDragParameterString.isEmpty() )
        {
            if( !aRet.isEmpty() )
                aRet.append(":");
            aRet.append( m_aDragParameterEquals );
            aRet.append( rDragParameterString );
        }
    }
    return aRet.makeStringAndClear();
}

typedef std::map< TitleHelper::eTitleType, OUString > tTitleMap;
const tTitleMap& lcl_getTitleMap()
{
    //maps the title type to the ParentParticle for that title
    static tTitleMap s_aTitleMap{
        {TitleHelper::MAIN_TITLE, ""},
        {TitleHelper::SUB_TITLE, "D=0"},
        {TitleHelper::X_AXIS_TITLE, "D=0:CS=0:Axis=0,0"},
        {TitleHelper::Y_AXIS_TITLE, "D=0:CS=0:Axis=1,0"},
        {TitleHelper::Z_AXIS_TITLE, "D=0:CS=0:Axis=2,0"},
        {TitleHelper::SECONDARY_X_AXIS_TITLE, "D=0:CS=0:Axis=0,1"},
        {TitleHelper::SECONDARY_Y_AXIS_TITLE, "D=0:CS=0:Axis=1,1"}};
    return s_aTitleMap;
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
        return nullptr;

    //iterate through all coordinate systems
    Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is())
        return nullptr;

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
    return nullptr;
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
    if( !rIndexString.isEmpty() )
    {
        nRet = rIndexString.toInt32();
        if( nRet < -1 )
            nRet = -1;
    }
    return nRet;
}

void lcl_parseCooSysIndices( sal_Int32& rnDiagram, sal_Int32& rnCooSys, const OUString& rString )
{
    rnDiagram = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, "D=" ) );
    rnCooSys = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, "CS=" ) );
}

void lcl_parseAxisIndices( sal_Int32& rnDimensionIndex, sal_Int32& rnAxisIndex, const OUString& rString )
{
    OUString aAxisIndexString = lcl_getIndexStringAfterString( rString, ":Axis=" );
    sal_Int32 nCharacterIndex=0;
    rnDimensionIndex = lcl_StringToIndex( aAxisIndexString.getToken( 0, ',', nCharacterIndex ) );
    rnAxisIndex = lcl_StringToIndex( aAxisIndexString.getToken( 0, ',', nCharacterIndex ) );
}

void lcl_parseGridIndices( sal_Int32& rnSubGridIndex, const OUString& rString )
{
    rnSubGridIndex = -1;
    rnSubGridIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, ":SubGrid=" ) );
}

void lcl_parseSeriesIndices( sal_Int32& rnChartTypeIndex, sal_Int32& rnSeriesIndex, sal_Int32& rnPointIndex, const OUString& rString )
{
    rnChartTypeIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, "CT=" ) );
    rnSeriesIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, "Series=" ) );
    rnPointIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, "Point=" ) );
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
    ,m_xAdditionalShape( nullptr )
{
}

ObjectIdentifier::ObjectIdentifier( const OUString& rObjectCID )
    :m_aObjectCID( rObjectCID )
    ,m_xAdditionalShape( nullptr )
{
}

ObjectIdentifier::ObjectIdentifier( const Reference< drawing::XShape >& rxShape )
    :m_aObjectCID( OUString() )
    ,m_xAdditionalShape( rxShape )
{
}

ObjectIdentifier::ObjectIdentifier( const Any& rAny )
    :m_aObjectCID( OUString() )
    ,m_xAdditionalShape( nullptr )
{
    const uno::Type& rType = rAny.getValueType();
    if ( rType == cppu::UnoType<OUString>::get() )
    {
        rAny >>= m_aObjectCID;
    }
    else if ( rType == cppu::UnoType< drawing::XShape >::get() )
    {
        rAny >>= m_xAdditionalShape;
    }
}

bool ObjectIdentifier::operator==( const ObjectIdentifier& rOID ) const
{
    return areIdenticalObjects( m_aObjectCID, rOID.m_aObjectCID ) &&
         ( m_xAdditionalShape == rOID.m_xAdditionalShape );
}

bool ObjectIdentifier::operator!=( const ObjectIdentifier& rOID ) const
{
    return !operator==( rOID );
}

bool ObjectIdentifier::operator<( const ObjectIdentifier& rOID ) const
{
    bool bReturn = false;
    if ( !(m_aObjectCID.isEmpty() || rOID.m_aObjectCID.isEmpty()) )
    {
        bReturn = ( m_aObjectCID.compareTo( rOID.m_aObjectCID ) < 0 );
    }
    else if ( !m_aObjectCID.isEmpty() )
    {
        bReturn = true;
    }
    else if ( !rOID.m_aObjectCID.isEmpty() )
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
        , ChartModel& rModel)
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
            if( TitleHelper::getTitleType( aTitleType, xTitle, rModel ) )
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
            Reference< XCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemOfAxis( xAxis, rModel.getFirstDiagram() ) );
            OUString aCooSysParticle( createParticleForCoordinateSystem( xCooSys, rModel ) );
            sal_Int32 nDimensionIndex=-1;
            sal_Int32 nAxisIndex=-1;
            AxisHelper::getIndicesForAxis( xAxis, xCooSys, nDimensionIndex, nAxisIndex );
            OUString aAxisParticle( createParticleForAxis( nDimensionIndex, nAxisIndex ) );
            return createClassifiedIdentifierForParticles( aCooSysParticle, aAxisParticle );
        }

        //legend
        Reference< XLegend > xLegend( xObject, uno::UNO_QUERY );
        if( xLegend.is() )
        {
            return createClassifiedIdentifierForParticle( createParticleForLegend( rModel ) );
        }

        //diagram
        Reference< XDiagram > xDiagram( xObject, uno::UNO_QUERY );
        if( xDiagram.is() )
        {
            return createClassifiedIdentifierForParticle( createParticleForDiagram() );
        }

        //todo
        //XDataSeries
        //CooSys
        //charttype
        //datapoint?
        //Gridproperties
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    if( eObjectType != OBJECTTYPE_UNKNOWN )
    {
        aRet = ObjectIdentifier::createClassifiedIdentifierWithParent(
            eObjectType, aObjectID, aParentParticle, aDragMethodServiceName, aDragParameterString );
    }
    else
    {
        OSL_FAIL("give object could not be identified in createClassifiedIdentifierForObject");
    }

    return aRet;
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
            OUString aCooSysParticle( createParticleForCoordinateSystem( xCooSys, xChartModel ) );
            sal_Int32 nDimensionIndex=-1;
            sal_Int32 nAxisIndex=-1;
            AxisHelper::getIndicesForAxis( xAxis, xCooSys, nDimensionIndex, nAxisIndex );
            OUString aAxisParticle( createParticleForAxis( nDimensionIndex, nAxisIndex ) );
            return createClassifiedIdentifierForParticles( aCooSysParticle, aAxisParticle );
        }

        //legend
        Reference< XLegend > xLegend( xObject, uno::UNO_QUERY );
        if( xLegend.is() )
        {
            return createClassifiedIdentifierForParticle( createParticleForLegend( xChartModel ) );
        }

        //diagram
        Reference< XDiagram > xDiagram( xObject, uno::UNO_QUERY );
        if( xDiagram.is() )
        {
            return createClassifiedIdentifierForParticle( createParticleForDiagram() );
        }

        //todo
        //XDataSeries
        //CooSys
        //charttype
        //datapoint?
        //Gridproperties
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    if( eObjectType != OBJECTTYPE_UNKNOWN )
    {
        aRet = ObjectIdentifier::createClassifiedIdentifierWithParent(
            eObjectType, aObjectID, aParentParticle, aDragMethodServiceName, aDragParameterString );
    }
    else
    {
        OSL_FAIL("give object could not be identified in createClassifiedIdentifierForObject");
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
    if(aRet.getLength() > static_cast<sal_Int32>(strlen(m_aProtocol)))
        aRet.append("/");

    if(!rParentParticle.isEmpty())
    {
        aRet.append(rParentParticle);
        if( !rChildParticle.isEmpty() )
            aRet.append(":");
    }
    aRet.append(rChildParticle);

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createParticleForDiagram()
{
    //TODO: if more than one diagram is implemented, add the correct diagram index here
    return OUString("D=0");
}

OUString ObjectIdentifier::createParticleForCoordinateSystem(
          const Reference< XCoordinateSystem >& xCooSys
        , ChartModel& rModel )
{
    OUStringBuffer aRet;

    Reference< XDiagram > xDiagram( rModel.getFirstDiagram() );
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
                aRet = ObjectIdentifier::createParticleForDiagram();
                aRet.append(":CS=");
                aRet.append( OUString::number( nCooSysIndex ) );
                break;
            }
        }
    }

    return aRet.makeStringAndClear();
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
                aRet = ObjectIdentifier::createParticleForDiagram();
                aRet.append(":CS=");
                aRet.append( OUString::number( nCooSysIndex ) );
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
    OUStringBuffer aRet("Axis=");

    aRet.append( OUString::number( nDimensionIndex ) );
    aRet.append(",");
    aRet.append( OUString::number( nAxisIndex ) );

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createParticleForGrid(
            sal_Int32 nDimensionIndex
          , sal_Int32 nAxisIndex )
{
    OUStringBuffer aRet("Axis=");
    aRet.append( OUString::number( nDimensionIndex ) );
    aRet.append(",");
    aRet.append( OUString::number( nAxisIndex ) );
    aRet.append( ":Grid=0" );

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createClassifiedIdentifierForGrid(
          const Reference< XAxis >& xAxis
        , const Reference< frame::XModel >& xChartModel
        , sal_Int32 nSubGridIndex )
{
    //-1: main grid, 0: first subgrid etc

    OUString aAxisCID( createClassifiedIdentifierForObject( xAxis, xChartModel ) );
    OUString aGridCID( addChildParticle( aAxisCID
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

    aRet.append("D=");
    aRet.append( OUString::number( nDiagramIndex ) );
    aRet.append(":CS=");
    aRet.append( OUString::number( nCooSysIndex ) );
    aRet.append(":CT=");
    aRet.append( OUString::number( nChartTypeIndex ) );
    aRet.append(":");
    aRet.append(getStringForType( OBJECTTYPE_DATA_SERIES ));
    aRet.append("=");
    aRet.append( OUString::number( nSeriesIndex ) );

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createParticleForLegend( ChartModel& rModel )
{
    OUStringBuffer aRet;

    Reference< XDiagram > xDiagram( rModel.getFirstDiagram() );
    //todo: if more than one diagram is implemented, find the correct diagram which is owner of the given legend

    aRet.append( ObjectIdentifier::createParticleForDiagram() );
    aRet.append(":");
    aRet.append(getStringForType( OBJECTTYPE_LEGEND ));
    aRet.append("=");

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createParticleForLegend(
        const Reference< frame::XModel >& xChartModel )
{
    OUStringBuffer aRet;

    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    //todo: if more than one diagram is implemented, find the correct diagram which is owner of the given legend

    aRet.append( ObjectIdentifier::createParticleForDiagram() );
    aRet.append(":");
    aRet.append(getStringForType( OBJECTTYPE_LEGEND ));
    aRet.append("=");

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createClassifiedIdentifier(
        enum ObjectType eObjectType //e.g. OBJECTTYPE_DATA_SERIES
        , const OUString& rParticleID )//e.g. SeriesID
{
    return createClassifiedIdentifierWithParent(
        eObjectType, rParticleID, OUString() );
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
    if(aRet.getLength() > static_cast<sal_Int32>(strlen(m_aProtocol)))
        aRet.append("/");
    aRet.append(rParentPartical);
    if(!rParentPartical.isEmpty())
        aRet.append(":");

    aRet.append(getStringForType( eObjectType ));
    aRet.append("=");
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
    OUStringBuffer aRet( OUString::number( nOffsetPercent ) );
    aRet.append( ',');
    aRet.append( OUString::number( rMinimumPosition.X ) );
    aRet.append( ',');
    aRet.append( OUString::number( rMinimumPosition.Y ) );
    aRet.append( ',');
    aRet.append( OUString::number( rMaximumPosition.X ) );
    aRet.append( ',');
    aRet.append( OUString::number( rMaximumPosition.Y ) );
    return aRet.makeStringAndClear();
}

bool ObjectIdentifier::parsePieSegmentDragParameterString(
          const OUString& rDragParameterString
        , sal_Int32& rOffsetPercent
        , awt::Point& rMinimumPosition
        , awt::Point& rMaximumPosition )
{
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
    return nCharacterIndex >= 0;
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
    bool bReturn = false;
    ObjectType eObjectType = ObjectIdentifier::getObjectType( rClassifiedIdentifier );
    switch( eObjectType )
    {
        case OBJECTTYPE_TITLE:
        case OBJECTTYPE_LEGEND:
        case OBJECTTYPE_DIAGRAM:
        case OBJECTTYPE_DATA_CURVE_EQUATION:
        //case OBJECTTYPE_DIAGRAM_WALL:
            bReturn = true;
            break;
        default:
            OUString aDragMethodServiceName( ObjectIdentifier::getDragMethodServiceName( rClassifiedIdentifier ) );
            bReturn = !aDragMethodServiceName.isEmpty();
            break;
    }
    return bReturn;
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
    bool bReturn = false;
    ObjectType eObjectType = ObjectIdentifier::getObjectType( rClassifiedIdentifier );
    switch( eObjectType )
    {
        case OBJECTTYPE_DIAGRAM:
        //case OBJECTTYPE_DIAGRAM_WALL:
            bReturn = true;
            break;
        default:
            bReturn = false;
            break;
    }
    return bReturn;
}

bool ObjectIdentifier::isMultiClickObject( const OUString& rClassifiedIdentifier )
{
    //the name of a shape is it's ClassifiedIdentifier

    //a MultiClickObject is an object that is selectable by more than one click only ;
    //before a MultiClickObject can be selected it is necessary that a named parent group object
    //was selected before;

    //!!!!! by definition the name of a MultiClickObject starts with "CID/MultiClick:"
    bool bRet = rClassifiedIdentifier.match( m_aMultiClick, strlen(m_aProtocol) );
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
        if( !aParent1.isEmpty() )
        {
            OUString aParent2( ObjectIdentifier::getFullParentParticle( rCID2 ) );
            bRet=aParent1 == aParent2;
        }
        //legend entries are special:
        if(!bRet)
        {
            if( getObjectType(rCID1) == OBJECTTYPE_LEGEND_ENTRY
                && getObjectType(rCID2) == OBJECTTYPE_LEGEND_ENTRY )
                bRet = true;
        }
    }
    return bRet;
}

bool ObjectIdentifier::areIdenticalObjects( const OUString& rCID1, const OUString& rCID2 )
{
    if( rCID1 == rCID2 )
        return true;
    //draggable pie or donut segments need special treatment, as their CIDs do change with offset
    {
        if( rCID1.indexOf( m_aPieSegmentDragMethodServiceName ) < 0
            || rCID2.indexOf( m_aPieSegmentDragMethodServiceName ) < 0 )
            return false;

        OUString aID1( ObjectIdentifier::getObjectID( rCID1 ) );
        OUString aID2( ObjectIdentifier::getObjectID( rCID2 ) );
        if( !aID1.isEmpty() &&  aID1 == aID2 )
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
                aRet="Page";
                break;
        case OBJECTTYPE_TITLE:
                aRet="Title";
                break;
        case OBJECTTYPE_LEGEND:
                aRet="Legend";
                break;
        case OBJECTTYPE_LEGEND_ENTRY:
                aRet="LegendEntry";
                break;
        case OBJECTTYPE_DIAGRAM:
                aRet="D";
                break;
        case OBJECTTYPE_DIAGRAM_WALL:
                aRet="DiagramWall";
                break;
        case OBJECTTYPE_DIAGRAM_FLOOR:
                aRet="DiagramFloor";
                break;
        case OBJECTTYPE_AXIS:
                aRet="Axis";
                break;
        case OBJECTTYPE_AXIS_UNITLABEL:
                aRet="AxisUnitLabel";
                break;
        case OBJECTTYPE_GRID:
                aRet="Grid";
                break;
        case OBJECTTYPE_SUBGRID:
                aRet="SubGrid";
                break;
        case OBJECTTYPE_DATA_SERIES:
                aRet="Series";
                break;
        case OBJECTTYPE_DATA_POINT:
                aRet="Point";
                break;
        case OBJECTTYPE_DATA_LABELS:
                aRet="DataLabels";
                break;
        case OBJECTTYPE_DATA_LABEL:
                aRet="DataLabel";
                break;
        case OBJECTTYPE_DATA_ERRORS_X:
                aRet="ErrorsX";
                break;
        case OBJECTTYPE_DATA_ERRORS_Y:
                aRet="ErrorsY";
                break;
        case OBJECTTYPE_DATA_ERRORS_Z:
                aRet="ErrorsZ";
                break;
        case OBJECTTYPE_DATA_CURVE:
                aRet="Curve";
                break;
        case OBJECTTYPE_DATA_CURVE_EQUATION:
                aRet="Equation";
                break;
        case OBJECTTYPE_DATA_AVERAGE_LINE:
                aRet="Average";
                break;
        case OBJECTTYPE_DATA_STOCK_RANGE:
                aRet="StockRange";
                break;
        case OBJECTTYPE_DATA_STOCK_LOSS:
                aRet="StockLoss";
                break;
        case OBJECTTYPE_DATA_STOCK_GAIN:
                aRet="StockGain";
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

    if( rCID.match("Page",nLastSign) )
        eRet = OBJECTTYPE_PAGE;
    else if( rCID.match("Title",nLastSign) )
        eRet = OBJECTTYPE_TITLE;
    else if( rCID.match("LegendEntry",nLastSign) )
        eRet = OBJECTTYPE_LEGEND_ENTRY;
    else if( rCID.match("Legend",nLastSign) )
        eRet = OBJECTTYPE_LEGEND;
    else if( rCID.match("DiagramWall",nLastSign) )
        eRet = OBJECTTYPE_DIAGRAM_WALL;
    else if( rCID.match("DiagramFloor",nLastSign) )
        eRet = OBJECTTYPE_DIAGRAM_FLOOR;
    else if( rCID.match("D=",nLastSign) )
        eRet = OBJECTTYPE_DIAGRAM;
    else if( rCID.match("AxisUnitLabel",nLastSign) )
        eRet = OBJECTTYPE_AXIS_UNITLABEL;
    else if( rCID.match("Axis",nLastSign) )
        eRet = OBJECTTYPE_AXIS;
    else if( rCID.match("Grid",nLastSign) )
        eRet = OBJECTTYPE_GRID;
    else if( rCID.match("SubGrid",nLastSign) )
        eRet = OBJECTTYPE_SUBGRID;
    else if( rCID.match("Series",nLastSign) )
        eRet = OBJECTTYPE_DATA_SERIES;
    else if( rCID.match("Point",nLastSign) )
        eRet = OBJECTTYPE_DATA_POINT;
    else if( rCID.match("DataLabels",nLastSign) )
        eRet = OBJECTTYPE_DATA_LABELS;
    else if( rCID.match("DataLabel",nLastSign) )
        eRet = OBJECTTYPE_DATA_LABEL;
    else if( rCID.match("ErrorsX",nLastSign) )
        eRet = OBJECTTYPE_DATA_ERRORS_X;
    else if( rCID.match("ErrorsY",nLastSign) )
        eRet = OBJECTTYPE_DATA_ERRORS_Y;
    else if( rCID.match("ErrorsZ",nLastSign) )
        eRet = OBJECTTYPE_DATA_ERRORS_Z;
    else if( rCID.match("Curve",nLastSign) )
        eRet = OBJECTTYPE_DATA_CURVE;
    else if( rCID.match("Equation",nLastSign) )
        eRet = OBJECTTYPE_DATA_CURVE_EQUATION;
    else if( rCID.match("Average",nLastSign) )
        eRet = OBJECTTYPE_DATA_AVERAGE_LINE;
    else if( rCID.match("StockRange",nLastSign) )
        eRet = OBJECTTYPE_DATA_STOCK_RANGE;
    else if( rCID.match("StockLoss",nLastSign) )
        eRet = OBJECTTYPE_DATA_STOCK_LOSS;
    else if( rCID.match("StockGain",nLastSign) )
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
    OUString aParticleID( OUString::number( nCurveIndex ) );
    ObjectType eType = bAverageLine ? OBJECTTYPE_DATA_AVERAGE_LINE : OBJECTTYPE_DATA_CURVE;
    return createClassifiedIdentifierWithParent( eType, aParticleID, rSeriesParticle );
}

OUString ObjectIdentifier::createDataCurveEquationCID(
                                const OUString& rSeriesParticle
                                , sal_Int32 nCurveIndex )
{
    OUString aParticleID( OUString::number( nCurveIndex ) );
    return createClassifiedIdentifierWithParent( OBJECTTYPE_DATA_CURVE_EQUATION, aParticleID, rSeriesParticle );
}

OUString ObjectIdentifier::addChildParticle( const OUString& rParticle, const OUString& rChildParticle )
{
    OUStringBuffer aRet(rParticle);

    if( !aRet.isEmpty() && !rChildParticle.isEmpty() )
        aRet.append(":");
    if( !rChildParticle.isEmpty() )
        aRet.append(rChildParticle);

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createChildParticleWithIndex( ObjectType eObjectType, sal_Int32 nIndex )
{
    OUStringBuffer aRet( getStringForType( eObjectType ) );
    if( !aRet.isEmpty() )
    {
        aRet.append("=");
        aRet.append(OUString::number(nIndex));
    }
    return aRet.makeStringAndClear();
}

sal_Int32 ObjectIdentifier::getIndexFromParticleOrCID( const OUString& rParticleOrCID )
{
    OUString aIndexString = lcl_getIndexStringAfterString( rParticleOrCID, "=" );
    sal_Int32 nCharacterIndex=0;
    sal_Int32 nRet = lcl_StringToIndex( aIndexString.getToken( 0, ',', nCharacterIndex ) );

    return nRet;
}

OUString ObjectIdentifier::createSeriesSubObjectStub( ObjectType eSubObjectType
                    , const OUString& rSeriesParticle
                    , const OUString& rDragMethodServiceName
                    , const OUString& rDragParameterString )
{
    OUString aChildParticle = getStringForType( eSubObjectType ) + "=";

    return createClassifiedIdentifierForParticles(
            rSeriesParticle, aChildParticle
          , rDragMethodServiceName, rDragParameterString );
}

OUString ObjectIdentifier::createPointCID( const OUString& rPointCID_Stub, sal_Int32 nIndex  )
{
    OUString aRet(rPointCID_Stub);
    return aRet+=OUString::number( nIndex );
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

OUString ObjectIdentifier::getObjectID( const OUString& rCID )
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
    return !rName.isEmpty() && rName.match( m_aProtocol );
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
    if(rObjectCID.isEmpty())
        return nullptr;
    if(!xChartModel.is())
        return nullptr;

    Reference< beans::XPropertySet > xObjectProperties = nullptr;
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
                        xObjectProperties.set( xSeries, uno::UNO_QUERY );

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
            case OBJECTTYPE_DATA_ERRORS_X:
            case OBJECTTYPE_DATA_ERRORS_Y:
            case OBJECTTYPE_DATA_ERRORS_Z:
                {
                    Reference< XDataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID(
                        rObjectCID, xChartModel ) );
                    if(xSeries.is())
                    {
                        Reference< beans::XPropertySet > xSeriesProp( xSeries, uno::UNO_QUERY );
                        Reference< beans::XPropertySet > xErrorBarProp;
                        if( xSeriesProp.is() )
                        {
                            OUString errorBar;

                            if ( eObjectType == OBJECTTYPE_DATA_ERRORS_X)
                                errorBar = CHART_UNONAME_ERRORBAR_X;
                            else if (eObjectType == OBJECTTYPE_DATA_ERRORS_Y)
                                errorBar = CHART_UNONAME_ERRORBAR_Y;
                            else
                                errorBar = "ErrorBarZ";

                            xSeriesProp->getPropertyValue( errorBar ) >>= xErrorBarProp;
                            xObjectProperties.set( xErrorBarProp, uno::UNO_QUERY );
                        }
                    }
                    break;
                }
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
                            xChartTypeProps->getPropertyValue( "BlackDay" ) >>= xObjectProperties;
                    }
                    break;
            case OBJECTTYPE_DATA_STOCK_GAIN:
                    {
                        Reference<XChartType> xChartType( lcl_getFirstStockChartType( xChartModel ) );
                        Reference< beans::XPropertySet > xChartTypeProps( xChartType, uno::UNO_QUERY );
                        if(xChartTypeProps.is())
                            xChartTypeProps->getPropertyValue( "WhiteDay" ) >>= xObjectProperties;
                    }
                    break;
            default: //OBJECTTYPE_UNKNOWN
                    break;
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
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
    Reference< XDataSeries > xSeries(nullptr);

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
                  const OUString& rObjectCID
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
        if( aParentParticle == (*aIt).second )
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

OUString ObjectIdentifier::getMovedSeriesCID( const OUString& rObjectCID, bool bForward )
{
    sal_Int32 nDiagramIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rObjectCID, "CID/D=" ) );
    sal_Int32 nCooSysIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rObjectCID, "CS=" ) );
    sal_Int32 nChartTypeIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rObjectCID, "CT=" ) );
    sal_Int32 nSeriesIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rObjectCID, "Series=" ) );

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
    return ( !m_aObjectCID.isEmpty() );
}

bool ObjectIdentifier::isAdditionalShape() const
{
    return m_xAdditionalShape.is();
}

Any ObjectIdentifier::getAny() const
{
    Any aAny;
    if ( isAutoGeneratedObject() )
    {
        aAny <<= getObjectCID();
    }
    else if ( isAdditionalShape() )
    {
        aAny <<= getAdditionalShape();
    }
    return aAny;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
