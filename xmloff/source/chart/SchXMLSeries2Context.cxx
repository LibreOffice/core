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

#include "SchXMLSeries2Context.hxx"
#include "SchXMLPlotAreaContext.hxx"
#include "SchXMLRegressionCurveObjectContext.hxx"
#include "SchXMLPropertyMappingContext.hxx"
#include "SchXMLTools.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XPivotTableDataProvider.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>

#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelFieldType.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelField.hpp>

#include <com/sun/star/chart/ChartAxisAssign.hpp>
#include <com/sun/star/chart/ChartSymbolType.hpp>
#include <com/sun/star/chart/ChartDataCaption.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>

#include <comphelper/processfactory.hxx>

#include <sal/log.hxx>
#include <utility>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/SchXMLSeriesHelper.hxx>
#include <xmloff/prstylei.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm> // std::find_if

using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

class SchXMLDomain2Context : public SvXMLImportContext
{
private:
    ::std::vector< OUString > & mrAddresses;

public:
    SchXMLDomain2Context( SvXMLImport& rImport,
                          ::std::vector< OUString > & rAddresses );
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

SchXMLDomain2Context::SchXMLDomain2Context(
    SvXMLImport& rImport,
    ::std::vector< OUString > & rAddresses ) :
        SvXMLImportContext( rImport ),
        mrAddresses( rAddresses )
{
}

void SchXMLDomain2Context::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if (aIter.getToken() == XML_ELEMENT(TABLE, XML_CELL_RANGE_ADDRESS) )
            mrAddresses.push_back( aIter.toString() );
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }
}

void lcl_setAutomaticSymbolSize( const uno::Reference< beans::XPropertySet >& xSeriesOrPointProp, const SvXMLImport& rImport )
{
    awt::Size aSymbolSize(140,140);//old default for standard sized charts 7cm height

    uno::Reference< chart::XChartDocument > xChartDoc( rImport.GetModel(), uno::UNO_QUERY );
    if( xChartDoc.is() )
    {
        double fScale = 1;
        uno::Reference< beans::XPropertySet > xLegendProp( xChartDoc->getLegend(), uno::UNO_QUERY );
        chart::ChartLegendPosition aLegendPosition = chart::ChartLegendPosition_NONE;
        if( xLegendProp.is() && (xLegendProp->getPropertyValue(u"Alignment"_ustr) >>= aLegendPosition)
            && chart::ChartLegendPosition_NONE != aLegendPosition )
        {

            double fFontHeight = 6.0;
            if( xLegendProp->getPropertyValue(u"CharHeight"_ustr) >>= fFontHeight )
                fScale = 0.75*fFontHeight/6.0;
        }
        else
        {
            uno::Reference< embed::XVisualObject > xVisualObject( rImport.GetModel(), uno::UNO_QUERY );
            if( xVisualObject.is() )
            {
                awt::Size aPageSize( xVisualObject->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT ) );
                fScale = aPageSize.Height/7000.0;
            }
        }
        if( fScale>0 )
        {
            aSymbolSize.Height = static_cast<sal_Int32>( fScale * aSymbolSize.Height );
            aSymbolSize.Width = aSymbolSize.Height;
        }
    }
    xSeriesOrPointProp->setPropertyValue(u"SymbolSize"_ustr,uno::Any( aSymbolSize ));
}

void lcl_setSymbolSizeIfNeeded( const uno::Reference< beans::XPropertySet >& xSeriesOrPointProp, const SvXMLImport& rImport )
{
    if( !xSeriesOrPointProp.is() )
        return;

    sal_Int32 nSymbolType = chart::ChartSymbolType::NONE;
    if( !(xSeriesOrPointProp.is() && ( xSeriesOrPointProp->getPropertyValue(u"SymbolType"_ustr) >>= nSymbolType)) )
        return;

    if(chart::ChartSymbolType::NONE!=nSymbolType)
    {
        if( chart::ChartSymbolType::BITMAPURL==nSymbolType )
        {
            //set special size for graphics to indicate to use the bitmap size itself
            xSeriesOrPointProp->setPropertyValue(u"SymbolSize"_ustr,uno::Any( awt::Size(-1,-1) ));
        }
        else
        {
            lcl_setAutomaticSymbolSize( xSeriesOrPointProp, rImport );
        }
    }
}

void lcl_resetSymbolSizeForPointsIfNecessary( const uno::Reference< beans::XPropertySet >& xPointProp, const SvXMLImport& rImport
    , const XMLPropStyleContext * pPropStyleContext, const SvXMLStylesContext* pStylesCtxt )
{
    uno::Any aASymbolSize( SchXMLTools::getPropertyFromContext( u"SymbolSize", pPropStyleContext, pStylesCtxt ) );
    if( !aASymbolSize.hasValue() )
        lcl_setSymbolSizeIfNeeded( xPointProp, rImport );
}

void lcl_setLinkNumberFormatToSourceIfNeeded( const uno::Reference< beans::XPropertySet >& xPointProp
    , const XMLPropStyleContext* pPropStyleContext, const SvXMLStylesContext* pStylesCtxt )
{
    uno::Any aAny( SchXMLTools::getPropertyFromContext(u"LinkNumberFormatToSource", pPropStyleContext, pStylesCtxt) );
    if( aAny.hasValue() )
        return;

    if( !xPointProp.is() )
        return;

    bool bLinkToSource = false;
    if( xPointProp.is() && (xPointProp->getPropertyValue(u"LinkNumberFormatToSource"_ustr) >>= bLinkToSource) )
    {
        if( bLinkToSource )
        {
            xPointProp->setPropertyValue(u"LinkNumberFormatToSource"_ustr, uno::Any(false));
        }
    }
}

void lcl_insertErrorBarLSequencesToMap(
    tSchXMLLSequencesPerIndex & rInOutMap,
    const uno::Reference< beans::XPropertySet > & xSeriesProp )
{
    Reference< chart2::data::XDataSource > xErrorBarSource;
    if( ( xSeriesProp->getPropertyValue( u"ErrorBarY"_ustr ) >>= xErrorBarSource ) &&
        xErrorBarSource.is() )
    {
        const Sequence< Reference< chart2::data::XLabeledDataSequence > > aLSequences(
            xErrorBarSource->getDataSequences());
        for( const auto& rLSequence : aLSequences )
        {
            // use "0" as data index. This is ok, as it is not used for error bars
            rInOutMap.emplace(
                    tSchXMLIndexWithPart( 0, SCH_XML_PART_ERROR_BARS ), rLSequence );
        }
    }
}

