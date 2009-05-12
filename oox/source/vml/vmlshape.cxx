/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vmlshape.cxx,v $
 * $Revision: 1.5 $
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

#include <rtl/ustring.hxx>
#include "oox/vml/shape.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

using namespace com::sun::star;

namespace oox { namespace vml {

Shape::Shape( const rtl::OUString& rServiceName )
: msServiceName( rServiceName )
, mnCoordWidth( 0 )
, mnCoordHeight( 0 )
, mnStroked( 0 )
{
}
Shape::~Shape()
{
}

void Shape::applyAttributes( const vml::Shape& rSource )
{
    if ( rSource.msId.getLength() )
        msId = rSource.msId;
    if ( rSource.msType.getLength() )
        msType = rSource.msType;
    if ( rSource.msShapeType.getLength() )
        msShapeType = rSource.msShapeType;
    if ( rSource.mnCoordWidth )
        mnCoordWidth = rSource.mnCoordWidth;
    if ( rSource.mnCoordHeight )
        mnCoordHeight = rSource.mnCoordHeight;
    if ( rSource.mnStroked )
        mnStroked = rSource.mnStroked;
    if ( rSource.moFilled )
        moFilled = rSource.moFilled;
    if ( rSource.moFillColor )
        moFillColor = rSource.moFillColor;
    if ( rSource.maPath.Name.getLength() )
        maPath = rSource.maPath;
    if ( rSource.msPosition.getLength() )
        msPosition = rSource.msPosition;
    maPosition = rSource.maPosition;
    maSize = rSource.maSize;
}

::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > Shape::createAndInsert(
    const ::oox::core::XmlFilterBase& rFilterBase, const ::oox::vml::Shape& rShape,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
            const awt::Rectangle* pShapeRect )
{
    uno::Reference< drawing::XShape > xShape;
    if ( rShape.msServiceName )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), uno::UNO_QUERY_THROW );
            xShape.set( xServiceFact->createInstance( rShape.msServiceName ), uno::UNO_QUERY_THROW );
            rxShapes->add( xShape );
            awt::Point aPosition;
            awt::Size aSize;
            if ( pShapeRect )
            {
                aPosition.X = pShapeRect->X;
                aPosition.Y = pShapeRect->Y;
                aSize.Width = pShapeRect->Width;
                aSize.Height = pShapeRect->Height;
            }
            else
            {
                aPosition = maPosition;
                aSize = maSize;
            }
            xShape->setPosition( aPosition );
            xShape->setSize( aSize );
            uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
            try
            {
                if ( maPath.Name.getLength() )
                    xPropSet->setPropertyValue( maPath.Name, maPath.Value );

                ::rtl::OUString sFillStyle( rtl::OUString::createFromAscii( "FillStyle" ) );
                if ( moFilled )
                    xPropSet->setPropertyValue( sFillStyle, uno::Any( *moFilled ? drawing::FillStyle_SOLID : drawing::FillStyle_NONE ) );
            }
            catch ( uno::Exception& )
            {
            }
            ::rtl::OUString rServiceName( rtl::OUString::createFromAscii( "com.sun.star.drawing.CustomShape" ) );
            if ( rShape.msShapeType.getLength() && ( msServiceName == rServiceName ) )
            {
                uno::Reference< drawing::XEnhancedCustomShapeDefaulter > xDefaulter( xShape, uno::UNO_QUERY );
                if( xDefaulter.is() )
                    xDefaulter->createCustomShapeDefaults( rShape.msShapeType );
            }
            mxShape = xShape;
        }
        catch( uno::Exception& )
        {
        }
    }
    return xShape;
}

