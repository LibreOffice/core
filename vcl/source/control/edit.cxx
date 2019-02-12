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

#include <vcl/IDialogRenderable.hxx>
#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/cursor.hxx>
#include <vcl/virdev.hxx>
#include <vcl/menu.hxx>
#include <vcl/edit.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <vcl/ptrstyle.hxx>

#include <window.h>
#include <svdata.hxx>
#include <strings.hrc>
#include <controldata.hxx>

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
#include <sal/log.hxx>

#include <i18nlangtag/languagetag.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/unohelp2.hxx>

#include <officecfg/Office/Common.hxx>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

// - Redo
// - if Tracking-Cancel recreate DefaultSelection

static FncGetSpecialChars pImplFncGetSpecialChars = nullptr;

#define EDIT_ALIGN_LEFT             1
#define EDIT_ALIGN_CENTER           2
#define EDIT_ALIGN_RIGHT            3

#define EDIT_DEL_LEFT               1
#define EDIT_DEL_RIGHT              2

#define EDIT_DELMODE_SIMPLE         11
#define EDIT_DELMODE_RESTOFWORD     12
#define EDIT_DELMODE_RESTOFCONTENT  13

struct DDInfo
{
    vcl::Cursor     aCursor;
    Selection       aDndStartSel;
    sal_Int32       nDropPos;
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

struct Impl_IMEInfos
{
    OUString const aOldTextAfterStartPos;
    std::unique_ptr<ExtTextInputAttr[]>
                  pAttribs;
    sal_Int32 const nPos;
    sal_Int32     nLen;
    bool          bCursor;
    bool          bWasCursorOverwrite;

    Impl_IMEInfos(sal_Int32 nPos, const OUString& rOldTextAfterStartPos);

    void        CopyAttribs(const ExtTextInputAttr* pA, sal_Int32 nL);
    void        DestroyAttribs();
};

Impl_IMEInfos::Impl_IMEInfos(sal_Int32 nP, const OUString& rOldTextAfterStartPos)
    : aOldTextAfterStartPos(rOldTextAfterStartPos),
    nPos(nP),
    nLen(0),
    bCursor(true),
    bWasCursorOverwrite(false)
{
}

void Impl_IMEInfos::CopyAttribs(const ExtTextInputAttr* pA, sal_Int32 nL)
{
    nLen = nL;
    pAttribs.reset(new ExtTextInputAttr[ nL ]);
    memcpy( pAttribs.get(), pA, nL*sizeof(ExtTextInputAttr) );
}

void Impl_IMEInfos::DestroyAttribs()
{
    pAttribs.reset();
    nLen = 0;
}

Edit::Edit( WindowType nType )
    : Control( nType )
{
    ImplInitEditData();
}

Edit::Edit( vcl::Window* pParent, WinBits nStyle )
    : Control( WindowType::EDIT )
{
    ImplInitEditData();
    ImplInit( pParent, nStyle );
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

bool Edit::set_property(const OString &rKey, const OUString &rValue)
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
        SetReadOnly(!toBool(rValue));
    }
    else if (rKey == "visibility")
    {
        mbPassword = false;
        if (!toBool(rValue))
            mbPassword = true;
    }
    else if (rKey == "placeholder-text")
        SetPlaceholderText(rValue);
    else
        return Control::set_property(rKey, rValue);
    return true;
}

Edit::~Edit()
{
    disposeOnce();
}

void Edit::dispose()
{
    mpUIBuilder.reset();
    mpDDInfo.reset();

    vcl::Cursor* pCursor = GetCursor();
    if ( pCursor )
    {
        SetCursor( nullptr );
        delete pCursor;
    }

    mpIMEInfos.reset();
    mpUpdateDataTimer.reset();

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
        mxDnDListener.clear();
    }

    SetType(WindowType::WINDOW);

    mpSubEdit.disposeAndClear();
    Control::dispose();
}

void Edit::ImplInitEditData()
{
    mpSubEdit               = VclPtr<Edit>();
    mpUpdateDataTimer       = nullptr;
    mpFilterText            = nullptr;
    mnXOffset               = 0;
    mnAlign                 = EDIT_ALIGN_LEFT;
    mnMaxTextLen            = EDIT_NOLIMIT;
    mnWidthInChars          = -1;
    mnMaxWidthChars         = -1;
    mbModified              = false;
    mbInternModified        = false;
    mbReadOnly              = false;
    mbSelectAllSingleClick  = false;
    mbInsertMode            = true;
    mbClickedInSelection    = false;
    mbActivePopup           = false;
    mbIsSubEdit             = false;
    mbForceControlBackground = false;
    mbPassword              = false;
    mpDDInfo                = nullptr;
    mpIMEInfos              = nullptr;
    mcEchoChar              = 0;

    // no default mirroring for Edit controls
    // note: controls that use a subedit will revert this (SpinField, ComboBox)
    EnableRTL( false );

    vcl::unohelper::DragAndDropWrapper* pDnDWrapper = new vcl::unohelper::DragAndDropWrapper( this );
    mxDnDListener = pDnDWrapper;
}

bool Edit::ImplUseNativeBorder(vcl::RenderContext const & rRenderContext, WinBits nStyle)
{
    bool bRet = rRenderContext.IsNativeControlSupported(ImplGetNativeControlType(),
                                                        ControlPart::HasBackgroundTexture)
                                 && ((nStyle & WB_BORDER) && !(nStyle & WB_NOBORDER));
    if (!bRet && mbIsSubEdit)
    {
        vcl::Window* pWindow = GetParent();
        nStyle = pWindow->GetStyle();
        bRet = pWindow->IsNativeControlSupported(ImplGetNativeControlType(),
                                                 ControlPart::HasBackgroundTexture)
               && ((nStyle & WB_BORDER) && !(nStyle & WB_NOBORDER));
    }
    return bRet;
}

void Edit::ImplInit(vcl::Window* pParent, WinBits nStyle)
{
    nStyle = ImplInitStyle(nStyle);

    if (!(nStyle & (WB_CENTER | WB_RIGHT)))
        nStyle |= WB_LEFT;

    Control::ImplInit(pParent, nStyle, nullptr);

    mbReadOnly = (nStyle & WB_READONLY) != 0;

    mnAlign = EDIT_ALIGN_LEFT;

    // hack: right align until keyinput and cursor travelling works
    if( IsRTLEnabled() )
        mnAlign = EDIT_ALIGN_RIGHT;

    if ( nStyle & WB_RIGHT )
        mnAlign = EDIT_ALIGN_RIGHT;
    else if ( nStyle & WB_CENTER )
        mnAlign = EDIT_ALIGN_CENTER;

    SetCursor( new vcl::Cursor );

    SetPointer( PointerStyle::Text );

    uno::Reference< datatransfer::dnd::XDragGestureListener> xDGL( mxDnDListener, uno::UNO_QUERY );
    uno::Reference< datatransfer::dnd::XDragGestureRecognizer > xDGR = GetDragGestureRecognizer();
    if ( xDGR.is() )
    {
        xDGR->addDragGestureListener( xDGL );
        uno::Reference< datatransfer::dnd::XDropTargetListener> xDTL( mxDnDListener, uno::UNO_QUERY );
        GetDropTarget()->addDropTargetListener( xDTL );
        GetDropTarget()->setActive( true );
        GetDropTarget()->setDefaultActions( datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE );
    }
}

WinBits Edit::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;

    return nStyle;
}

bool Edit::IsCharInput( const KeyEvent& rKeyEvent )
{
    // In the future we must use new Unicode functions for this
    sal_Unicode cCharCode = rKeyEvent.GetCharCode();
    return ((cCharCode >= 32) && (cCharCode != 127) &&
            !rKeyEvent.GetKeyCode().IsMod3() &&
            !rKeyEvent.GetKeyCode().IsMod2() &&
            !rKeyEvent.GetKeyCode().IsMod1() );
}

void Edit::ImplModified()
{
    mbModified = true;
    Modify();
}

void Edit::ApplySettings(vcl::RenderContext& rRenderContext)
{
    Control::ApplySettings(rRenderContext);

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    const vcl::Font& aFont = rStyleSettings.GetFieldFont();
    ApplyControlFont(rRenderContext, aFont);

    ImplClearLayoutData();

    Color aTextColor = rStyleSettings.GetFieldTextColor();
    ApplyControlForeground(rRenderContext, aTextColor);

    if (ImplUseNativeBorder(rRenderContext, GetStyle()) || IsPaintTransparent())
    {
        // Transparent background
        rRenderContext.SetBackground();
        rRenderContext.SetFillColor();
    }
    else if (IsControlBackground())
    {
        rRenderContext.SetBackground(GetControlBackground());
        rRenderContext.SetFillColor(GetControlBackground());
    }
    else
    {
        rRenderContext.SetBackground(rStyleSettings.GetFieldColor());
        rRenderContext.SetFillColor(rStyleSettings.GetFieldColor());
    }
}

long Edit::ImplGetExtraXOffset() const
{
    // MT 09/2002: nExtraOffsetX should become a member, instead of checking every time,
    // but I need an incompatible update for this...
    // #94095# Use extra offset only when edit has a border
    long nExtraOffset = 0;
    if( ( GetStyle() & WB_BORDER ) || ( mbIsSubEdit && ( GetParent()->GetStyle() & WB_BORDER ) ) )
        nExtraOffset = 2;

    return nExtraOffset;
}

long Edit::ImplGetExtraYOffset() const
{
    long nExtraOffset = 0;
    ControlType eCtrlType = ImplGetNativeControlType();
    if (eCtrlType != ControlType::EditboxNoBorder)
    {
        // add some space between text entry and border
        nExtraOffset = 2;
    }
    return nExtraOffset;
}

OUString Edit::ImplGetText() const
{
    if ( mcEchoChar || mbPassword )
    {
        sal_Unicode cEchoChar;
        if ( mcEchoChar )
            cEchoChar = mcEchoChar;
        else
            cEchoChar = u'\x2022';
        OUStringBuffer aText;
        comphelper::string::padToLength(aText, maText.getLength(), cEchoChar);
        return aText.makeStringAndClear();
    }
    else
        return maText.toString();
}

