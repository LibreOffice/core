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

#include "AccessibleCsvControl.hxx"
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <tools/debug.hxx>
#include <toolkit/helper/convert.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include "scitems.hxx"
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/langitem.hxx>
#include "csvcontrol.hxx"
#include "csvruler.hxx"
#include "csvgrid.hxx"
#include "AccessibleText.hxx"
#include "editsrc.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "scmod.hxx"
#include <svtools/colorcfg.hxx>
#include <vcl/svapp.hxx>
// ause
#include "editutil.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::utl::AccessibleRelationSetHelper;
using ::utl::AccessibleStateSetHelper;
using ::accessibility::AccessibleStaticTextBase;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::lang::DisposedException;
using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::beans::PropertyValue;
using namespace ::com::sun::star::accessibility;


// ----------------------------------------------------------------------------

const sal_uInt16 nRulerRole         = AccessibleRole::TEXT;
const sal_uInt16 nGridRole          = AccessibleRole::TABLE;
const sal_uInt16 nCellRole          = AccessibleRole::TEXT;

#define RULER_IMPL_NAME             "ScAccessibleCsvRuler"
#define GRID_IMPL_NAME              "ScAccessibleCsvGrid"
#define CELL_IMPL_NAME              "ScAccessibleCsvCell"

const sal_Unicode cRulerDot         = '.';
const sal_Unicode cRulerLine        = '|';

const sal_Int32 CSV_LINE_HEADER     = CSV_POS_INVALID;
const sal_uInt32 CSV_COLUMN_HEADER  = CSV_COLUMN_INVALID;


// CSV base control ===========================================================

DBG_NAME( ScAccessibleCsvControl )

ScAccessibleCsvControl::ScAccessibleCsvControl(
        const Reference< XAccessible >& rxParent,
        ScCsvControl& rControl,
        sal_uInt16 nRole ) :
    ScAccessibleContextBase( rxParent, nRole ),
    mpControl( &rControl )
{
    DBG_CTOR( ScAccessibleCsvControl, NULL );
}

ScAccessibleCsvControl::~ScAccessibleCsvControl()
{
    DBG_DTOR( ScAccessibleCsvControl, NULL );
    implDispose();
}

void SAL_CALL ScAccessibleCsvControl::disposing()
{
    SolarMutexGuard aGuard;
    mpControl = NULL;
    ScAccessibleContextBase::disposing();
}


// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL ScAccessibleCsvControl::getAccessibleAtPoint( const AwtPoint& /* rPoint */ )
        throw( RuntimeException )
{
    ensureAlive();
    return NULL;
}

sal_Bool SAL_CALL ScAccessibleCsvControl::isVisible() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetControl().IsVisible();
}

void SAL_CALL ScAccessibleCsvControl::grabFocus() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    implGetControl().GrabFocus();
}


// events ---------------------------------------------------------------------

void ScAccessibleCsvControl::SendFocusEvent( bool bFocused )
{
    if( bFocused )
        CommitFocusGained();
    else
        CommitFocusLost();
}

void ScAccessibleCsvControl::SendCaretEvent()
{
    OSL_FAIL( "ScAccessibleCsvControl::SendCaretEvent - Illegal call" );
}

void ScAccessibleCsvControl::SendVisibleEvent()
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::VISIBLE_DATA_CHANGED;
    aEvent.Source = Reference< XAccessible >( this );
    CommitChange( aEvent );
}

void ScAccessibleCsvControl::SendSelectionEvent()
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::SELECTION_CHANGED;
    aEvent.Source = Reference< XAccessible >( this );
    CommitChange( aEvent );
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

Rectangle ScAccessibleCsvControl::GetBoundingBoxOnScreen() const throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetControl().GetWindowExtentsRelative( NULL );
}

Rectangle ScAccessibleCsvControl::GetBoundingBox() const throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetControl().GetWindowExtentsRelative( implGetControl().GetAccessibleParentWindow() );
}

void ScAccessibleCsvControl::ensureAlive() const throw( DisposedException )
{
    if( !implIsAlive() )
        throw DisposedException();
}

ScCsvControl& ScAccessibleCsvControl::implGetControl() const
{
    OSL_ENSURE( mpControl, "ScAccessibleCsvControl::implGetControl - missing control" );
    return *mpControl;
}

