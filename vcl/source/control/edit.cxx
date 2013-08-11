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


#include <tools/rc.h>
#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/cursor.hxx>
#include <vcl/virdev.hxx>
#include <vcl/menu.hxx>
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

#include <com/sun/star/i18n/InputSequenceChecker.hpp>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <com/sun/star/uno/Any.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <sal/macros.h>

#include <vcl/unohelp.hxx>
#include <vcl/unohelp2.hxx>

#include <officecfg/Office/Common.hxx>




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
    bool            bStarterOfDD;
    bool            bDroppedInMe;
    bool            bVisCursor;
    bool            bIsStringSupported;

    DDInfo()
    {
        aCursor.SetStyle( CURSOR_SHADOW );
        nDropPos = 0;
        bStarterOfDD = false;
        bDroppedInMe = false;
        bVisCursor = false;
        bIsStringSupported = false;
    }
};

// =======================================================================

struct Impl_IMEInfos
{
    OUString      aOldTextAfterStartPos;
    sal_uInt16*   pAttribs;
    xub_StrLen    nPos;
    xub_StrLen    nLen;
    bool          bCursor;
    bool          bWasCursorOverwrite;

                Impl_IMEInfos( xub_StrLen nPos, const OUString& rOldTextAfterStartPos );
                ~Impl_IMEInfos();

    void        CopyAttribs( const xub_StrLen* pA, xub_StrLen nL );
    void        DestroyAttribs();
};

// -----------------------------------------------------------------------