void Edit::ImplInvalidateOrRepaint()
{
    if( IsPaintTransparent() )
    {
        Invalidate();
        // FIXME: this is currently only on macOS
        if( ImplGetSVData()->maNWFData.mbNoFocusRects )
            Update();
    }
    else
        Invalidate();
}

long Edit::ImplGetTextYPosition() const
{
    if ( GetStyle() & WB_TOP )
        return ImplGetExtraXOffset();
    else if ( GetStyle() & WB_BOTTOM )
        return GetOutputSizePixel().Height() - GetTextHeight() - ImplGetExtraXOffset();
    return ( GetOutputSizePixel().Height() - GetTextHeight() ) / 2;
}

void Edit::ImplRepaint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRectangle)
{
    if (!IsReallyVisible())
        return;

    ApplySettings(rRenderContext);

    const OUString aText = ImplGetText();
    const sal_Int32 nLen = aText.getLength();

    long nDXBuffer[256];
    std::unique_ptr<long[]> pDXBuffer;
    long* pDX = nDXBuffer;

    if (nLen)
    {
        if (static_cast<size_t>(2 * nLen) > SAL_N_ELEMENTS(nDXBuffer))
        {
            pDXBuffer.reset(new long[2 * (nLen + 1)]);
            pDX = pDXBuffer.get();
        }

        GetCaretPositions(aText, pDX, 0, nLen);
    }

    long nTH = GetTextHeight();
    Point aPos(mnXOffset, ImplGetTextYPosition());

    vcl::Cursor* pCursor = GetCursor();
    bool bVisCursor = pCursor && pCursor->IsVisible();
    if (pCursor)
        pCursor->Hide();

    ImplClearBackground(rRenderContext, rRectangle, 0, GetOutputSizePixel().Width()-1);

    bool bPaintPlaceholderText = aText.isEmpty() && !maPlaceholderText.isEmpty();

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    if (!IsEnabled() || bPaintPlaceholderText)
        rRenderContext.SetTextColor(rStyleSettings.GetDisableColor());

    // Set background color of the normal text
    if (mbForceControlBackground && IsControlBackground())
    {
        // check if we need to set ControlBackground even in NWF case
        rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(GetControlBackground());
        rRenderContext.DrawRect(tools::Rectangle(aPos, Size(GetOutputSizePixel().Width() - 2 * mnXOffset, GetOutputSizePixel().Height())));
        rRenderContext.Pop();

        rRenderContext.SetTextFillColor(GetControlBackground());
    }
    else if (IsPaintTransparent() || ImplUseNativeBorder(rRenderContext, GetStyle()))
        rRenderContext.SetTextFillColor();
    else
        rRenderContext.SetTextFillColor(IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor());

    ImplPaintBorder(rRenderContext);

    bool bDrawSelection = maSelection.Len() && (HasFocus() || (GetStyle() & WB_NOHIDESELECTION) || mbActivePopup);

    aPos.setX( mnXOffset + ImplGetExtraXOffset() );
    if (bPaintPlaceholderText)
    {
        rRenderContext.DrawText(aPos, maPlaceholderText);
    }
    else if (!bDrawSelection && !mpIMEInfos)
    {
        rRenderContext.DrawText(aPos, aText, 0, nLen);
    }
    else
    {
        // save graphics state
        rRenderContext.Push();
        // first calculate highlighted and non highlighted clip regions
        vcl::Region aHighlightClipRegion;
        vcl::Region aNormalClipRegion;
        Selection aTmpSel(maSelection);
        aTmpSel.Justify();
        // selection is highlighted
        for(sal_Int32 i = 0; i < nLen; ++i)
        {
            tools::Rectangle aRect(aPos, Size(10, nTH));
            aRect.SetLeft( pDX[2 * i] + mnXOffset + ImplGetExtraXOffset() );
            aRect.SetRight( pDX[2 * i + 1] + mnXOffset + ImplGetExtraXOffset() );
            aRect.Justify();
            bool bHighlight = false;
            if (i >= aTmpSel.Min() && i < aTmpSel.Max())
                bHighlight = true;

            if (mpIMEInfos && mpIMEInfos->pAttribs &&
                i >= mpIMEInfos->nPos && i < (mpIMEInfos->nPos+mpIMEInfos->nLen) &&
                (mpIMEInfos->pAttribs[i - mpIMEInfos->nPos] & ExtTextInputAttr::Highlight))
            {
                bHighlight = true;
            }

            if (bHighlight)
                aHighlightClipRegion.Union(aRect);
            else
                aNormalClipRegion.Union(aRect);
        }
        // draw normal text
        Color aNormalTextColor = rRenderContext.GetTextColor();
        rRenderContext.SetClipRegion(aNormalClipRegion);

        if (IsPaintTransparent())
            rRenderContext.SetTextFillColor();
        else
        {
            // Set background color when part of the text is selected
            if (ImplUseNativeBorder(rRenderContext, GetStyle()))
            {
                if( mbForceControlBackground && IsControlBackground() )
                    rRenderContext.SetTextFillColor(GetControlBackground());
                else
                    rRenderContext.SetTextFillColor();
            }
            else
            {
                rRenderContext.SetTextFillColor(IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor());
            }
        }
        rRenderContext.DrawText(aPos, aText, 0, nLen);

        // draw highlighted text
        rRenderContext.SetClipRegion(aHighlightClipRegion);
        rRenderContext.SetTextColor(rStyleSettings.GetHighlightTextColor());
        rRenderContext.SetTextFillColor(rStyleSettings.GetHighlightColor());
        rRenderContext.DrawText(aPos, aText, 0, nLen);

        // if IME info exists loop over portions and output different font attributes
        if (mpIMEInfos && mpIMEInfos->pAttribs)
        {
            for(int n = 0; n < 2; n++)
            {
                vcl::Region aRegion;
                if (n == 0)
                {
                    rRenderContext.SetTextColor(aNormalTextColor);
                    if (IsPaintTransparent())
                        rRenderContext.SetTextFillColor();
                    else
                        rRenderContext.SetTextFillColor(IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor());
                    aRegion = aNormalClipRegion;
                }
                else
                {
                    rRenderContext.SetTextColor(rStyleSettings.GetHighlightTextColor());
                    rRenderContext.SetTextFillColor(rStyleSettings.GetHighlightColor());
                    aRegion = aHighlightClipRegion;
                }

                for(int i = 0; i < mpIMEInfos->nLen; )
                {
                    ExtTextInputAttr nAttr = mpIMEInfos->pAttribs[i];
                    vcl::Region aClip;
                    int nIndex = i;
                    while (nIndex < mpIMEInfos->nLen && mpIMEInfos->pAttribs[nIndex] == nAttr)  // #112631# check nIndex before using it
                    {
                        tools::Rectangle aRect( aPos, Size( 10, nTH ) );
                        aRect.SetLeft( pDX[2 * (nIndex + mpIMEInfos->nPos)] + mnXOffset + ImplGetExtraXOffset() );
                        aRect.SetRight( pDX[2 * (nIndex + mpIMEInfos->nPos) + 1] + mnXOffset + ImplGetExtraXOffset() );
                        aRect.Justify();
                        aClip.Union(aRect);
                        nIndex++;
                    }
                    i = nIndex;
                    aClip.Intersect(aRegion);
                    if (!aClip.IsEmpty() && nAttr != ExtTextInputAttr::NONE)
                    {
                        vcl::Font aFont = rRenderContext.GetFont();
                        if (nAttr & ExtTextInputAttr::Underline)
                            aFont.SetUnderline(LINESTYLE_SINGLE);
                        else if (nAttr & ExtTextInputAttr::BoldUnderline)
                            aFont.SetUnderline( LINESTYLE_BOLD);
                        else if (nAttr & ExtTextInputAttr::DottedUnderline)
                            aFont.SetUnderline( LINESTYLE_DOTTED);
                        else if (nAttr & ExtTextInputAttr::DashDotUnderline)
                            aFont.SetUnderline( LINESTYLE_DASHDOT);
                        else if (nAttr & ExtTextInputAttr::GrayWaveline)
                        {
                            aFont.SetUnderline(LINESTYLE_WAVE);
                            rRenderContext.SetTextLineColor(COL_LIGHTGRAY);
                        }
                        rRenderContext.SetFont(aFont);

                        if (nAttr & ExtTextInputAttr::RedText)
                            rRenderContext.SetTextColor(COL_RED);
                        else if (nAttr & ExtTextInputAttr::HalfToneText)
                            rRenderContext.SetTextColor(COL_LIGHTGRAY);

                        rRenderContext.SetClipRegion(aClip);
                        rRenderContext.DrawText(aPos, aText, 0, nLen);
                    }
                }
            }
        }

        // restore graphics state
        rRenderContext.Pop();
    }

    if (bVisCursor && (!mpIMEInfos || mpIMEInfos->bCursor))
        pCursor->Show();
}

void Edit::ImplDelete( const Selection& rSelection, sal_uInt8 nDirection, sal_uInt8 nMode )
{
    const sal_Int32 nTextLen = ImplGetText().getLength();

    // deleting possible?
    if ( !rSelection.Len() &&
         (((rSelection.Min() == 0) && (nDirection == EDIT_DEL_LEFT)) ||
          ((rSelection.Max() == nTextLen) && (nDirection == EDIT_DEL_RIGHT))) )
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
                        GetSettings().GetLanguageTag().getLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, true );
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
                aSelection.Max() = nTextLen;
            }
            else
            {
                sal_Int32 nCount = 1;
                aSelection.Max() = xBI->nextCharacters( maText.toString(), aSelection.Max(),
                        GetSettings().GetLanguageTag().getLocale(), i18n::CharacterIteratorMode::SKIPCHARACTER, nCount, nCount );
            }
        }
    }

    const auto nSelectionMin = aSelection.Min();
    maText.remove( static_cast<sal_Int32>(nSelectionMin), static_cast<sal_Int32>(aSelection.Len()) );
    maSelection.Min() = nSelectionMin;
    maSelection.Max() = nSelectionMin;
    ImplAlignAndPaint();
    mbInternModified = true;
}

