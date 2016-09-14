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
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/uitest/uiobject.hxx>

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

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

// - Redo
// - Bei Tracking-Cancel DefaultSelection wieder herstellen

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
    OUString      aOldTextAfterStartPos;
    ExtTextInputAttr* pAttribs;
    sal_Int32     nPos;
    sal_Int32     nLen;
    bool          bCursor;
    bool          bWasCursorOverwrite;

    Impl_IMEInfos(sal_Int32 nPos, const OUString& rOldTextAfterStartPos);
    ~Impl_IMEInfos();

    void        CopyAttribs(const ExtTextInputAttr* pA, sal_Int32 nL);
    void        DestroyAttribs();
};

Impl_IMEInfos::Impl_IMEInfos(sal_Int32 nP, const OUString& rOldTextAfterStartPos)
    : aOldTextAfterStartPos(rOldTextAfterStartPos)
{
    nPos = nP;
    nLen = 0;
    bCursor = true;
    pAttribs = nullptr;
    bWasCursorOverwrite = false;
}

Impl_IMEInfos::~Impl_IMEInfos()
{
    delete[] pAttribs;
}

void Impl_IMEInfos::CopyAttribs(const ExtTextInputAttr* pA, sal_Int32 nL)
{
    nLen = nL;
    delete[] pAttribs;
    pAttribs = new ExtTextInputAttr[ nL ];
    memcpy( pAttribs, pA, nL*sizeof(ExtTextInputAttr) );
}

void Impl_IMEInfos::DestroyAttribs()
{
    delete[] pAttribs;
    pAttribs = nullptr;
    nLen = 0;
}

Edit::Edit( WindowType nType )
    : Control( nType )
{
    ImplInitEditData();
}

Edit::Edit( vcl::Window* pParent, WinBits nStyle )
    : Control( WINDOW_EDIT )
{
    ImplInitEditData();
    ImplInit( pParent, nStyle );
}

