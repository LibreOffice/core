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

#include <tools/lineend.hxx>
#include <tools/poly.hxx>

#include <vcl/image.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/edit.hxx>
#include <vcl/layout.hxx>
#include <vcl/vclstatuslistener.hxx>
#include <vcl/uitest/uiobject.hxx>

#include <strings.hrc>
#include <bitmaps.hlst>
#include <svdata.hxx>
#include <window.h>
#include <controldata.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/lok.hxx>
#include <officecfg/Office/Common.hxx>


using namespace css;

static constexpr auto PUSHBUTTON_VIEW_STYLE = WB_3DLOOK |
                                     WB_LEFT | WB_CENTER | WB_RIGHT |
                                     WB_TOP | WB_VCENTER | WB_BOTTOM |
                                     WB_WORDBREAK | WB_NOLABEL |
                                     WB_DEFBUTTON | WB_NOLIGHTBORDER |
                                     WB_RECTSTYLE | WB_SMALLSTYLE |
                                     WB_TOGGLE;
static constexpr auto RADIOBUTTON_VIEW_STYLE = WB_3DLOOK |
                                     WB_LEFT | WB_CENTER | WB_RIGHT |
                                     WB_TOP | WB_VCENTER | WB_BOTTOM |
                                     WB_WORDBREAK | WB_NOLABEL;
static constexpr auto CHECKBOX_VIEW_STYLE = WB_3DLOOK |
                                     WB_LEFT | WB_CENTER | WB_RIGHT |
                                     WB_TOP | WB_VCENTER | WB_BOTTOM |
                                     WB_WORDBREAK | WB_NOLABEL;

#define STYLE_RADIOBUTTON_MONO      (sal_uInt16(0x0001)) // legacy
#define STYLE_CHECKBOX_MONO         (sal_uInt16(0x0001)) // legacy

class ImplCommonButtonData
{
public:
    ImplCommonButtonData();

    tools::Rectangle       maFocusRect;
    long            mnSeparatorX;
    DrawButtonFlags mnButtonState;
    bool            mbSmallSymbol;

    Image           maImage;
    ImageAlign      meImageAlign;
    SymbolAlign     meSymbolAlign;

    /** StatusListener. Updates the button as the slot state changes */
    rtl::Reference<VclStatusListener<Button>> mpStatusListener;
};

ImplCommonButtonData::ImplCommonButtonData() : maFocusRect(), mnSeparatorX(0), mnButtonState(DrawButtonFlags::NONE),
mbSmallSymbol(false), maImage(), meImageAlign(ImageAlign::Top), meSymbolAlign(SymbolAlign::LEFT)
{
}

Button::Button( WindowType nType ) :
    Control( nType ),
    mpButtonData( std::make_unique<ImplCommonButtonData>() )
{
}

Button::~Button()
{
    disposeOnce();
}

void Button::dispose()
{
    if (mpButtonData->mpStatusListener.is())
        mpButtonData->mpStatusListener->dispose();
    Control::dispose();
}

void Button::SetCommandHandler(const OUString& aCommand)
{
    maCommand = aCommand;
    SetClickHdl( LINK( this, Button, dispatchCommandHandler) );

    mpButtonData->mpStatusListener = new VclStatusListener<Button>(this, aCommand);
    mpButtonData->mpStatusListener->startListening();
}

void Button::Click()
{
    ImplCallEventListenersAndHandler( VclEventId::ButtonClick, [this] () { maClickHdl.Call(this); } );
}

OUString Button::GetStandardText(StandardButtonType eButton)
{
    static const char* aResIdAry[static_cast<int>(StandardButtonType::Count)] =
    {
        // http://lists.freedesktop.org/archives/libreoffice/2013-January/044513.html
        // Under windows we don't want accelerators on ok/cancel but do on other
        // buttons
#ifdef _WIN32
        SV_BUTTONTEXT_OK_NOMNEMONIC,
        SV_BUTTONTEXT_CANCEL_NOMNEMONIC,
#else
        SV_BUTTONTEXT_OK,
        SV_BUTTONTEXT_CANCEL,
#endif
        SV_BUTTONTEXT_YES,
        SV_BUTTONTEXT_NO,
        SV_BUTTONTEXT_RETRY,
        SV_BUTTONTEXT_HELP,
        SV_BUTTONTEXT_CLOSE,
        SV_BUTTONTEXT_MORE,
        SV_BUTTONTEXT_IGNORE,
        SV_BUTTONTEXT_ABORT,
        SV_BUTTONTEXT_LESS,
    };

    return VclResId(aResIdAry[static_cast<sal_uInt16>(eButton)]);
}

void Button::SetModeImage( const Image& rImage )
{
    if ( rImage != mpButtonData->maImage )
    {
        mpButtonData->maImage = rImage;
        StateChanged( StateChangedType::Data );
        queue_resize();
    }
}

Image const & Button::GetModeImage( ) const
{
    return mpButtonData->maImage;
}

bool Button::HasImage() const
{
    return !!(mpButtonData->maImage);
}

void Button::SetImageAlign( ImageAlign eAlign )
{
    if ( mpButtonData->meImageAlign != eAlign )
    {
        mpButtonData->meImageAlign = eAlign;
        StateChanged( StateChangedType::Data );
    }
}

ImageAlign Button::GetImageAlign() const
{
    return mpButtonData->meImageAlign;
}

long Button::ImplGetSeparatorX() const
{
    return mpButtonData->mnSeparatorX;
}

void Button::ImplSetSeparatorX( long nX )
{
    mpButtonData->mnSeparatorX = nX;
}

DrawTextFlags Button::ImplGetTextStyle( WinBits nWinStyle, DrawFlags nDrawFlags )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    DrawTextFlags nTextStyle = FixedText::ImplGetTextStyle(nWinStyle & ~WB_DEFBUTTON);

    if (!IsEnabled())
        nTextStyle |= DrawTextFlags::Disable;

    if ((nDrawFlags & DrawFlags::Mono) ||
        (rStyleSettings.GetOptions() & StyleSettingsOptions::Mono))
    {
        nTextStyle |= DrawTextFlags::Mono;
    }

    return nTextStyle;
}

void Button::ImplDrawAlignedImage(OutputDevice* pDev, Point& rPos,
                                  Size& rSize,
                                  sal_uLong nImageSep,
                                  DrawTextFlags nTextStyle, tools::Rectangle *pSymbolRect,
                                  bool bAddImageSep)
{
    OUString aText(GetText());
    bool bDrawImage = HasImage() && ! (ImplGetButtonState() & DrawButtonFlags::NoImage);
    bool bDrawText  = !aText.isEmpty() && ! (ImplGetButtonState() & DrawButtonFlags::NoText);
    bool bHasSymbol = pSymbolRect != nullptr;

    // No text and no image => nothing to do => return
    if (!bDrawImage && !bDrawText && !bHasSymbol)
        return;

    WinBits nWinStyle = GetStyle();
    tools::Rectangle aOutRect( rPos, rSize );
    ImageAlign eImageAlign = mpButtonData->meImageAlign;
    Size aImageSize = mpButtonData->maImage.GetSizePixel();

    aImageSize.setWidth( CalcZoom( aImageSize.Width() ) );
    aImageSize.setHeight( CalcZoom( aImageSize.Height() ) );

    // Drawing text or symbol only is simple, use style and output rectangle
    if (bHasSymbol && !bDrawImage && !bDrawText)
    {
        *pSymbolRect = aOutRect;
        return;
    }
    else if (bDrawText && !bDrawImage && !bHasSymbol)
    {
        aOutRect = DrawControlText(*pDev, aOutRect, aText, nTextStyle, nullptr, nullptr);

        ImplSetFocusRect(aOutRect);
        rSize = aOutRect.GetSize();
        rPos = aOutRect.TopLeft();

        return;
    }

    // check for HC mode ( image only! )
    Image* pImage = &(mpButtonData->maImage);

    Size aTextSize;
    Size aSymbolSize;
    Size aDeviceTextSize;
    Size aMax;
    Point aImagePos = rPos;
    Point aTextPos = rPos;
    tools::Rectangle aUnion = tools::Rectangle(aImagePos, aImageSize);
    tools::Rectangle aSymbol;
    long nSymbolHeight = 0;

    if (bDrawText || bHasSymbol)
    {
        // Get the size of the text output area ( the symbol will be drawn in
        // this area as well, so the symbol rectangle will be calculated here, too )

        tools::Rectangle aRect = tools::Rectangle(Point(), rSize);
        Size aTSSize;

        if (bHasSymbol)
        {
            if (bDrawText)
            {
                nSymbolHeight = pDev->GetTextHeight();
                if (mpButtonData->mbSmallSymbol)
                    nSymbolHeight = nSymbolHeight * 3 / 4;

                aSymbol = tools::Rectangle(Point(), Size(nSymbolHeight, nSymbolHeight));
                ImplCalcSymbolRect(aSymbol);
                aRect.AdjustLeft(3 * nSymbolHeight / 2 );
                aTSSize.setWidth( 3 * nSymbolHeight / 2 );
            }
            else
            {
                aSymbol = tools::Rectangle(Point(), rSize);
                ImplCalcSymbolRect(aSymbol);
                aTSSize.setWidth( aSymbol.GetWidth() );
            }
            aTSSize.setHeight( aSymbol.GetHeight() );
            aSymbolSize = aSymbol.GetSize();
        }

        if (bDrawText)
        {
            if ((eImageAlign == ImageAlign::LeftTop)     ||
                (eImageAlign == ImageAlign::Left )        ||
                (eImageAlign == ImageAlign::LeftBottom)  ||
                (eImageAlign == ImageAlign::RightTop)    ||
                (eImageAlign == ImageAlign::Right)        ||
                (eImageAlign == ImageAlign::RightBottom))
            {
                aRect.AdjustRight( -sal_Int32(aImageSize.Width() + nImageSep) );
            }
            else if ((eImageAlign == ImageAlign::TopLeft)    ||
                     (eImageAlign == ImageAlign::Top)         ||
                     (eImageAlign == ImageAlign::TopRight)   ||
                     (eImageAlign == ImageAlign::BottomLeft) ||
                     (eImageAlign == ImageAlign::Bottom)      ||
                     (eImageAlign == ImageAlign::BottomRight))
            {
                aRect.AdjustBottom( -sal_Int32(aImageSize.Height() + nImageSep) );
            }

            aRect = GetControlTextRect(*pDev, aRect, aText, nTextStyle, &aDeviceTextSize);
            aTextSize = aRect.GetSize();

            aTSSize.AdjustWidth(aTextSize.Width() );

            if (aTSSize.Height() < aTextSize.Height())
                aTSSize.setHeight( aTextSize.Height() );

            if (bAddImageSep && bDrawImage)
            {
                long nDiff = (aImageSize.Height() - aTextSize.Height()) / 3;
                if (nDiff > 0)
                    nImageSep += nDiff;
            }
        }

        aMax.setWidth( std::max(aTSSize.Width(), aImageSize.Width()) );
        aMax.setHeight( std::max(aTSSize.Height(), aImageSize.Height()) );

        // Now calculate the output area for the image and the text according to the image align flags

        if ((eImageAlign == ImageAlign::Left) ||
            (eImageAlign == ImageAlign::Right))
        {
            aImagePos.setY( rPos.Y() + (aMax.Height() - aImageSize.Height()) / 2 );
            aTextPos.setY( rPos.Y() + (aMax.Height() - aTSSize.Height()) / 2 );
        }
        else if ((eImageAlign == ImageAlign::LeftBottom) ||
                 (eImageAlign == ImageAlign::RightBottom))
        {
            aImagePos.setY( rPos.Y() + aMax.Height() - aImageSize.Height() );
            aTextPos.setY( rPos.Y() + aMax.Height() - aTSSize.Height() );
        }
        else if ((eImageAlign == ImageAlign::Top) ||
                 (eImageAlign == ImageAlign::Bottom))
        {
            aImagePos.setX( rPos.X() + (aMax.Width() - aImageSize.Width()) / 2 );
            aTextPos.setX( rPos.X() + (aMax.Width() - aTSSize.Width()) / 2 );
        }
        else if ((eImageAlign == ImageAlign::TopRight) ||
                 (eImageAlign == ImageAlign::BottomRight))
        {
            aImagePos.setX( rPos.X() + aMax.Width() - aImageSize.Width() );
            aTextPos.setX( rPos.X() + aMax.Width() - aTSSize.Width() );
        }

        if ((eImageAlign == ImageAlign::LeftTop) ||
            (eImageAlign == ImageAlign::Left)     ||
            (eImageAlign == ImageAlign::LeftBottom))
        {
            aTextPos.setX( rPos.X() + aImageSize.Width() + nImageSep );
        }
        else if ((eImageAlign == ImageAlign::RightTop) ||
                 (eImageAlign == ImageAlign::Right)     ||
                 (eImageAlign == ImageAlign::RightBottom))
        {
            aImagePos.setX( rPos.X() + aTSSize.Width() + nImageSep );
        }
        else if ((eImageAlign == ImageAlign::TopLeft) ||
                 (eImageAlign == ImageAlign::Top)      ||
                 (eImageAlign == ImageAlign::TopRight))
        {
            aTextPos.setY( rPos.Y() + aImageSize.Height() + nImageSep );
        }
        else if ((eImageAlign == ImageAlign::BottomLeft) ||
                 (eImageAlign == ImageAlign::Bottom)      ||
                 (eImageAlign == ImageAlign::BottomRight))
        {
            aImagePos.setY( rPos.Y() + aTSSize.Height() + nImageSep );
        }
        else if (eImageAlign == ImageAlign::Center)
        {
            aImagePos.setX( rPos.X() + (aMax.Width()  - aImageSize.Width()) / 2 );
            aImagePos.setY( rPos.Y() + (aMax.Height() - aImageSize.Height()) / 2 );
            aTextPos.setX( rPos.X() + (aMax.Width()  - aTSSize.Width()) / 2 );
            aTextPos.setY( rPos.Y() + (aMax.Height() - aTSSize.Height()) / 2 );
        }
        aUnion = tools::Rectangle(aImagePos, aImageSize);
        aUnion.Union(tools::Rectangle(aTextPos, aTSSize));
    }

    // Now place the combination of text and image in the output area of the button
    // according to the window style (WinBits)
    long nXOffset = 0;
    long nYOffset = 0;

    if (nWinStyle & WB_CENTER)
    {
        nXOffset = (rSize.Width() - aUnion.GetWidth()) / 2;
    }
    else if (nWinStyle & WB_RIGHT)
    {
        nXOffset = rSize.Width() - aUnion.GetWidth();
    }

    if (nWinStyle & WB_VCENTER)
    {
        nYOffset = (rSize.Height() - aUnion.GetHeight()) / 2;
    }
    else if (nWinStyle & WB_BOTTOM)
    {
        nYOffset = rSize.Height() - aUnion.GetHeight();
    }

    // the top left corner should always be visible, so we don't allow negative offsets
    if (nXOffset < 0) nXOffset = 0;
    if (nYOffset < 0) nYOffset = 0;

    aImagePos.AdjustX(nXOffset );
    aImagePos.AdjustY(nYOffset );
    aTextPos.AdjustX(nXOffset );
    aTextPos.AdjustY(nYOffset );

    // set rPos and rSize to the union
    rSize = aUnion.GetSize();
    rPos.AdjustX(nXOffset );
    rPos.AdjustY(nYOffset );

    if (bHasSymbol)
    {
        if (mpButtonData->meSymbolAlign == SymbolAlign::RIGHT)
        {
            Point aRightPos = Point(aTextPos.X() + aTextSize.Width() + aSymbolSize.Width() / 2, aTextPos.Y());
            *pSymbolRect = tools::Rectangle(aRightPos, aSymbolSize);
        }
        else
        {
            *pSymbolRect = tools::Rectangle(aTextPos, aSymbolSize);
            aTextPos.AdjustX(3 * nSymbolHeight / 2 );
        }
        if (mpButtonData->mbSmallSymbol)
        {
            nYOffset = (aUnion.GetHeight() - aSymbolSize.Height()) / 2;
            pSymbolRect->setY(aTextPos.Y() + nYOffset);
        }
    }

    DrawImageFlags nStyle = DrawImageFlags::NONE;

    if (!IsEnabled())
    {
        nStyle |= DrawImageFlags::Disable;
    }

    if (IsZoom())
        pDev->DrawImage(aImagePos, aImageSize, *pImage, nStyle);
    else
        pDev->DrawImage(aImagePos, *pImage, nStyle);

    if (bDrawText)
    {
        const tools::Rectangle aTOutRect(aTextPos, aTextSize);
        ImplSetFocusRect(aTOutRect);
        DrawControlText(*pDev, aTOutRect, aText, nTextStyle, nullptr, nullptr, &aDeviceTextSize);
    }
    else
    {
        ImplSetFocusRect(tools::Rectangle(aImagePos, aImageSize));
    }
}

