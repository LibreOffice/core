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
#include <basic/sberrors.hxx>
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

SwVbaSelection::SwVbaSelection( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< frame::XModel >& rModel ) : SwVbaSelection_BASE( rParent, rContext ), mxModel( rModel )
{
    mxTextViewCursor = word::getXTextViewCursor( mxModel );
}

SwVbaSelection::~SwVbaSelection()
{
}

uno::Reference< text::XTextRange > SwVbaSelection::GetSelectedRange()
{
    uno::Reference< text::XTextRange > xTextRange;
    uno::Reference< lang::XServiceInfo > xServiceInfo( mxModel->getCurrentSelection(), uno::UNO_QUERY_THROW );
    if( !xServiceInfo->supportsService("com.sun.star.text.TextRanges") )
    {
        throw uno::RuntimeException("Not implemented" );
    }

    uno::Reference< container::XIndexAccess > xTextRanges( xServiceInfo, uno::UNO_QUERY_THROW );
    if( xTextRanges->getCount() > 0 )
    {
        // if there are multiple selection, just return the last selected Range.
        xTextRange.set( xTextRanges->getByIndex( xTextRanges->getCount()-1 ), uno::UNO_QUERY_THROW );
    }

    return xTextRange;
}

uno::Reference< word::XRange > SAL_CALL
SwVbaSelection::getRange()
{
    uno::Reference< text::XTextRange > xTextRange = GetSelectedRange();
    uno::Reference< text::XTextDocument > xDocument( mxModel, uno::UNO_QUERY_THROW );
    return uno::Reference< word::XRange >( new SwVbaRange( this, mxContext, xDocument, xTextRange->getStart(), xTextRange->getEnd(), mxTextViewCursor->getText() ) );
}

OUString SAL_CALL
SwVbaSelection::getText()
{
    return getRange()->getText();
}

void SAL_CALL
SwVbaSelection::setText( const OUString& rText )
{
    getRange()->setText( rText );
}

void SAL_CALL
SwVbaSelection::TypeText( const OUString& rText )
{
    // FIXME: handle the property Options.ReplaceSelection, the default value is true
    setText( rText );
}

void SAL_CALL
SwVbaSelection::HomeKey( const uno::Any& _unit, const uno::Any& _extend )
{
    sal_Int32 nUnit = word::WdUnits::wdLine;
    sal_Int32 nExtend = word::WdMovementType::wdMove;
    _unit >>= nUnit;
    _extend >>= nExtend;
    bool bExtend = nExtend == word::WdMovementType::wdExtend;

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
            throw uno::RuntimeException("Not implemented" );
        }
    }
}

void SAL_CALL
SwVbaSelection::EndKey( const uno::Any& _unit, const uno::Any& _extend )
{
    sal_Int32 nUnit = word::WdUnits::wdLine;
    sal_Int32 nExtend = word::WdMovementType::wdMove;
    _unit >>= nUnit;
    _extend >>= nExtend;
    bool bExtend = nExtend == word::WdMovementType::wdExtend;

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
            throw uno::RuntimeException("Not implemented" );
        }
    }
}

void SAL_CALL
SwVbaSelection::Delete( const uno::Any& _unit, const uno::Any& _count )
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
                mxTextViewCursor->goRight( nCount, true );
                break;
            }
            default:
            {
                throw uno::RuntimeException("Not implemented" );
            }
        }
    }
    OUString url = ".uno:Delete";
    dispatchRequests( mxModel,url );
}