Reference< XAccessible > ScAccessibleCsvControl::implGetChildByRole(
        const Reference< XAccessible >& rxParentObj, sal_uInt16 nRole ) throw( RuntimeException )
{
    Reference< XAccessible > xAccObj;
    if( rxParentObj.is() )
    {
        Reference< XAccessibleContext > xParentCtxt = rxParentObj->getAccessibleContext();
        if( xParentCtxt.is() )
        {
            sal_Int32 nCount = xParentCtxt->getAccessibleChildCount();
            sal_Int32 nIndex = 0;
            while( !xAccObj.is() && (nIndex < nCount) )
            {
                Reference< XAccessible > xCurrObj = xParentCtxt->getAccessibleChild( nIndex );
                if( xCurrObj.is() )
                {
                    Reference< XAccessibleContext > xCurrCtxt = xCurrObj->getAccessibleContext();
                    if( xCurrCtxt.is() && (xCurrCtxt->getAccessibleRole() == nRole) )
                        xAccObj = xCurrObj;
                }
                ++nIndex;
            }
        }
    }
    return xAccObj;
}

AccessibleStateSetHelper* ScAccessibleCsvControl::implCreateStateSet()
{
    SolarMutexGuard aGuard;
    AccessibleStateSetHelper* pStateSet = new AccessibleStateSetHelper();
    if( implIsAlive() )
    {
        const ScCsvControl& rCtrl = implGetControl();
        pStateSet->AddState( AccessibleStateType::OPAQUE );
        if( rCtrl.IsEnabled() )
            pStateSet->AddState( AccessibleStateType::ENABLED );
        if( isShowing() )
            pStateSet->AddState( AccessibleStateType::SHOWING );
        if( isVisible() )
            pStateSet->AddState( AccessibleStateType::VISIBLE );
    }
    else
        pStateSet->AddState( AccessibleStateType::DEFUNC );
    return pStateSet;
}

void ScAccessibleCsvControl::implDispose()
{
    if( implIsAlive() )
    {
        // prevent multiple call of dtor
        osl_atomic_increment( &m_refCount );
        dispose();
    }
}

Point ScAccessibleCsvControl::implGetAbsPos( const Point& rPos ) const
{
    return rPos + implGetControl().GetWindowExtentsRelative( NULL ).TopLeft();
}


// Ruler ======================================================================

/** Converts a ruler cursor position to API text index. */
sal_Int32 lcl_GetApiPos( sal_Int32 nRulerPos )
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
sal_Int32 lcl_GetRulerPos( sal_Int32 nApiPos )
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
    return nRulerPos + nRelPos / nDiv * 10 + ::std::max( nRelPos % nDiv - nDiv + 10L, 0L );
}

/** Expands the sequence's size and returns the base index of the new inserted elements. */
inline sal_Int32 lcl_ExpandSequence( Sequence< PropertyValue >& rSeq, sal_Int32 nExp )
{
    OSL_ENSURE( nExp > 0, "lcl_ExpandSequence - invalid value" );
    rSeq.realloc( rSeq.getLength() + nExp );
    return rSeq.getLength() - nExp;
}

/** Fills the property value rVal with the specified name and value from the item. */
inline void lcl_FillProperty( PropertyValue& rVal, const OUString& rPropName, const SfxPoolItem& rItem, sal_uInt8 nMID )
{
    rVal.Name = rPropName;
    rItem.QueryValue( rVal.Value, nMID );
}

/** Fills the sequence with all font attributes of rFont. */
void lcl_FillFontAttributes( Sequence< PropertyValue >& rSeq, const Font& rFont )
{
    SvxFontItem aFontItem( rFont.GetFamily(), rFont.GetName(), rFont.GetStyleName(), rFont.GetPitch(), rFont.GetCharSet(), ATTR_FONT );
    SvxFontHeightItem aHeightItem( rFont.GetSize().Height(), 100, ATTR_FONT_HEIGHT );
    SvxLanguageItem aLangItem( rFont.GetLanguage(), ATTR_FONT_LANGUAGE );

    sal_Int32 nIndex = lcl_ExpandSequence( rSeq, 7 );
    lcl_FillProperty( rSeq[ nIndex++ ], "CharFontName",      aFontItem,   MID_FONT_FAMILY_NAME );
    lcl_FillProperty( rSeq[ nIndex++ ], "CharFontFamily",    aFontItem,   MID_FONT_FAMILY );
    lcl_FillProperty( rSeq[ nIndex++ ], "CharFontStyleName", aFontItem,   MID_FONT_STYLE_NAME );
    lcl_FillProperty( rSeq[ nIndex++ ], "CharFontCharSet",   aFontItem,   MID_FONT_PITCH );
    lcl_FillProperty( rSeq[ nIndex++ ], "CharFontPitch",     aFontItem,   MID_FONT_CHAR_SET );
    lcl_FillProperty( rSeq[ nIndex++ ], "CharHeight",        aHeightItem, MID_FONTHEIGHT );
    lcl_FillProperty( rSeq[ nIndex++ ], "CharLocale",        aLangItem,   MID_LANG_LOCALE );
}



// ----------------------------------------------------------------------------

DBG_NAME( ScAccessibleCsvRuler )

