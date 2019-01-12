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
#include "PropertyMap.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XPivotTableDataProvider.hpp>

#include <com/sun/star/chart/ChartAxisAssign.hpp>
#include <com/sun/star/chart/ChartSymbolType.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/processfactory.hxx>

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/SchXMLSeriesHelper.hxx>
#include <SchXMLImport.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlprmap.hxx>
#include <tools/diagnose_ex.h>

#include <typeinfo>

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
                          sal_uInt16 nPrefix,
                          const OUString& rLocalName,
                          ::std::vector< OUString > & rAddresses );
    virtual void StartElement( const Reference< xml::sax::XAttributeList >& xAttrList ) override;
};

SchXMLDomain2Context::SchXMLDomain2Context(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    ::std::vector< OUString > & rAddresses ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrAddresses( rAddresses )
{
}

void SchXMLDomain2Context::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
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
            mrAddresses.push_back( xAttrList->getValueByIndex( i ));
        }
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
        if( xLegendProp.is() && (xLegendProp->getPropertyValue("Alignment") >>= aLegendPosition)
            && chart::ChartLegendPosition_NONE != aLegendPosition )
        {

            double fFontHeight = 6.0;
            if( xLegendProp->getPropertyValue("CharHeight") >>= fFontHeight )
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
    xSeriesOrPointProp->setPropertyValue("SymbolSize",uno::makeAny( aSymbolSize ));
}

void lcl_setSymbolSizeIfNeeded( const uno::Reference< beans::XPropertySet >& xSeriesOrPointProp, const SvXMLImport& rImport )
{
    if( !xSeriesOrPointProp.is() )
        return;

    sal_Int32 nSymbolType = chart::ChartSymbolType::NONE;
    if( xSeriesOrPointProp.is() && ( xSeriesOrPointProp->getPropertyValue("SymbolType") >>= nSymbolType) )
    {
        if(chart::ChartSymbolType::NONE!=nSymbolType)
        {
            if( chart::ChartSymbolType::BITMAPURL==nSymbolType )
            {
                //set special size for graphics to indicate to use the bitmap size itself
                xSeriesOrPointProp->setPropertyValue("SymbolSize",uno::makeAny( awt::Size(-1,-1) ));
            }
            else
            {
                lcl_setAutomaticSymbolSize( xSeriesOrPointProp, rImport );
            }
        }
    }
}

void lcl_resetSymbolSizeForPointsIfNecessary( const uno::Reference< beans::XPropertySet >& xPointProp, const SvXMLImport& rImport
    , const XMLPropStyleContext * pPropStyleContext, const SvXMLStylesContext* pStylesCtxt )
{
    uno::Any aASymbolSize( SchXMLTools::getPropertyFromContext( "SymbolSize", pPropStyleContext, pStylesCtxt ) );
    if( !aASymbolSize.hasValue() )
        lcl_setSymbolSizeIfNeeded( xPointProp, rImport );
}

