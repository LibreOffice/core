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

using namespace ::com::sun::star;
using namespace ::ooo::vba;

class FileDialogItemEnumeration : public ::cppu::WeakImplHelper< container::XEnumeration >
{
    std::vector< OUString > m_sItems;
    std::vector< OUString >::iterator mIt;
public:
    explicit FileDialogItemEnumeration( const std::vector< OUString >& rVector ) : m_sItems( rVector ), mIt( m_sItems.begin() ) {}
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
       ,const std::vector< OUString >& rItems)
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
    if (!(aSource >>= nPosition))
        throw uno::RuntimeException("not an sal_Int32");
    if (nPosition < 0 || nPosition >= static_cast<sal_Int32>(m_sItems.size()))
        throw uno::RuntimeException("out of range");

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
    return "ScVbaFileDialogSelectedItems";
}

uno::Sequence<OUString>
ScVbaFileDialogSelectedItems::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.FileDialogSelectedItems"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
