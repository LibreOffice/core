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


#include <tools/rc.h>
#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/cursor.hxx>
#include <vcl/virdev.hxx>
#include <vcl/menu.hxx>
#include <vcl/cmdevt.h>
#include <vcl/edit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include <window.h>
#include <svdata.hxx>
#include <svids.hrc>
#include <controldata.hxx>

#include <osl/mutex.hxx>


#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>

#include <com/sun/star/i18n/XExtendedInputSequenceChecker.hpp>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <com/sun/star/uno/Any.hxx>

#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <sal/macros.h>

#include <vcl/unohelp.hxx>
#include <vcl/unohelp2.hxx>




using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

// - Redo
// - Bei Tracking-Cancel DefaultSelection wieder herstellen

// =======================================================================

static FncGetSpecialChars pImplFncGetSpecialChars = NULL;

// =======================================================================

#define EDIT_ALIGN_LEFT             1
#define EDIT_ALIGN_CENTER           2
#define EDIT_ALIGN_RIGHT            3

#define EDIT_DEL_LEFT               1
#define EDIT_DEL_RIGHT              2

#define EDIT_DELMODE_SIMPLE         11
#define EDIT_DELMODE_RESTOFWORD     12
#define EDIT_DELMODE_RESTOFCONTENT  13

// =======================================================================

struct DDInfo
{
    Cursor          aCursor;
    Selection       aDndStartSel;
    xub_StrLen      nDropPos;
    sal_Bool            bStarterOfDD;
    sal_Bool            bDroppedInMe;
    sal_Bool            bVisCursor;
    sal_Bool            bIsStringSupported;

    DDInfo()
    {
        aCursor.SetStyle( CURSOR_SHADOW );
        nDropPos = 0;
        bStarterOfDD = sal_False;
        bDroppedInMe = sal_False;
        bVisCursor = sal_False;
        bIsStringSupported = sal_False;
    }
};

// =======================================================================

struct Impl_IMEInfos
{
    String      aOldTextAfterStartPos;
    sal_uInt16*     pAttribs;
    xub_StrLen  nPos;
    xub_StrLen  nLen;
    sal_Bool        bCursor;
    sal_Bool        bWasCursorOverwrite;

                Impl_IMEInfos( xub_StrLen nPos, const String& rOldTextAfterStartPos );
                ~Impl_IMEInfos();

    void        CopyAttribs( const xub_StrLen* pA, xub_StrLen nL );
    void        DestroyAttribs();
};

// -----------------------------------------------------------------------

Impl_IMEInfos::Impl_IMEInfos( xub_StrLen nP, const String& rOldTextAfterStartPos )
 : aOldTextAfterStartPos( rOldTextAfterStartPos )
{
    nPos = nP;
    nLen = 0;
    bCursor = sal_True;
    pAttribs = NULL;
    bWasCursorOverwrite = sal_False;
}

// -----------------------------------------------------------------------

Impl_IMEInfos::~Impl_IMEInfos()
{
    delete[] pAttribs;
}

// -----------------------------------------------------------------------

void Impl_IMEInfos::CopyAttribs( const xub_StrLen* pA, xub_StrLen nL )
{
    nLen = nL;
    delete[] pAttribs;
    pAttribs = new sal_uInt16[ nL ];
    memcpy( pAttribs, pA, nL*sizeof(sal_uInt16) );
}

// -----------------------------------------------------------------------

void Impl_IMEInfos::DestroyAttribs()
{
    delete[] pAttribs;
    pAttribs = NULL;
    nLen = 0;
}

// =======================================================================

Edit::Edit( WindowType nType ) :
    Control( nType )
{
    ImplInitEditData();
}

// -----------------------------------------------------------------------

Edit::Edit( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_EDIT )
{
    ImplInitEditData();
    ImplInit( pParent, nStyle );
}

Edit::Edit( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_EDIT )
{
    rResId.SetRT( RSC_EDIT );
    WinBits nStyle = ImplInitRes( rResId );

    if (VclBuilderContainer::replace_buildable(pParent, rResId, *this))
        return;

    ImplInitEditData();
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    // Derived MultiLineEdit takes care to call Show only after MultiLineEdit
    // ctor has already started:
    if ( !(nStyle & WB_HIDE) && rResId.GetRT() != RSC_MULTILINEEDIT )
        Show();
}

void Edit::SetWidthInChars(sal_Int32 nWidthInChars)
{
    if (mnWidthInChars != nWidthInChars)
    {
        mnWidthInChars = nWidthInChars;
        queue_resize();
    }
}

bool Edit::set_property(const rtl::OString &rKey, const rtl::OString &rValue)
{
    if (rKey == "width-chars")
        SetWidthInChars(rValue.toInt32());
    else if (rKey == "max-length")
    {
        sal_Int32 nTextLen = rValue.toInt32();
        SetMaxTextLen(nTextLen == 0 ? EDIT_NOLIMIT : nTextLen);
    }
    else if (rKey == "editable")
        SetReadOnly(!toBool(rValue));
    else if (rKey == "visibility")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_PASSWORD);
        if (!toBool(rValue))
            nBits |= WB_PASSWORD;
        SetStyle(nBits);
    }
    else
        return Control::set_property(rKey, rValue);
    return true;
}

void Edit::take_properties(Window &rOther)
{
    if (!GetParent())
    {
        ImplInitEditData();
        ImplInit(rOther.GetParent(), rOther.GetStyle());
    }

    Control::take_properties(rOther);

    Edit &rOtherEdit = static_cast<Edit&>(rOther);
    maText = rOtherEdit.maText;
    maSaveValue = rOtherEdit.maSaveValue;
    maUndoText = rOtherEdit.maUndoText;
    maRedoText = rOtherEdit.maRedoText;
    mnXOffset = rOtherEdit.mnXOffset;
    maSelection = rOtherEdit.maSelection;
    mnAlign = rOtherEdit.mnAlign;
    mnMaxTextLen = rOtherEdit.mnMaxTextLen;
    mnWidthInChars = rOtherEdit.mnWidthInChars;
    meAutocompleteAction = rOtherEdit.meAutocompleteAction;
    mcEchoChar = rOtherEdit.mcEchoChar;
    mbModified = rOtherEdit.mbModified;
    mbInternModified = rOtherEdit.mbInternModified;
    mbReadOnly = rOtherEdit.mbReadOnly;
    mbInsertMode = rOtherEdit.mbInsertMode;
    mbClickedInSelection = rOtherEdit.mbClickedInSelection;
    mbIsSubEdit = rOtherEdit.mbIsSubEdit;
    mbInMBDown = rOtherEdit.mbInMBDown;
    mbActivePopup = rOtherEdit.mbActivePopup;
}

// -----------------------------------------------------------------------

Edit::Edit( Window* pParent, const ResId& rResId, bool bDisableAccessibleLabeledByRelation ) :
    Control( WINDOW_EDIT )
{
    ImplInitEditData();
    rResId.SetRT( RSC_EDIT );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );
    if ( bDisableAccessibleLabeledByRelation )
        ImplGetWindowImpl()->mbDisableAccessibleLabeledByRelation = sal_True;

    // Derived MultiLineEdit takes care to call Show only after MultiLineEdit
    // ctor has already started:
    if ( !(nStyle & WB_HIDE) && rResId.GetRT() != RSC_MULTILINEEDIT )
        Show();
}

// -----------------------------------------------------------------------

Edit::~Edit()
{
    delete mpDDInfo;
    Cursor* pCursor = GetCursor();
    if ( pCursor )
    {
        SetCursor( NULL );
        delete pCursor;
    }

    delete mpIMEInfos;

    delete mpUpdateDataTimer;

    if ( mxDnDListener.is() )
    {
        if ( GetDragGestureRecognizer().is() )
        {
            uno::Reference< datatransfer::dnd::XDragGestureListener> xDGL( mxDnDListener, uno::UNO_QUERY );
            GetDragGestureRecognizer()->removeDragGestureListener( xDGL );
        }
        if ( GetDropTarget().is() )
        {
            uno::Reference< datatransfer::dnd::XDropTargetListener> xDTL( mxDnDListener, uno::UNO_QUERY );
            GetDropTarget()->removeDropTargetListener( xDTL );
        }

        uno::Reference< lang::XEventListener> xEL( mxDnDListener, uno::UNO_QUERY );
        xEL->disposing( lang::EventObject() );  // #95154# #96585# Empty Source means it's the Client
    }
}

// -----------------------------------------------------------------------

void Edit::ImplInitEditData()
{
    mpSubEdit               = NULL;
    mpUpdateDataTimer       = NULL;
    mnXOffset               = 0;
    mnAlign                 = EDIT_ALIGN_LEFT;
    mnMaxTextLen            = EDIT_NOLIMIT;
    mnWidthInChars          = -1;
    meAutocompleteAction    = AUTOCOMPLETE_KEYINPUT;
    mbModified              = sal_False;
    mbInternModified        = sal_False;
    mbReadOnly              = sal_False;
    mbInsertMode            = sal_True;
    mbClickedInSelection    = sal_False;
    mbActivePopup           = sal_False;
    mbIsSubEdit             = sal_False;
    mbInMBDown              = sal_False;
    mpDDInfo                = NULL;
    mpIMEInfos              = NULL;
    mcEchoChar              = 0;

    // --- RTL --- no default mirroring for Edit controls
    // note: controls that use a subedit will revert this (SpinField, ComboBox)
    EnableRTL( sal_False );

    vcl::unohelper::DragAndDropWrapper* pDnDWrapper = new vcl::unohelper::DragAndDropWrapper( this );
    mxDnDListener = pDnDWrapper;
}

// -----------------------------------------------------------------------

bool Edit::ImplUseNativeBorder( WinBits nStyle )
{
    bool bRet =
        IsNativeControlSupported(ImplGetNativeControlType(), HAS_BACKGROUND_TEXTURE)
                                 && ((nStyle&WB_BORDER) && !(nStyle&WB_NOBORDER));
    if( ! bRet && mbIsSubEdit )
    {
        Window* pWindow = GetParent();
        nStyle = pWindow->GetStyle();
        bRet = pWindow->IsNativeControlSupported(ImplGetNativeControlType(), HAS_BACKGROUND_TEXTURE)
               && ((nStyle&WB_BORDER) && !(nStyle&WB_NOBORDER));
    }
    return bRet;
}

void Edit::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    if ( !(nStyle & (WB_CENTER | WB_RIGHT)) )
        nStyle |= WB_LEFT;

    Control::ImplInit( pParent, nStyle, NULL );

    mbReadOnly = (nStyle & WB_READONLY) != 0;

    mnAlign = EDIT_ALIGN_LEFT;

    // --- RTL --- hack: right align until keyinput and cursor travelling works
    if( IsRTLEnabled() )
        mnAlign = EDIT_ALIGN_RIGHT;

    if ( nStyle & WB_RIGHT )
        mnAlign = EDIT_ALIGN_RIGHT;
    else if ( nStyle & WB_CENTER )
        mnAlign = EDIT_ALIGN_CENTER;

    SetCursor( new Cursor );

    SetPointer( Pointer( POINTER_TEXT ) );
    ImplInitSettings( sal_True, sal_True, sal_True );

    uno::Reference< datatransfer::dnd::XDragGestureListener> xDGL( mxDnDListener, uno::UNO_QUERY );
    uno::Reference< datatransfer::dnd::XDragGestureRecognizer > xDGR = GetDragGestureRecognizer();
    if ( xDGR.is() )
    {
        xDGR->addDragGestureListener( xDGL );
        uno::Reference< datatransfer::dnd::XDropTargetListener> xDTL( mxDnDListener, uno::UNO_QUERY );
        GetDropTarget()->addDropTargetListener( xDTL );
        GetDropTarget()->setActive( sal_True );
        GetDropTarget()->setDefaultActions( datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE );
    }
}

// -----------------------------------------------------------------------

WinBits Edit::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;

    return nStyle;
}

// -----------------------------------------------------------------------

sal_Bool Edit::IsCharInput( const KeyEvent& rKeyEvent )
{
    // In the future we must use new Unicode functions for this
    sal_Unicode cCharCode = rKeyEvent.GetCharCode();
    return ((cCharCode >= 32) && (cCharCode != 127) &&
            !rKeyEvent.GetKeyCode().IsMod3() &&
            !rKeyEvent.GetKeyCode().IsMod2() &&
            !rKeyEvent.GetKeyCode().IsMod1() );
}

// -----------------------------------------------------------------------

void Edit::ImplModified()
{
    mbModified = sal_True;
    Modify();
}

// -----------------------------------------------------------------------

void Edit::ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetFieldFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
        ImplClearLayoutData();
    }

    if ( bFont || bForeground )
    {
        Color aTextColor = rStyleSettings.GetFieldTextColor();
        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if ( ImplUseNativeBorder( GetStyle() ) || IsPaintTransparent() )
        {
            // Transparent background
            SetBackground();
            SetFillColor();
        }
        else if ( IsControlBackground() )
        {
            SetBackground( GetControlBackground() );
            SetFillColor( GetControlBackground() );
        }
        else
        {
            SetBackground( rStyleSettings.GetFieldColor() );
            SetFillColor( rStyleSettings.GetFieldColor() );
        }
    }
}

