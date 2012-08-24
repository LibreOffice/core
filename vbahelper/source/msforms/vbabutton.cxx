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
    return ScVbaControl::getAutoSize();
}

void SAL_CALL ScVbaButton::setAutoSize( sal_Bool bAutoSize ) throw (uno::RuntimeException)
{
    ScVbaControl::setAutoSize( bAutoSize );
}

sal_Bool SAL_CALL ScVbaButton::getLocked() throw (uno::RuntimeException)
{
    return ScVbaControl::getLocked();
}

void SAL_CALL ScVbaButton::setLocked( sal_Bool bLocked ) throw (uno::RuntimeException)
{
    ScVbaControl::setLocked( bLocked );
}

sal_Bool SAL_CALL ScVbaButton::getCancel() throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL ScVbaButton::setCancel( sal_Bool /*bCancel*/ ) throw (uno::RuntimeException)
{
    // #STUB
}

sal_Bool SAL_CALL ScVbaButton::getDefault() throw (uno::RuntimeException)
{
    // #STUB
    return sal_False;
}

void SAL_CALL ScVbaButton::setDefault( sal_Bool /*bDefault*/ ) throw (uno::RuntimeException)
{
    // #STUB
}

sal_Int32 SAL_CALL ScVbaButton::getBackColor() throw (uno::RuntimeException)
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL ScVbaButton::setBackColor( sal_Int32 nBackColor ) throw (uno::RuntimeException)
{
    ScVbaControl::setBackColor( nBackColor );
}

sal_Int32 SAL_CALL ScVbaButton::getForeColor() throw (uno::RuntimeException)
{
    return ScVbaControl::getForeColor();
}

void SAL_CALL ScVbaButton::setForeColor( sal_Int32 /*nForeColor*/ ) throw (uno::RuntimeException)
{
    // #STUB
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaButton::getFont() throw (uno::RuntimeException)
{
    return new VbaNewFont( this, mxContext, m_xProps );
}

rtl::OUString
ScVbaButton::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaButton"));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
