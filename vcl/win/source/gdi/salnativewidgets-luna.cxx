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

// General info:
//   http://msdn.microsoft.com/en-us/library/windows/desktop/hh270423%28v=vs.85%29.aspx
//   http://msdn.microsoft.com/en-us/library/windows/desktop/bb773178%28v=vs.85%29.aspx
//
// Useful tool to explore the themes & their rendering:
//   http://privat.rejbrand.se/UxExplore.exe
// (found at http://stackoverflow.com/questions/4009701/windows-visual-themes-gallery-of-parts-and-states/4009712#4009712)
//
// Theme subclasses:
//   http://msdn.microsoft.com/en-us/library/windows/desktop/bb773218%28v=vs.85%29.aspx
//
// Drawing in non-client area (general DWM-related info):
//   http://msdn.microsoft.com/en-us/library/windows/desktop/bb688195%28v=vs.85%29.aspx

#include "rtl/ustring.h"

#include "osl/module.h"

#include "vcl/svapp.hxx"

#include "win/svsys.h"
#include "win/salgdi.h"
#include "win/saldata.hxx"

#include "uxtheme.h"
#include "vssym32.h"

#include <map>
#include <string>

using namespace std;

using ::rtl::OUString;

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
    sal_Bool IsAvailable()  { return (mhModule != NULL); }

    HTHEME OpenThemeData( HWND hwnd, LPCWSTR pszClassList );
    HRESULT CloseThemeData( HTHEME hTheme );
    HRESULT GetThemeBackgroundContentRect( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pBoundingRect, RECT *pContentRect );
    HRESULT DrawThemeBackground( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect );
    HRESULT DrawThemeText( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect );
    HRESULT GetThemePartSize( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, RECT *prc, THEMESIZE eSize, SIZE *psz );
    BOOL IsThemeActive( void );
};

static VisualStylesAPI vsAPI;

VisualStylesAPI::VisualStylesAPI()
    : lpfnOpenThemeData( NULL ),
      lpfnCloseThemeData( NULL ),
      lpfnGetThemeBackgroundContentRect( NULL ),
      lpfnDrawThemeBackground( NULL ),
      lpfnDrawThemeText( NULL ),
      lpfnGetThemePartSize( NULL ),
      lpfnIsThemeActive( NULL )
{
    OUString aLibraryName( RTL_CONSTASCII_USTRINGPARAM( "uxtheme.dll" ) );
    mhModule = osl_loadModule( aLibraryName.pData, SAL_LOADMODULE_DEFAULT );

    if ( mhModule )
    {
        lpfnOpenThemeData = (OpenThemeData_Proc_T)osl_getAsciiFunctionSymbol( mhModule, "OpenThemeData" );
        lpfnCloseThemeData = (CloseThemeData_Proc_T)osl_getAsciiFunctionSymbol( mhModule, "CloseThemeData" );
        lpfnGetThemeBackgroundContentRect = (GetThemeBackgroundContentRect_Proc_T)osl_getAsciiFunctionSymbol( mhModule, "GetThemeBackgroundContentRect" );
        lpfnDrawThemeBackground = (DrawThemeBackground_Proc_T)osl_getAsciiFunctionSymbol( mhModule, "DrawThemeBackground" );
        lpfnDrawThemeText = (DrawThemeText_Proc_T)osl_getAsciiFunctionSymbol( mhModule, "DrawThemeText" );
        lpfnGetThemePartSize = (GetThemePartSize_Proc_T)osl_getAsciiFunctionSymbol( mhModule, "GetThemePartSize" );
        lpfnIsThemeActive = (IsThemeActive_Proc_T)osl_getAsciiFunctionSymbol( mhModule, "IsThemeActive" );
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
        return NULL;
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

BOOL VisualStylesAPI::IsThemeActive( void )
{
    if(lpfnIsThemeActive)
        return (*lpfnIsThemeActive) ();
    else
        return FALSE;
}

/*********************************************************
 * Initialize XP theming and local stuff
 *********************************************************/
void SalData::initNWF( void )
{
    ImplSVData* pSVData = ImplGetSVData();

    // the menu bar and the top docking area should have a common background (gradient)
    pSVData->maNWFData.mbMenuBarDockingAreaCommonBG = true;
}


// *********************************************************
// * Release theming handles
// ********************************************************
void SalData::deInitNWF( void )
{
    ThemeMap::iterator iter = aThemeMap.begin();
    while( iter != aThemeMap.end() )
    {
        vsAPI.CloseThemeData(iter->second);
        ++iter;
    }
    aThemeMap.clear();
    if( maDwmLib )
        osl_unloadModule( maDwmLib );
}

static HTHEME getThemeHandle( HWND hWnd, LPCWSTR name )
{
    if( GetSalData()->mbThemeChanged )
    {
        // throw away invalid theme handles
        GetSalData()->deInitNWF();
        GetSalData()->mbThemeChanged = FALSE;
    }

    ThemeMap::iterator iter;
    if( (iter = aThemeMap.find( name )) != aThemeMap.end() )
        return iter->second;
    // theme not found -> add it to map
    HTHEME hTheme = vsAPI.OpenThemeData( hWnd, name );
    if( hTheme != NULL )
        aThemeMap[name] = hTheme;
    return hTheme;
}

/*
 * IsNativeControlSupported()
 *
 *  Returns TRUE if the platform supports native
 *  drawing of the control defined by nPart
 */
sal_Bool WinSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    HTHEME hTheme = NULL;

    switch( nType )
    {
        case CTRL_PUSHBUTTON:
        case CTRL_RADIOBUTTON:
        case CTRL_CHECKBOX:
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"Button");
            break;
        case CTRL_SCROLLBAR:
            if( nPart == PART_DRAW_BACKGROUND_HORZ || nPart == PART_DRAW_BACKGROUND_VERT )
                return FALSE;   // no background painting needed
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"Scrollbar");
            break;
        case CTRL_COMBOBOX:
            if( nPart == HAS_BACKGROUND_TEXTURE )
                return FALSE;   // we do not paint the inner part (ie the selection background/focus indication)
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"Edit");
            else if( nPart == PART_BUTTON_DOWN )
                hTheme = getThemeHandle( mhWnd, L"Combobox");
            break;
        case CTRL_SPINBOX:
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"Edit");
            else if( nPart == PART_ALL_BUTTONS ||
                nPart == PART_BUTTON_UP || nPart == PART_BUTTON_DOWN ||
                nPart == PART_BUTTON_LEFT|| nPart == PART_BUTTON_RIGHT )
                hTheme = getThemeHandle( mhWnd, L"Spin");
            break;
        case CTRL_SPINBUTTONS:
            if( nPart == PART_ENTIRE_CONTROL || nPart == PART_ALL_BUTTONS )
                hTheme = getThemeHandle( mhWnd, L"Spin");
            break;
        case CTRL_EDITBOX:
        case CTRL_MULTILINE_EDITBOX:
            if( nPart == HAS_BACKGROUND_TEXTURE )
                return FALSE;   // we do not paint the inner part (ie the selection background/focus indication)
                //return TRUE;
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"Edit");
            break;
        case CTRL_LISTBOX:
            if( nPart == HAS_BACKGROUND_TEXTURE )
                return FALSE;   // we do not paint the inner part (ie the selection background/focus indication)
            if( nPart == PART_ENTIRE_CONTROL || nPart == PART_WINDOW )
                hTheme = getThemeHandle( mhWnd, L"Listview");
            else if( nPart == PART_BUTTON_DOWN )
                hTheme = getThemeHandle( mhWnd, L"Combobox");
            break;
        case CTRL_TAB_PANE:
        case CTRL_TAB_BODY:
        case CTRL_TAB_ITEM:
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"Tab");
            break;
        case CTRL_TOOLBAR:
            if( nPart == PART_ENTIRE_CONTROL || nPart == PART_BUTTON )
                hTheme = getThemeHandle( mhWnd, L"Toolbar");
            else
                // use rebar theme for grip and background
                hTheme = getThemeHandle( mhWnd, L"Rebar");
            break;
        case CTRL_MENUBAR:
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"Rebar");
            else if( GetSalData()->mbThemeMenuSupport )
            {
                if( nPart == PART_MENU_ITEM )
                    hTheme = getThemeHandle( mhWnd, L"Menu" );
            }
            break;
        case CTRL_MENU_POPUP:
            if( GetSalData()->mbThemeMenuSupport )
            {
                if( nPart == PART_ENTIRE_CONTROL ||
                    nPart == PART_MENU_ITEM ||
                    nPart == PART_MENU_ITEM_CHECK_MARK ||
                    nPart == PART_MENU_ITEM_RADIO_MARK ||
                    nPart == PART_MENU_SEPARATOR )
                    hTheme = getThemeHandle( mhWnd, L"Menu" );
            }
            break;
        case CTRL_PROGRESS:
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"Progress");
            break;
        case CTRL_SLIDER:
            if( nPart == PART_TRACK_HORZ_AREA || nPart == PART_TRACK_VERT_AREA )
                hTheme = getThemeHandle( mhWnd, L"Trackbar" );
            break;
        case CTRL_LISTNODE:
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"TreeView" );
            break;
        default:
            hTheme = NULL;
            break;
    }

    return (hTheme != NULL);
}