void Button::ImplSetFocusRect(const tools::Rectangle &rFocusRect)
{
    tools::Rectangle aFocusRect = rFocusRect;
    tools::Rectangle aOutputRect = tools::Rectangle(Point(), GetOutputSizePixel());

    if (!aFocusRect.IsEmpty())
    {
        aFocusRect.AdjustLeft( -1 );
        aFocusRect.AdjustTop( -1 );
        aFocusRect.AdjustRight( 1 );
        aFocusRect.AdjustBottom( 1 );
    }

    if (aFocusRect.Left()   < aOutputRect.Left())
        aFocusRect.SetLeft( aOutputRect.Left() );
    if (aFocusRect.Top()    < aOutputRect.Top())
        aFocusRect.SetTop( aOutputRect.Top() );
    if (aFocusRect.Right()  > aOutputRect.Right())
        aFocusRect.SetRight( aOutputRect.Right() );
    if (aFocusRect.Bottom() > aOutputRect.Bottom())
        aFocusRect.SetBottom( aOutputRect.Bottom() );

    mpButtonData->maFocusRect = aFocusRect;
}

const tools::Rectangle& Button::ImplGetFocusRect() const
{
    return mpButtonData->maFocusRect;
}

DrawButtonFlags& Button::ImplGetButtonState()
{
    return mpButtonData->mnButtonState;
}

DrawButtonFlags Button::ImplGetButtonState() const
{
    return mpButtonData->mnButtonState;
}

void Button::ImplSetSymbolAlign( SymbolAlign eAlign )
{
    if ( mpButtonData->meSymbolAlign != eAlign )
    {
        mpButtonData->meSymbolAlign = eAlign;
        StateChanged( StateChangedType::Data );
    }
}

void Button::SetSmallSymbol()
{
    mpButtonData->mbSmallSymbol = true;
}

void Button::EnableImageDisplay( bool bEnable )
{
    if( bEnable )
        mpButtonData->mnButtonState &= ~DrawButtonFlags::NoImage;
    else
        mpButtonData->mnButtonState |= DrawButtonFlags::NoImage;
}

void Button::EnableTextDisplay( bool bEnable )
{
    if( bEnable )
        mpButtonData->mnButtonState &= ~DrawButtonFlags::NoText;
    else
        mpButtonData->mnButtonState |= DrawButtonFlags::NoText;
}

bool Button::IsSmallSymbol () const
{
    return mpButtonData->mbSmallSymbol;
}

bool Button::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "image-position")
    {
        ImageAlign eAlign = ImageAlign::Left;
        if (rValue == "left")
            eAlign = ImageAlign::Left;
        else if (rValue == "right")
            eAlign = ImageAlign::Right;
        else if (rValue == "top")
            eAlign = ImageAlign::Top;
        else if (rValue == "bottom")
            eAlign = ImageAlign::Bottom;
        SetImageAlign(eAlign);
    }
    else if (rKey == "focus-on-click")
    {
        WinBits nBits = GetStyle();
        nBits &= ~WB_NOPOINTERFOCUS;
        if (!toBool(rValue))
            nBits |= WB_NOPOINTERFOCUS;
        SetStyle(nBits);
    }
    else
        return Control::set_property(rKey, rValue);
    return true;
}

void Button::statusChanged(const css::frame::FeatureStateEvent& rEvent)
{
    Enable(rEvent.IsEnabled);
}

FactoryFunction Button::GetUITestFactory() const
{
    return ButtonUIObject::create;
}

IMPL_STATIC_LINK( Button, dispatchCommandHandler, Button*, pButton, void )
{
    if (pButton == nullptr)
        return;

    comphelper::dispatchCommand(pButton->maCommand, uno::Sequence<beans::PropertyValue>());
}

void PushButton::ImplInitPushButtonData()
{
    mpWindowImpl->mbPushButton    = true;

    meSymbol        = SymbolType::DONTKNOW;
    meState         = TRISTATE_FALSE;
    mnDDStyle       = PushButtonDropdownStyle::NONE;
    mbIsActive    = false;
    mbPressed       = false;
    mbIsAction      = false;
}

namespace
{
    vcl::Window* getPreviousSibling(vcl::Window const *pParent)
    {
        return pParent ? pParent->GetWindow(GetWindowType::LastChild) : nullptr;
    }
}

void PushButton::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle(getPreviousSibling(pParent), nStyle);
    Button::ImplInit( pParent, nStyle, nullptr );

    if ( nStyle & WB_NOLIGHTBORDER )
        ImplGetButtonState() |= DrawButtonFlags::NoLightBorder;

    ImplInitSettings( true );
}

WinBits PushButton::ImplInitStyle( const vcl::Window* pPrevWindow, WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;

    // if no alignment is given, default to "vertically centered". This is because since
    // #i26046#, we respect the vertical alignment flags (previously we didn't completely),
    // but we of course want to look as before when no vertical alignment is specified
    if ( ( nStyle & ( WB_TOP | WB_VCENTER | WB_BOTTOM ) ) == 0 )
        nStyle |= WB_VCENTER;

    if ( !(nStyle & WB_NOGROUP) &&
         (!pPrevWindow ||
          ((pPrevWindow->GetType() != WindowType::PUSHBUTTON  ) &&
           (pPrevWindow->GetType() != WindowType::OKBUTTON    ) &&
           (pPrevWindow->GetType() != WindowType::CANCELBUTTON) &&
           (pPrevWindow->GetType() != WindowType::HELPBUTTON  )) ) )
        nStyle |= WB_GROUP;
    return nStyle;
}

const vcl::Font& PushButton::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetPushButtonFont();
}

const Color& PushButton::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetButtonTextColor();
}

void PushButton::ImplInitSettings( bool bBackground )
{
    Button::ImplInitSettings();

    if ( bBackground )
    {
        SetBackground();
        // #i38498#: do not check for GetParent()->IsChildTransparentModeEnabled()
        // otherwise the formcontrol button will be overdrawn due to ParentClipMode::NoClip
        // for radio and checkbox this is ok as they should appear transparent in documents
        if ( IsNativeControlSupported( ControlType::Pushbutton, ControlPart::Entire ) ||
             (GetStyle() & WB_FLATBUTTON) != 0 )
        {
            EnableChildTransparentMode();
            SetParentClipMode( ParentClipMode::NoClip );
            SetPaintTransparent( true );

            if ((GetStyle() & WB_FLATBUTTON) == 0)
                mpWindowImpl->mbUseNativeFocus = ImplGetSVData()->maNWFData.mbNoFocusRects;
            else
                mpWindowImpl->mbUseNativeFocus = ImplGetSVData()->maNWFData.mbNoFocusRectsForFlatButtons;
        }
        else
        {
            EnableChildTransparentMode( false );
            SetParentClipMode();
            SetPaintTransparent( false );
        }
    }
}

void PushButton::ImplDrawPushButtonFrame(vcl::RenderContext& rRenderContext,
                                         tools::Rectangle& rRect, DrawButtonFlags nStyle)
{
    if (!(GetStyle() & (WB_RECTSTYLE | WB_SMALLSTYLE)))
    {
        StyleSettings aStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        if (IsControlBackground())
            aStyleSettings.Set3DColors(GetControlBackground());
    }

    DecorationView aDecoView(&rRenderContext);
    if (IsControlBackground())
    {
        AllSettings aSettings = rRenderContext.GetSettings();
        AllSettings aOldSettings = aSettings;
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        aStyleSettings.Set3DColors(GetControlBackground());
        aSettings.SetStyleSettings(aStyleSettings);

        // Call OutputDevice::SetSettings() explicitly, as rRenderContext may
        // be a vcl::Window in fact, and vcl::Window::SetSettings() will call
        // Invalidate(), which is a problem, since we're in Paint().
        rRenderContext.OutputDevice::SetSettings(aSettings);
        rRect = aDecoView.DrawButton(rRect, nStyle);
        rRenderContext.OutputDevice::SetSettings(aOldSettings);
    }
    else
        rRect = aDecoView.DrawButton(rRect, nStyle);
}

bool PushButton::ImplHitTestPushButton( vcl::Window const * pDev,
                                        const Point& rPos )
{
    tools::Rectangle   aTestRect( Point(), pDev->GetOutputSizePixel() );

    return aTestRect.IsInside( rPos );
}

DrawTextFlags PushButton::ImplGetTextStyle( DrawFlags nDrawFlags ) const
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    DrawTextFlags nTextStyle = DrawTextFlags::Mnemonic | DrawTextFlags::MultiLine | DrawTextFlags::EndEllipsis;

    if ( ( rStyleSettings.GetOptions() & StyleSettingsOptions::Mono ) ||
         ( nDrawFlags & DrawFlags::Mono ) )
        nTextStyle |= DrawTextFlags::Mono;

    if ( GetStyle() & WB_WORDBREAK )
        nTextStyle |= DrawTextFlags::WordBreak;
    if ( GetStyle() & WB_NOLABEL )
        nTextStyle &= ~DrawTextFlags::Mnemonic;

    if ( GetStyle() & WB_LEFT )
        nTextStyle |= DrawTextFlags::Left;
    else if ( GetStyle() & WB_RIGHT )
        nTextStyle |= DrawTextFlags::Right;
    else
        nTextStyle |= DrawTextFlags::Center;

    if ( GetStyle() & WB_TOP )
        nTextStyle |= DrawTextFlags::Top;
    else if ( GetStyle() & WB_BOTTOM )
        nTextStyle |= DrawTextFlags::Bottom;
    else
        nTextStyle |= DrawTextFlags::VCenter;

    if ( !IsEnabled() )
        nTextStyle |= DrawTextFlags::Disable;

    return nTextStyle;
}