void
SwVbaSelection::Move( const uno::Any& _unit, const uno::Any& _count, const uno::Any& _extend, word::E_DIRECTION eDirection )
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

    bool bExpand = nExtend != word::WdMovementType::wdMove;

    switch( nUnit )
    {
        case word::WdUnits::wdCell:
        {
            if(  nExtend == word::WdMovementType::wdExtend )
            {
                DebugHelper::basicexception(ERRCODE_BASIC_BAD_ARGUMENT, OUString());
                return;
            }
            NextCell( nCount, eDirection );
            break;
        }
        case word::WdUnits::wdLine:
        {
            if( eDirection == word::MOVE_LEFT || eDirection == word::MOVE_RIGHT )
            {
                throw uno::RuntimeException("Not implemented" );
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
                throw uno::RuntimeException("Not implemented" );
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
                    xCursorProps->getPropertyValue("TextTable") >>= xTextTable;
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
                    throw uno::RuntimeException("Not implemented" );
                }
                uno::Reference< text::XParagraphCursor > xParagraphCursor( xTextCursor, uno::UNO_QUERY_THROW );
                for( sal_Int32 i=0; i<nCount; i++ )
                {
                    if( ( eDirection == word::MOVE_UP ) && !xParagraphCursor->gotoPreviousParagraph( bExpand ) )
                        break;
                    else if( ( eDirection == word::MOVE_DOWN ) && !xParagraphCursor->gotoNextParagraph( bExpand ) )
                        break;
                }
            }
            else if( nUnit == word::WdUnits::wdWord )
            {
                if( eDirection == word::MOVE_UP || eDirection == word::MOVE_DOWN )
                {
                    throw uno::RuntimeException("Not implemented" );
                }
                uno::Reference< text::XWordCursor > xWordCursor( xTextCursor, uno::UNO_QUERY_THROW );
                for( sal_Int32 i=0; i<nCount; i++ )
                {
                    if( (eDirection == word::MOVE_LEFT ) && !xWordCursor->gotoPreviousWord( bExpand ) )
                        break;
                    else if( ( eDirection == word::MOVE_RIGHT ) && !xWordCursor->gotoNextWord( bExpand ) )
                        break;
                }
            }
            mxTextViewCursor->gotoRange( xTextCursor->getStart(), false );
            mxTextViewCursor->gotoRange( xTextCursor->getEnd(), true );
            break;
        }
        default:
        {
            throw uno::RuntimeException("Not implemented" );
        }
    }
}

void SwVbaSelection::NextCell(sal_Int32 nCount, word::E_DIRECTION eDirection)
{
    uno::Reference< beans::XPropertySet > xCursorProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextTable > xTextTable;
    uno::Reference< table::XCell > xCell;
    xCursorProps->getPropertyValue("TextTable") >>= xTextTable;
    xCursorProps->getPropertyValue("Cell") >>= xCell;
    if( !xTextTable.is() || !xCell.is() )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_BAD_ARGUMENT, OUString());
        return;
    }
    uno::Reference< beans::XPropertySet > xCellProps( xCell, uno::UNO_QUERY_THROW );
    OUString aCellName;
    xCellProps->getPropertyValue("CellName") >>= aCellName;
    uno::Reference< text::XTextTableCursor > xTextTableCursor = xTextTable->createCursorByCellName( aCellName );
    // move the table cursor
    switch( eDirection )
    {
        case word::MOVE_LEFT:
        {
            xTextTableCursor->goLeft( nCount, false );
            break;
        }
        case word::MOVE_RIGHT:
        {
            xTextTableCursor->goRight( nCount, false );
            break;
        }
        case word::MOVE_UP:
        {
            xTextTableCursor->goUp( nCount, false );
            break;
        }
        case word::MOVE_DOWN:
        {
            xTextTableCursor->goDown( nCount, false );
            break;
        }
        default:
        {
            DebugHelper::basicexception(ERRCODE_BASIC_BAD_ARGUMENT, OUString());
            return;
        }
    }
    // move the view cursor
    xCell = xTextTable->getCellByName( xTextTableCursor->getRangeName() );
    mxTextViewCursor->gotoRange( uno::Reference< text::XTextRange >( xCell, uno::UNO_QUERY_THROW ), false );
}