/*
 * HitTestNativeControl()
 *
 *  If the return value is TRUE, bIsInside contains information whether
 *  aPos was or was not inside the native widget specified by the
 *  nType/nPart combination.
 */
sal_Bool WinSalGraphics::hitTestNativeControl( ControlType,
                              ControlPart,
                              const Rectangle&,
                              const Point&,
                              sal_Bool& )
{
    return FALSE;
}

sal_Bool ImplDrawTheme( HTHEME hTheme, HDC hDC, int iPart, int iState, RECT rc, const OUString& aStr)
{
    HRESULT hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, 0);

    if( aStr.getLength() )
    {
        RECT rcContent;
        hr = vsAPI.GetThemeBackgroundContentRect( hTheme, hDC, iPart, iState, &rc, &rcContent);
        hr = vsAPI.DrawThemeText( hTheme, hDC, iPart, iState,
            reinterpret_cast<LPCWSTR>(aStr.getStr()), -1,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE,
            0, &rcContent);
    }
    return (hr == S_OK);
}


Rectangle ImplGetThemeRect( HTHEME hTheme, HDC hDC, int iPart, int iState, const Rectangle& /* aRect */, THEMESIZE eTS = TS_TRUE )
{
    SIZE aSz;
    HRESULT hr = vsAPI.GetThemePartSize( hTheme, hDC, iPart, iState, NULL, eTS, &aSz ); // TS_TRUE returns optimal size
    if( hr == S_OK )
        return Rectangle( 0, 0, aSz.cx, aSz.cy );
    else
        return Rectangle();
}

// Helper functions
// ----

