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

ScVbaToggleButton::ScVbaToggleButton( const css::uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper ) : ToggleButtonImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
    SAL_INFO("vbahelper", "ScVbaToggleButton(ctor)");
    m_xProps->setPropertyValue( "Toggle", uno::makeAny( sal_True ) );
}

ScVbaToggleButton::~ScVbaToggleButton()
{
    SAL_INFO("vbahelper", "~ScVbaToggleButton(dtor)");
}

// Attributes
OUString SAL_CALL
ScVbaToggleButton::getCaption() throw (css::uno::RuntimeException, std::exception)
{
    OUString Label;
    m_xProps->getPropertyValue( "Label" ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaToggleButton::setCaption( const OUString& _caption ) throw (css::uno::RuntimeException, std::exception)
{
    m_xProps->setPropertyValue( "Label", uno::makeAny( _caption ) );
}

uno::Any SAL_CALL
ScVbaToggleButton::getValue() throw (uno::RuntimeException, std::exception)
{
    sal_Int16 nState = 0;
        m_xProps->getPropertyValue( "State" ) >>= nState;
     return uno::makeAny( nState ? sal_Int16( -1 ) : sal_Int16( 0 ) );
}


void SAL_CALL
ScVbaToggleButton::setValue( const uno::Any& _value ) throw (uno::RuntimeException, std::exception)
{
    sal_Int16 nState = 0;
    if ( ! ( _value >>= nState ) )
    {
        bool bState = false;
        _value >>= bState;
        if ( bState )
            nState = -1;
    }
    SAL_INFO("vbahelper", "nState - " << nState );
    nState = ( nState == -1 ) ?  1 : 0;
    SAL_INFO("vbahelper", "nState - " << nState );
    m_xProps->setPropertyValue( "State", uno::makeAny(   nState ) );
}

sal_Bool SAL_CALL ScVbaToggleButton::getAutoSize() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getAutoSize();
}

void SAL_CALL ScVbaToggleButton::setAutoSize( sal_Bool bAutoSize ) throw (uno::RuntimeException, std::exception)
{
    ScVbaControl::setAutoSize( bAutoSize );
}

sal_Bool SAL_CALL ScVbaToggleButton::getCancel() throw (uno::RuntimeException, std::exception)
{
    // #STUB
    return sal_False;
}

void SAL_CALL ScVbaToggleButton::setCancel( sal_Bool /*bCancel*/ ) throw (uno::RuntimeException, std::exception)
{
    // #STUB
}

sal_Bool SAL_CALL ScVbaToggleButton::getDefault() throw (uno::RuntimeException, std::exception)
{
    // #STUB
    return sal_False;
}

void SAL_CALL ScVbaToggleButton::setDefault( sal_Bool /*bDefault*/ ) throw (uno::RuntimeException, std::exception)
{
    // #STUB
}

sal_Int32 SAL_CALL ScVbaToggleButton::getBackColor() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL ScVbaToggleButton::setBackColor( sal_Int32 nBackColor ) throw (uno::RuntimeException, std::exception)
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

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaToggleButton::getFont() throw (uno::RuntimeException, std::exception)
{
    return new VbaNewFont( m_xProps );
}

sal_Bool SAL_CALL ScVbaToggleButton::getLocked() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getLocked();
}

void SAL_CALL ScVbaToggleButton::setLocked( sal_Bool bLocked ) throw (uno::RuntimeException, std::exception)
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
