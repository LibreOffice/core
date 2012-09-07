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
#include <vector>
#include <com/sun/star/container/XNameContainer.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

const OUString SVALUE( "MultiPageValue" );

typedef cppu::WeakImplHelper1< container::XIndexAccess > PagesImpl_Base;
class PagesImpl : public PagesImpl_Base
{
    sal_Int32 mnPages;
public:
    PagesImpl( sal_Int32 nPages ) : mnPages( nPages ) {}
    virtual ::sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException) { return mnPages; }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, ::uno::RuntimeException)
    {
        if ( Index < 0 || Index > mnPages )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( uno::Reference< uno::XInterface >() );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException)
    {
        // no Pages object yet #FIXME
        //return msforms::XPage::static_type(0);
        return uno::XInterface::static_type(0);
    }
    virtual ::sal_Bool SAL_CALL hasElements( ) throw (uno::RuntimeException)
    {
        return ( mnPages > 0 );
    }
};
uno::Reference< container::XIndexAccess >
ScVbaMultiPage::getPages( sal_Int32 nPages )
{
    return new PagesImpl( nPages );
}

ScVbaMultiPage::ScVbaMultiPage(
        const uno::Reference< ov::XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< uno::XInterface >& xControl,
        const uno::Reference< frame::XModel >& xModel,
        AbstractGeometryAttributes* pGeomHelper) :
    MultiPageImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
}

// Attributes
sal_Int32 SAL_CALL
ScVbaMultiPage::getValue() throw (css::uno::RuntimeException)
{
    sal_Int32 nValue = 0;
    m_xProps->getPropertyValue( SVALUE ) >>= nValue;
    // VBA 0 based tab index
    return nValue - 1;
}

void SAL_CALL
ScVbaMultiPage::setValue( const sal_Int32 _value ) throw (::com::sun::star::uno::RuntimeException)
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
    return OUString( "ScVbaMultiPage" );
}

uno::Any SAL_CALL
ScVbaMultiPage::Pages( const uno::Any& index ) throw (uno::RuntimeException)
{
    // get the container model
    uno::Reference< container::XNameContainer > xContainer( m_xProps, uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xColl( new ScVbaPages( this, mxContext, getPages( xContainer->getElementNames().getLength() ) ) );
    if ( !index.hasValue() )
        return uno::makeAny( xColl );
    return xColl->Item( uno::makeAny( index ), uno::Any() );
}

uno::Sequence< OUString >
ScVbaMultiPage::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msforms.MultiPage";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
