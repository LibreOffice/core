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

#include "SchXMLChartContext.hxx"
#include "SchXMLImport.hxx"
#include "SchXMLLegendContext.hxx"
#include "SchXMLPlotAreaContext.hxx"
#include "SchXMLParagraphContext.hxx"
#include "SchXMLTableContext.hxx"
#include "SchXMLSeries2Context.hxx"
#include "SchXMLTools.hxx"
#include <comphelper/mediadescriptor.hxx>
#include <tools/debug.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlement.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/SchXMLSeriesHelper.hxx>

#include "vector"
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XDiagram.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/util/XStringMapping.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/ChartSeriesAddress.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XTitled.hpp>

using namespace com::sun::star;
using namespace ::xmloff::token;
using com::sun::star::uno::Reference;
using namespace ::SchXMLTools;

namespace
{

void lcl_setRoleAtLabeledSequence(
    const uno::Reference< chart2::data::XLabeledDataSequence > & xLSeq,
    const OUString &rRole )
{
    // set role of sequence
    uno::Reference< chart2::data::XDataSequence > xValues( xLSeq->getValues());
    if( xValues.is())
    {
        uno::Reference< beans::XPropertySet > xProp( xValues, uno::UNO_QUERY );
        if( xProp.is())
            xProp->setPropertyValue("Role", uno::makeAny( rRole ));
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
    catch(const uno::Exception&)
    {
        SAL_WARN("xmloff.chart", "Exception caught while moving data to candlestick series" );
    }
}

void lcl_setRoleAtFirstSequence(
    const uno::Reference< chart2::XDataSeries > & xSeries,
    const OUString & rRole )
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
    catch(const uno::Exception& ex)
    {
        OString aBStr(OUStringToOString(ex.Message, RTL_TEXTENCODING_ASCII_US));
        SAL_INFO("xmloff.chart", "Exception caught while removing empty chart types: " << aBStr);
    }
}

uno::Sequence< sal_Int32 > lcl_getNumberSequenceFromString( const OUString& rStr, bool bAddOneToEachOldIndex )
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

} // anonymous namespace

