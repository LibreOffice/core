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

#include <sax/tools/converter.hxx>

#include "SchXMLAxisContext.hxx"
#include "SchXMLChartContext.hxx"
#include "SchXMLTools.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>

#include <tools/color.hxx>

#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart/ChartAxisType.hpp>
#include <com/sun/star/chart/TimeIncrement.hpp>
#include <com/sun/star/chart/TimeInterval.hpp>
#include <com/sun/star/chart/TimeUnit.hpp>
#include <com/sun/star/chart/XAxis.hpp>
#include <com/sun/star/chart/XAxisSupplier.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>

#include <com/sun/star/drawing/LineStyle.hpp>

using namespace ::xmloff::token;
using namespace com::sun::star;

using com::sun::star::uno::Reference;

static const SvXMLEnumMapEntry aXMLAxisDimensionMap[] =
{
    { XML_X,  SCH_XML_AXIS_X  },
    { XML_Y,  SCH_XML_AXIS_Y  },
    { XML_Z,  SCH_XML_AXIS_Z  },
    { XML_TOKEN_INVALID, 0 }
};

static const SvXMLEnumMapEntry aXMLAxisTypeMap[] =
{
    { XML_AUTO,  ::com::sun::star::chart::ChartAxisType::AUTOMATIC },
    { XML_TEXT,  ::com::sun::star::chart::ChartAxisType::CATEGORY },
    { XML_DATE,  ::com::sun::star::chart::ChartAxisType::DATE },
    { XML_TOKEN_INVALID, 0 }
};