ScAccessibleCsvRuler::ScAccessibleCsvRuler( ScCsvRuler& rRuler ) :
    ScAccessibleCsvControl( rRuler.GetAccessibleParentWindow()->GetAccessible(), rRuler, nRulerRole )
{
    DBG_CTOR( ScAccessibleCsvRuler, NULL );
    constructStringBuffer();
}

ScAccessibleCsvRuler::~ScAccessibleCsvRuler()
{
    DBG_DTOR( ScAccessibleCsvRuler, NULL );
    implDispose();
}

// XAccessibleComponent -----------------------------------------------------

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getForeground(  )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetRuler().GetSettings().GetStyleSettings().GetLabelTextColor().GetColor();
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getBackground(  )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetRuler().GetSettings().GetStyleSettings().GetFaceColor().GetColor();
}

// XAccessibleContext ---------------------------------------------------------

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getAccessibleChildCount() throw( RuntimeException )
{
    ensureAlive();
    return 0;
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvRuler::getAccessibleChild( sal_Int32 /* nIndex */ )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ensureAlive();
    throw IndexOutOfBoundsException();
}

Reference< XAccessibleRelationSet > SAL_CALL ScAccessibleCsvRuler::getAccessibleRelationSet()
        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    AccessibleRelationSetHelper* pRelationSet = new AccessibleRelationSetHelper();
    Reference< XAccessible > xAccObj = implGetChildByRole( getAccessibleParent(), nGridRole );
    if( xAccObj.is() )
    {
        Sequence< Reference< XInterface > > aSeq( 1 );
        aSeq[ 0 ] = xAccObj;
        pRelationSet->AddRelation( AccessibleRelation( AccessibleRelationType::CONTROLLER_FOR, aSeq ) );
    }
    return pRelationSet;
}

Reference< XAccessibleStateSet > SAL_CALL ScAccessibleCsvRuler::getAccessibleStateSet()
        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    AccessibleStateSetHelper* pStateSet = implCreateStateSet();
    if( implIsAlive() )
    {
        pStateSet->AddState( AccessibleStateType::FOCUSABLE );
        pStateSet->AddState( AccessibleStateType::SINGLE_LINE );
        if( implGetRuler().HasFocus() )
            pStateSet->AddState( AccessibleStateType::FOCUSED );
    }
    return pStateSet;
}


// XAccessibleText ------------------------------------------------------------

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getCaretPosition() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return lcl_GetApiPos( implGetRuler().GetRulerCursorPos() );
}

sal_Bool SAL_CALL ScAccessibleCsvRuler::setCaretPosition( sal_Int32 nIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
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
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nIndex );
    return maBuffer[nIndex];
}

Sequence< PropertyValue > SAL_CALL ScAccessibleCsvRuler::getCharacterAttributes( sal_Int32 nIndex,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* aRequestedAttributes */ )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndexWithEnd( nIndex );
    Sequence< PropertyValue > aSeq;
    lcl_FillFontAttributes( aSeq, implGetRuler().GetFont() );
    return aSeq;
}

ScAccessibleCsvRuler::AwtRectangle SAL_CALL ScAccessibleCsvRuler::getCharacterBounds( sal_Int32 nIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndexWithEnd( nIndex );
    ScCsvRuler& rRuler = implGetRuler();
    Point aPos( rRuler.GetX( lcl_GetRulerPos( nIndex ) ) - rRuler.GetCharWidth() / 2, 0 );
    AwtRectangle aRect( aPos.X(), aPos.Y(), rRuler.GetCharWidth(), rRuler.GetSizePixel().Height() );
    // do not return rectangle out of window
    sal_Int32 nWidth = rRuler.GetOutputSizePixel().Width();
    if( aRect.X >= nWidth )
        throw IndexOutOfBoundsException();
    if( aRect.X + aRect.Width > nWidth )
        aRect.Width = nWidth - aRect.X;
    return aRect;
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getCharacterCount() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetTextLength();
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getIndexAtPoint( const AwtPoint& rPoint )
        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ScCsvRuler& rRuler = implGetRuler();
    // use object's coordinate system, convert to API position
    return lcl_GetApiPos( ::std::min( ::std::max( rRuler.GetPosFromX( rPoint.X ), static_cast<sal_Int32>(0) ), rRuler.GetPosCount() ) );
}

OUString SAL_CALL ScAccessibleCsvRuler::getSelectedText() throw( RuntimeException )
{
    ensureAlive();
    return OUString();
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getSelectionStart() throw( RuntimeException )
{
    ensureAlive();
    return -1;
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getSelectionEnd() throw( RuntimeException )
{
    ensureAlive();
    return -1;
}

sal_Bool SAL_CALL ScAccessibleCsvRuler::setSelection( sal_Int32 /* nStartIndex */, sal_Int32 /* nEndIndex */ )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ensureAlive();
    return false;
}

OUString SAL_CALL ScAccessibleCsvRuler::getText() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return OUString( maBuffer.getStr(), implGetTextLength() );
}