void lcl_insertErrorBarLSequencesToMap(
    tSchXMLLSequencesPerIndex & rInOutMap,
    const uno::Reference< beans::XPropertySet > & xSeriesProp )
{
    Reference< chart2::data::XDataSource > xErrorBarSource;
    if( ( xSeriesProp->getPropertyValue( "ErrorBarY" ) >>= xErrorBarSource ) &&
        xErrorBarSource.is() )
    {
        Sequence< Reference< chart2::data::XLabeledDataSequence > > aLSequences(
            xErrorBarSource->getDataSequences());
        for( sal_Int32 nIndex = 0; nIndex < aLSequences.getLength(); ++nIndex )
        {
            // use "0" as data index. This is ok, as it is not used for error bars
            rInOutMap.emplace(
                    tSchXMLIndexWithPart( 0, SCH_XML_PART_ERROR_BARS ), aLSequences[ nIndex ] );
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
        xSeqProp->setPropertyValue("Role", uno::makeAny( rRole));
    xLabeledSeq->setValues( xSeq );

    // add new sequence to data series / push to front to have the correct sequence order if charttype is changed afterwards
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aOldSeq( xSeriesSource->getDataSequences());
    sal_Int32 nOldCount = aOldSeq.getLength();
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aNewSeq( nOldCount + 1 );
    aNewSeq[0].set(xLabeledSeq, uno::UNO_QUERY_THROW);
    for( sal_Int32 nN=0; nN<nOldCount; nN++ )
        aNewSeq[nN+1] = aOldSeq[nN];
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
    SvXMLImport& rImport, const OUString& rLocalName,
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
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
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
        maChartSize( rChartSize )
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

void SchXMLSeries2Context::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetSeriesAttrTokenMap();
    mnAttachedAxis = 1;

    bool bHasRange = false;
    OUString aSeriesLabelRange;
    OUString aSeriesLabelString;
    bool bHideLegend = false;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_SERIES_CELL_RANGE:
                m_aSeriesRange = aValue;
                bHasRange = true;
                break;
            case XML_TOK_SERIES_LABEL_ADDRESS:
                aSeriesLabelRange = aValue;
                break;
            case XML_TOK_SERIES_LABEL_STRING:
                aSeriesLabelString = aValue;
                break;
            case XML_TOK_SERIES_ATTACHED_AXIS:
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
            case XML_TOK_SERIES_STYLE_NAME:
                msAutoStyleName = aValue;
                break;
            case XML_TOK_SERIES_CHART_CLASS:
                {
                    OUString aClassName;
                    sal_uInt16 nClassPrefix =
                        GetImport().GetNamespaceMap().GetKeyByAttrName(
                            aValue, &aClassName );
                    if( XML_NAMESPACE_CHART == nClassPrefix )
                        maSeriesChartTypeName = SchXMLTools::GetChartTypeByClassName( aClassName, false /* bUseOldNames */ );

                    if( maSeriesChartTypeName.isEmpty())
                        maSeriesChartTypeName = aClassName;
                }
                break;
            case XML_TOK_SERIES_HIDE_LEGEND:
                bHideLegend = aValue.toBoolean();
                break;
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
                xSeriesProp->setPropertyValue("ShowLegendEntry", uno::makeAny(false));

            if( bIsCandleStick )
            {
                // set default color for range-line to black (before applying styles)
                xSeriesProp->setPropertyValue("Color",
                        uno::makeAny( sal_Int32( 0x000000 ))); // black
            }
            else if ( maSeriesChartTypeName == "com.sun.star.chart2.PieChartType" )
            {
                //@todo: this property should be saved
                xSeriesProp->setPropertyValue("VaryColorsByPoint",
                        uno::makeAny( true ));
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
            OUString aMainRole("values-y");
            if (maSeriesChartTypeName == "com.sun.star.chart2.BubbleChartType")
                aMainRole = "values-size";
            xSeqProp->setPropertyValue("Role", uno::makeAny(aMainRole));
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

                uno::Any aASymbolSize( SchXMLTools::getPropertyFromContext( "SymbolSize"
                    , pPropStyleContext, pStylesCtxt ) );
                mbSymbolSizeIsMissingInFile = !aASymbolSize.hasValue();
            }
        }
    }
    catch( const uno::Exception & )
    {
    }
}

struct DomainInfo
{
    DomainInfo( const OUString& rRole, const OUString& rRange, sal_Int32 nIndex )
        : aRole(rRole), aRange(rRange), nIndexForLocalData(nIndex)
    {}

    OUString const aRole;
    OUString aRange;
    sal_Int32 nIndexForLocalData;
};