void SAL_CALL
SwVbaSelection::MoveRight(const uno::Any& _unit, const uno::Any& _count, const uno::Any& _extend)
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
SwVbaSelection::MoveLeft(const uno::Any& _unit, const uno::Any& _count, const uno::Any& _extend)
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
SwVbaSelection::MoveDown(const uno::Any& _unit, const uno::Any& _count, const uno::Any& _extend)
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
SwVbaSelection::MoveUp(const uno::Any& _unit, const uno::Any& _count, const uno::Any& _extend)
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
SwVbaSelection::TypeParagraph()
{
    // #FIXME: if the selection is an entire paragraph, it's replaced
    // by the new paragraph
    bool isCollapsed = mxTextViewCursor->isCollapsed();
    InsertParagraph();
    if( isCollapsed )
        mxTextViewCursor->collapseToStart();
}

void SAL_CALL
SwVbaSelection::InsertParagraph()
{
    // #FIXME: the selection should include the new paragraph.
    getRange()->InsertParagraph();
}

void SAL_CALL
SwVbaSelection::InsertParagraphBefore()
{
    getRange()->InsertParagraphBefore();
}

void SAL_CALL
SwVbaSelection::InsertParagraphAfter()
{
    getRange()->InsertParagraphAfter();
}

uno::Reference< word::XParagraphFormat > SAL_CALL
SwVbaSelection::getParagraphFormat()
{
    return getRange()->getParagraphFormat();
}

void SAL_CALL
SwVbaSelection::setParagraphFormat( const uno::Reference< word::XParagraphFormat >& rParagraphFormat )
{
    return getRange()->setParagraphFormat( rParagraphFormat );
}

uno::Reference< word::XFind > SAL_CALL
SwVbaSelection::getFind()
{
    uno::Reference< text::XTextRange > xTextRange = GetSelectedRange();
    return uno::Reference< word::XFind >( new SwVbaFind( this, mxContext, mxModel, xTextRange ) );
}

uno::Any SAL_CALL
SwVbaSelection::getStyle()
{
    return getRange()->getStyle();
}

void SAL_CALL
SwVbaSelection::setStyle( const uno::Any& rStyle )
{
    uno::Reference< beans::XPropertySet > xParaProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
    return SwVbaStyle::setStyle( xParaProps, rStyle );
}

uno::Reference< word::XFont > SAL_CALL
SwVbaSelection::getFont()
{
    return getRange()->getFont();
}

void SAL_CALL
SwVbaSelection::TypeBackspace()
{
    OUString url = ".uno:SwBackspace";
    dispatchRequests( mxModel,url );
}

uno::Reference< word::XRange > SAL_CALL SwVbaSelection::GoTo( const uno::Any& _what, const uno::Any& _which, const uno::Any& _count, const uno::Any& _name )
{
    sal_Int32 nWhat = 0;
    if( !( _what >>= nWhat ) )
         DebugHelper::basicexception(ERRCODE_BASIC_BAD_ARGUMENT, OUString());
    switch( nWhat )
    {
        case word::WdGoToItem::wdGoToBookmark:
        {
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
                OUString sName;
                _name >>= sName;
                sal_Int32 nName = sName.toInt32();
                if( nName !=0 )
                    nPage = nName;
            }
            if( nPage <= 0 )
               nPage = 1;
            if( nPage > nLastPage )
               nPage = nLastPage;
            xPageCursor->jumpToPage( ( sal_Int16 )nPage );
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
            if( nPage == 0 )
                throw uno::RuntimeException("Not implemented" );
            xPageCursor->jumpToPage( ( sal_Int16 )nPage );
            break;
        }
        default:
            throw uno::RuntimeException("Not implemented" );
    }
    return getRange();
}

::sal_Int32 SAL_CALL SwVbaSelection::getLanguageID()
{
    return getRange()->getLanguageID();
}

void SAL_CALL SwVbaSelection::setLanguageID( ::sal_Int32 _languageid )
{
    getRange()->setLanguageID( _languageid );
}

