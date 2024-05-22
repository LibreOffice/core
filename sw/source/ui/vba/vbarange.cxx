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
#include "vbarange.hxx"
#include <utility>
#include <vbahelper/vbahelper.hxx>
#include <basic/sberrors.hxx>
#include "vbarangehelper.hxx"
#include <ooo/vba/word/WdBreakType.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include "vbaparagraphformat.hxx"
#include "vbastyle.hxx"
#include "vbafont.hxx"
#include "vbafind.hxx"
#include "vbapalette.hxx"
#include "vbapagesetup.hxx"
#include "vbalistformat.hxx"
#include "vbarevisions.hxx"
#include "vbabookmarks.hxx"
#include "vbasections.hxx"
#include "vbafield.hxx"
#include "wordvbahelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaRange::SwVbaRange( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, uno::Reference< text::XTextDocument > xTextDocument, const uno::Reference< text::XTextRange >& rStart ) : SwVbaRange_BASE( rParent, rContext ), mxTextDocument(std::move( xTextDocument ))
{
    uno::Reference< text::XTextRange > xEnd;
    initialize( rStart, xEnd );
}

SwVbaRange::SwVbaRange( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, uno::Reference< text::XTextDocument > xTextDocument, const uno::Reference< text::XTextRange >& rStart, const uno::Reference< text::XTextRange >& rEnd ) : SwVbaRange_BASE( rParent, rContext ), mxTextDocument(std::move( xTextDocument ))
{
    initialize( rStart, rEnd );
}

SwVbaRange::SwVbaRange( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, uno::Reference< text::XTextDocument > xTextDocument, const uno::Reference< text::XTextRange >& rStart, const uno::Reference< text::XTextRange >& rEnd, uno::Reference< text::XText > xText ) : SwVbaRange_BASE( rParent, rContext ),mxTextDocument(std::move( xTextDocument )), mxText(std::move( xText ))
{
    initialize( rStart, rEnd );
}

SwVbaRange::~SwVbaRange()
{
}

void SwVbaRange::initialize( const uno::Reference< text::XTextRange >& rStart, const uno::Reference< text::XTextRange >& rEnd )
{
    if( !mxText.is() )
    {
        mxText = mxTextDocument->getText();
    }

    mxTextCursor = SwVbaRangeHelper::initCursor( rStart, mxText );
    if( !mxTextCursor.is() )
        throw uno::RuntimeException(u"Fails to create text cursor"_ustr );
    mxTextCursor->collapseToStart();

    if( rEnd.is() )
        mxTextCursor->gotoRange( rEnd, true );
    else
        mxTextCursor->gotoEnd( true );
}

uno::Reference< text::XTextRange > SAL_CALL
SwVbaRange::getXTextRange()
{
    uno::Reference< text::XTextRange > xTextRange( mxTextCursor, uno::UNO_QUERY_THROW );
    return xTextRange;
}

/**
* The complexity in this method is because we need to workaround
* an issue that the last paragraph in a document does not have a trailing CRLF.
* @return
*/
OUString SAL_CALL
SwVbaRange::getText()
{
    OUString aText = mxTextCursor->getString();
    sal_Int32 nLen = aText.getLength();

    // FIXME: should add a line separator if the range includes the last paragraph
    if( nLen == 0 )
    {
        if( mxTextCursor->isCollapsed() )
        {
            mxTextCursor->goRight( 1, true );
            aText = mxTextCursor->getString();
            mxTextCursor->collapseToStart();
        }
        else
        {
            uno::Reference< text::XTextRange > xStart = mxTextCursor->getStart();
            uno::Reference< text::XTextRange > xEnd = mxTextCursor->getEnd();
            mxTextCursor->collapseToEnd();
            mxTextCursor->goRight( 1, true );
            mxTextCursor->gotoRange( xStart, false );
            mxTextCursor->gotoRange( xEnd, true );
        }
    }

    return aText;
}

