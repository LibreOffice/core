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

#include "wrapper.hxx"

#include <com/sun/star/awt/XLayoutRoot.hpp>
#include <com/sun/star/awt/XLayoutContainer.hpp>
#include <comphelper/processfactory.hxx>
#include <layout/core/helper.hxx>
#include <tools/debug.hxx>

using namespace ::com::sun::star;

namespace layout
{

Container::Container( Context const* context, char const* pId )
    : mxContainer( context->GetPeerHandle( pId ), uno::UNO_QUERY )
{
    if ( !mxContainer.is() )
    {
        OSL_TRACE( "Error: failed to associate container with '%s'", pId );
    }
}

Container::Container( rtl::OUString const& rName, sal_Int32 nBorder )
{
    mxContainer = layoutimpl::WidgetFactory::createContainer( rName );

    uno::Reference< beans::XPropertySet > xProps( mxContainer, uno::UNO_QUERY_THROW );
    xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Border" ) ),
                              uno::Any( nBorder ) );
}

void Container::Add( Window *pChild )
{
    if ( pChild )
    {
        uno::Reference< awt::XLayoutConstrains > xChild( pChild->GetPeer(), uno::UNO_QUERY );
        mxContainer->addChild( xChild );
    }
}

void Container::Add( Container *pChild )
{
    if ( pChild )
    {
        uno::Reference< awt::XLayoutConstrains > xChild( pChild->getImpl(), uno::UNO_QUERY );
        mxContainer->addChild( xChild );
    }
}

void Container::Remove( Window *pChild )
{
    if ( pChild )
    {
        uno::Reference< awt::XLayoutConstrains > xChild( pChild->GetPeer(), uno::UNO_QUERY );
        mxContainer->removeChild( xChild );
    }
}

void Container::Remove( Container *pChild )
{
    if ( pChild )
    {
        uno::Reference< awt::XLayoutConstrains > xChild( pChild->getImpl(), uno::UNO_QUERY );
        mxContainer->removeChild( xChild );
    }
}

void Container::Clear()
{
    css::uno::Sequence< css::uno::Reference < css::awt::XLayoutConstrains > > children;
    children = mxContainer->getChildren();
    for (int i = 0; i < children.getLength(); i++)
        mxContainer->removeChild( children[i] );
}

void Container::ShowAll( bool bShow )
{
    struct inner
    {
        static void setChildrenVisible( uno::Reference < awt::XLayoutContainer > xCont,
                                        bool bVisible )  /* auxiliary */
        {
            if ( xCont.is() )
            {
                uno::Sequence< uno::Reference < awt::XLayoutConstrains > > aChildren;
                aChildren = xCont->getChildren();
                for (int i = 0; i < aChildren.getLength(); i++)
                {
                    uno::Reference < awt::XLayoutConstrains > xChild( aChildren[ i ] );

                    uno::Reference< awt::XWindow > xWin( xChild, uno::UNO_QUERY);
                    if ( xWin.is() )
                        xWin->setVisible( bVisible );

                    uno::Reference < awt::XLayoutContainer > xChildCont(
                        xChild, uno::UNO_QUERY );
                    setChildrenVisible( xChildCont, bVisible );
                }
            }
        }
    };

    inner::setChildrenVisible( mxContainer, bShow );
}

void Container::Show()
{
    ShowAll( true );
}

void Container::Hide()
{
    ShowAll( false );
}

Table::Table( sal_Int32 nBorder, sal_Int32 nColumns )
    : Container( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "table" ) ), nBorder )
{
    uno::Reference< beans::XPropertySet > xProps( mxContainer, uno::UNO_QUERY_THROW );
    xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Columns" ) ),
                              uno::Any( nColumns ) );
}

void Table::Add( Window *window, bool bXExpand, bool bYExpand,
                 sal_Int32 nXSpan, sal_Int32 nYSpan )
{
    if ( !window )
        return;
    WindowImpl &rImpl = window->getImpl();
    uno::Reference< awt::XLayoutConstrains > xChild( rImpl.mxWindow,
                                                     uno::UNO_QUERY );
    mxContainer->addChild( xChild );
    setProps( xChild, bXExpand, bYExpand, nXSpan, nYSpan );
}

