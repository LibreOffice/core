/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "vbaselection.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbarange.hxx"
#include "vbafind.hxx"
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTableCursor.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <comphelper/string.hxx>
#include <ooo/vba/word/WdUnits.hpp>
#include <ooo/vba/word/WdMovementType.hpp>
#include <ooo/vba/word/WdGoToItem.hpp>
#include <ooo/vba/word/WdGoToDirection.hpp>
#include <ooo/vba/word/XBookmark.hpp>
#include <ooo/vba/word/XApplication.hpp>
#include <ooo/vba/word/WdCollapseDirection.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include "unotbl.hxx"
#include "unocoll.hxx"
#include "vbatable.hxx"
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XViewCursor.hpp>
#include <com/sun/star/view/XLineCursor.hpp>
#include <com/sun/star/text/XWordCursor.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <ooo/vba/word/WdInformation.hpp>
#include <ooo/vba/word/WdHeaderFooterIndex.hpp>
#include <ooo/vba/word/WdSeekView.hpp>
#include "vbainformationhelper.hxx"
#include "vbafield.hxx"
#include "vbaheaderfooter.hxx"
#include "vbaheaderfooterhelper.hxx"
#include <vbahelper/vbashaperange.hxx>
#include <com/sun/star/drawing/ShapeCollection.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include "vbarows.hxx"
#include "vbacolumns.hxx"
#include "vbatablehelper.hxx"
#include "vbacells.hxx"
#include "vbaview.hxx"
#include "vbaparagraph.hxx"
#include "vbastyle.hxx"
#include <docsh.hxx>
#include <tblenum.hxx>
#include <fesh.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaSelection::SwVbaSelection( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< frame::XModel >& rModel ) throw ( uno::RuntimeException ) : SwVbaSelection_BASE( rParent, rContext ), mxModel( rModel )
{
    mxTextViewCursor = word::getXTextViewCursor( mxModel );
}

SwVbaSelection::~SwVbaSelection()
{
}

uno::Reference< text::XTextRange > SwVbaSelection::GetSelectedRange() throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextRange > xTextRange;
    uno::Reference< lang::XServiceInfo > xServiceInfo( mxModel->getCurrentSelection(), uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextRanges") ) ) )
    {
        uno::Reference< container::XIndexAccess > xTextRanges( xServiceInfo, uno::UNO_QUERY_THROW );
        if( xTextRanges->getCount() > 0 )
        {
            // if there are multipul selection, just return the last selected Range.
            xTextRange.set( xTextRanges->getByIndex( xTextRanges->getCount()-1 ), uno::UNO_QUERY_THROW );
        }
    }
    else
    {
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
    }
    return xTextRange;
}

uno::Reference< word::XRange > SAL_CALL
SwVbaSelection::getRange() throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextRange > xTextRange = GetSelectedRange();
    uno::Reference< text::XTextDocument > xDocument( mxModel, uno::UNO_QUERY_THROW );
    return uno::Reference< word::XRange >( new SwVbaRange( this, mxContext, xDocument, xTextRange->getStart(), xTextRange->getEnd(), mxTextViewCursor->getText() ) );
}

rtl::OUString SAL_CALL
SwVbaSelection::getText() throw ( uno::RuntimeException )
{
    return getRange()->getText();
}

void SAL_CALL
SwVbaSelection::setText( const rtl::OUString& rText ) throw ( uno::RuntimeException )
{
    getRange()->setText( rText );
}

void SAL_CALL
SwVbaSelection::TypeText( const rtl::OUString& rText ) throw ( uno::RuntimeException )
{
    // FIXME: handle the property Options.ReplaceSelection, the default value is sal_True
    setText( rText );
}

void SAL_CALL
SwVbaSelection::HomeKey( const uno::Any& _unit, const uno::Any& _extend ) throw ( uno::RuntimeException )
{
    sal_Int32 nUnit = word::WdUnits::wdLine;
    sal_Int32 nExtend = word::WdMovementType::wdMove;
    _unit >>= nUnit;
    _extend >>= nExtend;
    sal_Bool bExtend = ( nExtend == word::WdMovementType::wdExtend ) ? sal_True : sal_False;

    switch( nUnit )
    {
        case word::WdUnits::wdStory:
        {
            // go to the valid text first so that the current view cursor is valid to call gotoRange.
            word::gotoSelectedObjectAnchor(mxModel);
            // go to the begin of the document
            uno::Reference< text::XText > xCurrentText = word::getCurrentXText( mxModel );
            uno::Reference< text::XTextRange > xFirstRange = word::getFirstObjectPosition( xCurrentText );
            mxTextViewCursor->gotoRange( xFirstRange, bExtend );
            break;
        }
        case word::WdUnits::wdLine:
        {
            // go to the begin of the Line
            uno::Reference< view::XLineCursor > xLineCursor( mxTextViewCursor, uno::UNO_QUERY_THROW );
            xLineCursor->gotoStartOfLine( bExtend );
            break;
        }
        default:
        {
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
        }
    }
}