static void ImplDrawBtnDropDownArrow( OutputDevice* pDev,
                                      long nX, long nY,
                                      Color const & rColor, bool bBlack )
{
    Color aOldLineColor = pDev->GetLineColor();
    Color aOldFillColor = pDev->GetFillColor();

    pDev->SetLineColor();
    if ( bBlack )
        pDev->SetFillColor( COL_BLACK );
    else
        pDev->SetFillColor( rColor );
    pDev->DrawRect( tools::Rectangle( nX+0, nY+0, nX+6, nY+0 ) );
    pDev->DrawRect( tools::Rectangle( nX+1, nY+1, nX+5, nY+1 ) );
    pDev->DrawRect( tools::Rectangle( nX+2, nY+2, nX+4, nY+2 ) );
    pDev->DrawRect( tools::Rectangle( nX+3, nY+3, nX+3, nY+3 ) );
    if ( bBlack )
    {
        pDev->SetFillColor( rColor );
        pDev->DrawRect( tools::Rectangle( nX+2, nY+1, nX+4, nY+1 ) );
        pDev->DrawRect( tools::Rectangle( nX+3, nY+2, nX+3, nY+2 ) );
    }
    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

void PushButton::ImplDrawPushButtonContent(OutputDevice* pDev, DrawFlags nDrawFlags,
                                           const tools::Rectangle& rRect, bool bMenuBtnSep,
                                           DrawButtonFlags nButtonFlags)
{
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    tools::Rectangle               aInRect = rRect;
    Color                   aColor;
    DrawTextFlags           nTextStyle = ImplGetTextStyle( nDrawFlags );
    DrawSymbolFlags         nStyle;

    if( aInRect.Right() < aInRect.Left() || aInRect.Bottom() < aInRect.Top() )
        aInRect.SetEmpty();

    pDev->Push( PushFlags::CLIPREGION );
    pDev->IntersectClipRegion( aInRect );

    if ( nDrawFlags & DrawFlags::Mono )
        aColor = COL_BLACK;
    else if( (nButtonFlags & DrawButtonFlags::Highlight) && IsNativeControlSupported(ControlType::Pushbutton, ControlPart::Entire) )
    {
        if (nButtonFlags & DrawButtonFlags::Pressed)
            aColor = rStyleSettings.GetButtonPressedRolloverTextColor();
        else
            aColor = rStyleSettings.GetButtonRolloverTextColor();
    }
    else if ( IsControlForeground() )
        aColor = GetControlForeground();
    else if( nButtonFlags & DrawButtonFlags::Highlight )
    {
        if (nButtonFlags & DrawButtonFlags::Pressed)
            aColor = rStyleSettings.GetButtonPressedRolloverTextColor();
        else
            aColor = rStyleSettings.GetButtonRolloverTextColor();
    }
    else
        aColor = rStyleSettings.GetButtonTextColor();

    pDev->SetTextColor( aColor );

    if ( IsEnabled() )
        nStyle = DrawSymbolFlags::NONE;
    else
        nStyle = DrawSymbolFlags::Disable;

    Size aSize = rRect.GetSize();
    Point aPos = rRect.TopLeft();

    sal_uLong nImageSep = 1 + (pDev->GetTextHeight()-10)/2;
    if( nImageSep < 1 )
        nImageSep = 1;
    if ( mnDDStyle == PushButtonDropdownStyle::MenuButton ||
         mnDDStyle == PushButtonDropdownStyle::SplitMenuButton )
    {
        long nSeparatorX = 0;
        tools::Rectangle aSymbolRect = aInRect;
        if (!(ImplGetButtonState() & DrawButtonFlags::NoText))
        {
            // calculate symbol size
            long nSymbolSize    = pDev->GetTextHeight() / 2 + 1;

            nSeparatorX = aInRect.Right() - 2*nSymbolSize;
            aSize.AdjustWidth( -(2*nSymbolSize) );

            // center symbol rectangle in the separated area
            aSymbolRect.AdjustRight( -(nSymbolSize/2) );
            aSymbolRect.SetLeft( aSymbolRect.Right() - nSymbolSize );

            ImplDrawAlignedImage( pDev, aPos, aSize, nImageSep,
                                  nTextStyle, nullptr, true );
        }
        else
            ImplCalcSymbolRect( aSymbolRect );

        long nDistance = (aSymbolRect.GetHeight() > 10) ? 2 : 1;
        DecorationView aDecoView( pDev );
        if( bMenuBtnSep && nSeparatorX > 0 )
        {
            Point aStartPt( nSeparatorX, aSymbolRect.Top()+nDistance );
            Point aEndPt( nSeparatorX, aSymbolRect.Bottom()-nDistance );
            aDecoView.DrawSeparator( aStartPt, aEndPt );
        }
        ImplSetSeparatorX( nSeparatorX );

        aDecoView.DrawSymbol( aSymbolRect, SymbolType::SPIN_DOWN, aColor, nStyle );

    }
    else
    {
        tools::Rectangle aSymbolRect;
        ImplDrawAlignedImage( pDev, aPos, aSize, nImageSep,
                              nTextStyle, IsSymbol() ? &aSymbolRect : nullptr, true );

        if ( IsSymbol() )
        {
            DecorationView aDecoView( pDev );
            aDecoView.DrawSymbol( aSymbolRect, meSymbol, aColor, nStyle );
        }

        if ( mnDDStyle == PushButtonDropdownStyle::Toolbox )
        {
            bool bBlack = false;
            Color   aArrowColor( COL_BLACK );

            if ( !(nDrawFlags & DrawFlags::Mono) )
            {
                if ( !IsEnabled() )
                    aArrowColor = rStyleSettings.GetShadowColor();
                else
                {
                    aArrowColor = COL_LIGHTGREEN;
                    bBlack = true;
                }
            }

            ImplDrawBtnDropDownArrow( pDev, aInRect.Right()-6, aInRect.Top()+1,
                                      aArrowColor, bBlack );
        }
    }

    pDev->Pop();  // restore clipregion
}

void PushButton::ImplDrawPushButton(vcl::RenderContext& rRenderContext)
{
    HideFocus();

    DrawButtonFlags nButtonStyle = ImplGetButtonState();
    Size aOutSz(GetOutputSizePixel());
    tools::Rectangle aRect(Point(), aOutSz);
    tools::Rectangle aInRect = aRect;
    bool bNativeOK = false;

    // adjust style if button should be rendered 'pressed'
    if (mbPressed || mbIsActive)
        nButtonStyle |= DrawButtonFlags::Pressed;

    // TODO: move this to Window class or make it a member !!!
    ControlType aCtrlType = ControlType::Generic;
    switch(GetParent()->GetType())
    {
        case WindowType::LISTBOX:
        case WindowType::MULTILISTBOX:
        case WindowType::TREELISTBOX:
            aCtrlType = ControlType::Listbox;
            break;

        case WindowType::COMBOBOX:
        case WindowType::PATTERNBOX:
        case WindowType::NUMERICBOX:
        case WindowType::METRICBOX:
        case WindowType::CURRENCYBOX:
        case WindowType::DATEBOX:
        case WindowType::TIMEBOX:
        case WindowType::LONGCURRENCYBOX:
            aCtrlType = ControlType::Combobox;
            break;
        default:
            break;
    }

    bool bDropDown = (IsSymbol() && (GetSymbol() == SymbolType::SPIN_DOWN) && GetText().isEmpty());

    if( bDropDown && (aCtrlType == ControlType::Combobox || aCtrlType == ControlType::Listbox))
    {
        if (GetParent()->IsNativeControlSupported(aCtrlType, ControlPart::Entire))
        {
            // skip painting if the button was already drawn by the theme
            if (aCtrlType == ControlType::Combobox)
            {
                Edit* pEdit = static_cast<Edit*>(GetParent());
                if (pEdit->ImplUseNativeBorder(rRenderContext, pEdit->GetStyle()))
                    bNativeOK = true;
            }
            else if (GetParent()->IsNativeControlSupported(aCtrlType, ControlPart::HasBackgroundTexture))
            {
                bNativeOK = true;
            }

            if (!bNativeOK && GetParent()->IsNativeControlSupported(aCtrlType, ControlPart::ButtonDown))
            {
                // let the theme draw it, note we then need support
                // for ControlType::Listbox/ControlPart::ButtonDown and ControlType::Combobox/ControlPart::ButtonDown

                ImplControlValue aControlValue;
                ControlState nState = ControlState::NONE;

                if (mbPressed || mbIsActive)
                    nState |= ControlState::PRESSED;
                if (ImplGetButtonState() & DrawButtonFlags::Pressed)
                    nState |= ControlState::PRESSED;
                if (HasFocus())
                    nState |= ControlState::FOCUSED;
                if (ImplGetButtonState() & DrawButtonFlags::Default)
                    nState |= ControlState::DEFAULT;
                if (Window::IsEnabled())
                    nState |= ControlState::ENABLED;

                if (IsMouseOver() && aInRect.IsInside(GetPointerPosPixel()))
                    nState |= ControlState::ROLLOVER;

                if ( IsMouseOver() && aInRect.IsInside(GetPointerPosPixel()) && mbIsActive)
                {
                    nState |= ControlState::ROLLOVER;
                    nButtonStyle &= ~DrawButtonFlags::Pressed;
                }

                bNativeOK = rRenderContext.DrawNativeControl(aCtrlType, ControlPart::ButtonDown, aInRect, nState,
                                                             aControlValue, OUString());
            }
        }
    }

    if (bNativeOK)
        return;

    bool bRollOver = (IsMouseOver() && aInRect.IsInside(GetPointerPosPixel()));
    if (bRollOver)
        nButtonStyle |= DrawButtonFlags::Highlight;
    bool bDrawMenuSep = mnDDStyle == PushButtonDropdownStyle::SplitMenuButton;
    if (GetStyle() & WB_FLATBUTTON)
    {
        if (!bRollOver && !HasFocus())
            bDrawMenuSep = false;
    }
    bNativeOK = rRenderContext.IsNativeControlSupported(ControlType::Pushbutton, ControlPart::Entire);
    if (bNativeOK)
    {
        PushButtonValue aControlValue;
        aControlValue.mbIsAction = isAction();

        tools::Rectangle aCtrlRegion(aInRect);
        ControlState nState = ControlState::NONE;

        if (mbPressed || IsChecked() || mbIsActive)
        {
            nState |= ControlState::PRESSED;
            nButtonStyle |= DrawButtonFlags::Pressed;
        }
        if (ImplGetButtonState() & DrawButtonFlags::Pressed)
            nState |= ControlState::PRESSED;
        if (HasFocus())
            nState |= ControlState::FOCUSED;
        if (ImplGetButtonState() & DrawButtonFlags::Default)
            nState |= ControlState::DEFAULT;
        if (Window::IsEnabled())
            nState |= ControlState::ENABLED;

        if (bRollOver || mbIsActive)
        {
            nButtonStyle |= DrawButtonFlags::Highlight;
            nState |= ControlState::ROLLOVER;
        }

        if (mbIsActive && bRollOver)
        {
            nState &= ~ControlState::PRESSED;
            nButtonStyle &= ~DrawButtonFlags::Pressed;
        }

        if (GetStyle() & WB_BEVELBUTTON)
            aControlValue.mbBevelButton = true;

        // draw frame into invisible window to have aInRect modified correctly
        // but do not shift the inner rect for pressed buttons (ie remove DrawButtonFlags::Pressed)
        // this assumes the theme has enough visual cues to signalize the button was pressed
        //Window aWin( this );
        //ImplDrawPushButtonFrame( &aWin, aInRect, nButtonStyle & ~DrawButtonFlags::Pressed );

        // looks better this way as symbols were displaced slightly using the above approach
        aInRect.AdjustTop(4 );
        aInRect.AdjustBottom( -4 );
        aInRect.AdjustLeft(4 );
        aInRect.AdjustRight( -4 );

        // prepare single line hint (needed on mac to decide between normal push button and
        // rectangular bevel button look)
        Size aFontSize(Application::GetSettings().GetStyleSettings().GetPushButtonFont().GetFontSize());
        aFontSize = rRenderContext.LogicToPixel(aFontSize, MapMode(MapUnit::MapPoint));
        Size aInRectSize(rRenderContext.LogicToPixel(Size(aInRect.GetWidth(), aInRect.GetHeight())));
        aControlValue.mbSingleLine = (aInRectSize.Height() < 2 * aFontSize.Height());

        if ((nState & ControlState::ROLLOVER) || !(GetStyle() & WB_FLATBUTTON))
        {
            bNativeOK = rRenderContext.DrawNativeControl(ControlType::Pushbutton, ControlPart::Entire, aCtrlRegion, nState,
                                                         aControlValue, OUString() /*PushButton::GetText()*/);
        }
        else
        {
            bNativeOK = true;
        }

        // draw content using the same aInRect as non-native VCL would do
        ImplDrawPushButtonContent(&rRenderContext, DrawFlags::NONE,
                                  aInRect, bDrawMenuSep, nButtonStyle);

        if (HasFocus())
            ShowFocus(ImplGetFocusRect());
    }

    if (!bNativeOK)
    {
        // draw PushButtonFrame, aInRect has content size afterwards
        if (GetStyle() & WB_FLATBUTTON)
        {
            tools::Rectangle aTempRect(aInRect);
            if (bRollOver)
                ImplDrawPushButtonFrame(rRenderContext, aTempRect, nButtonStyle);
            aInRect.AdjustLeft(2 );
            aInRect.AdjustTop(2 );
            aInRect.AdjustRight( -2 );
            aInRect.AdjustBottom( -2 );
        }
        else
        {
            ImplDrawPushButtonFrame(rRenderContext, aInRect, nButtonStyle);
        }

        // draw content
        ImplDrawPushButtonContent(&rRenderContext, DrawFlags::NONE, aInRect, bDrawMenuSep, nButtonStyle);

        if (HasFocus())
        {
            ShowFocus(ImplGetFocusRect());
        }
    }
}

void PushButton::ImplSetDefButton( bool bSet )
{
    Size aSize( GetSizePixel() );
    Point aPos( GetPosPixel() );
    int dLeft(0), dRight(0), dTop(0), dBottom(0);
    bool bSetPos = false;

    if ( IsNativeControlSupported(ControlType::Pushbutton, ControlPart::Entire) )
    {
        tools::Rectangle aBound, aCont;
        tools::Rectangle aCtrlRegion( 0, 0, 80, 20 ); // use a constant size to avoid accumulating
                                             // will not work if the theme has dynamic adornment sizes
        ImplControlValue aControlValue;

        // get native size of a 'default' button
        // and adjust the VCL button if more space for adornment is required
        if( GetNativeControlRegion( ControlType::Pushbutton, ControlPart::Entire, aCtrlRegion,
                                    ControlState::DEFAULT|ControlState::ENABLED,
                                    aControlValue,
                                    aBound, aCont ) )
        {
            dLeft = aCont.Left() - aBound.Left();
            dTop = aCont.Top() - aBound.Top();
            dRight = aBound.Right() - aCont.Right();
            dBottom = aBound.Bottom() - aCont.Bottom();
            bSetPos = dLeft || dTop || dRight || dBottom;
        }
    }

    if ( bSet )
    {
        if( !(ImplGetButtonState() & DrawButtonFlags::Default) && bSetPos )
        {
            // adjust pos/size when toggling from non-default to default
            aPos.Move(-dLeft, -dTop);
            aSize.AdjustWidth(dLeft + dRight );
            aSize.AdjustHeight(dTop + dBottom );
        }
        ImplGetButtonState() |= DrawButtonFlags::Default;
    }
    else
    {
        if( (ImplGetButtonState() & DrawButtonFlags::Default) && bSetPos )
        {
            // adjust pos/size when toggling from default to non-default
            aPos.Move(dLeft, dTop);
            aSize.AdjustWidth( -(dLeft + dRight) );
            aSize.AdjustHeight( -(dTop + dBottom) );
        }
        ImplGetButtonState() &= ~DrawButtonFlags::Default;
    }
    if( bSetPos )
        setPosSizePixel( aPos.X(), aPos.Y(), aSize.Width(), aSize.Height() );

    Invalidate();
}

bool PushButton::ImplIsDefButton() const
{
    return bool(ImplGetButtonState() & DrawButtonFlags::Default);
}

PushButton::PushButton( WindowType nType ) :
    Button( nType )
{
    ImplInitPushButtonData();
}

PushButton::PushButton( vcl::Window* pParent, WinBits nStyle ) :
    Button( WindowType::PUSHBUTTON )
{
    ImplInitPushButtonData();
    ImplInit( pParent, nStyle );
}

void PushButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() &&
         ImplHitTestPushButton( this, rMEvt.GetPosPixel() ) )
    {
        StartTrackingFlags nTrackFlags = StartTrackingFlags::NONE;

        if ( ( GetStyle() & WB_REPEAT ) &&
             ! ( GetStyle() & WB_TOGGLE ) )
            nTrackFlags |= StartTrackingFlags::ButtonRepeat;

        ImplGetButtonState() |= DrawButtonFlags::Pressed;
        Invalidate();
        StartTracking( nTrackFlags );

        if ( nTrackFlags & StartTrackingFlags::ButtonRepeat )
            Click();
    }
}

void PushButton::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( ImplGetButtonState() & DrawButtonFlags::Pressed )
        {
            if ( !(GetStyle() & WB_NOPOINTERFOCUS) && !rTEvt.IsTrackingCanceled() )
                GrabFocus();

            if ( GetStyle() & WB_TOGGLE )
            {
                // Don't toggle, when aborted
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( IsChecked() )
                    {
                        Check( false );
                        ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
                    }
                    else
                        Check();
                }
            }
            else
                ImplGetButtonState() &= ~DrawButtonFlags::Pressed;

            Invalidate();

            // do not call Click handler if aborted
            if ( !rTEvt.IsTrackingCanceled() )
            {
                if ( ! ( ( GetStyle() & WB_REPEAT ) &&
                         ! ( GetStyle() & WB_TOGGLE ) ) )
                    Click();
            }
        }
    }
    else
    {
        if ( ImplHitTestPushButton( this, rTEvt.GetMouseEvent().GetPosPixel() ) )
        {
            if ( ImplGetButtonState() & DrawButtonFlags::Pressed )
            {
                if ( rTEvt.IsTrackingRepeat() && (GetStyle() & WB_REPEAT) &&
                     ! ( GetStyle() & WB_TOGGLE ) )
                    Click();
            }
            else
            {
                ImplGetButtonState() |= DrawButtonFlags::Pressed;
                Invalidate();
            }
        }
        else
        {
            if ( ImplGetButtonState() & DrawButtonFlags::Pressed )
            {
                ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
                Invalidate();
            }
        }
    }
}

void PushButton::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( !aKeyCode.GetModifier() &&
         ((aKeyCode.GetCode() == KEY_RETURN) || (aKeyCode.GetCode() == KEY_SPACE)) )
    {
        if ( !(ImplGetButtonState() & DrawButtonFlags::Pressed) )
        {
            ImplGetButtonState() |= DrawButtonFlags::Pressed;
            Invalidate();
        }

        if ( ( GetStyle() & WB_REPEAT ) &&
             ! ( GetStyle() & WB_TOGGLE ) )
            Click();
    }
    else if ( (ImplGetButtonState() & DrawButtonFlags::Pressed) && (aKeyCode.GetCode() == KEY_ESCAPE) )
    {
        ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
        Invalidate();
    }
    else
        Button::KeyInput( rKEvt );
}

void PushButton::KeyUp( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( (ImplGetButtonState() & DrawButtonFlags::Pressed) &&
         ((aKeyCode.GetCode() == KEY_RETURN) || (aKeyCode.GetCode() == KEY_SPACE)) )
    {
        if ( GetStyle() & WB_TOGGLE )
        {
            if ( IsChecked() )
            {
                Check( false );
                ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
            }
            else
                Check();

            Toggle();
        }
        else
            ImplGetButtonState() &= ~DrawButtonFlags::Pressed;

        Invalidate();

        if ( !( ( GetStyle() & WB_REPEAT )  &&
                ! ( GetStyle() & WB_TOGGLE ) ) )
            Click();
    }
    else
        Button::KeyUp( rKEvt );
}

void PushButton::FillLayoutData() const
{
    mpControlData->mpLayoutData.reset( new vcl::ControlLayoutData );
    const_cast<PushButton*>(this)->Invalidate();
}

void PushButton::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    ImplDrawPushButton(rRenderContext);
}

void PushButton::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                       DrawFlags nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    tools::Rectangle   aRect( aPos, aSize );
    vcl::Font   aFont = GetDrawPixelFont( pDev );

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    if ( nFlags & DrawFlags::Mono )
    {
        pDev->SetTextColor( COL_BLACK );
    }
    else
    {
        pDev->SetTextColor( GetTextColor() );

        // DecoView uses the FaceColor...
        AllSettings aSettings = pDev->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        if ( IsControlBackground() )
            aStyleSettings.SetFaceColor( GetControlBackground() );
        else
            aStyleSettings.SetFaceColor( GetSettings().GetStyleSettings().GetFaceColor() );
        aSettings.SetStyleSettings( aStyleSettings );
        pDev->OutputDevice::SetSettings( aSettings );
    }
    pDev->SetTextFillColor();

    DecorationView aDecoView( pDev );
    DrawButtonFlags nButtonStyle = DrawButtonFlags::NONE;
    if ( nFlags & DrawFlags::Mono )
        nButtonStyle |= DrawButtonFlags::Mono;
    if ( IsChecked() )
        nButtonStyle |= DrawButtonFlags::Checked;
    aRect = aDecoView.DrawButton( aRect, nButtonStyle );

    ImplDrawPushButtonContent( pDev, nFlags, aRect, true, nButtonStyle );
    pDev->Pop();
}

void PushButton::Resize()
{
    Control::Resize();
    Invalidate();
}

void PushButton::GetFocus()
{
    ShowFocus( ImplGetFocusRect() );
    SetInputContext( InputContext( GetFont() ) );
    Button::GetFocus();
}

void PushButton::LoseFocus()
{
    EndSelection();
    HideFocus();
    Button::LoseFocus();
}