Reference< chart2::data::XLabeledDataSequence2 > lcl_createAndAddSequenceToSeries( const OUString& rRole
        , const OUString& rRange
        , const Reference< chart2::XChartDocument >& xChartDoc
        , const Reference< chart2::XDataSeries >& xSeries )
{
    Reference< chart2::data::XLabeledDataSequence2 > xLabeledSeq;

    Reference< chart2::data::XDataSource > xSeriesSource( xSeries,uno::UNO_QUERY );
    Reference< chart2::data::XDataSink > xSeriesSink( xSeries, uno::UNO_QUERY );

    if( !(!rRange.isEmpty() && xChartDoc.is() && xSeriesSource.is() && xSeriesSink.is()) )
        return xLabeledSeq;

    // create a new sequence
    xLabeledSeq = SchXMLTools::GetNewLabeledDataSequence();

    // set values at the new sequence
    Reference< chart2::data::XDataSequence > xSeq = SchXMLTools::CreateDataSequence( rRange, xChartDoc );
    Reference< beans::XPropertySet > xSeqProp( xSeq, uno::UNO_QUERY );
    if( xSeqProp.is())
        xSeqProp->setPropertyValue(u"Role"_ustr, uno::Any( rRole));
    xLabeledSeq->setValues( xSeq );

    // add new sequence to data series / push to front to have the correct sequence order if charttype is changed afterwards
    const Sequence< Reference< chart2::data::XLabeledDataSequence > > aOldSeq( xSeriesSource->getDataSequences());
    sal_Int32 nOldCount = aOldSeq.getLength();
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aNewSeq( nOldCount + 1 );
    auto pNewSeq = aNewSeq.getArray();
    pNewSeq[0].set(xLabeledSeq, uno::UNO_QUERY_THROW);
    std::copy(aOldSeq.begin(), aOldSeq.end(), std::next(pNewSeq));
    xSeriesSink->setData( aNewSeq );

    return xLabeledSeq;
}

XMLPropStyleContext* lcl_GetStylePropContext(
                        const SvXMLStylesContext* pStylesCtxt,
                        const SvXMLStyleContext*& rpStyle,
                        OUString const & rStyleName )
{
    rpStyle = pStylesCtxt->FindStyleChildContext( SchXMLImportHelper::GetChartFamilyID(), rStyleName );
    XMLPropStyleContext* pPropStyleContext =
                    const_cast< XMLPropStyleContext* >(dynamic_cast< const XMLPropStyleContext* >( rpStyle ));
    return pPropStyleContext;
}

} // anonymous namespace

SchXMLSeries2Context::SchXMLSeries2Context(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    const Reference< chart2::XChartDocument > & xNewDoc,
    std::vector< SchXMLAxis >& rAxes,
    ::std::vector< DataRowPointStyle >& rStyleVector,
    ::std::vector< RegressionStyle >& rRegressionStyleVector,
    sal_Int32 nSeriesIndex,
    bool bStockHasVolume,
    GlobalSeriesImportInfo& rGlobalSeriesImportInfo,
    const OUString & aGlobalChartTypeName,
    tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
    bool& rGlobalChartTypeUsedBySeries,
    const awt::Size & rChartSize ) :
        SvXMLImportContext( rImport ),
        mrImportHelper( rImpHelper ),
        mxNewDoc( xNewDoc ),
        mrAxes( rAxes ),
        mrStyleVector( rStyleVector ),
        mrRegressionStyleVector( rRegressionStyleVector ),
        mnSeriesIndex( nSeriesIndex ),
        mnDataPointIndex( 0 ),
        m_bStockHasVolume( bStockHasVolume ),
        m_rGlobalSeriesImportInfo(rGlobalSeriesImportInfo),
        mpAttachedAxis( nullptr ),
        mnAttachedAxis( 0 ),
        maGlobalChartTypeName( aGlobalChartTypeName ),
        maSeriesChartTypeName( aGlobalChartTypeName ),
        m_bHasDomainContext(false),
        mrLSequencesPerIndex( rLSequencesPerIndex ),
        mrGlobalChartTypeUsedBySeries( rGlobalChartTypeUsedBySeries ),
        mbSymbolSizeIsMissingInFile(false),
        maChartSize( rChartSize ),
        // A series manages the DataRowPointStyle-struct of a data-label child element.
        mDataLabel(DataRowPointStyle::DATA_LABEL_SERIES, OUString{})
{
    if( aGlobalChartTypeName == "com.sun.star.chart2.DonutChartType" )
    {
        maSeriesChartTypeName = "com.sun.star.chart2.PieChartType";
        maGlobalChartTypeName = maSeriesChartTypeName;
    }
}

SchXMLSeries2Context::~SchXMLSeries2Context()
{
    SAL_WARN_IF( !maPostponedSequences.empty(), "xmloff.chart", "maPostponedSequences is NULL");
}

