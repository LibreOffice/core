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

#include <memory>
#include <sal/config.h>

#include <utility>

#include <AccessibleCsvControl.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <unotools/accessiblerelationsethelper.hxx>
#include <comphelper/sequence.hxx>
#include <scitems.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/langitem.hxx>
#include <csvtablebox.hxx>
#include <csvcontrol.hxx>
#include <csvruler.hxx>
#include <csvgrid.hxx>
#include <AccessibleText.hxx>
#include <editsrc.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <scmod.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <o3tl/string_view.hxx>

using ::utl::AccessibleRelationSetHelper;
using ::accessibility::AccessibleStaticTextBase;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::beans::PropertyValue;
using namespace ::com::sun::star::accessibility;

const sal_Unicode cRulerDot         = '.';
const sal_Unicode cRulerLine        = '|';

const sal_Int32 CSV_LINE_HEADER     = CSV_POS_INVALID;
const sal_uInt32 CSV_COLUMN_HEADER  = CSV_COLUMN_INVALID;

ScAccessibleCsvControl::ScAccessibleCsvControl(ScCsvControl& rControl)
    : mpControl(&rControl)
{
}

ScAccessibleCsvControl::~ScAccessibleCsvControl()
{
    ensureDisposed();
}

void SAL_CALL ScAccessibleCsvControl::disposing()
{
    SolarMutexGuard aGuard;
    mpControl = nullptr;
    comphelper::OAccessibleComponentHelper::disposing();
}

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL ScAccessibleCsvControl::getAccessibleAtPoint( const css::awt::Point& /* rPoint */ )
{
    ensureAlive();
    return nullptr;
}

void SAL_CALL ScAccessibleCsvControl::grabFocus()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    implGetControl().GrabFocus();
}

// events ---------------------------------------------------------------------

void ScAccessibleCsvControl::SendFocusEvent( bool bFocused )
{
    Any aOldAny, aNewAny;
    if (bFocused)
        aNewAny <<= AccessibleStateType::FOCUSED;
    else
        aOldAny <<= AccessibleStateType::FOCUSED;
    NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny);
}

void ScAccessibleCsvControl::SendCaretEvent()
{
    OSL_FAIL( "ScAccessibleCsvControl::SendCaretEvent - Illegal call" );
}

void ScAccessibleCsvControl::SendVisibleEvent()
{
    NotifyAccessibleEvent(AccessibleEventId::VISIBLE_DATA_CHANGED, Any(), Any());
}

void ScAccessibleCsvControl::SendSelectionEvent()
{
    NotifyAccessibleEvent(AccessibleEventId::SELECTION_CHANGED, Any(), Any());
}

void ScAccessibleCsvControl::SendTableUpdateEvent( sal_uInt32 /* nFirstColumn */, sal_uInt32 /* nLastColumn */, bool /* bAllRows */ )
{
    OSL_FAIL( "ScAccessibleCsvControl::SendTableUpdateEvent - Illegal call" );
}

void ScAccessibleCsvControl::SendInsertColumnEvent( sal_uInt32 /* nFirstColumn */, sal_uInt32 /* nLastColumn */ )
{
    OSL_FAIL( "ScAccessibleCsvControl::SendInsertColumnEvent - Illegal call" );
}

void ScAccessibleCsvControl::SendRemoveColumnEvent( sal_uInt32 /* nFirstColumn */, sal_uInt32 /* nLastColumn */ )
{
    OSL_FAIL( "ScAccessibleCsvControl::SendRemoveColumnEvent - Illegal call" );
}

// helpers --------------------------------------------------------------------

css::awt::Rectangle ScAccessibleCsvControl::implGetBounds()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    Size aOutSize(implGetControl().GetOutputSizePixel());
    return css::awt::Rectangle(0, 0, aOutSize.Width(), aOutSize.Height());
}

ScCsvControl& ScAccessibleCsvControl::implGetControl() const
{
    assert(mpControl && "ScAccessibleCsvControl::implGetControl - missing control");
    return *mpControl;
}

sal_Int64 ScAccessibleCsvControl::implCreateStateSet()
{
    SolarMutexGuard aGuard;
    sal_Int64 nStateSet = 0;
    if (isAlive())
    {
        const ScCsvControl& rCtrl = implGetControl();
        nStateSet |= AccessibleStateType::OPAQUE;
        if( rCtrl.IsEnabled() )
            nStateSet |= AccessibleStateType::ENABLED;
        if( rCtrl.IsReallyVisible() )
            nStateSet |= AccessibleStateType::SHOWING;
        if( rCtrl.IsVisible() )
            nStateSet |= AccessibleStateType::VISIBLE;
    }
    else
        nStateSet |= AccessibleStateType::DEFUNC;
    return nStateSet;
}

// Ruler ======================================================================

/** Converts a ruler cursor position to API text index. */
static sal_Int32 lcl_GetApiPos( sal_Int32 nRulerPos )
{
    sal_Int32 nApiPos = nRulerPos;
    sal_Int32 nStart = (nRulerPos - 1) / 10;
    sal_Int32 nExp = 1;
    while( nStart >= nExp )
    {
        nApiPos += nStart - nExp + 1;
        nExp *= 10;
    }
    return ::std::max( nApiPos, static_cast<sal_Int32>(0) );
}

/** Converts an API text index to a ruler cursor position. */
static sal_Int32 lcl_GetRulerPos( sal_Int32 nApiPos )
{
    sal_Int32 nDiv = 10;
    sal_Int32 nExp = 10;
    sal_Int32 nRulerPos = 0;
    sal_Int32 nApiBase = 0;
    sal_Int32 nApiLimit = 10;
    while( nApiPos >= nApiLimit )
    {
        ++nDiv;
        nRulerPos = nExp;
        nExp *= 10;
        nApiBase = nApiLimit;
        nApiLimit = lcl_GetApiPos( nExp );
    }
    sal_Int32 nRelPos = nApiPos - nApiBase;
    return nRulerPos + nRelPos / nDiv * 10 + ::std::max<sal_Int32>( nRelPos % nDiv - nDiv + 10, 0 );
}

