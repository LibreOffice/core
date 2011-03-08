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

#include "bin.hxx"

#include <sal/macros.h>

namespace layoutimpl
{

using namespace css;

/* Bin */

Bin::Bin() : Container()
{
}

void SAL_CALL
Bin::addChild( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw (uno::RuntimeException, awt::MaxChildrenException)
{
    if ( mxChild.is() )
        throw awt::MaxChildrenException();
    if ( xChild.is() )
    {
        mxChild = xChild;
        setChildParent( xChild );
        queueResize();
    }
}

void SAL_CALL
Bin::removeChild( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw (uno::RuntimeException)
{
    if ( xChild == mxChild )
    {
        mxChild = uno::Reference< awt::XLayoutConstrains >();
        unsetChildParent( xChild );
        queueResize();
    }
}

uno::Sequence< uno::Reference< awt::XLayoutConstrains > > SAL_CALL
Bin::getChildren()
    throw (uno::RuntimeException)
{
    return getSingleChild (mxChild);
}

void SAL_CALL
Bin::allocateArea( const awt::Rectangle &rArea )
    throw (uno::RuntimeException)
{
    maAllocation = rArea;
    if ( mxChild.is() )
        allocateChildAt( mxChild, rArea );
}

awt::Size SAL_CALL
Bin::getMinimumSize()
    throw(uno::RuntimeException)
{
    if ( mxChild.is() )
        return maRequisition = maChildRequisition = mxChild->getMinimumSize();
    return maRequisition = awt::Size( 0, 0 );
}

uno::Reference< beans::XPropertySet > SAL_CALL
Bin::getChildProperties( const uno::Reference< awt::XLayoutConstrains >& )
    throw (uno::RuntimeException)
{
    return uno::Reference< beans::XPropertySet >();
}

sal_Bool SAL_CALL
Bin::hasHeightForWidth()
    throw(uno::RuntimeException)
{
    uno::Reference< awt::XLayoutContainer > xChildCont( mxChild, uno::UNO_QUERY );
    if ( xChildCont.is() )
        return xChildCont->hasHeightForWidth();
    return false;
}

sal_Int32 SAL_CALL
Bin::getHeightForWidth( sal_Int32 nWidth )
    throw(uno::RuntimeException)
{
    uno::Reference< awt::XLayoutContainer > xChildCont( mxChild, uno::UNO_QUERY );
    if ( xChildCont.is() )
        return xChildCont->getHeightForWidth( nWidth );
    return maRequisition.Height;
}

/* Align */

Align::Align() : Bin()
{
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Halign" ),
             ::getCppuType( static_cast< const float* >( NULL ) ),
             &fHorAlign );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Valign" ),
             ::getCppuType( static_cast< const float* >( NULL ) ),
             &fVerAlign );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Hfill" ),
             ::getCppuType( static_cast< const float* >( NULL ) ),
             &fHorFill );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Vfill" ),
             ::getCppuType( static_cast< const float* >( NULL ) ),
             &fVerFill );

    fHorAlign = fVerAlign = 0.5;
    fHorFill = fVerFill = 0;
}

void SAL_CALL
Align::allocateArea( const awt::Rectangle &rArea )
    throw (uno::RuntimeException)
{
    maAllocation = rArea;
    if ( !mxChild.is() )
        return;

    awt::Rectangle aChildArea;
    aChildArea.Width = SAL_MIN( rArea.Width, maChildRequisition.Width );
    aChildArea.Width += (sal_Int32) SAL_MAX(
        0, (rArea.Width - maChildRequisition.Width) * fHorFill );
    aChildArea.Height = SAL_MIN( rArea.Height, maChildRequisition.Height );
    aChildArea.Height += (sal_Int32) SAL_MAX(
        0, (rArea.Height - maChildRequisition.Height) * fVerFill );

    aChildArea.X = rArea.X + (sal_Int32)( (rArea.Width - aChildArea.Width) * fHorAlign );
    aChildArea.Y = rArea.Y + (sal_Int32)( (rArea.Height - aChildArea.Height) * fVerAlign );

    allocateChildAt( mxChild, aChildArea );
}

bool
Align::emptyVisible ()
{
    return true;
}

/* MinSize */

MinSize::MinSize() : Bin()
{
    mnMinWidth = mnMinHeight = 0;
    addProp( RTL_CONSTASCII_USTRINGPARAM( "MinWidth" ),
             ::getCppuType( static_cast< const long* >( NULL ) ),
             &mnMinWidth );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "MinHeight" ),
             ::getCppuType( static_cast< const long* >( NULL ) ),
             &mnMinHeight );
}

bool
MinSize::emptyVisible ()
{
    return true;
}

awt::Size SAL_CALL MinSize::getMinimumSize()
    throw(uno::RuntimeException)
{
    Bin::getMinimumSize();
    maRequisition.Width = SAL_MAX( maRequisition.Width, mnMinWidth );
    maRequisition.Height = SAL_MAX( maRequisition.Height, mnMinHeight );
    return maRequisition;
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