void SAL_CALL
SwVbaRange::setText( const OUString& rText )
{
    // Emulate the MSWord behavior, Don't delete the bookmark
    // which contains no text string in current inserting position,
    OUString sName;
    uno::Reference< text::XTextRange > xRange( mxTextCursor, uno::UNO_QUERY_THROW );
    try
    {
        uno::Reference< text::XTextContent > xBookmark = SwVbaRangeHelper::findBookmarkByPosition( mxTextDocument, xRange->getStart() );
        if( xBookmark.is() )
        {
            uno::Reference< container::XNamed > xNamed( xBookmark, uno::UNO_QUERY_THROW );
            sName = xNamed->getName();
        }
    }
    catch (const uno::Exception&)
    {
        // do nothing
    }

    if( rText.indexOf( '\n' )  != -1 )
    {
        mxTextCursor->setString( OUString() );
        // process CR in strings
        SwVbaRangeHelper::insertString( xRange, mxText, rText, true );
    }
    else
    {
        mxTextCursor->setString( rText );
    }

    // insert the bookmark if the bookmark is deleted during setting text string
    if( !sName.isEmpty() )
    {
        uno::Reference< text::XBookmarksSupplier > xBookmarksSupplier( mxTextDocument, uno::UNO_QUERY_THROW );
        uno::Reference< container::XNameAccess > xNameAccess( xBookmarksSupplier->getBookmarks(), uno::UNO_SET_THROW );
        if( !xNameAccess->hasByName( sName ) )
        {
            uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
            SwVbaBookmarks::addBookmarkByName( xModel, sName, xRange->getStart() );
        }
    }
}

// FIXME: test is not pass
void SAL_CALL SwVbaRange::InsertBreak(const uno::Any& _breakType)
{
    // default type is wdPageBreak;
    sal_Int32 nBreakType = word::WdBreakType::wdPageBreak;
    if( _breakType.hasValue() )
        _breakType >>= nBreakType;

    style::BreakType eBreakType = style::BreakType_NONE;
    switch( nBreakType )
    {
        case word::WdBreakType::wdPageBreak:
            eBreakType = style::BreakType_PAGE_BEFORE;
            break;
        case word::WdBreakType::wdColumnBreak:
            eBreakType = style::BreakType_COLUMN_AFTER;
            break;
        case word::WdBreakType::wdLineBreak:
        case word::WdBreakType::wdLineBreakClearLeft:
        case word::WdBreakType::wdLineBreakClearRight:
        case word::WdBreakType::wdSectionBreakContinuous:
        case word::WdBreakType::wdSectionBreakEvenPage:
        case word::WdBreakType::wdSectionBreakNextPage:
        case word::WdBreakType::wdSectionBreakOddPage:
        case word::WdBreakType::wdTextWrappingBreak:
            DebugHelper::basicexception( ERRCODE_BASIC_NOT_IMPLEMENTED, {} );
            break;
        default:
            DebugHelper::basicexception( ERRCODE_BASIC_BAD_PARAMETER, {} );
    }

    if( eBreakType != style::BreakType_NONE )
    {
        if( !mxTextCursor->isCollapsed() )
        {
            mxTextCursor->setString( OUString() );
            mxTextCursor->collapseToStart();
        }

        uno::Reference< beans::XPropertySet > xProp( mxTextCursor, uno::UNO_QUERY_THROW );
        xProp->setPropertyValue(u"BreakType"_ustr, uno::Any( eBreakType ) );
    }
}

void SAL_CALL
SwVbaRange::Select()
{
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextViewCursor > xTextViewCursor = word::getXTextViewCursor( xModel );
    xTextViewCursor->gotoRange( mxTextCursor->getStart(), false );
    xTextViewCursor->gotoRange( mxTextCursor->getEnd(), true );
}

void SAL_CALL
SwVbaRange::InsertParagraph()
{
    mxTextCursor->setString( u""_ustr );
    InsertParagraphBefore();
}

void SAL_CALL
SwVbaRange::InsertParagraphBefore()
{
    uno::Reference< text::XTextRange > xTextRange = mxTextCursor->getStart();
    mxText->insertControlCharacter( xTextRange, text::ControlCharacter::PARAGRAPH_BREAK, true );
    mxTextCursor->gotoRange( xTextRange, true );
}