Edit::Edit( vcl::Window* pParent, const ResId& rResId )
    : Control( WINDOW_EDIT )
{
    rResId.SetRT( RSC_EDIT );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInitEditData();
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
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

Edit::~Edit()
{
    disposeOnce();
}

void Edit::dispose()
{
    delete mpDDInfo;
    mpDDInfo = nullptr;

    vcl::Cursor* pCursor = GetCursor();
    if ( pCursor )
    {
        SetCursor( nullptr );
        delete pCursor;
    }

    delete mpIMEInfos;
    mpIMEInfos = nullptr;

    delete mpUpdateDataTimer;
    mpUpdateDataTimer = nullptr;

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

    SetType(WINDOW_WINDOW);

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
    mbInsertMode            = true;
    mbClickedInSelection    = false;
    mbActivePopup           = false;
    mbIsSubEdit             = false;
    mbInMBDown              = false;
    mpDDInfo                = nullptr;
    mpIMEInfos              = nullptr;
    mcEchoChar              = 0;

    // --- RTL --- no default mirroring for Edit controls
    // note: controls that use a subedit will revert this (SpinField, ComboBox)
    EnableRTL( false );

    vcl::unohelper::DragAndDropWrapper* pDnDWrapper = new vcl::unohelper::DragAndDropWrapper( this );
    mxDnDListener = pDnDWrapper;
}

bool Edit::ImplUseNativeBorder(vcl::RenderContext& rRenderContext, WinBits nStyle)
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

    // --- RTL --- hack: right align until keyinput and cursor travelling works
    if( IsRTLEnabled() )
        mnAlign = EDIT_ALIGN_RIGHT;

    if ( nStyle & WB_RIGHT )
        mnAlign = EDIT_ALIGN_RIGHT;
    else if ( nStyle & WB_CENTER )
        mnAlign = EDIT_ALIGN_CENTER;

    SetCursor( new vcl::Cursor );

    SetPointer( Pointer( PointerStyle::Text ) );

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

void Edit::ImplInvalidateOrRepaint()
{
    if( IsPaintTransparent() )
    {
        Invalidate();
        // FIXME: this is currently only on OS X
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

void Edit::ImplRepaint(vcl::RenderContext& rRenderContext, const Rectangle& rRectangle)
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
        if ((size_t) (2 * nLen) > SAL_N_ELEMENTS(nDXBuffer))
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
    if ((GetStyle() & WB_FORCECTRLBACKGROUND) != 0 && IsControlBackground())
    {
        // check if we need to set ControlBackground even in NWF case
        rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(GetControlBackground());
        rRenderContext.DrawRect(Rectangle(aPos, Size(GetOutputSizePixel().Width() - 2 * mnXOffset, GetOutputSizePixel().Height())));
        rRenderContext.Pop();

        rRenderContext.SetTextFillColor(GetControlBackground());
    }
    else if (IsPaintTransparent() || ImplUseNativeBorder(rRenderContext, GetStyle()))
        rRenderContext.SetTextFillColor();
    else
        rRenderContext.SetTextFillColor(IsControlBackground() ? GetControlBackground() : rStyleSettings.GetFieldColor());

    ImplPaintBorder(rRenderContext, 0, GetOutputSizePixel().Width());

    bool bDrawSelection = maSelection.Len() && (HasFocus() || (GetStyle() & WB_NOHIDESELECTION) || mbActivePopup);

    aPos.X() = mnXOffset + ImplGetExtraXOffset();
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
        // first calculate higlighted and non highlighted clip regions
        vcl::Region aHiglightClipRegion;
        vcl::Region aNormalClipRegion;
        Selection aTmpSel(maSelection);
        aTmpSel.Justify();
        // selection is highlighted
        for(sal_Int32 i = 0; i < nLen; ++i)
        {
            Rectangle aRect(aPos, Size(10, nTH));
            aRect.Left() = pDX[2 * i] + mnXOffset + ImplGetExtraXOffset();
            aRect.Right() = pDX[2 * i + 1] + mnXOffset + ImplGetExtraXOffset();
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
                aHiglightClipRegion.Union(aRect);
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
                if( (GetStyle() & WB_FORCECTRLBACKGROUND) != 0 && IsControlBackground() )
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
        rRenderContext.SetClipRegion(aHiglightClipRegion);
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
                    aRegion = aHiglightClipRegion;
                }

                for(int i = 0; i < mpIMEInfos->nLen; )
                {
                    ExtTextInputAttr nAttr = mpIMEInfos->pAttribs[i];
                    vcl::Region aClip;
                    int nIndex = i;
                    while (nIndex < mpIMEInfos->nLen && mpIMEInfos->pAttribs[nIndex] == nAttr)  // #112631# check nIndex before using it
                    {
                        Rectangle aRect( aPos, Size( 10, nTH ) );
                        aRect.Left() = pDX[2 * (nIndex + mpIMEInfos->nPos)] + mnXOffset + ImplGetExtraXOffset();
                        aRect.Right() = pDX[2 * (nIndex + mpIMEInfos->nPos) + 1] + mnXOffset + ImplGetExtraXOffset();
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
                            rRenderContext.SetTextLineColor(Color(COL_LIGHTGRAY));
                        }
                        rRenderContext.SetFont(aFont);

                        if (nAttr & ExtTextInputAttr::RedText)
                            rRenderContext.SetTextColor(Color(COL_RED));
                        else if (nAttr & ExtTextInputAttr::HalfToneText)
                            rRenderContext.SetTextColor(Color(COL_LIGHTGRAY));

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

    // loeschen moeglich?
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

    maText.remove( static_cast<sal_Int32>(aSelection.Min()), static_cast<sal_Int32>(aSelection.Len()) );
    maSelection.Min() = aSelection.Min();
    maSelection.Max() = aSelection.Min();
    ImplAlignAndPaint();
    mbInternModified = true;
}

OUString Edit::ImplGetValidString( const OUString& rString ) const
{
    OUString aValidString( rString );
    aValidString = aValidString.replaceAll("\n", "").replaceAll("\r", "");
    aValidString = aValidString.replace('\t', ' ');
    return aValidString;
}

uno::Reference < i18n::XBreakIterator > Edit::ImplGetBreakIterator() const
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

void Edit::ShowTruncationWarning( vcl::Window* pParent )
{
    ResMgr* pResMgr = ImplGetResMgr();
    if( pResMgr )
    {
        ScopedVclPtrInstance< MessageDialog > aBox( pParent, ResId(SV_EDIT_WARNING_STR, *pResMgr), VclMessageType::Warning );
        aBox->Execute();
    }
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

        CallEventListeners( VCLEVENT_EDIT_MODIFY );
    }
}

ControlType Edit::ImplGetNativeControlType() const
{
    ControlType nCtrl = ControlType::Generic;
    const vcl::Window* pControl = mbIsSubEdit ? GetParent() : this;

    switch (pControl->GetType())
    {
        case WINDOW_COMBOBOX:
        case WINDOW_PATTERNBOX:
        case WINDOW_NUMERICBOX:
        case WINDOW_METRICBOX:
        case WINDOW_CURRENCYBOX:
        case WINDOW_DATEBOX:
        case WINDOW_TIMEBOX:
        case WINDOW_LONGCURRENCYBOX:
            nCtrl = ControlType::Combobox;
            break;

        case WINDOW_MULTILINEEDIT:
            if ( GetWindow( GetWindowType::Border ) != this )
                nCtrl = ControlType::MultilineEditbox;
            else
                nCtrl = ControlType::EditboxNoBorder;
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

void Edit::ImplClearBackground(vcl::RenderContext& rRenderContext, const Rectangle& rRectangle, long nXStart, long nXEnd )
{
    /*
    * note: at this point the cursor must be switched off already
    */
    Point aTmpPoint;
    Rectangle aRect(aTmpPoint, GetOutputSizePixel());
    aRect.Left() = nXStart;
    aRect.Right() = nXEnd;

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

void Edit::ImplPaintBorder(vcl::RenderContext& rRenderContext, long nXStart, long nXEnd)
{
    // this is not needed when double-buffering
    if (SupportsDoubleBuffering())
        return;

    Point aTmpPoint;
    Rectangle aRect(aTmpPoint, GetOutputSizePixel());
    aRect.Left() = nXStart;
    aRect.Right() = nXEnd;

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
                    Rectangle aBounds(aClipRgn.GetBoundRect());
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

                pBorder->Paint(*pBorder, Rectangle());

                pBorder->SetClipRegion(oldRgn);
            }
            else
            {
                pBorder->Paint(*pBorder, Rectangle());
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
        if( (size_t) (2*aText.getLength()) > SAL_N_ELEMENTS(nDXBuffer) )
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
            // Etwas mehr?
            if ( (aOutSize.Width()-ImplGetExtraXOffset()) < nTextPos )
            {
                long nMaxNegX = (aOutSize.Width()-ImplGetExtraXOffset()) - GetTextWidth( aText );
                mnXOffset -= aOutSize.Width() / 5;
                if ( mnXOffset < nMaxNegX )  // beides negativ...
                    mnXOffset = nMaxNegX;
            }
        }

        nCursorPosX = nTextPos + mnXOffset + ImplGetExtraXOffset();
        if ( nCursorPosX == aOutSize.Width() )  // dann nicht sichtbar...
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
    long nTextWidth = GetTextWidth( ImplGetText() );
    long nOutWidth = GetOutputSizePixel().Width();

    if ( mnAlign == EDIT_ALIGN_LEFT )
    {
        if( mnXOffset && ( nTextWidth < nOutWidth ) )
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
    if( (size_t) (2*aText.getLength()) > SAL_N_ELEMENTS(nDXBuffer) )
    {
        pDXBuffer.reset(new long[2*(aText.getLength()+1)]);
        pDX = pDXBuffer.get();
    }

    GetCaretPositions( aText, pDX, 0, aText.getLength() );
    long nX = rWindowPos.X() - mnXOffset - ImplGetExtraXOffset();
    for( sal_Int32 i = 0; i < aText.getLength(); i++ )
    {
        if( (pDX[2*i] >= nX && pDX[2*i+1] <= nX) ||
            (pDX[2*i+1] >= nX && pDX[2*i] <= nX))
        {
            nIndex = i;
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
    if( nIndex == EDIT_NOLIMIT )
    {
        nIndex = 0;
        long nDiff = std::abs( pDX[0]-nX );
        for( sal_Int32 i = 1; i < aText.getLength(); i++ )
        {
            long nNewDiff = std::abs( pDX[2*i]-nX );

            if( nNewDiff < nDiff )
            {
                nIndex = i;
                nDiff = nNewDiff;
            }
        }
        if( nIndex == aText.getLength()-1 && std::abs( pDX[2*nIndex+1] - nX ) < nDiff )
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

void Edit::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    sal_uInt16 nTextLength = ReadShortRes();
    if ( nTextLength )
        SetMaxTextLen( nTextLength );
}

void Edit::ImplCopyToSelectionClipboard()
{
    if ( GetSelection().Len() )
    {
        css::uno::Reference<css::datatransfer::clipboard::XClipboard> aSelection(GetPrimarySelection());
        ImplCopy( aSelection );
    }
}

void Edit::ImplCopy( uno::Reference< datatransfer::clipboard::XClipboard >& rxClipboard )
{
    vcl::unohelper::TextDataObject::CopyStringTo( GetSelected(), rxClipboard );
}

void Edit::ImplPaste( uno::Reference< datatransfer::clipboard::XClipboard >& rxClipboard )
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
                    ShowTruncationWarning( this );
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

    mbInMBDown = true;  // then do not select all in GetFocus
    GrabFocus();
    mbInMBDown = false;
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
                if ( !mbReadOnly && maSelection.Len() && !(GetStyle() & WB_PASSWORD) )
                {
                    Cut();
                    ImplModified();
                    bDone = true;
                }
            }
            break;

            case KeyFuncType::COPY:
            {
                if ( !(GetStyle() & WB_PASSWORD) )
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
                        SAL_FALLTHROUGH;
                    case css::awt::Key::MOVE_TO_BEGIN_OF_LINE:
                    case css::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH:
                    case css::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT:
                        bGoHome = true;break;
                    case css::awt::Key::SELECT_TO_END_OF_LINE:
                    case css::awt::Key::SELECT_TO_END_OF_PARAGRAPH:
                    case css::awt::Key::SELECT_TO_END_OF_DOCUMENT:
                        bSelect = true;
                        SAL_FALLTHROUGH;
                    case css::awt::Key::MOVE_TO_END_OF_LINE:
                    case css::awt::Key::MOVE_TO_END_OF_PARAGRAPH:
                    case css::awt::Key::MOVE_TO_END_OF_DOCUMENT:
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
    mpControlData->mpLayoutData = new vcl::ControlLayoutData();
    const_cast<Edit*>(this)->Invalidate();
}

void Edit::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRectangle)
{
    if (!mpSubEdit)
        ImplRepaint(rRenderContext, rRectangle);
}

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
    bool bBorder = !(nFlags & DrawFlags::NoBorder ) && (GetStyle() & WB_BORDER);
    bool bBackground = !(nFlags & DrawFlags::NoBackground) && IsControlBackground();
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
    if ( ( nFlags & DrawFlags::Mono ) || ( eOutDevType == OUTDEV_PRINTER ) )
        pDev->SetTextColor( Color( COL_BLACK ) );
    else
    {
        if ( !(nFlags & DrawFlags::NoDisable ) && !IsEnabled() )
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
    Rectangle aTextRect( aPos, aSize );

    if ( GetStyle() & WB_CENTER )
        nTextStyle |= DrawTextFlags::Center;
    else if ( GetStyle() & WB_RIGHT )
        nTextStyle |= DrawTextFlags::Right;
    else
        nTextStyle |= DrawTextFlags::Left;

    aTextRect.Left() += nOffX;
    aTextRect.Right() -= nOffX;

    OUString    aText = ImplGetText();
    long        nTextHeight = pDev->GetTextHeight();
    long        nTextWidth = pDev->GetTextWidth( aText );
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
                static_cast<Edit*>(GetParent())->CallEventListeners( VCLEVENT_EDIT_SELECTIONCHANGED );
            else
                CallEventListeners( VCLEVENT_EDIT_SELECTIONCHANGED );
        }

        ImplShowCursor();

        // FIXME: this is currently only on OS X
        // check for other platforms that need similar handling
        if( ImplGetSVData()->maNWFData.mbNoFocusRects &&
            IsNativeWidgetEnabled() &&
            IsNativeControlSupported( ControlType::Editbox, ControlPart::Entire ) )
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
        // FIXME: this is currently only on OS X
        // check for other platforms that need similar handling
        if( ImplGetSVData()->maNWFData.mbNoFocusRects &&
            IsNativeWidgetEnabled() &&
            IsNativeControlSupported( ControlType::Editbox, ControlPart::Entire ) )
        {
            ImplInvalidateOutermostBorder( mbIsSubEdit ? GetParent() : this );
        }

        if ( !mbActivePopup && !( GetStyle() & WB_NOHIDESELECTION ) && maSelection.Len() )
            ImplInvalidateOrRepaint();    // Selektion malen
    }

    Control::LoseFocus();
}

void Edit::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        VclPtr<PopupMenu> pPopup = Edit::CreatePopupMenu();

        if ( !maSelection.Len() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_CUT, false );
            pPopup->EnableItem( SV_MENU_EDIT_COPY, false );
            pPopup->EnableItem( SV_MENU_EDIT_DELETE, false );
        }

        if ( IsReadOnly() )
        {
            pPopup->EnableItem( SV_MENU_EDIT_CUT, false );
            pPopup->EnableItem( SV_MENU_EDIT_PASTE, false );
            pPopup->EnableItem( SV_MENU_EDIT_DELETE, false );
            pPopup->EnableItem( SV_MENU_EDIT_INSERTSYMBOL, false );
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
            pPopup->EnableItem( SV_MENU_EDIT_PASTE, bData );
        }

        if ( maUndoText == maText.getStr() )
            pPopup->EnableItem( SV_MENU_EDIT_UNDO, false );
        if ( ( maSelection.Min() == 0 ) && ( maSelection.Max() == maText.getLength() ) )
            pPopup->EnableItem( SV_MENU_EDIT_SELECTALL, false );
        if ( !pImplFncGetSpecialChars )
        {
            sal_uInt16 nPos = pPopup->GetItemPos( SV_MENU_EDIT_INSERTSYMBOL );
            pPopup->RemoveItem( nPos );
            pPopup->RemoveItem( nPos-1 );
        }

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
        pPopup.disposeAndClear();
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
        mbActivePopup = false;
    }
    else if ( rCEvt.GetCommand() == CommandEventId::StartExtTextInput )
    {
        DeleteSelected();
        delete mpIMEInfos;
        sal_Int32 nPos = static_cast<sal_Int32>(maSelection.Max());
        mpIMEInfos = new Impl_IMEInfos( nPos, OUString(maText.getStr() + nPos ) );
        mpIMEInfos->bWasCursorOverwrite = !IsInsertMode();
    }
    else if ( rCEvt.GetCommand() == CommandEventId::EndExtTextInput )
    {
        bool bInsertMode = !mpIMEInfos->bWasCursorOverwrite;
        delete mpIMEInfos;
        mpIMEInfos = nullptr;

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
                if( (size_t) (2*aText.getLength()) > SAL_N_ELEMENTS(nDXBuffer) )
                {
                    pDXBuffer.reset(new long[2*(aText.getLength()+1)]);
                    pDX = pDXBuffer.get();
                }

                GetCaretPositions( aText, pDX, 0, aText.getLength() );
            }
            long    nTH = GetTextHeight();
            Point   aPos( mnXOffset, ImplGetTextYPosition() );

            std::unique_ptr<Rectangle[]> aRects(new Rectangle[ mpIMEInfos->nLen ]);
            for ( int nIndex = 0; nIndex < mpIMEInfos->nLen; ++nIndex )
            {
                Rectangle aRect( aPos, Size( 10, nTH ) );
                aRect.Left() = pDX[2*(nIndex+mpIMEInfos->nPos)] + mnXOffset + ImplGetExtraXOffset();
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

        // --- RTL --- hack: right align until keyinput and cursor travelling works
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
        Rectangle aCursorRect( Point( nTextWidth + mnXOffset, (GetOutputSize().Height()-nTextHeight)/2 ), Size( 2, nTextHeight ) );
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

        if ( ImplCallEventListenersAndHandler( VCLEVENT_EDIT_MODIFY, [this] () { maModifyHdl.Call(*this); } ) )
            // have been destroyed while calling into the handlers
            return;

        // #i13677# notify edit listeners about caret position change
        CallEventListeners( VCLEVENT_EDIT_CARETCHANGED );
        // FIXME: this is currently only on OS X
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

IMPL_LINK_NOARG_TYPED(Edit, ImplUpdateDataHdl, Timer *, void)
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
            mpUpdateDataTimer = new Timer("UpdateDataTimer");
            mpUpdateDataTimer->SetTimeoutHdl( LINK( this, Edit, ImplUpdateDataHdl ) );
            mpUpdateDataTimer->SetDebugName( "vcl::Edit mpUpdateDataTimer" );
        }

        mpUpdateDataTimer->SetTimeout( nTimeout );
    }
}

