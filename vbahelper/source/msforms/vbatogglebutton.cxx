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
#include "vbatogglebutton.hxx"
#include <vector>

using namespace com::sun::star;
using namespace ooo::vba;


const static rtl::OUString LABEL( RTL_CONSTASCII_USTRINGPARAM("Label") );
const static rtl::OUString TOGGLE( RTL_CONSTASCII_USTRINGPARAM("Toggle") );
const static rtl::OUString STATE( RTL_CONSTASCII_USTRINGPARAM("State") );
ScVbaToggleButton::ScVbaToggleButton( const css::uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper ) : ToggleButtonImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
    OSL_TRACE("ScVbaToggleButton(ctor)");
    m_xProps->setPropertyValue( TOGGLE, uno::makeAny( sal_True ) );
}

ScVbaToggleButton::~ScVbaToggleButton()
{
    OSL_TRACE("~ScVbaToggleButton(dtor)");
}

// Attributes
rtl::OUString SAL_CALL
ScVbaToggleButton::getCaption() throw (css::uno::RuntimeException)
{
    rtl::OUString Label;
    m_xProps->getPropertyValue( LABEL ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaToggleButton::setCaption( const rtl::OUString& _caption ) throw (::com::sun::star::uno::RuntimeException)
{
    m_xProps->setPropertyValue( LABEL, uno::makeAny( _caption ) );
}

uno::Any SAL_CALL
ScVbaToggleButton::getValue() throw (uno::RuntimeException)
{
    sal_Int16 nState = 0;
        m_xProps->getPropertyValue( STATE ) >>= nState;
     return uno::makeAny( nState ? sal_Int16( -1 ) : sal_Int16( 0 ) );
}


void SAL_CALL
ScVbaToggleButton::setValue( const uno::Any& _value ) throw (uno::RuntimeException)
{
    sal_Int16 nState = 0;
    if (_value.getValueTypeClass() == uno::TypeClass_BOOLEAN)
    {
        sal_Bool bValue;
        _value >>= bValue;
        nState = static_cast< sal_Int16 >(bValue);
    }
    else if (_value.getValueTypeClass() == uno::TypeClass_BYTE)
    {
        sal_Int8 nValue;
        _value >>= nValue;
        nState = ( nValue == 1) ? 1 : 0;
    }
    else
    {
        _value >>= nState;
        OSL_TRACE( "nState - %d", nState );
        nState = ( nState == -1 ) ?  1 : 0;
        OSL_TRACE( "nState - %d", nState );
    }
    m_xProps->setPropertyValue( STATE, uno::makeAny(  nState ) );
}


rtl::OUString&
ScVbaToggleButton::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaToggleButton") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaToggleButton::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msforms.ToggleButton" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