void SAL_CALL
SwVbaRange::InsertParagraphAfter()
{
    uno::Reference< text::XTextRange > xTextRange = mxTextCursor->getEnd();
    mxText->insertControlCharacter( xTextRange, text::ControlCharacter::PARAGRAPH_BREAK, true );
}

uno::Reference< word::XParagraphFormat > SAL_CALL
SwVbaRange::getParagraphFormat()
{
    uno::Reference< beans::XPropertySet > xParaProps( mxTextCursor, uno::UNO_QUERY_THROW );
    return uno::Reference< word::XParagraphFormat >( new SwVbaParagraphFormat( this, mxContext, xParaProps ) );
}

void SAL_CALL
SwVbaRange::setParagraphFormat( const uno::Reference< word::XParagraphFormat >& /*rParagraphFormat*/ )
{
    throw uno::RuntimeException(u"Not implemented"_ustr );
}

void SwVbaRange::GetStyleInfo(OUString& aStyleName, OUString& aStyleType )
{
    uno::Reference< beans::XPropertySet > xProp( mxTextCursor, uno::UNO_QUERY_THROW );
    if( ( xProp->getPropertyValue(u"CharStyleName"_ustr) >>= aStyleName ) && !aStyleName.isEmpty() )
    {
        aStyleType = "CharacterStyles";
    }
    else if( ( xProp->getPropertyValue(u"ParaStyleName"_ustr) >>= aStyleName ) && !aStyleName.isEmpty() )
    {
        aStyleType = "ParagraphStyles";
    }
    if( aStyleType.isEmpty() )
    {
        DebugHelper::runtimeexception( ERRCODE_BASIC_INTERNAL_ERROR );
    }
}

