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
#include "vbascrollbar.hxx"
#include <vector>

using namespace com::sun::star;
using namespace ooo::vba;


const static OUString LARGECHANGE( "BlockIncrement");
const static OUString SMALLCHANGE( "LineIncrement");
const static OUString SCROLLVALUE( "ScrollValue");
const static OUString SCROLLMAX( "ScrollValueMax");
const static OUString SCROLLMIN( "ScrollValueMin");

ScVbaScrollBar::ScVbaScrollBar(  const css::uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper ) : ScrollBarImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
}

// Attributes
uno::Any SAL_CALL
ScVbaScrollBar::getValue() throw (css::uno::RuntimeException)
{
    return  m_xProps->getPropertyValue( SCROLLVALUE );
}

void SAL_CALL
ScVbaScrollBar::setValue( const uno::Any& _value ) throw (::com::sun::star::uno::RuntimeException)
{
    m_xProps->setPropertyValue( SCROLLVALUE, _value );
}

::sal_Int32 SAL_CALL
ScVbaScrollBar::getMax() throw (uno::RuntimeException)
{
    sal_Int32 nMax = 0;
    m_xProps->getPropertyValue( SCROLLMAX ) >>= nMax;
    return nMax;
}

void SAL_CALL
ScVbaScrollBar::setMax( sal_Int32 nVal ) throw (uno::RuntimeException)
{
    m_xProps->setPropertyValue( SCROLLMAX, uno::makeAny( nVal ) );
}

::sal_Int32 SAL_CALL
ScVbaScrollBar::getMin() throw (uno::RuntimeException)
{
    sal_Int32 nVal = 0;
    m_xProps->getPropertyValue( SCROLLMIN ) >>= nVal;
    return nVal;
}

void SAL_CALL
ScVbaScrollBar::setMin( sal_Int32 nVal ) throw (uno::RuntimeException)
{
    m_xProps->setPropertyValue( SCROLLMIN, uno::makeAny( nVal ) );
}

void SAL_CALL
ScVbaScrollBar::setLargeChange( ::sal_Int32 _largechange ) throw (uno::RuntimeException)
{
    m_xProps->setPropertyValue( LARGECHANGE, uno::makeAny( _largechange ) );
}

::sal_Int32 SAL_CALL
ScVbaScrollBar::getLargeChange() throw (uno::RuntimeException)
{
    sal_Int32 nVal = 0;
    m_xProps->getPropertyValue( LARGECHANGE ) >>= nVal;
    return nVal;
}

::sal_Int32 SAL_CALL
ScVbaScrollBar::getSmallChange() throw (uno::RuntimeException)
{
    sal_Int32 nSmallChange = 0;
    m_xProps->getPropertyValue( SMALLCHANGE ) >>= nSmallChange;
    return nSmallChange;
}

void SAL_CALL
ScVbaScrollBar::setSmallChange( ::sal_Int32 _smallchange ) throw (uno::RuntimeException)
{
    m_xProps->setPropertyValue( SMALLCHANGE, uno::makeAny( _smallchange ) );
}

OUString
ScVbaScrollBar::getServiceImplName()
{
    return OUString("ScVbaScrollBar" );
}

uno::Sequence< OUString >
ScVbaScrollBar::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msforms.Frame";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