// -----------------------------------------------------------------------

long Edit::ImplGetExtraOffset() const
{
    // MT 09/2002: nExtraOffsetX should become a member, instead of checking every time,
    // but I need an incompatible update for this...
    // #94095# Use extra offset only when edit has a border
    long nExtraOffset = 0;
    if( ( GetStyle() & WB_BORDER ) || ( mbIsSubEdit && ( GetParent()->GetStyle() & WB_BORDER ) ) )
        nExtraOffset = 2;

    return nExtraOffset;
}


// -----------------------------------------------------------------------

XubString Edit::ImplGetText() const
{
    if ( mcEchoChar || (GetStyle() & WB_PASSWORD) )
    {
        XubString   aText;
        sal_Unicode cEchoChar;
        if ( mcEchoChar )
            cEchoChar = mcEchoChar;
        else
            cEchoChar = '*';
        aText.Fill( maText.Len(), cEchoChar );
        return aText;
    }
    else
        return maText;
}

// -----------------------------------------------------------------------

void Edit::ImplInvalidateOrRepaint( xub_StrLen nStart, xub_StrLen nEnd )
{
    if( IsPaintTransparent() )
    {
        Invalidate();
        // FIXME: this is currently only on aqua
        if( ImplGetSVData()->maNWFData.mbNoFocusRects )
            Update();
    }
    else
        ImplRepaint( nStart, nEnd );
}

// -----------------------------------------------------------------------

long Edit::ImplGetTextYPosition() const
{
    if ( GetStyle() & WB_TOP )
        return ImplGetExtraOffset();
    else if ( GetStyle() & WB_BOTTOM )
        return GetOutputSizePixel().Height() - GetTextHeight() - ImplGetExtraOffset();
    return ( GetOutputSizePixel().Height() - GetTextHeight() ) / 2;
}

// -----------------------------------------------------------------------

void Edit::ImplRepaint( xub_StrLen nStart, xub_StrLen nEnd, bool bLayout )
{
    if ( !IsReallyVisible() )
        return;

    XubString aText = ImplGetText();
    nStart = 0;
    nEnd = aText.Len();

    sal_Int32   nDXBuffer[256];
    sal_Int32*  pDXBuffer = NULL;
    sal_Int32*  pDX = nDXBuffer;

    if( aText.Len() )
    {
        if( 2*aText.Len() > xub_StrLen(SAL_N_ELEMENTS(nDXBuffer)) )
        {
            pDXBuffer = new sal_Int32[2*(aText.Len()+1)];
            pDX = pDXBuffer;
        }

        GetCaretPositions( aText, pDX, nStart, nEnd );
    }

    long    nTH = GetTextHeight();
    Point   aPos( mnXOffset, ImplGetTextYPosition() );

    if( bLayout )
    {
        long nPos = nStart ? pDX[2*nStart] : 0;
        aPos.X() = nPos + mnXOffset + ImplGetExtraOffset();

        MetricVector* pVector = &mpControlData->mpLayoutData->m_aUnicodeBoundRects;
        String* pDisplayText = &mpControlData->mpLayoutData->m_aDisplayText;

        DrawText( aPos, aText, nStart, nEnd - nStart, pVector, pDisplayText );

        if( pDXBuffer )
            delete [] pDXBuffer;
        return;
    }

    Cursor* pCursor = GetCursor();
    sal_Bool bVisCursor = pCursor ? pCursor->IsVisible() : sal_False;
    if ( pCursor )
        pCursor->Hide();

    ImplClearBackground( 0, GetOutputSizePixel().Width() );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if ( IsEnabled() )
        ImplInitSettings( sal_False, sal_True, sal_False );
    else
        SetTextColor( rStyleSettings.GetDisableColor() );

    // Set background color of the normal text
    if( (GetStyle() & WB_FORCECTRLBACKGROUND) != 0 && IsControlBackground() )
    {
        // check if we need to set ControlBackground even in NWF case
        Push( PUSH_FILLCOLOR | PUSH_LINECOLOR );
        SetLineColor();
        SetFillColor( GetControlBackground() );
        DrawRect( Rectangle( aPos, Size( GetOutputSizePixel().Width() - 2*mnXOffset, GetOutputSizePixel().Height() ) ) );
        Pop();

        SetTextFillColor( GetControlBackground() );
    }
    else if( IsPaintTransparent() || ImplUseNativeBorder( GetStyle() ) )
        SetTextFillColor();
    else
        SetTextFillColor( IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor() );

    ImplPaintBorder( 0, GetOutputSizePixel().Width() );

    sal_Bool bDrawSelection = maSelection.Len() && ( HasFocus() || ( GetStyle() & WB_NOHIDESELECTION ) || mbActivePopup );

    long nPos = nStart ? pDX[2*nStart] : 0;
    aPos.X() = nPos + mnXOffset + ImplGetExtraOffset();
    if ( !bDrawSelection && !mpIMEInfos )
    {
        DrawText( aPos, aText, nStart, nEnd - nStart );
    }
    else
    {
        // save graphics state
        Push();
        // first calculate higlighted and non highlighted clip regions
        Region aHiglightClipRegion;
        Region aNormalClipRegion;
        Selection aTmpSel( maSelection );
        aTmpSel.Justify();
        // selection is highlighted
        int i;
        for( i = 0; i < aText.Len(); i++ )
        {
            Rectangle aRect( aPos, Size( 10, nTH ) );
            aRect.Left() = pDX[2*i] + mnXOffset + ImplGetExtraOffset();
            aRect.Right() = pDX[2*i+1] + mnXOffset + ImplGetExtraOffset();
            aRect.Justify();
            bool bHighlight = false;
            if( i >= aTmpSel.Min() && i < aTmpSel.Max() )
                bHighlight = true;

            if( mpIMEInfos && mpIMEInfos->pAttribs &&
                i >= mpIMEInfos->nPos && i < (mpIMEInfos->nPos+mpIMEInfos->nLen ) &&
                ( mpIMEInfos->pAttribs[i-mpIMEInfos->nPos] & EXTTEXTINPUT_ATTR_HIGHLIGHT) )
                bHighlight = true;

            if( bHighlight )
                aHiglightClipRegion.Union( aRect );
            else
                aNormalClipRegion.Union( aRect );
        }
        // draw normal text
        Color aNormalTextColor = GetTextColor();
        SetClipRegion( aNormalClipRegion );

        if( IsPaintTransparent() )
            SetTextFillColor();
        else
        {
            // Set background color when part of the text is selected
            if ( ImplUseNativeBorder( GetStyle() ) )
            {
                if( (GetStyle() & WB_FORCECTRLBACKGROUND) != 0 && IsControlBackground() )
                    SetTextFillColor( GetControlBackground() );
                else
                    SetTextFillColor();
            }
            else
                SetTextFillColor( IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor() );
        }
        DrawText( aPos, aText, nStart, nEnd - nStart );

        // draw highlighted text
        SetClipRegion( aHiglightClipRegion );
        SetTextColor( rStyleSettings.GetHighlightTextColor() );
        SetTextFillColor( rStyleSettings.GetHighlightColor() );
        DrawText( aPos, aText, nStart, nEnd - nStart );

        // if IME info exists loop over portions and output different font attributes
        if( mpIMEInfos && mpIMEInfos->pAttribs )
        {
            for( int n = 0; n < 2; n++ )
            {
                Region aRegion;
                if( n == 0 )
                {
                    SetTextColor( aNormalTextColor );
                    if( IsPaintTransparent() )
                        SetTextFillColor();
                    else
                        SetTextFillColor( IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor() );
                    aRegion = aNormalClipRegion;
                }
                else
                {
                    SetTextColor( rStyleSettings.GetHighlightTextColor() );
                    SetTextFillColor( rStyleSettings.GetHighlightColor() );
                    aRegion = aHiglightClipRegion;
                }

                for( i = 0; i < mpIMEInfos->nLen; )
                {
                    sal_uInt16 nAttr = mpIMEInfos->pAttribs[i];
                    Region aClip;
                    int nIndex = i;
                    while( nIndex < mpIMEInfos->nLen && mpIMEInfos->pAttribs[nIndex] == nAttr)  // #112631# check nIndex before using it
                    {
                        Rectangle aRect( aPos, Size( 10, nTH ) );
                        aRect.Left() = pDX[2*(nIndex+mpIMEInfos->nPos)] + mnXOffset + ImplGetExtraOffset();
                        aRect.Right() = pDX[2*(nIndex+mpIMEInfos->nPos)+1] + mnXOffset + ImplGetExtraOffset();
                        aRect.Justify();
                        aClip.Union( aRect );
                        nIndex++;
                    }
                    i = nIndex;
		    aClip.Intersect(aRegion);
                    if( !aClip.IsEmpty() && nAttr )
                    {
                        Font aFont = GetFont();
                        if ( nAttr & EXTTEXTINPUT_ATTR_UNDERLINE )
                            aFont.SetUnderline( UNDERLINE_SINGLE );
                        else if ( nAttr & EXTTEXTINPUT_ATTR_BOLDUNDERLINE )
                            aFont.SetUnderline( UNDERLINE_BOLD );
                        else if ( nAttr & EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE )
                            aFont.SetUnderline( UNDERLINE_DOTTED );
                        else if ( nAttr & EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE )
                            aFont.SetUnderline( UNDERLINE_DOTTED );
                        else if ( nAttr & EXTTEXTINPUT_ATTR_GRAYWAVELINE )
                        {
                            aFont.SetUnderline( UNDERLINE_WAVE );
                            SetTextLineColor( Color( COL_LIGHTGRAY ) );
                        }
                        SetFont( aFont );

                        if ( nAttr & EXTTEXTINPUT_ATTR_REDTEXT )
                            SetTextColor( Color( COL_RED ) );
                        else if ( nAttr & EXTTEXTINPUT_ATTR_HALFTONETEXT )
                            SetTextColor( Color( COL_LIGHTGRAY ) );

                        SetClipRegion( aClip );
                        DrawText( aPos, aText, nStart, nEnd - nStart );
                    }
                }
            }
        }

        // restore graphics state
        Pop();
    }

    if ( bVisCursor && ( !mpIMEInfos || mpIMEInfos->bCursor ) )
        pCursor->Show();

    if( pDXBuffer )
        delete [] pDXBuffer;
}

// -----------------------------------------------------------------------

