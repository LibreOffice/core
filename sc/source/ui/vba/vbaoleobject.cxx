/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaoleobject.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:56:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include "vbaoleobject.hxx"

using namespace com::sun::star;
using namespace org::openoffice;


sal_Int32 pt2mm( double pt ) //1/100mm
{
    return static_cast<sal_Int32>(pt * 0.352778);
}

double mm2pt( sal_Int32 mm )
{
    return mm * 2.8345;
}


ScVbaOLEObject::ScVbaOLEObject( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext,
            css::uno::Reference< css::drawing::XControlShape > xControlShape )
: OLEObjectImpl_BASE( xParent, xContext ), m_xControlShape( xControlShape )
{
    //init m_xWindowPeer
    uno::Reference< awt::XControlModel > xControlModel( xControlShape->getControl(), css::uno::UNO_QUERY_THROW );
    uno::Reference< container::XChild > xChild( xControlModel, uno::UNO_QUERY_THROW );
    xChild.set( xChild->getParent(), uno::UNO_QUERY_THROW );
    xChild.set( xChild->getParent(), uno::UNO_QUERY_THROW );
    css::uno::Reference< css::frame::XModel > xModel( xChild->getParent(), uno::UNO_QUERY_THROW );
    css::uno::Reference< css::view::XControlAccess > xControlAccess( xModel->getCurrentController(), css::uno::UNO_QUERY_THROW );
    m_xWindowPeer = xControlAccess->getControl( xControlModel )->getPeer();
}

uno::Reference< uno::XInterface > SAL_CALL
ScVbaOLEObject::getObject() throw (uno::RuntimeException)
{
    return uno::Reference< uno::XInterface >( m_xControlShape, uno::UNO_QUERY_THROW );
}

sal_Bool SAL_CALL
ScVbaOLEObject::getEnabled() throw (uno::RuntimeException)
{
    uno::Reference< css::awt::XWindow2 > xWindow2( m_xWindowPeer, css::uno::UNO_QUERY_THROW );
    return xWindow2->isEnabled();
}

void SAL_CALL
ScVbaOLEObject::setEnabled( sal_Bool _enabled ) throw (uno::RuntimeException)
{
    uno::Reference< css::awt::XWindow2 > xWindow2( m_xWindowPeer, css::uno::UNO_QUERY_THROW );
    xWindow2->setEnable( _enabled );
}

sal_Bool SAL_CALL
ScVbaOLEObject::getVisible() throw (uno::RuntimeException)
{
    uno::Reference< css::awt::XWindow2 > xWindow2( m_xWindowPeer, css::uno::UNO_QUERY_THROW );
    return xWindow2->isVisible();
}

void SAL_CALL
ScVbaOLEObject::setVisible( sal_Bool _visible ) throw (uno::RuntimeException)
{
    uno::Reference< css::awt::XWindow2 > xWindow2( m_xWindowPeer, css::uno::UNO_QUERY_THROW );
    xWindow2->setVisible( _visible );
}

double SAL_CALL
ScVbaOLEObject::getLeft() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    return mm2pt( xShape->getPosition().X / 100 );
}

void SAL_CALL
ScVbaOLEObject::setLeft( double _left ) throw (uno::RuntimeException)
{
    awt::Point oldPosition;
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    oldPosition = xShape->getPosition();
    oldPosition.X = pt2mm( _left ) * 100;
    xShape->setPosition( oldPosition );

}

double SAL_CALL
ScVbaOLEObject::getTop() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    return mm2pt( xShape->getPosition().Y / 100 );
}

void SAL_CALL
ScVbaOLEObject::setTop( double _top ) throw (uno::RuntimeException)
{
    awt::Point oldPosition;
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    oldPosition = xShape->getPosition();
    oldPosition.Y = pt2mm( _top ) * 100;;
    xShape->setPosition( oldPosition );
}

double SAL_CALL
ScVbaOLEObject::getHeight() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    return mm2pt( xShape->getSize().Height / 100 );//1pt = 1/72in
}

void SAL_CALL
ScVbaOLEObject::setHeight( double _height ) throw (uno::RuntimeException)
{
    awt::Size oldSize;
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    oldSize = xShape->getSize();
    oldSize.Height = pt2mm( _height ) * 100;
    xShape->setSize( oldSize );
}

double SAL_CALL
ScVbaOLEObject::getWidth() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    return mm2pt ( xShape->getSize().Width / 100 );
}

void SAL_CALL
ScVbaOLEObject::setWidth( double _width ) throw (uno::RuntimeException)
{
    awt::Size oldSize;
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    oldSize = xShape->getSize();
    oldSize.Width = pt2mm( _width ) * 100;
    xShape->setSize( oldSize );
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
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.OLEObject" ) );
    }
    return aServiceNames;
}
