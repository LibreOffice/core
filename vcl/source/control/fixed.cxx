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

#include "tools/rc.h"

#include "vcl/decoview.hxx"
#include "vcl/dialog.hxx"
#include "vcl/event.hxx"
#include "vcl/fixed.hxx"
#include "vcl/svapp.hxx"
#include "vcl/settings.hxx"

#include <comphelper/string.hxx>
#include "controldata.hxx"
#include "impimagetree.hxx"
#include "window.h"

#define FIXEDLINE_TEXT_BORDER    4

#define FIXEDTEXT_VIEW_STYLE    (WB_3DLOOK |                        \
                                 WB_LEFT | WB_CENTER | WB_RIGHT |   \
                                 WB_TOP | WB_VCENTER | WB_BOTTOM |  \
                                 WB_WORDBREAK | WB_NOLABEL |        \
                                 WB_INFO | WB_PATHELLIPSIS)
#define FIXEDLINE_VIEW_STYLE    (WB_3DLOOK | WB_NOLABEL)
#define FIXEDBITMAP_VIEW_STYLE  (WB_3DLOOK |                        \
                                 WB_LEFT | WB_CENTER | WB_RIGHT |   \
                                 WB_TOP | WB_VCENTER | WB_BOTTOM |  \
                                 WB_SCALE)
#define FIXEDIMAGE_VIEW_STYLE   (WB_3DLOOK |                        \
                                 WB_LEFT | WB_CENTER | WB_RIGHT |   \
                                 WB_TOP | WB_VCENTER | WB_BOTTOM |  \
                                 WB_SCALE)

static Point ImplCalcPos( WinBits nStyle, const Point& rPos,
                          const Size& rObjSize, const Size& rWinSize )
{
    long    nX;
    long    nY;

    if ( nStyle & WB_LEFT )
        nX = 0;
    else if ( nStyle & WB_RIGHT )
        nX = rWinSize.Width()-rObjSize.Width();
    else
        nX = (rWinSize.Width()-rObjSize.Width())/2;

    if ( nStyle & WB_TOP )
        nY = 0;
    else if ( nStyle & WB_BOTTOM )
        nY = rWinSize.Height()-rObjSize.Height();
    else
        nY = (rWinSize.Height()-rObjSize.Height())/2;

    if ( nStyle & WB_TOPLEFTVISIBLE )
    {
        if ( nX < 0 )
            nX = 0;
        if ( nY < 0 )
            nY = 0;
    }

    Point aPos( nX+rPos.X(), nY+rPos.Y() );
    return aPos;
}

void FixedText::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );
    ImplInitSettings( true, true, true );
}

WinBits FixedText::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

const vcl::Font& FixedText::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return ( GetStyle() & WB_INFO ) ? _rStyle.GetInfoFont() : _rStyle.GetLabelFont();
}

const Color& FixedText::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return ( GetStyle() & WB_INFO ) ? _rStyle.GetInfoTextColor() : _rStyle.GetLabelTextColor();
}

void FixedText::ImplInitSettings( bool bFont,
                                  bool bForeground, bool bBackground )
{
    Control::ImplInitSettings( bFont, bForeground );

    if ( bBackground )
    {
        vcl::Window* pParent = GetParent();
        if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
        {
            EnableChildTransparentMode( true );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( true );
            SetBackground();
        }
        else
        {
            EnableChildTransparentMode( false );
            SetParentClipMode( 0 );
            SetPaintTransparent( false );

            if ( IsControlBackground() )
                SetBackground( GetControlBackground() );
            else
                SetBackground( pParent->GetBackground() );
        }
    }
}

FixedText::FixedText( vcl::Window* pParent, WinBits nStyle )
    : Control(WINDOW_FIXEDTEXT)
    , m_nMaxWidthChars(-1)
    , m_nMinWidthChars(-1)
    , m_pMnemonicWindow(NULL)
{
    ImplInit( pParent, nStyle );
}

