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

#include <cstddef>
#include <map>

#include <ObjectIdentifier.hxx>
#include <TitleHelper.hxx>
#include <ChartModel.hxx>
#include <ChartType.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <Diagram.hxx>
#include <unonames.hxx>
#include <BaseCoordinateSystem.hxx>
#include <DataSeries.hxx>
#include <RegressionCurveModel.hxx>

#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include <rtl/ustrbuf.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <utility>

namespace com::sun::star::drawing { class XShape; }

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;

constexpr OUString m_aMultiClick = u"MultiClick"_ustr;
constexpr OUString m_aDragMethodEquals = u"DragMethod="_ustr;
constexpr OUString m_aDragParameterEquals = u"DragParameter="_ustr;
constexpr OUString m_aProtocol = u"CID/"_ustr;
constexpr OUString m_aPieSegmentDragMethodServiceName(u"PieSegmentDragging"_ustr);

namespace
{

OUString lcl_createClassificationStringForType( ObjectType eObjectType
            , std::u16string_view rDragMethodServiceName
            , std::u16string_view rDragParameterString
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
    if( !rDragMethodServiceName.empty() )
    {
        if( !aRet.isEmpty() )
            aRet.append(":");
        aRet.append( OUString::Concat(m_aDragMethodEquals) + rDragMethodServiceName );

        if( !rDragParameterString.empty() )
        {
            if( !aRet.isEmpty() )
                aRet.append(":");
            aRet.append( OUString::Concat(m_aDragParameterEquals) + rDragParameterString );
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

rtl::Reference<ChartType> lcl_getFirstStockChartType( const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    rtl::Reference< Diagram > xDiagram( xChartModel->getFirstChartDiagram() );
    if(!xDiagram.is())
        return nullptr;

    //iterate through all coordinate systems

    const std::vector< rtl::Reference< BaseCoordinateSystem > > & aCooSysList( xDiagram->getBaseCoordinateSystems() );
    for( rtl::Reference< BaseCoordinateSystem > const & coords : aCooSysList )
    {
        //iterate through all chart types in the current coordinate system
        for( rtl::Reference< ChartType > const & xChartType : coords->getChartTypes2() )
        {
            OUString aChartType = xChartType->getChartType();
            if( aChartType.equalsIgnoreAsciiCase(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
                return xChartType;
        }
    }
    return nullptr;
}

std::u16string_view lcl_getIndexStringAfterString( std::u16string_view rString, std::u16string_view rSearchString )
{
    size_t nIndexStart = rString.rfind( rSearchString );
    if( nIndexStart == std::u16string_view::npos )
        return std::u16string_view();
    nIndexStart += rSearchString.size();
    size_t nIndexEnd = rString.size();
    size_t nNextColon = rString.find( ':', nIndexStart );
    if( nNextColon != std::u16string_view::npos )
        nIndexEnd = nNextColon;
    return rString.substr(nIndexStart,nIndexEnd-nIndexStart);
}

sal_Int32 lcl_StringToIndex( std::u16string_view rIndexString )
{
    sal_Int32 nRet = -1;
    if( !rIndexString.empty() )
    {
        nRet = o3tl::toInt32(rIndexString);
        if( nRet < -1 )
            nRet = -1;
    }
    return nRet;
}

void lcl_parseCooSysIndices( sal_Int32& rnDiagram, sal_Int32& rnCooSys, std::u16string_view rString )
{
    rnDiagram = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, u"D=" ) );
    rnCooSys = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, u"CS=" ) );
}

void lcl_parseAxisIndices( sal_Int32& rnDimensionIndex, sal_Int32& rnAxisIndex, std::u16string_view rString )
{
    std::u16string_view aAxisIndexString = lcl_getIndexStringAfterString( rString, u":Axis=" );
    sal_Int32 nCharacterIndex=0;
    rnDimensionIndex = lcl_StringToIndex( o3tl::getToken(aAxisIndexString, 0, ',', nCharacterIndex ) );
    rnAxisIndex = lcl_StringToIndex( o3tl::getToken(aAxisIndexString, 0, ',', nCharacterIndex ) );
}

void lcl_parseGridIndices( sal_Int32& rnSubGridIndex, std::u16string_view rString )
{
    rnSubGridIndex = -1;
    rnSubGridIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, u":SubGrid=" ) );
}

void lcl_parseSeriesIndices( sal_Int32& rnChartTypeIndex, sal_Int32& rnSeriesIndex, sal_Int32& rnPointIndex, std::u16string_view rString )
{
    rnChartTypeIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, u"CT=" ) );
    rnSeriesIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, u"Series=" ) );
    rnPointIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rString, u"Point=" ) );
}

