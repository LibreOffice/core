/*************************************************************************
 *
 *  $RCSfile: SchXMLPlotAreaContext.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:03 $
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

#include "SchXMLChartContext.hxx"
#include "SchXMLPlotAreaContext.hxx"
#include "SchXMLImport.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include "prstylei.hxx"
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
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
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGMAPPING_HPP_
#include <com/sun/star/util/XStringMapping.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif

using namespace com::sun::star;

static __FAR_DATA SvXMLEnumMapEntry aXMLAxisClassMap[] =
{
    { sXML_category,    SCH_XML_AXIS_CATEGORY   },
    { sXML_domain,      SCH_XML_AXIS_DOMAIN     },
    { sXML_value,       SCH_XML_AXIS_VALUE      },
    { sXML_series,      SCH_XML_AXIS_SERIES     },
    { 0, 0 }
};

SchXMLPlotAreaContext::SchXMLPlotAreaContext( SchXMLImportHelper& rImpHelper,
                                              SvXMLImport& rImport, const rtl::OUString& rLocalName,
                                              uno::Sequence< chart::ChartSeriesAddress >& rSeriesAddresses,
                                              rtl::OUString& rCategoriesAddress ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrSeriesAddresses( rSeriesAddresses ),
        mrCategoriesAddress( rCategoriesAddress )
{
    // get Diagram
    uno::Reference< chart::XChartDocument > xDoc( rImpHelper.GetChartDocument(), uno::UNO_QUERY );
    if( xDoc.is())
    {
        mxDiagram = xDoc->getDiagram();
    }
    DBG_ASSERT( mxDiagram.is(), "Couldn't get XDiagram" );

    // turn off all axes initially
    uno::Any aFalseBool;
    aFalseBool <<= (sal_Bool)(sal_False);

    uno::Reference< lang::XServiceInfo > xInfo( mxDiagram, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xProp( mxDiagram, uno::UNO_QUERY );
    if( xInfo.is() &&
        xProp.is())
    {
        try
        {
            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxisXSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasXAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasXAxisDescription" ), aFalseBool );
            }
            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartTwoAxisXSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasSecondaryXAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasSecondaryXAxisDescription" ), aFalseBool );
            }

            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxisYSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasYAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasYAxisDescription" ), aFalseBool );
            }
            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartTwoAxisYSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasSecondaryYAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasSecondaryYAxisDescription" ), aFalseBool );
            }

            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxisZSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasZAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasZAxisDescription" ), aFalseBool );
            }
            // data from now on only comes out of columns
            uno::Any aAny;
            chart::ChartDataRowSource eSource = chart::ChartDataRowSource_COLUMNS;
            aAny <<= eSource;
            xProp->setPropertyValue( rtl::OUString::createFromAscii( "DataRowSource" ), aAny );
        }
        catch( beans::UnknownPropertyException )
        {
            DBG_ERROR( "Property required by service not supported" );
        }
    }
}

SchXMLPlotAreaContext::~SchXMLPlotAreaContext()
{}

SvXMLImportContext* SchXMLPlotAreaContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetPlotAreaElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_PA_AXIS:
            pContext = new SchXMLAxisContext( mrImportHelper, GetImport(), rLocalName, mxDiagram, maAxes );
            break;

        case XML_TOK_PA_SERIES:
            {
                sal_Int32 nIndex = mrSeriesAddresses.getLength();
                mrSeriesAddresses.realloc( nIndex + 1 );

                pContext = new SchXMLSeriesContext( mrImportHelper, GetImport(), rLocalName,
                                                    mxDiagram, mrSeriesAddresses[ nIndex ],
                                                    nIndex );
            }
            break;

        case XML_TOK_PA_CATEGORIES:
            pContext = new SchXMLCategoriesDomainContext( mrImportHelper, GetImport(),
                                                          nPrefix, rLocalName,
                                                          mrCategoriesAddress );
            break;

        case XML_TOK_PA_WALL:
        case XML_TOK_PA_FLOOR:
        default:
            pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void SchXMLPlotAreaContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    rtl::OUString aValue;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetPlotAreaAttrTokenMap();

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
                rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_PA_X:
            case XML_TOK_PA_Y:
            case XML_TOK_PA_WIDTH:
            case XML_TOK_PA_HEIGHT:
                break;

            case XML_TOK_PA_STYLE_NAME:
                msAutoStyleName = aValue;
                break;
        }
    }
}

void SchXMLPlotAreaContext::EndElement()
{
    // set properties
    if( msAutoStyleName.getLength())
    {
        uno::Reference< beans::XPropertySet > xProp( mxDiagram, uno::UNO_QUERY );
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

// ========================================

SchXMLAxisContext::SchXMLAxisContext( SchXMLImportHelper& rImpHelper,
                                      SvXMLImport& rImport, const rtl::OUString& rLocalName,
                                      uno::Reference< chart::XDiagram > xDiagram,
                                      std::vector< SchXMLAxis >& aAxes ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mxDiagram( xDiagram ),
        maAxes( aAxes )
{
}

SchXMLAxisContext::~SchXMLAxisContext()
{}

void SchXMLAxisContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    rtl::OUString aValue;
    SchXMLImport& rImport = ( SchXMLImport& )GetImport();
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetAxisAttrTokenMap();

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_AXIS_CLASS:
                {
                    USHORT nEnumVal;
                    if( rImport.GetMM100UnitConverter().convertEnum( nEnumVal, aValue, aXMLAxisClassMap ))
                        maCurrentAxis.eClass = ( SchXMLAxisClass )nEnumVal;
                }
                break;
            case XML_TOK_AXIS_NAME:
                maCurrentAxis.aName = aValue;
                break;
            case XML_TOK_AXIS_STYLE_NAME:
                msAutoStyleName = aValue;
                break;
        }
    }

    // check for number of axes with same category
    maCurrentAxis.nIndexInCategory = 0;
    sal_Int32 nNumOfAxes = maAxes.size();
    for( sal_Int32 nCurrent = 0; nCurrent < nNumOfAxes; nCurrent++ )
    {
        if( maAxes[ nCurrent ].eClass == maCurrentAxis.eClass )
            maCurrentAxis.nIndexInCategory++;
    }
}

void SchXMLAxisContext::EndElement()
{
    // add new Axis to list
    maAxes.push_back( maCurrentAxis );

    // set axis at chart
    uno::Reference< beans::XPropertySet > xDiaProp( mxDiagram, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xProp;
    uno::Any aTrueBool;
    aTrueBool <<= (sal_Bool)(sal_True);
    sal_Bool bHasTitle = ( maCurrentAxis.aTitle.getLength() > 0 );

    switch( maCurrentAxis.eClass )
    {
        case SCH_XML_AXIS_CATEGORY:
        case SCH_XML_AXIS_DOMAIN:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasXAxis" ), aTrueBool );
                    if( bHasTitle )
                        xDiaProp->setPropertyValue(
                            rtl::OUString::createFromAscii( "HasXAxisTitle" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Couldn't turn on x axis" );
                }
                uno::Reference< chart::XAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    xProp = xSuppl->getXAxis();
                    if( bHasTitle )
                    {
                        uno::Reference< text::XTextRange > xRange( xSuppl->getXAxisTitle(), uno::UNO_QUERY );
                        if( xRange.is())
                            xRange->setString( maCurrentAxis.aTitle );
                    }
                }
            }
            else
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasSecondaryXAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Couldn't turn on second x axis" );
                }
                uno::Reference< chart::XTwoAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getSecondaryXAxis();
            }
            break;

        case SCH_XML_AXIS_VALUE:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasYAxis" ), aTrueBool );
                    if( bHasTitle )
                        xDiaProp->setPropertyValue(
                            rtl::OUString::createFromAscii( "HasYAxisTitle" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Couldn't turn on y axis" );
                }
                uno::Reference< chart::XAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    xProp = xSuppl->getYAxis();
                    if( bHasTitle )
                    {
                        uno::Reference< text::XTextRange > xRange( xSuppl->getYAxisTitle(), uno::UNO_QUERY );
                        if( xRange.is())
                            xRange->setString( maCurrentAxis.aTitle );
                    }
                }
            }
            else
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasSecondaryYAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Couldn't turn on second y axis" );
                }
                uno::Reference< chart::XTwoAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getSecondaryYAxis();
            }
            break;

        case SCH_XML_AXIS_SERIES:
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasZAxis" ), aTrueBool );
                    if( bHasTitle )
                        xDiaProp->setPropertyValue(
                            rtl::OUString::createFromAscii( "HasZAxisTitle" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Couldn't turn on z axis" );
                }
                uno::Reference< chart::XAxisZSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    xProp = xSuppl->getZAxis();
                    if( bHasTitle )
                    {
                        uno::Reference< text::XTextRange > xRange( xSuppl->getZAxisTitle(), uno::UNO_QUERY );
                        if( xRange.is())
                            xRange->setString( maCurrentAxis.aTitle );
                    }
                }
            }
            break;
    }

    // set properties
    if( msAutoStyleName.getLength() &&
        xProp.is())
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

SvXMLImportContext* SchXMLAxisContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    SchXMLImport& rImport = ( SchXMLImport& )GetImport();

    if( nPrefix == XML_NAMESPACE_CHART )
    {
        if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_title )))
        {
            pContext = new SchXMLTitleContext( rImport, rLocalName, maCurrentAxis.aTitle );
        }
        else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_grid )))
        {
        }
    }

    if( ! pContext )
        pContext = new SvXMLImportContext( rImport, nPrefix, rLocalName );

    return pContext;
}


// ========================================


SchXMLSeriesContext::SchXMLSeriesContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport, const rtl::OUString& rLocalName,
    uno::Reference< chart::XDiagram >& xDiagram,
    com::sun::star::chart::ChartSeriesAddress& rSeriesAddress,
    sal_Int32 nSeriesIndex ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mxDiagram( xDiagram ),
        mrImportHelper( rImpHelper ),
        mrSeriesAddress( rSeriesAddress ),
        mnSeriesIndex( nSeriesIndex ),
        mnDataPointIndex( -1 )
{
}

SchXMLSeriesContext::~SchXMLSeriesContext()
{
}

void SchXMLSeriesContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    rtl::OUString aValue;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetSeriesAttrTokenMap();

    for( sal_Int32 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_SERIES_CELL_RANGE:
                mrSeriesAddress.DataRangeAddress = aValue;
                break;
            case XML_TOK_SERIES_LABEL_ADDRESS:
                mrSeriesAddress.LabelAddress = aValue;
                break;
            case XML_TOK_SERIES_STYLE_NAME:
                msAutoStyleName = aValue;
                break;
            case XML_TOK_SERIES_CHART_CLASS:
                // not supported yet
                break;
        }
    }
}

void SchXMLSeriesContext::EndElement()
{
    if( msAutoStyleName.getLength() &&
        mxDiagram.is())
    {
        uno::Reference< beans::XPropertySet > xProp;

        try
        {
            xProp = mxDiagram->getDataRowProperties( mnSeriesIndex );
        }
        catch( lang::IndexOutOfBoundsException )
        {
            try
            {
                mrImportHelper.ResizeChartData( mnSeriesIndex + 1 );
                xProp = mxDiagram->getDataRowProperties( mnSeriesIndex );
            }
            catch( lang::IndexOutOfBoundsException )
            {
                DBG_ERROR( "resizing of chart data didn't work!" );
            }
        }

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

SvXMLImportContext* SchXMLSeriesContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetSeriesElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_SERIES_DOMAIN:
            {
                sal_Int32 nIndex = mrSeriesAddress.DomainRangeAddresses.getLength();
                mrSeriesAddress.DomainRangeAddresses.realloc( nIndex + 1 );
                pContext = new SchXMLCategoriesDomainContext(
                    mrImportHelper, GetImport(),
                    nPrefix, rLocalName,
                    mrSeriesAddress.DomainRangeAddresses[ nIndex ] );
            }
            break;
        case XML_TOK_SERIES_DATA_POINT:
            mnDataPointIndex++;
            pContext = new SchXMLDataPointContext( mrImportHelper, GetImport(), rLocalName, mxDiagram,
                                                   mnSeriesIndex, mnDataPointIndex );
        default:
            pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

// ========================================

SchXMLDataPointContext::SchXMLDataPointContext(  SchXMLImportHelper& rImpHelper,
                                                 SvXMLImport& rImport, const rtl::OUString& rLocalName,
                                                 uno::Reference< chart::XDiagram >& xDiagram,
                                                 sal_Int32 nSeries, sal_Int32& rIndex ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mxDiagram( xDiagram ),
        mnSeries( nSeries ),
        mrIndex( rIndex )
{
}

SchXMLDataPointContext::~SchXMLDataPointContext()
{
}

void SchXMLDataPointContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    rtl::OUString aValue;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_CHART )
        {
            if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_style_name )))
                msAutoStyleName = xAttrList->getValueByIndex( i );
            else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_repeated )))
                mrIndex += xAttrList->getValueByIndex( i ).toInt32() - 1;
        }
    }
}

void SchXMLDataPointContext::EndElement()
{
    if( msAutoStyleName.getLength() &&
        mxDiagram.is())
    {
        uno::Reference< beans::XPropertySet > xProp;

        try
        {
            xProp = mxDiagram->getDataPointProperties( mnSeries, mrIndex );
        }
        catch( lang::IndexOutOfBoundsException )
        {
            // resize chart
            mrImportHelper.ResizeChartData( mnSeries + 1, mrIndex + 1 );
            xProp = mxDiagram->getDataPointProperties( mnSeries, mrIndex );
        }

        // set properties
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

// ========================================

SchXMLCategoriesDomainContext::SchXMLCategoriesDomainContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    rtl::OUString& rAddress ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrAddress( rAddress )
{
}

SchXMLCategoriesDomainContext::~SchXMLCategoriesDomainContext()
{
}

void SchXMLCategoriesDomainContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    rtl::OUString aValue;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_TABLE &&
            aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_cell_range_address )))
        {
            mrAddress = xAttrList->getValueByIndex( i );
        }
    }
}