void SchXMLSeries2Context::startFastElement (sal_Int32 /*Element*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // parse attributes
    mnAttachedAxis = 1;

    bool bHasRange = false;
    OUString aSeriesLabelRange;
    OUString aSeriesLabelString;
    bool bHideLegend = false;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        OUString aValue = aIter.toString();
        switch(aIter.getToken())
        {
            case XML_ELEMENT(CHART, XML_VALUES_CELL_RANGE_ADDRESS):
                m_aSeriesRange = aValue;
                bHasRange = true;
                break;
            case XML_ELEMENT(CHART, XML_LABEL_CELL_ADDRESS):
                aSeriesLabelRange = aValue;
                break;
            case XML_ELEMENT(LO_EXT, XML_LABEL_STRING):
                aSeriesLabelString = aValue;
                break;
            case XML_ELEMENT(CHART, XML_ATTACHED_AXIS):
                {
                    sal_Int32 nNumOfAxes = mrAxes.size();
                    for( sal_Int32 nCurrent = 0; nCurrent < nNumOfAxes; nCurrent++ )
                    {
                        if( aValue == mrAxes[ nCurrent ].aName &&
                            mrAxes[ nCurrent ].eDimension == SCH_XML_AXIS_Y )
                        {
                            mpAttachedAxis = &( mrAxes[ nCurrent ] );
                        }
                    }
                }
                break;
            case XML_ELEMENT(CHART, XML_STYLE_NAME):
                msAutoStyleName = aValue;
                break;
            case XML_ELEMENT(CHART, XML_CLASS):
                {
                    OUString aClassName;
                    sal_uInt16 nClassPrefix =
                        GetImport().GetNamespaceMap().GetKeyByAttrValueQName(
                            aValue, &aClassName );
                    if( XML_NAMESPACE_CHART == nClassPrefix )
                        maSeriesChartTypeName = SchXMLTools::GetChartTypeByClassName( aClassName, false /* bUseOldNames */ );

                    if( maSeriesChartTypeName.isEmpty())
                        maSeriesChartTypeName = aClassName;
                }
                break;
            case XML_ELEMENT(LO_EXT, XML_HIDE_LEGEND):
                bHideLegend = aValue.toBoolean();
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if( mpAttachedAxis )
    {
        if( mpAttachedAxis->nAxisIndex > 0 )
        {
            // secondary axis => property has to be set (primary is default)
            mnAttachedAxis = 2;
        }
    }

    try
    {
        SAL_WARN_IF( !mxNewDoc.is(), "xmloff.chart", "mxNewDoc is NULL");
        if( m_rGlobalSeriesImportInfo.rbAllRangeAddressesAvailable && ! bHasRange )
            m_rGlobalSeriesImportInfo.rbAllRangeAddressesAvailable = false;

        bool bIsCandleStick = maGlobalChartTypeName == "com.sun.star.chart2.CandleStickChartType";
        if( !maSeriesChartTypeName.isEmpty() )
        {
            bIsCandleStick = maSeriesChartTypeName == "com.sun.star.chart2.CandleStickChartType";
        }
        else
        {
            if( bIsCandleStick
                && m_bStockHasVolume
                && mnSeriesIndex == 0 )
            {
                maSeriesChartTypeName = "com.sun.star.chart2.ColumnChartType";
                bIsCandleStick = false;
            }
            else
            {
                maSeriesChartTypeName = maGlobalChartTypeName;
            }
        }
        if( ! mrGlobalChartTypeUsedBySeries )
            mrGlobalChartTypeUsedBySeries = (maSeriesChartTypeName == maGlobalChartTypeName);
        sal_Int32 const nCoordinateSystemIndex = 0;//so far we can only import one coordinate system
        m_xSeries.set(
            SchXMLImportHelper::GetNewDataSeries( mxNewDoc, nCoordinateSystemIndex, maSeriesChartTypeName, ! mrGlobalChartTypeUsedBySeries ));
        Reference< chart2::data::XLabeledDataSequence > xLabeledSeq( SchXMLTools::GetNewLabeledDataSequence(), uno::UNO_QUERY_THROW );

        Reference< beans::XPropertySet > xSeriesProp( m_xSeries, uno::UNO_QUERY );
        if (xSeriesProp.is())
        {
            if (bHideLegend)
                xSeriesProp->setPropertyValue(u"ShowLegendEntry"_ustr, uno::Any(false));

            if( bIsCandleStick )
            {
                // set default color for range-line to black (before applying styles)
                xSeriesProp->setPropertyValue(u"Color"_ustr,
                        uno::Any( sal_Int32( 0x000000 ))); // black
            }
            else if ( maSeriesChartTypeName == "com.sun.star.chart2.PieChartType" )
            {
                //@todo: this property should be saved
                xSeriesProp->setPropertyValue(u"VaryColorsByPoint"_ustr,
                        uno::Any( true ));
            }

        }

        Reference<chart2::data::XDataProvider> xDataProvider(mxNewDoc->getDataProvider());
        Reference<chart2::data::XPivotTableDataProvider> xPivotTableDataProvider(xDataProvider, uno::UNO_QUERY);

        Reference<chart2::data::XDataSequence> xSequenceValues;

        // values
        if (xPivotTableDataProvider.is()) // is pivot chart
        {
            xSequenceValues.set(xPivotTableDataProvider->createDataSequenceOfValuesByIndex(mnSeriesIndex));
        }
        else
        {
            if (bHasRange && !m_aSeriesRange.isEmpty())
                xSequenceValues = SchXMLTools::CreateDataSequence(m_aSeriesRange, mxNewDoc);
        }

        Reference<beans::XPropertySet> xSeqProp(xSequenceValues, uno::UNO_QUERY);
        if (xSeqProp.is())
        {
            OUString aMainRole(u"values-y"_ustr);
            if (maSeriesChartTypeName == "com.sun.star.chart2.BubbleChartType")
                aMainRole = "values-size";
            xSeqProp->setPropertyValue(u"Role"_ustr, uno::Any(aMainRole));
        }
        xLabeledSeq->setValues(xSequenceValues);

        // register for setting local data if external data provider is not present
        maPostponedSequences.emplace(
                tSchXMLIndexWithPart( m_rGlobalSeriesImportInfo.nCurrentDataIndex, SCH_XML_PART_VALUES ), xLabeledSeq );

        // label
        Reference<chart2::data::XDataSequence> xSequenceLabel;

        if (xPivotTableDataProvider.is())
        {
            xSequenceLabel.set(xPivotTableDataProvider->createDataSequenceOfLabelsByIndex(mnSeriesIndex));
        }
        else
        {
            if (!aSeriesLabelRange.isEmpty())
            {
                xSequenceLabel.set(SchXMLTools::CreateDataSequence(aSeriesLabelRange, mxNewDoc));
            }
            else if (!aSeriesLabelString.isEmpty())
            {
                xSequenceLabel.set(SchXMLTools::CreateDataSequenceWithoutConvert(aSeriesLabelString, mxNewDoc));
            }
        }

        //Labels should always include hidden cells
        Reference<beans::XPropertySet> xSeqLabelProp(xSequenceLabel, uno::UNO_QUERY);
        if (xSeqLabelProp.is() && xSeqLabelProp->getPropertySetInfo()->hasPropertyByName(u"IncludeHiddenCells"_ustr))
        {
            xSeqLabelProp->setPropertyValue( u"IncludeHiddenCells"_ustr, uno::Any(true));
        }

        xLabeledSeq->setLabel(xSequenceLabel);

        // Note: Even if we have no label, we have to register the label
        // for creation, because internal data always has labels. If
        // they don't exist in the original, auto-generated labels are
        // used for the internal data.
        maPostponedSequences.emplace(
                tSchXMLIndexWithPart( m_rGlobalSeriesImportInfo.nCurrentDataIndex, SCH_XML_PART_LABEL ), xLabeledSeq );

        Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeq( &xLabeledSeq, 1 );
        Reference< chart2::data::XDataSink > xSink( m_xSeries, uno::UNO_QUERY_THROW );
        xSink->setData( aSeq );
    }
    catch( const uno::Exception &)
    {
        DBG_UNHANDLED_EXCEPTION("xmloff.chart");
    }

    //init mbSymbolSizeIsMissingInFile:
    try
    {
        if( !msAutoStyleName.isEmpty() )
        {
            const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
            if( pStylesCtxt )
            {
                const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                    SchXMLImportHelper::GetChartFamilyID(), msAutoStyleName );

                const XMLPropStyleContext* pPropStyleContext = dynamic_cast< const XMLPropStyleContext * >( pStyle );

                uno::Any aASymbolSize( SchXMLTools::getPropertyFromContext( u"SymbolSize"
                    , pPropStyleContext, pStylesCtxt ) );
                mbSymbolSizeIsMissingInFile = !aASymbolSize.hasValue();
            }
        }
    }
    catch( const uno::Exception & )
    {
    }
}

namespace {

struct DomainInfo
{
    DomainInfo( OUString _aRole, OUString _aRange, sal_Int32 nIndex )
        : aRole(std::move(_aRole)), aRange(std::move(_aRange)), nIndexForLocalData(nIndex)
    {}

    OUString aRole;
    OUString aRange;
    sal_Int32 nIndexForLocalData;
};

}

