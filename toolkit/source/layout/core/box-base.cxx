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

#include <com/sun/star/awt/XWindow2.hpp>

// fixed point precision for distributing error
#define FIXED_PT 16

namespace layoutimpl
{

using namespace css;

Box_Base::ChildData::ChildData( uno::Reference< awt::XLayoutConstrains > const& xChild )
    : mxChild( xChild )
    , mxProps()
    , maRequisition()
{
}

static bool isVisible( uno::Reference< awt::XLayoutConstrains > xWidget )
{
    if ( !xWidget.is() )
    {
        OSL_FAIL( "FIXME: invalid child !" );
        return true;
    }

    uno::Reference< awt::XWindow2 > xWindow( xWidget, uno::UNO_QUERY );
    if ( xWindow.is() && !xWindow->isVisible() )
        return false;

    uno::Reference< awt::XLayoutContainer > xContainer( xWidget, uno::UNO_QUERY );
    if ( xContainer.is() )
    {
        uno::Sequence< uno::Reference< awt::XLayoutConstrains > > aChildren
            = xContainer->getChildren();

        if (!aChildren.getLength ())
            if (Container *c = dynamic_cast <Container*> (xWidget.get ()))
                return c->emptyVisible ();

        for ( int i = 0; i < aChildren.getLength(); i++ )
            if ( isVisible( aChildren[i] ) )
                return true;
        return false; // this would kill flow without workaround above
    }

    return true;
}

bool Box_Base::ChildData::isVisible()
{
    // FIXME: call the 'isVisible' method on it ?
    return layoutimpl::isVisible( mxChild );
}

void
Box_Base::AddChild (uno::Reference <awt::XLayoutConstrains> const& xChild)
{
    ChildData *pData = createChild (xChild);
    maChildren.push_back (pData);
    queueResize ();
}

void SAL_CALL
Box_Base::addChild (uno::Reference <awt::XLayoutConstrains> const& xChild)
    throw (uno::RuntimeException, awt::MaxChildrenException)
{
    if (xChild.is ())
    {
        AddChild (xChild);
        setChildParent (xChild);
    }
}

Box_Base::ChildData*
Box_Base::removeChildData( std::list< ChildData* >& lst, css::uno::Reference< css::awt::XLayoutConstrains > const& xChild )
{
    for ( std::list< ChildData* >::iterator it = lst.begin();
          it != lst.end(); ++it )
    {
        if ( (*it)->mxChild == xChild )
        {
            ChildData* pRet = *it;
            lst.erase( it );
            return pRet;
        }
    }
    return 0;
}

void SAL_CALL
Box_Base::removeChild( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw (uno::RuntimeException)
{
    if ( ChildData* p = removeChildData( maChildren, xChild ) )
    {
        delete p;
        unsetChildParent( xChild );
        queueResize();
    }
    else
    {
        OSL_FAIL( "Box_Base: removeChild: no such child" );
    }
}

uno::Sequence< uno::Reference < awt::XLayoutConstrains > > SAL_CALL
Box_Base::getChildren()
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Reference< awt::XLayoutConstrains > > children( maChildren.size() );
    unsigned int index = 0;
    for ( std::list< ChildData* >::iterator it = maChildren.begin();
          it != maChildren.end(); ++it, ++index )
        children[index] = ( *it )->mxChild;

    return children;
}

uno::Reference< beans::XPropertySet > SAL_CALL
Box_Base::getChildProperties( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw (uno::RuntimeException)
{

    for ( std::list< ChildData * >::iterator it = maChildren.begin();
          it != maChildren.end(); ++it)
    {
        if ( ( *it )->mxChild == xChild )
        {
            if ( !( *it )->mxProps.is() )
            {
                PropHelper *pProps = createChildProps( *it );
                pProps->setChangeListener( this );
                ( *it )->mxProps = pProps;
            }
            return (*it)->mxProps;
        }
    }
    return uno::Reference< beans::XPropertySet >();
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
