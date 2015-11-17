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

VbaButton::VbaButton( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper ) : ButtonImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
}

// Attributes
OUString SAL_CALL
VbaButton::getCaption() throw (css::uno::RuntimeException, std::exception)
{
    OUString Label;
    m_xProps->getPropertyValue( "Label" ) >>= Label;
    return Label;
}

void SAL_CALL
VbaButton::setCaption( const OUString& _caption ) throw (css::uno::RuntimeException, std::exception)
{
    m_xProps->setPropertyValue( "Label", uno::makeAny( _caption ) );
}

sal_Bool SAL_CALL VbaButton::getAutoSize() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getAutoSize();
}

void SAL_CALL VbaButton::setAutoSize( sal_Bool bAutoSize ) throw (uno::RuntimeException, std::exception)
{
    ScVbaControl::setAutoSize( bAutoSize );
}

sal_Bool SAL_CALL VbaButton::getLocked() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getLocked();
}

void SAL_CALL VbaButton::setLocked( sal_Bool bLocked ) throw (uno::RuntimeException, std::exception)
{
    ScVbaControl::setLocked( bLocked );
}

sal_Bool SAL_CALL VbaButton::getCancel() throw (uno::RuntimeException, std::exception)
{
    return sal_False;
}

void SAL_CALL VbaButton::setCancel( sal_Bool /*bCancel*/ ) throw (uno::RuntimeException, std::exception)
{
    // #STUB
}

sal_Bool SAL_CALL VbaButton::getDefault() throw (uno::RuntimeException, std::exception)
{
    // #STUB
    return sal_False;
}

void SAL_CALL VbaButton::setDefault( sal_Bool /*bDefault*/ ) throw (uno::RuntimeException, std::exception)
{
    // #STUB
}

sal_Int32 SAL_CALL VbaButton::getBackColor() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL VbaButton::setBackColor( sal_Int32 nBackColor ) throw (uno::RuntimeException, std::exception)
{
    ScVbaControl::setBackColor( nBackColor );
}

sal_Int32 SAL_CALL VbaButton::getForeColor() throw (uno::RuntimeException)
{
    return ScVbaControl::getForeColor();
}

void SAL_CALL VbaButton::setForeColor( sal_Int32 /*nForeColor*/ ) throw (uno::RuntimeException)
{
    // #STUB
}

uno::Reference< msforms::XNewFont > SAL_CALL VbaButton::getFont() throw (uno::RuntimeException, std::exception)
{
    return new VbaNewFont( m_xProps );
}

OUString
VbaButton::getServiceImplName()
{
    return OUString("VbaButton");
}

uno::Sequence< OUString >
VbaButton::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msforms.Button";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