OUString Edit::ImplGetValidString( const OUString& rString )
{
    OUString aValidString( rString );
    aValidString = aValidString.replaceAll("\n", "").replaceAll("\r", "");
    aValidString = aValidString.replace('\t', ' ');
    return aValidString;
}

uno::Reference < i18n::XBreakIterator > Edit::ImplGetBreakIterator()
{
    //!! since we don't want to become incompatible in the next minor update
    //!! where this code will get integrated into, xISC will be a local
    //!! variable instead of a class member!
    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    return i18n::BreakIterator::create(xContext);
}

uno::Reference < i18n::XExtendedInputSequenceChecker > const & Edit::ImplGetInputSequenceChecker()
{
    if ( !mxISC.is() )
    {
        mxISC = i18n::InputSequenceChecker::create(
                ::comphelper::getProcessComponentContext() );
    }
    return mxISC;
}

void Edit::ShowTruncationWarning(weld::Widget* pParent)
{
    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent, VclMessageType::Warning,
                                              VclButtonsType::Ok, VclResId(SV_EDIT_WARNING_STR)));
    xBox->run();
}

bool Edit::ImplTruncateToMaxLen( OUString& rStr, sal_Int32 nSelectionLen ) const
{
    bool bWasTruncated = false;
    if (maText.getLength() - nSelectionLen > mnMaxTextLen - rStr.getLength())
    {
        sal_Int32 nErasePos = mnMaxTextLen - maText.getLength() + nSelectionLen;
        rStr = rStr.copy( 0, nErasePos );
        bWasTruncated = true;
    }
    return bWasTruncated;
}

void Edit::ImplInsertText( const OUString& rStr, const Selection* pNewSel, bool bIsUserInput )
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
        SAL_WARN_IF( rStr.getLength() != 1, "vcl", "unexpected string length. User input is expected to provide 1 char only!" );

        // determine if input-sequence-checking should be applied or not

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
            const OUString aOldText( maText.getStr(), nTmpPos);
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

                const OUString aChgText( aTmpText.copy( nChgPos ) );

                // remove text from first pos to be changed to current pos
                maText.remove( nChgPos, nTmpPos - nChgPos );

                if (!aChgText.isEmpty())
                {
                    aNewText = aChgText;
                    aSelection.Min() = nChgPos; // position for new text to be inserted
                }
                else
                    aNewText.clear();
            }
            else
            {
                // should the character be ignored (i.e. not get inserted) ?
                if (!xISC->checkInputSequence( aOldText, nTmpPos - 1, cChar, nCheckMode ))
                    aNewText.clear();
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
    mbInternModified = true;
}

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
                ImplSetSelection( *pNewSelection, false );

            if ( mnXOffset && !pNewSelection )
                maSelection.Max() = 0;

            Invalidate();
        }
        else
            ImplInsertText( rText, pNewSelection );

        CallEventListeners( VclEventId::EditModify );
    }
}

ControlType Edit::ImplGetNativeControlType() const
{
    ControlType nCtrl = ControlType::Generic;
    const vcl::Window* pControl = mbIsSubEdit ? GetParent() : this;

    switch (pControl->GetType())
    {
        case WindowType::COMBOBOX:
        case WindowType::PATTERNBOX:
        case WindowType::NUMERICBOX:
        case WindowType::METRICBOX:
        case WindowType::CURRENCYBOX:
        case WindowType::DATEBOX:
        case WindowType::TIMEBOX:
        case WindowType::LONGCURRENCYBOX:
            nCtrl = ControlType::Combobox;
            break;

        case WindowType::MULTILINEEDIT:
            if ( GetWindow( GetWindowType::Border ) != this )
                nCtrl = ControlType::MultilineEditbox;
            else
                nCtrl = ControlType::EditboxNoBorder;
            break;

        case WindowType::EDIT:
        case WindowType::PATTERNFIELD:
        case WindowType::METRICFIELD:
        case WindowType::CURRENCYFIELD:
        case WindowType::DATEFIELD:
        case WindowType::TIMEFIELD:
        case WindowType::LONGCURRENCYFIELD:
        case WindowType::NUMERICFIELD:
        case WindowType::SPINFIELD:
            if (pControl->GetStyle() & WB_SPIN)
                nCtrl = ControlType::Spinbox;
            else
            {
                if (GetWindow(GetWindowType::Border) != this)
                    nCtrl = ControlType::Editbox;
                else
                    nCtrl = ControlType::EditboxNoBorder;
            }
            break;

        default:
            nCtrl = ControlType::Editbox;
    }
    return nCtrl;
}

void Edit::ImplClearBackground(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRectangle, long nXStart, long nXEnd )
{
    /*
    * note: at this point the cursor must be switched off already
    */
    tools::Rectangle aRect(Point(), GetOutputSizePixel());
    aRect.SetLeft( nXStart );
    aRect.SetRight( nXEnd );

    if( !(ImplUseNativeBorder(rRenderContext, GetStyle()) || IsPaintTransparent()))
        rRenderContext.Erase(aRect);
    else if (SupportsDoubleBuffering() && mbIsSubEdit)
    {
        // ImplPaintBorder() is a NOP, we have a native border, and this is a sub-edit of a control.
        // That means we have to draw the parent native widget to paint the edit area to clear our background.
        PaintBufferGuard g(ImplGetWindowImpl()->mpFrameData, GetParent());
        GetParent()->Paint(rRenderContext, rRectangle);
    }
}

void Edit::ImplPaintBorder(vcl::RenderContext const & rRenderContext)
{
    // this is not needed when double-buffering
    if (SupportsDoubleBuffering())
        return;

    if (ImplUseNativeBorder(rRenderContext, GetStyle()) || IsPaintTransparent())
    {
        // draw the inner part by painting the whole control using its border window
        vcl::Window* pBorder = GetWindow(GetWindowType::Border);
        if (pBorder == this)
        {
            // we have no border, use parent
            vcl::Window* pControl = mbIsSubEdit ? GetParent() : this;
            pBorder = pControl->GetWindow(GetWindowType::Border);
            if (pBorder == this)
                pBorder = GetParent();
        }

        if (pBorder)
        {
            // set proper clipping region to not overdraw the whole control
            vcl::Region aClipRgn = GetPaintRegion();
            if (!aClipRgn.IsNull())
            {
                // transform clipping region to border window's coordinate system
                if (IsRTLEnabled() != pBorder->IsRTLEnabled() && AllSettings::GetLayoutRTL())
                {
                    // need to mirror in case border is not RTL but edit is (or vice versa)

                    // mirror
                    tools::Rectangle aBounds(aClipRgn.GetBoundRect());
                    int xNew = GetOutputSizePixel().Width() - aBounds.GetWidth() - aBounds.Left();
                    aClipRgn.Move(xNew - aBounds.Left(), 0);

                    // move offset of border window
                    Point aBorderOffs;
                    aBorderOffs = pBorder->ScreenToOutputPixel(OutputToScreenPixel(aBorderOffs));
                    aClipRgn.Move(aBorderOffs.X(), aBorderOffs.Y());
                }
                else
                {
                    // normal case
                    Point aBorderOffs;
                    aBorderOffs = pBorder->ScreenToOutputPixel(OutputToScreenPixel(aBorderOffs));
                    aClipRgn.Move(aBorderOffs.X(), aBorderOffs.Y());
                }

                vcl::Region oldRgn(pBorder->GetClipRegion());
                pBorder->SetClipRegion(aClipRgn);

                pBorder->Paint(*pBorder, tools::Rectangle());

                pBorder->SetClipRegion(oldRgn);
            }
            else
            {
                pBorder->Paint(*pBorder, tools::Rectangle());
            }
        }
    }
}

void Edit::ImplShowCursor( bool bOnlyIfVisible )
{
    if ( !IsUpdateMode() || ( bOnlyIfVisible && !IsReallyVisible() ) )
        return;

    vcl::Cursor* pCursor = GetCursor();
    OUString aText = ImplGetText();

    long nTextPos = 0;

    long   nDXBuffer[256];
    std::unique_ptr<long[]> pDXBuffer;
    long*  pDX = nDXBuffer;

    if( !aText.isEmpty() )
    {
        if( static_cast<size_t>(2*aText.getLength()) > SAL_N_ELEMENTS(nDXBuffer) )
        {
            pDXBuffer.reset(new long[2*(aText.getLength()+1)]);
            pDX = pDXBuffer.get();
        }

        GetCaretPositions( aText, pDX, 0, aText.getLength() );

        if( maSelection.Max() < aText.getLength() )
            nTextPos = pDX[ 2*maSelection.Max() ];
        else
            nTextPos = pDX[ 2*aText.getLength()-1 ];
    }

    long nCursorWidth = 0;
    if ( !mbInsertMode && !maSelection.Len() && (maSelection.Max() < aText.getLength()) )
        nCursorWidth = GetTextWidth(aText, maSelection.Max(), 1);
    long nCursorPosX = nTextPos + mnXOffset + ImplGetExtraXOffset();

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
            mnXOffset = (aOutSize.Width()-ImplGetExtraXOffset()) - nTextPos;
            // Something more?
            if ( (aOutSize.Width()-ImplGetExtraXOffset()) < nTextPos )
            {
                long nMaxNegX = (aOutSize.Width()-ImplGetExtraXOffset()) - GetTextWidth( aText );
                mnXOffset -= aOutSize.Width() / 5;
                if ( mnXOffset < nMaxNegX )  // both negative...
                    mnXOffset = nMaxNegX;
            }
        }

        nCursorPosX = nTextPos + mnXOffset + ImplGetExtraXOffset();
        if ( nCursorPosX == aOutSize.Width() )  // then invisible...
            nCursorPosX--;

        if ( mnXOffset != nOldXOffset )
            ImplInvalidateOrRepaint();
    }

    const long nTextHeight = GetTextHeight();
    const long nCursorPosY = ImplGetTextYPosition();
    if (pCursor)
    {
        pCursor->SetPos( Point( nCursorPosX, nCursorPosY ) );
        pCursor->SetSize( Size( nCursorWidth, nTextHeight ) );
        pCursor->Show();
    }
}

