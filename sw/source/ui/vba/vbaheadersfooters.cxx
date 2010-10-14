/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#include "vbaheadersfooters.hxx"
#include "vbaheaderfooter.hxx"
#include <ooo/vba/word/WdHeaderFooterIndex.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

// I assume there is only one headersfooters in Writer
typedef ::cppu::WeakImplHelper1<container::XIndexAccess > HeadersFootersIndexAccess_Base;
class HeadersFootersIndexAccess : public HeadersFootersIndexAccess_Base
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxModel;
    uno::Reference< beans::XPropertySet > mxPageStyleProps;
    sal_Bool mbHeader;

public:
    HeadersFootersIndexAccess( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< beans::XPropertySet >& xPageStyleProps, sal_Bool bHeader ) : mxParent( xParent ), mxContext( xContext ), mxModel( xModel ), mxPageStyleProps( xPageStyleProps ), mbHeader( bHeader ) {}
    ~HeadersFootersIndexAccess(){}

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        // first page, evenpages and primary page
        return 3;
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if( Index < 1 || Index > 3 )
            throw container::NoSuchElementException();
        return uno::makeAny( uno::Reference< word::XHeaderFooter >( new SwVbaHeaderFooter( mxParent,  mxContext, mxModel, mxPageStyleProps, mbHeader, Index ) ) );
    }
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return word::XHeaderFooter::static_type(0);
    }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return sal_True;
    }
};

class HeadersFootersEnumWrapper : public EnumerationHelper_BASE
{
    SwVbaHeadersFooters* pHeadersFooters;
    sal_Int32 nIndex;
public:
    HeadersFootersEnumWrapper( SwVbaHeadersFooters* _pHeadersFooters ) : pHeadersFooters( _pHeadersFooters ), nIndex( 0 ) {}
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( nIndex < pHeadersFooters->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( nIndex < pHeadersFooters->getCount() )
            return pHeadersFooters->Item( uno::makeAny( ++nIndex ), uno::Any() );
        throw container::NoSuchElementException();
    }
};

SwVbaHeadersFooters::SwVbaHeadersFooters( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< beans::XPropertySet >& xPageStyleProps, sal_Bool isHeader ): SwVbaHeadersFooters_BASE( xParent, xContext, new HeadersFootersIndexAccess( xParent, xContext, xModel, xPageStyleProps, isHeader ) ),  mxModel( xModel ), mxPageStyleProps( xPageStyleProps ), mbHeader( isHeader )
{
}

::sal_Int32 SAL_CALL SwVbaHeadersFooters::getCount() throw (uno::RuntimeException)
{
    // wdHeaderFooterFirstPage, wdHeaderFooterPrimary and wdHeaderFooterEvenPages
    return 3;
}

uno::Any SAL_CALL SwVbaHeadersFooters::Item( const uno::Any& Index1, const uno::Any& ) throw (uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    Index1 >>= nIndex;
    if( ( nIndex < 1 ) || ( nIndex > 3 ) )
    {
        throw container::NoSuchElementException();
    }
    return uno::makeAny( uno::Reference< word::XHeaderFooter >( new SwVbaHeaderFooter( this,  mxContext, mxModel, mxPageStyleProps, mbHeader, nIndex ) ) );
}

// XEnumerationAccess
uno::Type
SwVbaHeadersFooters::getElementType() throw (uno::RuntimeException)
{
    return word::XHeaderFooter::static_type(0);
}
uno::Reference< container::XEnumeration >

SwVbaHeadersFooters::createEnumeration() throw (uno::RuntimeException)
{
    return new HeadersFootersEnumWrapper( this );
}

uno::Any
SwVbaHeadersFooters::createCollectionObject( const uno::Any& aSource )
{
    return aSource;
}

rtl::OUString&
SwVbaHeadersFooters::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaHeadersFooters") );
    return sImplName;
}

uno::Sequence<rtl::OUString>
SwVbaHeadersFooters::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.HeadersFooters") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
