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
#include "vbapanes.hxx"
#include "vbapane.hxx"
#include <cppuhelper/implbase.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

// I assume there is only one pane in Writer
class PanesIndexAccess : public ::cppu::WeakImplHelper<container::XIndexAccess >
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxModel;

public:
    PanesIndexAccess( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel ) : mxParent( xParent ), mxContext( xContext ), mxModel( xModel ) {}

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override
    {
        return 1;
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override
    {
        if( Index != 0 )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( uno::Reference< word::XPane >( new SwVbaPane( mxParent,  mxContext, mxModel ) ) );
    }
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        return cppu::UnoType<word::XPane>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return true;
    }
};

class PanesEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<container::XIndexAccess > m_xIndexAccess;
    sal_Int32 nIndex;
public:
    explicit PanesEnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) : m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( nIndex < m_xIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( nIndex < m_xIndexAccess->getCount() )
            return m_xIndexAccess->getByIndex( nIndex++ );
        throw container::NoSuchElementException();
    }
};

SwVbaPanes::SwVbaPanes( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel ): SwVbaPanes_BASE( xParent, xContext, new PanesIndexAccess( xParent, xContext, xModel ) )
{
}
// XEnumerationAccess
uno::Type
SwVbaPanes::getElementType()
{
    return cppu::UnoType<word::XPane>::get();
}
uno::Reference< container::XEnumeration >
SwVbaPanes::createEnumeration()
{
    return new PanesEnumWrapper( m_xIndexAccess );
}

uno::Any
SwVbaPanes::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

OUString
SwVbaPanes::getServiceImplName()
{
    return "SwVbaPanes";
}

css::uno::Sequence<OUString>
SwVbaPanes::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        "ooo.vba.word.Panes"
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
