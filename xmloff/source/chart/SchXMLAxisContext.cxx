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
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmluconv.hxx>

#include <rtl/math.hxx>
#include <tools/color.hxx>
#include <sal/log.hxx>

#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart/ChartAxisType.hpp>
#include <com/sun/star/chart/TimeIncrement.hpp>
#include <com/sun/star/chart/TimeInterval.hpp>
#include <com/sun/star/chart/TimeUnit.hpp>
#include <com/sun/star/chart/XAxis.hpp>
#include <com/sun/star/chart/XAxisSupplier.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>

#include <com/sun/star/drawing/LineStyle.hpp>

using namespace ::xmloff::token;
using namespace com::sun::star;

using com::sun::star::uno::Reference;

const SvXMLEnumMapEntry<SchXMLAxisDimension> aXMLAxisDimensionMap[] =
{
    { XML_X,  SCH_XML_AXIS_X  },
    { XML_Y,  SCH_XML_AXIS_Y  },
    { XML_Z,  SCH_XML_AXIS_Z  },
    { XML_TOKEN_INVALID, SchXMLAxisDimension(0) }
};

const SvXMLEnumMapEntry<sal_uInt16> aXMLAxisTypeMap[] =
{
    { XML_AUTO,  css::chart::ChartAxisType::AUTOMATIC },
    { XML_TEXT,  css::chart::ChartAxisType::CATEGORY },
    { XML_DATE,  css::chart::ChartAxisType::DATE },
    { XML_TOKEN_INVALID, 0 }
};

namespace {

class SchXMLCategoriesContext : public SvXMLImportContext
{
private:
    OUString& mrAddress;

public:
    SchXMLCategoriesContext( SvXMLImport& rImport,
                                   OUString& rAddress );
    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class DateScaleContext : public SvXMLImportContext
{
public:
    DateScaleContext( SvXMLImport& rImport,
                        const Reference< beans::XPropertySet >& rAxisProps );

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

private:
    Reference< beans::XPropertySet > m_xAxisProps;
};

}

SchXMLAxisContext::SchXMLAxisContext( SchXMLImportHelper& rImpHelper,
                                      SvXMLImport& rImport,
                                      Reference< chart::XDiagram > const & xDiagram,
                                      std::vector< SchXMLAxis >& rAxes,
                                      OUString & rCategoriesAddress,
                                      bool bAddMissingXAxisForNetCharts,
                                      bool bAdaptWrongPercentScaleValues,
                                      bool bAdaptXAxisOrientationForOld2DBarCharts,
                                      bool& rbAxisPositionAttributeImported ) :
        SvXMLImportContext( rImport ),
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

static Reference< chart::XAxis > lcl_getChartAxis(const SchXMLAxis& rCurrentAxis, const Reference< chart::XDiagram >& rDiagram )
{
    Reference< chart::XAxis > xAxis;
    Reference< chart::XAxisSupplier > xAxisSuppl( rDiagram, uno::UNO_QUERY );
    if( !xAxisSuppl.is() )
        return xAxis;
    if( rCurrentAxis.nAxisIndex == 0 )
        xAxis = xAxisSuppl->getAxis(rCurrentAxis.eDimension);
    else
        xAxis = xAxisSuppl->getSecondaryAxis(rCurrentAxis.eDimension);
    return xAxis;
}

/* returns a shape for the current axis's title. The property
   "Has...AxisTitle" is set to "True" to get the shape
 */
Reference< drawing::XShape > SchXMLAxisContext::getTitleShape() const
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
    xDiaProp->setPropertyValue( aPropName, uno::Any(true) );
    xResult.set( xAxis->getAxisTitle(), uno::UNO_QUERY );
    return xResult;
}

void SchXMLAxisContext::CreateGrid( const OUString& sAutoStyleName, bool bIsMajor )
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
    xDiaProp->setPropertyValue( aPropName, uno::Any(true) );

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
                                     uno::Any( COL_BLACK ));
        if (!sAutoStyleName.isEmpty())
            m_rImportHelper.FillAutoStyle(sAutoStyleName, xGridProp);
    }
}