void Edit::ImplDelete( const Selection& rSelection, sal_uInt8 nDirection, sal_uInt8 nMode )
{
    XubString aText = ImplGetText();

    // loeschen moeglich?
    if ( !rSelection.Len() &&
         (((rSelection.Min() == 0) && (nDirection == EDIT_DEL_LEFT)) ||
          ((rSelection.Max() == aText.Len()) && (nDirection == EDIT_DEL_RIGHT))) )
        return;

    ImplClearLayoutData();

    Selection aSelection( rSelection );
    aSelection.Justify();

    if ( !aSelection.Len() )
    {
        uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
        if ( nDirection == EDIT_DEL_LEFT )
        {
            if ( nMode == EDIT_DELMODE_RESTOFWORD )
            {
                i18n::Boundary aBoundary = xBI->getWordBoundary( maText, aSelection.Min(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
                if ( aBoundary.startPos == aSelection.Min() )
                    aBoundary = xBI->previousWord( maText, aSelection.Min(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
                aSelection.Min() = aBoundary.startPos;
            }
            else if ( nMode == EDIT_DELMODE_RESTOFCONTENT )
               {
                aSelection.Min() = 0;
            }
            else
            {
                sal_Int32 nCount = 1;
                aSelection.Min() = xBI->previousCharacters( maText, aSelection.Min(), GetSettings().GetLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
            }
        }
        else
        {
            if ( nMode == EDIT_DELMODE_RESTOFWORD )
            {
                i18n::Boundary aBoundary = xBI->nextWord( maText, aSelection.Max(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
                aSelection.Max() = aBoundary.startPos;
            }
            else if ( nMode == EDIT_DELMODE_RESTOFCONTENT )
            {
                aSelection.Max() = aText.Len();
            }
            else
            {
                sal_Int32 nCount = 1;
                aSelection.Max() = xBI->nextCharacters( maText, aSelection.Max(), GetSettings().GetLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
            }
        }
    }

    maText.Erase( (xub_StrLen)aSelection.Min(), (xub_StrLen)aSelection.Len() );
    maSelection.Min() = aSelection.Min();
    maSelection.Max() = aSelection.Min();
    ImplAlignAndPaint();
    mbInternModified = sal_True;
}

// -----------------------------------------------------------------------

String Edit::ImplGetValidString( const String& rString ) const
{
    rtl::OUString aValidString( rString );
    aValidString = comphelper::string::remove(aValidString, _LF);
    aValidString = comphelper::string::remove(aValidString, _CR);
    aValidString = aValidString.replace('\t', ' ');
    return aValidString;
}

// -----------------------------------------------------------------------
uno::Reference < i18n::XBreakIterator > Edit::ImplGetBreakIterator() const
{
    //!! since we don't want to become incompatible in the next minor update
    //!! where this code will get integrated into, xISC will be a local
    //!! variable instead of a class member!
    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    return i18n::BreakIterator::create(xContext);
}
// -----------------------------------------------------------------------

uno::Reference < i18n::XExtendedInputSequenceChecker > Edit::ImplGetInputSequenceChecker() const
{
    //!! since we don't want to become incompatible in the next minor update
    //!! where this code will get integrated into, xISC will be a local
    //!! variable instead of a class member!
    uno::Reference < i18n::XExtendedInputSequenceChecker > xISC;
//    if ( !xISC.is() )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        uno::Reference < XInterface > xI = xMSF->createInstance( OUString("com.sun.star.i18n.InputSequenceChecker") );
        if ( xI.is() )
        {
            Any x = xI->queryInterface( ::getCppuType((const uno::Reference< i18n::XExtendedInputSequenceChecker >*)0) );
            x >>= xISC;
        }
    }
    return xISC;
}

// -----------------------------------------------------------------------

void Edit::ShowTruncationWarning( Window* pParent )
{
    ResMgr* pResMgr = ImplGetResMgr();
    if( pResMgr )
    {
        WarningBox aBox( pParent, ResId( SV_EDIT_WARNING_BOX, *pResMgr ) );
        aBox.Execute();
    }
}

// -----------------------------------------------------------------------

bool Edit::ImplTruncateToMaxLen( rtl::OUString& rStr, sal_uInt32 nSelectionLen ) const
{
    bool bWasTruncated = false;
    const sal_uInt32 nMaxLen = mnMaxTextLen < 65534 ? mnMaxTextLen : 65534;
    sal_uInt32 nLenAfter = static_cast<sal_uInt32>(maText.Len()) + rStr.getLength() - nSelectionLen;
    if ( nLenAfter > nMaxLen )
    {
        sal_uInt32 nErasePos = nMaxLen - static_cast<sal_uInt32>(maText.Len()) + nSelectionLen;
        rStr = rStr.copy( 0, nErasePos );
        bWasTruncated = true;
    }
    return bWasTruncated;
}

// -----------------------------------------------------------------------

void Edit::ImplInsertText( const rtl::OUString& rStr, const Selection* pNewSel, sal_Bool bIsUserInput )
{
    Selection aSelection( maSelection );
    aSelection.Justify();

    rtl::OUString aNewText( ImplGetValidString( rStr ) );
    ImplTruncateToMaxLen( aNewText, aSelection.Len() );

    ImplClearLayoutData();

    if ( aSelection.Len() )
        maText.Erase( (xub_StrLen)aSelection.Min(), (xub_StrLen)aSelection.Len() );
    else if ( !mbInsertMode && (aSelection.Max() < maText.Len()) )
        maText.Erase( (xub_StrLen)aSelection.Max(), 1 );

    // take care of input-sequence-checking now
    if (bIsUserInput && !rStr.isEmpty())
    {
        DBG_ASSERT( rStr.getLength() == 1, "unexpected string length. User input is expected to providse 1 char only!" );

        // determine if input-sequence-checking should be applied or not
        //
        static OUString sModule( "/org.openoffice.Office.Common/I18N" );
        static OUString sRelNode( "CTL" );
        static OUString sCTLSequenceChecking( "CTLSequenceChecking" );
        static OUString sCTLSequenceCheckingRestricted( "CTLSequenceCheckingRestricted" );
        static OUString sCTLSequenceCheckingTypeAndReplace( "CTLSequenceCheckingTypeAndReplace" );
        static OUString sCTLFont( "CTLFont" );
        //
        sal_Bool bCTLSequenceChecking               = sal_False;
        sal_Bool bCTLSequenceCheckingRestricted     = sal_False;
        sal_Bool bCTLSequenceCheckingTypeAndReplace = sal_False;
        sal_Bool bCTLFontEnabled                    = sal_False;
        sal_Bool bIsInputSequenceChecking           = sal_False;
        //
        // get access to the configuration of this office module
        try
        {
            uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            uno::Reference< container::XNameAccess > xModuleCfg( ::comphelper::ConfigurationHelper::openConfig(
                                    xContext,
                                    sModule,
                                    ::comphelper::ConfigurationHelper::E_READONLY ),
                                uno::UNO_QUERY );

            //!! get values from configuration.
            //!! we can't use SvtCTLOptions here since vcl must not be linked
            //!! against svtools. (It is already the other way around.)
            Any aCTLSequenceChecking                = ::comphelper::ConfigurationHelper::readRelativeKey( xModuleCfg, sRelNode, sCTLSequenceChecking );
            Any aCTLSequenceCheckingRestricted      = ::comphelper::ConfigurationHelper::readRelativeKey( xModuleCfg, sRelNode, sCTLSequenceCheckingRestricted );
            Any aCTLSequenceCheckingTypeAndReplace  = ::comphelper::ConfigurationHelper::readRelativeKey( xModuleCfg, sRelNode, sCTLSequenceCheckingTypeAndReplace );
            Any aCTLFontEnabled                     = ::comphelper::ConfigurationHelper::readRelativeKey( xModuleCfg, sRelNode, sCTLFont );
            aCTLSequenceChecking                >>= bCTLSequenceChecking;
            aCTLSequenceCheckingRestricted      >>= bCTLSequenceCheckingRestricted;
            aCTLSequenceCheckingTypeAndReplace  >>= bCTLSequenceCheckingTypeAndReplace;
            aCTLFontEnabled                     >>= bCTLFontEnabled;
        }
        catch(...)
        {
            bIsInputSequenceChecking = sal_False;   // continue with inserting the new text
        }
        //
        uno::Reference < i18n::XBreakIterator > xBI( ImplGetBreakIterator(), UNO_QUERY );
        bIsInputSequenceChecking = rStr.getLength() == 1 &&
                bCTLFontEnabled &&
                bCTLSequenceChecking &&
                aSelection.Min() > 0 && /* first char needs not to be checked */
                xBI.is() && i18n::ScriptType::COMPLEX == xBI->getScriptType( rStr, 0 );


        uno::Reference < i18n::XExtendedInputSequenceChecker > xISC;
        if (bIsInputSequenceChecking && (xISC = ImplGetInputSequenceChecker()).is())
        {
            sal_Unicode cChar = rStr[0];
            xub_StrLen nTmpPos = static_cast< xub_StrLen >( aSelection.Min() );
            sal_Int16 nCheckMode = bCTLSequenceCheckingRestricted ?
                    i18n::InputSequenceCheckMode::STRICT : i18n::InputSequenceCheckMode::BASIC;

            // the text that needs to be checked is only the one
            // before the current cursor position
            rtl::OUString aOldText( maText.Copy(0, nTmpPos) );
            rtl::OUString aTmpText( aOldText );
            if (bCTLSequenceCheckingTypeAndReplace)
            {
                xISC->correctInputSequence( aTmpText, nTmpPos - 1, cChar, nCheckMode );

                // find position of first character that has changed
                sal_Int32 nOldLen = aOldText.getLength();
                sal_Int32 nTmpLen = aTmpText.getLength();
                const sal_Unicode *pOldTxt = aOldText.getStr();
                const sal_Unicode *pTmpTxt = aTmpText.getStr();
                sal_Int32 nChgPos = 0;
                while ( nChgPos < nOldLen && nChgPos < nTmpLen &&
                        pOldTxt[nChgPos] == pTmpTxt[nChgPos] )
                    ++nChgPos;

                String aChgText( aTmpText.copy( nChgPos ) );

                // remove text from first pos to be changed to current pos
                maText.Erase( static_cast< xub_StrLen >( nChgPos ), static_cast< xub_StrLen >( nTmpPos - nChgPos ) );

                if (aChgText.Len())
                {
                    aNewText = aChgText;
                    aSelection.Min() = nChgPos; // position for new text to be inserted
                }
                else
                    aNewText = String::EmptyString();
            }
            else
            {
                // should the character be ignored (i.e. not get inserted) ?
                if (!xISC->checkInputSequence( aOldText, nTmpPos - 1, cChar, nCheckMode ))
                    aNewText = String::EmptyString();
            }
        }

        // at this point now we will insert the non-empty text 'normally' some lines below...
    }

    if ( !aNewText.isEmpty() )
        maText.Insert( String( aNewText ), (xub_StrLen)aSelection.Min() );

    if ( !pNewSel )
    {
        maSelection.Min() = aSelection.Min() + aNewText.getLength();
        maSelection.Max() = maSelection.Min();
    }
    else
    {
        maSelection = *pNewSel;
        if ( maSelection.Min() > maText.Len() )
            maSelection.Min() = maText.Len();
        if ( maSelection.Max() > maText.Len() )
            maSelection.Max() = maText.Len();
    }

    ImplAlignAndPaint();
    mbInternModified = sal_True;
}

// -----------------------------------------------------------------------

void Edit::ImplSetText( const XubString& rText, const Selection* pNewSelection )
{
    // we delete text by "selecting" the old text completely then calling InsertText; this is flicker free
    if ( ( rText.Len() <= mnMaxTextLen ) && ( (rText != maText) || (pNewSelection && (*pNewSelection != maSelection)) ) )
    {
        ImplClearLayoutData();
        maSelection.Min() = 0;
        maSelection.Max() = maText.Len();
        if ( mnXOffset || HasPaintEvent() )
        {
            mnXOffset = 0;
            maText = ImplGetValidString( rText );

            // #i54929# recalculate mnXOffset before ImplSetSelection,
            // else cursor ends up in wrong position
            ImplAlign();

            if ( pNewSelection )
                ImplSetSelection( *pNewSelection, sal_False );

            if ( mnXOffset && !pNewSelection )
                maSelection.Max() = 0;

            Invalidate();
        }
        else
            ImplInsertText( rText, pNewSelection );

        ImplCallEventListeners( VCLEVENT_EDIT_MODIFY );
    }
}

// -----------------------------------------------------------------------

int Edit::ImplGetNativeControlType()
{
    int nCtrl = 0;
    Window *pControl = mbIsSubEdit ? GetParent() : this;

    switch( pControl->GetType() )
    {
        case WINDOW_COMBOBOX:
        case WINDOW_PATTERNBOX:
        case WINDOW_NUMERICBOX:
        case WINDOW_METRICBOX:
        case WINDOW_CURRENCYBOX:
        case WINDOW_DATEBOX:
        case WINDOW_TIMEBOX:
        case WINDOW_LONGCURRENCYBOX:
            nCtrl = CTRL_COMBOBOX;
            break;

        case WINDOW_MULTILINEEDIT:
            if ( GetWindow( WINDOW_BORDER ) != this )
                nCtrl = CTRL_MULTILINE_EDITBOX;
            else
                nCtrl = CTRL_EDITBOX_NOBORDER;
            break;

        case WINDOW_EDIT:
        case WINDOW_PATTERNFIELD:
        case WINDOW_METRICFIELD:
        case WINDOW_CURRENCYFIELD:
        case WINDOW_DATEFIELD:
        case WINDOW_TIMEFIELD:
        case WINDOW_LONGCURRENCYFIELD:
        case WINDOW_NUMERICFIELD:
        case WINDOW_SPINFIELD:
            if( pControl->GetStyle() & WB_SPIN )
                nCtrl = CTRL_SPINBOX;
            else
            {
                if ( GetWindow( WINDOW_BORDER ) != this )
                    nCtrl = CTRL_EDITBOX;
                else
                    nCtrl = CTRL_EDITBOX_NOBORDER;
            }
            break;

        default:
            nCtrl = CTRL_EDITBOX;
    }
    return nCtrl;
}

void Edit::ImplClearBackground( long nXStart, long nXEnd )
{
    /*
    * note: at this point the cursor must be switched off already
    */
    Point aTmpPoint;
    Rectangle aRect( aTmpPoint, GetOutputSizePixel() );
    aRect.Left() = nXStart;
    aRect.Right() = nXEnd;

    if( !(ImplUseNativeBorder( GetStyle() ) || IsPaintTransparent()) )
        Erase( aRect );
}

void Edit::ImplPaintBorder( long nXStart, long nXEnd )
{
    Point aTmpPoint;
    Rectangle aRect( aTmpPoint, GetOutputSizePixel() );
    aRect.Left() = nXStart;
    aRect.Right() = nXEnd;

    if( ImplUseNativeBorder( GetStyle() ) || IsPaintTransparent() )
    {
        // draw the inner part by painting the whole control using its border window
        Window *pControl = this;
        Window *pBorder = GetWindow( WINDOW_BORDER );
        if( pBorder == this )
        {
            // we have no border, use parent
            pControl = mbIsSubEdit ? GetParent() : this;
            pBorder = pControl->GetWindow( WINDOW_BORDER );
            if( pBorder == this )
                pBorder = GetParent();
        }

        if( pBorder )
        {
            // set proper clipping region to not overdraw the whole control
            Region aClipRgn = GetPaintRegion();
            if( !aClipRgn.IsNull() )
            {
                // transform clipping region to border window's coordinate system
                if( IsRTLEnabled() != pBorder->IsRTLEnabled() && Application::GetSettings().GetLayoutRTL() )
                {
                    // need to mirror in case border is not RTL but edit is (or vice versa)

                    // mirror
                    Rectangle aBounds( aClipRgn.GetBoundRect() );
                    int xNew = GetOutputSizePixel().Width() - aBounds.GetWidth() - aBounds.Left();
                    aClipRgn.Move( xNew - aBounds.Left(), 0 );

                    // move offset of border window
                    Point aBorderOffs;
                    aBorderOffs = pBorder->ScreenToOutputPixel( OutputToScreenPixel( aBorderOffs ) );
                    aClipRgn.Move( aBorderOffs.X(), aBorderOffs.Y() );
                }
                else
                {
                    // normal case
                    Point aBorderOffs;
                    aBorderOffs = pBorder->ScreenToOutputPixel( OutputToScreenPixel( aBorderOffs ) );
                    aClipRgn.Move( aBorderOffs.X(), aBorderOffs.Y() );
                }

                Region oldRgn( pBorder->GetClipRegion() );
                pBorder->SetClipRegion( aClipRgn );

                pBorder->Paint( Rectangle() );

                pBorder->SetClipRegion( oldRgn );
            }
            else
                pBorder->Paint( Rectangle() );

        }
    }
}

// -----------------------------------------------------------------------

void Edit::ImplShowCursor( sal_Bool bOnlyIfVisible )
{
    if ( !IsUpdateMode() || ( bOnlyIfVisible && !IsReallyVisible() ) )
        return;

    Cursor*     pCursor = GetCursor();
    XubString   aText = ImplGetText();

    long nTextPos = 0;

    sal_Int32   nDXBuffer[256];
    sal_Int32*  pDXBuffer = NULL;
    sal_Int32*  pDX = nDXBuffer;

    if( aText.Len() )
    {
        if( 2*aText.Len() > xub_StrLen(SAL_N_ELEMENTS(nDXBuffer)) )
        {
            pDXBuffer = new sal_Int32[2*(aText.Len()+1)];
            pDX = pDXBuffer;
        }

        GetCaretPositions( aText, pDX, 0, aText.Len() );

        if( maSelection.Max() < aText.Len() )
            nTextPos = pDX[ 2*maSelection.Max() ];
        else
            nTextPos = pDX[ 2*aText.Len()-1 ];
    }

    long nCursorWidth = 0;
    if ( !mbInsertMode && !maSelection.Len() && (maSelection.Max() < aText.Len()) )
        nCursorWidth = GetTextWidth( aText, (xub_StrLen)maSelection.Max(), 1 );
    long nCursorPosX = nTextPos + mnXOffset + ImplGetExtraOffset();

    // cursor should land in visible area
    const Size aOutSize = GetOutputSizePixel();
    if ( (nCursorPosX < 0) || (nCursorPosX >= aOutSize.Width()) )
    {
        long nOldXOffset = mnXOffset;

        if ( nCursorPosX < 0 )
        {
            mnXOffset = - nTextPos;
            long nMaxX = 0;
            mnXOffset += aOutSize.Width() / 5;
            if ( mnXOffset > nMaxX )
                mnXOffset = nMaxX;
        }
        else
        {
            mnXOffset = (aOutSize.Width()-ImplGetExtraOffset()) - nTextPos;
            // Etwas mehr?
            if ( (aOutSize.Width()-ImplGetExtraOffset()) < nTextPos )
            {
                long nMaxNegX = (aOutSize.Width()-ImplGetExtraOffset()) - GetTextWidth( aText );
                mnXOffset -= aOutSize.Width() / 5;
                if ( mnXOffset < nMaxNegX )  // beides negativ...
                    mnXOffset = nMaxNegX;
            }
        }

        nCursorPosX = nTextPos + mnXOffset + ImplGetExtraOffset();
        if ( nCursorPosX == aOutSize.Width() )  // dann nicht sichtbar...
            nCursorPosX--;

        if ( mnXOffset != nOldXOffset )
            ImplInvalidateOrRepaint();
    }

    const long nTextHeight = GetTextHeight();
    const long nCursorPosY = ImplGetTextYPosition();
    pCursor->SetPos( Point( nCursorPosX, nCursorPosY ) );
    pCursor->SetSize( Size( nCursorWidth, nTextHeight ) );
    pCursor->Show();

    if( pDXBuffer )
        delete [] pDXBuffer;
}

// -----------------------------------------------------------------------

void Edit::ImplAlign()
{
    long nTextWidth = GetTextWidth( ImplGetText() );
    long nOutWidth = GetOutputSizePixel().Width();

    if ( mnAlign == EDIT_ALIGN_LEFT )
    {
        if( mnXOffset && ( nTextWidth < nOutWidth ) )
            mnXOffset = 0;

    }
    else if ( mnAlign == EDIT_ALIGN_RIGHT )
    {
        long nMinXOffset = nOutWidth - nTextWidth - 1 - ImplGetExtraOffset();
        bool bRTL = IsRTLEnabled();
        if( mbIsSubEdit && GetParent() )
            bRTL = GetParent()->IsRTLEnabled();
        if( bRTL )
        {
            if( nTextWidth < nOutWidth )
                mnXOffset = nMinXOffset;
        }
        else
        {
            if( nTextWidth < nOutWidth )
                mnXOffset = nMinXOffset;
            else if ( mnXOffset < nMinXOffset )
                mnXOffset = nMinXOffset;
        }
    }
    else if( mnAlign == EDIT_ALIGN_CENTER )
    {
            // would be nicer with check while scrolling but then it's not centred in scrolled state
            mnXOffset = (nOutWidth - nTextWidth) / 2;
    }
}


// -----------------------------------------------------------------------

void Edit::ImplAlignAndPaint()
{
    ImplAlign();
    ImplInvalidateOrRepaint( 0, STRING_LEN );
    ImplShowCursor();
}

// -----------------------------------------------------------------------

xub_StrLen Edit::ImplGetCharPos( const Point& rWindowPos ) const
{
    xub_StrLen nIndex = STRING_LEN;
    String aText = ImplGetText();

    sal_Int32   nDXBuffer[256];
    sal_Int32*  pDXBuffer = NULL;
    sal_Int32*  pDX = nDXBuffer;
    if( 2*aText.Len() > xub_StrLen(SAL_N_ELEMENTS(nDXBuffer)) )
    {
        pDXBuffer = new sal_Int32[2*(aText.Len()+1)];
        pDX = pDXBuffer;
    }

    GetCaretPositions( aText, pDX, 0, aText.Len() );
    long nX = rWindowPos.X() - mnXOffset - ImplGetExtraOffset();
    for( int i = 0; i < aText.Len(); i++ )
    {
        if( (pDX[2*i] >= nX && pDX[2*i+1] <= nX) ||
            (pDX[2*i+1] >= nX && pDX[2*i] <= nX))
        {
            nIndex = sal::static_int_cast<xub_StrLen>(i);
            if( pDX[2*i] < pDX[2*i+1] )
            {
                if( nX > (pDX[2*i]+pDX[2*i+1])/2 )
                    nIndex++;
            }
            else
            {
                if( nX < (pDX[2*i]+pDX[2*i+1])/2 )
                    nIndex++;
            }
            break;
        }
    }
    if( nIndex == STRING_LEN )
    {
        nIndex = 0;
        long nDiff = Abs( pDX[0]-nX );
        for( int i = 1; i < aText.Len(); i++ )
        {
            long nNewDiff = Abs( pDX[2*i]-nX );

            if( nNewDiff < nDiff )
            {
                nIndex = sal::static_int_cast<xub_StrLen>(i);
                nDiff = nNewDiff;
            }
        }
        if( nIndex == aText.Len()-1 && Abs( pDX[2*nIndex+1] - nX ) < nDiff )
            nIndex = STRING_LEN;
    }

    if( pDXBuffer )
        delete [] pDXBuffer;

    return nIndex;
}

// -----------------------------------------------------------------------

void Edit::ImplSetCursorPos( xub_StrLen nChar, sal_Bool bSelect )
{
    Selection aSelection( maSelection );
    aSelection.Max() = nChar;
    if ( !bSelect )
        aSelection.Min() = aSelection.Max();
    ImplSetSelection( aSelection );
}

// -----------------------------------------------------------------------

void Edit::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    xub_StrLen nTextLength = ReadShortRes();
    if ( nTextLength )
        SetMaxTextLen( nTextLength );
}

// -----------------------------------------------------------------------

void Edit::ImplCopyToSelectionClipboard()
{
    if ( GetSelection().Len() )
    {
        ::com::sun::star::uno::Reference<com::sun::star::datatransfer::clipboard::XClipboard> aSelection(GetPrimarySelection());
        ImplCopy( aSelection );
    }
}

void Edit::ImplCopy( uno::Reference< datatransfer::clipboard::XClipboard >& rxClipboard )
{
    ::vcl::unohelper::TextDataObject::CopyStringTo( GetSelected(), rxClipboard );
}

// -----------------------------------------------------------------------

void Edit::ImplPaste( uno::Reference< datatransfer::clipboard::XClipboard >& rxClipboard )
{
    if ( rxClipboard.is() )
    {
        uno::Reference< datatransfer::XTransferable > xDataObj;

        const sal_uInt32 nRef = Application::ReleaseSolarMutex();

        try
        {
            xDataObj = rxClipboard->getContents();
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }

        Application::AcquireSolarMutex( nRef );

        if ( xDataObj.is() )
        {
            datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
            try
            {
                uno::Any aData = xDataObj->getTransferData( aFlavor );
                ::rtl::OUString aText;
                aData >>= aText;
                if( ImplTruncateToMaxLen( aText, maSelection.Len() ) )
                    ShowTruncationWarning( const_cast<Edit*>(this) );
                ReplaceSelected( aText );
            }
            catch( const ::com::sun::star::uno::Exception& )
            {
            }
        }
    }
}

// -----------------------------------------------------------------------

void Edit::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( mpSubEdit )
    {
        Control::MouseButtonDown( rMEvt );
        return;
    }

    xub_StrLen nChar = ImplGetCharPos( rMEvt.GetPosPixel() );
    Selection aSelection( maSelection );
    aSelection.Justify();

    if ( rMEvt.GetClicks() < 4 )
    {
        mbClickedInSelection = sal_False;
        if ( rMEvt.GetClicks() == 3 )
        {
            ImplSetSelection( Selection( 0, 0xFFFF ) );
            ImplCopyToSelectionClipboard();

        }
        else if ( rMEvt.GetClicks() == 2 )
        {
            uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
             i18n::Boundary aBoundary = xBI->getWordBoundary( maText, aSelection.Max(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
            ImplSetSelection( Selection( aBoundary.startPos, aBoundary.endPos ) );
            ImplCopyToSelectionClipboard();
        }
        else if ( !rMEvt.IsShift() && HasFocus() && aSelection.IsInside( nChar ) )
            mbClickedInSelection = sal_True;
        else if ( rMEvt.IsLeft() )
            ImplSetCursorPos( nChar, rMEvt.IsShift() );

        if ( !mbClickedInSelection && rMEvt.IsLeft() && ( rMEvt.GetClicks() == 1 ) )
            StartTracking( STARTTRACK_SCROLLREPEAT );
    }

    mbInMBDown = sal_True;  // then do not select all in GetFocus
    GrabFocus();
    mbInMBDown = sal_False;
}

// -----------------------------------------------------------------------

void Edit::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( mbClickedInSelection && rMEvt.IsLeft() )
    {
        xub_StrLen nChar = ImplGetCharPos( rMEvt.GetPosPixel() );
        ImplSetCursorPos( nChar, sal_False );
        mbClickedInSelection = sal_False;
    }
    else if ( rMEvt.IsMiddle() && !mbReadOnly &&
              ( GetSettings().GetMouseSettings().GetMiddleButtonAction() == MOUSE_MIDDLE_PASTESELECTION ) )
    {
        ::com::sun::star::uno::Reference<com::sun::star::datatransfer::clipboard::XClipboard> aSelection(Window::GetPrimarySelection());
        ImplPaste( aSelection );
        ImplModified();
    }
}

// -----------------------------------------------------------------------

void Edit::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( mbClickedInSelection )
        {
            xub_StrLen nChar = ImplGetCharPos( rTEvt.GetMouseEvent().GetPosPixel() );
            ImplSetCursorPos( nChar, sal_False );
            mbClickedInSelection = sal_False;
        }
        else if ( rTEvt.GetMouseEvent().IsLeft() )
        {
            ImplCopyToSelectionClipboard();
        }
    }
    else
    {
        if( !mbClickedInSelection )
        {
            xub_StrLen nChar = ImplGetCharPos( rTEvt.GetMouseEvent().GetPosPixel() );
            ImplSetCursorPos( nChar, sal_True );
        }
    }

    if ( mpUpdateDataTimer && !mbIsSubEdit && mpUpdateDataTimer->IsActive() )
        mpUpdateDataTimer->Start();//do not update while the user is still travelling in the control
}

// -----------------------------------------------------------------------

sal_Bool Edit::ImplHandleKeyEvent( const KeyEvent& rKEvt )
{
    sal_Bool        bDone = sal_False;
    sal_uInt16      nCode = rKEvt.GetKeyCode().GetCode();
    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();

    mbInternModified = sal_False;

    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_CUT:
            {
                if ( !mbReadOnly && maSelection.Len() && !(GetStyle() & WB_PASSWORD) )
                {
                    Cut();
                    ImplModified();
                    bDone = sal_True;
                }
            }
            break;

            case KEYFUNC_COPY:
            {
                if ( !(GetStyle() & WB_PASSWORD) )
                {
                    Copy();
                    bDone = sal_True;
                }
            }
            break;

            case KEYFUNC_PASTE:
            {
                if ( !mbReadOnly )
                {
                    Paste();
                    bDone = sal_True;
                }
            }
            break;

            case KEYFUNC_UNDO:
            {
                if ( !mbReadOnly )
                {
                    Undo();
                    bDone = sal_True;
                }
            }
            break;

            default:
                eFunc = KEYFUNC_DONTKNOW;
        }
    }

    if ( !bDone && rKEvt.GetKeyCode().IsMod1() && !rKEvt.GetKeyCode().IsMod2() )
    {
        if ( nCode == KEY_A )
        {
            ImplSetSelection( Selection( 0, maText.Len() ) );
            bDone = sal_True;
        }
        else if ( rKEvt.GetKeyCode().IsShift() && (nCode == KEY_S) )
        {
            if ( pImplFncGetSpecialChars )
            {
                Selection aSaveSel = GetSelection(); // if someone changes the selection in Get/LoseFocus, e.g. URL bar
                XubString aChars = pImplFncGetSpecialChars( this, GetFont() );
                SetSelection( aSaveSel );
                if ( aChars.Len() )
                {
                    ImplInsertText( aChars );
                    ImplModified();
                }
                bDone = sal_True;
            }
        }
    }

    if ( eFunc == KEYFUNC_DONTKNOW && ! bDone )
    {
        switch ( nCode )
        {
            case com::sun::star::awt::Key::SELECT_ALL:
            {
                ImplSetSelection( Selection( 0, maText.Len() ) );
                bDone = sal_True;
            }
            break;

            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_HOME:
            case KEY_END:
            case com::sun::star::awt::Key::MOVE_WORD_FORWARD:
            case com::sun::star::awt::Key::SELECT_WORD_FORWARD:
            case com::sun::star::awt::Key::MOVE_WORD_BACKWARD:
            case com::sun::star::awt::Key::SELECT_WORD_BACKWARD:
            case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_LINE:
            case com::sun::star::awt::Key::MOVE_TO_END_OF_LINE:
            case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_LINE:
            case com::sun::star::awt::Key::SELECT_TO_END_OF_LINE:
            case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
            case com::sun::star::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
            case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
            case com::sun::star::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
            case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
            case com::sun::star::awt::Key::MOVE_TO_END_OF_DOCUMENT:
            case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
            case com::sun::star::awt::Key::SELECT_TO_END_OF_DOCUMENT:
            {
                if ( !rKEvt.GetKeyCode().IsMod2() )
                {
                    ImplClearLayoutData();
                    uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();

                    Selection aSel( maSelection );
                    bool bWord = rKEvt.GetKeyCode().IsMod1();
                    bool bSelect = rKEvt.GetKeyCode().IsShift();
                    bool bGoLeft = (nCode == KEY_LEFT);
                    bool bGoRight = (nCode == KEY_RIGHT);
                    bool bGoHome = (nCode == KEY_HOME);
                    bool bGoEnd = (nCode == KEY_END);

                    switch( nCode )
                    {
                    case com::sun::star::awt::Key::MOVE_WORD_FORWARD:
                        bGoRight = bWord = true;break;
                    case com::sun::star::awt::Key::SELECT_WORD_FORWARD:
                        bGoRight = bSelect = bWord = true;break;
                    case com::sun::star::awt::Key::MOVE_WORD_BACKWARD:
                        bGoLeft = bWord = true;break;
                    case com::sun::star::awt::Key::SELECT_WORD_BACKWARD:
                        bGoLeft = bSelect = bWord = true;break;
                    case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_LINE:
                    case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
                    case com::sun::star::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
                        bSelect = true;
                        // fallthrough intended
                    case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_LINE:
                    case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
                    case com::sun::star::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
                        bGoHome = true;break;
                    case com::sun::star::awt::Key::SELECT_TO_END_OF_LINE:
                    case com::sun::star::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
                    case com::sun::star::awt::Key::SELECT_TO_END_OF_DOCUMENT:
                        bSelect = true;
                        // fallthrough intended
                    case com::sun::star::awt::Key::MOVE_TO_END_OF_LINE:
                    case com::sun::star::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
                    case com::sun::star::awt::Key::MOVE_TO_END_OF_DOCUMENT:
                        bGoEnd = true;break;
                    default:
                        break;
                    };

                    // Range wird in ImplSetSelection geprueft...
                    if ( bGoLeft && aSel.Max() )
                    {
                        if ( bWord )
                        {
                            i18n::Boundary aBoundary = xBI->getWordBoundary( maText, aSel.Max(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
                            if ( aBoundary.startPos == aSel.Max() )
                                aBoundary = xBI->previousWord( maText, aSel.Max(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
                            aSel.Max() = aBoundary.startPos;
                        }
                        else
                        {
                            sal_Int32 nCount = 1;
                            aSel.Max() = xBI->previousCharacters( maText, aSel.Max(), GetSettings().GetLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
                        }
                    }
                    else if ( bGoRight && ( aSel.Max() < maText.Len() ) )
                    {
                        if ( bWord )
                           {
                            i18n::Boundary aBoundary = xBI->nextWord( maText, aSel.Max(), GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
                            aSel.Max() = aBoundary.startPos;
                        }
                        else
                        {
                            sal_Int32 nCount = 1;
                            aSel.Max() = xBI->nextCharacters( maText, aSel.Max(), GetSettings().GetLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
                        }
                    }
                    else if ( bGoHome )
                    {
                        aSel.Max() = 0;
                    }
                    else if ( bGoEnd )
                    {
                        aSel.Max() = 0xFFFF;
                    }

                    if ( !bSelect )
                        aSel.Min() = aSel.Max();

                    if ( aSel != GetSelection() )
                    {
                        ImplSetSelection( aSel );
                        ImplCopyToSelectionClipboard();
                    }

                    if ( bGoEnd && maAutocompleteHdl.IsSet() && !rKEvt.GetKeyCode().GetModifier() )
                    {
                        if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.Len()) )
                        {
                            meAutocompleteAction = AUTOCOMPLETE_KEYINPUT;
                            maAutocompleteHdl.Call( this );
                        }
                    }

                    bDone = sal_True;
                }
            }
            break;

            case com::sun::star::awt::Key::DELETE_WORD_BACKWARD:
            case com::sun::star::awt::Key::DELETE_WORD_FORWARD:
            case com::sun::star::awt::Key::DELETE_TO_BEGIN_OF_LINE:
            case com::sun::star::awt::Key::DELETE_TO_END_OF_LINE:
            case KEY_BACKSPACE:
            case KEY_DELETE:
            {
                if ( !mbReadOnly && !rKEvt.GetKeyCode().IsMod2() )
                {
                    sal_uInt8 nDel = (nCode == KEY_DELETE) ? EDIT_DEL_RIGHT : EDIT_DEL_LEFT;
                    sal_uInt8 nMode = rKEvt.GetKeyCode().IsMod1() ? EDIT_DELMODE_RESTOFWORD : EDIT_DELMODE_SIMPLE;
                    if ( (nMode == EDIT_DELMODE_RESTOFWORD) && rKEvt.GetKeyCode().IsShift() )
                        nMode = EDIT_DELMODE_RESTOFCONTENT;
                    switch( nCode )
                    {
                    case com::sun::star::awt::Key::DELETE_WORD_BACKWARD:
                        nDel = EDIT_DEL_LEFT;
                        nMode = EDIT_DELMODE_RESTOFWORD;
                        break;
                    case com::sun::star::awt::Key::DELETE_WORD_FORWARD:
                        nDel = EDIT_DEL_RIGHT;
                        nMode = EDIT_DELMODE_RESTOFWORD;
                        break;
                    case com::sun::star::awt::Key::DELETE_TO_BEGIN_OF_LINE:
                        nDel = EDIT_DEL_LEFT;
                        nMode = EDIT_DELMODE_RESTOFCONTENT;
                        break;
                    case com::sun::star::awt::Key::DELETE_TO_END_OF_LINE:
                        nDel = EDIT_DEL_RIGHT;
                        nMode = EDIT_DELMODE_RESTOFCONTENT;
                        break;
                    default: break;
                    }
                    xub_StrLen nOldLen = maText.Len();
                    ImplDelete( maSelection, nDel, nMode );
                    if ( maText.Len() != nOldLen )
                        ImplModified();
                    bDone = sal_True;
                }
            }
            break;

            case KEY_INSERT:
            {
                if ( !mpIMEInfos && !mbReadOnly && !rKEvt.GetKeyCode().IsMod2() )
                {
                    SetInsertMode( !mbInsertMode );
                    bDone = sal_True;
                }
            }
            break;

            /* #i101255# disable autocomplete tab forward/backward
               users expect tab/shif-tab to move the focus to other controls
               not suddenly to cycle the autocompletion
            case KEY_TAB:
            {
                if ( !mbReadOnly && maAutocompleteHdl.IsSet() &&
                     maSelection.Min() && (maSelection.Min() == maText.Len()) &&
                     !rKEvt.GetKeyCode().IsMod1() && !rKEvt.GetKeyCode().IsMod2() )
                {
                    // Kein Autocomplete wenn alles Selektiert oder Edit leer, weil dann
                    // keine vernuenftige Tab-Steuerung!
                    if ( rKEvt.GetKeyCode().IsShift() )
                        meAutocompleteAction = AUTOCOMPLETE_TABBACKWARD;
                    else
                        meAutocompleteAction = AUTOCOMPLETE_TABFORWARD;

                    maAutocompleteHdl.Call( this );

                    // Wurde nichts veraendert, dann TAB fuer DialogControl
                    if ( GetSelection().Len() )
                        bDone = sal_True;
                }
            }
            break;
            */

            default:
            {
                if ( IsCharInput( rKEvt ) )
                {
                    bDone = sal_True;   // read characters also when in ReadOnly
                    if ( !mbReadOnly )
                    {
                        ImplInsertText(rtl::OUString(rKEvt.GetCharCode()), 0, sal_True);
                        if ( maAutocompleteHdl.IsSet() )
                        {
                            if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.Len()) )
                            {
                                meAutocompleteAction = AUTOCOMPLETE_KEYINPUT;
                                maAutocompleteHdl.Call( this );
                            }
                        }
                    }
                }
            }
        }
    }

    if ( mbInternModified )
        ImplModified();

    return bDone;
}

// -----------------------------------------------------------------------

void Edit::KeyInput( const KeyEvent& rKEvt )
{
    if ( mpUpdateDataTimer && !mbIsSubEdit && mpUpdateDataTimer->IsActive() )
        mpUpdateDataTimer->Start();//do not update while the user is still travelling in the control

    if ( mpSubEdit || !ImplHandleKeyEvent( rKEvt ) )
        Control::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void Edit::FillLayoutData() const
{
    mpControlData->mpLayoutData = new vcl::ControlLayoutData();
    const_cast<Edit*>(this)->ImplRepaint( 0, STRING_LEN, true );
}

// -----------------------------------------------------------------------

void Edit::Paint( const Rectangle& )
{
    if ( !mpSubEdit )
        ImplRepaint();
}

// -----------------------------------------------------------------------

void Edit::Resize()
{
    if ( !mpSubEdit && IsReallyVisible() )
    {
        Control::Resize();
        // Wegen vertikaler Zentrierung...
        mnXOffset = 0;
        ImplAlign();
        Invalidate();
        ImplShowCursor();
    }
}

// -----------------------------------------------------------------------

void Edit::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    ImplInitSettings( sal_True, sal_True, sal_True );

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );
    Font aFont = GetDrawPixelFont( pDev );
    OutDevType eOutDevType = pDev->GetOutDevType();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    pDev->SetTextFillColor();

    // Border/Background
    pDev->SetLineColor();
    pDev->SetFillColor();
    sal_Bool bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
    sal_Bool bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
    if ( bBorder || bBackground )
    {
        Rectangle aRect( aPos, aSize );
        if ( bBorder )
        {
            ImplDrawFrame( pDev, aRect );
        }
        if ( bBackground )
        {
            pDev->SetFillColor( GetControlBackground() );
            pDev->DrawRect( aRect );
        }
    }

    // Inhalt
    if ( ( nFlags & WINDOW_DRAW_MONO ) || ( eOutDevType == OUTDEV_PRINTER ) )
        pDev->SetTextColor( Color( COL_BLACK ) );
    else
    {
        if ( !(nFlags & WINDOW_DRAW_NODISABLE ) && !IsEnabled() )
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            pDev->SetTextColor( rStyleSettings.GetDisableColor() );
        }
        else
        {
            pDev->SetTextColor( GetTextColor() );
        }
    }

    XubString   aText = ImplGetText();
    long        nTextHeight = pDev->GetTextHeight();
    long        nTextWidth = pDev->GetTextWidth( aText );
    long        nOnePixel = GetDrawPixel( pDev, 1 );
    long        nOffX = 3*nOnePixel;
    long        nOffY = (aSize.Height() - nTextHeight) / 2;

    // Clipping?
    if ( (nOffY < 0) ||
         ((nOffY+nTextHeight) > aSize.Height()) ||
         ((nOffX+nTextWidth) > aSize.Width()) )
    {
        Rectangle aClip( aPos, aSize );
        if ( nTextHeight > aSize.Height() )
            aClip.Bottom() += nTextHeight-aSize.Height()+1;  // prevent HP printers from 'optimizing'
        pDev->IntersectClipRegion( aClip );
    }

    if ( GetStyle() & WB_CENTER )
    {
        aPos.X() += (aSize.Width() - nTextWidth) / 2;
        nOffX = 0;
    }
    else if ( GetStyle() & WB_RIGHT )
    {
        aPos.X() += aSize.Width() - nTextWidth;
        nOffX = -nOffX;
    }

    pDev->DrawText( Point( aPos.X() + nOffX, aPos.Y() + nOffY ), aText );
    pDev->Pop();

    if ( GetSubEdit() )
    {
        GetSubEdit()->Draw( pDev, rPos, rSize, nFlags );
    }
}

// -----------------------------------------------------------------------

void Edit::ImplInvalidateOutermostBorder( Window* pWin )
{
    // allow control to show focused state
    Window *pInvalWin = pWin, *pBorder = pWin;
    while( ( pBorder = pInvalWin->GetWindow( WINDOW_BORDER ) ) != pInvalWin && pBorder &&
           pInvalWin->ImplGetFrame() == pBorder->ImplGetFrame() )
    {
        pInvalWin = pBorder;
    }

    pInvalWin->Invalidate( INVALIDATE_CHILDREN | INVALIDATE_UPDATE );
}

void Edit::GetFocus()
{
    if ( mpSubEdit )
        mpSubEdit->ImplGrabFocus( GetGetFocusFlags() );
    else if ( !mbActivePopup )
    {
        maUndoText = maText;

        sal_uLong nSelOptions = GetSettings().GetStyleSettings().GetSelectionOptions();
        if ( !( GetStyle() & (WB_NOHIDESELECTION|WB_READONLY) )
                && ( GetGetFocusFlags() & (GETFOCUS_INIT|GETFOCUS_TAB|GETFOCUS_CURSOR|GETFOCUS_MNEMONIC) ) )
        {
            if ( nSelOptions & SELECTION_OPTION_SHOWFIRST )
            {
                maSelection.Min() = maText.Len();
                maSelection.Max() = 0;
            }
            else
            {
                maSelection.Min() = 0;
                maSelection.Max() = maText.Len();
            }
            if ( mbIsSubEdit )
                ((Edit*)GetParent())->ImplCallEventListeners( VCLEVENT_EDIT_SELECTIONCHANGED );
            else
                ImplCallEventListeners( VCLEVENT_EDIT_SELECTIONCHANGED );
        }

        ImplShowCursor();

        // FIXME: this is currently only on aqua
        // check for other platforms that need similar handling
        if( ImplGetSVData()->maNWFData.mbNoFocusRects &&
            IsNativeWidgetEnabled() &&
            IsNativeControlSupported( CTRL_EDITBOX, PART_ENTIRE_CONTROL ) )
        {
            ImplInvalidateOutermostBorder( mbIsSubEdit ? GetParent() : this );
        }
        else if ( maSelection.Len() )
        {
            // Selektion malen
            if ( !HasPaintEvent() )
                ImplInvalidateOrRepaint();
            else
                Invalidate();
        }

        SetInputContext( InputContext( GetFont(), !IsReadOnly() ? INPUTCONTEXT_TEXT|INPUTCONTEXT_EXTTEXTINPUT : 0 ) );
    }

    Control::GetFocus();
}

// -----------------------------------------------------------------------

Window* Edit::GetPreferredKeyInputWindow()
{
    if ( mpSubEdit )
        return mpSubEdit->GetPreferredKeyInputWindow();
    else
        return this;
}

// -----------------------------------------------------------------------

void Edit::LoseFocus()
{
    if ( mpUpdateDataTimer && !mbIsSubEdit && mpUpdateDataTimer->IsActive() )
    {
        //notify an update latest when the focus is lost
        mpUpdateDataTimer->Stop();
        mpUpdateDataTimer->Timeout();
    }

    if ( !mpSubEdit )
    {
        // FIXME: this is currently only on aqua
        // check for other platforms that need similar handling
        if( ImplGetSVData()->maNWFData.mbNoFocusRects &&
            IsNativeWidgetEnabled() &&
            IsNativeControlSupported( CTRL_EDITBOX, PART_ENTIRE_CONTROL ) )
        {
            ImplInvalidateOutermostBorder( mbIsSubEdit ? GetParent() : this );
        }

        if ( !mbActivePopup && !( GetStyle() & WB_NOHIDESELECTION ) && maSelection.Len() )
            ImplInvalidateOrRepaint();    // Selektion malen
    }

    Control::LoseFocus();
}

// -----------------------------------------------------------------------

void Edit::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        PopupMenu* pPopup = Edit::CreatePopupMenu();

        if ( !maSelection.Len() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_CUT, sal_False );
            pPopup->EnableItem( SV_MENU_EDIT_COPY, sal_False );
            pPopup->EnableItem( SV_MENU_EDIT_DELETE, sal_False );
        }

        if ( IsReadOnly() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_CUT, sal_False );
            pPopup->EnableItem( SV_MENU_EDIT_PASTE, sal_False );
            pPopup->EnableItem( SV_MENU_EDIT_DELETE, sal_False );
            pPopup->EnableItem( SV_MENU_EDIT_INSERTSYMBOL, sal_False );
        }
        else
        {
            // Paste nur, wenn Text im Clipboard
            sal_Bool bData = sal_False;
            uno::Reference< datatransfer::clipboard::XClipboard > xClipboard = GetClipboard();
            if ( xClipboard.is() )
            {
                const sal_uInt32 nRef = Application::ReleaseSolarMutex();
                uno::Reference< datatransfer::XTransferable > xDataObj = xClipboard->getContents();
                Application::AcquireSolarMutex( nRef );
                if ( xDataObj.is() )
                {
                    datatransfer::DataFlavor aFlavor;
                    SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
                    bData = xDataObj->isDataFlavorSupported( aFlavor );
                }
            }
            pPopup->EnableItem( SV_MENU_EDIT_PASTE, bData );
        }

        if ( maUndoText == maText )
            pPopup->EnableItem( SV_MENU_EDIT_UNDO, sal_False );
        if ( ( maSelection.Min() == 0 ) && ( maSelection.Max() == maText.Len() ) )
            pPopup->EnableItem( SV_MENU_EDIT_SELECTALL, sal_False );
        if ( !pImplFncGetSpecialChars )
        {
            sal_uInt16 nPos = pPopup->GetItemPos( SV_MENU_EDIT_INSERTSYMBOL );
            pPopup->RemoveItem( nPos );
            pPopup->RemoveItem( nPos-1 );
        }

        mbActivePopup = sal_True;
        Selection aSaveSel = GetSelection(); // if someone changes selection in Get/LoseFocus, e.g. URL bar
        Point aPos = rCEvt.GetMousePosPixel();
        if ( !rCEvt.IsMouseEvent() )
        {
            // Show menu enventually centered in selection
            Size aSize = GetOutputSizePixel();
            aPos = Point( aSize.Width()/2, aSize.Height()/2 );
        }
        sal_uInt16 n = pPopup->Execute( this, aPos );
        Edit::DeletePopupMenu( pPopup );
        SetSelection( aSaveSel );
        switch ( n )
        {
            case SV_MENU_EDIT_UNDO:
                Undo();
                ImplModified();
                break;
            case SV_MENU_EDIT_CUT:
                Cut();
                ImplModified();
                break;
            case SV_MENU_EDIT_COPY:
                Copy();
                break;
            case SV_MENU_EDIT_PASTE:
                Paste();
                ImplModified();
                break;
            case SV_MENU_EDIT_DELETE:
                DeleteSelected();
                ImplModified();
                break;
            case SV_MENU_EDIT_SELECTALL:
                ImplSetSelection( Selection( 0, maText.Len() ) );
                break;
            case SV_MENU_EDIT_INSERTSYMBOL:
                {
                    XubString aChars = pImplFncGetSpecialChars( this, GetFont() );
                    SetSelection( aSaveSel );
                    if ( aChars.Len() )
                    {
                        ImplInsertText( aChars );
                        ImplModified();
                    }
                }
                break;
        }
        mbActivePopup = sal_False;
    }
    else if ( rCEvt.GetCommand() == COMMAND_VOICE )
    {
        const CommandVoiceData* pData = rCEvt.GetVoiceData();
        if ( pData->GetType() == VOICECOMMANDTYPE_DICTATION )
        {
            switch ( pData->GetCommand() )
            {
                case DICTATIONCOMMAND_UNKNOWN:
                {
                    ReplaceSelected( pData->GetText() );
                }
                break;
                case DICTATIONCOMMAND_LEFT:
                {
                    ImplHandleKeyEvent( KeyEvent( 0, KeyCode( KEY_LEFT, KEY_MOD1  ) ) );
                }
                break;
                case DICTATIONCOMMAND_RIGHT:
                {
                    ImplHandleKeyEvent( KeyEvent( 0, KeyCode( KEY_RIGHT, KEY_MOD1  ) ) );
                }
                break;
                case DICTATIONCOMMAND_UNDO:
                {
                    Undo();
                }
                break;
                case DICTATIONCOMMAND_DEL:
                {
                    ImplHandleKeyEvent( KeyEvent( 0, KeyCode( KEY_LEFT, KEY_MOD1|KEY_SHIFT  ) ) );
                    DeleteSelected();
                }
                break;
            }
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_STARTEXTTEXTINPUT )
    {
        DeleteSelected();
        delete mpIMEInfos;
        xub_StrLen nPos = (xub_StrLen)maSelection.Max();
        mpIMEInfos = new Impl_IMEInfos( nPos, maText.Copy( nPos ) );
        mpIMEInfos->bWasCursorOverwrite = !IsInsertMode();
    }
    else if ( rCEvt.GetCommand() == COMMAND_ENDEXTTEXTINPUT )
    {
        sal_Bool bInsertMode = !mpIMEInfos->bWasCursorOverwrite;
        delete mpIMEInfos;
        mpIMEInfos = NULL;

        // set font without attributes, because it will not be re-initialised in Repaint anymore
        ImplInitSettings( sal_True, sal_False, sal_False );

        SetInsertMode( bInsertMode );

        ImplModified();

        // #i25161# call auto complete handler for ext text commit also
        if ( maAutocompleteHdl.IsSet() )
        {
            if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.Len()) )
            {
                meAutocompleteAction = AUTOCOMPLETE_KEYINPUT;
                maAutocompleteHdl.Call( this );
            }
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_EXTTEXTINPUT )
    {
        const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();

        maText.Erase( mpIMEInfos->nPos, mpIMEInfos->nLen );
        maText.Insert( pData->GetText(), mpIMEInfos->nPos );
        if ( mpIMEInfos->bWasCursorOverwrite )
        {
            sal_uInt16 nOldIMETextLen = mpIMEInfos->nLen;
            sal_uInt16 nNewIMETextLen = pData->GetText().Len();
            if ( ( nOldIMETextLen > nNewIMETextLen ) &&
                 ( nNewIMETextLen < mpIMEInfos->aOldTextAfterStartPos.Len() ) )
            {
                // restore old characters
                sal_uInt16 nRestore = nOldIMETextLen - nNewIMETextLen;
                maText.Insert( mpIMEInfos->aOldTextAfterStartPos.Copy( nNewIMETextLen, nRestore ), mpIMEInfos->nPos + nNewIMETextLen );
            }
            else if ( ( nOldIMETextLen < nNewIMETextLen ) &&
                      ( nOldIMETextLen < mpIMEInfos->aOldTextAfterStartPos.Len() ) )
            {
                // overwrite
                sal_uInt16 nOverwrite = nNewIMETextLen - nOldIMETextLen;
                if ( ( nOldIMETextLen + nOverwrite ) > mpIMEInfos->aOldTextAfterStartPos.Len() )
                    nOverwrite = mpIMEInfos->aOldTextAfterStartPos.Len() - nOldIMETextLen;
                maText.Erase( mpIMEInfos->nPos + nNewIMETextLen, nOverwrite );
            }
        }


        if ( pData->GetTextAttr() )
        {
            mpIMEInfos->CopyAttribs( pData->GetTextAttr(), pData->GetText().Len() );
            mpIMEInfos->bCursor = pData->IsCursorVisible();
        }
        else
        {
            mpIMEInfos->DestroyAttribs();
        }

        ImplAlignAndPaint();
        xub_StrLen nCursorPos = mpIMEInfos->nPos + pData->GetCursorPos();
        SetSelection( Selection( nCursorPos, nCursorPos ) );
        SetInsertMode( !pData->IsCursorOverwrite() );

        if ( pData->IsCursorVisible() )
            GetCursor()->Show();
        else
            GetCursor()->Hide();
    }
    else if ( rCEvt.GetCommand() == COMMAND_CURSORPOS )
    {
        if ( mpIMEInfos )
        {
            xub_StrLen nCursorPos = (sal_uInt16)GetSelection().Max();
            SetCursorRect( NULL, GetTextWidth(
                maText, nCursorPos, mpIMEInfos->nPos+mpIMEInfos->nLen-nCursorPos ) );
        }
        else
        {
            SetCursorRect();
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_SELECTIONCHANGE )
    {
        const CommandSelectionChangeData *pData = rCEvt.GetSelectionChangeData();
        Selection aSelection( pData->GetStart(), pData->GetEnd() );
        SetSelection(aSelection);
    }
    else
        Control::Command( rCEvt );
}

