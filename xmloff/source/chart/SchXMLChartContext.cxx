/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchXMLChartContext.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 15:57:02 $
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

#include "SchXMLChartContext.hxx"
#include "SchXMLImport.hxx"
#include "SchXMLPlotAreaContext.hxx"
#include "SchXMLParagraphContext.hxx"
#include "SchXMLTableContext.hxx"
#include "SchXMLSeriesHelper.hxx"
#include "SchXMLSeries2Context.hxx"
#include "SchXMLTools.hxx"
#include "SchXMLErrorBuildIds.hxx"

#ifndef _COMPHELPER_MEDIADESCRIPTOR_HXX_
#include <comphelper/mediadescriptor.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
// header for class ByteString
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmloff/xmlement.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include <xmloff/xmlstyle.hxx>
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include <xmloff/prstylei.hxx>
#endif

#include "vector"

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
#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XCHARTDATAARRAY_HPP_
#include <com/sun/star/chart/XChartDataArray.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XVISUALOBJECT_HPP_
#include <com/sun/star/embed/XVisualObject.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XTitled.hpp>

using namespace com::sun::star;
using namespace ::xmloff::token;
using ::rtl::OUString;
using com::sun::star::uno::Reference;
using namespace ::SchXMLTools;

namespace
{
uno::Reference< chart2::XChartTypeTemplate > lcl_getTemplate( const uno::Reference< chart2::XChartDocument > & xDoc )
{
    uno::Reference< chart2::XChartTypeTemplate > xResult;
    try
    {
        if( !xDoc.is())
            return xResult;
        uno::Reference< lang::XMultiServiceFactory > xChartTypeManager( xDoc->getChartTypeManager(), uno::UNO_QUERY );
        if( !xChartTypeManager.is())
            return xResult;
        uno::Reference< chart2::XDiagram > xDiagram( xDoc->getFirstDiagram());
        if( !xDiagram.is())
            return xResult;

        uno::Sequence< ::rtl::OUString > aServiceNames( xChartTypeManager->getAvailableServiceNames());
        const sal_Int32 nLength = aServiceNames.getLength();

        for( sal_Int32 i = 0; i < nLength; ++i )
        {
            try
            {
                uno::Reference< chart2::XChartTypeTemplate > xTempl(
                    xChartTypeManager->createInstance( aServiceNames[ i ] ), uno::UNO_QUERY_THROW );

                if( xTempl->matchesTemplate( xDiagram, sal_True ))
                {
                    xResult.set( xTempl );
                    break;
                }
            }
            catch( uno::Exception & )
            {
                DBG_ERROR( "Exception during determination of chart type template" );
            }
        }
    }
    catch( uno::Exception & )
    {
        DBG_ERROR( "Exception during import lcl_getTemplate" );
    }
    return xResult;
}

void lcl_setRoleAtLabeledSequence(
    const uno::Reference< chart2::data::XLabeledDataSequence > & xLSeq,
    const ::rtl::OUString &rRole )
{
    // set role of sequence
    uno::Reference< chart2::data::XDataSequence > xValues( xLSeq->getValues());
    if( xValues.is())
    {
        uno::Reference< beans::XPropertySet > xProp( xValues, uno::UNO_QUERY );
        if( xProp.is())
            xProp->setPropertyValue(OUString::createFromAscii("Role"), uno::makeAny( rRole ));
    }
}

void lcl_MoveDataToCandleStickSeries(
    const uno::Reference< chart2::data::XDataSource > & xDataSource,
    const uno::Reference< chart2::XDataSeries > & xDestination,
    const OUString & rRole )
{
    try
    {
        uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aLabeledSeq(
            xDataSource->getDataSequences());
        if( aLabeledSeq.getLength())
        {
            lcl_setRoleAtLabeledSequence( aLabeledSeq[0], rRole );

            // add to data series
            uno::Reference< chart2::data::XDataSource > xSource( xDestination, uno::UNO_QUERY_THROW );
            // @todo: realloc only once outside this function
            uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aData( xSource->getDataSequences());
            aData.realloc( aData.getLength() + 1);
            aData[ aData.getLength() - 1 ] = aLabeledSeq[0];
            uno::Reference< chart2::data::XDataSink > xSink( xDestination, uno::UNO_QUERY_THROW );
            xSink->setData( aData );
        }
    }
    catch( uno::Exception & )
    {
        OSL_ENSURE( false, "Exception caught while moving data to candlestick series" );
    }
}

void lcl_setRoleAtFirstSequence(
    const uno::Reference< chart2::XDataSeries > & xSeries,
    const ::rtl::OUString & rRole )
{
    uno::Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
    if( xSource.is())
    {
        uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSeq( xSource->getDataSequences());
        if( aSeq.getLength())
            lcl_setRoleAtLabeledSequence( aSeq[0], rRole );
    }
}

void lcl_removeEmptyChartTypeGroups( const uno::Reference< chart2::XChartDocument > & xDoc )
{
    if( ! xDoc.is())
        return;

    uno::Reference< chart2::XDiagram > xDia( xDoc->getFirstDiagram());
    if( ! xDia.is())
        return;

    try
    {
        // count all charttype groups to be able to leave at least one
        sal_Int32 nRemainingGroups = 0;
        uno::Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDia, uno::UNO_QUERY_THROW );
        uno::Sequence< uno::Reference< chart2::XCoordinateSystem > >
            aCooSysSeq( xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 nI = aCooSysSeq.getLength(); nI--; )
        {
            uno::Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[nI], uno::UNO_QUERY_THROW );
            nRemainingGroups += xCTCnt->getChartTypes().getLength();
        }