FixedText::FixedText( vcl::Window* pParent, const ResId& rResId )
    : Control(WINDOW_FIXEDTEXT)
    , m_nMaxWidthChars(-1)
    , m_nMinWidthChars(-1)
    , m_pMnemonicWindow(NULL)
{
    rResId.SetRT( RSC_TEXT );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

sal_uInt16 FixedText::ImplGetTextStyle( WinBits nWinStyle )
{
    sal_uInt16 nTextStyle = TEXT_DRAW_MNEMONIC | TEXT_DRAW_ENDELLIPSIS;

    if( ! (nWinStyle & WB_NOMULTILINE) )
        nTextStyle |= TEXT_DRAW_MULTILINE;

    if ( nWinStyle & WB_RIGHT )
        nTextStyle |= TEXT_DRAW_RIGHT;
    else if ( nWinStyle & WB_CENTER )
        nTextStyle |= TEXT_DRAW_CENTER;
    else
        nTextStyle |= TEXT_DRAW_LEFT;
    if ( nWinStyle & WB_BOTTOM )
        nTextStyle |= TEXT_DRAW_BOTTOM;
    else if ( nWinStyle & WB_VCENTER )
        nTextStyle |= TEXT_DRAW_VCENTER;
    else
        nTextStyle |= TEXT_DRAW_TOP;
    if ( nWinStyle & WB_WORDBREAK )
    {
        nTextStyle |= TEXT_DRAW_WORDBREAK;
        if ( (nWinStyle & WB_HYPHENATION ) == WB_HYPHENATION )
            nTextStyle |= TEXT_DRAW_WORDBREAK_HYPHENATION;
    }
    if ( nWinStyle & WB_NOLABEL )
        nTextStyle &= ~TEXT_DRAW_MNEMONIC;

    return nTextStyle;
}

void FixedText::ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                          const Point& rPos, const Size& rSize,
                          bool bFillLayout
                          ) const
{
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    WinBits                 nWinStyle = GetStyle();
    OUString           aText( GetText() );
    sal_uInt16              nTextStyle = FixedText::ImplGetTextStyle( nWinStyle );
    Point                   aPos = rPos;

    if ( nWinStyle & WB_EXTRAOFFSET )
        aPos.X() += 2;

    if ( nWinStyle & WB_PATHELLIPSIS )
    {
        nTextStyle &= ~(TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK);
        nTextStyle |= TEXT_DRAW_PATHELLIPSIS;
    }
    if ( nDrawFlags & WINDOW_DRAW_NOMNEMONIC )
    {
        if ( nTextStyle & TEXT_DRAW_MNEMONIC )
        {
            aText = GetNonMnemonicString( aText );
            nTextStyle &= ~TEXT_DRAW_MNEMONIC;
        }
    }
    if ( !(nDrawFlags & WINDOW_DRAW_NODISABLE) )
    {
        if ( !IsEnabled() )
            nTextStyle |= TEXT_DRAW_DISABLE;
    }
    if ( (nDrawFlags & WINDOW_DRAW_MONO) ||
         (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        nTextStyle |= TEXT_DRAW_MONO;

    if( bFillLayout )
        (mpControlData->mpLayoutData->m_aDisplayText).clear();

    Rectangle aRect( Rectangle( aPos, rSize ) );
    DrawControlText( *pDev, aRect, aText, nTextStyle,
        bFillLayout ? &mpControlData->mpLayoutData->m_aUnicodeBoundRects : NULL,
        bFillLayout ? &mpControlData->mpLayoutData->m_aDisplayText : NULL
    );
}

void FixedText::Paint( const Rectangle& )
{
    ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

void FixedText::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                      sal_uLong nFlags )
{
    ImplInitSettings( true, true, true );

    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    vcl::Font   aFont = GetDrawPixelFont( pDev );

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    if ( nFlags & WINDOW_DRAW_MONO )
        pDev->SetTextColor( Color( COL_BLACK ) );
    else
        pDev->SetTextColor( GetTextColor() );
    pDev->SetTextFillColor();

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

    ImplDraw( pDev, nFlags, aPos, aSize );
    pDev->Pop();
}

void FixedText::Resize()
{
    Control::Resize();
    Invalidate();
}

void FixedText::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == StateChangedType::ENABLE) ||
         (nType == StateChangedType::TEXT) ||
         (nType == StateChangedType::UPDATEMODE) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & FIXEDTEXT_VIEW_STYLE) !=
             (GetStyle() & FIXEDTEXT_VIEW_STYLE) )
        {
            ImplInitSettings( true, false, false );
            Invalidate();
        }
    }
    else if ( (nType == StateChangedType::ZOOM)  ||
              (nType == StateChangedType::CONTROLFONT) )
    {
        ImplInitSettings( true, false, false );
        Invalidate();
    }
    else if ( nType == StateChangedType::CONTROLFOREGROUND )
    {
        ImplInitSettings( false, true, false );
        Invalidate();
    }
    else if ( nType == StateChangedType::CONTROLBACKGROUND )
    {
        ImplInitSettings( false, false, true );
        Invalidate();
    }
}

