/*************************************************************************
 *
 *  $RCSfile: VLegend.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-14 17:17:09 $
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
#ifndef _COM_SUN_STAR_DRAWING_TEXTVERTICALADJUST_HPP_
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
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

    LegendEntry( uno::Reference< drawing::XShape > _shape,
                 uno::Reference< beans::XPropertySet > _prop ) :
            xShape( _shape ),
            xSymbolProperties( _prop )
    {}
};
typedef ::std::vector< LegendEntry > tEntryGroup;

struct LegendEntryGroup
{
    tEntryGroup                           aEntryGroup;
    awt::Size                             aMaxExtent;
    uno::Reference< chart2::XChartType >  xChartType;
};
typedef ::std::vector< LegendEntryGroup > tEntryGroupContainer;

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

void lcl_getLegendEntries(
    const uno::Reference< chart2::XDataSeriesTreeParent > & xParent,
    tEntryGroup & rOutEntryContainer,
    const uno::Reference< drawing::XShapes > & xTarget,
    const uno::Reference< lang::XMultiServiceFactory > & xShapeFactory,
    const tPropertyValues & rProperties,
    awt::Size & rOutMaxExtentSoFar )
{
    uno::Sequence< uno::Reference< chart2::XDataSeriesTreeNode > > aChildren(
        xParent->getChildren());

    for( sal_Int32 nI = 0; nI < aChildren.getLength(); ++nI )
    {
        uno::Reference< chart2::XDataSeriesTreeParent > xNewParent( aChildren[ nI ], uno::UNO_QUERY );
        if( xNewParent.is())
        {
            // If there should be lines between two stacking groups, this would
            // be the place to create and place them.

            // recurse !
            lcl_getLegendEntries( xNewParent, rOutEntryContainer, xTarget, xShapeFactory,
                                  rProperties, rOutMaxExtentSoFar );
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
                    rProperties.first, rProperties.second, xEntry );

                // adapt max-extent
                awt::Size aEntrySize( xEntry->getSize() );
                rOutMaxExtentSoFar.Width = ::std::max(  rOutMaxExtentSoFar.Width,  aEntrySize.Width );
                rOutMaxExtentSoFar.Height = ::std::max( rOutMaxExtentSoFar.Height, aEntrySize.Height );

                // add entry to list
                uno::Reference< beans::XPropertySet > xSeriesProp( xSeriesSource, uno::UNO_QUERY );
                rOutEntryContainer.push_back(
                    tEntryGroup::value_type( xEntry, xSeriesProp ));
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
}

void lcl_getProperties(
    const uno::Reference< beans::XPropertySet > & xLegendProp,
    tPropertyValues & rOutLineFillProperties,
    tPropertyValues & rOutTextProperties )
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
        drawing::TextVerticalAdjust eVertAdjust( drawing::TextVerticalAdjust_TOP );
        aTextValueMap[ C2U("TextAutoGrowHeight") ] = uno::makeAny( sal_True );
        aTextValueMap[ C2U("TextAutoGrowWidth") ] = uno::makeAny( sal_True );
        aTextValueMap[ C2U("TextHorizontalAdjust") ] = uno::makeAny( eHorizAdjust );
        aTextValueMap[ C2U("TextVerticalAdjust") ] = uno::makeAny( eVertAdjust );

        ::chart::PropertyMapper::getMultiPropertyListsFromValueMap(
            rOutTextProperties.first, rOutTextProperties.second, aTextValueMap );
    }
}


} // anonymous namespace

//.............................................................................
namespace chart
{
//.............................................................................

VLegend::VLegend(
    const uno::Reference< chart2::XLegend > & xLegend ) :
        m_xLegend( xLegend )
//         m_aMaxSize( -1, -1 )
{
}

void SAL_CALL VLegend::init(
    const uno::Reference< drawing::XShapes >& xTargetPage,
    const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    m_xTarget = xTargetPage;
    m_xShapeFactory = xFactory;
}

void VLegend::createShapes(
    const awt::Size & rAvailableSpace )
{
    if(! (m_xLegend.is() &&
          m_xShapeFactory.is() &&
          m_xTarget.is()))
        return;

    try
    {
        awt::Size aCurrentExtent;
        chart2::LegendExpansion eExpansion = chart2::LegendExpansion_HIGH;
        tEntryGroupContainer aEntryContainer;

        //create shape and add to page
        m_xShape.set(
            m_xShapeFactory->createInstance(
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
            if( xLegendProp.is())
            {
                lcl_getProperties( xLegendProp, aLineFillProperties, aTextProperties );

                // get Expansion property
                xLegendProp->getPropertyValue( C2U( "Expansion" )) >>= eExpansion;
            }

            if( xBorder.is())
            {
                xLegendContainer->add( xBorder );

                // apply legend properties
                PropertyMapper::setMultiProperties(
                    aLineFillProperties.first, aLineFillProperties.second, xBorder );
            }

            // create a group containing symbol and text
            uno::Reference< drawing::XShape > xLegendEntryShape(
                m_xShapeFactory->createInstance(
                    C2U( "com.sun.star.drawing.GroupShape" )), uno::UNO_QUERY );
            xLegendContainer->add( xLegendEntryShape );
            uno::Reference< drawing::XShapes > xLegendEntryGroup(
                xLegendEntryShape, uno::UNO_QUERY );
            OSL_ASSERT( xLegendEntryGroup.is());

            // create entries
            uno::Sequence< uno::Reference< chart2::XLegendEntry > > aEntries( m_xLegend->getEntries());
            const sal_Int32 nNumOfChartTypes = aEntries.getLength();

            std::vector< uno::Reference< drawing::XShape > > aSeparators;

            for( sal_Int32 nI = 0; nI < nNumOfChartTypes; ++nI )
            {
                uno::Reference< chart2::XDataSeriesTreeParent > xGroup( aEntries[ nI ], uno::UNO_QUERY );
                if( xGroup.is())
                {
                    aEntryContainer.resize( aEntryContainer.size() + 1 );
                    lcl_getLegendEntries( xGroup, aEntryContainer.back().aEntryGroup,
                                          xLegendEntryGroup,
                                          m_xShapeFactory,
                                          aTextProperties, aEntryContainer.back().aMaxExtent );

                    uno::Reference< chart2::XChartTypeGroup > xCTGroup( xGroup, uno::UNO_QUERY );
                    if( xCTGroup.is())
                        aEntryContainer.back().xChartType = xCTGroup->getChartType();
                }
            }

            // place entries
//             const sal_Int32 nVerticalPadding = 200;
//             const sal_Int32 nHorizontalPadding = 200;
            const sal_Int32 nSeparatorDist = 200;
            const sal_Int32 nXOffset = 100;
            const sal_Int32 nYOffset = 100;

//             sal_Int32 nCurrentWidth = aSize.Width;
//             sal_Int32 nCurrentHeight = nVerticalPadding;

            for( tEntryGroupContainer::const_iterator aGroupIt = aEntryContainer.begin();
                 aGroupIt != aEntryContainer.end(); ++aGroupIt )
            {
                if( eExpansion == chart2::LegendExpansion_HIGH )
                {
                    awt::Size aMaxExtent( (*aGroupIt).aMaxExtent );
                    awt::Size aMaxSymbolSpace( aMaxExtent.Height * 3 / 2, aMaxExtent.Height );
                    sal_Int32 nCurrentYPos = nYOffset; // nVerticalPadding;

                    for( tEntryGroup::const_iterator aEntryIt = ((*aGroupIt).aEntryGroup).begin();
                         aEntryIt != ((*aGroupIt).aEntryGroup).end(); ++aEntryIt )
                    {

                        // symbol
                        uno::Reference< drawing::XShape > xSymbol(
                            lcl_getSymbol( (*aGroupIt).xChartType,
                                           (*aEntryIt).xSymbolProperties,
                                           m_xShapeFactory,
                                           xLegendEntryGroup ));

//                         sal_Int32 nSymbolHeight = 0;
//                         sal_Int32 nDiff = 0;

                        if( xSymbol.is())
                        {
                            // Note: aspect ratio should always be 3:2

                            // set symbol size to 75% of maximum space
                            awt::Size aSymbolSize(
                                aMaxSymbolSpace.Width  * 75 / 100,
                                aMaxSymbolSpace.Height * 75 / 100 );
                            xSymbol->setSize( aSymbolSize );
                            xSymbol->setPosition(
                                awt::Point(
                                    nXOffset + ((aMaxSymbolSpace.Width - aSymbolSize.Width) / 2),
                                    nCurrentYPos + ((aMaxSymbolSpace.Height - aSymbolSize.Height) / 2)));
                        }

                        // position text shape
                        (*aEntryIt).xShape->setPosition(
                            awt::Point( 2 * nXOffset + aMaxSymbolSpace.Width, nCurrentYPos ));

                        nCurrentYPos += (aMaxExtent.Height + nYOffset);
                    }

                    aCurrentExtent.Width = ::std::max(
                        3 * nXOffset + aMaxSymbolSpace.Width + aMaxExtent.Width,
                        aCurrentExtent.Width );
                    aCurrentExtent.Height = nCurrentYPos;
                }
                else
                {
                    awt::Size aMaxExtent( (*aGroupIt).aMaxExtent );
                    awt::Size aMaxSymbolSpace( aMaxExtent.Height * 3 / 2, aMaxExtent.Height );
                    sal_Int32 nCurrentXPos = nXOffset;

                    for( tEntryGroup::const_iterator aEntryIt = ((*aGroupIt).aEntryGroup).begin();
                         aEntryIt != ((*aGroupIt).aEntryGroup).end(); ++aEntryIt )
                    {

                        // symbol
                        uno::Reference< drawing::XShape > xSymbol(
                            lcl_getSymbol( (*aGroupIt).xChartType,
                                           (*aEntryIt).xSymbolProperties,
                                           m_xShapeFactory,
                                           xLegendEntryGroup ));

//                         sal_Int32 nSymbolHeight = 0;
//                         sal_Int32 nDiff = 0;

                        if( xSymbol.is())
                        {
                            // Note: aspect ratio should always be 3:2

                            // set symbol size to 75% of maximum space
                            awt::Size aSymbolSize(
                                aMaxSymbolSpace.Width  * 75 / 100,
                                aMaxSymbolSpace.Height * 75 / 100 );
                            xSymbol->setSize( aSymbolSize );
                            xSymbol->setPosition(
                                awt::Point(
                                    nCurrentXPos + ((aMaxSymbolSpace.Width - aSymbolSize.Width) / 2),
                                    nYOffset + ((aMaxSymbolSpace.Height - aSymbolSize.Height) / 2)));
                        }

                        // position text shape
                        (*aEntryIt).xShape->setPosition(
                            awt::Point( nCurrentXPos + aMaxSymbolSpace.Width, nYOffset ));

                        nCurrentXPos += 3 * nXOffset + aMaxSymbolSpace.Width +
                            (*aEntryIt).xShape->getSize().Width;
                    }

                    aCurrentExtent.Width = nCurrentXPos;
                    aCurrentExtent.Height = 2 * nYOffset /*nVerticalPadding*/ + aMaxExtent.Height;
                }

                // separator between chart type groups
