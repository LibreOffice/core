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

#include "SchXMLChartContext.hxx"
#include "SchXMLPlotAreaContext.hxx"
#include "SchXMLParagraphContext.hxx"
#include "SchXMLTableContext.hxx"

#include <tools/debug.hxx>

#include "xmlnmspe.hxx"
#include "nmspmap.hxx"
#include "xmluconv.hxx"
#include "prstylei.hxx"

#include "vector"

#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

#define SCH_BUILDCHART(xDoc) if( xDoc->hasControllersLocked() ) {\
    xDoc->unlockControllers();\
    xDoc->lockControllers(); }

enum SchXMLChartType
{
    XML_CHART_CLASS_LINE,
    XML_CHART_CLASS_AREA,
    XML_CHART_CLASS_CIRCLE,
    XML_CHART_CLASS_RING,
    XML_CHART_CLASS_SCATTER,
    XML_CHART_CLASS_RADAR,
    XML_CHART_CLASS_BAR,
    XML_CHART_CLASS_STOCK,
    XML_CHART_CLASS_BUBBLE,	// not yet implemented
    XML_CHART_CLASS_ADDIN
};

// ----------------------------------------

static __FAR_DATA SvXMLEnumMapEntry aXMLChartClassMap[] =
{
    { XML_LINE,	    	XML_CHART_CLASS_LINE	},
    { XML_AREA,		    XML_CHART_CLASS_AREA	},
    { XML_CIRCLE,		XML_CHART_CLASS_CIRCLE	},
    { XML_RING,		    XML_CHART_CLASS_RING	},
    { XML_SCATTER,		XML_CHART_CLASS_SCATTER	},
    { XML_RADAR,		XML_CHART_CLASS_RADAR	},
    { XML_BAR,			XML_CHART_CLASS_BAR		},
    { XML_STOCK,		XML_CHART_CLASS_STOCK	},
    { XML_BUBBLE,		XML_CHART_CLASS_BUBBLE	},
    { XML_ADD_IN,       XML_CHART_CLASS_ADDIN   },
    { XML_TOKEN_INVALID, 0 }
};

static __FAR_DATA SvXMLEnumMapEntry aXMLLegendAlignmentMap[] =
{
    { XML_LEFT, 		chart::ChartLegendPosition_LEFT		},
    { XML_TOP,			chart::ChartLegendPosition_TOP		},
    { XML_RIGHT,		chart::ChartLegendPosition_RIGHT	},
    { XML_BOTTOM,		chart::ChartLegendPosition_BOTTOM	},
    { XML_TOKEN_INVALID, 0 }
};

// ----------------------------------------