void PushButton::StateChanged( StateChangedType nType )
{
    Button::StateChanged( nType );

    if ( (nType == StateChangedType::Enable) ||
         (nType == StateChangedType::Text) ||
         (nType == StateChangedType::Data) ||
         (nType == StateChangedType::State) ||
         (nType == StateChangedType::UpdateMode) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::Style )
    {
        SetStyle( ImplInitStyle( GetWindow( GetWindowType::Prev ), GetStyle() ) );

        bool bIsDefButton = ( GetStyle() & WB_DEFBUTTON ) != 0;
        bool bWasDefButton = ( GetPrevStyle() & WB_DEFBUTTON ) != 0;
        if ( bIsDefButton != bWasDefButton )
            ImplSetDefButton( bIsDefButton );

        if ( IsReallyVisible() && IsUpdateMode() )
        {
            if ( (GetPrevStyle() & PUSHBUTTON_VIEW_STYLE) !=
                 (GetStyle() & PUSHBUTTON_VIEW_STYLE) )
                Invalidate();
        }
    }
    else if ( (nType == StateChangedType::Zoom) ||
              (nType == StateChangedType::ControlFont) )
    {
        ImplInitSettings( false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        ImplInitSettings( false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings( true );
        Invalidate();
    }
}

void PushButton::DataChanged( const DataChangedEvent& rDCEvt )
{
    Button::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplInitSettings( true );
        Invalidate();
    }
}

bool PushButton::PreNotify( NotifyEvent& rNEvt )
{
    const MouseEvent* pMouseEvt = nullptr;

    if( (rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != nullptr )
    {
        if( pMouseEvt->IsEnterWindow() || pMouseEvt->IsLeaveWindow() )
        {
            // trigger redraw as mouse over state has changed

            // TODO: move this to Window class or make it a member !!!
            ControlType aCtrlType = ControlType::Generic;
            switch( GetParent()->GetType() )
            {
                case WindowType::LISTBOX:
                case WindowType::MULTILISTBOX:
                case WindowType::TREELISTBOX:
                    aCtrlType = ControlType::Listbox;
                    break;

                case WindowType::COMBOBOX:
                case WindowType::PATTERNBOX:
                case WindowType::NUMERICBOX:
                case WindowType::METRICBOX:
                case WindowType::CURRENCYBOX:
                case WindowType::DATEBOX:
                case WindowType::TIMEBOX:
                case WindowType::LONGCURRENCYBOX:
                    aCtrlType = ControlType::Combobox;
                    break;
                default:
                    break;
            }

            bool bDropDown = ( IsSymbol() && (GetSymbol()==SymbolType::SPIN_DOWN) && GetText().isEmpty() );

            if( bDropDown && GetParent()->IsNativeControlSupported( aCtrlType, ControlPart::Entire) &&
                   !GetParent()->IsNativeControlSupported( aCtrlType, ControlPart::ButtonDown) )
            {
                vcl::Window *pBorder = GetParent()->GetWindow( GetWindowType::Border );
                if(aCtrlType == ControlType::Combobox)
                {
                    // only paint the button part to avoid flickering of the combobox text
                    tools::Rectangle aClipRect( Point(), GetOutputSizePixel() );
                    aClipRect.SetPos(pBorder->ScreenToOutputPixel(OutputToScreenPixel(aClipRect.TopLeft())));
                    pBorder->Invalidate( aClipRect );
                }
                else
                {
                    pBorder->Invalidate( InvalidateFlags::NoErase );
                    pBorder->Update();
                }
            }
            else if( (GetStyle() & WB_FLATBUTTON) ||
                     IsNativeControlSupported(ControlType::Pushbutton, ControlPart::Entire) )
            {
                Invalidate();
            }
        }
    }

    return Button::PreNotify(rNEvt);
}

void PushButton::Toggle()
{
    ImplCallEventListenersAndHandler( VclEventId::PushbuttonToggle, nullptr );
}

void PushButton::SetSymbol( SymbolType eSymbol )
{
    if ( meSymbol != eSymbol )
    {
        meSymbol = eSymbol;
        CompatStateChanged( StateChangedType::Data );
    }
}

void PushButton::SetSymbolAlign( SymbolAlign eAlign )
{
    ImplSetSymbolAlign( eAlign );
}

void PushButton::SetDropDown( PushButtonDropdownStyle nStyle )
{
    if ( mnDDStyle != nStyle )
    {
        mnDDStyle = nStyle;
        CompatStateChanged( StateChangedType::Data );
    }
}

void PushButton::SetState( TriState eState )
{
    if ( meState != eState )
    {
        meState = eState;
        if ( meState == TRISTATE_FALSE )
            ImplGetButtonState() &= ~DrawButtonFlags(DrawButtonFlags::Checked | DrawButtonFlags::DontKnow);
        else if ( meState == TRISTATE_TRUE )
        {
            ImplGetButtonState() &= ~DrawButtonFlags::DontKnow;
            ImplGetButtonState() |= DrawButtonFlags::Checked;
        }
        else // TRISTATE_INDET
        {
            ImplGetButtonState() &= ~DrawButtonFlags::Checked;
            ImplGetButtonState() |= DrawButtonFlags::DontKnow;
        }

        CompatStateChanged( StateChangedType::State );
        Toggle();
    }
}

void PushButton::statusChanged(const css::frame::FeatureStateEvent& rEvent)
{
    Button::statusChanged(rEvent);
    if (rEvent.State.has<bool>())
        SetPressed(rEvent.State.get<bool>());
}

void PushButton::SetPressed( bool bPressed )
{
    if ( mbPressed != bPressed )
    {
        mbPressed = bPressed;
        CompatStateChanged( StateChangedType::Data );
    }
}

void PushButton::EndSelection()
{
    EndTracking( TrackingEventFlags::Cancel );
    if ( !IsDisposed() &&
         ImplGetButtonState() & DrawButtonFlags::Pressed )
    {
        ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
        if ( !mbPressed )
            Invalidate();
    }
}

Size PushButton::CalcMinimumSize() const
{
    Size aSize;

    if ( IsSymbol() )
    {
        if ( IsSmallSymbol ())
            aSize = Size( 16, 12 );
        else
            aSize = Size( 26, 24 );
    }
    else if ( Button::HasImage() && ! (ImplGetButtonState() & DrawButtonFlags::NoImage) )
        aSize = GetModeImage().GetSizePixel();
    if( mnDDStyle == PushButtonDropdownStyle::MenuButton ||
        mnDDStyle == PushButtonDropdownStyle::SplitMenuButton )
    {
        long nSymbolSize = GetTextHeight() / 2 + 1;
        aSize.AdjustWidth(2*nSymbolSize );
    }
    if ( !PushButton::GetText().isEmpty() && ! (ImplGetButtonState() & DrawButtonFlags::NoText) )
    {
        Size textSize = GetTextRect( tools::Rectangle( Point(), Size( 0x7fffffff, 0x7fffffff ) ),
                                     PushButton::GetText(), ImplGetTextStyle( DrawFlags::NONE ) ).GetSize();
        aSize.AdjustWidth(textSize.Width() );
        aSize.setHeight( std::max( aSize.Height(), long( textSize.Height() * 1.15 ) ) );
    }

    // cf. ImplDrawPushButton ...
    if( (GetStyle() & WB_SMALLSTYLE) == 0 )
    {
        aSize.AdjustWidth(24 );
        aSize.AdjustHeight(12 );
    }

    return CalcWindowSize( aSize );
}

Size PushButton::GetOptimalSize() const
{
    return CalcMinimumSize();
}

bool PushButton::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "has-default")
    {
        WinBits nBits = GetStyle();
        nBits &= ~WB_DEFBUTTON;
        if (toBool(rValue))
            nBits |= WB_DEFBUTTON;
        SetStyle(nBits);
    }
    else
        return Button::set_property(rKey, rValue);
    return true;
}

void PushButton::ShowFocus(const tools::Rectangle& rRect)
{
    if (IsNativeControlSupported(ControlType::Pushbutton, ControlPart::Focus))
    {
        PushButtonValue aControlValue;
        aControlValue.mbIsAction = isAction();
        tools::Rectangle aInRect(Point(), GetOutputSizePixel());
        GetOutDev()->DrawNativeControl(ControlType::Pushbutton, ControlPart::Focus, aInRect,
                                       ControlState::FOCUSED, aControlValue, OUString());
    }
    Button::ShowFocus(rRect);
}

void OKButton::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    set_id("ok");
    PushButton::ImplInit( pParent, nStyle );

    SetText( Button::GetStandardText( StandardButtonType::OK ) );
}

OKButton::OKButton( vcl::Window* pParent, WinBits nStyle ) :
    PushButton( WindowType::OKBUTTON )
{
    ImplInit( pParent, nStyle );
}

void OKButton::Click()
{
    // close parent if no link set
    if ( !GetClickHdl() )
    {
        vcl::Window* pParent = getNonLayoutParent(this);
        if ( pParent->IsSystemWindow() )
        {
            if ( pParent->IsDialog() )
            {
                if ( static_cast<Dialog*>(pParent)->IsInExecute() )
                    static_cast<Dialog*>(pParent)->EndDialog( RET_OK );
                // prevent recursive calls
                else if ( !static_cast<Dialog*>(pParent)->IsInClose() )
                {
                    if ( pParent->GetStyle() & WB_CLOSEABLE )
                        static_cast<Dialog*>(pParent)->Close();
                }
            }
            else
            {
                if ( pParent->GetStyle() & WB_CLOSEABLE )
                    static_cast<SystemWindow*>(pParent)->Close();
            }
        }
    }
    else
    {
        PushButton::Click();
    }
}

void CancelButton::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    set_id("cancel");
    PushButton::ImplInit( pParent, nStyle );

    SetText( Button::GetStandardText( StandardButtonType::Cancel ) );
}

CancelButton::CancelButton( vcl::Window* pParent, WinBits nStyle ) :
    PushButton( WindowType::CANCELBUTTON )
{
    ImplInit( pParent, nStyle );
}

void CancelButton::Click()
{
    // close parent if link not set
    if ( !GetClickHdl() )
    {
        vcl::Window* pParent = getNonLayoutParent(this);
        if ( pParent->IsSystemWindow() )
        {
            if ( pParent->IsDialog() )
            {
                if ( static_cast<Dialog*>(pParent)->IsInExecute() )
                    static_cast<Dialog*>(pParent)->EndDialog();
                // prevent recursive calls
                else if ( !static_cast<Dialog*>(pParent)->IsInClose() )
                {
                    if ( pParent->GetStyle() & WB_CLOSEABLE )
                        static_cast<Dialog*>(pParent)->Close();
                }
            }
            else
            {
                if ( pParent->GetStyle() & WB_CLOSEABLE )
                    static_cast<SystemWindow*>(pParent)->Close();
            }
        }
    }
    else
    {
        PushButton::Click();
    }
}

CloseButton::CloseButton( vcl::Window* pParent, WinBits nStyle )
    : CancelButton(pParent, nStyle)
{
    SetText( Button::GetStandardText( StandardButtonType::Close ) );
}

void HelpButton::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    set_id("help");
    PushButton::ImplInit( pParent, nStyle | WB_NOPOINTERFOCUS );

    SetText( Button::GetStandardText( StandardButtonType::Help ) );
}

HelpButton::HelpButton( vcl::Window* pParent, WinBits nStyle ) :
    PushButton( WindowType::HELPBUTTON )
{
    ImplInit( pParent, nStyle );
}

void HelpButton::Click()
{
    // trigger help if no link set
    if ( !GetClickHdl() )
    {
        vcl::Window* pFocusWin = Application::GetFocusWindow();
        if ( !pFocusWin )
            pFocusWin = this;

        HelpEvent aEvt( pFocusWin->GetPointerPosPixel(), HelpEventMode::CONTEXT );
        pFocusWin->RequestHelp( aEvt );
    }
    PushButton::Click();
}

void HelpButton::StateChanged( StateChangedType nStateChange )
{
    // Hide when we have no help URL.
    if (comphelper::LibreOfficeKit::isActive() &&
        officecfg::Office::Common::Help::HelpRootURL::get().isEmpty())
        Hide();
    else
        PushButton::StateChanged(nStateChange);
}

void RadioButton::ImplInitRadioButtonData()
{
    mbChecked       = false;
    mbSaveValue     = false;
    mbRadioCheck    = true;
    mbStateChanged  = false;
}

void RadioButton::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle(getPreviousSibling(pParent), nStyle);
    Button::ImplInit( pParent, nStyle, nullptr );

    ImplInitSettings( true );
}

WinBits RadioButton::ImplInitStyle( const vcl::Window* pPrevWindow, WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) &&
         (!pPrevWindow || (pPrevWindow->GetType() != WindowType::RADIOBUTTON)) )
        nStyle |= WB_GROUP;
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    return nStyle;
}

const vcl::Font& RadioButton::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetRadioCheckFont();
}

const Color& RadioButton::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetRadioCheckTextColor();
}

void RadioButton::ImplInitSettings( bool bBackground )
{
    Button::ImplInitSettings();

    if ( bBackground )
    {
        vcl::Window* pParent = GetParent();
        if ( !IsControlBackground() &&
            (pParent->IsChildTransparentModeEnabled() || IsNativeControlSupported( ControlType::Radiobutton, ControlPart::Entire ) ) )
        {
            EnableChildTransparentMode();
            SetParentClipMode( ParentClipMode::NoClip );
            SetPaintTransparent( true );
            SetBackground();
            if( IsNativeControlSupported( ControlType::Radiobutton, ControlPart::Entire ) )
                mpWindowImpl->mbUseNativeFocus = ImplGetSVData()->maNWFData.mbNoFocusRects;
        }
        else
        {
            EnableChildTransparentMode( false );
            SetParentClipMode();
            SetPaintTransparent( false );

            if ( IsControlBackground() )
                SetBackground( GetControlBackground() );
            else
                SetBackground( pParent->GetBackground() );
        }
    }
}

