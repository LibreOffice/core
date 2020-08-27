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
#include "vbamultipage.hxx"
#include <ooo/vba/XCollection.hpp>
#include "vbapages.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <cppuhelper/implbase.hxx>

using namespace com::sun::star;
using namespace ooo::vba;

const OUStringLiteral SVALUE( u"MultiPageValue" );

namespace {

class PagesImpl : public cppu::WeakImplHelper< container::XIndexAccess >
{
    sal_Int32 mnPages;
public:
    explicit PagesImpl( sal_Int32 nPages ) : mnPages( nPages ) {}
    virtual ::sal_Int32 SAL_CALL getCount() override { return mnPages; }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        if ( Index < 0 || Index > mnPages )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( uno::Reference< uno::XInterface >() );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override
    {
        // no Pages object yet #FIXME
        //return cppu::UnoType<msforms::XPage>::get();
        return cppu::UnoType<uno::XInterface>::get();
    }
    virtual sal_Bool SAL_CALL hasElements( ) override
    {
        return ( mnPages > 0 );
    }
};

}

ScVbaMultiPage::ScVbaMultiPage(
        const uno::Reference< ov::XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< uno::XInterface >& xControl,
        const uno::Reference< frame::XModel >& xModel,
        std::unique_ptr<ov::AbstractGeometryAttributes> pGeomHelper) :
    MultiPageImpl_BASE( xParent, xContext, xControl, xModel, std::move(pGeomHelper) )
{
}

// Attributes
sal_Int32 SAL_CALL
ScVbaMultiPage::getValue()
{
    sal_Int32 nValue = 0;
    m_xProps->getPropertyValue( SVALUE ) >>= nValue;
    // VBA 0 based tab index
    return nValue - 1;
}

void SAL_CALL
ScVbaMultiPage::setValue( const sal_Int32 _value )
{
    // Openoffice 1 based tab index
    sal_Int32 nVal = _value + 1;
    sal_Int32 nOldVal = getValue();
    m_xProps->setPropertyValue( SVALUE, uno::makeAny( nVal ) );
    if ( nVal != nOldVal )
        fireChangeEvent();
}

OUString
ScVbaMultiPage::getServiceImplName()
{
    return "ScVbaMultiPage";
}

uno::Any SAL_CALL
ScVbaMultiPage::Pages( const uno::Any& index )
{
    // get the container model
    uno::Reference< container::XNameContainer > xContainer( m_xProps, uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xColl( new ScVbaPages( this, mxContext, new PagesImpl( xContainer->getElementNames().getLength() ) ) );
    if ( !index.hasValue() )
        return uno::makeAny( xColl );
    return xColl->Item( index, uno::Any() );
}

uno::Sequence< OUString >
ScVbaMultiPage::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.msforms.MultiPage"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