class SchXMLCategoriesContext : public SvXMLImportContext
{
private:
    OUString& mrAddress;

public:
    SchXMLCategoriesContext( SvXMLImport& rImport,
                                   sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   OUString& rAddress );
    virtual ~SchXMLCategoriesContext();
    virtual void StartElement( const Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

class DateScaleContext : public SvXMLImportContext
{
public:
    DateScaleContext( SvXMLImport& rImport,
                        sal_uInt16 nPrefix, const OUString& rLocalName,
                        const Reference< beans::XPropertySet > xAxisProps );

    virtual ~DateScaleContext();
    virtual void StartElement( const Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    Reference< beans::XPropertySet > m_xAxisProps;
};

SchXMLAxisContext::SchXMLAxisContext( SchXMLImportHelper& rImpHelper,
                                      SvXMLImport& rImport, const OUString& rLocalName,
                                      Reference< chart::XDiagram > xDiagram,
                                      std::vector< SchXMLAxis >& rAxes,
                                      OUString & rCategoriesAddress,
                                      bool bAddMissingXAxisForNetCharts,
                                      bool bAdaptWrongPercentScaleValues,
                                      bool bAdaptXAxisOrientationForOld2DBarCharts,
                                      bool& rbAxisPositionAttributeImported ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        m_rImportHelper( rImpHelper ),
        m_xDiagram( xDiagram ),
        m_rAxes( rAxes ),
        m_rCategoriesAddress( rCategoriesAddress ),
        m_nAxisType(chart::ChartAxisType::AUTOMATIC),
        m_bAxisTypeImported(false),
        m_bDateScaleImported(false),
        m_bAddMissingXAxisForNetCharts( bAddMissingXAxisForNetCharts ),
        m_bAdaptWrongPercentScaleValues( bAdaptWrongPercentScaleValues ),
        m_bAdaptXAxisOrientationForOld2DBarCharts( bAdaptXAxisOrientationForOld2DBarCharts ),
        m_rbAxisPositionAttributeImported( rbAxisPositionAttributeImported )
{
}

SchXMLAxisContext::~SchXMLAxisContext()
{}

static Reference< chart::XAxis > lcl_getChartAxis( SchXMLAxis aCurrentAxis, const Reference< chart::XDiagram > xDiagram )
{
    Reference< chart::XAxis > xAxis;
    Reference< chart::XAxisSupplier > xAxisSuppl( xDiagram, uno::UNO_QUERY );
    if( !xAxisSuppl.is() )
        return xAxis;
    if( aCurrentAxis.nAxisIndex == 0 )
        xAxis = xAxisSuppl->getAxis(aCurrentAxis.eDimension);
    else
        xAxis = xAxisSuppl->getSecondaryAxis(aCurrentAxis.eDimension);
    return xAxis;
}

/* returns a shape for the current axis's title. The property
   "Has...AxisTitle" is set to "True" to get the shape
 */
Reference< drawing::XShape > SchXMLAxisContext::getTitleShape()
{
    Reference< drawing::XShape > xResult;
    Reference< beans::XPropertySet > xDiaProp( m_rImportHelper.GetChartDocument()->getDiagram(), uno::UNO_QUERY );
    Reference< chart::XAxis > xAxis( lcl_getChartAxis( m_aCurrentAxis, m_xDiagram ) );
    if( !xDiaProp.is() || !xAxis.is() )
        return xResult;

    OUString aPropName;
    switch( m_aCurrentAxis.eDimension )
    {
        case SCH_XML_AXIS_X:
            if( m_aCurrentAxis.nAxisIndex == 0 )
                aPropName = "HasXAxisTitle";
            else
                aPropName = "HasSecondaryXAxisTitle";
            break;
        case SCH_XML_AXIS_Y:
            if( m_aCurrentAxis.nAxisIndex == 0 )
                aPropName = "HasYAxisTitle";
            else
                aPropName = "HasSecondaryYAxisTitle";
            break;
        case SCH_XML_AXIS_Z:
            aPropName = "HasZAxisTitle";
            break;
        case SCH_XML_AXIS_UNDEF:
            SAL_INFO("xmloff.chart", "Invalid axis" );
            break;
    }
    xDiaProp->setPropertyValue( aPropName, uno::makeAny(sal_True) );
    xResult = Reference< drawing::XShape >( xAxis->getAxisTitle(), uno::UNO_QUERY );
    return xResult;
}

void SchXMLAxisContext::CreateGrid( OUString sAutoStyleName, bool bIsMajor )
{
    Reference< beans::XPropertySet > xDiaProp( m_rImportHelper.GetChartDocument()->getDiagram(), uno::UNO_QUERY );
    Reference< chart::XAxis > xAxis( lcl_getChartAxis( m_aCurrentAxis, m_xDiagram ) );
    if( !xDiaProp.is() || !xAxis.is() )
        return;

    OUString aPropName;
    switch( m_aCurrentAxis.eDimension )
    {
        case SCH_XML_AXIS_X:
            if( bIsMajor )
                aPropName = "HasXAxisGrid";
            else
                aPropName = "HasXAxisHelpGrid";
            break;
        case SCH_XML_AXIS_Y:
            if( bIsMajor )
                aPropName = "HasYAxisGrid";
            else
                aPropName = "HasYAxisHelpGrid";
            break;
        case SCH_XML_AXIS_Z:
            if( bIsMajor )
                aPropName = "HasZAxisGrid";
            else
                aPropName = "HasZAxisHelpGrid";
            break;
        case SCH_XML_AXIS_UNDEF:
            SAL_INFO("xmloff.chart", "Invalid axis" );
            break;
    }
    xDiaProp->setPropertyValue( aPropName, uno::makeAny(sal_True) );

    Reference< beans::XPropertySet > xGridProp;
    if( bIsMajor )
        xGridProp = xAxis->getMajorGrid();
    else
        xGridProp = xAxis->getMinorGrid();

    // set properties
    if( xGridProp.is())
    {
        // the line color is black as default, in the model it is a light gray
        xGridProp->setPropertyValue("LineColor",
                                     uno::makeAny( COL_BLACK ));
        if( !sAutoStyleName.isEmpty())
        {
            const SvXMLStylesContext* pStylesCtxt = m_rImportHelper.GetAutoStylesContext();
            if( pStylesCtxt )
            {
                const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                    m_rImportHelper.GetChartFamilyID(), sAutoStyleName );

                if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                    (( XMLPropStyleContext* )pStyle )->FillPropertySet( xGridProp );
            }
        }
    }
}

namespace
{
enum AxisAttributeTokens
{
    XML_TOK_AXIS_DIMENSION,
    XML_TOK_AXIS_NAME,
    XML_TOK_AXIS_STYLE_NAME,
    XML_TOK_AXIS_TYPE,
    XML_TOK_AXIS_TYPE_EXT
};

SvXMLTokenMapEntry aAxisAttributeTokenMap[] =
{
    { XML_NAMESPACE_CHART,      XML_DIMENSION,  XML_TOK_AXIS_DIMENSION      },
    { XML_NAMESPACE_CHART,      XML_NAME,       XML_TOK_AXIS_NAME           },
    { XML_NAMESPACE_CHART,      XML_STYLE_NAME, XML_TOK_AXIS_STYLE_NAME     },
    { XML_NAMESPACE_CHART,      XML_AXIS_TYPE,  XML_TOK_AXIS_TYPE           },
    { XML_NAMESPACE_CHART_EXT,  XML_AXIS_TYPE,  XML_TOK_AXIS_TYPE_EXT       },
    XML_TOKEN_MAP_END
};

class AxisAttributeTokenMap : public SvXMLTokenMap
{
public:
    AxisAttributeTokenMap(): SvXMLTokenMap( aAxisAttributeTokenMap ) {}
    virtual ~AxisAttributeTokenMap() {}
};

//a AxisAttributeTokenMap Singleton
struct theAxisAttributeTokenMap : public rtl::Static< AxisAttributeTokenMap, theAxisAttributeTokenMap > {};
}

void SchXMLAxisContext::StartElement( const Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    SchXMLImport& rImport = ( SchXMLImport& )GetImport();
    const SvXMLTokenMap& rAttrTokenMap = theAxisAttributeTokenMap::get();

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_AXIS_DIMENSION:
                {
                    sal_uInt16 nEnumVal;
                    if( rImport.GetMM100UnitConverter().convertEnum( nEnumVal, aValue, aXMLAxisDimensionMap ))
                        m_aCurrentAxis.eDimension = ( SchXMLAxisDimension )nEnumVal;
                }
                break;
            case XML_TOK_AXIS_NAME:
                m_aCurrentAxis.aName = aValue;
                break;
            case XML_TOK_AXIS_TYPE:
            case XML_TOK_AXIS_TYPE_EXT:
                sal_uInt16 nEnumVal;
                if( rImport.GetMM100UnitConverter().convertEnum( nEnumVal, aValue, aXMLAxisTypeMap ))
                {
                    m_nAxisType = nEnumVal;
                    m_bAxisTypeImported = true;
                }
                break;
            case XML_TOK_AXIS_STYLE_NAME:
                m_aAutoStyleName = aValue;
                break;
        }
    }

    // check for number of axes with same dimension
    m_aCurrentAxis.nAxisIndex = 0;
    sal_Int32 nNumOfAxes = m_rAxes.size();
    for( sal_Int32 nCurrent = 0; nCurrent < nNumOfAxes; nCurrent++ )
    {
        if( m_rAxes[ nCurrent ].eDimension == m_aCurrentAxis.eDimension )
            m_aCurrentAxis.nAxisIndex++;
    }
    CreateAxis();
}
namespace
{

Reference< chart2::XAxis > lcl_getAxis( const Reference< frame::XModel >& xChartModel,
                                            sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    Reference< chart2::XAxis > xAxis;

    try
    {
        Reference< chart2::XChartDocument > xChart2Document( xChartModel, uno::UNO_QUERY );
        if( xChart2Document.is() )
        {
            Reference< chart2::XDiagram > xDiagram( xChart2Document->getFirstDiagram());
            Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY_THROW );
            uno::Sequence< Reference< chart2::XCoordinateSystem > >
                aCooSysSeq( xCooSysCnt->getCoordinateSystems());
            sal_Int32 nCooSysIndex = 0;
            if( nCooSysIndex < aCooSysSeq.getLength() )
            {
                Reference< chart2::XCoordinateSystem > xCooSys( aCooSysSeq[nCooSysIndex] );
                if( xCooSys.is() && nDimensionIndex < xCooSys->getDimension() )
                {
                    const sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
                    if( nAxisIndex <= nMaxAxisIndex )
                        xAxis = xCooSys->getAxisByDimension( nDimensionIndex, nAxisIndex );
                }
            }
        }
    }
    catch( uno::Exception & )
    {
        SAL_INFO("xmloff.chart", "Couldn't get axis" );
    }

