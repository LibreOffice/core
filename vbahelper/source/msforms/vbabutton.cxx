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

#include "vbabutton.hxx"
#include "vbanewfont.hxx"

using namespace com::sun::star;
using namespace ooo::vba;


const static rtl::OUString LABEL( RTL_CONSTASCII_USTRINGPARAM("Label") );
ScVbaButton::ScVbaButton( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper ) : ButtonImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
}

// Attributes
rtl::OUString SAL_CALL
ScVbaButton::getCaption() throw (css::uno::RuntimeException)
{
    rtl::OUString Label;
    m_xProps->getPropertyValue( LABEL ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaButton::setCaption( const rtl::OUString& _caption ) throw (::com::sun::star::uno::RuntimeException)
{
    m_xProps->setPropertyValue( LABEL, uno::makeAny( _caption ) );
}

sal_Bool SAL_CALL ScVbaButton::getAutoSize() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL ScVbaButton::setAutoSize( sal_Bool /*bAutoSize*/ ) throw (uno::RuntimeException)
{
}

sal_Bool SAL_CALL ScVbaButton::getCancel() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL ScVbaButton::setCancel( sal_Bool /*bCancel*/ ) throw (uno::RuntimeException)
{
}

sal_Bool SAL_CALL ScVbaButton::getDefault() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL ScVbaButton::setDefault( sal_Bool /*bDefault*/ ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL ScVbaButton::getBackColor() throw (uno::RuntimeException)
{
    return 0;
}

void SAL_CALL ScVbaButton::setBackColor( sal_Int32 /*nBackColor*/ ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL ScVbaButton::getForeColor() throw (uno::RuntimeException)
{
    return 0;
}

void SAL_CALL ScVbaButton::setForeColor( sal_Int32 /*nForeColor*/ ) throw (uno::RuntimeException)
{
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaButton::getFont() throw (uno::RuntimeException)
{
    return new VbaNewFont( this, mxContext, m_xProps );
}

rtl::OUString&
ScVbaButton::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaButton") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaButton::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msforms.Button" ) );
    }
    return aServiceNames;
}