OUString SAL_CALL ScAccessibleCsvRuler::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidRange( nStartIndex, nEndIndex );
    return OUString( maBuffer.getStr() + nStartIndex, nEndIndex - nStartIndex );
}

TextSegment SAL_CALL ScAccessibleCsvRuler::getTextAtIndex( sal_Int32 nIndex, sal_Int16 nTextType )
        throw( IndexOutOfBoundsException, IllegalArgumentException, RuntimeException )
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
        throw( IndexOutOfBoundsException, IllegalArgumentException, RuntimeException )
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
                aResult = getTextAtIndex( nIndex - 1, nTextType );
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
        throw( IndexOutOfBoundsException, IllegalArgumentException, RuntimeException )
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
                aResult = getTextAtIndex( nIndex + 1, nTextType );
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
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ensureAlive();
    return false;
}


// XInterface -----------------------------------------------------------------

Any SAL_CALL ScAccessibleCsvRuler::queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( RuntimeException )
{
    Any aAny( ScAccessibleCsvRulerImpl::queryInterface( rType ) );
    return aAny.hasValue() ? aAny : ScAccessibleCsvControl::queryInterface( rType );
}

void SAL_CALL ScAccessibleCsvRuler::acquire() throw ()
{
    ScAccessibleCsvControl::acquire();
}

void SAL_CALL ScAccessibleCsvRuler::release() throw ()
{
    ScAccessibleCsvControl::release();
}


// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL ScAccessibleCsvRuler::getImplementationName() throw( RuntimeException )
{
    return OUString( RULER_IMPL_NAME );
}


// XTypeProvider --------------------------------------------------------------

Sequence< ::com::sun::star::uno::Type > SAL_CALL ScAccessibleCsvRuler::getTypes() throw( RuntimeException )
{
    Sequence< ::com::sun::star::uno::Type > aSeq( 1 );
    aSeq[ 0 ] = getCppuType( static_cast< const Reference< XAccessibleText >* >( NULL ) );
    return ::comphelper::concatSequences( ScAccessibleCsvControl::getTypes(), aSeq );
}

namespace
{
    class theScAccessibleCsvRulerImplementationId : public rtl::Static< UnoTunnelIdInit, theScAccessibleCsvRulerImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL ScAccessibleCsvRuler::getImplementationId() throw( RuntimeException )
{
    return theScAccessibleCsvRulerImplementationId::get().getSeq();
}


// events ---------------------------------------------------------------------

void ScAccessibleCsvRuler::SendCaretEvent()
{
    sal_Int32 nPos = implGetRuler().GetRulerCursorPos();
    if( nPos != CSV_POS_INVALID )
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CARET_CHANGED;
        aEvent.Source = Reference< XAccessible >( this );
        aEvent.NewValue <<= nPos;
        CommitChange( aEvent );
    }
}


// helpers --------------------------------------------------------------------

OUString SAL_CALL ScAccessibleCsvRuler::createAccessibleName() throw( RuntimeException )
{
    return String( ScResId( STR_ACC_CSVRULER_NAME ) );
}

OUString SAL_CALL ScAccessibleCsvRuler::createAccessibleDescription() throw( RuntimeException )
{
    return String( ScResId( STR_ACC_CSVRULER_DESCR ) );
}

void ScAccessibleCsvRuler::ensureValidIndex( sal_Int32 nIndex ) const
        throw( IndexOutOfBoundsException )
{
    if( (nIndex < 0) || (nIndex >= implGetTextLength()) )
        throw IndexOutOfBoundsException();
}

void ScAccessibleCsvRuler::ensureValidIndexWithEnd( sal_Int32 nIndex ) const
        throw( IndexOutOfBoundsException )
{
    if( (nIndex < 0) || (nIndex > implGetTextLength()) )
        throw IndexOutOfBoundsException();
}

void ScAccessibleCsvRuler::ensureValidRange( sal_Int32& rnStartIndex, sal_Int32& rnEndIndex ) const
        throw( IndexOutOfBoundsException )
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

void ScAccessibleCsvRuler::constructStringBuffer() throw( RuntimeException )
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


// Grid =======================================================================

/** Converts a grid columnm index to an API column index. */
inline sal_Int32 lcl_GetApiColumn( sal_uInt32 nGridColumn )
{
    return (nGridColumn != CSV_COLUMN_HEADER) ? static_cast< sal_Int32 >( nGridColumn + 1 ) : 0;
}

/** Converts an API columnm index to a ScCsvGrid column index. */
inline sal_uInt32 lcl_GetGridColumn( sal_Int32 nApiColumn )
{
    return (nApiColumn > 0) ? static_cast< sal_uInt32 >( nApiColumn - 1 ) : CSV_COLUMN_HEADER;
}


// ----------------------------------------------------------------------------

DBG_NAME( ScAccessibleCsvGrid )

