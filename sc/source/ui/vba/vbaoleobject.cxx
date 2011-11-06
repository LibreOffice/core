/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <ooo/vba/XControlProvider.hpp>

#include "vbaoleobject.hxx"

using namespace com::sun::star;
using namespace ooo::vba;


sal_Int32 pt2mm( double pt ) //1/100mm
{
    return static_cast<sal_Int32>(pt * 0.352778);
}

double mm2pt( sal_Int32 mm )
{
    return mm * 2.8345;
}


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
rtl::OUString&
ScVbaOLEObject::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaOLEObject") );
    return sImplName;
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
