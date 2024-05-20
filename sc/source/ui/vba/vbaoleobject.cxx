/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <ooo/vba/XControlProvider.hpp>

#include "vbaoleobject.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaOLEObject::ScVbaOLEObject( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext,
            css::uno::Reference< css::drawing::XControlShape > const & xControlShape )
: OLEObjectImpl_BASE( xParent, xContext )
{
    //init m_xWindowPeer
    uno::Reference< awt::XControlModel > xControlModel( xControlShape->getControl(), css::uno::UNO_SET_THROW );
    uno::Reference< container::XChild > xChild( xControlModel, uno::UNO_QUERY_THROW );
    xChild.set( xChild->getParent(), uno::UNO_QUERY_THROW );
    xChild.set( xChild->getParent(), uno::UNO_QUERY_THROW );
    uno::Reference<frame::XModel> xModel( xChild->getParent(), uno::UNO_QUERY_THROW );
    uno::Reference<lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_SET_THROW );
    uno::Reference< XControlProvider > xControlProvider( xServiceManager->createInstanceWithContext(u"ooo.vba.ControlProvider"_ustr, mxContext ), uno::UNO_QUERY_THROW );
    m_xControl.set( xControlProvider->createControl(  xControlShape, xModel ) );
}

uno::Reference< uno::XInterface > SAL_CALL
ScVbaOLEObject::getObject()
{
    return uno::Reference< uno::XInterface >( m_xControl, uno::UNO_QUERY_THROW );
}

sal_Bool SAL_CALL
ScVbaOLEObject::getEnabled()
{
    return m_xControl->getEnabled();
}

void SAL_CALL
ScVbaOLEObject::setEnabled( sal_Bool _enabled )
{
    m_xControl->setEnabled( _enabled );
}

sal_Bool SAL_CALL
ScVbaOLEObject::getVisible()
{
    return m_xControl->getVisible();
}

void SAL_CALL
ScVbaOLEObject::setVisible( sal_Bool _visible )
{
    m_xControl->setVisible( _visible );
}

double SAL_CALL
ScVbaOLEObject::getLeft()
{
    return m_xControl->getLeft();
}

void SAL_CALL
ScVbaOLEObject::setLeft( double _left )
{
    m_xControl->setLeft( _left );

}

double SAL_CALL
ScVbaOLEObject::getTop()
{
    return m_xControl->getTop();
}

void SAL_CALL
ScVbaOLEObject::setTop( double _top )
{
    m_xControl->setTop( _top );
}

double SAL_CALL
ScVbaOLEObject::getHeight()
{
    return m_xControl->getHeight();
}

void SAL_CALL
ScVbaOLEObject::setHeight( double _height )
{
    m_xControl->setHeight( _height );
}

double SAL_CALL
ScVbaOLEObject::getWidth()
{
    return m_xControl->getWidth();
}

void SAL_CALL
ScVbaOLEObject::setWidth( double _width )
{
    m_xControl->setWidth( _width );
}

OUString SAL_CALL ScVbaOLEObject::getLinkedCell()
{
    return m_xControl->getControlSource();
}

void SAL_CALL ScVbaOLEObject::setLinkedCell( const OUString& _linkedcell )
{
    m_xControl->setControlSource( _linkedcell );
}

OUString
ScVbaOLEObject::getServiceImplName()
{
    return u"ScVbaOLEObject"_ustr;
}

uno::Sequence< OUString >
ScVbaOLEObject::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.OLEObject"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
