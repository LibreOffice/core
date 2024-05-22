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
#include <utility>

#include "vbalisttemplates.hxx"
#include "vbalisttemplate.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace {

class ListTemplatesEnumWrapper : public EnumerationHelper_BASE
{
    SwVbaListTemplates* m_pListTemplates;
    sal_Int32 m_nIndex;
public:
    explicit ListTemplatesEnumWrapper( SwVbaListTemplates* pTemplates ) : m_pListTemplates( pTemplates ), m_nIndex( 1 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( m_nIndex <= m_pListTemplates->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( m_nIndex <= m_pListTemplates->getCount() )
            return m_pListTemplates->Item( uno::Any( m_nIndex++ ), uno::Any() );
        throw container::NoSuchElementException();
    }
};

}

SwVbaListTemplates::SwVbaListTemplates( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, uno::Reference< text::XTextDocument >  xTextDoc, sal_Int32 nType ) : SwVbaListTemplates_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >() ),  mxTextDocument(std::move( xTextDoc )), mnGalleryType( nType )
{
}

::sal_Int32 SAL_CALL SwVbaListTemplates::getCount()
{
    // 3 types of list( bullet, numbered and outline )
    return 7;
}

uno::Any SAL_CALL SwVbaListTemplates::Item( const uno::Any& Index1, const uno::Any& /*not processed in this base class*/ )
{
    sal_Int32 nIndex = 0;
    if( !( Index1 >>= nIndex ) )
        throw uno::RuntimeException();
    if( nIndex <=0 || nIndex > getCount() )
        throw  uno::RuntimeException(u"Index out of bounds"_ustr );

    return uno::Any( uno::Reference< word::XListTemplate >( new SwVbaListTemplate( this, mxContext, mxTextDocument, mnGalleryType, nIndex ) ) );
}

// XEnumerationAccess
uno::Type
SwVbaListTemplates::getElementType()
{
    return cppu::UnoType<word::XListTemplate>::get();
}

uno::Reference< container::XEnumeration >
SwVbaListTemplates::createEnumeration()
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
    return u"SwVbaListTemplates"_ustr;
}

css::uno::Sequence<OUString>
SwVbaListTemplates::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.ListTemplates"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