/** Expands the sequence's size and returns the base index of the new inserted elements. */
static sal_Int32 lcl_ExpandSequence( Sequence< PropertyValue >& rSeq, sal_Int32 nExp )
{
    OSL_ENSURE( nExp > 0, "lcl_ExpandSequence - invalid value" );
    rSeq.realloc( rSeq.getLength() + nExp );
    return rSeq.getLength() - nExp;
}

/** Fills the property value rVal with the specified name and value from the item. */
static void lcl_FillProperty( PropertyValue& rVal, const OUString& rPropName, const SfxPoolItem& rItem, sal_uInt8 nMID )
{
    rVal.Name = rPropName;
    rItem.QueryValue( rVal.Value, nMID );
}

/** Fills the sequence with all font attributes of rFont. */
static void lcl_FillFontAttributes( Sequence< PropertyValue >& rSeq, const vcl::Font& rFont )
{
    SvxFontItem aFontItem( rFont.GetFamilyType(), rFont.GetFamilyName(), rFont.GetStyleName(), rFont.GetPitch(), rFont.GetCharSet(), ATTR_FONT );
    SvxFontHeightItem aHeightItem( rFont.GetFontSize().Height(), 100, ATTR_FONT_HEIGHT );
    SvxLanguageItem aLangItem( rFont.GetLanguage(), ATTR_FONT_LANGUAGE );

    sal_Int32 nIndex = lcl_ExpandSequence( rSeq, 7 );
    auto pSeq = rSeq.getArray();
    lcl_FillProperty( pSeq[ nIndex++ ], "CharFontName",      aFontItem,   MID_FONT_FAMILY_NAME );
    lcl_FillProperty( pSeq[ nIndex++ ], "CharFontFamily",    aFontItem,   MID_FONT_FAMILY );
    lcl_FillProperty( pSeq[ nIndex++ ], "CharFontStyleName", aFontItem,   MID_FONT_STYLE_NAME );
    lcl_FillProperty( pSeq[ nIndex++ ], "CharFontCharSet",   aFontItem,   MID_FONT_PITCH );
    lcl_FillProperty( pSeq[ nIndex++ ], "CharFontPitch",     aFontItem,   MID_FONT_CHAR_SET );
    lcl_FillProperty( pSeq[ nIndex++ ], "CharHeight",        aHeightItem, MID_FONTHEIGHT );
    lcl_FillProperty( pSeq[ nIndex++ ], "CharLocale",        aLangItem,   MID_LANG_LOCALE );
}

ScAccessibleCsvRuler::ScAccessibleCsvRuler(ScCsvRuler& rRuler)
    : ImplInheritanceHelper(rRuler)
{
    constructStringBuffer();
}

ScAccessibleCsvRuler::~ScAccessibleCsvRuler()
{
    ensureDisposed();
}

// XAccessibleComponent -----------------------------------------------------

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getForeground(  )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return sal_Int32(Application::GetSettings().GetStyleSettings().GetLabelTextColor());
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getBackground(  )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return sal_Int32(Application::GetSettings().GetStyleSettings().GetFaceColor());
}

// XAccessibleContext ---------------------------------------------------------

sal_Int64 SAL_CALL ScAccessibleCsvRuler::getAccessibleChildCount()
{
    ensureAlive();
    return 0;
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvRuler::getAccessibleChild( sal_Int64 /* nIndex */ )
{
    ensureAlive();
    throw IndexOutOfBoundsException();
}

Reference< XAccessibleRelationSet > SAL_CALL ScAccessibleCsvRuler::getAccessibleRelationSet()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    rtl::Reference<AccessibleRelationSetHelper> pRelationSet = new AccessibleRelationSetHelper();

    ScCsvRuler& rRuler = implGetRuler();
    ScCsvTableBox* pTableBox = rRuler.GetTableBox();
    ScCsvGrid& rGrid = pTableBox->GetGrid();

    css::uno::Reference<css::accessibility::XAccessible> xAccObj(static_cast<ScAccessibleCsvGrid*>(rGrid.GetAccessible()));
    if( xAccObj.is() )
    {
        Sequence<Reference<css::accessibility::XAccessible>> aSeq{ xAccObj };
        pRelationSet->AddRelation( AccessibleRelation( AccessibleRelationType::CONTROLLER_FOR, aSeq ) );
    }

    return pRelationSet;
}

sal_Int64 SAL_CALL ScAccessibleCsvRuler::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;
    sal_Int64 nStateSet = implCreateStateSet();
    if( isAlive() )
    {
        nStateSet |= AccessibleStateType::FOCUSABLE;
        nStateSet |= AccessibleStateType::SINGLE_LINE;
        if( implGetRuler().HasFocus() )
            nStateSet |= AccessibleStateType::FOCUSED;
    }
    return nStateSet;
}

// XAccessibleText ------------------------------------------------------------

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getCaretPosition()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return lcl_GetApiPos( implGetRuler().GetRulerCursorPos() );
}

sal_Bool SAL_CALL ScAccessibleCsvRuler::setCaretPosition( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nIndex );
    ScCsvRuler& rRuler = implGetRuler();
    sal_Int32 nOldCursor = rRuler.GetRulerCursorPos();
    rRuler.Execute( CSVCMD_MOVERULERCURSOR, lcl_GetRulerPos( nIndex ) );
    return rRuler.GetRulerCursorPos() != nOldCursor;
}

sal_Unicode SAL_CALL ScAccessibleCsvRuler::getCharacter( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nIndex );
    return maBuffer[nIndex];
}

