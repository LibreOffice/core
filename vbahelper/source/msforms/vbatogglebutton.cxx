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

#include "vbatogglebutton.hxx"
#include "vbanewfont.hxx"

using namespace com::sun::star;
using namespace ooo::vba;


const static OUString LABEL( "Label" );
const static OUString TOGGLE( "Toggle" );
const static OUString STATE( "State" );
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
OUString SAL_CALL
ScVbaToggleButton::getCaption() throw (css::uno::RuntimeException)
{
    OUString Label;
    m_xProps->getPropertyValue( LABEL ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaToggleButton::setCaption( const OUString& _caption ) throw (::com::sun::star::uno::RuntimeException)
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
    _value >>= nState;
    OSL_TRACE( "nState - %d", nState );
    nState = ( nState == -1 ) ?  1 : 0;
    OSL_TRACE( "nState - %d", nState );
    m_xProps->setPropertyValue( STATE, uno::makeAny(  nState ) );
}

sal_Bool SAL_CALL ScVbaToggleButton::getAutoSize() throw (uno::RuntimeException)
{
    return ScVbaControl::getAutoSize();
}

void SAL_CALL ScVbaToggleButton::setAutoSize( sal_Bool bAutoSize ) throw (uno::RuntimeException)
{
    ScVbaControl::setAutoSize( bAutoSize );
}

sal_Bool SAL_CALL ScVbaToggleButton::getCancel() throw (uno::RuntimeException)
{
    // #STUB
    return sal_False;
}

void SAL_CALL ScVbaToggleButton::setCancel( sal_Bool /*bCancel*/ ) throw (uno::RuntimeException)
{
    // #STUB
}

sal_Bool SAL_CALL ScVbaToggleButton::getDefault() throw (uno::RuntimeException)
{
    // #STUB
    return sal_False;
}

void SAL_CALL ScVbaToggleButton::setDefault( sal_Bool /*bDefault*/ ) throw (uno::RuntimeException)
{
    // #STUB
}

sal_Int32 SAL_CALL ScVbaToggleButton::getBackColor() throw (uno::RuntimeException)
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL ScVbaToggleButton::setBackColor( sal_Int32 nBackColor ) throw (uno::RuntimeException)
{
    ScVbaControl::setBackColor( nBackColor );
}

sal_Int32 SAL_CALL ScVbaToggleButton::getForeColor() throw (uno::RuntimeException)
{
    // #STUB
    return 0;
}

void SAL_CALL ScVbaToggleButton::setForeColor( sal_Int32 /*nForeColor*/ ) throw (uno::RuntimeException)
{
    // #STUB
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaToggleButton::getFont() throw (uno::RuntimeException)
{
    return new VbaNewFont( this, mxContext, m_xProps );
}

sal_Bool SAL_CALL ScVbaToggleButton::getLocked() throw (uno::RuntimeException)
{
    return ScVbaControl::getLocked();
}

void SAL_CALL ScVbaToggleButton::setLocked( sal_Bool bLocked ) throw (uno::RuntimeException)
{
    ScVbaControl::setLocked( bLocked );
}

OUString
ScVbaToggleButton::getServiceImplName()
{
    return OUString( "ScVbaToggleButton" );
}

uno::Sequence< OUString >
ScVbaToggleButton::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msforms.ToggleButton";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
