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

// General info:
//   http://msdn.microsoft.com/en-us/library/windows/desktop/hh270423%28v=vs.85%29.aspx
//   http://msdn.microsoft.com/en-us/library/windows/desktop/bb773178%28v=vs.85%29.aspx

// Useful tool to explore the themes & their rendering:
//   http://privat.rejbrand.se/UxExplore.exe
// (found at http://stackoverflow.com/questions/4009701/windows-visual-themes-gallery-of-parts-and-states/4009712#4009712)

// Theme subclasses:
//   http://msdn.microsoft.com/en-us/library/windows/desktop/bb773218%28v=vs.85%29.aspx

// Drawing in non-client area (general DWM-related info):
//   http://msdn.microsoft.com/en-us/library/windows/desktop/bb688195%28v=vs.85%29.aspx

#include <rtl/ustring.h>

#include <osl/diagnose.h>
#include <osl/module.h>
#include <o3tl/char16_t2wchar_t.hxx>

#include <opengl/win/gdiimpl.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <win/svsys.h>
#include <win/salgdi.h>
#include <win/saldata.hxx>
#include <win/scoped_gdi.hxx>

#include <uxtheme.h>
#include <vssym32.h>

#include <map>
#include <string>
#include <boost/optional.hpp>
#include <ControlCacheKey.hxx>

using namespace std;

typedef map< wstring, HTHEME > ThemeMap;
static ThemeMap aThemeMap;

/****************************************************
 wrap visual styles API to avoid linking against it
 it is not available on all Windows platforms
*****************************************************/

class VisualStylesAPI
{
private:
    typedef HTHEME  (WINAPI * OpenThemeData_Proc_T) ( HWND hwnd, LPCWSTR pszClassList );
    typedef HRESULT (WINAPI * CloseThemeData_Proc_T) ( HTHEME hTheme );
    typedef HRESULT (WINAPI * GetThemeBackgroundContentRect_Proc_T) ( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pBoundingRect, RECT *pContentRect );
    typedef HRESULT (WINAPI * DrawThemeBackground_Proc_T) ( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect );
    typedef HRESULT (WINAPI * DrawThemeText_Proc_T) ( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect );
    typedef HRESULT (WINAPI * GetThemePartSize_Proc_T) ( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, RECT *prc, THEMESIZE eSize, SIZE *psz );
    typedef BOOL    (WINAPI * IsThemeActive_Proc_T) ( void );

    OpenThemeData_Proc_T                    lpfnOpenThemeData;
    CloseThemeData_Proc_T                   lpfnCloseThemeData;
    GetThemeBackgroundContentRect_Proc_T    lpfnGetThemeBackgroundContentRect;
    DrawThemeBackground_Proc_T              lpfnDrawThemeBackground;
    DrawThemeText_Proc_T                    lpfnDrawThemeText;
    GetThemePartSize_Proc_T                 lpfnGetThemePartSize;
    IsThemeActive_Proc_T                    lpfnIsThemeActive;

    oslModule mhModule;

public:
    VisualStylesAPI();
    ~VisualStylesAPI();

    HTHEME OpenThemeData( HWND hwnd, LPCWSTR pszClassList );
    HRESULT CloseThemeData( HTHEME hTheme );
    HRESULT GetThemeBackgroundContentRect( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pBoundingRect, RECT *pContentRect );
    HRESULT DrawThemeBackground( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect );
    HRESULT DrawThemeText( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect );
    HRESULT GetThemePartSize( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, RECT *prc, THEMESIZE eSize, SIZE *psz );
    BOOL IsThemeActive();
};

static VisualStylesAPI vsAPI;

VisualStylesAPI::VisualStylesAPI()
    : lpfnOpenThemeData( nullptr ),
      lpfnCloseThemeData( nullptr ),
      lpfnGetThemeBackgroundContentRect( nullptr ),
      lpfnDrawThemeBackground( nullptr ),
      lpfnDrawThemeText( nullptr ),
      lpfnGetThemePartSize( nullptr ),
      lpfnIsThemeActive( nullptr )
{
    OUString aLibraryName( "uxtheme.dll" );
    mhModule = osl_loadModule( aLibraryName.pData, SAL_LOADMODULE_DEFAULT );

    if ( mhModule )
    {
        lpfnOpenThemeData = reinterpret_cast<OpenThemeData_Proc_T>(osl_getAsciiFunctionSymbol( mhModule, "OpenThemeData" ));
        lpfnCloseThemeData = reinterpret_cast<CloseThemeData_Proc_T>(osl_getAsciiFunctionSymbol( mhModule, "CloseThemeData" ));
        lpfnGetThemeBackgroundContentRect = reinterpret_cast<GetThemeBackgroundContentRect_Proc_T>(osl_getAsciiFunctionSymbol( mhModule, "GetThemeBackgroundContentRect" ));
        lpfnDrawThemeBackground = reinterpret_cast<DrawThemeBackground_Proc_T>(osl_getAsciiFunctionSymbol( mhModule, "DrawThemeBackground" ));
        lpfnDrawThemeText = reinterpret_cast<DrawThemeText_Proc_T>(osl_getAsciiFunctionSymbol( mhModule, "DrawThemeText" ));
        lpfnGetThemePartSize = reinterpret_cast<GetThemePartSize_Proc_T>(osl_getAsciiFunctionSymbol( mhModule, "GetThemePartSize" ));
        lpfnIsThemeActive = reinterpret_cast<IsThemeActive_Proc_T>(osl_getAsciiFunctionSymbol( mhModule, "IsThemeActive" ));
    }
}

VisualStylesAPI::~VisualStylesAPI()
{
    if( mhModule )
        osl_unloadModule( mhModule );
}

HTHEME VisualStylesAPI::OpenThemeData( HWND hwnd, LPCWSTR pszClassList )
{
    if(lpfnOpenThemeData)
        return (*lpfnOpenThemeData) (hwnd, pszClassList);
    else
        return nullptr;
}

HRESULT VisualStylesAPI::CloseThemeData( HTHEME hTheme )
{
    if(lpfnCloseThemeData)
        return (*lpfnCloseThemeData) (hTheme);
    else
        return S_FALSE;
}

HRESULT VisualStylesAPI::GetThemeBackgroundContentRect( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pBoundingRect, RECT *pContentRect )
{
    if(lpfnGetThemeBackgroundContentRect)
        return (*lpfnGetThemeBackgroundContentRect) ( hTheme, hdc, iPartId, iStateId, pBoundingRect, pContentRect );
    else
        return S_FALSE;
}

