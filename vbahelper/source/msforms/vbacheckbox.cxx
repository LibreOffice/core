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

#include "vbacheckbox.hxx"
#include "vbanewfont.hxx"
#include <vbahelper/helperdecl.hxx>

using namespace com::sun::star;
using namespace ooo::vba;


const static rtl::OUString LABEL( RTL_CONSTASCII_USTRINGPARAM("Label") );
const static rtl::OUString STATE( RTL_CONSTASCII_USTRINGPARAM("State") );
ScVbaCheckbox::ScVbaCheckbox( const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper ) : CheckBoxImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
}

// Attributes
rtl::OUString SAL_CALL
ScVbaCheckbox::getCaption() throw (css::uno::RuntimeException)
{
    rtl::OUString Label;
    m_xProps->getPropertyValue( LABEL ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaCheckbox::setCaption( const rtl::OUString& _caption ) throw (::com::sun::star::uno::RuntimeException)
{
    m_xProps->setPropertyValue( LABEL, uno::makeAny( _caption ) );
}

uno::Any SAL_CALL
ScVbaCheckbox::getValue() throw (css::uno::RuntimeException)
{
    sal_Int16 nValue = -1;
    m_xProps->getPropertyValue( STATE ) >>= nValue;
    if( nValue != 0 )
        nValue = -1;
//    return uno::makeAny( nValue );
// I must be missing something MSO says value should be -1 if selected, 0 if not
// selected
    return uno::makeAny( ( nValue == -1 ) ? sal_True : sal_False );
}

void SAL_CALL
ScVbaCheckbox::setValue( const uno::Any& _value ) throw (css::uno::RuntimeException)
{
    sal_Int16 nValue = 0;
    sal_Bool bValue = false;
    if( _value >>= nValue )
    {
        if( nValue == -1)
            nValue = 1;
    }
    else if ( _value >>= bValue )
    {
        if ( bValue )
            nValue = 1;
    }
    m_xProps->setPropertyValue( STATE, uno::makeAny( nValue ) );
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaCheckbox::getFont() throw (uno::RuntimeException)
{
    return new VbaNewFont( this, mxContext, m_xProps );
}

rtl::OUString&
ScVbaCheckbox::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaCheckbox") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaCheckbox::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msforms.CheckBox" ) );
    }
    return aServiceNames;
}

