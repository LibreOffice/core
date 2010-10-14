/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( nIndex < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if( nIndex < mxIndexAccess->getCount() )
        {
            return uno::makeAny( uno::Reference< word::XColumn > ( new SwVbaColumn( mxParent, mxContext, mxTextTable, nIndex++ ) ) );
        }
        throw container::NoSuchElementException();
    }
};

SwVbaColumns::SwVbaColumns( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextTable >& xTextTable, const uno::Reference< table::XTableColumns >& xTableColumns ) throw (uno::RuntimeException) : SwVbaColumns_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( xTableColumns, uno::UNO_QUERY_THROW ) ), mxTextTable( xTextTable ), mxTableColumns( xTableColumns )
{
    mnStartColumnIndex = 0;
    SwVbaTableHelper aTableHelper( mxTextTable );
    mnEndColumnIndex = aTableHelper.getTabColumnsMaxCount( ) - 1;
}

SwVbaColumns::SwVbaColumns( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextTable >& xTextTable, const uno::Reference< table::XTableColumns >& xTableColumns, sal_Int32 nStartCol, sal_Int32 nEndCol ) throw (uno::RuntimeException) : SwVbaColumns_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( xTableColumns, uno::UNO_QUERY_THROW ) ), mxTextTable( xTextTable ), mxTableColumns( xTableColumns ), mnStartColumnIndex( nStartCol ), mnEndColumnIndex( nEndCol )
{
    if( mnEndColumnIndex < mnStartColumnIndex )
        throw uno::RuntimeException();
}

uno::Reference< word::XColumn > SwVbaColumns::getColumnAtIndex( sal_Int32 index ) throw (uno::RuntimeException)
{
    return uno::Reference< word::XColumn >( new SwVbaColumn( this, mxContext, mxTextTable, index ) );
}

::sal_Int32 SAL_CALL SwVbaColumns::getWidth() throw (uno::RuntimeException)
{
    return getColumnAtIndex( mnStartColumnIndex )->getWidth();
}

void SAL_CALL SwVbaColumns::setWidth( ::sal_Int32 _width ) throw (uno::RuntimeException)
{
    for( sal_Int32 index = mnStartColumnIndex; index <= mnEndColumnIndex; index++ )
    {
        getColumnAtIndex( index )->setWidth( _width );
    }
}

void SAL_CALL SwVbaColumns::Select(  ) throw (uno::RuntimeException)
{
    SwVbaColumn::SelectColumn( getCurrentWordDoc(mxContext), mxTextTable, mnStartColumnIndex, mnEndColumnIndex );
}

::sal_Int32 SAL_CALL SwVbaColumns::getCount() throw (uno::RuntimeException)
{
    return ( mnEndColumnIndex - mnStartColumnIndex + 1 );
}

uno::Any SAL_CALL SwVbaColumns::Item( const uno::Any& Index1, const uno::Any& /*not processed in this base class*/ ) throw (uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    if( ( Index1 >>= nIndex ) == sal_True )
    {
        if( nIndex <= 0 || nIndex > getCount() )
        {
            throw  lang::IndexOutOfBoundsException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Index out of bounds") ), uno::Reference< uno::XInterface >() );
        }
        return uno::makeAny( uno::Reference< word::XColumn >( new SwVbaColumn( this, mxContext, mxTextTable, nIndex - 1 ) ) );
    }
    throw  uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Index out of bounds") ), uno::Reference< uno::XInterface >() );
}

// XEnumerationAccess
uno::Type
SwVbaColumns::getElementType() throw (uno::RuntimeException)
{
    return word::XColumn::static_type(0);
}
uno::Reference< container::XEnumeration >
SwVbaColumns::createEnumeration() throw (uno::RuntimeException)
{
    return new ColumnsEnumWrapper( this, mxContext, mxTextTable );
}

uno::Any
SwVbaColumns::createCollectionObject( const uno::Any& aSource )
{
    return aSource;
}

rtl::OUString&
SwVbaColumns::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaColumns") );
    return sImplName;
}

uno::Sequence<rtl::OUString>
SwVbaColumns::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Columns") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