void lcl_getDiagramAndCooSys( std::u16string_view rObjectCID
                , const rtl::Reference<::chart::ChartModel>& xChartModel
                , rtl::Reference< Diagram >& xDiagram
                , rtl::Reference< BaseCoordinateSystem >& xCooSys )
{
    sal_Int32 nDiagramIndex = -1;
    sal_Int32 nCooSysIndex = -1;
    lcl_parseCooSysIndices( nDiagramIndex, nCooSysIndex, rObjectCID );
    xDiagram = xChartModel->getFirstChartDiagram();//todo use nDiagramIndex when more than one diagram is possible in future
    if( !xDiagram.is() )
        return;

    if( nCooSysIndex > -1 )
    {
        const std::vector< rtl::Reference< BaseCoordinateSystem > > aCooSysList( xDiagram->getBaseCoordinateSystems() );
        if( o3tl::make_unsigned(nCooSysIndex) < aCooSysList.size() )
            xCooSys = aCooSysList[nCooSysIndex];
    }
}

} //anonymous namespace

ObjectIdentifier::ObjectIdentifier()
{
}

ObjectIdentifier::ObjectIdentifier( OUString aObjectCID )
    :m_aObjectCID(std::move( aObjectCID ))
{
}

ObjectIdentifier::ObjectIdentifier( const Reference< drawing::XShape >& rxShape )
    : m_xAdditionalShape( rxShape )
{
}

ObjectIdentifier::ObjectIdentifier( const Any& rAny )
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
          const rtl::Reference< ::chart::Title >& xTitle
        , const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    TitleHelper::eTitleType aTitleType;
    OUString aRet;
    const std::u16string_view aObjectID;
    const std::u16string_view aDragMethodServiceName;
    const std::u16string_view aDragParameterString;
    if( TitleHelper::getTitleType( aTitleType, xTitle, xChartModel ) )
    {
        enum ObjectType eObjectType = OBJECTTYPE_TITLE;
        OUString aParentParticle = lcl_getTitleParentParticle( aTitleType );
        aRet = ObjectIdentifier::createClassifiedIdentifierWithParent(
            eObjectType, aObjectID, aParentParticle, aDragMethodServiceName, aDragParameterString );
    }
    return aRet;
}

