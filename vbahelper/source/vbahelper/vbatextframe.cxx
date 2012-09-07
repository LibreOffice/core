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
#include <vbahelper/helperdecl.hxx>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/text/XText.hpp>
#include <vbahelper/vbatextframe.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

VbaTextFrame::VbaTextFrame( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, uno::Reference< drawing::XShape > xShape ) : VbaTextFrame_BASE( xParent, xContext ), m_xShape( xShape )
{
    m_xPropertySet.set( m_xShape, uno::UNO_QUERY_THROW );
}

void
VbaTextFrame::setAsMSObehavior()
{
    //set property TextWordWrap default as False.
    // TextFitToSize control the text content. it seems we should set the default as False.
    // com.sun.star.drawing.TextFitToSizeType.NONE
    m_xPropertySet->setPropertyValue( "TextWordWrap", uno::makeAny( sal_False ) );
    m_xPropertySet->setPropertyValue( "TextFitToSize", uno::makeAny( drawing::TextFitToSizeType_NONE ) );
}

sal_Int32 VbaTextFrame::getMargin( OUString sMarginType )
{
    sal_Int32 nMargin = 0;
    uno::Any aMargin = m_xPropertySet->getPropertyValue( sMarginType );
    aMargin >>= nMargin;
    return nMargin;
}

void VbaTextFrame::setMargin( OUString sMarginType, float fMargin )
{
    sal_Int32 nMargin = Millimeter::getInHundredthsOfOneMillimeter( fMargin );
    m_xPropertySet->setPropertyValue( sMarginType, uno::makeAny( nMargin ) );
}

// Attributes
sal_Bool SAL_CALL
VbaTextFrame::getAutoSize() throw (uno::RuntimeException)
{
    // I don't know why, but in OOo, TextAutoGrowHeight is the property control autosize. not TextFitToSize.
    // TextFitToSize control the text content.
    // and in mso, there isnot option TextWordWrap which means auto wrap. the default is False.
    sal_Bool bAutosize = sal_False;
    uno::Any aTextAutoGrowHeight = m_xPropertySet->getPropertyValue( "TextAutoGrowHeight" );
    aTextAutoGrowHeight >>= bAutosize;
    return bAutosize;
}

void SAL_CALL
VbaTextFrame::setAutoSize( sal_Bool _autosize ) throw (uno::RuntimeException)
{
    setAsMSObehavior();
    m_xPropertySet->setPropertyValue( "TextAutoGrowHeight", uno::makeAny( _autosize ) );
}

float SAL_CALL
VbaTextFrame::getMarginBottom() throw (uno::RuntimeException)
{
    sal_Int32 nMargin = getMargin( "TextLowerDistance" );
    float fMargin = (float)Millimeter::getInPoints( nMargin );
    return fMargin;
}

void SAL_CALL
VbaTextFrame::setMarginBottom( float _marginbottom ) throw (uno::RuntimeException)
{
    setMargin( "TextLowerDistance", _marginbottom );
}

float SAL_CALL
VbaTextFrame::getMarginTop() throw (uno::RuntimeException)
{
    sal_Int32 nMargin = getMargin( "TextUpperDistance" );
    float fMargin = (float)Millimeter::getInPoints( nMargin );
    return fMargin;
}

void SAL_CALL
VbaTextFrame::setMarginTop( float _margintop ) throw (uno::RuntimeException)
{
    setMargin( "TextUpperDistance", _margintop );
}

float SAL_CALL
VbaTextFrame::getMarginLeft() throw (uno::RuntimeException)
{
    sal_Int32 nMargin = getMargin( "TextLeftDistance" );
    float fMargin = (float)Millimeter::getInPoints( nMargin );
    return fMargin;
}

void SAL_CALL
VbaTextFrame::setMarginLeft( float _marginleft ) throw (uno::RuntimeException)
{
    setMargin( "TextLeftDistance", _marginleft );
}

float SAL_CALL
VbaTextFrame::getMarginRight() throw (uno::RuntimeException)
{
    sal_Int32 nMargin = getMargin( "TextRightDistance" );
    float fMargin = (float)Millimeter::getInPoints( nMargin );
    return fMargin;
}

void SAL_CALL
VbaTextFrame::setMarginRight( float _marginright ) throw (uno::RuntimeException)
{
    setMargin( "TextRightDistance" , _marginright );
}


// Methods
uno::Any SAL_CALL
VbaTextFrame::Characters() throw (uno::RuntimeException)
{
    throw uno::RuntimeException( "Not implemented" , uno::Reference< uno::XInterface >() );
}

OUString
VbaTextFrame::getServiceImplName()
{
    return OUString("VbaTextFrame");
}

uno::Sequence< OUString >
VbaTextFrame::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msforms.TextFrame";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