ScAccessibleCsvGrid::ScAccessibleCsvGrid( ScCsvGrid& rGrid ) :
    ScAccessibleCsvControl( rGrid.GetAccessibleParentWindow()->GetAccessible(), rGrid, nGridRole )
{
    DBG_CTOR( ScAccessibleCsvGrid, NULL );
}

ScAccessibleCsvGrid::~ScAccessibleCsvGrid()
{
    DBG_DTOR( ScAccessibleCsvGrid, NULL );
    implDispose();
}


// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getAccessibleAtPoint( const AwtPoint& rPoint )
        throw( RuntimeException )
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
        xRet = implCreateCellObj( nRow, nColumn );
    }
    return xRet;
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getForeground(  )
throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetGrid().GetSettings().GetStyleSettings().GetButtonTextColor().GetColor();
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getBackground(  )
throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return SC_MOD()->GetColorConfig().GetColorValue( ::svtools::DOCCOLOR ).nColor;
}

// XAccessibleContext ---------------------------------------------------------

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleChildCount() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetCellCount();
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getAccessibleChild( sal_Int32 nIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nIndex );
    return implCreateCellObj( implGetRow( nIndex ), implGetColumn( nIndex ) );
}

Reference< XAccessibleRelationSet > SAL_CALL ScAccessibleCsvGrid::getAccessibleRelationSet()
        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    AccessibleRelationSetHelper* pRelationSet = new AccessibleRelationSetHelper();
    Reference< XAccessible > xAccObj = implGetChildByRole( getAccessibleParent(), nRulerRole );
    if( xAccObj.is() )
    {
        Sequence< Reference< XInterface > > aSeq( 1 );
        aSeq[ 0 ] = xAccObj;
        pRelationSet->AddRelation( AccessibleRelation( AccessibleRelationType::CONTROLLED_BY, aSeq ) );
    }
    return pRelationSet;
}

Reference< XAccessibleStateSet > SAL_CALL ScAccessibleCsvGrid::getAccessibleStateSet()
        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    AccessibleStateSetHelper* pStateSet = implCreateStateSet();
    if( implIsAlive() )
    {
        pStateSet->AddState( AccessibleStateType::FOCUSABLE );
        pStateSet->AddState( AccessibleStateType::MULTI_SELECTABLE );
        pStateSet->AddState( AccessibleStateType::MANAGES_DESCENDANTS );
        if( implGetGrid().HasFocus() )
            pStateSet->AddState( AccessibleStateType::FOCUSED );
    }
    else
        pStateSet->AddState( AccessibleStateType::DEFUNC );
    return pStateSet;
}


// XAccessibleTable -----------------------------------------------------------

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleRowCount() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetRowCount();
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleColumnCount() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetColumnCount();
}

OUString SAL_CALL ScAccessibleCsvGrid::getAccessibleRowDescription( sal_Int32 nRow )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidPosition( nRow, 0 );
    return implGetCellText( nRow, 0 );
}

OUString SAL_CALL ScAccessibleCsvGrid::getAccessibleColumnDescription( sal_Int32 nColumn )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidPosition( 0, nColumn );
    return implGetCellText( 0, nColumn );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ensureAlive();
    ensureValidPosition( nRow, nColumn );
    return 1;
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ensureAlive();
    ensureValidPosition( nRow, nColumn );
    return 1;
}

Reference< XAccessibleTable > SAL_CALL ScAccessibleCsvGrid::getAccessibleRowHeaders()
        throw( RuntimeException )
{
    ensureAlive();
    return NULL;
}

Reference< XAccessibleTable > SAL_CALL ScAccessibleCsvGrid::getAccessibleColumnHeaders()
        throw( RuntimeException )
{
    ensureAlive();
    return NULL;
}

Sequence< sal_Int32 > SAL_CALL ScAccessibleCsvGrid::getSelectedAccessibleRows()
        throw( RuntimeException )
{
    ensureAlive();
    return Sequence< sal_Int32 >();
}

Sequence< sal_Int32 > SAL_CALL ScAccessibleCsvGrid::getSelectedAccessibleColumns()
        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();

    ScCsvGrid& rGrid = implGetGrid();
    Sequence< sal_Int32 > aSeq( implGetColumnCount() );

    sal_Int32 nSeqIx = 0;
    sal_uInt32 nColIx = rGrid.GetFirstSelected();
    for( ; nColIx != CSV_COLUMN_INVALID; ++nSeqIx, nColIx = rGrid.GetNextSelected( nColIx ) )
        aSeq[ nSeqIx ] = lcl_GetApiColumn( nColIx );

    aSeq.realloc( nSeqIx );
    return aSeq;
}

sal_Bool SAL_CALL ScAccessibleCsvGrid::isAccessibleRowSelected( sal_Int32 /* nRow */ )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ensureAlive();
    return false;
}