void SAL_CALL
SwVbaSelection::EndKey( const uno::Any& _unit, const uno::Any& _extend ) throw ( uno::RuntimeException )
{
    sal_Int32 nUnit = word::WdUnits::wdLine;
    sal_Int32 nExtend = word::WdMovementType::wdMove;
    _unit >>= nUnit;
    _extend >>= nExtend;
    sal_Bool bExtend = ( nExtend == word::WdMovementType::wdExtend ) ? sal_True : sal_False;

    switch( nUnit )
    {
        case word::WdUnits::wdStory:
        {
            // go to the valid text first so that the current view cursor is valid to call gotoRange.
            word::gotoSelectedObjectAnchor(mxModel);
            // go to the end of the document
            uno::Reference< text::XText > xCurrentText = word::getCurrentXText( mxModel );
            uno::Reference< text::XTextRange > xEnd = xCurrentText->getEnd();
            mxTextViewCursor->gotoRange( xEnd, bExtend );
            break;
        }
        case word::WdUnits::wdLine:
        {
            // go to the end of the Line
            uno::Reference< view::XLineCursor > xLineCursor( mxTextViewCursor, uno::UNO_QUERY_THROW );
            xLineCursor->gotoEndOfLine( bExtend );
            break;
        }
        default:
        {
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
        }
    }
}

void SAL_CALL
SwVbaSelection::Delete( const uno::Any& _unit, const uno::Any& _count ) throw ( uno::RuntimeException )
{
    sal_Int32 nUnit = word::WdUnits::wdLine;
    sal_Int32 nCount = 0;
    if( _count.hasValue() )
        _count >>= nCount;
    if( _unit.hasValue() && ( nCount > 0 ) )
    {
        _unit >>= nUnit;
        switch( nUnit )
        {
            case word::WdUnits::wdCharacter:
            {
                if( HasSelection() )
                    nCount--;
                mxTextViewCursor->goRight( nCount, sal_True );
                break;
            }
            default:
            {
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
            }
        }
    }
    rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Delete"));
    dispatchRequests( mxModel,url );
}

void
SwVbaSelection::Move( const uno::Any& _unit, const uno::Any& _count, const uno::Any& _extend, word::E_DIRECTION eDirection ) throw ( uno::RuntimeException )
{
    sal_Int32 nUnit = word::WdUnits::wdCharacter;
    sal_Int32 nCount = 1;
    sal_Int32 nExtend = word::WdMovementType::wdMove;

    if( _unit.hasValue() )
        _unit >>= nUnit;
    if( _count.hasValue() )
        _count >>= nCount;
    if( _extend.hasValue() )
        _extend >>= nExtend;

    if( nCount == 0 )
        return;

    sal_Bool bExpand = ( nExtend == word::WdMovementType::wdMove ) ? sal_False : sal_True;

    switch( nUnit )
    {
        case word::WdUnits::wdCell:
        {
            if(  nExtend == word::WdMovementType::wdExtend )
            {
                DebugHelper::exception(SbERR_BAD_ARGUMENT, rtl::OUString());
                return;
            }
            NextCell( nCount, eDirection );
            break;
        }
        case word::WdUnits::wdLine:
        {
            if( eDirection == word::MOVE_LEFT || eDirection == word::MOVE_RIGHT )
            {
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
            }
            uno::Reference< view::XViewCursor > xViewCursor( mxTextViewCursor, uno::UNO_QUERY_THROW );
            if( eDirection == word::MOVE_UP )
                xViewCursor->goUp( nCount, bExpand );
            else if( eDirection == word::MOVE_DOWN )
                xViewCursor->goDown( nCount, bExpand );
            break;
        }
        case word::WdUnits::wdCharacter:
        {
            if( eDirection == word::MOVE_UP || eDirection == word::MOVE_DOWN )
            {
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
            }
            if( word::gotoSelectedObjectAnchor( mxModel ) )
            {
                nCount--;
            }
            uno::Reference< view::XViewCursor > xViewCursor( mxTextViewCursor, uno::UNO_QUERY_THROW );
            if( eDirection == word::MOVE_LEFT )
            {
                // if current select is a cellrange or table,
                // the first count of move should move to the first selected cell.
                uno::Reference< text::XTextTableCursor > xTextTableCursor( mxModel->getCurrentSelection(), uno::UNO_QUERY );
                if ( xTextTableCursor.is() )
                {
                    uno::Reference< beans::XPropertySet > xCursorProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
                    uno::Reference< text::XTextTable > xTextTable;
                    xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextTable") ) ) >>= xTextTable;
                    if( xTextTable.is() )
                    {
                        uno::Reference< text::XTextRange > xRange( xTextTable->getCellByName( xTextTableCursor->getRangeName()), uno::UNO_QUERY_THROW );
                        mxTextViewCursor->gotoRange( xRange->getStart(), bExpand );
                        nCount--;
                    }
                }
                xViewCursor->goLeft( nCount, bExpand );
            }
            else if( eDirection == word::MOVE_RIGHT )
                xViewCursor->goRight( nCount, bExpand );
            break;
        }
        case word::WdUnits::wdWord:
        case word::WdUnits::wdParagraph:
        {
            uno::Reference< text::XTextRange > xRange = GetSelectedRange();
            uno::Reference< text::XText > xText = xRange->getText();
            uno::Reference< text::XTextCursor > xTextCursor = xText->createTextCursorByRange( xRange );
            if( nUnit == word::WdUnits::wdParagraph )
            {
                if( eDirection == word::MOVE_LEFT || eDirection == word::MOVE_RIGHT )
                {
                    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
                }
                uno::Reference< text::XParagraphCursor > xParagraphCursor( xTextCursor, uno::UNO_QUERY_THROW );
                for( sal_Int32 i=0; i<nCount; i++ )
                {
                    if( ( eDirection == word::MOVE_UP ) && ( xParagraphCursor->gotoPreviousParagraph( bExpand ) == sal_False ) )
                        break;
                    else if( ( eDirection == word::MOVE_DOWN ) && ( xParagraphCursor->gotoNextParagraph( bExpand ) == sal_False ) )
                        break;
                }
            }
            else if( nUnit == word::WdUnits::wdWord )
            {
                if( eDirection == word::MOVE_UP || eDirection == word::MOVE_DOWN )
                {
                    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
                }
                uno::Reference< text::XWordCursor > xWordCursor( xTextCursor, uno::UNO_QUERY_THROW );
                for( sal_Int32 i=0; i<nCount; i++ )
                {
                    if( (eDirection == word::MOVE_LEFT ) && ( xWordCursor->gotoPreviousWord( bExpand ) == sal_False ) )
                        break;
                    else if( ( eDirection == word::MOVE_RIGHT ) && ( xWordCursor->gotoNextWord( bExpand ) == sal_False ) )
                        break;
                }
            }
            mxTextViewCursor->gotoRange( xTextCursor->getStart(), sal_False );
            mxTextViewCursor->gotoRange( xTextCursor->getEnd(), sal_True );
            break;
        }
        default:
        {
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
            break;
        }
    }
}