void Edit::DisableUpdateData()
{
    delete mpUpdateDataTimer;
    mpUpdateDataTimer = nullptr;
}

void Edit::SetEchoChar( sal_Unicode c )
{
    mcEchoChar = c;
    if ( mpSubEdit )
        mpSubEdit->SetEchoChar( c );
}

void Edit::SetReadOnly( bool bReadOnly )
{
    if ( mbReadOnly != bool(bReadOnly) )
    {
        mbReadOnly = bReadOnly;
        if ( mpSubEdit )
            mpSubEdit->SetReadOnly( bReadOnly );

        CompatStateChanged( StateChangedType::ReadOnly );
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
                        static_cast<Edit*>(GetParent())->CallEventListeners( VCLEVENT_EDIT_SELECTIONCHANGED );
                    else
                        CallEventListeners( VCLEVENT_EDIT_SELECTIONCHANGED );
                }

                if (bCaret)
                {
                    if ( mbIsSubEdit )
                        static_cast<Edit*>(GetParent())->CallEventListeners( VCLEVENT_EDIT_CARETCHANGED );
                    else
                        CallEventListeners( VCLEVENT_EDIT_CARETCHANGED );
                }

                // #103511# notify combobox listeners of deselection
                if( !maSelection && GetParent() && GetParent()->GetType() == WINDOW_COMBOBOX )
                    static_cast<Edit*>(GetParent())->CallEventListeners( VCLEVENT_COMBOBOX_DESELECT );
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
    if ( !(GetStyle() & WB_PASSWORD ) )
    {
        Copy();
        ReplaceSelected( OUString() );
    }
}

