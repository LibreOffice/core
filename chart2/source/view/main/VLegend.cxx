/*************************************************************************
 *
 *  $RCSfile: VLegend.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-28 13:38:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "VLegend.hxx"
#include "macros.hxx"
#include "PropertyMapper.hxx"
#include "CommonConverters.hxx"
#include "VLegendSymbolFactory.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "LayoutHelper.hxx"

#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDATASERIESTREEPARENT_HPP_
#include <drafts/com/sun/star/chart2/XDataSeriesTreeParent.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTHORIZONTALADJUST_HPP_
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDATASOURCE_HPP_
#include <drafts/com/sun/star/chart2/XDataSource.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTTYPEGROUP_HPP_
#include <drafts/com/sun/star/chart2/XChartTypeGroup.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <drafts/com/sun/star/chart2/XIdentifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEJOINT_HPP_
#include <com/sun/star/drawing/LineJoint.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_LEGENDEXPANSION_HPP_
#include <drafts/com/sun/star/chart2/LegendExpansion.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_LAYOUT_XANCHOREDOBJECT_HPP_
#include <drafts/com/sun/star/layout/XAnchoredObject.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_LEGENDPOSITION_HPP_
#include <drafts/com/sun/star/chart2/LegendPosition.hpp>
#endif

// header for class Matrix3D
#ifndef _B2D_MATRIX3D_HXX
#include <goodies/matrix3d.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#include <vector>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace
{

struct LegendEntry
{
    uno::Reference< drawing::XShape >      xShape;
    uno::Reference< beans::XPropertySet >  xSymbolProperties;
    uno::Reference< chart2::XChartType >   xChartType;

    LegendEntry( uno::Reference< drawing::XShape > _shape,
                 uno::Reference< beans::XPropertySet > _prop,
                 uno::Reference< chart2::XChartType > _chart_type ) :
            xShape( _shape ),
            xSymbolProperties( _prop ),
            xChartType( _chart_type )
    {}
};
typedef ::std::vector< LegendEntry > tEntryGroup;

typedef ::std::pair< ::chart::tNameSequence, ::chart::tAnySequence > tPropertyValues;


::rtl::OUString lcl_DataToString( const uno::Sequence< uno::Any > & rSeq )
{
    sal_Int32 nMax = rSeq.getLength() - 1;
    ::rtl::OUString aVal;
    ::rtl::OUStringBuffer aBuf;
    for( sal_Int32 i = 0; i <= nMax; ++i )
    {
        if( rSeq[i] >>= aVal )
        {
            aBuf.append( aVal );
            if( i < nMax )
                aBuf.append( sal_Unicode( ' ' ));
        }
    }
    return aBuf.makeStringAndClear();
}

::rtl::OUString lcl_getLabelForSeries( const uno::Reference< chart2::XDataSource > & xSource )
{
    ::rtl::OUString aResult;

    if( xSource.is())
    {
        uno::Sequence< uno::Reference< chart2::XDataSequence > > aSeq( xSource->getDataSequences());
        for( sal_Int32 i=0; i < aSeq.getLength(); ++i )
        {
            uno::Reference< beans::XPropertySet > xProp( aSeq[i], uno::UNO_QUERY );
            ::rtl::OUString aRole;
            if( xProp.is() &&
                (xProp->getPropertyValue( C2U( "Role" )) >>= aRole) )
            {
                if( aRole.equals( C2U( "label" )))
                {
                    aResult = lcl_DataToString( aSeq[i]->getData() );
                    break;
                }
            }
        }
    }

    return aResult;
}

void lcl_getProperties(
    const uno::Reference< beans::XPropertySet > & xLegendProp,
    tPropertyValues & rOutLineFillProperties,
    tPropertyValues & rOutTextProperties,
    sal_Int32 nMaxLabelWidth )
{
    // Get Line- and FillProperties from model legend
    if( xLegendProp.is())
    {
        // set rOutLineFillProperties
        ::chart::tPropertyNameValueMap aLineFillValueMap;
        ::chart::tMakePropertyNameMap aNameMap = ::chart::PropertyMapper::getPropertyNameMapForFillProperties();
        const ::chart::tMakePropertyNameMap& rLinePropMap = ::chart::PropertyMapper::getPropertyNameMapForLineProperties();
        aNameMap.insert( rLinePropMap.begin(), rLinePropMap.end());
        ::chart::PropertyMapper::getValueMap( aLineFillValueMap, aNameMap, xLegendProp );

        aLineFillValueMap[ C2U("LineJoint") ] = uno::makeAny( drawing::LineJoint_ROUND );

        ::chart::PropertyMapper::getMultiPropertyListsFromValueMap(
            rOutLineFillProperties.first, rOutLineFillProperties.second, aLineFillValueMap );

        // set rOutTextProperties
        ::chart::tPropertyNameValueMap aTextValueMap;
        ::chart::tMakePropertyNameMap aCharNameMap = ::chart::PropertyMapper::getPropertyNameMapForCharacterProperties();
        ::chart::PropertyMapper::getValueMap( aTextValueMap, aCharNameMap, xLegendProp );

        drawing::TextHorizontalAdjust eHorizAdjust( drawing::TextHorizontalAdjust_LEFT );
        aTextValueMap[ C2U("TextAutoGrowHeight") ] = uno::makeAny( sal_True );
        aTextValueMap[ C2U("TextAutoGrowWidth") ] = uno::makeAny( sal_True );
        aTextValueMap[ C2U("TextHorizontalAdjust") ] = uno::makeAny( eHorizAdjust );
        aTextValueMap[ C2U("TextMaximumFrameWidth") ] = uno::makeAny( nMaxLabelWidth );

        ::chart::PropertyMapper::getMultiPropertyListsFromValueMap(
            rOutTextProperties.first, rOutTextProperties.second, aTextValueMap );
    }
}

/** creates a symbol shape that is returned and added to the given
    xShapeContainer The adding to the container may have to be done here, if you
    use group shapes, because you can only add shapes to a group that itself has
    been added to an XShapes before.
 */