// -----------------------------------------------------------------------

void Edit::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( !mpSubEdit )
        {
            mnXOffset = 0;  // if GrabFocus before while size was still wrong
            ImplAlign();
            if ( !mpSubEdit )
                ImplShowCursor( sal_False );
        }
        // update background (eventual SetPaintTransparent)
        ImplInitSettings( sal_False, sal_False, sal_True );
    }
    else if ( nType == STATE_CHANGE_ENABLE )
    {
        if ( !mpSubEdit )
        {
            // change text color only
            ImplInvalidateOrRepaint( 0, 0xFFFF );
        }
    }
    else if ( nType == STATE_CHANGE_STYLE || nType == STATE_CHANGE_MIRRORING )
    {
        WinBits nStyle = GetStyle();
        if( nType == STATE_CHANGE_STYLE )
        {
            nStyle = ImplInitStyle( GetStyle() );
            SetStyle( nStyle );
        }

        sal_uInt16 nOldAlign = mnAlign;
        mnAlign = EDIT_ALIGN_LEFT;

        // --- RTL --- hack: right align until keyinput and cursor travelling works
        // edits are always RTL disabled
        // however the parent edits contain the correct setting
        if( mbIsSubEdit && GetParent()->IsRTLEnabled() )
        {
            if( GetParent()->GetStyle() & WB_LEFT )
                mnAlign = EDIT_ALIGN_RIGHT;
            if ( nType == STATE_CHANGE_MIRRORING )
                SetLayoutMode( TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_TEXTORIGIN_LEFT );
        }
        else if( mbIsSubEdit && !GetParent()->IsRTLEnabled() )
        {
            if ( nType == STATE_CHANGE_MIRRORING )
                SetLayoutMode( TEXT_LAYOUT_BIDI_LTR | TEXT_LAYOUT_TEXTORIGIN_LEFT );
        }

        if ( nStyle & WB_RIGHT )
            mnAlign = EDIT_ALIGN_RIGHT;
        else if ( nStyle & WB_CENTER )
            mnAlign = EDIT_ALIGN_CENTER;
        if ( maText.Len() && ( mnAlign != nOldAlign ) )
        {
            ImplAlign();
            Invalidate();
        }

    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( sal_True, sal_False, sal_False );
            ImplShowCursor( sal_True );
            Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( sal_True, sal_False, sal_False );
            ImplShowCursor();
            Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( sal_False, sal_True, sal_False );
            Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( sal_False, sal_False, sal_True );
            Invalidate();
        }
    }

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void Edit::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        if ( !mpSubEdit )
        {
            ImplInitSettings( sal_True, sal_True, sal_True );
            ImplShowCursor( sal_True );
            Invalidate();
        }
    }

    Control::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