//                 if( nI < nNumOfChartTypes - 1 )
//                 {
//                     uno::Reference< drawing::XShape > xSeparator(
//                         m_xShapeFactory->createInstance(
//                             C2U( "com.sun.star.drawing.LineShape" )), uno::UNO_QUERY );

//                     if( xSeparator.is())
//                     {
//                         sal_Int32 nLineWidth = 0;
//                         if( xLegendProp.is())
//                             xLegendProp->getPropertyValue( C2U("LineWidth")) >>= nLineWidth;

//                         nCurrentHeight += nSeparatorDist + nLineWidth/2;
//                         xLegendContainer->add( xSeparator );
//                         // correct resizing is done later, when the required size is known
//                         if( eExpansion == chart2::LegendExpansion_HIGH )
//                         {
//                             xSeparator->setSize( awt::Size( 100, 0 ));
//                             xSeparator->setPosition( awt::Point( 0, nCurrentHeight ));
//                         }
//                         else
//                         {
//                             xSeparator->setSize( awt::Size( 0, 100 ));
//                             xSeparator->setPosition( awt::Point( nCurrentWidth, 0 ));
//                         }
//                         nCurrentHeight += nSeparatorDist + nLineWidth/2;

//                         // apply legend properties
//                         PropertyMapper::setMultiProperties(
//                             aLineFillProperties.first, aLineFillProperties.second, xSeparator );