void FixedText::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings( true, true, true );
        Invalidate();
    }
}

Size FixedText::getTextDimensions(Control const *pControl, const OUString &rTxt, long nMaxWidth)
{
    sal_uInt16 nStyle = ImplGetTextStyle( pControl->GetStyle() );
    if ( !( pControl->GetStyle() & WB_NOLABEL ) )
        nStyle |= TEXT_DRAW_MNEMONIC;

    return pControl->GetTextRect(Rectangle( Point(), Size(nMaxWidth, 0x7fffffff)),
                                       rTxt, nStyle).GetSize();
}

Size FixedText::CalcMinimumTextSize( Control const *pControl, long nMaxWidth )
{
    Size aSize = getTextDimensions(pControl, pControl->GetText(), nMaxWidth);

    if ( pControl->GetStyle() & WB_EXTRAOFFSET )
        aSize.Width() += 2;

    // GetTextRect cannot take an empty string
    if ( aSize.Width() < 0 )
        aSize.Width() = 0;
    if ( aSize.Height() <= 0 )
        aSize.Height() = pControl->GetTextHeight();

    return aSize;
}

Size FixedText::CalcMinimumSize( long nMaxWidth ) const
{
    return CalcWindowSize( CalcMinimumTextSize ( this, nMaxWidth ) );
}

Size FixedText::GetOptimalSize() const
{
    sal_Int32 nMaxAvailWidth = 0x7fffffff;
    const OUString &rTxt = GetText();
    if (m_nMaxWidthChars != -1 && m_nMaxWidthChars < rTxt.getLength())
    {
        nMaxAvailWidth = getTextDimensions(this,
            rTxt.copy(0, m_nMaxWidthChars), 0x7fffffff).Width();
    }
    Size aRet = CalcMinimumSize(nMaxAvailWidth);
    if (m_nMinWidthChars != -1)
    {
        OUStringBuffer aBuf;
        comphelper::string::padToLength(aBuf, m_nMinWidthChars, 'x');
        Size aMinAllowed = getTextDimensions(this,
            aBuf.makeStringAndClear(), 0x7fffffff);
        if (aMinAllowed.Width() > aRet.Width())
            aRet = aMinAllowed;
    }
    return aRet;
}

void FixedText::FillLayoutData() const
{
    mpControlData->mpLayoutData = new vcl::ControlLayoutData();
    ImplDraw( const_cast<FixedText*>(this), 0, Point(), GetOutputSizePixel(), true );
}

void FixedText::setMaxWidthChars(sal_Int32 nWidth)
{
    if (nWidth != m_nMaxWidthChars)
    {
        m_nMaxWidthChars = nWidth;
        queue_resize();
    }
}

void FixedText::setMinWidthChars(sal_Int32 nWidth)
{
    if (nWidth != m_nMinWidthChars)
    {
        m_nMinWidthChars = nWidth;
        queue_resize();
    }
}