HRESULT VisualStylesAPI::DrawThemeBackground( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect )
{
    if(lpfnDrawThemeBackground)
        return (*lpfnDrawThemeBackground) (hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
    else
        return S_FALSE;
}

HRESULT VisualStylesAPI::DrawThemeText( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect )
{
    if(lpfnDrawThemeText)
        return (*lpfnDrawThemeText) (hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
    else
        return S_FALSE;
}

HRESULT VisualStylesAPI::GetThemePartSize( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, RECT *prc, THEMESIZE eSize, SIZE *psz )
{
    if(lpfnGetThemePartSize)
        return (*lpfnGetThemePartSize) (hTheme, hdc, iPartId, iStateId, prc, eSize, psz);
    else
        return S_FALSE;
}

BOOL VisualStylesAPI::IsThemeActive()
{
    if(lpfnIsThemeActive)
        return (*lpfnIsThemeActive) ();
    else
        return FALSE;
}

/*********************************************************
 * Initialize XP theming and local stuff
 *********************************************************/
void SalData::initNWF()
{
    ImplSVData* pSVData = ImplGetSVData();

    // the menu bar and the top docking area should have a common background (gradient)
    pSVData->maNWFData.mbMenuBarDockingAreaCommonBG = true;
}

// *********************************************************
// * Release theming handles
// ********************************************************
void SalData::deInitNWF()
{
    for( auto& rEntry : aThemeMap )
        vsAPI.CloseThemeData(rEntry.second);
    aThemeMap.clear();
}

static HTHEME getThemeHandle( HWND hWnd, LPCWSTR name )
{
    if( GetSalData()->mbThemeChanged )
    {
        // throw away invalid theme handles
        SalData::deInitNWF();
        GetSalData()->mbThemeChanged = false;
    }

    ThemeMap::iterator iter;
    if( (iter = aThemeMap.find( name )) != aThemeMap.end() )
        return iter->second;
    // theme not found -> add it to map
    HTHEME hTheme = vsAPI.OpenThemeData( hWnd, name );
    if( hTheme != nullptr )
        aThemeMap[name] = hTheme;
    return hTheme;
}

bool WinSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    HTHEME hTheme = nullptr;

    switch( nType )
    {
        case ControlType::Pushbutton:
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"Button");
            break;
        case ControlType::Scrollbar:
            if( nPart == ControlPart::DrawBackgroundHorz || nPart == ControlPart::DrawBackgroundVert )
                return FALSE;   // no background painting needed
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"Scrollbar");
            break;
        case ControlType::Combobox:
            if( nPart == ControlPart::HasBackgroundTexture )
                return FALSE;   // we do not paint the inner part (ie the selection background/focus indication)
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"Edit");
            else if( nPart == ControlPart::ButtonDown )
                hTheme = getThemeHandle( mhWnd, L"Combobox");
            break;
        case ControlType::Spinbox:
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"Edit");
            else if( nPart == ControlPart::AllButtons ||
                nPart == ControlPart::ButtonUp || nPart == ControlPart::ButtonDown ||
                nPart == ControlPart::ButtonLeft|| nPart == ControlPart::ButtonRight )
                hTheme = getThemeHandle( mhWnd, L"Spin");
            break;
        case ControlType::SpinButtons:
            if( nPart == ControlPart::Entire || nPart == ControlPart::AllButtons )
                hTheme = getThemeHandle( mhWnd, L"Spin");
            break;
        case ControlType::Editbox:
        case ControlType::MultilineEditbox:
            if( nPart == ControlPart::HasBackgroundTexture )
                return FALSE;   // we do not paint the inner part (ie the selection background/focus indication)
                //return TRUE;
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"Edit");
            break;
        case ControlType::Listbox:
            if( nPart == ControlPart::HasBackgroundTexture )
                return FALSE;   // we do not paint the inner part (ie the selection background/focus indication)
            if( nPart == ControlPart::Entire || nPart == ControlPart::ListboxWindow )
                hTheme = getThemeHandle( mhWnd, L"Listview");
            else if( nPart == ControlPart::ButtonDown )
                hTheme = getThemeHandle( mhWnd, L"Combobox");
            break;
        case ControlType::TabPane:
        case ControlType::TabBody:
        case ControlType::TabItem:
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"Tab");
            break;
        case ControlType::Toolbar:
            if( nPart == ControlPart::Entire || nPart == ControlPart::Button )
                hTheme = getThemeHandle( mhWnd, L"Toolbar");
            else
                // use rebar theme for grip and background
                hTheme = getThemeHandle( mhWnd, L"Rebar");
            break;
        case ControlType::Menubar:
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"Rebar");
            else if( GetSalData()->mbThemeMenuSupport )
            {
                if( nPart == ControlPart::MenuItem )
                    hTheme = getThemeHandle( mhWnd, L"Menu" );
            }
            break;
        case ControlType::MenuPopup:
            if( GetSalData()->mbThemeMenuSupport )
            {
                if( nPart == ControlPart::Entire ||
                    nPart == ControlPart::MenuItem ||
                    nPart == ControlPart::MenuItemCheckMark ||
                    nPart == ControlPart::MenuItemRadioMark ||
                    nPart == ControlPart::Separator )
                    hTheme = getThemeHandle( mhWnd, L"Menu" );
            }
            break;
        case ControlType::Progress:
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"Progress");
            break;
        case ControlType::Slider:
            if( nPart == ControlPart::TrackHorzArea || nPart == ControlPart::TrackVertArea )
                hTheme = getThemeHandle( mhWnd, L"Trackbar" );
            break;
        case ControlType::ListNode:
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"TreeView" );
            break;
        default:
            hTheme = nullptr;
            break;
    }

    return (hTheme != nullptr);
}

bool WinSalGraphics::hitTestNativeControl( ControlType,
                              ControlPart,
                              const tools::Rectangle&,
                              const Point&,
                              bool& )
{
    return FALSE;
}

static bool ImplDrawTheme( HTHEME hTheme, HDC hDC, int iPart, int iState, RECT rc, const OUString& aStr)
{
    HRESULT hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, nullptr);

    if( aStr.getLength() )
    {
        RECT rcContent;
        hr = vsAPI.GetThemeBackgroundContentRect( hTheme, hDC, iPart, iState, &rc, &rcContent);
        hr = vsAPI.DrawThemeText( hTheme, hDC, iPart, iState,
            o3tl::toW(aStr.getStr()), -1,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE,
            0, &rcContent);
    }
    return (hr == S_OK);
}

static tools::Rectangle ImplGetThemeRect( HTHEME hTheme, HDC hDC, int iPart, int iState, const tools::Rectangle& /* aRect */, THEMESIZE eTS = TS_TRUE )
{
    SIZE aSz;
    HRESULT hr = vsAPI.GetThemePartSize( hTheme, hDC, iPart, iState, nullptr, eTS, &aSz ); // TS_TRUE returns optimal size
    if( hr == S_OK )
        return tools::Rectangle( 0, 0, aSz.cx, aSz.cy );
    else
        return tools::Rectangle();
}

// Helper functions

static void ImplConvertSpinbuttonValues( ControlPart nControlPart, const ControlState& rState, const tools::Rectangle& rRect,
                                 int* pLunaPart, int *pLunaState, RECT *pRect )
{
    if( nControlPart == ControlPart::ButtonDown )
    {
        *pLunaPart = SPNP_DOWN;
        if( rState & ControlState::PRESSED )
            *pLunaState = DNS_PRESSED;
        else if( !(rState & ControlState::ENABLED) )
            *pLunaState = DNS_DISABLED;
        else if( rState & ControlState::ROLLOVER )
            *pLunaState = DNS_HOT;
        else
            *pLunaState = DNS_NORMAL;
    }
    if( nControlPart == ControlPart::ButtonUp )
    {
        *pLunaPart = SPNP_UP;
        if( rState & ControlState::PRESSED )
            *pLunaState = UPS_PRESSED;
        else if( !(rState & ControlState::ENABLED) )
            *pLunaState = UPS_DISABLED;
        else if( rState & ControlState::ROLLOVER )
            *pLunaState = UPS_HOT;
        else
            *pLunaState = UPS_NORMAL;
    }
    if( nControlPart == ControlPart::ButtonRight )
    {
        *pLunaPart = SPNP_UPHORZ;
        if( rState & ControlState::PRESSED )
            *pLunaState = DNHZS_PRESSED;
        else if( !(rState & ControlState::ENABLED) )
            *pLunaState = DNHZS_DISABLED;
        else if( rState & ControlState::ROLLOVER )
            *pLunaState = DNHZS_HOT;
        else
            *pLunaState = DNHZS_NORMAL;
    }
    if( nControlPart == ControlPart::ButtonLeft )
    {
        *pLunaPart = SPNP_DOWNHORZ;
        if( rState & ControlState::PRESSED )
            *pLunaState = UPHZS_PRESSED;
        else if( !(rState & ControlState::ENABLED) )
            *pLunaState = UPHZS_DISABLED;
        else if( rState & ControlState::ROLLOVER )
            *pLunaState = UPHZS_HOT;
        else
            *pLunaState = UPHZS_NORMAL;
    }

    pRect->left   = rRect.Left();
    pRect->right  = rRect.Right()+1;
    pRect->top    = rRect.Top();
    pRect->bottom = rRect.Bottom()+1;
}