void Edit::ImplShowDDCursor()
{
    if ( !mpDDInfo->bVisCursor )
    {
        long nTextWidth = GetTextWidth( maText, 0, mpDDInfo->nDropPos );
        long nTextHeight = GetTextHeight();
        Rectangle aCursorRect( Point( nTextWidth + mnXOffset, (GetOutputSize().Height()-nTextHeight)/2 ), Size( 2, nTextHeight ) );
        mpDDInfo->aCursor.SetWindow( this );
        mpDDInfo->aCursor.SetPos( aCursorRect.TopLeft() );
        mpDDInfo->aCursor.SetSize( aCursorRect.GetSize() );
        mpDDInfo->aCursor.Show();
        mpDDInfo->bVisCursor = sal_True;
    }
}

// -----------------------------------------------------------------------

void Edit::ImplHideDDCursor()
{
    if ( mpDDInfo && mpDDInfo->bVisCursor )
    {
        mpDDInfo->aCursor.Hide();
        mpDDInfo->bVisCursor = sal_False;
    }
}

// -----------------------------------------------------------------------

void Edit::Modify()
{
    if ( mbIsSubEdit )
    {
        ((Edit*)GetParent())->Modify();
    }
    else
    {
        if ( mpUpdateDataTimer )
            mpUpdateDataTimer->Start();

        if ( ImplCallEventListenersAndHandler( VCLEVENT_EDIT_MODIFY, maModifyHdl, this ) )
            // have been destroyed while calling into the handlers
            return;

        // #i13677# notify edit listeners about caret position change
        ImplCallEventListeners( VCLEVENT_EDIT_SELECTIONCHANGED );

        // FIXME: this is currently only on aqua
        // check for other platforms that need similar handling
        if( ImplGetSVData()->maNWFData.mbNoFocusRects &&
            IsNativeWidgetEnabled() &&
            IsNativeControlSupported( CTRL_EDITBOX, PART_ENTIRE_CONTROL ) )
        {
            ImplInvalidateOutermostBorder( this );
        }
    }
}

