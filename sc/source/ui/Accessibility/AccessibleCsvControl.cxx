/*************************************************************************
 *
 *  $RCSfile: AccessibleCsvControl.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:02:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// ============================================================================

#ifndef _SC_ACCESSIBLECSVCONTROL_HXX
#include "AccessibleCsvControl.hxx"
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLERELATIONTYPE_HPP_
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETEXTTYPE_HPP_
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETABLEMODELCHANGE_HPP_
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETABLEMODELCHANGETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX_
#include <unotools/accessiblerelationsethelper.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif

#ifndef _SC_CSVCONTROL_HXX
#include "csvcontrol.hxx"
#endif
#ifndef _SC_CSVRULER_HXX
#include "csvruler.hxx"
#endif
#ifndef _SC_CSVGRID_HXX
#include "csvgrid.hxx"
#endif
#ifndef _SC_ACCESSIBLETEXT_HXX
#include "AccessibleText.hxx"
#endif
#ifndef SC_EDITSRC_HXX
#include "editsrc.hxx"
#endif

#ifndef SC_UNOGUARD_HXX
#include "unoguard.hxx"
#endif

#ifndef SC_SCRESID_HXX
#include "scresid.hxx"
#endif
#include "sc.hrc"
#ifndef SC_SCMOD_HXX
#include "scmod.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

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

#define CREATE_OUSTRING( name )     OUString( RTL_CONSTASCII_USTRINGPARAM( name ) )

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
    ScUnoGuard aGuard;
    mpControl = NULL;
    ScAccessibleContextBase::disposing();
}


// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL ScAccessibleCsvControl::getAccessibleAtPoint( const AwtPoint& rPoint )
        throw( RuntimeException )
{
    ensureAlive();
    return NULL;
}

sal_Bool SAL_CALL ScAccessibleCsvControl::isVisible() throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    return implGetControl().IsVisible();
}

void SAL_CALL ScAccessibleCsvControl::grabFocus() throw( RuntimeException )
{
    ScUnoGuard aGuard;
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
    DBG_ERRORFILE( "ScAccessibleCsvControl::SendCaretEvent - Illegal call" );
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

void ScAccessibleCsvControl::SendTableUpdateEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn, bool bAllRows )
{
    DBG_ERRORFILE( "ScAccessibleCsvControl::SendTableUpdateEvent - Illegal call" );
}

void ScAccessibleCsvControl::SendInsertColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn )
{
    DBG_ERRORFILE( "ScAccessibleCsvControl::SendInsertColumnEvent - Illegal call" );
}

void ScAccessibleCsvControl::SendRemoveColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn )
{
    DBG_ERRORFILE( "ScAccessibleCsvControl::SendRemoveColumnEvent - Illegal call" );
}


// helpers --------------------------------------------------------------------

Rectangle ScAccessibleCsvControl::GetBoundingBoxOnScreen() const throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    return implGetControl().GetWindowExtentsRelative( NULL );
}

Rectangle ScAccessibleCsvControl::GetBoundingBox() const throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    return implGetControl().GetWindowExtentsRelative( implGetControl().GetAccessibleParentWindow() );
}

void ScAccessibleCsvControl::getUuid( Sequence< sal_Int8 >& rSeq )
{
    ScUnoGuard aGuard;
    ensureAlive();
    if( !rSeq.hasElements() )
    {
        rSeq.realloc( 16 );
        rtl_createUuid( reinterpret_cast< sal_uInt8* >( rSeq.getArray() ), NULL, sal_True );
    }
}

void ScAccessibleCsvControl::ensureAlive() const throw( DisposedException )
{
    if( !implIsAlive() )
        throw DisposedException();
}

ScCsvControl& ScAccessibleCsvControl::implGetControl() const
{
    DBG_ASSERT( mpControl, "ScAccessibleCsvControl::implGetControl - missing control" );
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
    ScUnoGuard aGuard;
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
        osl_incrementInterlockedCount( &m_refCount );
        dispose();
    }
}

Point ScAccessibleCsvControl::implGetRelPos( const Point& rPos ) const
{
    return rPos - implGetControl().GetWindowExtentsRelative( NULL ).TopLeft();
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
    return ::std::max( nApiPos, 0L );
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
    DBG_ASSERT( nExp > 0, "lcl_ExpandSequence - invalid value" );
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
    SvxFontItem aFontItem( rFont.GetFamily(), rFont.GetName(), rFont.GetStyleName(), rFont.GetPitch(), rFont.GetCharSet() );
    SvxFontHeightItem aHeightItem( rFont.GetSize().Height() );
    SvxLanguageItem aLangItem( rFont.GetLanguage() );

    sal_Int32 nIndex = lcl_ExpandSequence( rSeq, 7 );
    lcl_FillProperty( rSeq[ nIndex++ ], CREATE_OUSTRING( "CharFontName" ),      aFontItem,   MID_FONT_FAMILY_NAME );
    lcl_FillProperty( rSeq[ nIndex++ ], CREATE_OUSTRING( "CharFontFamily" ),    aFontItem,   MID_FONT_FAMILY );
    lcl_FillProperty( rSeq[ nIndex++ ], CREATE_OUSTRING( "CharFontStyleName" ), aFontItem,   MID_FONT_STYLE_NAME );
    lcl_FillProperty( rSeq[ nIndex++ ], CREATE_OUSTRING( "CharFontCharSet" ),   aFontItem,   MID_FONT_PITCH );
    lcl_FillProperty( rSeq[ nIndex++ ], CREATE_OUSTRING( "CharFontPitch" ),     aFontItem,   MID_FONT_CHAR_SET );
    lcl_FillProperty( rSeq[ nIndex++ ], CREATE_OUSTRING( "CharHeight" ),        aHeightItem, MID_FONTHEIGHT );
    lcl_FillProperty( rSeq[ nIndex++ ], CREATE_OUSTRING( "CharLocale" ),        aLangItem,   MID_LANG_LOCALE );
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
    ScUnoGuard aGuard;
    ensureAlive();
    return implGetRuler().GetSettings().GetStyleSettings().GetLabelTextColor().GetColor();
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getBackground(  )
    throw (RuntimeException)
{
    ScUnoGuard aGuard;
    ensureAlive();
    return implGetRuler().GetSettings().GetStyleSettings().GetFaceColor().GetColor();
}

// XAccessibleContext ---------------------------------------------------------

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getAccessibleChildCount() throw( RuntimeException )
{
    ensureAlive();
    return 0;
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvRuler::getAccessibleChild( sal_Int32 nIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ensureAlive();
    throw IndexOutOfBoundsException();
}

Reference< XAccessibleRelationSet > SAL_CALL ScAccessibleCsvRuler::getAccessibleRelationSet()
        throw( RuntimeException )
{
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
    ensureAlive();
    return lcl_GetApiPos( implGetRuler().GetRulerCursorPos() );
}

sal_Bool SAL_CALL ScAccessibleCsvRuler::setCaretPosition( sal_Int32 nIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
    ensureAlive();
    ensureValidIndex( nIndex );
    return maBuffer.charAt( nIndex );
}

Sequence< PropertyValue > SAL_CALL ScAccessibleCsvRuler::getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    ensureValidIndexWithEnd( nIndex );
    Sequence< PropertyValue > aSeq;
    lcl_FillFontAttributes( aSeq, implGetRuler().GetFont() );
//! TODO split attribute: waiting for #102221#
//    if( implHasSplit( nIndex ) )
//    {
//        sal_Int32 nIndex = lcl_ExpandSequence( aSeq, 1 );
//        aSeq[ nIndex ].Name = CREATE_OUSTRING( "..." );
//        aSeq[ nIndex ].Value <<= ...;
//    }
    return aSeq;
}

ScAccessibleCsvRuler::AwtRectangle SAL_CALL ScAccessibleCsvRuler::getCharacterBounds( sal_Int32 nIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    ensureValidIndexWithEnd( nIndex );
    ScCsvRuler& rRuler = implGetRuler();
    Point aPos( rRuler.GetX( lcl_GetRulerPos( nIndex ) ) - rRuler.GetCharWidth() / 2, 0 );
    AwtRectangle aRect( aPos.X(), aPos.Y(), rRuler.GetCharWidth(), rRuler.GetSizePixel().Height() );
    // #107054# do not return rectangle out of window
    sal_Int32 nWidth = rRuler.GetOutputSizePixel().Width();
    if( aRect.X >= nWidth )
        throw IndexOutOfBoundsException();
    if( aRect.X + aRect.Width > nWidth )
        aRect.Width = nWidth - aRect.X;
    return aRect;
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getCharacterCount() throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    return implGetTextLength();
}

sal_Int32 SAL_CALL ScAccessibleCsvRuler::getIndexAtPoint( const AwtPoint& rPoint )
        throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    ScCsvRuler& rRuler = implGetRuler();
    // #107054# use object's coordinate system, convert to API position
    return lcl_GetApiPos( ::std::min( ::std::max( rRuler.GetPosFromX( rPoint.X ), 0L ), rRuler.GetPosCount() ) );
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

sal_Bool SAL_CALL ScAccessibleCsvRuler::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ensureAlive();
    return sal_False;
}

OUString SAL_CALL ScAccessibleCsvRuler::getText() throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    return OUString( maBuffer.getStr(), implGetTextLength() );
}

OUString SAL_CALL ScAccessibleCsvRuler::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    ensureValidRange( nStartIndex, nEndIndex );
    return OUString( maBuffer.getStr() + nStartIndex, nEndIndex - nStartIndex );
}

TextSegment SAL_CALL ScAccessibleCsvRuler::getTextAtIndex( sal_Int32 nIndex, sal_Int16 nTextType )
        throw( IndexOutOfBoundsException, IllegalArgumentException, RuntimeException )
{
    ScUnoGuard aGuard;
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
            aResultText.append( maBuffer.charAt( nIndex ) );
        }
        break;

        // entire number or single dot/line
        case AccessibleTextType::WORD:
        case AccessibleTextType::GLYPH:
            aResult.SegmentStart = nIndex;
            if( nRulerPos % 10 )
                aResultText.append( maBuffer.charAt( nIndex ) );
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
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
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

sal_Bool SAL_CALL ScAccessibleCsvRuler::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ensureAlive();
    return sal_False;
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
    return CREATE_OUSTRING( RULER_IMPL_NAME );
}


// XTypeProvider --------------------------------------------------------------

Sequence< ::com::sun::star::uno::Type > SAL_CALL ScAccessibleCsvRuler::getTypes() throw( RuntimeException )
{
    Sequence< ::com::sun::star::uno::Type > aSeq( 1 );
    aSeq[ 0 ] = getCppuType( static_cast< const Reference< XAccessibleText >* >( NULL ) );
    return ::comphelper::concatSequences( ScAccessibleCsvControl::getTypes(), aSeq );
}

Sequence< sal_Int8 > SAL_CALL ScAccessibleCsvRuler::getImplementationId() throw( RuntimeException )
{
    static Sequence< sal_Int8 > aSeq;
    getUuid( aSeq );
    return aSeq;
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
    ScUnoGuard aGuard;
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
        ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
    ensureAlive();
    return implGetGrid().GetSettings().GetStyleSettings().GetButtonTextColor().GetColor();
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getBackground(  )
throw (RuntimeException)
{
    ScUnoGuard aGuard;
    ensureAlive();
    return SC_MOD()->GetColorConfig().GetColorValue( ::svtools::DOCCOLOR ).nColor;
}

// XAccessibleContext ---------------------------------------------------------

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleChildCount() throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    return implGetCellCount();
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getAccessibleChild( sal_Int32 nIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    ensureValidIndex( nIndex );
    return implCreateCellObj( implGetRow( nIndex ), implGetColumn( nIndex ) );
}

Reference< XAccessibleRelationSet > SAL_CALL ScAccessibleCsvGrid::getAccessibleRelationSet()
        throw( RuntimeException )
{
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
    ensureAlive();
    return implGetRowCount();
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleColumnCount() throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    return implGetColumnCount();
}

OUString SAL_CALL ScAccessibleCsvGrid::getAccessibleRowDescription( sal_Int32 nRow )
        throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    ensureValidPosition( nRow, 0 );
    return implGetCellText( nRow, 0 );
}

OUString SAL_CALL ScAccessibleCsvGrid::getAccessibleColumnDescription( sal_Int32 nColumn )
        throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    ensureValidPosition( 0, nColumn );
    return implGetCellText( 0, nColumn );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw( RuntimeException )
{
    ensureAlive();
    ensureValidPosition( nRow, nColumn );
    return 1;
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw( RuntimeException )
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
    ScUnoGuard aGuard;
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

sal_Bool SAL_CALL ScAccessibleCsvGrid::isAccessibleRowSelected( sal_Int32 nRow )
        throw( RuntimeException )
{
    ensureAlive();
    return sal_False;
}

sal_Bool SAL_CALL ScAccessibleCsvGrid::isAccessibleColumnSelected( sal_Int32 nColumn )
        throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    ensureValidIndex( nColumn );
    return implIsColumnSelected( nColumn );
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ScUnoGuard aGuard;
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

sal_Bool SAL_CALL ScAccessibleCsvGrid::isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
        throw( RuntimeException )
{
    return isAccessibleColumnSelected( nColumn );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    ensureValidPosition( nRow, nColumn );
    return implGetIndex( nRow, nColumn );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleRow( sal_Int32 nChildIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    ensureValidIndex( nChildIndex );
    return implGetRow( nChildIndex );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getAccessibleColumn( sal_Int32 nChildIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    ensureValidIndex( nChildIndex );
    return implGetColumn( nChildIndex );
}


// XAccessibleSelection -------------------------------------------------------

void SAL_CALL ScAccessibleCsvGrid::selectAccessibleChild( sal_Int32 nChildIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
    ensureAlive();
    ensureValidIndex( nChildIndex );
    sal_Int32 nColumn = implGetColumn( nChildIndex );
    return implIsColumnSelected( nColumn );
}

void SAL_CALL ScAccessibleCsvGrid::clearAccessibleSelection() throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    implGetGrid().SelectAll( false );
}

void SAL_CALL ScAccessibleCsvGrid::selectAllAccessibleChildren() throw( RuntimeException )
{
    selectAccessibleChild( 0 );
}

sal_Int32 SAL_CALL ScAccessibleCsvGrid::getSelectedAccessibleChildCount() throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    return implGetRowCount() * implGetSelColumnCount();
}

Reference< XAccessible > SAL_CALL ScAccessibleCsvGrid::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw( IndexOutOfBoundsException, RuntimeException )
{
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
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
    return CREATE_OUSTRING( GRID_IMPL_NAME );
}


// XTypeProvider --------------------------------------------------------------

Sequence< ::com::sun::star::uno::Type > SAL_CALL ScAccessibleCsvGrid::getTypes() throw( RuntimeException )
{
    Sequence< ::com::sun::star::uno::Type > aSeq( 2 );
    aSeq[ 0 ] = getCppuType( static_cast< const Reference< XAccessibleTable >* >( NULL ) );
    aSeq[ 1 ] = getCppuType( static_cast< const Reference< XAccessibleSelection >* >( NULL ) );
    return ::comphelper::concatSequences( ScAccessibleCsvControl::getTypes(), aSeq );
}

Sequence< sal_Int8 > SAL_CALL ScAccessibleCsvGrid::getImplementationId() throw( RuntimeException )
{
    static Sequence< sal_Int8 > aSeq;
    getUuid( aSeq );
    return aSeq;
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
    ScUnoGuard aGuard;
    SetEditSource( SvxEditSourcePtr( NULL ) );
    ScAccessibleCsvControl::disposing();
}


// XAccessibleComponent -------------------------------------------------------

void SAL_CALL ScAccessibleCsvCell::grabFocus() throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    ScCsvGrid& rGrid = implGetGrid();
    rGrid.Execute( CSVCMD_MOVEGRIDCURSOR, rGrid.GetColumnPos( mnColumn ) );
}

sal_Int32 SAL_CALL ScAccessibleCsvCell::getForeground(  )
throw (RuntimeException)
{
    ScUnoGuard aGuard;
    ensureAlive();
    return implGetGrid().GetSettings().GetStyleSettings().GetButtonTextColor().GetColor();
}

sal_Int32 SAL_CALL ScAccessibleCsvCell::getBackground(  )
throw (RuntimeException)
{
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
    ensureAlive();
    return mnIndex;
}

Reference< XAccessibleRelationSet > SAL_CALL ScAccessibleCsvCell::getAccessibleRelationSet()
        throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    return new AccessibleRelationSetHelper();
}

Reference< XAccessibleStateSet > SAL_CALL ScAccessibleCsvCell::getAccessibleStateSet()
        throw( RuntimeException )
{
    ScUnoGuard aGuard;
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

Any SAL_CALL ScAccessibleCsvCell::queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( RuntimeException )
{
    Any aAny;
    if( rType == getCppuType( static_cast< const Reference< XAccessibleText >* >( NULL ) ) )
        aAny <<= Reference< XAccessibleText >( this );
    else
        aAny <<= ScAccessibleCsvControl::queryInterface( rType );
    return aAny;
}

void SAL_CALL ScAccessibleCsvCell::acquire() throw()
{
    ScAccessibleCsvControl::acquire();
}

void SAL_CALL ScAccessibleCsvCell::release() throw()
{
    ScAccessibleCsvControl::release();
}

// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL ScAccessibleCsvCell::getImplementationName() throw( RuntimeException )
{
    return CREATE_OUSTRING( CELL_IMPL_NAME );
}


// XTypeProvider --------------------------------------------------------------

Sequence< ::com::sun::star::uno::Type > SAL_CALL ScAccessibleCsvCell::getTypes() throw( RuntimeException )
{
    Sequence< ::com::sun::star::uno::Type > aSeq( 1 );
    aSeq[ 0 ] = getCppuType( static_cast< const Reference< XAccessibleText >* >( NULL ) );
    return ::comphelper::concatSequences( ScAccessibleCsvControl::getTypes(), aSeq );
}

Sequence< sal_Int8 > SAL_CALL ScAccessibleCsvCell::getImplementationId() throw( RuntimeException )
{
    static Sequence< sal_Int8 > aSeq;
    getUuid( aSeq );
    return aSeq;
}


// helpers --------------------------------------------------------------------

Rectangle ScAccessibleCsvCell::GetBoundingBoxOnScreen() const throw( RuntimeException )
{
    ScUnoGuard aGuard;
    ensureAlive();
    Rectangle aRect( implGetBoundingBox() );
    aRect.SetPos( implGetAbsPos( aRect.TopLeft() ) );
    return aRect;
}

Rectangle ScAccessibleCsvCell::GetBoundingBox() const throw( RuntimeException )
{
    ScUnoGuard aGuard;
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


// ============================================================================

