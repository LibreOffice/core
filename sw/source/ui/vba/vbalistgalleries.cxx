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
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace {

class ListGalleriesEnumWrapper : public EnumerationHelper_BASE
{
    SwVbaListGalleries* m_pListGalleries;
    sal_Int32 m_nIndex;
public:
    explicit ListGalleriesEnumWrapper( SwVbaListGalleries* pGalleries ) : m_pListGalleries( pGalleries ), m_nIndex( 1 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( m_nIndex <= m_pListGalleries->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( m_nIndex <= m_pListGalleries->getCount() )
            return m_pListGalleries->Item( uno::Any( m_nIndex++ ), uno::Any() );
        throw container::NoSuchElementException();
    }
};

}

SwVbaListGalleries::SwVbaListGalleries( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, uno::Reference< text::XTextDocument >  xTextDoc ) : SwVbaListGalleries_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >() ),  mxTextDocument(std::move( xTextDoc ))
{
}

::sal_Int32 SAL_CALL SwVbaListGalleries::getCount()
{
    // 3 types of list( bullet, numbered and outline )
    return 3;
}

uno::Any SAL_CALL SwVbaListGalleries::Item( const uno::Any& Index1, const uno::Any& /*not processed in this base class*/ )
{
    sal_Int32 nIndex = 0;
    if( Index1 >>= nIndex )
    {
        if( nIndex == word::WdListGalleryType::wdBulletGallery
            || nIndex == word::WdListGalleryType::wdNumberGallery
            || nIndex == word::WdListGalleryType::wdOutlineNumberGallery )
            return uno::Any( uno::Reference< word::XListGallery >( new SwVbaListGallery( this, mxContext, mxTextDocument, nIndex ) ) );
    }
    throw  uno::RuntimeException(u"Index out of bounds"_ustr );
}

// XEnumerationAccess
uno::Type
SwVbaListGalleries::getElementType()
{
    return cppu::UnoType<word::XListGallery>::get();
}

uno::Reference< container::XEnumeration >
SwVbaListGalleries::createEnumeration()
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
    return u"SwVbaListGalleries"_ustr;
}

css::uno::Sequence<OUString>
SwVbaListGalleries::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.ListGalleries"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