void Shape::addChilds( const ::oox::core::XmlFilterBase& rFilterBase, const ::oox::vml::Drawing& rDrawing,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
            const awt::Rectangle& rClientRect )
{
    sal_Int32 nGlobalLeft  = SAL_MAX_INT32;
    sal_Int32 nGlobalRight = SAL_MIN_INT32;
    sal_Int32 nGlobalTop   = SAL_MAX_INT32;
    sal_Int32 nGlobalBottom= SAL_MIN_INT32;
    std::vector< ShapePtr >::iterator aIter( maChilds.begin() );
    while( aIter != maChilds.end() )
    {
        sal_Int32 l = (*aIter)->maPosition.X;
        sal_Int32 t = (*aIter)->maPosition.Y;
        sal_Int32 r = l + (*aIter)->maSize.Width;
        sal_Int32 b = t + (*aIter)->maSize.Height;
        if ( nGlobalLeft > l )
            nGlobalLeft = l;
        if ( nGlobalRight < r )
            nGlobalRight = r;
        if ( nGlobalTop > t )
            nGlobalTop = t;
        if ( nGlobalBottom < b )
            nGlobalBottom = b;
        aIter++;
    }
    aIter = maChilds.begin();
    while( aIter != maChilds.end() )
    {
        awt::Rectangle aShapeRect;
        awt::Rectangle* pShapeRect = 0;
        if ( ( nGlobalLeft != SAL_MAX_INT32 ) && ( nGlobalRight != SAL_MIN_INT32 ) && ( nGlobalTop != SAL_MAX_INT32 ) && ( nGlobalBottom != SAL_MIN_INT32 ) )
        {
            sal_Int32 nGlobalWidth = nGlobalRight - nGlobalLeft;
            sal_Int32 nGlobalHeight = nGlobalBottom - nGlobalTop;
            if ( nGlobalWidth && nGlobalHeight )
            {
                double fWidth = (*aIter)->maSize.Width;
                double fHeight= (*aIter)->maSize.Height;
                double fXScale = (double)rClientRect.Width / (double)nGlobalWidth;
                double fYScale = (double)rClientRect.Height / (double)nGlobalHeight;
                aShapeRect.X = static_cast< sal_Int32 >( ( ( (*aIter)->maPosition.X - nGlobalLeft ) * fXScale ) + rClientRect.X );
                aShapeRect.Y = static_cast< sal_Int32 >( ( ( (*aIter)->maPosition.Y - nGlobalTop  ) * fYScale ) + rClientRect.Y );
                fWidth *= fXScale;
                fHeight *= fYScale;
                aShapeRect.Width = static_cast< sal_Int32 >( fWidth );
                aShapeRect.Height = static_cast< sal_Int32 >( fHeight );
                pShapeRect = &aShapeRect;
            }
        }
        (*aIter++)->addShape( rFilterBase, rDrawing, rxShapes, pShapeRect );
    }
}

void Shape::addShape( const ::oox::core::XmlFilterBase& rFilterBase, const ::oox::vml::Drawing& rDrawing,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
            const awt::Rectangle* pShapeRect )
{
    oox::vml::Shape aShape( msServiceName );
    if ( msType.getLength() )
    {
        std::vector< ShapePtr >& rShapeTypes = const_cast< ::oox::vml::Drawing& >( rDrawing ).getShapeTypes();
        std::vector< ShapePtr >::const_iterator aShapeTypeIter( rShapeTypes.begin() );
        while( aShapeTypeIter != rShapeTypes.end() )
        {
            if ( (*aShapeTypeIter)->msType == aShape.msType )
            {
                aShape.applyAttributes( *(*aShapeTypeIter).get() );
                break;
            }
            aShapeTypeIter++;
        }
    }
    aShape.applyAttributes( *this );

    // creating XShape
    uno::Reference< drawing::XShape > xShape( createAndInsert( rFilterBase, aShape, rxShapes, pShapeRect ) );

    // creating GroupShape if possible
    uno::Reference< drawing::XShapes > xShapes( xShape, uno::UNO_QUERY );
    if ( xShapes.is() )
    {
        awt::Rectangle aChildRect;
        if ( pShapeRect )
            aChildRect = *pShapeRect;
        else
        {
            aChildRect.X = maPosition.X;
            aChildRect.Y = maPosition.Y;
            aChildRect.Width = maSize.Width;
            aChildRect.Height = maSize.Height;
        }
        addChilds( rFilterBase, rDrawing, xShapes, aChildRect );
    }
}

} }
