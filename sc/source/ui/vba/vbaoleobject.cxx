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

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <ooo/vba/XControlProvider.hpp>

#include "vbaoleobject.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaOLEObject::ScVbaOLEObject( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext,
            css::uno::Reference< css::drawing::XControlShape > xControlShape )
: OLEObjectImpl_BASE( xParent, xContext ), m_xControlShape( xControlShape )
{
    //init m_xWindowPeer
    uno::Reference< awt::XControlModel > xControlModel( xControlShape->getControl(), css::uno::UNO_QUERY_THROW );
    uno::Reference< container::XChild > xChild( xControlModel, uno::UNO_QUERY_THROW );
    xChild.set( xChild->getParent(), uno::UNO_QUERY_THROW );
    xChild.set( xChild->getParent(), uno::UNO_QUERY_THROW );
    css::uno::Reference< css::frame::XModel > xModel( xChild->getParent(), uno::UNO_QUERY_THROW );
    uno::Reference<lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< XControlProvider > xControlProvider( xServiceManager->createInstanceWithContext( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.ControlProvider" ) ), mxContext ), uno::UNO_QUERY_THROW );
    m_xControl.set( xControlProvider->createControl(  xControlShape, xModel ) );
}

uno::Reference< uno::XInterface > SAL_CALL
ScVbaOLEObject::getObject() throw (uno::RuntimeException)
{
    return uno::Reference< uno::XInterface >( m_xControlShape, uno::UNO_QUERY_THROW );
}

sal_Bool SAL_CALL
ScVbaOLEObject::getEnabled() throw (uno::RuntimeException)
{
    return m_xControl->getEnabled();
}

void SAL_CALL
ScVbaOLEObject::setEnabled( sal_Bool _enabled ) throw (uno::RuntimeException)
{
    m_xControl->setEnabled( _enabled );
}

sal_Bool SAL_CALL
ScVbaOLEObject::getVisible() throw (uno::RuntimeException)
{
    OSL_TRACE("OleObject %s returning visible %s", rtl::OUStringToOString( m_xControl->getName(), RTL_TEXTENCODING_UTF8 ).getStr(), m_xControl->getVisible() ? "true" : "false" );
    return m_xControl->getVisible();
}

void SAL_CALL
ScVbaOLEObject::setVisible( sal_Bool _visible ) throw (uno::RuntimeException)
{
    OSL_TRACE("OleObject %s set visible %s", rtl::OUStringToOString( m_xControl->getName(), RTL_TEXTENCODING_UTF8 ).getStr(), _visible ? "true" : "false" );
    m_xControl->setVisible( _visible );
}

double SAL_CALL
ScVbaOLEObject::getLeft() throw (uno::RuntimeException)
{
    return m_xControl->getLeft();
}

void SAL_CALL
ScVbaOLEObject::setLeft( double _left ) throw (uno::RuntimeException)
{
    m_xControl->setLeft( _left );

}

double SAL_CALL
ScVbaOLEObject::getTop() throw (uno::RuntimeException)
{
    return m_xControl->getTop();
}

void SAL_CALL
ScVbaOLEObject::setTop( double _top ) throw (uno::RuntimeException)
{
    m_xControl->setTop( _top );
}

double SAL_CALL
ScVbaOLEObject::getHeight() throw (uno::RuntimeException)
{
    return m_xControl->getHeight();
}

void SAL_CALL
ScVbaOLEObject::setHeight( double _height ) throw (uno::RuntimeException)
{
    m_xControl->setHeight( _height );
}

double SAL_CALL
ScVbaOLEObject::getWidth() throw (uno::RuntimeException)
{
    return m_xControl->getWidth();
}

void SAL_CALL
ScVbaOLEObject::setWidth( double _width ) throw (uno::RuntimeException)
{
    m_xControl->setWidth( _width );
}

rtl::OUString
ScVbaOLEObject::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaOLEObject"));
}

uno::Sequence< rtl::OUString >
ScVbaOLEObject::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.OLEObject" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
