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

#include "VLegend.hxx"
#include "macros.hxx"
#include "PropertyMapper.hxx"
#include "CommonConverters.hxx"
#include "ObjectIdentifier.hxx"
#include "RelativePositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "RelativeSizeHelper.hxx"
#include "LegendEntryProvider.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <rtl/ustrbuf.hxx>
#include <svl/languageoptions.hxx>

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

namespace
{

typedef ::std::pair< ::chart::tNameSequence, ::chart::tAnySequence > tPropertyValues;

typedef ::std::vector< ViewLegendEntry > tViewLegendEntryContainer;

double lcl_CalcViewFontSize(
    const Reference< beans::XPropertySet > & xProp,
    const awt::Size & rReferenceSize )
{
    double fResult = 10.0;

    awt::Size aPropRefSize;
    float fFontHeight( 0.0 );
    if( xProp.is() && ( xProp->getPropertyValue( "CharHeight") >>= fFontHeight ))
    {
        fResult = fFontHeight;
        try
        {
            if( (xProp->getPropertyValue( "ReferencePageSize") >>= aPropRefSize) &&
                (aPropRefSize.Height > 0))
            {
                fResult = ::chart::RelativeSizeHelper::calculate( fFontHeight, aPropRefSize, rReferenceSize );
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    // pt -> 1/100th mm
    return (fResult * (2540.0 / 72.0));
}

void lcl_getProperties(
    const Reference< beans::XPropertySet > & xLegendProp,
    tPropertyValues & rOutLineFillProperties,
    tPropertyValues & rOutTextProperties,
    const awt::Size & rReferenceSize )
{
    // Get Line- and FillProperties from model legend
    if( xLegendProp.is())
    {
        // set rOutLineFillProperties
        ::chart::tPropertyNameValueMap aLineFillValueMap;
        ::chart::PropertyMapper::getValueMap( aLineFillValueMap, ::chart::PropertyMapper::getPropertyNameMapForFillAndLineProperties(), xLegendProp );

        aLineFillValueMap[ "LineJoint" ] = uno::makeAny( drawing::LineJoint_ROUND );

        ::chart::PropertyMapper::getMultiPropertyListsFromValueMap(
            rOutLineFillProperties.first, rOutLineFillProperties.second, aLineFillValueMap );

        // set rOutTextProperties
        ::chart::tPropertyNameValueMap aTextValueMap;
        ::chart::PropertyMapper::getValueMap( aTextValueMap, ::chart::PropertyMapper::getPropertyNameMapForCharacterProperties(), xLegendProp );

        drawing::TextHorizontalAdjust eHorizAdjust( drawing::TextHorizontalAdjust_LEFT );
        aTextValueMap[ "TextAutoGrowHeight" ] = uno::makeAny( sal_True );
        aTextValueMap[ "TextAutoGrowWidth" ] = uno::makeAny( sal_True );
        aTextValueMap[ "TextHorizontalAdjust" ] = uno::makeAny( eHorizAdjust );
        aTextValueMap[ "TextMaximumFrameWidth" ] = uno::makeAny( rReferenceSize.Width ); //needs to be overwritten by actual available space in the legend

        // recalculate font size
        awt::Size aPropRefSize;
        float fFontHeight( 0.0 );
        if( (xLegendProp->getPropertyValue( "ReferencePageSize") >>= aPropRefSize) &&
            (aPropRefSize.Height > 0) &&
            (aTextValueMap[ "CharHeight" ] >>= fFontHeight) )
        {
            aTextValueMap[ "CharHeight" ] = uno::makeAny(
                static_cast< float >(
                    ::chart::RelativeSizeHelper::calculate( fFontHeight, aPropRefSize, rReferenceSize )));

            if( aTextValueMap[ "CharHeightAsian" ] >>= fFontHeight )
            {
                aTextValueMap[ "CharHeightAsian" ] = uno::makeAny(
                    static_cast< float >(
                        ::chart::RelativeSizeHelper::calculate( fFontHeight, aPropRefSize, rReferenceSize )));
            }
            if( aTextValueMap[ "CharHeightComplex" ] >>= fFontHeight )
            {
                aTextValueMap[ "CharHeightComplex" ] = uno::makeAny(
                    static_cast< float >(
                        ::chart::RelativeSizeHelper::calculate( fFontHeight, aPropRefSize, rReferenceSize )));
            }
        }

        ::chart::PropertyMapper::getMultiPropertyListsFromValueMap(
            rOutTextProperties.first, rOutTextProperties.second, aTextValueMap );
    }
}

awt::Size lcl_createTextShapes(
    const tViewLegendEntryContainer & rEntries,
    const Reference< lang::XMultiServiceFactory > & xShapeFactory,
    const Reference< drawing::XShapes > & xTarget,
    ::std::vector< Reference< drawing::XShape > > & rOutTextShapes,
    const tPropertyValues & rTextProperties )
{
    awt::Size aResult;

    for( tViewLegendEntryContainer::const_iterator aIt( rEntries.begin());
         aIt != rEntries.end(); ++aIt )
    {
        try
        {
            // create label shape
            Reference< drawing::XShape > xEntry(
                xShapeFactory->createInstance(
                    "com.sun.star.drawing.TextShape"), uno::UNO_QUERY_THROW );
            xTarget->add( xEntry );

            // set label text
            Sequence< Reference< XFormattedString2 > > aLabelSeq = (*aIt).aLabel;
            for( sal_Int32 i = 0; i < aLabelSeq.getLength(); ++i )
            {
                // todo: support more than one text range
                if( i == 1 )
                    break;

                Reference< text::XTextRange > xRange( xEntry, uno::UNO_QUERY );
                OUString aLabelString( aLabelSeq[i]->getString());
                // workaround for Issue #i67540#
                if( aLabelString.isEmpty())
                    aLabelString = " ";
                if( xRange.is())
                    xRange->setString( aLabelString );

                PropertyMapper::setMultiProperties(
                    rTextProperties.first, rTextProperties.second,
                    Reference< beans::XPropertySet >( xRange, uno::UNO_QUERY ));

                // adapt max-extent
                awt::Size aCurrSize( xEntry->getSize());
                aResult.Width  = ::std::max( aResult.Width,  aCurrSize.Width  );
                aResult.Height = ::std::max( aResult.Height, aCurrSize.Height );
            }

            rOutTextShapes.push_back( xEntry );
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return aResult;
}

void lcl_collectColumnWidths( std::vector< sal_Int32 >& rColumnWidths, const sal_Int32 nNumberOfRows, const sal_Int32 nNumberOfColumns
                             , const ::std::vector< Reference< drawing::XShape > > aTextShapes, sal_Int32 nSymbolPlusDistanceWidth )
{
    rColumnWidths.clear();
    sal_Int32 nRow = 0;
    sal_Int32 nColumn = 0;
    sal_Int32 nNumberOfEntries = aTextShapes.size();
    for( ; nRow < nNumberOfRows; ++nRow )
    {
        for( nColumn = 0; nColumn < nNumberOfColumns; ++nColumn )
        {
            sal_Int32 nEntry = (nColumn + nRow * nNumberOfColumns);
            if( nEntry < nNumberOfEntries )
            {
                awt::Size aTextSize( aTextShapes[ nEntry ]->getSize() );
                sal_Int32 nWidth = nSymbolPlusDistanceWidth + aTextSize.Width;
                if( nRow==0 )
                    rColumnWidths.push_back( nWidth );
                else
                    rColumnWidths[nColumn] = ::std::max( nWidth, rColumnWidths[nColumn] );
            }
        }
    }
}

void lcl_collectRowHeighs( std::vector< sal_Int32 >& rRowHeights, const sal_Int32 nNumberOfRows, const sal_Int32 nNumberOfColumns
                          , const ::std::vector< Reference< drawing::XShape > > aTextShapes )
{
    // calculate maximum height for each row
    // and collect column widths
    rRowHeights.clear();
    sal_Int32 nRow = 0;
    sal_Int32 nColumn = 0;
    sal_Int32 nNumberOfEntries = aTextShapes.size();
    for( ; nRow < nNumberOfRows; ++nRow )
    {
        sal_Int32 nCurrentRowHeight = 0;
        for( nColumn = 0; nColumn < nNumberOfColumns; ++nColumn )
        {
            sal_Int32 nEntry = (nColumn + nRow * nNumberOfColumns);
            if( nEntry < nNumberOfEntries )
            {
                awt::Size aTextSize( aTextShapes[ nEntry ]->getSize() );
                nCurrentRowHeight = ::std::max( nCurrentRowHeight, aTextSize.Height );
            }
        }
        rRowHeights.push_back( nCurrentRowHeight );
    }
}

sal_Int32 lcl_getTextLineHeight( const std::vector< sal_Int32 >& aRowHeights, const sal_Int32 nNumberOfRows, double fViewFontSize )
{
    const sal_Int32 nFontHeight = static_cast< sal_Int32 >( fViewFontSize );
    sal_Int32 nTextLineHeight = nFontHeight;
    for( sal_Int32 nR=0; nR<nNumberOfRows; nR++ )
    {
        sal_Int32 nFullTextHeight = aRowHeights[ nR ];
        if( ( nFullTextHeight / nFontHeight ) <= 1 )
        {
            nTextLineHeight = nFullTextHeight;//found an entry with one line-> have real text height
            break;
        }
    }
    return nTextLineHeight;
}

//returns resulting legend size
awt::Size lcl_placeLegendEntries(
    tViewLegendEntryContainer & rEntries,
    ::com::sun::star::chart::ChartLegendExpansion eExpansion,
    bool bSymbolsLeftSide,
    double fViewFontSize,
    const awt::Size& rMaxSymbolExtent,
    tPropertyValues & rTextProperties,
    const Reference< drawing::XShapes > & xTarget,
    const Reference< lang::XMultiServiceFactory > & xShapeFactory,
    const awt::Size & rAvailableSpace )
{
    bool bIsCustomSize = (eExpansion == ::com::sun::star::chart::ChartLegendExpansion_CUSTOM);
    awt::Size aResultingLegendSize(0,0);
    if( bIsCustomSize )
        aResultingLegendSize = rAvailableSpace;

    // #i109336# Improve auto positioning in chart
    sal_Int32 nXPadding = static_cast< sal_Int32 >( std::max( 100.0, fViewFontSize * 0.33 ) );
    //sal_Int32 nXPadding = static_cast< sal_Int32 >( std::max( 200.0, fViewFontSize * 0.33 ) );
    sal_Int32 nXOffset  = static_cast< sal_Int32 >( std::max( 100.0, fViewFontSize * 0.66 ) );
    sal_Int32 nYPadding = static_cast< sal_Int32 >( std::max( 100.0, fViewFontSize * 0.2 ) );
    sal_Int32 nYOffset  = static_cast< sal_Int32 >( std::max( 100.0, fViewFontSize * 0.2 ) );
    //sal_Int32 nYOffset  = static_cast< sal_Int32 >( std::max( 230.0, fViewFontSize * 0.45 ) );

    const sal_Int32 nSymbolToTextDistance = static_cast< sal_Int32 >( std::max( 100.0, fViewFontSize * 0.22 ) );//minimum 1mm
    const sal_Int32 nSymbolPlusDistanceWidth = rMaxSymbolExtent.Width + nSymbolToTextDistance;
    sal_Int32 nMaxTextWidth = rAvailableSpace.Width - (2 * nXPadding) - nSymbolPlusDistanceWidth;
    OUString aPropNameTextMaximumFrameWidth( "TextMaximumFrameWidth" );
    uno::Any* pFrameWidthAny = PropertyMapper::getValuePointer( rTextProperties.second, rTextProperties.first, aPropNameTextMaximumFrameWidth);
    if(pFrameWidthAny)
    {
        if( eExpansion == ::com::sun::star::chart::ChartLegendExpansion_HIGH )
        {
            // limit the width of texts to 30% of the total available width
            // #i109336# Improve auto positioning in chart
            nMaxTextWidth = rAvailableSpace.Width * 3 / 10;
        }
        *pFrameWidthAny = uno::makeAny(nMaxTextWidth);
    }

    ::std::vector< Reference< drawing::XShape > > aTextShapes;
    awt::Size aMaxEntryExtent = lcl_createTextShapes( rEntries, xShapeFactory, xTarget, aTextShapes, rTextProperties );
    OSL_ASSERT( aTextShapes.size() == rEntries.size());

    sal_Int32 nMaxEntryWidth = nXOffset + nSymbolPlusDistanceWidth + aMaxEntryExtent.Width;
    sal_Int32 nMaxEntryHeight = nYOffset + aMaxEntryExtent.Height;
    sal_Int32 nNumberOfEntries = rEntries.size();

    sal_Int32 nNumberOfColumns = 0, nNumberOfRows = 0;
    std::vector< sal_Int32 > aColumnWidths;
    std::vector< sal_Int32 > aRowHeights;

    sal_Int32 nTextLineHeight = static_cast< sal_Int32 >( fViewFontSize );

    // determine layout depending on LegendExpansion
    if( eExpansion == ::com::sun::star::chart::ChartLegendExpansion_CUSTOM )
    {
        sal_Int32 nCurrentRow=0;
        sal_Int32 nCurrentColumn=-1;
        sal_Int32 nMaxColumnCount=-1;
        for( sal_Int32 nN=0; nN<static_cast<sal_Int32>(aTextShapes.size()); nN++ )
        {
            Reference< drawing::XShape > xShape( aTextShapes[nN] );
            if( !xShape.is() )
                continue;
            awt::Size aSize( xShape->getSize() );
            sal_Int32 nNewWidth = aSize.Width + nSymbolPlusDistanceWidth;
            sal_Int32 nCurrentColumnCount = aColumnWidths.size();

            //are we allowed to add a new column?
            if( nMaxColumnCount==-1 || (nCurrentColumn+1) < nMaxColumnCount )
            {
                //try add a new column
                nCurrentColumn++;
                if( nCurrentColumn < nCurrentColumnCount )
                {
                    //check whether the current column width is sufficient for the new entry
                    if( aColumnWidths[nCurrentColumn]>=nNewWidth )
                    {
                        //all good proceed with next entry
                        continue;
                    }
                }
                if( nCurrentColumn < nCurrentColumnCount )
                    aColumnWidths[nCurrentColumn] = std::max( nNewWidth, aColumnWidths[nCurrentColumn] );
                else
                    aColumnWidths.push_back(nNewWidth);

                //do the columns still fit into the given size?
                nCurrentColumnCount = aColumnWidths.size();//update count
                sal_Int32 nSumWidth = 0;
                for( sal_Int32 nC=0; nC<nCurrentColumnCount; nC++ )
                    nSumWidth += aColumnWidths[nC];

                if( nSumWidth <= rAvailableSpace.Width || nCurrentColumnCount==1 )
                {
                    //all good proceed with next entry
                    continue;
                }
                else
                {
                    //not enough space for the current amount of columns
                    //try again with less columns
                    nMaxColumnCount = nCurrentColumnCount-1;
                    nN=-1;
                    nCurrentRow=0;
                    nCurrentColumn=-1;
                    aColumnWidths.clear();
                }
            }
            else
            {
                //add a new row and try the same entry again
                nCurrentRow++;
                nCurrentColumn=-1;
                nN--;
            }
        }
        nNumberOfColumns = aColumnWidths.size();
        nNumberOfRows = nCurrentRow+1;

        //check if there is not enough space so that some entries must be removed
        lcl_collectRowHeighs( aRowHeights, nNumberOfRows, nNumberOfColumns, aTextShapes );
        nTextLineHeight = lcl_getTextLineHeight( aRowHeights, nNumberOfRows, fViewFontSize );
        sal_Int32 nSumHeight = 0;
        for( sal_Int32 nR=0; nR<nNumberOfRows; nR++ )
            nSumHeight += aRowHeights[nR];
        sal_Int32 nRemainingSpace = rAvailableSpace.Height - nSumHeight;

        if( nRemainingSpace<0 )
        {
            //remove entries that are too big
            for( sal_Int32 nR=nNumberOfRows; nR--; )
            {
                for( sal_Int32 nC=nNumberOfColumns; nC--; )
                {
                    sal_Int32 nEntry = (nC + nR * nNumberOfColumns);
                    if( nEntry < static_cast<sal_Int32>(aTextShapes.size()) )
                    {
                        DrawModelWrapper::removeShape( aTextShapes[nEntry] );
                        aTextShapes.pop_back();
                    }
                    if( nEntry < nNumberOfEntries )
                    {
                        DrawModelWrapper::removeShape( rEntries[ nEntry ].aSymbol );
                        rEntries.pop_back();
                        nNumberOfEntries--;
                    }
                }
                nSumHeight -= aRowHeights[nR];
                aRowHeights.pop_back();
                nRemainingSpace = rAvailableSpace.Height - nSumHeight;
                if( nRemainingSpace>=0 )
                    break;
            }
            nNumberOfRows = static_cast<sal_Int32>(aRowHeights.size());
        }
        if( nRemainingSpace > 0 )
        {
            sal_Int32 nNormalSpacingHeight = 2*nYPadding+(nNumberOfRows-1)*nYOffset;
            if( nRemainingSpace < nNormalSpacingHeight )
            {
                //reduce spacing between the entries
                nYPadding = nYOffset = nRemainingSpace/(nNumberOfRows+1);
            }
            else
            {
                //we have some space left that should be spread equally between all rows
                sal_Int32 nRemainingSingleSpace = (nRemainingSpace-nNormalSpacingHeight)/(nNumberOfRows+1);
                nYPadding += nRemainingSingleSpace;
                nYOffset += nRemainingSingleSpace;
            }
        }

        //check spacing between columns
        sal_Int32 nSumWidth = 0;
        for( sal_Int32 nC=0; nC<nNumberOfColumns; nC++ )
            nSumWidth += aColumnWidths[nC];
        nRemainingSpace = rAvailableSpace.Width - nSumWidth;
        if( nRemainingSpace>=0 )
        {
            sal_Int32 nNormalSpacingWidth = 2*nXPadding+(nNumberOfColumns-1)*nXOffset;
            if( nRemainingSpace < nNormalSpacingWidth )
            {
                //reduce spacing between the entries
                nXPadding = nXOffset = nRemainingSpace/(nNumberOfColumns+1);
            }
            else
            {
                //we have some space left that should be spread equally between all columns
                sal_Int32 nRemainingSingleSpace = (nRemainingSpace-nNormalSpacingWidth)/(nNumberOfColumns+1);
                nXPadding += nRemainingSingleSpace;
                nXOffset += nRemainingSingleSpace;
            }
        }
    }
    else if( eExpansion == ::com::sun::star::chart::ChartLegendExpansion_HIGH )
    {
        sal_Int32 nMaxNumberOfRows = nMaxEntryHeight
            ? (rAvailableSpace.Height - 2*nYPadding ) / nMaxEntryHeight
            : 0;

        nNumberOfColumns = nMaxNumberOfRows
            ? static_cast< sal_Int32 >(
                ceil( static_cast< double >( nNumberOfEntries ) /
                      static_cast< double >( nMaxNumberOfRows ) ))
            : 0;
        nNumberOfRows =  nNumberOfColumns
            ? static_cast< sal_Int32 >(
                ceil( static_cast< double >( nNumberOfEntries ) /
                      static_cast< double >( nNumberOfColumns ) ))
            : 0;
    }
    else if( eExpansion == ::com::sun::star::chart::ChartLegendExpansion_WIDE )
    {
        sal_Int32 nMaxNumberOfColumns = nMaxEntryWidth
            ? (rAvailableSpace.Width - 2*nXPadding ) / nMaxEntryWidth
            : 0;

        nNumberOfRows = nMaxNumberOfColumns
            ? static_cast< sal_Int32 >(
                ceil( static_cast< double >( nNumberOfEntries ) /
                      static_cast< double >( nMaxNumberOfColumns ) ))
            : 0;
        nNumberOfColumns = nNumberOfRows
            ? static_cast< sal_Int32 >(
                ceil( static_cast< double >( nNumberOfEntries ) /
                      static_cast< double >( nNumberOfRows ) ))
            : 0;
    }
    else // ::com::sun::star::chart::ChartLegendExpansion_BALANCED
    {
        double fAspect = nMaxEntryHeight
            ? static_cast< double >( nMaxEntryWidth ) / static_cast< double >( nMaxEntryHeight )
            : 0.0;

        nNumberOfRows = static_cast< sal_Int32 >(
            ceil( sqrt( static_cast< double >( nNumberOfEntries ) * fAspect )));
        nNumberOfColumns = nNumberOfRows
            ? static_cast< sal_Int32 >(
                ceil( static_cast< double >( nNumberOfEntries ) /
                      static_cast< double >( nNumberOfRows ) ))
            : 0;
    }

    if(nNumberOfRows<=0)
        return aResultingLegendSize;

    if( eExpansion != ::com::sun::star::chart::ChartLegendExpansion_CUSTOM )
    {
        lcl_collectColumnWidths( aColumnWidths, nNumberOfRows, nNumberOfColumns, aTextShapes, nSymbolPlusDistanceWidth );
        lcl_collectRowHeighs( aRowHeights, nNumberOfRows, nNumberOfColumns, aTextShapes );
        nTextLineHeight = lcl_getTextLineHeight( aRowHeights, nNumberOfRows, fViewFontSize );
    }

    sal_Int32 nCurrentXPos = nXPadding;
    sal_Int32 nCurrentYPos = nYPadding;
    if( !bSymbolsLeftSide )
        nCurrentXPos = -nXPadding;

    // place entries into column and rows
    sal_Int32 nMaxYPos = 0;
    sal_Int32 nRow = 0;
    sal_Int32 nColumn = 0;
    for( nColumn = 0; nColumn < nNumberOfColumns; ++nColumn )
    {
        nCurrentYPos = nYPadding;
        for( nRow = 0; nRow < nNumberOfRows; ++nRow )
        {
            sal_Int32 nEntry = (nColumn + nRow * nNumberOfColumns);
            if( nEntry >= nNumberOfEntries )
                break;

            // text shape
            Reference< drawing::XShape > xTextShape( aTextShapes[nEntry] );
            if( xTextShape.is() )
            {
                awt::Size aTextSize( xTextShape->getSize() );
                sal_Int32 nTextXPos = nCurrentXPos + nSymbolPlusDistanceWidth;
                if( !bSymbolsLeftSide )
                    nTextXPos = nCurrentXPos - nSymbolPlusDistanceWidth - aTextSize.Width;
                xTextShape->setPosition( awt::Point( nTextXPos, nCurrentYPos ));
            }

            // symbol
            Reference< drawing::XShape > xSymbol( rEntries[ nEntry ].aSymbol );
            if( xSymbol.is() )
            {
                awt::Size aSymbolSize( rMaxSymbolExtent );
                sal_Int32 nSymbolXPos = nCurrentXPos;
                if( !bSymbolsLeftSide )
                    nSymbolXPos = nCurrentXPos - rMaxSymbolExtent.Width;
                sal_Int32 nSymbolYPos = nCurrentYPos + ( ( nTextLineHeight - aSymbolSize.Height ) / 2 );
                xSymbol->setPosition( awt::Point( nSymbolXPos, nSymbolYPos ) );
            }

            nCurrentYPos += aRowHeights[ nRow ];
            if( nRow+1 < nNumberOfRows )
                nCurrentYPos += nYOffset;
            nMaxYPos = ::std::max( nMaxYPos, nCurrentYPos );
        }
        if( bSymbolsLeftSide )
        {
            nCurrentXPos += aColumnWidths[nColumn];
            if( nColumn+1 < nNumberOfColumns )
                nCurrentXPos += nXOffset;
        }
        else
        {
            nCurrentXPos -= aColumnWidths[nColumn];
            if( nColumn+1 < nNumberOfColumns )
                nCurrentXPos -= nXOffset;
        }
    }

    if( !bIsCustomSize )
    {
        if( bSymbolsLeftSide )
            aResultingLegendSize.Width  = nCurrentXPos + nXPadding;
        else
        {
            sal_Int32 nLegendWidth = -(nCurrentXPos-nXPadding);
            aResultingLegendSize.Width  = nLegendWidth;
        }
        aResultingLegendSize.Height = nMaxYPos + nYPadding;
    }

    if( !bSymbolsLeftSide )
    {
        sal_Int32 nLegendWidth = aResultingLegendSize.Width;
        awt::Point aPos(0,0);
        for( sal_Int32 nEntry=0; nEntry<nNumberOfEntries; nEntry++ )
        {
            Reference< drawing::XShape > xSymbol( rEntries[ nEntry ].aSymbol );
            aPos = xSymbol->getPosition();
            aPos.X += nLegendWidth;
            xSymbol->setPosition( aPos );
            Reference< drawing::XShape > xText( aTextShapes[ nEntry ] );
            aPos = xText->getPosition();
            aPos.X += nLegendWidth;
            xText->setPosition( aPos );
        }
    }

    return aResultingLegendSize;
}

// #i109336# Improve auto positioning in chart
sal_Int32 lcl_getLegendLeftRightMargin()
{
    return 210;  // 1/100 mm
}

// #i109336# Improve auto positioning in chart
sal_Int32 lcl_getLegendTopBottomMargin()
{
    return 185;  // 1/100 mm
}

chart2::RelativePosition lcl_getDefaultPosition( LegendPosition ePos, const awt::Rectangle& rOutAvailableSpace, const awt::Size & rPageSize )
{
    chart2::RelativePosition aResult;

    switch( ePos )
    {
        case LegendPosition_LINE_START:
            {
                // #i109336# Improve auto positioning in chart
                const double fDefaultDistance = ( static_cast< double >( lcl_getLegendLeftRightMargin() ) /
                    static_cast< double >( rPageSize.Width ) );
                aResult = chart2::RelativePosition(
                    fDefaultDistance, 0.5, drawing::Alignment_LEFT );
            }
            break;
        case LegendPosition_LINE_END:
            {
                // #i109336# Improve auto positioning in chart
                const double fDefaultDistance = ( static_cast< double >( lcl_getLegendLeftRightMargin() ) /
                    static_cast< double >( rPageSize.Width ) );
                aResult = chart2::RelativePosition(
                    1.0 - fDefaultDistance, 0.5, drawing::Alignment_RIGHT );
            }
            break;
        case LegendPosition_PAGE_START:
            {
                // #i109336# Improve auto positioning in chart
                const double fDefaultDistance = ( static_cast< double >( lcl_getLegendTopBottomMargin() ) /
                    static_cast< double >( rPageSize.Height ) );
                double fDistance = (static_cast<double>(rOutAvailableSpace.Y)/static_cast<double>(rPageSize.Height)) + fDefaultDistance;
                aResult = chart2::RelativePosition(
                    0.5, fDistance, drawing::Alignment_TOP );
            }
            break;
        case LegendPosition_PAGE_END:
            {
                // #i109336# Improve auto positioning in chart
                const double fDefaultDistance = ( static_cast< double >( lcl_getLegendTopBottomMargin() ) /
                    static_cast< double >( rPageSize.Height ) );
                aResult = chart2::RelativePosition(
                    0.5, 1.0 - fDefaultDistance, drawing::Alignment_BOTTOM );
            }
            break;

        case LegendPosition_CUSTOM:
            // to avoid warning
        case LegendPosition_MAKE_FIXED_SIZE:
            // nothing to be set
            break;
    }

    return aResult;
}

/**  @return
         a point relative to the upper left corner that can be used for
         XShape::setPosition()
*/
awt::Point lcl_calculatePositionAndRemainingSpace(
    awt::Rectangle & rRemainingSpace,
    const awt::Size & rPageSize,
    chart2::RelativePosition aRelPos,
    LegendPosition ePos,
    const awt::Size& aLegendSize )
{
    // calculate position
    awt::Point aResult(
        static_cast< sal_Int32 >( aRelPos.Primary * rPageSize.Width ),
        static_cast< sal_Int32 >( aRelPos.Secondary * rPageSize.Height ));

    aResult = RelativePositionHelper::getUpperLeftCornerOfAnchoredObject(
        aResult, aLegendSize, aRelPos.Anchor );

    // adapt rRemainingSpace if LegendPosition is not CUSTOM
    // #i109336# Improve auto positioning in chart
    sal_Int32 nXDistance = lcl_getLegendLeftRightMargin();
    sal_Int32 nYDistance = lcl_getLegendTopBottomMargin();
    switch( ePos )
    {
        case LegendPosition_LINE_START:
            {
                sal_Int32 nExtent = aLegendSize.Width;
                rRemainingSpace.Width -= ( nExtent + nXDistance );
                rRemainingSpace.X += ( nExtent + nXDistance );
            }
        break;
        case LegendPosition_LINE_END:
            {
                rRemainingSpace.Width -= ( aLegendSize.Width + nXDistance );
            }
            break;
        case LegendPosition_PAGE_START:
            {
                sal_Int32 nExtent = aLegendSize.Height;
                rRemainingSpace.Height -= ( nExtent + nYDistance );
                rRemainingSpace.Y += ( nExtent + nYDistance );
            }
        break;
        case LegendPosition_PAGE_END:
            {
                rRemainingSpace.Height -= ( aLegendSize.Height + nYDistance );
            }
            break;

        default:
            // nothing
            break;
    }

    // adjust the legend position. Esp. for old files that had slightly smaller legends
    const sal_Int32 nEdgeDistance( 30 );
    if( aResult.X + aLegendSize.Width > rPageSize.Width )
    {
        sal_Int32 nNewX( (rPageSize.Width - aLegendSize.Width) - nEdgeDistance );
        if( nNewX > rPageSize.Width / 4 )
            aResult.X = nNewX;
    }
    if( aResult.Y + aLegendSize.Height > rPageSize.Height )
    {
        sal_Int32 nNewY( (rPageSize.Height - aLegendSize.Height) - nEdgeDistance );
        if( nNewY > rPageSize.Height / 4 )
            aResult.Y = nNewY;
    }

    return aResult;
}

bool lcl_shouldSymbolsBePlacedOnTheLeftSide( const Reference< beans::XPropertySet >& xLegendProp, sal_Int16 nDefaultWritingMode )
{
    bool bSymbolsLeftSide = true;
    try
    {
        if( SvtLanguageOptions().IsCTLFontEnabled() )
        {
            if(xLegendProp.is())
            {
                sal_Int16 nWritingMode=-1;
                if( (xLegendProp->getPropertyValue( "WritingMode" ) >>= nWritingMode) )
                {
                    if( nWritingMode == text::WritingMode2::PAGE )
                        nWritingMode = nDefaultWritingMode;
                    if( nWritingMode == text::WritingMode2::RL_TB )
                        bSymbolsLeftSide=false;
                }
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return bSymbolsLeftSide;
}

} // anonymous namespace

VLegend::VLegend(
    const Reference< XLegend > & xLegend,
    const Reference< uno::XComponentContext > & xContext,
    const std::vector< LegendEntryProvider* >& rLegendEntryProviderList ) :
        m_xLegend( xLegend ),
        m_xContext( xContext ),
        m_aLegendEntryProviderList( rLegendEntryProviderList )
{
}

void VLegend::init(
    const Reference< drawing::XShapes >& xTargetPage,
    const Reference< lang::XMultiServiceFactory >& xFactory,
    const Reference< frame::XModel >& xModel )
{
    m_xTarget = xTargetPage;
    m_xShapeFactory = xFactory;
    m_xModel = xModel;
}

void VLegend::setDefaultWritingMode( sal_Int16 nDefaultWritingMode )
{
    m_nDefaultWritingMode = nDefaultWritingMode;
}

bool VLegend::isVisible( const Reference< XLegend > & xLegend )
{
    if( ! xLegend.is())
        return sal_False;

    sal_Bool bShow = sal_False;
    try
    {
        Reference< beans::XPropertySet > xLegendProp( xLegend, uno::UNO_QUERY_THROW );
        xLegendProp->getPropertyValue( "Show") >>= bShow;
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return bShow;
}

void VLegend::createShapes(
    const awt::Size & rAvailableSpace,
    const awt::Size & rPageSize )
{
    if(! (m_xLegend.is() &&
          m_xShapeFactory.is() &&
          m_xTarget.is()))
        return;

    try
    {
        //create shape and add to page
        m_xShape.set( m_xShapeFactory->createInstance(
                          "com.sun.star.drawing.GroupShape"), uno::UNO_QUERY );
        m_xTarget->add( m_xShape );

        // set name to enable selection
        {
            OUString aLegendParticle( ObjectIdentifier::createParticleForLegend( m_xLegend, m_xModel ) );
            ShapeFactory::setShapeName( m_xShape, ObjectIdentifier::createClassifiedIdentifierForParticle( aLegendParticle ) );
        }

        // create and insert sub-shapes
        Reference< drawing::XShapes > xLegendContainer( m_xShape, uno::UNO_QUERY );
        if( xLegendContainer.is())
        {
            Reference< drawing::XShape > xBorder(
                m_xShapeFactory->createInstance(
                    "com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY );

            // for quickly setting properties
            tPropertyValues aLineFillProperties;
            tPropertyValues aTextProperties;

            Reference< beans::XPropertySet > xLegendProp( m_xLegend, uno::UNO_QUERY );
            ::com::sun::star::chart::ChartLegendExpansion eExpansion = ::com::sun::star::chart::ChartLegendExpansion_HIGH;
            awt::Size aLegendSize( rAvailableSpace );

            if( xLegendProp.is())
            {
                // get Expansion property
                xLegendProp->getPropertyValue( "Expansion") >>= eExpansion;
                if( eExpansion == ::com::sun::star::chart::ChartLegendExpansion_CUSTOM )
                {
                    RelativeSize aRelativeSize;
                    if ((xLegendProp->getPropertyValue( "RelativeSize") >>= aRelativeSize))
                    {
                        aLegendSize.Width = static_cast<sal_Int32>(::rtl::math::approxCeil( aRelativeSize.Primary * rPageSize.Width ));
                        aLegendSize.Height = static_cast<sal_Int32>(::rtl::math::approxCeil( aRelativeSize.Secondary * rPageSize.Height ));
                    }
                    else
                        eExpansion = ::com::sun::star::chart::ChartLegendExpansion_HIGH;
                }
                lcl_getProperties( xLegendProp, aLineFillProperties, aTextProperties, rPageSize );
            }

            if( xBorder.is())
            {
                xLegendContainer->add( xBorder );

                // apply legend properties
                PropertyMapper::setMultiProperties(
                    aLineFillProperties.first, aLineFillProperties.second,
                    Reference< beans::XPropertySet >( xBorder, uno::UNO_QUERY ));

                //because of this name this border will be used for marking the legend
                ShapeFactory(m_xShapeFactory).setShapeName( xBorder, "MarkHandles" );
            }

            // create entries
            double fViewFontSize = lcl_CalcViewFontSize( xLegendProp, rPageSize );//todo
            // #i109336# Improve auto positioning in chart
            sal_Int32 nSymbolHeigth = static_cast< sal_Int32 >( fViewFontSize * 0.6  );
            sal_Int32 nSymbolWidth = static_cast< sal_Int32 >( nSymbolHeigth );

            ::std::vector< LegendEntryProvider* >::const_iterator       aIter = m_aLegendEntryProviderList.begin();
            const ::std::vector< LegendEntryProvider* >::const_iterator aEnd  = m_aLegendEntryProviderList.end();
            for( aIter = m_aLegendEntryProviderList.begin(); aIter != aEnd; ++aIter )
            {
                LegendEntryProvider* pLegendEntryProvider( *aIter );
                if( pLegendEntryProvider )
                {
                    awt::Size aCurrentRatio = pLegendEntryProvider->getPreferredLegendKeyAspectRatio();
                    sal_Int32 nCurrentWidth = aCurrentRatio.Width;
                    if( aCurrentRatio.Height > 0 )
                    {
                        nCurrentWidth = nSymbolHeigth* aCurrentRatio.Width/aCurrentRatio.Height;
                    }
                    nSymbolWidth = std::max( nSymbolWidth, nCurrentWidth );
                }
            }
            awt::Size aMaxSymbolExtent( nSymbolWidth, nSymbolHeigth );

            tViewLegendEntryContainer aViewEntries;
            for( aIter = m_aLegendEntryProviderList.begin(); aIter != aEnd; ++aIter )
            {
                LegendEntryProvider* pLegendEntryProvider( *aIter );
                if( pLegendEntryProvider )
                {
                    std::vector< ViewLegendEntry > aNewEntries = pLegendEntryProvider->createLegendEntries( aMaxSymbolExtent, eExpansion, xLegendProp, xLegendContainer, m_xShapeFactory, m_xContext );
                    aViewEntries.insert( aViewEntries.end(), aNewEntries.begin(), aNewEntries.end() );
                }
            }

            bool bSymbolsLeftSide = lcl_shouldSymbolsBePlacedOnTheLeftSide( xLegendProp, m_nDefaultWritingMode );

            if( aViewEntries.size() ) {
                // place entries
                aLegendSize = lcl_placeLegendEntries( aViewEntries, eExpansion, bSymbolsLeftSide, fViewFontSize, aMaxSymbolExtent,
                                                      aTextProperties, xLegendContainer, m_xShapeFactory, aLegendSize );

                if( xBorder.is() )
                    xBorder->setSize( aLegendSize );
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void VLegend::changePosition(
    awt::Rectangle & rOutAvailableSpace,
    const awt::Size & rPageSize )
{
    if(! m_xShape.is())
        return;

    try
    {
        // determine position and alignment depending on default position
        awt::Size aLegendSize = m_xShape->getSize();
        Reference< beans::XPropertySet > xLegendProp( m_xLegend, uno::UNO_QUERY_THROW );
        chart2::RelativePosition aRelativePosition;

        bool bAutoPosition =
            ! (xLegendProp->getPropertyValue( "RelativePosition") >>= aRelativePosition);

        LegendPosition ePos = LegendPosition_CUSTOM;
        xLegendProp->getPropertyValue( "AnchorPosition") >>= ePos;

        //calculate position
        if( bAutoPosition )
        {
            // auto position: relative to remaining space
            aRelativePosition = lcl_getDefaultPosition( ePos, rOutAvailableSpace, rPageSize );
            awt::Point aPos = lcl_calculatePositionAndRemainingSpace(
                rOutAvailableSpace, rPageSize, aRelativePosition, ePos, aLegendSize );
            m_xShape->setPosition( aPos );
        }
        else
        {
            // manual position: relative to whole page
            awt::Rectangle aAvailableSpace( 0, 0, rPageSize.Width, rPageSize.Height );
            awt::Point aPos = lcl_calculatePositionAndRemainingSpace(
                aAvailableSpace, rPageSize, aRelativePosition, ePos, aLegendSize );
            m_xShape->setPosition( aPos );

            if( ePos != LegendPosition_CUSTOM )
            {
                // calculate remaining space as if having autoposition:
                aRelativePosition = lcl_getDefaultPosition( ePos, rOutAvailableSpace, rPageSize );
                lcl_calculatePositionAndRemainingSpace(
                    rOutAvailableSpace, rPageSize, aRelativePosition, ePos, aLegendSize );
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