void SchXMLSeries2Context::EndElement()
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
        DomainInfo aDomainInfo( "values-x", m_rGlobalSeriesImportInfo.aFirstFirstDomainAddress, m_rGlobalSeriesImportInfo.nFirstFirstDomainIndex ) ;
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
            DomainInfo aDomainInfo( "values-x", maDomainAddresses[1], m_rGlobalSeriesImportInfo.nCurrentDataIndex ) ;
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
            DomainInfo aDomainInfo( "values-x", m_rGlobalSeriesImportInfo.aFirstSecondDomainAddress, m_rGlobalSeriesImportInfo.nFirstSecondDomainIndex ) ;
            aDomainInfos.push_back( aDomainInfo );
        }
        if( nDomainCount>0)
        {
            DomainInfo aDomainInfo( "values-y", maDomainAddresses.front(), m_rGlobalSeriesImportInfo.nCurrentDataIndex ) ;
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
            DomainInfo aDomainInfo( "values-y", m_rGlobalSeriesImportInfo.aFirstFirstDomainAddress, m_rGlobalSeriesImportInfo.nFirstFirstDomainIndex ) ;
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

SvXMLImportContextRef SchXMLSeries2Context::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >&  )
{
    SvXMLImportContext* pContext = nullptr;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetSeriesElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_SERIES_DOMAIN:
            if( m_xSeries.is())
            {
                m_bHasDomainContext = true;
                pContext = new SchXMLDomain2Context(
                    GetImport(),
                    nPrefix, rLocalName,
                    maDomainAddresses );
            }
            break;

        case XML_TOK_SERIES_MEAN_VALUE_LINE:
            pContext = new SchXMLStatisticsObjectContext(
                mrImportHelper, GetImport(),
                nPrefix, rLocalName, msAutoStyleName,
                mrStyleVector, m_xSeries,
                SchXMLStatisticsObjectContext::CONTEXT_TYPE_MEAN_VALUE_LINE,
                mrLSequencesPerIndex );
            break;
        case XML_TOK_SERIES_REGRESSION_CURVE:
            pContext = new SchXMLRegressionCurveObjectContext(
                mrImportHelper, GetImport(),
                nPrefix, rLocalName, mrRegressionStyleVector,
                m_xSeries, maChartSize );
            break;
        case XML_TOK_SERIES_ERROR_INDICATOR:
            pContext = new SchXMLStatisticsObjectContext(
                mrImportHelper, GetImport(),
                nPrefix, rLocalName, msAutoStyleName,
                mrStyleVector, m_xSeries,
                SchXMLStatisticsObjectContext::CONTEXT_TYPE_ERROR_INDICATOR,
                mrLSequencesPerIndex );
            break;

        case XML_TOK_SERIES_DATA_POINT:
            pContext = new SchXMLDataPointContext( GetImport(), rLocalName,
                                                   mrStyleVector, m_xSeries, mnDataPointIndex, mbSymbolSizeIsMissingInFile );
            break;
        case XML_TOK_SERIES_PROPERTY_MAPPING:
            pContext = new SchXMLPropertyMappingContext( mrImportHelper,
                    GetImport(), rLocalName,
                    mrLSequencesPerIndex, m_xSeries );
            break;

        default:
            pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
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

    for (auto & seriesStyle : rSeriesDefaultsAndStyles.maSeriesStyleVector)
    {
        if( seriesStyle.meType != DataRowPointStyle::DATA_SERIES )
            continue;

        try
        {
            uno::Reference< beans::XPropertySet > xSeries( seriesStyle.m_xOldAPISeries );
            if( !xSeries.is() )
                continue;

            if( rSeriesDefaultsAndStyles.maSymbolTypeDefault.hasValue() )
                xSeries->setPropertyValue("SymbolType",rSeriesDefaultsAndStyles.maSymbolTypeDefault);
            if( rSeriesDefaultsAndStyles.maDataCaptionDefault.hasValue() )
                xSeries->setPropertyValue("DataCaption",rSeriesDefaultsAndStyles.maDataCaptionDefault);

            if( rSeriesDefaultsAndStyles.maErrorIndicatorDefault.hasValue() )
                xSeries->setPropertyValue("ErrorIndicator",rSeriesDefaultsAndStyles.maErrorIndicatorDefault);
            if( rSeriesDefaultsAndStyles.maErrorCategoryDefault.hasValue() )
                xSeries->setPropertyValue("ErrorCategory",rSeriesDefaultsAndStyles.maErrorCategoryDefault);
            if( rSeriesDefaultsAndStyles.maConstantErrorLowDefault.hasValue() )
                xSeries->setPropertyValue("ConstantErrorLow",rSeriesDefaultsAndStyles.maConstantErrorLowDefault);
            if( rSeriesDefaultsAndStyles.maConstantErrorHighDefault.hasValue() )
                xSeries->setPropertyValue("ConstantErrorHigh",rSeriesDefaultsAndStyles.maConstantErrorHighDefault);
            if( rSeriesDefaultsAndStyles.maPercentageErrorDefault.hasValue() )
                xSeries->setPropertyValue("PercentageError",rSeriesDefaultsAndStyles.maPercentageErrorDefault);
            if( rSeriesDefaultsAndStyles.maErrorMarginDefault.hasValue() )
                xSeries->setPropertyValue("ErrorMargin",rSeriesDefaultsAndStyles.maErrorMarginDefault);

            if( rSeriesDefaultsAndStyles.maMeanValueDefault.hasValue() )
                xSeries->setPropertyValue("MeanValue",rSeriesDefaultsAndStyles.maMeanValueDefault);
            if( rSeriesDefaultsAndStyles.maRegressionCurvesDefault.hasValue() )
                xSeries->setPropertyValue("RegressionCurves",rSeriesDefaultsAndStyles.maRegressionCurvesDefault);
        }
        catch( uno::Exception &  )
        {
            //end of series reached
        }
    }
}

