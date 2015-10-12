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
#include "vbalisttemplates.hxx"
#include "vbalisttemplate.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class ListTemplatesEnumWrapper : public EnumerationHelper_BASE
{
    SwVbaListTemplates* pListTemplates;
    sal_Int32 nIndex;
public:
    explicit ListTemplatesEnumWrapper( SwVbaListTemplates* pTemplates ) : pListTemplates( pTemplates ), nIndex( 1 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException, std::exception) override
    {
        return ( nIndex <= pListTemplates->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
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
    if( !( Index1 >>= nIndex ) )
        throw uno::RuntimeException();
    if( nIndex <=0 || nIndex > getCount() )
        throw  uno::RuntimeException("Index out of bounds" );

    return uno::makeAny( uno::Reference< word::XListTemplate >( new SwVbaListTemplate( this, mxContext, mxTextDocument, mnGalleryType, nIndex ) ) );
}

// XEnumerationAccess
uno::Type
SwVbaListTemplates::getElementType() throw (uno::RuntimeException)
{
    return cppu::UnoType<word::XListTemplate>::get();
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

OUString
SwVbaListTemplates::getServiceImplName()
{
    return OUString("SwVbaListTemplates");
}

css::uno::Sequence<OUString>
SwVbaListTemplates::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.word.ListTemplates";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
