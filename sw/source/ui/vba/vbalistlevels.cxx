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
#include "vbalistlevels.hxx"
#include "vbalistlevel.hxx"
#include <ooo/vba/word/WdListGalleryType.hpp>
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace {

class ListLevelsEnumWrapper : public EnumerationHelper_BASE
{
    SwVbaListLevels* m_pListLevels;
    sal_Int32 m_nIndex;
public:
    explicit ListLevelsEnumWrapper( SwVbaListLevels* pLevels ) : m_pListLevels( pLevels ), m_nIndex( 1 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( m_nIndex <= m_pListLevels->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( m_nIndex <= m_pListLevels->getCount() )
            return m_pListLevels->Item( uno::Any( m_nIndex++ ), uno::Any() );
        throw container::NoSuchElementException();
    }
};

}

SwVbaListLevels::SwVbaListLevels( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, SwVbaListHelperRef  pHelper ) : SwVbaListLevels_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >() ), m_pListHelper(std::move( pHelper ))
{
}

::sal_Int32 SAL_CALL SwVbaListLevels::getCount()
{
    sal_Int32 nGalleryType = m_pListHelper->getGalleryType();
    if( nGalleryType == word::WdListGalleryType::wdBulletGallery
        || nGalleryType == word::WdListGalleryType::wdNumberGallery )
        return 1;
    else if( nGalleryType == word::WdListGalleryType::wdOutlineNumberGallery )
        return 9;
    return 0;
}

uno::Any SAL_CALL SwVbaListLevels::Item( const uno::Any& Index1, const uno::Any& /*not processed in this base class*/ )
{
    sal_Int32 nIndex = 0;
    if( !( Index1 >>= nIndex ) )
        throw uno::RuntimeException();
    if( nIndex <=0 || nIndex > getCount() )
        throw  uno::RuntimeException(u"Index out of bounds"_ustr );

    return uno::Any( uno::Reference< word::XListLevel >( new SwVbaListLevel( this, mxContext, m_pListHelper, nIndex - 1 ) ) );
}

// XEnumerationAccess
uno::Type
SwVbaListLevels::getElementType()
{
    return cppu::UnoType<word::XListLevel>::get();
}

uno::Reference< container::XEnumeration >
SwVbaListLevels::createEnumeration()
{
    return new ListLevelsEnumWrapper( this );
}

uno::Any
SwVbaListLevels::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

OUString
SwVbaListLevels::getServiceImplName()
{
    return u"SwVbaListLevels"_ustr;
}

css::uno::Sequence<OUString>
SwVbaListLevels::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.ListLevels"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