void Edit::ImplAlign()
{
    if (mnAlign == EDIT_ALIGN_LEFT && !mnXOffset)
    {
        // short circuit common case and avoid slow GetTextWidth() calc
        return;
    }

    long nTextWidth = GetTextWidth( ImplGetText() );
    long nOutWidth = GetOutputSizePixel().Width();

    if ( mnAlign == EDIT_ALIGN_LEFT )
    {
        if (nTextWidth < nOutWidth)
            mnXOffset = 0;
    }
    else if ( mnAlign == EDIT_ALIGN_RIGHT )
    {
        long nMinXOffset = nOutWidth - nTextWidth - 1 - ImplGetExtraXOffset();
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

void Edit::ImplAlignAndPaint()
{
    ImplAlign();
    ImplInvalidateOrRepaint();
    ImplShowCursor();
}

sal_Int32 Edit::ImplGetCharPos( const Point& rWindowPos ) const
{
    sal_Int32 nIndex = EDIT_NOLIMIT;
    OUString aText = ImplGetText();

    long   nDXBuffer[256];
    std::unique_ptr<long[]> pDXBuffer;
    long*  pDX = nDXBuffer;
    if( static_cast<size_t>(2*aText.getLength()) > SAL_N_ELEMENTS(nDXBuffer) )
    {
        pDXBuffer.reset(new long[2*(aText.getLength()+1)]);
        pDX = pDXBuffer.get();
    }

    GetCaretPositions( aText, pDX, 0, aText.getLength() );
    long nX = rWindowPos.X() - mnXOffset - ImplGetExtraXOffset();
    for (sal_Int32 i = 0; i < aText.getLength(); aText.iterateCodePoints(&i))
    {
        if( (pDX[2*i] >= nX && pDX[2*i+1] <= nX) ||
            (pDX[2*i+1] >= nX && pDX[2*i] <= nX))
        {
            nIndex = i;
            if( pDX[2*i] < pDX[2*i+1] )
            {
                if( nX > (pDX[2*i]+pDX[2*i+1])/2 )
                    aText.iterateCodePoints(&nIndex);
            }
            else
            {
                if( nX < (pDX[2*i]+pDX[2*i+1])/2 )
                    aText.iterateCodePoints(&nIndex);
            }
            break;
        }
    }
    if( nIndex == EDIT_NOLIMIT )
    {
        nIndex = 0;
        sal_Int32 nFinalIndex = 0;
        long nDiff = std::abs( pDX[0]-nX );
        sal_Int32 i = 0;
        if (!aText.isEmpty())
        {
            aText.iterateCodePoints(&i);    //skip the first character
        }
        while (i < aText.getLength())
        {
            long nNewDiff = std::abs( pDX[2*i]-nX );

            if( nNewDiff < nDiff )
            {
                nIndex = i;
                nDiff = nNewDiff;
            }

            nFinalIndex = i;

            aText.iterateCodePoints(&i);
        }
        if (nIndex == nFinalIndex && std::abs( pDX[2*nIndex+1] - nX ) < nDiff)
            nIndex = EDIT_NOLIMIT;
    }

    return nIndex;
}

void Edit::ImplSetCursorPos( sal_Int32 nChar, bool bSelect )
{
    Selection aSelection( maSelection );
    aSelection.Max() = nChar;
    if ( !bSelect )
        aSelection.Min() = aSelection.Max();
    ImplSetSelection( aSelection );
}

void Edit::ImplCopyToSelectionClipboard()
{
    if ( GetSelection().Len() )
    {
        css::uno::Reference<css::datatransfer::clipboard::XClipboard> aSelection(GetPrimarySelection());
        ImplCopy( aSelection );
    }
}

void Edit::ImplCopy( uno::Reference< datatransfer::clipboard::XClipboard > const & rxClipboard )
{
    vcl::unohelper::TextDataObject::CopyStringTo( GetSelected(), rxClipboard );
}

void Edit::ImplPaste( uno::Reference< datatransfer::clipboard::XClipboard > const & rxClipboard )
{
    if ( rxClipboard.is() )
    {
        uno::Reference< datatransfer::XTransferable > xDataObj;

        try
            {
                SolarMutexReleaser aReleaser;
                xDataObj = rxClipboard->getContents();
            }
        catch( const css::uno::Exception& )
            {
            }

        if ( xDataObj.is() )
        {
            datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aFlavor );
            try
            {
                uno::Any aData = xDataObj->getTransferData( aFlavor );
                OUString aText;
                aData >>= aText;
                if( ImplTruncateToMaxLen( aText, maSelection.Len() ) )
                    ShowTruncationWarning(GetFrameWeld());
                ReplaceSelected( aText );
            }
            catch( const css::uno::Exception& )
            {
            }
        }
    }
}

void Edit::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( mpSubEdit )
    {
        Control::MouseButtonDown( rMEvt );
        return;
    }

    sal_Int32 nCharPos = ImplGetCharPos( rMEvt.GetPosPixel() );
    Selection aSelection( maSelection );
    aSelection.Justify();

    if ( rMEvt.GetClicks() < 4 )
    {
        mbClickedInSelection = false;
        if ( rMEvt.GetClicks() == 3 )
        {
            ImplSetSelection( Selection( 0, EDIT_NOLIMIT) );
            ImplCopyToSelectionClipboard();

        }
        else if ( rMEvt.GetClicks() == 2 )
        {
            uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
            i18n::Boundary aBoundary = xBI->getWordBoundary( maText.toString(), aSelection.Max(),
                     GetSettings().GetLanguageTag().getLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, true );
            ImplSetSelection( Selection( aBoundary.startPos, aBoundary.endPos ) );
            ImplCopyToSelectionClipboard();
        }
        else if ( !rMEvt.IsShift() && HasFocus() && aSelection.IsInside( nCharPos ) )
            mbClickedInSelection = true;
        else if ( rMEvt.IsLeft() )
            ImplSetCursorPos( nCharPos, rMEvt.IsShift() );

        if ( !mbClickedInSelection && rMEvt.IsLeft() && ( rMEvt.GetClicks() == 1 ) )
            StartTracking( StartTrackingFlags::ScrollRepeat );
    }

    GrabFocus();
}

void Edit::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( mbClickedInSelection && rMEvt.IsLeft() )
    {
        sal_Int32 nCharPos = ImplGetCharPos( rMEvt.GetPosPixel() );
        ImplSetCursorPos( nCharPos, false );
        mbClickedInSelection = false;
    }
    else if ( rMEvt.IsMiddle() && !mbReadOnly &&
              ( GetSettings().GetMouseSettings().GetMiddleButtonAction() == MouseMiddleButtonAction::PasteSelection ) )
    {
        css::uno::Reference<css::datatransfer::clipboard::XClipboard> aSelection(Window::GetPrimarySelection());
        ImplPaste( aSelection );
        ImplModified();
    }
}

void Edit::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( mbClickedInSelection )
        {
            sal_Int32 nCharPos = ImplGetCharPos( rTEvt.GetMouseEvent().GetPosPixel() );
            ImplSetCursorPos( nCharPos, false );
            mbClickedInSelection = false;
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
            sal_Int32 nCharPos = ImplGetCharPos( rTEvt.GetMouseEvent().GetPosPixel() );
            ImplSetCursorPos( nCharPos, true );
        }
    }

    if ( mpUpdateDataTimer && !mbIsSubEdit && mpUpdateDataTimer->IsActive() )
        mpUpdateDataTimer->Start();//do not update while the user is still travelling in the control
}

