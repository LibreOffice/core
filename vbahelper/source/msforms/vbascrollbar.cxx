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

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaScrollBar::ScVbaScrollBar(  const css::uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, std::unique_ptr<AbstractGeometryAttributes> pGeomHelper )
    : ScrollBarImpl_BASE( xParent, xContext, xControl, xModel, std::move(pGeomHelper) )
{
}

// Attributes
uno::Any SAL_CALL
ScVbaScrollBar::getValue()
{
    return  m_xProps->getPropertyValue( "ScrollValue" );
}

void SAL_CALL
ScVbaScrollBar::setValue( const uno::Any& _value )
{
    m_xProps->setPropertyValue( "ScrollValue", _value );
}

::sal_Int32 SAL_CALL
ScVbaScrollBar::getMax()
{
    sal_Int32 nMax = 0;
    m_xProps->getPropertyValue( "ScrollValueMax" ) >>= nMax;
    return nMax;
}

void SAL_CALL
ScVbaScrollBar::setMax( sal_Int32 nVal )
{
    m_xProps->setPropertyValue( "ScrollValueMax", uno::makeAny( nVal ) );
}

::sal_Int32 SAL_CALL
ScVbaScrollBar::getMin()
{
    sal_Int32 nVal = 0;
    m_xProps->getPropertyValue( "ScrollValueMin" ) >>= nVal;
    return nVal;
}

void SAL_CALL
ScVbaScrollBar::setMin( sal_Int32 nVal )
{
    m_xProps->setPropertyValue( "ScrollValueMin", uno::makeAny( nVal ) );
}

void SAL_CALL
ScVbaScrollBar::setLargeChange( ::sal_Int32 _largechange )
{
    m_xProps->setPropertyValue( "BlockIncrement", uno::makeAny( _largechange ) );
}

::sal_Int32 SAL_CALL
ScVbaScrollBar::getLargeChange()
{
    sal_Int32 nVal = 0;
    m_xProps->getPropertyValue( "BlockIncrement" ) >>= nVal;
    return nVal;
}

::sal_Int32 SAL_CALL
ScVbaScrollBar::getSmallChange()
{
    sal_Int32 nSmallChange = 0;
    m_xProps->getPropertyValue( "LineIncrement" ) >>= nSmallChange;
    return nSmallChange;
}

void SAL_CALL
ScVbaScrollBar::setSmallChange( ::sal_Int32 _smallchange )
{
    m_xProps->setPropertyValue( "LineIncrement", uno::makeAny( _smallchange ) );
}

OUString
ScVbaScrollBar::getServiceImplName()
{
    return "ScVbaScrollBar";
}

uno::Sequence< OUString >
ScVbaScrollBar::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.msforms.Frame"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
