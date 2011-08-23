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
    ListGalleriesEnumWrapper( SwVbaListGalleries* pGalleries ) : pListGalleries( pGalleries ), nIndex( 1 ) {}
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( nIndex <= pListGalleries->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
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
    if( ( Index1 >>= nIndex ) == sal_True )
    {
        if( nIndex == word::WdListGalleryType::wdBulletGallery
            || nIndex == word::WdListGalleryType::wdNumberGallery
            || nIndex == word::WdListGalleryType::wdOutlineNumberGallery )
            return uno::makeAny( uno::Reference< word::XListGallery >( new SwVbaListGallery( this, mxContext, mxTextDocument, nIndex ) ) );
    }
    throw  uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Index out of bounds") ), uno::Reference< uno::XInterface >() );
}

// XEnumerationAccess
uno::Type
SwVbaListGalleries::getElementType() throw (uno::RuntimeException)
{
    return word::XListGallery::static_type(0);
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

rtl::OUString&
SwVbaListGalleries::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaListGalleries") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
SwVbaListGalleries::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.ListGalleries") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
