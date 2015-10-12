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
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <ooo/vba/word/WdRowAlignment.hpp>
#include <ooo/vba/word/WdConstants.hpp>
#include <ooo/vba/word/WdRulerStyle.hpp>
#include "wordvbahelper.hxx"
#include "vbacolumns.hxx"
#include "vbatablehelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class RowsEnumWrapper : public EnumerationHelper_BASE
{
    uno::WeakReference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< text::XTextTable > mxTextTable;
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 nIndex;

public:
    RowsEnumWrapper( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< text::XTextTable >& xTextTable ) : mxParent( xParent ), mxContext( xContext ), mxTextTable( xTextTable ), nIndex( 0 )
    {
        mxIndexAccess.set( mxTextTable->getRows(), uno::UNO_QUERY );
    }
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException, std::exception) override
    {
        return ( nIndex < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        if( nIndex < mxIndexAccess->getCount() )
        {
            return uno::makeAny( uno::Reference< word::XRow > ( new SwVbaRow( mxParent, mxContext, mxTextTable, nIndex++ ) ) );
        }
        throw container::NoSuchElementException();
    }
};

SwVbaRows::SwVbaRows( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextTable >& xTextTable, const uno::Reference< table::XTableRows >& xTableRows ) throw (uno::RuntimeException) : SwVbaRows_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( xTableRows, uno::UNO_QUERY_THROW ) ), mxTextTable( xTextTable ), mxTableRows( xTableRows )
{
    mnStartRowIndex = 0;
    mnEndRowIndex = m_xIndexAccess->getCount() - 1;
}

SwVbaRows::SwVbaRows( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextTable >& xTextTable, const uno::Reference< table::XTableRows >& xTableRows, sal_Int32 nStarIndex, sal_Int32 nEndIndex ) throw (uno::RuntimeException) : SwVbaRows_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( xTableRows, uno::UNO_QUERY_THROW ) ), mxTextTable( xTextTable ), mxTableRows( xTableRows ), mnStartRowIndex( nStarIndex ), mnEndRowIndex( nEndIndex )
{
    if( mnEndRowIndex < mnStartRowIndex )
        throw uno::RuntimeException();
}

/**
 * get the alignment of the rows: SO format com.sun.star.text.HoriOrientation
 * is mapped to WdRowAlignment in Word
 * @return the alignment
 */
::sal_Int32 SAL_CALL SwVbaRows::getAlignment() throw (uno::RuntimeException, std::exception)
{
    sal_Int16 nAlignment = text::HoriOrientation::LEFT;
    uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    xTableProps->getPropertyValue("HoriOrient") >>= nAlignment;
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

void SAL_CALL SwVbaRows::setAlignment( ::sal_Int32 _alignment ) throw (uno::RuntimeException, std::exception)
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
    xTableProps->setPropertyValue("HoriOrient", uno::makeAny( nAlignment ) );
}

uno::Any SAL_CALL SwVbaRows::getAllowBreakAcrossPages() throw (uno::RuntimeException, std::exception)
{
    bool bAllowBreak = false;
    uno::Reference< container::XIndexAccess > xRowsAccess( mxTableRows, uno::UNO_QUERY_THROW );
    for( sal_Int32 index = mnStartRowIndex; index <= mnEndRowIndex; ++index )
    {
        uno::Reference< beans::XPropertySet > xRowProps( xRowsAccess->getByIndex( index ), uno::UNO_QUERY_THROW );
        bool bSplit = false;
        xRowProps->getPropertyValue("IsSplitAllowed") >>= bSplit;
        if( index == 0 )
        {
            bAllowBreak = bSplit;
        }
        if( bSplit != bAllowBreak )
        {
            sal_Int32 nRet = word::WdConstants::wdUndefined;
            return uno::makeAny( nRet );
        }
    }
    return uno::makeAny( bAllowBreak );
}

