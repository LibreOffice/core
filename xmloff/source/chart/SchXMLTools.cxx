/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchXMLTools.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-08-17 12:06:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include "SchXMLTools.hxx"

/*
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
*/
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX
#include <comphelper/InlineContainer.hxx>
#endif
// header for class SvXMLUnitConverter
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
// header for struct SvXMLEnumMapEntry
#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmloff/xmlement.hxx>
#endif
// header for define __FAR_DATA
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

// header for class SvXMLImportPropertyMapper
#ifndef _XMLOFF_XMLIMPPR_HXX
#include <xmloff/xmlimppr.hxx>
#endif
// header for class XMLPropStyleContext
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include <xmloff/prstylei.hxx>
#endif
// header for class XMLPropertySetMapper
#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include <xmloff/xmlprmap.hxx>
#endif

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>

#include <comphelper/processfactory.hxx>

using namespace com::sun::star;
using namespace ::xmloff::token;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
Reference< uno::XComponentContext > lcl_getComponentContext()
{
    Reference< uno::XComponentContext > xContext;
    try
    {
        Reference< beans::XPropertySet > xFactProp( comphelper::getProcessServiceFactory(), uno::UNO_QUERY );
        if( xFactProp.is())
            xFactProp->getPropertyValue(OUString::createFromAscii("DefaultContext")) >>= xContext;
    }
    catch( uno::Exception& )
    {}

    return xContext;
}
} // anonymous namespace

// ----------------------------------------

namespace SchXMLTools
{

static __FAR_DATA SvXMLEnumMapEntry aXMLChartClassMap[] =
{
    { XML_LINE,         XML_CHART_CLASS_LINE    },
    { XML_AREA,         XML_CHART_CLASS_AREA    },
    { XML_CIRCLE,       XML_CHART_CLASS_CIRCLE  },
    { XML_RING,         XML_CHART_CLASS_RING    },
    { XML_SCATTER,      XML_CHART_CLASS_SCATTER },
    { XML_RADAR,        XML_CHART_CLASS_RADAR   },
    { XML_BAR,          XML_CHART_CLASS_BAR     },
    { XML_STOCK,        XML_CHART_CLASS_STOCK   },
    { XML_BUBBLE,       XML_CHART_CLASS_BUBBLE  },
    { XML_ADD_IN,       XML_CHART_CLASS_ADDIN   },
    { XML_TOKEN_INVALID, XML_CHART_CLASS_UNKNOWN }
};

SchXMLChartTypeEnum GetChartTypeEnum( const OUString& rClassName )
{
    USHORT nEnumVal = XML_CHART_CLASS_UNKNOWN;
    if( !SvXMLUnitConverter::convertEnum(
                                    nEnumVal, rClassName, aXMLChartClassMap ) )
        nEnumVal = XML_CHART_CLASS_UNKNOWN;
    return SchXMLChartTypeEnum(nEnumVal);
}

typedef ::comphelper::MakeMap< ::rtl::OUString, ::rtl::OUString > tMakeStringStringMap;
//static
const tMakeStringStringMap& lcl_getChartTypeNameMap()
{
    //shape property -- chart model object property
    static tMakeStringStringMap g_aChartTypeNameMap =
        tMakeStringStringMap
        ( ::rtl::OUString::createFromAscii( "com.sun.star.chart.LineDiagram" )
        , ::rtl::OUString::createFromAscii( "com.sun.star.chart2.LineChartType" ) )

        ( ::rtl::OUString::createFromAscii( "com.sun.star.chart.AreaDiagram" )
        , ::rtl::OUString::createFromAscii( "com.sun.star.chart2.AreaChartType" ) )

        ( ::rtl::OUString::createFromAscii( "com.sun.star.chart.BarDiagram" )
        , ::rtl::OUString::createFromAscii( "com.sun.star.chart2.ColumnChartType" ) )

        ( ::rtl::OUString::createFromAscii( "com.sun.star.chart.PieDiagram" )
        , ::rtl::OUString::createFromAscii( "com.sun.star.chart2.PieChartType" ) )

        ( ::rtl::OUString::createFromAscii( "com.sun.star.chart.DonutDiagram" )
        , ::rtl::OUString::createFromAscii( "com.sun.star.chart2.DonutChartType" ) )

        ( ::rtl::OUString::createFromAscii( "com.sun.star.chart.XYDiagram" )
        , ::rtl::OUString::createFromAscii( "com.sun.star.chart2.ScatterChartType" ) )

        ( ::rtl::OUString::createFromAscii( "com.sun.star.chart.NetDiagram" )
        , ::rtl::OUString::createFromAscii( "com.sun.star.chart2.NetChartType" ) )

        ( ::rtl::OUString::createFromAscii( "com.sun.star.chart.StockDiagram" )
        , ::rtl::OUString::createFromAscii( "com.sun.star.chart2.CandleStickChartType" ) )

        ;
    return g_aChartTypeNameMap;
}


OUString GetNewChartTypeName( const OUString & rOldChartTypeName )
{
    OUString aNew(rOldChartTypeName);

    const tMakeStringStringMap& rMap = lcl_getChartTypeNameMap();
    tMakeStringStringMap::const_iterator aIt( rMap.find( rOldChartTypeName ));
    if( aIt != rMap.end())
    {
        aNew = aIt->second;
    }
    return aNew;
}

OUString GetChartTypeByClassName(
    const OUString & rClassName, bool bUseOldNames )
{
    OUStringBuffer aResultBuffer;
    bool bInternalType = false;

    if( bUseOldNames )
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart."));
    else
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2."));

    bInternalType = true;

    if( IsXMLToken( rClassName, XML_LINE ))
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Line"));
    else if( IsXMLToken( rClassName, XML_AREA ))
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Area"));
    else if( IsXMLToken( rClassName, XML_BAR ))
    {
        if( bUseOldNames )
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Bar"));
        else
        {
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Column"));
            // @todo: might be Bar
        }
    }
    else if( IsXMLToken( rClassName, XML_CIRCLE ))
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Pie"));
    else if( IsXMLToken( rClassName, XML_RING ))
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Donut"));
    else if( IsXMLToken( rClassName, XML_SCATTER ))
    {
        if( bUseOldNames )
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("XY"));
        else
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Scatter"));
    }
    else if( IsXMLToken( rClassName, XML_RADAR ))
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Net"));
    else if( IsXMLToken( rClassName, XML_STOCK ))
    {
        if( bUseOldNames )
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Stock"));
        else
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("CandleStick"));
    }
    else
        bInternalType = false;

    if( ! bInternalType )
        return OUString();

    if( bUseOldNames )
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Diagram"));
    else
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("ChartType"));

    return aResultBuffer.makeStringAndClear();

}