bool FixedText::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "max-width-chars")
        setMaxWidthChars(rValue.toInt32());
    else if (rKey == "width-chars")
        setMinWidthChars(rValue.toInt32());
    else if (rKey == "ellipsize")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_PATHELLIPSIS);
        if (rValue != "none")
        {
            SAL_WARN_IF(rValue != "end", "vcl.layout", "Only endellipsis support for now");
            nBits |= WB_PATHELLIPSIS;
        }
        SetStyle(nBits);
    }
    else
        return Control::set_property(rKey, rValue);
    return true;
}

vcl::Window* FixedText::getAccessibleRelationLabelFor() const
{
    vcl::Window *pWindow = Control::getAccessibleRelationLabelFor();
    if (pWindow)
        return pWindow;
    return get_mnemonic_widget();
}

void FixedText::set_mnemonic_widget(vcl::Window *pWindow)
{
    if (pWindow == m_pMnemonicWindow)
        return;
    if (m_pMnemonicWindow)
    {
        vcl::Window *pTempReEntryGuard = m_pMnemonicWindow;
        m_pMnemonicWindow = NULL;
        pTempReEntryGuard->remove_mnemonic_label(this);
    }
    m_pMnemonicWindow = pWindow;
    if (m_pMnemonicWindow)
        m_pMnemonicWindow->add_mnemonic_label(this);
}

FixedText::~FixedText()
{
    set_mnemonic_widget(NULL);
}

SelectableFixedText::SelectableFixedText(vcl::Window* pParent, WinBits nStyle)
    : Edit(pParent, nStyle)
{
    // no border
    SetBorderStyle( WindowBorderStyle::NOBORDER );
    // read-only
    SetReadOnly();
    // make it transparent
    SetControlBackground();
    SetBackground();
    SetPaintTransparent( true );
}

void SelectableFixedText::LoseFocus()
{
    Edit::LoseFocus();
    // clear cursor
    Invalidate();
}

void FixedLine::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );
    ImplInitSettings( true, true, true );
}

WinBits FixedLine::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

const vcl::Font& FixedLine::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetGroupFont();
}

const Color& FixedLine::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetGroupTextColor();
}

void FixedLine::ImplInitSettings( bool bFont,
                                  bool bForeground, bool bBackground )
{
    Control::ImplInitSettings( bFont, bForeground );

    if ( bBackground )
    {
        vcl::Window* pParent = GetParent();
        if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
        {
            EnableChildTransparentMode( true );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( true );
            SetBackground();
        }
        else
        {
            EnableChildTransparentMode( false );
            SetParentClipMode( 0 );
            SetPaintTransparent( false );

            if ( IsControlBackground() )
                SetBackground( GetControlBackground() );
            else
                SetBackground( pParent->GetBackground() );
        }
    }
}

