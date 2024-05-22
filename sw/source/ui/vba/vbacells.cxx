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
#include "vbacells.hxx"
#include "vbacell.hxx"
#include "vbarow.hxx"
#include <cppuhelper/implbase.hxx>
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace {

class CellsEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 mnIndex;

public:
    explicit CellsEnumWrapper( uno::Reference< container::XIndexAccess > xIndexAccess ) : mxIndexAccess(std::move( xIndexAccess )), mnIndex( 0 )
    {
    }
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( mnIndex < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if( mnIndex < mxIndexAccess->getCount() )
        {
            return mxIndexAccess->getByIndex( mnIndex++ );
        }
        throw container::NoSuchElementException();
    }
};

class CellCollectionHelper : public ::cppu::WeakImplHelper< container::XIndexAccess,
                                                            container::XEnumerationAccess >
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< css::text::XTextTable > mxTextTable;
    sal_Int32 mnLeft;
    sal_Int32 mnTop;
    sal_Int32 mnRight;
    sal_Int32 mnBottom;

public:
    /// @throws css::uno::RuntimeException
    CellCollectionHelper( css::uno::Reference< ov::XHelperInterface > xParent, css::uno::Reference< css::uno::XComponentContext > xContext, css::uno::Reference< css::text::XTextTable >  xTextTable, sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ): mxParent(std::move( xParent )), mxContext(std::move( xContext )), mxTextTable(std::move( xTextTable )), mnLeft( nLeft ), mnTop( nTop ), mnRight( nRight ), mnBottom( nBottom )
    {
    }

    virtual sal_Int32 SAL_CALL getCount(  ) override
    {
        return ( mnRight - mnLeft + 1 ) * ( mnBottom - mnTop + 1 );
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override
    {
        if ( Index < 0 || Index >= getCount() )
            throw css::lang::IndexOutOfBoundsException();

        for( sal_Int32 row = mnTop; row <= mnBottom; row++ )
        {
            for( sal_Int32 col = mnLeft; col <= mnRight; col++ )
            {
                if( Index == ( ( row - mnTop ) * ( mnRight - mnLeft + 1 ) + ( col - mnLeft ) ) )
                    return uno::Any( uno::Reference< word::XCell >( new SwVbaCell( mxParent, mxContext, mxTextTable, col, row ) ) );
            }
        }
        throw css::lang::IndexOutOfBoundsException();

    }
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        return cppu::UnoType<word::XCell>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return true;
    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) override
    {
        return new CellsEnumWrapper( this );
    }
};

}

SwVbaCells::SwVbaCells( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextTable >& xTextTable, sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) : SwVbaCells_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new CellCollectionHelper( xParent, xContext, xTextTable, nLeft, nTop, nRight, nBottom ) ) ), mxTextTable( xTextTable ), mnTop( nTop ), mnBottom( nBottom )
{
}

::sal_Int32 SAL_CALL SwVbaCells::getWidth()
{
    uno::Reference< word::XCell > xCell( m_xIndexAccess->getByIndex( 0 ), uno::UNO_QUERY_THROW );
    return xCell->getWidth();
}

void SAL_CALL SwVbaCells::setWidth( ::sal_Int32 _width )
{
    sal_Int32 nIndex = 0;
    while( nIndex < m_xIndexAccess->getCount() )
    {
        uno::Reference< word::XCell > xCell( m_xIndexAccess->getByIndex( nIndex++ ), uno::UNO_QUERY_THROW );
        xCell->setWidth( _width );
    }
}

uno::Any SAL_CALL SwVbaCells::getHeight()
{
    uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, mnTop ) );
    return xRow->getHeight();
}

void SAL_CALL SwVbaCells::setHeight( const uno::Any& _height )
{
    for( sal_Int32 row = mnTop; row <= mnBottom; row++ )
    {
        uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, row ) );
        xRow->setHeight( _height );
    }
}

::sal_Int32 SAL_CALL SwVbaCells::getHeightRule()
{
    uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, mnTop ) );
    return xRow->getHeightRule();
}

void SAL_CALL SwVbaCells::setHeightRule( ::sal_Int32 _heightrule )
{
    for( sal_Int32 row = mnTop; row <= mnBottom; row++ )
    {
        uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, row ) );
        xRow->setHeightRule( _heightrule );
    }
}

void SAL_CALL SwVbaCells::SetWidth( float width, sal_Int32 rulestyle )
{
    sal_Int32 nIndex = 0;
    while( nIndex < m_xIndexAccess->getCount() )
    {
        uno::Reference< word::XCell > xCell( m_xIndexAccess->getByIndex( nIndex++ ), uno::UNO_QUERY_THROW );
        xCell->SetWidth( width, rulestyle );
    }
}

void SAL_CALL SwVbaCells::SetHeight( float height, sal_Int32 heightrule )
{
    for( sal_Int32 row = mnTop; row <= mnBottom; row++ )
    {
        uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, row ) );
        xRow->SetHeight( height, heightrule );
    }
}

// XEnumerationAccess
uno::Type
SwVbaCells::getElementType()
{
    return cppu::UnoType<word::XCell>::get();
}

uno::Reference< container::XEnumeration >
SwVbaCells::createEnumeration()
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return xEnumAccess->createEnumeration();
}

uno::Any
SwVbaCells::createCollectionObject( const uno::Any& aSource )
{
    return aSource;
}

OUString
SwVbaCells::getServiceImplName()
{
    return u"SwVbaCells"_ustr;
}

uno::Sequence<OUString>
SwVbaCells::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.Cells"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