void SwVbaSelection::NextCell( sal_Int32 nCount, word::E_DIRECTION eDirection ) throw ( uno::RuntimeException )
{
    uno::Reference< beans::XPropertySet > xCursorProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextTable > xTextTable;
    uno::Reference< table::XCell > xCell;
    xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextTable") ) ) >>= xTextTable;
    xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Cell") ) ) >>= xCell;
    if( !xTextTable.is() || !xCell.is() )
    {
        DebugHelper::exception(SbERR_BAD_ARGUMENT, rtl::OUString());
        return;
    }
    uno::Reference< beans::XPropertySet > xCellProps( xCell, uno::UNO_QUERY_THROW );
    rtl::OUString aCellName;
    xCellProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CellName") ) ) >>= aCellName;
    uno::Reference< text::XTextTableCursor > xTextTableCursor = xTextTable->createCursorByCellName( aCellName );
    // move the table cursor
    switch( eDirection )
    {
        case word::MOVE_LEFT:
        {
            xTextTableCursor->goLeft( nCount, sal_False );
            break;
        }
        case word::MOVE_RIGHT:
        {
            xTextTableCursor->goRight( nCount, sal_False );
            break;
        }
        case word::MOVE_UP:
        {
            xTextTableCursor->goUp( nCount, sal_False );
            break;
        }
        case word::MOVE_DOWN:
        {
            xTextTableCursor->goDown( nCount, sal_False );
            break;
        }
        default:
        {
            DebugHelper::exception(SbERR_BAD_ARGUMENT, rtl::OUString());
            return;
        }
    }
    // move the view cursor
    xCell = xTextTable->getCellByName( xTextTableCursor->getRangeName() );
    mxTextViewCursor->gotoRange( uno::Reference< text::XTextRange >( xCell, uno::UNO_QUERY_THROW ), sal_False );
}

void SAL_CALL
SwVbaSelection::MoveRight( const uno::Any& _unit, const uno::Any& _count, const uno::Any& _extend ) throw ( uno::RuntimeException )
{
    sal_Int32 nCount = 1;

    if( _count.hasValue() )
        _count >>= nCount;

    if( nCount == 0 )
        return;

    if( nCount < 0 )
    {
        MoveLeft( _unit, uno::makeAny( -nCount ), _extend );
        return;
    }

    Move( _unit, _count, _extend, word::MOVE_RIGHT );
}

void SAL_CALL
SwVbaSelection::MoveLeft( const uno::Any& _unit, const uno::Any& _count, const uno::Any& _extend ) throw ( uno::RuntimeException )
{
    sal_Int32 nCount = 1;
    if( _count.hasValue() )
        _count >>= nCount;

    if( nCount == 0 )
        return;

    if( nCount < 0 )
    {
        MoveRight( _unit, uno::makeAny( -nCount ), _extend );
        return;
    }

    Move( _unit, _count, _extend, word::MOVE_LEFT );
}

void SAL_CALL
SwVbaSelection::MoveDown( const uno::Any& _unit, const uno::Any& _count, const uno::Any& _extend ) throw ( uno::RuntimeException )
{
    sal_Int32 nCount = 1;

    if( _count.hasValue() )
        _count >>= nCount;

    if( nCount == 0 )
        return;

    if( nCount < 0 )
    {
        MoveUp( _unit, uno::makeAny( -nCount ), _extend );
        return;
    }

    Move( _unit, _count, _extend, word::MOVE_DOWN );
}