SchXMLChartContext::SchXMLChartContext( SchXMLImportHelper& rImpHelper,
                                        SvXMLImport& rImport, const OUString& rLocalName ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        m_bHasRangeAtPlotArea( false ),
        m_bHasTableElement( false ),
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

    uno::Reference< embed::XVisualObject > xVisualObject( mrImportHelper.GetChartDocument(), uno::UNO_QUERY);
    SAL_WARN_IF(!xVisualObject.is(), "xmloff.chart", "need xVisualObject for page size");
    if( xVisualObject.is() )
        maChartSize = xVisualObject->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT ); //#i103460# take the size given from the parent frame as default

    // this flag is necessarry for pie charts in the core
    bool bSetSwitchData = false;

    OUString sAutoStyleName;
    OUString aOldChartTypeName;
    bool bHasAddin = false;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_CHART_HREF:
                m_aXLinkHRefAttributeToIndicateDataProvider = aValue;
                break;

            case XML_TOK_CHART_CLASS:
                {
                    OUString sClassName;
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
                                bSetSwitchData = true;
                                break;
                            case XML_CHART_CLASS_STOCK:
                                mbIsStockChart = true;
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

                        aOldChartTypeName = sClassName;
                        maChartTypeServiceName = sClassName;
                    }
                }
                break;

            case XML_TOK_CHART_WIDTH:
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        maChartSize.Width, aValue );
                break;

            case XML_TOK_CHART_HEIGHT:
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        maChartSize.Height, aValue );
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

    if( aOldChartTypeName.isEmpty() )
    {
        SAL_WARN("xmloff.chart", "need a charttype to create a diagram" );
        //set a fallback value:
        OUString aChartClass_Bar( GetXMLToken(XML_BAR ) );
        aOldChartTypeName = SchXMLTools::GetChartTypeByClassName( aChartClass_Bar, true /* bUseOldNames */ );
        maChartTypeServiceName = SchXMLTools::GetChartTypeByClassName( aChartClass_Bar, false /* bUseOldNames */ );
    }

    //  Set the size of the draw page.
    if( xVisualObject.is() )
        xVisualObject->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT, maChartSize );

    InitChart( aOldChartTypeName, bSetSwitchData);

    if( bHasAddin )
    {
        //correct charttype serveice name when having an addin
        //and don't refresh addin during load
        uno::Reference< beans::XPropertySet > xDocProp( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );
        if( xDocProp.is() )
        {
            try
            {
                xDocProp->getPropertyValue("BaseDiagram") >>= aOldChartTypeName;
                maChartTypeServiceName =  SchXMLTools::GetNewChartTypeName( aOldChartTypeName );
                xDocProp->setPropertyValue("RefreshAddInAllowed", uno::makeAny( sal_False) );
            }
            catch(const uno::Exception&)
            {
                SAL_WARN("xmloff.chart", "Exception during import SchXMLChartContext::StartElement" );
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
    OUString msStyleName;
    sal_Int32 mnAttachedAxis;

    ::std::vector< OUString > m_aSeriesStyles;
    ::std::vector< OUString > m_aPointStyles;

    NewDonutSeries( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >& xSeries, sal_Int32 nPointCount )
                    : m_xSeries( xSeries )
                    , mnAttachedAxis( 1 )
    {
        m_aPointStyles.resize(nPointCount);
        m_aSeriesStyles.resize(nPointCount);
    }

    void setSeriesStyleNameToPoint( const OUString& rStyleName, sal_Int32 nPointIndex )
    {
        SAL_WARN_IF(nPointIndex >= static_cast<sal_Int32>(m_aSeriesStyles.size()), "xmloff.chart", "donut point <-> series count mismatch");
        if( nPointIndex < static_cast<sal_Int32>(m_aSeriesStyles.size()) )
            m_aSeriesStyles[nPointIndex]=rStyleName;
    }

    void setPointStyleNameToPoint( const OUString& rStyleName, sal_Int32 nPointIndex )
    {
        SAL_WARN_IF(nPointIndex >= static_cast<sal_Int32>(m_aPointStyles.size()), "xmloff.chart", "donut point <-> series count mismatch");
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
        ::std::vector< OUString >::iterator aPointIt( m_aPointStyles.begin() );
        ::std::vector< OUString >::iterator aPointEnd( m_aPointStyles.end() );
        while( aPointIt != aPointEnd )
        {
            DataRowPointStyle aPointStyle( DataRowPointStyle::DATA_POINT
                , m_xSeries, nPointIndex, 1, *aPointIt, mnAttachedAxis );
            if( nPointIndex < static_cast<sal_Int32>(m_aSeriesStyles.size()) )
            {
                aPointStyle.msSeriesStyleNameForDonuts = m_aSeriesStyles[nPointIndex];
            }
            if( !aPointStyle.msSeriesStyleNameForDonuts.isEmpty()
                || !aPointStyle.msStyleName.isEmpty() )
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
    const OUString & rServiceName,
    const SvXMLImport & rImport )
{
    bool bResult = false;
    if( rServiceName == "com.sun.star.chart2.DonutChartType" )
    {
        bResult = SchXMLTools::isDocumentGeneratedWithOpenOfficeOlderThan2_3( rImport.GetModel() );
    }
    return bResult;
}

} // anonymous namespace

static void lcl_ApplyDataFromRectangularRangeToDiagram(
        const uno::Reference< chart2::XChartDocument >& xNewDoc
        , const OUString& rRectangularRange
        , ::com::sun::star::chart::ChartDataRowSource eDataRowSource
        , bool bRowHasLabels, bool bColHasLabels
        , bool bSwitchOnLabelsAndCategoriesForOwnData
        , const OUString& sColTrans
        , const OUString& sRowTrans )
{
    if( !xNewDoc.is() )
        return;

    uno::Reference< chart2::XDiagram > xNewDia( xNewDoc->getFirstDiagram());
    uno::Reference< chart2::data::XDataProvider > xDataProvider( xNewDoc->getDataProvider() );
    if( !xNewDia.is() || !xDataProvider.is() )
        return;

    sal_Bool bFirstCellAsLabel =
        (eDataRowSource==chart::ChartDataRowSource_COLUMNS)? bRowHasLabels : bColHasLabels;
    sal_Bool bHasCateories =
        (eDataRowSource==chart::ChartDataRowSource_COLUMNS)? bColHasLabels : bRowHasLabels;

    if( bSwitchOnLabelsAndCategoriesForOwnData )
    {
        bFirstCellAsLabel = true;
        bHasCateories = true;
    }

    uno::Sequence< beans::PropertyValue > aArgs( 3 );
    aArgs[0] = beans::PropertyValue(
        OUString( "CellRangeRepresentation" ),
        -1, uno::makeAny( rRectangularRange ),
        beans::PropertyState_DIRECT_VALUE );
    aArgs[1] = beans::PropertyValue(
        OUString( "DataRowSource" ),
        -1, uno::makeAny( eDataRowSource ),
        beans::PropertyState_DIRECT_VALUE );
    aArgs[2] = beans::PropertyValue(
        OUString( "FirstCellAsLabel" ),
        -1, uno::makeAny( bFirstCellAsLabel ),
        beans::PropertyState_DIRECT_VALUE );

    if( !sColTrans.isEmpty() || !sRowTrans.isEmpty() )
    {
        aArgs.realloc( aArgs.getLength() + 1 );
        aArgs[ sal::static_int_cast<sal_uInt32>(aArgs.getLength()) - 1 ] = beans::PropertyValue(
            OUString( "SequenceMapping" ),
            -1, uno::makeAny( !sColTrans.isEmpty()
                ? lcl_getNumberSequenceFromString( sColTrans, bHasCateories && !xNewDoc->hasInternalDataProvider() )
                : lcl_getNumberSequenceFromString( sRowTrans, bHasCateories && !xNewDoc->hasInternalDataProvider() ) ),
        beans::PropertyState_DIRECT_VALUE );
    }

    //work around wrong writer ranges ( see Issue 58464 )
    {
        OUString aChartOleObjectName;
        uno::Reference< frame::XModel > xModel(xNewDoc, uno::UNO_QUERY );
        if( xModel.is() )
        {
            comphelper::MediaDescriptor aMediaDescriptor( xModel->getArgs() );

            comphelper::MediaDescriptor::const_iterator aIt(
                aMediaDescriptor.find( OUString(  "HierarchicalDocumentName" )));
            if( aIt != aMediaDescriptor.end() )
            {
                aChartOleObjectName = (*aIt).second.get< OUString >();
            }
        }
        if( !aChartOleObjectName.isEmpty() )
        {
            aArgs.realloc( aArgs.getLength() + 1 );
            aArgs[ sal::static_int_cast<sal_uInt32>(aArgs.getLength()) - 1 ] = beans::PropertyValue(
                OUString( "ChartOleObjectName" ),
                -1, uno::makeAny( aChartOleObjectName ),
                beans::PropertyState_DIRECT_VALUE );
        }
    }

    uno::Reference< chart2::data::XDataSource > xDataSource(
        xDataProvider->createDataSource( aArgs ));

    aArgs.realloc( aArgs.getLength() + 2 );
    aArgs[ sal::static_int_cast<sal_uInt32>(aArgs.getLength()) - 2 ] = beans::PropertyValue(
        OUString( "HasCategories" ),
        -1, uno::makeAny( bHasCateories ),
        beans::PropertyState_DIRECT_VALUE );
    aArgs[ sal::static_int_cast<sal_uInt32>(aArgs.getLength()) - 1 ] = beans::PropertyValue(
        OUString("UseCategoriesAsX"),
        -1, uno::makeAny( sal_False ),//categories in ODF files are not to be used as x values (independent from what is offered in our ui)
        beans::PropertyState_DIRECT_VALUE );

    xNewDia->setDiagramData( xDataSource, aArgs );
}

void SchXMLChartContext::EndElement()
{
    uno::Reference< chart::XChartDocument > xDoc = mrImportHelper.GetChartDocument();
    uno::Reference< beans::XPropertySet > xProp( xDoc, uno::UNO_QUERY );
    uno::Reference< chart2::XChartDocument > xNewDoc( xDoc, uno::UNO_QUERY );

    if( xProp.is())
    {
        if( !maMainTitle.isEmpty())
        {
            uno::Reference< beans::XPropertySet > xTitleProp( xDoc->getTitle(), uno::UNO_QUERY );
            if( xTitleProp.is())
            {
                try
                {
                    uno::Any aAny;
                    aAny <<= maMainTitle;
                    xTitleProp->setPropertyValue("String", aAny );
                }
                catch(const beans::UnknownPropertyException&)
                {
                    SAL_WARN("xmloff.chart", "Property String for Title not available" );
                }
            }
        }
        if( !maSubTitle.isEmpty())
        {
            uno::Reference< beans::XPropertySet > xTitleProp( xDoc->getSubTitle(), uno::UNO_QUERY );
            if( xTitleProp.is())
            {
                try
                {
                    uno::Any aAny;
                    aAny <<= maSubTitle;
                    xTitleProp->setPropertyValue("String", aAny );
                }
                catch(const beans::UnknownPropertyException&)
                {
                    SAL_WARN("xmloff.chart", "Property String for Title not available" );
                }
            }
        }
    }

    // cleanup: remove empty chart type groups
    lcl_removeEmptyChartTypeGroups( xNewDoc );

    // set stack mode before a potential chart type detection (in case we have a rectangular range)
    uno::Reference< chart::XDiagram > xDiagram( xDoc->getDiagram() );
    uno::Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY );
    if( xDiaProp.is())
    {
        if( maSeriesDefaultsAndStyles.maStackedDefault.hasValue())
            xDiaProp->setPropertyValue("Stacked",maSeriesDefaultsAndStyles.maStackedDefault);
        if( maSeriesDefaultsAndStyles.maPercentDefault.hasValue())
            xDiaProp->setPropertyValue("Percent",maSeriesDefaultsAndStyles.maPercentDefault);
        if( maSeriesDefaultsAndStyles.maDeepDefault.hasValue())
            xDiaProp->setPropertyValue("Deep",maSeriesDefaultsAndStyles.maDeepDefault);
        if( maSeriesDefaultsAndStyles.maStackedBarsConnectedDefault.hasValue())
            xDiaProp->setPropertyValue("StackedBarsConnected",maSeriesDefaultsAndStyles.maStackedBarsConnectedDefault);
    }

    //the OOo 2.0 implementation and older has a bug with donuts
    bool bSpecialHandlingForDonutChart = lcl_SpecialHandlingForDonutChartNeeded(
        maChartTypeServiceName, GetImport());

    // apply data
    if(!xNewDoc.is())
        return;

    bool bHasOwnData = false;
    if( m_aXLinkHRefAttributeToIndicateDataProvider == "." ) //data comes from the chart itself
        bHasOwnData = true;
    else if( m_aXLinkHRefAttributeToIndicateDataProvider == ".." ) //data comes from the parent application
        bHasOwnData = false;
    else if( !m_aXLinkHRefAttributeToIndicateDataProvider.isEmpty() ) //not supported so far to get the data by sibling objects -> fall back to chart itself if data are available
        bHasOwnData = m_bHasTableElement;
    else
        bHasOwnData = !m_bHasRangeAtPlotArea;

    if( xNewDoc->hasInternalDataProvider())
    {
        if( !m_bHasTableElement && m_aXLinkHRefAttributeToIndicateDataProvider != "." )
        {
            //#i103147# ODF, workaround broken files with a missing table:cell-range-address at the plot-area
            bool bSwitchSuccessful = SchXMLTools::switchBackToDataProviderFromParent( xNewDoc, maLSequencesPerIndex );
            bHasOwnData = !bSwitchSuccessful;
        }
        else
            bHasOwnData = true;//e.g. in case of copy->paste from calc to impress
    }
    else if( bHasOwnData )
    {
        xNewDoc->createInternalDataProvider( sal_False /* bCloneExistingData */ );
    }
    if( bHasOwnData )
        msChartAddress = "all";

    bool bSwitchRangesFromOuterToInternalIfNecessary = false;
    if( !bHasOwnData && mbAllRangeAddressesAvailable )
    {
        // special handling for stock chart (merge series together)
        if( mbIsStockChart )
            MergeSeriesForStockChart();
    }
    else if( !msChartAddress.isEmpty() )
    {
        //own data or only rectangular range available

        if( xNewDoc->hasInternalDataProvider() )
            SchXMLTableHelper::applyTableToInternalDataProvider( maTable, xNewDoc, meDataRowSource );

        bool bOlderThan2_3 = SchXMLTools::isDocumentGeneratedWithOpenOfficeOlderThan2_3( Reference< frame::XModel >( xNewDoc, uno::UNO_QUERY ));
        bool bOldFileWithOwnDataFromRows = (bOlderThan2_3 && bHasOwnData && (meDataRowSource==chart::ChartDataRowSource_ROWS)); // in this case there are range addresses that are simply wrong.

        if( mbAllRangeAddressesAvailable && !bSpecialHandlingForDonutChart && !mbIsStockChart &&
            !bOldFileWithOwnDataFromRows )
        {
            //bHasOwnData is true in this case!
            //e.g. for normal files with own data or also in case of copy paste scenario (e.g. calc to impress)
            bSwitchRangesFromOuterToInternalIfNecessary = true;
        }
        else
        {
            //apply data from rectangular range

            // create datasource from data provider with rectangular range parameters and change the diagram setDiagramData
            try
            {
                if( bOlderThan2_3 && xDiaProp.is() )//for older charts the hidden cells were removed by calc on the fly
                    xDiaProp->setPropertyValue("IncludeHiddenCells",uno::makeAny(false));

                // note: mbRowHasLabels means the first row contains labels, that means we have "column-descriptions",
                // (analogously mbColHasLabels means we have "row-descriptions")
                lcl_ApplyDataFromRectangularRangeToDiagram( xNewDoc, msChartAddress, meDataRowSource, mbRowHasLabels, mbColHasLabels, bHasOwnData, msColTrans, msRowTrans );
            }
            catch(const uno::Exception&)
            {
                //try to fallback to internal data
                SAL_WARN("xmloff.chart", "Exception during import SchXMLChartContext::lcl_ApplyDataFromRectangularRangeToDiagram try to fallback to internal data" );
                if(!bHasOwnData)
                {
                    bHasOwnData = true;
                    msChartAddress = "all";
                    if( !xNewDoc->hasInternalDataProvider() )
                    {
                        xNewDoc->createInternalDataProvider( sal_False /* bCloneExistingData */ );
                        SchXMLTableHelper::applyTableToInternalDataProvider( maTable, xNewDoc, meDataRowSource );
                        try
                        {
                            lcl_ApplyDataFromRectangularRangeToDiagram( xNewDoc, msChartAddress, meDataRowSource, mbRowHasLabels, mbColHasLabels, bHasOwnData, msColTrans, msRowTrans );
                        }
                        catch(const uno::Exception&)
                        {
                            SAL_WARN("xmloff.chart", "Exception during import SchXMLChartContext::lcl_ApplyDataFromRectangularRangeToDiagram fallback to internal data failed also" );
                        }
                    }
                }
            }
        }
    }
    else
    {
        SAL_WARN("xmloff.chart", "Must not get here" );
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
                if( 0 == maChartTypeServiceName.reverseCompareTo( "com.sun.star.chart2.ScatterChartType" ) )
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
        OUString sCurrStyleName;

        if( pStylesCtxt )
        {
            //iterate over data-series first
            //don't set series styles for donut charts
            if( !bSpecialHandlingForDonutChart )
            {
                SchXMLSeries2Context::setStylesToSeries(
                                        maSeriesDefaultsAndStyles, pStylesCtxt, pStyle,
                                        sCurrStyleName, mrImportHelper, GetImport(),
                                        mbIsStockChart, maLSequencesPerIndex );
                // ... then set attributes for statistics (after their existence was set in the series)
                SchXMLSeries2Context::setStylesToStatisticsObjects(
                                        maSeriesDefaultsAndStyles, pStylesCtxt,
                                        pStyle, sCurrStyleName );

                SchXMLSeries2Context::setStylesToRegressionCurves(
                                        maSeriesDefaultsAndStyles, pStylesCtxt,
                                        pStyle, sCurrStyleName );
            }
        }

        //#i98319# call switchRangesFromOuterToInternalIfNecessary before the data point styles are applied, otherwise in copy->paste scenario the data point styles do get lost
        if( bSwitchRangesFromOuterToInternalIfNecessary )
        {
            if( xNewDoc->hasInternalDataProvider() )
                SchXMLTableHelper::switchRangesFromOuterToInternalIfNecessary( maTable, maLSequencesPerIndex, xNewDoc, meDataRowSource );
        }

        if( pStylesCtxt )
        {
            // ... then iterate over data-point attributes, so the latter are not overwritten
            SchXMLSeries2Context::setStylesToDataPoints( maSeriesDefaultsAndStyles
                            , pStylesCtxt, pStyle, sCurrStyleName, mrImportHelper, GetImport(), mbIsStockChart, bSpecialHandlingForDonutChart, bSwitchOffLinesForScatter );
        }
    }

    if( xProp.is())
        xProp->setPropertyValue("RefreshAddInAllowed", uno::makeAny( sal_True) );
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
                if( aChartTypes[nCTIdx]->getChartType() == "com.sun.star.chart2.CandleStickChartType" )
                {
                    xDSContainer.set( aChartTypes[nCTIdx], uno::UNO_QUERY_THROW );
                    uno::Reference< beans::XPropertySet > xCTProp( aChartTypes[nCTIdx], uno::UNO_QUERY_THROW );
                    xCTProp->getPropertyValue("Japanese") >>= bHasJapaneseCandlestick;
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
                    lcl_setRoleAtFirstSequence( aSeriesSeq[ nSeriesIndex ], OUString( "values-first" ));
                    aNewSeries[i] = aSeriesSeq[ nSeriesIndex ];
                    // low values
                    lcl_MoveDataToCandleStickSeries(
                        uno::Reference< chart2::data::XDataSource >( aSeriesSeq[ ++nSeriesIndex ], uno::UNO_QUERY_THROW ),
                        aNewSeries[i], OUString( "values-min" ));
                }
                else
                {
                    // low values
                    lcl_setRoleAtFirstSequence( aSeriesSeq[ nSeriesIndex ], OUString( "values-min" ));
                    aNewSeries[i] = aSeriesSeq[ nSeriesIndex ];
                }
                // high values
                lcl_MoveDataToCandleStickSeries(
                    uno::Reference< chart2::data::XDataSource >( aSeriesSeq[ ++nSeriesIndex ], uno::UNO_QUERY_THROW ),
                    aNewSeries[i], OUString( "values-max" ));
                // close values
                lcl_MoveDataToCandleStickSeries(
                    uno::Reference< chart2::data::XDataSource >( aSeriesSeq[ ++nSeriesIndex ], uno::UNO_QUERY_THROW ),
                    aNewSeries[i], OUString( "values-last" ));
            }
            xDSContainer->setDataSeries( aNewSeries );
        }
    }
    catch(const uno::Exception&)
    {
        SAL_WARN("xmloff.chart", "Exception while merging series for stock chart" );
    }
}

