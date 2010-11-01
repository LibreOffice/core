/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "box.hxx"

#include <tools/debug.hxx>
#include <sal/macros.h>

// fixed point precision for distributing error
#define FIXED_PT 16

namespace layoutimpl
{

using namespace css;

Box::ChildProps::ChildProps( Box::ChildData *pData )
{
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Expand" ),
             ::getCppuType( static_cast< const sal_Bool* >( NULL ) ),
             &(pData->mbExpand) );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Fill" ),
             ::getCppuType( static_cast< const sal_Bool* >( NULL ) ),
             &(pData->mbFill) );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Padding" ),
             ::getCppuType( static_cast< const sal_Int32* >( NULL ) ),
             &(pData->mnPadding) );
}

Box::ChildData::ChildData( uno::Reference< awt::XLayoutConstrains > const& xChild )
    : Box_Base::ChildData( xChild )
    , mnPadding( 0 )
    , mbExpand( true )
    , mbFill( true )
{
}

Box::ChildData*
Box::createChild( uno::Reference< awt::XLayoutConstrains > const& xChild )
    {
    return new ChildData( xChild );
    }

Box::ChildProps*
Box::createChildProps( Box_Base::ChildData *pData )
{
    return new ChildProps( static_cast<Box::ChildData*> ( pData ) );
}

Box::Box( bool horizontal )
    : Box_Base()
    , mnSpacing( 0 )
    , mbHomogeneous( false )
    , mbHorizontal( horizontal )
{
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Homogeneous" ),
             ::getCppuType( static_cast< const sal_Bool* >( NULL ) ),
             &mbHomogeneous );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Spacing" ),
             ::getCppuType( static_cast< const sal_Int32* >( NULL ) ),
             &mnSpacing );
    mbHasFlowChildren = false;
}

awt::Size
Box::calculateSize( long nWidth )
{
    int nVisibleChildren = 0;
    // primary vs secundary axis (instead of a X and Y)
    int nPrimSize = 0;
    int nSecSize = 0;
    int nFlowMinWidth = 0;  // in case the box only has flow children

    mbHasFlowChildren = false;

    for ( std::list<Box_Base::ChildData *>::const_iterator it
              = maChildren.begin(); it != maChildren.end(); it++ )
    {
        ChildData *child = static_cast<Box::ChildData*> ( *it );
        if ( !child->isVisible() )
            continue;

        uno::Reference< awt::XLayoutContainer > xChildCont( child->mxChild, uno::UNO_QUERY );
        bool bFlow = xChildCont.is() && xChildCont->hasHeightForWidth();

        awt::Size aChildSize = child->maRequisition = child->mxChild->getMinimumSize();

        if ( !mbHorizontal /*vertical*/ && bFlow )
        {
            if ( nFlowMinWidth == 0 || nFlowMinWidth > aChildSize.Width )
                nFlowMinWidth = aChildSize.Width;
            mbHasFlowChildren = true;
        }
        else
        {
            int size = primDim( aChildSize ) + child->mnPadding * 2;
            if ( mbHomogeneous )
                nPrimSize = SAL_MAX( nPrimSize, size );
            else
                nPrimSize += size;

            nSecSize = SAL_MAX( nSecSize, secDim( aChildSize ) );
        }
        nVisibleChildren++;
    }

    if ( nVisibleChildren )
    {
        if ( mbHomogeneous )
            nPrimSize *= nVisibleChildren;
        nPrimSize += (nVisibleChildren - 1) * mnSpacing;
    }

    if ( mbHasFlowChildren )
    {
        if ( nWidth == 0 )
            nWidth = nSecSize ? nSecSize : nFlowMinWidth;
        for ( std::list<Box_Base::ChildData *>::const_iterator it
                  = maChildren.begin(); it != maChildren.end(); it++ )
        {
            ChildData *child = static_cast<Box::ChildData*> ( *it );
            if ( !child->isVisible() )
                continue;

            uno::Reference< awt::XLayoutContainer > xChildCont( child->mxChild, uno::UNO_QUERY );
            bool bFlow = xChildCont.is() && xChildCont->hasHeightForWidth();

            if ( bFlow )
                nPrimSize += xChildCont->getHeightForWidth( nWidth );
        }
    }

    nPrimSize += mnBorderWidth * 2;
    nSecSize += mnBorderWidth * 2;
    return awt::Size( mbHorizontal ? nPrimSize : nSecSize,
                      mbHorizontal ? nSecSize : nPrimSize );
}