bool Edit::ImplHandleKeyEvent( const KeyEvent& rKEvt )
{
    bool        bDone = false;
    sal_uInt16      nCode = rKEvt.GetKeyCode().GetCode();
    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();

    mbInternModified = false;

    if ( eFunc != KeyFuncType::DONTKNOW )
    {
        switch ( eFunc )
        {
            case KeyFuncType::CUT:
            {
                if ( !mbReadOnly && maSelection.Len() && !mbPassword )
                {
                    Cut();
                    ImplModified();
                    bDone = true;
                }
            }
            break;

            case KeyFuncType::COPY:
            {
                if ( !mbPassword )
                {
                    Copy();
                    bDone = true;
                }
            }
            break;

            case KeyFuncType::PASTE:
            {
                if ( !mbReadOnly )
                {
                    Paste();
                    bDone = true;
                }
            }
            break;

            case KeyFuncType::UNDO:
            {
                if ( !mbReadOnly )
                {
                    Undo();
                    bDone = true;
                }
            }
            break;

            default:
                eFunc = KeyFuncType::DONTKNOW;
        }
    }

    if ( !bDone && rKEvt.GetKeyCode().IsMod1() && !rKEvt.GetKeyCode().IsMod2() )
    {
        if ( nCode == KEY_A )
        {
            ImplSetSelection( Selection( 0, maText.getLength() ) );
            bDone = true;
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
                bDone = true;
            }
        }
    }

    if ( eFunc == KeyFuncType::DONTKNOW && ! bDone )
    {
        switch ( nCode )
        {
            case css::awt::Key::SELECT_ALL:
            {
                ImplSetSelection( Selection( 0, maText.getLength() ) );
                bDone = true;
            }
            break;

            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_HOME:
            case KEY_END:
            case css::awt::Key::MOVE_WORD_FORWARD:
            case css::awt::Key::SELECT_WORD_FORWARD:
            case css::awt::Key::MOVE_WORD_BACKWARD:
            case css::awt::Key::SELECT_WORD_BACKWARD:
            case css::awt::Key::MOVE_TO_BEGIN_OF_LINE:
            case css::awt::Key::MOVE_TO_END_OF_LINE:
            case css::awt::Key::SELECT_TO_BEGIN_OF_LINE:
            case css::awt::Key::SELECT_TO_END_OF_LINE:
            case css::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
            case css::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
            case css::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
            case css::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
            case css::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
            case css::awt::Key::MOVE_TO_END_OF_DOCUMENT:
            case css::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
            case css::awt::Key::SELECT_TO_END_OF_DOCUMENT:
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
                    case css::awt::Key::MOVE_WORD_FORWARD:
                        bGoRight = bWord = true;break;
                    case css::awt::Key::SELECT_WORD_FORWARD:
                        bGoRight = bSelect = bWord = true;break;
                    case css::awt::Key::MOVE_WORD_BACKWARD:
                        bGoLeft = bWord = true;break;
                    case css::awt::Key::SELECT_WORD_BACKWARD:
                        bGoLeft = bSelect = bWord = true;break;
                    case css::awt::Key::SELECT_TO_BEGIN_OF_LINE:
                    case css::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH:
                    case css::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT:
                        bSelect = true;
                        [[fallthrough]];
                    case css::awt::Key::MOVE_TO_BEGIN_OF_LINE:
                    case css::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
                    case css::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
                        bGoHome = true;break;
                    case css::awt::Key::SELECT_TO_END_OF_LINE:
                    case css::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
                    case css::awt::Key::SELECT_TO_END_OF_DOCUMENT:
                        bSelect = true;
                        [[fallthrough]];
                    case css::awt::Key::MOVE_TO_END_OF_LINE:
                    case css::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
                    case css::awt::Key::MOVE_TO_END_OF_DOCUMENT:
                        bGoEnd = true;break;
                    default:
                        break;
                    };

                    // range is checked in ImplSetSelection ...
                    if ( bGoLeft && aSel.Max() )
                    {
                        if ( bWord )
                        {
                            i18n::Boundary aBoundary = xBI->getWordBoundary( maText.toString(), aSel.Max(),
                                    GetSettings().GetLanguageTag().getLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, true );
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
                        aSel.Max() = EDIT_NOLIMIT;
                    }

                    if ( !bSelect )
                        aSel.Min() = aSel.Max();

                    if ( aSel != GetSelection() )
                    {
                        ImplSetSelection( aSel );
                        ImplCopyToSelectionClipboard();
                    }

                    if (bGoEnd && maAutocompleteHdl.IsSet() && !rKEvt.GetKeyCode().GetModifier())
                    {
                        if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.getLength()) )
                        {
                            maAutocompleteHdl.Call(*this);
                        }
                    }

                    bDone = true;
                }
            }
            break;

            case css::awt::Key::DELETE_WORD_BACKWARD:
            case css::awt::Key::DELETE_WORD_FORWARD:
            case css::awt::Key::DELETE_TO_BEGIN_OF_LINE:
            case css::awt::Key::DELETE_TO_END_OF_LINE:
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
                    case css::awt::Key::DELETE_WORD_BACKWARD:
                        nDel = EDIT_DEL_LEFT;
                        nMode = EDIT_DELMODE_RESTOFWORD;
                        break;
                    case css::awt::Key::DELETE_WORD_FORWARD:
                        nDel = EDIT_DEL_RIGHT;
                        nMode = EDIT_DELMODE_RESTOFWORD;
                        break;
                    case css::awt::Key::DELETE_TO_BEGIN_OF_LINE:
                        nDel = EDIT_DEL_LEFT;
                        nMode = EDIT_DELMODE_RESTOFCONTENT;
                        break;
                    case css::awt::Key::DELETE_TO_END_OF_LINE:
                        nDel = EDIT_DEL_RIGHT;
                        nMode = EDIT_DELMODE_RESTOFCONTENT;
                        break;
                    default: break;
                    }
                    sal_Int32 nOldLen = maText.getLength();
                    ImplDelete( maSelection, nDel, nMode );
                    if ( maText.getLength() != nOldLen )
                        ImplModified();
                    bDone = true;
                }
            }
            break;

            case KEY_INSERT:
            {
                if ( !mpIMEInfos && !mbReadOnly && !rKEvt.GetKeyCode().IsMod2() )
                {
                    SetInsertMode( !mbInsertMode );
                    bDone = true;
                }
            }
            break;

            case KEY_RETURN:
                if (maActivateHdl.IsSet())
                {
                    bDone = maActivateHdl.Call(*this);
                }
            break;

            default:
            {
                if ( IsCharInput( rKEvt ) )
                {
                    bDone = true;   // read characters also when in ReadOnly
                    if ( !mbReadOnly )
                    {
                        ImplInsertText(OUString(rKEvt.GetCharCode()), nullptr, true);
                        if (maAutocompleteHdl.IsSet())
                        {
                            if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.getLength()) )
                            {
                                maAutocompleteHdl.Call(*this);
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

void Edit::KeyInput( const KeyEvent& rKEvt )
{
    if ( mpUpdateDataTimer && !mbIsSubEdit && mpUpdateDataTimer->IsActive() )
        mpUpdateDataTimer->Start();//do not update while the user is still travelling in the control

    if ( mpSubEdit || !ImplHandleKeyEvent( rKEvt ) )
        Control::KeyInput( rKEvt );
}

void Edit::FillLayoutData() const
{
    mpControlData->mpLayoutData.reset( new vcl::ControlLayoutData );
    const_cast<Edit*>(this)->Invalidate();
}

void Edit::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRectangle)
{
    if (!mpSubEdit)
        ImplRepaint(rRenderContext, rRectangle);
}

void Edit::Resize()
{
    if ( !mpSubEdit && IsReallyVisible() )
    {
        Control::Resize();
        // because of vertical centering...
        mnXOffset = 0;
        ImplAlign();
        Invalidate();
        ImplShowCursor();
    }
}

void Edit::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags )
{
    ApplySettings(*pDev);

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );
    vcl::Font aFont = GetDrawPixelFont( pDev );
    OutDevType eOutDevType = pDev->GetOutDevType();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    pDev->SetTextFillColor();

    // Border/Background
    pDev->SetLineColor();
    pDev->SetFillColor();
    bool bBorder = (GetStyle() & WB_BORDER);
    bool bBackground = IsControlBackground();
    if ( bBorder || bBackground )
    {
        tools::Rectangle aRect( aPos, aSize );
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

    // Content
    if ( ( nFlags & DrawFlags::Mono ) || ( eOutDevType == OUTDEV_PRINTER ) )
        pDev->SetTextColor( COL_BLACK );
    else
    {
        if ( !IsEnabled() )
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            pDev->SetTextColor( rStyleSettings.GetDisableColor() );
        }
        else
        {
            pDev->SetTextColor( GetTextColor() );
        }
    }

    const long nOnePixel = GetDrawPixel( pDev, 1 );
    const long nOffX = 3*nOnePixel;
    DrawTextFlags nTextStyle = DrawTextFlags::VCenter;
    tools::Rectangle aTextRect( aPos, aSize );

    if ( GetStyle() & WB_CENTER )
        nTextStyle |= DrawTextFlags::Center;
    else if ( GetStyle() & WB_RIGHT )
        nTextStyle |= DrawTextFlags::Right;
    else
        nTextStyle |= DrawTextFlags::Left;

    aTextRect.AdjustLeft(nOffX );
    aTextRect.AdjustRight( -nOffX );

    OUString    aText = ImplGetText();
    long        nTextHeight = pDev->GetTextHeight();
    long        nTextWidth = pDev->GetTextWidth( aText );
    long        nOffY = (aSize.Height() - nTextHeight) / 2;

    // Clipping?
    if ( (nOffY < 0) ||
         ((nOffY+nTextHeight) > aSize.Height()) ||
         ((nOffX+nTextWidth) > aSize.Width()) )
    {
        tools::Rectangle aClip( aPos, aSize );
        if ( nTextHeight > aSize.Height() )
            aClip.AdjustBottom(nTextHeight-aSize.Height()+1 );  // prevent HP printers from 'optimizing'
        pDev->IntersectClipRegion( aClip );
    }

    pDev->DrawText( aTextRect, aText, nTextStyle );
    pDev->Pop();

    if ( GetSubEdit() )
    {
        GetSubEdit()->Draw( pDev, rPos, rSize, nFlags );
    }
}

void Edit::ImplInvalidateOutermostBorder( vcl::Window* pWin )
{
    // allow control to show focused state
    vcl::Window *pInvalWin = pWin, *pBorder = pWin;
    while( ( pBorder = pInvalWin->GetWindow( GetWindowType::Border ) ) != pInvalWin && pBorder &&
           pInvalWin->ImplGetFrame() == pBorder->ImplGetFrame() )
    {
        pInvalWin = pBorder;
    }

    pInvalWin->Invalidate( InvalidateFlags::Children | InvalidateFlags::Update );
}

void Edit::GetFocus()
{
    if ( mpSubEdit )
        mpSubEdit->ImplGrabFocus( GetGetFocusFlags() );
    else if ( !mbActivePopup )
    {
        maUndoText = maText.toString();
        if(mbSelectAllSingleClick)
        {
            maSelection.Min() = 0;
            maSelection.Max() = maText.getLength();
        }
        SelectionOptions nSelOptions = GetSettings().GetStyleSettings().GetSelectionOptions();
        if ( !( GetStyle() & (WB_NOHIDESELECTION|WB_READONLY) )
                && ( GetGetFocusFlags() & (GetFocusFlags::Init|GetFocusFlags::Tab|GetFocusFlags::CURSOR|GetFocusFlags::Mnemonic) ) )
        {
            if ( nSelOptions & SelectionOptions::ShowFirst )
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
                static_cast<Edit*>(GetParent())->CallEventListeners( VclEventId::EditSelectionChanged );
            else
                CallEventListeners( VclEventId::EditSelectionChanged );
        }

        ImplShowCursor();

        // FIXME: this is currently only on macOS
        // check for other platforms that need similar handling
        if( ImplGetSVData()->maNWFData.mbNoFocusRects &&
            IsNativeWidgetEnabled() &&
            IsNativeControlSupported( ControlType::Editbox, ControlPart::Entire ) )
        {
            ImplInvalidateOutermostBorder( mbIsSubEdit ? GetParent() : this );
        }
        else if ( maSelection.Len() )
        {
            // paint the selection
            if ( !HasPaintEvent() )
                ImplInvalidateOrRepaint();
            else
                Invalidate();
        }

        SetInputContext( InputContext( GetFont(), !IsReadOnly() ? InputContextFlags::Text|InputContextFlags::ExtText : InputContextFlags::NONE ) );
    }

    Control::GetFocus();
}