void SAL_CALL SwVbaRows::setAllowBreakAcrossPages( const uno::Any& _allowbreakacrosspages ) throw (uno::RuntimeException, std::exception)
{
    bool bAllowBreak = false;
    _allowbreakacrosspages >>= bAllowBreak;
    uno::Reference< container::XIndexAccess > xRowsAccess( mxTableRows, uno::UNO_QUERY_THROW );
    for( sal_Int32 index = mnStartRowIndex; index <= mnEndRowIndex; ++index )
    {
        uno::Reference< beans::XPropertySet > xRowProps( xRowsAccess->getByIndex( index ), uno::UNO_QUERY_THROW );
        xRowProps->setPropertyValue("IsSplitAllowed", uno::makeAny( bAllowBreak ) );
    }
}

float SAL_CALL SwVbaRows::getSpaceBetweenColumns() throw (uno::RuntimeException, std::exception)
{
    // just get the first spacing of the first cell
    uno::Reference< table::XCellRange > xCellRange( mxTextTable, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xCellProps( xCellRange->getCellByPosition( 0, mnStartRowIndex ), uno::UNO_QUERY_THROW );
    sal_Int32 nLeftBorderDistance = 0;
    sal_Int32 nRightBorderDistance = 0;
    xCellProps->getPropertyValue("LeftBorderDistance") >>= nLeftBorderDistance;
    xCellProps->getPropertyValue("RightBorderDistance") >>= nRightBorderDistance;
    return static_cast< float >( Millimeter::getInPoints( nLeftBorderDistance + nRightBorderDistance ) );
}

void SAL_CALL SwVbaRows::setSpaceBetweenColumns( float _spacebetweencolumns ) throw (uno::RuntimeException, std::exception)
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
            xCellProps->setPropertyValue("LeftBorderDistance", uno::makeAny( nSpace ) );
            xCellProps->setPropertyValue("RightBorderDistance", uno::makeAny( nSpace ) );
        }
    }
}

void SAL_CALL SwVbaRows::Delete(  ) throw (uno::RuntimeException, std::exception)
{
    mxTableRows->removeByIndex( mnStartRowIndex, getCount() );
}

void SAL_CALL SwVbaRows::SetLeftIndent( float LeftIndent, ::sal_Int32 RulerStyle ) throw (uno::RuntimeException, std::exception)
{
    uno::Reference< word::XColumns > xColumns( new SwVbaColumns( getParent(), mxContext, mxTextTable, mxTextTable->getColumns() ) );
    sal_Int32 nIndent = (sal_Int32)( LeftIndent );
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
            DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_ARGUMENT, OUString());
        }
    }
}