void SchXMLAxisContext::startFastElement( sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // parse attributes
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(CHART, XML_DIMENSION):
                {
                    SchXMLAxisDimension nEnumVal;
                    if( SvXMLUnitConverter::convertEnum( nEnumVal, aIter.toView(), aXMLAxisDimensionMap ))
                        m_aCurrentAxis.eDimension = nEnumVal;
                }
                break;
            case XML_ELEMENT(CHART, XML_NAME):
                m_aCurrentAxis.aName = aIter.toString();
                break;
            case XML_ELEMENT(CHART, XML_AXIS_TYPE):
            case XML_ELEMENT(CHART_EXT, XML_AXIS_TYPE):
                sal_uInt16 nEnumVal;
                if( SvXMLUnitConverter::convertEnum( nEnumVal, aIter.toView(), aXMLAxisTypeMap ))
                {
                    m_nAxisType = nEnumVal;
                    m_bAxisTypeImported = true;
                }
                break;
            case XML_ELEMENT(CHART, XML_STYLE_NAME):
                m_aAutoStyleName = aIter.toString();
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
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
        rDoubleAny <<= fValue;
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
                aPropName = "HasZAxis";
            break;
        case SCH_XML_AXIS_UNDEF:
            SAL_INFO("xmloff.chart", "Invalid axis" );
            break;
    }
    try
    {
        xDiaProp->setPropertyValue( aPropName, uno::Any(true) );
    }
    catch( beans::UnknownPropertyException & )
    {
        SAL_INFO("xmloff.chart", "Couldn't turn on axis" );
    }
    if( m_aCurrentAxis.eDimension==SCH_XML_AXIS_Z )
    {
        bool bSettingZAxisSucceeded = false;
        try
        {
            xDiaProp->getPropertyValue( aPropName ) >>= bSettingZAxisSucceeded;
        }
        catch( beans::UnknownPropertyException & )
        {
            SAL_INFO("xmloff.chart", "Couldn't turn on z axis" );
        }
        if( !bSettingZAxisSucceeded )
            return;
    }

    m_xAxisProps.set( lcl_getChartAxis( m_aCurrentAxis, m_xDiagram ), uno::UNO_QUERY );

    if( m_bAddMissingXAxisForNetCharts && m_aCurrentAxis.eDimension==SCH_XML_AXIS_Y && m_aCurrentAxis.nAxisIndex==0 )
    {
        try
        {
            xDiaProp->setPropertyValue("HasXAxis", uno::Any(true) );
        }
        catch( beans::UnknownPropertyException & )
        {
            SAL_INFO("xmloff.chart", "Couldn't turn on x axis" );
        }
    }

    // set properties
    if( !m_xAxisProps.is())
        return;

    uno::Any aTrueBool( uno::Any( true ));
    uno::Any aFalseBool( uno::Any( false ));

    // #i109879# the line color is black as default, in the model it is a light gray
    m_xAxisProps->setPropertyValue("LineColor",
                                 uno::Any( COL_BLACK ));

    m_xAxisProps->setPropertyValue("DisplayLabels", aFalseBool );

    // Compatibility option: starting from LibreOffice 5.1 the rotated
    // layout is preferred to staggering for axis labels.
    // So the import default value for having compatibility with ODF
    // documents created with earlier LibreOffice versions is `true`.
    if( GetImport().getGeneratorVersion() != SvXMLImport::ProductVersionUnknown )
        m_xAxisProps->setPropertyValue("TryStaggeringFirst", aTrueBool );

    // #88077# AutoOrigin 'on' is default
    m_xAxisProps->setPropertyValue("AutoOrigin", aTrueBool );

    if( m_bAxisTypeImported )
        m_xAxisProps->setPropertyValue("AxisType", uno::Any(m_nAxisType) );

    if( !m_aAutoStyleName.isEmpty())
    {
        const SvXMLStylesContext* pStylesCtxt = m_rImportHelper.GetAutoStylesContext();
        if (pStylesCtxt)
        {
            SvXMLStyleContext* pStyle = const_cast<SvXMLStyleContext*>(pStylesCtxt->FindStyleChildContext(SchXMLImportHelper::GetChartFamilyID(), m_aAutoStyleName));

            if (XMLPropStyleContext * pPropStyleContext = dynamic_cast<XMLPropStyleContext*>(pStyle))
            {
                pPropStyleContext->FillPropertySet(m_xAxisProps);

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
                        pPropStyleContext->FillPropertySet(xXAxisProp);
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
                            , uno::Any(drawing::LineStyle_NONE));
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
                                if( aCooSysSeq.hasElements() )
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
                    u"CrossoverPosition", pPropStyleContext, pStylesCtxt ).hasValue();
            }
        }
    }

    if (m_aCurrentAxis.eDimension != SCH_XML_AXIS_X)
        return;

    Reference<chart2::XAxis> xAxis(lcl_getAxis(GetImport().GetModel(), m_aCurrentAxis.eDimension, m_aCurrentAxis.nAxisIndex));
    if (!xAxis.is())
        return;

    chart2::ScaleData aScaleData(xAxis->getScaleData());
    bool bIs3DChart = false;
    double fMajorOrigin = -1;
    OUString sChartType = m_xDiagram->getDiagramType();
    if ((xDiaProp->getPropertyValue("Dim3D") >>= bIs3DChart) && bIs3DChart
        && (sChartType == "com.sun.star.chart.BarDiagram" || sChartType == "com.sun.star.chart.StockDiagram"))
    {
        aScaleData.ShiftedCategoryPosition = true;
        xAxis->setScaleData(aScaleData);
    }
    else if ((m_xAxisProps->getPropertyValue("MajorOrigin") >>= fMajorOrigin)
            && (rtl::math::approxEqual(fMajorOrigin, 0.0) || rtl::math::approxEqual(fMajorOrigin, 0.5)))
    {
        aScaleData.ShiftedCategoryPosition = rtl::math::approxEqual(fMajorOrigin, 0.5);
        xAxis->setScaleData(aScaleData);
        m_xAxisProps->setPropertyValue("MajorOrigin", uno::Any());
    }
}