void Edit::Copy()
{
    if ( !(GetStyle() & WB_PASSWORD ) )
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
        SetPointer(PointerStyle::Arrow);    // Nur das SubEdit hat den BEAM...
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

        aSize.Height() = GetTextHeight();
        aSize.Width() = GetTextWidth(aString);
        aSize.Width() += ImplGetExtraXOffset() * 2;

        // do not create edit fields in which one cannot enter anything
        // a default minimum width should exist for at least 3 characters

        //CalcSize calls CalcWindowSize, but we will call that also in this
        //function, so undo the first one with CalcOutputSize
        Size aMinSize(CalcOutputSize(CalcSize(3)));
        if (aSize.Width() < aMinSize.Width())
            aSize.Width() = aMinSize.Width();
    }

    aSize.Height() += ImplGetExtraYOffset() * 2;

    aSize = CalcWindowSize( aSize );

    // ask NWF what if it has an opinion, too
    ImplControlValue aControlValue;
    Rectangle aRect( Point( 0, 0 ), aSize );
    Rectangle aContent, aBound;
    if (GetNativeControlRegion(eCtrlType, ControlPart::Entire, aRect, ControlState::NONE,
                               aControlValue, OUString(), aBound, aContent))
    {
        if (aBound.GetHeight() > aSize.Height())
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
    aSz.Width() *= nChars;
    aSz.Width() += ImplGetExtraXOffset() * 2;
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
    ResMgr* pResMgr = ImplGetResMgr();
    if( ! pResMgr )
        return VclPtr<PopupMenu>::Create();

    VclPtrInstance<PopupMenu> pPopup( ResId( SV_RESID_MENU_EDIT, *pResMgr ) );
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if ( rStyleSettings.GetHideDisabledMenuItems() )
        pPopup->SetMenuFlags( MenuFlags::HideDisabledEntries );
    else
        pPopup->SetMenuFlags ( MenuFlags::AlwaysShowDisabledEntries );
    if ( rStyleSettings.GetContextMenuShortcuts() )
    {
        pPopup->SetAccelKey( SV_MENU_EDIT_UNDO, vcl::KeyCode( KeyFuncType::UNDO ) );
        pPopup->SetAccelKey( SV_MENU_EDIT_CUT, vcl::KeyCode( KeyFuncType::CUT ) );
        pPopup->SetAccelKey( SV_MENU_EDIT_COPY, vcl::KeyCode( KeyFuncType::COPY ) );
        pPopup->SetAccelKey( SV_MENU_EDIT_PASTE, vcl::KeyCode( KeyFuncType::PASTE ) );
        pPopup->SetAccelKey( SV_MENU_EDIT_DELETE, vcl::KeyCode( KeyFuncType::DELETE ) );
        pPopup->SetAccelKey( SV_MENU_EDIT_SELECTALL, vcl::KeyCode( KEY_A, false, true, false, false ) );
        pPopup->SetAccelKey( SV_MENU_EDIT_INSERTSYMBOL, vcl::KeyCode( KEY_S, true, true, false, false ) );
    }
    return pPopup;
}