void SchXMLSeries2Context::endFastElement(sal_Int32 )
{
    // special handling for different chart types.  This is necessary as the
    // roles are not yet saved in the file format
    sal_Int32 nDomainCount = maDomainAddresses.size();
    bool bIsScatterChart = maSeriesChartTypeName == "com.sun.star.chart2.ScatterChartType";
    bool bIsBubbleChart = maSeriesChartTypeName == "com.sun.star.chart2.BubbleChartType";
    bool bDeleteSeries = false;
    std::vector< DomainInfo > aDomainInfos;

    //different handling for different chart types necessary
    if( bIsScatterChart || ( nDomainCount==1 && !bIsBubbleChart ) )
    {
        DomainInfo aDomainInfo( u"values-x"_ustr, m_rGlobalSeriesImportInfo.aFirstFirstDomainAddress, m_rGlobalSeriesImportInfo.nFirstFirstDomainIndex ) ;
        bool bCreateXValues = true;
        if( !maDomainAddresses.empty() )
        {
            if( m_rGlobalSeriesImportInfo.aFirstFirstDomainAddress.isEmpty() )
            {
                m_rGlobalSeriesImportInfo.aFirstFirstDomainAddress = maDomainAddresses.front();
                m_rGlobalSeriesImportInfo.nFirstFirstDomainIndex = m_rGlobalSeriesImportInfo.nCurrentDataIndex;
            }
            aDomainInfo.aRange = maDomainAddresses.front();
            aDomainInfo.nIndexForLocalData = m_rGlobalSeriesImportInfo.nCurrentDataIndex;
            m_rGlobalSeriesImportInfo.nCurrentDataIndex++;
        }
        else if( m_rGlobalSeriesImportInfo.aFirstFirstDomainAddress.isEmpty() && !m_bHasDomainContext && mnSeriesIndex==0 )
        {
            if( SchXMLTools::isDocumentGeneratedWithOpenOfficeOlderThan2_3( GetImport().GetModel() ) ) //wrong old chart files:
            {
                //for xy charts the first series needs to have a domain
                //if this by error iss not the case the first series is taken s x values
                //needed for wrong files created while having an addin (e.g. BoxPlot)
                m_rGlobalSeriesImportInfo.aFirstFirstDomainAddress = m_aSeriesRange;
                m_rGlobalSeriesImportInfo.nFirstFirstDomainIndex = m_rGlobalSeriesImportInfo.nCurrentDataIndex++;
                bDeleteSeries = true;
                bCreateXValues = false;//they will be created for the next series
            }
        }
        if( bCreateXValues )
            aDomainInfos.push_back( aDomainInfo );
    }
    else if( bIsBubbleChart )
    {
        if( nDomainCount>1 )
        {
            DomainInfo aDomainInfo( u"values-x"_ustr, maDomainAddresses[1], m_rGlobalSeriesImportInfo.nCurrentDataIndex ) ;
            if( m_rGlobalSeriesImportInfo.aFirstSecondDomainAddress.isEmpty() )
            {
                //for bubble chart the second domain contains the x values which should become an index smaller than y values for own data table
                //->so second first
                m_rGlobalSeriesImportInfo.aFirstSecondDomainAddress = maDomainAddresses[1];
                m_rGlobalSeriesImportInfo.nFirstSecondDomainIndex = m_rGlobalSeriesImportInfo.nCurrentDataIndex;
            }
            aDomainInfos.push_back( aDomainInfo );
            m_rGlobalSeriesImportInfo.nCurrentDataIndex++;
        }
        else if( !m_rGlobalSeriesImportInfo.aFirstSecondDomainAddress.isEmpty() )
        {
            DomainInfo aDomainInfo( u"values-x"_ustr, m_rGlobalSeriesImportInfo.aFirstSecondDomainAddress, m_rGlobalSeriesImportInfo.nFirstSecondDomainIndex ) ;
            aDomainInfos.push_back( aDomainInfo );
        }
        if( nDomainCount>0)
        {
            DomainInfo aDomainInfo( u"values-y"_ustr, maDomainAddresses.front(), m_rGlobalSeriesImportInfo.nCurrentDataIndex ) ;
            if( m_rGlobalSeriesImportInfo.aFirstFirstDomainAddress.isEmpty() )
            {
                m_rGlobalSeriesImportInfo.aFirstFirstDomainAddress = maDomainAddresses.front();
                m_rGlobalSeriesImportInfo.nFirstFirstDomainIndex = m_rGlobalSeriesImportInfo.nCurrentDataIndex;
            }
            aDomainInfos.push_back( aDomainInfo );
            m_rGlobalSeriesImportInfo.nCurrentDataIndex++;
        }
        else if( !m_rGlobalSeriesImportInfo.aFirstFirstDomainAddress.isEmpty() )
        {
            DomainInfo aDomainInfo( u"values-y"_ustr, m_rGlobalSeriesImportInfo.aFirstFirstDomainAddress, m_rGlobalSeriesImportInfo.nFirstFirstDomainIndex ) ;
            aDomainInfos.push_back( aDomainInfo );
        }
    }

    if( bDeleteSeries )
    {
        //delete created series
        SchXMLImportHelper::DeleteDataSeries(
            m_xSeries, Reference< chart2::XChartDocument >( GetImport().GetModel(), uno::UNO_QUERY ) );
    }
    else
    {
        //add style
        if( !msAutoStyleName.isEmpty() || mnAttachedAxis != 1 )
        {
            DataRowPointStyle aStyle(
                DataRowPointStyle::DATA_SERIES,
                m_xSeries,
                -1, 1,
                msAutoStyleName, mnAttachedAxis );
            aStyle.mbSymbolSizeForSeriesIsMissingInFile=mbSymbolSizeIsMissingInFile;
            mrStyleVector.push_back( aStyle );
        }
        // And styles for a data-label child element too. In contrast to data-labels as child of data points,
        // an information about absolute position is useless here. We need only style information.
        if (!mDataLabel.msStyleName.isEmpty())
        {
            mDataLabel.msStyleNameOfParent = msAutoStyleName;
            mDataLabel.m_xSeries = m_xSeries;
            mDataLabel.mnAttachedAxis = mnAttachedAxis; // not needed, but be consistent with its parent
            mrStyleVector.push_back(mDataLabel);
        }
    }

    for( std::vector< DomainInfo >::reverse_iterator aIt( aDomainInfos.rbegin() ); aIt!= aDomainInfos.rend(); ++aIt )
    {
        DomainInfo aDomainInfo( *aIt );
        Reference< chart2::data::XLabeledDataSequence2 > xLabeledSeq =
            lcl_createAndAddSequenceToSeries( aDomainInfo.aRole, aDomainInfo.aRange, mxNewDoc, m_xSeries );
        if( xLabeledSeq.is() )
        {
            // register for setting local data if external data provider is not present
            mrLSequencesPerIndex.emplace(
                    tSchXMLIndexWithPart( aDomainInfo.nIndexForLocalData, SCH_XML_PART_VALUES ),
                    Reference< chart2::data::XLabeledDataSequence >(xLabeledSeq, uno::UNO_QUERY_THROW) );
        }
    }

    if( !bDeleteSeries )
    {
        for (auto const& postponedSequence : maPostponedSequences)
        {
            sal_Int32 nNewIndex = postponedSequence.first.first + nDomainCount;
            mrLSequencesPerIndex.emplace( tSchXMLIndexWithPart( nNewIndex, postponedSequence.first.second ), postponedSequence.second );
        }
        m_rGlobalSeriesImportInfo.nCurrentDataIndex++;
    }
    maPostponedSequences.clear();
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLSeries2Context::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    SvXMLImportContext* pContext = nullptr;

    switch(nElement)
    {
        case XML_ELEMENT(CHART, XML_DOMAIN):
            if( m_xSeries.is())
            {
                m_bHasDomainContext = true;
                pContext = new SchXMLDomain2Context(
                    GetImport(), maDomainAddresses );
            }
            break;

        case XML_ELEMENT(CHART, XML_MEAN_VALUE):
            pContext = new SchXMLStatisticsObjectContext(
                mrImportHelper, GetImport(),
                msAutoStyleName,
                mrStyleVector, m_xSeries,
                SchXMLStatisticsObjectContext::CONTEXT_TYPE_MEAN_VALUE_LINE,
                mrLSequencesPerIndex );
            break;
        case XML_ELEMENT(CHART, XML_REGRESSION_CURVE):
            pContext = new SchXMLRegressionCurveObjectContext(
                mrImportHelper, GetImport(),
                mrRegressionStyleVector,
                m_xSeries, maChartSize );
            break;
        case XML_ELEMENT(CHART, XML_ERROR_INDICATOR):
            pContext = new SchXMLStatisticsObjectContext(
                mrImportHelper, GetImport(),
                msAutoStyleName,
                mrStyleVector, m_xSeries,
                SchXMLStatisticsObjectContext::CONTEXT_TYPE_ERROR_INDICATOR,
                mrLSequencesPerIndex );
            break;

        case XML_ELEMENT(CHART, XML_DATA_POINT):
            pContext = new SchXMLDataPointContext( GetImport(),
                                                   mrStyleVector, m_xSeries, mnDataPointIndex, mbSymbolSizeIsMissingInFile );
            break;
        case XML_ELEMENT(CHART, XML_DATA_LABEL):
            // CustomLabels are useless for a data label element as child of a series, because it serves as default
            // for all data labels. But the ctor expects it, so use that of the mDataLabel struct as ersatz.
            pContext = new SchXMLDataLabelContext(GetImport(), mDataLabel.mCustomLabels,
                                                  mDataLabel);
            break;

        case XML_ELEMENT(LO_EXT, XML_PROPERTY_MAPPING):
            pContext = new SchXMLPropertyMappingContext(
                    GetImport(),
                    mrLSequencesPerIndex, m_xSeries );
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    }

    return pContext;
}