void RadioButton::ImplDrawRadioButtonState(vcl::RenderContext& rRenderContext)
{
    bool bNativeOK = false;

    // no native drawing for image radio buttons
    if (!maImage && rRenderContext.IsNativeControlSupported(ControlType::Radiobutton, ControlPart::Entire))
    {
        ImplControlValue aControlValue( mbChecked ? ButtonValue::On : ButtonValue::Off );
        tools::Rectangle aCtrlRect(maStateRect.TopLeft(), maStateRect.GetSize());
        ControlState nState = ControlState::NONE;

        if (ImplGetButtonState() & DrawButtonFlags::Pressed)
            nState |= ControlState::PRESSED;
        if (HasFocus())
            nState |= ControlState::FOCUSED;
        if (ImplGetButtonState() & DrawButtonFlags::Default)
            nState |= ControlState::DEFAULT;
        if (IsEnabled())
            nState |= ControlState::ENABLED;

        if (IsMouseOver() && maMouseRect.IsInside(GetPointerPosPixel()))
            nState |= ControlState::ROLLOVER;

        bNativeOK = rRenderContext.DrawNativeControl(ControlType::Radiobutton, ControlPart::Entire, aCtrlRect,
                                                     nState, aControlValue, OUString());
    }

    if (!bNativeOK)
    {
        if (!maImage)
        {
            DrawButtonFlags nStyle = ImplGetButtonState();
            if (!IsEnabled())
                nStyle |= DrawButtonFlags::Disabled;
            if (mbChecked)
                nStyle |= DrawButtonFlags::Checked;
            Image aImage = GetRadioImage(rRenderContext.GetSettings(), nStyle);
            if (IsZoom())
                rRenderContext.DrawImage(maStateRect.TopLeft(), maStateRect.GetSize(), aImage);
            else
                rRenderContext.DrawImage(maStateRect.TopLeft(), aImage);
        }
        else
        {
            HideFocus();

            DecorationView aDecoView(&rRenderContext);
            const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
            tools::Rectangle aImageRect  = maStateRect;
            Size aImageSize = maImage.GetSizePixel();
            bool bEnabled = IsEnabled();

            aImageSize.setWidth( CalcZoom(aImageSize.Width()) );
            aImageSize.setHeight( CalcZoom(aImageSize.Height()) );

            aImageRect.AdjustLeft( 1 );
            aImageRect.AdjustTop( 1 );
            aImageRect.AdjustRight( -1 );
            aImageRect.AdjustBottom( -1 );

            // display border and selection status
            aImageRect = aDecoView.DrawFrame(aImageRect, DrawFrameStyle::DoubleIn);
            if ((ImplGetButtonState() & DrawButtonFlags::Pressed) || !bEnabled)
                rRenderContext.SetFillColor( rStyleSettings.GetFaceColor());
            else
                rRenderContext.SetFillColor(rStyleSettings.GetFieldColor());
            rRenderContext.SetLineColor();
            rRenderContext.DrawRect(aImageRect);

            // display image
            DrawImageFlags nImageStyle = DrawImageFlags::NONE;
            if (!bEnabled)
                nImageStyle |= DrawImageFlags::Disable;

            Image* pImage = &maImage;

            Point aImagePos(aImageRect.TopLeft());
            aImagePos.AdjustX((aImageRect.GetWidth() - aImageSize.Width()) / 2 );
            aImagePos.AdjustY((aImageRect.GetHeight() - aImageSize.Height()) / 2 );
            if (IsZoom())
                rRenderContext.DrawImage(aImagePos, aImageSize, *pImage, nImageStyle);
            else
                rRenderContext.DrawImage(aImagePos, *pImage, nImageStyle);

            aImageRect.AdjustLeft( 1 );
            aImageRect.AdjustTop( 1 );
            aImageRect.AdjustRight( -1 );
            aImageRect.AdjustBottom( -1 );

            ImplSetFocusRect(aImageRect);

            if (mbChecked)
            {
                rRenderContext.SetLineColor(rStyleSettings.GetHighlightColor());
                rRenderContext.SetFillColor();
                if ((aImageSize.Width() >= 20) || (aImageSize.Height() >= 20))
                {
                    aImageRect.AdjustLeft( 1 );
                    aImageRect.AdjustTop( 1 );
                    aImageRect.AdjustRight( -1 );
                    aImageRect.AdjustBottom( -1 );
                }
                rRenderContext.DrawRect(aImageRect);
                aImageRect.AdjustLeft( 1 );
                aImageRect.AdjustTop( 1 );
                aImageRect.AdjustRight( -1 );
                aImageRect.AdjustBottom( -1 );
                rRenderContext.DrawRect(aImageRect);
            }

            if (HasFocus())
                ShowFocus(ImplGetFocusRect());
        }
    }
}

void RadioButton::ImplDraw( OutputDevice* pDev, DrawFlags nDrawFlags,
                            const Point& rPos, const Size& rSize,
                            const Size& rImageSize, tools::Rectangle& rStateRect,
                            tools::Rectangle& rMouseRect )
{
    WinBits                 nWinStyle = GetStyle();
    OUString                aText( GetText() );

    pDev->Push( PushFlags::CLIPREGION );
    pDev->IntersectClipRegion( tools::Rectangle( rPos, rSize ) );

    // no image radio button
    if ( !maImage )
    {
        if ( ( !aText.isEmpty() && ! (ImplGetButtonState() & DrawButtonFlags::NoText) ) ||
             ( HasImage() &&  ! (ImplGetButtonState() & DrawButtonFlags::NoImage) ) )
        {
            DrawTextFlags nTextStyle = Button::ImplGetTextStyle( nWinStyle, nDrawFlags );

            const long nImageSep = GetDrawPixel( pDev, ImplGetImageToTextDistance() );
            Size aSize( rSize );
            Point aPos( rPos );
            aPos.AdjustX(rImageSize.Width() + nImageSep );
            aSize.AdjustWidth( -(rImageSize.Width() + nImageSep) );

            // if the text rect height is smaller than the height of the image
            // then for single lines the default should be centered text
            if( (nWinStyle & (WB_TOP|WB_VCENTER|WB_BOTTOM)) == 0 &&
                (rImageSize.Height() > rSize.Height() || ! (nWinStyle & WB_WORDBREAK)  ) )
            {
                nTextStyle &= ~DrawTextFlags(DrawTextFlags::Top|DrawTextFlags::Bottom);
                nTextStyle |= DrawTextFlags::VCenter;
                aSize.setHeight( rImageSize.Height() );
            }

            ImplDrawAlignedImage( pDev, aPos, aSize, 1, nTextStyle );

            rMouseRect          = tools::Rectangle( aPos, aSize );
            rMouseRect.SetLeft( rPos.X() );

            rStateRect.SetLeft( rPos.X() );
            rStateRect.SetTop( rMouseRect.Top() );

            if ( aSize.Height() > rImageSize.Height() )
                rStateRect.AdjustTop(( aSize.Height() - rImageSize.Height() ) / 2 );
            else
            {
                rStateRect.AdjustTop( -(( rImageSize.Height() - aSize.Height() ) / 2) );
                if( rStateRect.Top() < 0 )
                    rStateRect.SetTop( 0 );
            }

            rStateRect.SetRight( rStateRect.Left() + rImageSize.Width()-1 );
            rStateRect.SetBottom( rStateRect.Top() + rImageSize.Height()-1 );

            if ( rStateRect.Bottom() > rMouseRect.Bottom() )
                rMouseRect.SetBottom( rStateRect.Bottom() );
        }
        else
        {
            rStateRect.SetLeft( rPos.X() );
            if ( nWinStyle & WB_VCENTER )
                rStateRect.SetTop( rPos.Y()+((rSize.Height()-rImageSize.Height())/2) );
            else if ( nWinStyle & WB_BOTTOM )
                rStateRect.SetTop( rPos.Y()+rSize.Height()-rImageSize.Height() ); //-1;
            else
                rStateRect.SetTop( rPos.Y() );
            rStateRect.SetRight( rStateRect.Left()+rImageSize.Width()-1 );
            rStateRect.SetBottom( rStateRect.Top()+rImageSize.Height()-1 );
            rMouseRect          = rStateRect;

            ImplSetFocusRect( rStateRect );
        }
    }
    else
    {
        bool        bTopImage   = (nWinStyle & WB_TOP) != 0;
        Size        aImageSize  = maImage.GetSizePixel();
        tools::Rectangle   aImageRect( rPos, rSize );
        long        nTextHeight = pDev->GetTextHeight();
        long        nTextWidth  = pDev->GetCtrlTextWidth( aText );

        // calculate position and sizes
        if ( !aText.isEmpty() && ! (ImplGetButtonState() & DrawButtonFlags::NoText) )
        {
            Size aTmpSize( (aImageSize.Width()+8), (aImageSize.Height()+8) );
            if ( bTopImage )
            {
                aImageRect.SetLeft( (rSize.Width()-aTmpSize.Width())/2 );
                aImageRect.SetTop( (rSize.Height()-(aTmpSize.Height()+nTextHeight+6))/2 );
            }
            else
                aImageRect.SetTop( (rSize.Height()-aTmpSize.Height())/2 );

            aImageRect.SetRight( aImageRect.Left()+aTmpSize.Width() );
            aImageRect.SetBottom( aImageRect.Top()+aTmpSize.Height() );

            // display text
            Point aTxtPos = rPos;
            if ( bTopImage )
            {
                aTxtPos.AdjustX((rSize.Width()-nTextWidth)/2 );
                aTxtPos.AdjustY(aImageRect.Bottom()+6 );
            }
            else
            {
                aTxtPos.AdjustX(aImageRect.Right()+8 );
                aTxtPos.AdjustY((rSize.Height()-nTextHeight)/2 );
            }
            pDev->DrawCtrlText( aTxtPos, aText, 0, aText.getLength() );
        }

        rMouseRect = aImageRect;
        rStateRect = aImageRect;
    }

    pDev->Pop();
}

void RadioButton::ImplDrawRadioButton(vcl::RenderContext& rRenderContext)
{
    HideFocus();

    Size aImageSize;
    if (!maImage)
        aImageSize = ImplGetRadioImageSize();
    else
        aImageSize  = maImage.GetSizePixel();

    aImageSize.setWidth( CalcZoom(aImageSize.Width()) );
    aImageSize.setHeight( CalcZoom(aImageSize.Height()) );

    // Draw control text
    ImplDraw(&rRenderContext, DrawFlags::NONE, Point(), GetOutputSizePixel(),
             aImageSize, maStateRect, maMouseRect);

    if (!maImage && HasFocus())
        ShowFocus(ImplGetFocusRect());

    ImplDrawRadioButtonState(rRenderContext);
}

void RadioButton::group(RadioButton &rOther)
{
    if (&rOther == this)
        return;

    if (!m_xGroup)
    {
        m_xGroup.reset(new std::vector<VclPtr<RadioButton> >);
        m_xGroup->push_back(this);
    }

    auto aFind = std::find(m_xGroup->begin(), m_xGroup->end(), VclPtr<RadioButton>(&rOther));
    if (aFind == m_xGroup->end())
    {
        m_xGroup->push_back(&rOther);

        if (rOther.m_xGroup)
        {
            std::vector< VclPtr<RadioButton> > aOthers(rOther.GetRadioButtonGroup(false));
            //make all members of the group share the same button group
            for (auto const& elem : aOthers)
            {
                aFind = std::find(m_xGroup->begin(), m_xGroup->end(), elem);
                if (aFind == m_xGroup->end())
                    m_xGroup->push_back(elem);
            }
        }

        //make all members of the group share the same button group
        for (VclPtr<RadioButton> const & pButton : *m_xGroup)
        {
            pButton->m_xGroup = m_xGroup;
        }
    }

    //if this one is checked, uncheck all the others
    if (mbChecked)
        ImplUncheckAllOther();
}

std::vector< VclPtr<RadioButton> > RadioButton::GetRadioButtonGroup(bool bIncludeThis) const
{
    if (m_xGroup)
    {
        if (bIncludeThis)
            return *m_xGroup;
        std::vector< VclPtr<RadioButton> > aGroup;
        for (VclPtr<RadioButton> const & pRadioButton : *m_xGroup)
        {
            if (pRadioButton == this)
                continue;
            aGroup.push_back(pRadioButton);
        }
        return aGroup;
    }

    //old-school

    // go back to first in group;
    vcl::Window* pFirst = const_cast<RadioButton*>(this);
    while( ( pFirst->GetStyle() & WB_GROUP ) == 0 )
    {
        vcl::Window* pWindow = pFirst->GetWindow( GetWindowType::Prev );
        if( pWindow )
            pFirst = pWindow;
        else
            break;
    }
    std::vector< VclPtr<RadioButton> > aGroup;
    // insert radiobuttons up to next group
    do
    {
        if( pFirst->GetType() == WindowType::RADIOBUTTON )
        {
            if( pFirst != this || bIncludeThis )
                aGroup.emplace_back(static_cast<RadioButton*>(pFirst) );
        }
        pFirst = pFirst->GetWindow( GetWindowType::Next );
    } while( pFirst && ( ( pFirst->GetStyle() & WB_GROUP ) == 0 ) );

    return aGroup;
}

void RadioButton::ImplUncheckAllOther()
{
    mpWindowImpl->mnStyle |= WB_TABSTOP;

    std::vector<VclPtr<RadioButton> > aGroup(GetRadioButtonGroup(false));
    // iterate over radio button group and checked buttons
    for (VclPtr<RadioButton>& pWindow : aGroup)
    {
        if ( pWindow->IsChecked() )
        {
            pWindow->SetState( false );
            if ( pWindow->IsDisposed() )
                return;
        }

        // not inside if clause to always remove wrongly set WB_TABSTOPS
        pWindow->mpWindowImpl->mnStyle &= ~WB_TABSTOP;
    }
}

void RadioButton::ImplCallClick( bool bGrabFocus, GetFocusFlags nFocusFlags )
{
    mbStateChanged = !mbChecked;
    mbChecked = true;
    mpWindowImpl->mnStyle |= WB_TABSTOP;
    Invalidate();
    Update();
    VclPtr<vcl::Window> xWindow = this;
    if ( mbRadioCheck )
        ImplUncheckAllOther();
    if ( xWindow->IsDisposed() )
        return;
    if ( bGrabFocus )
        ImplGrabFocus( nFocusFlags );
    if ( xWindow->IsDisposed() )
        return;
    if ( mbStateChanged )
        Toggle();
    if ( xWindow->IsDisposed() )
        return;
    Click();
    if ( xWindow->IsDisposed() )
        return;
    mbStateChanged = false;
}

RadioButton::RadioButton( vcl::Window* pParent, WinBits nStyle ) :
    Button( WindowType::RADIOBUTTON )
{
    ImplInitRadioButtonData();
    ImplInit( pParent, nStyle );
}

RadioButton::~RadioButton()
{
    disposeOnce();
}

void RadioButton::dispose()
{
    if (m_xGroup)
    {
        m_xGroup->erase(std::remove(m_xGroup->begin(), m_xGroup->end(), VclPtr<RadioButton>(this)),
                        m_xGroup->end());
        m_xGroup.reset();
    }
    Button::dispose();
}

void RadioButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && maMouseRect.IsInside( rMEvt.GetPosPixel() ) )
    {
        ImplGetButtonState() |= DrawButtonFlags::Pressed;
        Invalidate();
        Update();
        StartTracking();
        return;
    }

    Button::MouseButtonDown( rMEvt );
}

void RadioButton::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( ImplGetButtonState() & DrawButtonFlags::Pressed )
        {
            if ( !(GetStyle() & WB_NOPOINTERFOCUS) && !rTEvt.IsTrackingCanceled() )
                GrabFocus();

            ImplGetButtonState() &= ~DrawButtonFlags::Pressed;

            // do not call click handler if aborted
            if ( !rTEvt.IsTrackingCanceled() )
                ImplCallClick();
            else
            {
                Invalidate();
                Update();
            }
        }
    }
    else
    {
        if ( maMouseRect.IsInside( rTEvt.GetMouseEvent().GetPosPixel() ) )
        {
            if ( !(ImplGetButtonState() & DrawButtonFlags::Pressed) )
            {
                ImplGetButtonState() |= DrawButtonFlags::Pressed;
                Invalidate();
                Update();
            }
        }
        else
        {
            if ( ImplGetButtonState() & DrawButtonFlags::Pressed )
            {
                ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
                Invalidate();
                Update();
            }
        }
    }
}

void RadioButton::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( !aKeyCode.GetModifier() && (aKeyCode.GetCode() == KEY_SPACE) )
    {
        if ( !(ImplGetButtonState() & DrawButtonFlags::Pressed) )
        {
            ImplGetButtonState() |= DrawButtonFlags::Pressed;
            Invalidate();
            Update();
        }
    }
    else if ( (ImplGetButtonState() & DrawButtonFlags::Pressed) && (aKeyCode.GetCode() == KEY_ESCAPE) )
    {
        ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
        Invalidate();
        Update();
    }
    else
        Button::KeyInput( rKEvt );
}

void RadioButton::KeyUp( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( (ImplGetButtonState() & DrawButtonFlags::Pressed) && (aKeyCode.GetCode() == KEY_SPACE) )
    {
        ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
        ImplCallClick();
    }
    else
        Button::KeyUp( rKEvt );
}

void RadioButton::FillLayoutData() const
{
    mpControlData->mpLayoutData.reset( new vcl::ControlLayoutData );
    const_cast<RadioButton*>(this)->Invalidate();
}

void RadioButton::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& )
{
    ImplDrawRadioButton(rRenderContext);
}