    return xAxis;
}

bool lcl_divideBy100( uno::Any& rDoubleAny )
{
    bool bChanged = false;
    double fValue=0.0;
    if( (rDoubleAny>>=fValue) && (fValue!=0.0) )
    {
        fValue/=100.0;
        rDoubleAny = uno::makeAny(fValue);
        bChanged = true;
    }
    return bChanged;
}

bool lcl_AdaptWrongPercentScaleValues(chart2::ScaleData& rScaleData)
{
    bool bChanged = lcl_divideBy100( rScaleData.Minimum );
    bChanged = lcl_divideBy100( rScaleData.Maximum ) || bChanged;
    bChanged = lcl_divideBy100( rScaleData.Origin ) || bChanged;
    bChanged = lcl_divideBy100( rScaleData.IncrementData.Distance ) || bChanged;
    return bChanged;
}

}//end anonymous namespace

void SchXMLAxisContext::CreateAxis()
{
    m_rAxes.push_back( m_aCurrentAxis );

    Reference< beans::XPropertySet > xDiaProp( m_rImportHelper.GetChartDocument()->getDiagram(), uno::UNO_QUERY );
    if( !xDiaProp.is() )
        return;
    OUString aPropName;
    switch( m_aCurrentAxis.eDimension )
    {
        case SCH_XML_AXIS_X:
            if( m_aCurrentAxis.nAxisIndex == 0 )
                aPropName = "HasXAxis";
            else
                aPropName = "HasSecondaryXAxis";
            break;
        case SCH_XML_AXIS_Y:
            if( m_aCurrentAxis.nAxisIndex == 0 )
                aPropName = "HasYAxis";
            else
                aPropName = "HasSecondaryYAxis";
            break;
        case SCH_XML_AXIS_Z:
            if( m_aCurrentAxis.nAxisIndex == 0 )
                aPropName = "HasXAxis";
            else
                aPropName = "HasSecondaryXAxis";
            break;
        case SCH_XML_AXIS_UNDEF:
            SAL_INFO("xmloff.chart", "Invalid axis" );
            break;
    }
    try
    {
        xDiaProp->setPropertyValue( aPropName, uno::makeAny(sal_True) );
    }
    catch( beans::UnknownPropertyException & )
    {
        SAL_INFO("xmloff.chart", "Couldn't turn on axis" );
    }
    if( m_aCurrentAxis.eDimension==SCH_XML_AXIS_Z )
    {
        bool bSettingZAxisSuccedded = false;
        try
        {
            xDiaProp->getPropertyValue( aPropName ) >>= bSettingZAxisSuccedded;
        }
        catch( beans::UnknownPropertyException & )
        {
            SAL_INFO("xmloff.chart", "Couldn't turn on z axis" );
        }
        if( !bSettingZAxisSuccedded )
            return;
    }

    m_xAxisProps = Reference<beans::XPropertySet>( lcl_getChartAxis( m_aCurrentAxis, m_xDiagram ), uno::UNO_QUERY );

    if( m_bAddMissingXAxisForNetCharts && m_aCurrentAxis.eDimension==SCH_XML_AXIS_Y && m_aCurrentAxis.nAxisIndex==0 )
    {
        try
        {
            xDiaProp->setPropertyValue("HasXAxis", uno::makeAny(sal_True) );
        }
        catch( beans::UnknownPropertyException & )
        {
            SAL_INFO("xmloff.chart", "Couldn't turn on x axis" );
        }
    }

    // set properties
    if( m_xAxisProps.is())
    {
        uno::Any aTrueBool( uno::makeAny( sal_True ));
        uno::Any aFalseBool( uno::makeAny( sal_False ));

        // #i109879# the line color is black as default, in the model it is a light gray
        m_xAxisProps->setPropertyValue("LineColor",
                                     uno::makeAny( COL_BLACK ));

        m_xAxisProps->setPropertyValue("DisplayLabels", aFalseBool );

        // #88077# AutoOrigin 'on' is default
        m_xAxisProps->setPropertyValue("AutoOrigin", aTrueBool );

        if( m_bAxisTypeImported )
            m_xAxisProps->setPropertyValue("AxisType", uno::makeAny(m_nAxisType) );

        if( !m_aAutoStyleName.isEmpty())
        {
            const SvXMLStylesContext* pStylesCtxt = m_rImportHelper.GetAutoStylesContext();
            if( pStylesCtxt )
            {
                const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                    m_rImportHelper.GetChartFamilyID(), m_aAutoStyleName );

                if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                {
                    // note: SvXMLStyleContext::FillPropertySet is not const
                    XMLPropStyleContext * pPropStyleContext = const_cast< XMLPropStyleContext * >( dynamic_cast< const XMLPropStyleContext * >( pStyle ));
                    if( pPropStyleContext )
                        pPropStyleContext->FillPropertySet( m_xAxisProps );

                    if( m_bAdaptWrongPercentScaleValues && m_aCurrentAxis.eDimension==SCH_XML_AXIS_Y )
                    {
                        //set scale data of added x axis back to default
                        Reference< chart2::XAxis > xAxis( lcl_getAxis( GetImport().GetModel(),
                                            m_aCurrentAxis.eDimension, m_aCurrentAxis.nAxisIndex ) );
                        if( xAxis.is() )
                        {
                            chart2::ScaleData aScaleData( xAxis->getScaleData());
                            if( lcl_AdaptWrongPercentScaleValues(aScaleData) )
                                xAxis->setScaleData( aScaleData );
                        }
                    }

                    if( m_bAddMissingXAxisForNetCharts )
                    {
                        //copy style from y axis to added x axis:

                        Reference< chart::XAxisSupplier > xAxisSuppl( xDiaProp, uno::UNO_QUERY );
                        if( xAxisSuppl.is() )
                        {
                            Reference< beans::XPropertySet > xXAxisProp( xAxisSuppl->getAxis(0), uno::UNO_QUERY );
                            (( XMLPropStyleContext* )pStyle )->FillPropertySet( xXAxisProp );
                        }

                        //set scale data of added x axis back to default
                        Reference< chart2::XAxis > xAxis( lcl_getAxis( GetImport().GetModel(),
                                            0 /*nDimensionIndex*/, 0 /*nAxisIndex*/ ) );
                        if( xAxis.is() )
                        {
                            chart2::ScaleData aScaleData;
                            aScaleData.AxisType = chart2::AxisType::CATEGORY;
                            aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
                            xAxis->setScaleData( aScaleData );
                        }

                        //set line style of added x axis to invisible
                        Reference< beans::XPropertySet > xNewAxisProp( xAxis, uno::UNO_QUERY );
                        if( xNewAxisProp.is() )
                        {
                            xNewAxisProp->setPropertyValue("LineStyle"
                                , uno::makeAny(drawing::LineStyle_NONE));
                        }
                    }

                    if( m_bAdaptXAxisOrientationForOld2DBarCharts && m_aCurrentAxis.eDimension == SCH_XML_AXIS_X )
                    {
                        bool bIs3DChart = false;
                        if( xDiaProp.is() && ( xDiaProp->getPropertyValue("Dim3D") >>= bIs3DChart )
                            && !bIs3DChart )
                        {
                            Reference< chart2::XChartDocument > xChart2Document( GetImport().GetModel(), uno::UNO_QUERY );
                            if( xChart2Document.is() )
                            {
                                Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xChart2Document->getFirstDiagram(), uno::UNO_QUERY );
                                if( xCooSysCnt.is() )
                                {
                                    uno::Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems() );
                                    if( aCooSysSeq.getLength() )
                                    {
                                        bool bSwapXandYAxis = false;
                                        Reference< chart2::XCoordinateSystem > xCooSys( aCooSysSeq[0] );
                                        Reference< beans::XPropertySet > xCooSysProp( xCooSys, uno::UNO_QUERY );
                                        if( xCooSysProp.is() && ( xCooSysProp->getPropertyValue("SwapXAndYAxis") >>= bSwapXandYAxis )
                                            && bSwapXandYAxis )
                                        {
                                            Reference< chart2::XAxis > xAxis = xCooSys->getAxisByDimension( 0, m_aCurrentAxis.nAxisIndex );
                                            if( xAxis.is() )
                                            {
                                                chart2::ScaleData aScaleData = xAxis->getScaleData();
                                                aScaleData.Orientation = chart2::AxisOrientation_REVERSE;
                                                xAxis->setScaleData( aScaleData );
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    m_rbAxisPositionAttributeImported = m_rbAxisPositionAttributeImported || SchXMLTools::getPropertyFromContext(
                        OUString("CrossoverPosition"), pPropStyleContext, pStylesCtxt ).hasValue();
                }
            }
        }
    }
}

void SchXMLAxisContext::SetAxisTitle()
{
    if( m_aCurrentAxis.aTitle.isEmpty() )
        return;

    Reference< chart::XAxis > xAxis( lcl_getChartAxis( m_aCurrentAxis, m_xDiagram ) );
    if( !xAxis.is() )
        return;

    Reference< beans::XPropertySet > xTitleProp( xAxis->getAxisTitle() );
    if( xTitleProp.is() )
    {
        try
        {
            xTitleProp->setPropertyValue("String", uno::makeAny(m_aCurrentAxis.aTitle) );
        }
        catch( beans::UnknownPropertyException & )
        {
            SAL_INFO("xmloff.chart", "Property String for Title not available" );
        }
    }
}

namespace
{
enum AxisChildTokens
{
    XML_TOK_AXIS_TITLE,
    XML_TOK_AXIS_CATEGORIES,
    XML_TOK_AXIS_GRID,
    XML_TOK_AXIS_DATE_SCALE,
    XML_TOK_AXIS_DATE_SCALE_EXT
};

SvXMLTokenMapEntry aAxisChildTokenMap[] =
{
    { XML_NAMESPACE_CHART,      XML_TITLE,              XML_TOK_AXIS_TITLE          },
    { XML_NAMESPACE_CHART,      XML_CATEGORIES,         XML_TOK_AXIS_CATEGORIES     },
    { XML_NAMESPACE_CHART,      XML_GRID,               XML_TOK_AXIS_GRID           },
    { XML_NAMESPACE_CHART,      XML_DATE_SCALE,         XML_TOK_AXIS_DATE_SCALE     },
    { XML_NAMESPACE_CHART_EXT,  XML_DATE_SCALE,         XML_TOK_AXIS_DATE_SCALE_EXT },
    XML_TOKEN_MAP_END
};

class AxisChildTokenMap : public SvXMLTokenMap
{
public:
    AxisChildTokenMap(): SvXMLTokenMap( aAxisChildTokenMap ) {}
    virtual ~AxisChildTokenMap() {}
};

//a AxisChildTokenMap Singleton
struct theAxisChildTokenMap : public rtl::Static< AxisChildTokenMap, theAxisChildTokenMap > {};
}

SvXMLImportContext* SchXMLAxisContext::CreateChildContext(
    sal_uInt16 p_nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = theAxisChildTokenMap::get();

    switch( rTokenMap.Get( p_nPrefix, rLocalName ))
    {
        case XML_TOK_AXIS_TITLE:
        {
            Reference< drawing::XShape > xTitleShape = getTitleShape();
            pContext = new SchXMLTitleContext( m_rImportHelper, GetImport(), rLocalName,
                                               m_aCurrentAxis.aTitle,
                                               xTitleShape );
        }
        break;

        case XML_TOK_AXIS_CATEGORIES:
            pContext = new SchXMLCategoriesContext( GetImport(),
                                                          p_nPrefix, rLocalName,
                                                          m_rCategoriesAddress );
            m_aCurrentAxis.bHasCategories = true;
            break;

        case XML_TOK_AXIS_DATE_SCALE:
        case XML_TOK_AXIS_DATE_SCALE_EXT:
            pContext = new DateScaleContext( GetImport(),
                            p_nPrefix, rLocalName, m_xAxisProps );
            m_bDateScaleImported = true;
            break;

        case XML_TOK_AXIS_GRID:
        {
            sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
            bool bIsMajor = true;       // default value for class is "major"
            OUString sAutoStyleName;

            for( sal_Int16 i = 0; i < nAttrCount; i++ )
            {
                OUString sAttrName = xAttrList->getNameByIndex( i );
                OUString aLocalName;
                sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

                if( nPrefix == XML_NAMESPACE_CHART )
                {
                    if( IsXMLToken( aLocalName, XML_CLASS ) )
                    {
                        if( IsXMLToken( xAttrList->getValueByIndex( i ), XML_MINOR ) )
                            bIsMajor = false;
                    }
                    else if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                        sAutoStyleName = xAttrList->getValueByIndex( i );
                }
            }

            CreateGrid( sAutoStyleName, bIsMajor );

            // don't create a context => use default context. grid elements are empty
            pContext = new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );
        }
        break;

        default:
            pContext = new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );
            break;
    }

    return pContext;
}

void SchXMLAxisContext::EndElement()
{
    if( !m_bDateScaleImported && m_nAxisType==chart::ChartAxisType::AUTOMATIC )
    {
        Reference< chart2::XAxis > xAxis( lcl_getAxis( GetImport().GetModel(), m_aCurrentAxis.eDimension, m_aCurrentAxis.nAxisIndex ) );
        if( xAxis.is() )
        {
            chart2::ScaleData aScaleData( xAxis->getScaleData());
            aScaleData.AutoDateAxis = false;//different default for older documents
            xAxis->setScaleData( aScaleData );
        }
    }

    SetAxisTitle();
}

namespace
{

Reference< chart2::XAxis > lcl_getAxis( const Reference< chart2::XCoordinateSystem > xCooSys, sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    Reference< chart2::XAxis > xAxis;
    try
    {
        xAxis = xCooSys->getAxisByDimension( nDimensionIndex, nAxisIndex );
    }
    catch( uno::Exception & )
    {
    }
    return xAxis;
}

} // anonymous namespace

void SchXMLAxisContext::CorrectAxisPositions( const Reference< chart2::XChartDocument >& xNewDoc,
                          const OUString& rChartTypeServiceName,
                          const OUString& rODFVersionOfFile,
                          bool bAxisPositionAttributeImported )
{
    if( ( rODFVersionOfFile.isEmpty() || rODFVersionOfFile == "1.0" || rODFVersionOfFile == "1.1"
        || ( rODFVersionOfFile == "1.2" && !bAxisPositionAttributeImported ) ) )
    {
        try
        {
            Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xNewDoc->getFirstDiagram(), uno::UNO_QUERY_THROW );
            uno::Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems());
            if( aCooSysSeq.getLength() )
            {
                Reference< chart2::XCoordinateSystem > xCooSys( aCooSysSeq[0] );
                if( xCooSys.is() )
                {
                    Reference< chart2::XAxis > xMainXAxis = lcl_getAxis( xCooSys, 0, 0 );
                    Reference< chart2::XAxis > xMainYAxis = lcl_getAxis( xCooSys, 1, 0 );
                    //Reference< chart2::XAxis > xMajorZAxis = lcl_getAxis( xCooSys, 2, 0 );
                    Reference< chart2::XAxis > xSecondaryXAxis = lcl_getAxis( xCooSys, 0, 1 );
                    Reference< chart2::XAxis > xSecondaryYAxis = lcl_getAxis( xCooSys, 1, 1 );

                    Reference< beans::XPropertySet > xMainXAxisProp( xMainXAxis, uno::UNO_QUERY );
                    Reference< beans::XPropertySet > xMainYAxisProp( xMainYAxis, uno::UNO_QUERY );
                    Reference< beans::XPropertySet > xSecondaryXAxisProp( xSecondaryXAxis, uno::UNO_QUERY );
                    Reference< beans::XPropertySet > xSecondaryYAxisProp( xSecondaryYAxis, uno::UNO_QUERY );

                    if( xMainXAxisProp.is() && xMainYAxisProp.is() )
                    {
                        chart2::ScaleData aMainXScale = xMainXAxis->getScaleData();
                        if( 0 == rChartTypeServiceName.reverseCompareTo( "com.sun.star.chart2.ScatterChartType" ) )
                        {
                            xMainYAxisProp->setPropertyValue("CrossoverPosition"
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_VALUE) );
                            double fCrossoverValue = 0.0;
                            aMainXScale.Origin >>= fCrossoverValue;
                            xMainYAxisProp->setPropertyValue("CrossoverValue"
                                    , uno::makeAny( fCrossoverValue ) );

                            if( aMainXScale.Orientation == chart2::AxisOrientation_REVERSE )
                            {
                                xMainYAxisProp->setPropertyValue("LabelPosition"
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_END) );
                                xMainYAxisProp->setPropertyValue("MarkPosition"
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisMarkPosition_AT_LABELS) );
                                if( xSecondaryYAxisProp.is() )
                                    xSecondaryYAxisProp->setPropertyValue("CrossoverPosition"
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_START) );
                            }
                            else
                            {
                                xMainYAxisProp->setPropertyValue("LabelPosition"
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_START) );
                                xMainYAxisProp->setPropertyValue("MarkPosition"
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisMarkPosition_AT_LABELS) );
                                if( xSecondaryYAxisProp.is() )
                                    xSecondaryYAxisProp->setPropertyValue("CrossoverPosition"
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_END) );
                            }
                        }
                        else
                        {
                            if( aMainXScale.Orientation == chart2::AxisOrientation_REVERSE )
                            {
                                xMainYAxisProp->setPropertyValue("CrossoverPosition"
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_END) );
                                if( xSecondaryYAxisProp.is() )
                                    xSecondaryYAxisProp->setPropertyValue("CrossoverPosition"
                                        , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_START) );
                            }
                            else
                            {
                                xMainYAxisProp->setPropertyValue("CrossoverPosition"
                                    , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_START) );
                                if( xSecondaryYAxisProp.is() )
                                    xSecondaryYAxisProp->setPropertyValue("CrossoverPosition"
                                        , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_END) );
                            }
                        }

                        chart2::ScaleData aMainYScale = xMainYAxis->getScaleData();
                        xMainXAxisProp->setPropertyValue("CrossoverPosition"
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_VALUE) );
                        double fCrossoverValue = 0.0;
                        aMainYScale.Origin >>= fCrossoverValue;
                        xMainXAxisProp->setPropertyValue("CrossoverValue"
                                , uno::makeAny( fCrossoverValue ) );

                        if( aMainYScale.Orientation == chart2::AxisOrientation_REVERSE )
                        {
                            xMainXAxisProp->setPropertyValue("LabelPosition"
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_END) );
                            xMainXAxisProp->setPropertyValue("MarkPosition"
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisMarkPosition_AT_LABELS) );
                            if( xSecondaryXAxisProp.is() )
                                xSecondaryXAxisProp->setPropertyValue("CrossoverPosition"
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_START) );
                        }
                        else
                        {
                            xMainXAxisProp->setPropertyValue("LabelPosition"
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_START) );
                            xMainXAxisProp->setPropertyValue("MarkPosition"
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisMarkPosition_AT_LABELS) );
                            if( xSecondaryXAxisProp.is() )
                                xSecondaryXAxisProp->setPropertyValue("CrossoverPosition"
                                , uno::makeAny( ::com::sun::star::chart::ChartAxisPosition_END) );
                        }
                    }
                }
            }
        }
        catch( uno::Exception & )
        {
        }
    }
}

