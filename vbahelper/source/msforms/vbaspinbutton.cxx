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
#include "vbaspinbutton.hxx"
#include <vector>

using namespace com::sun::star;
using namespace ooo::vba;


const static rtl::OUString ORIENTATION( RTL_CONSTASCII_USTRINGPARAM("Orientation") );
const static rtl::OUString SPINVALUE( RTL_CONSTASCII_USTRINGPARAM("SpinValue") );
const static rtl::OUString SPINMAX( RTL_CONSTASCII_USTRINGPARAM("SpinValueMax") );
const static rtl::OUString SPINMIN( RTL_CONSTASCII_USTRINGPARAM("SpinValueMin") );

ScVbaSpinButton::ScVbaSpinButton(  const css::uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper ) : SpinButtonImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
}

// Attributes
uno::Any SAL_CALL
ScVbaSpinButton::getValue() throw (css::uno::RuntimeException)
{
    return  m_xProps->getPropertyValue( SPINVALUE );
}

void SAL_CALL
ScVbaSpinButton::setValue( const uno::Any& _value ) throw (::com::sun::star::uno::RuntimeException)
{
    m_xProps->setPropertyValue( SPINVALUE, _value );
}

::sal_Int32 SAL_CALL
ScVbaSpinButton::getMax() throw (uno::RuntimeException)
{
    sal_Int32 nMax = 0;
    m_xProps->getPropertyValue( SPINMAX ) >>= nMax;
    return nMax;
}

void SAL_CALL
ScVbaSpinButton::setMax( sal_Int32 nVal ) throw (uno::RuntimeException)
{
    m_xProps->setPropertyValue( SPINMAX, uno::makeAny( nVal ) );
}

::sal_Int32 SAL_CALL
ScVbaSpinButton::getMin() throw (uno::RuntimeException)
{
    sal_Int32 nVal = 0;
    m_xProps->getPropertyValue( SPINMIN ) >>= nVal;
    return nVal;
}

void SAL_CALL
ScVbaSpinButton::setMin( sal_Int32 nVal ) throw (uno::RuntimeException)
{
    m_xProps->setPropertyValue( SPINMIN, uno::makeAny( nVal ) );
}

rtl::OUString&
ScVbaSpinButton::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaSpinButton") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaSpinButton::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msforms.Frame" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