//static
void SchXMLSeries2Context::initSeriesPropertySets( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const uno::Reference< frame::XModel >& xChartModel )
{
    // iterate over series first and remind propertysets in map
    // new api <-> old api wrapper
    ::std::map< Reference< chart2::XDataSeries >, Reference< beans::XPropertySet > > aSeriesMap;
    for (auto & seriesStyle : rSeriesDefaultsAndStyles.maSeriesStyleVector)
    {
        if( seriesStyle.meType != DataRowPointStyle::DATA_SERIES )
            continue;

        if( !seriesStyle.m_xOldAPISeries.is() )
            seriesStyle.m_xOldAPISeries = SchXMLSeriesHelper::createOldAPISeriesPropertySet( seriesStyle.m_xSeries, xChartModel );

        aSeriesMap[seriesStyle.m_xSeries] = seriesStyle.m_xOldAPISeries;

    }

    //initialize m_xOldAPISeries for all other styles also
    for (auto & seriesStyle : rSeriesDefaultsAndStyles.maSeriesStyleVector)
    {
        if( seriesStyle.meType == DataRowPointStyle::DATA_SERIES )
            continue;
        seriesStyle.m_xOldAPISeries = aSeriesMap[seriesStyle.m_xSeries];
    }
}

//static
void SchXMLSeries2Context::setDefaultsToSeries( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles )
{
    // iterate over series
    // call initSeriesPropertySets first

    for (const auto & seriesStyle : rSeriesDefaultsAndStyles.maSeriesStyleVector)
    {
        if( seriesStyle.meType != DataRowPointStyle::DATA_SERIES )
            continue;

        try
        {
            uno::Reference< beans::XPropertySet > xSeries( seriesStyle.m_xOldAPISeries );
            if( !xSeries.is() )
                continue;

            if( rSeriesDefaultsAndStyles.maSymbolTypeDefault.hasValue() )
                xSeries->setPropertyValue(u"SymbolType"_ustr,rSeriesDefaultsAndStyles.maSymbolTypeDefault);
            if( rSeriesDefaultsAndStyles.maDataCaptionDefault.hasValue() )
                xSeries->setPropertyValue(u"DataCaption"_ustr,rSeriesDefaultsAndStyles.maDataCaptionDefault);

            if( rSeriesDefaultsAndStyles.maErrorIndicatorDefault.hasValue() )
                xSeries->setPropertyValue(u"ErrorIndicator"_ustr,rSeriesDefaultsAndStyles.maErrorIndicatorDefault);
            if( rSeriesDefaultsAndStyles.maErrorCategoryDefault.hasValue() )
                xSeries->setPropertyValue(u"ErrorCategory"_ustr,rSeriesDefaultsAndStyles.maErrorCategoryDefault);
            if( rSeriesDefaultsAndStyles.maConstantErrorLowDefault.hasValue() )
                xSeries->setPropertyValue(u"ConstantErrorLow"_ustr,rSeriesDefaultsAndStyles.maConstantErrorLowDefault);
            if( rSeriesDefaultsAndStyles.maConstantErrorHighDefault.hasValue() )
                xSeries->setPropertyValue(u"ConstantErrorHigh"_ustr,rSeriesDefaultsAndStyles.maConstantErrorHighDefault);
            if( rSeriesDefaultsAndStyles.maPercentageErrorDefault.hasValue() )
                xSeries->setPropertyValue(u"PercentageError"_ustr,rSeriesDefaultsAndStyles.maPercentageErrorDefault);
            if( rSeriesDefaultsAndStyles.maErrorMarginDefault.hasValue() )
                xSeries->setPropertyValue(u"ErrorMargin"_ustr,rSeriesDefaultsAndStyles.maErrorMarginDefault);

            if( rSeriesDefaultsAndStyles.maMeanValueDefault.hasValue() )
                xSeries->setPropertyValue(u"MeanValue"_ustr,rSeriesDefaultsAndStyles.maMeanValueDefault);
            if( rSeriesDefaultsAndStyles.maRegressionCurvesDefault.hasValue() )
                xSeries->setPropertyValue(u"RegressionCurves"_ustr,rSeriesDefaultsAndStyles.maRegressionCurvesDefault);
        }
        catch( uno::Exception &  )
        {
            //end of series reached
        }
    }
}