void ImplConvertSpinbuttonValues( int nControlPart, const ControlState& rState, const Rectangle& rRect,
                                 int* pLunaPart, int *pLunaState, RECT *pRect )
{
    if( nControlPart == PART_BUTTON_DOWN )
    {
        *pLunaPart = SPNP_DOWN;
        if( rState & CTRL_STATE_PRESSED )
            *pLunaState = DNS_PRESSED;
        else if( !(rState & CTRL_STATE_ENABLED) )
            *pLunaState = DNS_DISABLED;
        else if( rState & CTRL_STATE_ROLLOVER )
            *pLunaState = DNS_HOT;
        else
            *pLunaState = DNS_NORMAL;
    }
    if( nControlPart == PART_BUTTON_UP )
    {
        *pLunaPart = SPNP_UP;
        if( rState & CTRL_STATE_PRESSED )
            *pLunaState = UPS_PRESSED;
        else if( !(rState & CTRL_STATE_ENABLED) )
            *pLunaState = UPS_DISABLED;
        else if( rState & CTRL_STATE_ROLLOVER )
            *pLunaState = UPS_HOT;
        else
            *pLunaState = UPS_NORMAL;
    }
    if( nControlPart == PART_BUTTON_RIGHT )
    {
        *pLunaPart = SPNP_UPHORZ;
        if( rState & CTRL_STATE_PRESSED )
            *pLunaState = DNHZS_PRESSED;
        else if( !(rState & CTRL_STATE_ENABLED) )
            *pLunaState = DNHZS_DISABLED;
        else if( rState & CTRL_STATE_ROLLOVER )
            *pLunaState = DNHZS_HOT;
        else
            *pLunaState = DNHZS_NORMAL;
    }
    if( nControlPart == PART_BUTTON_LEFT )
    {
        *pLunaPart = SPNP_DOWNHORZ;
        if( rState & CTRL_STATE_PRESSED )
            *pLunaState = UPHZS_PRESSED;
        else if( !(rState & CTRL_STATE_ENABLED) )
            *pLunaState = UPHZS_DISABLED;
        else if( rState & CTRL_STATE_ROLLOVER )
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
void impl_drawAeroToolbar( HDC hDC, RECT rc, bool bHorizontal )
{
    if ( rc.top == 0 && bHorizontal )
    {
        const long GRADIENT_HEIGHT = 32;

        long gradient_break = rc.top;
        GRADIENT_RECT g_rect[1] = { { 0, 1 } };

        // very slow gradient at the top (if we have space for that)
        if ( rc.bottom - rc.top > GRADIENT_HEIGHT )
        {
            gradient_break = rc.bottom - GRADIENT_HEIGHT;

            TRIVERTEX vert[2] = {
                { rc.left, rc.top,          0xff00, 0xff00, 0xff00, 0xff00 },
                { rc.right, gradient_break, 0xfa00, 0xfa00, 0xfa00, 0xff00 },
            };
            GradientFill( hDC, vert, 2, g_rect, 1, GRADIENT_FILL_RECT_V );
        }

        // gradient at the bottom
        TRIVERTEX vert[2] = {
            { rc.left, gradient_break, 0xfa00, 0xfa00, 0xfa00, 0xff00 },
            { rc.right, rc.bottom,     0xf000, 0xf000, 0xf000, 0xff00 }
        };
        GradientFill( hDC, vert, 2, g_rect, 1, GRADIENT_FILL_RECT_V );
    }
    else
    {
        HBRUSH hbrush = CreateSolidBrush( RGB( 0xf0, 0xf0, 0xf0 ) );
        FillRect( hDC, &rc, hbrush );
        DeleteObject( hbrush );
    }
}

sal_Bool ImplDrawNativeControl( HDC hDC, HTHEME hTheme, RECT rc,
                            ControlType nType,
                            ControlPart nPart,
                            ControlState nState,
                            const ImplControlValue& aValue,
                            OUString aCaption )
{
    // a listbox dropdown is actually a combobox dropdown
    if( nType == CTRL_LISTBOX )
        if( nPart == PART_BUTTON_DOWN )
            nType = CTRL_COMBOBOX;

    // draw entire combobox as a large edit box
    if( nType == CTRL_COMBOBOX )
        if( nPart == PART_ENTIRE_CONTROL )
            nType = CTRL_EDITBOX;

    // draw entire spinbox as a large edit box
    if( nType == CTRL_SPINBOX )
        if( nPart == PART_ENTIRE_CONTROL )
            nType = CTRL_EDITBOX;

    int iPart(0), iState(0);
    if( nType == CTRL_SCROLLBAR )
    {
        HRESULT hr;
        if( nPart == PART_BUTTON_UP )
        {
            iPart = SBP_ARROWBTN;
            if( nState & CTRL_STATE_PRESSED )
                iState = ABS_UPPRESSED;
            else if( !(nState & CTRL_STATE_ENABLED) )
                iState = ABS_UPDISABLED;
            else if( nState & CTRL_STATE_ROLLOVER )
                iState = ABS_UPHOT;
            else
                iState = ABS_UPNORMAL;
            hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, 0);
            return (hr == S_OK);
        }
        if( nPart == PART_BUTTON_DOWN )
        {
            iPart = SBP_ARROWBTN;
            if( nState & CTRL_STATE_PRESSED )
                iState = ABS_DOWNPRESSED;
            else if( !(nState & CTRL_STATE_ENABLED) )
                iState = ABS_DOWNDISABLED;
            else if( nState & CTRL_STATE_ROLLOVER )
                iState = ABS_DOWNHOT;
            else
                iState = ABS_DOWNNORMAL;
            hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, 0);
            return (hr == S_OK);
        }
        if( nPart == PART_BUTTON_LEFT )
        {
            iPart = SBP_ARROWBTN;
            if( nState & CTRL_STATE_PRESSED )
                iState = ABS_LEFTPRESSED;
            else if( !(nState & CTRL_STATE_ENABLED) )
                iState = ABS_LEFTDISABLED;
            else if( nState & CTRL_STATE_ROLLOVER )
                iState = ABS_LEFTHOT;
            else
                iState = ABS_LEFTNORMAL;
            hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, 0);
            return (hr == S_OK);
        }
        if( nPart == PART_BUTTON_RIGHT )
        {
            iPart = SBP_ARROWBTN;
            if( nState & CTRL_STATE_PRESSED )
                iState = ABS_RIGHTPRESSED;
            else if( !(nState & CTRL_STATE_ENABLED) )
                iState = ABS_RIGHTDISABLED;
            else if( nState & CTRL_STATE_ROLLOVER )
                iState = ABS_RIGHTHOT;
            else
                iState = ABS_RIGHTNORMAL;
            hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, 0);
            return (hr == S_OK);
        }
        if( nPart == PART_THUMB_HORZ || nPart == PART_THUMB_VERT )
        {
            iPart = (nPart == PART_THUMB_HORZ) ? SBP_THUMBBTNHORZ : SBP_THUMBBTNVERT;
            if( nState & CTRL_STATE_PRESSED )
                iState = SCRBS_PRESSED;
            else if( !(nState & CTRL_STATE_ENABLED) )
                iState = SCRBS_DISABLED;
            else if( nState & CTRL_STATE_ROLLOVER )
                iState = SCRBS_HOT;
            else
                iState = SCRBS_NORMAL;

            SIZE sz;
            vsAPI.GetThemePartSize(hTheme, hDC, iPart, iState, NULL, TS_MIN, &sz);
            vsAPI.GetThemePartSize(hTheme, hDC, iPart, iState, NULL, TS_TRUE, &sz);
            vsAPI.GetThemePartSize(hTheme, hDC, iPart, iState, NULL, TS_DRAW, &sz);

            hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, 0);
            // paint gripper on thumb if enough space
            if( ( (nPart == PART_THUMB_VERT) && (rc.bottom-rc.top > 12) ) ||
                ( (nPart == PART_THUMB_HORZ) && (rc.right-rc.left > 12) ) )
            {
                iPart = (nPart == PART_THUMB_HORZ) ? SBP_GRIPPERHORZ : SBP_GRIPPERVERT;
                iState = 0;
                vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, 0);
            }
            return (hr == S_OK);
        }
        if( nPart == PART_TRACK_HORZ_LEFT || nPart == PART_TRACK_HORZ_RIGHT || nPart == PART_TRACK_VERT_UPPER || nPart == PART_TRACK_VERT_LOWER )
        {
            switch( nPart )
            {
                case PART_TRACK_HORZ_LEFT:  iPart = SBP_UPPERTRACKHORZ; break;
                case PART_TRACK_HORZ_RIGHT: iPart = SBP_LOWERTRACKHORZ; break;
                case PART_TRACK_VERT_UPPER: iPart = SBP_UPPERTRACKVERT; break;
                case PART_TRACK_VERT_LOWER: iPart = SBP_LOWERTRACKVERT; break;
            }

            if( nState & CTRL_STATE_PRESSED )
                iState = SCRBS_PRESSED;
            else if( !(nState & CTRL_STATE_ENABLED) )
                iState = SCRBS_DISABLED;
            else if( nState & CTRL_STATE_ROLLOVER )
                iState = SCRBS_HOT;
            else
                iState = SCRBS_NORMAL;
            hr = vsAPI.DrawThemeBackground( hTheme, hDC, iPart, iState, &rc, 0);
            return (hr == S_OK);
        }
    }
    if( nType == CTRL_SPINBUTTONS && nPart == PART_ALL_BUTTONS )
    {
        if( aValue.getType() == CTRL_SPINBUTTONS )
        {
            const SpinbuttonValue *pValue = static_cast<const SpinbuttonValue*>(&aValue);
            sal_Bool bOk = sal_False;

            RECT rect;
            ImplConvertSpinbuttonValues( pValue->mnUpperPart, pValue->mnUpperState, pValue->maUpperRect, &iPart, &iState, &rect );
            bOk = ImplDrawTheme( hTheme, hDC, iPart, iState, rect, aCaption);

            if( bOk )
            {
                ImplConvertSpinbuttonValues( pValue->mnLowerPart, pValue->mnLowerState, pValue->maLowerRect, &iPart, &iState, &rect );
                bOk = ImplDrawTheme( hTheme, hDC, iPart, iState, rect, aCaption);
            }

            return bOk;
        }
    }
    if( nType == CTRL_SPINBOX )
    {
        // decrease spinbutton rects a little
        //rc.right--;
        //rc.bottom--;
        if( nPart == PART_ALL_BUTTONS )
        {
            if( aValue.getType() == CTRL_SPINBUTTONS )
            {
                const SpinbuttonValue *pValue = static_cast<const SpinbuttonValue*>(&aValue);
                sal_Bool bOk = sal_False;

                RECT rect;
                ImplConvertSpinbuttonValues( pValue->mnUpperPart, pValue->mnUpperState, pValue->maUpperRect, &iPart, &iState, &rect );
                bOk = ImplDrawTheme( hTheme, hDC, iPart, iState, rect, aCaption);

                if( bOk )
                {
                    ImplConvertSpinbuttonValues( pValue->mnLowerPart, pValue->mnLowerState, pValue->maLowerRect, &iPart, &iState, &rect );
                    bOk = ImplDrawTheme( hTheme, hDC, iPart, iState, rect, aCaption);
                }

                return bOk;
            }
        }

        if( nPart == PART_BUTTON_DOWN )
        {
            iPart = SPNP_DOWN;
            if( nState & CTRL_STATE_PRESSED )
                iState = DNS_PRESSED;
            else if( !(nState & CTRL_STATE_ENABLED) )
                iState = DNS_DISABLED;
            else if( nState & CTRL_STATE_ROLLOVER )
                iState = DNS_HOT;
            else
                iState = DNS_NORMAL;
        }
        if( nPart == PART_BUTTON_UP )
        {
            iPart = SPNP_UP;
            if( nState & CTRL_STATE_PRESSED )
                iState = UPS_PRESSED;
            else if( !(nState & CTRL_STATE_ENABLED) )
                iState = UPS_DISABLED;
            else if( nState & CTRL_STATE_ROLLOVER )
                iState = UPS_HOT;
            else
                iState = UPS_NORMAL;
        }
        if( nPart == PART_BUTTON_RIGHT )
        {
            iPart = SPNP_DOWNHORZ;
            if( nState & CTRL_STATE_PRESSED )
                iState = DNHZS_PRESSED;
            else if( !(nState & CTRL_STATE_ENABLED) )
                iState = DNHZS_DISABLED;
            else if( nState & CTRL_STATE_ROLLOVER )
                iState = DNHZS_HOT;
            else
                iState = DNHZS_NORMAL;
        }
        if( nPart == PART_BUTTON_LEFT )
        {
            iPart = SPNP_UPHORZ;
            if( nState & CTRL_STATE_PRESSED )
                iState = UPHZS_PRESSED;
            else if( !(nState & CTRL_STATE_ENABLED) )
                iState = UPHZS_DISABLED;
            else if( nState & CTRL_STATE_ROLLOVER )
                iState = UPHZS_HOT;
            else
                iState = UPHZS_NORMAL;
        }
        if( nPart == PART_BUTTON_LEFT || nPart == PART_BUTTON_RIGHT || nPart == PART_BUTTON_UP || nPart == PART_BUTTON_DOWN )
            return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }
    if( nType == CTRL_COMBOBOX )
    {
        if( nPart == PART_BUTTON_DOWN )
        {
            iPart = CP_DROPDOWNBUTTON;
            if( nState & CTRL_STATE_PRESSED )
                iState = CBXS_PRESSED;
            else if( !(nState & CTRL_STATE_ENABLED) )
                iState = CBXS_DISABLED;
            else if( nState & CTRL_STATE_ROLLOVER )
                iState = CBXS_HOT;
            else
                iState = CBXS_NORMAL;
            return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
        }
    }
    if( nType == CTRL_PUSHBUTTON )
    {
        iPart = BP_PUSHBUTTON;
        if( nState & CTRL_STATE_PRESSED )
            iState = PBS_PRESSED;
        else if( !(nState & CTRL_STATE_ENABLED) )
            iState = PBS_DISABLED;
        else if( nState & CTRL_STATE_ROLLOVER )
            iState = PBS_HOT;
        else if( nState & CTRL_STATE_DEFAULT )
            iState = PBS_DEFAULTED;
        //else if( nState & CTRL_STATE_FOCUSED )
        //    iState = PBS_DEFAULTED;    // may need to draw focus rect
        else
            iState = PBS_NORMAL;

        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( nType == CTRL_RADIOBUTTON )
    {
        iPart = BP_RADIOBUTTON;
        sal_Bool bChecked = ( aValue.getTristateVal() == BUTTONVALUE_ON );

        if( nState & CTRL_STATE_PRESSED )
            iState = bChecked ? RBS_CHECKEDPRESSED : RBS_UNCHECKEDPRESSED;
        else if( !(nState & CTRL_STATE_ENABLED) )
            iState = bChecked ? RBS_CHECKEDDISABLED : RBS_UNCHECKEDDISABLED;
        else if( nState & CTRL_STATE_ROLLOVER )
            iState = bChecked ? RBS_CHECKEDHOT : RBS_UNCHECKEDHOT;
        else
            iState = bChecked ? RBS_CHECKEDNORMAL : RBS_UNCHECKEDNORMAL;

        //if( nState & CTRL_STATE_FOCUSED )
        //    iState |= PBS_DEFAULTED;    // may need to draw focus rect

        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( nType == CTRL_CHECKBOX )
    {
        iPart = BP_CHECKBOX;
        ButtonValue v = aValue.getTristateVal();

        if( nState & CTRL_STATE_PRESSED )
            iState = (v == BUTTONVALUE_ON)  ? CBS_CHECKEDPRESSED :
                    ( (v == BUTTONVALUE_OFF) ? CBS_UNCHECKEDPRESSED : CBS_MIXEDPRESSED );
        else if( !(nState & CTRL_STATE_ENABLED) )
            iState = (v == BUTTONVALUE_ON)  ? CBS_CHECKEDDISABLED :
                    ( (v == BUTTONVALUE_OFF) ? CBS_UNCHECKEDDISABLED : CBS_MIXEDDISABLED );
        else if( nState & CTRL_STATE_ROLLOVER )
            iState = (v == BUTTONVALUE_ON)  ? CBS_CHECKEDHOT :
                    ( (v == BUTTONVALUE_OFF) ? CBS_UNCHECKEDHOT : CBS_MIXEDHOT );
        else
            iState = (v == BUTTONVALUE_ON)  ? CBS_CHECKEDNORMAL :
                    ( (v == BUTTONVALUE_OFF) ? CBS_UNCHECKEDNORMAL : CBS_MIXEDNORMAL );

        //if( nState & CTRL_STATE_FOCUSED )
        //    iState |= PBS_DEFAULTED;    // may need to draw focus rect

        //SIZE sz;
        //THEMESIZE eSize = TS_DRAW; // TS_MIN, TS_TRUE, TS_DRAW
        //vsAPI.GetThemePartSize( hTheme, hDC, iPart, iState, &rc, eSize, &sz);

        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( ( nType == CTRL_EDITBOX ) || ( nType == CTRL_MULTILINE_EDITBOX ) )
    {
        iPart = EP_EDITTEXT;
        if( !(nState & CTRL_STATE_ENABLED) )
            iState = ETS_DISABLED;
        else if( nState & CTRL_STATE_FOCUSED )
            iState = ETS_FOCUSED;
        else if( nState & CTRL_STATE_ROLLOVER )
            iState = ETS_HOT;
        else
            iState = ETS_NORMAL;

        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( nType == CTRL_LISTBOX )
    {
        if( nPart == PART_ENTIRE_CONTROL || nPart == PART_WINDOW )
        {
            iPart = LVP_EMPTYTEXT; // ??? no idea which part to choose here
            return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
        }
    }

    if( nType == CTRL_TAB_PANE )
    {
        iPart = TABP_PANE;
        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( nType == CTRL_TAB_BODY )
    {
        iPart = TABP_BODY;
        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( nType == CTRL_TAB_ITEM )
    {
        iPart = TABP_TABITEMLEFTEDGE;
        rc.bottom--;

        OSL_ASSERT( aValue.getType() == CTRL_TAB_ITEM );

        const TabitemValue *pValue = static_cast<const TabitemValue*>(&aValue);
        if( pValue->isBothAligned() )
        {
            iPart = TABP_TABITEMLEFTEDGE;
            rc.right--;
        }
        else if( pValue->isLeftAligned() )
            iPart = TABP_TABITEMLEFTEDGE;
        else if( pValue->isRightAligned() )
            iPart = TABP_TABITEMRIGHTEDGE;
        else iPart = TABP_TABITEM;

        if( !(nState & CTRL_STATE_ENABLED) )
            iState = TILES_DISABLED;
        else if( nState & CTRL_STATE_SELECTED )
        {
            iState = TILES_SELECTED;
            // increase the selected tab
            rc.left-=2;
            if( pValue && !pValue->isBothAligned() )
            {
                if( pValue->isLeftAligned() || pValue->isNotAligned() )
                    rc.right+=2;
                if( pValue->isRightAligned() )
                    rc.right+=1;
            }
            rc.top-=2;
            rc.bottom+=2;
        }
        else if( nState & CTRL_STATE_ROLLOVER )
            iState = TILES_HOT;
        else if( nState & CTRL_STATE_FOCUSED )
            iState = TILES_FOCUSED;    // may need to draw focus rect
        else
            iState = TILES_NORMAL;
        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
    }

    if( nType == CTRL_TOOLBAR )
    {
        if( nPart == PART_BUTTON )
        {
            iPart = TP_BUTTON;
            sal_Bool bChecked = ( aValue.getTristateVal() == BUTTONVALUE_ON );
            if( !(nState & CTRL_STATE_ENABLED) )
                //iState = TS_DISABLED;
                // disabled buttons are typically not painted at all but we need visual
                // feedback when travelling by keyboard over disabled entries
                iState = TS_HOT;
            else if( nState & CTRL_STATE_PRESSED )
                iState = TS_PRESSED;
            else if( nState & CTRL_STATE_ROLLOVER )
                iState = bChecked ? TS_HOTCHECKED : TS_HOT;
            else
                iState = bChecked ? TS_CHECKED : TS_NORMAL;
            return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
        }
        else if( nPart == PART_THUMB_HORZ || nPart == PART_THUMB_VERT )
        {
            // the vertical gripper is not supported in most themes and it makes no
            // sense to only support horizontal gripper
            //iPart = (nPart == PART_THUMB_HORZ) ? RP_GRIPPERVERT : RP_GRIPPER;
            //return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
        }
        else if( nPart == PART_DRAW_BACKGROUND_HORZ || nPart == PART_DRAW_BACKGROUND_VERT )
        {
            if( aValue.getType() == CTRL_TOOLBAR )
            {
                const ToolbarValue *pValue = static_cast<const ToolbarValue*>(&aValue);
                if( pValue->mbIsTopDockingArea )
                    rc.top = 0; // extend potential gradient to cover menu bar as well
            }

            // make it more compatible with Aero
            if( ImplGetSVData()->maNWFData.mbDockingAreaAvoidTBFrames )
            {
                impl_drawAeroToolbar( hDC, rc, nPart == PART_DRAW_BACKGROUND_HORZ );
                return sal_True;
            }

            return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
        }
    }

    if( nType == CTRL_MENUBAR )
    {
        if( nPart == PART_ENTIRE_CONTROL )
        {
            if( aValue.getType() == CTRL_MENUBAR )
            {
                const MenubarValue *pValue = static_cast<const MenubarValue*>(&aValue);
                rc.bottom += pValue->maTopDockingAreaHeight;    // extend potential gradient to cover docking area as well

                // make it more compatible with Aero
                if( ImplGetSVData()->maNWFData.mbDockingAreaAvoidTBFrames )
                {
                    impl_drawAeroToolbar( hDC, rc, true );
                    return sal_True;
                }
            }
            return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption);
        }
        else if( nPart == PART_MENU_ITEM )
        {
            if( (nState & CTRL_STATE_ENABLED) )
                iState = (nState & CTRL_STATE_SELECTED) ? MBI_HOT : MBI_NORMAL;
            else
                iState = (nState & CTRL_STATE_SELECTED) ? MBI_DISABLEDHOT : MBI_DISABLED;
            return ImplDrawTheme( hTheme, hDC, MENU_BARITEM, iState, rc, aCaption );
        }
    }

    if( nType == CTRL_PROGRESS )
    {
        if( nPart != PART_ENTIRE_CONTROL )
            return FALSE;

        if( ! ImplDrawTheme( hTheme, hDC, PP_BAR, iState, rc, aCaption) )
            return false;
        RECT aProgressRect = rc;
        if( vsAPI.GetThemeBackgroundContentRect( hTheme, hDC, PP_BAR, iState, &rc, &aProgressRect) != S_OK )
            return false;

        long nProgressWidth = aValue.getNumericVal();
        nProgressWidth *= (aProgressRect.right - aProgressRect.left);
        nProgressWidth /= (rc.right - rc.left);
        if( Application::GetSettings().GetLayoutRTL() )
            aProgressRect.left = aProgressRect.right - nProgressWidth;
        else
            aProgressRect.right = aProgressRect.left + nProgressWidth;

        return ImplDrawTheme( hTheme, hDC, PP_CHUNK, iState, aProgressRect, aCaption );
    }

    if( nType == CTRL_SLIDER )
    {
        iPart = (nPart == PART_TRACK_HORZ_AREA) ? TKP_TRACK : TKP_TRACKVERT;
        iState = (nPart == PART_TRACK_HORZ_AREA) ? static_cast<int>(TRS_NORMAL) : static_cast<int>(TRVS_NORMAL);

        Rectangle aTrackRect = ImplGetThemeRect( hTheme, hDC, iPart, iState, Rectangle() );
        RECT aTRect = rc;
        if( nPart == PART_TRACK_HORZ_AREA )
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
        OSL_ASSERT( aValue.getType() == CTRL_SLIDER );
        const SliderValue* pVal = static_cast<const SliderValue*>(&aValue);
        aThumbRect.left   = pVal->maThumbRect.Left();
        aThumbRect.top    = pVal->maThumbRect.Top();
        aThumbRect.right  = pVal->maThumbRect.Right();
        aThumbRect.bottom = pVal->maThumbRect.Bottom();
        iPart = (nPart == PART_TRACK_HORZ_AREA) ? TKP_THUMB : TKP_THUMBVERT;
        iState = (nState & CTRL_STATE_ENABLED) ? TUS_NORMAL : TUS_DISABLED;
        return ImplDrawTheme( hTheme, hDC, iPart, iState, aThumbRect, aCaption );
    }

    if( nType == CTRL_LISTNODE )
    {
        if( nPart != PART_ENTIRE_CONTROL )
            return FALSE;

        ButtonValue aButtonValue = aValue.getTristateVal();
        iPart = TVP_GLYPH;
        switch( aButtonValue )
        {
        case BUTTONVALUE_ON:
            iState = GLPS_OPENED;
            break;
        case BUTTONVALUE_OFF:
            iState = GLPS_CLOSED;
            break;
        default:
            return FALSE;
        }
        return ImplDrawTheme( hTheme, hDC, iPart, iState, rc, aCaption );
    }

    if( GetSalData()->mbThemeMenuSupport )
    {
        if( nType == CTRL_MENU_POPUP )
        {
            if( nPart == PART_ENTIRE_CONTROL )
            {
                RECT aGutterRC = rc;
                if( Application::GetSettings().GetLayoutRTL() )
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
            else if( nPart == PART_MENU_ITEM )
            {
                if( (nState & CTRL_STATE_ENABLED) )
                    iState = (nState & CTRL_STATE_SELECTED) ? MPI_HOT : MPI_NORMAL;
                else
                    iState = (nState & CTRL_STATE_SELECTED) ? MPI_DISABLEDHOT : MPI_DISABLED;
                return ImplDrawTheme( hTheme, hDC, MENU_POPUPITEM, iState, rc, aCaption );
            }
            else if( nPart == PART_MENU_ITEM_CHECK_MARK || nPart == PART_MENU_ITEM_RADIO_MARK )
            {
                if( (nState & CTRL_STATE_PRESSED) )
                {
                    RECT aBGRect = rc;
                    if( aValue.getType() == CTRL_MENU_POPUP )
                    {
                        const MenupopupValue& rMVal( static_cast<const MenupopupValue&>(aValue) );
                        aBGRect.top    = rMVal.maItemRect.Top();
                        aBGRect.bottom = rMVal.maItemRect.Bottom()+1; // see below in drawNativeControl
                        if( Application::GetSettings().GetLayoutRTL() )
                        {
                            aBGRect.right = rMVal.maItemRect.Right()+1;
                            aBGRect.left = aBGRect.right - (rMVal.getNumericVal()-rMVal.maItemRect.Left());
                        }
                        else
                        {
                            aBGRect.right = rMVal.getNumericVal();
                            aBGRect.left  = rMVal.maItemRect.Left();
                        }
                        rc = aBGRect;
                    }
                    iState = (nState & CTRL_STATE_ENABLED) ? MCB_NORMAL : MCB_DISABLED;
                    ImplDrawTheme( hTheme, hDC, MENU_POPUPCHECKBACKGROUND, iState, aBGRect, aCaption );
                    if( nPart == PART_MENU_ITEM_CHECK_MARK )
                        iState = (nState & CTRL_STATE_ENABLED) ? MC_CHECKMARKNORMAL : MC_CHECKMARKDISABLED;
                    else
                        iState = (nState & CTRL_STATE_ENABLED) ? MC_BULLETNORMAL : MC_BULLETDISABLED;
                    return ImplDrawTheme( hTheme, hDC, MENU_POPUPCHECK, iState, rc, aCaption );
                }
                else
                    return true; // unchecked: do nothing
            }
            else if( nPart == PART_MENU_SEPARATOR )
            {
                // adjust for gutter position
                if( Application::GetSettings().GetLayoutRTL() )
                    rc.right -= aValue.getNumericVal()+1;
                else
                    rc.left += aValue.getNumericVal()+1;
                Rectangle aRect( ImplGetThemeRect( hTheme, hDC,
                    MENU_POPUPSEPARATOR, 0, Rectangle( rc.left, rc.top, rc.right, rc.bottom ) ) );
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

/*
 * DrawNativeControl()
 *
 *  Draws the requested control described by nPart/nState.
 *
 *  rControlRegion: The bounding region of the complete control in VCL frame coordinates.
 *  aValue:         An optional value (tristate/numerical/string)
 *  aCaption:   A caption or title string (like button text etc)
 */
sal_Bool WinSalGraphics::drawNativeControl( ControlType nType,
                            ControlPart nPart,
                            const Rectangle& rControlRegion,
                            ControlState nState,
                            const ImplControlValue& aValue,
                            const OUString& aCaption )
{
    sal_Bool bOk = false;
    HTHEME hTheme = NULL;

    switch( nType )
    {
        case CTRL_PUSHBUTTON:
        case CTRL_RADIOBUTTON:
        case CTRL_CHECKBOX:
            hTheme = getThemeHandle( mhWnd, L"Button");
            break;
        case CTRL_SCROLLBAR:
            hTheme = getThemeHandle( mhWnd, L"Scrollbar");
            break;
        case CTRL_COMBOBOX:
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"Edit");
            else if( nPart == PART_BUTTON_DOWN )
                hTheme = getThemeHandle( mhWnd, L"Combobox");
            break;
        case CTRL_SPINBOX:
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"Edit");
            else
                hTheme = getThemeHandle( mhWnd, L"Spin");
            break;
        case CTRL_SPINBUTTONS:
            hTheme = getThemeHandle( mhWnd, L"Spin");
            break;
        case CTRL_EDITBOX:
        case CTRL_MULTILINE_EDITBOX:
            hTheme = getThemeHandle( mhWnd, L"Edit");
            break;
        case CTRL_LISTBOX:
            if( nPart == PART_ENTIRE_CONTROL || nPart == PART_WINDOW )
                hTheme = getThemeHandle( mhWnd, L"Listview");
            else if( nPart == PART_BUTTON_DOWN )
                hTheme = getThemeHandle( mhWnd, L"Combobox");
            break;
        case CTRL_TAB_PANE:
        case CTRL_TAB_BODY:
        case CTRL_TAB_ITEM:
            hTheme = getThemeHandle( mhWnd, L"Tab");
            break;
        case CTRL_TOOLBAR:
            if( nPart == PART_ENTIRE_CONTROL || nPart == PART_BUTTON )
                hTheme = getThemeHandle( mhWnd, L"Toolbar");
            else
                // use rebar for grip and background
                hTheme = getThemeHandle( mhWnd, L"Rebar");
            break;
        case CTRL_MENUBAR:
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"Rebar");
            else if( GetSalData()->mbThemeMenuSupport )
            {
                if( nPart == PART_MENU_ITEM )
                    hTheme = getThemeHandle( mhWnd, L"Menu" );
            }
            break;
        case CTRL_PROGRESS:
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"Progress");
            break;
        case CTRL_LISTNODE:
            if( nPart == PART_ENTIRE_CONTROL )
                hTheme = getThemeHandle( mhWnd, L"TreeView");
            break;
        case CTRL_SLIDER:
            if( nPart == PART_TRACK_HORZ_AREA || nPart == PART_TRACK_VERT_AREA )
                hTheme = getThemeHandle( mhWnd, L"Trackbar" );
            break;
        case CTRL_MENU_POPUP:
            if( GetSalData()->mbThemeMenuSupport )
            {
                if( nPart == PART_ENTIRE_CONTROL || nPart == PART_MENU_ITEM ||
                    nPart == PART_MENU_ITEM_CHECK_MARK || nPart == PART_MENU_ITEM_RADIO_MARK ||
                    nPart == PART_MENU_SEPARATOR
                    )
                    hTheme = getThemeHandle( mhWnd, L"Menu" );
            }
            break;
        default:
            hTheme = NULL;
            break;
    }

    if( !hTheme )
        return false;

    Rectangle buttonRect = rControlRegion;
    RECT rc;
    rc.left   = buttonRect.Left();
    rc.right  = buttonRect.Right()+1;
    rc.top    = buttonRect.Top();
    rc.bottom = buttonRect.Bottom()+1;

    // set default text alignment
    int ta = SetTextAlign( mhDC, TA_LEFT|TA_TOP|TA_NOUPDATECP );

    OUString aCaptionStr( aCaption.replace('~', '&') ); // translate mnemonics
    bOk = ImplDrawNativeControl(mhDC, hTheme, rc,
                            nType, nPart, nState, aValue,
                            aCaptionStr );

    // restore alignment
    SetTextAlign( mhDC, ta );


    //GdiFlush();

    return bOk;
}

/*
 * GetNativeControlRegion()
 *
 *  If the return value is TRUE, rNativeBoundingRegion
 *  contains the true bounding region covered by the control
 *  including any adornment, while rNativeContentRegion contains the area
 *  within the control that can be safely drawn into without drawing over
 *  the borders of the control.
 *
 *  rControlRegion: The bounding region of the control in VCL frame coordinates.
 *  aValue:     An optional value (tristate/numerical/string)
 *  aCaption:       A caption or title string (like button text etc)
 */
sal_Bool WinSalGraphics::getNativeControlRegion(  ControlType nType,
                                ControlPart nPart,
                                const Rectangle& rControlRegion,
                                ControlState nState,
                                const ImplControlValue& rControlValue,
                                const OUString&,
                                Rectangle &rNativeBoundingRegion,
                                Rectangle &rNativeContentRegion )
{
    sal_Bool bRet = FALSE;

    HDC hDC = GetDC( mhWnd );
    if( nType == CTRL_TOOLBAR )
    {
        if( nPart == PART_THUMB_HORZ || nPart == PART_THUMB_VERT )
        {
            /*
            // the vertical gripper is not supported in most themes and it makes no
            // sense to only support horizontal gripper

            HTHEME hTheme = getThemeHandle( mhWnd, L"Rebar");
            if( hTheme )
            {
                Rectangle aRect( ImplGetThemeRect( hTheme, hDC, nPart == PART_THUMB_HORZ ? RP_GRIPPERVERT : RP_GRIPPER,
                    0, rControlRegion.GetBoundRect() ) );
                if( nPart == PART_THUMB_HORZ && !aRect.IsEmpty() )
                {
                    Rectangle aVertRect( 0, 0, aRect.getHeight(), aRect.getWidth() );
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
        if( nPart == PART_BUTTON )
        {
            HTHEME hTheme = getThemeHandle( mhWnd, L"Toolbar");
            if( hTheme )
            {
                Rectangle aRect( ImplGetThemeRect( hTheme, hDC, TP_SPLITBUTTONDROPDOWN,
                    TS_HOT, rControlRegion ) );
                rNativeContentRegion = aRect;
                rNativeBoundingRegion = rNativeContentRegion;
                if( !rNativeContentRegion.IsEmpty() )
                    bRet = TRUE;
            }
        }
    }
    if( nType == CTRL_PROGRESS && nPart == PART_ENTIRE_CONTROL )
    {
        HTHEME hTheme = getThemeHandle( mhWnd, L"Progress");
        if( hTheme )
        {
            Rectangle aRect( ImplGetThemeRect( hTheme, hDC, PP_BAR,
                0, rControlRegion ) );
            rNativeContentRegion = aRect;
            rNativeBoundingRegion = rNativeContentRegion;
            if( !rNativeContentRegion.IsEmpty() )
                bRet = TRUE;
        }
    }
    if( (nType == CTRL_LISTBOX || nType == CTRL_COMBOBOX ) && nPart == PART_ENTIRE_CONTROL )
    {
        HTHEME hTheme = getThemeHandle( mhWnd, L"Combobox");
        if( hTheme )
        {
            Rectangle aBoxRect( rControlRegion );
            Rectangle aRect( ImplGetThemeRect( hTheme, hDC, CP_DROPDOWNBUTTON,
                                               CBXS_NORMAL, aBoxRect ) );
            if( aRect.GetHeight() > aBoxRect.GetHeight() )
                aBoxRect.Bottom() = aBoxRect.Top() + aRect.GetHeight();
            if( aRect.GetWidth() > aBoxRect.GetWidth() )
                aBoxRect.Right() = aBoxRect.Left() + aRect.GetWidth();
            rNativeContentRegion = aBoxRect;
            rNativeBoundingRegion = rNativeContentRegion;
            if( !aRect.IsEmpty() )
                bRet = TRUE;
        }
    }

    if( (nType == CTRL_EDITBOX || nType == CTRL_SPINBOX) && nPart == PART_ENTIRE_CONTROL )
    {
        HTHEME hTheme = getThemeHandle( mhWnd, L"Edit");
        if( hTheme )
        {
            // get border size
            Rectangle aBoxRect( rControlRegion );
            Rectangle aRect( ImplGetThemeRect( hTheme, hDC, EP_BACKGROUNDWITHBORDER,
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
                    aRect.Bottom() += aRect.GetHeight();
                    aRect.Bottom() += nFontHeight;
                    if( aRect.GetHeight() > aBoxRect.GetHeight() )
                        aBoxRect.Bottom() = aBoxRect.Top() + aRect.GetHeight();
                    if( aRect.GetWidth() > aBoxRect.GetWidth() )
                        aBoxRect.Right() = aBoxRect.Left() + aRect.GetWidth();
                    rNativeContentRegion = aBoxRect;
                    rNativeBoundingRegion = rNativeContentRegion;
                        bRet = TRUE;
                }
            }
        }
    }

    if( GetSalData()->mbThemeMenuSupport )
    {
        if( nType == CTRL_MENU_POPUP )
        {
            if( nPart == PART_MENU_ITEM_CHECK_MARK ||
                nPart == PART_MENU_ITEM_RADIO_MARK )
            {
                HTHEME hTheme = getThemeHandle( mhWnd, L"Menu");
                Rectangle aBoxRect( rControlRegion );
                Rectangle aRect( ImplGetThemeRect( hTheme, hDC,
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

    if( nType == CTRL_SLIDER && ( (nPart == PART_THUMB_HORZ) || (nPart == PART_THUMB_VERT) ) )
    {
        HTHEME hTheme = getThemeHandle( mhWnd, L"Trackbar");
        if( hTheme )
        {
            int iPart = (nPart == PART_THUMB_HORZ) ? TKP_THUMB : TKP_THUMBVERT;
            int iState = (nPart == PART_THUMB_HORZ) ? static_cast<int>(TUS_NORMAL) : static_cast<int>(TUVS_NORMAL);
            Rectangle aThumbRect = ImplGetThemeRect( hTheme, hDC, iPart, iState, Rectangle() );
            if( nPart == PART_THUMB_HORZ )
            {
                long nW = aThumbRect.GetWidth();
                Rectangle aRect( rControlRegion );
                aRect.Right() = aRect.Left() + nW - 1;
                rNativeContentRegion = aRect;
                rNativeBoundingRegion = rNativeContentRegion;
            }
            else
            {
                long nH = aThumbRect.GetHeight();
                Rectangle aRect( rControlRegion );
                aRect.Bottom() = aRect.Top() + nH - 1;
                rNativeContentRegion = aRect;
                rNativeBoundingRegion = rNativeContentRegion;
            }
            bRet = TRUE;
        }
    }

    if ( ( nType == CTRL_TAB_ITEM ) && ( nPart == PART_ENTIRE_CONTROL ) )
    {
        Rectangle aControlRect( rControlRegion );
        rNativeContentRegion = aControlRect;

        --aControlRect.Bottom();

        if( rControlValue.getType() == CTRL_TAB_ITEM )
        {
            const TabitemValue *pValue = static_cast<const TabitemValue*>(&rControlValue);
            if ( pValue->isBothAligned() )
                --aControlRect.Right();

            if ( nState & CTRL_STATE_SELECTED )
            {
                aControlRect.Left() -= 2;
                if ( pValue && !pValue->isBothAligned() )
                {
                    if ( pValue->isLeftAligned() || pValue->isNotAligned() )
                        aControlRect.Right() += 2;
                    if ( pValue->isRightAligned() )
                        aControlRect.Right() += 1;
                }
                aControlRect.Top() -= 2;
                aControlRect.Bottom() += 2;
            }
        }
        rNativeBoundingRegion = aControlRect;
        bRet = TRUE;
    }

    ReleaseDC( mhWnd, hDC );
    return( bRet );
}

void WinSalGraphics::updateSettingsNative( AllSettings& rSettings )
{
    if ( !vsAPI.IsThemeActive() )
        return;

    StyleSettings aStyleSettings = rSettings.GetStyleSettings();
    ImplSVData* pSVData = ImplGetSVData();

    // check if vista or newer runs
    // in Aero theme (and similar ?) the menu text color does not change
    // for selected items; also on WinXP and earlier menus are not themed
    // FIXME get the color directly from the theme, not from the settings
    if( aSalShlData.maVersionInfo.dwMajorVersion >= 6 )
    {
        // in aero menuitem highlight text is drawn in the same color as normal
        aStyleSettings.SetMenuHighlightTextColor( aStyleSettings.GetMenuTextColor() );
        pSVData->maNWFData.mnMenuFormatBorderX = 2;
        pSVData->maNWFData.mnMenuFormatBorderY = 2;
        pSVData->maNWFData.maMenuBarHighlightTextColor = aStyleSettings.GetMenuTextColor();
        GetSalData()->mbThemeMenuSupport = TRUE;

        // don't draw frame around each and every toolbar
        pSVData->maNWFData.mbDockingAreaAvoidTBFrames = true;
    }

    rSettings.SetStyleSettings( aStyleSettings );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