void Edit::LoseFocus()
{
    if ( mpUpdateDataTimer && !mbIsSubEdit && mpUpdateDataTimer->IsActive() )
    {
        //notify an update latest when the focus is lost
        mpUpdateDataTimer->Stop();
        mpUpdateDataTimer->Invoke();
    }

    if ( !mpSubEdit )
    {
        // FIXME: this is currently only on macOS
        // check for other platforms that need similar handling
        if( ImplGetSVData()->maNWFData.mbNoFocusRects &&
            IsNativeWidgetEnabled() &&
            IsNativeControlSupported( ControlType::Editbox, ControlPart::Entire ) )
        {
            ImplInvalidateOutermostBorder( mbIsSubEdit ? GetParent() : this );
        }

        if ( !mbActivePopup && !( GetStyle() & WB_NOHIDESELECTION ) && maSelection.Len() )
            ImplInvalidateOrRepaint();    // paint the selection
    }

    Control::LoseFocus();
}

void Edit::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        VclPtr<PopupMenu> pPopup = Edit::CreatePopupMenu();

        bool bEnableCut = true;
        bool bEnableCopy = true;
        bool bEnableDelete = true;
        bool bEnablePaste = true;
        bool bEnableSpecialChar = true;

        if ( !maSelection.Len() )
        {
            bEnableCut = false;
            bEnableCopy = false;
            bEnableDelete = false;
        }

        if ( IsReadOnly() )
        {
            bEnableCut = false;
            bEnablePaste = false;
            bEnableDelete = false;
            bEnableSpecialChar = false;
        }
        else
        {
            // only paste if text available in clipboard
            bool bData = false;
            uno::Reference< datatransfer::clipboard::XClipboard > xClipboard = GetClipboard();

            if ( xClipboard.is() )
            {
                uno::Reference< datatransfer::XTransferable > xDataObj;
                {
                    SolarMutexReleaser aReleaser;
                    xDataObj = xClipboard->getContents();
                }
                if ( xDataObj.is() )
                {
                    datatransfer::DataFlavor aFlavor;
                    SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aFlavor );
                    bData = xDataObj->isDataFlavorSupported( aFlavor );
                }
            }
            bEnablePaste = bData;
        }

        pPopup->EnableItem(pPopup->GetItemId("cut"), bEnableCut);
        pPopup->EnableItem(pPopup->GetItemId("copy"), bEnableCopy);
        pPopup->EnableItem(pPopup->GetItemId("delete"), bEnableDelete);
        pPopup->EnableItem(pPopup->GetItemId("paste"), bEnablePaste);
        pPopup->EnableItem(pPopup->GetItemId("specialchar"), bEnableSpecialChar);
        pPopup->EnableItem(pPopup->GetItemId("undo"), maUndoText != maText.getStr());
        bool bAllSelected = maSelection.Min() == 0 && maSelection.Max() == maText.getLength();
        pPopup->EnableItem(pPopup->GetItemId("selectall"), !bAllSelected);
        pPopup->ShowItem(pPopup->GetItemId("specialchar"), pImplFncGetSpecialChars != nullptr);

        mbActivePopup = true;
        Selection aSaveSel = GetSelection(); // if someone changes selection in Get/LoseFocus, e.g. URL bar
        Point aPos = rCEvt.GetMousePosPixel();
        if ( !rCEvt.IsMouseEvent() )
        {
            // Show menu eventually centered in selection
            Size aSize = GetOutputSizePixel();
            aPos = Point( aSize.Width()/2, aSize.Height()/2 );
        }
        sal_uInt16 n = pPopup->Execute( this, aPos );
        SetSelection( aSaveSel );
        OString sCommand = pPopup->GetItemIdent(n);
        if (sCommand == "undo")
        {
            Undo();
            ImplModified();
        }
        else if (sCommand == "cut")
        {
            Cut();
            ImplModified();
        }
        else if (sCommand == "copy")
        {
            Copy();
        }
        else if (sCommand == "paste")
        {
            Paste();
            ImplModified();
        }
        else if (sCommand == "delete")
        {
            DeleteSelected();
            ImplModified();
        }
        else if (sCommand == "selectall")
        {
            ImplSetSelection( Selection( 0, maText.getLength() ) );
        }
        else if (sCommand == "specialchar" && pImplFncGetSpecialChars)
        {
            OUString aChars = pImplFncGetSpecialChars( this, GetFont() );
            SetSelection( aSaveSel );
            if (!aChars.isEmpty())
            {
                ImplInsertText( aChars );
                ImplModified();
            }
        }
        pPopup.clear();
        mbActivePopup = false;
    }
    else if ( rCEvt.GetCommand() == CommandEventId::StartExtTextInput )
    {
        DeleteSelected();
        sal_Int32 nPos = static_cast<sal_Int32>(maSelection.Max());
        mpIMEInfos.reset(new Impl_IMEInfos( nPos, OUString(maText.getStr() + nPos ) ));
        mpIMEInfos->bWasCursorOverwrite = !IsInsertMode();
    }
    else if ( rCEvt.GetCommand() == CommandEventId::EndExtTextInput )
    {
        bool bInsertMode = !mpIMEInfos->bWasCursorOverwrite;
        mpIMEInfos.reset();

        SetInsertMode(bInsertMode);
        ImplModified();

        Invalidate();

        // #i25161# call auto complete handler for ext text commit also
        if (maAutocompleteHdl.IsSet())
        {
            if ( (maSelection.Min() == maSelection.Max()) && (maSelection.Min() == maText.getLength()) )
            {
                maAutocompleteHdl.Call(*this);
            }
        }
    }
    else if ( rCEvt.GetCommand() == CommandEventId::ExtTextInput )
    {
        const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();

        maText.remove( mpIMEInfos->nPos, mpIMEInfos->nLen );
        maText.insert( mpIMEInfos->nPos, pData->GetText() );
        if ( mpIMEInfos->bWasCursorOverwrite )
        {
            const sal_Int32 nOldIMETextLen = mpIMEInfos->nLen;
            const sal_Int32 nNewIMETextLen = pData->GetText().getLength();
            if ( ( nOldIMETextLen > nNewIMETextLen ) &&
                 ( nNewIMETextLen < mpIMEInfos->aOldTextAfterStartPos.getLength() ) )
            {
                // restore old characters
                const sal_Int32 nRestore = nOldIMETextLen - nNewIMETextLen;
                maText.insert( mpIMEInfos->nPos + nNewIMETextLen, mpIMEInfos->aOldTextAfterStartPos.copy( nNewIMETextLen, nRestore ) );
            }
            else if ( ( nOldIMETextLen < nNewIMETextLen ) &&
                      ( nOldIMETextLen < mpIMEInfos->aOldTextAfterStartPos.getLength() ) )
            {
                const sal_Int32 nOverwrite = ( nNewIMETextLen > mpIMEInfos->aOldTextAfterStartPos.getLength()
                    ? mpIMEInfos->aOldTextAfterStartPos.getLength() : nNewIMETextLen ) - nOldIMETextLen;
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
        sal_Int32 nCursorPos = mpIMEInfos->nPos + pData->GetCursorPos();
        SetSelection( Selection( nCursorPos, nCursorPos ) );
        SetInsertMode( !pData->IsCursorOverwrite() );

        if ( pData->IsCursorVisible() )
            GetCursor()->Show();
        else
            GetCursor()->Hide();
    }
    else if ( rCEvt.GetCommand() == CommandEventId::CursorPos )
    {
        if ( mpIMEInfos )
        {
            sal_Int32 nCursorPos = GetSelection().Max();
            SetCursorRect( nullptr, GetTextWidth( maText.toString(), nCursorPos, mpIMEInfos->nPos+mpIMEInfos->nLen-nCursorPos ) );
        }
        else
        {
            SetCursorRect();
        }
    }
    else if ( rCEvt.GetCommand() == CommandEventId::SelectionChange )
    {
        const CommandSelectionChangeData *pData = rCEvt.GetSelectionChangeData();
        Selection aSelection( pData->GetStart(), pData->GetEnd() );
        SetSelection(aSelection);
    }
    else if ( rCEvt.GetCommand() == CommandEventId::QueryCharPosition )
    {
        if (mpIMEInfos && mpIMEInfos->nLen > 0)
        {
            OUString aText = ImplGetText();
            long   nDXBuffer[256];
            std::unique_ptr<long[]> pDXBuffer;
            long*  pDX = nDXBuffer;

            if( !aText.isEmpty() )
            {
                if( static_cast<size_t>(2*aText.getLength()) > SAL_N_ELEMENTS(nDXBuffer) )
                {
                    pDXBuffer.reset(new long[2*(aText.getLength()+1)]);
                    pDX = pDXBuffer.get();
                }

                GetCaretPositions( aText, pDX, 0, aText.getLength() );
            }
            long    nTH = GetTextHeight();
            Point   aPos( mnXOffset, ImplGetTextYPosition() );

            std::unique_ptr<tools::Rectangle[]> aRects(new tools::Rectangle[ mpIMEInfos->nLen ]);
            for ( int nIndex = 0; nIndex < mpIMEInfos->nLen; ++nIndex )
            {
                tools::Rectangle aRect( aPos, Size( 10, nTH ) );
                aRect.SetLeft( pDX[2*(nIndex+mpIMEInfos->nPos)] + mnXOffset + ImplGetExtraXOffset() );
                aRects[ nIndex ] = aRect;
            }
            SetCompositionCharRect( aRects.get(), mpIMEInfos->nLen );
        }
    }
    else
        Control::Command( rCEvt );
}

void Edit::StateChanged( StateChangedType nType )
{
    if (nType == StateChangedType::InitShow)
    {
        if (!mpSubEdit)
        {
            mnXOffset = 0;  // if GrabFocus before while size was still wrong
            ImplAlign();
            if (!mpSubEdit)
                ImplShowCursor(false);
            Invalidate();
        }
    }
    else if (nType == StateChangedType::Enable)
    {
        if (!mpSubEdit)
        {
            // change text color only
            ImplInvalidateOrRepaint();
        }
    }
    else if (nType == StateChangedType::Style || nType == StateChangedType::Mirroring)
    {
        WinBits nStyle = GetStyle();
        if (nType == StateChangedType::Style)
        {
            nStyle = ImplInitStyle(GetStyle());
            SetStyle(nStyle);
        }

        sal_uInt16 nOldAlign = mnAlign;
        mnAlign = EDIT_ALIGN_LEFT;

        // hack: right align until keyinput and cursor travelling works
        // edits are always RTL disabled
        // however the parent edits contain the correct setting
        if (mbIsSubEdit && GetParent()->IsRTLEnabled())
        {
            if (GetParent()->GetStyle() & WB_LEFT)
                mnAlign = EDIT_ALIGN_RIGHT;
            if (nType == StateChangedType::Mirroring)
                SetLayoutMode(ComplexTextLayoutFlags::BiDiRtl | ComplexTextLayoutFlags::TextOriginLeft);
        }
        else if (mbIsSubEdit && !GetParent()->IsRTLEnabled())
        {
            if (nType == StateChangedType::Mirroring)
                SetLayoutMode(ComplexTextLayoutFlags::TextOriginLeft);
        }

        if (nStyle & WB_RIGHT)
            mnAlign = EDIT_ALIGN_RIGHT;
        else if (nStyle & WB_CENTER)
            mnAlign = EDIT_ALIGN_CENTER;
        if (!maText.isEmpty() && (mnAlign != nOldAlign))
        {
            ImplAlign();
            Invalidate();
        }

    }
    else if (nType == StateChangedType::Zoom)
    {
        if (!mpSubEdit)
        {
            ApplySettings(*this);
            ImplShowCursor();
            Invalidate();
        }
    }
    else if (nType == StateChangedType::ControlFont)
    {
        if (!mpSubEdit)
        {
            ApplySettings(*this);
            ImplShowCursor();
            Invalidate();
        }
    }
    else if (nType == StateChangedType::ControlForeground)
    {
        if (!mpSubEdit)
        {
            ApplySettings(*this);
            Invalidate();
        }
    }
    else if (nType == StateChangedType::ControlBackground)
    {
        if (!mpSubEdit)
        {
            ApplySettings(*this);
            Invalidate();
        }
    }

    Control::StateChanged(nType);
}

void Edit::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        if ( !mpSubEdit )
        {
            ApplySettings(*this);
            ImplShowCursor();
            Invalidate();
        }
    }

    Control::DataChanged( rDCEvt );
}

