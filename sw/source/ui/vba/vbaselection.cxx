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
#include "wordvbahelper.hxx"
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTableCursor.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <ooo/vba/word/WdUnits.hpp>
#include <ooo/vba/word/WdMovementType.hpp>
#include <ooo/vba/word/WdGoToItem.hpp>
#include <ooo/vba/word/WdGoToDirection.hpp>
#include <ooo/vba/word/XBookmark.hpp>
#include <ooo/vba/word/XApplication.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include "unotbl.hxx"
#include "unocoll.hxx"
#include "vbatable.hxx"
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XViewCursor.hpp>
#include <ooo/vba/word/WdInformation.hpp>
#include <ooo/vba/word/WdHeaderFooterIndex.hpp>
#include "vbainformationhelper.hxx"
#include "vbafield.hxx"
#include "vbaheaderfooter.hxx"
#include "vbaheaderfooterhelper.hxx"
#include <vbahelper/vbashaperange.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

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

    switch( nUnit )
    {
        case word::WdUnits::wdStory:
        {
            // go to the begin of the document
            rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GoToStartOfDoc"));
            dispatchRequests( mxModel,url );
            // If something is selected, it needs to go twice
            dispatchRequests( mxModel,url );
            break;
        }
        case word::WdUnits::wdLine:
        {
            // go to the begin of the Line
            rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GoToStartOfLine"));
            dispatchRequests( mxModel,url );
            break;
        }
        default:
        {
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
            break;
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

    switch( nUnit )
    {
        case word::WdUnits::wdStory:
        {
            // go to the end of the document
            rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GoToEndOfDoc"));
            dispatchRequests( mxModel,url );
            // If something is selected, it needs to go twice
            dispatchRequests( mxModel,url );
            break;
        }
        case word::WdUnits::wdLine:
        {
            // go to the end of the Line
            rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GoToEndOfLine"));
            dispatchRequests( mxModel,url );
            break;
        }
        default:
        {
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
            break;
        }
    }
}

void SAL_CALL
SwVbaSelection::Delete( const uno::Any& /*_unit*/, const uno::Any& /*_count*/ ) throw ( uno::RuntimeException )
{
    // FIXME: handle the arguments: _unit and _count
    rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Delete"));
    dispatchRequests( mxModel,url );
}

void SwVbaSelection::NextCell( sal_Int32 nCount, E_DIRECTION eDirection ) throw ( uno::RuntimeException )
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
        case MOVE_LEFT:
        {
            xTextTableCursor->goLeft( nCount, sal_False );
            break;
        }
        case MOVE_RIGHT:
        {
            xTextTableCursor->goRight( nCount, sal_False );
            break;
        }
        case MOVE_UP:
        {
            xTextTableCursor->goUp( nCount, sal_False );
            break;
        }
        case MOVE_DOWN:
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

    if( nCount < 0 )
    {
        // TODO: call MoveLeft;
        MoveLeft( _unit, uno::makeAny( -nCount ), _extend );
        return;
    }

    switch( nUnit )
    {
        case word::WdUnits::wdCell:
        {
            if(  nExtend == word::WdMovementType::wdExtend )
            {
                DebugHelper::exception(SbERR_BAD_ARGUMENT, rtl::OUString());
                return;
            }
            NextCell( nCount, MOVE_RIGHT );
            break;
        }
        default:
        {
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
            break;
        }
    }

}

void SAL_CALL
SwVbaSelection::MoveLeft( const uno::Any& _unit, const uno::Any& _count, const uno::Any& _extend ) throw ( uno::RuntimeException )
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

    if( nCount < 0 )
    {
        MoveRight( _unit, uno::makeAny( -nCount ), _extend );
        return;
    }

    switch( nUnit )
    {
        case word::WdUnits::wdCell:
        {
            if(  nExtend == word::WdMovementType::wdExtend )
            {
                DebugHelper::exception(SbERR_BAD_ARGUMENT, rtl::OUString());
                return;
            }
            NextCell( nCount, MOVE_LEFT );
            break;
        }
        default:
        {
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
            break;
        }
    }

}