// -----------------------------------------------------------------------

void Edit::UpdateData()
{
    maUpdateDataHdl.Call( this );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(Edit, ImplUpdateDataHdl)
{
    UpdateData();
    return 0;
}

// -----------------------------------------------------------------------

void Edit::EnableUpdateData( sal_uLong nTimeout )
{
    if ( !nTimeout )
        DisableUpdateData();
    else
    {
        if ( !mpUpdateDataTimer )
        {
            mpUpdateDataTimer = new Timer;
            mpUpdateDataTimer->SetTimeoutHdl( LINK( this, Edit, ImplUpdateDataHdl ) );
        }

        mpUpdateDataTimer->SetTimeout( nTimeout );
    }
}

// -----------------------------------------------------------------------

void Edit::SetEchoChar( sal_Unicode c )
{
    mcEchoChar = c;
    if ( mpSubEdit )
        mpSubEdit->SetEchoChar( c );
}

// -----------------------------------------------------------------------

void Edit::SetReadOnly( sal_Bool bReadOnly )
{
    if ( mbReadOnly != bReadOnly )
    {
        mbReadOnly = bReadOnly;
        if ( mpSubEdit )
            mpSubEdit->SetReadOnly( bReadOnly );

        StateChanged( STATE_CHANGE_READONLY );
    }
}

// -----------------------------------------------------------------------

void Edit::SetAutocompleteHdl( const Link& rHdl )
{
    maAutocompleteHdl = rHdl;
    if ( mpSubEdit )
        mpSubEdit->SetAutocompleteHdl( rHdl );
}

// -----------------------------------------------------------------------

void Edit::SetInsertMode( sal_Bool bInsert )
{
    if ( bInsert != mbInsertMode )
    {
        mbInsertMode = bInsert;
        if ( mpSubEdit )
            mpSubEdit->SetInsertMode( bInsert );
        else
            ImplShowCursor();
    }
}

// -----------------------------------------------------------------------

sal_Bool Edit::IsInsertMode() const
{
    if ( mpSubEdit )
        return mpSubEdit->IsInsertMode();
    else
        return mbInsertMode;
}

// -----------------------------------------------------------------------

void Edit::SetMaxTextLen( xub_StrLen nMaxLen )
{
    mnMaxTextLen = nMaxLen ? nMaxLen : EDIT_NOLIMIT;

    if ( mpSubEdit )
        mpSubEdit->SetMaxTextLen( mnMaxTextLen );
    else
    {
        if ( maText.Len() > mnMaxTextLen )
            ImplDelete( Selection( mnMaxTextLen, maText.Len() ), EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
    }
}

// -----------------------------------------------------------------------

void Edit::SetSelection( const Selection& rSelection )
{
    // If the selection was changed from outside, e.g. by MouseButtonDown, don't call Tracking()
    // directly afterwards which would change the selection again
    if ( IsTracking() )
        EndTracking();
    else if ( mpSubEdit && mpSubEdit->IsTracking() )
        mpSubEdit->EndTracking();

    ImplSetSelection( rSelection );
}

// -----------------------------------------------------------------------

void Edit::ImplSetSelection( const Selection& rSelection, sal_Bool bPaint )
{
    if ( mpSubEdit )
        mpSubEdit->ImplSetSelection( rSelection );
    else
    {
        if ( rSelection != maSelection )
        {
            Selection aOld( maSelection );
            Selection aNew( rSelection );

            if ( aNew.Min() > maText.Len() )
                aNew.Min() = maText.Len();
            if ( aNew.Max() > maText.Len() )
                aNew.Max() = maText.Len();
            if ( aNew.Min() < 0 )
                aNew.Min() = 0;
            if ( aNew.Max() < 0 )
                aNew.Max() = 0;

            if ( aNew != maSelection )
            {
                ImplClearLayoutData();
                maSelection = aNew;

                if ( bPaint && ( aOld.Len() || aNew.Len() || IsPaintTransparent() ) )
                    ImplInvalidateOrRepaint( 0, maText.Len() );
                ImplShowCursor();
                if ( mbIsSubEdit )
                    ((Edit*)GetParent())->ImplCallEventListeners( VCLEVENT_EDIT_SELECTIONCHANGED );
                else
                    ImplCallEventListeners( VCLEVENT_EDIT_SELECTIONCHANGED );
                // #103511# notify combobox listeners of deselection
                if( !maSelection && GetParent() && GetParent()->GetType() == WINDOW_COMBOBOX )
                    ((Edit*)GetParent())->ImplCallEventListeners( VCLEVENT_COMBOBOX_DESELECT );
            }
        }
    }
}

// -----------------------------------------------------------------------

const Selection& Edit::GetSelection() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetSelection();
    else
        return maSelection;
}

// -----------------------------------------------------------------------

void Edit::ReplaceSelected( const XubString& rStr )
{
    if ( mpSubEdit )
        mpSubEdit->ReplaceSelected( rStr );
    else
        ImplInsertText( rStr );
}

// -----------------------------------------------------------------------

void Edit::DeleteSelected()
{
    if ( mpSubEdit )
        mpSubEdit->DeleteSelected();
    else
    {
        if ( maSelection.Len() )
            ImplDelete( maSelection, EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
    }
}

// -----------------------------------------------------------------------

XubString Edit::GetSelected() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetSelected();
    else
    {
        Selection aSelection( maSelection );
        aSelection.Justify();
        return maText.Copy( (xub_StrLen)aSelection.Min(), (xub_StrLen)aSelection.Len() );
    }
}

// -----------------------------------------------------------------------

void Edit::Cut()
{
    if ( !(GetStyle() & WB_PASSWORD ) )
    {
        Copy();
        ReplaceSelected( ImplGetSVEmptyStr() );
    }
}

// -----------------------------------------------------------------------

void Edit::Copy()
{
    if ( !(GetStyle() & WB_PASSWORD ) )
    {
        ::com::sun::star::uno::Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipboard(GetClipboard());
        ImplCopy( aClipboard );
    }
}

// -----------------------------------------------------------------------

void Edit::Paste()
{
        ::com::sun::star::uno::Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipboard(GetClipboard());
    ImplPaste( aClipboard );
}

// -----------------------------------------------------------------------

void Edit::Undo()
{
    if ( mpSubEdit )
        mpSubEdit->Undo();
    else
    {
        XubString aText( maText );
        ImplDelete( Selection( 0, aText.Len() ), EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
        ImplInsertText( maUndoText );
        ImplSetSelection( Selection( 0, maUndoText.Len() ) );
        maUndoText = aText;
    }
}

// -----------------------------------------------------------------------

void Edit::SetText( const XubString& rStr )
{
    if ( mpSubEdit )
        mpSubEdit->SetText( rStr );     // not directly ImplSetText if SetText overloaded
    else
    {
        Selection aNewSel( 0, 0 );  // prevent scrolling
        ImplSetText( rStr, &aNewSel );
    }
}

// -----------------------------------------------------------------------

void Edit::SetText( const XubString& rStr, const Selection& rSelection )
{
    if ( mpSubEdit )
        mpSubEdit->SetText( rStr, rSelection );
    else
        ImplSetText( rStr, &rSelection );
}

// -----------------------------------------------------------------------

XubString Edit::GetText() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetText();
    else
        return maText;
}

// -----------------------------------------------------------------------

void Edit::SetModifyFlag()
{
    if ( mpSubEdit )
        mpSubEdit->mbModified = sal_True;
    else
        mbModified = sal_True;
}

// -----------------------------------------------------------------------

void Edit::ClearModifyFlag()
{
    if ( mpSubEdit )
        mpSubEdit->mbModified = sal_False;
    else
        mbModified = sal_False;
}

// -----------------------------------------------------------------------

void Edit::SetSubEdit( Edit* pEdit )
{
    mpSubEdit = pEdit;
    if ( mpSubEdit )
    {
        SetPointer( POINTER_ARROW );    // Nur das SubEdit hat den BEAM...
        mpSubEdit->mbIsSubEdit = sal_True;

        mpSubEdit->SetReadOnly( mbReadOnly );
    }
}

Size Edit::CalcMinimumSizeForText(const rtl::OUString &rString) const
{
    Size aSize;
    if (mnWidthInChars != -1)
    {
        aSize = CalcSize(mnWidthInChars);
    }
    else
    {
        aSize.Height() = GetTextHeight();
        aSize.Width() = GetTextWidth(rString);
        aSize.Width() += ImplGetExtraOffset() * 2;
        // do not create edit fields in which one cannot enter anything
        // a default minimum width should exist for at least 3 characters
        Size aMinSize(CalcSize(3));
        if (aSize.Width() < aMinSize.Width())
            aSize.Width() = aMinSize.Width();
    }
    // add some space between text entry and border
    aSize.Height() += 4;

    aSize = CalcWindowSize( aSize );

    // ask NWF what if it has an opinion, too
    ImplControlValue aControlValue;
    Rectangle aRect( Point( 0, 0 ), aSize );
    Rectangle aContent, aBound;
    if( const_cast<Edit*>(this)->GetNativeControlRegion(
                   CTRL_EDITBOX, PART_ENTIRE_CONTROL,
                   aRect, 0, aControlValue, rtl::OUString(), aBound, aContent) )
    {
        if( aBound.GetHeight() > aSize.Height() )
            aSize.Height() = aBound.GetHeight();
    }
    return aSize;
}

Size Edit::CalcMinimumSize() const
{
    return CalcMinimumSizeForText(GetText());
}

Size Edit::GetMinimumEditSize()
{
    Window* pDefWin = ImplGetDefaultWindow();
    Edit aEdit( pDefWin, WB_BORDER );
    Size aSize( aEdit.CalcMinimumSize() );
    return aSize;
}

// -----------------------------------------------------------------------

Size Edit::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM:
        return CalcMinimumSize();
    default:
        return Control::GetOptimalSize( eType );
    }
}