Sequence< PropertyValue > SAL_CALL ScAccessibleCsvRuler::getCharacterAttributes( sal_Int32 nIndex,
            const css::uno::Sequence< OUString >& /* aRequestedAttributes */ )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndexWithEnd( nIndex );
    Sequence< PropertyValue > aSeq;
    lcl_FillFontAttributes( aSeq, implGetRuler().GetDrawingArea()->get_ref_device().GetFont() );
    return aSeq;
}

css::awt::Rectangle SAL_CALL ScAccessibleCsvRuler::getCharacterBounds( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndexWithEnd( nIndex );
    ScCsvRuler& rRuler = implGetRuler();
    Point aPos( rRuler.GetX( lcl_GetRulerPos( nIndex ) ) - rRuler.GetCharWidth() / 2, 0 );
    css::awt::Rectangle aRect( aPos.X(), aPos.Y(), rRuler.GetCharWidth(), rRuler.GetOutputSizePixel().Height() );
    // do not return rectangle out of window
    sal_Int32 nWidth = rRuler.GetOutputSizePixel().Width();
    if( aRect.X >= nWidth )
        throw IndexOutOfBoundsException();
    if( aRect.X + aRect.Width > nWidth )
        aRect.Width = nWidth - aRect.X;
    return aRect;
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getCharacterCount()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetTextLength();
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getIndexAtPoint( const css::awt::Point& rPoint )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ScCsvRuler& rRuler = implGetRuler();
    // use object's coordinate system, convert to API position
    return lcl_GetApiPos( ::std::clamp( rRuler.GetPosFromX( rPoint.X ), sal_Int32(0), rRuler.GetPosCount() ) );
}

OUString SAL_CALL ScAccessibleCsvRuler::getSelectedText()
{
    ensureAlive();
    return OUString();
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getSelectionStart()
{
    ensureAlive();
    return -1;
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getSelectionEnd()
{
    ensureAlive();
    return -1;
}

sal_Bool SAL_CALL ScAccessibleCsvRuler::setSelection( sal_Int32 /* nStartIndex */, sal_Int32 /* nEndIndex */ )
{
    ensureAlive();
    return false;
}

OUString SAL_CALL ScAccessibleCsvRuler::getText()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return OUString(maBuffer.subView( 0, implGetTextLength() ));
}

OUString SAL_CALL ScAccessibleCsvRuler::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidRange( nStartIndex, nEndIndex );
    return OUString( maBuffer.getStr() + nStartIndex, nEndIndex - nStartIndex );
}

TextSegment SAL_CALL ScAccessibleCsvRuler::getTextAtIndex( sal_Int32 nIndex, sal_Int16 nTextType )
{
    SolarMutexGuard aGuard;
    ensureAlive();

    TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;

    if( (nIndex == implGetTextLength()) && (nTextType != AccessibleTextType::LINE) )
        return aResult;

    ensureValidIndex( nIndex );

    OUStringBuffer aResultText;     // will be assigned to aResult.SegmentText below
    sal_Int32 nRulerPos = lcl_GetRulerPos( nIndex );

    switch( nTextType )
    {
        // single character
        case AccessibleTextType::CHARACTER:
        {
            aResult.SegmentStart = nIndex;
            aResult.SegmentEnd = nIndex;
            o3tl::iterateCodePoints(maBuffer, &aResult.SegmentEnd);
            for (; nIndex < aResult.SegmentEnd; nIndex++)
                aResultText.append(maBuffer[nIndex]);
        }
        break;

        // entire number or single dot/line
        case AccessibleTextType::WORD:
        case AccessibleTextType::GLYPH:
            aResult.SegmentStart = nIndex;
            if( nRulerPos % 10 )
                aResultText.append(maBuffer[nIndex]);
            else
                aResultText.append( nRulerPos );    // string representation of sal_Int32!!!
        break;

        // entire text
        case AccessibleTextType::SENTENCE:
        case AccessibleTextType::PARAGRAPH:
        case AccessibleTextType::LINE:
            aResult.SegmentStart = 0;
            aResultText.append( maBuffer.getStr(), implGetTextLength() );
        break;

        // equal-formatted text
        case AccessibleTextType::ATTRIBUTE_RUN:
        {
            sal_Int32 nFirstIndex = implGetFirstEqualFormatted( nIndex );
            sal_Int32 nLastIndex = implGetLastEqualFormatted( nIndex );
            aResult.SegmentStart = nFirstIndex;
            aResultText.append( maBuffer.getStr() + nFirstIndex, nLastIndex - nFirstIndex + 1 );
        }
        break;

        default:
            throw RuntimeException();
    }

    aResult.SegmentText = aResultText.makeStringAndClear();
    aResult.SegmentEnd = aResult.SegmentStart + aResult.SegmentText.getLength();
    return aResult;
}

TextSegment SAL_CALL ScAccessibleCsvRuler::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 nTextType )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndexWithEnd( nIndex );

    TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;

    sal_Int32 nRulerPos = lcl_GetRulerPos( nIndex );

    switch( nTextType )
    {
        // single character
        case AccessibleTextType::CHARACTER:
            if( nIndex > 0 )
            {
                o3tl::iterateCodePoints(maBuffer, &nIndex, -1);
                aResult = getTextAtIndex(nIndex, nTextType);
            }
            // else empty
        break;

        // entire number or single dot/line
        case AccessibleTextType::WORD:
        case AccessibleTextType::GLYPH:
            if( nRulerPos > 0 )
                aResult = getTextAtIndex( lcl_GetApiPos( nRulerPos - 1 ), nTextType );
            // else empty
        break;

        // entire text
        case AccessibleTextType::SENTENCE:
        case AccessibleTextType::PARAGRAPH:
        case AccessibleTextType::LINE:
            // empty
        break;

        // equal-formatted text
        case AccessibleTextType::ATTRIBUTE_RUN:
        {
            sal_Int32 nFirstIndex = implGetFirstEqualFormatted( nIndex );
            if( nFirstIndex > 0 )
                aResult = getTextAtIndex( nFirstIndex - 1, nTextType );
            // else empty
        }
        break;

        default:
            throw RuntimeException();
    }
    return aResult;
}

