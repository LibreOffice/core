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

VbaButton::VbaButton( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, std::unique_ptr<AbstractGeometryAttributes> pGeomHelper )
    : ButtonImpl_BASE( xParent, xContext, xControl, xModel, std::move(pGeomHelper) )
{
}

// Attributes
OUString SAL_CALL
VbaButton::getCaption()
{
    OUString Label;
    m_xProps->getPropertyValue( u"Label"_ustr ) >>= Label;
    return Label;
}

void SAL_CALL
VbaButton::setCaption( const OUString& _caption )
{
    m_xProps->setPropertyValue( u"Label"_ustr, uno::Any( _caption ) );
}

sal_Bool SAL_CALL VbaButton::getAutoSize()
{
    return ScVbaControl::getAutoSize();
}

void SAL_CALL VbaButton::setAutoSize( sal_Bool bAutoSize )
{
    ScVbaControl::setAutoSize( bAutoSize );
}

sal_Bool SAL_CALL VbaButton::getLocked()
{
    return ScVbaControl::getLocked();
}

void SAL_CALL VbaButton::setLocked( sal_Bool bLocked )
{
    ScVbaControl::setLocked( bLocked );
}

sal_Bool SAL_CALL VbaButton::getCancel()
{
    return false;
}

void SAL_CALL VbaButton::setCancel( sal_Bool /*bCancel*/ )
{
    // #STUB
}

sal_Bool SAL_CALL VbaButton::getDefault()
{
    // #STUB
    return false;
}

void SAL_CALL VbaButton::setDefault( sal_Bool /*bDefault*/ )
{
    // #STUB
}

sal_Int32 SAL_CALL VbaButton::getBackColor()
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL VbaButton::setBackColor( sal_Int32 nBackColor )
{
    ScVbaControl::setBackColor( nBackColor );
}

sal_Int32 SAL_CALL VbaButton::getForeColor()
{
    return ScVbaControl::getForeColor();
}

void SAL_CALL VbaButton::setForeColor( sal_Int32 /*nForeColor*/ )
{
    // #STUB
}

uno::Reference< msforms::XNewFont > SAL_CALL VbaButton::getFont()
{
    return new VbaNewFont( m_xProps );
}

OUString
VbaButton::getServiceImplName()
{
    return u"VbaButton"_ustr;
}

uno::Sequence< OUString >
VbaButton::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.msforms.Button"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
