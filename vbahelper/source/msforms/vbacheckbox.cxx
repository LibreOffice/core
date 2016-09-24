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

#include "vbacheckbox.hxx"
#include "vbanewfont.hxx"
#include <vbahelper/helperdecl.hxx>

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaCheckbox::ScVbaCheckbox( const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper ) : CheckBoxImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
}

// Attributes
OUString SAL_CALL
ScVbaCheckbox::getCaption() throw (css::uno::RuntimeException, std::exception)
{
    OUString Label;
    m_xProps->getPropertyValue( "Label" ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaCheckbox::setCaption( const OUString& _caption ) throw (css::uno::RuntimeException, std::exception)
{
    m_xProps->setPropertyValue( "Label", uno::makeAny( _caption ) );
}

uno::Any SAL_CALL
ScVbaCheckbox::getValue() throw (css::uno::RuntimeException, std::exception)
{
    sal_Int16 nValue = -1;
    m_xProps->getPropertyValue( "State" ) >>= nValue;
    if( nValue != 0 )
        nValue = -1;
//    return uno::makeAny( nValue );
// I must be missing something MSO says value should be -1 if selected, 0 if not
// selected
    return uno::makeAny( nValue == -1 );
}

void SAL_CALL
ScVbaCheckbox::setValue( const uno::Any& _value ) throw (css::uno::RuntimeException, std::exception)
{
    sal_Int16 nValue = 0;
    sal_Int16 nOldValue = 0;
    m_xProps->getPropertyValue( "State" ) >>= nOldValue;
    if( !( _value >>= nValue ) )
    {
        bool bValue = false;
        _value >>= bValue;
        if ( bValue )
            nValue = -1;
    }

    if( nValue == -1)
        nValue = 1;
    m_xProps->setPropertyValue( "State", uno::makeAny( nValue ) );
    if ( nValue != nOldValue )
        fireClickEvent();
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaCheckbox::getFont() throw (uno::RuntimeException, std::exception)
{
    return new VbaNewFont( m_xProps );
}

OUString
ScVbaCheckbox::getServiceImplName()
{
    return OUString("ScVbaCheckbox");
}

uno::Sequence< OUString >
ScVbaCheckbox::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msforms.CheckBox";
    }
    return aServiceNames;
}

sal_Int32 SAL_CALL ScVbaCheckbox::getBackColor() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL ScVbaCheckbox::setBackColor( sal_Int32 nBackColor ) throw (uno::RuntimeException, std::exception)
{
    ScVbaControl::setBackColor( nBackColor );
}

sal_Bool SAL_CALL ScVbaCheckbox::getAutoSize() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getAutoSize();
}

void SAL_CALL ScVbaCheckbox::setAutoSize( sal_Bool bAutoSize ) throw (uno::RuntimeException, std::exception)
{
    ScVbaControl::setAutoSize( bAutoSize );
}

sal_Bool SAL_CALL ScVbaCheckbox::getLocked() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getLocked();
}

void SAL_CALL ScVbaCheckbox::setLocked( sal_Bool bLocked ) throw (uno::RuntimeException, std::exception)
{
    ScVbaControl::setLocked( bLocked );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