void SAL_CALL
SwVbaSelection::MoveUp( const uno::Any& _unit, const uno::Any& _count, const uno::Any& _extend ) throw ( uno::RuntimeException )
{
    sal_Int32 nCount = 1;

    if( _count.hasValue() )
        _count >>= nCount;

    if( nCount == 0 )
        return;

    if( nCount < 0 )
    {
        MoveDown( _unit, uno::makeAny( -nCount ), _extend );
        return;
    }

    Move( _unit, _count, _extend, word::MOVE_UP );
}

void SAL_CALL
SwVbaSelection::TypeParagraph() throw ( uno::RuntimeException )
{
    // #FIXME: if the selection is an entire paragraph, it's replaced
    // by the new paragraph
    sal_Bool isCollapsed = mxTextViewCursor->isCollapsed();
    InsertParagraph();
    if( isCollapsed )
        mxTextViewCursor->collapseToStart();
}

void SAL_CALL
SwVbaSelection::InsertParagraph() throw ( uno::RuntimeException )
{
    // #FIME: the selection should include the new paragraph.
    getRange()->InsertParagraph();
}

void SAL_CALL
SwVbaSelection::InsertParagraphBefore() throw ( uno::RuntimeException )
{
    getRange()->InsertParagraphBefore();
}

void SAL_CALL
SwVbaSelection::InsertParagraphAfter() throw ( uno::RuntimeException )
{
    getRange()->InsertParagraphAfter();
}

uno::Reference< word::XParagraphFormat > SAL_CALL
SwVbaSelection::getParagraphFormat() throw ( uno::RuntimeException )
{
    return getRange()->getParagraphFormat();
}

void SAL_CALL
SwVbaSelection::setParagraphFormat( const uno::Reference< word::XParagraphFormat >& rParagraphFormat ) throw ( uno::RuntimeException )
{
    return getRange()->setParagraphFormat( rParagraphFormat );
}

uno::Reference< word::XFind > SAL_CALL
SwVbaSelection::getFind() throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextRange > xTextRange = GetSelectedRange();
    return uno::Reference< word::XFind >( new SwVbaFind( this, mxContext, mxModel, xTextRange ) );
}

uno::Any SAL_CALL
SwVbaSelection::getStyle() throw ( uno::RuntimeException )
{
    return getRange()->getStyle();
}

void SAL_CALL
SwVbaSelection::setStyle( const uno::Any& rStyle ) throw ( uno::RuntimeException )
{
    uno::Reference< beans::XPropertySet > xParaProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
    return SwVbaStyle::setStyle( xParaProps, rStyle );
}

uno::Reference< word::XFont > SAL_CALL
SwVbaSelection::getFont() throw ( uno::RuntimeException )
{
    return getRange()->getFont();
}

void SAL_CALL
SwVbaSelection::TypeBackspace() throw ( uno::RuntimeException )
{
    rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:SwBackspace"));
    dispatchRequests( mxModel,url );
}

uno::Reference< word::XRange > SAL_CALL SwVbaSelection::GoTo( const uno::Any& _what, const uno::Any& _which, const uno::Any& _count, const uno::Any& _name ) throw (uno::RuntimeException)
{
    sal_Int32 nWhat = 0;
    if( ( _what >>= nWhat ) != sal_True )
         DebugHelper::exception(SbERR_BAD_ARGUMENT, rtl::OUString());
    switch( nWhat )
    {
        case word::WdGoToItem::wdGoToBookmark:
        {
            rtl::OUString sName;
            uno::Reference< word::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
            uno::Reference< word::XBookmark > xBookmark( xApplication->getActiveDocument()->Bookmarks(_name), uno::UNO_QUERY_THROW );
            xBookmark->Select();
            break;
        }
        case word::WdGoToItem::wdGoToPage:
        {
            uno::Reference< text::XPageCursor > xPageCursor( mxTextViewCursor, uno::UNO_QUERY_THROW );
            sal_Int32 nCurrPage = xPageCursor->getPage();
            sal_Int32 nLastPage = word::getPageCount( mxModel );
            sal_Int32 nCount = 0;
            if( _count.hasValue() )
                _count >>= nCount;
            sal_Int32 nWhich = 0;
            if( _which.hasValue() )
                _which >>= nWhich;
            sal_Int32 nPage = 0;
            switch( nWhich )
            {
               case word::WdGoToDirection::wdGoToLast:
               {
                   nPage = nLastPage;
                   break;
               }
               case word::WdGoToDirection::wdGoToNext:
               {
                   if( nCount !=0 )
                       nPage = nCurrPage + nCount;
                   else
                       nPage = nCurrPage + 1;
                   break;
               }
               case word::WdGoToDirection::wdGoToPrevious:
               {
                   if( nCount !=0 )
                       nPage = nCurrPage - nCount;
                   else
                       nPage = nCurrPage - 1;
                   break;
               }
               default:
               {
                   nPage = nCount;
               }
            }
            if( _name.hasValue() )
            {
                rtl::OUString sName;
                _name >>= sName;
                sal_Int32 nName = sName.toInt32();
                if( nName !=0 )
                    nPage = nName;
            }
            if( nPage <= 0 )
               nPage = 1;
            if( nPage > nLastPage )
               nPage = nLastPage;
            xPageCursor->jumpToPage( ( sal_Int16 )( nPage ) );
            break;
        }
        case word::WdGoToItem::wdGoToSection:
        {
            uno::Reference< text::XPageCursor > xPageCursor( mxTextViewCursor, uno::UNO_QUERY_THROW );
            sal_Int32 nCount = 0;
            if( _count.hasValue() )
                _count >>= nCount;
            sal_Int32 nWhich = 0;
            if( _which.hasValue() )
                _which >>= nWhich;
            sal_Int32 nPage = 0;
            switch( nWhich )
            {
               case word::WdGoToDirection::wdGoToAbsolute:
               {
                    // currently only support this type
                    if( nCount == 1 )
                        nPage = 1;
                    break;
               }
               default:
               {
                    nPage = 0;
               }
            }
            if( nPage != 0 )
                xPageCursor->jumpToPage( ( sal_Int16 )( nPage ) );
            else
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
            break;
        }
        default:
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
    }
    return getRange();
}