/// Draw an own toolbar style on Windows Vista or later, looks better there
static void impl_drawAeroToolbar( HDC hDC, RECT rc, bool bHorizontal )
{
    if ( rc.top == 0 && bHorizontal )
    {
        const long GRADIENT_HEIGHT = 32;

        long gradient_break = rc.top;
        long gradient_bottom = rc.bottom - 1;
        GRADIENT_RECT g_rect[1] = { { 0, 1 } };

        // very slow gradient at the top (if we have space for that)
        if ( gradient_bottom - rc.top > GRADIENT_HEIGHT )
        {
            gradient_break = gradient_bottom - GRADIENT_HEIGHT;

            TRIVERTEX vert[2] = {
                { rc.left, rc.top,          0xff00, 0xff00, 0xff00, 0xff00 },
                { rc.right, gradient_break, 0xfa00, 0xfa00, 0xfa00, 0xff00 },
            };
            GdiGradientFill( hDC, vert, 2, g_rect, 1, GRADIENT_FILL_RECT_V );
        }

        // gradient at the bottom
        TRIVERTEX vert[2] = {
            { rc.left, gradient_break,   0xfa00, 0xfa00, 0xfa00, 0xff00 },
            { rc.right, gradient_bottom, 0xf000, 0xf000, 0xf000, 0xff00 }
        };
        GdiGradientFill( hDC, vert, 2, g_rect, 1, GRADIENT_FILL_RECT_V );

        // and a darker horizontal line under that
        ScopedSelectedHPEN hPen(hDC, CreatePen(PS_SOLID, 1, RGB( 0xb0, 0xb0, 0xb0)));

        MoveToEx( hDC, rc.left, gradient_bottom, nullptr );
        LineTo( hDC, rc.right, gradient_bottom );
    }
    else
    {
        ScopedHBRUSH hbrush(CreateSolidBrush(RGB(0xf0, 0xf0, 0xf0)));
        FillRect(hDC, &rc, hbrush.get());

        // darker line to distinguish the toolbar and viewshell
        // it is drawn only for the horizontal toolbars; it did not look well
        // when done for the vertical ones too
        if ( bHorizontal )
        {
            long from_x, from_y, to_x, to_y;

            from_x = rc.left;
            to_x = rc.right;
            from_y = to_y = rc.top;

            ScopedSelectedHPEN hPen(hDC, CreatePen(PS_SOLID, 1, RGB( 0xb0, 0xb0, 0xb0)));

            MoveToEx( hDC, from_x, from_y, nullptr );
            LineTo( hDC, to_x, to_y );
        }
    }
}

/**
 * Gives the actual rectangle used for rendering by ControlType::MenuPopup's
 * ControlPart::MenuItemCheckMark or ControlPart::MenuItemRadioMark.
 */
static tools::Rectangle GetMenuPopupMarkRegion(const ImplControlValue& rValue)
{
    tools::Rectangle aRet;

    auto pMVal = dynamic_cast<const MenupopupValue*>(&rValue);
    if (!pMVal)
        return aRet;

    aRet.SetTop(pMVal->maItemRect.Top());
    aRet.SetBottom(pMVal->maItemRect.Bottom() + 1); // see below in drawNativeControl
    if (AllSettings::GetLayoutRTL())
    {
        aRet.SetRight(pMVal->maItemRect.Right() + 1);
        aRet.SetLeft(aRet.Right() - (pMVal->getNumericVal() - pMVal->maItemRect.Left()));
    }
    else
    {
        aRet.SetRight(pMVal->getNumericVal());
        aRet.SetLeft(pMVal->maItemRect.Left());
    }

    return aRet;
}