uno::Reference< drawing::XShape >
    lcl_getSymbol(
    const uno::Reference< chart2::XChartType > & xChartType,
    const uno::Reference< beans::XPropertySet > & xSeriesProp,
    const uno::Reference< lang::XMultiServiceFactory > & xFact,
    const uno::Reference< drawing::XShapes > & xShapeContainer )
{
    uno::Reference< drawing::XShape > xResult;

    if( xChartType.is())
    {
        ::rtl::OUString aChartType( xChartType->getChartType());

        xResult.set( xFact->createInstance(
                         C2U( "com.sun.star.drawing.GroupShape" )), uno::UNO_QUERY );
        xShapeContainer->add( xResult );
        uno::Reference< drawing::XShapes > xGroup( xResult, uno::UNO_QUERY );
        if( ! xGroup.is())
            return uno::Reference< drawing::XShape >();

        xShapeContainer->add( xResult );

        chart2::LegendSymbolStyle eSymbolStyle = chart2::LegendSymbolStyle_BOX;

        // todo: offer UNO components that support the given service-name
        // and are capable of creating data representations as well as
        // legend symbols

        if( aChartType.equals( C2U( "com.sun.star.chart2.BarChart" )) ||
            aChartType.equals( C2U( "com.sun.star.chart2.AreaChart" )))
        {
            eSymbolStyle = chart2::LegendSymbolStyle_BOX;
        }
        else if( aChartType.equals( C2U( "com.sun.star.chart2.LineChart" )))
        {
            eSymbolStyle = chart2::LegendSymbolStyle_LINE;
        }
        else if( aChartType.equals( C2U( "com.sun.star.chart2.PieChart" )))
        {
            eSymbolStyle = chart2::LegendSymbolStyle_CIRCLE;
        }
        else if( aChartType.equals( C2U( "com.sun.star.chart2.NetChart" )))
        {
            eSymbolStyle = chart2::LegendSymbolStyle_DIAGONAL_LINE;
        }

        ::chart::VLegendSymbolFactory::createSymbol( xGroup, eSymbolStyle, xFact, xSeriesProp );
    }
    else
    {
        OSL_ENSURE( false, "No ChartTypeGroup!" );
    }

    return xResult;
}

/** Note: rOutMinExtentSoFar is the smallest non-zero size
 */