::sal_Int32 SAL_CALL SwVbaSelection::getLanguageID() throw (uno::RuntimeException)
{
    return getRange()->getLanguageID();
}

void SAL_CALL SwVbaSelection::setLanguageID( ::sal_Int32 _languageid ) throw (uno::RuntimeException)
{
    getRange()->setLanguageID( _languageid );
}

uno::Any SAL_CALL SwVbaSelection::Information( sal_Int32 _type ) throw (uno::RuntimeException)
{
    uno::Any result;
    switch( _type )
    {
        case word::WdInformation::wdActiveEndPageNumber:
        {
            result = uno::makeAny( SwVbaInformationHelper::handleWdActiveEndPageNumber( mxTextViewCursor ) );
            break;
        }
        case word::WdInformation::wdNumberOfPagesInDocument:
        {
            result = uno::makeAny( SwVbaInformationHelper::handleWdNumberOfPagesInDocument( mxModel ) );
            break;
        }
        case word::WdInformation::wdVerticalPositionRelativeToPage:
        {
            result = uno::makeAny( SwVbaInformationHelper::handleWdVerticalPositionRelativeToPage( mxModel, mxTextViewCursor ) );
            break;
        }
        case word::WdInformation::wdWithInTable:
        {
            uno::Reference< beans::XPropertySet > xCursorProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
            uno::Reference< text::XTextTable > xTextTable;
            xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextTable") ) ) >>= xTextTable;
            if( xTextTable.is() )
                result = uno::makeAny( sal_True );
            else
                result = uno::makeAny( sal_False );
            break;
        }
        case word::WdInformation::wdHeaderFooterType:
        {
            uno::Reference< word::XView > xView( new SwVbaView( this, mxContext, mxModel ) );
            sal_Int32 nView = xView->getSeekView();
            sal_Int32 nHeaderFooterType = 0;
            switch( nView )
            {
                case word::WdSeekView::wdSeekMainDocument:
                {
                    nHeaderFooterType = -1; // not in a header or footer
                    break;
                }
                case word::WdSeekView::wdSeekEvenPagesHeader:
                {
                    nHeaderFooterType = 0; // even page header
                    break;
                }
                case word::WdSeekView::wdSeekPrimaryHeader:
                {
                    nHeaderFooterType = 1; // odd page header
                    break;
                }
                case word::WdSeekView::wdSeekEvenPagesFooter:
                {
                    nHeaderFooterType = 2; // even page footer
                    break;
                }
                case word::WdSeekView::wdSeekPrimaryFooter:
                {
                    nHeaderFooterType = 3; // odd page footer
                    break;
                }
                case word::WdSeekView::wdSeekFirstPageHeader:
                case word::WdSeekView::wdSeekFirstPageFooter:
                {
                    uno::Reference< beans::XPropertySet > xCursorProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
                    rtl::OUString aPageStyleName;
                    xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageStyleName"))) >>= aPageStyleName;
                    sal_Bool bFirstPage = sal_False;
                    if ( aPageStyleName == "First Page" )
                        bFirstPage = sal_True;
                    if( nView == word::WdSeekView::wdSeekFirstPageHeader )
                    {
                        if( bFirstPage )
                            nHeaderFooterType = 4;
                        else
                            nHeaderFooterType = 1;
                    }
                    else
                    {
                        if( bFirstPage )
                            nHeaderFooterType = 5;
                        else
                            nHeaderFooterType = 3;
                    }
                    break;
                }
                default:
                {
                    nHeaderFooterType = -1;
                }
            }
            result = uno::makeAny( nHeaderFooterType );
            break;
        }
        default:
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
    }
    return result;
}

void SAL_CALL SwVbaSelection::InsertBreak( const uno::Any& _breakType ) throw (uno::RuntimeException)
{
    getRange()->InsertBreak( _breakType );
}