void Edit::ImplShowDDCursor()
{
    if (!mpDDInfo->bVisCursor)
    {
        long nTextWidth = GetTextWidth( maText.toString(), 0, mpDDInfo->nDropPos );
        long nTextHeight = GetTextHeight();
        tools::Rectangle aCursorRect( Point( nTextWidth + mnXOffset, (GetOutputSize().Height()-nTextHeight)/2 ), Size( 2, nTextHeight ) );
        mpDDInfo->aCursor.SetWindow( this );
        mpDDInfo->aCursor.SetPos( aCursorRect.TopLeft() );
        mpDDInfo->aCursor.SetSize( aCursorRect.GetSize() );
        mpDDInfo->aCursor.Show();
        mpDDInfo->bVisCursor = true;
    }
}

void Edit::ImplHideDDCursor()
{
    if ( mpDDInfo && mpDDInfo->bVisCursor )
    {
        mpDDInfo->aCursor.Hide();
        mpDDInfo->bVisCursor = false;
    }
}

TextFilter::TextFilter(const OUString &rForbiddenChars)
    : sForbiddenChars(rForbiddenChars)
{
}

TextFilter::~TextFilter()
{
}

OUString TextFilter::filter(const OUString &rText)
{
    OUString sTemp(rText);
    for (sal_Int32 i = 0; i < sForbiddenChars.getLength(); ++i)
    {
        sTemp = sTemp.replaceAll(OUStringLiteral1(sForbiddenChars[i]), "");
    }
    return sTemp;
}

void Edit::filterText()
{
    Selection aSel = GetSelection();
    const OUString sOrig = GetText();
    const OUString sNew = mpFilterText->filter(GetText());
    if (sOrig != sNew)
    {
        sal_Int32 nDiff = sOrig.getLength() - sNew.getLength();
        if (nDiff)
        {
            aSel.setMin(aSel.getMin() - nDiff);
            aSel.setMax(aSel.getMin());
        }
        SetText(sNew);
        SetSelection(aSel);
    }
}

void Edit::Modify()
{
    if (mpFilterText)
        filterText();

    if ( mbIsSubEdit )
    {
        static_cast<Edit*>(GetParent())->Modify();
    }
    else
    {
        if ( mpUpdateDataTimer )
            mpUpdateDataTimer->Start();

        if ( ImplCallEventListenersAndHandler( VclEventId::EditModify, [this] () { maModifyHdl.Call(*this); } ) )
            // have been destroyed while calling into the handlers
            return;

        // #i13677# notify edit listeners about caret position change
        CallEventListeners( VclEventId::EditCaretChanged );
        // FIXME: this is currently only on macOS
        // check for other platforms that need similar handling
        if( ImplGetSVData()->maNWFData.mbNoFocusRects &&
            IsNativeWidgetEnabled() &&
            IsNativeControlSupported( ControlType::Editbox, ControlPart::Entire ) )
        {
            ImplInvalidateOutermostBorder( this );
        }
    }
}

void Edit::UpdateData()
{
    maUpdateDataHdl.Call( *this );
}

IMPL_LINK_NOARG(Edit, ImplUpdateDataHdl, Timer *, void)
{
    UpdateData();
}

void Edit::EnableUpdateData( sal_uLong nTimeout )
{
    if ( !nTimeout )
        DisableUpdateData();
    else
    {
        if ( !mpUpdateDataTimer )
        {
            mpUpdateDataTimer.reset(new Timer("UpdateDataTimer"));
            mpUpdateDataTimer->SetInvokeHandler( LINK( this, Edit, ImplUpdateDataHdl ) );
            mpUpdateDataTimer->SetDebugName( "vcl::Edit mpUpdateDataTimer" );
        }

        mpUpdateDataTimer->SetTimeout( nTimeout );
    }
}

void Edit::DisableUpdateData()
{
    mpUpdateDataTimer.reset();
}

void Edit::SetEchoChar( sal_Unicode c )
{
    mcEchoChar = c;
    if ( mpSubEdit )
        mpSubEdit->SetEchoChar( c );
}

void Edit::SetReadOnly( bool bReadOnly )
{
    if ( mbReadOnly != bReadOnly )
    {
        mbReadOnly = bReadOnly;
        if ( mpSubEdit )
            mpSubEdit->SetReadOnly( bReadOnly );

        CompatStateChanged( StateChangedType::ReadOnly );
    }
}

void Edit::SetSelectAllSingleClick( bool bSelectAllSingleClick )
{
    if ( mbSelectAllSingleClick != bSelectAllSingleClick )
    {
        mbSelectAllSingleClick = bSelectAllSingleClick;
        if ( mpSubEdit )
            mpSubEdit->SetSelectAllSingleClick( bSelectAllSingleClick );
    }
}
void Edit::SetInsertMode( bool bInsert )
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

bool Edit::IsInsertMode() const
{
    if ( mpSubEdit )
        return mpSubEdit->IsInsertMode();
    else
        return mbInsertMode;
}

void Edit::SetMaxTextLen(sal_Int32 nMaxLen)
{
    mnMaxTextLen = nMaxLen > 0 ? nMaxLen : EDIT_NOLIMIT;

    if ( mpSubEdit )
        mpSubEdit->SetMaxTextLen( mnMaxTextLen );
    else
    {
        if ( maText.getLength() > mnMaxTextLen )
            ImplDelete( Selection( mnMaxTextLen, maText.getLength() ), EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
    }
}

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

void Edit::ImplSetSelection( const Selection& rSelection, bool bPaint )
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
                Selection aTemp = maSelection;
                maSelection = aNew;

                if ( bPaint && ( aOld.Len() || aNew.Len() || IsPaintTransparent() ) )
                    ImplInvalidateOrRepaint();
                ImplShowCursor();

                bool bCaret = false, bSelection = false;
                long nB=aNew.Max(), nA=aNew.Min(),oB=aTemp.Max(), oA=aTemp.Min();
                long nGap = nB-nA, oGap = oB-oA;
                if (nB != oB)
                    bCaret = true;
                if (nGap != 0 || oGap != 0)
                    bSelection = true;

                if (bSelection)
                {
                    if ( mbIsSubEdit )
                        static_cast<Edit*>(GetParent())->CallEventListeners( VclEventId::EditSelectionChanged );
                    else
                        CallEventListeners( VclEventId::EditSelectionChanged );
                }

                if (bCaret)
                {
                    if ( mbIsSubEdit )
                        static_cast<Edit*>(GetParent())->CallEventListeners( VclEventId::EditCaretChanged );
                    else
                        CallEventListeners( VclEventId::EditCaretChanged );
                }

                // #103511# notify combobox listeners of deselection
                if( !maSelection && GetParent() && GetParent()->GetType() == WindowType::COMBOBOX )
                    static_cast<Edit*>(GetParent())->CallEventListeners( VclEventId::ComboboxDeselect );
            }
        }
    }
}

const Selection& Edit::GetSelection() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetSelection();
    else
        return maSelection;
}

void Edit::ReplaceSelected( const OUString& rStr )
{
    if ( mpSubEdit )
        mpSubEdit->ReplaceSelected( rStr );
    else
        ImplInsertText( rStr );
}

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

void Edit::Cut()
{
    if ( !mbPassword )
    {
        Copy();
        ReplaceSelected( OUString() );
    }
}

void Edit::Copy()
{
    if ( !mbPassword )
    {
        css::uno::Reference<css::datatransfer::clipboard::XClipboard> aClipboard(GetClipboard());
        ImplCopy( aClipboard );
    }
}

void Edit::Paste()
{
    css::uno::Reference<css::datatransfer::clipboard::XClipboard> aClipboard(GetClipboard());
    ImplPaste( aClipboard );
}

void Edit::Undo()
{
    if ( mpSubEdit )
        mpSubEdit->Undo();
    else
    {
        const OUString aText( maText.toString() );
        ImplDelete( Selection( 0, aText.getLength() ), EDIT_DEL_RIGHT, EDIT_DELMODE_SIMPLE );
        ImplInsertText( maUndoText );
        ImplSetSelection( Selection( 0, maUndoText.getLength() ) );
        maUndoText = aText;
    }
}