void Table::Add( Container *pContainer, bool bXExpand, bool bYExpand,
                 sal_Int32 nXSpan, sal_Int32 nYSpan )
{
    if ( !pContainer )
        return;
    uno::Reference< awt::XLayoutConstrains > xChild( pContainer->getImpl(),
                                                     uno::UNO_QUERY );
    mxContainer->addChild( xChild );
    setProps( xChild, bXExpand, bYExpand, nXSpan, nYSpan );
}

void Table::setProps( uno::Reference< awt::XLayoutConstrains > xChild,
                      bool bXExpand, bool bYExpand, sal_Int32 nXSpan, sal_Int32 nYSpan )
{
    uno::Reference< beans::XPropertySet > xProps
        ( mxContainer->getChildProperties( xChild ), uno::UNO_QUERY_THROW );
    xProps->setPropertyValue( rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM( "XExpand" ) ),
                              uno::Any( bXExpand ) );
    xProps->setPropertyValue( rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM( "YExpand" ) ),
                              uno::Any( bYExpand ) );
    xProps->setPropertyValue( rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM( "ColSpan" ) ),
                              uno::Any( nXSpan ) );
    xProps->setPropertyValue( rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM( "RowSpan" ) ),
                              uno::Any( nYSpan ) );
}

Box::Box( rtl::OUString const& rName, sal_Int32 nBorder, bool bHomogeneous )
    : Container( rName, nBorder )
{
    uno::Reference< beans::XPropertySet > xProps( mxContainer, uno::UNO_QUERY_THROW );
    xProps->setPropertyValue( rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM( "Homogeneous" ) ),
                              uno::Any( bHomogeneous ) );
}

void Box::Add( Window *window, bool bExpand, bool bFill, sal_Int32 nPadding)
{
    if ( !window )
        return;
    WindowImpl &rImpl = window->getImpl();
    uno::Reference< awt::XLayoutConstrains > xChild( rImpl.mxWindow,
                                                     uno::UNO_QUERY );

    mxContainer->addChild( xChild );
    setProps( xChild, bExpand, bFill, nPadding );
}

void Box::Add( Container *pContainer, bool bExpand, bool bFill, sal_Int32 nPadding)
{
    if ( !pContainer )
        return;

    uno::Reference< awt::XLayoutConstrains > xChild( pContainer->getImpl(),
                                                     uno::UNO_QUERY );
    mxContainer->addChild( xChild );
    setProps( xChild, bExpand, bFill, nPadding );
}

void Box::setProps( uno::Reference< awt::XLayoutConstrains > xChild,
                    bool bExpand, bool bFill, sal_Int32 nPadding )
{
    uno::Reference< beans::XPropertySet > xProps
        ( mxContainer->getChildProperties( xChild ), uno::UNO_QUERY_THROW );

    xProps->setPropertyValue( rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM( "Expand" ) ),
                              uno::Any( bExpand ) );
    xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Fill" ) ),
                              uno::Any( bFill ) );
    xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Padding" ) ),
                              uno::Any( nPadding ) );
}

Table::Table( Context const* context, char const* pId )
    : Container( context, pId )
{
}

Box::Box( Context const* context, char const* pId )
    : Container( context, pId )
{
}

HBox::HBox( sal_Int32 nBorder, bool bHomogeneous )
    : Box( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "hbox" ) ),
           nBorder, bHomogeneous )
{
}

HBox::HBox( Context const* context, char const* pId )
    : Box( context, pId )
{
}

VBox::VBox( sal_Int32 nBorder, bool bHomogeneous )
    : Box( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "vbox" ) ),
           nBorder, bHomogeneous )
{
}

VBox::VBox( Context const* context, char const* pId )
    : Box( context, pId )
{
}

} // namespace layout

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