SchXMLCategoriesContext::SchXMLCategoriesContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    OUString& rAddress ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrAddress( rAddress )
{
}

SchXMLCategoriesContext::~SchXMLCategoriesContext()
{
}

void SchXMLCategoriesContext::StartElement( const Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_TABLE &&
            IsXMLToken( aLocalName, XML_CELL_RANGE_ADDRESS ) )
        {
            Reference< chart2::XChartDocument > xNewDoc( GetImport().GetModel(), uno::UNO_QUERY );
            mrAddress = xAttrList->getValueByIndex( i );
        }
    }
}

DateScaleContext::DateScaleContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< beans::XPropertySet > xAxisProps ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        m_xAxisProps( xAxisProps )
{
}

DateScaleContext::~DateScaleContext()
{
}

namespace
{
enum DateScaleAttributeTokens
{
    XML_TOK_DATESCALE_BASE_TIME_UNIT,
    XML_TOK_DATESCALE_MAJOR_INTERVAL_VALUE,
    XML_TOK_DATESCALE_MAJOR_INTERVAL_UNIT,
    XML_TOK_DATESCALE_MINOR_INTERVAL_VALUE,
    XML_TOK_DATESCALE_MINOR_INTERVAL_UNIT
};

SvXMLTokenMapEntry aDateScaleAttributeTokenMap[] =
{
    { XML_NAMESPACE_CHART,  XML_BASE_TIME_UNIT,         XML_TOK_DATESCALE_BASE_TIME_UNIT  },
    { XML_NAMESPACE_CHART,  XML_MAJOR_INTERVAL_VALUE,   XML_TOK_DATESCALE_MAJOR_INTERVAL_VALUE  },
    { XML_NAMESPACE_CHART,  XML_MAJOR_INTERVAL_UNIT,    XML_TOK_DATESCALE_MAJOR_INTERVAL_UNIT  },
    { XML_NAMESPACE_CHART,  XML_MINOR_INTERVAL_VALUE,   XML_TOK_DATESCALE_MINOR_INTERVAL_VALUE  },
    { XML_NAMESPACE_CHART,  XML_MINOR_INTERVAL_UNIT,    XML_TOK_DATESCALE_MINOR_INTERVAL_UNIT  },
    XML_TOKEN_MAP_END
};

class DateScaleAttributeTokenMap : public SvXMLTokenMap
{
public:
    DateScaleAttributeTokenMap(): SvXMLTokenMap( aDateScaleAttributeTokenMap ) {}
    virtual ~DateScaleAttributeTokenMap() {}
};

struct theDateScaleAttributeTokenMap : public rtl::Static< DateScaleAttributeTokenMap, theDateScaleAttributeTokenMap > {};

sal_Int32 lcl_getTimeUnit( const OUString& rValue )
{
    sal_Int32 nTimeUnit = ::com::sun::star::chart::TimeUnit::DAY;
    if( IsXMLToken( rValue, XML_DAYS ) )
        nTimeUnit = ::com::sun::star::chart::TimeUnit::DAY;
    else if( IsXMLToken( rValue, XML_MONTHS ) )
        nTimeUnit = ::com::sun::star::chart::TimeUnit::MONTH;
    else if( IsXMLToken( rValue, XML_YEARS ) )
        nTimeUnit = ::com::sun::star::chart::TimeUnit::YEAR;
    return nTimeUnit;
}

}