static bool ImplDrawNativeControl( HDC hDC, HTHEME hTheme, RECT rc,
                            ControlType nType,
                            ControlPart nPart,
                            ControlState nState,
                            const ImplControlValue& aValue,
                            OUString const & aCaption )
{
    // a listbox dropdown is actually a combobox dropdown
    if( nType == ControlType::Listbox )
        if( nPart == ControlPart::ButtonDown )
            nType = ControlType::Combobox;

    // draw entire combobox as a large edit box
    if( nType == ControlType::Combobox )
        if( nPart == ControlPart::Entire )
            nType = ControlType::Editbox;

    // draw entire spinbox as a large edit box
    if( nType == ControlType::Spinbox )
        if( nPart == ControlPart::Entire )
            nType = ControlType::Editbox;

    int iPart(0), iState(0);
    if( nType == ControlType::Scrollbar )
    {
        HRESULT hr;
        if( nPart == ControlPart::ButtonUp )
        {
            iPart = SBP_ARROWBTN;
            if( nState & ControlState::PRESSED )
                iState = ABS_UPPRESSED;
            else if( !(nState & ControlState::ENABLED) )
                iState = ABS_UPDISABLED;
            else if( nState & ControlState::ROLLOVER )
                iState = ABS_UPHOT;
            else
                iState = ABS_UPNORMAL;
            hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, nullptr);
            return (hr == S_OK);
        }
        if( nPart == ControlPart::ButtonDown )
        {
            iPart = SBP_ARROWBTN;
            if( nState & ControlState::PRESSED )
                iState = ABS_DOWNPRESSED;
            else if( !(nState & ControlState::ENABLED) )
                iState = ABS_DOWNDISABLED;
            else if( nState & ControlState::ROLLOVER )
                iState = ABS_DOWNHOT;
            else
                iState = ABS_DOWNNORMAL;
            hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, nullptr);
            return (hr == S_OK);
        }
        if( nPart == ControlPart::ButtonLeft )
        {
            iPart = SBP_ARROWBTN;
            if( nState & ControlState::PRESSED )
                iState = ABS_LEFTPRESSED;
            else if( !(nState & ControlState::ENABLED) )
                iState = ABS_LEFTDISABLED;
            else if( nState & ControlState::ROLLOVER )
                iState = ABS_LEFTHOT;
            else
                iState = ABS_LEFTNORMAL;
            hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, nullptr);
            return (hr == S_OK);
        }
        if( nPart == ControlPart::ButtonRight )
        {
            iPart = SBP_ARROWBTN;
            if( nState & ControlState::PRESSED )
                iState = ABS_RIGHTPRESSED;
            else if( !(nState & ControlState::ENABLED) )
                iState = ABS_RIGHTDISABLED;
            else if( nState & ControlState::ROLLOVER )
                iState = ABS_RIGHTHOT;
            else
                iState = ABS_RIGHTNORMAL;
            hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, nullptr);
            return (hr == S_OK);
        }
        if( nPart == ControlPart::ThumbHorz || nPart == ControlPart::ThumbVert )
        {
            iPart = (nPart == ControlPart::ThumbHorz) ? SBP_THUMBBTNHORZ : SBP_THUMBBTNVERT;
            if( nState & ControlState::PRESSED )
                iState = SCRBS_PRESSED;
            else if( !(nState & ControlState::ENABLED) )
                iState = SCRBS_DISABLED;
            else if( nState & ControlState::ROLLOVER )
                iState = SCRBS_HOT;
            else
                iState = SCRBS_NORMAL;

            SIZE sz;
            vsAPI.GetThemePartSize(hTheme, hDC, iPart, iState, nullptr, TS_MIN, &sz);
            vsAPI.GetThemePartSize(hTheme, hDC, iPart, iState, nullptr, TS_TRUE, &sz);
            vsAPI.GetThemePartSize(hTheme, hDC, iPart, iState, nullptr, TS_DRAW, &sz);

            hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, nullptr);
            // paint gripper on thumb if enough space
            if( ( (nPart == ControlPart::ThumbVert) && (rc.bottom-rc.top > 12) ) ||
                ( (nPart == ControlPart::ThumbHorz) && (rc.right-rc.left > 12) ) )
            {
                iPart = (nPart == ControlPart::ThumbHorz) ? SBP_GRIPPERHORZ : SBP_GRIPPERVERT;
                iState = 0;
                vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, nullptr);
            }
            return (hr == S_OK);
        }
        if( nPart == ControlPart::TrackHorzLeft || nPart == ControlPart::TrackHorzRight || nPart == ControlPart::TrackVertUpper || nPart == ControlPart::TrackVertLower )
        {
            switch( nPart )
            {
                case ControlPart::TrackHorzLeft:  iPart = SBP_UPPERTRACKHORZ; break;
                case ControlPart::TrackHorzRight: iPart = SBP_LOWERTRACKHORZ; break;
                case ControlPart::TrackVertUpper: iPart = SBP_UPPERTRACKVERT; break;
                case ControlPart::TrackVertLower: iPart = SBP_LOWERTRACKVERT; break;
                default: break;
            }

            if( nState & ControlState::PRESSED )
                iState = SCRBS_PRESSED;
            else if( !(nState & ControlState::ENABLED) )
                iState = SCRBS_DISABLED;
            else if( nState & ControlState::ROLLOVER )
                iState = SCRBS_HOT;
            else
                iState = SCRBS_NORMAL;
            hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, nullptr);
            return (hr == S_OK);
        }
    }
    if( nType == ControlType::SpinButtons && nPart == ControlPart::AllButtons )
    {
        if( aValue.getType() == ControlType::SpinButtons )
        {
            const SpinbuttonValue* pValue = (aValue.getType() == ControlType::SpinButtons) ? static_cast<const SpinbuttonValue*>(&aValue) : nullptr;

            RECT rect;
            ImplConvertSpinbuttonValues( pValue->mnUpperPart, pValue->mnUpperState, pValue->maUpperRect, &iPart, &iState, &rect );
            bool bOk = ImplDrawTheme( hTheme, hDC, iPart, iState, rect, aCaption);

            if( bOk )
            {
                ImplConvertSpinbuttonValues( pValue->mnLowerPart, pValue->mnLowerState, pValue->maLowerRect, &iPart, &iState, &rect );
                bOk = ImplDrawTheme( hTheme, hDC, iPart, iState, rect, aCaption);
            }

            return bOk;
        }
    }
    if( nType == ControlType::Spinbox )
    {
        if( nPart == ControlPart::AllButtons )
        {
            if( aValue.getType() == ControlType::SpinButtons )
            {
                const SpinbuttonValue *pValue = static_cast<const SpinbuttonValue*>(&aValue);

                RECT rect;
                ImplConvertSpinbuttonValues( pValue->mnUpperPart, pValue->mnUpperState, pValue->maUpperRect, &iPart, &iState, &rect );
                bool bOk = ImplDrawTheme( hTheme, hDC, iPart, iState, rect, aCaption);

                if( bOk )
                {
                    ImplConvertSpinbuttonValues( pValue->mnLowerPart, pValue->mnLowerState, pValue->maLowerRect, &iPart, &iState, &rect );
                    bOk = ImplDrawTheme( hTheme, hDC, iPart, iState, rect, aCaption);
                }

                return bOk;
            }
        }

        if( nPart == ControlPart::ButtonDown )
        {
            iPart = SPNP_DOWN;
            if( nState & ControlState::PRESSED )
                iState = DNS_PRESSED;
            else if( !(nState & ControlState::ENABLED) )
                iState = DNS_DISABLED;
            else if( nState & ControlState::ROLLOVER )
                iState = DNS_HOT;
            else
                iState = DNS_NORMAL;
        }
        if( nPart == ControlPart::ButtonUp )
        {
            iPart = SPNP_UP;
            if( nState & ControlState::PRESSED )
                iState = UPS_PRESSED;
            else if( !(nState & ControlState::ENABLED) )
                iState = UPS_DISABLED;
            else if( nState & ControlState::ROLLOVER )
                iState = UPS_HOT;
            else
                iState = UPS_NORMAL;
        }
        if( nPart == ControlPart::ButtonRight )
        {
            iPart = SPNP_DOWNHORZ;
            if( nState & ControlState::PRESSED )
                iState = DNHZS_PRESSED;
            else if( !(nState & ControlState::ENABLED) )
                iState = DNHZS_DISABLED;
            else if( nState & ControlState::ROLLOVER )
                iState = DNHZS_HOT;
            else
                iState = DNHZS_NORMAL;
        }
        if( nPart == ControlPart::ButtonLeft )
        {
            iPart = SPNP_UPHORZ;
            if( nState & ControlState::PRESSED )
                iState = UPHZS_PRESSED;
            else if( !(nState & ControlState::ENABLED) )
                iState = UPHZS_DISABLED;
            else if( nState & ControlState::ROLLOVER )
                iState = UPHZS_HOT;
            else
                iState = UPHZS_NORMAL;
        }
        if( nPart == ControlPart::ButtonLeft || nPart == ControlPart::ButtonRight || nPart == ControlPart::ButtonUp || nPart == ControlPart::ButtonDown )
            return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }
    if( nType == ControlType::Combobox )
    {
        if( nPart == ControlPart::ButtonDown )
        {
            iPart = CP_DROPDOWNBUTTON;
            if( nState & ControlState::PRESSED )
                iState = CBXS_PRESSED;
            else if( !(nState & ControlState::ENABLED) )
                iState = CBXS_DISABLED;
            else if( nState & ControlState::ROLLOVER )
                iState = CBXS_HOT;
            else
                iState = CBXS_NORMAL;
            return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
        }
    }
    if( nType == ControlType::Pushbutton )
    {
        iPart = BP_PUSHBUTTON;
        if( nState & ControlState::PRESSED )
            iState = PBS_PRESSED;
        else if( !(nState & ControlState::ENABLED) )
            iState = PBS_DISABLED;
        else if( nState & ControlState::ROLLOVER )
            iState = PBS_HOT;
        else if( nState & ControlState::DEFAULT )
            iState = PBS_DEFAULTED;
        //else if( nState & ControlState::FOCUSED )
        //    iState = PBS_DEFAULTED;    // may need to draw focus rect
        else
            iState = PBS_NORMAL;

        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( nType == ControlType::Radiobutton )
    {
        iPart = BP_RADIOBUTTON;
        bool bChecked = ( aValue.getTristateVal() == ButtonValue::On );

        if( nState & ControlState::PRESSED )
            iState = bChecked ? RBS_CHECKEDPRESSED : RBS_UNCHECKEDPRESSED;
        else if( !(nState & ControlState::ENABLED) )
            iState = bChecked ? RBS_CHECKEDDISABLED : RBS_UNCHECKEDDISABLED;
        else if( nState & ControlState::ROLLOVER )
            iState = bChecked ? RBS_CHECKEDHOT : RBS_UNCHECKEDHOT;
        else
            iState = bChecked ? RBS_CHECKEDNORMAL : RBS_UNCHECKEDNORMAL;

        //if( nState & ControlState::FOCUSED )
        //    iState |= PBS_DEFAULTED;    // may need to draw focus rect

        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( nType == ControlType::Checkbox )
    {
        iPart = BP_CHECKBOX;
        ButtonValue v = aValue.getTristateVal();

        if( nState & ControlState::PRESSED )
            iState = (v == ButtonValue::On)  ? CBS_CHECKEDPRESSED :
                    ( (v == ButtonValue::Off) ? CBS_UNCHECKEDPRESSED : CBS_MIXEDPRESSED );
        else if( !(nState & ControlState::ENABLED) )
            iState = (v == ButtonValue::On)  ? CBS_CHECKEDDISABLED :
                    ( (v == ButtonValue::Off) ? CBS_UNCHECKEDDISABLED : CBS_MIXEDDISABLED );
        else if( nState & ControlState::ROLLOVER )
            iState = (v == ButtonValue::On)  ? CBS_CHECKEDHOT :
                    ( (v == ButtonValue::Off) ? CBS_UNCHECKEDHOT : CBS_MIXEDHOT );
        else
            iState = (v == ButtonValue::On)  ? CBS_CHECKEDNORMAL :
                    ( (v == ButtonValue::Off) ? CBS_UNCHECKEDNORMAL : CBS_MIXEDNORMAL );

        //if( nState & ControlState::FOCUSED )
        //    iState |= PBS_DEFAULTED;    // may need to draw focus rect

        //SIZE sz;
        //THEMESIZE eSize = TS_DRAW; // TS_MIN, TS_TRUE, TS_DRAW
        //vsAPI.GetThemePartSize( hTheme, hDC, iPart, iState, &rc, eSize, &sz);

        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( ( nType == ControlType::Editbox ) || ( nType == ControlType::MultilineEditbox ) )
    {
        iPart = EP_EDITTEXT;
        if( !(nState & ControlState::ENABLED) )
            iState = ETS_DISABLED;
        else if( nState & ControlState::FOCUSED )
            iState = ETS_FOCUSED;
        else if( nState & ControlState::ROLLOVER )
            iState = ETS_HOT;
        else
            iState = ETS_NORMAL;

        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( nType == ControlType::Listbox )
    {
        if( nPart == ControlPart::Entire || nPart == ControlPart::ListboxWindow )
        {
            iPart = LVP_EMPTYTEXT; // ??? no idea which part to choose here
            return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
        }
    }

    if( nType == ControlType::TabPane )
    {
        iPart = TABP_PANE;
        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( nType == ControlType::TabBody )
    {
        iPart = TABP_BODY;
        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( nType == ControlType::TabItem )
    {
        iPart = TABP_TABITEMLEFTEDGE;
        rc.bottom--;

        OSL_ASSERT( aValue.getType() == ControlType::TabItem );

        const TabitemValue& rValue = static_cast<const TabitemValue&>(aValue);
        if (rValue.isBothAligned())
        {
            iPart = TABP_TABITEMLEFTEDGE;
            rc.right--;
        }
        else if (rValue.isLeftAligned())
            iPart = TABP_TABITEMLEFTEDGE;
        else if (rValue.isRightAligned())
            iPart = TABP_TABITEMRIGHTEDGE;
        else iPart = TABP_TABITEM;

        if( !(nState & ControlState::ENABLED) )
            iState = TILES_DISABLED;
        else if( nState & ControlState::SELECTED )
        {
            iState = TILES_SELECTED;
            // increase the selected tab
            rc.left-=2;
            if (rValue.isBothAligned())
            {
                if (rValue.isLeftAligned() || rValue.isNotAligned())
                    rc.right+=2;
                if (rValue.isRightAligned())
                    rc.right+=1;
            }
            rc.top-=2;
            rc.bottom+=2;
        }
        else if( nState & ControlState::ROLLOVER )
            iState = TILES_HOT;
        else if( nState & ControlState::FOCUSED )
            iState = TILES_FOCUSED;    // may need to draw focus rect
        else
            iState = TILES_NORMAL;
        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( nType == ControlType::Toolbar )
    {
        if( nPart == ControlPart::Button )
        {
            iPart = TP_BUTTON;
            bool bChecked = ( aValue.getTristateVal() == ButtonValue::On );
            if( !(nState & ControlState::ENABLED) )
                //iState = TS_DISABLED;
                // disabled buttons are typically not painted at all but we need visual
                // feedback when travelling by keyboard over disabled entries
                iState = TS_HOT;
            else if( nState & ControlState::PRESSED )
                iState = TS_PRESSED;
            else if( nState & ControlState::ROLLOVER )
                iState = bChecked ? TS_HOTCHECKED : TS_HOT;
            else
                iState = bChecked ? TS_CHECKED : TS_NORMAL;
            return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
        }
        else if( nPart == ControlPart::ThumbHorz || nPart == ControlPart::ThumbVert )
        {
            // the vertical gripper is not supported in most themes and it makes no
            // sense to only support horizontal gripper
            //iPart = (nPart == ControlPart::ThumbHorz) ? RP_GRIPPERVERT : RP_GRIPPER;
            //return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
        }
        else if( nPart == ControlPart::DrawBackgroundHorz || nPart == ControlPart::DrawBackgroundVert )
        {
            if( aValue.getType() == ControlType::Toolbar )
            {
                const ToolbarValue *pValue = static_cast<const ToolbarValue*>(&aValue);
                if( pValue->mbIsTopDockingArea )
                    rc.top = 0; // extend potential gradient to cover menu bar as well
            }

            // make it more compatible with Aero
            if( ImplGetSVData()->maNWFData.mbDockingAreaAvoidTBFrames )
            {
                impl_drawAeroToolbar( hDC, rc, nPart == ControlPart::DrawBackgroundHorz );
                return true;
            }

            return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
        }
    }

    if( nType == ControlType::Menubar )
    {
        if( nPart == ControlPart::Entire )
        {
            if( aValue.getType() == ControlType::Menubar )
            {
                const MenubarValue *pValue = static_cast<const MenubarValue*>(&aValue);
                rc.bottom += pValue->maTopDockingAreaHeight;    // extend potential gradient to cover docking area as well

                // make it more compatible with Aero
                if( ImplGetSVData()->maNWFData.mbDockingAreaAvoidTBFrames )
                {
                    impl_drawAeroToolbar( hDC, rc, true );
                    return true;
                }
            }
            return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
        }
        else if( nPart == ControlPart::MenuItem )
        {
            if( nState & ControlState::ENABLED )
            {
                if( nState & ControlState::SELECTED )
                    iState = MBI_PUSHED;
                else if( nState & ControlState::ROLLOVER )
                    iState = MBI_HOT;
                else
                    iState = MBI_NORMAL;
            }
            else
            {
                if( nState & ControlState::SELECTED )
                    iState = MBI_DISABLEDPUSHED;
                else if( nState & ControlState::ROLLOVER )
                    iState = MBI_DISABLEDHOT;
                else
                    iState = MBI_DISABLED;
            }
            return ImplDrawTheme( hTheme, hDC, MENU_BARITEM, iState, rc, aCaption );
        }
    }

    if( nType == ControlType::Progress )
    {
        if( nPart != ControlPart::Entire )
            return FALSE;

        if( ! ImplDrawTheme( hTheme, hDC, PP_BAR, iState, rc, aCaption) )
            return false;
        RECT aProgressRect = rc;
        if( vsAPI.GetThemeBackgroundContentRect( hTheme, hDC, PP_BAR, iState, &rc, &aProgressRect) != S_OK )
            return false;

        long nProgressWidth = aValue.getNumericVal();
        nProgressWidth *= (aProgressRect.right - aProgressRect.left);
        nProgressWidth /= (rc.right - rc.left);
        if( AllSettings::GetLayoutRTL() )
            aProgressRect.left = aProgressRect.right - nProgressWidth;
        else
            aProgressRect.right = aProgressRect.left + nProgressWidth;

        return ImplDrawTheme( hTheme, hDC, PP_CHUNK, iState, aProgressRect, aCaption );
    }

    if( nType == ControlType::Slider )
    {
        iPart = (nPart == ControlPart::TrackHorzArea) ? TKP_TRACK : TKP_TRACKVERT;
        iState = (nPart == ControlPart::TrackHorzArea) ? static_cast<int>(TRS_NORMAL) : static_cast<int>(TRVS_NORMAL);

        tools::Rectangle aTrackRect = ImplGetThemeRect( hTheme, hDC, iPart, iState, tools::Rectangle() );
        RECT aTRect = rc;
        if( nPart == ControlPart::TrackHorzArea )
        {
            long nH = aTrackRect.GetHeight();
            aTRect.top += (rc.bottom - rc.top - nH)/2;
            aTRect.bottom = aTRect.top + nH;
        }
        else
        {
            long nW = aTrackRect.GetWidth();
            aTRect.left += (rc.right - rc.left - nW)/2;
            aTRect.right = aTRect.left + nW;
        }
        ImplDrawTheme( hTheme, hDC, iPart, iState, aTRect, aCaption );

        RECT aThumbRect;
        OSL_ASSERT( aValue.getType() == ControlType::Slider );
        const SliderValue* pVal = static_cast<const SliderValue*>(&aValue);
        aThumbRect.left   = pVal->maThumbRect.Left();
        aThumbRect.top    = pVal->maThumbRect.Top();
        aThumbRect.right  = pVal->maThumbRect.Right();
        aThumbRect.bottom = pVal->maThumbRect.Bottom();
        iPart = (nPart == ControlPart::TrackHorzArea) ? TKP_THUMB : TKP_THUMBVERT;
        iState = (nState & ControlState::ENABLED) ? TUS_NORMAL : TUS_DISABLED;
        return ImplDrawTheme( hTheme, hDC, iPart, iState, aThumbRect, aCaption );
    }

    if( nType == ControlType::ListNode )
    {
        if( nPart != ControlPart::Entire )
            return FALSE;

        ButtonValue aButtonValue = aValue.getTristateVal();
        iPart = TVP_GLYPH;
        switch( aButtonValue )
        {
        case ButtonValue::On:
            iState = GLPS_OPENED;
            break;
        case ButtonValue::Off:
            iState = GLPS_CLOSED;
            break;
        default:
            return FALSE;
        }
        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption );
    }

    if( GetSalData()->mbThemeMenuSupport )
    {
        if( nType == ControlType::MenuPopup )
        {
            if( nPart == ControlPart::Entire )
            {
                RECT aGutterRC = rc;
                if( AllSettings::GetLayoutRTL() )
                {
                    aGutterRC.right -= aValue.getNumericVal()+1;
                    aGutterRC.left = aGutterRC.right-3;
                }
                else
                {
                    aGutterRC.left += aValue.getNumericVal();
                    aGutterRC.right = aGutterRC.left+3;
                }
                return
                ImplDrawTheme( hTheme, hDC, MENU_POPUPBACKGROUND, 0, rc, aCaption ) &&
                ImplDrawTheme( hTheme, hDC, MENU_POPUPGUTTER, 0, aGutterRC, aCaption )
                ;
            }
            else if( nPart == ControlPart::MenuItem )
            {
                if( nState & ControlState::ENABLED )
                    iState = (nState & ControlState::SELECTED) ? MPI_HOT : MPI_NORMAL;
                else
                    iState = (nState & ControlState::SELECTED) ? MPI_DISABLEDHOT : MPI_DISABLED;
                return ImplDrawTheme( hTheme, hDC, MENU_POPUPITEM, iState, rc, aCaption );
            }
            else if( nPart == ControlPart::MenuItemCheckMark || nPart == ControlPart::MenuItemRadioMark )
            {
                if( nState & ControlState::PRESSED )
                {
                    RECT aBGRect = rc;
                    if( aValue.getType() == ControlType::MenuPopup )
                    {
                        tools::Rectangle aRectangle = GetMenuPopupMarkRegion(aValue);
                        aBGRect.top = aRectangle.Top();
                        aBGRect.left = aRectangle.Left();
                        aBGRect.bottom = aRectangle.Bottom();
                        aBGRect.right = aRectangle.Right();
                        rc = aBGRect;
                    }
                    iState = (nState & ControlState::ENABLED) ? MCB_NORMAL : MCB_DISABLED;
                    ImplDrawTheme( hTheme, hDC, MENU_POPUPCHECKBACKGROUND, iState, aBGRect, aCaption );
                    if( nPart == ControlPart::MenuItemCheckMark )
                        iState = (nState & ControlState::ENABLED) ? MC_CHECKMARKNORMAL : MC_CHECKMARKDISABLED;
                    else
                        iState = (nState & ControlState::ENABLED) ? MC_BULLETNORMAL : MC_BULLETDISABLED;
                    return ImplDrawTheme( hTheme, hDC, MENU_POPUPCHECK, iState, rc, aCaption );
                }
                else
                    return true; // unchecked: do nothing
            }
            else if( nPart == ControlPart::Separator )
            {
                // adjust for gutter position
                if( AllSettings::GetLayoutRTL() )
                    rc.right -= aValue.getNumericVal()+1;
                else
                    rc.left += aValue.getNumericVal()+1;
                tools::Rectangle aRect( ImplGetThemeRect( hTheme, hDC,
                    MENU_POPUPSEPARATOR, 0, tools::Rectangle( rc.left, rc.top, rc.right, rc.bottom ) ) );
                // center the separator inside the passed rectangle
                long nDY = ((rc.bottom - rc.top + 1) - aRect.GetHeight()) / 2;
                rc.top += nDY;
                rc.bottom = rc.top+aRect.GetHeight()-1;
                return ImplDrawTheme( hTheme, hDC, MENU_POPUPSEPARATOR, 0, rc, aCaption );
            }
        }
    }

    return false;
}

bool WinSalGraphics::drawNativeControl( ControlType nType,
                                        ControlPart nPart,
                                        const tools::Rectangle& rControlRegion,
                                        ControlState nState,
                                        const ImplControlValue& aValue,
                                        const OUString& aCaption )
{
    bool bOk = false;
    HTHEME hTheme = nullptr;

    tools::Rectangle buttonRect = rControlRegion;
    tools::Rectangle cacheRect = rControlRegion;
    Size keySize = cacheRect.GetSize();

    WinOpenGLSalGraphicsImpl* pImpl = dynamic_cast<WinOpenGLSalGraphicsImpl*>(mpImpl.get());

    // tdf#95618 - A few controls render outside the region they're given.
    if (pImpl && nType == ControlType::TabItem)
    {
        tools::Rectangle rNativeBoundingRegion;
        tools::Rectangle rNativeContentRegion;
        if (getNativeControlRegion(nType, nPart, rControlRegion, nState, aValue, aCaption,
                                   rNativeBoundingRegion, rNativeContentRegion))
        {
            cacheRect = rNativeBoundingRegion;
            keySize = rNativeBoundingRegion.GetSize();
        }
    }

    if (pImpl && nType == ControlType::MenuPopup && (nPart == ControlPart::MenuItemCheckMark || nPart == ControlPart::MenuItemRadioMark))
    {
        tools::Rectangle aRectangle = GetMenuPopupMarkRegion(aValue);
        if (!aRectangle.IsEmpty())
        {
            cacheRect = GetMenuPopupMarkRegion(aValue);
            buttonRect = cacheRect;
            keySize = cacheRect.GetSize();
        }
    }


    ControlCacheKey aControlCacheKey(nType, nPart, nState, keySize);
    if (pImpl != nullptr && pImpl->TryRenderCachedNativeControl(aControlCacheKey, buttonRect.Left(), buttonRect.Top()))
    {
        return true;
    }

    switch( nType )
    {
        case ControlType::Pushbutton:
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
            hTheme = getThemeHandle( mhWnd, L"Button");
            break;
        case ControlType::Scrollbar:
            hTheme = getThemeHandle( mhWnd, L"Scrollbar");
            break;
        case ControlType::Combobox:
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"Edit");
            else if( nPart == ControlPart::ButtonDown )
                hTheme = getThemeHandle( mhWnd, L"Combobox");
            break;
        case ControlType::Spinbox:
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"Edit");
            else
                hTheme = getThemeHandle( mhWnd, L"Spin");
            break;
        case ControlType::SpinButtons:
            hTheme = getThemeHandle( mhWnd, L"Spin");
            break;
        case ControlType::Editbox:
        case ControlType::MultilineEditbox:
            hTheme = getThemeHandle( mhWnd, L"Edit");
            break;
        case ControlType::Listbox:
            if( nPart == ControlPart::Entire || nPart == ControlPart::ListboxWindow )
                hTheme = getThemeHandle( mhWnd, L"Listview");
            else if( nPart == ControlPart::ButtonDown )
                hTheme = getThemeHandle( mhWnd, L"Combobox");
            break;
        case ControlType::TabPane:
        case ControlType::TabBody:
        case ControlType::TabItem:
            hTheme = getThemeHandle( mhWnd, L"Tab");
            break;
        case ControlType::Toolbar:
            if( nPart == ControlPart::Entire || nPart == ControlPart::Button )
                hTheme = getThemeHandle( mhWnd, L"Toolbar");
            else
                // use rebar for grip and background
                hTheme = getThemeHandle( mhWnd, L"Rebar");
            break;
        case ControlType::Menubar:
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"Rebar");
            else if( GetSalData()->mbThemeMenuSupport )
            {
                if( nPart == ControlPart::MenuItem )
                    hTheme = getThemeHandle( mhWnd, L"Menu" );
            }
            break;
        case ControlType::Progress:
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"Progress");
            break;
        case ControlType::ListNode:
            if( nPart == ControlPart::Entire )
                hTheme = getThemeHandle( mhWnd, L"TreeView");
            break;
        case ControlType::Slider:
            if( nPart == ControlPart::TrackHorzArea || nPart == ControlPart::TrackVertArea )
                hTheme = getThemeHandle( mhWnd, L"Trackbar" );
            break;
        case ControlType::MenuPopup:
            if( GetSalData()->mbThemeMenuSupport )
            {
                if( nPart == ControlPart::Entire || nPart == ControlPart::MenuItem ||
                    nPart == ControlPart::MenuItemCheckMark || nPart == ControlPart::MenuItemRadioMark ||
                    nPart == ControlPart::Separator
                    )
                    hTheme = getThemeHandle( mhWnd, L"Menu" );
            }
            break;
        default:
            hTheme = nullptr;
            break;
    }

    if( !hTheme )
        return false;

    RECT rc;
    rc.left   = buttonRect.Left();
    rc.right  = buttonRect.Right()+1;
    rc.top    = buttonRect.Top();
    rc.bottom = buttonRect.Bottom()+1;

    OUString aCaptionStr(aCaption.replace('~', '&')); // translate mnemonics

    if (pImpl == nullptr)
    {
        // set default text alignment
        int ta = SetTextAlign(getHDC(), TA_LEFT|TA_TOP|TA_NOUPDATECP);

        bOk = ImplDrawNativeControl(getHDC(), hTheme, rc, nType, nPart, nState, aValue, aCaptionStr);

        // restore alignment
        SetTextAlign(getHDC(), ta);
    }
    else
    {
        // We can do OpenGL
        OpenGLCompatibleDC aBlackDC(*this, cacheRect.Left(), cacheRect.Top(), cacheRect.GetWidth()+1, cacheRect.GetHeight()+1);
        SetTextAlign(aBlackDC.getCompatibleHDC(), TA_LEFT|TA_TOP|TA_NOUPDATECP);
        aBlackDC.fill(RGB(0, 0, 0));

        OpenGLCompatibleDC aWhiteDC(*this, cacheRect.Left(), cacheRect.Top(), cacheRect.GetWidth()+1, cacheRect.GetHeight()+1);
        SetTextAlign(aWhiteDC.getCompatibleHDC(), TA_LEFT|TA_TOP|TA_NOUPDATECP);
        aWhiteDC.fill(RGB(0xff, 0xff, 0xff));

        if (ImplDrawNativeControl(aBlackDC.getCompatibleHDC(), hTheme, rc, nType, nPart, nState, aValue, aCaptionStr) &&
            ImplDrawNativeControl(aWhiteDC.getCompatibleHDC(), hTheme, rc, nType, nPart, nState, aValue, aCaptionStr))
        {
            bOk = pImpl->RenderAndCacheNativeControl(aWhiteDC, aBlackDC, cacheRect.Left(), cacheRect.Top(), aControlCacheKey);
        }
    }

    return bOk;
}

bool WinSalGraphics::getNativeControlRegion(  ControlType nType,
                                ControlPart nPart,
                                const tools::Rectangle& rControlRegion,
                                ControlState nState,
                                const ImplControlValue& rControlValue,
                                const OUString&,
                                tools::Rectangle &rNativeBoundingRegion,
                                tools::Rectangle &rNativeContentRegion )
{
    bool bRet = FALSE;

    // FIXME: rNativeBoundingRegion has a different origin
    //        depending on which part is used; horrors.

    HDC hDC = GetDC( mhWnd );
    if( nType == ControlType::Toolbar )
    {
        if( nPart == ControlPart::ThumbHorz || nPart == ControlPart::ThumbVert )
        {
            /*
            // the vertical gripper is not supported in most themes and it makes no
            // sense to only support horizontal gripper

            HTHEME hTheme = getThemeHandle( mhWnd, L"Rebar");
            if( hTheme )
            {
            tools::Rectangle aRect( ImplGetThemeRect( hTheme, hDC, nPart == ControlPart::ThumbHorz ? RP_GRIPPERVERT : RP_GRIPPER,
                    0, rControlRegion.GetBoundRect() ) );
                if( nPart == ControlPart::ThumbHorz && !aRect.IsEmpty() )
                {
                    tools::Rectangle aVertRect( 0, 0, aRect.getHeight(), aRect.getWidth() );
                    rNativeContentRegion = aVertRect;
                }
                else
                    rNativeContentRegion = aRect;
                rNativeBoundingRegion = rNativeContentRegion;
                if( !rNativeContentRegion.IsEmpty() )
                    bRet = TRUE;
            }
            */
        }
        if( nPart == ControlPart::Button )
        {
            HTHEME hTheme = getThemeHandle( mhWnd, L"Toolbar");
            if( hTheme )
            {
                tools::Rectangle aRect( ImplGetThemeRect( hTheme, hDC, TP_SPLITBUTTONDROPDOWN,
                    TS_HOT, rControlRegion ) );
                rNativeContentRegion = aRect;
                rNativeBoundingRegion = rNativeContentRegion;
                if( !rNativeContentRegion.IsEmpty() )
                    bRet = TRUE;
            }
        }
    }
    if( nType == ControlType::Progress && nPart == ControlPart::Entire )
    {
        HTHEME hTheme = getThemeHandle( mhWnd, L"Progress");
        if( hTheme )
        {
            tools::Rectangle aRect( ImplGetThemeRect( hTheme, hDC, PP_BAR,
                0, rControlRegion ) );
            rNativeContentRegion = aRect;
            rNativeBoundingRegion = rNativeContentRegion;
            if( !rNativeContentRegion.IsEmpty() )
                bRet = TRUE;
        }
    }
    if( (nType == ControlType::Listbox || nType == ControlType::Combobox ) && nPart == ControlPart::Entire )
    {
        HTHEME hTheme = getThemeHandle( mhWnd, L"Combobox");
        if( hTheme )
        {
            tools::Rectangle aBoxRect( rControlRegion );
            tools::Rectangle aRect( ImplGetThemeRect( hTheme, hDC, CP_DROPDOWNBUTTON,
                                               CBXS_NORMAL, aBoxRect ) );
            if( aRect.GetHeight() > aBoxRect.GetHeight() )
                aBoxRect.SetBottom( aBoxRect.Top() + aRect.GetHeight() );
            if( aRect.GetWidth() > aBoxRect.GetWidth() )
                aBoxRect.SetRight( aBoxRect.Left() + aRect.GetWidth() );
            rNativeContentRegion = aBoxRect;
            rNativeBoundingRegion = rNativeContentRegion;
            if( !aRect.IsEmpty() )
                bRet = TRUE;
        }
    }

    if( (nType == ControlType::Editbox || nType == ControlType::Spinbox) && nPart == ControlPart::Entire )
    {
        HTHEME hTheme = getThemeHandle( mhWnd, L"Edit");
        if( hTheme )
        {
            // get border size
            tools::Rectangle aBoxRect( rControlRegion );
            tools::Rectangle aRect( ImplGetThemeRect( hTheme, hDC, EP_BACKGROUNDWITHBORDER,
                                               EBWBS_HOT, aBoxRect ) );
            // ad app font height
            NONCLIENTMETRICSW aNonClientMetrics;
            aNonClientMetrics.cbSize = sizeof( aNonClientMetrics );
            if ( SystemParametersInfoW( SPI_GETNONCLIENTMETRICS, sizeof( aNonClientMetrics ), &aNonClientMetrics, 0 ) )
            {
                long nFontHeight = aNonClientMetrics.lfMessageFont.lfHeight;
                if( nFontHeight < 0 )
                    nFontHeight = -nFontHeight;

                if( aRect.GetHeight() && nFontHeight )
                {
                    aRect.AdjustBottom(aRect.GetHeight());
                    aRect.AdjustBottom(nFontHeight);
                    if( aRect.GetHeight() > aBoxRect.GetHeight() )
                        aBoxRect.SetBottom( aBoxRect.Top() + aRect.GetHeight() );
                    if( aRect.GetWidth() > aBoxRect.GetWidth() )
                        aBoxRect.SetRight( aBoxRect.Left() + aRect.GetWidth() );
                    rNativeContentRegion = aBoxRect;
                    rNativeBoundingRegion = rNativeContentRegion;
                    bRet = TRUE;
                }
            }
        }
    }

    if( GetSalData()->mbThemeMenuSupport )
    {
        if( nType == ControlType::MenuPopup )
        {
            if( nPart == ControlPart::MenuItemCheckMark ||
                nPart == ControlPart::MenuItemRadioMark )
            {
                HTHEME hTheme = getThemeHandle( mhWnd, L"Menu");
                tools::Rectangle aBoxRect( rControlRegion );
                tools::Rectangle aRect( ImplGetThemeRect( hTheme, hDC,
                    MENU_POPUPCHECK,
                    MC_CHECKMARKNORMAL,
                    aBoxRect ) );
                if( aBoxRect.GetWidth() && aBoxRect.GetHeight() )
                {
                    rNativeContentRegion = aRect;
                    rNativeBoundingRegion = rNativeContentRegion;
                    bRet = TRUE;
                }
            }
        }
    }

    if( nType == ControlType::Slider && ( (nPart == ControlPart::ThumbHorz) || (nPart == ControlPart::ThumbVert) ) )
    {
        HTHEME hTheme = getThemeHandle( mhWnd, L"Trackbar");
        if( hTheme )
        {
            int iPart = (nPart == ControlPart::ThumbHorz) ? TKP_THUMB : TKP_THUMBVERT;
            int iState = (nPart == ControlPart::ThumbHorz) ? static_cast<int>(TUS_NORMAL) : static_cast<int>(TUVS_NORMAL);
            tools::Rectangle aThumbRect = ImplGetThemeRect( hTheme, hDC, iPart, iState, tools::Rectangle() );
            if( nPart == ControlPart::ThumbHorz )
            {
                long nW = aThumbRect.GetWidth();
                tools::Rectangle aRect( rControlRegion );
                aRect.SetRight( aRect.Left() + nW - 1 );
                rNativeContentRegion = aRect;
                rNativeBoundingRegion = rNativeContentRegion;
            }
            else
            {
                long nH = aThumbRect.GetHeight();
                tools::Rectangle aRect( rControlRegion );
                aRect.SetBottom( aRect.Top() + nH - 1 );
                rNativeContentRegion = aRect;
                rNativeBoundingRegion = rNativeContentRegion;
            }
            bRet = TRUE;
        }
    }

    if ( ( nType == ControlType::TabItem ) && ( nPart == ControlPart::Entire ) )
    {
        tools::Rectangle aControlRect( rControlRegion );
        rNativeContentRegion = aControlRect;

        aControlRect.AdjustBottom(-1);

        if( rControlValue.getType() == ControlType::TabItem )
        {
            const TabitemValue& rValue = static_cast<const TabitemValue&>(rControlValue);
            if (rValue.isBothAligned())
                aControlRect.AdjustRight(-1);

            if ( nState & ControlState::SELECTED )
            {
                aControlRect.AdjustLeft(-2);
                if (!rValue.isBothAligned())
                {
                    if (rValue.isLeftAligned() || rValue.isNotAligned())
                        aControlRect.AdjustRight(2);
                    if (rValue.isRightAligned())
                        aControlRect.AdjustRight(1);
                }
                aControlRect.AdjustTop(-2);
                aControlRect.AdjustBottom(2);
            }
        }
        rNativeBoundingRegion = aControlRect;
        bRet = TRUE;
    }

    ReleaseDC( mhWnd, hDC );
    return bRet;
}

void WinSalGraphics::updateSettingsNative( AllSettings& rSettings )
{
    if ( !vsAPI.IsThemeActive() )
        return;

    StyleSettings aStyleSettings = rSettings.GetStyleSettings();
    ImplSVData* pSVData = ImplGetSVData();

    // don't draw frame around each and every toolbar
    pSVData->maNWFData.mbDockingAreaAvoidTBFrames = true;

    // FIXME get the color directly from the theme, not from the settings
    Color aMenuBarTextColor = aStyleSettings.GetPersonaMenuBarTextColor().get_value_or( aStyleSettings.GetMenuTextColor() );
    // in aero menuitem highlight text is drawn in the same color as normal
    aStyleSettings.SetMenuHighlightTextColor( aStyleSettings.GetMenuTextColor() );
    aStyleSettings.SetMenuBarRolloverTextColor( aMenuBarTextColor );
    aStyleSettings.SetMenuBarHighlightTextColor( aMenuBarTextColor );
    pSVData->maNWFData.mnMenuFormatBorderX = 2;
    pSVData->maNWFData.mnMenuFormatBorderY = 2;
    pSVData->maNWFData.maMenuBarHighlightTextColor = aMenuBarTextColor;
    GetSalData()->mbThemeMenuSupport = true;

    rSettings.SetStyleSettings( aStyleSettings );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