        // delete all empty groups, but leave at least  group (empty or not)
        for( sal_Int32 nI = aCooSysSeq.getLength(); nI-- && (nRemainingGroups > 1); )
        {
            uno::Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[nI], uno::UNO_QUERY_THROW );
            uno::Sequence< uno::Reference< chart2::XChartType > > aCTSeq( xCTCnt->getChartTypes());
            for( sal_Int32 nJ=aCTSeq.getLength(); nJ-- && (nRemainingGroups > 1); )
            {
                uno::Reference< chart2::XDataSeriesContainer > xDSCnt( aCTSeq[nJ], uno::UNO_QUERY_THROW );
                if( xDSCnt->getDataSeries().getLength() == 0 )
                {
                    // note: iterator stays valid as we have a local sequence
                    xCTCnt->removeChartType( aCTSeq[nJ] );
                    --nRemainingGroups;
                }
            }
        }
    }
    catch( uno::Exception & ex )
    {
        String aStr( ex.Message );
        ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR1( "Exception caught while removing empty chart types: %s", aBStr.GetBuffer());
    }
}

} // anonymous namespace

static __FAR_DATA SvXMLEnumMapEntry aXMLLegendAlignmentMap[] =
{
//  { XML_LEFT,         chart::ChartLegendPosition_LEFT     },
    // #i35421#
    { XML_START,        chart::ChartLegendPosition_LEFT     },
    { XML_TOP,          chart::ChartLegendPosition_TOP      },
//  { XML_RIGHT,        chart::ChartLegendPosition_RIGHT    },
    // #i35421#
    { XML_END,          chart::ChartLegendPosition_RIGHT    },
    { XML_BOTTOM,       chart::ChartLegendPosition_BOTTOM   },
    { XML_TOKEN_INVALID, 0 }
};

// ----------------------------------------

SchXMLChartContext::SchXMLChartContext( SchXMLImportHelper& rImpHelper,
                                        SvXMLImport& rImport, const rtl::OUString& rLocalName ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mbHasOwnTable( sal_True ),
        mbAllRangeAddressesAvailable( sal_True ),
        mbColHasLabels( sal_False ),
        mbRowHasLabels( sal_False ),
        meDataRowSource( chart::ChartDataRowSource_COLUMNS ),
        mbIsStockChart( false )
{
}

SchXMLChartContext::~SchXMLChartContext()
{}

void SchXMLChartContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetChartAttrTokenMap();
    awt::Size aChartSize;
    // this flag is necessarry for pie charts in the core
    sal_Bool bSetSwitchData = sal_False;
    sal_Bool bDomainForDefaultDataNeeded = sal_False;

    ::rtl::OUString sAutoStyleName;
    ::rtl::OUString aOldChartTypeName;
    bool bHasAddin = false;

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
                    rtl::OUString sClassName;
                    sal_uInt16 nClassPrefix =
                        GetImport().GetNamespaceMap().GetKeyByAttrName(
                                aValue, &sClassName );
                    if( XML_NAMESPACE_CHART == nClassPrefix )
                    {
                        SchXMLChartTypeEnum eChartTypeEnum = SchXMLTools::GetChartTypeEnum( sClassName );
                        if( eChartTypeEnum != XML_CHART_CLASS_UNKNOWN )
                        {
                            aOldChartTypeName = SchXMLTools::GetChartTypeByClassName( sClassName, true /* bUseOldNames */ );
                            maChartTypeServiceName = SchXMLTools::GetChartTypeByClassName( sClassName, false /* bUseOldNames */ );
                            switch( eChartTypeEnum )
                            {
                            case XML_CHART_CLASS_CIRCLE:
                                bSetSwitchData = sal_True;
                                break;
                            case XML_CHART_CLASS_SCATTER:
                                bDomainForDefaultDataNeeded = sal_True;
                                break;
                            case XML_CHART_CLASS_STOCK:
                                mbIsStockChart = true;
                                break;
                            case XML_CHART_CLASS_BUBBLE:
                                DBG_ERROR( "Bubble chart not supported yet" );
                                break;
                            default:
                                break;
                            }
                        }
                    }
                    else if( XML_NAMESPACE_OOO == nClassPrefix )
                    {
                        // service is taken from add-in-name attribute
                        bHasAddin = true;

                        // for service charts assume domain in base type
                        // if base type doesn't use a domain this is ok,
                        // the data just grows bigger
                        bDomainForDefaultDataNeeded = sal_True;
                        aOldChartTypeName = sClassName;
                        maChartTypeServiceName = sClassName;
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

            case XML_TOK_CHART_COL_MAPPING:
                msColTrans = aValue;
                break;
            case XML_TOK_CHART_ROW_MAPPING:
                msRowTrans = aValue;
                break;
        }
    }

    if( aOldChartTypeName.getLength()<= 0 )
    {
        DBG_ERROR( "need a charttype to create a diagram" );
        //set a fallback value:
        ::rtl::OUString aChartClass_Bar( GetXMLToken(XML_BAR ) );
        aOldChartTypeName = SchXMLTools::GetChartTypeByClassName( aChartClass_Bar, true /* bUseOldNames */ );
        maChartTypeServiceName = SchXMLTools::GetChartTypeByClassName( aChartClass_Bar, false /* bUseOldNames */ );
    }

    InitChart (aChartSize, bDomainForDefaultDataNeeded, aOldChartTypeName, bSetSwitchData);

    if( bHasAddin )
    {
        //correct charttype serveice name when having an addin
        //and don't refresh addin during load
        uno::Reference< beans::XPropertySet > xDocProp( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );
        if( xDocProp.is() )
        {
            try
            {
                xDocProp->getPropertyValue( ::rtl::OUString::createFromAscii("BaseDiagram")) >>= aOldChartTypeName;
                maChartTypeServiceName =  SchXMLTools::GetNewChartTypeName( aOldChartTypeName );
                xDocProp->setPropertyValue( rtl::OUString::createFromAscii( "RefreshAddInAllowed" ) , uno::makeAny( sal_False) );
            }
            catch( uno::Exception & )
            {
                DBG_ERROR( "Exception during import SchXMLChartContext::StartElement" );
            }
        }
    }

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
}

