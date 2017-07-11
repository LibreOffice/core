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

class FileDialogItemEnumeration : public ::cppu::WeakImplHelper< container::XEnumeration >
{
    FileDialogSelectedItems_BASE* m_pItems;
    sal_Int32 m_nCurrentPosition;
public:
    explicit FileDialogItemEnumeration( FileDialogSelectedItems_BASE* pItems ) : m_pItems( pItems ), m_nCurrentPosition( 0 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements() override
    {
        if( m_nCurrentPosition < m_pItems->getCount() )
            return true;
        return false;
    }
    virtual uno::Any SAL_CALL nextElement() override
    {
        if( hasMoreElements() )
        {
            return m_pItems->createCollectionObject( uno::makeAny( m_nCurrentPosition++ ) );
        }
        else
            throw container::NoSuchElementException();
    }
};

ScVbaFileDialogSelectedItems::ScVbaFileDialogSelectedItems(
        const css::uno::Reference< ov::XHelperInterface >& xParent
       ,const css::uno::Reference< css::uno::XComponentContext >& xContext
       ,const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess):
    FileDialogSelectedItems_BASE( xParent, xContext, xIndexAccess) {}


// XEnumerationAccess
uno::Type SAL_CALL
ScVbaFileDialogSelectedItems::getElementType()
{
    return cppu::UnoType<OUString>::get();
}

uno::Reference< container::XEnumeration >
ScVbaFileDialogSelectedItems::createEnumeration()
{
    return uno::Reference< container::XEnumeration >( new FileDialogItemEnumeration( this ) );
}

uno::Any
ScVbaFileDialogSelectedItems::createCollectionObject( const uno::Any& aSource )
{
    sal_Int32 nPosition = -1;
    aSource >>= nPosition;

    OUString sPath;
    m_xIndexAccess->getByIndex( nPosition ) >>= sPath;

    return uno::makeAny( sPath );
}

// Methods
uno::Any SAL_CALL
ScVbaFileDialogSelectedItems::Item( const uno::Any& aIndex, const uno::Any& /*aIndex*/ )
{
    sal_Int32 nPosition = -1;
    aIndex >>= nPosition;

    if( nPosition < 0 || nPosition >= getCount() )
    {
        throw uno::RuntimeException();
    }

    return createCollectionObject( uno::makeAny( nPosition ) );
}

OUString SAL_CALL
ScVbaFileDialogSelectedItems::Add( const OUString& sPath )
{
    uno::Reference< container::XIndexContainer > xIndexContainer( m_xIndexAccess, uno::UNO_QUERY_THROW );
    xIndexContainer->insertByIndex( m_xIndexAccess->getCount(), uno::makeAny( sPath ) );
    return sPath;
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