TextSegment SAL_CALL ScAccessibleCsvRuler::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 nTextType )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndexWithEnd( nIndex );

    TextSegment aResult;
    aResult.SegmentStart = -1;
    aResult.SegmentEnd = -1;

    sal_Int32 nRulerPos = lcl_GetRulerPos( nIndex );
    sal_Int32 nLastValid = implGetTextLength();

    switch( nTextType )
    {
        // single character
        case AccessibleTextType::CHARACTER:
            if( nIndex < nLastValid )
            {
                o3tl::iterateCodePoints(maBuffer, &nIndex);
                aResult = getTextAtIndex(nIndex, nTextType);
            }
            // else empty
        break;

        // entire number or single dot/line
        case AccessibleTextType::WORD:
        case AccessibleTextType::GLYPH:
            if( nRulerPos < implGetRuler().GetPosCount() )
                aResult = getTextAtIndex( lcl_GetApiPos( nRulerPos + 1 ), nTextType );
            // else empty
        break;

        // entire text
        case AccessibleTextType::SENTENCE:
        case AccessibleTextType::PARAGRAPH:
        case AccessibleTextType::LINE:
            // empty
        break;

        // equal-formatted text
        case AccessibleTextType::ATTRIBUTE_RUN:
        {
            sal_Int32 nLastIndex = implGetLastEqualFormatted( nIndex );
            if( nLastIndex < nLastValid )
                aResult = getTextAtIndex( nLastIndex + 1, nTextType );
            // else empty
        }
        break;

        default:
            throw RuntimeException();
    }
    return aResult;
}

sal_Bool SAL_CALL ScAccessibleCsvRuler::copyText( sal_Int32 /* nStartIndex */, sal_Int32 /* nEndIndex */ )
{
    ensureAlive();
    return false;
}

sal_Bool SAL_CALL ScAccessibleCsvRuler::scrollSubstringTo( sal_Int32 /* nStartIndex */, sal_Int32/* nEndIndex */, AccessibleScrollType /* aScrollType */ )
{
    return false;
}

// events ---------------------------------------------------------------------

void ScAccessibleCsvRuler::SendCaretEvent()
{
    sal_Int32 nPos = implGetRuler().GetRulerCursorPos();
    if (nPos != CSV_POS_INVALID)
    {
        Any aOldValue, aNewValue;
        aNewValue <<= nPos;
        NotifyAccessibleEvent( AccessibleEventId::CARET_CHANGED, aOldValue, aNewValue );
    }
}

// helpers --------------------------------------------------------------------

OUString SAL_CALL ScAccessibleCsvRuler::getAccessibleName()
{
    return ScResId( STR_ACC_CSVRULER_NAME );
}

OUString SAL_CALL ScAccessibleCsvRuler::getAccessibleDescription()
{
    return ScResId( STR_ACC_CSVRULER_DESCR );
}

void ScAccessibleCsvRuler::ensureValidIndex( sal_Int32 nIndex ) const
{
    if( (nIndex < 0) || (nIndex >= implGetTextLength()) )
        throw IndexOutOfBoundsException();
}

void ScAccessibleCsvRuler::ensureValidIndexWithEnd( sal_Int32 nIndex ) const
{
    if( (nIndex < 0) || (nIndex > implGetTextLength()) )
        throw IndexOutOfBoundsException();
}

void ScAccessibleCsvRuler::ensureValidRange( sal_Int32& rnStartIndex, sal_Int32& rnEndIndex ) const
{
    if( rnStartIndex > rnEndIndex )
        ::std::swap( rnStartIndex, rnEndIndex );
    if( (rnStartIndex < 0) || (rnEndIndex > implGetTextLength()) )
        throw IndexOutOfBoundsException();
}

ScCsvRuler& ScAccessibleCsvRuler::implGetRuler() const
{
    return static_cast< ScCsvRuler& >( implGetControl() );
}

void ScAccessibleCsvRuler::constructStringBuffer()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    // extend existing string buffer to new ruler size
    sal_Int32 nRulerCount = implGetRuler().GetPosCount();
    sal_Int32 nRulerPos = lcl_GetRulerPos( maBuffer.getLength() );
    for( ; nRulerPos <= nRulerCount; ++nRulerPos ) // include last position
    {
        switch( nRulerPos % 10 )
        {
            case 0:     maBuffer.append( nRulerPos );   break;
            case 5:     maBuffer.append( cRulerLine );  break;
            default:    maBuffer.append( cRulerDot );
        }
    }
}

sal_Int32 ScAccessibleCsvRuler::implGetTextLength() const
{
    return lcl_GetApiPos( implGetRuler().GetPosCount() + 1 );
}

bool ScAccessibleCsvRuler::implHasSplit( sal_Int32 nApiPos )
{
    sal_Int32 nRulerPos = lcl_GetRulerPos( nApiPos );
    return implGetRuler().HasSplit( nRulerPos ) && (nApiPos == lcl_GetApiPos( nRulerPos ));
}

sal_Int32 ScAccessibleCsvRuler::implGetFirstEqualFormatted( sal_Int32 nApiPos )
{
    bool bSplit = implHasSplit( nApiPos );
    while( (nApiPos > 0) && (implHasSplit( nApiPos - 1 ) == bSplit) )
        --nApiPos;
    return nApiPos;
}

sal_Int32 ScAccessibleCsvRuler::implGetLastEqualFormatted( sal_Int32 nApiPos )
{
    bool bSplit = implHasSplit( nApiPos );
    sal_Int32 nLength = implGetTextLength();
    while( (nApiPos < nLength - 1) && (implHasSplit( nApiPos + 1 ) == bSplit) )
        ++nApiPos;
    return nApiPos;
}

