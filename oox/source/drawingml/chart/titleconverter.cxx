/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: titleconverter.cxx,v $
 *
 * $Revision: 1.2 $
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
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XFormattedString.hpp>
#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include "oox/drawingml/chart/datasourceconverter.hxx"
#include "oox/drawingml/chart/titlemodel.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::chart2::XDiagram;
using ::com::sun::star::chart2::XFormattedString;
using ::com::sun::star::chart2::XLegend;
using ::com::sun::star::chart2::XTitle;
using ::com::sun::star::chart2::XTitled;
using ::com::sun::star::chart2::data::XDataSequence;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

namespace {

void lclAppendFormattedString( ::std::vector< Reference< XFormattedString > >& orStringVec, const OUString& rString, bool bAddNewLine )
{
    try
    {
        Reference< XFormattedString > xFmtStr( ConverterRoot::createInstance( CREATE_OUSTRING( "com.sun.star.chart2.FormattedString" ) ), UNO_QUERY_THROW );
        xFmtStr->setString( bAddNewLine ? (rString + OUString( sal_Unicode( '\n' ) )) : rString );
        orStringVec.push_back( xFmtStr );
    }
    catch( Exception& )
    {
    }
}

} // namespace

// ----------------------------------------------------------------------------

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

Sequence< Reference< XFormattedString > > TextConverter::createStringSequence( const OUString& rDefaultText )
{
    ::std::vector< Reference< XFormattedString > > aStringVec;

    OSL_ENSURE( !mrModel.mxDataSeq || !mrModel.mxTextBody, "TextConverter::createStringSequence - linked string and rich text found" );

    // try to create string from linked data
    if( mrModel.mxDataSeq.is() )
    {
        const DataSequenceModel::AnyMap& rData = mrModel.mxDataSeq->maData;
        if( !rData.empty() )
        {
            ::rtl::OUString aString;
            if( rData.begin()->second >>= aString )
                lclAppendFormattedString( aStringVec, aString, false );
        }
    }
    else if( mrModel.mxTextBody.is() )
    {
        const TextParagraphVector& rParas = mrModel.mxTextBody->getParagraphs();
        for( TextParagraphVector::const_iterator aPIt = rParas.begin(), aPEnd = rParas.end(); aPIt != aPEnd; ++aPIt )
        {
            const TextRunVector& rRuns = (*aPIt)->getRuns();
            for( TextRunVector::const_iterator aRIt = rRuns.begin(), aREnd = rRuns.end(); aRIt != aREnd; ++aRIt )
                lclAppendFormattedString( aStringVec, (*aRIt)->getText(), (aRIt + 1 == aREnd) && (aPIt + 1 != aPEnd) );
        }
    }
    else if( rDefaultText.getLength() > 0 )
    {
        lclAppendFormattedString( aStringVec, rDefaultText, false );
    }

    return ContainerHelper::vectorToSequence( aStringVec );
}

// ============================================================================

TitleConverter::TitleConverter( const ConverterRoot& rParent, TitleModel& rModel ) :
    ConverterBase< TitleModel >( rParent, rModel )
{
}

TitleConverter::~TitleConverter()
{
}

void TitleConverter::convertModelToDocument( const Reference< XTitled >& rxTitled, const ::rtl::OUString& rAutoTitle )
{
    if( rxTitled.is() )
    {
        // create the formatted strings
        TextConverter aTextConv( *this, mrModel.mxText.getOrCreate() );
        Sequence< Reference< XFormattedString > > aStringSeq = aTextConv.createStringSequence( rAutoTitle );
        if( aStringSeq.hasElements() ) try
        {
            // create the title object and set the string data
            Reference< XTitle > xTitle( createInstance( CREATE_OUSTRING( "com.sun.star.chart2.Title" ) ), UNO_QUERY_THROW );
            xTitle->setText( aStringSeq );
            rxTitled->setTitleObject( xTitle );
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

void LegendConverter::convertModelToDocument( const Reference< XDiagram >& rxDiagram )
{
    if( rxDiagram.is() ) try
    {
        Reference< XLegend > xLegend( createInstance( CREATE_OUSTRING( "com.sun.star.chart2.Legend" ) ), UNO_QUERY_THROW );
        rxDiagram->setLegend( xLegend );
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

