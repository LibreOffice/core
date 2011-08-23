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

#include "oox/drawingml/chart/titleconverter.hxx"
#include <com/sun/star/chart2/LegendExpansion.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XFormattedString.hpp>
#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include "properties.hxx"
#include "oox/drawingml/textbody.hxx"
#include "oox/drawingml/textparagraph.hxx"
#include "oox/drawingml/chart/datasourceconverter.hxx"
#include "oox/drawingml/chart/titlemodel.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::awt::Rectangle;
using ::com::sun::star::chart2::XDiagram;
using ::com::sun::star::chart2::XFormattedString;
using ::com::sun::star::chart2::XLegend;
using ::com::sun::star::chart2::XTitle;
using ::com::sun::star::chart2::XTitled;
using ::com::sun::star::chart2::data::XDataSequence;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

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
                bool bAddNewLine = (aRIt + 1 == aREnd) && (aPIt + 1 != aPEnd);
                Reference< XFormattedString > xFmtStr = appendFormattedString( aStringVec, rTextRun.getText(), bAddNewLine );
                PropertySet aPropSet( xFmtStr );
                TextCharacterProperties aRunProps( rParaProps );
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
        if( aString.getLength() == 0 )
            aString = rDefaultText;

        // create formatted string object
        if( aString.getLength() > 0 )
        {
            Reference< XFormattedString > xFmtStr = appendFormattedString( aStringVec, aString, false );
            PropertySet aPropSet( xFmtStr );
            getFormatter().convertTextFormatting( aPropSet, rxTextProp, eObjType );
        }
    }

    return ContainerHelper::vectorToSequence( aStringVec );
}

Reference< XFormattedString > TextConverter::appendFormattedString(
        ::std::vector< Reference< XFormattedString > >& orStringVec, const OUString& rString, bool bAddNewLine ) const
{
    Reference< XFormattedString > xFmtStr;
    try
    {
        xFmtStr.set( ConverterRoot::createInstance( CREATE_OUSTRING( "com.sun.star.chart2.FormattedString" ) ), UNO_QUERY_THROW );
        xFmtStr->setString( bAddNewLine ? (rString + OUString( sal_Unicode( '\n' ) )) : rString );
        orStringVec.push_back( xFmtStr );
    }
    catch( Exception& )
    {
    }
    return xFmtStr;
}

// ============================================================================

TitleConverter::TitleConverter( const ConverterRoot& rParent, TitleModel& rModel ) :
    ConverterBase< TitleModel >( rParent, rModel )
{
}

TitleConverter::~TitleConverter()
{
}

void TitleConverter::convertFromModel( const Reference< XTitled >& rxTitled, const OUString& rAutoTitle, ObjectType eObjType, sal_Int32 nMainIdx, sal_Int32 nSubIdx )
{
    if( rxTitled.is() )
    {
        // create the formatted strings
        TextModel& rText = mrModel.mxText.getOrCreate();
        TextConverter aTextConv( *this, rText );
        Sequence< Reference< XFormattedString > > aStringSeq = aTextConv.createStringSequence( rAutoTitle, mrModel.mxTextProp, eObjType );
        if( aStringSeq.hasElements() ) try
        {
            // create the title object and set the string data
            Reference< XTitle > xTitle( createInstance( CREATE_OUSTRING( "com.sun.star.chart2.Title" ) ), UNO_QUERY_THROW );
            xTitle->setText( aStringSeq );
            rxTitled->setTitleObject( xTitle );

            // frame formatting (text formatting already done in TextConverter::createStringSequence())
            PropertySet aPropSet( xTitle );
            getFormatter().convertFrameFormatting( aPropSet, mrModel.mxShapeProp, eObjType );

            // frame rotation
            OSL_ENSURE( !mrModel.mxTextProp || !rText.mxTextBody, "TitleConverter::convertFromModel - multiple text properties" );
            ModelRef< TextBody > xTextProp = mrModel.mxTextProp.is() ? mrModel.mxTextProp : rText.mxTextBody;
            getFormatter().convertTextRotation( aPropSet, xTextProp, true );

            // register the title and layout data for conversion of position
            registerTitleLayout( xTitle, mrModel.mxLayout, eObjType, nMainIdx, nSubIdx );
        }
        catch( Exception& )
        {
        }
    }
}

// ============================================================================

LegendConverter::LegendConverter( const ConverterRoot& rParent, LegendModel& rModel ) :
    ConverterBase< LegendModel >( rParent, rModel )
{
}

LegendConverter::~LegendConverter()
{
}

void LegendConverter::convertFromModel( const Reference< XDiagram >& rxDiagram )
{
    if( rxDiagram.is() ) try
    {
        namespace cssc2 = ::com::sun::star::chart2;

        // create the legend
        Reference< XLegend > xLegend( createInstance( CREATE_OUSTRING( "com.sun.star.chart2.Legend" ) ), UNO_QUERY_THROW );
        rxDiagram->setLegend( xLegend );
        PropertySet aPropSet( xLegend );
        aPropSet.setProperty( PROP_Show, true );

        // legend formatting
        getFormatter().convertFormatting( aPropSet, mrModel.mxShapeProp, mrModel.mxTextProp, OBJECTTYPE_LEGEND );

        // predefined legend position and expansion
        cssc2::LegendPosition eLegendPos = cssc2::LegendPosition_CUSTOM;
        cssc2::LegendExpansion eLegendExpand = cssc2::LegendExpansion_HIGH;
        switch( mrModel.mnPosition )
        {
            case XML_l:
                eLegendPos = cssc2::LegendPosition_LINE_START;
                eLegendExpand = cssc2::LegendExpansion_HIGH;
            break;
            case XML_r:
                eLegendPos = cssc2::LegendPosition_LINE_END;
                eLegendExpand = cssc2::LegendExpansion_HIGH;
            break;
            case XML_t:
                eLegendPos = cssc2::LegendPosition_PAGE_START;
                eLegendExpand = cssc2::LegendExpansion_WIDE;
            break;
            case XML_b:
                eLegendPos = cssc2::LegendPosition_PAGE_END;
                eLegendExpand = cssc2::LegendExpansion_WIDE;
            break;
            case XML_tr:
                eLegendPos = cssc2::LegendPosition_LINE_END; // top-right not supported
                eLegendExpand = cssc2::LegendExpansion_HIGH;
            break;
        }

        // manual positioning
        LayoutModel& rLayout = mrModel.mxLayout.getOrCreate();
        LayoutConverter aLayoutConv( *this, rLayout );
        aLayoutConv.convertFromModel( aPropSet );
        Rectangle aLegendRect;
        if( aLayoutConv.calcAbsRectangle( aLegendRect ) )
        {
            // #i71697# it is not possible to set the size directly, do some magic here
            double fRatio = static_cast< double >( aLegendRect.Width ) / aLegendRect.Height;
            if( fRatio > 1.5 )
                eLegendExpand = cssc2::LegendExpansion_WIDE;
            else if( fRatio < 0.75 )
                eLegendExpand = cssc2::LegendExpansion_HIGH;
            else
                eLegendExpand = cssc2::LegendExpansion_BALANCED;
        }

        // set position and expansion properties
        aPropSet.setProperty( PROP_AnchorPosition, eLegendPos );
        aPropSet.setProperty( PROP_Expansion, eLegendExpand );
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

