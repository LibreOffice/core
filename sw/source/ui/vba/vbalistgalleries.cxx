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
#include "vbalistgalleries.hxx"
#include "vbalistgallery.hxx"
#include <ooo/vba/word/WdListGalleryType.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class ListGalleriesEnumWrapper : public EnumerationHelper_BASE
{
    SwVbaListGalleries* pListGalleries;
    sal_Int32 nIndex;
public:
    explicit ListGalleriesEnumWrapper( SwVbaListGalleries* pGalleries ) : pListGalleries( pGalleries ), nIndex( 1 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException, std::exception) override
    {
        return ( nIndex <= pListGalleries->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        if ( nIndex <= pListGalleries->getCount() )
            return pListGalleries->Item( uno::makeAny( nIndex++ ), uno::Any() );
        throw container::NoSuchElementException();
    }
};

SwVbaListGalleries::SwVbaListGalleries( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextDocument >& xTextDoc ) throw (uno::RuntimeException) : SwVbaListGalleries_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >() ),  mxTextDocument( xTextDoc )
{
}

::sal_Int32 SAL_CALL SwVbaListGalleries::getCount() throw (uno::RuntimeException)
{
    // 3 types of list( bullet, numbered and outline )
    return 3;
}

uno::Any SAL_CALL SwVbaListGalleries::Item( const uno::Any& Index1, const uno::Any& /*not processed in this base class*/ ) throw (uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    if( ( Index1 >>= nIndex ) )
    {
        if( nIndex == word::WdListGalleryType::wdBulletGallery
            || nIndex == word::WdListGalleryType::wdNumberGallery
            || nIndex == word::WdListGalleryType::wdOutlineNumberGallery )
            return uno::makeAny( uno::Reference< word::XListGallery >( new SwVbaListGallery( this, mxContext, mxTextDocument, nIndex ) ) );
    }
    throw  uno::RuntimeException("Index out of bounds" );
}

// XEnumerationAccess
uno::Type
SwVbaListGalleries::getElementType() throw (uno::RuntimeException)
{
    return cppu::UnoType<word::XListGallery>::get();
}

uno::Reference< container::XEnumeration >
SwVbaListGalleries::createEnumeration() throw (uno::RuntimeException)
{
    return new ListGalleriesEnumWrapper( this );
}

uno::Any
SwVbaListGalleries::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

OUString
SwVbaListGalleries::getServiceImplName()
{
    return OUString("SwVbaListGalleries");
}

css::uno::Sequence<OUString>
SwVbaListGalleries::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.word.ListGalleries";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