void FixedLine::ImplDraw( bool bLayout )
{
    Size                    aOutSize = GetOutputSizePixel();
    OUString                aText = GetText();
    WinBits                 nWinStyle = GetStyle();
    MetricVector*           pVector = bLayout ? &mpControlData->mpLayoutData->m_aUnicodeBoundRects : NULL;
    OUString*               pDisplayText = bLayout ? &mpControlData->mpLayoutData->m_aDisplayText : NULL;

    DecorationView aDecoView( this );
    if ( aText.isEmpty() )
    {
        if( !pVector )
        {
            if ( nWinStyle & WB_VERT )
            {
                long nX = (aOutSize.Width()-1)/2;
                aDecoView.DrawSeparator( Point( nX, 0 ), Point( nX, aOutSize.Height()-1 ) );
            }
            else
            {
                long nY = (aOutSize.Height()-1)/2;
                aDecoView.DrawSeparator( Point( 0, nY ), Point( aOutSize.Width()-1, nY ), false );
            }
        }
    }
    else if( (nWinStyle & WB_VERT) )
    {
        long nWidth = GetTextWidth( aText );
        Push( PushFlags::FONT );
        vcl::Font aFont( GetFont() );
        aFont.SetOrientation( 900 );
        SetFont( aFont );
        Point aStartPt( aOutSize.Width()/2, aOutSize.Height()-1 );
        if( (nWinStyle & WB_VCENTER) )
            aStartPt.Y() -= (aOutSize.Height() - nWidth)/2;
        Point aTextPt( aStartPt );
        aTextPt.X() -= GetTextHeight()/2;
        DrawText( aTextPt, aText, 0, aText.getLength(), pVector, pDisplayText );
        Pop();
        if( aOutSize.Height() - aStartPt.Y() > FIXEDLINE_TEXT_BORDER )
            aDecoView.DrawSeparator( Point( aStartPt.X(), aOutSize.Height()-1 ),
                                     Point( aStartPt.X(), aStartPt.Y() + FIXEDLINE_TEXT_BORDER ) );
        if( aStartPt.Y() - nWidth - FIXEDLINE_TEXT_BORDER > 0 )
            aDecoView.DrawSeparator( Point( aStartPt.X(), aStartPt.Y() - nWidth - FIXEDLINE_TEXT_BORDER ),
                                     Point( aStartPt.X(), 0 ) );
    }
    else
    {
        sal_uInt16      nStyle = TEXT_DRAW_MNEMONIC | TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER | TEXT_DRAW_ENDELLIPSIS;
        Rectangle   aRect( 0, 0, aOutSize.Width(), aOutSize.Height() );
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        if( (nWinStyle & WB_CENTER) )
            nStyle |= TEXT_DRAW_CENTER;

        if ( !IsEnabled() )
            nStyle |= TEXT_DRAW_DISABLE;
        if ( GetStyle() & WB_NOLABEL )
            nStyle &= ~TEXT_DRAW_MNEMONIC;
        if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
            nStyle |= TEXT_DRAW_MONO;

        DrawControlText( *this, aRect, aText, nStyle, pVector, pDisplayText );

        if( !pVector )
        {
            long nTop = aRect.Top() + ((aRect.GetHeight()-1)/2);
            aDecoView.DrawSeparator( Point( aRect.Right()+FIXEDLINE_TEXT_BORDER, nTop ), Point( aOutSize.Width()-1, nTop ), false );
            if( aRect.Left() > FIXEDLINE_TEXT_BORDER )
                aDecoView.DrawSeparator( Point( 0, nTop ), Point( aRect.Left()-FIXEDLINE_TEXT_BORDER, nTop ), false );
        }
    }
}

FixedLine::FixedLine( vcl::Window* pParent, WinBits nStyle ) :
    Control( WINDOW_FIXEDLINE )
{
    ImplInit( pParent, nStyle );
    SetSizePixel( Size( 2, 2 ) );
}

FixedLine::FixedLine( vcl::Window* pParent, const ResId& rResId ) :
    Control( WINDOW_FIXEDLINE )
{
    rResId.SetRT( RSC_FIXEDLINE );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

void  FixedLine::FillLayoutData() const
{
    mpControlData->mpLayoutData = new vcl::ControlLayoutData();
    const_cast<FixedLine*>(this)->ImplDraw( true );
}

void FixedLine::Paint( const Rectangle& )
{
    ImplDraw();
}

void FixedLine::Draw( OutputDevice*, const Point&, const Size&, sal_uLong )
{
}

void FixedLine::Resize()
{
    Control::Resize();
    Invalidate();
}

void FixedLine::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == StateChangedType::ENABLE) ||
         (nType == StateChangedType::TEXT) ||
         (nType == StateChangedType::UPDATEMODE) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & FIXEDLINE_VIEW_STYLE) !=
             (GetStyle() & FIXEDLINE_VIEW_STYLE) )
            Invalidate();
    }
    else if ( (nType == StateChangedType::ZOOM)  ||
              (nType == StateChangedType::STYLE) ||
              (nType == StateChangedType::CONTROLFONT) )
    {
        ImplInitSettings( true, false, false );
        Invalidate();
    }
    else if ( nType == StateChangedType::CONTROLFOREGROUND )
    {
        ImplInitSettings( false, true, false );
        Invalidate();
    }
    else if ( nType == StateChangedType::CONTROLBACKGROUND )
    {
        ImplInitSettings( false, false, true );
        Invalidate();
    }
}