void SwVbaRows::setIndentWithAdjustNone( sal_Int32 indent ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    sal_Int32 nMargin = 0;
    xTableProps->getPropertyValue("LeftMargin") >>= nMargin;
    nMargin += indent;
    xTableProps->setPropertyValue("LeftMargin", uno::makeAny( nMargin ) );
}

 void SwVbaRows::setIndentWithAdjustFirstColumn( const uno::Reference< word::XColumns >& xColumns, sal_Int32 indent ) throw (uno::RuntimeException)
 {
    sal_Int32 nIndex = 1;
    uno::Reference< XCollection > xCol( xColumns, uno::UNO_QUERY_THROW );
    uno::Reference< word::XColumn > xColumn( xCol->Item( uno::makeAny( nIndex ), uno::Any() ), uno::UNO_QUERY_THROW );
    sal_Int32 nWidth = xColumn->getWidth();
    nWidth -= indent;
    xColumn->setWidth( nWidth );
    setIndentWithAdjustNone( indent );
 }

 void SwVbaRows::setIndentWithAdjustProportional(
    const uno::Reference< word::XColumns >& xColumns,
    sal_Int32 indent
) throw (uno::RuntimeException)
 {
    // calculate the new width and get the proportion between old and new
    uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    sal_Int32 nWidth = 0;
    xTableProps->getPropertyValue("Width") >>= nWidth;
    sal_Int32 nNewWidth = nWidth - indent;
    if ((nNewWidth <= 0) || (nWidth <= 0))
    {
        throw uno::RuntimeException(
            "Pb with width, in SwVbaRows::setIndentWithAdjustProportional "
            "(nNewWidth <= 0) || (nWidth <= 0)"
        );
    }
    double propFactor = (double)nNewWidth/(double)nWidth;

    // get all columns, calculate and set the new width of the columns
    uno::Reference< XCollection > xCol( xColumns, uno::UNO_QUERY_THROW );
    sal_Int32 nColCount = xCol->getCount();
    for( sal_Int32 i = 0; i < nColCount; i++ )
    {
        uno::Reference< word::XColumn > xColumn( xCol->Item( uno::makeAny( i ), uno::Any() ), uno::UNO_QUERY_THROW );
        sal_Int32 nColWidth = xColumn->getWidth();
        sal_Int32 nNewColWidth = ( sal_Int32 )( propFactor * nColWidth );
        xColumn->setWidth( nNewColWidth );
    }

    // set the width and position of the table
    setIndentWithAdjustNone( indent );
    xTableProps->setPropertyValue("Width", uno::makeAny( nNewWidth ) );
 }

 void SwVbaRows::setIndentWithAdjustSameWidth( const uno::Reference< word::XColumns >& xColumns, sal_Int32 indent ) throw (uno::RuntimeException)
 {
    // calculate the new width and get the width of all columns
    uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    sal_Int32 nWidth = 0;
    xTableProps->getPropertyValue("Width") >>= nWidth;
    sal_Int32 nNewWidth = nWidth - indent;

    // get all columns, calculate and set the new width of the columns
    uno::Reference< XCollection > xCol( xColumns, uno::UNO_QUERY_THROW );
    sal_Int32 nColCount = xCol->getCount();
    sal_Int32 nNewColWidth = (sal_Int32)( double( nNewWidth )/nColCount );
    for( sal_Int32 i = 0; i < nColCount; i++ )
    {
        uno::Reference< word::XColumn > xColumn( xCol->Item( uno::makeAny( i ), uno::Any() ), uno::UNO_QUERY_THROW );
        xColumn->setWidth( nNewColWidth );
    }

    // set the width and position of the table
    setIndentWithAdjustNone( indent );
    xTableProps->setPropertyValue("Width", uno::makeAny( nNewWidth ) );
 }

void SAL_CALL SwVbaRows::Select(  ) throw (uno::RuntimeException, std::exception)
{
    SwVbaRow::SelectRow( getCurrentWordDoc(mxContext), mxTextTable, mnStartRowIndex, mnEndRowIndex );
}

::sal_Int32 SAL_CALL SwVbaRows::getCount() throw (uno::RuntimeException)
{
    return ( mnEndRowIndex - mnStartRowIndex + 1 );
}

uno::Any SAL_CALL SwVbaRows::Item( const uno::Any& Index1, const uno::Any& /*not processed in this base class*/ ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    if( ( Index1 >>= nIndex ) )
    {
        if( nIndex <= 0 || nIndex > getCount() )
        {
            throw  lang::IndexOutOfBoundsException("Index out of bounds" );
        }
        return uno::makeAny( uno::Reference< word::XRow >( new SwVbaRow( this, mxContext, mxTextTable, nIndex - 1 ) ) );
    }
    throw  uno::RuntimeException("Index out of bounds" );
}

// XEnumerationAccess
uno::Type
SwVbaRows::getElementType() throw (uno::RuntimeException)
{
    return cppu::UnoType<word::XRow>::get();
}
uno::Reference< container::XEnumeration >
SwVbaRows::createEnumeration() throw (uno::RuntimeException)
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
    return OUString("SwVbaRows");
}

uno::Sequence<OUString>
SwVbaRows::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.word.Rows";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
