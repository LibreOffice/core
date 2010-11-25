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
#include "vbarange.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include "vbarangehelper.hxx"
#include <ooo/vba/word/WdBreakType.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include "wordvbahelper.hxx"
#include "vbaparagraphformat.hxx"
#include "vbastyle.hxx"
#include "vbafont.hxx"
#include "vbapalette.hxx"
#include "vbapagesetup.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaRange::SwVbaRange( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XTextDocument >& rTextDocument, const uno::Reference< text::XTextRange >& rStart, sal_Bool _bMaySpanEndOfDocument ) throw (uno::RuntimeException) : SwVbaRange_BASE( rParent, rContext ), mxTextDocument( rTextDocument ), mbMaySpanEndOfDocument( _bMaySpanEndOfDocument )
{
    uno::Reference< text::XTextRange > xEnd;
    initialize( rStart, xEnd );
}

SwVbaRange::SwVbaRange( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XTextDocument >& rTextDocument, const uno::Reference< text::XTextRange >& rStart, const uno::Reference< text::XTextRange >& rEnd, sal_Bool _bMaySpanEndOfDocument ) throw (uno::RuntimeException) : SwVbaRange_BASE( rParent, rContext ), mxTextDocument( rTextDocument ), mbMaySpanEndOfDocument( _bMaySpanEndOfDocument )
{
    initialize( rStart, rEnd );
}

SwVbaRange::SwVbaRange( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XTextDocument >& rTextDocument, const uno::Reference< text::XTextRange >& rStart, const uno::Reference< text::XTextRange >& rEnd, const uno::Reference< text::XText >& rText, sal_Bool _bMaySpanEndOfDocument ) throw (uno::RuntimeException) : SwVbaRange_BASE( rParent, rContext ),mxTextDocument( rTextDocument ), mxText( rText ), mbMaySpanEndOfDocument( _bMaySpanEndOfDocument )
{
    initialize( rStart, rEnd );
}

SwVbaRange::~SwVbaRange()
{
}

void SwVbaRange::initialize( const uno::Reference< text::XTextRange >& rStart, const uno::Reference< text::XTextRange >& rEnd ) throw (uno::RuntimeException)
{
    if( !mxText.is() )
    {
        mxText = mxTextDocument->getText();
    }

    mxTextCursor = SwVbaRangeHelper::initCursor( rStart, mxText );
    mxTextCursor->collapseToStart();

    if( rEnd.is() )
        mxTextCursor->gotoRange( rEnd, sal_True );
    else
        mxTextCursor->gotoEnd( sal_True );
}

uno::Reference< text::XTextRange > SAL_CALL
SwVbaRange::getXTextRange() throw (uno::RuntimeException)
{
    uno::Reference< text::XTextRange > xTextRange( mxTextCursor, uno::UNO_QUERY_THROW );
    return xTextRange;
}
#ifdef TOMORROW
void SwVbaRange::setXTextRange( const uno::Reference< text::XTextRange >& xRange ) throw (uno::RuntimeException)
{
    mxTextCursor->gotoRange( xRange->getStart(), sal_False );
    mxTextCursor->gotoRange( xRange->getEnd(), sal_True );
}
#endif
/**
* The complexity in this method is because we need to workaround
* an issue that the last paragraph in a document does not have a trailing CRLF.
* @return
*/
rtl::OUString SAL_CALL
SwVbaRange::getText() throw ( uno::RuntimeException )
{
    rtl::OUString aText = mxTextCursor->getString();
    sal_Int32 nLen = aText.getLength();

    // FIXME: should add a line separator if the range includes the last paragraph
    if( nLen == 0 )
    {
        if( mxTextCursor->isCollapsed() )
        {
            mxTextCursor->goRight( 1, sal_True );
            aText = mxTextCursor->getString();
            mxTextCursor->collapseToStart();
        }
        else
        {
            uno::Reference< text::XTextRange > xStart = mxTextCursor->getStart();
            uno::Reference< text::XTextRange > xEnd = mxTextCursor->getEnd();
            mxTextCursor->collapseToEnd();
            mxTextCursor->goRight( 1, sal_True );
            mxTextCursor->gotoRange( xStart, sal_False );
            mxTextCursor->gotoRange( xEnd, sal_True );
        }
    }

    return aText;
}