// -----------------------------------------------------------------------

Size Edit::CalcSize( xub_StrLen nChars ) const
{
    // width for N characters, independent from content.
    // works only correct for fixed fonts, average otherwise
    Size aSz( GetTextWidth( rtl::OUString('x') ), GetTextHeight() );
    aSz.Width() *= nChars;
    aSz.Width() += ImplGetExtraOffset() * 2;
    aSz = CalcWindowSize( aSz );
    return aSz;
}

// -----------------------------------------------------------------------

xub_StrLen Edit::GetMaxVisChars() const
{
    const Window* pW = mpSubEdit ? mpSubEdit : this;
    long nOutWidth = pW->GetOutputSizePixel().Width();
    long nCharWidth = GetTextWidth( rtl::OUString('x') );
    return nCharWidth ? (xub_StrLen)(nOutWidth/nCharWidth) : 0;
}

// -----------------------------------------------------------------------

xub_StrLen Edit::GetCharPos( const Point& rWindowPos ) const
{
    return ImplGetCharPos( rWindowPos );
}

// -----------------------------------------------------------------------

void Edit::SetGetSpecialCharsFunction( FncGetSpecialChars fn )
{
    pImplFncGetSpecialChars = fn;
}

// -----------------------------------------------------------------------

FncGetSpecialChars Edit::GetGetSpecialCharsFunction()
{
    return pImplFncGetSpecialChars;
}