void RadioButton::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                        DrawFlags nFlags )
{
    if ( !maImage )
    {
        MapMode     aResMapMode( MapUnit::Map100thMM );
        Point       aPos  = pDev->LogicToPixel( rPos );
        Size        aSize = pDev->LogicToPixel( rSize );
        Size        aImageSize = pDev->LogicToPixel( Size( 300, 300 ), aResMapMode );
        Size        aBrd1Size = pDev->LogicToPixel( Size( 20, 20 ), aResMapMode );
        Size        aBrd2Size = pDev->LogicToPixel( Size( 60, 60 ), aResMapMode );
        vcl::Font   aFont = GetDrawPixelFont( pDev );
        tools::Rectangle   aStateRect;
        tools::Rectangle   aMouseRect;

        aImageSize.setWidth( CalcZoom( aImageSize.Width() ) );
        aImageSize.setHeight( CalcZoom( aImageSize.Height() ) );
        aBrd1Size.setWidth( CalcZoom( aBrd1Size.Width() ) );
        aBrd1Size.setHeight( CalcZoom( aBrd1Size.Height() ) );
        aBrd2Size.setWidth( CalcZoom( aBrd2Size.Width() ) );
        aBrd2Size.setHeight( CalcZoom( aBrd2Size.Height() ) );

        if ( !aBrd1Size.Width() )
            aBrd1Size.setWidth( 1 );
        if ( !aBrd1Size.Height() )
            aBrd1Size.setHeight( 1 );
        if ( !aBrd2Size.Width() )
            aBrd2Size.setWidth( 1 );
        if ( !aBrd2Size.Height() )
            aBrd2Size.setHeight( 1 );

        pDev->Push();
        pDev->SetMapMode();
        pDev->SetFont( aFont );
        if ( nFlags & DrawFlags::Mono )
            pDev->SetTextColor( COL_BLACK );
        else
            pDev->SetTextColor( GetTextColor() );
        pDev->SetTextFillColor();

        ImplDraw( pDev, nFlags, aPos, aSize,
                  aImageSize, aStateRect, aMouseRect );

        Point   aCenterPos = aStateRect.Center();
        long    nRadX = aImageSize.Width()/2;
        long    nRadY = aImageSize.Height()/2;

        pDev->SetLineColor();
        pDev->SetFillColor( COL_BLACK );
        pDev->DrawPolygon( tools::Polygon( aCenterPos, nRadX, nRadY ) );
        nRadX -= aBrd1Size.Width();
        nRadY -= aBrd1Size.Height();
        pDev->SetFillColor( COL_WHITE );
        pDev->DrawPolygon( tools::Polygon( aCenterPos, nRadX, nRadY ) );
        if ( mbChecked )
        {
            nRadX -= aBrd1Size.Width();
            nRadY -= aBrd1Size.Height();
            if ( !nRadX )
                nRadX = 1;
            if ( !nRadY )
                nRadY = 1;
            pDev->SetFillColor( COL_BLACK );
            pDev->DrawPolygon( tools::Polygon( aCenterPos, nRadX, nRadY ) );
        }

        pDev->Pop();
    }
    else
    {
        OSL_FAIL( "RadioButton::Draw() - not implemented for RadioButton with Image" );
    }
}

void RadioButton::Resize()
{
    Control::Resize();
    Invalidate();
}

void RadioButton::GetFocus()
{
    ShowFocus( ImplGetFocusRect() );
    SetInputContext( InputContext( GetFont() ) );
    Button::GetFocus();
}

void RadioButton::LoseFocus()
{
    if ( ImplGetButtonState() & DrawButtonFlags::Pressed )
    {
        ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
        Invalidate();
        Update();
    }

    HideFocus();
    Button::LoseFocus();
}

void RadioButton::StateChanged( StateChangedType nType )
{
    Button::StateChanged( nType );

    if ( nType == StateChangedType::State )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate( maStateRect );
    }
    else if ( (nType == StateChangedType::Enable) ||
              (nType == StateChangedType::Text) ||
              (nType == StateChangedType::Data) ||
              (nType == StateChangedType::UpdateMode) )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::Style )
    {
        SetStyle( ImplInitStyle( GetWindow( GetWindowType::Prev ), GetStyle() ) );

        if ( (GetPrevStyle() & RADIOBUTTON_VIEW_STYLE) !=
             (GetStyle() & RADIOBUTTON_VIEW_STYLE) )
        {
            if ( IsUpdateMode() )
                Invalidate();
        }
    }
    else if ( (nType == StateChangedType::Zoom) ||
              (nType == StateChangedType::ControlFont) )
    {
        ImplInitSettings( false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        ImplInitSettings( false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings( true );
        Invalidate();
    }
}

void RadioButton::DataChanged( const DataChangedEvent& rDCEvt )
{
    Button::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplInitSettings( true );
        Invalidate();
    }
}

bool RadioButton::PreNotify( NotifyEvent& rNEvt )
{
    const MouseEvent* pMouseEvt = nullptr;

    if( (rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != nullptr )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // trigger redraw if mouse over state has changed
            if( IsNativeControlSupported(ControlType::Radiobutton, ControlPart::Entire) )
            {
                if (maMouseRect.IsInside(GetPointerPosPixel()) != maMouseRect.IsInside(GetLastPointerPosPixel()) ||
                    pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow())
                {
                    Invalidate( maStateRect );
                }
            }
        }
    }

    return Button::PreNotify(rNEvt);
}

void RadioButton::Toggle()
{
    ImplCallEventListenersAndHandler( VclEventId::RadiobuttonToggle, [this] () { maToggleHdl.Call(*this); } );
}

void RadioButton::SetModeRadioImage( const Image& rImage )
{
    if ( rImage != maImage )
    {
        maImage = rImage;
        CompatStateChanged( StateChangedType::Data );
        queue_resize();
    }
}


void RadioButton::SetState( bool bCheck )
{
    // carry the TabStop flag along correctly
    if ( bCheck )
        mpWindowImpl->mnStyle |= WB_TABSTOP;
    else
        mpWindowImpl->mnStyle &= ~WB_TABSTOP;

    if ( mbChecked != bCheck )
    {
        mbChecked = bCheck;
        CompatStateChanged( StateChangedType::State );
        Toggle();
    }
}

bool RadioButton::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "active")
        SetState(toBool(rValue));
    else if (rKey == "image-position")
    {
        WinBits nBits = GetStyle();
        if (rValue == "left")
        {
            nBits &= ~(WB_CENTER | WB_RIGHT);
            nBits |= WB_LEFT;
        }
        else if (rValue == "right")
        {
            nBits &= ~(WB_CENTER | WB_LEFT);
            nBits |= WB_RIGHT;
        }
        else if (rValue == "top")
        {
            nBits &= ~(WB_VCENTER | WB_BOTTOM);
            nBits |= WB_TOP;
        }
        else if (rValue == "bottom")
        {
            nBits &= ~(WB_VCENTER | WB_TOP);
            nBits |= WB_BOTTOM;
        }
        //Its rather mad to have to set these bits when there is the other
        //image align. Looks like e.g. the radiobuttons etc weren't converted
        //over to image align fully.
        SetStyle(nBits);
        //Deliberate to set the sane ImageAlign property
        return Button::set_property(rKey, rValue);
    }
    else
        return Button::set_property(rKey, rValue);
    return true;
}

void RadioButton::Check( bool bCheck )
{
    // TabStop-Flag richtig mitfuehren
    if ( bCheck )
        mpWindowImpl->mnStyle |= WB_TABSTOP;
    else
        mpWindowImpl->mnStyle &= ~WB_TABSTOP;

    if ( mbChecked != bCheck )
    {
        mbChecked = bCheck;
        VclPtr<vcl::Window> xWindow = this;
        CompatStateChanged( StateChangedType::State );
        if ( xWindow->IsDisposed() )
            return;
        if ( bCheck && mbRadioCheck )
            ImplUncheckAllOther();
        if ( xWindow->IsDisposed() )
            return;
        Toggle();
    }
}

long RadioButton::ImplGetImageToTextDistance() const
{
    // 4 pixels, but take zoom into account, so the text doesn't "jump" relative to surrounding elements,
    // which might have been aligned with the text of the check box
    return CalcZoom( 4 );
}

Size RadioButton::ImplGetRadioImageSize() const
{
    Size aSize;
    bool bDefaultSize = true;
    if( IsNativeControlSupported( ControlType::Radiobutton, ControlPart::Entire ) )
    {
        ImplControlValue aControlValue;
        tools::Rectangle aCtrlRegion( Point( 0, 0 ), GetSizePixel() );
        tools::Rectangle aBoundingRgn, aContentRgn;

        // get native size of a radio button
        if( GetNativeControlRegion( ControlType::Radiobutton, ControlPart::Entire, aCtrlRegion,
                                    ControlState::DEFAULT|ControlState::ENABLED,
                                    aControlValue,
                                    aBoundingRgn, aContentRgn ) )
        {
            aSize = aContentRgn.GetSize();
            bDefaultSize = false;
        }
    }
    if( bDefaultSize )
        aSize = GetRadioImage( GetSettings(), DrawButtonFlags::NONE ).GetSizePixel();
    return aSize;
}

static void LoadThemedImageList(const StyleSettings &rStyleSettings,
                                std::vector<Image>& rList, const std::vector<OUString> &rResources)
{
    Color aColorAry1[6];
    Color aColorAry2[6];
    aColorAry1[0] = Color( 0xC0, 0xC0, 0xC0 );
    aColorAry1[1] = Color( 0xFF, 0xFF, 0x00 );
    aColorAry1[2] = Color( 0xFF, 0xFF, 0xFF );
    aColorAry1[3] = Color( 0x80, 0x80, 0x80 );
    aColorAry1[4] = Color( 0x00, 0x00, 0x00 );
    aColorAry1[5] = Color( 0x00, 0xFF, 0x00 );
    aColorAry2[0] = rStyleSettings.GetFaceColor();
    aColorAry2[1] = rStyleSettings.GetWindowColor();
    aColorAry2[2] = rStyleSettings.GetLightColor();
    aColorAry2[3] = rStyleSettings.GetShadowColor();
    aColorAry2[4] = rStyleSettings.GetDarkShadowColor();
    aColorAry2[5] = rStyleSettings.GetWindowTextColor();

    static_assert( sizeof(aColorAry1) == sizeof(aColorAry2), "aColorAry1 must match aColorAry2" );

    for (const auto &a : rResources)
    {
        BitmapEx aBmpEx(a);
        aBmpEx.Replace(aColorAry1, aColorAry2, SAL_N_ELEMENTS(aColorAry1));
        rList.emplace_back(aBmpEx);
    }
}

Image RadioButton::GetRadioImage( const AllSettings& rSettings, DrawButtonFlags nFlags )
{
    ImplSVData*             pSVData = ImplGetSVData();
    const StyleSettings&    rStyleSettings = rSettings.GetStyleSettings();
    sal_uInt16              nStyle = 0;

    if ( rStyleSettings.GetOptions() & StyleSettingsOptions::Mono )
        nStyle = STYLE_RADIOBUTTON_MONO;

    if ( pSVData->maCtrlData.maRadioImgList.empty() ||
         (pSVData->maCtrlData.mnRadioStyle != nStyle) ||
         (pSVData->maCtrlData.mnLastRadioFColor != rStyleSettings.GetFaceColor()) ||
         (pSVData->maCtrlData.mnLastRadioWColor != rStyleSettings.GetWindowColor()) ||
         (pSVData->maCtrlData.mnLastRadioLColor != rStyleSettings.GetLightColor()) )
    {
        pSVData->maCtrlData.maRadioImgList.clear();

        pSVData->maCtrlData.mnLastRadioFColor = rStyleSettings.GetFaceColor();
        pSVData->maCtrlData.mnLastRadioWColor = rStyleSettings.GetWindowColor();
        pSVData->maCtrlData.mnLastRadioLColor = rStyleSettings.GetLightColor();

        std::vector<OUString> aResources;
        if (nStyle)
        {
            aResources.emplace_back(SV_RESID_BITMAP_RADIOMONO1);
            aResources.emplace_back(SV_RESID_BITMAP_RADIOMONO2);
            aResources.emplace_back(SV_RESID_BITMAP_RADIOMONO3);
            aResources.emplace_back(SV_RESID_BITMAP_RADIOMONO4);
            aResources.emplace_back(SV_RESID_BITMAP_RADIOMONO5);
            aResources.emplace_back(SV_RESID_BITMAP_RADIOMONO6);
        }
        else
        {
            aResources.emplace_back(SV_RESID_BITMAP_RADIO1);
            aResources.emplace_back(SV_RESID_BITMAP_RADIO2);
            aResources.emplace_back(SV_RESID_BITMAP_RADIO3);
            aResources.emplace_back(SV_RESID_BITMAP_RADIO4);
            aResources.emplace_back(SV_RESID_BITMAP_RADIO5);
            aResources.emplace_back(SV_RESID_BITMAP_RADIO6);
        }
        LoadThemedImageList( rStyleSettings, pSVData->maCtrlData.maRadioImgList, aResources);
        pSVData->maCtrlData.mnRadioStyle = nStyle;
    }

    sal_uInt16 nIndex;
    if ( nFlags & DrawButtonFlags::Disabled )
    {
        if ( nFlags & DrawButtonFlags::Checked )
            nIndex = 5;
        else
            nIndex = 4;
    }
    else if ( nFlags & DrawButtonFlags::Pressed )
    {
        if ( nFlags & DrawButtonFlags::Checked )
            nIndex = 3;
        else
            nIndex = 2;
    }
    else
    {
        if ( nFlags & DrawButtonFlags::Checked )
            nIndex = 1;
        else
            nIndex = 0;
    }
    return pSVData->maCtrlData.maRadioImgList[nIndex];
}

void RadioButton::ImplSetMinimumNWFSize()
{
    Push( PushFlags::MAPMODE );
    SetMapMode(MapMode(MapUnit::MapPixel));

    ImplControlValue aControlValue;
    Size aCurSize( GetSizePixel() );
    tools::Rectangle aCtrlRegion( Point( 0, 0 ), aCurSize );
    tools::Rectangle aBoundingRgn, aContentRgn;

    // get native size of a radiobutton
    if( GetNativeControlRegion( ControlType::Radiobutton, ControlPart::Entire, aCtrlRegion,
                                ControlState::DEFAULT|ControlState::ENABLED, aControlValue,
                                aBoundingRgn, aContentRgn ) )
    {
        Size aSize = aContentRgn.GetSize();

        if( aSize.Height() > aCurSize.Height() )
        {
            aCurSize.setHeight( aSize.Height() );
            SetSizePixel( aCurSize );
        }
    }

    Pop();
}

Size RadioButton::CalcMinimumSize() const
{
    Size aSize;
    if ( !maImage )
        aSize = ImplGetRadioImageSize();
    else
    {
        aSize = maImage.GetSizePixel();
        aSize.AdjustWidth(8);
        aSize.AdjustHeight(8);
    }

    if (Button::HasImage() && !(ImplGetButtonState() & DrawButtonFlags::NoImage))
    {
        Size aImgSize = GetModeImage().GetSizePixel();
        aSize = Size(std::max(aImgSize.Width(), aSize.Width()),
                     std::max(aImgSize.Height(), aSize.Height()));
    }

    OUString aText = GetText();
    if ( !aText.isEmpty() && ! (ImplGetButtonState() & DrawButtonFlags::NoText) )
    {
        bool bTopImage = (GetStyle() & WB_TOP) != 0;

        Size aTextSize = GetTextRect( tools::Rectangle( Point(), Size( 0x7fffffff, 0x7fffffff ) ),
                                      aText, FixedText::ImplGetTextStyle( GetStyle() ) ).GetSize();

        aSize.AdjustWidth(2 );   // for focus rect

        if (!bTopImage)
        {
            aSize.AdjustWidth(ImplGetImageToTextDistance() );
            aSize.AdjustWidth(aTextSize.Width() );
            if ( aSize.Height() < aTextSize.Height() )
                aSize.setHeight( aTextSize.Height() );
        }
        else
        {
            aSize.AdjustHeight(6 );
            aSize.AdjustHeight(GetTextHeight() );
            if ( aSize.Width() < aTextSize.Width() )
                aSize.setWidth( aTextSize.Width() );
        }
    }

    return CalcWindowSize( aSize );
}

Size RadioButton::GetOptimalSize() const
{
    return CalcMinimumSize();
}

void RadioButton::ShowFocus(const tools::Rectangle& rRect)
{
    if (IsNativeControlSupported(ControlType::Radiobutton, ControlPart::Focus))
    {
        ImplControlValue aControlValue;
        tools::Rectangle aInRect(Point(0, 0), GetSizePixel());

        aInRect.SetLeft( rRect.Left() );  // exclude the radio element itself from the focusrect

        DrawNativeControl(ControlType::Radiobutton, ControlPart::Focus, aInRect,
                          ControlState::FOCUSED, aControlValue, OUString());
    }
    Button::ShowFocus(rRect);
}