void DateScaleContext::StartElement( const Reference< xml::sax::XAttributeList >& xAttrList )
{
    if( !m_xAxisProps.is() )
        return;

    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    const SvXMLTokenMap& rAttrTokenMap = theDateScaleAttributeTokenMap::get();

    bool bSetNewIncrement=false;
    chart::TimeIncrement aIncrement;
    m_xAxisProps->getPropertyValue("TimeIncrement") >>= aIncrement;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_DATESCALE_BASE_TIME_UNIT:
                {
                    aIncrement.TimeResolution = uno::makeAny( lcl_getTimeUnit(aValue) );
                    bSetNewIncrement = true;
                }
                break;
            case XML_TOK_DATESCALE_MAJOR_INTERVAL_VALUE:
                {
                    chart::TimeInterval aInterval(1,0);
                    aIncrement.MajorTimeInterval >>= aInterval;
                    ::sax::Converter::convertNumber( aInterval.Number, aValue );
                    aIncrement.MajorTimeInterval = uno::makeAny(aInterval);
                    bSetNewIncrement = true;
                }
                break;
            case XML_TOK_DATESCALE_MAJOR_INTERVAL_UNIT:
                {
                    chart::TimeInterval aInterval(1,0);
                    aIncrement.MajorTimeInterval >>= aInterval;
                    aInterval.TimeUnit = lcl_getTimeUnit(aValue);
                    aIncrement.MajorTimeInterval = uno::makeAny(aInterval);
                    bSetNewIncrement = true;
                }
                break;
            case XML_TOK_DATESCALE_MINOR_INTERVAL_VALUE:
                {
                    chart::TimeInterval aInterval(1,0);
                    aIncrement.MinorTimeInterval >>= aInterval;
                    ::sax::Converter::convertNumber( aInterval.Number, aValue );
                    aIncrement.MinorTimeInterval = uno::makeAny(aInterval);
                    bSetNewIncrement = true;
                }
                break;
            case XML_TOK_DATESCALE_MINOR_INTERVAL_UNIT:
                {
                    chart::TimeInterval aInterval(1,0);
                    aIncrement.MinorTimeInterval >>= aInterval;
                    aInterval.TimeUnit = lcl_getTimeUnit(aValue);
                    aIncrement.MinorTimeInterval = uno::makeAny(aInterval);
                    bSetNewIncrement = true;
                }
                break;
        }
    }

    if( bSetNewIncrement )
        m_xAxisProps->setPropertyValue("TimeIncrement", uno::makeAny( aIncrement ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