css::uno::Reference<css::accessibility::XAccessible> SAL_CALL ScAccessibleCsvRuler::getAccessibleParent()
{
    return implGetControl().GetDrawingArea()->get_accessible_parent();
}

// Grid =======================================================================

/** Converts a grid columnm index to an API column index. */
static sal_Int32 lcl_GetApiColumn( sal_uInt32 nGridColumn )
{
    return (nGridColumn != CSV_COLUMN_HEADER) ? static_cast< sal_Int32 >( nGridColumn + 1 ) : 0;
}

/** Converts an API columnm index to a ScCsvGrid column index. */
static sal_uInt32 lcl_GetGridColumn( sal_Int32 nApiColumn )
{
    return (nApiColumn > 0) ? static_cast< sal_uInt32 >( nApiColumn - 1 ) : CSV_COLUMN_HEADER;
}

ScAccessibleCsvGrid::ScAccessibleCsvGrid(ScCsvGrid& rGrid)
    : ImplInheritanceHelper(rGrid)
{
}

ScAccessibleCsvGrid::~ScAccessibleCsvGrid()
{
    ensureDisposed();
}

void ScAccessibleCsvGrid::disposing()
{
    SolarMutexGuard aGuard;
    for (auto& rEntry : maAccessibleChildren)
        rEntry.second->dispose();
    maAccessibleChildren.clear();
    ScAccessibleCsvControl::disposing();
}

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getAccessibleAtPoint( const css::awt::Point& rPoint )
{
    Reference< XAccessible > xRet;
    if( containsPoint( rPoint ) )
    {
        SolarMutexGuard aGuard;
        ensureAlive();

        const ScCsvGrid& rGrid = implGetGrid();
        // #102679#; use <= instead of <, because the offset is the size and not the point
        sal_Int32 nColumn = ((rGrid.GetFirstX() <= rPoint.X) && (rPoint.X <= rGrid.GetLastX())) ?
            lcl_GetApiColumn( rGrid.GetColumnFromX( rPoint.X ) ) : 0;
        sal_Int32 nRow = (rPoint.Y >= rGrid.GetHdrHeight()) ?
            (rGrid.GetLineFromY( rPoint.Y ) - rGrid.GetFirstVisLine() + 1) : 0;
        xRet = getAccessibleCell(nRow, nColumn);
    }
    return xRet;
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getForeground(  )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return sal_Int32(Application::GetSettings().GetStyleSettings().GetButtonTextColor());
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getBackground(  )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return sal_Int32(SC_MOD()->GetColorConfig().GetColorValue( ::svtools::DOCCOLOR ).nColor);
}

// XAccessibleContext ---------------------------------------------------------

sal_Int64 SAL_CALL ScAccessibleCsvGrid::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetCellCount();
}

Reference<XAccessible> ScAccessibleCsvGrid::getAccessibleCell(sal_Int32 nRow, sal_Int32 nColumn)
{
    sal_Int64 nIndex = implGetIndex(nRow, nColumn);

    XAccessibleSet::iterator aI = maAccessibleChildren.lower_bound(nIndex);
    if (aI != maAccessibleChildren.end() && !(maAccessibleChildren.key_comp()(nIndex, aI->first)))
    {
        // key already exists
        return aI->second;
    }
    // key does not exist
    rtl::Reference<ScAccessibleCsvCell> xNew = implCreateCellObj(nRow, nColumn);
    maAccessibleChildren.insert(aI, XAccessibleSet::value_type(nIndex, xNew));
    return xNew;
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getAccessibleChild( sal_Int64 nIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nIndex );

    return getAccessibleCell(implGetRow(nIndex), implGetColumn(nIndex));
}

Reference< XAccessibleRelationSet > SAL_CALL ScAccessibleCsvGrid::getAccessibleRelationSet()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    rtl::Reference<AccessibleRelationSetHelper> pRelationSet = new AccessibleRelationSetHelper();

    ScCsvGrid& rGrid = implGetGrid();
    ScCsvTableBox* pTableBox = rGrid.GetTableBox();
    ScCsvRuler& rRuler = pTableBox->GetRuler();

    if (rRuler.IsVisible())
    {
        css::uno::Reference<css::accessibility::XAccessible> xAccObj(static_cast<ScAccessibleCsvGrid*>(rRuler.GetAccessible()));
        if( xAccObj.is() )
        {
            Sequence<Reference<css::accessibility::XAccessible>> aSeq{ xAccObj };
            pRelationSet->AddRelation( AccessibleRelation( AccessibleRelationType::CONTROLLED_BY, aSeq ) );
        }
    }

    return pRelationSet;
}

sal_Int64 SAL_CALL ScAccessibleCsvGrid::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;
    sal_Int64 nStateSet = implCreateStateSet();
    if( isAlive() )
    {
        nStateSet |= AccessibleStateType::FOCUSABLE;
        nStateSet |= AccessibleStateType::MULTI_SELECTABLE;
        nStateSet |= AccessibleStateType::MANAGES_DESCENDANTS;
        if( implGetGrid().HasFocus() )
            nStateSet |= AccessibleStateType::FOCUSED;
    }
    else
        nStateSet |= AccessibleStateType::DEFUNC;
    return nStateSet;
}

// XAccessibleTable -----------------------------------------------------------

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleRowCount()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetRowCount();
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleColumnCount()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetColumnCount();
}

OUString SAL_CALL ScAccessibleCsvGrid::getAccessibleRowDescription( sal_Int32 nRow )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidPosition( nRow, 0 );
    return implGetCellText( nRow, 0 );
}