sal_Bool SAL_CALL ScAccessibleCsvGrid::isAccessibleColumnSelected( sal_Int32 nColumn )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nColumn );
    return implIsColumnSelected( nColumn );
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidPosition( nRow, nColumn );
    return implCreateCellObj( nRow, nColumn );
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getAccessibleCaption()
        throw( RuntimeException )
{
    ensureAlive();
    return NULL;
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getAccessibleSummary()
        throw( RuntimeException )
{
    ensureAlive();
    return NULL;
}

sal_Bool SAL_CALL ScAccessibleCsvGrid::isAccessibleSelected( sal_Int32 /* nRow */, sal_Int32 nColumn )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    return isAccessibleColumnSelected( nColumn );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidPosition( nRow, nColumn );
    return implGetIndex( nRow, nColumn );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleRow( sal_Int32 nChildIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nChildIndex );
    return implGetRow( nChildIndex );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleColumn( sal_Int32 nChildIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nChildIndex );
    return implGetColumn( nChildIndex );
}


// XAccessibleSelection -------------------------------------------------------

void SAL_CALL ScAccessibleCsvGrid::selectAccessibleChild( sal_Int32 nChildIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
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

sal_Bool SAL_CALL ScAccessibleCsvGrid::isAccessibleChildSelected( sal_Int32 nChildIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ensureValidIndex( nChildIndex );
    sal_Int32 nColumn = implGetColumn( nChildIndex );
    return implIsColumnSelected( nColumn );
}

void SAL_CALL ScAccessibleCsvGrid::clearAccessibleSelection() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    implGetGrid().SelectAll( false );
}

void SAL_CALL ScAccessibleCsvGrid::selectAllAccessibleChildren() throw( RuntimeException )
{
    selectAccessibleChild( 0 );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getSelectedAccessibleChildCount() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetRowCount() * implGetSelColumnCount();
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
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

void SAL_CALL ScAccessibleCsvGrid::deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    sal_Int32 nColumns = implGetSelColumnCount();
    if( nColumns == 0 )
        throw IndexOutOfBoundsException();

    sal_Int32 nColumn = implGetSelColumn( nSelectedChildIndex % nColumns );
    ensureValidPosition( nSelectedChildIndex / nColumns, nColumn );
    if( nColumn > 0 )
        implSelectColumn( nColumn, false );
}


// XInterface -----------------------------------------------------------------

Any SAL_CALL ScAccessibleCsvGrid::queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( RuntimeException )
{
    Any aAny( ScAccessibleCsvGridImpl::queryInterface( rType ) );
    return aAny.hasValue() ? aAny : ScAccessibleCsvControl::queryInterface( rType );
}

void SAL_CALL ScAccessibleCsvGrid::acquire() throw ()
{
    ScAccessibleCsvControl::acquire();
}

void SAL_CALL ScAccessibleCsvGrid::release() throw ()
{
    ScAccessibleCsvControl::release();
}


// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL ScAccessibleCsvGrid::getImplementationName() throw( RuntimeException )
{
    return OUString( GRID_IMPL_NAME );
}


// XTypeProvider --------------------------------------------------------------

Sequence< ::com::sun::star::uno::Type > SAL_CALL ScAccessibleCsvGrid::getTypes() throw( RuntimeException )
{
    Sequence< ::com::sun::star::uno::Type > aSeq( 2 );
    aSeq[ 0 ] = getCppuType( static_cast< const Reference< XAccessibleTable >* >( NULL ) );
    aSeq[ 1 ] = getCppuType( static_cast< const Reference< XAccessibleSelection >* >( NULL ) );
    return ::comphelper::concatSequences( ScAccessibleCsvControl::getTypes(), aSeq );
}

namespace
{
    class theScAccessibleCsvGridImplementationId  : public rtl::Static< UnoTunnelIdInit, theScAccessibleCsvGridImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL ScAccessibleCsvGrid::getImplementationId() throw( RuntimeException )
{
    return theScAccessibleCsvGridImplementationId::get().getSeq();
}


// events ---------------------------------------------------------------------

void ScAccessibleCsvGrid::SendFocusEvent( bool bFocused )
{
    ScAccessibleCsvControl::SendFocusEvent( bFocused );

    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::ACTIVE_DESCENDANT_CHANGED;
    aEvent.Source = Reference< XAccessible >( this );
    (bFocused ? aEvent.NewValue : aEvent.OldValue) <<=
        getAccessibleCellAt( 0, lcl_GetApiColumn( implGetGrid().GetFocusColumn() ) );
    CommitChange( aEvent );
}

void ScAccessibleCsvGrid::SendTableUpdateEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn, bool bAllRows )
{
    if( nFirstColumn <= nLastColumn )
    {
        AccessibleTableModelChange aModelChange(
            AccessibleTableModelChangeType::UPDATE, 0, bAllRows ? implGetRowCount() - 1 : 0,
            lcl_GetApiColumn( nFirstColumn ), lcl_GetApiColumn( nLastColumn ) );
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::TABLE_MODEL_CHANGED;
        aEvent.Source = Reference< XAccessible >( this );
        aEvent.NewValue <<= aModelChange;
        CommitChange( aEvent );
    }
}