SchXMLChartContext::SchXMLChartContext( SchXMLImportHelper& rImpHelper,
                                        SvXMLImport& rImport, const ::rtl::OUString& rLocalName ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mbHasOwnTable( sal_False ),
        mbHasLegend( sal_False )
{
}

SchXMLChartContext::~SchXMLChartContext()
{}

void SchXMLChartContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    ::rtl::OUString aValue;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetChartAttrTokenMap();
    awt::Size aChartSize;
    // this flag is necessarry for pie charts in the core
    sal_Bool bSetSwitchData = sal_False;
    sal_Bool bDomainForDefaultDataNeeded = sal_False;

    ::rtl::OUString aServiceName;
    ::rtl::OUString sAutoStyleName;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        ::rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_CHART_CLASS:
                {
                    USHORT nEnumVal;
                    if( GetImport().GetMM100UnitConverter().convertEnum( nEnumVal, aValue, aXMLChartClassMap ))
                    {
                        switch( nEnumVal )
                        {
                            case XML_CHART_CLASS_LINE:
                                aServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.LineDiagram" ));
                                break;
                            case XML_CHART_CLASS_AREA:
                                aServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.AreaDiagram" ));
                                break;
                            case XML_CHART_CLASS_CIRCLE:
                                aServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.PieDiagram" ));
                                bSetSwitchData = sal_True;
                                break;
                            case XML_CHART_CLASS_RING:
                                aServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.DonutDiagram" ));
                                break;
                            case XML_CHART_CLASS_SCATTER:
                                aServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.XYDiagram" ));
                                bDomainForDefaultDataNeeded = sal_True;
                                break;
                            case XML_CHART_CLASS_RADAR:
                                aServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.NetDiagram" ));
                                break;
                            case XML_CHART_CLASS_BAR:
                                aServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.BarDiagram" ));
                                break;
                            case XML_CHART_CLASS_STOCK:
                                aServiceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.StockDiagram" ));
                                break;
                            case XML_CHART_CLASS_BUBBLE:
                                DBG_ERROR( "Bubble chart not supported yet" );
                                break;
                            case XML_CHART_CLASS_ADDIN:
                                // service is taken from add-in-name attribute

                                // for service charts assume domain in base type
                                // if base type doesn't use a domain this is ok,
                                // the data just grows bigger
                                bDomainForDefaultDataNeeded = sal_True;
                                break;
                        }
                    }
                }
                break;

            case XML_TOK_CHART_WIDTH:
                GetImport().GetMM100UnitConverter().convertMeasure( aChartSize.Width, aValue );
                break;

            case XML_TOK_CHART_HEIGHT:
                GetImport().GetMM100UnitConverter().convertMeasure( aChartSize.Height, aValue );
                break;

            case XML_TOK_CHART_STYLE_NAME:
                sAutoStyleName = aValue;
                break;

            case XML_TOK_CHART_ADDIN_NAME:
                aServiceName = aValue;
                break;

            case XML_TOK_CHART_COL_MAPPING:
                msColTrans = aValue;
                break;
            case XML_TOK_CHART_ROW_MAPPING:
                msRowTrans = aValue;
                break;
        }
    }

    InitChart (aChartSize, bDomainForDefaultDataNeeded, aServiceName, bSetSwitchData);
    
    // set auto-styles for Area
    uno::Reference< beans::XPropertySet > xProp( mrImportHelper.GetChartDocument()->getArea(), uno::UNO_QUERY );
    if( xProp.is())
    {
        const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
        if( pStylesCtxt )
        {
            const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                mrImportHelper.GetChartFamilyID(), sAutoStyleName );

            if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                (( XMLPropStyleContext* )pStyle )->FillPropertySet( xProp );
        }
    }

    // prevent BuildChart from now on
    uno::Reference< frame::XModel > xModel( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );
    if( xModel.is())
        xModel->lockControllers();
}