//                         aSeparators.push_back( xSeparator );
//                     }
//                 }
            }

//             aSize.Height = nCurrentHeight + nVerticalPadding;
//             aSize.Width = nCurrentWidth;
            if( xBorder.is())
                xBorder->setSize( aCurrentExtent );

            // post-process separators (set correct size)
//             for( std::vector< uno::Reference< drawing::XShape > >::const_iterator aSepIt = aSeparators.begin();
//                  aSepIt != aSeparators.end(); ++aSepIt )
//             {
//                 if( (*aSepIt).is())
//                 {
//                     if( eExpansion == chart2::LegendExpansion_HIGH )
//                         (*aSepIt)->setSize( ::awt::Size( aSize.Width, 0 ));
//                     else
//                         (*aSepIt)->setSize( ::awt::Size( 0, aSize.Height ));
//                 }
//             }
        }

        m_aBoundRect.Width = aCurrentExtent.Width;
        m_aBoundRect.Height = aCurrentExtent.Height;
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void VLegend::changePosition(
    const awt::Point & rPos,
    const layout::Alignment& rAlignment )
{
    if(! m_xShape.is())
        return;

    m_aBoundRect.X = rPos.X;
    m_aBoundRect.Y = rPos.Y;
    awt::Point aUpperLeft( rPos );
    aUpperLeft.X -= static_cast< sal_Int32 >(
        ::rtl::math::round( rAlignment.Primary * static_cast< double >( m_aBoundRect.Width )));
    aUpperLeft.Y -= static_cast< sal_Int32 >(
        ::rtl::math::round( rAlignment.Secondary * static_cast< double >( m_aBoundRect.Height )));

    m_xShape->setPosition( aUpperLeft );
}

awt::Size VLegend::getSize() const
{
    return awt::Size( m_aBoundRect.Width, m_aBoundRect.Height );
}

//.............................................................................
} //namespace chart
//.............................................................................