uno::Any SAL_CALL SwVbaSelection::Information( sal_Int32 _type )
{
    uno::Any result;
    switch( _type )
    {
        case word::WdInformation::wdActiveEndPageNumber:
        {
            result <<= SwVbaInformationHelper::handleWdActiveEndPageNumber( mxTextViewCursor );
            break;
        }
        case word::WdInformation::wdNumberOfPagesInDocument:
        {
            result <<= SwVbaInformationHelper::handleWdNumberOfPagesInDocument( mxModel );
            break;
        }
        case word::WdInformation::wdVerticalPositionRelativeToPage:
        {
            result <<= SwVbaInformationHelper::handleWdVerticalPositionRelativeToPage( mxModel, mxTextViewCursor );
            break;
        }
        case word::WdInformation::wdWithInTable:
        {
            uno::Reference< beans::XPropertySet > xCursorProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
            uno::Reference< text::XTextTable > xTextTable;
            xCursorProps->getPropertyValue("TextTable") >>= xTextTable;
            result <<= xTextTable.is();
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
                    OUString aPageStyleName;
                    xCursorProps->getPropertyValue("PageStyleName") >>= aPageStyleName;
                    bool bFirstPage = false;
                    if ( aPageStyleName == "First Page" )
                        bFirstPage = true;
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
            result <<= nHeaderFooterType;
            break;
        }
        default:
            throw uno::RuntimeException("Not implemented" );
    }
    return result;
}

void SAL_CALL SwVbaSelection::InsertBreak( const uno::Any& _breakType )
{
    getRange()->InsertBreak( _breakType );
}

uno::Any SAL_CALL
SwVbaSelection::Tables( const uno::Any& aIndex )
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
    xCursorProps->getPropertyValue("TextTable") >>= xTextTable;
    if( xTextTable.is() )
    {
            uno::Reference< css::text::XTextDocument > xTextDoc( mxModel, uno::UNO_QUERY_THROW );
            uno::Reference< word::XTable > xVBATable = new SwVbaTable( mxParent, mxContext, xTextDoc, xTextTable );
            aRet <<= xVBATable;
            return aRet;
    }

    // if the current selection is a XTextTableCursor and the index is 1 then we can service this request, otherwise we just have to throw
    uno::Reference< text::XTextTableCursor > xTextTableCursor( mxModel->getCurrentSelection(), uno::UNO_QUERY_THROW );
    SwXTextTableCursor* pTTCursor = dynamic_cast< SwXTextTableCursor* >( xTextTableCursor.get() );
    if ( pTTCursor )
    {
        SwFrameFormat* pFormat = pTTCursor->GetFrameFormat();
        if ( pFormat )
        {
            uno::Reference< text::XTextTable > xTable = SwXTextTables::GetObject(*pFormat);
            uno::Reference< css::text::XTextDocument > xTextDoc( mxModel, uno::UNO_QUERY_THROW );
            uno::Reference< word::XTable > xVBATable = new SwVbaTable( mxParent, mxContext, xTextDoc, xTable );
            aRet <<= xVBATable;
        }
    }
    return aRet;

}