void ScAccessibleCsvGrid::SendInsertColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn )
{
    if( nFirstColumn <= nLastColumn )
    {
        AccessibleTableModelChange aModelChange(
            AccessibleTableModelChangeType::INSERT, 0, implGetRowCount() - 1,
            lcl_GetApiColumn( nFirstColumn ), lcl_GetApiColumn( nLastColumn ) );
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::TABLE_MODEL_CHANGED;
        aEvent.Source = Reference< XAccessible >( this );
        aEvent.NewValue <<= aModelChange;
        CommitChange( aEvent );
    }
}

void ScAccessibleCsvGrid::SendRemoveColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn )
{
    if( nFirstColumn <= nLastColumn )
    {
        AccessibleTableModelChange aModelChange(
            AccessibleTableModelChangeType::DELETE, 0, implGetRowCount() - 1,
            lcl_GetApiColumn( nFirstColumn ), lcl_GetApiColumn( nLastColumn ) );
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::TABLE_MODEL_CHANGED;
        aEvent.Source = Reference< XAccessible >( this );
        aEvent.NewValue <<= aModelChange;
        CommitChange( aEvent );
    }
}


// helpers --------------------------------------------------------------------

OUString SAL_CALL ScAccessibleCsvGrid::createAccessibleName() throw( RuntimeException )
{
    return String( ScResId( STR_ACC_CSVGRID_NAME ) );
}

OUString SAL_CALL ScAccessibleCsvGrid::createAccessibleDescription() throw( RuntimeException )
{
    return String( ScResId( STR_ACC_CSVGRID_DESCR ) );
}

void ScAccessibleCsvGrid::ensureValidIndex( sal_Int32 nIndex ) const
        throw( IndexOutOfBoundsException )
{
    if( (nIndex < 0) || (nIndex >= implGetCellCount()) )
        throw IndexOutOfBoundsException();
}

void ScAccessibleCsvGrid::ensureValidPosition( sal_Int32 nRow, sal_Int32 nColumn ) const
        throw( IndexOutOfBoundsException )
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

String ScAccessibleCsvGrid::implGetCellText( sal_Int32 nRow, sal_Int32 nColumn ) const
{
    ScCsvGrid& rGrid = implGetGrid();
    sal_Int32 nLine = nRow + rGrid.GetFirstVisLine() - 1;
    String aCellStr;
    if( (nColumn > 0) && (nRow > 0) )
        aCellStr = rGrid.GetCellText( lcl_GetGridColumn( nColumn ), nLine );
    else if( nRow > 0 )
        aCellStr = String::CreateFromInt32( nLine + 1L );
    else if( nColumn > 0 )
        aCellStr = rGrid.GetColumnTypeName( lcl_GetGridColumn( nColumn ) );
    return aCellStr;
}


ScAccessibleCsvControl* ScAccessibleCsvGrid::implCreateCellObj( sal_Int32 nRow, sal_Int32 nColumn ) const
{
    return new ScAccessibleCsvCell( implGetGrid(), implGetCellText( nRow, nColumn ), nRow, nColumn );
}


// ============================================================================

DBG_NAME( ScAccessibleCsvCell )

ScAccessibleCsvCell::ScAccessibleCsvCell(
        ScCsvGrid& rGrid,
        const String& rCellText,
        sal_Int32 nRow, sal_Int32 nColumn ) :
    ScAccessibleCsvControl( rGrid.GetAccessible(), rGrid, nCellRole ),
    AccessibleStaticTextBase( SvxEditSourcePtr( NULL ) ),
    maCellText( rCellText ),
    mnLine( nRow ? (nRow + rGrid.GetFirstVisLine() - 1) : CSV_LINE_HEADER ),
    mnColumn( lcl_GetGridColumn( nColumn ) ),
    mnIndex( nRow * (rGrid.GetColumnCount() + 1) + nColumn )
{
    DBG_CTOR( ScAccessibleCsvCell, NULL );
    SetEditSource( implCreateEditSource() );
}

ScAccessibleCsvCell::~ScAccessibleCsvCell()
{
    DBG_DTOR( ScAccessibleCsvCell, NULL );
}

void SAL_CALL ScAccessibleCsvCell::disposing()
{
    SolarMutexGuard aGuard;
    SetEditSource( SvxEditSourcePtr( NULL ) );
    ScAccessibleCsvControl::disposing();
}


// XAccessibleComponent -------------------------------------------------------

