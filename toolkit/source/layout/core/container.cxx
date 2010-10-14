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

#include "container.hxx"

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <tools/debug.hxx>

namespace layoutimpl {

using namespace css;

Container::Container()
    : Container_Base()
    , PropHelper()
    , mnBorderWidth( 0 )
{
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Border" ),
             ::getCppuType( static_cast< const sal_Int32* >( NULL ) ),
             &mnBorderWidth );
    setChangeListener( this );
}

bool
Container::emptyVisible ()
{
    return false;
}

uno::Any
Container::queryInterface( const uno::Type & rType ) throw (uno::RuntimeException)
{
    uno::Any aRet = Container_Base::queryInterface( rType );
    return aRet.hasValue() ? aRet : PropHelper::queryInterface( rType );
}

void
Container::allocateChildAt( const uno::Reference< awt::XLayoutConstrains > &xChild,
                            const awt::Rectangle &rArea )
    throw( uno::RuntimeException )
{
    uno::Reference< awt::XLayoutContainer > xCont( xChild, uno::UNO_QUERY );
    if ( xCont.is() )
        xCont->allocateArea( rArea );
    else
    {
        uno::Reference< awt::XWindow > xWindow( xChild, uno::UNO_QUERY );
        if ( xWindow.is() )
            xWindow->setPosSize( rArea.X, rArea.Y, rArea.Width, rArea.Height,
                                 awt::PosSize::POSSIZE );
        else
        {
            DBG_ERROR( "Error: non-sizeable child" );
        }
    }
}

uno::Sequence< uno::Reference< awt::XLayoutConstrains > >
Container::getSingleChild ( uno::Reference< awt::XLayoutConstrains >const &xChildOrNil )
{
    uno::Sequence< uno::Reference< awt::XLayoutConstrains > > aSeq( ( xChildOrNil.is() ? 1 : 0 ) );
    if ( xChildOrNil.is() )
        aSeq[0] = xChildOrNil;
    return aSeq;
}

void
Container::queueResize()
{
    if ( mxLayoutUnit.is() )
        mxLayoutUnit->queueResize( uno::Reference< awt::XLayoutContainer >( this ) );
}

void
Container::setChildParent( const uno::Reference< awt::XLayoutConstrains >& xChild )
{
    uno::Reference< awt::XLayoutContainer > xContChild( xChild, uno::UNO_QUERY );
    if ( xContChild.is() )
    {
        xContChild->setParent( uno::Reference< awt::XLayoutContainer >( this ) );
#if 0
        assert( !mxLayoutUnit.is() );
        xContChild->setLayoutUnit( mxLayoutUnit );
#endif
    }
}

void
Container::unsetChildParent( const uno::Reference< awt::XLayoutConstrains >& xChild )
{
    uno::Reference< awt::XLayoutContainer > xContChild( xChild, uno::UNO_QUERY );
    if ( xContChild.is() )
    {
        xContChild->setParent( uno::Reference< awt::XLayoutContainer >() );
#if 0
        xContChild->setLayoutUnit( uno::Reference< awt::XLayoutUnit >() );
#endif
    }
}

#if 0
std::string
Container::getLabel()  // debug label
{
    std::string depth;
    uno::Reference< awt::XLayoutContainer > xContainer( this );
    while ( xContainer.is() )
    {
        int node = 0;  // child nb
        uno::Reference< awt::XLayoutContainer > xParent = xContainer->getContainerParent();
        if ( xParent.is() )
        {

            uno::Sequence< uno::Reference< awt::XLayoutConstrains > > aChildren;
            aChildren = xParent->getChildren();
            for ( node = 0; node < aChildren.getLength(); node++ )
                if ( aChildren[ node ] == xContainer )
                    break;
        }

        char str[ 8 ];
        snprintf( str, 8, "%d", node );
        if ( depth.empty() )
            depth = std::string( str );
        else
            depth = std::string( str ) + ":" + depth;

        xContainer = xParent;
    }

    return std::string( getName() ) + " (" + depth + ")";
}
#endif

void Container::propertiesChanged()
{
    // cl: why this assertion? This is also called to set properties at the top level widget which has no parent!?
    // DBG_ASSERT( mxParent.is(), "Properties listener: error container doesn't have parent" );

    if ( mxLayoutUnit.is() && mxParent.is() )
        mxLayoutUnit->queueResize( mxParent );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
