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
#include "wordvbahelper.hxx"
#include "vbatablehelper.hxx"
#include "vbarow.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

typedef ::cppu::WeakImplHelper2< container::XIndexAccess, container::XEnumerationAccess > CellCollectionHelper_Base;

class CellsEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 nIndex;

public:
    CellsEnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) : mxIndexAccess( xIndexAccess ), nIndex( 0 )
    {
    }
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( nIndex < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if( nIndex < mxIndexAccess->getCount() )
        {
            return mxIndexAccess->getByIndex( nIndex++ );
        }
        throw container::NoSuchElementException();
    }
};

class CellCollectionHelper : public CellCollectionHelper_Base
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
    CellCollectionHelper( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::text::XTextTable >& xTextTable, sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) throw ( css::uno::RuntimeException ): mxParent( xParent ), mxContext( xContext ), mxTextTable( xTextTable ), mnLeft( nLeft ), mnTop( nTop ), mnRight( nRight ), mnBottom( nBottom )
    {
    }
    virtual ~CellCollectionHelper() {}

    virtual sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return ( mnRight - mnLeft + 1 ) * ( mnBottom - mnTop + 1 );
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( Index < 0 || Index >= getCount() )
            throw css::lang::IndexOutOfBoundsException();

        for( sal_Int32 row = mnTop; row <= mnBottom; row++ )
        {
            for( sal_Int32 col = mnLeft; col <= mnRight; col++ )
            {
                if( Index == ( ( row - mnTop ) * ( mnRight - mnLeft + 1 ) + ( col - mnLeft ) ) )
                    return uno::makeAny( uno::Reference< word::XCell >( new SwVbaCell( mxParent, mxContext, mxTextTable, col, row ) ) );
            }
        }
        throw css::lang::IndexOutOfBoundsException();

    }
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return word::XCell::static_type(0);
    }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return sal_True;
    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        return new CellsEnumWrapper( this );
    }
};

SwVbaCells::SwVbaCells( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextTable >& xTextTable, sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) throw (uno::RuntimeException) : SwVbaCells_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new CellCollectionHelper( xParent, xContext, xTextTable, nLeft, nTop, nRight, nBottom ) ) ), mxTextTable( xTextTable ), mnTop( nTop ), mnBottom( nBottom )
{
}

::sal_Int32 SAL_CALL SwVbaCells::getWidth() throw (css::uno::RuntimeException)
{
    uno::Reference< word::XCell > xCell( m_xIndexAccess->getByIndex( 0 ), uno::UNO_QUERY_THROW );
    return xCell->getWidth();
}

void SAL_CALL SwVbaCells::setWidth( ::sal_Int32 _width ) throw (css::uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    while( nIndex < m_xIndexAccess->getCount() )
    {
        uno::Reference< word::XCell > xCell( m_xIndexAccess->getByIndex( nIndex++ ), uno::UNO_QUERY_THROW );
        xCell->setWidth( _width );
    }
}

uno::Any SAL_CALL SwVbaCells::getHeight() throw (css::uno::RuntimeException)
{
    uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, mnTop ) );
    return xRow->getHeight();
}

void SAL_CALL SwVbaCells::setHeight( const uno::Any& _height ) throw (css::uno::RuntimeException)
{
    for( sal_Int32 row = mnTop; row <= mnBottom; row++ )
    {
        uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, row ) );
        xRow->setHeight( _height );
    }
}

::sal_Int32 SAL_CALL SwVbaCells::getHeightRule() throw (css::uno::RuntimeException)
{
    uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, mnTop ) );
    return xRow->getHeightRule();
}

void SAL_CALL SwVbaCells::setHeightRule( ::sal_Int32 _heightrule ) throw (css::uno::RuntimeException)
{
    for( sal_Int32 row = mnTop; row <= mnBottom; row++ )
    {
        uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, row ) );
        xRow->setHeightRule( _heightrule );
    }
}

void SAL_CALL SwVbaCells::SetWidth( float width, sal_Int32 rulestyle ) throw (css::uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    while( nIndex < m_xIndexAccess->getCount() )
    {
        uno::Reference< word::XCell > xCell( m_xIndexAccess->getByIndex( nIndex++ ), uno::UNO_QUERY_THROW );
        xCell->SetWidth( width, rulestyle );
    }
}

void SAL_CALL SwVbaCells::SetHeight( float height, sal_Int32 heightrule ) throw (css::uno::RuntimeException)
{
    for( sal_Int32 row = mnTop; row <= mnBottom; row++ )
    {
        uno::Reference< word::XRow > xRow( new SwVbaRow( getParent(), mxContext, mxTextTable, row ) );
        xRow->SetHeight( height, heightrule );
    }
}

// XEnumerationAccess
uno::Type
SwVbaCells::getElementType() throw (uno::RuntimeException)
{
    return word::XCell::static_type(0);
}

uno::Reference< container::XEnumeration >
SwVbaCells::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return xEnumAccess->createEnumeration();
}

uno::Any
SwVbaCells::createCollectionObject( const uno::Any& aSource )
{
    return aSource;
}

rtl::OUString
SwVbaCells::getServiceImplName()
{
    return rtl::OUString("SwVbaCells");
}

uno::Sequence<rtl::OUString>
SwVbaCells::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString("ooo.vba.word.Cells");
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