void SchXMLChartContext::EndElement()
{
    uno::Reference< chart::XChartDocument > xDoc = mrImportHelper.GetChartDocument();
    uno::Reference< beans::XPropertySet > xProp( xDoc, uno::UNO_QUERY );

    if( xProp.is())
    {
        if( maMainTitle.getLength())
        {
            uno::Reference< beans::XPropertySet > xTitleProp( xDoc->getTitle(), uno::UNO_QUERY );
            if( xTitleProp.is())
            {
                try
                {
                    uno::Any aAny;
                    aAny <<= maMainTitle;
                    xTitleProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Property String for Title not available" );
                }
/*				uno::Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                if( xShape.is())
                {
                    // perform build chart with new title string
                    // so that setting the position works correctly
                    if( xDoc.is())
                    {
                        xDoc->unlockControllers();
                        xDoc->lockControllers();
                    }
                    xShape->setPosition( maMainTitlePos );
                }
*/			}
        }
        if( maSubTitle.getLength())
        {
            uno::Reference< beans::XPropertySet > xTitleProp( xDoc->getSubTitle(), uno::UNO_QUERY );
            if( xTitleProp.is())
            {
                try
                {
                    uno::Any aAny;
                    aAny <<= maSubTitle;
                    xTitleProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Property String for Title not available" );
                }
/*				uno::Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                if( xShape.is())
                {
                    // perform build chart with new title string
                    // so that setting the position works correctly
                    if( xDoc.is())
                    {
                        xDoc->unlockControllers();
                        xDoc->lockControllers();
                    }
                    xShape->setPosition( maSubTitlePos );
                }
*/			}
        }
    }
    
    
    if( mbHasOwnTable )
    {
        // apply data read in table sub-element to chart
          SchXMLTableHelper::applyTableSimple( maTable, xDoc );
    }
    else
    {
        // deprecated method
        // translate cell-address strings
        if( maSeriesAddresses.getLength() ||
            msCategoriesAddress.getLength())
        {
            uno::Reference< util::XStringMapping > xTableAddressMapper = mrImportHelper.GetTableAddressMapper();
            if( xTableAddressMapper.is())
            {
                // series
                sal_Int32 nLength = maSeriesAddresses.getLength();
                sal_Int32 nIdx;
                uno::Sequence< ::rtl::OUString > aStrSeq( nLength * 2 + 1 );
                sal_Bool bHasDomain = sal_False;

                for( nIdx = 0; nIdx < nLength; nIdx++ )
                {
                    aStrSeq[ nIdx * 2 ] = maSeriesAddresses[ nIdx ].DataRangeAddress;
                    aStrSeq[ nIdx * 2 + 1 ] = maSeriesAddresses[ nIdx ].LabelAddress;

                    // domains
                    if( maSeriesAddresses[ nIdx ].DomainRangeAddresses.getLength())
                    {
                        xTableAddressMapper->mapStrings( maSeriesAddresses[ nIdx ].DomainRangeAddresses );
                        bHasDomain = sal_True;
                    }
                }
                // categories
                aStrSeq[ nLength * 2 ] = msCategoriesAddress;

                // translate
                xTableAddressMapper->mapStrings( aStrSeq );

                // write back
                sal_Int32 nOffset = 0;
                for( nIdx = 0; nIdx < nLength; nIdx++ )
                {
                    // #81525# convert addresses for xy charts
                    // this should be done by calc in the future
                    if( nIdx == 0 &&
                        bHasDomain )
                    {
                        // enlarge the sequence
                        maSeriesAddresses.realloc( maSeriesAddresses.getLength() + 1 );

                        // copy the domain as first series
                        if( maSeriesAddresses[ nIdx + nOffset ].DomainRangeAddresses.getLength() > 0 )
                            maSeriesAddresses[ nIdx + nOffset ].DataRangeAddress =
                                maSeriesAddresses[ nIdx + nOffset ].DomainRangeAddresses[ 0 ];
                        // the current data range becomes the second series
                        nOffset++;
                    }

                    maSeriesAddresses[ nIdx + nOffset ].DataRangeAddress = aStrSeq[ nIdx * 2 ];
                    maSeriesAddresses[ nIdx + nOffset ].LabelAddress = aStrSeq[ nIdx * 2 + 1 ];
                }
                msCategoriesAddress = aStrSeq[ nLength * 2 ];
            }
        }
    }
    
    // set table references at document
    // even when having own table (Writer)
    if( xProp.is())
    {
        try
        {
            uno::Any aAny;
            if( msChartAddress.getLength())
            {
                aAny <<= msChartAddress;
                xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "ChartRangeAddress" ), aAny );

                if( msTableNumberList.getLength())
                {
                    aAny <<= msTableNumberList;
                    xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "TableNumberList" ), aAny );
                }
            }
            else
            {
                // deprecated
                if( msCategoriesAddress.getLength())
                {
                    aAny <<= msCategoriesAddress;
                    xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "CategoriesRangeAddress" ), aAny );
                }
                    
                // deprecated
                if( maSeriesAddresses.getLength())
                {
                    aAny <<= maSeriesAddresses;
                    xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "SeriesAddresses" ), aAny );
                }
            }

            // row / col translations
            bool bHasColTrans = (msColTrans.getLength() > 0);
            bool bHasRowTrans = (msRowTrans.getLength() > 0);
            if( bHasColTrans )
            {
                uno::Sequence< sal_Int32 > aSeq = GetNumberSequenceFromString( msColTrans );
                aAny <<= aSeq;
                xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "TranslatedColumns" ), aAny );
            }
            else if( bHasRowTrans )
            {
                uno::Sequence< sal_Int32 > aSeq = GetNumberSequenceFromString( msRowTrans );
                aAny <<= aSeq;
                xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "TranslatedRows" ), aAny );
            }
        }
        catch( beans::UnknownPropertyException )
        {
            DBG_WARNING( "Required property not found in ChartDocument" );
        }
    }

    // allow BuildChart again
