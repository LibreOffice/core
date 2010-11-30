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

#include "flow.hxx"

#include <sal/macros.h>

namespace layoutimpl
{

using namespace css;

bool Flow::ChildData::isVisible()
{
    return xChild.is();
}

Flow::Flow()
    : Container()
    , mnSpacing( 0 )
    , mbHomogeneous( false )
{
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Homogeneous" ),
             ::getCppuType( static_cast< const sal_Bool* >( NULL ) ),
             &mbHomogeneous );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Spacing" ),
             ::getCppuType( static_cast< const sal_Int32* >( NULL ) ),
             &mnSpacing );
}

bool
Flow::emptyVisible ()
{
    return true;
}

void SAL_CALL
Flow::addChild( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw (uno::RuntimeException, css::awt::MaxChildrenException)
{
    if ( xChild.is() )
    {
        ChildData *pData = new ChildData();
        pData->xChild = xChild;
        maChildren.push_back( pData );

        setChildParent( xChild );
        queueResize();
    }
}

void SAL_CALL
Flow::removeChild( const css::uno::Reference< css::awt::XLayoutConstrains >& xChild )
    throw (css::uno::RuntimeException)
{
    for ( std::list< ChildData * >::iterator it = maChildren.begin();
          it != maChildren.end(); ++it )
    {
        if ( (*it)->xChild == xChild )
        {
            delete *it;
            maChildren.erase( it );

            unsetChildParent( xChild );
            queueResize();
            break;
        }
    }
}

css::uno::Sequence< css::uno::Reference < css::awt::XLayoutConstrains > > SAL_CALL
Flow::getChildren()
    throw (css::uno::RuntimeException)
{
    uno::Sequence< uno::Reference< awt::XLayoutConstrains > > children( maChildren.size() );
    unsigned int i = 0;
    for ( std::list< ChildData * >::iterator it = maChildren.begin();
          it != maChildren.end(); ++it, ++i )
        children[i] = (*it)->xChild;

    return children;
}

uno::Reference< beans::XPropertySet > SAL_CALL
Flow::getChildProperties( const uno::Reference< awt::XLayoutConstrains >& /*xChild*/ )
    throw (uno::RuntimeException)
{
    return uno::Reference< beans::XPropertySet >();
}

css::awt::Size
Flow::calculateSize( long nMaxWidth )
{
    long nNeedHeight = 0;

    std::list<ChildData *>::const_iterator it;
    mnEachWidth = 0;
    // first pass, for homogeneous property
    for (it = maChildren.begin(); it != maChildren.end(); ++it)
    {
        if ( !(*it)->isVisible() )
            continue;
        (*it)->aRequisition = (*it)->xChild->getMinimumSize();
        if ( mbHomogeneous )
            mnEachWidth = SAL_MAX( mnEachWidth, (*it)->aRequisition.Width );
    }

    long nRowWidth = 0, nRowHeight = 0;
    for (it = maChildren.begin(); it != maChildren.end(); ++it)
    {
        if ( !(*it)->isVisible() )
            continue;

        awt::Size aChildSize = (*it)->aRequisition;
        if ( mbHomogeneous )
            aChildSize.Width = mnEachWidth;

        if ( nMaxWidth && nRowWidth > 0 && nRowWidth + aChildSize.Width > nMaxWidth )
        {
            nRowWidth = 0;
            nNeedHeight += nRowHeight;
            nRowHeight = 0;
        }
        nRowHeight = SAL_MAX( nRowHeight, aChildSize.Height );
        nRowWidth += aChildSize.Width;
    }
    nNeedHeight += nRowHeight;

    return awt::Size( nRowWidth, nNeedHeight );
}

awt::Size SAL_CALL
Flow::getMinimumSize() throw(uno::RuntimeException)
{
    return maRequisition = calculateSize( 0 );
}

sal_Bool SAL_CALL
Flow::hasHeightForWidth()
    throw(css::uno::RuntimeException)
{
    return true;
}

sal_Int32 SAL_CALL
Flow::getHeightForWidth( sal_Int32 nWidth )
    throw(css::uno::RuntimeException)
{
    return calculateSize( nWidth ).Height;
}

void SAL_CALL
Flow::allocateArea( const css::awt::Rectangle &rArea )
    throw (css::uno::RuntimeException)
{
    maAllocation = rArea;

    std::list<ChildData *>::const_iterator it;
    long nX = 0, nY = 0, nRowHeight = 0;
    for (it = maChildren.begin(); it != maChildren.end(); ++it)
    {
        ChildData *child = *it;
        if ( !child->isVisible() )
            continue;

        awt::Size aChildSize( child->aRequisition );
        if ( mbHomogeneous )
            aChildSize.Width = mnEachWidth;

        if ( nX > 0 && nX + aChildSize.Width > rArea.Width )
        {
            nX = 0;
            nY += nRowHeight;
            nRowHeight = 0;
        }
        nRowHeight = SAL_MAX( nRowHeight, aChildSize.Height );

        allocateChildAt( child->xChild,
                         awt::Rectangle( rArea.X + nX, rArea.Y + nY, aChildSize.Width, aChildSize.Height ) );

        nX += aChildSize.Width;
    }
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