// -----------------------------------------------------------------------

PopupMenu* Edit::CreatePopupMenu()
{
    ResMgr* pResMgr = ImplGetResMgr();
    if( ! pResMgr )
        return new PopupMenu();

    PopupMenu* pPopup = new PopupMenu( ResId( SV_RESID_MENU_EDIT, *pResMgr ) );
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if ( rStyleSettings.GetHideDisabledMenuItems() )
        pPopup->SetMenuFlags( MENU_FLAG_HIDEDISABLEDENTRIES );
    else
        pPopup->SetMenuFlags ( MENU_FLAG_ALWAYSSHOWDISABLEDENTRIES );
    if ( rStyleSettings.GetAcceleratorsInContextMenus() )
    {
        pPopup->SetAccelKey( SV_MENU_EDIT_UNDO, KeyCode( KEYFUNC_UNDO ) );
        pPopup->SetAccelKey( SV_MENU_EDIT_CUT, KeyCode( KEYFUNC_CUT ) );
        pPopup->SetAccelKey( SV_MENU_EDIT_COPY, KeyCode( KEYFUNC_COPY ) );
        pPopup->SetAccelKey( SV_MENU_EDIT_PASTE, KeyCode( KEYFUNC_PASTE ) );
        pPopup->SetAccelKey( SV_MENU_EDIT_DELETE, KeyCode( KEYFUNC_DELETE ) );
        pPopup->SetAccelKey( SV_MENU_EDIT_SELECTALL, KeyCode( KEY_A, sal_False, sal_True, sal_False, sal_False ) );
        pPopup->SetAccelKey( SV_MENU_EDIT_INSERTSYMBOL, KeyCode( KEY_S, sal_True, sal_True, sal_False, sal_False ) );
    }
    return pPopup;
}

// -----------------------------------------------------------------------

void Edit::DeletePopupMenu( PopupMenu* pMenu )
{
    delete pMenu;
}

// ::com::sun::star::datatransfer::dnd::XDragGestureListener
void Edit::dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& rDGE ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aVclGuard;

    if ( !IsTracking() && maSelection.Len() &&
         !(GetStyle() & WB_PASSWORD) && (!mpDDInfo || mpDDInfo->bStarterOfDD == sal_False) ) // Kein Mehrfach D&D
    {
        Selection aSel( maSelection );
        aSel.Justify();

        // Nur wenn Maus in der Selektion...
        Point aMousePos( rDGE.DragOriginX, rDGE.DragOriginY );
        xub_StrLen nChar = ImplGetCharPos( aMousePos );
        if ( (nChar >= aSel.Min()) && (nChar < aSel.Max()) )
        {
            if ( !mpDDInfo )
                mpDDInfo = new DDInfo;

            mpDDInfo->bStarterOfDD = sal_True;
            mpDDInfo->aDndStartSel = aSel;


            if ( IsTracking() )
                EndTracking();  // Vor D&D Tracking ausschalten

            ::vcl::unohelper::TextDataObject* pDataObj = new ::vcl::unohelper::TextDataObject( GetSelected() );
            sal_Int8 nActions = datatransfer::dnd::DNDConstants::ACTION_COPY;
            if ( !IsReadOnly() )
                nActions |= datatransfer::dnd::DNDConstants::ACTION_MOVE;
            rDGE.DragSource->startDrag( rDGE, nActions, 0 /*cursor*/, 0 /*image*/, pDataObj, mxDnDListener );
            if ( GetCursor() )
                GetCursor()->Hide();

        }
    }
}

// ::com::sun::star::datatransfer::dnd::XDragSourceListener
void Edit::dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& rDSDE ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aVclGuard;

    if ( rDSDE.DropSuccess && ( rDSDE.DropAction & datatransfer::dnd::DNDConstants::ACTION_MOVE ) )
    {
        Selection aSel( mpDDInfo->aDndStartSel );
        if ( mpDDInfo->bDroppedInMe )
        {
            if ( aSel.Max() > mpDDInfo->nDropPos )
            {
                long nLen = aSel.Len();
                aSel.Min() += nLen;
                aSel.Max() += nLen;
            }
        }
        ImplDelete( aSel, EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
        ImplModified();
    }

    ImplHideDDCursor();
    delete mpDDInfo;
    mpDDInfo = NULL;
}

// ::com::sun::star::datatransfer::dnd::XDropTargetListener
void Edit::drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aVclGuard;

    sal_Bool bChanges = sal_False;
    if ( !mbReadOnly && mpDDInfo )
    {
        ImplHideDDCursor();

        Selection aSel( maSelection );
        aSel.Justify();

        if ( aSel.Len() && !mpDDInfo->bStarterOfDD )
            ImplDelete( aSel, EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );

        mpDDInfo->bDroppedInMe = sal_True;

        aSel.Min() = mpDDInfo->nDropPos;
        aSel.Max() = mpDDInfo->nDropPos;
        ImplSetSelection( aSel );

        uno::Reference< datatransfer::XTransferable > xDataObj = rDTDE.Transferable;
        if ( xDataObj.is() )
        {
            datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
            if ( xDataObj->isDataFlavorSupported( aFlavor ) )
            {
                uno::Any aData = xDataObj->getTransferData( aFlavor );
                ::rtl::OUString aText;
                aData >>= aText;
                ImplInsertText( aText );
                bChanges = sal_True;
                ImplModified();
            }
        }

        if ( !mpDDInfo->bStarterOfDD )
        {
            delete mpDDInfo;
            mpDDInfo = NULL;
        }
    }

    rDTDE.Context->dropComplete( bChanges );
}

void Edit::dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( !mpDDInfo )
    {
        mpDDInfo = new DDInfo;
    }
    // search for string data type
    const Sequence< com::sun::star::datatransfer::DataFlavor >& rFlavors( rDTDE.SupportedDataFlavors );
    sal_Int32 nEle = rFlavors.getLength();
    mpDDInfo->bIsStringSupported = sal_False;
    for( sal_Int32 i = 0; i < nEle; i++ )
    {
        sal_Int32 nIndex = 0;
        rtl::OUString aMimetype = rFlavors[i].MimeType.getToken( 0, ';', nIndex );
        if ( aMimetype == "text/plain" )
        {
            mpDDInfo->bIsStringSupported = sal_True;
            break;
        }
    }
}

void Edit::dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aVclGuard;

    ImplHideDDCursor();
}

void Edit::dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aVclGuard;

    Point aMousePos( rDTDE.LocationX, rDTDE.LocationY );

    xub_StrLen nPrevDropPos = mpDDInfo->nDropPos;
    mpDDInfo->nDropPos = ImplGetCharPos( aMousePos );

    /*
    Size aOutSize = GetOutputSizePixel();
    if ( ( aMousePos.X() < 0 ) || ( aMousePos.X() > aOutSize.Width() ) )
    {
        // Scroll?
        // No, I will not receive events in this case....
    }
    */

    Selection aSel( maSelection );
    aSel.Justify();

    // Don't accept drop in selection or read-only field...
    if ( IsReadOnly() || aSel.IsInside( mpDDInfo->nDropPos ) || ! mpDDInfo->bIsStringSupported )
    {
        ImplHideDDCursor();
        rDTDE.Context->rejectDrag();
    }
    else
    {
        // Alten Cursor wegzeichnen...
        if ( !mpDDInfo->bVisCursor || ( nPrevDropPos != mpDDInfo->nDropPos ) )
        {
            ImplHideDDCursor();
            ImplShowDDCursor();
        }
        rDTDE.Context->acceptDrag( rDTDE.DropAction );
    }
}

rtl::OUString Edit::GetSurroundingText() const
{
    if (mpSubEdit)
        return mpSubEdit->GetSurroundingText();
    return maText;
}

Selection Edit::GetSurroundingTextSelection() const
{
  return GetSelection();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