FactoryFunction RadioButton::GetUITestFactory() const
{
    return RadioButtonUIObject::create;
}

void CheckBox::ImplInitCheckBoxData()
{
    meState         = TRISTATE_FALSE;
    meSaveValue     = TRISTATE_FALSE;
    mbTriState      = false;
}

void CheckBox::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle(getPreviousSibling(pParent), nStyle);
    Button::ImplInit( pParent, nStyle, nullptr );

    ImplInitSettings( true );
}

WinBits CheckBox::ImplInitStyle( const vcl::Window* pPrevWindow, WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) &&
         (!pPrevWindow || (pPrevWindow->GetType() != WindowType::CHECKBOX)) )
        nStyle |= WB_GROUP;
    return nStyle;
}

const vcl::Font& CheckBox::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetRadioCheckFont();
}

const Color& CheckBox::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetRadioCheckTextColor();
}

void CheckBox::ImplInitSettings( bool bBackground )
{
    Button::ImplInitSettings();

    if ( bBackground )
    {
        vcl::Window* pParent = GetParent();
        if ( !IsControlBackground() &&
            (pParent->IsChildTransparentModeEnabled() || IsNativeControlSupported( ControlType::Checkbox, ControlPart::Entire ) ) )
        {
            EnableChildTransparentMode();
            SetParentClipMode( ParentClipMode::NoClip );
            SetPaintTransparent( true );
            SetBackground();
            if( IsNativeControlSupported( ControlType::Checkbox, ControlPart::Entire ) )
                ImplGetWindowImpl()->mbUseNativeFocus = ImplGetSVData()->maNWFData.mbNoFocusRects;
        }
        else
        {
            EnableChildTransparentMode( false );
            SetParentClipMode();
            SetPaintTransparent( false );

            if ( IsControlBackground() )
                SetBackground( GetControlBackground() );
            else
                SetBackground( pParent->GetBackground() );
        }
    }
}

void CheckBox::ImplDrawCheckBoxState(vcl::RenderContext& rRenderContext)
{
    bool bNativeOK = rRenderContext.IsNativeControlSupported(ControlType::Checkbox, ControlPart::Entire);
    if (bNativeOK)
    {
        ImplControlValue aControlValue(meState == TRISTATE_TRUE ? ButtonValue::On : ButtonValue::Off);
        tools::Rectangle aCtrlRegion(maStateRect);
        ControlState nState = ControlState::NONE;

        if (HasFocus())
            nState |= ControlState::FOCUSED;
        if (ImplGetButtonState() & DrawButtonFlags::Default)
            nState |= ControlState::DEFAULT;
        if (ImplGetButtonState() & DrawButtonFlags::Pressed)
            nState |= ControlState::PRESSED;
        if (IsEnabled())
            nState |= ControlState::ENABLED;

        if (meState == TRISTATE_TRUE)
            aControlValue.setTristateVal(ButtonValue::On);
        else if (meState == TRISTATE_INDET)
            aControlValue.setTristateVal(ButtonValue::Mixed);

        if (IsMouseOver() && maMouseRect.IsInside(GetPointerPosPixel()))
            nState |= ControlState::ROLLOVER;

        bNativeOK = rRenderContext.DrawNativeControl(ControlType::Checkbox, ControlPart::Entire, aCtrlRegion,
                                                     nState, aControlValue, OUString());
    }

    if (!bNativeOK)
    {
        DrawButtonFlags nStyle = ImplGetButtonState();
        if (!IsEnabled())
            nStyle |= DrawButtonFlags::Disabled;
        if (meState == TRISTATE_INDET)
            nStyle |= DrawButtonFlags::DontKnow;
        else if (meState == TRISTATE_TRUE)
            nStyle |= DrawButtonFlags::Checked;
        Image aImage = GetCheckImage(GetSettings(), nStyle);
        if (IsZoom())
            rRenderContext.DrawImage(maStateRect.TopLeft(), maStateRect.GetSize(), aImage);
        else
            rRenderContext.DrawImage(maStateRect.TopLeft(), aImage);
    }
}

void CheckBox::ImplDraw( OutputDevice* pDev, DrawFlags nDrawFlags,
                         const Point& rPos, const Size& rSize,
                         const Size& rImageSize, tools::Rectangle& rStateRect,
                         tools::Rectangle& rMouseRect )
{
    WinBits                 nWinStyle = GetStyle();
    OUString                aText( GetText() );

    pDev->Push( PushFlags::CLIPREGION | PushFlags::LINECOLOR );
    pDev->IntersectClipRegion( tools::Rectangle( rPos, rSize ) );

    if ( ( !aText.isEmpty() && ! (ImplGetButtonState() & DrawButtonFlags::NoText) ) ||
         ( HasImage() && !  (ImplGetButtonState() & DrawButtonFlags::NoImage) ) )
    {
        DrawTextFlags nTextStyle = Button::ImplGetTextStyle( nWinStyle, nDrawFlags );

        const long nImageSep = GetDrawPixel( pDev, ImplGetImageToTextDistance() );
        Size aSize( rSize );
        Point aPos( rPos );
        aPos.AdjustX(rImageSize.Width() + nImageSep );
        aSize.AdjustWidth( -(rImageSize.Width() + nImageSep) );

        // if the text rect height is smaller than the height of the image
        // then for single lines the default should be centered text
        if( (nWinStyle & (WB_TOP|WB_VCENTER|WB_BOTTOM)) == 0 &&
            (rImageSize.Height() > rSize.Height() || ! (nWinStyle & WB_WORDBREAK) ) )
        {
            nTextStyle &= ~DrawTextFlags(DrawTextFlags::Top|DrawTextFlags::Bottom);
            nTextStyle |= DrawTextFlags::VCenter;
            aSize.setHeight( rImageSize.Height() );
        }

        ImplDrawAlignedImage( pDev, aPos, aSize, 1, nTextStyle );

        rMouseRect          = tools::Rectangle( aPos, aSize );
        rMouseRect.SetLeft( rPos.X() );
        rStateRect.SetLeft( rPos.X() );
        rStateRect.SetTop( rMouseRect.Top() );

        if ( aSize.Height() > rImageSize.Height() )
            rStateRect.AdjustTop(( aSize.Height() - rImageSize.Height() ) / 2 );
        else
        {
            rStateRect.AdjustTop( -(( rImageSize.Height() - aSize.Height() ) / 2) );
            if( rStateRect.Top() < 0 )
                rStateRect.SetTop( 0 );
        }

        rStateRect.SetRight( rStateRect.Left()+rImageSize.Width()-1 );
        rStateRect.SetBottom( rStateRect.Top()+rImageSize.Height()-1 );
        if ( rStateRect.Bottom() > rMouseRect.Bottom() )
            rMouseRect.SetBottom( rStateRect.Bottom() );
    }
    else
    {
        if ( mbLegacyNoTextAlign && ( nWinStyle & WB_CENTER ) )
            rStateRect.SetLeft( rPos.X()+((rSize.Width()-rImageSize.Width())/2) );
        else if ( mbLegacyNoTextAlign && ( nWinStyle & WB_RIGHT ) )
            rStateRect.SetLeft( rPos.X()+rSize.Width()-rImageSize.Width() );
        else
            rStateRect.SetLeft( rPos.X() );
        if ( nWinStyle & WB_VCENTER )
            rStateRect.SetTop( rPos.Y()+((rSize.Height()-rImageSize.Height())/2) );
        else if ( nWinStyle & WB_BOTTOM )
            rStateRect.SetTop( rPos.Y()+rSize.Height()-rImageSize.Height() );
        else
            rStateRect.SetTop( rPos.Y() );
        rStateRect.SetRight( rStateRect.Left()+rImageSize.Width()-1 );
        rStateRect.SetBottom( rStateRect.Top()+rImageSize.Height()-1 );
        // provide space for focusrect
        // note: this assumes that the control's size was adjusted
        // accordingly in Get/LoseFocus, so the onscreen position won't change
        if( HasFocus() )
            rStateRect.Move( 1, 1 );
        rMouseRect          = rStateRect;

        ImplSetFocusRect( rStateRect );
    }

    pDev->Pop();
}

void CheckBox::ImplDrawCheckBox(vcl::RenderContext& rRenderContext)
{
    Size aImageSize = ImplGetCheckImageSize();
    aImageSize.setWidth( CalcZoom( aImageSize.Width() ) );
    aImageSize.setHeight( CalcZoom( aImageSize.Height() ) );

    HideFocus();

    ImplDraw(&rRenderContext, DrawFlags::NONE, Point(), GetOutputSizePixel(),
             aImageSize, maStateRect, maMouseRect);

    ImplDrawCheckBoxState(rRenderContext);
    if (HasFocus())
        ShowFocus(ImplGetFocusRect());
}

void CheckBox::ImplCheck()
{
    TriState eNewState;
    if ( meState == TRISTATE_FALSE )
        eNewState = TRISTATE_TRUE;
    else if ( !mbTriState )
        eNewState = TRISTATE_FALSE;
    else if ( meState == TRISTATE_TRUE )
        eNewState = TRISTATE_INDET;
    else
        eNewState = TRISTATE_FALSE;
    meState = eNewState;

    VclPtr<vcl::Window> xWindow = this;
    Invalidate();
    Update();
    Toggle();
    if ( xWindow->IsDisposed() )
        return;
    Click();
}

CheckBox::CheckBox( vcl::Window* pParent, WinBits nStyle ) :
    Button( WindowType::CHECKBOX ), mbLegacyNoTextAlign( false )
{
    ImplInitCheckBoxData();
    ImplInit( pParent, nStyle );
}

void CheckBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && maMouseRect.IsInside( rMEvt.GetPosPixel() ) )
    {
        ImplGetButtonState() |= DrawButtonFlags::Pressed;
        Invalidate();
        Update();
        StartTracking();
        return;
    }

    Button::MouseButtonDown( rMEvt );
}

void CheckBox::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( ImplGetButtonState() & DrawButtonFlags::Pressed )
        {
            if ( !(GetStyle() & WB_NOPOINTERFOCUS) && !rTEvt.IsTrackingCanceled() )
                GrabFocus();

            ImplGetButtonState() &= ~DrawButtonFlags::Pressed;

            // do not call click handler if aborted
            if ( !rTEvt.IsTrackingCanceled() )
                ImplCheck();
            else
            {
                Invalidate();
                Update();
            }
        }
    }
    else
    {
        if ( maMouseRect.IsInside( rTEvt.GetMouseEvent().GetPosPixel() ) )
        {
            if ( !(ImplGetButtonState() & DrawButtonFlags::Pressed) )
            {
                ImplGetButtonState() |= DrawButtonFlags::Pressed;
                Invalidate();
                Update();
            }
        }
        else
        {
            if ( ImplGetButtonState() & DrawButtonFlags::Pressed )
            {
                ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
                Invalidate();
                Update();
            }
        }
    }
}

void CheckBox::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( !aKeyCode.GetModifier() && (aKeyCode.GetCode() == KEY_SPACE) )
    {
        if ( !(ImplGetButtonState() & DrawButtonFlags::Pressed) )
        {
            ImplGetButtonState() |= DrawButtonFlags::Pressed;
            Invalidate();
            Update();
        }
    }
    else if ( (ImplGetButtonState() & DrawButtonFlags::Pressed) && (aKeyCode.GetCode() == KEY_ESCAPE) )
    {
        ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
        Invalidate();
        Update();
    }
    else
        Button::KeyInput( rKEvt );
}

void CheckBox::KeyUp( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( (ImplGetButtonState() & DrawButtonFlags::Pressed) && (aKeyCode.GetCode() == KEY_SPACE) )
    {
        ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
        ImplCheck();
    }
    else
        Button::KeyUp( rKEvt );
}

void CheckBox::FillLayoutData() const
{
    mpControlData->mpLayoutData.reset( new vcl::ControlLayoutData );
    const_cast<CheckBox*>(this)->Invalidate();
}

void CheckBox::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    ImplDrawCheckBox(rRenderContext);
}

void CheckBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                     DrawFlags nFlags )
{
    MapMode     aResMapMode( MapUnit::Map100thMM );
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    Size        aImageSize = pDev->LogicToPixel( Size( 300, 300 ), aResMapMode );
    Size        aBrd1Size = pDev->LogicToPixel( Size( 20, 20 ), aResMapMode );
    Size        aBrd2Size = pDev->LogicToPixel( Size( 30, 30 ), aResMapMode );
    long        nCheckWidth = pDev->LogicToPixel( Size( 20, 20 ), aResMapMode ).Width();
    vcl::Font   aFont = GetDrawPixelFont( pDev );
    tools::Rectangle   aStateRect;
    tools::Rectangle   aMouseRect;

    aImageSize.setWidth( CalcZoom( aImageSize.Width() ) );
    aImageSize.setHeight( CalcZoom( aImageSize.Height() ) );
    aBrd1Size.setWidth( CalcZoom( aBrd1Size.Width() ) );
    aBrd1Size.setHeight( CalcZoom( aBrd1Size.Height() ) );
    aBrd2Size.setWidth( CalcZoom( aBrd2Size.Width() ) );
    aBrd2Size.setHeight( CalcZoom( aBrd2Size.Height() ) );

    if ( !aBrd1Size.Width() )
        aBrd1Size.setWidth( 1 );
    if ( !aBrd1Size.Height() )
        aBrd1Size.setHeight( 1 );
    if ( !aBrd2Size.Width() )
        aBrd2Size.setWidth( 1 );
    if ( !aBrd2Size.Height() )
        aBrd2Size.setHeight( 1 );
    if ( !nCheckWidth )
        nCheckWidth = 1;

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    if ( nFlags & DrawFlags::Mono )
        pDev->SetTextColor( COL_BLACK );
    else
        pDev->SetTextColor( GetTextColor() );
    pDev->SetTextFillColor();

    ImplDraw( pDev, nFlags, aPos, aSize,
              aImageSize, aStateRect, aMouseRect );

    pDev->SetLineColor();
    pDev->SetFillColor( COL_BLACK );
    pDev->DrawRect( aStateRect );
    aStateRect.AdjustLeft(aBrd1Size.Width() );
    aStateRect.AdjustTop(aBrd1Size.Height() );
    aStateRect.AdjustRight( -(aBrd1Size.Width()) );
    aStateRect.AdjustBottom( -(aBrd1Size.Height()) );
    if ( meState == TRISTATE_INDET )
        pDev->SetFillColor( COL_LIGHTGRAY );
    else
        pDev->SetFillColor( COL_WHITE );
    pDev->DrawRect( aStateRect );

    if ( meState == TRISTATE_TRUE )
    {
        aStateRect.AdjustLeft(aBrd2Size.Width() );
        aStateRect.AdjustTop(aBrd2Size.Height() );
        aStateRect.AdjustRight( -(aBrd2Size.Width()) );
        aStateRect.AdjustBottom( -(aBrd2Size.Height()) );
        Point   aPos11( aStateRect.TopLeft() );
        Point   aPos12( aStateRect.BottomRight() );
        Point   aPos21( aStateRect.TopRight() );
        Point   aPos22( aStateRect.BottomLeft() );
        Point   aTempPos11( aPos11 );
        Point   aTempPos12( aPos12 );
        Point   aTempPos21( aPos21 );
        Point   aTempPos22( aPos22 );
        pDev->SetLineColor( COL_BLACK );
        long nDX = 0;
        for ( long i = 0; i < nCheckWidth; i++ )
        {
            if ( !(i % 2) )
            {
                aTempPos11.setX( aPos11.X()+nDX );
                aTempPos12.setX( aPos12.X()+nDX );
                aTempPos21.setX( aPos21.X()+nDX );
                aTempPos22.setX( aPos22.X()+nDX );
            }
            else
            {
                nDX++;
                aTempPos11.setX( aPos11.X()-nDX );
                aTempPos12.setX( aPos12.X()-nDX );
                aTempPos21.setX( aPos21.X()-nDX );
                aTempPos22.setX( aPos22.X()-nDX );
            }
            pDev->DrawLine( aTempPos11, aTempPos12 );
            pDev->DrawLine( aTempPos21, aTempPos22 );
        }
    }

    pDev->Pop();
}

void CheckBox::Resize()
{
    Control::Resize();
    Invalidate();
}

