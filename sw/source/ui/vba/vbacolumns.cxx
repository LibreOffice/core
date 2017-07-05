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
#include "vbacolumns.hxx"
#include "vbacolumn.hxx"
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <ooo/vba/word/WdConstants.hpp>
#include <ooo/vba/word/WdRulerStyle.hpp>
#include "wordvbahelper.hxx"
#include "vbatablehelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class ColumnsEnumWrapper : public EnumerationHelper_BASE
{
    uno::WeakReference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< text::XTextTable > mxTextTable;
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 nIndex;

public:
    ColumnsEnumWrapper( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< text::XTextTable >& xTextTable ) : mxParent( xParent ), mxContext( xContext ), mxTextTable( xTextTable ), nIndex( 0 )
    {
        mxIndexAccess.set( mxTextTable->getColumns(), uno::UNO_QUERY );
    }
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( nIndex < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if( nIndex < mxIndexAccess->getCount() )
        {
            return uno::makeAny( uno::Reference< word::XColumn > ( new SwVbaColumn( mxParent, mxContext, mxTextTable, nIndex++ ) ) );
        }
        throw container::NoSuchElementException();
    }
};

SwVbaColumns::SwVbaColumns( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextTable >& xTextTable, const uno::Reference< table::XTableColumns >& xTableColumns ) : SwVbaColumns_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( xTableColumns, uno::UNO_QUERY_THROW ) ), mxTextTable( xTextTable )
{
    mnStartColumnIndex = 0;
    SwVbaTableHelper aTableHelper( mxTextTable );
    mnEndColumnIndex = aTableHelper.getTabColumnsMaxCount( ) - 1;
}

SwVbaColumns::SwVbaColumns( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextTable >& xTextTable, const uno::Reference< table::XTableColumns >& xTableColumns, sal_Int32 nStartCol, sal_Int32 nEndCol ) : SwVbaColumns_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( xTableColumns, uno::UNO_QUERY_THROW ) ), mxTextTable( xTextTable ), mnStartColumnIndex( nStartCol ), mnEndColumnIndex( nEndCol )
{
    if( mnEndColumnIndex < mnStartColumnIndex )
        throw uno::RuntimeException();
}

uno::Reference< word::XColumn > SwVbaColumns::getColumnAtIndex( sal_Int32 index )
{
    return uno::Reference< word::XColumn >( new SwVbaColumn( this, mxContext, mxTextTable, index ) );
}

::sal_Int32 SAL_CALL SwVbaColumns::getWidth()
{
    return getColumnAtIndex( mnStartColumnIndex )->getWidth();
}

void SAL_CALL SwVbaColumns::setWidth( ::sal_Int32 _width )
{
    for( sal_Int32 index = mnStartColumnIndex; index <= mnEndColumnIndex; index++ )
    {
        getColumnAtIndex( index )->setWidth( _width );
    }
}

void SAL_CALL SwVbaColumns::Select(  )
{
    SwVbaColumn::SelectColumn( getCurrentWordDoc(mxContext), mxTextTable, mnStartColumnIndex, mnEndColumnIndex );
}

::sal_Int32 SAL_CALL SwVbaColumns::getCount()
{
    return ( mnEndColumnIndex - mnStartColumnIndex + 1 );
}

uno::Any SAL_CALL SwVbaColumns::Item( const uno::Any& Index1, const uno::Any& /*not processed in this base class*/ )
{
    sal_Int32 nIndex = 0;
    if( Index1 >>= nIndex )
    {
        if( nIndex <= 0 || nIndex > getCount() )
        {
            throw  lang::IndexOutOfBoundsException("Index out of bounds" );
        }
        return uno::makeAny( uno::Reference< word::XColumn >( new SwVbaColumn( this, mxContext, mxTextTable, nIndex - 1 ) ) );
    }
    throw  uno::RuntimeException("Index out of bounds" );
}

// XEnumerationAccess
uno::Type
SwVbaColumns::getElementType()
{
    return cppu::UnoType<word::XColumn>::get();
}
uno::Reference< container::XEnumeration >
SwVbaColumns::createEnumeration()
{
    return new ColumnsEnumWrapper( this, mxContext, mxTextTable );
}

uno::Any
SwVbaColumns::createCollectionObject( const uno::Any& aSource )
{
    return aSource;
}

OUString
SwVbaColumns::getServiceImplName()
{
    return OUString("SwVbaColumns");
}

uno::Sequence<OUString>
SwVbaColumns::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.word.Columns";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
