/*************************************************************************
 *
 *  $RCSfile: SchXMLExport.cxx,v $
 *
 *  $Revision: 1.52 $
 *
 *  last change: $Author: bm $ $Date: 2001-08-06 16:32:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <xmlprmap.hxx>

#include "SchXMLExport.hxx"
#include "XMLChartPropertySetMapper.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#ifndef _SOT_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif

#ifndef _SVTOOLS_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif
#ifndef _XMLOFF_XMLASTPLP_HXX
#include "xmlaustp.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLMETAE_HXX
#include "xmlmetae.hxx"
#endif
#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif
#ifndef _TOOLS_SOLMATH_HXX
#include <tools/solmath.hxx>
#endif
// header for any2enum
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HPP_
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTLEGENDPOSITION_HPP_
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XTWOAXISXSUPPLIER_HPP_
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XTWOAXISYSUPPLIER_HPP_
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XAXISZSUPPLIER_HPP_
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XCHARTDATAARRAY_HPP_
#include <com/sun/star/chart/XChartDataArray.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTAXISASSIGN_HPP_
#include <com/sun/star/chart/ChartAxisAssign.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSERIESADDRESS_HPP_
#include <com/sun/star/chart/ChartSeriesAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_X3DDISPLAY_HPP_
#include <com/sun/star/chart/X3DDisplay.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGMAPPING_HPP_
#include <com/sun/star/util/XStringMapping.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif

#include    "MultiPropertySetHandler.hxx"

using namespace rtl;
using namespace com::sun::star;
using namespace ::xmloff::token;
using namespace com::sun::star::uno;


// ========================================
// class SchXMLExportHelper
// ========================================

SchXMLExportHelper::SchXMLExportHelper(
    SvXMLExport& rExport,
    SvXMLAutoStylePoolP& rASPool ) :
        mrExport( rExport ),
        mrAutoStylePool( rASPool ),
        mnSeriesCount( 0 ),
        mnSeriesLength( 0 ),
        mnDomainAxes( 0 ),
        mbHasSeriesLabels( sal_False ),
        mbHasCategoryLabels( sal_False ),
        mbRowSourceColumns( sal_True ),
        msCLSID( rtl::OUString( SvGlobalName( SO3_SCH_CLASSID ).GetHexName()))
{
    msTableName = rtl::OUString::createFromAscii( "local-table" );

    // create factory
    mxPropertyHandlerFactory = new XMLChartPropHdlFactory;

    if( mxPropertyHandlerFactory.is() )
    {
        // create property set mapper
        mxPropertySetMapper = new XMLChartPropertySetMapper;

    }

    mxExpPropMapper = new XMLChartExportPropertyMapper( mxPropertySetMapper, rExport );

    // register chart auto-style family
    mrAutoStylePool.AddFamily(
        XML_STYLE_FAMILY_SCH_CHART_ID,
        rtl::OUString::createFromAscii( XML_STYLE_FAMILY_SCH_CHART_NAME ),
        mxExpPropMapper.get(),
        rtl::OUString::createFromAscii( XML_STYLE_FAMILY_SCH_CHART_PREFIX ));
}

const rtl::OUString& SchXMLExportHelper::getChartCLSID()
{
    return msCLSID;
}

void SchXMLExportHelper::exportAutoStyles()
{
    if( mxExpPropMapper.is())
    {
        //ToDo: when embedded in calc/writer this is not necessary because the
        // numberformatter is shared between both documents
        mrExport.exportAutoDataStyles();
        // export auto styles
        mrAutoStylePool.exportXML(
            XML_STYLE_FAMILY_SCH_CHART_ID,
            mrExport.GetDocHandler(),
            mrExport.GetMM100UnitConverter(),
            mrExport.GetNamespaceMap());
    }
}

void SchXMLExportHelper::collectAutoStyles( uno::Reference< chart::XChartDocument > rChartDoc )
{
    parseDocument( rChartDoc, sal_False );
}

void SchXMLExportHelper::exportChart( uno::Reference< chart::XChartDocument > rChartDoc,
                                      sal_Bool bIncludeTable )
{
    parseDocument( rChartDoc, sal_True, bIncludeTable );
    DBG_ASSERT( maAutoStyleNameQueue.empty(), "There are still remaining autostyle names in the queue" );
}


// private methods
// ---------------

/// if bExportContent is false the auto-styles are collected
void SchXMLExportHelper::parseDocument( uno::Reference< chart::XChartDocument >& rChartDoc,
                                        sal_Bool bExportContent,
                                        sal_Bool bIncludeTable )
{
    uno::Reference< chart::XDiagram > xDiagram = rChartDoc->getDiagram();

    // determine if data is in rows
    sal_Bool bSwitchData = sal_False;
    uno::Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY );
    if( xDiaProp.is())
    {
        try
        {
            chart::ChartDataRowSource eRowSource;
            uno::Any aAny =
                xDiaProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DataRowSource" )));

            cppu::any2enum< chart::ChartDataRowSource >( eRowSource, aAny );
            mbRowSourceColumns = ( eRowSource == chart::ChartDataRowSource_COLUMNS );

            // the chart core treats donut chart with interchanged rows/columns
            rtl::OUString sChartType = xDiagram->getDiagramType();
            if( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.DonutDiagram" )))
            {
                mbRowSourceColumns = ! mbRowSourceColumns;
            }
        }
        catch( uno::Exception )
        {
            DBG_ERROR( "exportChart: Error getting Property \"DataRowSource\"" );
        }
    }

    if( ! rChartDoc.is())
    {
        DBG_ERROR( "No XChartDocument was given for export." );
        return;
    }

    // get Properties of ChartDocument
    sal_Bool bHasMainTitle = sal_False;
    sal_Bool bHasSubTitle = sal_False;
    sal_Bool bHasLegend = sal_False;

    std::vector< XMLPropertyState > aPropertyStates;

    uno::Reference< beans::XPropertySet > xDocPropSet( rChartDoc, uno::UNO_QUERY );
    if( xDocPropSet.is())
    {
        try
        {
            uno::Any aAny( xDocPropSet->getPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasMainTitle" ))));
            aAny >>= bHasMainTitle;
            aAny = xDocPropSet->getPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasSubTitle" )));
            aAny >>= bHasSubTitle;
            aAny = xDocPropSet->getPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasLegend" )));
            aAny >>= bHasLegend;
        }
        catch( beans::UnknownPropertyException )
        {
            DBG_WARNING( "Required property not found in ChartDocument" );
        }
    }

    // get some values of general interest

    // reset
    mbHasSeriesLabels = mbHasCategoryLabels = sal_False;
    mnSeriesCount = mnSeriesLength = 0;

    // set
    uno::Reference< chart::XChartDataArray > xData( rChartDoc->getData(), uno::UNO_QUERY );
    if( xData.is())
    {
        uno::Sequence< uno::Sequence< double > > xValues = xData->getData();

        if( xValues.getLength())
        {
            // determine size of data
            const uno::Sequence< double >* pSequence = xValues.getConstArray();
            uno::Sequence< rtl::OUString > xSeriesLabels;
            uno::Sequence< rtl::OUString > xCategoryLabels;

            if( mbRowSourceColumns )
            {
                mnSeriesCount = pSequence->getLength();
                mnSeriesLength = xValues.getLength();
                xSeriesLabels = xData->getColumnDescriptions();
                xCategoryLabels = xData->getRowDescriptions();
            }
            else
            {
                mnSeriesCount = xValues.getLength();
                mnSeriesLength = pSequence->getLength();
                xSeriesLabels = xData->getRowDescriptions();
                xCategoryLabels = xData->getColumnDescriptions();
            }

            // determine existence of headers
            mbHasCategoryLabels = ( xCategoryLabels.getLength() > 0 );
            mbHasSeriesLabels = ( xSeriesLabels.getLength() > 0 );
        }
    }

    // chart element
    // -------------

    SvXMLElementExport* pElChart = 0;
    // get property states for autostyles
    if( mxExpPropMapper.is())
    {
        uno::Reference< beans::XPropertySet > xPropSet( rChartDoc->getArea(), uno::UNO_QUERY );
        if( xPropSet.is())
            aPropertyStates = mxExpPropMapper->Filter( xPropSet );
    }

    // attributes
    // determine class
    rtl::OUString sChartType;
    if( xDiagram.is())
    {
        // domain axes are also needed for auto-styles
        sChartType = xDiagram->getDiagramType();
        if( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.XYDiagram" )))
            mnDomainAxes = 1;
        else
            mnDomainAxes = 0;
    }

    if( bExportContent )
    {
        // attributes
        // determine class
        if( sChartType.getLength())
        {
            enum XMLTokenEnum eXMLChartType = XML_TOKEN_INVALID;
            rtl::OUString sAddInName;

            if( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.LineDiagram" )))
                eXMLChartType = XML_LINE;
            else if( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.AreaDiagram" )))
                eXMLChartType = XML_AREA;
            else if( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.BarDiagram" )))
                eXMLChartType = XML_BAR;
            else if( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.PieDiagram" )))
                eXMLChartType = XML_CIRCLE;
            else if( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.DonutDiagram" )))
                eXMLChartType = XML_RING;
            else if( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.XYDiagram" )))
            {
                eXMLChartType = XML_SCATTER;
                mnDomainAxes = 1;
            }
            else if( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.NetDiagram" )))
                eXMLChartType = XML_RADAR;
            else if( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.StockDiagram" )))
                eXMLChartType = XML_STOCK;
            else    // servie-name of add-in
            {
                eXMLChartType = XML_ADD_IN;
                sAddInName = sChartType;
            }

            if( eXMLChartType != XML_TOKEN_INVALID )
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, eXMLChartType );

            if( sAddInName.getLength())
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ADD_IN_NAME, sAddInName );
        }
        // write style name
        AddAutoStyleAttribute( aPropertyStates );

        //element
        pElChart = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_CHART, sal_True, sal_True );
    }
    else    // autostyles
    {
        CollectAutoStyle( aPropertyStates );
    }
    // remove property states for autostyles
    aPropertyStates.clear();

    // title element
    // -------------

    if( bHasMainTitle )
    {
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            uno::Reference< beans::XPropertySet > xPropSet( rChartDoc->getTitle(), uno::UNO_QUERY );
            if( xPropSet.is())
                aPropertyStates = mxExpPropMapper->Filter( xPropSet );
        }
        if( bExportContent )
        {
            uno::Reference< drawing::XShape > xShape = rChartDoc->getTitle();
            if( xShape.is())    // && "hasTitleBeenMoved"
                addPosition( xShape );

            // write style name
            AddAutoStyleAttribute( aPropertyStates );

            // element
            SvXMLElementExport aElTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, sal_True, sal_True );

            // content (text:p)
            uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
            if( xPropSet.is())
            {
                uno::Any aAny( xPropSet->getPropertyValue(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" ))));
                rtl::OUString aText;
                aAny >>= aText;
                exportText( aText );
            }
        }
        else    // autostyles
        {
            CollectAutoStyle( aPropertyStates );
        }
        // remove property states for autostyles
        aPropertyStates.clear();
    }

    // subtitle element
    // ----------------

    if( bHasSubTitle )
    {
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            uno::Reference< beans::XPropertySet > xPropSet( rChartDoc->getSubTitle(), uno::UNO_QUERY );
            if( xPropSet.is())
                aPropertyStates = mxExpPropMapper->Filter( xPropSet );
        }

        if( bExportContent )
        {
            uno::Reference< drawing::XShape > xShape = rChartDoc->getSubTitle();
            if( xShape.is())
                addPosition( xShape );

            // write style name
            AddAutoStyleAttribute( aPropertyStates );

            // element (has no subelements)
            SvXMLElementExport aElSubTitle( mrExport, XML_NAMESPACE_CHART, XML_SUBTITLE, sal_True, sal_True );

            // content (text:p)
            uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
            if( xPropSet.is())
            {
                uno::Any aAny( xPropSet->getPropertyValue(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" ))));
                rtl::OUString aText;
                aAny >>= aText;
                exportText( aText );
            }
        }
        else    // autostyles
        {
            CollectAutoStyle( aPropertyStates );
        }
        // remove property states for autostyles
        aPropertyStates.clear();
    }

    // legend element
    // --------------
    if( bHasLegend )
    {
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            uno::Reference< beans::XPropertySet > xPropSet( rChartDoc->getLegend(), uno::UNO_QUERY );
            if( xPropSet.is())
                aPropertyStates = mxExpPropMapper->Filter( xPropSet );
        }

        if( bExportContent )
        {
            uno::Reference< beans::XPropertySet > xProp( rChartDoc->getLegend(), uno::UNO_QUERY );
            if( xProp.is())
            {
                chart::ChartLegendPosition aLegendPos = chart::ChartLegendPosition_NONE;
                try
                {
                    uno::Any aAny( xProp->getPropertyValue(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Alignment" ))));
                    aAny >>= aLegendPos;
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_WARNING( "Property Align not found in ChartLegend" );
                }

                switch( aLegendPos )
                {
                    case chart::ChartLegendPosition_LEFT:
                        msString = GetXMLToken(XML_LEFT);
                        break;
                    case chart::ChartLegendPosition_RIGHT:
                        msString = GetXMLToken(XML_RIGHT);
                        break;
                    case chart::ChartLegendPosition_TOP:
                        msString = GetXMLToken(XML_TOP);
                        break;
                    case chart::ChartLegendPosition_BOTTOM:
                        msString = GetXMLToken(XML_BOTTOM);
                        break;
                }

                // export anchor position
                if( msString.getLength())
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LEGEND_POSITION, msString );

                // export absolute position
                msString = rtl::OUString();
                uno::Reference< drawing::XShape > xShape( xProp, uno::UNO_QUERY );
                if( xShape.is())
                    addPosition( xShape );
            }

            // write style name
            AddAutoStyleAttribute( aPropertyStates );

            // element
            SvXMLElementExport aLegend( mrExport, XML_NAMESPACE_CHART, XML_LEGEND, sal_True, sal_True );
        }
        else    // autostyles
        {
            CollectAutoStyle( aPropertyStates );
        }
        // remove property states for autostyles
        aPropertyStates.clear();
    }

    // plot-area element
    // -----------------
    if( xDiagram.is())
        exportPlotArea( xDiagram, bExportContent, bIncludeTable );

    // table element
    // (is included as subelement of chart)
    // ------------------------------------
    if( bExportContent )
    {
        // #85929# always export table, otherwise clipboard may loose data
//          sal_Bool bExportTable = bIncludeTable;
//          // check for clipboard flag on document => export Table
//          if( xDocPropSet.is() &&
//              ! bExportTable )
//          {
//              uno::Any aAny;
//              try
//              {
//                  aAny = xDocPropSet->getPropertyValue( ::rtl::OUString::createFromAscii( "ExportData" ));
//                  aAny >>= bExportTable;
//              }
//              catch( uno::Exception )
//              {
//                  DBG_ERROR( "Property ExportData not found" );
//              }
//          }

        // export of table element
//          if( bExportTable )

        exportTable( xData );
    }

    // close <chart:chart> element
    if( pElChart )
        delete pElChart;
}

void SchXMLExportHelper::exportTable( uno::Reference< chart::XChartDataArray >& rData )
{
    // table element
    // -------------
    mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_NAME, msTableName );
    SvXMLElementExport aTable( mrExport, XML_NAMESPACE_TABLE, XML_TABLE, sal_True, sal_True );

    if( rData.is())
    {
        double fData;

        // get NaN
        double fSolarNaN;
        SolarMath::SetNAN( fSolarNaN, FALSE );
        double fNaN = fSolarNaN;
        sal_Bool bConvertNaN = sal_False;
        uno::Reference< chart::XChartData > xChartData( rData, uno::UNO_QUERY );
        if( xChartData.is())
        {
            fNaN = xChartData->getNotANumber();
            bConvertNaN = ( ! SolarMath::IsNAN( fNaN ));
        }

        uno::Sequence< uno::Sequence< double > > xValues = rData->getData();
        if( xValues.getLength())
        {
            const uno::Sequence< double >* pSequence = xValues.getConstArray();
            const double* pData = 0;

            sal_Int32 nSeries, nDataPoint;

            // export column headers
            uno::Sequence< rtl::OUString > xSeriesLabels = rData->getColumnDescriptions();
            uno::Sequence< rtl::OUString > xCategoryLabels = rData->getRowDescriptions();
            sal_Int32 nSeriesCount, nSeriesLength;
            if( mbRowSourceColumns )
            {
                nSeriesLength = mnSeriesLength;
                nSeriesCount = mnSeriesCount;
            }
            else
            {
                nSeriesLength = mnSeriesCount;
                nSeriesCount = mnSeriesLength;
            }
            sal_Int32 nSeriesLablesLength = xSeriesLabels.getLength();
            sal_Int32 nCategoryLabelsLength = xCategoryLabels.getLength();

            // columns
            if( mbHasCategoryLabels )
            {
                // row description are put in the first column
                SvXMLElementExport aHeaderColumns( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS, sal_True, sal_True );
                SvXMLElementExport aHeaderColumn( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True );
            }
            // non-header columns
            if( mnSeriesCount )
            {
                SvXMLElementExport aColumns( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMNS, sal_True, sal_True );
                mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED,
                                       rtl::OUString::valueOf( (sal_Int64) mnSeriesCount ));
                SvXMLElementExport aColumn( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True );
            }

            // rows
            if( mbHasSeriesLabels )
            {
                SvXMLElementExport aHeaderRows( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS, sal_True, sal_True );
                SvXMLElementExport aRow( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True );
                // write one empty cell (the cell A1 is never used)
                {
                    SvXMLElementExport aEmptyCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True );
                }
                for( nSeries = 0; nSeries < nSeriesLablesLength; nSeries++ )
                {
                    mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_VALUE_TYPE, XML_STRING );
                    SvXMLElementExport aCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True );
                    exportText( xSeriesLabels[ nSeries ] );
                }
            }

            // export data
            SvXMLElementExport aRows( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROWS, sal_True, sal_True );
            for( nDataPoint = 0; nDataPoint < nSeriesLength; nDataPoint++ )
            {
                // <table:table-row>
                SvXMLElementExport aRow( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True );
                pData = pSequence[ nDataPoint ].getConstArray();

                if( mbHasCategoryLabels )
                {
                    // cells containing row descriptions (in the first column)
                    mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_VALUE_TYPE, XML_STRING );
                    SvXMLElementExport aCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True );
                    if( nDataPoint < nCategoryLabelsLength )
                        exportText( xCategoryLabels[ nDataPoint ] );
                }

                for( nSeries = 0; nSeries < nSeriesCount; nSeries++ )
                {
                    // get string by value
                    fData = pData[ nSeries ];

                        // convert NaN
                    if( bConvertNaN &&  // implies xChartData.is()
                        xChartData->isNotANumber( fData ))
                        fData = fSolarNaN;

                    SvXMLUnitConverter::convertDouble( msStringBuffer, fData );
                    msString = msStringBuffer.makeStringAndClear();

                        // <table:table-cell>
                    mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_VALUE_TYPE, XML_FLOAT );
                    mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_VALUE, msString );
                    SvXMLElementExport aCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True );

                    // <text:p>
                    exportText( msString );
                }
            }
        }
    }
}

void SchXMLExportHelper::exportPlotArea( uno::Reference< chart::XDiagram > xDiagram,
                                         sal_Bool bExportContent,
                                         sal_Bool bIncludeTable )
{
    DBG_ASSERT( xDiagram.is(), "Invalid XDiagram as parameter" );
    if( ! xDiagram.is())
        return;

    // variables for autostyles
    uno::Reference< beans::XPropertySet > xPropSet;
    std::vector< XMLPropertyState > aPropertyStates;
    sal_Int32 nStyleFamily = XML_STYLE_FAMILY_SCH_CHART_ID;
    rtl::OUString aASName;
    sal_Bool bHasTwoYAxes = sal_False;
    sal_Bool bIs3DChart = sal_False;
    drawing::HomogenMatrix aTransMatrix;

    msStringBuffer.setLength( 0 );

    // plot-area element
    // -----------------

    SvXMLElementExport* pElPlotArea = 0;
    // get property states for autostyles
    xPropSet = uno::Reference< beans::XPropertySet >( xDiagram, uno::UNO_QUERY );
    if( xPropSet.is())
    {
        if( mxExpPropMapper.is())
            aPropertyStates = mxExpPropMapper->Filter( xPropSet );
    }
    if( bExportContent )
    {
        UniReference< XMLShapeExport > rShapeExport;

        // write style name
        AddAutoStyleAttribute( aPropertyStates );

        if( msChartAddress.getLength())
        {
            mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, msChartAddress );

            uno::Reference< chart::XChartDocument > xDoc( mrExport.GetModel(), uno::UNO_QUERY );
            if( xDoc.is() )
            {
                uno::Reference< beans::XPropertySet > xDocProp( xDoc, uno::UNO_QUERY );
                if( xDocProp.is())
                {
                    uno::Any aAny;
                    sal_Bool bFirstCol, bFirstRow;

                    try
                    {
                        aAny = xDocProp->getPropertyValue(
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DataSourceLabelsInFirstColumn" )));
                        aAny >>= bFirstCol;
                        aAny = xDocProp->getPropertyValue(
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DataSourceLabelsInFirstRow" )));
                        aAny >>= bFirstRow;

                        if( bFirstCol || bFirstRow )
                        {
                            mrExport.AddAttribute( XML_NAMESPACE_CHART,
                                                   ::xmloff::token::GetXMLToken( ::xmloff::token::XML_DATA_SOURCE_HAS_LABELS ),
                                                   ( bFirstCol
                                                     ? ( bFirstRow
                                                         ?  ::xmloff::token::GetXMLToken( ::xmloff::token::XML_BOTH )
                                                         :  ::xmloff::token::GetXMLToken( ::xmloff::token::XML_COLUMN ))
                                                     : ::xmloff::token::GetXMLToken( ::xmloff::token::XML_ROW )));
                        }
                    }
                    catch( beans::UnknownPropertyException )
                    {
                        DBG_ERRORFILE( "Properties missing" );
                    }
                }
            }
        }

        if( msTableNumberList.getLength())
        {
            // this attribute is for charts embedded in calc documents only.
            // With this you are able to store a file again in 5.0 binary format
            mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_TABLE_NUMBER_LIST, msTableNumberList );
        }

        // attributes
        uno::Reference< drawing::XShape > xShape ( xDiagram, uno::UNO_QUERY );
        if( xShape.is())
        {
            addPosition( xShape );
            addSize( xShape );
        }

        if( xPropSet.is())
        {
            uno::Any aAny;
            try
            {
                aAny = xPropSet->getPropertyValue(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasSecondaryYAxis" )));
                aAny >>= bHasTwoYAxes;
            }
            catch( beans::UnknownPropertyException )
            {
                DBG_ERROR( "Property HasSecondaryYAxis not found in Diagram" );
            }

            // 3d attributes
            try
            {
                aAny = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Dim3D" )));
                aAny >>= bIs3DChart;

                if( bIs3DChart )
                {
                    rShapeExport = mrExport.GetShapeExport();
                    if( rShapeExport.is())
                        rShapeExport->export3DSceneAttributes( xPropSet );
                }
            }
            catch( uno::Exception aEx )
            {
#ifdef DBG_UTIL
        String aStr( aEx.Message );
        ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR1( "chart:exportPlotAreaException caught: %s", aBStr.GetBuffer());
#endif
            }
        }

        // element
        pElPlotArea = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_PLOT_AREA, sal_True, sal_True );

        // light sources (inside plot area element)
        if( bIs3DChart &&
            rShapeExport.is())
            rShapeExport->export3DLamps( xPropSet );
    }
    else    // autostyles
    {
        CollectAutoStyle( aPropertyStates );
    }
    // remove property states for autostyles
    aPropertyStates.clear();

    // axis elements
    // -------------
    exportAxes( xDiagram, bExportContent );

    // categories element
    // ------------------
    sal_Int32 nDataPointOffset = mbHasSeriesLabels? 1 : 0;
    sal_Int32 nSeriesOffset = mbHasCategoryLabels ? 1 : 0;

    if( bExportContent &&
        mbHasCategoryLabels )
    {
        // fill msString with cell-range-address of categories
        if( bIncludeTable )
        {
            // export own table references
            msStringBuffer.append( msTableName );

            getCellAddress( 0, nDataPointOffset );
            msStringBuffer.append( (sal_Unicode) ':' );
            getCellAddress( 0, mnSeriesLength + nDataPointOffset - 1 );
        }
        else
        {
            // deprecated
//              msString = msCategoriesAddress;
        }

        if( msStringBuffer.getLength())
        {
            msString = msStringBuffer.makeStringAndClear();
            mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, msString );
        }
        SvXMLElementExport aCategories( mrExport, XML_NAMESPACE_CHART, XML_CATEGORIES, sal_True, sal_True );
    }

    // series elements
    // ---------------
    msStringBuffer.setLength( 0 );
    SvXMLElementExport* pSeries = NULL;
    rtl::OUString aSeriesASName;
    sal_Bool bWrite = sal_False;
    sal_Int32 nAttachedAxis;

    for( sal_Int32 nSeries = mnDomainAxes; nSeries < mnSeriesCount; nSeries++ )
    {
        nAttachedAxis = chart::ChartAxisAssign::PRIMARY_Y;

        // get property states for autostyles
        xPropSet = xDiagram->getDataRowProperties( nSeries );
        if( xPropSet.is())
        {
            // determine attached axis
            try
            {
                uno::Any aAny( xPropSet->getPropertyValue(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Axis" ))));
                aAny >>= nAttachedAxis;
            }
            catch( beans::UnknownPropertyException )
            {
                DBG_WARNING( "Required property not found in DataRowProperties" );
            }

            if( mxExpPropMapper.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xPropSet );
            }
        }

        if( bExportContent )
        {
            if( bIncludeTable )
            {
                // export own table references
                msStringBuffer.append( msTableName );

                getCellAddress( nSeries + nDataPointOffset, nSeriesOffset );
                msStringBuffer.append( (sal_Unicode) ':' );
                getCellAddress( nSeries + nDataPointOffset, nSeriesOffset + mnSeriesLength - 1 );

                msString = msStringBuffer.makeStringAndClear();
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, msString );

                // reference to label
                if( mbHasSeriesLabels )
                {
                    msStringBuffer.append( msTableName );
                    getCellAddress( nSeries + nDataPointOffset, 0 );
                    msString = msStringBuffer.makeStringAndClear();
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, msString );
                }
            }
            else
            {
                // #81525# convert addresses for xy charts
                // this should be done by calc in the future

                // deprecated
//                  if( maSeriesAddresses.getLength() > nSeries )
//                  {
//                      mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS,
//                                             maSeriesAddresses[ nSeries ].DataRangeAddress );
//                      if( maSeriesAddresses[ nSeries ].LabelAddress.getLength())
//                          mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS,
//                                                 maSeriesAddresses[ nSeries ].LabelAddress );
//                  }
            }

            if( bHasTwoYAxes )
            {
                if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                else
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
            }

            // write style name
//            AddAutoStyleAttribute( aPropertyStates );   // can't be used here because we need the name
            if( aPropertyStates.size())
            {
                DBG_ASSERT( ! maAutoStyleNameQueue.empty(), "Autostyle queue empty!" );
                aSeriesASName = maAutoStyleNameQueue.front();
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_STYLE_NAME, aSeriesASName );
                maAutoStyleNameQueue.pop();
            }

            // open series element until end of for loop
            pSeries = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_SERIES, sal_True, sal_True );
        }
        else    // autostyles
        {
            CollectAutoStyle( aPropertyStates );
        }
        // remove property states for autostyles
        aPropertyStates.clear();

        // domain element if necessary
        // ---------------------------
        if( bExportContent &&
            mnDomainAxes > 0 &&
            nSeries ==  mnDomainAxes )
        {
            msStringBuffer.setLength( 0 );
            for( sal_Int32 nDomain = 0; nDomain < mnDomainAxes; nDomain++ )
            {
                if( bIncludeTable )
                {
                    // first series has a domain, that is the first table row (column)
                    msStringBuffer.append( msTableName );

                    getCellAddress( nDomain + nSeriesOffset, nDataPointOffset );
                    msStringBuffer.append( (sal_Unicode) ':' );
                    getCellAddress( nDomain + nSeriesOffset, nDataPointOffset + mnSeriesLength - 1 );
                }
                else
                {
                    // #81525# convert addresses for xy charts
                    // this should be done by calc in the future

                    // deprecated
//                      if( maSeriesAddresses.getLength() > 0 )
//                      {
//                          msStringBuffer.append( maSeriesAddresses[ 0 ].DataRangeAddress );
//                      }
                }

                msString = msStringBuffer.makeStringAndClear();
                if( msString.getLength())
                {
                    mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, msString );
                }
                SvXMLElementExport aDomain( mrExport, XML_NAMESPACE_CHART, XML_DOMAIN, sal_True, sal_True );
            }
        }

        // data-points
        // -----------
        // write data-points only if they contain autostyles
        // objects with equal autostyles are grouped using the attribute
        // repeat="number"

        // Note: if only the nth data-point has autostyles there is an element
        // without style and repeat="n-1" attribute written in advance.

        sal_Int32 nRepeated = 1;
        if( mxExpPropMapper.is())
        {
            sal_Bool bIsEmpty = sal_False;
            rtl::OUString aLastASName;
            aASName = rtl::OUString();

            for( sal_Int32 nElement = 0; nElement < mnSeriesLength; nElement++ )
            {
                // get property states for autostyles
                try
                {
                    xPropSet = xDiagram->getDataPointProperties( nElement, nSeries );
                }
                catch( uno::Exception aEx )
                {
                    String aStr( aEx.Message );
                    ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
                    DBG_ERROR1( "Exception caught during Export of data point: %s", aBStr.GetBuffer());
                }
                if( xPropSet.is())
                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
                bIsEmpty = ( aPropertyStates.size() == 0 );

                if( bExportContent )
                {
                    if( bIsEmpty )
                        aASName = ::rtl::OUString();
                    else
                    {
//                          AddAutoStyleAttribute( aPropertyStates );   // can't be used here because we need the name
                        DBG_ASSERT( ! maAutoStyleNameQueue.empty(), "Autostyle queue empty!" );
                        aASName = maAutoStyleNameQueue.front();
                        maAutoStyleNameQueue.pop();
                    }


                    //  The following conditional realizes a run-length compression.  For every run of data
                    //  points with the same style only one point is written together with a repeat count.
                    //  The style of the current data point is compared with that of the last data point.
                    //  If they differ, then the _last_ data point and, if greater than 1, the repreat count
                    //  are written, else the repeat count is increased but no output takes place.
                    //  This has two consequences: 1. the first data point is skipped,
                    //  because it can not be compared to a predecessor and 2. the last data point (or series
                    //  of data points with the same style) is written outside and after the enclosing loop.
                    if( nElement )
                    {
                        if( aASName.equals( aLastASName ))
                        {
                            nRepeated++;
                        }
                        else
                        {
                            //  Write the style of the last data point(s).

                            //  Write reapeat counter (if data point run contains more than one point).
                            if( nRepeated > 1 )
                            {
                                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_REPEATED,
                                                       rtl::OUString::valueOf( (sal_Int64)( nRepeated ) ));
                            }

                            //  Write style if it is present and is not the same as that of the data series.
                            if( aLastASName.getLength() &&
                                ! aLastASName.equals( aSeriesASName ))
                            {
                                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_STYLE_NAME, aLastASName );
                            }

                            //  Write the actual point data.
                            SvXMLElementExport aPoint( mrExport, XML_NAMESPACE_CHART, XML_DATA_POINT, sal_True, sal_True );

                            //  Reset repeat counter for the new style.
                            nRepeated = 1;
                            aLastASName = aASName;
                        }
                    }
                    else
                        //  Remember the name of the first data point's style as that of the next point's
                        //  predecessor.
                        aLastASName = aASName;
                }
                else
                {
                    if( ! bIsEmpty )
                        CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }   //  End of loop over data points.

            //  Now write the style for the last data point(s).
            if( bExportContent )
            {
                if( nRepeated > 1 )
                {
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_REPEATED,
                                           rtl::OUString::valueOf( (sal_Int64)( nRepeated ) ));
                }

                if( ! bIsEmpty &&
                    aLastASName.getLength() &&
                    ! aLastASName.equals( aSeriesASName ))
                {
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_STYLE_NAME, aLastASName );
                }

                SvXMLElementExport aPoint( mrExport, XML_NAMESPACE_CHART, XML_DATA_POINT, sal_True, sal_True );
            }
        }

        // close series element
        if( pSeries )
            delete pSeries;
    }

    // wall element
    // ------------

    uno::Reference< chart::X3DDisplay > xWallSupplier( xDiagram, uno::UNO_QUERY );
    if( mxExpPropMapper.is() &&
        xWallSupplier.is())
    {
        // remove property states for autostyles
        aPropertyStates.clear();

        uno::Reference< beans::XPropertySet > xWallPropSet( xWallSupplier->getWall(), uno::UNO_QUERY );
        if( xWallPropSet.is())
        {
            aPropertyStates = mxExpPropMapper->Filter( xWallPropSet );

            if( aPropertyStates.size() > 0 )
            {
                // write element
                if( bExportContent )
                {
                    // add style name attribute
                    AddAutoStyleAttribute( aPropertyStates );

                    SvXMLElementExport aWall( mrExport, XML_NAMESPACE_CHART, XML_WALL, sal_True, sal_True );
                }
                else    // autostyles
                {
                    CollectAutoStyle( aPropertyStates );
                }
            }
        }
    }

    if( pElPlotArea )
        delete pElPlotArea;
}

void SchXMLExportHelper::exportAxes( uno::Reference< chart::XDiagram > xDiagram, sal_Bool bExportContent )
{
    DBG_ASSERT( xDiagram.is(), "Invalid XDiagram as parameter" );
    if( ! xDiagram.is())
        return;

    // variables for autostyles
    const ::rtl::OUString sNumFormat( ::rtl::OUString::createFromAscii( "NumberFormat" ));
    sal_Int32 nNumberFormat;
    uno::Reference< beans::XPropertySet > xPropSet;
    std::vector< XMLPropertyState > aPropertyStates;
    sal_Int32 nStyleFamily = XML_STYLE_FAMILY_SCH_CHART_ID;
    rtl::OUString aASName;

    // get some properties from document first
    sal_Bool bHasXAxis = sal_False,
        bHasYAxis = sal_False,
        bHasZAxis = sal_False,
        bHasSecondaryXAxis = sal_False,
        bHasSecondaryYAxis = sal_False;
    sal_Bool bHasXAxisTitle = sal_False,
        bHasYAxisTitle = sal_False,
        bHasZAxisTitle = sal_False;
    sal_Bool bHasXAxisMajorGrid = sal_False,
        bHasXAxisMinorGrid = sal_False,
        bHasYAxisMajorGrid = sal_False,
        bHasYAxisMinorGrid = sal_False,
        bHasZAxisMajorGrid = sal_False,
        bHasZAxisMinorGrid = sal_False;

#if 0
    uno::Reference< beans::XPropertySet > xProp( xDiagram, uno::UNO_QUERY );
    if( xProp.is())
    {
        try
        {
            uno::Any aAny;

            // check for supported services ...
            uno::Reference< lang::XServiceInfo > xServ( xDiagram, uno::UNO_QUERY );
            if( xServ.is())
            {
                bHasXAxis = xServ->supportsService(
                    rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxisXSupplier" ));
                bHasYAxis = xServ->supportsService(
                    rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxisYSupplier" ));
                bHasZAxis = xServ->supportsService(
                    rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxisZSupplier" ));
                bHasSecondaryXAxis = xServ->supportsService(
                    rtl::OUString::createFromAscii( "com.sun.star.chart.ChartTwoAxisXSupplier" ));
                bHasSecondaryYAxis = xServ->supportsService(
                    rtl::OUString::createFromAscii( "com.sun.star.chart.ChartTwoAxisYSupplier" ));
            }

            // ... and then the properties provided by this service
            if( bHasXAxis )
            {
                aAny = xProp->getPropertyValue(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasXAxis" )));
                aAny >>= bHasXAxis;
            }
            if( bHasYAxis )
            {
                aAny = xProp->getPropertyValue(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasYAxis" )));
                aAny >>= bHasYAxis;
            }
            if( bHasZAxis )
            {
                aAny = xProp->getPropertyValue(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasZAxis" )));
                aAny >>= bHasZAxis;
            }
            if( bHasSecondaryXAxis )
            {
                aAny = xProp->getPropertyValue(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasSecondaryXAxis" )));
                aAny >>= bHasSecondaryXAxis;
            }
            if( bHasSecondaryYAxis )
            {
                aAny = xProp->getPropertyValue(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasSecondaryYAxis" )));
                aAny >>= bHasSecondaryYAxis;
            }

            aAny = xProp->getPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasXAxisTitle" )));
            aAny >>= bHasXAxisTitle;
            aAny = xProp->getPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasYAxisTitle" )));
            aAny >>= bHasYAxisTitle;
            aAny = xProp->getPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasZAxisTitle" )));
            aAny >>= bHasZAxisTitle;

            aAny = xProp->getPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasXAxisGrid" )));
            aAny >>= bHasXAxisMajorGrid;
            aAny = xProp->getPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasXAxisHelpGrid" )));
            aAny >>= bHasXAxisMinorGrid;

            aAny = xProp->getPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasYAxisGrid" )));
            aAny >>= bHasYAxisMajorGrid;
            aAny = xProp->getPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasYAxisHelpGrid" )));
            aAny >>= bHasYAxisMinorGrid;

            aAny = xProp->getPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasZAxisGrid" )));
            aAny >>= bHasZAxisMajorGrid;
            aAny = xProp->getPropertyValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasZAxisHelpGrid" )));
            aAny >>= bHasZAxisMinorGrid;
        }
        catch( beans::UnknownPropertyException )
        {
            DBG_WARNING( "Required property not found in ChartDocument" );
        }
    }
#else
    MultiPropertySetHandler aDiagramProperties (xDiagram);

    //  Check for supported services and then the properties provided by this service.
    Reference<lang::XServiceInfo> xServiceInfo (xDiagram, UNO_QUERY);
    if (xServiceInfo.is())
    {
        if (xServiceInfo->supportsService(
            rtl::OUString::createFromAscii ("com.sun.star.chart.ChartAxisXSupplier")))
        {
            aDiagramProperties.Add (
                OUString(RTL_CONSTASCII_USTRINGPARAM("HasXAxis")), bHasXAxis);
        }
        if (xServiceInfo->supportsService(
            rtl::OUString::createFromAscii ("com.sun.star.chart.ChartAxisYSupplier")))
        {
            aDiagramProperties.Add (
                OUString(RTL_CONSTASCII_USTRINGPARAM("HasYAxis")), bHasXAxis);
        }
        if (xServiceInfo->supportsService(
            rtl::OUString::createFromAscii ("com.sun.star.chart.ChartAxisZSupplier")))
        {
            aDiagramProperties.Add (
                OUString(RTL_CONSTASCII_USTRINGPARAM("HasZAxis")), bHasXAxis);
        }
        if (xServiceInfo->supportsService(
            rtl::OUString::createFromAscii ("com.sun.star.chart.ChartTwoAxisSupplier")))
        {
            aDiagramProperties.Add (
                OUString(RTL_CONSTASCII_USTRINGPARAM("HasSecondaryXAxis")), bHasSecondaryXAxis);
        }
        if (xServiceInfo->supportsService(
            rtl::OUString::createFromAscii ("com.sun.star.chart.ChartTwoAxisYSupplier")))
        {
            aDiagramProperties.Add (
                OUString(RTL_CONSTASCII_USTRINGPARAM("HasSecondaryYAxis")), bHasSecondaryYAxis);
        }
    }

    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasXAxisTitle")), bHasXAxisTitle);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasYAxisTitle")), bHasYAxisTitle);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasZAxisTitle")), bHasZAxisTitle);

    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasXAxisGrid")), bHasXAxisMajorGrid);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasYAxisGrid")), bHasYAxisMajorGrid);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasZAxisGrid")), bHasZAxisMajorGrid);

    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasXAxisHelpGrid")), bHasXAxisMinorGrid);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasYAxisHelpGrid")), bHasYAxisMinorGrid);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasZAxisHelpGrid")), bHasZAxisMinorGrid);

    if ( ! aDiagramProperties.GetProperties ())
        DBG_WARNING ("Required properties not found in Chart diagram");
#endif

    SvXMLElementExport* pAxis = NULL;

    // x axis
    // -------

    if( bHasXAxis )
    {
        uno::Reference< chart::XAxisXSupplier > xAxisSupp( xDiagram, uno::UNO_QUERY );
        if( xAxisSupp.is())
        {
            // get property states for autostyles
            if( mxExpPropMapper.is())
            {
                xPropSet = xAxisSupp->getXAxis();
                if( xPropSet.is())
                {
                    uno::Any aNumAny = xPropSet->getPropertyValue( sNumFormat );
                    aNumAny >>= nNumberFormat;
                    mrExport.addDataStyle( nNumberFormat );

                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
                }
            }
            if( bExportContent )
            {
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, (mnDomainAxes > 0)
                                       ? XML_DOMAIN      // scatter (or bubble) chart
                                       : XML_CATEGORY );

                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_NAME, XML_PRIMARY_X );


                // write style name
                AddAutoStyleAttribute( aPropertyStates );

                // element
                pAxis = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_AXIS, sal_True, sal_True );
            }
            else    // autostyles
            {
                CollectAutoStyle( aPropertyStates );
            }
            aPropertyStates.clear();

            // axis-title
            if( bHasXAxisTitle )
            {
                uno::Reference< beans::XPropertySet > xTitleProp( xAxisSupp->getXAxisTitle(), uno::UNO_QUERY );
                if( xTitleProp.is())
                {
                    aPropertyStates = mxExpPropMapper->Filter( xTitleProp );
                    if( bExportContent )
                    {
                        ::rtl::OUString aText;
                        uno::Any aAny( xTitleProp->getPropertyValue(
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" ))));
                        aAny >>= aText;

                        uno::Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                        if( xShape.is())
                            addPosition( xShape );

                        AddAutoStyleAttribute( aPropertyStates );
                        SvXMLElementExport aTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, sal_True, sal_True );

                        // paragraph containing title
                        exportText( aText );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                    aPropertyStates.clear();
                }
            }

            // grid
            uno::Reference< beans::XPropertySet > xMajorGrid( xAxisSupp->getXMainGrid(), uno::UNO_QUERY );
            if( bHasXAxisMajorGrid && xMajorGrid.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xMajorGrid );
                if( bExportContent )
                {
                    AddAutoStyleAttribute( aPropertyStates );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_MAJOR );
                    SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
                }
                else
                {
                    CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }
            uno::Reference< beans::XPropertySet > xMinorGrid( xAxisSupp->getXHelpGrid(), uno::UNO_QUERY );
            if( bHasXAxisMinorGrid && xMinorGrid.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xMinorGrid );
                if( bExportContent )
                {
                    AddAutoStyleAttribute( aPropertyStates );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_MINOR );
                    SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
                }
                else
                {
                    CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }
            if( pAxis )
            {
                delete pAxis;
                pAxis = NULL;
            }
        }
    }

    // secondary x axis
    if( bHasSecondaryXAxis )
    {
        uno::Reference< chart::XTwoAxisXSupplier > xAxisSupp( xDiagram, uno::UNO_QUERY );
        if( xAxisSupp.is())
        {
            // get property states for autostyles
            if( mxExpPropMapper.is())
            {
                xPropSet = xAxisSupp->getSecondaryXAxis();
                if( xPropSet.is())
                {
                    uno::Any aNumAny = xPropSet->getPropertyValue( sNumFormat );
                    aNumAny >>= nNumberFormat;
                    mrExport.addDataStyle( nNumberFormat );

                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
                }
            }
            if( bExportContent )
            {
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, (mnDomainAxes > 0)
                                       ? XML_DOMAIN      // scatter (or bubble) chart
                                       : XML_CATEGORY );

                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_NAME, XML_SECONDARY_X );
                AddAutoStyleAttribute( aPropertyStates );
                pAxis = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_AXIS, sal_True, sal_True );
            }
            else    // autostyles
            {
                CollectAutoStyle( aPropertyStates );
            }
            aPropertyStates.clear();

            if( pAxis )
            {
                delete pAxis;
                pAxis = NULL;
            }
        }
    }

    // y axis
    // -------

    if( bHasYAxis ||
        ! ( bHasYAxis && bHasSecondaryYAxis ))  // no y axes at all => write at least primary
    {
        uno::Reference< chart::XAxisYSupplier > xAxisSupp( xDiagram, uno::UNO_QUERY );
        if( xAxisSupp.is())
        {
            // get property states for autostyles
            if( mxExpPropMapper.is())
            {
                xPropSet = xAxisSupp->getYAxis();
                if( xPropSet.is())
                {
                    uno::Any aNumAny = xPropSet->getPropertyValue( sNumFormat );
                    aNumAny >>= nNumberFormat;
                    mrExport.addDataStyle( nNumberFormat );

                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
                }
            }
            if( bExportContent )
            {
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_VALUE );
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_NAME, XML_PRIMARY_Y );
                AddAutoStyleAttribute( aPropertyStates );
                pAxis = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_AXIS, sal_True, sal_True );
            }
            else
            {
                CollectAutoStyle( aPropertyStates );
            }
            aPropertyStates.clear();

            // axis-title
            if( bHasYAxisTitle )
            {
                uno::Reference< beans::XPropertySet > xTitleProp( xAxisSupp->getYAxisTitle(), uno::UNO_QUERY );
                if( xTitleProp.is())
                {
                    aPropertyStates = mxExpPropMapper->Filter( xTitleProp );
                    if( bExportContent )
                    {
                        ::rtl::OUString aText;
                        uno::Any aAny( xTitleProp->getPropertyValue(
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" ))));
                        aAny >>= aText;

                        uno::Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                        if( xShape.is())
                            addPosition( xShape );

                        AddAutoStyleAttribute( aPropertyStates );
                        SvXMLElementExport aTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, sal_True, sal_True );

                        // paragraph containing title
                        exportText( aText );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                    aPropertyStates.clear();
                }
            }

            // grid
            uno::Reference< beans::XPropertySet > xMajorGrid( xAxisSupp->getYMainGrid(), uno::UNO_QUERY );
            if( bHasYAxisMajorGrid && xMajorGrid.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xMajorGrid );

                if( bExportContent )
                {
                    AddAutoStyleAttribute( aPropertyStates );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_MAJOR );
                    SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
                }
                else
                {
                    CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }
            // minor grid
            uno::Reference< beans::XPropertySet > xMinorGrid( xAxisSupp->getYHelpGrid(), uno::UNO_QUERY );
            if( bHasYAxisMinorGrid && xMinorGrid.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xMinorGrid );

                if( bExportContent )
                {
                    AddAutoStyleAttribute( aPropertyStates );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_MINOR );
                    SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
                }
                else
                {
                    CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }
            if( pAxis )
            {
                delete pAxis;
                pAxis = NULL;
            }
        }
    }
    if( bHasSecondaryYAxis )
    {
        uno::Reference< chart::XTwoAxisYSupplier > xAxisSupp( xDiagram, uno::UNO_QUERY );
        if( xAxisSupp.is())
        {
            // get property states for autostyles
            if( mxExpPropMapper.is())
            {
                xPropSet = xAxisSupp->getSecondaryYAxis();
                if( xPropSet.is())
                {
                    uno::Any aNumAny = xPropSet->getPropertyValue( sNumFormat );
                    aNumAny >>= nNumberFormat;
                    mrExport.addDataStyle( nNumberFormat );

                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
                }
            }
            if( bExportContent )
            {
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_VALUE );
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_NAME, XML_SECONDARY_Y );
                AddAutoStyleAttribute( aPropertyStates );
                pAxis = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_AXIS, sal_True, sal_True );
            }
            else    // autostyles
            {
                CollectAutoStyle( aPropertyStates );
            }
            aPropertyStates.clear();
            if( pAxis )
            {
                delete pAxis;
                pAxis = NULL;
            }
        }
    }

    // z axis
    // -------

    if( bHasZAxis )
    {
        uno::Reference< chart::XAxisZSupplier > xAxisSupp( xDiagram, uno::UNO_QUERY );
        if( xAxisSupp.is())
        {
            // get property states for autostyles
            if( mxExpPropMapper.is())
            {
                xPropSet = xAxisSupp->getZAxis();
                if( xPropSet.is())
                {
                    uno::Any aNumAny = xPropSet->getPropertyValue( sNumFormat );
                    aNumAny >>= nNumberFormat;
                    mrExport.addDataStyle( nNumberFormat );

                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
                }
            }
            if( bExportContent )
            {
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_SERIES );
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_NAME, XML_PRIMARY_Z );

                AddAutoStyleAttribute( aPropertyStates );
                pAxis = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_AXIS, sal_True, sal_True );
            }
            else
            {
                CollectAutoStyle( aPropertyStates );
            }
            aPropertyStates.clear();

            // axis-title
            if( bHasZAxisTitle )
            {
                uno::Reference< beans::XPropertySet > xTitleProp( xAxisSupp->getZAxisTitle(), uno::UNO_QUERY );
                if( xTitleProp.is())
                {
                    aPropertyStates = mxExpPropMapper->Filter( xTitleProp );
                    if( bExportContent )
                    {
                        ::rtl::OUString aText;
                        uno::Any aAny( xTitleProp->getPropertyValue(
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" ))));
                        aAny >>= aText;

                        uno::Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                        if( xShape.is())
                            addPosition( xShape );

                        AddAutoStyleAttribute( aPropertyStates );
                        SvXMLElementExport aTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, sal_True, sal_True );

                        // paragraph containing title
                        exportText( aText );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                    aPropertyStates.clear();
                }
            }

            // grid
            uno::Reference< beans::XPropertySet > xMajorGrid( xAxisSupp->getZMainGrid(), uno::UNO_QUERY );
            if( bHasZAxisMajorGrid && xMajorGrid.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xMajorGrid );

                if( bExportContent )
                {
                    AddAutoStyleAttribute( aPropertyStates );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_MAJOR );
                    SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
                }
                else
                {
                    CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }
            // minor grid
            uno::Reference< beans::XPropertySet > xMinorGrid( xAxisSupp->getZHelpGrid(), uno::UNO_QUERY );
            if( bHasZAxisMinorGrid && xMinorGrid.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xMinorGrid );

                if( bExportContent )
                {
                    AddAutoStyleAttribute( aPropertyStates );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_MINOR );
                    SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
                }
                else
                {
                    CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }
        }
        if( pAxis )
        {
            delete pAxis;
            pAxis = NULL;
        }
    }
}

void SchXMLExportHelper::getCellAddress( sal_Int32 nCol, sal_Int32 nRow )
{
    msStringBuffer.append( (sal_Unicode)'.' );
    if( nCol < 26 )
        msStringBuffer.append( (sal_Unicode)('A' + nCol) );
    else if( nCol < 702 )
    {
        msStringBuffer.append( (sal_Unicode)('A' + nCol / 26 - 1 ));
        msStringBuffer.append( (sal_Unicode)('A' + nCol % 26) );
    }
    else
    {
        msStringBuffer.append( (sal_Unicode)('A' + nCol / 702 - 1 ));
        msStringBuffer.append( (sal_Unicode)('A' + (nCol % 702) / 26 ));
        msStringBuffer.append( (sal_Unicode)('A' + nCol % 26) );
    }

    msStringBuffer.append( nRow + (sal_Int32)1 );
}

void SchXMLExportHelper::addPosition( uno::Reference< drawing::XShape > xShape )
{
    if( xShape.is())
    {
        awt::Point aPos = xShape->getPosition();

        mrExport.GetMM100UnitConverter().convertMeasure( msStringBuffer, aPos.X );
        msString = msStringBuffer.makeStringAndClear();
        mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_X, msString );

        mrExport.GetMM100UnitConverter().convertMeasure( msStringBuffer, aPos.Y );
        msString = msStringBuffer.makeStringAndClear();
        mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_Y, msString );
    }
}

void SchXMLExportHelper::addSize( uno::Reference< drawing::XShape > xShape )
{
    if( xShape.is())
    {
        awt::Size aSize = xShape->getSize();

        mrExport.GetMM100UnitConverter().convertMeasure( msStringBuffer, aSize.Width );
        msString = msStringBuffer.makeStringAndClear();
        mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH,  msString );

        mrExport.GetMM100UnitConverter().convertMeasure( msStringBuffer, aSize.Height );
        msString = msStringBuffer.makeStringAndClear();
        mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_HEIGHT, msString );
    }
}

void SchXMLExportHelper::swapDataArray( com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< double > >& rSequence )
{
    sal_Int32 nOuterSize = rSequence.getLength();
    sal_Int32 nInnerSize = rSequence[0].getLength();    // assume that all subsequences have same length
    sal_Int32 i, o;

    uno::Sequence< uno::Sequence< double > > aResult( nInnerSize );
    uno::Sequence< double >* pArray = aResult.getArray();
    for( i = 0; i < nInnerSize; i++ )
    {
        pArray[ i ].realloc( nOuterSize );
        for( o = 0 ; o < nOuterSize ; o++ )
            aResult[ i ][ o ] = rSequence[ o ][ i ];
    }

    rSequence = aResult;
}

void SchXMLExportHelper::CollectAutoStyle( const std::vector< XMLPropertyState >& aStates )
{
    if( aStates.size())
        maAutoStyleNameQueue.push( GetAutoStylePoolP().Add( XML_STYLE_FAMILY_SCH_CHART_ID, aStates ));
}

void SchXMLExportHelper::AddAutoStyleAttribute( const std::vector< XMLPropertyState >& aStates )
{
    if( aStates.size())
    {
        DBG_ASSERT( ! maAutoStyleNameQueue.empty(), "Autostyle queue empty!" );

        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_STYLE_NAME, maAutoStyleNameQueue.front());
        maAutoStyleNameQueue.pop();
    }
}

void SchXMLExportHelper::exportText( const ::rtl::OUString& rText )
{
    sal_Int32 nStartPos = 0;
    sal_Int32 nEndPos = rText.getLength();
    sal_Unicode cChar;

    SvXMLElementExport aPara( mrExport, XML_NAMESPACE_TEXT,
                              ::xmloff::token::GetXMLToken( ::xmloff::token::XML_P ),
                              sal_True, sal_False );

    for( sal_Int32 nPos = 0; nPos < nEndPos; nPos++ )
    {
        cChar = rText[ nPos ];
        switch( cChar )
        {
            case 0x0009:        // tabulator
                {
                    if( nPos > nStartPos )
                        mrExport.GetDocHandler()->characters( rText.copy( nStartPos, (nPos - nStartPos)) );
                    nStartPos = nPos + 1;

                    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_TEXT,
                                              ::xmloff::token::GetXMLToken( ::xmloff::token::XML_TAB_STOP ),
                                              sal_False, sal_False );
                }
                break;

            case 0x000A:        // linefeed
                {
                    if( nPos > nStartPos )
                        mrExport.GetDocHandler()->characters( rText.copy( nStartPos, (nPos - nStartPos)) );
                    nStartPos = nPos + 1;

                    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_TEXT,
                                              ::xmloff::token::GetXMLToken( ::xmloff::token::XML_LINE_BREAK ),
                                              sal_False, sal_False );
                }
                break;
        }
    }
    if( nEndPos > nStartPos )
    {
        if( nStartPos == 0 )
            mrExport.GetDocHandler()->characters( rText );
        else
            mrExport.GetDocHandler()->characters( rText.copy( nStartPos, (nEndPos - nStartPos)) );
    }
}

// ========================================
// class SchXMLExport
// ========================================

SchXMLExport::SchXMLExport( sal_uInt16 nExportFlags ) :
        SvXMLExport( MAP_CM, ::xmloff::token::XML_CHART, nExportFlags ),
        maAutoStylePool( *this ),
        maExportHelper( *this, maAutoStylePool )
{
}


SchXMLExport::~SchXMLExport()
{
    // stop progress view
    if( mxStatusIndicator.is())
    {
        mxStatusIndicator->end();
        mxStatusIndicator->reset();
    }
}

void SchXMLExport::_ExportStyles( sal_Bool bUsed )
{
    SvXMLExport::_ExportStyles( bUsed );
}

void SchXMLExport::_ExportMasterStyles()
{
    // not available in chart
    DBG_WARNING( "Master Style Export requested. Not available for Chart" );
}

void SchXMLExport::_ExportAutoStyles()
{
    // there are no styles that require their own autostyles
    if( getExportFlags() & EXPORT_CONTENT )
    {
        uno::Reference< chart::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
        if( xChartDoc.is())
        {
            maExportHelper.collectAutoStyles( xChartDoc );
            maExportHelper.exportAutoStyles();
        }
        else
        {
            DBG_ERROR( "Couldn't export chart due to wrong XModel (must be XChartDocument)" );
        }
    }
}

void SchXMLExport::_ExportContent()
{
    uno::Reference< chart::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
    if( xChartDoc.is())
    {
        // add size for chart element in standalone case
        uno::Reference< drawing::XShape > xShape ( xChartDoc->getArea(), uno::UNO_QUERY );
        if( xShape.is())
        {
            awt::Size aSize = xShape->getSize();
            rtl::OUStringBuffer sStringBuffer;
            rtl::OUString sString;

            GetMM100UnitConverter().convertMeasure( sStringBuffer, aSize.Width );
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH,  sString );

            GetMM100UnitConverter().convertMeasure( sStringBuffer, aSize.Height );
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_SVG, XML_HEIGHT, sString );
        }

        // determine if data comes from the outside
        sal_Bool bIncludeTable = sal_True;
        uno::Reference< lang::XServiceInfo > xServ( xChartDoc, uno::UNO_QUERY );
        if( xServ.is())
        {
            if( xServ->supportsService(
                rtl::OUString::createFromAscii( "com.sun.star.chart.ChartTableAddressSupplier" )))
            {
                uno::Reference< beans::XPropertySet > xProp( xServ, uno::UNO_QUERY );
                if( xProp.is())
                {
                    uno::Any aAny;
                    try
                    {
                        ::rtl::OUString sChartAddress;
                        aAny = xProp->getPropertyValue(
                            ::rtl::OUString::createFromAscii( "ChartRangeAddress" ));
                        aAny >>= sChartAddress;
                        maExportHelper.SetChartRangeAddress( sChartAddress );

                        ::rtl::OUString sTableNumberList;
                        aAny = xProp->getPropertyValue(
                            ::rtl::OUString::createFromAscii( "TableNumberList" ));
                        aAny >>= sTableNumberList;
                        maExportHelper.SetTableNumberList( sTableNumberList );

                        // do not include own table if there are external addresses
                        bIncludeTable = (sChartAddress.getLength() == 0);
                    }
                    catch( beans::UnknownPropertyException )
                    {
                        DBG_ERROR( "Property ChartRangeAddress not supported by ChartDocument" );
                    }
                }
            }
        }
        maExportHelper.exportChart( xChartDoc, bIncludeTable );
    }
    else
    {
        DBG_ERROR( "Couldn't export chart due to wrong XModel" );
    }
}

void SchXMLExport::SetProgress( sal_Int32 nPercentage )
{
    // set progress view
    if( mxStatusIndicator.is())
        mxStatusIndicator->setValue( nPercentage );
}

// export components ========================================

// first version: everything goes in one storage

uno::Sequence< OUString > SAL_CALL SchXMLExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLExport.Compact" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SchXMLExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLExport();
}

// ============================================================

// multiple storage version: one for content / styles / meta

uno::Sequence< OUString > SAL_CALL SchXMLExport_Styles_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLStylesExporter" ));
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Styles_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLExport.Styles" ));
}

uno::Reference< uno::XInterface > SAL_CALL SchXMLExport_Styles_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLExport( EXPORT_STYLES );
}

// ------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL SchXMLExport_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLContentExporter" ));
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Content_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLExport.Content" ));
}

uno::Reference< uno::XInterface > SAL_CALL SchXMLExport_Content_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLExport( EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_FONTDECLS );
}

// ------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL SchXMLExport_Meta_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLMetaExporter" ));
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Meta_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLExport.Meta" ));
}

uno::Reference< uno::XInterface > SAL_CALL SchXMLExport_Meta_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLExport( EXPORT_META );
}
