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
#include <sal/log.hxx>

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaToggleButton::ScVbaToggleButton( const css::uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, std::unique_ptr<ov::AbstractGeometryAttributes> pGeomHelper )
    : ToggleButtonImpl_BASE( xParent, xContext, xControl, xModel, std::move(pGeomHelper) )
{
    SAL_INFO("vbahelper", "ScVbaToggleButton(ctor)");
    m_xProps->setPropertyValue( "Toggle", uno::makeAny( true ) );
}

ScVbaToggleButton::~ScVbaToggleButton()
{
    SAL_INFO("vbahelper", "~ScVbaToggleButton(dtor)");
}

// Attributes
OUString SAL_CALL
ScVbaToggleButton::getCaption()
{
    OUString Label;
    m_xProps->getPropertyValue( "Label" ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaToggleButton::setCaption( const OUString& _caption )
{
    m_xProps->setPropertyValue( "Label", uno::makeAny( _caption ) );
}

uno::Any SAL_CALL
ScVbaToggleButton::getValue()
{
    sal_Int16 nState = 0;
    m_xProps->getPropertyValue( "State" ) >>= nState;
    return uno::makeAny( nState ? sal_Int16( -1 ) : sal_Int16( 0 ) );
}


void SAL_CALL
ScVbaToggleButton::setValue( const uno::Any& _value )
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

sal_Bool SAL_CALL ScVbaToggleButton::getAutoSize()
{
    return ScVbaControl::getAutoSize();
}

void SAL_CALL ScVbaToggleButton::setAutoSize( sal_Bool bAutoSize )
{
    ScVbaControl::setAutoSize( bAutoSize );
}

sal_Bool SAL_CALL ScVbaToggleButton::getCancel()
{
    // #STUB
    return false;
}

void SAL_CALL ScVbaToggleButton::setCancel( sal_Bool /*bCancel*/ )
{
    // #STUB
}

sal_Bool SAL_CALL ScVbaToggleButton::getDefault()
{
    // #STUB
    return false;
}

void SAL_CALL ScVbaToggleButton::setDefault( sal_Bool /*bDefault*/ )
{
    // #STUB
}

sal_Int32 SAL_CALL ScVbaToggleButton::getBackColor()
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL ScVbaToggleButton::setBackColor( sal_Int32 nBackColor )
{
    ScVbaControl::setBackColor( nBackColor );
}

sal_Int32 SAL_CALL ScVbaToggleButton::getForeColor()
{
    // #STUB
    return 0;
}

void SAL_CALL ScVbaToggleButton::setForeColor( sal_Int32 /*nForeColor*/ )
{
    // #STUB
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaToggleButton::getFont()
{
    return new VbaNewFont( m_xProps );
}

sal_Bool SAL_CALL ScVbaToggleButton::getLocked()
{
    return ScVbaControl::getLocked();
}

void SAL_CALL ScVbaToggleButton::setLocked( sal_Bool bLocked )
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
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.msforms.ToggleButton"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
