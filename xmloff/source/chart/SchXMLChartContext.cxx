/*************************************************************************
 *
 *  $RCSfile: SchXMLChartContext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: bm $ $Date: 2000-11-27 09:09:12 $
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
#include "SchXMLImport.hxx"
#include "SchXMLPlotAreaContext.hxx"
#include "SchXMLParagraphContext.hxx"
#include "SchXMLTableContext.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include "prstylei.hxx"
#endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XDIAGRAM_HPP_
#include <com/sun/star/chart/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTLEGENDPOSITION_HPP_
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGMAPPING_HPP_
#include <com/sun/star/util/XStringMapping.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif

using namespace com::sun::star;

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
    XML_CHART_CLASS_BUBBLE, // not yet implemented
    XML_CHART_CLASS_ADDIN
};

// ----------------------------------------

static __FAR_DATA SvXMLEnumMapEntry aXMLChartClassMap[] =
{
    { sXML_line,        XML_CHART_CLASS_LINE    },
    { sXML_area,        XML_CHART_CLASS_AREA    },
    { sXML_circle,      XML_CHART_CLASS_CIRCLE  },
    { sXML_ring,        XML_CHART_CLASS_RING    },
    { sXML_scatter,     XML_CHART_CLASS_SCATTER },
    { sXML_radar,       XML_CHART_CLASS_RADAR   },
    { sXML_bar,         XML_CHART_CLASS_BAR     },
    { sXML_stock,       XML_CHART_CLASS_STOCK   },
    { sXML_bubble,      XML_CHART_CLASS_BUBBLE  },
    { 0, 0 }
};

static __FAR_DATA SvXMLEnumMapEntry aXMLLegendAlignmentMap[] =
{
    { sXML_left,        chart::ChartLegendPosition_LEFT     },
    { sXML_top,         chart::ChartLegendPosition_TOP      },
    { sXML_right,       chart::ChartLegendPosition_RIGHT    },
    { sXML_bottom,      chart::ChartLegendPosition_BOTTOM   },
    { 0, 0 }
};

// ----------------------------------------

SchXMLChartContext::SchXMLChartContext( SchXMLImportHelper& rImpHelper,
                                        SvXMLImport& rImport, const rtl::OUString& rLocalName ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mbHasOwnTable( sal_False )
{
    // hide title, subtitle and legend
    uno::Reference< beans::XPropertySet > xProp( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );
    if( xProp.is())
    {
        uno::Any aFalseBool;
        aFalseBool <<= (sal_Bool)(sal_False);
        try
        {
            xProp->setPropertyValue( rtl::OUString::createFromAscii( "HasMainTitle" ), aFalseBool );
            xProp->setPropertyValue( rtl::OUString::createFromAscii( "HasSubTitle" ), aFalseBool );
            xProp->setPropertyValue( rtl::OUString::createFromAscii( "HasLegend" ), aFalseBool );
        }
        catch( beans::UnknownPropertyException )
        {
            DBG_ERROR( "XML-Chart Import: Property not found" );
        }
    }
}

SchXMLChartContext::~SchXMLChartContext()
{}

void SchXMLChartContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    rtl::OUString aValue;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetChartAttrTokenMap();
    awt::Size aChartSize;

    rtl::OUString aServiceName;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
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
                                aServiceName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.LineDiagram" ));
                                break;
                            case XML_CHART_CLASS_AREA:
                                aServiceName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.AreaDiagram" ));
                                break;
                            case XML_CHART_CLASS_CIRCLE:
                                aServiceName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.PieDiagram" ));
                                break;
                            case XML_CHART_CLASS_RING:
                                aServiceName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.DonutDiagram" ));
                                break;
                            case XML_CHART_CLASS_SCATTER:
                                aServiceName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.XYDiagram" ));
                                break;
                            case XML_CHART_CLASS_RADAR:
                                aServiceName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.NetDiagram" ));
                                break;
                            case XML_CHART_CLASS_BAR:
                                aServiceName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.BarDiagram" ));
                                break;
                            case XML_CHART_CLASS_STOCK:
                                aServiceName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.chart.StockDiagram" ));
                                break;
                            case XML_CHART_CLASS_BUBBLE:
                                DBG_ERROR( "Bubble chart not supported yet" );
                                break;
                            case XML_CHART_CLASS_ADDIN:
                                // service is taken from add-in-name attribute
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
                msAutoStyleName = aValue;
                break;

            case XML_TOK_CHART_ADDIN_NAME:
                aServiceName = aValue;
        }
    }

    if( aServiceName.getLength())
    {
        uno::Reference< chart::XChartDocument > xDoc = mrImportHelper.GetChartDocument();
        if( xDoc.is())
        {
            uno::Reference< lang::XMultiServiceFactory > xFact( xDoc, uno::UNO_QUERY );
            if( xFact.is())
            {
                uno::Reference< chart::XDiagram > xDia( xFact->createInstance( aServiceName ), uno::UNO_QUERY );
                if( xDia.is())
                    xDoc->setDiagram( xDia );
            }
        }
    }

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
                xPageProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" )), aAny );

                aAny <<= (sal_Int32)( aChartSize.Height );
                xPageProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" )), aAny );
            }
            catch( beans::UnknownPropertyException )
            {
                DBG_ERROR( "Cannot set page size" );
            }
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
    uno::Any aTrueBool;
    aTrueBool <<= (sal_Bool)(sal_True);

    if( xProp.is())
    {
        if( maMainTitle.getLength())
        {
            try
            {
                xProp->setPropertyValue( rtl::OUString::createFromAscii( "HasMainTitle" ), aTrueBool );
            }
            catch( beans::UnknownPropertyException )
            {
                DBG_ERROR1( "Couldn't set property %s on document", "HasMainTitle" );
            }

            uno::Reference< beans::XPropertySet > xTitleProp( xDoc->getTitle(), uno::UNO_QUERY );
            if( xTitleProp.is())
            {
                try
                {
                    uno::Any aAny;
                    aAny <<= maMainTitle;
                    xTitleProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Property String for Title not available" );
                }
            }
        }
        if( maSubTitle.getLength())
        {
            try
            {
                xProp->setPropertyValue( rtl::OUString::createFromAscii( "HasSubTitle" ), aTrueBool );
            }
            catch( beans::UnknownPropertyException )
            {
                DBG_ERROR1( "Couldn't set property %s on document", "HasSubTitle" );
            }

            uno::Reference< beans::XPropertySet > xTitleProp( xDoc->getSubTitle(), uno::UNO_QUERY );
            if( xTitleProp.is())
            {
                try
                {
                    uno::Any aAny;
                    aAny <<= maSubTitle;
                    xTitleProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                }
                catch( beans::UnknownPropertyException )
                {
                    DBG_ERROR( "Property String for Title not available" );
                }
            }
        }
    }

    if( mbHasOwnTable )
    {
        // apply data read in table sub-element to chart
        SchXMLTableHelper::applyTable( maTable, maSeriesAddresses, msCategoriesAddress, xDoc );
    }
    else
    {
        // translate cell-address strings
        uno::Reference< util::XStringMapping > xTableAddressMapper = mrImportHelper.GetTableAddressMapper();
        if( xTableAddressMapper.is())
        {
            // series
            sal_Int32 nLength = maSeriesAddresses.getLength();
            sal_Int32 nIdx;
            uno::Sequence< rtl::OUString > aStrSeq( nLength * 2 + 1 );

            for( nIdx = 0; nIdx < nLength; nIdx++ )
            {
                aStrSeq[ nIdx * 2 ] = maSeriesAddresses[ nIdx ].DataRangeAddress;
                aStrSeq[ nIdx * 2 + 1 ] = maSeriesAddresses[ nIdx ].LabelAddress;

                // domains
                if( maSeriesAddresses[ nIdx ].DomainRangeAddresses.getLength())
                    xTableAddressMapper->mapStrings( maSeriesAddresses[ nIdx ].DomainRangeAddresses );
            }
            // categories
            aStrSeq[ nLength * 2 ] = msCategoriesAddress;

            // translate
            xTableAddressMapper->mapStrings( aStrSeq );

            // write back
            for( nIdx = 0; nIdx < nLength; nIdx++ )
            {
                maSeriesAddresses[ nIdx ].DataRangeAddress = aStrSeq[ nIdx * 2 ];
                maSeriesAddresses[ nIdx ].LabelAddress = aStrSeq[ nIdx * 2 + 1 ];
            }
            msCategoriesAddress = aStrSeq[ nLength * 2 ];
        }

        // set table references at document
        if( xProp.is())
        {
            try
            {
                uno::Any aAny;
                aAny <<= msCategoriesAddress;
                xProp->setPropertyValue( rtl::OUString::createFromAscii( "CategoriesRangeAddress" ), aAny );

                aAny <<= maSeriesAddresses;
                xProp->setPropertyValue( rtl::OUString::createFromAscii( "SeriesAddresses" ), aAny );
            }
            catch( beans::UnknownPropertyException )
            {
                DBG_WARNING( "Required property not found in ChartDocument" );
            }
        }
    }

    // allow BuildChart again
    uno::Reference< frame::XModel > xModel( xDoc, uno::UNO_QUERY );
    if( xModel.is())
        xModel->unlockControllers();
}

SvXMLImportContext* SchXMLChartContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetChartElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_CHART_PLOT_AREA:
            pContext = new SchXMLPlotAreaContext( mrImportHelper, GetImport(), rLocalName,
                                                  maSeriesAddresses, msCategoriesAddress );
            break;

        case XML_TOK_CHART_TITLE:
            pContext = new SchXMLTitleContext( GetImport(), rLocalName, maMainTitle );
            break;

        case XML_TOK_CHART_SUBTITLE:
            pContext = new SchXMLTitleContext( GetImport(), rLocalName, maSubTitle );
            break;

        case XML_TOK_CHART_LEGEND:
            pContext = new SchXMLLegendContext( mrImportHelper, GetImport(), rLocalName );
            break;

        case XML_TOK_CHART_TABLE:
            pContext = new SchXMLTableContext( mrImportHelper, GetImport(), rLocalName, maTable );
            if( pContext )
                mbHasOwnTable = sal_True;
            break;
    }

    if( ! pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

// ----------------------------------------

SchXMLTitleContext::SchXMLTitleContext( SvXMLImport& rImport, const rtl::OUString& rLocalName,
                                        rtl::OUString& aTitle ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        maTitle( aTitle )
{
}

SchXMLTitleContext::~SchXMLTitleContext()
{}


SvXMLImportContext* SchXMLTitleContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    if( nPrefix == XML_NAMESPACE_TEXT &&
        rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_p )))
    {
        pContext = new SchXMLParagraphContext( GetImport(), rLocalName, maTitle );
    }
    else
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

// ----------------------------------------

SchXMLLegendContext::SchXMLLegendContext( SchXMLImportHelper& rImpHelper,
                                          SvXMLImport& rImport, const rtl::OUString& rLocalName ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper )
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
            xDocProp->setPropertyValue( rtl::OUString::createFromAscii( "HasLegend" ), aTrueBool );
        }
        catch( beans::UnknownPropertyException )
        {
            DBG_ERROR( "Property HasLegend not found" );
        }
    }

    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    rtl::OUString aValue;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetLegendAttrTokenMap();

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_LEGEND_POSITION:
                {
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
                                xProp->setPropertyValue( rtl::OUString::createFromAscii( "Alignment" ), aAny );
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
            case XML_TOK_LEGEND_Y:
                break;
        }
    }
}

SchXMLLegendContext::~SchXMLLegendContext()
{
}