void FixedLine::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings( true, true, true );
        Invalidate();
    }
}

Size FixedLine::GetOptimalSize() const
{
    return CalcWindowSize( FixedText::CalcMinimumTextSize ( this, 0x7fffffff ) );
}

void FixedBitmap::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );
    ImplInitSettings();
}

WinBits FixedBitmap::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

void FixedBitmap::ImplInitSettings()
{
    vcl::Window* pParent = GetParent();
    if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
    {
        EnableChildTransparentMode( true );
        SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        SetPaintTransparent( true );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( false );
        SetParentClipMode( 0 );
        SetPaintTransparent( false );

        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}

FixedBitmap::FixedBitmap( vcl::Window* pParent, WinBits nStyle ) :
    Control( WINDOW_FIXEDBITMAP )
{
    ImplInit( pParent, nStyle );
}

FixedBitmap::~FixedBitmap()
{
}

void FixedBitmap::ImplDraw( OutputDevice* pDev, sal_uLong /* nDrawFlags */,
                            const Point& rPos, const Size& rSize )
{
    Bitmap* pBitmap = &maBitmap;

    // do we have a Bitmap?
    if ( !(!(*pBitmap)) )
    {
        if ( GetStyle() & WB_SCALE )
            pDev->DrawBitmap( rPos, rSize, *pBitmap );
        else
        {
            Point aPos = ImplCalcPos( GetStyle(), rPos, pBitmap->GetSizePixel(), rSize );
            pDev->DrawBitmap( aPos, *pBitmap );
        }
    }
}

void FixedBitmap::Paint( const Rectangle& )
{
    ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

void FixedBitmap::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                        sal_uLong nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    Rectangle   aRect( aPos, aSize );

    pDev->Push();
    pDev->SetMapMode();

    // Border
    if ( !(nFlags & WINDOW_DRAW_NOBORDER) && (GetStyle() & WB_BORDER) )
    {
        DecorationView aDecoView( pDev );
        aRect = aDecoView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN );
    }
    pDev->IntersectClipRegion( aRect );
    ImplDraw( pDev, nFlags, aRect.TopLeft(), aRect.GetSize() );

    pDev->Pop();
}

void FixedBitmap::Resize()
{
    Control::Resize();
    Invalidate();
}

void FixedBitmap::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == StateChangedType::DATA) ||
         (nType == StateChangedType::UPDATEMODE) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & FIXEDBITMAP_VIEW_STYLE) !=
             (GetStyle() & FIXEDBITMAP_VIEW_STYLE) )
            Invalidate();
    }
    else if ( nType == StateChangedType::CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void FixedBitmap::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void FixedBitmap::SetBitmap( const Bitmap& rBitmap )
{
    maBitmap = rBitmap;
    StateChanged( StateChangedType::DATA );
    queue_resize();
}

void FixedImage::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    mbInUserDraw = false;
    Control::ImplInit( pParent, nStyle, NULL );
    ImplInitSettings();
}

WinBits FixedImage::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