XMLTokenEnum getTokenByChartType(
    const OUString & rChartTypeService, bool bUseOldNames )
{
    XMLTokenEnum eResult = XML_TOKEN_INVALID;
    OUString aPrefix, aPostfix;

    if( bUseOldNames )
    {
        aPrefix = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart."));
        aPostfix = OUString( RTL_CONSTASCII_USTRINGPARAM("Diagram"));
    }
    else
    {
        aPrefix = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart2."));
        aPostfix = OUString( RTL_CONSTASCII_USTRINGPARAM("ChartType"));
    }

    if( rChartTypeService.match( aPrefix ))
    {
        sal_Int32 nSkip = aPrefix.getLength();
        OSL_ASSERT( rChartTypeService.getLength() >= nSkip );
        sal_Int32 nTypeLength = rChartTypeService.getLength() - nSkip - aPostfix.getLength();
        // if postfix matches and leaves a non-empty type
        if( nTypeLength > 0 && rChartTypeService.match( aPostfix, nSkip + nTypeLength ))
        {
            OUString aServiceName( rChartTypeService.copy( nSkip, nTypeLength ));

            if( aServiceName.equalsAscii("Line"))
                eResult = XML_LINE;
            else if( aServiceName.equalsAscii("Area"))
                eResult = XML_AREA;
            else if( aServiceName.equalsAscii("Bar") ||
                     (!bUseOldNames && aServiceName.equalsAscii("Column")))
                eResult = XML_BAR;
            else if( aServiceName.equalsAscii("Pie"))
                eResult = XML_CIRCLE;
            else if( aServiceName.equalsAscii("Donut"))
                eResult = XML_RING;
            else if( (bUseOldNames && aServiceName.equalsAscii("XY")) ||
                     (!bUseOldNames && aServiceName.equalsAscii("Scatter")))
                eResult = XML_SCATTER;
            else if( aServiceName.equalsAscii("Net"))
                eResult = XML_RADAR;
            else if( (bUseOldNames && aServiceName.equalsAscii("Stock")) ||
                     (!bUseOldNames && aServiceName.equalsAscii("CandleStick")))
                eResult = XML_STOCK;
        }
    }

    if( eResult == XML_TOKEN_INVALID && rChartTypeService.getLength() > 0 )
        eResult = XML_ADD_IN;

    return eResult;
}

