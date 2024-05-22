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
#include "vbarows.hxx"
#include "vbarow.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <ooo/vba/word/WdRowAlignment.hpp>
#include <ooo/vba/word/WdConstants.hpp>
#include <ooo/vba/word/WdRulerStyle.hpp>
#include <basic/sberrors.hxx>
#include <utility>
#include "vbacolumns.hxx"
#include "vbatablehelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace {

class RowsEnumWrapper : public EnumerationHelper_BASE
{
    uno::WeakReference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< text::XTextTable > mxTextTable;
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 m_nIndex;

public:
    RowsEnumWrapper( const uno::Reference< XHelperInterface >& xParent, uno::Reference< uno::XComponentContext >  xContext, uno::Reference< text::XTextTable >  xTextTable ) : mxParent( xParent ), mxContext(std::move( xContext )), mxTextTable(std::move( xTextTable )), m_nIndex( 0 )
    {
        mxIndexAccess = mxTextTable->getRows();
    }
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( m_nIndex < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if( m_nIndex < mxIndexAccess->getCount() )
        {
            return uno::Any( uno::Reference< word::XRow > ( new SwVbaRow( mxParent, mxContext, mxTextTable, m_nIndex++ ) ) );
        }
        throw container::NoSuchElementException();
    }
};

}

SwVbaRows::SwVbaRows( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, uno::Reference< text::XTextTable >  xTextTable, const uno::Reference< table::XTableRows >& xTableRows ) : SwVbaRows_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( xTableRows, uno::UNO_QUERY_THROW ) ), mxTextTable(std::move( xTextTable )), mxTableRows( xTableRows )
{
    mnStartRowIndex = 0;
    mnEndRowIndex = m_xIndexAccess->getCount() - 1;
}

SwVbaRows::SwVbaRows( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, uno::Reference< text::XTextTable >  xTextTable, const uno::Reference< table::XTableRows >& xTableRows, sal_Int32 nStarIndex, sal_Int32 nEndIndex ) : SwVbaRows_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( xTableRows, uno::UNO_QUERY_THROW ) ), mxTextTable(std::move( xTextTable )), mxTableRows( xTableRows ), mnStartRowIndex( nStarIndex ), mnEndRowIndex( nEndIndex )
{
    if( mnEndRowIndex < mnStartRowIndex )
        throw uno::RuntimeException();
}

/**
 * get the alignment of the rows: SO format com.sun.star.text.HoriOrientation
 * is mapped to WdRowAlignment in Word
 * @return the alignment
 */
::sal_Int32 SAL_CALL SwVbaRows::getAlignment()
{
    sal_Int16 nAlignment = text::HoriOrientation::LEFT;
    uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    xTableProps->getPropertyValue(u"HoriOrient"_ustr) >>= nAlignment;
    sal_Int32 nRet = 0;
    switch( nAlignment )
    {
        case text::HoriOrientation::CENTER:
        {
           nRet = word::WdRowAlignment::wdAlignRowCenter;
           break;
        }
        case text::HoriOrientation::RIGHT:
        {
            nRet = word::WdRowAlignment::wdAlignRowRight;
            break;
        }
        default:
        {
            nRet = word::WdRowAlignment::wdAlignRowLeft;
        }
    }
    return nRet;
}

void SAL_CALL SwVbaRows::setAlignment( ::sal_Int32 _alignment )
{
    sal_Int16 nAlignment = text::HoriOrientation::LEFT;
    switch( _alignment )
    {
        case word::WdRowAlignment::wdAlignRowCenter:
        {
            nAlignment = text::HoriOrientation::CENTER;
            break;
        }
        case word::WdRowAlignment::wdAlignRowRight:
        {
            nAlignment = text::HoriOrientation::RIGHT;
            break;
        }
        default:
        {
            nAlignment = text::HoriOrientation::LEFT;
        }
    }
    uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    xTableProps->setPropertyValue(u"HoriOrient"_ustr, uno::Any( nAlignment ) );
}

uno::Any SAL_CALL SwVbaRows::getAllowBreakAcrossPages()
{
    bool bAllowBreak = false;
    uno::Reference< container::XIndexAccess > xRowsAccess( mxTableRows, uno::UNO_QUERY_THROW );
    for( sal_Int32 index = mnStartRowIndex; index <= mnEndRowIndex; ++index )
    {
        uno::Reference< beans::XPropertySet > xRowProps( xRowsAccess->getByIndex( index ), uno::UNO_QUERY_THROW );
        bool bSplit = false;
        xRowProps->getPropertyValue(u"IsSplitAllowed"_ustr) >>= bSplit;
        if( index == 0 )
        {
            bAllowBreak = bSplit;
        }
        if( bSplit != bAllowBreak )
        {
            return uno::Any( sal_Int32(word::WdConstants::wdUndefined) );
        }
    }
    return uno::Any( bAllowBreak );
}