// ODF has the line and fill properties in a <style:style> element, which is referenced by the
// <chart:data-label> element. But LibreOffice has them as special label properties of the series
// or point respectively. The following array maps the API name of the ODF property to the name of
// the internal property. Those are of kind "LabelFoo".
// The array is used in methods setStylesToSeries and setStylesToDataPoints.
const std::pair<OUString, OUString> aApiToLabelFooPairs[]
    = { { "LineStyle", "LabelBorderStyle" },
        { "LineWidth", "LabelBorderWidth" },
        { "LineColor", "LabelBorderColor" },
        // The name "LabelBorderDash" is defined, but the associated API name "LineDash" belongs to
        // the <draw:stroke-dash> element and is not used directly as line property.
        //{"LineDash", "LabelBorderDash"},
        { "LineDashName", "LabelBorderDashName" },
        { "LineTransparence", "LabelBorderTransparency" },
        { "FillStyle", "LabelFillStyle" },
        { "FillBackground", "LabelFillBackground" },
        { "FillHatchName", "LabelFillHatchName" },
        { "FillColor", "LabelFillColor" } };


//static
void SchXMLSeries2Context::setStylesToSeries( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , OUString& rCurrStyleName
        , const SchXMLImportHelper& rImportHelper
        , const SvXMLImport& rImport
        , bool bIsStockChart
        , tSchXMLLSequencesPerIndex & rInOutLSequencesPerIndex )
{
    // iterate over series
    for (const auto & seriesStyle : rSeriesDefaultsAndStyles.maSeriesStyleVector)
    {
        if (seriesStyle.meType != DataRowPointStyle::DATA_SERIES)
            continue;
        try
        {
            uno::Reference< beans::XPropertySet > xSeriesProp( seriesStyle.m_xOldAPISeries );
            if( !xSeriesProp.is() )
                continue;

            if( seriesStyle.mnAttachedAxis != 1 )
            {
                xSeriesProp->setPropertyValue(u"Axis"_ustr
                    , uno::Any(chart::ChartAxisAssign::SECONDARY_Y) );
            }

            if( seriesStyle.msStyleName.isEmpty())
                continue;

            if( rCurrStyleName != seriesStyle.msStyleName )
            {
                rCurrStyleName = seriesStyle.msStyleName;
                rpStyle = pStylesCtxt->FindStyleChildContext(
                SchXMLImportHelper::GetChartFamilyID(), rCurrStyleName );
            }

            //set style to series
            // note: SvXMLStyleContext::FillPropertySet is not const
            XMLPropStyleContext * pPropStyleContext =
                const_cast< XMLPropStyleContext * >(
                    dynamic_cast< const XMLPropStyleContext * >( rpStyle ));

            if (!pPropStyleContext)
                continue;

            // error bar style must be set before the other error
            // bar properties (which may be alphabetically before
            // this property)
            bool bHasErrorBarRangesFromData = false;
            {
                static constexpr OUString aErrorBarStylePropName( u"ErrorBarStyle"_ustr);
                uno::Any aErrorBarStyle(
                    SchXMLTools::getPropertyFromContext( aErrorBarStylePropName, pPropStyleContext, pStylesCtxt ));
                if( aErrorBarStyle.hasValue())
                {
                    xSeriesProp->setPropertyValue( aErrorBarStylePropName, aErrorBarStyle );
                    sal_Int32 eEBStyle = chart::ErrorBarStyle::NONE;
                    bHasErrorBarRangesFromData =
                        ( ( aErrorBarStyle >>= eEBStyle ) &&
                         eEBStyle == chart::ErrorBarStyle::FROM_DATA );
                }
            }

            //don't set the style to the min max line series of a stock chart
            //otherwise the min max line properties gets overwritten and the series becomes invisible typically
            if (bIsStockChart)
            {
                if (SchXMLSeriesHelper::isCandleStickSeries(
                        seriesStyle.m_xSeries,
                        rImportHelper.GetChartDocument()))
                    continue;
            }

            // Has the series a data-label child element?
            auto pItLabel
                = std::find_if(rSeriesDefaultsAndStyles.maSeriesStyleVector.begin(),
                               rSeriesDefaultsAndStyles.maSeriesStyleVector.end(),
                               [&seriesStyle](const DataRowPointStyle& rStyle) {
                                   return rStyle.meType == DataRowPointStyle::DATA_LABEL_SERIES
                                          && rStyle.msStyleNameOfParent == seriesStyle.msStyleName;
                               });
            if (pItLabel != rSeriesDefaultsAndStyles.maSeriesStyleVector.end())
            {
                // Bring the information from the data-label to the series
                const SvXMLStyleContext* pLabelStyleContext(pStylesCtxt->FindStyleChildContext(
                    SchXMLImportHelper::GetChartFamilyID(), (*pItLabel).msStyleName));
                // note: SvXMLStyleContext::FillPropertySet is not const
                XMLPropStyleContext* pLabelPropStyleContext = const_cast<XMLPropStyleContext*>(
                    dynamic_cast<const XMLPropStyleContext*>(pLabelStyleContext));
                if (pLabelPropStyleContext)
                {
                    // Test each to be mapped property whether the data-label has a value for it.
                    // If found, set it at series.
                    uno::Reference<beans::XPropertySetInfo> xSeriesPropInfo(
                        xSeriesProp->getPropertySetInfo());
                    for (const auto& rPropPair : aApiToLabelFooPairs)
                    {
                        uno::Any aPropValue(SchXMLTools::getPropertyFromContext(
                            rPropPair.first, pLabelPropStyleContext, pStylesCtxt));
                        if (aPropValue.hasValue()
                            && xSeriesPropInfo->hasPropertyByName(rPropPair.second))
                            xSeriesProp->setPropertyValue(rPropPair.second, aPropValue);
                    }
                }
            }

            pPropStyleContext->FillPropertySet( xSeriesProp );
            if( seriesStyle.mbSymbolSizeForSeriesIsMissingInFile )
                lcl_setSymbolSizeIfNeeded( xSeriesProp, rImport );
            if( bHasErrorBarRangesFromData )
                lcl_insertErrorBarLSequencesToMap( rInOutLSequencesPerIndex, xSeriesProp );

        }
        catch( const uno::Exception & )
        {
                TOOLS_INFO_EXCEPTION("xmloff.chart", "Exception caught during setting styles to series" );
        }
    }
}