void SAL_CALL
SwVbaRange::setText( const rtl::OUString& rText ) throw ( uno::RuntimeException )
{
    if( rText.indexOf( '\n' )  != -1 )
    {
        mxTextCursor->setString( rtl::OUString() );
        // process CR in strings
        uno::Reference< text::XTextRange > xRange( mxTextCursor, uno::UNO_QUERY_THROW );
        SwVbaRangeHelper::insertString( xRange, mxText, rText, sal_True );
    }
    else
    {
        mxTextCursor->setString( rText );
    }
}

// FIXME: test is not pass
void SAL_CALL SwVbaRange::InsertBreak( const uno::Any& _breakType ) throw (uno::RuntimeException)
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
            DebugHelper::exception( SbERR_NOT_IMPLEMENTED, rtl::OUString() );
            break;
        default:
            DebugHelper::exception( SbERR_BAD_PARAMETER, rtl::OUString() );
    }

    if( eBreakType != style::BreakType_NONE )
    {
        if( !mxTextCursor->isCollapsed() )
        {
            mxTextCursor->setString( rtl::OUString() );
            mxTextCursor->collapseToStart();
        }

        uno::Reference< beans::XPropertySet > xProp( mxTextCursor, uno::UNO_QUERY_THROW );
        xProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BreakType") ), uno::makeAny( eBreakType ) );
    }
}

void SAL_CALL
SwVbaRange::Select() throw ( uno::RuntimeException )
{
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextViewCursor > xTextViewCursor = word::getXTextViewCursor( xModel );
    xTextViewCursor->gotoRange( mxTextCursor->getStart(), sal_False );
    xTextViewCursor->gotoRange( mxTextCursor->getEnd(), sal_True );
}

void SAL_CALL
SwVbaRange::InsertParagraph() throw ( uno::RuntimeException )
{
    mxTextCursor->setString( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("") ) );
    InsertParagraphBefore();
}

void SAL_CALL
SwVbaRange::InsertParagraphBefore() throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextRange > xTextRange = mxTextCursor->getStart();
    mxText->insertControlCharacter( xTextRange, text::ControlCharacter::PARAGRAPH_BREAK, sal_True );
    mxTextCursor->gotoRange( xTextRange, sal_True );
}

void SAL_CALL
SwVbaRange::InsertParagraphAfter() throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextRange > xTextRange = mxTextCursor->getEnd();
    mxText->insertControlCharacter( xTextRange, text::ControlCharacter::PARAGRAPH_BREAK, sal_True );
}

uno::Reference< word::XParagraphFormat > SAL_CALL
SwVbaRange::getParagraphFormat() throw ( uno::RuntimeException )
{
    uno::Reference< beans::XPropertySet > xParaProps( mxTextCursor, uno::UNO_QUERY_THROW );
    return uno::Reference< word::XParagraphFormat >( new SwVbaParagraphFormat( this, mxContext, mxTextDocument, xParaProps ) );
}

void SAL_CALL
SwVbaRange::setParagraphFormat( const uno::Reference< word::XParagraphFormat >& /*rParagraphFormat*/ ) throw ( uno::RuntimeException )
{
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
}