void Edit::SetText( const OUString& rStr )
{
    if ( mpSubEdit )
        mpSubEdit->SetText( rStr ); // not directly ImplSetText if SetText overridden
    else
    {
        Selection aNewSel( 0, 0 );  // prevent scrolling
        ImplSetText( rStr, &aNewSel );
    }
}

void Edit::SetText( const OUString& rStr, const Selection& rSelection )
{
    if ( mpSubEdit )
        mpSubEdit->SetText( rStr, rSelection );
    else
        ImplSetText( rStr, &rSelection );
}

OUString Edit::GetText() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetText();
    else
        return maText.toString();
}

void Edit::SetCursorAtLast(){
    ImplSetCursorPos( GetText().getLength(), false );
}

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

OUString Edit::GetPlaceholderText() const
{
    if ( mpSubEdit )
        return mpSubEdit->GetPlaceholderText();

    return maPlaceholderText;
}

void Edit::SetModifyFlag()
{
    if ( mpSubEdit )
        mpSubEdit->mbModified = true;
    else
        mbModified = true;
}

void Edit::ClearModifyFlag()
{
    if ( mpSubEdit )
        mpSubEdit->mbModified = false;
    else
        mbModified = false;
}

void Edit::SetSubEdit(Edit* pEdit)
{
    mpSubEdit.disposeAndClear();
    mpSubEdit.set(pEdit);

    if (mpSubEdit)
    {
        SetPointer(PointerStyle::Arrow);    // Only SubEdit has the BEAM...
        mpSubEdit->mbIsSubEdit = true;

        mpSubEdit->SetReadOnly(mbReadOnly);
        mpSubEdit->maAutocompleteHdl = maAutocompleteHdl;
    }
}

Size Edit::CalcMinimumSizeForText(const OUString &rString) const
{
    ControlType eCtrlType = ImplGetNativeControlType();

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

        aSize.setHeight( GetTextHeight() );
        aSize.setWidth( GetTextWidth(aString) );
        aSize.AdjustWidth(ImplGetExtraXOffset() * 2 );

        // do not create edit fields in which one cannot enter anything
        // a default minimum width should exist for at least 3 characters

        //CalcSize calls CalcWindowSize, but we will call that also in this
        //function, so undo the first one with CalcOutputSize
        Size aMinSize(CalcOutputSize(CalcSize(3)));
        if (aSize.Width() < aMinSize.Width())
            aSize.setWidth( aMinSize.Width() );
    }

    aSize.AdjustHeight(ImplGetExtraYOffset() * 2 );

    aSize = CalcWindowSize( aSize );

    // ask NWF what if it has an opinion, too
    ImplControlValue aControlValue;
    tools::Rectangle aRect( Point( 0, 0 ), aSize );
    tools::Rectangle aContent, aBound;
    if (GetNativeControlRegion(eCtrlType, ControlPart::Entire, aRect, ControlState::NONE,
                               aControlValue, aBound, aContent))
    {
        if (aBound.GetHeight() > aSize.Height())
            aSize.setHeight( aBound.GetHeight() );
    }
    return aSize;
}

Size Edit::CalcMinimumSize() const
{
    return CalcMinimumSizeForText(GetText());
}

Size Edit::GetMinimumEditSize()
{
    vcl::Window* pDefWin = ImplGetDefaultWindow();
    ScopedVclPtrInstance< Edit > aEdit(  pDefWin, WB_BORDER  );
    Size aSize( aEdit->CalcMinimumSize() );
    return aSize;
}

Size Edit::GetOptimalSize() const
{
    return CalcMinimumSize();
}

Size Edit::CalcSize(sal_Int32 nChars) const
{
    // width for N characters, independent from content.
    // works only correct for fixed fonts, average otherwise
    Size aSz( GetTextWidth( "x" ), GetTextHeight() );
    aSz.setWidth( aSz.Width() * nChars );
    aSz.AdjustWidth(ImplGetExtraXOffset() * 2 );
    aSz = CalcWindowSize( aSz );
    return aSz;
}

sal_Int32 Edit::GetMaxVisChars() const
{
    const vcl::Window* pW = mpSubEdit ? mpSubEdit : this;
    sal_Int32 nOutWidth = pW->GetOutputSizePixel().Width();
    sal_Int32 nCharWidth = GetTextWidth( "x" );
    return nCharWidth ? nOutWidth/nCharWidth : 0;
}

sal_Int32 Edit::GetCharPos( const Point& rWindowPos ) const
{
    return ImplGetCharPos( rWindowPos );
}

void Edit::SetGetSpecialCharsFunction( FncGetSpecialChars fn )
{
    pImplFncGetSpecialChars = fn;
}

FncGetSpecialChars Edit::GetGetSpecialCharsFunction()
{
    return pImplFncGetSpecialChars;
}

VclPtr<PopupMenu> Edit::CreatePopupMenu()
{
    if (!mpUIBuilder)
        mpUIBuilder.reset(new VclBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "vcl/ui/editmenu.ui", ""));
    VclPtr<PopupMenu> pPopup = mpUIBuilder->get_menu("menu");
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if (rStyleSettings.GetHideDisabledMenuItems())
        pPopup->SetMenuFlags( MenuFlags::HideDisabledEntries );
    else
        pPopup->SetMenuFlags ( MenuFlags::AlwaysShowDisabledEntries );
    if (rStyleSettings.GetContextMenuShortcuts())
    {
        pPopup->SetAccelKey(pPopup->GetItemId("undo"), vcl::KeyCode( KeyFuncType::UNDO));
        pPopup->SetAccelKey(pPopup->GetItemId("cut"), vcl::KeyCode( KeyFuncType::CUT));
        pPopup->SetAccelKey(pPopup->GetItemId("copy"), vcl::KeyCode( KeyFuncType::COPY));
        pPopup->SetAccelKey(pPopup->GetItemId("paste"), vcl::KeyCode( KeyFuncType::PASTE));
        pPopup->SetAccelKey(pPopup->GetItemId("delete"), vcl::KeyCode( KeyFuncType::DELETE));
        pPopup->SetAccelKey(pPopup->GetItemId("selectall"), vcl::KeyCode( KEY_A, false, true, false, false));
        pPopup->SetAccelKey(pPopup->GetItemId("specialchar"), vcl::KeyCode( KEY_S, true, true, false, false));
    }
    return pPopup;
}

// css::datatransfer::dnd::XDragGestureListener
void Edit::dragGestureRecognized( const css::datatransfer::dnd::DragGestureEvent& rDGE )
{
    SolarMutexGuard aVclGuard;

    if ( !IsTracking() && maSelection.Len() &&
         !mbPassword && (!mpDDInfo || !mpDDInfo->bStarterOfDD) ) // no repeated D&D
    {
        Selection aSel( maSelection );
        aSel.Justify();

        // only if mouse in the selection...
        Point aMousePos( rDGE.DragOriginX, rDGE.DragOriginY );
        sal_Int32 nCharPos = ImplGetCharPos( aMousePos );
        if ( (nCharPos >= aSel.Min()) && (nCharPos < aSel.Max()) )
        {
            if ( !mpDDInfo )
                mpDDInfo.reset(new DDInfo);

            mpDDInfo->bStarterOfDD = true;
            mpDDInfo->aDndStartSel = aSel;

            if ( IsTracking() )
                EndTracking();  // before D&D disable tracking

            vcl::unohelper::TextDataObject* pDataObj = new vcl::unohelper::TextDataObject( GetSelected() );
            sal_Int8 nActions = datatransfer::dnd::DNDConstants::ACTION_COPY;
            if ( !IsReadOnly() )
                nActions |= datatransfer::dnd::DNDConstants::ACTION_MOVE;
            rDGE.DragSource->startDrag( rDGE, nActions, 0 /*cursor*/, 0 /*image*/, pDataObj, mxDnDListener );
            if ( GetCursor() )
                GetCursor()->Hide();

        }
    }
}

// css::datatransfer::dnd::XDragSourceListener
void Edit::dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& rDSDE )
{
    SolarMutexGuard aVclGuard;

    if (rDSDE.DropSuccess && (rDSDE.DropAction & datatransfer::dnd::DNDConstants::ACTION_MOVE) && mpDDInfo)
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
    mpDDInfo.reset();
}

// css::datatransfer::dnd::XDropTargetListener
void Edit::drop( const css::datatransfer::dnd::DropTargetDropEvent& rDTDE )
{
    SolarMutexGuard aVclGuard;

    bool bChanges = false;
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
            SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aFlavor );
            if ( xDataObj->isDataFlavorSupported( aFlavor ) )
            {
                uno::Any aData = xDataObj->getTransferData( aFlavor );
                OUString aText;
                aData >>= aText;
                ImplInsertText( aText );
                bChanges = true;
                ImplModified();
            }
        }

        if ( !mpDDInfo->bStarterOfDD )
        {
            mpDDInfo.reset();
        }
    }

    rDTDE.Context->dropComplete( bChanges );
}

void Edit::dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& rDTDE )
{
    if ( !mpDDInfo )
    {
        mpDDInfo.reset(new DDInfo);
    }
    // search for string data type
    const Sequence< css::datatransfer::DataFlavor >& rFlavors( rDTDE.SupportedDataFlavors );
    sal_Int32 nEle = rFlavors.getLength();
    mpDDInfo->bIsStringSupported = false;
    for( sal_Int32 i = 0; i < nEle; i++ )
    {
        sal_Int32 nIndex = 0;
        const OUString aMimetype = rFlavors[i].MimeType.getToken( 0, ';', nIndex );
        if ( aMimetype == "text/plain" )
        {
            mpDDInfo->bIsStringSupported = true;
            break;
        }
    }
}

void Edit::dragExit( const css::datatransfer::dnd::DropTargetEvent& )
{
    SolarMutexGuard aVclGuard;

    ImplHideDDCursor();
}

void Edit::dragOver( const css::datatransfer::dnd::DropTargetDragEvent& rDTDE )
{
    SolarMutexGuard aVclGuard;

    Point aMousePos( rDTDE.LocationX, rDTDE.LocationY );

    sal_Int32 nPrevDropPos = mpDDInfo->nDropPos;
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
        // draw the old cursor away...
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

FactoryFunction Edit::GetUITestFactory() const
{
    return EditUIObject::create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
