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

#include <drawingml/chart/titleconverter.hxx>

#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/FormattedString.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>

#include <comphelper/sequence.hxx>
#include <osl/diagnose.h>
#include <drawingml/textbody.hxx>
#include <drawingml/textparagraph.hxx>
#include <drawingml/chart/datasourceconverter.hxx>
#include <drawingml/chart/titlemodel.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/drawing/Alignment.hpp>

#include <oox/drawingml/chart/modelbase.hxx>
namespace oox::drawingml::chart {

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::chart2::data;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;


TextConverter::TextConverter( const ConverterRoot& rParent, TextModel& rModel ) :
    ConverterBase< TextModel >( rParent, rModel )
{
}

TextConverter::~TextConverter()
{
}

Reference< XDataSequence > TextConverter::createDataSequence( const OUString& rRole )
{
    Reference< XDataSequence > xDataSeq;
    if( mrModel.mxDataSeq.is() )
    {
        DataSequenceConverter aDataSeqConv( *this, *mrModel.mxDataSeq );
        xDataSeq = aDataSeqConv.createDataSequence( rRole );
    }
    return xDataSeq;
}

Sequence< Reference< XFormattedString > > TextConverter::createStringSequence(
        const OUString& rDefaultText, const ModelRef< TextBody >& rxTextProp, ObjectType eObjType )
{
    OSL_ENSURE( !mrModel.mxDataSeq || !mrModel.mxTextBody, "TextConverter::createStringSequence - linked string and rich text found" );
    ::std::vector< Reference< XFormattedString > > aStringVec;
    if( mrModel.mxTextBody.is() )
    {
        // rich-formatted text objects can be created, but currently Chart2 is not able to show them
        const TextParagraphVector& rTextParas = mrModel.mxTextBody->getParagraphs();
        for( TextParagraphVector::const_iterator aPIt = rTextParas.begin(), aPEnd = rTextParas.end(); aPIt != aPEnd; ++aPIt )
        {
            const TextParagraph& rTextPara = **aPIt;
            const TextCharacterProperties& rParaProps = rTextPara.getProperties().getTextCharacterProperties();
            for( TextRunVector::const_iterator aRIt = rTextPara.getRuns().begin(), aREnd = rTextPara.getRuns().end(); aRIt != aREnd; ++aRIt )
            {
                const TextRun& rTextRun = **aRIt;
                bool bAddNewLine = ((aRIt + 1 == aREnd) && (aPIt + 1 != aPEnd)) || rTextRun.isLineBreak();
                Reference< XFormattedString > xFmtStr = appendFormattedString( aStringVec, rTextRun.getText(), bAddNewLine );
                PropertySet aPropSet( xFmtStr );
                TextCharacterProperties aRunProps;
                if (rParaProps.mbHasEmptyParaProperties && rxTextProp.is() && rxTextProp->hasParagraphProperties())
                {
                    const TextParagraphVector rDefTextParas = rxTextProp->getParagraphs();
                    TextParagraphVector::const_iterator aDefPIt = rDefTextParas.begin();
                    const TextParagraph& rDefTextPara = **aDefPIt;
                    aRunProps = rDefTextPara.getProperties().getTextCharacterProperties();
                }
                else
                    aRunProps = rParaProps;
                aRunProps.assignUsed( rTextRun.getTextCharacterProperties() );
                getFormatter().convertTextFormatting( aPropSet, aRunProps, eObjType );
            }
        }
    }
    else
    {
        OUString aString;
        // try to create string from linked data
        if( mrModel.mxDataSeq.is() && !mrModel.mxDataSeq->maData.empty() )
            mrModel.mxDataSeq->maData.begin()->second >>= aString;
        // no linked string -> fall back to default string
        if( aString.isEmpty() )
            aString = rDefaultText;

        // create formatted string object
        if( !aString.isEmpty() )
        {
            Reference< XFormattedString > xFmtStr = appendFormattedString( aStringVec, aString, false );
            PropertySet aPropSet( xFmtStr );
            getFormatter().convertTextFormatting( aPropSet, rxTextProp, eObjType );
        }
    }

    return comphelper::containerToSequence( aStringVec );
}

Reference< XFormattedString > TextConverter::appendFormattedString(
        ::std::vector< Reference< XFormattedString > >& orStringVec, const OUString& rString, bool bAddNewLine ) const
{
    Reference< XFormattedString2 > xFmtStr;
    try
    {
        xFmtStr = FormattedString::create( ConverterRoot::getComponentContext() );
        xFmtStr->setString( bAddNewLine ? (rString + "\n") : rString );
        orStringVec.emplace_back(xFmtStr );
    }
    catch( Exception& )
    {
    }
    return xFmtStr;
}

TitleConverter::TitleConverter( const ConverterRoot& rParent, TitleModel& rModel ) :
    ConverterBase< TitleModel >( rParent, rModel )
{
}

TitleConverter::~TitleConverter()
{
}

void TitleConverter::convertFromModel( const Reference< XTitled >& rxTitled, const OUString& rAutoTitle, ObjectType eObjType, sal_Int32 nMainIdx, sal_Int32 nSubIdx )
{
    if( !rxTitled.is() )
        return;

    // create the formatted strings
    TextModel& rText = mrModel.mxText.getOrCreate();
    TextConverter aTextConv( *this, rText );
    Sequence< Reference< XFormattedString > > aStringSeq = aTextConv.createStringSequence( rAutoTitle, mrModel.mxTextProp, eObjType );
    if( !aStringSeq.hasElements() )
        return;

    try
    {
        // create the title object and set the string data
        Reference< XTitle > xTitle( createInstance( "com.sun.star.chart2.Title" ), UNO_QUERY_THROW );
        xTitle->setText( aStringSeq );
        rxTitled->setTitleObject( xTitle );

        // frame formatting (text formatting already done in TextConverter::createStringSequence())
        PropertySet aPropSet( xTitle );
        getFormatter().convertFrameFormatting( aPropSet, mrModel.mxShapeProp, eObjType );

        // frame rotation
        OSL_ENSURE( !mrModel.mxTextProp || !rText.mxTextBody, "TitleConverter::convertFromModel - multiple text properties" );
        ModelRef< TextBody > xTextProp = mrModel.mxTextProp.is() ? mrModel.mxTextProp : rText.mxTextBody;
        ObjectFormatter::convertTextRotation( aPropSet, xTextProp, true, mrModel.mnDefaultRotation );

        // register the title and layout data for conversion of position
        registerTitleLayout( xTitle, mrModel.mxLayout, eObjType, nMainIdx, nSubIdx );
    }
    catch( Exception& )
    {
    }
}

LegendConverter::LegendConverter( const ConverterRoot& rParent, LegendModel& rModel ) :
    ConverterBase< LegendModel >( rParent, rModel )
{
}

LegendConverter::~LegendConverter()
{
}

void LegendConverter::convertFromModel( const Reference< XDiagram >& rxDiagram )
{
    if( !rxDiagram.is() )
        return;

    try
    {
        namespace cssc = ::com::sun::star::chart;
        namespace cssc2 = ::com::sun::star::chart2;

        // create the legend
        Reference< XLegend > xLegend( createInstance( "com.sun.star.chart2.Legend" ), UNO_QUERY_THROW );
        rxDiagram->setLegend( xLegend );
        PropertySet aPropSet( xLegend );
        aPropSet.setProperty( PROP_Show, true );

        // legend formatting
        getFormatter().convertFormatting( aPropSet, mrModel.mxShapeProp, mrModel.mxTextProp, OBJECTTYPE_LEGEND );

        // predefined legend position and expansion
        cssc2::LegendPosition eLegendPos = cssc2::LegendPosition_LINE_END;
        cssc::ChartLegendExpansion eLegendExpand = cssc::ChartLegendExpansion_CUSTOM;
        RelativePosition eRelPos;
        bool bTopRight=false;
        switch( mrModel.mnPosition )
        {
            case XML_l:
                eLegendPos = cssc2::LegendPosition_LINE_START;
                eLegendExpand = cssc::ChartLegendExpansion_HIGH;
            break;
            case XML_r:
                eLegendPos = cssc2::LegendPosition_LINE_END;
                eLegendExpand = cssc::ChartLegendExpansion_HIGH;
            break;
            case XML_tr:    // top-right not supported
                eRelPos.Primary = 1;
                eRelPos.Secondary =0;
                eRelPos.Anchor = Alignment_TOP_RIGHT;
                bTopRight=true;
            break;
            case XML_t:
                eLegendPos = cssc2::LegendPosition_PAGE_START;
                eLegendExpand = cssc::ChartLegendExpansion_WIDE;
            break;
            case XML_b:
                eLegendPos = cssc2::LegendPosition_PAGE_END;
                eLegendExpand = cssc::ChartLegendExpansion_WIDE;
            break;
        }
        bool bManualLayout=false;
        // manual positioning and size
        if( mrModel.mxLayout )
        {
            LayoutConverter aLayoutConv( *this, *mrModel.mxLayout );
            // manual size needs ChartLegendExpansion_CUSTOM
            if( aLayoutConv.convertFromModel( aPropSet ) )
            {
                eLegendExpand = cssc::ChartLegendExpansion_CUSTOM;
            }
            bManualLayout = !aLayoutConv.getAutoLayout();
        }

        // set position and expansion properties
        aPropSet.setProperty( PROP_AnchorPosition, eLegendPos );
        aPropSet.setProperty( PROP_Expansion, eLegendExpand );

        if (bTopRight && !bManualLayout)
            aPropSet.setProperty( PROP_RelativePosition , Any(eRelPos));

        aPropSet.setProperty(PROP_Overlay, mrModel.mbOverlay);

        if (mrModel.maLegendEntries.size() > 0)
            legendEntriesFormatting(rxDiagram);
    }
    catch( Exception& )
    {
    }
}

void LegendConverter::legendEntriesFormatting(const Reference<XDiagram>& rxDiagram)
{
    Reference<XCoordinateSystemContainer> xCooSysContainer(rxDiagram, UNO_QUERY_THROW);
    const Sequence<Reference<XCoordinateSystem>> xCooSysSequence(xCooSysContainer->getCoordinateSystems());
    if (!xCooSysSequence.hasElements())
        return;

    sal_Int32 nIndex = 0;
    for (const auto& rCooSys : xCooSysSequence)
    {
        PropertySet aCooSysProp(rCooSys);
        bool bSwapXAndY = aCooSysProp.getBoolProperty(PROP_SwapXAndYAxis);

        Reference<XChartTypeContainer> xChartTypeContainer(rCooSys, UNO_QUERY_THROW);
        const Sequence<Reference<XChartType>> xChartTypeSequence(xChartTypeContainer->getChartTypes());
        if (!xChartTypeSequence.hasElements())
            continue;

        for (const auto& rCT : xChartTypeSequence)
        {
            Reference<XDataSeriesContainer> xDSCont(rCT, UNO_QUERY);
            if (!xDSCont.is())
                continue;

            bool bIsPie
                = rCT->getChartType().equalsIgnoreAsciiCase("com.sun.star.chart2.PieChartType");
            if (bIsPie)
            {
                PropertySet xChartTypeProp(rCT);
                bIsPie = !xChartTypeProp.getBoolProperty(PROP_UseRings);
            }
            const Sequence<Reference<XDataSeries>> aDataSeriesSeq = xDSCont->getDataSeries();
            if (bSwapXAndY)
                nIndex += aDataSeriesSeq.getLength() - 1;
            for (const auto& rDataSeries : aDataSeriesSeq)
            {
                PropertySet aSeriesProp(rDataSeries);
                bool bVaryColorsByPoint = aSeriesProp.getBoolProperty(PROP_VaryColorsByPoint);

                if (bVaryColorsByPoint || bIsPie)
                {
                    Reference<XDataSource> xDSrc(rDataSeries, UNO_QUERY);
                    if (!xDSrc.is())
                        continue;

                    const Sequence<Reference<XLabeledDataSequence> > aDataSeqs = xDSrc->getDataSequences();
                    std::vector<sal_Int32> deletedLegendEntries;
                    sal_Int32 j = 0;
                    for (const auto& rDataSeq : aDataSeqs)
                    {
                        Reference<XDataSequence> xValues = rDataSeq->getValues();
                        if (!xValues.is())
                            continue;

                        sal_Int32 nDataSeqSize = xValues->getData().getLength();
                        for (sal_Int32 i = 0; i < nDataSeqSize; ++i)
                        {
                            for (const auto& rLegendEntry : mrModel.maLegendEntries)
                            {
                                if (nIndex == rLegendEntry->mnLegendEntryIdx && rLegendEntry->mbLabelDeleted)
                                {
                                    deletedLegendEntries.push_back(j + i);
                                    break;
                                }
                            }
                            nIndex++;
                        }
                        j += nDataSeqSize;
                    }
                    if (deletedLegendEntries.size() > 0)
                        aSeriesProp.setProperty(PROP_DeletedLegendEntries, comphelper::containerToSequence(deletedLegendEntries));
                }
                else
                {
                    for (const auto& rLegendEntry : mrModel.maLegendEntries)
                    {
                        if (nIndex == rLegendEntry->mnLegendEntryIdx)
                        {
                            aSeriesProp.setProperty(PROP_ShowLegendEntry, !rLegendEntry->mbLabelDeleted);
                            break;
                        }
                    }
                    bSwapXAndY ? nIndex-- : nIndex++;
                }
            }
            if (bSwapXAndY)
                nIndex += aDataSeriesSeq.getLength() + 1;
        }
    }
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