namespace
{

struct NewDonutSeries
{
    ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > m_xSeries;
    ::rtl::OUString msStyleName;
    sal_Int32 mnAttachedAxis;

    ::std::vector< ::rtl::OUString > m_aSeriesStyles;
    ::std::vector< ::rtl::OUString > m_aPointStyles;

    NewDonutSeries( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >& xSeries, sal_Int32 nPointCount )
                    : m_xSeries( xSeries )
                    , mnAttachedAxis( 1 )
    {
        m_aPointStyles.resize(nPointCount);
        m_aSeriesStyles.resize(nPointCount);
    }

    void setSeriesStyleNameToPoint( const ::rtl::OUString& rStyleName, sal_Int32 nPointIndex )
    {
        DBG_ASSERT(nPointIndex < static_cast<sal_Int32>(m_aSeriesStyles.size()),"donut point <-> series count mismatch");
        if( nPointIndex < static_cast<sal_Int32>(m_aSeriesStyles.size()) )
            m_aSeriesStyles[nPointIndex]=rStyleName;
    }

    void setPointStyleNameToPoint( const ::rtl::OUString& rStyleName, sal_Int32 nPointIndex )
    {
        DBG_ASSERT(nPointIndex < static_cast<sal_Int32>(m_aPointStyles.size()),"donut point <-> series count mismatch");
        if( nPointIndex < static_cast<sal_Int32>(m_aPointStyles.size()) )
            m_aPointStyles[nPointIndex]=rStyleName;
    }

