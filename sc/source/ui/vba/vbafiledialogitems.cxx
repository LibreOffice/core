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

#include <o3tl/safeint.hxx>

#include "vbafiledialogitems.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

namespace {

class FileDialogItemEnumeration : public ::cppu::WeakImplHelper< container::XEnumeration >
{
    std::vector< OUString > m_sItems;
    std::vector< OUString >::iterator mIt;
public:
    explicit FileDialogItemEnumeration( std::vector< OUString >&& rVector ) : m_sItems( std::move(rVector) ), mIt( m_sItems.begin() ) {}
    virtual sal_Bool SAL_CALL hasMoreElements() override
    {
        return ( mIt != m_sItems.end() );
    }
    virtual uno::Any SAL_CALL nextElement() override
    {
        if( !hasMoreElements() )
            throw container::NoSuchElementException();
        OUString sPath = *mIt++;
        return uno::Any( sPath );
    }
};

}

ScVbaFileDialogSelectedItems::ScVbaFileDialogSelectedItems(
        const css::uno::Reference< ov::XHelperInterface >& xParent
       ,const css::uno::Reference< css::uno::XComponentContext >& xContext
       ,std::vector< OUString >&& rItems)
    : FileDialogSelectedItems_BASE( xParent, xContext, uno::Reference< container::XIndexAccess>() )
    , m_sItems(std::move(rItems)) {}


// XEnumerationAccess
uno::Type SAL_CALL
ScVbaFileDialogSelectedItems::getElementType()
{
    return cppu::UnoType<OUString>::get();
}

uno::Reference< container::XEnumeration >
ScVbaFileDialogSelectedItems::createEnumeration()
{
    return uno::Reference< container::XEnumeration >( new FileDialogItemEnumeration( std::vector(m_sItems) ) );
}

uno::Any
ScVbaFileDialogSelectedItems::createCollectionObject( const uno::Any& aSource )
{
    sal_Int32 nPosition = -1;
    if (!(aSource >>= nPosition))
        throw uno::RuntimeException(u"not an sal_Int32"_ustr);
    if (nPosition < 0 || o3tl::make_unsigned(nPosition) >= m_sItems.size())
        throw uno::RuntimeException(u"out of range"_ustr);

    OUString sPath = m_sItems[nPosition];
    return uno::Any( sPath );
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

    return createCollectionObject( uno::Any( nPosition ) );
}

sal_Int32 ScVbaFileDialogSelectedItems::getCount()
{
    return m_sItems.size();
}

// XHelperInterface
OUString
ScVbaFileDialogSelectedItems::getServiceImplName()
{
    return u"ScVbaFileDialogSelectedItems"_ustr;
}

uno::Sequence<OUString>
ScVbaFileDialogSelectedItems::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.FileDialogSelectedItems"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