// static
void SchXMLSeries2Context::setStylesToRegressionCurves(
                                SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles,
                                const SvXMLStylesContext* pStylesCtxt,
                                const SvXMLStyleContext*& rpStyle,
                                OUString const & rCurrentStyleName )
{
    // iterate over regression etc
    for (auto const& regressionStyle : rSeriesDefaultsAndStyles.maRegressionStyleVector)
    {
        try
        {
            OUString aServiceName;
            XMLPropStyleContext* pPropStyleContext = nullptr;

            if (!rCurrentStyleName.isEmpty())
            {
                XMLPropStyleContext* pCurrent = lcl_GetStylePropContext(pStylesCtxt, rpStyle, rCurrentStyleName);
                if( pCurrent )
                {
                    pPropStyleContext = pCurrent;
                    uno::Any aAny = SchXMLTools::getPropertyFromContext(u"RegressionType", pPropStyleContext, pStylesCtxt);
                    if ( aAny.hasValue() )
                    {
                        aAny >>= aServiceName;
                    }
                }
            }

            if (!regressionStyle.msStyleName.isEmpty())
            {
                XMLPropStyleContext* pCurrent = lcl_GetStylePropContext(pStylesCtxt, rpStyle, regressionStyle.msStyleName);
                if( pCurrent )
                {
                    pPropStyleContext = pCurrent;
                    uno::Any aAny = SchXMLTools::getPropertyFromContext(u"RegressionType", pPropStyleContext, pStylesCtxt);
                    if ( aAny.hasValue() )
                    {
                        aAny >>= aServiceName;
                    }
                }
            }

            if( !aServiceName.isEmpty() )
            {
                Reference< lang::XMultiServiceFactory > xMSF = comphelper::getProcessServiceFactory();
                Reference< chart2::XRegressionCurve > xRegCurve( xMSF->createInstance( aServiceName ), uno::UNO_QUERY_THROW );
                Reference< chart2::XRegressionCurveContainer > xRegCurveCont( regressionStyle.m_xSeries, uno::UNO_QUERY_THROW );

                Reference< beans::XPropertySet > xCurveProperties( xRegCurve, uno::UNO_QUERY );
                if( pPropStyleContext != nullptr)
                    pPropStyleContext->FillPropertySet( xCurveProperties );

                xRegCurve->setEquationProperties( regressionStyle.m_xEquationProperties );

                xRegCurveCont->addRegressionCurve( xRegCurve );
            }
        }
        catch( const uno::Exception& )
        {
            TOOLS_INFO_EXCEPTION("xmloff.chart", "Exception caught during setting styles to series" );
        }

    }
}

// static
void SchXMLSeries2Context::setStylesToStatisticsObjects( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , OUString& rCurrStyleName )
{
    // iterate over regression etc
    for (auto const& seriesStyle : rSeriesDefaultsAndStyles.maSeriesStyleVector)
    {
        if( seriesStyle.meType == DataRowPointStyle::ERROR_INDICATOR ||
            seriesStyle.meType == DataRowPointStyle::MEAN_VALUE )
        {
            if ( seriesStyle.meType == DataRowPointStyle::ERROR_INDICATOR )
            {
                uno::Reference< beans::XPropertySet > xNewSeriesProp(seriesStyle.m_xSeries,uno::UNO_QUERY);

                if (seriesStyle.m_xErrorXProperties.is())
                    xNewSeriesProp->setPropertyValue(u"ErrorBarX"_ustr,uno::Any(seriesStyle.m_xErrorXProperties));

                if (seriesStyle.m_xErrorYProperties.is())
                    xNewSeriesProp->setPropertyValue(u"ErrorBarY"_ustr,uno::Any(seriesStyle.m_xErrorYProperties));
            }

            try
            {
                uno::Reference< beans::XPropertySet > xSeriesProp( seriesStyle.m_xOldAPISeries );
                if( !xSeriesProp.is() )
                    continue;

                if( !seriesStyle.msStyleName.isEmpty())
                {
                    if( rCurrStyleName != seriesStyle.msStyleName )
                    {
                        rCurrStyleName = seriesStyle.msStyleName;
                        rpStyle = pStylesCtxt->FindStyleChildContext(
                            SchXMLImportHelper::GetChartFamilyID(), rCurrStyleName );
                    }

                    // note: SvXMLStyleContext::FillPropertySet is not const
                    XMLPropStyleContext * pPropStyleContext =
                        const_cast< XMLPropStyleContext * >(
                            dynamic_cast< const XMLPropStyleContext * >( rpStyle ));
                    if( pPropStyleContext )
                    {
                        Reference< beans::XPropertySet > xStatPropSet;
                        switch( seriesStyle.meType )
                        {
                            case DataRowPointStyle::MEAN_VALUE:
                                xSeriesProp->getPropertyValue(u"DataMeanValueProperties"_ustr) >>= xStatPropSet;
                                break;
                            case DataRowPointStyle::ERROR_INDICATOR:
                                xSeriesProp->getPropertyValue(u"DataErrorProperties"_ustr)  >>= xStatPropSet;
                                break;
                            default:
                                break;
                        }
                        if( xStatPropSet.is())
                            pPropStyleContext->FillPropertySet( xStatPropSet );
                    }
                }
            }
            catch( const uno::Exception & )
            {
                TOOLS_INFO_EXCEPTION("xmloff.chart", "Exception caught during setting styles to series" );
            }
        }
    }
}

