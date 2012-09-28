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


const static OUString LABEL( "Label" );
ScVbaLabel::ScVbaLabel(  const css::uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper ) : LabelImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
}

// Attributes
OUString SAL_CALL
ScVbaLabel::getCaption() throw (css::uno::RuntimeException)
{
    OUString Label;
    m_xProps->getPropertyValue( LABEL ) >>= Label;
    return Label;
}

void SAL_CALL
ScVbaLabel::setCaption( const OUString& _caption ) throw (::com::sun::star::uno::RuntimeException)
{
    m_xProps->setPropertyValue( LABEL, uno::makeAny( _caption ) );
}
uno::Any SAL_CALL
ScVbaLabel::getValue() throw (css::uno::RuntimeException)
{
    return uno::makeAny( getCaption() );
}

void SAL_CALL
ScVbaLabel::setValue( const uno::Any& _value ) throw (::com::sun::star::uno::RuntimeException)
{
    OUString sCaption;
    _value >>= sCaption;
    setCaption( sCaption );
}

OUString SAL_CALL
ScVbaLabel::getAccelerator() throw (css::uno::RuntimeException)
{
    // #STUB
    return OUString();
}

void SAL_CALL
ScVbaLabel::setAccelerator( const OUString& /*_accelerator*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    // #STUB
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaLabel::getFont() throw (uno::RuntimeException)
{
    return new VbaNewFont( this, mxContext, m_xProps );
}

OUString ScVbaLabel::getServiceImplName()
{
    return OUString( "ScVbaLabel" );
}

sal_Int32 SAL_CALL ScVbaLabel::getBackColor() throw (uno::RuntimeException)
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL ScVbaLabel::setBackColor( sal_Int32 nBackColor ) throw (uno::RuntimeException)
{
    ScVbaControl::setBackColor( nBackColor );
}

sal_Bool SAL_CALL ScVbaLabel::getAutoSize() throw (uno::RuntimeException)
{
    return ScVbaControl::getAutoSize();
}

void SAL_CALL ScVbaLabel::setAutoSize( sal_Bool bAutoSize ) throw (uno::RuntimeException)
{
    ScVbaControl::setAutoSize( bAutoSize );
}

uno::Sequence< OUString >
ScVbaLabel::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msforms.Label";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
