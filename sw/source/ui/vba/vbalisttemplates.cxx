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
#include "vbalisttemplates.hxx"
#include "vbalisttemplate.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class ListTemplatesEnumWrapper : public EnumerationHelper_BASE
{
    SwVbaListTemplates* pListTemplates;
    sal_Int32 nIndex;
public:
    ListTemplatesEnumWrapper( SwVbaListTemplates* pTemplates ) : pListTemplates( pTemplates ), nIndex( 1 ) {}
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( nIndex <= pListTemplates->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( nIndex <= pListTemplates->getCount() )
            return pListTemplates->Item( uno::makeAny( nIndex++ ), uno::Any() );
        throw container::NoSuchElementException();
    }
};

SwVbaListTemplates::SwVbaListTemplates( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextDocument >& xTextDoc, sal_Int32 nType ) throw (uno::RuntimeException) : SwVbaListTemplates_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >() ),  mxTextDocument( xTextDoc ), mnGalleryType( nType )
{
}

::sal_Int32 SAL_CALL SwVbaListTemplates::getCount() throw (uno::RuntimeException)
{
    // 3 types of list( bullet, numbered and outline )
    return 7;
}

uno::Any SAL_CALL SwVbaListTemplates::Item( const uno::Any& Index1, const uno::Any& /*not processed in this base class*/ ) throw (uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    if( ( Index1 >>= nIndex ) == sal_False )
        throw uno::RuntimeException();
    if( nIndex <=0 || nIndex > getCount() )
        throw  uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Index out of bounds") ), uno::Reference< uno::XInterface >() );

    return uno::makeAny( uno::Reference< word::XListTemplate >( new SwVbaListTemplate( this, mxContext, mxTextDocument, mnGalleryType, nIndex ) ) );
}

// XEnumerationAccess
uno::Type
SwVbaListTemplates::getElementType() throw (uno::RuntimeException)
{
    return word::XListTemplate::static_type(0);
}

uno::Reference< container::XEnumeration >
SwVbaListTemplates::createEnumeration() throw (uno::RuntimeException)
{
    return new ListTemplatesEnumWrapper( this );
}

uno::Any
SwVbaListTemplates::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

rtl::OUString&
SwVbaListTemplates::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaListTemplates") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
SwVbaListTemplates::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.ListTemplates") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