    ::std::list< DataRowPointStyle > creatStyleList()
    {
        ::std::list< DataRowPointStyle > aRet;

        DataRowPointStyle aSeriesStyle( DataRowPointStyle::DATA_SERIES
            , m_xSeries, -1, 1, msStyleName, mnAttachedAxis );
        aRet.push_back( aSeriesStyle );

        sal_Int32 nPointIndex=0;
        ::std::vector< ::rtl::OUString >::iterator aPointIt( m_aPointStyles.begin() );
        ::std::vector< ::rtl::OUString >::iterator aPointEnd( m_aPointStyles.end() );
        while( aPointIt != aPointEnd )
        {
            DataRowPointStyle aPointStyle( DataRowPointStyle::DATA_POINT
                , m_xSeries, nPointIndex, 1, *aPointIt, mnAttachedAxis );
            if( nPointIndex < static_cast<sal_Int32>(m_aSeriesStyles.size()) )
            {
                aPointStyle.msSeriesStyleNameForDonuts = m_aSeriesStyles[nPointIndex];
            }
            if( aPointStyle.msSeriesStyleNameForDonuts.getLength()
                || aPointStyle.msStyleName.getLength() )
                aRet.push_back( aPointStyle );
            ++aPointIt;
            ++nPointIndex;
        }

        return aRet;
    }
};

void lcl_swapPointAndSeriesStylesForDonutCharts( ::std::list< DataRowPointStyle >& rStyleList
        , const ::std::map< ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries> , sal_Int32 >& rSeriesMap )
{
    ::std::list< DataRowPointStyle >::iterator aIt(rStyleList.begin());
    ::std::list< DataRowPointStyle >::iterator aEnd(rStyleList.end());

    //detect old series count
    //and add old series to aSeriesMap
    ::std::map< ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >, sal_Int32 > aSeriesMap(rSeriesMap);
    sal_Int32 nOldSeriesCount = 0;
    {
        sal_Int32 nMaxOldSeriesIndex = 0;
        sal_Int32 nOldSeriesIndex = 0;
        for( aIt = rStyleList.begin(); aIt != aEnd; ++aIt )
        {
            DataRowPointStyle aStyle(*aIt);
            if(aStyle.meType == DataRowPointStyle::DATA_SERIES &&
                    aStyle.m_xSeries.is() )
            {
                nMaxOldSeriesIndex = nOldSeriesIndex;

                if( aSeriesMap.end() == aSeriesMap.find(aStyle.m_xSeries) )
                    aSeriesMap[aStyle.m_xSeries] = nOldSeriesIndex;

                nOldSeriesIndex++;
            }
        }
        nOldSeriesCount = nMaxOldSeriesIndex+1;
    }
    /*
    sal_Int32 nOldSeriesCount = 0;
    {
        sal_Int32 nMaxOldSeriesIndex = 0;
        sal_Int32 nOldSeriesIndex = 0;
        for( aIt = rStyleList.begin(); aIt != aEnd; ++aIt )
        {
            DataRowPointStyle aStyle(*aIt);
            if(aStyle.meType == DataRowPointStyle::DATA_SERIES )
            {
                nMaxOldSeriesIndex = nOldSeriesIndex;
                nOldSeriesIndex++;
            }
        }
        nOldSeriesCount = nMaxOldSeriesIndex+1;
    }
    */


    //initialize new series styles
    ::std::map< Reference< chart2::XDataSeries >, sal_Int32 >::const_iterator aSeriesMapIt( aSeriesMap.begin() );
    ::std::map< Reference< chart2::XDataSeries >, sal_Int32 >::const_iterator aSeriesMapEnd( aSeriesMap.end() );

    //sort by index
    ::std::vector< NewDonutSeries > aNewSeriesVector;
    {
        ::std::map< sal_Int32, Reference< chart2::XDataSeries > > aIndexSeriesMap;
        for( ; aSeriesMapIt != aSeriesMapEnd; ++aSeriesMapIt )
            aIndexSeriesMap[aSeriesMapIt->second] = aSeriesMapIt->first;

        ::std::map< sal_Int32, Reference< chart2::XDataSeries > >::const_iterator aIndexIt( aIndexSeriesMap.begin() );
        ::std::map< sal_Int32, Reference< chart2::XDataSeries > >::const_iterator aIndexEnd( aIndexSeriesMap.end() );

        for( ; aIndexIt != aIndexEnd; ++aIndexIt )
            aNewSeriesVector.push_back( NewDonutSeries(aIndexIt->second,nOldSeriesCount) );
    }

    //overwrite attached axis information according to old series styles
    for( aIt = rStyleList.begin(); aIt != aEnd; ++aIt )
    {
        DataRowPointStyle aStyle(*aIt);
        if(aStyle.meType == DataRowPointStyle::DATA_SERIES )
        {
            aSeriesMapIt = aSeriesMap.find( aStyle.m_xSeries );
            if( aSeriesMapIt != aSeriesMapEnd && aSeriesMapIt->second < static_cast<sal_Int32>(aNewSeriesVector.size()) )
                aNewSeriesVector[aSeriesMapIt->second].mnAttachedAxis = aStyle.mnAttachedAxis;
        }
    }

    //overwrite new series style names with old series style name information
    for( aIt = rStyleList.begin(); aIt != aEnd; ++aIt )
    {
        DataRowPointStyle aStyle(*aIt);
        if( aStyle.meType == DataRowPointStyle::DATA_SERIES )
        {
            aSeriesMapIt = aSeriesMap.find(aStyle.m_xSeries);
            if( aSeriesMapEnd != aSeriesMapIt )
            {
                sal_Int32 nNewPointIndex = aSeriesMapIt->second;

                ::std::vector< NewDonutSeries >::iterator aNewSeriesIt( aNewSeriesVector.begin() );
                ::std::vector< NewDonutSeries >::iterator aNewSeriesEnd( aNewSeriesVector.end() );

                for( ;aNewSeriesIt!=aNewSeriesEnd; ++aNewSeriesIt)
                    aNewSeriesIt->setSeriesStyleNameToPoint( aStyle.msStyleName, nNewPointIndex );
            }
        }
    }

    //overwrite new series style names with point style name information
    for( aIt = rStyleList.begin(); aIt != aEnd; ++aIt )
    {
        DataRowPointStyle aStyle(*aIt);
        if( aStyle.meType == DataRowPointStyle::DATA_POINT )
        {
            aSeriesMapIt = aSeriesMap.find(aStyle.m_xSeries);
            if( aSeriesMapEnd != aSeriesMapIt )
            {
                sal_Int32 nNewPointIndex = aSeriesMapIt->second;
                sal_Int32 nNewSeriesIndex = aStyle.m_nPointIndex;
                sal_Int32 nRepeatCount = aStyle.m_nPointRepeat;

                while( nRepeatCount && (nNewSeriesIndex>=0) && (nNewSeriesIndex< static_cast<sal_Int32>(aNewSeriesVector.size()) ) )
                {
                    NewDonutSeries& rNewSeries( aNewSeriesVector[nNewSeriesIndex] );
                    rNewSeries.setPointStyleNameToPoint( aStyle.msStyleName, nNewPointIndex );

                    nRepeatCount--;
                    nNewSeriesIndex++;
                }
            }
        }
    }

    //put information from aNewSeriesVector to output parameter rStyleList
    rStyleList.clear();

    ::std::vector< NewDonutSeries >::iterator aNewSeriesIt( aNewSeriesVector.begin() );
    ::std::vector< NewDonutSeries >::iterator aNewSeriesEnd( aNewSeriesVector.end() );
    for( ;aNewSeriesIt!=aNewSeriesEnd; ++aNewSeriesIt)
    {
        ::std::list< DataRowPointStyle > aList( aNewSeriesIt->creatStyleList() );
        rStyleList.insert(rStyleList.end(),aList.begin(),aList.end());
    }
}

bool lcl_SpecialHandlingForDonutChartNeeded(
    const ::rtl::OUString & rServiceName,
    const SvXMLImport & rImport )
{
    bool bResult = false;
    if( rServiceName.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.DonutChartType" )))
    {
        sal_Int32 nBuildId = 0;
        sal_Int32 nUPD;
        if( (!rImport.getBuildIds( nUPD, nBuildId )))
            bResult= true;
    }
    return bResult;
}

} // anonymous namespace

void SchXMLChartContext::ChangeDiagramAccordingToTemplate(
        const uno::Reference< chart2::XChartDocument >& xNewDoc )
{
    if( !xNewDoc.is() )
        return;

    uno::Reference< chart2::XDiagram > xNewDia( xNewDoc->getFirstDiagram());
    uno::Reference< chart2::data::XDataProvider > xDataProvider( mrImportHelper.GetDataProvider( xNewDoc ) );
    if( !xNewDia.is() || !xDataProvider.is() )
        return;

    uno::Reference< chart2::XChartTypeTemplate > xTemplate( lcl_getTemplate( xNewDoc ));
    if(!xTemplate.is())
        return;

    sal_Bool bFirstCellAsLabel =
        (meDataRowSource==chart::ChartDataRowSource_COLUMNS)? mbRowHasLabels : mbColHasLabels;
    sal_Bool bHasCateories =
        (meDataRowSource==chart::ChartDataRowSource_COLUMNS)? mbColHasLabels : mbRowHasLabels;

    if( mbHasOwnTable )
    {
        bFirstCellAsLabel = true;
        bHasCateories = true;
    }

    uno::Sequence< beans::PropertyValue > aArgs( 3 );
    aArgs[0] = beans::PropertyValue(
        ::rtl::OUString::createFromAscii("CellRangeRepresentation"),
        -1, uno::makeAny( msChartAddress ),
        beans::PropertyState_DIRECT_VALUE );
    aArgs[1] = beans::PropertyValue(
        ::rtl::OUString::createFromAscii("DataRowSource"),
        -1, uno::makeAny( meDataRowSource ),
        beans::PropertyState_DIRECT_VALUE );
    aArgs[2] = beans::PropertyValue(
        ::rtl::OUString::createFromAscii("FirstCellAsLabel"),
        -1, uno::makeAny( bFirstCellAsLabel ),
        beans::PropertyState_DIRECT_VALUE );

    if( msColTrans.getLength() || msRowTrans.getLength() )
    {
        aArgs.realloc( aArgs.getLength() + 1 );
        aArgs[ aArgs.getLength() - 1 ] = beans::PropertyValue(
            ::rtl::OUString::createFromAscii("SequenceMapping"),
            -1, uno::makeAny( msColTrans.getLength()
                ? GetNumberSequenceFromString( msColTrans, bHasCateories && !xNewDoc->hasInternalDataProvider() )
                : GetNumberSequenceFromString( msRowTrans, bHasCateories && !xNewDoc->hasInternalDataProvider() ) ),
        beans::PropertyState_DIRECT_VALUE );
    }

    //work around wrong writer ranges ( see Issue 58464 )
    {
        rtl::OUString aChartOleObjectName;
        uno::Reference< frame::XModel > xModel(xNewDoc, uno::UNO_QUERY );
        if( xModel.is() )
        {
            comphelper::MediaDescriptor aMediaDescriptor( xModel->getArgs() );

            comphelper::MediaDescriptor::const_iterator aIt(
                aMediaDescriptor.find( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HierarchicalDocumentName" ))));
            if( aIt != aMediaDescriptor.end() )
            {
                aChartOleObjectName = (*aIt).second.get< ::rtl::OUString >();
            }
        }
        if( aChartOleObjectName.getLength() )
        {
            aArgs.realloc( aArgs.getLength() + 1 );
            aArgs[ aArgs.getLength() - 1 ] = beans::PropertyValue(
                ::rtl::OUString::createFromAscii("ChartOleObjectName"),
                -1, uno::makeAny( aChartOleObjectName ),
                beans::PropertyState_DIRECT_VALUE );
        }
    }


    uno::Reference< chart2::data::XDataSource > xDataSource(
        xDataProvider->createDataSource( aArgs ));

    aArgs.realloc( aArgs.getLength() + 1 );
    aArgs[ aArgs.getLength() - 1 ] = beans::PropertyValue(
        ::rtl::OUString::createFromAscii("HasCategories"),
        -1, uno::makeAny( bHasCateories ),
        beans::PropertyState_DIRECT_VALUE );

    xTemplate->changeDiagramData( xNewDia, xDataSource, aArgs );
}

void SchXMLChartContext::EndElement()
{
    uno::Reference< chart::XChartDocument > xDoc = mrImportHelper.GetChartDocument();
    uno::Reference< beans::XPropertySet > xProp( xDoc, uno::UNO_QUERY );
    uno::Reference< chart2::XChartDocument > xNewDoc( xDoc, uno::UNO_QUERY );

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

    // cleanup: remove empty chart type groups
    lcl_removeEmptyChartTypeGroups( xNewDoc );

    // set stack mode before a potential template detection (in case we have a
    // rectangular range)
    uno::Reference< chart::XDiagram > xDiagram( xDoc->getDiagram() );
    uno::Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY );
    if( xDiaProp.is())
    {
        if( maSeriesDefaultsAndStyles.maStackedDefault.hasValue())
            xDiaProp->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Stacked")),maSeriesDefaultsAndStyles.maStackedDefault);
        if( maSeriesDefaultsAndStyles.maPercentDefault.hasValue())
            xDiaProp->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Percent")),maSeriesDefaultsAndStyles.maPercentDefault);
        if( maSeriesDefaultsAndStyles.maDeepDefault.hasValue())
            xDiaProp->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Deep")),maSeriesDefaultsAndStyles.maDeepDefault);
        if( maSeriesDefaultsAndStyles.maStackedBarsConnectedDefault.hasValue())
            xDiaProp->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StackedBarsConnected")),maSeriesDefaultsAndStyles.maStackedBarsConnectedDefault);
    }

    //the OOo 2.0 implementation and older has a bug with donuts
    bool bSpecialHandlingForDonutChart = lcl_SpecialHandlingForDonutChartNeeded(
        maChartTypeServiceName, GetImport());

    // apply data
    if(!xNewDoc.is())
        return;

    // if we already have an internal data provider, we know that we cannot have
    // external data here.  If we can have external data but know that we have
    // internal data due to missing ranges, we must create an internal data
    // provider
    if( xNewDoc->hasInternalDataProvider())
        mbHasOwnTable = true;
    else if( mbHasOwnTable )
    {
        xNewDoc->createInternalDataProvider( sal_False /* bCloneExistingData */ );
    }
    if( mbHasOwnTable )
        msChartAddress = ::rtl::OUString::createFromAscii("all");

    bool bPostProcessTable = false;
    if( !mbHasOwnTable && mbAllRangeAddressesAvailable )
    {
        // special handling for stock chart (merge series together)
        if( mbIsStockChart )
            MergeSeriesForStockChart();
    }
    else if( msChartAddress.getLength() )
    {
        if( mbAllRangeAddressesAvailable && !bSpecialHandlingForDonutChart && !mbIsStockChart )
        {
            // note: mbRowHasLabels means the first row contains labels, that
            // means we have "column-descriptions", (analogously mbColHasLabels
            // means we have "row-descriptions")
            SchXMLTableHelper::applyTable( maTable, xNewDoc );
            bPostProcessTable = true;
        }
        else
        {
            // apply data read from the table sub-element to the chart
            // if the data provider supports the XChartDataArray interface like
            // the internal data provider
            uno::Reference< chart::XChartDataArray > xChartData( mrImportHelper.GetDataProvider( xNewDoc ), uno::UNO_QUERY );
            if( xChartData.is())
                SchXMLTableHelper::applyTableSimple( maTable, xChartData );

            // create datasource from data provider with rectangular range
            // parameters and change the diagram via template mechanism
            try
            {
                ChangeDiagramAccordingToTemplate( xNewDoc );
            }
            catch( uno::Exception & )
            {
                //try to fallback to internal data
                DBG_ERROR( "Exception during import SchXMLChartContext::ChangeDiagramAccordingToTemplate try to fallback to internal data" );
                if(!mbHasOwnTable)
                {
                    mbHasOwnTable = true;
                    msChartAddress = ::rtl::OUString::createFromAscii("all");
                    if( !xNewDoc->hasInternalDataProvider() )
                    {
                        xNewDoc->createInternalDataProvider( sal_False /* bCloneExistingData */ );
                        xChartData = uno::Reference< chart::XChartDataArray >( mrImportHelper.GetDataProvider( xNewDoc ), uno::UNO_QUERY );
                        if( xChartData.is())
                            SchXMLTableHelper::applyTableSimple( maTable, xChartData );
                        try
                        {
                            ChangeDiagramAccordingToTemplate( xNewDoc );
                        }
                        catch( uno::Exception & )
                        {
                            DBG_ERROR( "Exception during import SchXMLChartContext::ChangeDiagramAccordingToTemplate fallback to internal data failed also" );
                        }
                    }
                }
            }
        }
    }
    else
    {
        DBG_ERROR( " Must not get here" );
    }

    // now all series and data point properties are available and can be set
    {
        if( bSpecialHandlingForDonutChart )
        {
            uno::Reference< chart2::XDiagram > xNewDiagram( xNewDoc->getFirstDiagram() );
            lcl_swapPointAndSeriesStylesForDonutCharts( maSeriesDefaultsAndStyles.maSeriesStyleList
                , SchXMLSeriesHelper::getDataSeriesIndexMapFromDiagram(xNewDiagram) );
        }

        SchXMLSeries2Context::initSeriesPropertySets( maSeriesDefaultsAndStyles, uno::Reference< frame::XModel >(xDoc, uno::UNO_QUERY ) );

        //set defaults from diagram to the new series:
        //check whether we need to remove lines from symbol only charts
        bool bSwitchOffLinesForScatter = false;
        {
            bool bLinesOn = true;
            if( (maSeriesDefaultsAndStyles.maLinesOnProperty >>= bLinesOn) && !bLinesOn )
            {
                if( 0 == maChartTypeServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.ScatterChartType" ) ) )
                {
                    bSwitchOffLinesForScatter = true;
                    SchXMLSeries2Context::switchSeriesLinesOff( maSeriesDefaultsAndStyles.maSeriesStyleList );
                }
            }
        }
        SchXMLSeries2Context::setDefaultsToSeries( maSeriesDefaultsAndStyles );

        // set autostyles for series and data points
        const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
        const SvXMLStyleContext* pStyle = NULL;
        ::rtl::OUString sCurrStyleName;

        if( pStylesCtxt )
        {
            //iterate over data-series first
            //don't set series styles for donut charts
            if( !bSpecialHandlingForDonutChart )
            {
                SchXMLSeries2Context::setStylesToSeries( maSeriesDefaultsAndStyles
                                                         , pStylesCtxt, pStyle, sCurrStyleName, mrImportHelper, GetImport(), mbIsStockChart, maLSequencesPerIndex );
                // ... then set attributes for statistics (after their existence was set in the series)
                SchXMLSeries2Context::setStylesToStatisticsObjects( maSeriesDefaultsAndStyles
                            , pStylesCtxt, pStyle, sCurrStyleName );
            }

            // ... then iterate over data-point attributes, so the latter are not overwritten
            SchXMLSeries2Context::setStylesToDataPoints( maSeriesDefaultsAndStyles
                            , pStylesCtxt, pStyle, sCurrStyleName, mrImportHelper, GetImport(), mbIsStockChart, bSpecialHandlingForDonutChart, bSwitchOffLinesForScatter );
        }
    }

    if( bPostProcessTable )
        SchXMLTableHelper::postProcessTable( maTable, maLSequencesPerIndex, xNewDoc, meDataRowSource );

    if( xProp.is())
        xProp->setPropertyValue( rtl::OUString::createFromAscii( "RefreshAddInAllowed" ) , uno::makeAny( sal_True) );
}

void SchXMLChartContext::MergeSeriesForStockChart()
{
    OSL_ASSERT( mbIsStockChart );
    try
    {
        uno::Reference< chart::XChartDocument > xOldDoc( mrImportHelper.GetChartDocument());
        uno::Reference< chart2::XChartDocument > xDoc( xOldDoc, uno::UNO_QUERY_THROW );
        uno::Reference< chart2::XDiagram > xDiagram( xDoc->getFirstDiagram());
        if( ! xDiagram.is())
            return;

        bool bHasJapaneseCandlestick = true;
        uno::Reference< chart2::XDataSeriesContainer > xDSContainer;
        uno::Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY_THROW );
        uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 nCooSysIdx=0; nCooSysIdx<aCooSysSeq.getLength(); ++nCooSysIdx )
        {
            uno::Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY_THROW );
            uno::Sequence< uno::Reference< chart2::XChartType > > aChartTypes( xCTCnt->getChartTypes());
            for( sal_Int32 nCTIdx=0; nCTIdx<aChartTypes.getLength(); ++nCTIdx )
            {
                if( aChartTypes[nCTIdx]->getChartType().equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.CandleStickChartType")))
                {
                    xDSContainer.set( aChartTypes[nCTIdx], uno::UNO_QUERY_THROW );
                    uno::Reference< beans::XPropertySet > xCTProp( aChartTypes[nCTIdx], uno::UNO_QUERY_THROW );
                    xCTProp->getPropertyValue( ::rtl::OUString::createFromAscii("Japanese")) >>= bHasJapaneseCandlestick;
                    break;
                }
            }
        }

        if( xDSContainer.is())
        {
            // with japanese candlesticks: open, low, high, close
            // otherwise: low, high, close
            uno::Sequence< uno::Reference< chart2::XDataSeries > > aSeriesSeq( xDSContainer->getDataSeries());
            const sal_Int32 nSeriesCount( aSeriesSeq.getLength());
            const sal_Int32 nSeriesPerCandleStick = bHasJapaneseCandlestick ? 4: 3;
            sal_Int32 nCandleStickCount = nSeriesCount / nSeriesPerCandleStick;
            OSL_ASSERT( nSeriesPerCandleStick * nCandleStickCount == nSeriesCount );
            uno::Sequence< uno::Reference< chart2::XDataSeries > > aNewSeries( nCandleStickCount );
            for( sal_Int32 i=0; i<nCandleStickCount; ++i )
            {
                sal_Int32 nSeriesIndex = i*nSeriesPerCandleStick;
                if( bHasJapaneseCandlestick )
                {
                    // open values
                    lcl_setRoleAtFirstSequence( aSeriesSeq[ nSeriesIndex ], OUString::createFromAscii("values-first"));
                    aNewSeries[i] = aSeriesSeq[ nSeriesIndex ];
                    // low values
                    lcl_MoveDataToCandleStickSeries(
                        uno::Reference< chart2::data::XDataSource >( aSeriesSeq[ ++nSeriesIndex ], uno::UNO_QUERY_THROW ),
                        aNewSeries[i], OUString::createFromAscii("values-min"));
                }
                else
                {
                    // low values
                    lcl_setRoleAtFirstSequence( aSeriesSeq[ nSeriesIndex ], OUString::createFromAscii("values-min"));
                    aNewSeries[i] = aSeriesSeq[ nSeriesIndex ];
                }
                // high values
                lcl_MoveDataToCandleStickSeries(
                    uno::Reference< chart2::data::XDataSource >( aSeriesSeq[ ++nSeriesIndex ], uno::UNO_QUERY_THROW ),
                    aNewSeries[i], OUString::createFromAscii("values-max"));
                // close values
                lcl_MoveDataToCandleStickSeries(
                    uno::Reference< chart2::data::XDataSource >( aSeriesSeq[ ++nSeriesIndex ], uno::UNO_QUERY_THROW ),
                    aNewSeries[i], OUString::createFromAscii("values-last"));
            }
            xDSContainer->setDataSeries( aNewSeries );
        }
    }
    catch( uno::Exception & )
    {
        DBG_ERROR( "Exception while merging series for stock chart" );
    }
}

SvXMLImportContext* SchXMLChartContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
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
                                                  msChartAddress, mbHasOwnTable, mbAllRangeAddressesAvailable,
                                                  mbColHasLabels, mbRowHasLabels,
                                                  meDataRowSource,
                                                  maSeriesDefaultsAndStyles,
                                                  maChartTypeServiceName,
                                                  maLSequencesPerIndex, maChartSize );
            break;

        case XML_TOK_CHART_TITLE:
            if( xDoc.is())
            {
                if( xProp.is())
                {
                    xProp->setPropertyValue( rtl::OUString::createFromAscii( "HasMainTitle" ), aTrueBool );
                }
                uno::Reference< drawing::XShape > xTitleShape( xDoc->getTitle(), uno::UNO_QUERY );
                pContext = new SchXMLTitleContext( mrImportHelper, GetImport(),
                                                   rLocalName, maMainTitle, xTitleShape );
            }
            break;

        case XML_TOK_CHART_SUBTITLE:
            if( xDoc.is())
            {
                if( xProp.is())
                {
                    xProp->setPropertyValue( rtl::OUString::createFromAscii( "HasSubTitle" ), aTrueBool );
                }
                uno::Reference< drawing::XShape > xTitleShape( xDoc->getSubTitle(), uno::UNO_QUERY );
                pContext = new SchXMLTitleContext( mrImportHelper, GetImport(),
                                                   rLocalName, maSubTitle, xTitleShape );
            }
            break;

        case XML_TOK_CHART_LEGEND:
            pContext = new SchXMLLegendContext( mrImportHelper, GetImport(), rLocalName );
            break;

        case XML_TOK_CHART_TABLE:
            {
                SchXMLTableContext * pTableContext =
                    new SchXMLTableContext( mrImportHelper, GetImport(), rLocalName, maTable );
                // #i85913# take into account column- and row- mapping for
                // charts with own data only for those which were not copied
                // from a place where they got data from the container.  Note,
                // that this requires the plot-area been read before the table
                // (which is required in the ODF spec)
                // Note: For stock charts and donut charts with special handling
                // the mapping must not be applied!
                if( !msChartAddress.getLength() && !mbIsStockChart &&
                    !lcl_SpecialHandlingForDonutChartNeeded(
                        maChartTypeServiceName, GetImport()))
                {
                    if( msColTrans.getLength() > 0 )
                    {
                        OSL_ASSERT( msRowTrans.getLength() == 0 );
                        pTableContext->setColumnPermutation( GetNumberSequenceFromString( msColTrans, true ));
                    }
                    else if( msRowTrans.getLength() > 0 )
                    {
                        pTableContext->setRowPermutation( GetNumberSequenceFromString( msRowTrans, true ));
                    }
                }
                pContext = pTableContext;
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
        1.  Hide title, subtitle, and legend.
        2.  Set the size of the draw page.
        3.  Set a (logically) empty data set.
        4.  Set the chart type.
*/
void SchXMLChartContext::InitChart(
    awt::Size aChartSize,
    sal_Bool /* bDomainForDefaultDataNeeded */,
    const OUString & rChartTypeServiceName, // currently the old service name
    sal_Bool /* bSetSwitchData */ )
{
    maChartSize = aChartSize;
    uno::Reference< chart::XChartDocument > xDoc = mrImportHelper.GetChartDocument();
    DBG_ASSERT( xDoc.is(), "No valid document!" );
    uno::Reference< frame::XModel > xModel (xDoc, uno::UNO_QUERY );

    // Remove Title and Diagram ("De-InitNew")
    uno::Reference< chart2::XChartDocument > xNewDoc( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );
    if( xNewDoc.is())
    {
        xNewDoc->setFirstDiagram( 0 );
        uno::Reference< chart2::XTitled > xTitled( xNewDoc, uno::UNO_QUERY );
        if( xTitled.is())
            xTitled->setTitleObject( 0 );
    }

    //  Set the size of the draw page.
    uno::Reference< embed::XVisualObject > xVisualObject(xModel,uno::UNO_QUERY);
    DBG_ASSERT(xVisualObject.is(),"need xVisualObject for page size");
    if( xVisualObject.is() )
        xVisualObject->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT, aChartSize );

    //  Set the chart type via setting the diagram.
    if( rChartTypeServiceName.getLength() &&
        xDoc.is())
    {
        uno::Reference< lang::XMultiServiceFactory > xFact( xDoc, uno::UNO_QUERY );
        if( xFact.is())
        {
            uno::Reference< chart::XDiagram > xDia( xFact->createInstance( rChartTypeServiceName ), uno::UNO_QUERY );
            if( xDia.is())
                xDoc->setDiagram( xDia );
        }
    }
}