SvXMLImportContext* SchXMLChartContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
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
                                                  m_aXLinkHRefAttributeToIndicateDataProvider,
                                                  msCategoriesAddress,
                                                  msChartAddress, m_bHasRangeAtPlotArea, mbAllRangeAddressesAvailable,
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
                    xProp->setPropertyValue("HasMainTitle", aTrueBool );
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
                    xProp->setPropertyValue("HasSubTitle", aTrueBool );
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
                m_bHasTableElement = true;
                // #i85913# take into account column- and row- mapping for
                // charts with own data only for those which were not copied
                // from a place where they got data from the container.  Note,
                // that this requires the plot-area been read before the table
                // (which is required in the ODF spec)
                // Note: For stock charts and donut charts with special handling
                // the mapping must not be applied!
                if( msChartAddress.isEmpty() && !mbIsStockChart &&
                    !lcl_SpecialHandlingForDonutChartNeeded(
                        maChartTypeServiceName, GetImport()))
                {
                    if( !msColTrans.isEmpty() )
                    {
                        OSL_ASSERT( msRowTrans.isEmpty() );
                        pTableContext->setColumnPermutation( lcl_getNumberSequenceFromString( msColTrans, true ));
                        msColTrans = OUString();
                    }
                    else if( !msRowTrans.isEmpty() )
                    {
                        pTableContext->setRowPermutation( lcl_getNumberSequenceFromString( msRowTrans, true ));
                        msRowTrans = OUString();
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

                SAL_WARN_IF( !mxDrawPage.is(), "xmloff.chart", "Invalid Chart Page" );
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
    const OUString & rChartTypeServiceName, // currently the old service name
    bool /* bSetSwitchData */ )
{
    uno::Reference< chart::XChartDocument > xDoc = mrImportHelper.GetChartDocument();
    SAL_WARN_IF( !xDoc.is(), "xmloff.chart", "No valid document!" );
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

    //  Set the chart type via setting the diagram.
    if( !rChartTypeServiceName.isEmpty() && xDoc.is())
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

SchXMLTitleContext::SchXMLTitleContext( SchXMLImportHelper& rImpHelper, SvXMLImport& rImport,
                                        const OUString& rLocalName,
                                        OUString& rTitle,
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
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_SVG )
        {
            if( IsXMLToken( aLocalName, XML_X ) )
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        maPosition.X, aValue );
                bHasXPosition = true;
            }
            else if( IsXMLToken( aLocalName, XML_Y ) )
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        maPosition.Y, aValue );
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
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