/*	uno::Reference< frame::XModel > xModel( xDoc, uno::UNO_QUERY );
    if( xModel.is())
        xModel->unlockControllers();
*/		
    //	Set the main title's and subtitle's positions.
    if( maMainTitle.getLength() > 0)
    {
        uno::Reference<drawing::XShape> xMainTitleShape(xDoc->getTitle(), uno::UNO_QUERY);
        if( xMainTitleShape.is())
            xMainTitleShape->setPosition( maMainTitlePos );
    }
    if( maSubTitle.getLength() > 0)
    {
        uno::Reference<drawing::XShape> xSubTitleShape(xDoc->getSubTitle(), uno::UNO_QUERY);
        if( xSubTitleShape.is())
            xSubTitleShape->setPosition( maSubTitlePos );
    }

    // set absolute legend position after (BuildChart!)
    if( mbHasLegend )
    {
        uno::Reference< drawing::XShape > xLegendShape( xDoc->getLegend(), uno::UNO_QUERY );
        if( xLegendShape.is())
            xLegendShape->setPosition( maLegendPos );
    }

    // #102413# BuildChart to manifest legend position
    if( xDoc->hasControllersLocked())
        xDoc->unlockControllers();

    // AF: No more BuildCharts until Initialize is called (by Draw or SaveAs).

    // BM: There should be no further BuildCharts, and it is very dangerous to
    // leave the lock status on hoping that it is changed in Draw or SaveAs
    // (OLE-Clone?).  At least it isn't for the writer flat XML filter.  So,
    // leave Controllers unlocked from now on, as this is the last line of code
    // that the Chart XML import routine will call
}

SvXMLImportContext* SchXMLChartContext::CreateChildContext(
    USHORT nPrefix,
    const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    static const sal_Bool bTrue = sal_True;    
    static const uno::Any aTrueBool( &bTrue, ::getBooleanCppuType());

    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetChartElemTokenMap();
    uno::Reference< chart::XChartDocument > xDoc = mrImportHelper.GetChartDocument();
    uno::Reference< beans::XPropertySet > xProp( xDoc, uno::UNO_QUERY );

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_CHART_PLOT_AREA:
            pContext = new SchXMLPlotAreaContext( mrImportHelper, GetImport(), rLocalName,
                                                  maSeriesAddresses, msCategoriesAddress,
                                                  msChartAddress, msTableNumberList );
            break;

        case XML_TOK_CHART_TITLE:
            if( xDoc.is())
            {
                if( xProp.is())
                {
                    uno::Any aTrueBool;
                    aTrueBool <<= (sal_Bool)(sal_True);
                    xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "HasMainTitle" ), aTrueBool );

                    SCH_BUILDCHART( xDoc );
                }
                uno::Reference< drawing::XShape > xTitleShape( xDoc->getTitle(), uno::UNO_QUERY );
                pContext = new SchXMLTitleContext( mrImportHelper, GetImport(),
                                                   rLocalName, maMainTitle, xTitleShape, maMainTitlePos );
            }
            break;

        case XML_TOK_CHART_SUBTITLE:
            if( xDoc.is())
            {
                if( xProp.is())
                {
                    xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "HasSubTitle" ), aTrueBool );
                    SCH_BUILDCHART( xDoc );
                }
                uno::Reference< drawing::XShape > xTitleShape( xDoc->getSubTitle(), uno::UNO_QUERY );
                pContext = new SchXMLTitleContext( mrImportHelper, GetImport(),
                                                   rLocalName, maSubTitle, xTitleShape, maSubTitlePos );
            }
            break;

        case XML_TOK_CHART_LEGEND:
            pContext = new SchXMLLegendContext( mrImportHelper, GetImport(), rLocalName, maLegendPos );
            mbHasLegend =sal_True;
            break;

        case XML_TOK_CHART_TABLE:
            pContext = new SchXMLTableContext( mrImportHelper, GetImport(), rLocalName, maTable );
            if( pContext )
            {
                mbHasOwnTable = sal_True;
                if( xProp.is())
                    try
                    {
                        xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "ExportData" ), aTrueBool );
                    }
                    catch( uno::Exception )
                    {
                        DBG_ERRORFILE( "Property missing" );
                    }
            }
            break;

        default:
            // try importing as an additional shape
            if( ! mxDrawPage.is())
            {
                uno::Reference< drawing::XDrawPageSupplier  > xSupp( xDoc, uno::UNO_QUERY );
                if( xSupp.is())
                    mxDrawPage = uno::Reference< drawing::XShapes >( xSupp->getDrawPage(), uno::UNO_QUERY );

                DBG_ASSERT( mxDrawPage.is(), "Invalid Chart Page" );
            }
            if( mxDrawPage.is())
                pContext = GetImport().GetShapeImport()->CreateGroupChildContext(
                    GetImport(), nPrefix, rLocalName, xAttrList, mxDrawPage );
            break;
    }

    if( ! pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}