void SAL_CALL SwVbaRows::setAllowBreakAcrossPages( const uno::Any& _allowbreakacrosspages )
{
    bool bAllowBreak = false;
    _allowbreakacrosspages >>= bAllowBreak;
    uno::Reference< container::XIndexAccess > xRowsAccess( mxTableRows, uno::UNO_QUERY_THROW );
    for( sal_Int32 index = mnStartRowIndex; index <= mnEndRowIndex; ++index )
    {
        uno::Reference< beans::XPropertySet > xRowProps( xRowsAccess->getByIndex( index ), uno::UNO_QUERY_THROW );
        xRowProps->setPropertyValue(u"IsSplitAllowed"_ustr, uno::Any( bAllowBreak ) );
    }
}

float SAL_CALL SwVbaRows::getSpaceBetweenColumns()
{
    // just get the first spacing of the first cell
    uno::Reference< table::XCellRange > xCellRange( mxTextTable, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xCellProps( xCellRange->getCellByPosition( 0, mnStartRowIndex ), uno::UNO_QUERY_THROW );
    sal_Int32 nLeftBorderDistance = 0;
    sal_Int32 nRightBorderDistance = 0;
    xCellProps->getPropertyValue(u"LeftBorderDistance"_ustr) >>= nLeftBorderDistance;
    xCellProps->getPropertyValue(u"RightBorderDistance"_ustr) >>= nRightBorderDistance;
    return static_cast< float >( Millimeter::getInPoints( nLeftBorderDistance + nRightBorderDistance ) );
}

void SAL_CALL SwVbaRows::setSpaceBetweenColumns( float _spacebetweencolumns )
{
    sal_Int32 nSpace = Millimeter::getInHundredthsOfOneMillimeter( _spacebetweencolumns ) / 2;
    uno::Reference< container::XIndexAccess > xColumnAccess( mxTextTable->getColumns(), uno::UNO_QUERY_THROW );
    uno::Reference< table::XCellRange > xCellRange( mxTextTable, uno::UNO_QUERY_THROW );
    SwVbaTableHelper aTableHelper( mxTextTable );
    for( sal_Int32 row = mnStartRowIndex; row <= mnEndRowIndex; ++row )
    {
        sal_Int32 nColumns = aTableHelper.getTabColumnsCount( row );
        for( sal_Int32 column = 0; column < nColumns; ++column )
        {
            uno::Reference< beans::XPropertySet > xCellProps( xCellRange->getCellByPosition( column, row ), uno::UNO_QUERY_THROW );
            xCellProps->setPropertyValue(u"LeftBorderDistance"_ustr, uno::Any( nSpace ) );
            xCellProps->setPropertyValue(u"RightBorderDistance"_ustr, uno::Any( nSpace ) );
        }
    }
}

void SAL_CALL SwVbaRows::Delete(  )
{
    mxTableRows->removeByIndex( mnStartRowIndex, getCount() );
}

void SAL_CALL SwVbaRows::SetLeftIndent( float LeftIndent, ::sal_Int32 RulerStyle )
{
    uno::Reference< word::XColumns > xColumns( new SwVbaColumns( getParent(), mxContext, mxTextTable, mxTextTable->getColumns() ) );
    sal_Int32 nIndent = static_cast<sal_Int32>(LeftIndent);
    switch( RulerStyle )
    {
        case word::WdRulerStyle::wdAdjustFirstColumn:
        {
            setIndentWithAdjustFirstColumn( xColumns, nIndent );
            break;
        }
        case word::WdRulerStyle::wdAdjustNone:
        {
            setIndentWithAdjustNone( nIndent );
            break;
        }
        case word::WdRulerStyle::wdAdjustProportional:
        {
            setIndentWithAdjustProportional( xColumns, nIndent );
            break;
        }
        case word::WdRulerStyle::wdAdjustSameWidth:
        {
            setIndentWithAdjustSameWidth( xColumns, nIndent );
            break;
        }
        default:
        {
            DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_ARGUMENT);
        }
    }
}

