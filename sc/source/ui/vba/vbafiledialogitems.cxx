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
#include "vbafiledialogitems.hxx"
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

typedef std::vector< OUString > StringVector;

class FileDialogItemEnumeration : public ::cppu::WeakImplHelper< container::XEnumeration >
{
    StringVector m_sItems;
    StringVector::iterator mIt;
public:
    explicit FileDialogItemEnumeration( const StringVector& rVector ) : m_sItems( rVector ), mIt( m_sItems.begin() ) {}
    virtual sal_Bool SAL_CALL hasMoreElements() override
    {
        return ( mIt != m_sItems.end() );
    }
    virtual uno::Any SAL_CALL nextElement() override
    {
        if( !hasMoreElements() )
            throw container::NoSuchElementException();
        OUString sPath = *mIt++;
        return uno::makeAny( sPath );
    }
};

ScVbaFileDialogSelectedItems::ScVbaFileDialogSelectedItems(
        const css::uno::Reference< ov::XHelperInterface >& xParent
       ,const css::uno::Reference< css::uno::XComponentContext >& xContext
       ,const StringVector& rItems)
    : FileDialogSelectedItems_BASE( xParent, xContext, uno::Reference< container::XIndexAccess>() )
    , m_sItems(rItems) {}


// XEnumerationAccess
uno::Type SAL_CALL
ScVbaFileDialogSelectedItems::getElementType()
{
    return cppu::UnoType<OUString>::get();
}

uno::Reference< container::XEnumeration >
ScVbaFileDialogSelectedItems::createEnumeration()
{
    return uno::Reference< container::XEnumeration >( new FileDialogItemEnumeration( m_sItems ) );
}

uno::Any
ScVbaFileDialogSelectedItems::createCollectionObject( const uno::Any& aSource )
{
    sal_Int32 nPosition = -1;
    aSource >>= nPosition;

    OUString sPath = m_sItems[nPosition];
    return uno::makeAny( sPath );
}

// Methods
uno::Any SAL_CALL
ScVbaFileDialogSelectedItems::Item( const uno::Any& aIndex, const uno::Any& /*aIndex*/ )
{
    sal_Int32 nPosition = -1;
    aIndex >>= nPosition;

    --nPosition; // vba indexing starts with 1

    if( nPosition < 0 || nPosition >= getCount() )
    {
        throw uno::RuntimeException();
    }

    return createCollectionObject( uno::makeAny( nPosition ) );
}

sal_Int32 ScVbaFileDialogSelectedItems::getCount()
{
    return m_sItems.size();
}

// XHelperInterface
OUString
ScVbaFileDialogSelectedItems::getServiceImplName()
{
    return OUString("ScVbaFileDialogSelectedItems");
}

uno::Sequence<OUString>
ScVbaFileDialogSelectedItems::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.FileDialogSelectedItems";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