uno::Reference< word::XStyle > SAL_CALL
SwVbaRange::getStyle() throw ( uno::RuntimeException )
{
    rtl::OUString aStyleName;
    rtl::OUString aStyleType;
    uno::Reference< beans::XPropertySet > xProp( mxTextCursor, uno::UNO_QUERY_THROW );
    if( ( xProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CharStyleName") ) ) >>= aStyleName ) && aStyleName.getLength() )
    {
        aStyleType = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CharacterStyles") );
    }
    else if( ( xProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ParaStyleName") ) ) >>= aStyleName ) && aStyleName.getLength() )
    {
        aStyleType = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ParagraphStyles") );
    }
    if( aStyleType.getLength() == 0 )
    {
        DebugHelper::exception( SbERR_INTERNAL_ERROR, rtl::OUString() );
    }
    uno::Reference< style::XStyleFamiliesSupplier > xStyleSupplier( mxTextDocument, uno::UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xStylesAccess( xStyleSupplier->getStyleFamilies()->getByName( aStyleType ), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xStyleProps( xStylesAccess->getByName( aStyleName ), uno::UNO_QUERY_THROW );
    return uno::Reference< word::XStyle >( new SwVbaStyle( this, mxContext, xStyleProps ) );
}

void SAL_CALL
SwVbaRange::setStyle( const uno::Reference< word::XStyle >& rStyle ) throw ( uno::RuntimeException )
{
    uno::Reference< beans::XPropertySet > xParaProps( mxTextCursor, uno::UNO_QUERY_THROW );
    SwVbaStyle::setStyle( xParaProps, rStyle );
}

uno::Reference< word::XFont > SAL_CALL
SwVbaRange::getFont() throw ( uno::RuntimeException )
{
    VbaPalette aColors;
    return new SwVbaFont( mxParent, mxContext, aColors.getPalette(), uno::Reference< beans::XPropertySet >( getXTextRange(), uno::UNO_QUERY_THROW ) );
}

::sal_Int32 SAL_CALL SwVbaRange::getLanguageID() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xParaProps( mxTextCursor, uno::UNO_QUERY_THROW );
    return SwVbaStyle::getLanguageID( xParaProps );
}

void SAL_CALL SwVbaRange::setLanguageID( ::sal_Int32 _languageid ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xParaProps( mxTextCursor, uno::UNO_QUERY_THROW );
    SwVbaStyle::setLanguageID( xParaProps, _languageid );
}

uno::Any SAL_CALL
SwVbaRange::PageSetup( ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xParaProps( mxTextCursor, uno::UNO_QUERY_THROW );
    uno::Reference< frame::XModel > xModel( mxTextDocument, uno::UNO_QUERY_THROW );
    rtl::OUString aPageStyleName;
    xParaProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageStyleName"))) >>= aPageStyleName;
    uno::Reference< style::XStyleFamiliesSupplier > xSytleFamSupp( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xSytleFamNames( xSytleFamSupp->getStyleFamilies(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xPageStyles( xSytleFamNames->getByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageStyles") ) ), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPageProps( xPageStyles->getByName( aPageStyleName ), uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< word::XPageSetup >( new SwVbaPageSetup( this, mxContext, xModel, xPageProps ) ) );
}

::sal_Int32 SAL_CALL SwVbaRange::getStart() throw (uno::RuntimeException)
{
    uno::Reference< text::XText > xText = mxTextDocument->getText();
    return SwVbaRangeHelper::getPosition( xText, mxTextCursor->getStart() );
}

void SAL_CALL SwVbaRange::setStart( ::sal_Int32 _start ) throw (uno::RuntimeException)
{
    uno::Reference< text::XText > xText = mxTextDocument->getText();
    uno::Reference< text::XTextRange > xStart = SwVbaRangeHelper::getRangeByPosition( xText, _start );
    uno::Reference< text::XTextRange > xEnd = mxTextCursor->getEnd();

    mxTextCursor->gotoRange( xStart, sal_False );
    mxTextCursor->gotoRange( xEnd, sal_True );
}

::sal_Int32 SAL_CALL SwVbaRange::getEnd() throw (uno::RuntimeException)
{
    uno::Reference< text::XText > xText = mxTextDocument->getText();
    return SwVbaRangeHelper::getPosition( xText, mxTextCursor->getEnd() );
}

void SAL_CALL SwVbaRange::setEnd( ::sal_Int32 _end ) throw (uno::RuntimeException)
{
    uno::Reference< text::XText > xText = mxTextDocument->getText();
    uno::Reference< text::XTextRange > xEnd = SwVbaRangeHelper::getRangeByPosition( xText, _end );

    mxTextCursor->collapseToStart();
    mxTextCursor->gotoRange( xEnd, sal_True );
}

rtl::OUString&
SwVbaRange::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaRange") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaRange::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Range" ) );
    }
    return aServiceNames;
}