void SwVbaRows::setIndentWithAdjustNone( sal_Int32 indent )
{
    uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    sal_Int32 nMargin = 0;
    xTableProps->getPropertyValue(u"LeftMargin"_ustr) >>= nMargin;
    nMargin += indent;
    xTableProps->setPropertyValue(u"LeftMargin"_ustr, uno::Any( nMargin ) );
}

 void SwVbaRows::setIndentWithAdjustFirstColumn( const uno::Reference< word::XColumns >& xColumns, sal_Int32 indent )
 {
    uno::Reference< XCollection > xCol( xColumns, uno::UNO_QUERY_THROW );
    uno::Reference< word::XColumn > xColumn( xCol->Item( uno::Any( sal_Int32(1) ), uno::Any() ), uno::UNO_QUERY_THROW );
    sal_Int32 nWidth = xColumn->getWidth();
    nWidth -= indent;
    xColumn->setWidth( nWidth );
    setIndentWithAdjustNone( indent );
 }

 void SwVbaRows::setIndentWithAdjustProportional(
    const uno::Reference< word::XColumns >& xColumns,
    sal_Int32 indent
)
 {
    // calculate the new width and get the proportion between old and new
    uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    sal_Int32 nWidth = 0;
    xTableProps->getPropertyValue(u"Width"_ustr) >>= nWidth;
    sal_Int32 nNewWidth = nWidth - indent;
    if ((nNewWidth <= 0) || (nWidth <= 0))
    {
        throw uno::RuntimeException(
            u"Pb with width, in SwVbaRows::setIndentWithAdjustProportional "
            "(nNewWidth <= 0) || (nWidth <= 0)"_ustr
        );
    }
    double propFactor = static_cast<double>(nNewWidth)/static_cast<double>(nWidth);

    // get all columns, calculate and set the new width of the columns
    uno::Reference< XCollection > xCol( xColumns, uno::UNO_QUERY_THROW );
    sal_Int32 nColCount = xCol->getCount();
    for( sal_Int32 i = 0; i < nColCount; i++ )
    {
        uno::Reference< word::XColumn > xColumn( xCol->Item( uno::Any( i ), uno::Any() ), uno::UNO_QUERY_THROW );
        sal_Int32 nColWidth = xColumn->getWidth();
        sal_Int32 nNewColWidth = static_cast<sal_Int32>( propFactor * nColWidth );
        xColumn->setWidth( nNewColWidth );
    }

    // set the width and position of the table
    setIndentWithAdjustNone( indent );
    xTableProps->setPropertyValue(u"Width"_ustr, uno::Any( nNewWidth ) );
 }

 void SwVbaRows::setIndentWithAdjustSameWidth( const uno::Reference< word::XColumns >& xColumns, sal_Int32 indent )
 {
    // calculate the new width and get the width of all columns
    uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    sal_Int32 nWidth = 0;
    xTableProps->getPropertyValue(u"Width"_ustr) >>= nWidth;
    sal_Int32 nNewWidth = nWidth - indent;

    // get all columns, calculate and set the new width of the columns
    uno::Reference< XCollection > xCol( xColumns, uno::UNO_QUERY_THROW );
    sal_Int32 nColCount = xCol->getCount();
    sal_Int32 nNewColWidth = static_cast<sal_Int32>( double( nNewWidth )/nColCount );
    for( sal_Int32 i = 0; i < nColCount; i++ )
    {
        uno::Reference< word::XColumn > xColumn( xCol->Item( uno::Any( i ), uno::Any() ), uno::UNO_QUERY_THROW );
        xColumn->setWidth( nNewColWidth );
    }

    // set the width and position of the table
    setIndentWithAdjustNone( indent );
    xTableProps->setPropertyValue(u"Width"_ustr, uno::Any( nNewWidth ) );
 }

void SAL_CALL SwVbaRows::Select(  )
{
    SwVbaRow::SelectRow( getCurrentWordDoc(mxContext), mxTextTable, mnStartRowIndex, mnEndRowIndex );
}

::sal_Int32 SAL_CALL SwVbaRows::getCount()
{
    return ( mnEndRowIndex - mnStartRowIndex + 1 );
}

uno::Any SAL_CALL SwVbaRows::Item( const uno::Any& Index1, const uno::Any& /*not processed in this base class*/ )
{
    sal_Int32 nIndex = 0;
    if( Index1 >>= nIndex )
    {
        if( nIndex <= 0 || nIndex > getCount() )
        {
            throw  lang::IndexOutOfBoundsException(u"Index out of bounds"_ustr );
        }
        return uno::Any( uno::Reference< word::XRow >( new SwVbaRow( this, mxContext, mxTextTable, nIndex - 1 ) ) );
    }
    throw  uno::RuntimeException(u"Index out of bounds"_ustr );
}

// XEnumerationAccess
uno::Type
SwVbaRows::getElementType()
{
    return cppu::UnoType<word::XRow>::get();
}
uno::Reference< container::XEnumeration >
SwVbaRows::createEnumeration()
{
    return new RowsEnumWrapper( this, mxContext, mxTextTable );
}

uno::Any
SwVbaRows::createCollectionObject( const uno::Any& aSource )
{
    return aSource;
}

OUString
SwVbaRows::getServiceImplName()
{
    return u"SwVbaRows"_ustr;
}

uno::Sequence<OUString>
SwVbaRows::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.Rows"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
