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

#include "vbalabel.hxx"
#include "vbanewfont.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaLabel::ScVbaLabel(  const css::uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, std::unique_ptr<ov::AbstractGeometryAttributes> pGeomHelper )
    : LabelImpl_BASE( xParent, xContext, xControl, xModel, std::move(pGeomHelper) )
{
}

// Attributes
OUString SAL_CALL
ScVbaLabel::getCaption()
{
    OUString Label;
    m_xProps->getPropertyValue( u"Label"_ustr ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaLabel::setCaption( const OUString& _caption )
{
    m_xProps->setPropertyValue( u"Label"_ustr, uno::Any( _caption ) );
}
uno::Any SAL_CALL
ScVbaLabel::getValue()
{
    return uno::Any( getCaption() );
}

void SAL_CALL
ScVbaLabel::setValue( const uno::Any& _value )
{
    OUString sCaption;
    _value >>= sCaption;
    setCaption( sCaption );
}

OUString SAL_CALL
ScVbaLabel::getAccelerator()
{
    // #STUB
    return OUString();
}

void SAL_CALL
ScVbaLabel::setAccelerator( const OUString& /*_accelerator*/ )
{
    // #STUB
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaLabel::getFont()
{
    return new VbaNewFont( m_xProps );
}

OUString ScVbaLabel::getServiceImplName()
{
    return u"ScVbaLabel"_ustr;
}

sal_Int32 SAL_CALL ScVbaLabel::getBackColor()
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL ScVbaLabel::setBackColor( sal_Int32 nBackColor )
{
    ScVbaControl::setBackColor( nBackColor );
}

sal_Bool SAL_CALL ScVbaLabel::getAutoSize()
{
    return ScVbaControl::getAutoSize();
}

void SAL_CALL ScVbaLabel::setAutoSize( sal_Bool bAutoSize )
{
    ScVbaControl::setAutoSize( bAutoSize );
}

uno::Sequence< OUString >
ScVbaLabel::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.msforms.Label"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