void SAL_CALL ScAccessibleCsvCell::grabFocus() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    ScCsvGrid& rGrid = implGetGrid();
    rGrid.Execute( CSVCMD_MOVEGRIDCURSOR, rGrid.GetColumnPos( mnColumn ) );
}

sal_Int32 SAL_CALL ScAccessibleCsvCell::getForeground(  )
throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetGrid().GetSettings().GetStyleSettings().GetButtonTextColor().GetColor();
}

sal_Int32 SAL_CALL ScAccessibleCsvCell::getBackground(  )
throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return SC_MOD()->GetColorConfig().GetColorValue( ::svtools::DOCCOLOR ).nColor;
}

// XAccessibleContext -----------------------------------------------------

sal_Int32 SAL_CALL ScAccessibleCsvCell::getAccessibleChildCount() throw( RuntimeException )
{
    return AccessibleStaticTextBase::getAccessibleChildCount();
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvCell::getAccessibleChild( sal_Int32 nIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    return AccessibleStaticTextBase::getAccessibleChild( nIndex );
}

sal_Int32 SAL_CALL ScAccessibleCsvCell::getAccessibleIndexInParent() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return mnIndex;
}

Reference< XAccessibleRelationSet > SAL_CALL ScAccessibleCsvCell::getAccessibleRelationSet()
        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return new AccessibleRelationSetHelper();
}

Reference< XAccessibleStateSet > SAL_CALL ScAccessibleCsvCell::getAccessibleStateSet()
        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    AccessibleStateSetHelper* pStateSet = implCreateStateSet();
    if( implIsAlive() )
    {
        const ScCsvGrid& rGrid = implGetGrid();
        pStateSet->AddState( AccessibleStateType::SINGLE_LINE );
        if( mnColumn != CSV_COLUMN_HEADER )
            pStateSet->AddState( AccessibleStateType::SELECTABLE );
        if( rGrid.HasFocus() && (rGrid.GetFocusColumn() == mnColumn) && (mnLine == CSV_LINE_HEADER) )
            pStateSet->AddState( AccessibleStateType::ACTIVE );
        if( rGrid.IsSelected( mnColumn ) )
            pStateSet->AddState( AccessibleStateType::SELECTED );
    }
    return pStateSet;
}

// XInterface -----------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( ScAccessibleCsvCell, ScAccessibleCsvControl, AccessibleStaticTextBase )

// XTypeProvider --------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( ScAccessibleCsvCell, ScAccessibleCsvControl, AccessibleStaticTextBase )

// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL ScAccessibleCsvCell::getImplementationName() throw( RuntimeException )
{
    return OUString( CELL_IMPL_NAME );
}

// helpers --------------------------------------------------------------------

Rectangle ScAccessibleCsvCell::GetBoundingBoxOnScreen() const throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    Rectangle aRect( implGetBoundingBox() );
    aRect.SetPos( implGetAbsPos( aRect.TopLeft() ) );
    return aRect;
}

Rectangle ScAccessibleCsvCell::GetBoundingBox() const throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return implGetBoundingBox();
}

OUString SAL_CALL ScAccessibleCsvCell::createAccessibleName() throw( RuntimeException )
{
    return maCellText;
}

OUString SAL_CALL ScAccessibleCsvCell::createAccessibleDescription() throw( RuntimeException )
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

Rectangle ScAccessibleCsvCell::implGetBoundingBox() const
{
    ScCsvGrid& rGrid = implGetGrid();
    Rectangle aClipRect( Point( 0, 0 ), rGrid.GetSizePixel() );
    if( mnColumn != CSV_COLUMN_HEADER )
    {
        aClipRect.Left() = rGrid.GetFirstX();
        aClipRect.Right() = rGrid.GetLastX();
    }
    if( mnLine != CSV_LINE_HEADER )
         aClipRect.Top() = rGrid.GetHdrHeight();

    Rectangle aRect( implGetRealPos(), implGetRealSize() );
    aRect.Intersection( aClipRect );
    if( (aRect.GetWidth() <= 0) || (aRect.GetHeight() <= 0) )
        aRect.SetSize( Size( -1, -1 ) );
    return aRect;
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr< SvxEditSource > ScAccessibleCsvCell::implCreateEditSource()
{
    ScCsvGrid& rGrid = implGetGrid();
    Rectangle aBoundRect( implGetBoundingBox() );
    aBoundRect -= implGetRealPos();

    ::std::auto_ptr< ScAccessibleTextData > pCsvTextData( new ScAccessibleCsvTextData(
        &rGrid, rGrid.GetEditEngine(), maCellText, aBoundRect, implGetRealSize() ) );

    ::std::auto_ptr< SvxEditSource > pEditSource( new ScAccessibilityEditSource( pCsvTextData ) );
    return pEditSource;
}
SAL_WNODEPRECATED_DECLARATIONS_POP

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