uno::Any SAL_CALL
SwVbaSelection::Tables( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    // Hacky implementation due to missing api ( and lack of knowledge )
    // we can only support a selection that is a single table
    if ( !aIndex.hasValue() ) // currently we can't support multiple tables in a selection
       throw uno::RuntimeException();

    sal_Int32 nIndex = 0;
    aIndex >>= nIndex;

    uno::Any aRet;

    if ( nIndex != 1 )
       throw uno::RuntimeException();

    uno::Reference< beans::XPropertySet > xCursorProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextTable > xTextTable;
    xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextTable") ) ) >>= xTextTable;
    if( xTextTable.is() )
    {
            uno::Reference< css::text::XTextDocument > xTextDoc( mxModel, uno::UNO_QUERY_THROW );
            uno::Reference< word::XTable > xVBATbl = new SwVbaTable( mxParent, mxContext, xTextDoc, xTextTable );
            aRet <<= xVBATbl;
            return aRet;
    }

    // if the current selection is a XTextTableCursor and the index is 1 then we can service this request, otherwise we just have to throw
    uno::Reference< text::XTextTableCursor > xTextTableCursor( mxModel->getCurrentSelection(), uno::UNO_QUERY );

    if ( !xTextTableCursor.is() )
       throw uno::RuntimeException();

    SwXTextTableCursor* pTTCursor = dynamic_cast< SwXTextTableCursor* >( xTextTableCursor.get() );
    if ( pTTCursor )
    {
        SwFrmFmt* pFmt = pTTCursor->GetFrmFmt();
        if ( pFmt )
        {
            uno::Reference< text::XTextTable > xTbl = SwXTextTables::GetObject(*pFmt);
            uno::Reference< css::text::XTextDocument > xTextDoc( mxModel, uno::UNO_QUERY_THROW );
            uno::Reference< word::XTable > xVBATbl = new SwVbaTable( mxParent, mxContext, xTextDoc, xTbl );
            aRet <<= xVBATbl;
        }
    }
    return aRet;

}