awt::Size SAL_CALL
Box::getMinimumSize() throw(uno::RuntimeException)
{
    maRequisition = calculateSize();
    return maRequisition;
}

sal_Bool SAL_CALL
Box::hasHeightForWidth()
    throw(uno::RuntimeException)
{
    return mbHasFlowChildren;
}

sal_Int32 SAL_CALL
Box::getHeightForWidth( sal_Int32 nWidth )
    throw(uno::RuntimeException)
{
    if ( hasHeightForWidth() )
        return calculateSize( nWidth ).Height;
    return maRequisition.Height;
}

void SAL_CALL
Box::allocateArea( const awt::Rectangle &newArea )
    throw (uno::RuntimeException)
{
    maAllocation = newArea;
    int nVisibleChildren = 0, nExpandChildren = 0;

    for ( std::list<Box_Base::ChildData *>::const_iterator it
              = maChildren.begin(); it != maChildren.end(); it++ )
    {
        ChildData *child = static_cast<Box::ChildData*> ( *it );
        if ( child->isVisible() )
        {
            nVisibleChildren++;
            if ( child->mbExpand )
                nExpandChildren++;
        }
    }
    if ( !nVisibleChildren )
        return;

    // split rectangle for dimension helpers
    awt::Point newPoint( newArea.X, newArea.Y );
    awt::Size newSize( newArea.Width, newArea.Height );

    int nExtraSpace;
    if ( mbHomogeneous )
        nExtraSpace = ( ( primDim( newSize ) - mnBorderWidth * 2 -
                          ( nVisibleChildren - 1 ) * mnSpacing )) / nVisibleChildren;
    else if ( nExpandChildren )
    {
        int reqSize = primDim( maRequisition );
        if ( !mbHorizontal && hasHeightForWidth() )
            reqSize = getHeightForWidth( newArea.Width );
        nExtraSpace = ( primDim( newSize ) - reqSize ) / nExpandChildren;
    }
    else
        nExtraSpace = 0;

    int nChildPrimPoint, nChildSecPoint, nChildPrimSize, nChildSecSize;

    int nStartPoint = primDim( newPoint ) + mnBorderWidth;
    int nBoxSecSize = SAL_MAX( 1, secDim( newSize ) - mnBorderWidth * 2 );

    for ( std::list<Box_Base::ChildData *>::const_iterator it
              = maChildren.begin(); it != maChildren.end(); it++ )
    {
        ChildData *child = static_cast<Box::ChildData*> ( *it );
        if ( !child->isVisible() )
            continue;

        awt::Point aChildPos;
        int nBoxPrimSize;  // of the available box space

        if ( mbHomogeneous )
            nBoxPrimSize = nExtraSpace;
        else
        {
            uno::Reference< awt::XLayoutContainer > xChildCont( child->mxChild, uno::UNO_QUERY );
            bool bFlow = xChildCont.is() && xChildCont->hasHeightForWidth();
            if ( !mbHorizontal && bFlow )
                nBoxPrimSize = xChildCont->getHeightForWidth( newArea.Width );
            else
                nBoxPrimSize = primDim( child->maRequisition );
            nBoxPrimSize += child->mnPadding;
            if ( child->mbExpand )
                nBoxPrimSize += nExtraSpace;
        }

        nChildPrimPoint = nStartPoint + child->mnPadding;
        nChildSecPoint = secDim( newPoint ) + mnBorderWidth;

        nChildSecSize = nBoxSecSize;
        if ( child->mbFill )
            nChildPrimSize = SAL_MAX( 1, nBoxPrimSize - child->mnPadding);
        else
        {
            nChildPrimSize = primDim( child->maRequisition );
            nChildPrimPoint += (nBoxPrimSize - nChildPrimSize) / 2;

            nChildSecPoint += (nBoxSecSize - nChildSecSize) / 2;
        }

        awt::Rectangle area;
        area.X = mbHorizontal ? nChildPrimPoint : nChildSecPoint;
        area.Y = mbHorizontal ? nChildSecPoint : nChildPrimPoint;
        area.Width = mbHorizontal ? nChildPrimSize : nChildSecSize;
        area.Height = mbHorizontal ? nChildSecSize : nChildPrimSize;

        allocateChildAt( child->mxChild, area );

        nStartPoint += nBoxPrimSize + mnSpacing + child->mnPadding;
    }
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