//static
void SchXMLSeries2Context::setStylesToDataPoints( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , OUString& rCurrStyleName
        , const SchXMLImportHelper& rImportHelper
        , const SvXMLImport& rImport
        , bool bIsStockChart, bool bIsDonutChart, bool bSwitchOffLinesForScatter )
{
    for (auto const& seriesStyle : rSeriesDefaultsAndStyles.maSeriesStyleVector)
    {
        if( seriesStyle.meType != DataRowPointStyle::DATA_POINT )
            continue;

        if( seriesStyle.m_nPointIndex == -1 )
            continue;

        uno::Reference< beans::XPropertySet > xSeriesProp( seriesStyle.m_xOldAPISeries );
        if(!xSeriesProp.is())
            continue;

        //ignore datapoint properties for stock charts
        //... todo ...
        if( bIsStockChart )
        {
            if( SchXMLSeriesHelper::isCandleStickSeries( seriesStyle.m_xSeries, rImportHelper.GetChartDocument() ) )
                continue;
        }

        // data point style
        for( sal_Int32 i = 0; i < seriesStyle.m_nPointRepeat; i++ )
        {
            try
            {
                uno::Reference< beans::XPropertySet > xPointProp(
                    SchXMLSeriesHelper::createOldAPIDataPointPropertySet( seriesStyle.m_xSeries, seriesStyle.m_nPointIndex + i
                        , rImportHelper.GetChartDocument() ) );

                if( !xPointProp.is() )
                    continue;

                if( bIsDonutChart )
                {
                    //set special series styles for donut charts first
                    if( rCurrStyleName != seriesStyle.msSeriesStyleNameForDonuts )
                    {
                        rCurrStyleName = seriesStyle.msSeriesStyleNameForDonuts;
                        rpStyle = pStylesCtxt->FindStyleChildContext(
                            SchXMLImportHelper::GetChartFamilyID(), rCurrStyleName );
                    }

                    // note: SvXMLStyleContext::FillPropertySet is not const
                    XMLPropStyleContext * pPropStyleContext =
                        const_cast< XMLPropStyleContext * >(
                            dynamic_cast< const XMLPropStyleContext * >( rpStyle ));
                    if( pPropStyleContext )
                        pPropStyleContext->FillPropertySet( xPointProp );
                }

                try
                {
                    //need to set this explicitly here for old files as the new api does not support this property fully anymore
                    if( bSwitchOffLinesForScatter )
                        xPointProp->setPropertyValue(u"Lines"_ustr,uno::Any(false));
                }
                catch( const uno::Exception & )
                {
                }

                if( rCurrStyleName != seriesStyle.msStyleName )
                {
                    rCurrStyleName = seriesStyle.msStyleName;
                    rpStyle = pStylesCtxt->FindStyleChildContext(
                        SchXMLImportHelper::GetChartFamilyID(), rCurrStyleName );
                }

                // note: SvXMLStyleContext::FillPropertySet is not const
                XMLPropStyleContext * pPropStyleContext =
                    const_cast< XMLPropStyleContext * >(
                        dynamic_cast< const XMLPropStyleContext * >( rpStyle ));
                if (pPropStyleContext)
                {
                    // Has the point a data-label child element?
                    auto pItLabel = std::find_if(
                        rSeriesDefaultsAndStyles.maSeriesStyleVector.begin(),
                        rSeriesDefaultsAndStyles.maSeriesStyleVector.end(),
                        [&seriesStyle](const DataRowPointStyle& rStyle) {
                            return rStyle.meType == DataRowPointStyle::DATA_LABEL_POINT
                                   && rStyle.msStyleNameOfParent == seriesStyle.msStyleName;
                        });
                    if (pItLabel != rSeriesDefaultsAndStyles.maSeriesStyleVector.end())
                    {
                        // Bring the information from the data-label to the point
                        const SvXMLStyleContext* pLabelStyleContext(
                            pStylesCtxt->FindStyleChildContext(
                                SchXMLImportHelper::GetChartFamilyID(), (*pItLabel).msStyleName));
                        // note: SvXMLStyleContext::FillPropertySet is not const
                        XMLPropStyleContext* pLabelPropStyleContext
                            = const_cast<XMLPropStyleContext*>(
                                dynamic_cast<const XMLPropStyleContext*>(pLabelStyleContext));
                        if (pLabelPropStyleContext)
                        {
                            // Test each to be mapped property whether the data-label has a value for it.
                            // If found, set it at the point.
                            uno::Reference<beans::XPropertySetInfo> xPointPropInfo(
                                xPointProp->getPropertySetInfo());
                            for (const auto& rPropPair : aApiToLabelFooPairs)
                            {
                                uno::Any aPropValue(SchXMLTools::getPropertyFromContext(
                                    rPropPair.first, pLabelPropStyleContext, pStylesCtxt));
                                if (aPropValue.hasValue()
                                    && xPointPropInfo->hasPropertyByName(rPropPair.second))
                                    xPointProp->setPropertyValue(rPropPair.second, aPropValue);
                            }
                        }
                    }

                    pPropStyleContext->FillPropertySet( xPointProp );
                    if( seriesStyle.mbSymbolSizeForSeriesIsMissingInFile )
                        lcl_resetSymbolSizeForPointsIfNecessary( xPointProp, rImport, pPropStyleContext, pStylesCtxt );
                    if( !pPropStyleContext->isEmptyDataStyleName() )
                        lcl_setLinkNumberFormatToSourceIfNeeded( xPointProp, pPropStyleContext, pStylesCtxt );
                }

                // Custom labels might be passed as property
                if(const size_t nLabelCount = seriesStyle.mCustomLabels.mLabels.size(); nLabelCount > 0)
                {
                    auto& rCustomLabels = seriesStyle.mCustomLabels;

                    Sequence< Reference<chart2::XDataPointCustomLabelField>> xLabels(nLabelCount);
                    auto pxLabels = xLabels.getArray();
                    Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
                    for( size_t j = 0; j < nLabelCount; ++j )
                    {
                        Reference< chart2::XDataPointCustomLabelField > xCustomLabel = chart2::DataPointCustomLabelField::create(xContext);
                        pxLabels[j] = xCustomLabel;
                        xCustomLabel->setString(rCustomLabels.mLabels[j]);
                        if ( j == 0 && rCustomLabels.mbDataLabelsRange)
                        {
                            xCustomLabel->setFieldType(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CELLRANGE);
                            xCustomLabel->setGuid(rCustomLabels.msLabelGuid);
                            xCustomLabel->setCellRange(rCustomLabels.msLabelsCellRange);
                            xCustomLabel->setDataLabelsRange(true);
                        }
                        else
                        {
                            xCustomLabel->setFieldType(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT);
                        }

                        // Restore character properties on the text span manually, till
                        // SchXMLExportHelper_Impl::exportCustomLabel() does not write the style.
                        uno::Reference<beans::XPropertySetInfo> xPointPropInfo
                            = xPointProp->getPropertySetInfo();
                        if (xPointPropInfo.is())
                        {
                            uno::Sequence<beans::Property> aProperties = xPointPropInfo->getProperties();
                            for (const auto& rProperty : aProperties)
                            {
                                if (!rProperty.Name.startsWith("Char")
                                    || rProperty.Name.startsWith("Chart"))
                                {
                                    continue;
                                }

                                xCustomLabel->setPropertyValue(
                                    rProperty.Name, xPointProp->getPropertyValue(rProperty.Name));
                            }
                        }
                    }

                    xPointProp->setPropertyValue(u"CustomLabelFields"_ustr, uno::Any(xLabels));
                    xPointProp->setPropertyValue(u"DataCaption"_ustr, uno::Any(chart::ChartDataCaption::CUSTOM));
                }

                if( seriesStyle.mCustomLabelPos[0] != 0.0 || seriesStyle.mCustomLabelPos[1] != 0.0 )
                {
                    chart2::RelativePosition aCustomlabelPosition;
                    aCustomlabelPosition.Primary = seriesStyle.mCustomLabelPos[0];
                    aCustomlabelPosition.Secondary = seriesStyle.mCustomLabelPos[1];
                    xPointProp->setPropertyValue(u"CustomLabelPosition"_ustr, uno::Any(aCustomlabelPosition));
                }
            }
            catch( const uno::Exception & )
            {
                TOOLS_INFO_EXCEPTION("xmloff.chart", "Exception caught during setting styles to data points" );
            }
        }
    }   // styles iterator
}

//static
void SchXMLSeries2Context::switchSeriesLinesOff( ::std::vector< DataRowPointStyle >& rSeriesStyleVector )
{
    // iterate over series
    for (auto const& seriesStyle : rSeriesStyleVector)
    {
        if( seriesStyle.meType != DataRowPointStyle::DATA_SERIES )
            continue;

        try
        {
            uno::Reference< beans::XPropertySet > xSeries( seriesStyle.m_xOldAPISeries );
            if( !xSeries.is() )
                continue;

            xSeries->setPropertyValue(u"Lines"_ustr,uno::Any(false));
        }
        catch( uno::Exception &  )
        {
            //end of series reached
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