//static
void SchXMLSeries2Context::setStylesToSeries( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , OUString& rCurrStyleName
        , SchXMLImportHelper& rImportHelper
        , const SvXMLImport& rImport
        , bool bIsStockChart
        , tSchXMLLSequencesPerIndex & rInOutLSequencesPerIndex )
{
    // iterate over series
    for (auto & seriesStyle : rSeriesDefaultsAndStyles.maSeriesStyleVector)
    {
        if( seriesStyle.meType == DataRowPointStyle::DATA_SERIES )
        {
            try
            {
                uno::Reference< beans::XPropertySet > xSeriesProp( seriesStyle.m_xOldAPISeries );
                if( !xSeriesProp.is() )
                    continue;

                if( seriesStyle.mnAttachedAxis != 1 )
                {
                    xSeriesProp->setPropertyValue("Axis"
                        , uno::makeAny(chart::ChartAxisAssign::SECONDARY_Y) );
                }

                if( !seriesStyle.msStyleName.isEmpty())
                {
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
                    if( pPropStyleContext )
                    {
                        // error bar style must be set before the other error
                        // bar properties (which may be alphabetically before
                        // this property)
                        bool bHasErrorBarRangesFromData = false;
                        {
                            const OUString aErrorBarStylePropName( "ErrorBarStyle");
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
                        bool bIsMinMaxSeries = false;
                        if( bIsStockChart )
                        {
                            if( SchXMLSeriesHelper::isCandleStickSeries( seriesStyle.m_xSeries
                                    , uno::Reference< frame::XModel >( rImportHelper.GetChartDocument(), uno::UNO_QUERY ) ) )
                                bIsMinMaxSeries = true;
                        }
                        if( !bIsMinMaxSeries )
                        {
                            pPropStyleContext->FillPropertySet( xSeriesProp );
                            if( seriesStyle.mbSymbolSizeForSeriesIsMissingInFile )
                                lcl_setSymbolSizeIfNeeded( xSeriesProp, rImport );
                            if( bHasErrorBarRangesFromData )
                                lcl_insertErrorBarLSequencesToMap( rInOutLSequencesPerIndex, xSeriesProp );
                        }
                    }
                }
            }
            catch( const uno::Exception & rEx )
            {
                SAL_INFO("xmloff.chart", "Exception caught during setting styles to series: " << rEx );
            }
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
                    uno::Any aAny = SchXMLTools::getPropertyFromContext("RegressionType", pPropStyleContext, pStylesCtxt);
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
                    uno::Any aAny = SchXMLTools::getPropertyFromContext("RegressionType", pPropStyleContext, pStylesCtxt);
                    if ( aAny.hasValue() )
                    {
                        aAny >>= aServiceName;
                    }
                }
            }

            if( !aServiceName.isEmpty() )
            {
                Reference< lang::XMultiServiceFactory > xMSF( comphelper::getProcessServiceFactory(), uno::UNO_QUERY );
                Reference< chart2::XRegressionCurve > xRegCurve( xMSF->createInstance( aServiceName ), uno::UNO_QUERY_THROW );
                Reference< chart2::XRegressionCurveContainer > xRegCurveCont( regressionStyle.m_xSeries, uno::UNO_QUERY_THROW );

                Reference< beans::XPropertySet > xCurveProperties( xRegCurve, uno::UNO_QUERY );
                if( pPropStyleContext != nullptr)
                    pPropStyleContext->FillPropertySet( xCurveProperties );

                xRegCurve->setEquationProperties( regressionStyle.m_xEquationProperties );

                xRegCurveCont->addRegressionCurve( xRegCurve );
            }
        }
        catch( const uno::Exception& rEx )
        {
            SAL_INFO("xmloff.chart", "Exception caught during setting styles to series: " << rEx );
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
                    xNewSeriesProp->setPropertyValue("ErrorBarX",uno::makeAny(seriesStyle.m_xErrorXProperties));

                if (seriesStyle.m_xErrorYProperties.is())
                    xNewSeriesProp->setPropertyValue("ErrorBarY",uno::makeAny(seriesStyle.m_xErrorYProperties));
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
                                xSeriesProp->getPropertyValue("DataMeanValueProperties") >>= xStatPropSet;
                                break;
                            case DataRowPointStyle::ERROR_INDICATOR:
                                xSeriesProp->getPropertyValue("DataErrorProperties")  >>= xStatPropSet;
                                break;
                            default:
                                break;
                        }
                        if( xStatPropSet.is())
                            pPropStyleContext->FillPropertySet( xStatPropSet );
                    }
                }
            }
            catch( const uno::Exception & rEx )
            {
                SAL_INFO("xmloff.chart", "Exception caught during setting styles to series: " << rEx );
            }
        }
    }
}