uno::Any SAL_CALL
SwVbaSelection::Fields( const uno::Any& index ) throw (uno::RuntimeException)
{
    uno::Reference< XCollection > xCol( new SwVbaFields( mxParent, mxContext, mxModel ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

uno::Reference< word::XHeaderFooter > SAL_CALL
SwVbaSelection::getHeaderFooter() throw ( uno::RuntimeException )
{
    if( HeaderFooterHelper::isHeaderFooter( mxModel ) )
    {
        uno::Reference< beans::XPropertySet > xPageStyleProps( word::getCurrentPageStyle( mxModel ), uno::UNO_QUERY_THROW );
        sal_Int32 nIndex = word::WdHeaderFooterIndex::wdHeaderFooterPrimary;
        sal_Bool isHeader = HeaderFooterHelper::isHeader( mxModel );
        if( HeaderFooterHelper::isEvenPagesHeader( mxModel ) || HeaderFooterHelper::isEvenPagesFooter( mxModel ) )
            nIndex = word::WdHeaderFooterIndex::wdHeaderFooterEvenPages;
        else if( HeaderFooterHelper::isFirstPageHeader( mxModel ) || HeaderFooterHelper::isFirstPageFooter( mxModel ) )
            nIndex = word::WdHeaderFooterIndex::wdHeaderFooterFirstPage;

        return uno::Reference< word::XHeaderFooter >( new SwVbaHeaderFooter( this, mxContext, mxModel, xPageStyleProps, isHeader, nIndex ) );

    }
    return uno::Reference< word::XHeaderFooter >();
}

uno::Any SAL_CALL
SwVbaSelection::ShapeRange( ) throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShapes > xShapes( mxModel->getCurrentSelection(), uno::UNO_QUERY );
    if ( !xShapes.is() )
    {
        uno::Reference< drawing::XShape > xShape( mxModel->getCurrentSelection(), uno::UNO_QUERY_THROW );
        xShapes.set( drawing::ShapeCollection::create(mxContext) );
        xShapes->add( xShape );
    }

    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference< container::XIndexAccess > xShapesAccess( xShapes, uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< msforms::XShapeRange >( new ScVbaShapeRange( this, mxContext, xShapesAccess, xDrawPage, mxModel ) ) );
}

::sal_Int32 SAL_CALL SwVbaSelection::getStart() throw (uno::RuntimeException)
{
    return getRange()->getStart();
}

void SAL_CALL SwVbaSelection::setStart( ::sal_Int32 _start ) throw (uno::RuntimeException)
{
    getRange()->setStart( _start );
}
::sal_Int32 SAL_CALL SwVbaSelection::getEnd() throw (uno::RuntimeException)
{
    return getRange()->getEnd();
}

void SAL_CALL SwVbaSelection::setEnd( ::sal_Int32 _end ) throw (uno::RuntimeException)
{
    getRange()->setEnd( _end );
}

void SAL_CALL SwVbaSelection::SelectRow() throw (uno::RuntimeException)
{
    uno::Reference< word::XRows > xRows( Rows( uno::Any() ), uno::UNO_QUERY_THROW );
    xRows->Select();
}

void SAL_CALL SwVbaSelection::SelectColumn() throw (uno::RuntimeException)
{
    uno::Reference< word::XColumns > xColumns( Columns( uno::Any() ), uno::UNO_QUERY_THROW );
    xColumns->Select();
}

uno::Any SAL_CALL SwVbaSelection::Rows( const uno::Any& index ) throw (uno::RuntimeException)
{
    rtl::OUString sTLName;
    rtl::OUString sBRName;
    GetSelectedCellRange( sTLName, sBRName );

    sal_Int32 nStartRow = 0;
    sal_Int32 nEndRow = 0;
    uno::Reference< text::XTextTable > xTextTable = GetXTextTable();
    SwVbaTableHelper aTableHelper( xTextTable );
    nStartRow = aTableHelper.getTabRowIndex( sTLName );
    if( !sBRName.isEmpty() )
    {
        nEndRow = aTableHelper.getTabRowIndex( sBRName );
    }
    else
    {
        nEndRow = nStartRow;
    }

    uno::Reference< XCollection > xCol( new SwVbaRows( this, mxContext, xTextTable, xTextTable->getRows(), nStartRow, nEndRow ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

uno::Any SAL_CALL SwVbaSelection::Columns( const uno::Any& index ) throw (uno::RuntimeException)
{
    rtl::OUString sTLName;
    rtl::OUString sBRName;
    GetSelectedCellRange( sTLName, sBRName );
    sal_Int32 nStartColumn = 0;
    sal_Int32 nEndColumn = 0;

    uno::Reference< text::XTextTable > xTextTable = GetXTextTable();
    SwVbaTableHelper aTableHelper( xTextTable );
    nStartColumn = aTableHelper.getTabColIndex( sTLName );
    if( !sBRName.isEmpty() )
    {
        nEndColumn = aTableHelper.getTabColIndex( sBRName );
    }
    else
    {
        nEndColumn = nStartColumn;
    }

    uno::Reference< XCollection > xCol( new SwVbaColumns( this, mxContext, xTextTable, xTextTable->getColumns(), nStartColumn, nEndColumn ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

uno::Reference< text::XTextTable > SwVbaSelection::GetXTextTable() throw( uno::RuntimeException )
{
    uno::Reference< beans::XPropertySet > xCursorProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextTable > xTextTable;
    xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextTable") ) ) >>= xTextTable;
    return xTextTable;
}

sal_Bool SwVbaSelection::IsInTable() throw( uno::RuntimeException )
{
    uno::Reference< text::XTextTable > xTextTable = GetXTextTable();
    if( xTextTable.is() )
        return sal_True;
    return sal_False;
}

sal_Bool SwVbaSelection::HasSelection() throw( uno::RuntimeException )
{
    uno::Reference< text::XTextRange > xStart = mxTextViewCursor->getStart();
    uno::Reference< text::XTextRange > xEnd = mxTextViewCursor->getEnd();
    uno::Reference< text::XTextRangeCompare > xTRC( mxTextViewCursor->getText(), uno::UNO_QUERY_THROW );
    if( xTRC->compareRegionStarts( xStart, xEnd ) == 0 && xTRC->compareRegionEnds( xStart, xEnd ) == 0 )
        return sal_False;
    return sal_True;
}

void SwVbaSelection::GetSelectedCellRange( rtl::OUString& sTLName, rtl::OUString& sBRName ) throw( uno::RuntimeException )
{
    uno::Reference< beans::XPropertySet > xCursorProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextTable > xTextTable;
    xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextTable") ) ) >>= xTextTable;
    if( !xTextTable.is() )
        throw uno::RuntimeException( );

    uno::Reference< text::XTextTableCursor > xTextTableCursor( mxModel->getCurrentSelection(), uno::UNO_QUERY );
    if( xTextTableCursor.is() )
    {
        String sRange( xTextTableCursor->getRangeName() );
        if( comphelper::string::getTokenCount(sRange, ':') > 0 )
        {
            sTLName = sRange.GetToken(0, ':');
            sBRName = sRange.GetToken(1, ':');
        }
    }
    if( sTLName.isEmpty() )
    {
        uno::Reference< table::XCell > xCell;
        xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Cell") ) ) >>= xCell;
        if( !xCell.is() )
        {
            throw uno::RuntimeException( );
        }
        uno::Reference< beans::XPropertySet > xCellProps( xCell, uno::UNO_QUERY_THROW );
        xCellProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CellName") ) ) >>= sTLName;
    }
}

uno::Any SAL_CALL SwVbaSelection::Cells( const uno::Any& index ) throw (uno::RuntimeException)
{
    rtl::OUString sTLName;
    rtl::OUString sBRName;
    GetSelectedCellRange( sTLName, sBRName );
    sal_Int32 nLeft = 0;
    sal_Int32 nTop = 0;
    sal_Int32 nRight = 0;
    sal_Int32 nBottom = 0;

    uno::Reference< text::XTextTable > xTextTable = GetXTextTable();
    SwVbaTableHelper aTableHelper( xTextTable );
    nLeft = aTableHelper.getTabColIndex( sTLName );
    nTop = aTableHelper.getTabRowIndex( sTLName );
    if( !sBRName.isEmpty() )
    {
        nRight = aTableHelper.getTabColIndex( sBRName );
        nBottom = aTableHelper.getTabRowIndex( sBRName );
    }
    else
    {
        nRight = nLeft;
        nBottom = nTop;
    }

    uno::Reference< XCollection > xCol( new SwVbaCells( this, mxContext, xTextTable, nLeft, nTop, nRight, nBottom ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

void SAL_CALL SwVbaSelection::Copy(  ) throw (uno::RuntimeException)
{
    rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Copy"));
    dispatchRequests( mxModel,url );
}

void SAL_CALL SwVbaSelection::CopyAsPicture(  ) throw (uno::RuntimeException)
{
    // seems not support in Writer
    Copy();
}

void SAL_CALL SwVbaSelection::Paste(  ) throw (uno::RuntimeException)
{
    rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Paste"));
    dispatchRequests( mxModel,url );
}

void SAL_CALL SwVbaSelection::Collapse( const uno::Any& Direction ) throw (uno::RuntimeException)
{
    if( word::gotoSelectedObjectAnchor( mxModel ) )
        return;

    sal_Int32 nDirection = word::WdCollapseDirection::wdCollapseStart;
    if( Direction.hasValue() )
        Direction >>= nDirection;

    uno::Reference< text::XTextViewCursor > xTextViewCursor = word::getXTextViewCursor( mxModel );
    if( nDirection == word::WdCollapseDirection::wdCollapseStart )
    {
        // it is inaccurate if current seleciton is multipul cells, so it needs to go to start
        uno::Reference< text::XTextRange > xTextRange = mxTextViewCursor->getStart();
        xTextViewCursor->gotoRange( xTextRange, sal_False );
        xTextViewCursor->collapseToStart();
    }
    else if( nDirection == word::WdCollapseDirection::wdCollapseEnd )
    {
        uno::Reference< text::XTextRange > xTextRange = mxTextViewCursor->getEnd();
        xTextViewCursor->gotoRange( xTextRange, sal_False );
        xTextViewCursor->collapseToEnd();
    }
    else
    {
        throw uno::RuntimeException();
    }
}

void SAL_CALL SwVbaSelection::WholeStory(  ) throw (uno::RuntimeException)
{
    uno::Reference< text::XText > xText = word::getCurrentXText( mxModel );
    // FIXME: for i#7747,if the first line is a table, it fails to select all the contents in the story.
    // Temporary solution, insert an empty line before the table so that it could select all the contents.
    uno::Reference< container::XEnumerationAccess > xParaAccess( xText, uno::UNO_QUERY_THROW );
    uno::Reference< container::XEnumeration> xParaEnum = xParaAccess->createEnumeration();
    if( xParaEnum->hasMoreElements() )
    {
        uno::Reference< text::XTextTable > xTextTable( xParaEnum->nextElement(), uno::UNO_QUERY );
        if( xTextTable.is() )
        {
            // insert an empty line
            uno::Reference< text::XTextRange > xFirstCellRange = word::getFirstObjectPosition( xText );
            mxTextViewCursor->gotoRange( xFirstCellRange, sal_False );
            rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertPara"));
            dispatchRequests( mxModel,url );
        }
    }
    uno::Reference< text::XTextRange > xStart = xText->getStart();
    uno::Reference< text::XTextRange > xEnd = xText->getEnd();
    mxTextViewCursor->gotoRange( xStart, sal_False );
    mxTextViewCursor->gotoRange( xEnd, sal_True );
}

::sal_Bool SAL_CALL SwVbaSelection::InRange( const uno::Reference< ::ooo::vba::word::XRange >& Range ) throw (uno::RuntimeException)
{
    return getRange()->InRange( Range );
}

void SAL_CALL SwVbaSelection::SplitTable(  ) throw (uno::RuntimeException)
{
    if( !IsInTable() )
        throw uno::RuntimeException();

    SwDocShell* pDocShell = word::getDocShell( mxModel );
    if( pDocShell )
    {
        SwFEShell* pFEShell = pDocShell->GetFEShell();
        if( pFEShell )
        {
            pFEShell->SplitTable( HEADLINE_CNTNTCOPY );
        }
    }
}

uno::Any SAL_CALL
SwVbaSelection::Paragraphs( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    // Hacky implementation due to missing api ( and lack of knowledge )
    // we can only support a selection that is a single paragraph
    if ( !aIndex.hasValue() ) // currently we can't support multiple paragraphs in a selection
       throw uno::RuntimeException();

    sal_Int32 nIndex = 0;
    aIndex >>= nIndex;

    uno::Any aRet;

    if ( nIndex != 1 )
       throw uno::RuntimeException();

    uno::Reference< text::XTextRange > xTextRange = mxTextViewCursor->getStart();
    uno::Reference< text::XText > xText = xTextRange->getText();
    uno::Reference< text::XParagraphCursor > xParaCursor( xText->createTextCursor(), uno::UNO_QUERY_THROW );
    xParaCursor->gotoStartOfParagraph( sal_False );
    xParaCursor->gotoStartOfParagraph( sal_True );

    uno::Reference< text::XTextDocument > xTextDoc( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xParaRange( xParaCursor, uno::UNO_QUERY_THROW );
    uno::Reference< word::XParagraph > xParagraph = new SwVbaParagraph( mxParent, mxContext, xTextDoc, xParaRange );

    aRet <<= xParagraph;
    return aRet;
}

rtl::OUString
SwVbaSelection::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SwVbaSelection"));
}

uno::Sequence< rtl::OUString >
SwVbaSelection::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Selection" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