void lcl_getLegendEntries(
    const uno::Reference< chart2::XDataSeriesTreeParent > & xParent,
    tEntryGroup & rOutEntryContainer,
    const uno::Reference< drawing::XShapes > & xTarget,
    const uno::Reference< lang::XMultiServiceFactory > & xShapeFactory,
    const uno::Reference< chart2::XChartType > & xChartType,
    const tPropertyValues & rProperties,
    awt::Size & rOutMinExtentSoFar,
    awt::Size & rOutMaxExtentSoFar )
{
    uno::Sequence< uno::Reference< chart2::XDataSeriesTreeNode > > aChildren(
        xParent->getChildren());

    for( sal_Int32 nI = 0; nI < aChildren.getLength(); ++nI )
    {
        uno::Reference< chart2::XDataSeriesTreeParent > xNewParent( aChildren[ nI ], uno::UNO_QUERY );
        if( xNewParent.is())
        {
            // recurse !
            lcl_getLegendEntries( xNewParent, rOutEntryContainer, xTarget, xShapeFactory,
                                  xChartType, rProperties, rOutMinExtentSoFar, rOutMaxExtentSoFar );
        }
        else
        {
            try
            {
                // create label shape
                uno::Reference< drawing::XShape > xEntry(
                    xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.TextShape" )), uno::UNO_QUERY );
                if( !xEntry.is())
                    continue;

                xTarget->add( xEntry );
                // get label text via data source
                ::rtl::OUString aName;
                uno::Reference< chart2::XDataSource > xSeriesSource(
                    aChildren[ nI ], uno::UNO_QUERY );
                if( xSeriesSource.is())
                    aName = lcl_getLabelForSeries( xSeriesSource );

                // set label text
                uno::Reference< text::XTextRange > xRange( xEntry, uno::UNO_QUERY );
                if( xRange.is())
                    xRange->setString( aName );

                // set character properties
                ::chart::PropertyMapper::setMultiProperties(
                    rProperties.first, rProperties.second,
                    uno::Reference< beans::XPropertySet >( xEntry,uno::UNO_QUERY ) );

                // adapt min-/max-extent
                awt::Size aEntrySize( xEntry->getSize() );
                rOutMaxExtentSoFar.Width =  ::std::max( rOutMaxExtentSoFar.Width,  aEntrySize.Width );
                rOutMaxExtentSoFar.Height = ::std::max( rOutMaxExtentSoFar.Height, aEntrySize.Height );
                if( aEntrySize.Height > 0 )
                {
                    // setting initial value (otherwise (0,0) is always the minimum)
                    if( rOutMinExtentSoFar.Height == 0 )
                        rOutMinExtentSoFar = aEntrySize;
                    else
                    {
                        rOutMinExtentSoFar.Width =  ::std::min( rOutMinExtentSoFar.Width,  aEntrySize.Width );
                        rOutMinExtentSoFar.Height = ::std::min( rOutMinExtentSoFar.Height, aEntrySize.Height );
                    }
                }

                // add entry to list
                uno::Reference< beans::XPropertySet > xSeriesProp( xSeriesSource, uno::UNO_QUERY );
                rOutEntryContainer.push_back(
                    tEntryGroup::value_type( xEntry, xSeriesProp, xChartType ));
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
}

void lcl_createLegend(
    const tEntryGroup & rEntries,
    chart2::LegendExpansion eExpansion,
    const uno::Reference< drawing::XShapes > & xTarget,
    const uno::Reference< lang::XMultiServiceFactory > & xShapeFactory,
    const awt::Size & rAvailableSpace,
    awt::Size aMinEntryExtent,
    awt::Size aMaxEntryExtent,
    awt::Size & rOutSize
    )
{
    const sal_Int32 nXPadding = 200;
    const sal_Int32 nYPadding = 250;
    const sal_Int32 nXOffset = 100;
    const sal_Int32 nYOffset = 100;

    awt::Size aMaxSymbolExtent( aMinEntryExtent.Height * 3/2, aMinEntryExtent.Height );
    sal_Int32 nCurrentXPos = nXPadding;
    sal_Int32 nCurrentYPos = nYPadding;
    sal_Int32 nMaxEntryWidth = 2 * nXOffset + aMaxSymbolExtent.Width + aMaxEntryExtent.Width;
    sal_Int32 nMaxEntryHeight = nYOffset + aMaxEntryExtent.Height;
    sal_Int32 nNumberOfEntries = rEntries.size();

    sal_Int32 nNumberOfColumns = 0, nNumberOfRows = 0;

    // determine layout depending on LegendExpansion
    if( eExpansion == chart2::LegendExpansion_HIGH )
    {
        sal_Int32 nMaxNumberOfRows = (rAvailableSpace.Height - 2*nYPadding ) / nMaxEntryHeight;

        nNumberOfColumns = static_cast< sal_Int32 >(
            ceil( static_cast< double >( nNumberOfEntries ) /
                  static_cast< double >( nMaxNumberOfRows ) ));
        nNumberOfRows =  static_cast< sal_Int32 >(
            ceil( static_cast< double >( nNumberOfEntries ) /
                  static_cast< double >( nNumberOfColumns ) ));
    }
    else if( eExpansion == chart2::LegendExpansion_WIDE )
    {
        sal_Int32 nMaxNumberOfColumns = (rAvailableSpace.Width - 2*nXPadding ) / nMaxEntryWidth;

        nNumberOfRows = static_cast< sal_Int32 >(
            ceil( static_cast< double >( nNumberOfEntries ) /
                  static_cast< double >( nMaxNumberOfColumns ) ));
        nNumberOfColumns = static_cast< sal_Int32 >(
            ceil( static_cast< double >( nNumberOfEntries ) /
                  static_cast< double >( nNumberOfRows ) ));
    }
    else // chart2::LegendExpansion_BALANCED
    {
        double fAspect =
            static_cast< double >( nMaxEntryWidth ) / static_cast< double >( nMaxEntryHeight );

        nNumberOfRows = static_cast< sal_Int32 >(
            ceil( sqrt( static_cast< double >( nNumberOfEntries ) * fAspect )));
        nNumberOfColumns = static_cast< sal_Int32 >(
            ceil( static_cast< double >( nNumberOfEntries ) /
                  static_cast< double >( nNumberOfRows ) ));
    }

    // calculate maximum height for current row
    std::vector< sal_Int32 > nMaxHeights( nNumberOfRows );
    sal_Int32 nRow = 0;
    sal_Int32 nColumn = 0;
    for( ; nRow < nNumberOfRows; ++nRow )
    {
        sal_Int32 nMaxHeight = 0;
        for( nColumn = 0; nColumn < nNumberOfColumns; ++nColumn )
        {
            sal_Int32 nEntry = ( eExpansion == chart2::LegendExpansion_WIDE )
                ? (nColumn + nRow * nNumberOfColumns)
                // HIGH or BALANCED
                : (nRow + nColumn * nNumberOfRows);
            if( nEntry < nNumberOfEntries )
                nMaxHeight = ::std::max(
                    nMaxHeight, nYOffset + rEntries[ nEntry ].xShape->getSize().Height );
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
            sal_Int32 nEntry = ( eExpansion == chart2::LegendExpansion_WIDE )
                ? (nColumn + nRow * nNumberOfColumns)
                // HIGH or BALANCED
                : (nRow + nColumn * nNumberOfRows);

            if( nEntry >= nNumberOfEntries )
                break;

            const LegendEntry & rEntry = rEntries[ nEntry ];

            // symbol
            uno::Reference< drawing::XShape > xSymbol(
                lcl_getSymbol( rEntry.xChartType,
                               rEntry.xSymbolProperties,
                               xShapeFactory,
                               xTarget ));

            if( xSymbol.is())
            {
                // Note: aspect ratio should always be 3:2

                // set symbol size to 75% of maximum space
                awt::Size aSymbolSize(
                    aMaxSymbolExtent.Width  * 75 / 100,
                    aMaxSymbolExtent.Height * 75 / 100 );
                xSymbol->setSize( aSymbolSize );
                xSymbol->setPosition(
                    awt::Point(
                        nCurrentXPos + ((aMaxSymbolExtent.Width - aSymbolSize.Width) / 2),
                        nCurrentYPos + ((aMaxSymbolExtent.Height - aSymbolSize.Height) / 2)));
            }

            // position text shape
            awt::Size aTextSize( rEntry.xShape->getSize());
            nMaxWidth = ::std::max(
                nMaxWidth, 2 * nXOffset + aMaxSymbolExtent.Width + aTextSize.Width );
            rEntry.xShape->setPosition(
                awt::Point( nCurrentXPos + aMaxSymbolExtent.Width, nCurrentYPos ));

            nCurrentYPos += nMaxHeights[ nRow ];
            nMaxYPos = ::std::max( nMaxYPos, nCurrentYPos );
        }
        nCurrentXPos += nMaxWidth;
    }

    rOutSize.Width  = nCurrentXPos + nXPadding;
    rOutSize.Height = nMaxYPos + nYPadding;
}

} // anonymous namespace

//.............................................................................
namespace chart
{
//.............................................................................

VLegend::VLegend(
    const uno::Reference< chart2::XLegend > & xLegend ) :
        m_xLegend( xLegend )
{
}

// ----------------------------------------

void SAL_CALL VLegend::init(
    const uno::Reference< drawing::XShapes >& xTargetPage,
    const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    m_xTarget = xTargetPage;
    m_xShapeFactory = xFactory;
}

// ----------------------------------------

// static
bool VLegend::isVisible( const uno::Reference< chart2::XLegend > & xLegend )
{
    if( ! xLegend.is())
        return sal_False;

    sal_Bool bShow = sal_False;
    try
    {
        uno::Reference< beans::XPropertySet > xLegendProp( xLegend, uno::UNO_QUERY_THROW );
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
    const awt::Size & rAvailableSpace )
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

        // set Name
        {
            uno::Reference< beans::XPropertySet > xShapeProp( m_xShape, uno::UNO_QUERY );
            uno::Reference< chart2::XIdentifiable > xLegendIdent( m_xLegend, uno::UNO_QUERY );
            if( xShapeProp.is() &&
                xLegendIdent.is())
            {
                rtl::OUString aCID = ObjectIdentifier::createClassifiedIdentifier(
                    OBJECTTYPE_LEGEND, xLegendIdent->getIdentifier() );
                xShapeProp->setPropertyValue( C2U("Name"), uno::makeAny( aCID ));
            }
        }

        // create and insert sub-shapes
        uno::Reference< drawing::XShapes > xLegendContainer( m_xShape, uno::UNO_QUERY );
        if( xLegendContainer.is())
        {
            uno::Reference< drawing::XShape > xBorder(
                m_xShapeFactory->createInstance(
                    C2U( "com.sun.star.drawing.RectangleShape" )), uno::UNO_QUERY );

            // for quickly setting properties
            tPropertyValues aLineFillProperties;
            tPropertyValues aTextProperties;

            uno::Reference< beans::XPropertySet > xLegendProp( m_xLegend, uno::UNO_QUERY );
            chart2::LegendExpansion eExpansion = chart2::LegendExpansion_HIGH;
            float fMaxFontHeight = 6.0;
            if( xLegendProp.is())
            {
                // limit the width of texts to 20% of the total available width
                sal_Int32 nMaxLabelWidth = rAvailableSpace.Width / 5;
                lcl_getProperties( xLegendProp, aLineFillProperties, aTextProperties, nMaxLabelWidth );

                // get Expansion property
                xLegendProp->getPropertyValue( C2U( "Expansion" )) >>= eExpansion;
                xLegendProp->getPropertyValue( C2U( "CharHeight" )) >>= fMaxFontHeight;
            }

            if( xBorder.is())
            {
                xLegendContainer->add( xBorder );

                // apply legend properties
                PropertyMapper::setMultiProperties(
                    aLineFillProperties.first, aLineFillProperties.second,
                    uno::Reference< beans::XPropertySet >( xBorder, uno::UNO_QUERY ));
            }

            // create entries
            uno::Sequence< uno::Reference< chart2::XLegendEntry > > aEntries( m_xLegend->getEntries());
            tEntryGroup aEntryGroup;
            awt::Size aMinEntryExtent;
            awt::Size aMaxEntryExtent;

            for( sal_Int32 nI = 0; nI < aEntries.getLength(); ++nI )
            {
                uno::Reference< chart2::XDataSeriesTreeParent > xGroup( aEntries[ nI ], uno::UNO_QUERY );
                if( xGroup.is())
                {
                    uno::Reference< chart2::XChartTypeGroup > xCTGroup( xGroup, uno::UNO_QUERY );
                    uno::Reference< chart2::XChartType > xChartType;
                    if( xCTGroup.is())
                        xChartType.set( xCTGroup->getChartType());

                    lcl_getLegendEntries( xGroup, aEntryGroup, xLegendContainer,
                                          m_xShapeFactory, xChartType,
                                          aTextProperties,
                                          aMinEntryExtent, aMaxEntryExtent );
                }
            }

            // place entries
            awt::Size aLegendSize;
            lcl_createLegend( aEntryGroup, eExpansion,
                              xLegendContainer, m_xShapeFactory,
                              rAvailableSpace, aMinEntryExtent, aMaxEntryExtent,
                              aLegendSize );

            if( xBorder.is())
                xBorder->setSize( aLegendSize );

            m_aBoundRect.Width  = aLegendSize.Width;
            m_aBoundRect.Height = aLegendSize.Height;
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
    const awt::Size & rReferenceSize )
{
    if(! m_xShape.is())
        return;

    try
    {
        // determine position and alignment depending on anchor
        uno::Reference< layout::XAnchoredObject > xAnchObj( m_xLegend, uno::UNO_QUERY_THROW );
        layout::AnchorPoint aAnchor( xAnchObj->getAnchor());
        layout::RelativePoint aOffset( xAnchObj->getRelativePosition());

        helper::LayoutHelper::rotatePoint( aAnchor.EscapeDirection, aOffset.Primary, aOffset.Secondary );

        sal_Int32 nOffsetX = static_cast< sal_Int32 >(
            aOffset.Primary * rReferenceSize.Width );
        // the standard angles are in a coordinate system where y goes up, in
        // the drawing layer y goes down, so negate it
        sal_Int32 nOffsetY = - static_cast< sal_Int32 >(
            aOffset.Secondary * rReferenceSize.Height );

        awt::Point aPos( static_cast< sal_Int32 >(
                             rOutAvailableSpace.X +
                             rOutAvailableSpace.Width * aAnchor.Alignment.Primary) +
                             nOffsetX,
                         static_cast< sal_Int32 >(
                             rOutAvailableSpace.Y +
                             rOutAvailableSpace.Height * aAnchor.Alignment.Secondary) +
                             nOffsetY );
        layout::Alignment aAlignment( helper::LayoutHelper::getStandardAlignmentByAngle(
                                          aAnchor.EscapeDirection ));

        // set position according to Alignment
        aPos.X -= static_cast< sal_Int32 >(
            ::rtl::math::round( aAlignment.Primary * static_cast< double >( m_aBoundRect.Width )));
        aPos.Y -= static_cast< sal_Int32 >(
            ::rtl::math::round( aAlignment.Secondary * static_cast< double >( m_aBoundRect.Height )));

        m_xShape->setPosition( aPos );

        m_aBoundRect.X = aPos.X;
        m_aBoundRect.Y = aPos.Y;

        // adapt rOutAvailableSpace if LegendPosition is not CUSTOM
        chart2::LegendPosition ePos = chart2::LegendPosition_CUSTOM;
        uno::Reference< beans::XPropertySet > xLegendProp( m_xLegend, uno::UNO_QUERY_THROW );
        xLegendProp->getPropertyValue( C2U( "Position" )) >>= ePos;

        switch( ePos )
        {
            case chart2::LegendPosition_LINE_START:
            {
                sal_Int32 nExtent = m_aBoundRect.Width + (m_aBoundRect.X - rOutAvailableSpace.X)
                    + nOffsetX;
                rOutAvailableSpace.Width -= nExtent;
                rOutAvailableSpace.X += nExtent;
            }
            break;
            case chart2::LegendPosition_LINE_END:
                rOutAvailableSpace.Width = (m_aBoundRect.X + nOffsetX) - rOutAvailableSpace.X;
                break;
            case chart2::LegendPosition_PAGE_START:
            {
                sal_Int32 nExtent = m_aBoundRect.Height + (m_aBoundRect.Y - rOutAvailableSpace.Y)
                    + nOffsetY;
                rOutAvailableSpace.Height -= nExtent;
                rOutAvailableSpace.Y += nExtent;
            }
            break;
            case chart2::LegendPosition_PAGE_END:
                rOutAvailableSpace.Height = (m_aBoundRect.Y + nOffsetY) - rOutAvailableSpace.Y;
                break;

            default:
                // nothing
                break;
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