//static
void SchXMLSeries2Context::setStylesToDataPoints( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , OUString& rCurrStyleName
        , SchXMLImportHelper& rImportHelper
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
            if( SchXMLSeriesHelper::isCandleStickSeries( seriesStyle.m_xSeries, uno::Reference< frame::XModel >( rImportHelper.GetChartDocument(), uno::UNO_QUERY ) ) )
                continue;
        }

        // data point style
        for( sal_Int32 i = 0; i < seriesStyle.m_nPointRepeat; i++ )
        {
            try
            {
                uno::Reference< beans::XPropertySet > xPointProp(
                    SchXMLSeriesHelper::createOldAPIDataPointPropertySet( seriesStyle.m_xSeries, seriesStyle.m_nPointIndex + i
                        , uno::Reference< frame::XModel >( rImportHelper.GetChartDocument(), uno::UNO_QUERY ) ) );

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
                        xPointProp->setPropertyValue("Lines",uno::makeAny(false));
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
                if( pPropStyleContext )
                {
                    pPropStyleContext->FillPropertySet( xPointProp );
                    if( seriesStyle.mbSymbolSizeForSeriesIsMissingInFile )
                        lcl_resetSymbolSizeForPointsIfNecessary( xPointProp, rImport, pPropStyleContext, pStylesCtxt );
                }
            }
            catch( const uno::Exception & rEx )
            {
                SAL_INFO("xmloff.chart", "Exception caught during setting styles to data points: " << rEx );
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

            xSeries->setPropertyValue("Lines",uno::makeAny(false));
        }
        catch( uno::Exception &  )
        {
            //end of series reached
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