// css::datatransfer::dnd::XDragGestureListener
void Edit::dragGestureRecognized( const css::datatransfer::dnd::DragGestureEvent& rDGE ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aVclGuard;

    if ( !IsTracking() && maSelection.Len() &&
         !(GetStyle() & WB_PASSWORD) && (!mpDDInfo || !mpDDInfo->bStarterOfDD) ) // Kein Mehrfach D&D
    {
        Selection aSel( maSelection );
        aSel.Justify();

        // Nur wenn Maus in der Selektion...
        Point aMousePos( rDGE.DragOriginX, rDGE.DragOriginY );
        sal_Int32 nCharPos = ImplGetCharPos( aMousePos );
        if ( (nCharPos >= aSel.Min()) && (nCharPos < aSel.Max()) )
        {
            if ( !mpDDInfo )
                mpDDInfo = new DDInfo;

            mpDDInfo->bStarterOfDD = true;
            mpDDInfo->aDndStartSel = aSel;

            if ( IsTracking() )
                EndTracking();  // Vor D&D Tracking ausschalten

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
void Edit::dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& rDSDE ) throw (css::uno::RuntimeException, std::exception)
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
    mpDDInfo = nullptr;
}

// css::datatransfer::dnd::XDropTargetListener
void Edit::drop( const css::datatransfer::dnd::DropTargetDropEvent& rDTDE ) throw (css::uno::RuntimeException, std::exception)
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
            delete mpDDInfo;
            mpDDInfo = nullptr;
        }
    }

    rDTDE.Context->dropComplete( bChanges );
}

void Edit::dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& rDTDE ) throw (css::uno::RuntimeException, std::exception)
{
    if ( !mpDDInfo )
    {
        mpDDInfo = new DDInfo;
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

void Edit::dragExit( const css::datatransfer::dnd::DropTargetEvent& ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aVclGuard;

    ImplHideDDCursor();
}

void Edit::dragOver( const css::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (css::uno::RuntimeException, std::exception)
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

FactoryFunction Edit::GetUITestFactory() const
{
    return EditUIObject::create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