uno::Any SAL_CALL
SwVbaRange::getStyle()
{
    OUString aStyleName;
    OUString aStyleType;
    GetStyleInfo( aStyleName, aStyleType );
    uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( mxTextDocument, uno::UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xStylesAccess( xStyleSupplier->getStyleFamilies()->getByName( aStyleType ), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xStyleProps( xStylesAccess->getByName( aStyleName ), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    return uno::Any( uno::Reference< word::XStyle >( new SwVbaStyle( this, mxContext, xModel, xStyleProps ) ) );
}

void SAL_CALL
SwVbaRange::setStyle( const uno::Any& rStyle )
{
    uno::Reference< beans::XPropertySet > xParaProps( mxTextCursor, uno::UNO_QUERY_THROW );
    SwVbaStyle::setStyle( xParaProps, rStyle );
}

uno::Reference< word::XFont > SAL_CALL
SwVbaRange::getFont()
{
    VbaPalette aColors;
    return new SwVbaFont( mxParent, mxContext, aColors.getPalette(), uno::Reference< beans::XPropertySet >( getXTextRange(), uno::UNO_QUERY_THROW ) );
}

uno::Reference< word::XFind > SAL_CALL
SwVbaRange::getFind()
{
    uno::Reference< text::XTextRange > xTextRange = getXTextRange();
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    return SwVbaFind::GetOrCreateFind(this, mxContext, xModel, xTextRange);
}

uno::Reference< word::XListFormat > SAL_CALL
SwVbaRange::getListFormat()
{
    return uno::Reference< word::XListFormat >( new SwVbaListFormat( this, mxContext, getXTextRange() ) );
}

::sal_Int32 SAL_CALL SwVbaRange::getLanguageID()
{
    uno::Reference< beans::XPropertySet > xParaProps( mxTextCursor, uno::UNO_QUERY_THROW );
    return static_cast<sal_uInt16>(SwVbaStyle::getLanguageID( xParaProps ));
}

void SAL_CALL SwVbaRange::setLanguageID( ::sal_Int32 _languageid )
{
    uno::Reference< beans::XPropertySet > xParaProps( mxTextCursor, uno::UNO_QUERY_THROW );
    SwVbaStyle::setLanguageID( xParaProps, LanguageType(_languageid) );
}

uno::Any SAL_CALL
SwVbaRange::PageSetup( )
{
    uno::Reference< beans::XPropertySet > xParaProps( mxTextCursor, uno::UNO_QUERY_THROW );
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    OUString aPageStyleName;
    xParaProps->getPropertyValue(u"PageStyleName"_ustr) >>= aPageStyleName;
    uno::Reference< style::XStyleFamiliesSupplier > xSytleFamSupp( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xSytleFamNames( xSytleFamSupp->getStyleFamilies(), uno::UNO_SET_THROW );
    uno::Reference< container::XNameAccess > xPageStyles( xSytleFamNames->getByName(u"PageStyles"_ustr), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPageProps( xPageStyles->getByName( aPageStyleName ), uno::UNO_QUERY_THROW );
    return uno::Any( uno::Reference< word::XPageSetup >( new SwVbaPageSetup( this, mxContext, xModel, xPageProps ) ) );
}

::sal_Int32 SAL_CALL SwVbaRange::getStart()
{
    uno::Reference< text::XText > xText = mxTextDocument->getText();
    return SwVbaRangeHelper::getPosition( xText, mxTextCursor->getStart() );
}

void SAL_CALL SwVbaRange::setStart( ::sal_Int32 _start )
{
    uno::Reference< text::XText > xText = mxTextDocument->getText();
    uno::Reference< text::XTextRange > xStart = SwVbaRangeHelper::getRangeByPosition( xText, _start );
    uno::Reference< text::XTextRange > xEnd = mxTextCursor->getEnd();

    mxTextCursor->gotoRange( xStart, false );
    mxTextCursor->gotoRange( xEnd, true );
}

::sal_Int32 SAL_CALL SwVbaRange::getEnd()
{
    uno::Reference< text::XText > xText = mxTextDocument->getText();
    return SwVbaRangeHelper::getPosition( xText, mxTextCursor->getEnd() );
}

void SAL_CALL SwVbaRange::setEnd( ::sal_Int32 _end )
{
    uno::Reference< text::XText > xText = mxTextDocument->getText();
    uno::Reference< text::XTextRange > xEnd = SwVbaRangeHelper::getRangeByPosition( xText, _end );

    mxTextCursor->collapseToStart();
    mxTextCursor->gotoRange( xEnd, true );
}

sal_Bool SAL_CALL SwVbaRange::InRange( const uno::Reference< ::ooo::vba::word::XRange >& Range )
{
    SwVbaRange* pRange = dynamic_cast< SwVbaRange* >( Range.get() );
    if( !pRange )
        throw uno::RuntimeException();
    uno::Reference< text::XTextRange > xTextRange = pRange->getXTextRange();
    uno::Reference< text::XTextRangeCompare > xTRC( mxTextCursor->getText(), uno::UNO_QUERY_THROW );
    if( xTRC->compareRegionStarts( xTextRange, getXTextRange() ) >= 0 && xTRC->compareRegionEnds( xTextRange, getXTextRange() ) <= 0 )
        return true;
    return false;
}

uno::Any SAL_CALL
SwVbaRange::Revisions( const uno::Any& index )
{
    uno::Reference< text::XTextRange > xTextRange = getXTextRange();
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xCol( new SwVbaRevisions( mxParent, mxContext, xModel, xTextRange ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaRange::Sections( const uno::Any& index )
{
    uno::Reference< text::XTextRange > xTextRange = getXTextRange();
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xCol( new SwVbaSections( mxParent, mxContext, xModel, xTextRange ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

uno::Any SAL_CALL
SwVbaRange::Fields( const uno::Any& index )
{
    //FIXME: should be get the field in current range
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xCol( new SwVbaFields( mxParent, mxContext, xModel ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::Any( xCol );
}

OUString
SwVbaRange::getServiceImplName()
{
    return u"SwVbaRange"_ustr;
}

uno::Sequence< OUString >
SwVbaRange::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Range"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