/*
    With a locked controller the following is done here:
        1.	Hide title, subtitle, and legend.
        2.	Set the size of the draw page.
        3.	Set a (logically) empty data set.
        4.	Set the chart type.
*/
void	SchXMLChartContext::InitChart	(awt::Size aChartSize, 
                                        sal_Bool bDomainForDefaultDataNeeded,
                                        ::rtl::OUString aServiceName,
                                        sal_Bool bSetSwitchData)
{
    uno::Reference< chart::XChartDocument > xDoc = mrImportHelper.GetChartDocument();
    DBG_ASSERT( xDoc.is(), "No valid document!" );
    uno::Reference< frame::XModel > xModel (xDoc, uno::UNO_QUERY );
    if( xModel.is())
        xModel->lockControllers();

    //	Hide title, subtitle, and legend
    uno::Reference< beans::XPropertySet > xProp( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );
    if( xProp.is())
    {
        uno::Any aFalseBool;
        aFalseBool <<= (sal_Bool)(sal_False);
        try
        {
            xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "HasMainTitle" ), aFalseBool );
            xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "HasSubTitle" ), aFalseBool );
            xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "HasLegend" ), aFalseBool );
        }
        catch( beans::UnknownPropertyException )
        {
            DBG_ERROR( "XML-Chart Import: Property not found" );
        }
    }

    //	Set the size of the draw page.
    uno::Reference< drawing::XDrawPageSupplier > xPageSupp( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );
    if( xPageSupp.is())
    {
        uno::Reference< beans::XPropertySet > xPageProp( xPageSupp->getDrawPage(), uno::UNO_QUERY );
        if( xPageProp.is())
        {
            try
            {
                uno::Any aAny;
                aAny <<= (sal_Int32)( aChartSize.Width );
                xPageProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" )), aAny );

                aAny <<= (sal_Int32)( aChartSize.Height );
                xPageProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" )), aAny );
            }
            catch( beans::UnknownPropertyException )
            {
                DBG_ERROR( "Cannot set page size" );
            }
        }
    }

    //	We have to unlock the controllers and execute an implicit BuildChart because
    //	the following call to setData needs data structures created in a BuildChart.
    if( xModel.is())
        xModel->unlockControllers();
    
    //	Set a (logically) empty data set.  It will later be filled with the
    //	actual data.
    //	Because the chart does not work with a really empty data set a dummy data point
    //	and, if necessary, a dummy domain value (Not a number) are set.
    uno::Reference< chart::XChartDataArray > xArray( xDoc->getData(), uno::UNO_QUERY );
    if( xArray.is())
    {
        double fNan = 0.0;

        uno::Reference< chart::XChartData > xData( xDoc->getData(), uno::UNO_QUERY );
        if( xData.is())
            fNan = xData->getNotANumber();

        // attention: the data must at least be 1 x 1,
        // (or 2 x 2 for scatter charts)
        // otherwise BuildChart doesn't perform much.
        if( bDomainForDefaultDataNeeded )
        {
            uno::Sequence< uno::Sequence< double > > aAlmostEmptySeq( 2 );
            aAlmostEmptySeq[ 0 ].realloc( 2 );
            aAlmostEmptySeq[ 0 ][ 0 ] = 0.0;
            aAlmostEmptySeq[ 0 ][ 1 ] = fNan;

            aAlmostEmptySeq[ 1 ].realloc( 2 );
            aAlmostEmptySeq[ 1 ][ 0 ] = 0.0;
            aAlmostEmptySeq[ 1 ][ 1 ] = fNan;

            xArray->setData( aAlmostEmptySeq );
        }
        else
        {
            uno::Sequence< uno::Sequence< double > > aAlmostEmptySeq( 1 );
            aAlmostEmptySeq[ 0 ].realloc( 1 );
            aAlmostEmptySeq[ 0 ][ 0 ] = 0.0;

            xArray->setData( aAlmostEmptySeq );
        }
    }
    
    if( xModel.is())
        xModel->lockControllers();

    //	Set the chart type via setting the diagram.
    if( aServiceName.getLength() &&
        xDoc.is())
    {
        uno::Reference< lang::XMultiServiceFactory > xFact( xDoc, uno::UNO_QUERY );
        if( xFact.is())
        {
            uno::Reference< chart::XDiagram > xDia( xFact->createInstance( aServiceName ), uno::UNO_QUERY );
            if( xDia.is())
            {
                xDoc->setDiagram( xDia );

                // set data row source for pie charts to ROWS
                if( bSetSwitchData )
                {
                    uno::Reference< beans::XPropertySet > xDiaProp( xDia, uno::UNO_QUERY );
                    if( xDiaProp.is())
                    {
                        uno::Any aAny;
                        aAny <<= chart::ChartDataRowSource( chart::ChartDataRowSource_ROWS );
                        xDiaProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DataRowSource" )), aAny );
                    }
                }
            }
        }
    }

    if( xModel.is())
        xModel->unlockControllers();
}