OUString ObjectIdentifier::createClassifiedIdentifierForObject(
          const Reference< uno::XInterface >& xObject
        , const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    OUString aRet;

    enum ObjectType eObjectType = OBJECTTYPE_UNKNOWN;
    const std::u16string_view aObjectID;
    OUString aParentParticle;
    const std::u16string_view aDragMethodServiceName;
    const std::u16string_view aDragParameterString;

    try
    {
        //title
        if( ::chart::Title* pTitle = dynamic_cast<::chart::Title*>(xObject.get()) )
            return createClassifiedIdentifierForObject(rtl::Reference<Title>(pTitle), xChartModel);

        uno::Reference<chart2::XDataTable> xDataTable(xObject, uno::UNO_QUERY);
        if (xDataTable.is())
        {
            return createClassifiedIdentifierForParticle(createParticleForDataTable(xChartModel));
        }

        //axis
        rtl::Reference< Axis > xAxis = dynamic_cast<Axis*>( xObject.get() );
        if( xAxis.is() )
        {
            rtl::Reference<Diagram> xDiagram = xChartModel->getFirstChartDiagram();
            rtl::Reference< BaseCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemOfAxis( xAxis, xDiagram ) );
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

OUString ObjectIdentifier::createClassifiedIdentifierForObject(
          const rtl::Reference< Legend >& xLegend
        , const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    try
    {
        if( xLegend.is() )
        {
            return createClassifiedIdentifierForParticle( createParticleForLegend( xChartModel ) );
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    OSL_FAIL("give object could not be identified in createClassifiedIdentifierForObject");

    return OUString();
}

OUString ObjectIdentifier::createClassifiedIdentifierForObject(
          const rtl::Reference<::chart::Axis>& xAxis
        , const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    try
    {
        //axis
        if( xAxis.is() )
        {
            rtl::Reference<Diagram> xDiagram = xChartModel->getFirstChartDiagram();
            rtl::Reference< BaseCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemOfAxis( xAxis, xDiagram ) );
            OUString aCooSysParticle( createParticleForCoordinateSystem( xCooSys, xChartModel ) );
            sal_Int32 nDimensionIndex=-1;
            sal_Int32 nAxisIndex=-1;
            AxisHelper::getIndicesForAxis( xAxis, xCooSys, nDimensionIndex, nAxisIndex );
            OUString aAxisParticle( createParticleForAxis( nDimensionIndex, nAxisIndex ) );
            return createClassifiedIdentifierForParticles( aCooSysParticle, aAxisParticle );
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    OSL_FAIL("give object could not be identified in createClassifiedIdentifierForObject");

    return OUString();
}

OUString ObjectIdentifier::createClassifiedIdentifierForParticle(
         std::u16string_view rParticle )
{
    return ObjectIdentifier::createClassifiedIdentifierForParticles( rParticle, u"" );
}

OUString ObjectIdentifier::createClassifiedIdentifierForParticles(
            std::u16string_view rParentParticle
          , std::u16string_view rChildParticle
          , std::u16string_view rDragMethodServiceName
          , std::u16string_view rDragParameterString )
{
    ObjectType eObjectType( ObjectIdentifier::getObjectType( rChildParticle ) );
    if( eObjectType == OBJECTTYPE_UNKNOWN )
        eObjectType = ObjectIdentifier::getObjectType( rParentParticle );

    OUStringBuffer aRet( m_aProtocol +
        lcl_createClassificationStringForType( eObjectType, rDragMethodServiceName, rDragParameterString ));
    if(aRet.getLength() > m_aProtocol.getLength())
        aRet.append("/");

    if(!rParentParticle.empty())
    {
        aRet.append(rParentParticle);
        if( !rChildParticle.empty() )
            aRet.append(":");
    }
    aRet.append(rChildParticle);

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createParticleForDiagram()
{
    //TODO: if more than one diagram is implemented, add the correct diagram index here
    return u"D=0"_ustr;
}

OUString ObjectIdentifier::createParticleForCoordinateSystem(
          const rtl::Reference< BaseCoordinateSystem >& xCooSys
        , const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    OUString aRet;

    rtl::Reference< Diagram > xDiagram( xChartModel->getFirstChartDiagram() );
    if( xDiagram.is() )
    {
        std::size_t nCooSysIndex = 0;
        const std::vector< rtl::Reference< BaseCoordinateSystem > > & aCooSysList( xDiagram->getBaseCoordinateSystems() );
        for( ; nCooSysIndex < aCooSysList.size(); ++nCooSysIndex )
        {
            if( xCooSys == aCooSysList[nCooSysIndex] )
            {
                aRet = ObjectIdentifier::createParticleForDiagram() + ":CS=" + OUString::number( nCooSysIndex );
                break;
            }
        }
    }

    return aRet;
}

OUString ObjectIdentifier::createParticleForAxis(
            sal_Int32 nDimensionIndex
          , sal_Int32 nAxisIndex )
{
    return "Axis=" +
        OUString::number( nDimensionIndex ) +
        "," +
        OUString::number( nAxisIndex );
}

OUString ObjectIdentifier::createParticleForGrid(
            sal_Int32 nDimensionIndex
          , sal_Int32 nAxisIndex )
{
    OUString aRet = "Axis=" + OUString::number( nDimensionIndex )
                  + "," + OUString::number( nAxisIndex ) + ":Grid=0";

    return aRet;
}

OUString ObjectIdentifier::createClassifiedIdentifierForGrid(
          const Reference< XAxis >& xAxis
        , const rtl::Reference<::chart::ChartModel>& xChartModel
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
    return
        "D=" + OUString::number( nDiagramIndex ) +
        ":CS=" + OUString::number( nCooSysIndex ) +
        ":CT=" + OUString::number( nChartTypeIndex ) +
        ":" + getStringForType( OBJECTTYPE_DATA_SERIES ) + "=" +
        OUString::number( nSeriesIndex );
}


OUString ObjectIdentifier::createParticleForLegend(
        const rtl::Reference<::chart::ChartModel>& )
{
    //todo: if more than one diagram is implemented, find the correct diagram which is owner of the given legend

    return ObjectIdentifier::createParticleForDiagram() + ":" + getStringForType( OBJECTTYPE_LEGEND ) + "=";
}

OUString ObjectIdentifier::createParticleForDataTable(const rtl::Reference<::chart::ChartModel>& /* xChartModel */)
{
    return ObjectIdentifier::createParticleForDiagram() + ":" + getStringForType(OBJECTTYPE_DATA_TABLE) + "=";
}

OUString ObjectIdentifier::createClassifiedIdentifier(
        enum ObjectType eObjectType //e.g. OBJECTTYPE_DATA_SERIES
        , std::u16string_view rParticleID )//e.g. SeriesID
{
    return createClassifiedIdentifierWithParent(
        eObjectType, rParticleID, u"" );
}

OUString ObjectIdentifier::createClassifiedIdentifierWithParent(
        enum ObjectType eObjectType //e.g. OBJECTTYPE_DATA_POINT or OBJECTTYPE_GRID
        , std::u16string_view rParticleID //e.g. Point Index or SubGrid Index
        , std::u16string_view rParentPartical //e.g. "Series=SeriesID" or "Grid=GridId"
        , std::u16string_view rDragMethodServiceName
        , std::u16string_view rDragParameterString
        )
        //, bool bIsMultiClickObject ) //e.g. true
{
    //e.g. "MultiClick/Series=2:Point=34"

    OUStringBuffer aRet( m_aProtocol +
        lcl_createClassificationStringForType( eObjectType, rDragMethodServiceName, rDragParameterString ));
    if(aRet.getLength() > m_aProtocol.getLength())
        aRet.append("/");
    aRet.append(rParentPartical);
    if(!rParentPartical.empty())
        aRet.append(":");

    aRet.append(getStringForType( eObjectType ) + "=" + rParticleID);

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
    OUString aRet = OUString::number( nOffsetPercent )
                  + "," + OUString::number( rMinimumPosition.X )
                  + "," + OUString::number( rMinimumPosition.Y )
                  + "," + OUString::number( rMaximumPosition.X )
                  + "," + OUString::number( rMaximumPosition.Y );
    return aRet;
}

bool ObjectIdentifier::parsePieSegmentDragParameterString(
          std::u16string_view rDragParameterString
        , sal_Int32& rOffsetPercent
        , awt::Point& rMinimumPosition
        , awt::Point& rMaximumPosition )
{
    sal_Int32 nCharacterIndex = 0;

    std::u16string_view aValueString( o3tl::getToken(rDragParameterString, 0, ',', nCharacterIndex ) );
    rOffsetPercent = o3tl::toInt32(aValueString);
    if( nCharacterIndex < 0 )
        return false;

    aValueString = o3tl::getToken(rDragParameterString, 0, ',', nCharacterIndex );
    rMinimumPosition.X = o3tl::toInt32(aValueString);
    if( nCharacterIndex < 0 )
        return false;

    aValueString = o3tl::getToken(rDragParameterString, 0, ',', nCharacterIndex );
    rMinimumPosition.Y = o3tl::toInt32(aValueString);
    if( nCharacterIndex < 0 )
        return false;

    aValueString = o3tl::getToken(rDragParameterString, 0, ',', nCharacterIndex );
    rMaximumPosition.X = o3tl::toInt32(aValueString);
    if( nCharacterIndex < 0 )
        return false;

    aValueString = o3tl::getToken(rDragParameterString, 0, ',', nCharacterIndex );
    rMaximumPosition.Y = o3tl::toInt32(aValueString);
    return nCharacterIndex >= 0;
}

std::u16string_view ObjectIdentifier::getDragMethodServiceName( std::u16string_view rCID )
{
    std::u16string_view aRet;

    size_t nIndexStart = rCID.find( m_aDragMethodEquals );
    if( nIndexStart != std::u16string_view::npos )
    {
        nIndexStart = rCID.find( '=', nIndexStart );
        if( nIndexStart != std::u16string_view::npos )
        {
            nIndexStart++;
            size_t nNextSlash = rCID.find( '/', nIndexStart );
            if( nNextSlash != std::u16string_view::npos )
            {
                sal_Int32 nIndexEnd = nNextSlash;
                size_t nNextColon = rCID.find( ':', nIndexStart );
                if( nNextColon == std::u16string_view::npos || nNextColon < nNextSlash )
                    nIndexEnd = nNextColon;
                aRet = rCID.substr(nIndexStart,nIndexEnd-nIndexStart);
            }
        }
    }
    return aRet;
}

std::u16string_view ObjectIdentifier::getDragParameterString( std::u16string_view rCID )
{
    std::u16string_view aRet;

    size_t nIndexStart = rCID.find( m_aDragParameterEquals );
    if( nIndexStart != std::u16string_view::npos )
    {
        nIndexStart = rCID.find( '=', nIndexStart );
        if( nIndexStart != std::u16string_view::npos )
        {
            nIndexStart++;
            size_t nNextSlash = rCID.find( '/', nIndexStart );
            if( nNextSlash != std::u16string_view::npos )
            {
                sal_Int32 nIndexEnd = nNextSlash;
                size_t nNextColon = rCID.find( ':', nIndexStart );
                if( nNextColon == std::u16string_view::npos || nNextColon < nNextSlash )
                    nIndexEnd = nNextColon;
                aRet = rCID.substr(nIndexStart,nIndexEnd-nIndexStart);
            }
        }
    }
    return aRet;
}

bool ObjectIdentifier::isDragableObject( std::u16string_view rClassifiedIdentifier )
{
    bool bReturn = false;
    ObjectType eObjectType = ObjectIdentifier::getObjectType( rClassifiedIdentifier );
    switch( eObjectType )
    {
        case OBJECTTYPE_TITLE:
        case OBJECTTYPE_LEGEND:
        case OBJECTTYPE_DIAGRAM:
        case OBJECTTYPE_DATA_LABEL:
        case OBJECTTYPE_DATA_CURVE_EQUATION:
        //case OBJECTTYPE_DIAGRAM_WALL:
            bReturn = true;
            break;
        default:
            std::u16string_view aDragMethodServiceName( ObjectIdentifier::getDragMethodServiceName( rClassifiedIdentifier ) );
            bReturn = !aDragMethodServiceName.empty();
            break;
    }
    return bReturn;
}

bool ObjectIdentifier::isDragableObject() const
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

bool ObjectIdentifier::isRotateableObject( std::u16string_view rClassifiedIdentifier )
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

bool ObjectIdentifier::isMultiClickObject( std::u16string_view rClassifiedIdentifier )
{
    //the name of a shape is it's ClassifiedIdentifier

    //a MultiClickObject is an object that is selectable by more than one click only ;
    //before a MultiClickObject can be selected it is necessary that a named parent group object
    //was selected before;

    //!!!!! by definition the name of a MultiClickObject starts with "CID/MultiClick:"
    bool bRet = o3tl::starts_with(rClassifiedIdentifier.substr( m_aProtocol.getLength() ), m_aMultiClick);
    return bRet;
}

bool ObjectIdentifier::areSiblings( std::u16string_view rCID1, std::u16string_view rCID2 )
{
    bool bRet=false;
    size_t nLastSign1 = rCID1.rfind( '=' );
    size_t nLastSign2 = rCID2.rfind( '=' );
    if( nLastSign1 == rCID1.find( '=' ) )//CID cannot be sibling if only one "=" occurs
        bRet=false;
    else if( nLastSign2 == rCID2.find( '=' ) )//CID cannot be sibling if only one "=" occurs
        bRet=false;
    else if( ObjectIdentifier::areIdenticalObjects( rCID1, rCID2 ) )
        bRet=false;
    else
    {
        std::u16string_view aParent1( ObjectIdentifier::getFullParentParticle( rCID1 ) );
        if( !aParent1.empty() )
        {
            std::u16string_view aParent2( ObjectIdentifier::getFullParentParticle( rCID2 ) );
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

bool ObjectIdentifier::areIdenticalObjects( std::u16string_view rCID1, std::u16string_view rCID2 )
{
    if( rCID1 == rCID2 )
        return true;
    //draggable pie or donut segments need special treatment, as their CIDs do change with offset
    {
        if( rCID1.find( m_aPieSegmentDragMethodServiceName ) == std::u16string_view::npos
            || rCID2.find( m_aPieSegmentDragMethodServiceName ) == std::u16string_view::npos )
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
        case OBJECTTYPE_DATA_TABLE:
                aRet="DataTable";
                break;
        default: //OBJECTTYPE_UNKNOWN
            ;
    }
    return aRet;
}

ObjectType ObjectIdentifier::getObjectType( std::u16string_view aCID )
{
    ObjectType eRet;
    size_t nLastSign = aCID.rfind( ':' );//last sign before the type string
    if(nLastSign == std::u16string_view::npos)
        nLastSign = aCID.rfind( '/' );
    if(nLastSign == std::u16string_view::npos)
    {
        size_t nEndIndex = aCID.rfind( '=' );
        if(nEndIndex == std::u16string_view::npos)
            return OBJECTTYPE_UNKNOWN;
        nLastSign = 0;
    }
    if( nLastSign>0 )
        nLastSign++;

    aCID = aCID.substr(nLastSign);
    if( o3tl::starts_with(aCID, u"Page") )
        eRet = OBJECTTYPE_PAGE;
    else if( o3tl::starts_with(aCID, u"Title") )
        eRet = OBJECTTYPE_TITLE;
    else if( o3tl::starts_with(aCID, u"LegendEntry") )
        eRet = OBJECTTYPE_LEGEND_ENTRY;
    else if( o3tl::starts_with(aCID, u"Legend") )
        eRet = OBJECTTYPE_LEGEND;
    else if( o3tl::starts_with(aCID, u"DiagramWall") )
        eRet = OBJECTTYPE_DIAGRAM_WALL;
    else if( o3tl::starts_with(aCID, u"DiagramFloor") )
        eRet = OBJECTTYPE_DIAGRAM_FLOOR;
    else if( o3tl::starts_with(aCID, u"D=") )
        eRet = OBJECTTYPE_DIAGRAM;
    else if( o3tl::starts_with(aCID, u"AxisUnitLabel") )
        eRet = OBJECTTYPE_AXIS_UNITLABEL;
    else if( o3tl::starts_with(aCID, u"Axis") )
        eRet = OBJECTTYPE_AXIS;
    else if( o3tl::starts_with(aCID, u"Grid") )
        eRet = OBJECTTYPE_GRID;
    else if( o3tl::starts_with(aCID, u"SubGrid") )
        eRet = OBJECTTYPE_SUBGRID;
    else if( o3tl::starts_with(aCID, u"Series") )
        eRet = OBJECTTYPE_DATA_SERIES;
    else if( o3tl::starts_with(aCID, u"Point") )
        eRet = OBJECTTYPE_DATA_POINT;
    else if( o3tl::starts_with(aCID, u"DataLabels") )
        eRet = OBJECTTYPE_DATA_LABELS;
    else if( o3tl::starts_with(aCID, u"DataLabel") )
        eRet = OBJECTTYPE_DATA_LABEL;
    else if( o3tl::starts_with(aCID, u"ErrorsX") )
        eRet = OBJECTTYPE_DATA_ERRORS_X;
    else if( o3tl::starts_with(aCID, u"ErrorsY") )
        eRet = OBJECTTYPE_DATA_ERRORS_Y;
    else if( o3tl::starts_with(aCID, u"ErrorsZ") )
        eRet = OBJECTTYPE_DATA_ERRORS_Z;
    else if( o3tl::starts_with(aCID, u"Curve") )
        eRet = OBJECTTYPE_DATA_CURVE;
    else if( o3tl::starts_with(aCID, u"Equation") )
        eRet = OBJECTTYPE_DATA_CURVE_EQUATION;
    else if( o3tl::starts_with(aCID, u"Average") )
        eRet = OBJECTTYPE_DATA_AVERAGE_LINE;
    else if( o3tl::starts_with(aCID, u"StockRange") )
        eRet = OBJECTTYPE_DATA_STOCK_RANGE;
    else if( o3tl::starts_with(aCID, u"StockLoss") )
        eRet = OBJECTTYPE_DATA_STOCK_LOSS;
    else if( o3tl::starts_with(aCID, u"StockGain") )
        eRet = OBJECTTYPE_DATA_STOCK_GAIN;
    else if( o3tl::starts_with(aCID, u"DataTable") )
        eRet = OBJECTTYPE_DATA_TABLE;
    else
        eRet = OBJECTTYPE_UNKNOWN;

    return eRet;
}

ObjectType ObjectIdentifier::getObjectType() const
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
                                std::u16string_view rSeriesParticle
                                , sal_Int32 nCurveIndex
                                , bool bAverageLine )
{
    OUString aParticleID( OUString::number( nCurveIndex ) );
    ObjectType eType = bAverageLine ? OBJECTTYPE_DATA_AVERAGE_LINE : OBJECTTYPE_DATA_CURVE;
    return createClassifiedIdentifierWithParent( eType, aParticleID, rSeriesParticle );
}

OUString ObjectIdentifier::createDataCurveEquationCID(
                                std::u16string_view rSeriesParticle
                                , sal_Int32 nCurveIndex )
{
    OUString aParticleID( OUString::number( nCurveIndex ) );
    return createClassifiedIdentifierWithParent( OBJECTTYPE_DATA_CURVE_EQUATION, aParticleID, rSeriesParticle );
}

OUString ObjectIdentifier::addChildParticle( std::u16string_view rParticle, std::u16string_view rChildParticle )
{
    OUStringBuffer aRet(rParticle);

    if( !aRet.isEmpty() && !rChildParticle.empty() )
        aRet.append(":");
    if( !rChildParticle.empty() )
        aRet.append(rChildParticle);

    return aRet.makeStringAndClear();
}

OUString ObjectIdentifier::createChildParticleWithIndex( ObjectType eObjectType, sal_Int32 nIndex )
{
    OUStringBuffer aRet( getStringForType( eObjectType ) );
    if( !aRet.isEmpty() )
    {
        aRet.append("=" + OUString::number(nIndex));
    }
    return aRet.makeStringAndClear();
}

sal_Int32 ObjectIdentifier::getIndexFromParticleOrCID( std::u16string_view rParticleOrCID )
{
    const std::u16string_view aIndexString = lcl_getIndexStringAfterString( rParticleOrCID, u"=" );
    return lcl_StringToIndex( o3tl::getToken(aIndexString, 0, ',' ) );
}

OUString ObjectIdentifier::createSeriesSubObjectStub( ObjectType eSubObjectType
                    , std::u16string_view rSeriesParticle
                    , std::u16string_view rDragMethodServiceName
                    , std::u16string_view rDragParameterString )
{
    OUString aChildParticle = getStringForType( eSubObjectType ) + "=";

    return createClassifiedIdentifierForParticles(
            rSeriesParticle, aChildParticle
          , rDragMethodServiceName, rDragParameterString );
}

OUString ObjectIdentifier::createPointCID( std::u16string_view rPointCID_Stub, sal_Int32 nIndex  )
{
    return rPointCID_Stub + OUString::number( nIndex );
}

std::u16string_view ObjectIdentifier::getParticleID( std::u16string_view rCID )
{
    std::u16string_view aRet;
    size_t nLast = rCID.rfind('=');
    if(nLast != std::u16string_view::npos)
        aRet = rCID.substr(++nLast);
    return aRet;
}

std::u16string_view ObjectIdentifier::getFullParentParticle( std::u16string_view rCID )
{
    std::u16string_view aRet;

    size_t nStartPos = rCID.rfind('/');
    if( nStartPos != std::u16string_view::npos )
    {
        nStartPos++;
        size_t nEndPos = rCID.rfind(':');
        if( nEndPos != std::u16string_view::npos && nStartPos < nEndPos )
        {
            aRet = rCID.substr(nStartPos,nEndPos-nStartPos);
        }
    }

    return aRet;
}

OUString ObjectIdentifier::getObjectID( std::u16string_view rCID )
{
    OUString aRet;

    size_t nStartPos = rCID.rfind('/');
    if( nStartPos != std::u16string_view::npos )
    {
        nStartPos++;
        size_t nEndPos = rCID.size();
        aRet = rCID.substr(nStartPos,nEndPos-nStartPos);
    }

    return aRet;
}

bool ObjectIdentifier::isCID( std::u16string_view rName )
{
    return !rName.empty() && o3tl::starts_with( rName, m_aProtocol );
}

Reference< beans::XPropertySet > ObjectIdentifier::getObjectPropertySet(
                std::u16string_view rObjectCID
                , const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    //return the model object that is indicated by rObjectCID
    if(rObjectCID.empty())
        return nullptr;
    if(!xChartModel.is())
        return nullptr;

    Reference< beans::XPropertySet > xObjectProperties;
    try
    {
        ObjectType eObjectType = ObjectIdentifier::getObjectType( rObjectCID );
        std::u16string_view aParticleID = ObjectIdentifier::getParticleID( rObjectCID );

        rtl::Reference< Diagram > xDiagram;
        rtl::Reference< BaseCoordinateSystem > xCooSys;
        lcl_getDiagramAndCooSys( rObjectCID, xChartModel, xDiagram, xCooSys );

        switch(eObjectType)
        {
            case OBJECTTYPE_PAGE:
                {
                    xObjectProperties.set( xChartModel->getPageBackground() );
                }
                break;
            case OBJECTTYPE_TITLE:
                {
                    TitleHelper::eTitleType aTitleType = getTitleTypeForCID( rObjectCID );
                    rtl::Reference< Title > xTitle( TitleHelper::getTitle( aTitleType, xChartModel ) );
                    xObjectProperties = xTitle;
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
                    xObjectProperties = xDiagram;
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

                    rtl::Reference< Axis > xAxis =
                        AxisHelper::getAxis( nDimensionIndex, nAxisIndex, xCooSys );
                    if( xAxis.is() )
                        xObjectProperties = xAxis;
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

                    xObjectProperties = AxisHelper::getGridProperties( xCooSys , nDimensionIndex, nAxisIndex, nSubGridIndex );
                }
                break;
            case OBJECTTYPE_DATA_LABELS:
            case OBJECTTYPE_DATA_SERIES:
                {
                    rtl::Reference< DataSeries > xSeries( ObjectIdentifier::getDataSeriesForCID(
                        rObjectCID, xChartModel ) );
                    if( xSeries.is() )
                        xObjectProperties = xSeries;

                    break;
                }
            case OBJECTTYPE_DATA_LABEL:
            case OBJECTTYPE_DATA_POINT:
                {
                    rtl::Reference< DataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID(
                        rObjectCID, xChartModel );
                    if(xSeries.is())
                    {
                        sal_Int32 nIndex = o3tl::toInt32(aParticleID);
                        xObjectProperties = xSeries->getDataPointByIndex( nIndex );
                    }
                    break;
                }
            case OBJECTTYPE_DATA_ERRORS_X:
            case OBJECTTYPE_DATA_ERRORS_Y:
            case OBJECTTYPE_DATA_ERRORS_Z:
                {
                    rtl::Reference< DataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID(
                        rObjectCID, xChartModel );
                    if(xSeries.is())
                    {
                        Reference< beans::XPropertySet > xErrorBarProp;
                        OUString errorBar;

                        if ( eObjectType == OBJECTTYPE_DATA_ERRORS_X)
                            errorBar = CHART_UNONAME_ERRORBAR_X;
                        else if (eObjectType == OBJECTTYPE_DATA_ERRORS_Y)
                            errorBar = CHART_UNONAME_ERRORBAR_Y;
                        else
                            errorBar = "ErrorBarZ";

                        xSeries->getPropertyValue( errorBar ) >>= xErrorBarProp;
                        xObjectProperties = xErrorBarProp;
                    }
                    break;
                }
            case OBJECTTYPE_DATA_AVERAGE_LINE:
            case OBJECTTYPE_DATA_CURVE:
            case OBJECTTYPE_DATA_CURVE_EQUATION:
                {
                    rtl::Reference< DataSeries > xRegressionContainer = ObjectIdentifier::getDataSeriesForCID(
                        rObjectCID, xChartModel );
                    if(xRegressionContainer.is())
                    {
                        sal_Int32 nIndex = o3tl::toInt32(aParticleID);
                        const std::vector< rtl::Reference< RegressionCurveModel > > & aCurveList =
                            xRegressionContainer->getRegressionCurves2();
                        if( nIndex >= 0 && o3tl::make_unsigned(nIndex) < aCurveList.size() )
                        {
                            if( eObjectType == OBJECTTYPE_DATA_CURVE_EQUATION )
                                xObjectProperties = aCurveList[nIndex]->getEquationProperties();
                            else
                                xObjectProperties = aCurveList[nIndex];
                        }
                    }
                    break;
                }
            case OBJECTTYPE_DATA_STOCK_RANGE:
                    break;
            case OBJECTTYPE_DATA_STOCK_LOSS:
                    {
                        rtl::Reference<ChartType> xChartType( lcl_getFirstStockChartType( xChartModel ) );
                        if(xChartType.is())
                            xChartType->getPropertyValue( u"BlackDay"_ustr ) >>= xObjectProperties;
                    }
                    break;
            case OBJECTTYPE_DATA_STOCK_GAIN:
                    {
                        rtl::Reference<ChartType> xChartType( lcl_getFirstStockChartType( xChartModel ) );
                        if(xChartType.is())
                            xChartType->getPropertyValue( u"WhiteDay"_ustr ) >>= xObjectProperties;
                    }
                    break;
            case OBJECTTYPE_DATA_TABLE:
                    {
                        if (xDiagram.is())
                            xObjectProperties.set(xDiagram->getDataTable(), uno::UNO_QUERY);
                    }
                break;
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

rtl::Reference< Axis > ObjectIdentifier::getAxisForCID(
                std::u16string_view rObjectCID
                , const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    rtl::Reference< Diagram > xDiagram;
    rtl::Reference< BaseCoordinateSystem > xCooSys;
    lcl_getDiagramAndCooSys( rObjectCID, xChartModel, xDiagram, xCooSys );

    sal_Int32 nDimensionIndex = -1;
    sal_Int32 nAxisIndex = -1;
    lcl_parseAxisIndices( nDimensionIndex, nAxisIndex, rObjectCID );

    return AxisHelper::getAxis( nDimensionIndex, nAxisIndex, xCooSys );
}

rtl::Reference< DataSeries > ObjectIdentifier::getDataSeriesForCID(
                std::u16string_view rObjectCID
                , const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    rtl::Reference< Diagram > xDiagram;
    rtl::Reference< BaseCoordinateSystem > xCooSys;
    lcl_getDiagramAndCooSys( rObjectCID, xChartModel, xDiagram, xCooSys );

    sal_Int32 nChartTypeIndex = -1;
    sal_Int32 nSeriesIndex = -1;
    sal_Int32 nPointIndex = -1;
    lcl_parseSeriesIndices( nChartTypeIndex, nSeriesIndex, nPointIndex, rObjectCID );

    rtl::Reference< DataSeries > xSeries;
    if (xDiagram)
    {
        rtl::Reference< ChartType > xDataSeriesContainer( xDiagram->getChartTypeByIndex( nChartTypeIndex ) );
        if( xDataSeriesContainer.is() )
        {
            const std::vector< rtl::Reference< DataSeries > > & aDataSeriesSeq( xDataSeriesContainer->getDataSeries2() );
            if( nSeriesIndex >= 0 && o3tl::make_unsigned(nSeriesIndex) < aDataSeriesSeq.size() )
                xSeries = aDataSeriesSeq[nSeriesIndex];
        }
    }
    return xSeries;
}

rtl::Reference< Diagram > ObjectIdentifier::getDiagramForCID(
                  std::u16string_view rObjectCID
                , const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    rtl::Reference< Diagram > xDiagram;
    rtl::Reference< BaseCoordinateSystem > xCooSys;
    lcl_getDiagramAndCooSys( rObjectCID, xChartModel, xDiagram, xCooSys );

    return xDiagram;
}

TitleHelper::eTitleType ObjectIdentifier::getTitleTypeForCID( std::u16string_view rCID )
{
    TitleHelper::eTitleType eRet( TitleHelper::MAIN_TITLE );

    std::u16string_view aParentParticle = ObjectIdentifier::getFullParentParticle( rCID );
    const tTitleMap& rMap = lcl_getTitleMap();
    tTitleMap::const_iterator aIt = std::find_if(rMap.begin(), rMap.end(),
        [&aParentParticle](tTitleMap::const_reference rEntry) { return aParentParticle == rEntry.second; });
    if (aIt != rMap.end())
        eRet = (*aIt).first;

    return eRet;
}

OUString ObjectIdentifier::getSeriesParticleFromCID( std::u16string_view rCID )
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

OUString ObjectIdentifier::getMovedSeriesCID( std::u16string_view rObjectCID, bool bForward )
{
    sal_Int32 nDiagramIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rObjectCID, u"CID/D=" ) );
    sal_Int32 nCooSysIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rObjectCID, u"CS=" ) );
    sal_Int32 nChartTypeIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rObjectCID, u"CT=" ) );
    sal_Int32 nSeriesIndex = lcl_StringToIndex( lcl_getIndexStringAfterString( rObjectCID, u"Series=" ) );

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