OUString SAL_CALL ScAccessibleCsvGrid::getAccessibleColumnDescription( sal_Int32 nColumn )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidPosition( 0, nColumn );
    return implGetCellText( 0, nColumn );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
{
    ensureAlive();
    ensureValidPosition( nRow, nColumn );
    return 1;
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
{
    ensureAlive();
    ensureValidPosition( nRow, nColumn );
    return 1;
}

Reference< XAccessibleTable > SAL_CALL ScAccessibleCsvGrid::getAccessibleRowHeaders()
{
    ensureAlive();
    return nullptr;
}

Reference< XAccessibleTable > SAL_CALL ScAccessibleCsvGrid::getAccessibleColumnHeaders()
{
    ensureAlive();
    return nullptr;
}

Sequence< sal_Int32 > SAL_CALL ScAccessibleCsvGrid::getSelectedAccessibleRows()
{
    ensureAlive();
    return Sequence< sal_Int32 >();
}

Sequence< sal_Int32 > SAL_CALL ScAccessibleCsvGrid::getSelectedAccessibleColumns()
{
    SolarMutexGuard aGuard;
    ensureAlive();

    ScCsvGrid& rGrid = implGetGrid();
    Sequence< sal_Int32 > aSeq( implGetColumnCount() );
    auto pSeq = aSeq.getArray();

    sal_Int32 nSeqIx = 0;
    sal_uInt32 nColIx = rGrid.GetFirstSelected();
    for( ; nColIx != CSV_COLUMN_INVALID; ++nSeqIx, nColIx = rGrid.GetNextSelected( nColIx ) )
        pSeq[ nSeqIx ] = lcl_GetApiColumn( nColIx );

    aSeq.realloc( nSeqIx );
    return aSeq;
}

sal_Bool SAL_CALL ScAccessibleCsvGrid::isAccessibleRowSelected( sal_Int32 /* nRow */ )
{
    ensureAlive();
    return false;
}

sal_Bool SAL_CALL ScAccessibleCsvGrid::isAccessibleColumnSelected( sal_Int32 nColumn )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nColumn );
    return implIsColumnSelected( nColumn );
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidPosition( nRow, nColumn );
    return getAccessibleCell(nRow, nColumn);
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getAccessibleCaption()
{
    ensureAlive();
    return nullptr;
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getAccessibleSummary()
{
    ensureAlive();
    return nullptr;
}

sal_Bool SAL_CALL ScAccessibleCsvGrid::isAccessibleSelected( sal_Int32 /* nRow */, sal_Int32 nColumn )
{
    return isAccessibleColumnSelected( nColumn );
}

sal_Int64 SAL_CALL ScAccessibleCsvGrid::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidPosition( nRow, nColumn );
    return implGetIndex( nRow, nColumn );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleRow( sal_Int64 nChildIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nChildIndex );
    return implGetRow( nChildIndex );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleColumn( sal_Int64 nChildIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nChildIndex );
    return implGetColumn( nChildIndex );
}

// XAccessibleSelection -------------------------------------------------------

void SAL_CALL ScAccessibleCsvGrid::selectAccessibleChild( sal_Int64 nChildIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nChildIndex );
    sal_Int32 nColumn = implGetColumn( nChildIndex );
    if( nChildIndex == 0 )
        implGetGrid().SelectAll();
    else
        implSelectColumn( nColumn, true );
}

sal_Bool SAL_CALL ScAccessibleCsvGrid::isAccessibleChildSelected( sal_Int64 nChildIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nChildIndex );
    sal_Int32 nColumn = implGetColumn( nChildIndex );
    return implIsColumnSelected( nColumn );
}

void SAL_CALL ScAccessibleCsvGrid::clearAccessibleSelection()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    implGetGrid().SelectAll( false );
}

void SAL_CALL ScAccessibleCsvGrid::selectAllAccessibleChildren()
{
    selectAccessibleChild( 0 );
}

sal_Int64 SAL_CALL ScAccessibleCsvGrid::getSelectedAccessibleChildCount()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return static_cast<sal_Int64>(implGetRowCount()) * static_cast<sal_Int64>(implGetSelColumnCount());
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    sal_Int32 nColumns = implGetSelColumnCount();
    if( nColumns == 0 )
        throw IndexOutOfBoundsException();

    sal_Int32 nRow = nSelectedChildIndex / nColumns;
    sal_Int32 nColumn = implGetSelColumn( nSelectedChildIndex % nColumns );
    return getAccessibleCellAt( nRow, nColumn );
}

void SAL_CALL ScAccessibleCsvGrid::deselectAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex(nSelectedChildIndex);
    sal_Int32 nColumns = implGetSelColumnCount();
    if( nColumns == 0 )
        throw IndexOutOfBoundsException();

    sal_Int32 nColumn = implGetSelColumn( nSelectedChildIndex % nColumns );
    ensureValidPosition( nSelectedChildIndex / nColumns, nColumn );
    if( nColumn > 0 )
        implSelectColumn( nColumn, false );
}

// events ---------------------------------------------------------------------

void ScAccessibleCsvGrid::SendFocusEvent( bool bFocused )
{
    ScAccessibleCsvControl::SendFocusEvent( bFocused );
    Any aOldAny, aNewAny;
    (bFocused ? aNewAny : aOldAny) <<=
        getAccessibleCellAt( 0, lcl_GetApiColumn( implGetGrid().GetFocusColumn() ) );
    NotifyAccessibleEvent(AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny);
}

void ScAccessibleCsvGrid::SendTableUpdateEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn, bool bAllRows )
{
    if( nFirstColumn <= nLastColumn )
    {
        AccessibleTableModelChange aModelChange(
            AccessibleTableModelChangeType::UPDATE, 0, bAllRows ? implGetRowCount() - 1 : 0,
            lcl_GetApiColumn( nFirstColumn ), lcl_GetApiColumn( nLastColumn ) );
        Any aOldAny, aNewAny;
        aNewAny <<= aModelChange;
        NotifyAccessibleEvent(AccessibleEventId::TABLE_MODEL_CHANGED, aOldAny, aNewAny);
    }
}

