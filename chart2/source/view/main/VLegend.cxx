/*************************************************************************
 *
 *  $RCSfile: VLegend.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-08 17:40:39 $
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
#include "CommonConverters.hxx"

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

// header for class Matrix3D
#ifndef _B2D_MATRIX3D_HXX
#include <goodies/matrix3d.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace
{

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

} // anonymous namespace

//.............................................................................
namespace chart
{
//.............................................................................

VLegend::VLegend(
    const uno::Reference< chart2::XLegend > & xLegend ) :
        m_xLegend( xLegend ),
        m_aMaxSize( -1, -1 )
{
}

void SAL_CALL VLegend::init(
    const uno::Reference< drawing::XShapes >& xTargetPage,
    const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    m_xTarget = xTargetPage;
    m_xShapeFactory = xFactory;
}

void VLegend::createLegendEntries(
    const uno::Reference< drawing::XShapes > & xShapeContainer,
    const uno::Reference< chart2::XDataSeriesTreeParent > & xParent,
    sal_Int32 & nOutCurrentHeight )
{
    if( ! xParent.is())
        return;

    const sal_Int32 nXOffset = 100;
    const sal_Int32 nYOffset = 100;

    uno::Sequence< uno::Reference< chart2::XDataSeriesTreeNode > > aChildren(
        xParent->getChildren());

    for( sal_Int32 nI = 0; nI < aChildren.getLength(); ++nI )
    {
        uno::Reference< chart2::XDataSeriesTreeParent > xNewParent( aChildren[ nI ], uno::UNO_QUERY );
        if( xNewParent.is())
        {
            // recurse !
            createLegendEntries( xShapeContainer, xNewParent, nOutCurrentHeight );
        }
        else
        {
            try
            {
                uno::Reference< drawing::XShape > xEntry(
                    m_xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.TextShape" )), uno::UNO_QUERY );

                if( !xEntry.is())
                    continue;

                xShapeContainer->add( xEntry );

                ::rtl::OUString aName( RTL_CONSTASCII_USTRINGPARAM( "<none>" ));
                uno::Reference< chart2::XDataSource > xSeriesSource(
                    aChildren[ nI ], uno::UNO_QUERY );
                if( xSeriesSource.is())
                    aName = lcl_getLabelForSeries( xSeriesSource );

                uno::Reference< beans::XPropertySet > xTextProp( xEntry, uno::UNO_QUERY );
                if( xTextProp.is())
                {
                    drawing::TextHorizontalAdjust eHorizAdjust( drawing::TextHorizontalAdjust_LEFT );
                    drawing::TextVerticalAdjust eVertAdjust( drawing::TextVerticalAdjust_TOP );

                    xTextProp->setPropertyValue( C2U("CharHeight"), uno::makeAny( (float)(10.0)));
                    xTextProp->setPropertyValue( C2U("TextAutoGrowHeight"), uno::makeAny( sal_True ));
                    xTextProp->setPropertyValue( C2U("TextAutoGrowWidth"), uno::makeAny( sal_True ));
                    xTextProp->setPropertyValue( C2U("TextHorizontalAdjust"), uno::makeAny( eHorizAdjust ));
                    xTextProp->setPropertyValue( C2U("TextVerticalAdjust"), uno::makeAny( eVertAdjust ));
                }

                uno::Reference< text::XTextRange > xRange( xEntry, uno::UNO_QUERY );
                if( xRange.is())
                {
                    xRange->setString( aName );
                }

                awt::Size aTextSize( xEntry->getSize());
                xEntry->setPosition( awt::Point( 2*nXOffset + aTextSize.Height, nYOffset + nOutCurrentHeight ));

                // symbol
                uno::Reference< drawing::XShape > xSymbol(
                    m_xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.RectangleShape" )), uno::UNO_QUERY );

                if( ! xSymbol.is())
                    continue;

                sal_Int32 nSymbolWidth = aTextSize.Height * 8 / 10;
                sal_Int32 nDiff = (aTextSize.Height - nSymbolWidth) / 2;
                xShapeContainer->add( xSymbol );
                xSymbol->setSize( awt::Size( nSymbolWidth, nSymbolWidth ));
                xSymbol->setPosition( awt::Point( nXOffset + nDiff,
                                                  nYOffset + nOutCurrentHeight + nDiff ));

                uno::Reference< beans::XPropertySet > xSymbolProp( xSymbol, uno::UNO_QUERY );
                uno::Reference< beans::XPropertySet > xSeriesProp( xSeriesSource, uno::UNO_QUERY );
                if( xSymbolProp.is() &&
                    xSeriesProp.is() )
                {
                    xSymbolProp->setPropertyValue(
                        C2U("FillColor"),
                        xSeriesProp->getPropertyValue( C2U("Color")));
                }

                nOutCurrentHeight += aTextSize.Height;
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
}


void VLegend::createShapes()
{
    if(! (m_xLegend.is() &&
          m_xShapeFactory.is() &&
          m_xTarget.is()))
        return;

    try
    {
        awt::Size aSize( 3000, 5000 );
        if( m_aMaxSize.Width > 0 &&
            m_aMaxSize.Height > 0 )
            aSize = m_aMaxSize;

        //create shape and add to page
        m_xShape.set(
            m_xShapeFactory->createInstance(
                C2U( "com.sun.star.drawing.GroupShape" )), uno::UNO_QUERY );
        m_xTarget->add( m_xShape );

        uno::Reference< drawing::XShapes > xLegendContainer( m_xShape, uno::UNO_QUERY );
        if( xLegendContainer.is())
        {
            uno::Reference< drawing::XShape > xBorder(
                m_xShapeFactory->createInstance(
                    C2U( "com.sun.star.drawing.RectangleShape" )), uno::UNO_QUERY );
            if( xBorder.is())
            {
                xLegendContainer->add( xBorder );
                awt::Point aPos( 0, 0 );
                xBorder->setPosition( aPos );
            }

            const sal_Int32 nVerticalPadding = 200;
            const sal_Int32 nSeparatorDist = 100;
            sal_Int32 nCurrentHeight = nVerticalPadding;

            uno::Sequence< uno::Reference< chart2::XLegendEntry > > aEntries( m_xLegend->getEntries());
            const sal_Int32 nNumOfChartTypes = aEntries.getLength();

            for( sal_Int32 nI = 0; nI < nNumOfChartTypes; ++nI )
            {
                uno::Reference< chart2::XDataSeriesTreeParent > xGroup( aEntries[ nI ], uno::UNO_QUERY );
                if( xGroup.is())
                {
                    createLegendEntries( xLegendContainer, xGroup, nCurrentHeight );
                }

                // separator between chart type groups
                if( nI < nNumOfChartTypes - 1 )
                {
                    uno::Reference< drawing::XShape > xSeparator(
                        m_xShapeFactory->createInstance(
                            C2U( "com.sun.star.drawing.LineShape" )), uno::UNO_QUERY );

                    if( xSeparator.is())
                    {
                        nCurrentHeight += nSeparatorDist;
                        xLegendContainer->add( xSeparator );
                        xSeparator->setSize( awt::Size( aSize.Width, 0 ));
                        xSeparator->setPosition( awt::Point( 0, nCurrentHeight ));
                        nCurrentHeight += nSeparatorDist;
                    }
                }
            }

            aSize.Height = nCurrentHeight + nVerticalPadding;

            if( xBorder.is())
                xBorder->setSize( aSize );
        }

        m_aBoundRect.Width = aSize.Width;
        m_aBoundRect.Height = aSize.Height;
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void VLegend::setMaxSize( const awt::Size & rSize )
{
    m_aMaxSize = rSize;
}

void VLegend::changePosition( const awt::Point & rPos )
{
    if(! m_xShape.is())
        return;

    // anchor is right/middle
    m_aBoundRect.X = rPos.X;
    m_aBoundRect.Y = rPos.Y;
    awt::Point aUpperLeft( rPos );
    aUpperLeft.X -= (m_aBoundRect.Width + 400);
    aUpperLeft.Y -= (m_aBoundRect.Height / 2);

    m_xShape->setPosition( aUpperLeft );
}

//.............................................................................
} //namespace chart
//.............................................................................
