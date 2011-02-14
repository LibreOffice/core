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
#include <vbahelper/helperdecl.hxx>
#include <vbahelper/vbawindowbase.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/PosSize.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

VbaWindowBase::VbaWindowBase( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) : WindowBaseImpl_BASE( xParent, xContext ), m_xModel( xModel )
{
}

VbaWindowBase::VbaWindowBase( uno::Sequence< uno::Any > const & args, uno::Reference< uno::XComponentContext > const & xContext )
        : WindowBaseImpl_BASE( getXSomethingFromArgs< XHelperInterface >( args, 0 ), xContext ),
          m_xModel( getXSomethingFromArgs< frame::XModel >( args, 1 ) )
{
}

sal_Bool SAL_CALL
VbaWindowBase::getVisible() throw (uno::RuntimeException)
{
    sal_Bool bVisible = sal_True;
    uno::Reference< frame::XController > xController( m_xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< css::awt::XWindow > xWindow (xController->getFrame()->getContainerWindow(), uno::UNO_QUERY_THROW );
    uno::Reference< css::awt::XWindow2 > xWindow2 (xWindow, uno::UNO_QUERY_THROW );
    if( xWindow2.is() )
    {
        bVisible = xWindow2->isVisible();
    }
    return bVisible;
}

void SAL_CALL
VbaWindowBase::setVisible(sal_Bool _visible) throw (uno::RuntimeException)
{
    uno::Reference< frame::XController > xController( m_xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< css::awt::XWindow > xWindow (xController->getFrame()->getContainerWindow(), uno::UNO_QUERY_THROW );
    if( xWindow.is() )
    {
        xWindow->setVisible( _visible );
    }
}

css::awt::Rectangle getPosSize( const uno::Reference< frame::XModel >& xModel )
{
    css::awt::Rectangle aRect;
    uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< css::awt::XWindow > xWindow (xController->getFrame()->getContainerWindow(), uno::UNO_QUERY_THROW );
    if( xWindow.is() )
    {
        aRect = xWindow->getPosSize();
    }
    return aRect;
}

void setPosSize( const uno::Reference< frame::XModel >& xModel, sal_Int32 nValue, sal_uInt16 nFlag )
{
    uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< css::awt::XWindow > xWindow (xController->getFrame()->getContainerWindow(), uno::UNO_QUERY_THROW );
    if( xWindow.is() )
    {
        css::awt::Rectangle aRect = xWindow->getPosSize();
        switch( nFlag )
        {
            case css::awt::PosSize::X:
                xWindow->setPosSize( nValue, aRect.Y,   0, 0, css::awt::PosSize::X );
                break;
            case css::awt::PosSize::Y:
                xWindow->setPosSize( aRect.X, nValue,   0, 0, css::awt::PosSize::Y );
                break;
            case css::awt::PosSize::WIDTH:
                xWindow->setPosSize( 0, 0,  nValue, aRect.Height, css::awt::PosSize::WIDTH );
                break;
            case css::awt::PosSize::HEIGHT:
                xWindow->setPosSize( 0, 0,  aRect.Width, nValue, css::awt::PosSize::HEIGHT );
                break;
            default:
                break;
        }
    }
}

sal_Int32 SAL_CALL
VbaWindowBase::getHeight() throw (uno::RuntimeException)
{
    css::awt::Rectangle aRect = getPosSize(m_xModel);
    return aRect.Height;
}

void SAL_CALL
VbaWindowBase::setHeight( sal_Int32 _height ) throw (uno::RuntimeException)
{
    setPosSize(m_xModel, _height, css::awt::PosSize::HEIGHT);
}

sal_Int32 SAL_CALL
VbaWindowBase::getLeft() throw (uno::RuntimeException)
{
    css::awt::Rectangle aRect = getPosSize(m_xModel);
    return aRect.X;
}

void SAL_CALL
VbaWindowBase::setLeft( sal_Int32 _left ) throw (uno::RuntimeException)
{
    setPosSize(m_xModel, _left, css::awt::PosSize::X);
}
sal_Int32 SAL_CALL
VbaWindowBase::getTop() throw (uno::RuntimeException)
{
    css::awt::Rectangle aRect = getPosSize(m_xModel);
    return aRect.Y;
}

void SAL_CALL
VbaWindowBase::setTop( sal_Int32 _top ) throw (uno::RuntimeException)
{
    setPosSize(m_xModel, _top, css::awt::PosSize::Y);
}
sal_Int32 SAL_CALL
VbaWindowBase::getWidth() throw (uno::RuntimeException)
{
    css::awt::Rectangle aRect = getPosSize(m_xModel);
    return aRect.Width;
}

void SAL_CALL
VbaWindowBase::setWidth( sal_Int32 _width ) throw (uno::RuntimeException)
{
    setPosSize(m_xModel, _width, css::awt::PosSize::WIDTH);
}

rtl::OUString&
VbaWindowBase::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("VbaWindowBase") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
VbaWindowBase::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.VbaWindowBase" ) );
    }
    return aServiceNames;
}