void SchXMLAxisContext::SetAxisTitle()
{
    if( m_aCurrentAxis.maTitle.empty() )
        return;

    Reference< chart::XAxis > xAxis( lcl_getChartAxis( m_aCurrentAxis, m_xDiagram ) );
    if( !xAxis.is() )
        return;

    if (m_aCurrentAxis.maTitle.back().first.isEmpty() &&
        m_aCurrentAxis.maTitle.back().second == OUStringChar(u'\x0D'))
        m_aCurrentAxis.maTitle.pop_back(); // remove last end of paragraph break

    SchXMLTools::importFormattedText(GetImport(), m_aCurrentAxis.maTitle, xAxis->getAxisTitle());
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLAxisContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    switch( nElement )
    {
        case XML_ELEMENT(CHART, XML_TITLE):
        {
            return new SchXMLTitleContext( m_rImportHelper, GetImport(),
                                               m_aCurrentAxis.maTitle,
                                               getTitleShape() );
        }
        break;

        case XML_ELEMENT(CHART, XML_CATEGORIES):
            m_aCurrentAxis.bHasCategories = true;
            return new SchXMLCategoriesContext( GetImport(),
                                                m_rCategoriesAddress );
            break;

        case  XML_ELEMENT(CHART, XML_DATE_SCALE):
        case  XML_ELEMENT(CHART_EXT, XML_DATE_SCALE):
            m_bDateScaleImported = true;
            return new DateScaleContext( GetImport(), m_xAxisProps );

        case XML_ELEMENT(CHART, XML_GRID):
        {
            bool bIsMajor = true;       // default value for class is "major"
            OUString sAutoStyleName;

            for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
            {
                switch (aIter.getToken())
                {
                    case XML_ELEMENT(CHART, XML_CLASS):
                        if( IsXMLToken( aIter, XML_MINOR ) )
                            bIsMajor = false;
                        break;
                    case XML_ELEMENT(CHART, XML_STYLE_NAME):
                        sAutoStyleName = aIter.toString();
                        break;
                    default:
                        XMLOFF_WARN_UNKNOWN("xmloff", aIter);
                }
            }

            CreateGrid( sAutoStyleName, bIsMajor );

            // don't create a context => use default context. grid elements are empty
        }
        break;

        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
            break;
    }

    return nullptr;
}

void SchXMLAxisContext::endFastElement(sal_Int32 )
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

Reference< chart2::XAxis > lcl_getAxis( const Reference< chart2::XCoordinateSystem >& rCooSys, sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    Reference< chart2::XAxis > xAxis;
    try
    {
        xAxis = rCooSys->getAxisByDimension( nDimensionIndex, nAxisIndex );
    }
    catch( uno::Exception & )
    {
    }
    return xAxis;
}

} // anonymous namespace