void ScAccessibleCsvGrid::SendInsertColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn )
{
    if( nFirstColumn <= nLastColumn )
    {
        AccessibleTableModelChange aModelChange(
            AccessibleTableModelChangeType::COLUMNS_INSERTED, -1, -1,
            lcl_GetApiColumn( nFirstColumn ), lcl_GetApiColumn( nLastColumn ) );
        Any aOldAny, aNewAny;
        aNewAny <<= aModelChange;
        NotifyAccessibleEvent(AccessibleEventId::TABLE_MODEL_CHANGED, aOldAny, aNewAny);
    }
}

void ScAccessibleCsvGrid::SendRemoveColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn )
{
    if( nFirstColumn <= nLastColumn )
    {
        AccessibleTableModelChange aModelChange(
            AccessibleTableModelChangeType::COLUMNS_REMOVED, -1, -1,
            lcl_GetApiColumn( nFirstColumn ), lcl_GetApiColumn( nLastColumn ) );
        Any aOldAny, aNewAny;
        aNewAny <<= aModelChange;
        NotifyAccessibleEvent(AccessibleEventId::TABLE_MODEL_CHANGED, aOldAny, aNewAny);
    }
}

// helpers --------------------------------------------------------------------

OUString SAL_CALL ScAccessibleCsvGrid::getAccessibleName()
{
    return ScResId( STR_ACC_CSVGRID_NAME );
}

OUString SAL_CALL ScAccessibleCsvGrid::getAccessibleDescription()
{
    return ScResId( STR_ACC_CSVGRID_DESCR );
}

void ScAccessibleCsvGrid::ensureValidIndex( sal_Int64 nIndex ) const
{
    if( (nIndex < 0) || (nIndex >= implGetCellCount()) )
        throw IndexOutOfBoundsException();
}

void ScAccessibleCsvGrid::ensureValidPosition( sal_Int32 nRow, sal_Int32 nColumn ) const
{
    if( (nRow < 0) || (nRow >= implGetRowCount()) || (nColumn < 0) || (nColumn >= implGetColumnCount()) )
        throw IndexOutOfBoundsException();
}

ScCsvGrid& ScAccessibleCsvGrid::implGetGrid() const
{
    return static_cast< ScCsvGrid& >( implGetControl() );
}

bool ScAccessibleCsvGrid::implIsColumnSelected( sal_Int32 nColumn ) const
{
    return (nColumn > 0) && implGetGrid().IsSelected( lcl_GetGridColumn( nColumn ) );
}

void ScAccessibleCsvGrid::implSelectColumn( sal_Int32 nColumn, bool bSelect )
{
    if( nColumn > 0 )
        implGetGrid().Select( lcl_GetGridColumn( nColumn ), bSelect );
}

sal_Int32 ScAccessibleCsvGrid::implGetRowCount() const
{
    return static_cast< sal_Int32 >( implGetGrid().GetLastVisLine() - implGetGrid().GetFirstVisLine() + 2 );
}

sal_Int32 ScAccessibleCsvGrid::implGetColumnCount() const
{
    return static_cast< sal_Int32 >( implGetGrid().GetColumnCount() + 1 );
}

sal_Int32 ScAccessibleCsvGrid::implGetSelColumnCount() const
{
    ScCsvGrid& rGrid = implGetGrid();
    sal_Int32 nCount = 0;
    for( sal_uInt32 nColIx = rGrid.GetFirstSelected(); nColIx != CSV_COLUMN_INVALID; nColIx = rGrid.GetNextSelected( nColIx ) )
        ++nCount;
    return nCount;
}

sal_Int32 ScAccessibleCsvGrid::implGetSelColumn( sal_Int32 nSelColumn ) const
{
    ScCsvGrid& rGrid = implGetGrid();
    sal_Int32 nColumn = 0;
    for( sal_uInt32 nColIx = rGrid.GetFirstSelected(); nColIx != CSV_COLUMN_INVALID; nColIx = rGrid.GetNextSelected( nColIx ) )
    {
        if( nColumn == nSelColumn )
            return static_cast< sal_Int32 >( nColIx + 1 );
        ++nColumn;
    }
    return 0;
}

OUString ScAccessibleCsvGrid::implGetCellText( sal_Int32 nRow, sal_Int32 nColumn ) const
{
    ScCsvGrid& rGrid = implGetGrid();
    sal_Int32 nLine = nRow + rGrid.GetFirstVisLine() - 1;
    OUString aCellStr;
    if( (nColumn > 0) && (nRow > 0) )
        aCellStr = rGrid.GetCellText( lcl_GetGridColumn( nColumn ), nLine );
    else if( nRow > 0 )
        aCellStr = OUString::number( nLine + 1 );
    else if( nColumn > 0 )
        aCellStr = rGrid.GetColumnTypeName( lcl_GetGridColumn( nColumn ) );
    return aCellStr;
}

rtl::Reference<ScAccessibleCsvCell> ScAccessibleCsvGrid::implCreateCellObj( sal_Int32 nRow, sal_Int32 nColumn )
{
    return new ScAccessibleCsvCell(implGetGrid(), implGetCellText(nRow, nColumn), nRow, nColumn);
}

css::uno::Reference<css::accessibility::XAccessible> SAL_CALL ScAccessibleCsvGrid::getAccessibleParent()
{
    return implGetControl().GetDrawingArea()->get_accessible_parent();
}

ScAccessibleCsvCell::ScAccessibleCsvCell(
        ScCsvGrid& rGrid,
        OUString aCellText,
        sal_Int32 nRow, sal_Int32 nColumn ) :
    ImplInheritanceHelper( rGrid ),
    AccessibleStaticTextBase( SvxEditSourcePtr() ),
    maCellText(std::move( aCellText )),
    mnLine( nRow ? (nRow + rGrid.GetFirstVisLine() - 1) : CSV_LINE_HEADER ),
    mnColumn( lcl_GetGridColumn( nColumn ) ),
    mnIndex( nRow * (rGrid.GetColumnCount() + 1) + nColumn )
{
    SetEditSource( implCreateEditSource() );
}

