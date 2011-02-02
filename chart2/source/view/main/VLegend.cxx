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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "VLegend.hxx"
#include "macros.hxx"
#include "PropertyMapper.hxx"
#include "CommonConverters.hxx"
#include "ObjectIdentifier.hxx"
#include "RelativePositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "RelativeSizeHelper.hxx"
#include "LegendEntryProvider.hxx"
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/chart2/LegendExpansion.hpp>
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
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

//.............................................................................
namespace chart
{
//.............................................................................

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
    if( xProp.is() && ( xProp->getPropertyValue( C2U( "CharHeight" )) >>= fFontHeight ))
    {
        fResult = fFontHeight;
        try
        {
            if( (xProp->getPropertyValue( C2U( "ReferencePageSize" )) >>= aPropRefSize) &&
                (aPropRefSize.Height > 0))
            {
                // todo: find out if asian text is really used
//         Reference< beans::XPropertySetInfo >xInfo( xProp, uno::UNO_QUERY );
//         float fFontHeight2 = fFontHeight;
//         if( xInfo.is() &&
//             xInfo->hasPropertyByName(C2U("CharHeightAsian")) &&
//             (xProp->getPropertyValue(C2U("CharHeightAsian")) >>= fFontHeight2) &&
//             fFontHeight2 > fFontHeight )
//         {
//             fFontHeight = fFontHeight2;
//         }

//         if( xInfo.is() &&
//             xInfo->hasPropertyByName(C2U("CharHeightComplex")) &&
//             (xProp->getPropertyValue(C2U("CharHeightComplex")) >>= fFontHeight2) &&
//             fFontHeight2 > fFontHeight )
//         {
//             fFontHeight = fFontHeight2;
//         }

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
    sal_Int32 nMaxLabelWidth,
    const awt::Size & rReferenceSize )
{
    // Get Line- and FillProperties from model legend
    if( xLegendProp.is())
    {
        // set rOutLineFillProperties
        ::chart::tPropertyNameValueMap aLineFillValueMap;
        ::chart::PropertyMapper::getValueMap( aLineFillValueMap, ::chart::PropertyMapper::getPropertyNameMapForFillAndLineProperties(), xLegendProp );

        aLineFillValueMap[ C2U("LineJoint") ] = uno::makeAny( drawing::LineJoint_ROUND );

        ::chart::PropertyMapper::getMultiPropertyListsFromValueMap(
            rOutLineFillProperties.first, rOutLineFillProperties.second, aLineFillValueMap );

        // set rOutTextProperties
        ::chart::tPropertyNameValueMap aTextValueMap;
        ::chart::PropertyMapper::getValueMap( aTextValueMap, ::chart::PropertyMapper::getPropertyNameMapForCharacterProperties(), xLegendProp );

        drawing::TextHorizontalAdjust eHorizAdjust( drawing::TextHorizontalAdjust_LEFT );
        aTextValueMap[ C2U("TextAutoGrowHeight") ] = uno::makeAny( sal_True );
        aTextValueMap[ C2U("TextAutoGrowWidth") ] = uno::makeAny( sal_True );
        aTextValueMap[ C2U("TextHorizontalAdjust") ] = uno::makeAny( eHorizAdjust );
        aTextValueMap[ C2U("TextMaximumFrameWidth") ] = uno::makeAny( nMaxLabelWidth );

        // recalculate font size
        awt::Size aPropRefSize;
        float fFontHeight( 0.0 );
        if( (xLegendProp->getPropertyValue( C2U( "ReferencePageSize" )) >>= aPropRefSize) &&
            (aPropRefSize.Height > 0) &&
            (aTextValueMap[ C2U("CharHeight") ] >>= fFontHeight) )
        {
            aTextValueMap[ C2U("CharHeight") ] = uno::makeAny(
                static_cast< float >(
                    ::chart::RelativeSizeHelper::calculate( fFontHeight, aPropRefSize, rReferenceSize )));

            if( aTextValueMap[ C2U("CharHeightAsian") ] >>= fFontHeight )
            {
                aTextValueMap[ C2U("CharHeightAsian") ] = uno::makeAny(
                    static_cast< float >(
                        ::chart::RelativeSizeHelper::calculate( fFontHeight, aPropRefSize, rReferenceSize )));
            }
            if( aTextValueMap[ C2U("CharHeightComplex") ] >>= fFontHeight )
            {
                aTextValueMap[ C2U("CharHeightComplex") ] = uno::makeAny(
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
            Reference< drawing::XShape > xGroupShapeForSingleEntry(
                xShapeFactory->createInstance(
                    C2U( "com.sun.star.drawing.GroupShape" )), uno::UNO_QUERY_THROW );
            Reference< drawing::XShape >xEntry(
                xShapeFactory->createInstance(
                    C2U( "com.sun.star.drawing.TextShape" )), uno::UNO_QUERY_THROW );
            xTarget->add( xGroupShapeForSingleEntry );

            Reference< drawing::XShapes > xGroup( xGroupShapeForSingleEntry, uno::UNO_QUERY_THROW );
            xGroup->add( xEntry );

            // set label text
            Sequence< Reference< XFormattedString > > aLabelSeq = (*aIt).aLabel;
            for( sal_Int32 i = 0; i < aLabelSeq.getLength(); ++i )
            {
                // todo: support more than one text range
                if( i == 1 )
                    break;

                Reference< text::XTextRange > xRange( xEntry, uno::UNO_QUERY );
                OUString aLabelString( aLabelSeq[i]->getString());
                // workaround for Issue #i67540#
                if( !aLabelString.getLength())
                    aLabelString = C2U(" ");
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

            rOutTextShapes.push_back( xGroupShapeForSingleEntry );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return aResult;
}


void lcl_placeLegendEntries(
    const tViewLegendEntryContainer & rEntries,
    LegendExpansion eExpansion,
    bool bSymbolsLeftSide,
    const Reference< beans::XPropertySet > & xProperties,
    tPropertyValues & rTextProperties,
    const Reference< drawing::XShapes > & xTarget,
    const Reference< lang::XMultiServiceFactory > & xShapeFactory,
    const Reference< uno::XComponentContext > & /* xContext */,
    const awt::Size & rAvailableSpace,
    const awt::Size & rPageSize,
    awt::Size & rOutLegendSize )
{
    double fViewFontSize = lcl_CalcViewFontSize( xProperties, rPageSize );

    // padding as percentage of the font height
    // #i109336# Improve auto positioning in chart
    double fXPadding = 0.1;
    double fYPadding = 0.2;
    double fXOffset  = 0.15;
    double fYOffset  = 0.15;

    const sal_Int32 nXPadding = static_cast< sal_Int32 >( fViewFontSize * fXPadding );
    const sal_Int32 nYPadding = static_cast< sal_Int32 >( fViewFontSize * fYPadding );
    const sal_Int32 nXOffset  = static_cast< sal_Int32 >( fViewFontSize * fXOffset );
    const sal_Int32 nYOffset  = static_cast< sal_Int32 >( fViewFontSize * fYOffset );

    ::std::vector< Reference< drawing::XShape > > aTextShapes;
    awt::Size aMaxEntryExtent = lcl_createTextShapes(
        rEntries, xShapeFactory, xTarget, aTextShapes, rTextProperties );
    OSL_ASSERT( aTextShapes.size() == rEntries.size());

    // #i109336# Improve auto positioning in chart
    double fSymbolSizeFraction = 0.8;
    awt::Size aMaxSymbolExtent( static_cast< sal_Int32 >( fViewFontSize * fSymbolSizeFraction * 3.0 / 2.0 ),
                                static_cast< sal_Int32 >( fViewFontSize * fSymbolSizeFraction ) );

    sal_Int32 nCurrentXPos = nXPadding;
    sal_Int32 nCurrentYPos = nYPadding;
    sal_Int32 nMaxEntryWidth = 2 * nXOffset + aMaxSymbolExtent.Width + aMaxEntryExtent.Width;
    sal_Int32 nMaxEntryHeight = nYOffset + aMaxEntryExtent.Height;
    sal_Int32 nNumberOfEntries = rEntries.size();

    if( !bSymbolsLeftSide )
        nCurrentXPos = -nXPadding;

    sal_Int32 nNumberOfColumns = 0, nNumberOfRows = 0;

    // determine layout depending on LegendExpansion
    if( eExpansion == LegendExpansion_HIGH )
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
    else if( eExpansion == LegendExpansion_WIDE )
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
    else // LegendExpansion_BALANCED
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
        return;

    // calculate maximum height for current row
    std::vector< sal_Int32 > nMaxHeights( nNumberOfRows );
    sal_Int32 nRow = 0;
    sal_Int32 nColumn = 0;
    for( ; nRow < nNumberOfRows; ++nRow )
    {
        sal_Int32 nMaxHeight = 0;
        for( nColumn = 0; nColumn < nNumberOfColumns; ++nColumn )
        {
            sal_Int32 nEntry = ( eExpansion == LegendExpansion_WIDE )
                ? (nColumn + nRow * nNumberOfColumns)
                // HIGH or BALANCED
                : (nRow + nColumn * nNumberOfRows);
            if( nEntry < nNumberOfEntries )
                nMaxHeight = ::std::max(
                    nMaxHeight, nYOffset + aTextShapes[ nEntry ]->getSize().Height );
        }
        nMaxHeights[ nRow ] = nMaxHeight;
    }

    // place entries ordered in optimal-width columns
    sal_Int32 nMaxYPos = 0;
    for( nColumn = 0; nColumn < nNumberOfColumns; ++nColumn )
    {
        sal_Int32 nMaxWidth = 0;
        nCurrentYPos = nYPadding;

        for( nRow = 0; nRow < nNumberOfRows; ++nRow )
        {
            sal_Int32 nEntry = ( eExpansion == LegendExpansion_WIDE )
                ? (nColumn + nRow * nNumberOfColumns)
                // HIGH or BALANCED
                : (nRow + nColumn * nNumberOfRows);

            if( nEntry >= nNumberOfEntries )
                break;

            // symbol
            Reference< drawing::XShape > xSymbol( rEntries[ nEntry ].aSymbol );

            if( xSymbol.is() )
            {
                // Note: aspect ratio should always be 3:2

                // set symbol size to 75% of maximum space
                awt::Size aSymbolSize(
                    aMaxSymbolExtent.Width  * 75 / 100,
                    aMaxSymbolExtent.Height * 75 / 100 );
                xSymbol->setSize( aSymbolSize );
                sal_Int32 nSymbolXPos = nCurrentXPos + ((aMaxSymbolExtent.Width - aSymbolSize.Width) / 2);
                if( !bSymbolsLeftSide )
                    nSymbolXPos = nSymbolXPos - aMaxSymbolExtent.Width;

                // #i109336# Improve auto positioning in chart
                sal_Int32 nTextHeight = nMaxHeights[ nRow ] - nYOffset;
                sal_Int32 nFontSize = static_cast< sal_Int32 >( fViewFontSize );
                sal_Int32 nMaxRowHeight = ( ( ( nTextHeight / nFontSize ) <= 1 ) ? nTextHeight : nFontSize );
                sal_Int32 nSymbolYPos = nCurrentYPos + ( ( nMaxRowHeight - aSymbolSize.Height ) / 2 );
                xSymbol->setPosition( awt::Point( nSymbolXPos, nSymbolYPos ) );
            }

            // position text shape
            awt::Size aTextSize( aTextShapes[ nEntry ]->getSize());
            nMaxWidth = ::std::max( nMaxWidth, 2 * nXOffset + aMaxSymbolExtent.Width + aTextSize.Width );
            sal_Int32 nTextXPos = nCurrentXPos + aMaxSymbolExtent.Width;
            if( !bSymbolsLeftSide )
                nTextXPos = nCurrentXPos - aMaxSymbolExtent.Width - aTextSize.Width;
            aTextShapes[ nEntry ]->setPosition( awt::Point( nTextXPos, nCurrentYPos ));

            nCurrentYPos += nMaxHeights[ nRow ];
            nMaxYPos = ::std::max( nMaxYPos, nCurrentYPos );
        }
        if( bSymbolsLeftSide )
            nCurrentXPos += nMaxWidth;
        else
            nCurrentXPos -= nMaxWidth;
    }

    if( bSymbolsLeftSide )
        rOutLegendSize.Width  = nCurrentXPos + nXPadding;
    else
    {
        sal_Int32 nLegendWidth = -(nCurrentXPos-nXPadding);
        rOutLegendSize.Width  = nLegendWidth;

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
    rOutLegendSize.Height = nMaxYPos + nYPadding;
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

template< class T >
void lcl_appendSeqToVector( const Sequence< T > & rSource, ::std::vector< T > & rDest )
{
    const sal_Int32 nCount = rSource.getLength();
    for( sal_Int32 i = 0; i < nCount; ++i )
        rDest.push_back( rSource[ i ] );
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
                if( (xLegendProp->getPropertyValue( C2U("WritingMode") ) >>= nWritingMode) )
                {
                    if( nWritingMode == text::WritingMode2::PAGE )
                        nWritingMode = nDefaultWritingMode;
                    if( nWritingMode == text::WritingMode2::RL_TB )
                        bSymbolsLeftSide=false;
                }
            }
        }
    }
    catch( uno::Exception & ex )
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

// ----------------------------------------

void VLegend::init(
    const Reference< drawing::XShapes >& xTargetPage,
    const Reference< lang::XMultiServiceFactory >& xFactory,
    const Reference< frame::XModel >& xModel )
{
    m_xTarget = xTargetPage;
    m_xShapeFactory = xFactory;
    m_xModel = xModel;
}

// ----------------------------------------

void VLegend::setDefaultWritingMode( sal_Int16 nDefaultWritingMode )
{
    m_nDefaultWritingMode = nDefaultWritingMode;
}

// ----------------------------------------

bool VLegend::isVisible( const Reference< XLegend > & xLegend )
{
    if( ! xLegend.is())
        return sal_False;

    sal_Bool bShow = sal_False;
    try
    {
        Reference< beans::XPropertySet > xLegendProp( xLegend, uno::UNO_QUERY_THROW );
        xLegendProp->getPropertyValue( C2U( "Show" )) >>= bShow;
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return bShow;
}

// ----------------------------------------

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
                          C2U( "com.sun.star.drawing.GroupShape" )), uno::UNO_QUERY );
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
                    C2U( "com.sun.star.drawing.RectangleShape" )), uno::UNO_QUERY );

            // for quickly setting properties
            tPropertyValues aLineFillProperties;
            tPropertyValues aTextProperties;

            // limit the width of texts to 30% of the total available width
            // #i109336# Improve auto positioning in chart
            sal_Int32 nMaxLabelWidth = rAvailableSpace.Width * 3 / 10;
            Reference< beans::XPropertySet > xLegendProp( m_xLegend, uno::UNO_QUERY );
            LegendExpansion eExpansion = LegendExpansion_HIGH;
            if( xLegendProp.is())
            {
                // get Expansion property
                xLegendProp->getPropertyValue( C2U( "Expansion" )) >>= eExpansion;
                if( eExpansion == LegendExpansion_WIDE )
                {
                    //#i80377#
                    nMaxLabelWidth = (rAvailableSpace.Width * 5) / 6;//completely correct would be available width minus symbol size minus distances to page ...
                }

                lcl_getProperties( xLegendProp, aLineFillProperties, aTextProperties, nMaxLabelWidth,
                                   rPageSize );
            }

            if( xBorder.is())
            {
                xLegendContainer->add( xBorder );

                // apply legend properties
                PropertyMapper::setMultiProperties(
                    aLineFillProperties.first, aLineFillProperties.second,
                    Reference< beans::XPropertySet >( xBorder, uno::UNO_QUERY ));

                //because of this name this border will be used for marking the legend
                ShapeFactory(m_xShapeFactory).setShapeName( xBorder, C2U("MarkHandles") );
            }

            // create entries
            tViewLegendEntryContainer aViewEntries;
            {
                ::std::vector< LegendEntryProvider* >::const_iterator       aIter = m_aLegendEntryProviderList.begin();
                const ::std::vector< LegendEntryProvider* >::const_iterator aEnd  = m_aLegendEntryProviderList.end();
                for( ; aIter != aEnd; aIter++ )
                {
                    LegendEntryProvider* pLegendEntryProvider( *aIter );
                    if( pLegendEntryProvider )
                    {
                        lcl_appendSeqToVector< ViewLegendEntry >(
                            pLegendEntryProvider->createLegendEntries( eExpansion, xLegendProp, xLegendContainer, m_xShapeFactory, m_xContext )
                            , aViewEntries );
                    }
                }
            }

            bool bSymbolsLeftSide = lcl_shouldSymbolsBePlacedOnTheLeftSide( xLegendProp, m_nDefaultWritingMode );

            // place entries
            awt::Size aLegendSize;
            lcl_placeLegendEntries( aViewEntries, eExpansion, bSymbolsLeftSide
                , xLegendProp, aTextProperties
                , xLegendContainer, m_xShapeFactory, m_xContext
                , rAvailableSpace, rPageSize, aLegendSize );

            if( xBorder.is())
                xBorder->setSize( aLegendSize );
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ----------------------------------------

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
            ! (xLegendProp->getPropertyValue( C2U( "RelativePosition" )) >>= aRelativePosition);

        LegendPosition ePos = LegendPosition_CUSTOM;
        xLegendProp->getPropertyValue( C2U( "AnchorPosition" )) >>= ePos;

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
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