uno::Any SAL_CALL
SwVbaSelection::Fields( const uno::Any& index )
{
    uno::Reference< XCollection > xCol( new SwVbaFields( mxParent, mxContext, mxModel ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

uno::Reference< word::XHeaderFooter > SAL_CALL
SwVbaSelection::getHeaderFooter()
{
    if( HeaderFooterHelper::isHeaderFooter( mxModel ) )
    {
        uno::Reference< beans::XPropertySet > xPageStyleProps( word::getCurrentPageStyle( mxModel ), uno::UNO_QUERY_THROW );
        sal_Int32 nIndex = word::WdHeaderFooterIndex::wdHeaderFooterPrimary;
        bool isHeader = HeaderFooterHelper::isHeader( mxModel );
        if( HeaderFooterHelper::isEvenPagesHeader( mxModel ) || HeaderFooterHelper::isEvenPagesFooter( mxModel ) )
            nIndex = word::WdHeaderFooterIndex::wdHeaderFooterEvenPages;
        else if( HeaderFooterHelper::isFirstPageHeader( mxModel ) || HeaderFooterHelper::isFirstPageFooter( mxModel ) )
            nIndex = word::WdHeaderFooterIndex::wdHeaderFooterFirstPage;

        return uno::Reference< word::XHeaderFooter >( new SwVbaHeaderFooter( this, mxContext, mxModel, xPageStyleProps, isHeader, nIndex ) );

    }
    return uno::Reference< word::XHeaderFooter >();
}

uno::Any SAL_CALL
SwVbaSelection::ShapeRange( )
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

::sal_Int32 SAL_CALL SwVbaSelection::getStart()
{
    return getRange()->getStart();
}

void SAL_CALL SwVbaSelection::setStart( ::sal_Int32 _start )
{
    getRange()->setStart( _start );
}
::sal_Int32 SAL_CALL SwVbaSelection::getEnd()
{
    return getRange()->getEnd();
}

void SAL_CALL SwVbaSelection::setEnd( ::sal_Int32 _end )
{
    getRange()->setEnd( _end );
}

void SAL_CALL SwVbaSelection::SelectRow()
{
    uno::Reference< word::XRows > xRows( Rows( uno::Any() ), uno::UNO_QUERY_THROW );
    xRows->Select();
}

void SAL_CALL SwVbaSelection::SelectColumn()
{
    uno::Reference< word::XColumns > xColumns( Columns( uno::Any() ), uno::UNO_QUERY_THROW );
    xColumns->Select();
}

uno::Any SAL_CALL SwVbaSelection::Rows( const uno::Any& index )
{
    OUString sTLName;
    OUString sBRName;
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

uno::Any SAL_CALL SwVbaSelection::Columns( const uno::Any& index )
{
    OUString sTLName;
    OUString sBRName;
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

uno::Reference< text::XTextTable > SwVbaSelection::GetXTextTable()
{
    uno::Reference< beans::XPropertySet > xCursorProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextTable > xTextTable;
    xCursorProps->getPropertyValue("TextTable") >>= xTextTable;
    return xTextTable;
}

bool SwVbaSelection::IsInTable()
{
    uno::Reference< text::XTextTable > xTextTable = GetXTextTable();
    return xTextTable.is();
}

bool SwVbaSelection::HasSelection()
{
    uno::Reference< text::XTextRange > xStart = mxTextViewCursor->getStart();
    uno::Reference< text::XTextRange > xEnd = mxTextViewCursor->getEnd();
    uno::Reference< text::XTextRangeCompare > xTRC( mxTextViewCursor->getText(), uno::UNO_QUERY_THROW );
    return xTRC->compareRegionStarts( xStart, xEnd ) != 0 || xTRC->compareRegionEnds( xStart, xEnd ) != 0;
}

void SwVbaSelection::GetSelectedCellRange( OUString& sTLName, OUString& sBRName )
{
    uno::Reference< beans::XPropertySet > xCursorProps( mxTextViewCursor, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextTable > xTextTable;
    xCursorProps->getPropertyValue("TextTable") >>= xTextTable;
    if( !xTextTable.is() )
        throw uno::RuntimeException( );

    uno::Reference< text::XTextTableCursor > xTextTableCursor( mxModel->getCurrentSelection(), uno::UNO_QUERY );
    if( xTextTableCursor.is() )
    {
        OUString sRange( xTextTableCursor->getRangeName() );
        if( comphelper::string::getTokenCount(sRange, ':') > 0 )
        {
            sTLName = sRange.getToken(0, ':');
            sBRName = sRange.getToken(1, ':');
        }
    }
    if( sTLName.isEmpty() )
    {
        uno::Reference< table::XCell > xCell;
        xCursorProps->getPropertyValue("Cell") >>= xCell;
        if( !xCell.is() )
        {
            throw uno::RuntimeException( );
        }
        uno::Reference< beans::XPropertySet > xCellProps( xCell, uno::UNO_QUERY_THROW );
        xCellProps->getPropertyValue("CellName") >>= sTLName;
    }
}

uno::Any SAL_CALL SwVbaSelection::Cells( const uno::Any& index )
{
    OUString sTLName;
    OUString sBRName;
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

void SAL_CALL SwVbaSelection::Copy(  )
{
    OUString url = ".uno:Copy";
    dispatchRequests( mxModel,url );
}

void SAL_CALL SwVbaSelection::CopyAsPicture(  )
{
    // seems not support in Writer
    Copy();
}

void SAL_CALL SwVbaSelection::Paste(  )
{
    OUString url = ".uno:Paste";
    dispatchRequests( mxModel,url );
}

void SAL_CALL SwVbaSelection::Collapse( const uno::Any& Direction )
{
    if( word::gotoSelectedObjectAnchor( mxModel ) )
        return;

    sal_Int32 nDirection = word::WdCollapseDirection::wdCollapseStart;
    if( Direction.hasValue() )
        Direction >>= nDirection;

    uno::Reference< text::XTextViewCursor > xTextViewCursor = word::getXTextViewCursor( mxModel );
    if( nDirection == word::WdCollapseDirection::wdCollapseStart )
    {
        // it is inaccurate if current selection is multiple cells, so it needs to go to start
        uno::Reference< text::XTextRange > xTextRange = mxTextViewCursor->getStart();
        xTextViewCursor->gotoRange( xTextRange, false );
        xTextViewCursor->collapseToStart();
    }
    else if( nDirection == word::WdCollapseDirection::wdCollapseEnd )
    {
        uno::Reference< text::XTextRange > xTextRange = mxTextViewCursor->getEnd();
        xTextViewCursor->gotoRange( xTextRange, false );
        xTextViewCursor->collapseToEnd();
    }
    else
    {
        throw uno::RuntimeException();
    }
}

void SAL_CALL SwVbaSelection::WholeStory(  )
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
            mxTextViewCursor->gotoRange( xFirstCellRange, false );
            OUString url = ".uno:InsertPara";
            dispatchRequests( mxModel,url );
        }
    }
    uno::Reference< text::XTextRange > xStart = xText->getStart();
    uno::Reference< text::XTextRange > xEnd = xText->getEnd();
    mxTextViewCursor->gotoRange( xStart, false );
    mxTextViewCursor->gotoRange( xEnd, true );
}

sal_Bool SAL_CALL SwVbaSelection::InRange( const uno::Reference< ::ooo::vba::word::XRange >& Range )
{
    return getRange()->InRange( Range );
}

void SAL_CALL SwVbaSelection::SplitTable()
{
    if( !IsInTable() )
        throw uno::RuntimeException();

    SwDocShell* pDocShell = word::getDocShell( mxModel );
    if( pDocShell )
    {
        SwFEShell* pFEShell = pDocShell->GetFEShell();
        if( pFEShell )
        {
            pFEShell->SplitTable( SplitTable_HeadlineOption::ContentCopy );
        }
    }
}

uno::Any SAL_CALL
SwVbaSelection::Paragraphs( const uno::Any& aIndex )
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
    xParaCursor->gotoStartOfParagraph( false );
    xParaCursor->gotoStartOfParagraph( true );

    uno::Reference< text::XTextDocument > xTextDoc( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xParaRange( xParaCursor, uno::UNO_QUERY_THROW );
    uno::Reference< word::XParagraph > xParagraph = new SwVbaParagraph( mxParent, mxContext, xTextDoc, xParaRange );

    aRet <<= xParagraph;
    return aRet;
}

OUString
SwVbaSelection::getServiceImplName()
{
    return OUString("SwVbaSelection");
}

uno::Sequence< OUString >
SwVbaSelection::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.Selection";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
