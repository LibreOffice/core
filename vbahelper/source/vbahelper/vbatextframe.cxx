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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <utility>
#include <vbahelper/vbatextframe.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

VbaTextFrame::VbaTextFrame( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, uno::Reference< drawing::XShape > xShape ) : VbaTextFrame_BASE( xParent, xContext ), m_xShape(std::move( xShape ))
{
    m_xPropertySet.set( m_xShape, uno::UNO_QUERY_THROW );
}

void
VbaTextFrame::setAsMSObehavior()
{
    //set property TextWordWrap default as False.
    // TextFitToSize control the text content. It seems we should set the default as False.
    // com.sun.star.drawing.TextFitToSizeType.NONE
    m_xPropertySet->setPropertyValue( u"TextWordWrap"_ustr, uno::Any( false ) );
    m_xPropertySet->setPropertyValue( u"TextFitToSize"_ustr, uno::Any( drawing::TextFitToSizeType_NONE ) );
}

sal_Int32 VbaTextFrame::getMargin( const OUString& sMarginType )
{
    sal_Int32 nMargin = 0;
    uno::Any aMargin = m_xPropertySet->getPropertyValue( sMarginType );
    aMargin >>= nMargin;
    return nMargin;
}

void VbaTextFrame::setMargin( const OUString& sMarginType, float fMargin )
{
    sal_Int32 nMargin = Millimeter::getInHundredthsOfOneMillimeter( fMargin );
    m_xPropertySet->setPropertyValue( sMarginType, uno::Any( nMargin ) );
}

// Attributes
sal_Bool SAL_CALL
VbaTextFrame::getAutoSize()
{
    // I don't know why, but in OOo, TextAutoGrowHeight is the property control autosize. not TextFitToSize.
    // TextFitToSize control the text content.
    // and in mso, there isnot option TextWordWrap which means auto wrap. the default is False.
    bool bAutosize = false;
    uno::Any aTextAutoGrowHeight = m_xPropertySet->getPropertyValue( u"TextAutoGrowHeight"_ustr );
    aTextAutoGrowHeight >>= bAutosize;
    return bAutosize;
}

void SAL_CALL
VbaTextFrame::setAutoSize( sal_Bool _autosize )
{
    setAsMSObehavior();
    m_xPropertySet->setPropertyValue( u"TextAutoGrowHeight"_ustr, uno::Any( _autosize ) );
}

float SAL_CALL
VbaTextFrame::getMarginBottom()
{
    sal_Int32 nMargin = getMargin( u"TextLowerDistance"_ustr );
    float fMargin = static_cast<float>(Millimeter::getInPoints( nMargin ));
    return fMargin;
}

void SAL_CALL
VbaTextFrame::setMarginBottom( float _marginbottom )
{
    setMargin( u"TextLowerDistance"_ustr, _marginbottom );
}

float SAL_CALL
VbaTextFrame::getMarginTop()
{
    sal_Int32 nMargin = getMargin( u"TextUpperDistance"_ustr );
    float fMargin = static_cast<float>(Millimeter::getInPoints( nMargin ));
    return fMargin;
}

void SAL_CALL
VbaTextFrame::setMarginTop( float _margintop )
{
    setMargin( u"TextUpperDistance"_ustr, _margintop );
}

float SAL_CALL
VbaTextFrame::getMarginLeft()
{
    sal_Int32 nMargin = getMargin( u"TextLeftDistance"_ustr );
    float fMargin = static_cast<float>(Millimeter::getInPoints( nMargin ));
    return fMargin;
}

void SAL_CALL
VbaTextFrame::setMarginLeft( float _marginleft )
{
    setMargin( u"TextLeftDistance"_ustr, _marginleft );
}

float SAL_CALL
VbaTextFrame::getMarginRight()
{
    sal_Int32 nMargin = getMargin( u"TextRightDistance"_ustr );
    float fMargin = static_cast<float>(Millimeter::getInPoints( nMargin ));
    return fMargin;
}

void SAL_CALL
VbaTextFrame::setMarginRight( float _marginright )
{
    setMargin( u"TextRightDistance"_ustr , _marginright );
}


// Methods
uno::Any SAL_CALL
VbaTextFrame::Characters()
{
    throw uno::RuntimeException( u"Not implemented"_ustr );
}

OUString
VbaTextFrame::getServiceImplName()
{
    return u"VbaTextFrame"_ustr;
}

uno::Sequence< OUString >
VbaTextFrame::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.msforms.TextFrame"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