uno::Sequence< sal_Int32 > SchXMLChartContext::GetNumberSequenceFromString( const ::rtl::OUString& rStr, bool bAddOneToEachOldIndex )
{
    const sal_Unicode aSpace( ' ' );

    // count number of entries
    ::std::vector< sal_Int32 > aVec;
    sal_Int32 nLastPos = 0;
    sal_Int32 nPos = 0;
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

    if(!bAddOneToEachOldIndex)
    {
        sal_Int32* pSeqArr = aSeq.getArray();
        for( nPos = 0; nPos < nVecSize; ++nPos )
        {
            pSeqArr[ nPos ] = aVec[ nPos ];
        }
    }
    else if( bAddOneToEachOldIndex )
    {
        aSeq.realloc( nVecSize+1 );
        aSeq[0]=0;

        sal_Int32* pSeqArr = aSeq.getArray();
        for( nPos = 0; nPos < nVecSize; ++nPos )
        {
            pSeqArr[ nPos+1 ] = aVec[ nPos ]+1;
        }
    }

    return aSeq;
}

// ----------------------------------------

SchXMLTitleContext::SchXMLTitleContext( SchXMLImportHelper& rImpHelper, SvXMLImport& rImport,
                                        const rtl::OUString& rLocalName,
                                        rtl::OUString& rTitle,
                                        uno::Reference< drawing::XShape >& xTitleShape ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrTitle( rTitle ),
        mxTitleShape( xTitleShape )
{
}

SchXMLTitleContext::~SchXMLTitleContext()
{}

void SchXMLTitleContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;

    com::sun::star::awt::Point maPosition;
    bool bHasXPosition=false;
    bool bHasYPosition=false;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_SVG )
        {
            if( IsXMLToken( aLocalName, XML_X ) )
            {
                GetImport().GetMM100UnitConverter().convertMeasure( maPosition.X, aValue );
                bHasXPosition = true;
            }
            else if( IsXMLToken( aLocalName, XML_Y ) )
            {
                GetImport().GetMM100UnitConverter().convertMeasure( maPosition.Y, aValue );
                bHasYPosition = true;
            }
        }
        else if( nPrefix == XML_NAMESPACE_CHART )
        {
            if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                msAutoStyleName = aValue;
        }
    }


    if( mxTitleShape.is())
    {
        if( bHasXPosition && bHasYPosition )
            mxTitleShape->setPosition( maPosition );

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
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& )
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
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetLegendAttrTokenMap();

    awt::Point aLegendPos;
    bool bHasXPosition=false;
    bool bHasYPosition=false;

    rtl::OUString sAutoStyleName;

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
                GetImport().GetMM100UnitConverter().convertMeasure( aLegendPos.X, aValue );
                bHasXPosition = true;
                break;
            case XML_TOK_LEGEND_Y:
                GetImport().GetMM100UnitConverter().convertMeasure( aLegendPos.Y, aValue );
                bHasYPosition = true;
                break;
            case XML_TOK_LEGEND_STYLE_NAME:
                sAutoStyleName = aValue;
        }
    }

    uno::Reference< drawing::XShape > xLegendShape( xDoc->getLegend(), uno::UNO_QUERY );
    if( xLegendShape.is() && bHasXPosition && bHasYPosition )
        xLegendShape->setPosition( aLegendPos );

    // set auto-styles for Legend
    uno::Reference< beans::XPropertySet > xProp( xLegendShape, uno::UNO_QUERY );
    if( xProp.is())
    {
        // the fill style has the default "none" in XML, but "solid" in the model.
        xProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" )),
                                 uno::makeAny( drawing::FillStyle_NONE ));
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
