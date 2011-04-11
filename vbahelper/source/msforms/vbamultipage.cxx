/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "vbamultipage.hxx"
#include <ooo/vba/XCollection.hpp>
#include "vbapages.hxx"
#include <vector>
#include <com/sun/star/container/XNameContainer.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

const rtl::OUString SVALUE( RTL_CONSTASCII_USTRINGPARAM("MultiPageValue") );

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

ScVbaMultiPage::ScVbaMultiPage( const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper, const uno::Reference< uno::XInterface >& xDialog ) : MultiPageImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
    mxDialogProps.set( xDialog, uno::UNO_QUERY_THROW );
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


rtl::OUString&
ScVbaMultiPage::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaMultiPage") );
    return sImplName;
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

uno::Sequence< rtl::OUString >
ScVbaMultiPage::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msforms.MultiPage" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
