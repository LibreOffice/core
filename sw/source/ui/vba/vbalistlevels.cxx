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

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class ListLevelsEnumWrapper : public EnumerationHelper_BASE
{
    SwVbaListLevels* pListLevels;
    sal_Int32 nIndex;
public:
    explicit ListLevelsEnumWrapper( SwVbaListLevels* pLevels ) : pListLevels( pLevels ), nIndex( 1 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException, std::exception) override
    {
        return ( nIndex <= pListLevels->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        if ( nIndex <= pListLevels->getCount() )
            return pListLevels->Item( uno::makeAny( nIndex++ ), uno::Any() );
        throw container::NoSuchElementException();
    }
};

SwVbaListLevels::SwVbaListLevels( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, SwVbaListHelperRef pHelper ) throw (uno::RuntimeException) : SwVbaListLevels_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >() ), pListHelper( pHelper )
{
}

::sal_Int32 SAL_CALL SwVbaListLevels::getCount() throw (uno::RuntimeException)
{
    sal_Int32 nGalleryType = pListHelper->getGalleryType();
    if( nGalleryType == word::WdListGalleryType::wdBulletGallery
        || nGalleryType == word::WdListGalleryType::wdNumberGallery )
        return 1;
    else if( nGalleryType == word::WdListGalleryType::wdOutlineNumberGallery )
        return 9;
    return 0;
}

uno::Any SAL_CALL SwVbaListLevels::Item( const uno::Any& Index1, const uno::Any& /*not processed in this base class*/ ) throw (uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    if( !( Index1 >>= nIndex ) )
        throw uno::RuntimeException();
    if( nIndex <=0 || nIndex > getCount() )
        throw  uno::RuntimeException("Index out of bounds" );

    return uno::makeAny( uno::Reference< word::XListLevel >( new SwVbaListLevel( this, mxContext, pListHelper, nIndex - 1 ) ) );
}

// XEnumerationAccess
uno::Type
SwVbaListLevels::getElementType() throw (uno::RuntimeException)
{
    return cppu::UnoType<word::XListLevel>::get();
}

uno::Reference< container::XEnumeration >
SwVbaListLevels::createEnumeration() throw (uno::RuntimeException)
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
    return OUString("SwVbaListLevels");
}

css::uno::Sequence<OUString>
SwVbaListLevels::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.word.ListLevels";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