void SchXMLAxisContext::CorrectAxisPositions( const Reference< chart2::XChartDocument >& xNewDoc,
                          std::u16string_view rChartTypeServiceName,
                          std::u16string_view rODFVersionOfFile,
                          bool bAxisPositionAttributeImported )
{
    if( !(rODFVersionOfFile.empty() || rODFVersionOfFile == u"1.0" || rODFVersionOfFile == u"1.1"
        || ( rODFVersionOfFile == u"1.2" && !bAxisPositionAttributeImported )) )
        return;

    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xNewDoc->getFirstDiagram(), uno::UNO_QUERY_THROW );
        uno::Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems());
        if( aCooSysSeq.hasElements() )
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
                    if( rChartTypeServiceName == u"com.sun.star.chart2.ScatterChartType" )
                    {
                        xMainYAxisProp->setPropertyValue("CrossoverPosition"
                                , uno::Any( css::chart::ChartAxisPosition_VALUE) );
                        double fCrossoverValue = 0.0;
                        aMainXScale.Origin >>= fCrossoverValue;
                        xMainYAxisProp->setPropertyValue("CrossoverValue"
                                , uno::Any( fCrossoverValue ) );

                        if( aMainXScale.Orientation == chart2::AxisOrientation_REVERSE )
                        {
                            xMainYAxisProp->setPropertyValue("LabelPosition"
                                , uno::Any( css::chart::ChartAxisLabelPosition_OUTSIDE_END) );
                            xMainYAxisProp->setPropertyValue("MarkPosition"
                                , uno::Any( css::chart::ChartAxisMarkPosition_AT_LABELS) );
                            if( xSecondaryYAxisProp.is() )
                                xSecondaryYAxisProp->setPropertyValue("CrossoverPosition"
                                , uno::Any( css::chart::ChartAxisPosition_START) );
                        }
                        else
                        {
                            xMainYAxisProp->setPropertyValue("LabelPosition"
                                , uno::Any( css::chart::ChartAxisLabelPosition_OUTSIDE_START) );
                            xMainYAxisProp->setPropertyValue("MarkPosition"
                                , uno::Any( css::chart::ChartAxisMarkPosition_AT_LABELS) );
                            if( xSecondaryYAxisProp.is() )
                                xSecondaryYAxisProp->setPropertyValue("CrossoverPosition"
                                , uno::Any( css::chart::ChartAxisPosition_END) );
                        }
                    }
                    else
                    {
                        if( aMainXScale.Orientation == chart2::AxisOrientation_REVERSE )
                        {
                            xMainYAxisProp->setPropertyValue("CrossoverPosition"
                                , uno::Any( css::chart::ChartAxisPosition_END) );
                            if( xSecondaryYAxisProp.is() )
                                xSecondaryYAxisProp->setPropertyValue("CrossoverPosition"
                                    , uno::Any( css::chart::ChartAxisPosition_START) );
                        }
                        else
                        {
                            xMainYAxisProp->setPropertyValue("CrossoverPosition"
                                , uno::Any( css::chart::ChartAxisPosition_START) );
                            if( xSecondaryYAxisProp.is() )
                                xSecondaryYAxisProp->setPropertyValue("CrossoverPosition"
                                    , uno::Any( css::chart::ChartAxisPosition_END) );
                        }
                    }

                    chart2::ScaleData aMainYScale = xMainYAxis->getScaleData();
                    xMainXAxisProp->setPropertyValue("CrossoverPosition"
                            , uno::Any( css::chart::ChartAxisPosition_VALUE) );
                    double fCrossoverValue = 0.0;
                    aMainYScale.Origin >>= fCrossoverValue;
                    xMainXAxisProp->setPropertyValue("CrossoverValue"
                            , uno::Any( fCrossoverValue ) );

                    if( aMainYScale.Orientation == chart2::AxisOrientation_REVERSE )
                    {
                        xMainXAxisProp->setPropertyValue("LabelPosition"
                            , uno::Any( css::chart::ChartAxisLabelPosition_OUTSIDE_END) );
                        xMainXAxisProp->setPropertyValue("MarkPosition"
                            , uno::Any( css::chart::ChartAxisMarkPosition_AT_LABELS) );
                        if( xSecondaryXAxisProp.is() )
                            xSecondaryXAxisProp->setPropertyValue("CrossoverPosition"
                            , uno::Any( css::chart::ChartAxisPosition_START) );
                    }
                    else
                    {
                        xMainXAxisProp->setPropertyValue("LabelPosition"
                            , uno::Any( css::chart::ChartAxisLabelPosition_OUTSIDE_START) );
                        xMainXAxisProp->setPropertyValue("MarkPosition"
                            , uno::Any( css::chart::ChartAxisMarkPosition_AT_LABELS) );
                        if( xSecondaryXAxisProp.is() )
                            xSecondaryXAxisProp->setPropertyValue("CrossoverPosition"
                            , uno::Any( css::chart::ChartAxisPosition_END) );
                    }
                }
            }
        }
    }
    catch( uno::Exception & )
    {
    }
}