void FixedImage::ImplInitSettings()
{
    vcl::Window* pParent = GetParent();
    if ( pParent && pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
    {
        EnableChildTransparentMode( true );
        SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        SetPaintTransparent( true );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( false );
        SetParentClipMode( 0 );
        SetPaintTransparent( false );

        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else if ( pParent )
            SetBackground( pParent->GetBackground() );
    }
}

void FixedImage::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    sal_uLong nObjMask = ReadLongRes();

    if ( RSC_FIXEDIMAGE_IMAGE & nObjMask )
    {
        maImage = Image( ResId( (RSHEADER_TYPE*)GetClassRes(), *rResId.GetResMgr() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
    }
}

FixedImage::FixedImage( vcl::Window* pParent, WinBits nStyle ) :
    Control( WINDOW_FIXEDIMAGE )
{
    ImplInit( pParent, nStyle );
}

FixedImage::FixedImage( vcl::Window* pParent, const ResId& rResId ) :
    Control( WINDOW_FIXEDIMAGE )
{
    rResId.SetRT( RSC_FIXEDIMAGE );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

FixedImage::~FixedImage()
{
}

void FixedImage::ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                           const Point& rPos, const Size& rSize )
{
    sal_uInt16 nStyle = 0;
    if ( !(nDrawFlags & WINDOW_DRAW_NODISABLE) )
    {
        if ( !IsEnabled() )
            nStyle |= IMAGE_DRAW_DISABLE;
    }

    Image *pImage = &maImage;

    // do we have an image?
    if ( !(!(*pImage)) )
    {
        if ( GetStyle() & WB_SCALE )
            pDev->DrawImage( rPos, rSize, *pImage, nStyle );
        else
        {
            Point aPos = ImplCalcPos( GetStyle(), rPos, pImage->GetSizePixel(), rSize );
            pDev->DrawImage( aPos, *pImage, nStyle );
        }
    }

    mbInUserDraw = true;
    UserDrawEvent aUDEvt( pDev, Rectangle( rPos, rSize ), 0, nStyle );
    UserDraw( aUDEvt );
    mbInUserDraw = false;
}

void FixedImage::Paint( const Rectangle& )
{
    ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

Size FixedImage::GetOptimalSize() const
{
    return maImage.GetSizePixel();
}

void FixedImage::UserDraw( const UserDrawEvent& )
{
}

void FixedImage::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                       sal_uLong nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    Rectangle   aRect( aPos, aSize );

    pDev->Push();
    pDev->SetMapMode();

    // Border
    if ( !(nFlags & WINDOW_DRAW_NOBORDER) && (GetStyle() & WB_BORDER) )
    {
        ImplDrawFrame( pDev, aRect );
    }
    pDev->IntersectClipRegion( aRect );
    ImplDraw( pDev, nFlags, aRect.TopLeft(), aRect.GetSize() );

    pDev->Pop();
}

void FixedImage::Resize()
{
    Control::Resize();
    Invalidate();
}

void FixedImage::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == StateChangedType::ENABLE) ||
         (nType == StateChangedType::DATA) ||
         (nType == StateChangedType::UPDATEMODE) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & FIXEDIMAGE_VIEW_STYLE) !=
             (GetStyle() & FIXEDIMAGE_VIEW_STYLE) )
            Invalidate();
    }
    else if ( nType == StateChangedType::CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void FixedImage::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void FixedImage::SetImage( const Image& rImage )
{
    if ( rImage != maImage )
    {
        maImage = rImage;
        StateChanged( StateChangedType::DATA );
        queue_resize();
    }
}

bool FixedImage::SetModeImage( const Image& rImage )
{
    SetImage( rImage );
    return true;
}


Image FixedImage::loadThemeImage(const OString &rFileName)
{
    static ImplImageTreeSingletonRef aImageTree;
    OUString sIconTheme =
        Application::GetSettings().GetStyleSettings().DetermineIconTheme();
    const OUString sFileName(OStringToOUString(rFileName, RTL_TEXTENCODING_UTF8));
    BitmapEx aBitmap;
    bool bSuccess = aImageTree->loadImage(sFileName, sIconTheme, aBitmap, true);
    SAL_WARN_IF(!bSuccess, "vcl.layout", "Unable to load " << sFileName
        << " from theme " << sIconTheme);
    return Image(aBitmap);
}

bool FixedImage::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "pixbuf")
    {
        SetImage(FixedImage::loadThemeImage(rValue));
    }
    else
        return Control::set_property(rKey, rValue);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