ScAccessibleCsvCell::~ScAccessibleCsvCell()
{
}

void SAL_CALL ScAccessibleCsvCell::disposing()
{
    SolarMutexGuard aGuard;
    SetEditSource( SvxEditSourcePtr() );
    ScAccessibleCsvControl::disposing();
}

// XAccessibleComponent -------------------------------------------------------

void SAL_CALL ScAccessibleCsvCell::grabFocus()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ScCsvGrid& rGrid = implGetGrid();
    rGrid.Execute( CSVCMD_MOVEGRIDCURSOR, rGrid.GetColumnPos( mnColumn ) );
}

sal_Int32 SAL_CALL ScAccessibleCsvCell::getForeground(  )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return sal_Int32(Application::GetSettings().GetStyleSettings().GetButtonTextColor());
}

sal_Int32 SAL_CALL ScAccessibleCsvCell::getBackground(  )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return sal_Int32(SC_MOD()->GetColorConfig().GetColorValue( ::svtools::DOCCOLOR ).nColor);
}

// XAccessibleContext -----------------------------------------------------

sal_Int64 SAL_CALL ScAccessibleCsvCell::getAccessibleChildCount()
{
    return AccessibleStaticTextBase::getAccessibleChildCount();
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvCell::getAccessibleChild( sal_Int64 nIndex )
{
    return AccessibleStaticTextBase::getAccessibleChild( nIndex );
}

sal_Int64 SAL_CALL ScAccessibleCsvCell::getAccessibleIndexInParent()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return mnIndex;
}

Reference< XAccessibleRelationSet > SAL_CALL ScAccessibleCsvCell::getAccessibleRelationSet()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return new AccessibleRelationSetHelper();
}

sal_Int64 SAL_CALL ScAccessibleCsvCell::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;
    sal_Int64 nStateSet = implCreateStateSet();
    if( isAlive() )
    {
        const ScCsvGrid& rGrid = implGetGrid();
        nStateSet |= AccessibleStateType::SINGLE_LINE;
        if( mnColumn != CSV_COLUMN_HEADER )
            nStateSet |= AccessibleStateType::SELECTABLE;
        if( rGrid.HasFocus() && (rGrid.GetFocusColumn() == mnColumn) && (mnLine == CSV_LINE_HEADER) )
            nStateSet |= AccessibleStateType::ACTIVE;
        if( rGrid.IsSelected( mnColumn ) )
            nStateSet |= AccessibleStateType::SELECTED;
    }
    return nStateSet;
}

// XInterface -----------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( ScAccessibleCsvCell, ImplInheritanceHelper, AccessibleStaticTextBase )

// XTypeProvider --------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( ScAccessibleCsvCell, ImplInheritanceHelper, AccessibleStaticTextBase )

// helpers --------------------------------------------------------------------

OUString SAL_CALL ScAccessibleCsvCell::getAccessibleName()
{
    return maCellText;
}

OUString SAL_CALL ScAccessibleCsvCell::getAccessibleDescription()
{
    return OUString();
}

ScCsvGrid& ScAccessibleCsvCell::implGetGrid() const
{
    return static_cast< ScCsvGrid& >( implGetControl() );
}

Point ScAccessibleCsvCell::implGetRealPos() const
{
    ScCsvGrid& rGrid = implGetGrid();
    return Point(
        (mnColumn == CSV_COLUMN_HEADER) ? rGrid.GetHdrX() : rGrid.GetColumnX( mnColumn ),
        (mnLine == CSV_LINE_HEADER) ? 0 : rGrid.GetY( mnLine ) );
}

sal_uInt32 ScAccessibleCsvCell::implCalcPixelWidth(sal_uInt32 nChars) const
{
    ScCsvGrid& rGrid = implGetGrid();
    return rGrid.GetCharWidth() * nChars;
}

Size ScAccessibleCsvCell::implGetRealSize() const
{
    ScCsvGrid& rGrid = implGetGrid();
    return Size(
        (mnColumn == CSV_COLUMN_HEADER) ? rGrid.GetHdrWidth() : implCalcPixelWidth( rGrid.GetColumnWidth( mnColumn ) ),
        (mnLine == CSV_LINE_HEADER) ? rGrid.GetHdrHeight() : rGrid.GetLineHeight() );
}

css::awt::Rectangle ScAccessibleCsvCell::implGetBounds()
{
    ScCsvGrid& rGrid = implGetGrid();
    tools::Rectangle aClipRect( Point( 0, 0 ), rGrid.GetOutputSizePixel() );
    if( mnColumn != CSV_COLUMN_HEADER )
    {
        aClipRect.SetLeft( rGrid.GetFirstX() );
        aClipRect.SetRight( rGrid.GetLastX() );
    }
    if( mnLine != CSV_LINE_HEADER )
         aClipRect.SetTop( rGrid.GetHdrHeight() );

    tools::Rectangle aRect( implGetRealPos(), implGetRealSize() );
    aRect.Intersection( aClipRect );
    if( aRect.IsEmpty() )
        aRect.SetSize( Size( -1, -1 ) );

    return css::awt::Rectangle(aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight());
}

::std::unique_ptr< SvxEditSource > ScAccessibleCsvCell::implCreateEditSource()
{
    ScCsvGrid& rGrid = implGetGrid();

    ::std::unique_ptr< SvxEditSource > pEditSource( new ScAccessibilityEditSource( std::make_unique<ScAccessibleCsvTextData>(&rGrid.GetDrawingArea()->get_ref_device(), rGrid.GetEditEngine(), maCellText, implGetRealSize()) ) );
    return pEditSource;
}

css::uno::Reference<css::accessibility::XAccessible> SAL_CALL ScAccessibleCsvCell::getAccessibleParent()
{
    ScCsvGrid& rGrid = implGetGrid();

    ScAccessibleCsvGrid* pAcc = static_cast<ScAccessibleCsvGrid*>(rGrid.GetAccessible());

    return pAcc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