Reference< chart2::data::XLabeledDataSequence > GetNewLabeledDataSequence()
{
    Reference< chart2::data::XLabeledDataSequence >  xResult;
    Reference< uno::XComponentContext > xContext( lcl_getComponentContext());
    if( xContext.is() )
        xResult.set(
            xContext->getServiceManager()->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.chart2.data.LabeledDataSequence"),
                xContext ), uno::UNO_QUERY_THROW );
    return xResult;
}

void CreateCategories(
    const uno::Reference< chart2::data::XDataProvider > & xDataProvider,
    const uno::Reference< chart2::XChartDocument > & xNewDoc,
    const OUString & rRangeAddress,
    sal_Int32 nCooSysIndex,
    sal_Int32 nDimensionIndex,
    tSchXMLLSequencesPerIndex * pLSequencesPerIndex )
{
    try
    {
        if( xNewDoc.is() && rRangeAddress.getLength())
        {
            if( xDataProvider.is())
            {
                uno::Reference< chart2::XDiagram > xDia( xNewDoc->getFirstDiagram());
                if( !xDia.is())
                    return;

                uno::Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDia, uno::UNO_QUERY_THROW );
                uno::Sequence< uno::Reference< chart2::XCoordinateSystem > >
                    aCooSysSeq( xCooSysCnt->getCoordinateSystems());
                if( nCooSysIndex < aCooSysSeq.getLength())
                {
                    uno::Reference< chart2::XCoordinateSystem > xCooSys( aCooSysSeq[nCooSysIndex] );
                    OSL_ASSERT( xCooSys.is());
                    if( nDimensionIndex < xCooSys->getDimension() )
                    {
                        const sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
                        for(sal_Int32 nI=0; nI<=nMaxAxisIndex; ++nI)
                        {
                            uno::Reference< chart2::XAxis > xAxis( xCooSys->getAxisByDimension( nDimensionIndex, nI ));
                            if( xAxis.is() )
                            {
                                chart2::ScaleData aData( xAxis->getScaleData());
                                uno::Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
                                    GetNewLabeledDataSequence());
                                try
                                {
                                    xLabeledSeq->setValues(
                                        xDataProvider->createDataSequenceByRangeRepresentation( rRangeAddress ));
                                }
                                catch( const lang::IllegalArgumentException & ex )
                                {
                                    (void)ex; // avoid warning for pro build
                                    OSL_ENSURE( false, ::rtl::OUStringToOString(
                                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IllegalArgumentException caught, Message: " )) +
                                                    ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
                                }
                                aData.Categories.set( xLabeledSeq );
                                if( pLSequencesPerIndex )
                                {
                                    // register for setting local data if external data provider is not present
                                    pLSequencesPerIndex->insert(
                                        tSchXMLLSequencesPerIndex::value_type(
                                            tSchXMLIndexWithPart( SCH_XML_CATEGORIES_INDEX, SCH_XML_PART_VALUES ), xLabeledSeq ));
                                }
                                xAxis->setScaleData( aData );
                            }
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception & )
    {
        OSL_ENSURE( false, "Exception caught while creating Categories" );
    }
}


uno::Any getPropertyFromContext( const rtl::OUString& rPropertyName, const XMLPropStyleContext* pPropStyleContext, const SvXMLStylesContext* pStylesCtxt )
{
    uno::Any aRet;
    if( !pPropStyleContext || !pStylesCtxt )
        return aRet;
    const ::std::vector< XMLPropertyState >& rProperties = pPropStyleContext->GetProperties();
    const UniReference< XMLPropertySetMapper >& rMapper = pStylesCtxt->GetImportPropertyMapper( pPropStyleContext->GetFamily()/*XML_STYLE_FAMILY_SCH_CHART_ID*/ )->getPropertySetMapper();
    ::std::vector< XMLPropertyState >::const_iterator aEnd( rProperties.end() );
    ::std::vector< XMLPropertyState >::const_iterator aPropIter( rProperties.begin() );
    for( aPropIter = rProperties.begin(); aPropIter != aEnd; ++aPropIter )
    {
        sal_Int32 nIdx = aPropIter->mnIndex;
        if( nIdx == -1 )
            continue;
        OUString aPropName = rMapper->GetEntryAPIName( nIdx );
        if(rPropertyName.equals(aPropName))
            return aPropIter->maValue;
    }
    return aRet;
}

}