uno::Sequence< sal_Int32 > SchXMLChartContext::GetNumberSequenceFromString( const ::rtl::OUString& rStr )
{
    const sal_Unicode aSpace( ' ' );

    // count number of entries
    ::std::vector< sal_Int32 > aVec;
    sal_Int32 nLastPos = 0;
    sal_Int32 nPos = 0;
    const sal_Int32 nSize = rStr.getLength();
    while( nPos != -1 )
    {
        nPos = rStr.indexOf( aSpace, nLastPos );
        if( nPos > nLastPos )
        {
            aVec.push_back( rStr.copy( nLastPos, (nPos - nLastPos) ).toInt32() );
        }
        if( nPos != -1 )
            nLastPos = nPos + 1;
    }
    // last entry
    if( nLastPos != 0 &&
        rStr.getLength() > nLastPos )
    {
        aVec.push_back( rStr.copy( nLastPos, (rStr.getLength() - nLastPos) ).toInt32() );
    }

    const sal_Int32 nVecSize = aVec.size();
    uno::Sequence< sal_Int32 > aSeq( nVecSize );
    sal_Int32* pSeqArr = aSeq.getArray();
    for( nPos = 0; nPos < nVecSize; ++nPos )
    {
        pSeqArr[ nPos ] = aVec[ nPos ];
    }
    return aSeq;
}

// ----------------------------------------

SchXMLTitleContext::SchXMLTitleContext( SchXMLImportHelper& rImpHelper, SvXMLImport& rImport,
                                        const ::rtl::OUString& rLocalName,
                                        ::rtl::OUString& rTitle,
                                        uno::Reference< drawing::XShape >& xTitleShape,
                                        awt::Point& rPosition ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrTitle( rTitle ),
        mxTitleShape( xTitleShape ),
        mrPosition( rPosition )
{
}

SchXMLTitleContext::~SchXMLTitleContext()
{}

void SchXMLTitleContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    ::rtl::OUString aValue;

    if( mxTitleShape.is())
        mrPosition = mxTitleShape->getPosition();

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        ::rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_SVG )
        {
            if( IsXMLToken( aLocalName, XML_X ) )
                GetImport().GetMM100UnitConverter().convertMeasure( mrPosition.X, aValue );
            else if( IsXMLToken( aLocalName, XML_Y ) )
                GetImport().GetMM100UnitConverter().convertMeasure( mrPosition.Y, aValue );
        }
        else if( nPrefix == XML_NAMESPACE_CHART )
        {
            if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                msAutoStyleName = aValue;
        }
    }

    if( mxTitleShape.is())
    {
        uno::Reference< beans::XPropertySet > xProp( mxTitleShape, uno::UNO_QUERY );
        if( xProp.is())
        {
            const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
            if( pStylesCtxt )
            {
                const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                    mrImportHelper.GetChartFamilyID(), msAutoStyleName );

                if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                    (( XMLPropStyleContext* )pStyle )->FillPropertySet( xProp );
            }
        }
    }
}

