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
#include "vbaheadersfooters.hxx"
#include "vbaheaderfooter.hxx"
#include <ooo/vba/word/WdHeaderFooterIndex.hpp>
#include <cppuhelper/implbase.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

// I assume there is only one headersfooters in Writer
class HeadersFootersIndexAccess : public ::cppu::WeakImplHelper<container::XIndexAccess >
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxModel;
    uno::Reference< beans::XPropertySet > mxPageStyleProps;
    bool mbHeader;

public:
    HeadersFootersIndexAccess( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< beans::XPropertySet >& xPageStyleProps, bool bHeader ) : mxParent( xParent ), mxContext( xContext ), mxModel( xModel ), mxPageStyleProps( xPageStyleProps ), mbHeader( bHeader ) {}
    virtual ~HeadersFootersIndexAccess(){}

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException, std::exception) override
    {
        // first page, evenpages and primary page
        return 3;
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        if( Index < 1 || Index > 3 )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( uno::Reference< word::XHeaderFooter >( new SwVbaHeaderFooter( mxParent,  mxContext, mxModel, mxPageStyleProps, mbHeader, Index ) ) );
    }
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException, std::exception) override
    {
        return cppu::UnoType<word::XHeaderFooter>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException, std::exception) override
    {
        return sal_True;
    }
};

class HeadersFootersEnumWrapper : public EnumerationHelper_BASE
{
    SwVbaHeadersFooters* pHeadersFooters;
    sal_Int32 nIndex;
public:
    explicit HeadersFootersEnumWrapper( SwVbaHeadersFooters* _pHeadersFooters ) : pHeadersFooters( _pHeadersFooters ), nIndex( 0 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException, std::exception) override
    {
        return ( nIndex < pHeadersFooters->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        if ( nIndex < pHeadersFooters->getCount() )
            return pHeadersFooters->Item( uno::makeAny( ++nIndex ), uno::Any() );
        throw container::NoSuchElementException();
    }
};

SwVbaHeadersFooters::SwVbaHeadersFooters( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< beans::XPropertySet >& xPageStyleProps, bool isHeader ): SwVbaHeadersFooters_BASE( xParent, xContext, new HeadersFootersIndexAccess( xParent, xContext, xModel, xPageStyleProps, isHeader ) ),  mxModel( xModel ), mxPageStyleProps( xPageStyleProps ), mbHeader( isHeader )
{
}

::sal_Int32 SAL_CALL SwVbaHeadersFooters::getCount() throw (uno::RuntimeException)
{
    // wdHeaderFooterFirstPage, wdHeaderFooterPrimary and wdHeaderFooterEvenPages
    return 3;
}

uno::Any SAL_CALL SwVbaHeadersFooters::Item( const uno::Any& Index1, const uno::Any& ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    Index1 >>= nIndex;
    if( ( nIndex < 1 ) || ( nIndex > 3 ) )
    {
        throw lang::IndexOutOfBoundsException();
    }
    return uno::makeAny( uno::Reference< word::XHeaderFooter >( new SwVbaHeaderFooter( this,  mxContext, mxModel, mxPageStyleProps, mbHeader, nIndex ) ) );
}

// XEnumerationAccess
uno::Type
SwVbaHeadersFooters::getElementType() throw (uno::RuntimeException)
{
    return cppu::UnoType<word::XHeaderFooter>::get();
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

OUString
SwVbaHeadersFooters::getServiceImplName()
{
    return OUString("SwVbaHeadersFooters");
}

uno::Sequence<OUString>
SwVbaHeadersFooters::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.word.HeadersFooters";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