void SAL_CALL
SwVbaSelection::MoveDown( const uno::Any& _unit, const uno::Any& _count, const uno::Any& _extend ) throw ( uno::RuntimeException )
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

    if( nCount < 0 )
    {
        // TODO: call MoveLeft;
        //MoveUp( _unit, uno::makeAny( -nCount ), _extend );
        return;
    }

    switch( nUnit )
    {
        case word::WdUnits::wdLine:
        {
            uno::Reference< view::XViewCursor > xViewCursor( mxTextViewCursor, uno::UNO_QUERY_THROW );
            sal_Bool bExpand = ( nExtend == word::WdMovementType::wdMove ) ? sal_False : sal_True;
            xViewCursor->goDown( nCount, bExpand );
            break;
        }
        default:
        {
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
            break;
        }
    }

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

uno::Reference< word::XStyle > SAL_CALL
SwVbaSelection::getStyle() throw ( uno::RuntimeException )
{
    return getRange()->getStyle();
}

void SAL_CALL
SwVbaSelection::setStyle( const uno::Reference< word::XStyle >& rStyle ) throw ( uno::RuntimeException )
{
    return getRange()->setStyle( rStyle );
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
            //return uno::Reference< word::XRange >( xBookmark->Range(), uno::UNO_QUERY_THROW );
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
                    nPage = nCurrPage + 1;
                    break;
                }
                case word::WdGoToDirection::wdGoToPrevious:
                {
                    nPage = nCurrPage - 1;
                    break;
                }
                default:
                {
                    nPage = nCount;
                }
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
            // TODO: implement Section object
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
    //uno::Reference< view::XSelectionSupplier > xSel( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
    //uno::Any aSelectedObject = xSel->getSelection();
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
        default:
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
    }
    // This method fails to restore the previouse selection
    //xSel->select( aSelectedObject );
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
    // if the current selection is a XTextTableCursor and the index is 1 then we can service this request, otherwise we just have to throw
    uno::Reference< text::XTextTableCursor > xTextTableCursor( mxModel->getCurrentSelection(), uno::UNO_QUERY );

    if ( !xTextTableCursor.is() )
       throw uno::RuntimeException();

    sal_Int32 nIndex = 0;
    aIndex >>= nIndex;

    uno::Any aRet;

    if ( nIndex != 1 )
       throw uno::RuntimeException();
    SwXTextTableCursor* pTTCursor = dynamic_cast< SwXTextTableCursor* >( xTextTableCursor.get() );
    if ( pTTCursor )
    {
        SwFrmFmt* pFmt = pTTCursor->GetFrmFmt();
        rtl::OUString sTableName;
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
    uno::Reference< text::XText > xCurrentText = word::getXTextViewCursor( mxModel )->getText();
    if( HeaderFooterHelper::isHeader( mxModel, xCurrentText ) || HeaderFooterHelper::isFooter( mxModel, xCurrentText ) )
    {
        uno::Reference< beans::XPropertySet > xPageStyleProps( word::getCurrentPageStyle( mxModel ), uno::UNO_QUERY_THROW );
        sal_Int32 nIndex = word::WdHeaderFooterIndex::wdHeaderFooterPrimary;
        sal_Bool isHeader = HeaderFooterHelper::isHeader( mxModel, xCurrentText );
        if( HeaderFooterHelper::isEvenPagesHeader( mxModel, xCurrentText ) || HeaderFooterHelper::isEvenPagesFooter( mxModel, xCurrentText ) )
            nIndex = word::WdHeaderFooterIndex::wdHeaderFooterEvenPages;
        else if( HeaderFooterHelper::isFirstPageHeader( mxModel, xCurrentText ) || HeaderFooterHelper::isFirstPageFooter( mxModel, xCurrentText ) )
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
       throw uno::RuntimeException();

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

rtl::OUString&
SwVbaSelection::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaSelection") );
    return sImplName;
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