SvXMLImportContext* SchXMLTitleContext::CreateChildContext(
    USHORT nPrefix,
    const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    if( nPrefix == XML_NAMESPACE_TEXT &&
        IsXMLToken( rLocalName, XML_P ) )
    {
        pContext = new SchXMLParagraphContext( GetImport(), rLocalName, mrTitle );
    }
    else
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

// ----------------------------------------

SchXMLLegendContext::SchXMLLegendContext( SchXMLImportHelper& rImpHelper,
                                          SvXMLImport& rImport, const ::rtl::OUString& rLocalName,
                                          ::com::sun::star::awt::Point& rPosition ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrPosition( rPosition )
{
}

void SchXMLLegendContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    uno::Reference< chart::XChartDocument > xDoc = mrImportHelper.GetChartDocument();
    if( ! xDoc.is())
        return;

    // turn on legend
    uno::Reference< beans::XPropertySet > xDocProp( xDoc, uno::UNO_QUERY );
    if( xDocProp.is())
    {
        uno::Any aTrueBool;
        aTrueBool <<= (sal_Bool)(sal_True);
        try
        {
            xDocProp->setPropertyValue( ::rtl::OUString::createFromAscii( "HasLegend" ), aTrueBool );
            SCH_BUILDCHART( xDoc );
            
            // initialize position
            uno::Reference< drawing::XShape > xLegendShape( xDoc->getLegend(), uno::UNO_QUERY );
            if( xLegendShape.is())
                mrPosition = xLegendShape->getPosition();
        }
        catch( beans::UnknownPropertyException )
        {
            DBG_ERROR( "Property HasLegend not found" );
        }
    }

    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    ::rtl::OUString aValue;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetLegendAttrTokenMap();

    awt::Point aPosition;
    uno::Reference< drawing::XShape > xLegendShape( xDoc->getLegend(), uno::UNO_QUERY );
    if( xLegendShape.is())
        aPosition = xLegendShape->getPosition();

    ::rtl::OUString sAutoStyleName;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        ::rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_LEGEND_POSITION:
                {
                    // set anchor position
                    uno::Reference< beans::XPropertySet > xProp( xDoc->getLegend(), uno::UNO_QUERY );
                    if( xProp.is())
                    {
                        try
                        {
                            USHORT nEnumVal;
                            if( GetImport().GetMM100UnitConverter().convertEnum( nEnumVal, aValue, aXMLLegendAlignmentMap ))
                            {
                                uno::Any aAny;
                                aAny <<= (chart::ChartLegendPosition)(nEnumVal);
                                xProp->setPropertyValue( ::rtl::OUString::createFromAscii( "Alignment" ), aAny );
                            }
                        }
                        catch( beans::UnknownPropertyException )
                        {
                            DBG_ERROR( "Property Alignment (legend) not found" );
                        }
                    }
                }
                break;

            case XML_TOK_LEGEND_X:
                GetImport().GetMM100UnitConverter().convertMeasure( mrPosition.X, aValue );
                break;
            case XML_TOK_LEGEND_Y:
                GetImport().GetMM100UnitConverter().convertMeasure( mrPosition.Y, aValue );
                break;
            case XML_TOK_LEGEND_STYLE_NAME:
                sAutoStyleName = aValue;
        }
    }

    // set auto-styles for Area
    uno::Reference< beans::XPropertySet > xProp( xDoc->getLegend(), uno::UNO_QUERY );
    if( xProp.is())
    {
        const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
        if( pStylesCtxt )
        {
            const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                mrImportHelper.GetChartFamilyID(), sAutoStyleName );

            if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                (( XMLPropStyleContext* )pStyle )->FillPropertySet( xProp );
        }
    }
}

SchXMLLegendContext::~SchXMLLegendContext()
{
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