Impl_IMEInfos::Impl_IMEInfos( xub_StrLen nP, const OUString& rOldTextAfterStartPos )
 : aOldTextAfterStartPos( rOldTextAfterStartPos )
{
    nPos = nP;
    nLen = 0;
    bCursor = true;
    pAttribs = NULL;
    bWasCursorOverwrite = false;
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

void Edit::setMaxWidthChars(sal_Int32 nWidth)
{
    if (nWidth != mnMaxWidthChars)
    {
        mnMaxWidthChars = nWidth;
        queue_resize();
    }
}

bool Edit::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "width-chars")
        SetWidthInChars(rValue.toInt32());
    else if (rKey == "max-width-chars")
        setMaxWidthChars(rValue.toInt32());
    else if (rKey == "max-length")
    {
        sal_Int32 nTextLen = rValue.toInt32();
        SetMaxTextLen(nTextLen == 0 ? EDIT_NOLIMIT : nTextLen);
    }
    else if (rKey == "editable")
    {
        bool bReadOnly = !toBool(rValue);
        SetReadOnly(bReadOnly);
        //disable tab to traverse into readonly editables
        WinBits nBits = GetStyle();
        nBits &= ~(WB_TABSTOP|WB_NOTABSTOP);
        if (!bReadOnly)
            nBits |= WB_TABSTOP;
        else
            nBits |= WB_NOTABSTOP;
        SetStyle(nBits);
    }
    else if (rKey == "visibility")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_PASSWORD);
        if (!toBool(rValue))
            nBits |= WB_PASSWORD;
        SetStyle(nBits);
    }
    else if (rKey == "placeholder-text")
        SetPlaceholderText(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    else
        return Control::set_property(rKey, rValue);
    return true;
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
    mnMaxWidthChars         = -1;
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

OUString Edit::ImplGetText() const
{
    if ( mcEchoChar || (GetStyle() & WB_PASSWORD) )
    {
        sal_Unicode cEchoChar;
        if ( mcEchoChar )
            cEchoChar = mcEchoChar;
        else
            cEchoChar = '*';
        OUStringBuffer aText;
        comphelper::string::padToLength(aText, maText.getLength(), cEchoChar);
        return aText.makeStringAndClear();
    }
    else
        return maText.toString();
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

    OUString aText = ImplGetText();
    nStart = 0;
    nEnd = aText.getLength();

    sal_Int32   nDXBuffer[256];
    sal_Int32*  pDXBuffer = NULL;
    sal_Int32*  pDX = nDXBuffer;

    if( !aText.isEmpty() )
    {
        if( (size_t) (2*aText.getLength()) > SAL_N_ELEMENTS(nDXBuffer) )
        {
            pDXBuffer = new sal_Int32[2*(aText.getLength()+1)];
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
        OUString* pDisplayText = &mpControlData->mpLayoutData->m_aDisplayText;

        DrawText( aPos, aText, nStart, nEnd - nStart, pVector, pDisplayText );

        if( pDXBuffer )
            delete [] pDXBuffer;
        return;
    }

    Cursor* pCursor = GetCursor();
    bool bVisCursor = pCursor ? pCursor->IsVisible() : false;
    if ( pCursor )
        pCursor->Hide();

    ImplClearBackground( 0, GetOutputSizePixel().Width() );

    bool bPaintPlaceholderText = aText.isEmpty() && !maPlaceholderText.isEmpty();

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if ( IsEnabled() )
        ImplInitSettings( sal_False, sal_True, sal_False );
    if ( !IsEnabled() || bPaintPlaceholderText )
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

    bool bDrawSelection = maSelection.Len() && ( HasFocus() || ( GetStyle() & WB_NOHIDESELECTION ) || mbActivePopup );

    long nPos = nStart ? pDX[2*nStart] : 0;
    aPos.X() = nPos + mnXOffset + ImplGetExtraOffset();
    if ( bPaintPlaceholderText )
    {
        DrawText( aPos, maPlaceholderText );
    }
    else if ( !bDrawSelection && !mpIMEInfos )
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
        for( i = 0; i < aText.getLength(); i++ )
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
                            aFont.SetUnderline( UNDERLINE_DASHDOT );
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
    OUString aText = ImplGetText();

    // loeschen moeglich?
    if ( !rSelection.Len() &&
         (((rSelection.Min() == 0) && (nDirection == EDIT_DEL_LEFT)) ||
          ((rSelection.Max() == aText.getLength()) && (nDirection == EDIT_DEL_RIGHT))) )
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
                i18n::Boundary aBoundary = xBI->getWordBoundary( maText.toString(), aSelection.Min(),
                        GetSettings().GetLanguageTag().getLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
                if ( aBoundary.startPos == aSelection.Min() )
                    aBoundary = xBI->previousWord( maText.toString(), aSelection.Min(),
                            GetSettings().GetLanguageTag().getLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
                aSelection.Min() = aBoundary.startPos;
            }
            else if ( nMode == EDIT_DELMODE_RESTOFCONTENT )
               {
                aSelection.Min() = 0;
            }
            else
            {
                sal_Int32 nCount = 1;
                aSelection.Min() = xBI->previousCharacters( maText.toString(), aSelection.Min(),
                        GetSettings().GetLanguageTag().getLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
            }
        }
        else
        {
            if ( nMode == EDIT_DELMODE_RESTOFWORD )
            {
                i18n::Boundary aBoundary = xBI->nextWord( maText.toString(), aSelection.Max(),
                        GetSettings().GetLanguageTag().getLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
                aSelection.Max() = aBoundary.startPos;
            }
            else if ( nMode == EDIT_DELMODE_RESTOFCONTENT )
            {
                aSelection.Max() = aText.getLength();
            }
            else
            {
                sal_Int32 nCount = 1;
                aSelection.Max() = xBI->nextCharacters( maText.toString(), aSelection.Max(),
                        GetSettings().GetLanguageTag().getLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
            }
        }
    }

    maText.remove( static_cast<sal_Int32>(aSelection.Min()), static_cast<sal_Int32>(aSelection.Len()) );
    maSelection.Min() = aSelection.Min();
    maSelection.Max() = aSelection.Min();
    ImplAlignAndPaint();
    mbInternModified = sal_True;
}

// -----------------------------------------------------------------------

OUString Edit::ImplGetValidString( const OUString& rString ) const
{
    OUString aValidString( rString );
    aValidString = comphelper::string::remove(aValidString, '\n');
    aValidString = comphelper::string::remove(aValidString, '\r');
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
        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        xISC = i18n::InputSequenceChecker::create(xContext);
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

bool Edit::ImplTruncateToMaxLen( OUString& rStr, sal_uInt32 nSelectionLen ) const
{
    bool bWasTruncated = false;
    const sal_uInt32 nMaxLen = mnMaxTextLen < 65534 ? mnMaxTextLen : 65534;
    sal_uInt32 nLenAfter = static_cast<sal_uInt32>(maText.getLength()) + rStr.getLength() - nSelectionLen;
    if ( nLenAfter > nMaxLen )
    {
        sal_uInt32 nErasePos = nMaxLen - static_cast<sal_uInt32>(maText.getLength()) + nSelectionLen;
        rStr = rStr.copy( 0, nErasePos );
        bWasTruncated = true;
    }
    return bWasTruncated;
}

// -----------------------------------------------------------------------

void Edit::ImplInsertText( const OUString& rStr, const Selection* pNewSel, sal_Bool bIsUserInput )
{
    Selection aSelection( maSelection );
    aSelection.Justify();

    OUString aNewText( ImplGetValidString( rStr ) );
    ImplTruncateToMaxLen( aNewText, aSelection.Len() );

    ImplClearLayoutData();

    if ( aSelection.Len() )
        maText.remove( static_cast<sal_Int32>(aSelection.Min()), static_cast<sal_Int32>(aSelection.Len()) );
    else if ( !mbInsertMode && (aSelection.Max() < maText.getLength()) )
        maText.remove( static_cast<sal_Int32>(aSelection.Max()), 1 );

    // take care of input-sequence-checking now
    if (bIsUserInput && !rStr.isEmpty())
    {
        DBG_ASSERT( rStr.getLength() == 1, "unexpected string length. User input is expected to providse 1 char only!" );

        // determine if input-sequence-checking should be applied or not
        //
        uno::Reference < i18n::XBreakIterator > xBI( ImplGetBreakIterator(), UNO_QUERY );
        bool bIsInputSequenceChecking = rStr.getLength() == 1 &&
                officecfg::Office::Common::I18N::CTL::CTLFont::get() &&
                officecfg::Office::Common::I18N::CTL::CTLSequenceChecking::get() &&
                aSelection.Min() > 0 && /* first char needs not to be checked */
                xBI.is() && i18n::ScriptType::COMPLEX == xBI->getScriptType( rStr, 0 );


        uno::Reference < i18n::XExtendedInputSequenceChecker > xISC;
        if (bIsInputSequenceChecking && (xISC = ImplGetInputSequenceChecker()).is())
        {
            sal_Unicode cChar = rStr[0];
            sal_Int32 nTmpPos = static_cast< sal_Int32 >( aSelection.Min() );
            sal_Int16 nCheckMode = officecfg::Office::Common::I18N::CTL::CTLSequenceCheckingRestricted::get()?
                    i18n::InputSequenceCheckMode::STRICT : i18n::InputSequenceCheckMode::BASIC;

            // the text that needs to be checked is only the one
            // before the current cursor position
            OUString aOldText( maText.getStr(), nTmpPos);
            OUString aTmpText( aOldText );
            if (officecfg::Office::Common::I18N::CTL::CTLSequenceCheckingTypeAndReplace::get())
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

                OUString aChgText( aTmpText.copy( nChgPos ) );

                // remove text from first pos to be changed to current pos
                maText.remove( nChgPos, nTmpPos - nChgPos );

                if (!aChgText.isEmpty())
                {
                    aNewText = aChgText;
                    aSelection.Min() = nChgPos; // position for new text to be inserted
                }
                else
                    aNewText = "";
            }
            else
            {
                // should the character be ignored (i.e. not get inserted) ?
                if (!xISC->checkInputSequence( aOldText, nTmpPos - 1, cChar, nCheckMode ))
                    aNewText = "";
            }
        }

        // at this point now we will insert the non-empty text 'normally' some lines below...
    }

    if ( !aNewText.isEmpty() )
        maText.insert( static_cast<sal_Int32>(aSelection.Min()), aNewText );

    if ( !pNewSel )
    {
        maSelection.Min() = aSelection.Min() + aNewText.getLength();
        maSelection.Max() = maSelection.Min();
    }
    else
    {
        maSelection = *pNewSel;
        if ( maSelection.Min() > maText.getLength() )
            maSelection.Min() = maText.getLength();
        if ( maSelection.Max() > maText.getLength() )
            maSelection.Max() = maText.getLength();
    }

    ImplAlignAndPaint();
    mbInternModified = sal_True;
}

// -----------------------------------------------------------------------

void Edit::ImplSetText( const OUString& rText, const Selection* pNewSelection )
{
    // we delete text by "selecting" the old text completely then calling InsertText; this is flicker free
    if ( ( rText.getLength() <= mnMaxTextLen ) &&
         ( (rText != maText.getStr()) || (pNewSelection && (*pNewSelection != maSelection)) ) )
    {
        ImplClearLayoutData();
        maSelection.Min() = 0;
        maSelection.Max() = maText.getLength();
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

int Edit::ImplGetNativeControlType() const
{
    int nCtrl = 0;
    const Window *pControl = mbIsSubEdit ? GetParent() : this;

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
    OUString    aText = ImplGetText();

    long nTextPos = 0;

    sal_Int32   nDXBuffer[256];
    sal_Int32*  pDXBuffer = NULL;
    sal_Int32*  pDX = nDXBuffer;

    if( !aText.isEmpty() )
    {
        if( (size_t) (2*aText.getLength()) > SAL_N_ELEMENTS(nDXBuffer) )
        {
            pDXBuffer = new sal_Int32[2*(aText.getLength()+1)];
            pDX = pDXBuffer;
        }

        GetCaretPositions( aText, pDX, 0, aText.getLength() );

        if( maSelection.Max() < aText.getLength() )
            nTextPos = pDX[ 2*maSelection.Max() ];
        else
            nTextPos = pDX[ 2*aText.getLength()-1 ];
    }

    long nCursorWidth = 0;
    if ( !mbInsertMode && !maSelection.Len() && (maSelection.Max() < aText.getLength()) )
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
    OUString aText = ImplGetText();

    sal_Int32   nDXBuffer[256];
    sal_Int32*  pDXBuffer = NULL;
    sal_Int32*  pDX = nDXBuffer;
    if( (size_t) (2*aText.getLength()) > SAL_N_ELEMENTS(nDXBuffer) )
    {
        pDXBuffer = new sal_Int32[2*(aText.getLength()+1)];
        pDX = pDXBuffer;
    }

    GetCaretPositions( aText, pDX, 0, aText.getLength() );
    long nX = rWindowPos.X() - mnXOffset - ImplGetExtraOffset();
    for( int i = 0; i < aText.getLength(); i++ )
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
        long nDiff = std::abs( pDX[0]-nX );
        for( int i = 1; i < aText.getLength(); i++ )
        {
            long nNewDiff = std::abs( pDX[2*i]-nX );

            if( nNewDiff < nDiff )
            {
                nIndex = sal::static_int_cast<xub_StrLen>(i);
                nDiff = nNewDiff;
            }
        }
        if( nIndex == aText.getLength()-1 && std::abs( pDX[2*nIndex+1] - nX ) < nDiff )
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
                OUString aText;
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
            i18n::Boundary aBoundary = xBI->getWordBoundary( maText.toString(), aSelection.Max(),
                     GetSettings().GetLanguageTag().getLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
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
            ImplSetSelection( Selection( 0, maText.getLength() ) );
            bDone = sal_True;
        }
        else if ( rKEvt.GetKeyCode().IsShift() && (nCode == KEY_S) )
        {
            if ( pImplFncGetSpecialChars )
            {
                Selection aSaveSel = GetSelection(); // if someone changes the selection in Get/LoseFocus, e.g. URL bar
                OUString aChars = pImplFncGetSpecialChars( this, GetFont() );
                SetSelection( aSaveSel );
                if ( !aChars.isEmpty() )
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
                ImplSetSelection( Selection( 0, maText.getLength() ) );
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
                            i18n::Boundary aBoundary = xBI->getWordBoundary( maText.toString(), aSel.Max(),
                                    GetSettings().GetLanguageTag().getLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
                            if ( aBoundary.startPos == aSel.Max() )
                                aBoundary = xBI->previousWord( maText.toString(), aSel.Max(),
                                        GetSettings().GetLanguageTag().getLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
                            aSel.Max() = aBoundary.startPos;
                        }
                        else
                        {
                            sal_Int32 nCount = 1;
                            aSel.Max() = xBI->previousCharacters( maText.toString(), aSel.Max(),
                                    GetSettings().GetLanguageTag().getLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
                        }
                    }
                    else if ( bGoRight && ( aSel.Max() < maText.getLength() ) )
                    {
                        if ( bWord )
                           {
                            i18n::Boundary aBoundary = xBI->nextWord( maText.toString(), aSel.Max(),
                                    GetSettings().GetLanguageTag().getLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
                            aSel.Max() = aBoundary.startPos;
                        }
                        else
                        {
                            sal_Int32 nCount = 1;
                            aSel.Max() = xBI->nextCharacters( maText.toString(), aSel.Max(),
                                    GetSettings().GetLanguageTag().getLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
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
                        if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.getLength()) )
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
                    sal_Int32 nOldLen = maText.getLength();
                    ImplDelete( maSelection, nDel, nMode );
                    if ( maText.getLength() != nOldLen )
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
                        ImplInsertText(OUString(rKEvt.GetCharCode()), 0, sal_True);
                        if ( maAutocompleteHdl.IsSet() )
                        {
                            if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.getLength()) )
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
    bool bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
    bool bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
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

    OUString    aText = ImplGetText();
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
        maUndoText = maText.toString();

        sal_uLong nSelOptions = GetSettings().GetStyleSettings().GetSelectionOptions();
        if ( !( GetStyle() & (WB_NOHIDESELECTION|WB_READONLY) )
                && ( GetGetFocusFlags() & (GETFOCUS_INIT|GETFOCUS_TAB|GETFOCUS_CURSOR|GETFOCUS_MNEMONIC) ) )
        {
            if ( nSelOptions & SELECTION_OPTION_SHOWFIRST )
            {
                maSelection.Min() = maText.getLength();
                maSelection.Max() = 0;
            }
            else
            {
                maSelection.Min() = 0;
                maSelection.Max() = maText.getLength();
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

        if ( maUndoText == maText.getStr() )
            pPopup->EnableItem( SV_MENU_EDIT_UNDO, sal_False );
        if ( ( maSelection.Min() == 0 ) && ( maSelection.Max() == maText.getLength() ) )
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
                ImplSetSelection( Selection( 0, maText.getLength() ) );
                break;
            case SV_MENU_EDIT_INSERTSYMBOL:
                {
                    OUString aChars = pImplFncGetSpecialChars( this, GetFont() );
                    SetSelection( aSaveSel );
                    if ( !aChars.isEmpty() )
                    {
                        ImplInsertText( aChars );
                        ImplModified();
                    }
                }
                break;
        }
        mbActivePopup = sal_False;
    }
    else if ( rCEvt.GetCommand() == COMMAND_STARTEXTTEXTINPUT )
    {
        DeleteSelected();
        delete mpIMEInfos;
        sal_Int32 nPos = static_cast<sal_Int32>(maSelection.Max());
        mpIMEInfos = new Impl_IMEInfos( nPos, OUString(maText.getStr() + nPos ) );
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
            if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.getLength()) )
            {
                meAutocompleteAction = AUTOCOMPLETE_KEYINPUT;
                maAutocompleteHdl.Call( this );
            }
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_EXTTEXTINPUT )
    {
        const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();

        maText.remove( mpIMEInfos->nPos, mpIMEInfos->nLen );
        maText.insert( mpIMEInfos->nPos, pData->GetText() );
        if ( mpIMEInfos->bWasCursorOverwrite )
        {
            sal_Int32 nOldIMETextLen = mpIMEInfos->nLen;
            sal_Int32 nNewIMETextLen = pData->GetText().getLength();
            if ( ( nOldIMETextLen > nNewIMETextLen ) &&
                 ( nNewIMETextLen < mpIMEInfos->aOldTextAfterStartPos.getLength() ) )
            {
                // restore old characters
                sal_uInt16 nRestore = nOldIMETextLen - nNewIMETextLen;
                maText.insert( mpIMEInfos->nPos + nNewIMETextLen, mpIMEInfos->aOldTextAfterStartPos.copy( nNewIMETextLen, nRestore ) );
            }
            else if ( ( nOldIMETextLen < nNewIMETextLen ) &&
                      ( nOldIMETextLen < mpIMEInfos->aOldTextAfterStartPos.getLength() ) )
            {
                // overwrite
                sal_uInt16 nOverwrite = nNewIMETextLen - nOldIMETextLen;
                if ( ( nOldIMETextLen + nOverwrite ) > mpIMEInfos->aOldTextAfterStartPos.getLength() )
                    nOverwrite = mpIMEInfos->aOldTextAfterStartPos.getLength() - nOldIMETextLen;
                maText.remove( mpIMEInfos->nPos + nNewIMETextLen, nOverwrite );
            }
        }


        if ( pData->GetTextAttr() )
        {
            mpIMEInfos->CopyAttribs( pData->GetTextAttr(), pData->GetText().getLength() );
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
            SetCursorRect( NULL, GetTextWidth( maText.toString(), nCursorPos, mpIMEInfos->nPos+mpIMEInfos->nLen-nCursorPos ) );
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
    else if ( rCEvt.GetCommand() == COMMAND_QUERYCHARPOSITION )
    {
        if (mpIMEInfos && mpIMEInfos->nLen > 0)
        {
            OUString aText = ImplGetText();
            sal_Int32   nDXBuffer[256];
            sal_Int32*  pDXBuffer = NULL;
            sal_Int32*  pDX = nDXBuffer;

            if( !aText.isEmpty() )
            {
                if( (size_t) (2*aText.getLength()) > SAL_N_ELEMENTS(nDXBuffer) )
                {
                    pDXBuffer = new sal_Int32[2*(aText.getLength()+1)];
                    pDX = pDXBuffer;
                }

                GetCaretPositions( aText, pDX, 0, aText.getLength() );
            }
            long    nTH = GetTextHeight();
            Point   aPos( mnXOffset, ImplGetTextYPosition() );

            Rectangle* aRects = new Rectangle[ mpIMEInfos->nLen ];
            for ( int nIndex = 0; nIndex < mpIMEInfos->nLen; ++nIndex )
            {
                Rectangle aRect( aPos, Size( 10, nTH ) );
                aRect.Left() = pDX[2*(nIndex+mpIMEInfos->nPos)] + mnXOffset + ImplGetExtraOffset();
                aRects[ nIndex ] = aRect;
            }
            SetCompositionCharRect( aRects, mpIMEInfos->nLen );
            delete[] aRects;
            delete[] pDXBuffer;
        }
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
        if ( !maText.isEmpty() && ( mnAlign != nOldAlign ) )
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
        long nTextWidth = GetTextWidth( maText.toString(), 0, mpDDInfo->nDropPos );
        long nTextHeight = GetTextHeight();
        Rectangle aCursorRect( Point( nTextWidth + mnXOffset, (GetOutputSize().Height()-nTextHeight)/2 ), Size( 2, nTextHeight ) );
        mpDDInfo->aCursor.SetWindow( this );
        mpDDInfo->aCursor.SetPos( aCursorRect.TopLeft() );
        mpDDInfo->aCursor.SetSize( aCursorRect.GetSize() );
        mpDDInfo->aCursor.Show();
        mpDDInfo->bVisCursor = true;
    }
}

// -----------------------------------------------------------------------

void Edit::ImplHideDDCursor()
{
    if ( mpDDInfo && mpDDInfo->bVisCursor )
    {
        mpDDInfo->aCursor.Hide();
        mpDDInfo->bVisCursor = false;
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
        if ( maText.getLength() > mnMaxTextLen )
            ImplDelete( Selection( mnMaxTextLen, maText.getLength() ), EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
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

            if ( aNew.Min() > maText.getLength() )
                aNew.Min() = maText.getLength();
            if ( aNew.Max() > maText.getLength() )
                aNew.Max() = maText.getLength();
            if ( aNew.Min() < 0 )
                aNew.Min() = 0;
            if ( aNew.Max() < 0 )
                aNew.Max() = 0;

            if ( aNew != maSelection )
            {
                ImplClearLayoutData();
                maSelection = aNew;

                if ( bPaint && ( aOld.Len() || aNew.Len() || IsPaintTransparent() ) )
                    ImplInvalidateOrRepaint( 0, maText.getLength() );
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

void Edit::ReplaceSelected( const OUString& rStr )
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

OUString Edit::GetSelected() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetSelected();
    else
    {
        Selection aSelection( maSelection );
        aSelection.Justify();
        return OUString( maText.getStr() + aSelection.Min(), aSelection.Len() );
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
        OUString aText( maText.toString() );
        ImplDelete( Selection( 0, aText.getLength() ), EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
        ImplInsertText( maUndoText );
        ImplSetSelection( Selection( 0, maUndoText.getLength() ) );
        maUndoText = aText;
    }
}

// -----------------------------------------------------------------------

void Edit::SetText( const OUString& rStr )
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

void Edit::SetText( const OUString& rStr, const Selection& rSelection )
{
    if ( mpSubEdit )
        mpSubEdit->SetText( rStr, rSelection );
    else
        ImplSetText( rStr, &rSelection );
}

// -----------------------------------------------------------------------

OUString Edit::GetText() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetText();
    else
        return maText.toString();
}

// -----------------------------------------------------------------------

void Edit::SetPlaceholderText( const OUString& rStr )
{
    if ( mpSubEdit )
        mpSubEdit->SetPlaceholderText( rStr );
    else if ( maPlaceholderText != rStr )
    {
        maPlaceholderText = rStr;
        if ( GetText().isEmpty() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

OUString Edit::GetPlaceholderText() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetPlaceholderText();

    return maPlaceholderText;
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

Size Edit::CalcMinimumSizeForText(const OUString &rString) const
{
    int eCtrlType = ImplGetNativeControlType();

    Size aSize;
    if (mnWidthInChars != -1)
    {
        //CalcSize calls CalcWindowSize, but we will call that also in this
        //function, so undo the first one with CalcOutputSize
        aSize = CalcOutputSize(CalcSize(mnWidthInChars));
    }
    else
    {
        OUString aString;
        if (mnMaxWidthChars != -1 && mnMaxWidthChars < rString.getLength())
            aString = rString.copy(0, mnMaxWidthChars);
        else
            aString = rString;

        aSize.Height() = GetTextHeight();
        aSize.Width() = GetTextWidth(aString);
        aSize.Width() += ImplGetExtraOffset() * 2;

        // do not create edit fields in which one cannot enter anything
        // a default minimum width should exist for at least 3 characters

        //CalcSize calls CalcWindowSize, but we will call that also in this
        //function, so undo the first one with CalcOutputSize
        Size aMinSize(CalcOutputSize(CalcSize(3)));
        if (aSize.Width() < aMinSize.Width())
            aSize.Width() = aMinSize.Width();
    }

    if (eCtrlType != CTRL_EDITBOX_NOBORDER)
    {
        // add some space between text entry and border
        aSize.Height() += 4;
    }

    aSize = CalcWindowSize( aSize );

    // ask NWF what if it has an opinion, too
    ImplControlValue aControlValue;
    Rectangle aRect( Point( 0, 0 ), aSize );
    Rectangle aContent, aBound;
    if( GetNativeControlRegion(
                   eCtrlType, PART_ENTIRE_CONTROL,
                   aRect, 0, aControlValue, OUString(), aBound, aContent) )
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

Size Edit::GetOptimalSize() const
{
    return CalcMinimumSize();
}

// -----------------------------------------------------------------------

Size Edit::CalcSize( xub_StrLen nChars ) const
{
    // width for N characters, independent from content.
    // works only correct for fixed fonts, average otherwise
    Size aSz( GetTextWidth( OUString('x') ), GetTextHeight() );
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
    long nCharWidth = GetTextWidth( OUString('x') );
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
         !(GetStyle() & WB_PASSWORD) && (!mpDDInfo || mpDDInfo->bStarterOfDD == false) ) // Kein Mehrfach D&D
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

            mpDDInfo->bStarterOfDD = true;
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

        mpDDInfo->bDroppedInMe = true;

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
                OUString aText;
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
    mpDDInfo->bIsStringSupported = false;
    for( sal_Int32 i = 0; i < nEle; i++ )
    {
        sal_Int32 nIndex = 0;
        OUString aMimetype = rFlavors[i].MimeType.getToken( 0, ';', nIndex );
        if ( aMimetype == "text/plain" )
        {
            mpDDInfo->bIsStringSupported = true;
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

OUString Edit::GetSurroundingText() const
{
    if (mpSubEdit)
        return mpSubEdit->GetSurroundingText();
    return maText.toString();
}

Selection Edit::GetSurroundingTextSelection() const
{
  return GetSelection();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