void CheckBox::GetFocus()
{
    if ( GetText().isEmpty() || (ImplGetButtonState() & DrawButtonFlags::NoText) )
    {
        // increase button size to have space for focus rect
        // checkboxes without text will draw focusrect around the check
        // See CheckBox::ImplDraw()
        Point aPos( GetPosPixel() );
        Size aSize( GetSizePixel() );
        aPos.Move(-1,-1);
        aSize.AdjustHeight(2 );
        aSize.AdjustWidth(2 );
        setPosSizePixel( aPos.X(), aPos.Y(), aSize.Width(), aSize.Height() );
        Invalidate();
    }
    else
        ShowFocus( ImplGetFocusRect() );

    SetInputContext( InputContext( GetFont() ) );
    Button::GetFocus();
}

void CheckBox::LoseFocus()
{
    if ( ImplGetButtonState() & DrawButtonFlags::Pressed )
    {
        ImplGetButtonState() &= ~DrawButtonFlags::Pressed;
        Invalidate();
        Update();
    }

    HideFocus();
    Button::LoseFocus();

    if ( GetText().isEmpty() || (ImplGetButtonState() & DrawButtonFlags::NoText) )
    {
        // decrease button size again (see GetFocus())
        // checkboxes without text will draw focusrect around the check
        Point aPos( GetPosPixel() );
        Size aSize( GetSizePixel() );
        aPos.Move(1,1);
        aSize.AdjustHeight( -2 );
        aSize.AdjustWidth( -2 );
        setPosSizePixel( aPos.X(), aPos.Y(), aSize.Width(), aSize.Height() );
        Invalidate();
    }
}

void CheckBox::StateChanged( StateChangedType nType )
{
    Button::StateChanged( nType );

    if ( nType == StateChangedType::State )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate( maStateRect );
    }
    else if ( (nType == StateChangedType::Enable) ||
              (nType == StateChangedType::Text) ||
              (nType == StateChangedType::Data) ||
              (nType == StateChangedType::UpdateMode) )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::Style )
    {
        SetStyle( ImplInitStyle( GetWindow( GetWindowType::Prev ), GetStyle() ) );

        if ( (GetPrevStyle() & CHECKBOX_VIEW_STYLE) !=
             (GetStyle() & CHECKBOX_VIEW_STYLE) )
        {
            if ( IsUpdateMode() )
                Invalidate();
        }
    }
    else if ( (nType == StateChangedType::Zoom) ||
              (nType == StateChangedType::ControlFont) )
    {
        ImplInitSettings( false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        ImplInitSettings( false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings( true );
        Invalidate();
    }
}

void CheckBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Button::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplInitSettings( true );
        Invalidate();
    }
}

bool CheckBox::PreNotify( NotifyEvent& rNEvt )
{
    const MouseEvent* pMouseEvt = nullptr;

    if( (rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != nullptr )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // trigger redraw if mouse over state has changed
            if( IsNativeControlSupported(ControlType::Checkbox, ControlPart::Entire) )
            {
                if (maMouseRect.IsInside(GetPointerPosPixel()) != maMouseRect.IsInside(GetLastPointerPosPixel()) ||
                    pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow())
                {
                    Invalidate( maStateRect );
                }
            }
        }
    }

    return Button::PreNotify(rNEvt);
}

void CheckBox::Toggle()
{
    ImplCallEventListenersAndHandler( VclEventId::CheckboxToggle, [this] () { maToggleHdl.Call(*this); } );
}

void CheckBox::SetState( TriState eState )
{
    if ( !mbTriState && (eState == TRISTATE_INDET) )
        eState = TRISTATE_FALSE;

    if ( meState != eState )
    {
        meState = eState;
        StateChanged( StateChangedType::State );
        Toggle();
    }
}

bool CheckBox::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "active")
        SetState(toBool(rValue) ? TRISTATE_TRUE : TRISTATE_FALSE);
    else
        return Button::set_property(rKey, rValue);
    return true;
}

void CheckBox::EnableTriState( bool bTriState )
{
    if ( mbTriState != bTriState )
    {
        mbTriState = bTriState;

        if ( !bTriState && (meState == TRISTATE_INDET) )
            SetState( TRISTATE_FALSE );
    }
}

long CheckBox::ImplGetImageToTextDistance() const
{
    // 4 pixels, but take zoom into account, so the text doesn't "jump" relative to surrounding elements,
    // which might have been aligned with the text of the check box
    return CalcZoom( 4 );
}

Size CheckBox::ImplGetCheckImageSize() const
{
    Size aSize;
    bool bDefaultSize = true;
    if( IsNativeControlSupported( ControlType::Checkbox, ControlPart::Entire ) )
    {
        ImplControlValue aControlValue;
        tools::Rectangle aCtrlRegion( Point( 0, 0 ), GetSizePixel() );
        tools::Rectangle aBoundingRgn, aContentRgn;

        // get native size of a check box
        if( GetNativeControlRegion( ControlType::Checkbox, ControlPart::Entire, aCtrlRegion,
                                    ControlState::DEFAULT|ControlState::ENABLED,
                                    aControlValue,
                                    aBoundingRgn, aContentRgn ) )
        {
            aSize = aContentRgn.GetSize();
            bDefaultSize = false;
        }
    }
    if( bDefaultSize )
        aSize = GetCheckImage( GetSettings(), DrawButtonFlags::NONE ).GetSizePixel();
    return aSize;
}

Image CheckBox::GetCheckImage( const AllSettings& rSettings, DrawButtonFlags nFlags )
{
    ImplSVData*             pSVData = ImplGetSVData();
    const StyleSettings&    rStyleSettings = rSettings.GetStyleSettings();
    sal_uInt16              nStyle = 0;

    if ( rStyleSettings.GetOptions() & StyleSettingsOptions::Mono )
        nStyle = STYLE_CHECKBOX_MONO;

    if ( pSVData->maCtrlData.maCheckImgList.empty() ||
         (pSVData->maCtrlData.mnCheckStyle != nStyle) ||
         (pSVData->maCtrlData.mnLastCheckFColor != rStyleSettings.GetFaceColor()) ||
         (pSVData->maCtrlData.mnLastCheckWColor != rStyleSettings.GetWindowColor()) ||
         (pSVData->maCtrlData.mnLastCheckLColor != rStyleSettings.GetLightColor()) )
    {
        pSVData->maCtrlData.maCheckImgList.clear();

        pSVData->maCtrlData.mnLastCheckFColor = rStyleSettings.GetFaceColor();
        pSVData->maCtrlData.mnLastCheckWColor = rStyleSettings.GetWindowColor();
        pSVData->maCtrlData.mnLastCheckLColor = rStyleSettings.GetLightColor();

        std::vector<OUString> aResources;
        if (nStyle)
        {
            aResources.emplace_back(SV_RESID_BITMAP_CHECKMONO1);
            aResources.emplace_back(SV_RESID_BITMAP_CHECKMONO2);
            aResources.emplace_back(SV_RESID_BITMAP_CHECKMONO3);
            aResources.emplace_back(SV_RESID_BITMAP_CHECKMONO4);
            aResources.emplace_back(SV_RESID_BITMAP_CHECKMONO5);
            aResources.emplace_back(SV_RESID_BITMAP_CHECKMONO6);
            aResources.emplace_back(SV_RESID_BITMAP_CHECKMONO7);
            aResources.emplace_back(SV_RESID_BITMAP_CHECKMONO8);
            aResources.emplace_back(SV_RESID_BITMAP_CHECKMONO9);
        }
        else
        {
            aResources.emplace_back(SV_RESID_BITMAP_CHECK1);
            aResources.emplace_back(SV_RESID_BITMAP_CHECK2);
            aResources.emplace_back(SV_RESID_BITMAP_CHECK3);
            aResources.emplace_back(SV_RESID_BITMAP_CHECK4);
            aResources.emplace_back(SV_RESID_BITMAP_CHECK5);
            aResources.emplace_back(SV_RESID_BITMAP_CHECK6);
            aResources.emplace_back(SV_RESID_BITMAP_CHECK7);
            aResources.emplace_back(SV_RESID_BITMAP_CHECK8);
            aResources.emplace_back(SV_RESID_BITMAP_CHECK9);
        }
        LoadThemedImageList(rStyleSettings, pSVData->maCtrlData.maCheckImgList, aResources);
        pSVData->maCtrlData.mnCheckStyle = nStyle;
    }

    sal_uInt16 nIndex;
    if ( nFlags & DrawButtonFlags::Disabled )
    {
        if ( nFlags & DrawButtonFlags::DontKnow )
            nIndex = 8;
        else if ( nFlags & DrawButtonFlags::Checked )
            nIndex = 5;
        else
            nIndex = 4;
    }
    else if ( nFlags & DrawButtonFlags::Pressed )
    {
        if ( nFlags & DrawButtonFlags::DontKnow )
            nIndex = 7;
        else if ( nFlags & DrawButtonFlags::Checked )
            nIndex = 3;
        else
            nIndex = 2;
    }
    else
    {
        if ( nFlags & DrawButtonFlags::DontKnow )
            nIndex = 6;
        else if ( nFlags & DrawButtonFlags::Checked )
            nIndex = 1;
        else
            nIndex = 0;
    }
    return pSVData->maCtrlData.maCheckImgList[nIndex];
}

void CheckBox::ImplSetMinimumNWFSize()
{
    Push( PushFlags::MAPMODE );
    SetMapMode(MapMode(MapUnit::MapPixel));

    ImplControlValue aControlValue;
    Size aCurSize( GetSizePixel() );
    tools::Rectangle aCtrlRegion( Point( 0, 0 ), aCurSize );
    tools::Rectangle aBoundingRgn, aContentRgn;

    // get native size of a radiobutton
    if( GetNativeControlRegion( ControlType::Checkbox, ControlPart::Entire, aCtrlRegion,
                                ControlState::DEFAULT|ControlState::ENABLED, aControlValue,
                                aBoundingRgn, aContentRgn ) )
    {
        Size aSize = aContentRgn.GetSize();

        if( aSize.Height() > aCurSize.Height() )
        {
            aCurSize.setHeight( aSize.Height() );
            SetSizePixel( aCurSize );
        }
    }

    Pop();
}

Size CheckBox::CalcMinimumSize( long nMaxWidth ) const
{
    Size aSize = ImplGetCheckImageSize();
    nMaxWidth -= aSize.Width();

    OUString aText = GetText();
    if ( !aText.isEmpty() && ! (ImplGetButtonState() & DrawButtonFlags::NoText) )
    {
        // subtract what will be added later
        nMaxWidth-=2;
        nMaxWidth -= ImplGetImageToTextDistance();

        Size aTextSize = GetTextRect( tools::Rectangle( Point(), Size( nMaxWidth > 0 ? nMaxWidth : 0x7fffffff, 0x7fffffff ) ),
                                      aText, FixedText::ImplGetTextStyle( GetStyle() ) ).GetSize();
        aSize.AdjustWidth(2 );    // for focus rect
        aSize.AdjustWidth(ImplGetImageToTextDistance() );
        aSize.AdjustWidth(aTextSize.Width() );
        if ( aSize.Height() < aTextSize.Height() )
            aSize.setHeight( aTextSize.Height() );
    }
    else
    {
        // is this still correct ? since the checkbox now
        // shows a focus rect it should be 2 pixels wider and longer
/* since otherwise the controls in the Writer hang too far up
        aSize.Width() += 2;
        aSize.Height() += 2;
*/
    }

    return CalcWindowSize( aSize );
}

Size CheckBox::GetOptimalSize() const
{
    int nWidthRequest(get_width_request());
    return CalcMinimumSize(nWidthRequest != -1 ? nWidthRequest : 0);
}

void CheckBox::ShowFocus(const tools::Rectangle& rRect)
{
    if (IsNativeControlSupported(ControlType::Checkbox, ControlPart::Focus))
    {
        ImplControlValue aControlValue;
        tools::Rectangle aInRect(Point(0, 0), GetSizePixel());

        aInRect.SetLeft( rRect.Left() );  // exclude the checkbox itself from the focusrect

        DrawNativeControl(ControlType::Checkbox, ControlPart::Focus, aInRect,
                          ControlState::FOCUSED, aControlValue, OUString());
    }
    Button::ShowFocus(rRect);
}

FactoryFunction CheckBox::GetUITestFactory() const
{
    return CheckBoxUIObject::create;
}

ImageButton::ImageButton( vcl::Window* pParent, WinBits nStyle ) :
    PushButton( pParent, nStyle )
{
    ImplInitStyle();
}

void ImageButton::ImplInitStyle()
{
    WinBits nStyle = GetStyle();

    if ( ! ( nStyle & ( WB_RIGHT | WB_LEFT ) ) )
        nStyle |= WB_CENTER;

    if ( ! ( nStyle & ( WB_TOP | WB_BOTTOM ) ) )
        nStyle |= WB_VCENTER;

    SetStyle( nStyle );
}

ImageRadioButton::ImageRadioButton( vcl::Window* pParent ) :
    RadioButton( pParent, 0 )
{
}

TriStateBox::TriStateBox( vcl::Window* pParent, WinBits nStyle ) :
    CheckBox( pParent, nStyle )
{
    EnableTriState();
}

DisclosureButton::DisclosureButton( vcl::Window* pParent ) :
    CheckBox( pParent, 0 )
{
}

void DisclosureButton::ImplDrawCheckBoxState(vcl::RenderContext& rRenderContext)
{
    /* HACK: DisclosureButton is currently assuming, that the disclosure sign
       will fit into the rectangle occupied by a normal checkbox on all themes.
       If this does not hold true for some theme, ImplGetCheckImageSize
       would have to be overridden for DisclosureButton; also GetNativeControlRegion
       for ControlType::ListNode would have to be implemented and taken into account
    */

    tools::Rectangle aStateRect(GetStateRect());

    ImplControlValue aControlValue(GetState() == TRISTATE_TRUE ? ButtonValue::On : ButtonValue::Off);
    tools::Rectangle aCtrlRegion(aStateRect);
    ControlState nState = ControlState::NONE;

    if (HasFocus())
        nState |= ControlState::FOCUSED;
    if (ImplGetButtonState() & DrawButtonFlags::Default)
        nState |= ControlState::DEFAULT;
    if (Window::IsEnabled())
        nState |= ControlState::ENABLED;
    if (IsMouseOver() && GetMouseRect().IsInside(GetPointerPosPixel()))
        nState |= ControlState::ROLLOVER;

    if (rRenderContext.DrawNativeControl(ControlType::ListNode, ControlPart::Entire, aCtrlRegion,
                                          nState, aControlValue, OUString()))
        return;

    ImplSVCtrlData& rCtrlData(ImplGetSVData()->maCtrlData);
    if (!rCtrlData.mpDisclosurePlus)
        rCtrlData.mpDisclosurePlus.reset(new Image(StockImage::Yes, SV_DISCLOSURE_PLUS));
    if (!rCtrlData.mpDisclosureMinus)
        rCtrlData.mpDisclosureMinus.reset(new Image(StockImage::Yes, SV_DISCLOSURE_MINUS));

    Image* pImg
        = IsChecked() ? rCtrlData.mpDisclosureMinus.get() : rCtrlData.mpDisclosurePlus.get();

    DrawImageFlags nStyle = DrawImageFlags::NONE;
    if (!IsEnabled())
        nStyle |= DrawImageFlags::Disable;

    Size aSize(aStateRect.GetSize());
    Size aImgSize(pImg->GetSizePixel());
    Point aOff((aSize.Width() - aImgSize.Width()) / 2,
               (aSize.Height() - aImgSize.Height()) / 2);
    aOff += aStateRect.TopLeft();
    rRenderContext.DrawImage(aOff, *pImg, nStyle);
}

void DisclosureButton::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if( !aKeyCode.GetModifier()  &&
        ( ( aKeyCode.GetCode() == KEY_ADD ) ||
          ( aKeyCode.GetCode() == KEY_SUBTRACT ) )
        )
    {
        Check( aKeyCode.GetCode() == KEY_ADD );
    }
    else
        CheckBox::KeyInput( rKEvt );
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeSmallButton(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    rRet = VclPtr<PushButton>::Create(pParent, WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_FLATBUTTON|WB_SMALLSTYLE);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