SchXMLCategoriesContext::SchXMLCategoriesContext(
    SvXMLImport& rImport,
    OUString& rAddress ) :
        SvXMLImportContext( rImport ),
        mrAddress( rAddress )
{
}

void SchXMLCategoriesContext::startFastElement( sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if( aIter.getToken() == XML_ELEMENT(TABLE, XML_CELL_RANGE_ADDRESS) )
            mrAddress = aIter.toString();
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }
}

DateScaleContext::DateScaleContext(
    SvXMLImport& rImport,
    const Reference< beans::XPropertySet >& rAxisProps ) :
        SvXMLImportContext( rImport ),
        m_xAxisProps( rAxisProps )
{
}

namespace
{
sal_Int32 lcl_getTimeUnit( const sax_fastparser::FastAttributeList::FastAttributeIter& rValue )
{
    sal_Int32 nTimeUnit = css::chart::TimeUnit::DAY;
    if( IsXMLToken( rValue, XML_DAYS ) )
        nTimeUnit = css::chart::TimeUnit::DAY;
    else if( IsXMLToken( rValue, XML_MONTHS ) )
        nTimeUnit = css::chart::TimeUnit::MONTH;
    else if( IsXMLToken( rValue, XML_YEARS ) )
        nTimeUnit = css::chart::TimeUnit::YEAR;
    return nTimeUnit;
}

}

void DateScaleContext::startFastElement( sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( !m_xAxisProps.is() )
        return;

    // parse attributes
    bool bSetNewIncrement=false;
    chart::TimeIncrement aIncrement;
    m_xAxisProps->getPropertyValue("TimeIncrement") >>= aIncrement;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch( aIter.getToken() )
        {
            case  XML_ELEMENT(CHART, XML_BASE_TIME_UNIT):
                {
                    aIncrement.TimeResolution <<= lcl_getTimeUnit(aIter);
                    bSetNewIncrement = true;
                }
                break;
            case XML_ELEMENT(CHART, XML_MAJOR_INTERVAL_VALUE):
                {
                    chart::TimeInterval aInterval(1,0);
                    aIncrement.MajorTimeInterval >>= aInterval;
                    ::sax::Converter::convertNumber( aInterval.Number, aIter.toView() );
                    aIncrement.MajorTimeInterval <<= aInterval;
                    bSetNewIncrement = true;
                }
                break;
            case  XML_ELEMENT(CHART, XML_MAJOR_INTERVAL_UNIT):
                {
                    chart::TimeInterval aInterval(1,0);
                    aIncrement.MajorTimeInterval >>= aInterval;
                    aInterval.TimeUnit = lcl_getTimeUnit(aIter);
                    aIncrement.MajorTimeInterval <<= aInterval;
                    bSetNewIncrement = true;
                }
                break;
            case XML_ELEMENT(CHART, XML_MINOR_INTERVAL_VALUE):
                {
                    chart::TimeInterval aInterval(1,0);
                    aIncrement.MinorTimeInterval >>= aInterval;
                    ::sax::Converter::convertNumber( aInterval.Number, aIter.toView() );
                    aIncrement.MinorTimeInterval <<= aInterval;
                    bSetNewIncrement = true;
                }
                break;
            case  XML_ELEMENT(CHART, XML_MINOR_INTERVAL_UNIT):
                {
                    chart::TimeInterval aInterval(1,0);
                    aIncrement.MinorTimeInterval >>= aInterval;
                    aInterval.TimeUnit = lcl_getTimeUnit(aIter);
                    aIncrement.MinorTimeInterval <<= aInterval;
                    bSetNewIncrement = true;
                }
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if( bSetNewIncrement )
        m_xAxisProps->setPropertyValue("TimeIncrement", uno::Any( aIncrement ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
