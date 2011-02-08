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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include "svsys.h"
#include "vcl/salframe.hxx"
#include "vcl/salobj.hxx"
#include "vcl/salinst.hxx"
#include "vcl/salgtype.hxx"
#include "vcl/salgdi.hxx"
#include "vcl/salctrlhandle.hxx"

#include "vcl/unohelp.hxx"
#include "tools/time.hxx"
#include "tools/debug.hxx"
#include "tools/rc.h"
#include "vcl/svdata.hxx"
#include "vcl/windata.hxx"
#include "vcl/dbggui.hxx"
#include "vcl/outfont.hxx"
#include "vcl/outdev.h"
#include "vcl/region.h"
#include "vcl/event.hxx"
#include "vcl/help.hxx"
#include "vcl/cursor.hxx"
#include "vcl/svapp.hxx"
#include "vcl/window.h"
#include "vcl/window.hxx"
#include "vcl/syswin.hxx"
#include "vcl/syschild.hxx"
#include "vcl/brdwin.hxx"
#include "vcl/helpwin.hxx"
#include "vcl/dockwin.hxx"
#include "vcl/menu.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/wall.hxx"
#include "vcl/gradient.hxx"
#include "vcl/toolbox.h"
#include "unotools/fontcfg.hxx"
#include "vcl/sysdata.hxx"
#include "vcl/sallayout.hxx"
#include "vcl/button.hxx" // Button::GetStandardText
#include "vcl/taskpanelist.hxx"
#include "com/sun/star/awt/XWindowPeer.hpp"
#include "com/sun/star/rendering/XCanvas.hpp"
#include "com/sun/star/rendering/XSpriteCanvas.hpp"
#include "com/sun/star/awt/XWindow.hpp"
#include "comphelper/processfactory.hxx"
#include "com/sun/star/datatransfer/dnd/XDragSource.hpp"
#include "com/sun/star/datatransfer/dnd/XDropTarget.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboard.hpp"
#include "com/sun/star/awt/XTopWindow.hpp"
#include "com/sun/star/awt/XDisplayConnection.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XServiceName.hpp"
#include "com/sun/star/accessibility/XAccessible.hpp"
#include "com/sun/star/accessibility/AccessibleRole.hpp"

#include "vcl/dialog.hxx"
#include "vcl/unowrap.hxx"
#include "vcl/dndlcon.hxx"
#include "vcl/dndevdis.hxx"
#include "vcl/impbmpconv.hxx"
#include "unotools/confignode.hxx"
#include "vcl/gdimtf.hxx"
#include <sal/macros.h>
#include "vcl/pdfextoutdevdata.hxx"
#include "vcl/lazydelete.hxx"

#include <set>
#include <typeinfo>

using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::com::sun::star;
using namespace com::sun;

using ::com::sun::star::awt::XTopWindow;

// =======================================================================

DBG_NAME( Window )

// =======================================================================

#define IMPL_PAINT_PAINT            ((USHORT)0x0001)
#define IMPL_PAINT_PAINTALL         ((USHORT)0x0002)
#define IMPL_PAINT_PAINTALLCHILDS   ((USHORT)0x0004)
#define IMPL_PAINT_PAINTCHILDS      ((USHORT)0x0008)
#define IMPL_PAINT_ERASE            ((USHORT)0x0010)
#define IMPL_PAINT_CHECKRTL         ((USHORT)0x0020)

// -----------------------------------------------------------------------

typedef Window* PWINDOW;

// -----------------------------------------------------------------------

struct ImplCalcToTopData
{
    ImplCalcToTopData*  mpNext;
    Window*             mpWindow;
    Region*             mpInvalidateRegion;
};

struct ImplAccessibleInfos
{
    USHORT nAccessibleRole;
    String* pAccessibleName;
    String* pAccessibleDescription;

    ImplAccessibleInfos()
    {
        nAccessibleRole = 0xFFFF;
        pAccessibleName = NULL;
        pAccessibleDescription = NULL;
    }

    ~ImplAccessibleInfos()
    {
        delete pAccessibleName;
        delete pAccessibleDescription;
    }
};

// -----------------------------------------------------------------------

WindowImpl::WindowImpl()
{
}

WindowImpl::~WindowImpl()
{
}


// -----------------------------------------------------------------------

// helper method to allow inline constructor even for pWindow!=NULL case
void ImplDelData::AttachToWindow( const Window* pWindow )
{
    if( pWindow )
        const_cast<Window*>(pWindow)->ImplAddDel( this );
}

// -----------------------------------------------------------------------

// define dtor for ImplDelData
ImplDelData::~ImplDelData()
{
    // #112873# auto remove of ImplDelData
    // due to this code actively calling ImplRemoveDel() is not mandatory anymore
    if( !mbDel && mpWindow )
    {
        // the window still exists but we were not removed
        const_cast<Window*>(mpWindow)->ImplRemoveDel( this );
        mpWindow = NULL;
    }
}

// -----------------------------------------------------------------------

#ifdef DBG_UTIL
const char* ImplDbgCheckWindow( const void* pObj )
{
    DBG_TESTSOLARMUTEX();

    const Window* pWindow = (Window*)pObj;

    if ( (pWindow->GetType() < WINDOW_FIRST) || (pWindow->GetType() > WINDOW_LAST) )
        return "Window data overwrite";

    // Fenster-Verkettung ueberpruefen
    Window* pChild = pWindow->mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        if ( pChild->mpWindowImpl->mpParent != pWindow )
            return "Child-Window-Parent wrong";
        pChild = pChild->mpWindowImpl->mpNext;
    }

    return NULL;
}
#endif

// =======================================================================

void Window::ImplInitAppFontData( Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();
    long nTextHeight = pWindow->GetTextHeight();
    long nTextWidth = pWindow->GetTextWidth( XubString( RTL_CONSTASCII_USTRINGPARAM( "aemnnxEM" ) ) );
    long nSymHeight = nTextHeight*4;
    // Falls Font zu schmal ist, machen wir die Basis breiter,
    // damit die Dialoge symetrisch aussehen und nicht zu schmal
    // werden. Wenn der Dialog die gleiche breite hat, geben wir
    // noch etwas Spielraum dazu, da etwas mehr Platz besser ist.
    if ( nSymHeight > nTextWidth )
        nTextWidth = nSymHeight;
    else if ( nSymHeight+5 > nTextWidth )
        nTextWidth = nSymHeight+5;
    pSVData->maGDIData.mnAppFontX = nTextWidth * 10 / 8;
    pSVData->maGDIData.mnAppFontY = nTextHeight * 10;

    // FIXME: this is currently only on aqua, check with other
    // platforms
    if( pSVData->maNWFData.mbNoFocusRects )
    {
        // try to find out wether there is a large correction
        // of control sizes, if yes, make app font scalings larger
        // so dialog positioning is not completely off
        ImplControlValue aControlValue;
        Rectangle aCtrlRegion( Point(), Size( nTextWidth < 10 ? 10 : nTextWidth, nTextHeight < 10 ? 10 : nTextHeight ) );
        Rectangle aBoundingRgn( aCtrlRegion );
        Rectangle aContentRgn( aCtrlRegion );
        if( pWindow->GetNativeControlRegion( CTRL_EDITBOX, PART_ENTIRE_CONTROL, aCtrlRegion,
                                             CTRL_STATE_ENABLED, aControlValue, rtl::OUString(),
                                             aBoundingRgn, aContentRgn ) )
        {
            // comment: the magical +6 is for the extra border in bordered
            // (which is the standard) edit fields
            if( aContentRgn.GetHeight() - nTextHeight > (nTextHeight+4)/4 )
                pSVData->maGDIData.mnAppFontY = (aContentRgn.GetHeight()-4) * 10;
        }
    }


    pSVData->maGDIData.mnRealAppFontX = pSVData->maGDIData.mnAppFontX;
    if ( pSVData->maAppData.mnDialogScaleX )
        pSVData->maGDIData.mnAppFontX += (pSVData->maGDIData.mnAppFontX*pSVData->maAppData.mnDialogScaleX)/100;
}

// -----------------------------------------------------------------------

bool Window::ImplCheckUIFont( const Font& rFont )
{
    if( ImplGetSVData()->maGDIData.mbNativeFontConfig )
        return true;

    String aTestText;
    aTestText.Append( Button::GetStandardText( BUTTON_OK ) );
    aTestText.Append( Button::GetStandardText( BUTTON_CANCEL ) );
    aTestText.Append( Button::GetStandardText( BUTTON_YES ) );
    aTestText.Append( Button::GetStandardText( BUTTON_NO ) );
    aTestText.Append( Button::GetStandardText( BUTTON_RETRY ) );
    aTestText.Append( Button::GetStandardText( BUTTON_HELP ) );
    aTestText.Append( Button::GetStandardText( BUTTON_CLOSE ) );
    aTestText.Append( Button::GetStandardText( BUTTON_MORE ) );
    aTestText.Append( Button::GetStandardText( BUTTON_LESS ) );
    aTestText.Append( Button::GetStandardText( BUTTON_ABORT ) );

    return HasGlyphs( rFont, aTestText ) >= aTestText.Len();
}

// -----------------------------------------------------------------------

void Window::ImplUpdateGlobalSettings( AllSettings& rSettings, BOOL bCallHdl )
{
    StyleSettings aTmpSt( rSettings.GetStyleSettings() );
    aTmpSt.SetHighContrastMode( FALSE );
    rSettings.SetStyleSettings( aTmpSt );
    ImplGetFrame()->UpdateSettings( rSettings );

    // Verify availability of the configured UI font, otherwise choose "Andale Sans UI"
    String aUserInterfaceFont;
    bool bUseSystemFont = rSettings.GetStyleSettings().GetUseSystemUIFonts();

    // check whether system UI font can display a typical UI text
    if( bUseSystemFont )
        bUseSystemFont = ImplCheckUIFont( rSettings.GetStyleSettings().GetAppFont() );

    if ( !bUseSystemFont )
    {
        ImplInitFontList();
        String aConfigFont = utl::DefaultFontConfiguration::get()->getUserInterfaceFont( rSettings.GetUILocale() );
        xub_StrLen nIndex = 0;
        while( nIndex != STRING_NOTFOUND )
        {
            String aName( aConfigFont.GetToken( 0, ';', nIndex ) );
            if ( aName.Len() && mpWindowImpl->mpFrameData->mpFontList->FindFontFamily( aName ) )
            {
                aUserInterfaceFont = aConfigFont;
                break;
            }
        }

        if ( ! aUserInterfaceFont.Len() )
        {
            String aFallbackFont (RTL_CONSTASCII_USTRINGPARAM( "Andale Sans UI" ));
            if ( mpWindowImpl->mpFrameData->mpFontList->FindFontFamily( aFallbackFont ) )
                aUserInterfaceFont = aFallbackFont;
        }
    }

    if ( !bUseSystemFont && aUserInterfaceFont.Len() )
    {
        StyleSettings aStyleSettings = rSettings.GetStyleSettings();
        Font aFont = aStyleSettings.GetAppFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetAppFont( aFont );
        aFont = aStyleSettings.GetHelpFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetHelpFont( aFont );
        aFont = aStyleSettings.GetTitleFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetTitleFont( aFont );
        aFont = aStyleSettings.GetFloatTitleFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetFloatTitleFont( aFont );
        aFont = aStyleSettings.GetMenuFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetMenuFont( aFont );
        aFont = aStyleSettings.GetToolFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetToolFont( aFont );
        aFont = aStyleSettings.GetLabelFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetLabelFont( aFont );
        aFont = aStyleSettings.GetInfoFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetInfoFont( aFont );
        aFont = aStyleSettings.GetRadioCheckFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetRadioCheckFont( aFont );
        aFont = aStyleSettings.GetPushButtonFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetPushButtonFont( aFont );
        aFont = aStyleSettings.GetFieldFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetFieldFont( aFont );
        aFont = aStyleSettings.GetIconFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetIconFont( aFont );
        aFont = aStyleSettings.GetGroupFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetGroupFont( aFont );
        rSettings.SetStyleSettings( aStyleSettings );
    }

    StyleSettings aStyleSettings = rSettings.GetStyleSettings();
    // #97047: Force all fonts except Menu and Help to a fixed height
    // to avoid UI scaling due to large fonts
    // - but allow bigger fonts on bigger screens (i16682, i21238)
    //   dialogs were designed to fit 800x600 with an 8pt font, so scale accordingly
    int maxFontheight = 9; // #107886#: 9 is default for some asian systems, so always allow if requested
    if( GetDesktopRectPixel().getHeight() > 600 )
        maxFontheight = (int) ((( 8.0 * (double) GetDesktopRectPixel().getHeight()) / 600.0) + 1.5);

    Font aFont = aStyleSettings.GetMenuFont();
    int defFontheight = aFont.GetHeight();
    if( defFontheight > maxFontheight )
        defFontheight = maxFontheight;

    // if the UI is korean, chinese or another locale
    // where the system font size is kown to be often too small to
    // generate readable fonts enforce a minimum font size of 9 points
    bool bBrokenLangFontHeight = false;
    static const LanguageType eBrokenSystemFontSizeLanguages[] =
    { LANGUAGE_KOREAN, LANGUAGE_KOREAN_JOHAB,
      LANGUAGE_CHINESE_HONGKONG, LANGUAGE_CHINESE_MACAU, LANGUAGE_CHINESE_SIMPLIFIED, LANGUAGE_CHINESE_SINGAPORE, LANGUAGE_CHINESE_TRADITIONAL
    };
    static std::set< LanguageType > aBrokenSystemFontSizeLanguagesSet(
        eBrokenSystemFontSizeLanguages,
        eBrokenSystemFontSizeLanguages + SAL_N_ELEMENTS(eBrokenSystemFontSizeLanguages)
        );
    LanguageType aLang = Application::GetSettings().GetUILanguage();
    if( aBrokenSystemFontSizeLanguagesSet.find( aLang ) != aBrokenSystemFontSizeLanguagesSet.end() )
    {
        defFontheight = Max(9, defFontheight);
        bBrokenLangFontHeight = true;
    }

    // i22098, toolfont will be scaled differently to avoid bloated rulers and status bars for big fonts
    int toolfontheight = defFontheight;
    if( toolfontheight > 9 )
        toolfontheight = (defFontheight+8) / 2;

    aFont = aStyleSettings.GetAppFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetAppFont( aFont );
    aFont = aStyleSettings.GetTitleFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetTitleFont( aFont );
    aFont = aStyleSettings.GetFloatTitleFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetFloatTitleFont( aFont );
    // keep menu and help font size from system unless in broken locale size
    if( bBrokenLangFontHeight )
    {
        aFont = aStyleSettings.GetMenuFont();
        if( aFont.GetHeight() < defFontheight )
        {
            aFont.SetHeight( defFontheight );
            aStyleSettings.SetMenuFont( aFont );
        }
        aFont = aStyleSettings.GetHelpFont();
        if( aFont.GetHeight() < defFontheight )
        {
            aFont.SetHeight( defFontheight );
            aStyleSettings.SetHelpFont( aFont );
        }
    }

    // use different height for toolfont
    aFont = aStyleSettings.GetToolFont();
    aFont.SetHeight( toolfontheight );
    aStyleSettings.SetToolFont( aFont );

    aFont = aStyleSettings.GetLabelFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetLabelFont( aFont );
    aFont = aStyleSettings.GetInfoFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetInfoFont( aFont );
    aFont = aStyleSettings.GetRadioCheckFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetRadioCheckFont( aFont );
    aFont = aStyleSettings.GetPushButtonFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetPushButtonFont( aFont );
    aFont = aStyleSettings.GetFieldFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetFieldFont( aFont );
    aFont = aStyleSettings.GetIconFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetIconFont( aFont );
    aFont = aStyleSettings.GetGroupFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetGroupFont( aFont );

    // set workspace gradient to black in dark themes
    if( aStyleSettings.GetWindowColor().IsDark() )
        aStyleSettings.SetWorkspaceGradient( Wallpaper( Color( COL_BLACK ) ) );
    else
    {
        Gradient aGrad( GRADIENT_LINEAR, DEFAULT_WORKSPACE_GRADIENT_START_COLOR, DEFAULT_WORKSPACE_GRADIENT_END_COLOR );
        aStyleSettings.SetWorkspaceGradient( Wallpaper( aGrad ) );
    }

    rSettings.SetStyleSettings( aStyleSettings );

    // auto detect HC mode; if the system already set it to "yes"
    // (see above) then accept that
    if( !rSettings.GetStyleSettings().GetHighContrastMode() )
    {
        sal_Bool bTmp = sal_False, bAutoHCMode = sal_True;
        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithServiceFactory(
            vcl::unohelper::GetMultiServiceFactory(),
            OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Common/Accessibility")) );    // note: case sensisitive !
        if ( aNode.isValid() )
        {
            ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString(RTL_CONSTASCII_USTRINGPARAM("AutoDetectSystemHC")) );
            if( aValue >>= bTmp )
                bAutoHCMode = bTmp;
        }
        if( bAutoHCMode )
        {
            if( rSettings.GetStyleSettings().GetFaceColor().IsDark()
             || rSettings.GetStyleSettings().GetWindowColor().IsDark() )
            {
                aStyleSettings = rSettings.GetStyleSettings();
                aStyleSettings.SetHighContrastMode( TRUE );
                aStyleSettings.SetSymbolsStyle( STYLE_SYMBOLS_HICONTRAST );
                rSettings.SetStyleSettings( aStyleSettings );
            }
        }
    }

    static const char* pEnvHC = getenv( "SAL_FORCE_HC" );
    if( pEnvHC && *pEnvHC )
    {
        aStyleSettings.SetHighContrastMode( TRUE );
        aStyleSettings.SetSymbolsStyle( STYLE_SYMBOLS_HICONTRAST );
        rSettings.SetStyleSettings( aStyleSettings );
    }

#ifdef DBG_UTIL
    // Evt. AppFont auf Fett schalten, damit man feststellen kann,
    // ob fuer die Texte auf anderen Systemen genuegend Platz
    // vorhanden ist
    if ( DbgIsBoldAppFont() )
    {
        aStyleSettings = rSettings.GetStyleSettings();
        aFont = aStyleSettings.GetAppFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetAppFont( aFont );
        aFont = aStyleSettings.GetGroupFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetGroupFont( aFont );
        aFont = aStyleSettings.GetLabelFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetLabelFont( aFont );
        aFont = aStyleSettings.GetRadioCheckFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetRadioCheckFont( aFont );
        aFont = aStyleSettings.GetPushButtonFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetPushButtonFont( aFont );
        aFont = aStyleSettings.GetFieldFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetFieldFont( aFont );
        aFont = aStyleSettings.GetIconFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetIconFont( aFont );
        rSettings.SetStyleSettings( aStyleSettings );
    }
#endif

    if ( bCallHdl )
        GetpApp()->SystemSettingsChanging( rSettings, this );
}

// -----------------------------------------------------------------------

MouseEvent ImplTranslateMouseEvent( const MouseEvent& rE, Window* pSource, Window* pDest )
{
    Point aPos = pSource->OutputToScreenPixel( rE.GetPosPixel() );
    aPos = pDest->ScreenToOutputPixel( aPos );
    return MouseEvent( aPos, rE.GetClicks(), rE.GetMode(), rE.GetButtons(), rE.GetModifier() );
}

// -----------------------------------------------------------------------

CommandEvent ImplTranslateCommandEvent( const CommandEvent& rCEvt, Window* pSource, Window* pDest )
{
    if ( !rCEvt.IsMouseEvent() )
        return rCEvt;

    Point aPos = pSource->OutputToScreenPixel( rCEvt.GetMousePosPixel() );
    aPos = pDest->ScreenToOutputPixel( aPos );
    return CommandEvent( aPos, rCEvt.GetCommand(), rCEvt.IsMouseEvent(), rCEvt.GetData() );
}

// =======================================================================

void Window::ImplInitWindowData( WindowType nType )
{
    mpWindowImpl = new WindowImpl;

    meOutDevType        = OUTDEV_WINDOW;

    mpWindowImpl->maZoom              = Fraction( 1, 1 );
    mpWindowImpl->maWinRegion         = Region( REGION_NULL );
    mpWindowImpl->maWinClipRegion                   = Region( REGION_NULL );
    mpWindowImpl->mpWinData           = NULL;         // Extra Window Data, that we dont need for all windows
    mpWindowImpl->mpOverlapData       = NULL;         // Overlap Data
    mpWindowImpl->mpFrameData         = NULL;         // Frame Data
    mpWindowImpl->mpFrame             = NULL;         // Pointer to frame window
    mpWindowImpl->mpSysObj            = NULL;
    mpWindowImpl->mpFrameWindow       = NULL;         // window to top level parent (same as frame window)
    mpWindowImpl->mpOverlapWindow     = NULL;         // first overlap parent
    mpWindowImpl->mpBorderWindow      = NULL;         // Border-Window
    mpWindowImpl->mpClientWindow      = NULL;         // Client-Window of a FrameWindow
    mpWindowImpl->mpParent            = NULL;         // parent (inkl. BorderWindow)
    mpWindowImpl->mpRealParent        = NULL;         // real parent (exkl. BorderWindow)
    mpWindowImpl->mpFirstChild        = NULL;         // first child window
    mpWindowImpl->mpLastChild         = NULL;         // last child window
    mpWindowImpl->mpFirstOverlap      = NULL;         // first overlap window (only set in overlap windows)
    mpWindowImpl->mpLastOverlap       = NULL;         // last overlap window (only set in overlap windows)
    mpWindowImpl->mpPrev              = NULL;         // prev window
    mpWindowImpl->mpNext              = NULL;         // next window
    mpWindowImpl->mpNextOverlap       = NULL;         // next overlap window of frame
    mpWindowImpl->mpLastFocusWindow   = NULL;         // window for focus restore
    mpWindowImpl->mpDlgCtrlDownWindow = NULL;         // window for dialog control
    mpWindowImpl->mpFirstDel          = NULL;         // Dtor notification list
    mpWindowImpl->mpUserData          = NULL;         // user data
    mpWindowImpl->mpCursor            = NULL;         // cursor
    mpWindowImpl->mpControlFont       = NULL;         // font propertie
    mpWindowImpl->mpVCLXWindow        = NULL;
    mpWindowImpl->mpAccessibleInfos   = NULL;
    mpWindowImpl->maControlForeground = Color( COL_TRANSPARENT );     // kein Foreground gesetzt
    mpWindowImpl->maControlBackground = Color( COL_TRANSPARENT );     // kein Background gesetzt
    mpWindowImpl->mnLeftBorder        = 0;            // left border
    mpWindowImpl->mnTopBorder         = 0;            // top border
    mpWindowImpl->mnRightBorder       = 0;            // right border
    mpWindowImpl->mnBottomBorder      = 0;            // bottom border
    mpWindowImpl->mnX                 = 0;            // X-Position to Parent
    mpWindowImpl->mnY                 = 0;            // Y-Position to Parent
    mpWindowImpl->mnAbsScreenX        = 0;            // absolute X-position on screen, used for RTL window positioning
    mpWindowImpl->mnHelpId            = 0;            // help id
    mpWindowImpl->mnUniqId            = 0;            // unique id
    mpWindowImpl->mpChildClipRegion   = NULL;         // Child-Clip-Region when ClipChildren
    mpWindowImpl->mpPaintRegion       = NULL;         // Paint-ClipRegion
    mpWindowImpl->mnStyle             = 0;            // style (init in ImplInitWindow)
    mpWindowImpl->mnPrevStyle         = 0;            // prevstyle (set in SetStyle)
    mpWindowImpl->mnExtendedStyle     = 0;            // extended style (init in ImplInitWindow)
    mpWindowImpl->mnPrevExtendedStyle = 0;            // prevstyle (set in SetExtendedStyle)
    mpWindowImpl->mnType              = nType;        // type
    mpWindowImpl->mnGetFocusFlags     = 0;            // Flags fuer GetFocus()-Aufruf
    mpWindowImpl->mnWaitCount         = 0;            // Wait-Count (>1 == Warte-MousePointer)
    mpWindowImpl->mnPaintFlags        = 0;            // Flags for ImplCallPaint
    mpWindowImpl->mnParentClipMode    = 0;            // Flags for Parent-ClipChildren-Mode
    mpWindowImpl->mnActivateMode      = 0;            // Wird bei System/Overlap-Windows umgesetzt
    mpWindowImpl->mnDlgCtrlFlags      = 0;            // DialogControl-Flags
    mpWindowImpl->mnLockCount         = 0;            // LockCount
    mpWindowImpl->meAlwaysInputMode   = AlwaysInputNone; // neither AlwaysEnableInput nor AlwaysDisableInput called
    mpWindowImpl->mbFrame             = FALSE;        // TRUE: Window is a frame window
    mpWindowImpl->mbBorderWin         = FALSE;        // TRUE: Window is a border window
    mpWindowImpl->mbOverlapWin        = FALSE;        // TRUE: Window is a overlap window
    mpWindowImpl->mbSysWin            = FALSE;        // TRUE: SystemWindow is the base class
    mpWindowImpl->mbDialog            = FALSE;        // TRUE: Dialog is the base class
    mpWindowImpl->mbDockWin           = FALSE;        // TRUE: DockingWindow is the base class
    mpWindowImpl->mbFloatWin          = FALSE;        // TRUE: FloatingWindow is the base class
    mpWindowImpl->mbPushButton        = FALSE;        // TRUE: PushButton is the base class
    mpWindowImpl->mbToolBox                     = FALSE;                // TRUE: ToolBox is the base class
    mpWindowImpl->mbMenuFloatingWindow= FALSE;          // TRUE: MenuFloatingWindow is the base class
    mpWindowImpl->mbToolbarFloatingWindow= FALSE;               // TRUE: ImplPopupFloatWin is the base class, used for subtoolbars
    mpWindowImpl->mbSplitter                    = FALSE;                // TRUE: Splitter is the base class
    mpWindowImpl->mbVisible           = FALSE;        // TRUE: Show( true ) called
    mpWindowImpl->mbOverlapVisible    = FALSE;        // TRUE: Hide called for visible window from ImplHideAllOverlapWindow()
    mpWindowImpl->mbDisabled          = FALSE;        // TRUE: Enable( FALSE ) called
    mpWindowImpl->mbInputDisabled     = FALSE;        // TRUE: EnableInput( FALSE ) called
    mpWindowImpl->mbDropDisabled      = FALSE;        // TRUE: Drop is enabled
    mpWindowImpl->mbNoUpdate          = FALSE;        // TRUE: SetUpdateMode( FALSE ) called
    mpWindowImpl->mbNoParentUpdate    = FALSE;        // TRUE: SetParentUpdateMode( FALSE ) called
    mpWindowImpl->mbActive            = FALSE;        // TRUE: Window Active
    mpWindowImpl->mbParentActive      = FALSE;        // TRUE: OverlapActive from Parent
    mpWindowImpl->mbReallyVisible     = FALSE;        // TRUE: this and all parents to an overlaped window are visible
    mpWindowImpl->mbReallyShown       = FALSE;        // TRUE: this and all parents to an overlaped window are shown
    mpWindowImpl->mbInInitShow        = FALSE;        // TRUE: we are in InitShow
    mpWindowImpl->mbChildNotify       = FALSE;        // TRUE: ChildNotify
    mpWindowImpl->mbChildPtrOverwrite = FALSE;        // TRUE: PointerStyle overwrites Child-Pointer
    mpWindowImpl->mbNoPtrVisible      = FALSE;        // TRUE: ShowPointer( FALSE ) called
    mpWindowImpl->mbMouseMove         = FALSE;        // TRUE: BaseMouseMove called
    mpWindowImpl->mbPaintFrame        = FALSE;        // TRUE: Paint is visible, but not painted
    mpWindowImpl->mbInPaint           = FALSE;        // TRUE: Inside PaintHdl
    mpWindowImpl->mbMouseButtonDown   = FALSE;        // TRUE: BaseMouseButtonDown called
    mpWindowImpl->mbMouseButtonUp     = FALSE;        // TRUE: BaseMouseButtonUp called
    mpWindowImpl->mbKeyInput          = FALSE;        // TRUE: BaseKeyInput called
    mpWindowImpl->mbKeyUp             = FALSE;        // TRUE: BaseKeyUp called
    mpWindowImpl->mbCommand           = FALSE;        // TRUE: BaseCommand called
    mpWindowImpl->mbDefPos            = TRUE;         // TRUE: Position is not Set
    mpWindowImpl->mbDefSize           = TRUE;         // TRUE: Size is not Set
    mpWindowImpl->mbCallMove          = TRUE;         // TRUE: Move must be called by Show
    mpWindowImpl->mbCallResize        = TRUE;         // TRUE: Resize must be called by Show
    mpWindowImpl->mbWaitSystemResize  = TRUE;         // TRUE: Wait for System-Resize
    mpWindowImpl->mbInitWinClipRegion = TRUE;         // TRUE: Calc Window Clip Region
    mpWindowImpl->mbInitChildRegion   = FALSE;        // TRUE: InitChildClipRegion
    mpWindowImpl->mbWinRegion         = FALSE;        // TRUE: Window Region
    mpWindowImpl->mbClipChildren      = FALSE;        // TRUE: Child-Fenster muessen evt. geclippt werden
    mpWindowImpl->mbClipSiblings      = FALSE;        // TRUE: Nebeneinanderliegende Child-Fenster muessen evt. geclippt werden
    mpWindowImpl->mbChildTransparent  = FALSE;        // TRUE: Child-Fenster duerfen transparent einschalten (inkl. Parent-CLIPCHILDREN)
    mpWindowImpl->mbPaintTransparent  = FALSE;        // TRUE: Paints muessen auf Parent ausgeloest werden
    mpWindowImpl->mbMouseTransparent  = FALSE;        // TRUE: Window is transparent for Mouse
    mpWindowImpl->mbDlgCtrlStart      = FALSE;        // TRUE: Ab hier eigenes Dialog-Control
    mpWindowImpl->mbFocusVisible      = FALSE;        // TRUE: Focus Visible
    mpWindowImpl->mbUseNativeFocus    = FALSE;
    mpWindowImpl->mbNativeFocusVisible= FALSE;        // TRUE: native Focus Visible
    mpWindowImpl->mbInShowFocus       = FALSE;        // prevent recursion
    mpWindowImpl->mbInHideFocus       = FALSE;        // prevent recursion
    mpWindowImpl->mbTrackVisible      = FALSE;        // TRUE: Tracking Visible
    mpWindowImpl->mbControlForeground = FALSE;        // TRUE: Foreground-Property set
    mpWindowImpl->mbControlBackground = FALSE;        // TRUE: Background-Property set
    mpWindowImpl->mbAlwaysOnTop       = FALSE;        // TRUE: immer vor allen anderen normalen Fenstern sichtbar
    mpWindowImpl->mbCompoundControl   = FALSE;        // TRUE: Zusammengesetztes Control => Listener...
    mpWindowImpl->mbCompoundControlHasFocus = FALSE;  // TRUE: Zusammengesetztes Control hat irgendwo den Focus
    mpWindowImpl->mbPaintDisabled     = FALSE;        // TRUE: Paint soll nicht ausgefuehrt werden
    mpWindowImpl->mbAllResize         = FALSE;        // TRUE: Auch ResizeEvents mit 0,0 schicken
    mpWindowImpl->mbInDtor            = FALSE;        // TRUE: Wir befinden uns im Window-Dtor
    mpWindowImpl->mbExtTextInput      = FALSE;        // TRUE: ExtTextInput-Mode is active
    mpWindowImpl->mbInFocusHdl        = FALSE;        // TRUE: Innerhalb vom GetFocus-Handler
    mpWindowImpl->mbCreatedWithToolkit = FALSE;
    mpWindowImpl->mbSuppressAccessibilityEvents = FALSE; // TRUE: do not send any accessibility events
    mpWindowImpl->mbDrawSelectionBackground = FALSE;    // TRUE: draws transparent window background to indicate (toolbox) selection
    mpWindowImpl->mbIsInTaskPaneList = FALSE;           // TRUE: window was added to the taskpanelist in the topmost system window
    mpWindowImpl->mnNativeBackground  = 0;              // initialize later, depends on type
    mpWindowImpl->mbCallHandlersDuringInputDisabled = FALSE; // TRUE: call event handlers even if input is disabled
    mpWindowImpl->mbDisableAccessibleLabelForRelation = FALSE; // TRUE: do not set LabelFor relation on accessible objects
    mpWindowImpl->mbDisableAccessibleLabeledByRelation = FALSE; // TRUE: do not set LabeledBy relation on accessible objects
    mpWindowImpl->mbHelpTextDynamic = FALSE;          // TRUE: append help id in HELP_DEBUG case
    mpWindowImpl->mbFakeFocusSet = FALSE; // TRUE: pretend as if the window has focus.

    mbEnableRTL         = Application::GetSettings().GetLayoutRTL();         // TRUE: this outdev will be mirrored if RTL window layout (UI mirroring) is globally active
}

// -----------------------------------------------------------------------

void Window::ImplInit( Window* pParent, WinBits nStyle, const ::com::sun::star::uno::Any& /*aSystemWorkWindowToken*/ )
{
    ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

void Window::ImplInit( Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData )
{
    DBG_ASSERT( mpWindowImpl->mbFrame || pParent, "Window::Window(): pParent == NULL" );

    ImplSVData* pSVData = ImplGetSVData();
    Window*     pRealParent = pParent;

    // 3D-Look vererben
    if ( !mpWindowImpl->mbOverlapWin && pParent && (pParent->GetStyle() & WB_3DLOOK) )
        nStyle |= WB_3DLOOK;

    // create border window if necessary
    if ( !mpWindowImpl->mbFrame && !mpWindowImpl->mbBorderWin && !mpWindowImpl->mpBorderWindow
         && (nStyle & (WB_BORDER | WB_SYSTEMCHILDWINDOW) ) )
    {
        USHORT nBorderTypeStyle = 0;
        if( (nStyle & WB_SYSTEMCHILDWINDOW) )
        {
            // handle WB_SYSTEMCHILDWINDOW
            // these should be analogous to a top level frame; meaning they
            // should have a border window with style BORDERWINDOW_STYLE_FRAME
            // which controls their size
            nBorderTypeStyle |= BORDERWINDOW_STYLE_FRAME;
            nStyle |= WB_BORDER;
        }
        ImplBorderWindow* pBorderWin = new ImplBorderWindow( pParent, nStyle & (WB_BORDER | WB_DIALOGCONTROL | WB_NODIALOGCONTROL | WB_NEEDSFOCUS), nBorderTypeStyle );
        ((Window*)pBorderWin)->mpWindowImpl->mpClientWindow = this;
        pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
        mpWindowImpl->mpBorderWindow  = pBorderWin;
        pParent = mpWindowImpl->mpBorderWindow;
    }
    else if( !mpWindowImpl->mbFrame && ! pParent )
    {
        mpWindowImpl->mbOverlapWin  = TRUE;
        mpWindowImpl->mbFrame = TRUE;
    }

    // insert window in list
    ImplInsertWindow( pParent );
    mpWindowImpl->mnStyle = nStyle;

    // Overlap-Window-Daten
    if ( mpWindowImpl->mbOverlapWin )
    {
        mpWindowImpl->mpOverlapData                   = new ImplOverlapData;
        mpWindowImpl->mpOverlapData->mpSaveBackDev    = NULL;
        mpWindowImpl->mpOverlapData->mpSaveBackRgn    = NULL;
        mpWindowImpl->mpOverlapData->mpNextBackWin    = NULL;
        mpWindowImpl->mpOverlapData->mnSaveBackSize   = 0;
        mpWindowImpl->mpOverlapData->mbSaveBack       = FALSE;
        mpWindowImpl->mpOverlapData->mnTopLevel       = 1;
    }

    if( pParent && ! mpWindowImpl->mbFrame )
        mbEnableRTL = pParent->mbEnableRTL;

    // test for frame creation
    if ( mpWindowImpl->mbFrame )
    {
        // create frame
        ULONG nFrameStyle = 0;

        if ( nStyle & WB_MOVEABLE )
            nFrameStyle |= SAL_FRAME_STYLE_MOVEABLE;
        if ( nStyle & WB_SIZEABLE )
            nFrameStyle |= SAL_FRAME_STYLE_SIZEABLE;
        if ( nStyle & WB_CLOSEABLE )
            nFrameStyle |= SAL_FRAME_STYLE_CLOSEABLE;
        if ( nStyle & WB_APP )
            nFrameStyle |= SAL_FRAME_STYLE_DEFAULT;
        // check for undecorated floating window
        if( // 1. floating windows that are not moveable/sizeable (only closeable allowed)
            ( !(nFrameStyle & ~SAL_FRAME_STYLE_CLOSEABLE) &&
            ( mpWindowImpl->mbFloatWin || ((GetType() == WINDOW_BORDERWINDOW) && ((ImplBorderWindow*)this)->mbFloatWindow) || (nStyle & WB_SYSTEMFLOATWIN) ) ) ||
            // 2. borderwindows of floaters with ownerdraw decoration
            ( ((GetType() == WINDOW_BORDERWINDOW) && ((ImplBorderWindow*)this)->mbFloatWindow && (nStyle & WB_OWNERDRAWDECORATION) ) ) )
        {
            nFrameStyle = SAL_FRAME_STYLE_FLOAT;
            if( nStyle & WB_OWNERDRAWDECORATION )
                nFrameStyle |= (SAL_FRAME_STYLE_OWNERDRAWDECORATION | SAL_FRAME_STYLE_NOSHADOW);
            if( nStyle & WB_NEEDSFOCUS )
                nFrameStyle |= SAL_FRAME_STYLE_FLOAT_FOCUSABLE;
        }
        else if( mpWindowImpl->mbFloatWin )
            nFrameStyle |= SAL_FRAME_STYLE_TOOLWINDOW;

        if( nStyle & WB_INTROWIN )
            nFrameStyle |= SAL_FRAME_STYLE_INTRO;
        if( nStyle & WB_TOOLTIPWIN )
            nFrameStyle |= SAL_FRAME_STYLE_TOOLTIP;

        if( nStyle & WB_NOSHADOW )
            nFrameStyle |= SAL_FRAME_STYLE_NOSHADOW;

        if( nStyle & WB_SYSTEMCHILDWINDOW )
            nFrameStyle |= SAL_FRAME_STYLE_SYSTEMCHILD;

        switch (mpWindowImpl->mnType)
        {
            case WINDOW_DIALOG:
            case WINDOW_TABDIALOG:
            case WINDOW_MODALDIALOG:
            case WINDOW_MODELESSDIALOG:
            case WINDOW_MESSBOX:
            case WINDOW_INFOBOX:
            case WINDOW_WARNINGBOX:
            case WINDOW_ERRORBOX:
            case WINDOW_QUERYBOX:
                nFrameStyle |= SAL_FRAME_STYLE_DIALOG;
            default:
                break;
        }

        SalFrame* pParentFrame = NULL;
        if ( pParent )
            pParentFrame = pParent->mpWindowImpl->mpFrame;
        SalFrame* pFrame;
        if ( pSystemParentData )
            pFrame = pSVData->mpDefInst->CreateChildFrame( pSystemParentData, nFrameStyle | SAL_FRAME_STYLE_PLUG );
        else
            pFrame = pSVData->mpDefInst->CreateFrame( pParentFrame, nFrameStyle );
        if ( !pFrame )
        {
            // do not abort but throw an exception, may be the current thread terminates anyway (plugin-scenario)
            throw ::com::sun::star::uno::RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "Could not create system window!" ) ),
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );
            //GetpApp()->Exception( EXC_SYSOBJNOTCREATED );
        }

        pFrame->SetCallback( this, ImplWindowFrameProc );

        // set window frame data
        mpWindowImpl->mpFrameData     = new ImplFrameData;
        mpWindowImpl->mpFrame         = pFrame;
        mpWindowImpl->mpFrameWindow   = this;
        mpWindowImpl->mpOverlapWindow = this;

        // set frame data
        mpWindowImpl->mpFrameData->mpNextFrame        = pSVData->maWinData.mpFirstFrame;
        pSVData->maWinData.mpFirstFrame = this;
        mpWindowImpl->mpFrameData->mpFirstOverlap     = NULL;
        mpWindowImpl->mpFrameData->mpFocusWin         = NULL;
        mpWindowImpl->mpFrameData->mpMouseMoveWin     = NULL;
        mpWindowImpl->mpFrameData->mpMouseDownWin     = NULL;
        mpWindowImpl->mpFrameData->mpFirstBackWin     = NULL;
        mpWindowImpl->mpFrameData->mpFontList         = pSVData->maGDIData.mpScreenFontList;
        mpWindowImpl->mpFrameData->mpFontCache        = pSVData->maGDIData.mpScreenFontCache;
        mpWindowImpl->mpFrameData->mnAllSaveBackSize  = 0;
        mpWindowImpl->mpFrameData->mnFocusId          = 0;
        mpWindowImpl->mpFrameData->mnMouseMoveId      = 0;
        mpWindowImpl->mpFrameData->mnLastMouseX       = -1;
        mpWindowImpl->mpFrameData->mnLastMouseY       = -1;
        mpWindowImpl->mpFrameData->mnBeforeLastMouseX = -1;
        mpWindowImpl->mpFrameData->mnBeforeLastMouseY = -1;
        mpWindowImpl->mpFrameData->mnFirstMouseX      = -1;
        mpWindowImpl->mpFrameData->mnFirstMouseY      = -1;
        mpWindowImpl->mpFrameData->mnLastMouseWinX    = -1;
        mpWindowImpl->mpFrameData->mnLastMouseWinY    = -1;
        mpWindowImpl->mpFrameData->mnModalMode        = 0;
        mpWindowImpl->mpFrameData->mnMouseDownTime    = 0;
        mpWindowImpl->mpFrameData->mnClickCount       = 0;
        mpWindowImpl->mpFrameData->mnFirstMouseCode   = 0;
        mpWindowImpl->mpFrameData->mnMouseCode        = 0;
        mpWindowImpl->mpFrameData->mnMouseMode        = 0;
        mpWindowImpl->mpFrameData->meMapUnit          = MAP_PIXEL;
        mpWindowImpl->mpFrameData->mbHasFocus         = FALSE;
        mpWindowImpl->mpFrameData->mbInMouseMove      = FALSE;
        mpWindowImpl->mpFrameData->mbMouseIn          = FALSE;
        mpWindowImpl->mpFrameData->mbStartDragCalled  = FALSE;
        mpWindowImpl->mpFrameData->mbNeedSysWindow    = FALSE;
        mpWindowImpl->mpFrameData->mbMinimized        = FALSE;
        mpWindowImpl->mpFrameData->mbStartFocusState  = FALSE;
        mpWindowImpl->mpFrameData->mbInSysObjFocusHdl = FALSE;
        mpWindowImpl->mpFrameData->mbInSysObjToTopHdl = FALSE;
        mpWindowImpl->mpFrameData->mbSysObjFocus      = FALSE;
        mpWindowImpl->mpFrameData->maPaintTimer.SetTimeout( 30 );
        mpWindowImpl->mpFrameData->maPaintTimer.SetTimeoutHdl( LINK( this, Window, ImplHandlePaintHdl ) );
        mpWindowImpl->mpFrameData->maResizeTimer.SetTimeout( 50 );
        mpWindowImpl->mpFrameData->maResizeTimer.SetTimeoutHdl( LINK( this, Window, ImplHandleResizeTimerHdl ) );
        mpWindowImpl->mpFrameData->mbInternalDragGestureRecognizer = FALSE;

        if ( pRealParent && IsTopWindow() )
        {
            ImplWinData* pParentWinData = pRealParent->ImplGetWinData();
            pParentWinData->maTopWindowChildren.push_back( this );
        }
    }

    // init data
    mpWindowImpl->mpRealParent = pRealParent;

    // #99318: make sure fontcache and list is available before call to SetSettings
    mpFontList      = mpWindowImpl->mpFrameData->mpFontList;
    mpFontCache     = mpWindowImpl->mpFrameData->mpFontCache;

    if ( mpWindowImpl->mbFrame )
    {
        if ( pParent )
        {
            mpWindowImpl->mpFrameData->mnDPIX     = pParent->mpWindowImpl->mpFrameData->mnDPIX;
            mpWindowImpl->mpFrameData->mnDPIY     = pParent->mpWindowImpl->mpFrameData->mnDPIY;
        }
        else
        {
            if ( ImplGetGraphics() )
            {
                mpGraphics->GetResolution( mpWindowImpl->mpFrameData->mnDPIX, mpWindowImpl->mpFrameData->mnDPIY );
            }
        }

        // add ownerdraw decorated frame windows to list in the top-most frame window
        // so they can be hidden on lose focus
        if( nStyle & WB_OWNERDRAWDECORATION )
            ImplGetOwnerDrawList().push_back( this );

        // delay settings initialization until first "real" frame
        // this relies on the IntroWindow not needing any system settings
        if ( !pSVData->maAppData.mbSettingsInit &&
             ! (nStyle & (WB_INTROWIN|WB_DEFAULTWIN))
             )
        {
            // side effect: ImplUpdateGlobalSettings does an ImplGetFrame()->UpdateSettings
            ImplUpdateGlobalSettings( *pSVData->maAppData.mpSettings );
            OutputDevice::SetSettings( *pSVData->maAppData.mpSettings );
            pSVData->maAppData.mbSettingsInit = TRUE;
        }

        // If we create a Window with default size, query this
        // size directly, because we want resize all Controls to
        // the correct size before we display the window
        if ( nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_APP) )
            mpWindowImpl->mpFrame->GetClientSize( mnOutWidth, mnOutHeight );
    }
    else
    {
        if ( pParent )
        {
            if ( !ImplIsOverlapWindow() )
            {
                mpWindowImpl->mbDisabled          = pParent->mpWindowImpl->mbDisabled;
                mpWindowImpl->mbInputDisabled     = pParent->mpWindowImpl->mbInputDisabled;
                mpWindowImpl->meAlwaysInputMode   = pParent->mpWindowImpl->meAlwaysInputMode;
            }

            OutputDevice::SetSettings( pParent->GetSettings() );
        }

    }

    const StyleSettings& rStyleSettings = maSettings.GetStyleSettings();
    USHORT nScreenZoom = rStyleSettings.GetScreenZoom();
    mnDPIX          = (mpWindowImpl->mpFrameData->mnDPIX*nScreenZoom)/100;
    mnDPIY          = (mpWindowImpl->mpFrameData->mnDPIY*nScreenZoom)/100;
    maFont          = rStyleSettings.GetAppFont();
    ImplPointToLogic( maFont );

    if ( nStyle & WB_3DLOOK )
    {
        SetTextColor( rStyleSettings.GetButtonTextColor() );
        SetBackground( Wallpaper( rStyleSettings.GetFaceColor() ) );
    }
    else
    {
        SetTextColor( rStyleSettings.GetWindowTextColor() );
        SetBackground( Wallpaper( rStyleSettings.GetWindowColor() ) );
    }

    ImplUpdatePos();

    // calculate app font res (except for the Intro Window or the default window)
    if ( mpWindowImpl->mbFrame && !pSVData->maGDIData.mnAppFontX && ! (nStyle & (WB_INTROWIN|WB_DEFAULTWIN)) )
        ImplInitAppFontData( this );

    if ( GetAccessibleParentWindow()  && GetParent() != Application::GetDefDialogParent() )
        GetAccessibleParentWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_CHILDCREATED, this );
}

// -----------------------------------------------------------------------

void Window::ImplSetFrameParent( const Window* pParent )
{
    Window* pFrameWindow = ImplGetSVData()->maWinData.mpFirstFrame;
    while( pFrameWindow )
    {
        // search all frames that are children of this window
        // and reparent them
        if( ImplIsRealParentPath( pFrameWindow ) )
        {
            DBG_ASSERT( mpWindowImpl->mpFrame != pFrameWindow->mpWindowImpl->mpFrame, "SetFrameParent to own" );
            DBG_ASSERT( mpWindowImpl->mpFrame, "no frame" );
            SalFrame* pParentFrame = pParent ? pParent->mpWindowImpl->mpFrame : NULL;
            pFrameWindow->mpWindowImpl->mpFrame->SetParent( pParentFrame );
        }
        pFrameWindow = pFrameWindow->mpWindowImpl->mpFrameData->mpNextFrame;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInsertWindow( Window* pParent )
{
    mpWindowImpl->mpParent            = pParent;
    mpWindowImpl->mpRealParent        = pParent;

    if ( pParent && !mpWindowImpl->mbFrame )
    {
        // search frame window and set window frame data
        Window* pFrameParent = pParent->mpWindowImpl->mpFrameWindow;
        mpWindowImpl->mpFrameData     = pFrameParent->mpWindowImpl->mpFrameData;
        mpWindowImpl->mpFrame         = pFrameParent->mpWindowImpl->mpFrame;
        mpWindowImpl->mpFrameWindow   = pFrameParent;
        mpWindowImpl->mbFrame         = FALSE;

        // search overlap window and insert window in list
        if ( ImplIsOverlapWindow() )
        {
            Window* pFirstOverlapParent = pParent;
            while ( !pFirstOverlapParent->ImplIsOverlapWindow() )
                pFirstOverlapParent = pFirstOverlapParent->ImplGetParent();
            mpWindowImpl->mpOverlapWindow = pFirstOverlapParent;

            mpWindowImpl->mpNextOverlap = mpWindowImpl->mpFrameData->mpFirstOverlap;
            mpWindowImpl->mpFrameData->mpFirstOverlap = this;

            // Overlap-Windows sind per default die obersten
            mpWindowImpl->mpNext = pFirstOverlapParent->mpWindowImpl->mpFirstOverlap;
            pFirstOverlapParent->mpWindowImpl->mpFirstOverlap = this;
            if ( !pFirstOverlapParent->mpWindowImpl->mpLastOverlap )
                pFirstOverlapParent->mpWindowImpl->mpLastOverlap = this;
            else
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = this;
        }
        else
        {
            if ( pParent->ImplIsOverlapWindow() )
                mpWindowImpl->mpOverlapWindow = pParent;
            else
                mpWindowImpl->mpOverlapWindow = pParent->mpWindowImpl->mpOverlapWindow;
            mpWindowImpl->mpPrev = pParent->mpWindowImpl->mpLastChild;
            pParent->mpWindowImpl->mpLastChild = this;
            if ( !pParent->mpWindowImpl->mpFirstChild )
                pParent->mpWindowImpl->mpFirstChild = this;
            else
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = this;
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplRemoveWindow( BOOL bRemoveFrameData )
{
    // Fenster aus den Listen austragen
    if ( !mpWindowImpl->mbFrame )
    {
        if ( ImplIsOverlapWindow() )
        {
            if ( mpWindowImpl->mpFrameData->mpFirstOverlap == this )
                mpWindowImpl->mpFrameData->mpFirstOverlap = mpWindowImpl->mpNextOverlap;
            else
            {
                Window* pTempWin = mpWindowImpl->mpFrameData->mpFirstOverlap;
                while ( pTempWin->mpWindowImpl->mpNextOverlap != this )
                    pTempWin = pTempWin->mpWindowImpl->mpNextOverlap;
                pTempWin->mpWindowImpl->mpNextOverlap = mpWindowImpl->mpNextOverlap;
            }

            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = mpWindowImpl->mpPrev;
        }
        else
        {
            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            else
                mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = mpWindowImpl->mpPrev;
        }

        mpWindowImpl->mpPrev = NULL;
        mpWindowImpl->mpNext = NULL;
    }

    if ( bRemoveFrameData )
    {
        // Graphic freigeben
        ImplReleaseGraphics();
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallResize()
{
    mpWindowImpl->mbCallResize = FALSE;

    if( GetBackground().IsGradient() )
        Invalidate();

    Resize();

    // #88419# Most classes don't call the base class in Resize() and Move(),
    // => Call ImpleResize/Move instead of Resize/Move directly...
    ImplCallEventListeners( VCLEVENT_WINDOW_RESIZE );
}

// -----------------------------------------------------------------------

void Window::ImplCallMove()
{
    mpWindowImpl->mbCallMove = FALSE;

    if( mpWindowImpl->mbFrame )
    {
        // update frame position
        SalFrame *pParentFrame = NULL;
        Window *pParent = ImplGetParent();
        while( pParent )
        {
            if( pParent->mpWindowImpl->mpFrame != mpWindowImpl->mpFrame )
            {
                pParentFrame = pParent->mpWindowImpl->mpFrame;
                break;
            }
            pParent = pParent->GetParent();
        }

        SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
        mpWindowImpl->maPos = Point( g.nX, g.nY );
        if( pParentFrame )
        {
            g = pParentFrame->GetGeometry();
            mpWindowImpl->maPos -= Point( g.nX, g.nY );
        }
        // the client window and and all its subclients have the same position as the borderframe
        // this is important for floating toolbars where the borderwindow is a floating window
        // which has another borderwindow (ie the system floating window)
        Window *pClientWin = mpWindowImpl->mpClientWindow;
        while( pClientWin )
        {
            pClientWin->mpWindowImpl->maPos = mpWindowImpl->maPos;
            pClientWin = pClientWin->mpWindowImpl->mpClientWindow;
        }
    }

    Move();

    ImplCallEventListeners( VCLEVENT_WINDOW_MOVE );
}

// -----------------------------------------------------------------------

static ULONG ImplAutoHelpID( ResMgr* pResMgr )
{
    if ( !Application::IsAutoHelpIdEnabled() )
        return 0;

    ULONG nHID = 0;

    DBG_ASSERT( pResMgr, "No res mgr for auto help id" );
    if( ! pResMgr )
        return 0;

    nHID = pResMgr->GetAutoHelpId();

    return nHID;
}

// -----------------------------------------------------------------------

WinBits Window::ImplInitRes( const ResId& rResId )
{
    GetRes( rResId );

    char* pRes = (char*)GetClassRes();
    pRes += 8;
    sal_uInt32 nStyle = (sal_uInt32)GetLongRes( (void*)pRes );
    rResId.SetWinBits( nStyle );
    return nStyle;
}

// -----------------------------------------------------------------------

void Window::ImplLoadRes( const ResId& rResId )
{
    // newer move this line after IncrementRes
    char* pRes = (char*)GetClassRes();
    pRes += 12;
    sal_uInt32 nHelpId = (sal_uInt32)GetLongRes( (void*)pRes );
    if ( !nHelpId )
        nHelpId = ImplAutoHelpID( rResId.GetResMgr() );
    SetHelpId( nHelpId );

    ULONG nObjMask = ReadLongRes();

    // ResourceStyle
    ULONG nRSStyle = ReadLongRes();
    // WinBits
    ReadLongRes();
    // HelpId
    ReadLongRes();

    BOOL  bPos  = FALSE;
    BOOL  bSize = FALSE;
    Point aPos;
    Size  aSize;

    if ( nObjMask & (WINDOW_XYMAPMODE | WINDOW_X | WINDOW_Y) )
    {
        // Groessenangabe aus der Resource verwenden
        MapUnit ePosMap = MAP_PIXEL;

        bPos = TRUE;

        if ( nObjMask & WINDOW_XYMAPMODE )
            ePosMap = (MapUnit)ReadLongRes();
        if ( nObjMask & WINDOW_X )
            aPos.X() = ImplLogicUnitToPixelX( ReadLongRes(), ePosMap );
        if ( nObjMask & WINDOW_Y )
            aPos.Y() = ImplLogicUnitToPixelY( ReadLongRes(), ePosMap );
    }

    if ( nObjMask & (WINDOW_WHMAPMODE | WINDOW_WIDTH | WINDOW_HEIGHT) )
    {
        // Groessenangabe aus der Resource verwenden
        MapUnit eSizeMap = MAP_PIXEL;

        bSize = TRUE;

        if ( nObjMask & WINDOW_WHMAPMODE )
            eSizeMap = (MapUnit)ReadLongRes();
        if ( nObjMask & WINDOW_WIDTH )
            aSize.Width() = ImplLogicUnitToPixelX( ReadLongRes(), eSizeMap );
        if ( nObjMask & WINDOW_HEIGHT )
            aSize.Height() = ImplLogicUnitToPixelY( ReadLongRes(), eSizeMap );
    }

    // Wegen Optimierung so schlimm aussehend
    if ( nRSStyle & RSWND_CLIENTSIZE )
    {
        if ( bPos )
            SetPosPixel( aPos );
        if ( bSize )
            SetOutputSizePixel( aSize );
    }
    else if ( bPos && bSize )
        SetPosSizePixel( aPos, aSize );
    else if ( bPos )
        SetPosPixel( aPos );
    else if ( bSize )
        SetSizePixel( aSize );

    if ( nRSStyle & RSWND_DISABLED )
        Enable( FALSE );

    if ( nObjMask & WINDOW_TEXT )
        SetText( ReadStringRes() );
    if ( nObjMask & WINDOW_HELPTEXT )
    {
        SetHelpText( ReadStringRes() );
        mpWindowImpl->mbHelpTextDynamic = TRUE;
    }
    if ( nObjMask & WINDOW_QUICKTEXT )
        SetQuickHelpText( ReadStringRes() );
    if ( nObjMask & WINDOW_EXTRALONG )
    {
        sal_uIntPtr nRes = ReadLongRes();
        SetData( (void*)nRes );
    }
    if ( nObjMask & WINDOW_UNIQUEID )
        SetUniqueId( (ULONG)ReadLongRes() );

    if ( nObjMask & WINDOW_BORDER_STYLE )
    {
        USHORT nBorderStyle = (USHORT)ReadLongRes();
        SetBorderStyle( nBorderStyle );
    }
}

// -----------------------------------------------------------------------

ImplWinData* Window::ImplGetWinData() const
{
    if ( !mpWindowImpl->mpWinData )
    {
        static const char* pNoNWF = getenv( "SAL_NO_NWF" );

        ((Window*)this)->mpWindowImpl->mpWinData = new ImplWinData;
        mpWindowImpl->mpWinData->mpExtOldText     = NULL;
        mpWindowImpl->mpWinData->mpExtOldAttrAry  = NULL;
        mpWindowImpl->mpWinData->mpCursorRect     = 0;
        mpWindowImpl->mpWinData->mnCursorExtWidth = 0;
        mpWindowImpl->mpWinData->mpFocusRect      = NULL;
        mpWindowImpl->mpWinData->mpTrackRect      = NULL;
        mpWindowImpl->mpWinData->mnTrackFlags     = 0;
        mpWindowImpl->mpWinData->mnIsTopWindow  = (USHORT) ~0;  // not initialized yet, 0/1 will indicate TopWindow (see IsTopWindow())
        mpWindowImpl->mpWinData->mbMouseOver      = FALSE;
        mpWindowImpl->mpWinData->mbEnableNativeWidget = (pNoNWF && *pNoNWF) ? FALSE : TRUE; // TRUE: try to draw this control with native theme API
        mpWindowImpl->mpWinData->mpSmartHelpId    = NULL;
        mpWindowImpl->mpWinData->mpSmartUniqueId  = NULL;
   }

    return mpWindowImpl->mpWinData;
}

// -----------------------------------------------------------------------

SalGraphics* Window::ImplGetFrameGraphics() const
{
    if ( mpWindowImpl->mpFrameWindow->mpGraphics )
        mpWindowImpl->mpFrameWindow->mbInitClipRegion = TRUE;
    else
        mpWindowImpl->mpFrameWindow->ImplGetGraphics();
    mpWindowImpl->mpFrameWindow->mpGraphics->ResetClipRegion();
    return mpWindowImpl->mpFrameWindow->mpGraphics;
}

// -----------------------------------------------------------------------

Window* Window::ImplFindWindow( const Point& rFramePos )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Window* pTempWindow;
    Window* pFindWindow;

    // Zuerst alle ueberlappenden Fenster ueberpruefen
    pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        pFindWindow = pTempWindow->ImplFindWindow( rFramePos );
        if ( pFindWindow )
            return pFindWindow;
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }

    // dann testen wir unser Fenster
    if ( !mpWindowImpl->mbVisible )
        return NULL;

    USHORT nHitTest = ImplHitTest( rFramePos );
    if ( nHitTest & WINDOW_HITTEST_INSIDE )
    {
        // und danach gehen wir noch alle Child-Fenster durch
        pTempWindow = mpWindowImpl->mpFirstChild;
        while ( pTempWindow )
        {
            pFindWindow = pTempWindow->ImplFindWindow( rFramePos );
            if ( pFindWindow )
                return pFindWindow;
            pTempWindow = pTempWindow->mpWindowImpl->mpNext;
        }

        if ( nHitTest & WINDOW_HITTEST_TRANSPARENT )
            return NULL;
        else
            return this;
    }

    return NULL;
}

// -----------------------------------------------------------------------

USHORT Window::ImplHitTest( const Point& rFramePos )
{
    Point aFramePos( rFramePos );
    if( ImplIsAntiparallel() )
    {
        // - RTL - re-mirror frame pos at this window
        ImplReMirror( aFramePos );
    }
    Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
    if ( !aRect.IsInside( aFramePos ) )
        return 0;
    if ( mpWindowImpl->mbWinRegion )
    {
        Point aTempPos = aFramePos;
        aTempPos.X() -= mnOutOffX;
        aTempPos.Y() -= mnOutOffY;
        if ( !mpWindowImpl->maWinRegion.IsInside( aTempPos ) )
            return 0;
    }

    USHORT nHitTest = WINDOW_HITTEST_INSIDE;
    if ( mpWindowImpl->mbMouseTransparent )
        nHitTest |= WINDOW_HITTEST_TRANSPARENT;
    return nHitTest;
}

// -----------------------------------------------------------------------

BOOL Window::ImplIsRealParentPath( const Window* pWindow ) const
{
    pWindow = pWindow->GetParent();
    while ( pWindow )
    {
        if ( pWindow == this )
            return TRUE;
        pWindow = pWindow->GetParent();
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Window::ImplIsChild( const Window* pWindow, BOOL bSystemWindow ) const
{
    do
    {
        if ( !bSystemWindow && pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();

        if ( pWindow == this )
            return TRUE;
    }
    while ( pWindow );

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Window::ImplIsWindowOrChild( const Window* pWindow, BOOL bSystemWindow ) const
{
    if ( this == pWindow )
        return TRUE;
    return ImplIsChild( pWindow, bSystemWindow );
}

// -----------------------------------------------------------------------

Window* Window::ImplGetSameParent( const Window* pWindow ) const
{
    if ( mpWindowImpl->mpFrameWindow != pWindow->mpWindowImpl->mpFrameWindow )
        return NULL;
    else
    {
        if ( pWindow->ImplIsChild( this ) )
            return (Window*)pWindow;
        else
        {
            Window* pTestWindow = (Window*)this;
            while ( (pTestWindow == pWindow) || pTestWindow->ImplIsChild( pWindow ) )
                pTestWindow = pTestWindow->ImplGetParent();
            return pTestWindow;
        }
    }
}

// -----------------------------------------------------------------------

int Window::ImplTestMousePointerSet()
{
    // Wenn Mouse gecaptured ist, dann soll MousePointer umgeschaltet werden
    if ( IsMouseCaptured() )
        return TRUE;

    // Wenn sich Mouse ueber dem Fenster befindet, dann soll MousePointer
    // umgeschaltet werden
    Rectangle aClientRect( Point( 0, 0 ), GetOutputSizePixel() );
    if ( aClientRect.IsInside( GetPointerPosPixel() ) )
        return TRUE;

    return FALSE;
}

// -----------------------------------------------------------------------

PointerStyle Window::ImplGetMousePointer() const
{
    PointerStyle    ePointerStyle;
    BOOL            bWait = FALSE;

    if ( IsEnabled() && IsInputEnabled() && ! IsInModalMode() )
        ePointerStyle = GetPointer().GetStyle();
    else
        ePointerStyle = POINTER_ARROW;

    const Window* pWindow = this;
    do
    {
        // Wenn Pointer nicht sichtbar, dann wird suche abgebrochen, da
        // dieser Status nicht ueberschrieben werden darf
        if ( pWindow->mpWindowImpl->mbNoPtrVisible )
            return POINTER_NULL;

        if ( !bWait )
        {
            if ( pWindow->mpWindowImpl->mnWaitCount )
            {
                ePointerStyle = POINTER_WAIT;
                bWait = TRUE;
            }
            else
            {
                if ( pWindow->mpWindowImpl->mbChildPtrOverwrite )
                    ePointerStyle = pWindow->GetPointer().GetStyle();
            }
        }

        if ( pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();
    }
    while ( pWindow );

    return ePointerStyle;
}

// -----------------------------------------------------------------------

void Window::ImplResetReallyVisible()
{
    BOOL bBecameReallyInvisible = mpWindowImpl->mbReallyVisible;

    mbDevOutput     = FALSE;
    mpWindowImpl->mbReallyVisible = FALSE;
    mpWindowImpl->mbReallyShown   = FALSE;

    // the SHOW/HIDE events serve as indicators to send child creation/destroy events to the access bridge.
    // For this, the data member of the event must not be NULL.
    // Previously, we did this in Window::Show, but there some events got lost in certain situations.
    // #104887# - 2004-08-10 - fs@openoffice.org
    if( bBecameReallyInvisible && ImplIsAccessibleCandidate() )
        ImplCallEventListeners( VCLEVENT_WINDOW_HIDE, this );
        // TODO. It's kind of a hack that we're re-using the VCLEVENT_WINDOW_HIDE. Normally, we should
        // introduce another event which explicitly triggers the Accessibility implementations.

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
            pWindow->ImplResetReallyVisible();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
            pWindow->ImplResetReallyVisible();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplSetReallyVisible()
{
    // #i43594# it is possible that INITSHOW was never send, because the visibility state changed between
    // ImplCallInitShow() and ImplSetReallyVisible() when called from Show()
    // mbReallyShown is a useful indicator
    if( !mpWindowImpl->mbReallyShown )
        ImplCallInitShow();

    BOOL bBecameReallyVisible = !mpWindowImpl->mbReallyVisible;

    mbDevOutput     = TRUE;
    mpWindowImpl->mbReallyVisible = TRUE;
    mpWindowImpl->mbReallyShown   = TRUE;

    // the SHOW/HIDE events serve as indicators to send child creation/destroy events to the access bridge.
    // For this, the data member of the event must not be NULL.
    // Previously, we did this in Window::Show, but there some events got lost in certain situations. Now
    // we're doing it when the visibility really changes
    // #104887# - 2004-08-10 - fs@openoffice.org
    if( bBecameReallyVisible && ImplIsAccessibleCandidate() )
        ImplCallEventListeners( VCLEVENT_WINDOW_SHOW, this );
        // TODO. It's kind of a hack that we're re-using the VCLEVENT_WINDOW_SHOW. Normally, we should
        // introduce another event which explicitly triggers the Accessibility implementations.

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbVisible )
            pWindow->ImplSetReallyVisible();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbVisible )
            pWindow->ImplSetReallyVisible();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallInitShow()
{
    mpWindowImpl->mbReallyShown   = TRUE;
    mpWindowImpl->mbInInitShow    = TRUE;
    StateChanged( STATE_CHANGE_INITSHOW );
    mpWindowImpl->mbInInitShow    = FALSE;

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbVisible )
            pWindow->ImplCallInitShow();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbVisible )
            pWindow->ImplCallInitShow();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplAddDel( ImplDelData* pDel ) // TODO: make "const" when incompatiblity ok
{
    DBG_ASSERT( !pDel->mpWindow, "Window::ImplAddDel(): cannot add ImplDelData twice !" );
    if( !pDel->mpWindow )
    {
        pDel->mpWindow = this;  // #112873# store ref to this window, so pDel can remove itself
        pDel->mpNext = mpWindowImpl->mpFirstDel;
        mpWindowImpl->mpFirstDel = pDel;
    }
}

// -----------------------------------------------------------------------

void Window::ImplRemoveDel( ImplDelData* pDel ) // TODO: make "const" when incompatiblity ok
{
    pDel->mpWindow = NULL;      // #112873# pDel is not associated with a Window anymore
    if ( mpWindowImpl->mpFirstDel == pDel )
        mpWindowImpl->mpFirstDel = pDel->mpNext;
    else
    {
        ImplDelData* pData = mpWindowImpl->mpFirstDel;
        while ( pData->mpNext != pDel )
            pData = pData->mpNext;
        pData->mpNext = pDel->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInitResolutionSettings()
{
    // AppFont-Aufloesung und DPI-Aufloesung neu berechnen
    if ( mpWindowImpl->mbFrame )
    {
        const StyleSettings& rStyleSettings = maSettings.GetStyleSettings();
        USHORT nScreenZoom = rStyleSettings.GetScreenZoom();
        mnDPIX = (mpWindowImpl->mpFrameData->mnDPIX*nScreenZoom)/100;
        mnDPIY = (mpWindowImpl->mpFrameData->mnDPIY*nScreenZoom)/100;
        SetPointFont( rStyleSettings.GetAppFont() );
    }
    else if ( mpWindowImpl->mpParent )
    {
        mnDPIX  = mpWindowImpl->mpParent->mnDPIX;
        mnDPIY  = mpWindowImpl->mpParent->mnDPIY;
    }

    // Vorberechnete Werte fuer logische Einheiten updaten und auch
    // die entsprechenden Tools dazu
    if ( IsMapMode() )
    {
        MapMode aMapMode = GetMapMode();
        SetMapMode();
        SetMapMode( aMapMode );
    }
}

// -----------------------------------------------------------------------

void Window::ImplPointToLogic( Font& rFont ) const
{
    Size    aSize = rFont.GetSize();
    USHORT  nScreenFontZoom = maSettings.GetStyleSettings().GetScreenFontZoom();

    if ( aSize.Width() )
    {
        aSize.Width() *= mpWindowImpl->mpFrameData->mnDPIX;
        aSize.Width() += 72/2;
        aSize.Width() /= 72;
        aSize.Width() *= nScreenFontZoom;
        aSize.Width() /= 100;
    }
    aSize.Height() *= mpWindowImpl->mpFrameData->mnDPIY;
    aSize.Height() += 72/2;
    aSize.Height() /= 72;
    aSize.Height() *= nScreenFontZoom;
    aSize.Height() /= 100;

    if ( IsMapModeEnabled() )
        aSize = PixelToLogic( aSize );

    rFont.SetSize( aSize );
}

// -----------------------------------------------------------------------

void Window::ImplLogicToPoint( Font& rFont ) const
{
    Size    aSize = rFont.GetSize();
    USHORT  nScreenFontZoom = maSettings.GetStyleSettings().GetScreenFontZoom();

    if ( IsMapModeEnabled() )
        aSize = LogicToPixel( aSize );

    if ( aSize.Width() )
    {
        aSize.Width() *= 100;
        aSize.Width() /= nScreenFontZoom;
        aSize.Width() *= 72;
        aSize.Width() += mpWindowImpl->mpFrameData->mnDPIX/2;
        aSize.Width() /= mpWindowImpl->mpFrameData->mnDPIX;
    }
    aSize.Height() *= 100;
    aSize.Height() /= nScreenFontZoom;
    aSize.Height() *= 72;
    aSize.Height() += mpWindowImpl->mpFrameData->mnDPIY/2;
    aSize.Height() /= mpWindowImpl->mpFrameData->mnDPIY;

    rFont.SetSize( aSize );
}

// -----------------------------------------------------------------------

BOOL Window::ImplSysObjClip( const Region* pOldRegion )
{
    BOOL bUpdate = TRUE;

    if ( mpWindowImpl->mpSysObj )
    {
        bool bVisibleState = mpWindowImpl->mbReallyVisible;

        if ( bVisibleState )
        {
            Region* pWinChildClipRegion = ImplGetWinChildClipRegion();

            if ( !pWinChildClipRegion->IsEmpty() )
            {
                if ( pOldRegion )
                {
                    Region aNewRegion = *pWinChildClipRegion;
                    pWinChildClipRegion->Intersect( *pOldRegion );
                    bUpdate = aNewRegion == *pWinChildClipRegion;
                }

                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();

                Region      aRegion = *pWinChildClipRegion;
                Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                Region      aWinRectRegion( aWinRect );
                USHORT      nClipFlags = mpWindowImpl->mpSysObj->GetClipRegionType();

                if ( aRegion == aWinRectRegion )
                    mpWindowImpl->mpSysObj->ResetClipRegion();
                else
                {
                    if ( nClipFlags & SAL_OBJECT_CLIP_EXCLUDERECTS )
                    {
                        aWinRectRegion.Exclude( aRegion );
                        aRegion = aWinRectRegion;
                    }
                    if ( !(nClipFlags & SAL_OBJECT_CLIP_ABSOLUTE) )
                        aRegion.Move( -mnOutOffX, -mnOutOffY );

                    // ClipRegion setzen/updaten
                    long                nX;
                    long                nY;
                    long                nWidth;
                    long                nHeight;
                    ULONG               nRectCount;
                    ImplRegionInfo      aInfo;
                    BOOL                bRegionRect;

                    nRectCount = aRegion.GetRectCount();
                    mpWindowImpl->mpSysObj->BeginSetClipRegion( nRectCount );
                    bRegionRect = aRegion.ImplGetFirstRect( aInfo, nX, nY, nWidth, nHeight );
                    while ( bRegionRect )
                    {
                        mpWindowImpl->mpSysObj->UnionClipRegion( nX, nY, nWidth, nHeight );
                        bRegionRect = aRegion.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
                    }
                    mpWindowImpl->mpSysObj->EndSetClipRegion();
                }
            }
            else
                bVisibleState = FALSE;
        }

        // Visible-Status updaten
        mpWindowImpl->mpSysObj->Show( bVisibleState );
    }

    return bUpdate;
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjChildsClip()
{
    if ( mpWindowImpl->mpSysObj && mpWindowImpl->mbInitWinClipRegion )
        ImplSysObjClip( NULL );

    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        pWindow->ImplUpdateSysObjChildsClip();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjOverlapsClip()
{
    ImplUpdateSysObjChildsClip();

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        pWindow->ImplUpdateSysObjOverlapsClip();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjClip()
{
    if ( !ImplIsOverlapWindow() )
    {
        ImplUpdateSysObjChildsClip();

        // Schwestern muessen ihre ClipRegion auch neu berechnen
        if ( mpWindowImpl->mbClipSiblings )
        {
            Window* pWindow = mpWindowImpl->mpNext;
            while ( pWindow )
            {
                pWindow->ImplUpdateSysObjChildsClip();
                pWindow = pWindow->mpWindowImpl->mpNext;
            }
        }
    }
    else
        mpWindowImpl->mpFrameWindow->ImplUpdateSysObjOverlapsClip();
}

// -----------------------------------------------------------------------

BOOL Window::ImplSetClipFlagChilds( BOOL bSysObjOnlySmaller )
{
    BOOL bUpdate = TRUE;
    if ( mpWindowImpl->mpSysObj )
    {
        Region* pOldRegion = NULL;
        if ( bSysObjOnlySmaller && !mpWindowImpl->mbInitWinClipRegion )
            pOldRegion = new Region( mpWindowImpl->maWinClipRegion );

        mbInitClipRegion = TRUE;
        mpWindowImpl->mbInitWinClipRegion = TRUE;

        Window* pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            if ( !pWindow->ImplSetClipFlagChilds( bSysObjOnlySmaller ) )
                bUpdate = FALSE;
            pWindow = pWindow->mpWindowImpl->mpNext;
        }

        if ( !ImplSysObjClip( pOldRegion ) )
        {
            mbInitClipRegion = TRUE;
            mpWindowImpl->mbInitWinClipRegion = TRUE;
            bUpdate = FALSE;
        }

        if ( pOldRegion )
            delete pOldRegion;
    }
    else
    {
    mbInitClipRegion = TRUE;
    mpWindowImpl->mbInitWinClipRegion = TRUE;

    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagChilds( bSysObjOnlySmaller ) )
            bUpdate = FALSE;
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
    }
    return bUpdate;
}

// -----------------------------------------------------------------------

BOOL Window::ImplSetClipFlagOverlapWindows( BOOL bSysObjOnlySmaller )
{
    BOOL bUpdate = ImplSetClipFlagChilds( bSysObjOnlySmaller );

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller ) )
            bUpdate = FALSE;
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    return bUpdate;
}

// -----------------------------------------------------------------------

BOOL Window::ImplSetClipFlag( BOOL bSysObjOnlySmaller )
{
    if ( !ImplIsOverlapWindow() )
    {
        BOOL bUpdate = ImplSetClipFlagChilds( bSysObjOnlySmaller );

        Window* pParent = ImplGetParent();
        if ( pParent &&
             ((pParent->GetStyle() & WB_CLIPCHILDREN) || (mpWindowImpl->mnParentClipMode & PARENTCLIPMODE_CLIP)) )
        {
            pParent->mbInitClipRegion = TRUE;
            pParent->mpWindowImpl->mbInitChildRegion = TRUE;
        }

        // Schwestern muessen ihre ClipRegion auch neu berechnen
        if ( mpWindowImpl->mbClipSiblings )
        {
            Window* pWindow = mpWindowImpl->mpNext;
            while ( pWindow )
            {
                if ( !pWindow->ImplSetClipFlagChilds( bSysObjOnlySmaller ) )
                    bUpdate = FALSE;
                pWindow = pWindow->mpWindowImpl->mpNext;
            }
        }

        return bUpdate;
    }
    else
        return mpWindowImpl->mpFrameWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller );
}

// -----------------------------------------------------------------------

void Window::ImplIntersectWindowClipRegion( Region& rRegion )
{
    if ( mpWindowImpl->mbInitWinClipRegion )
        ImplInitWinClipRegion();

    rRegion.Intersect( mpWindowImpl->maWinClipRegion );
}

// -----------------------------------------------------------------------

void Window::ImplIntersectWindowRegion( Region& rRegion )
{
    rRegion.Intersect( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                  Size( mnOutWidth, mnOutHeight ) ) );
    if ( mpWindowImpl->mbWinRegion )
        rRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
}

// -----------------------------------------------------------------------

void Window::ImplExcludeWindowRegion( Region& rRegion )
{
    if ( mpWindowImpl->mbWinRegion )
    {
        Point aPoint( mnOutOffX, mnOutOffY );
        Region aRegion( Rectangle( aPoint,
                                   Size( mnOutWidth, mnOutHeight ) ) );
        aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
        rRegion.Exclude( aRegion );
    }
    else
    {
        Point aPoint( mnOutOffX, mnOutOffY );
        rRegion.Exclude( Rectangle( aPoint,
                                    Size( mnOutWidth, mnOutHeight ) ) );
    }
}

// -----------------------------------------------------------------------

void Window::ImplExcludeOverlapWindows( Region& rRegion )
{
    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
        {
            pWindow->ImplExcludeWindowRegion( rRegion );
            pWindow->ImplExcludeOverlapWindows( rRegion );
        }

        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplExcludeOverlapWindows2( Region& rRegion )
{
    if ( mpWindowImpl->mbReallyVisible )
        ImplExcludeWindowRegion( rRegion );

    ImplExcludeOverlapWindows( rRegion );
}

// -----------------------------------------------------------------------

void Window::ImplClipBoundaries( Region& rRegion, BOOL bThis, BOOL bOverlaps )
{
    if ( bThis )
        ImplIntersectWindowClipRegion( rRegion );
    else if ( ImplIsOverlapWindow() )
    {
        // Evt. noch am Frame clippen
        if ( !mpWindowImpl->mbFrame )
            rRegion.Intersect( Rectangle( Point( 0, 0 ), Size( mpWindowImpl->mpFrameWindow->mnOutWidth, mpWindowImpl->mpFrameWindow->mnOutHeight ) ) );

        if ( bOverlaps && !rRegion.IsEmpty() )
        {
            // Clip Overlap Siblings
            Window* pStartOverlapWindow = this;
            while ( !pStartOverlapWindow->mpWindowImpl->mbFrame )
            {
                Window* pOverlapWindow = pStartOverlapWindow->mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
                while ( pOverlapWindow && (pOverlapWindow != pStartOverlapWindow) )
                {
                    pOverlapWindow->ImplExcludeOverlapWindows2( rRegion );
                    pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
                }
                pStartOverlapWindow = pStartOverlapWindow->mpWindowImpl->mpOverlapWindow;
            }

            // Clip Child Overlap Windows
            ImplExcludeOverlapWindows( rRegion );
        }
    }
    else
        ImplGetParent()->ImplIntersectWindowClipRegion( rRegion );
}

// -----------------------------------------------------------------------

BOOL Window::ImplClipChilds( Region& rRegion )
{
    BOOL    bOtherClip = FALSE;
    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
        {
            // ParentClipMode-Flags auswerten
            USHORT nClipMode = pWindow->GetParentClipMode();
            if ( !(nClipMode & PARENTCLIPMODE_NOCLIP) &&
                 ((nClipMode & PARENTCLIPMODE_CLIP) || (GetStyle() & WB_CLIPCHILDREN)) )
                pWindow->ImplExcludeWindowRegion( rRegion );
            else
                bOtherClip = TRUE;
        }

        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    return bOtherClip;
}

// -----------------------------------------------------------------------

void Window::ImplClipAllChilds( Region& rRegion )
{
    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
            pWindow->ImplExcludeWindowRegion( rRegion );
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplClipSiblings( Region& rRegion )
{
    Window* pWindow = ImplGetParent()->mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow == this )
            break;

        if ( pWindow->mpWindowImpl->mbReallyVisible )
            pWindow->ImplExcludeWindowRegion( rRegion );

        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInitWinClipRegion()
{
    // Build Window Region
    mpWindowImpl->maWinClipRegion = Rectangle( Point( mnOutOffX, mnOutOffY ),
                                 Size( mnOutWidth, mnOutHeight ) );
    if ( mpWindowImpl->mbWinRegion )
        mpWindowImpl->maWinClipRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );

    // ClipSiblings
    if ( mpWindowImpl->mbClipSiblings && !ImplIsOverlapWindow() )
        ImplClipSiblings( mpWindowImpl->maWinClipRegion );

    // Clip Parent Boundaries
    ImplClipBoundaries( mpWindowImpl->maWinClipRegion, FALSE, TRUE );

    // Clip Children
    if ( (GetStyle() & WB_CLIPCHILDREN) || mpWindowImpl->mbClipChildren )
        mpWindowImpl->mbInitChildRegion = TRUE;

    mpWindowImpl->mbInitWinClipRegion = FALSE;
}

// -----------------------------------------------------------------------

void Window::ImplInitWinChildClipRegion()
{
    if ( !mpWindowImpl->mpFirstChild )
    {
        if ( mpWindowImpl->mpChildClipRegion )
        {
            delete mpWindowImpl->mpChildClipRegion;
            mpWindowImpl->mpChildClipRegion = NULL;
        }
    }
    else
    {
        if ( !mpWindowImpl->mpChildClipRegion )
            mpWindowImpl->mpChildClipRegion = new Region( mpWindowImpl->maWinClipRegion );
        else
            *mpWindowImpl->mpChildClipRegion = mpWindowImpl->maWinClipRegion;

        ImplClipChilds( *mpWindowImpl->mpChildClipRegion );
    }

    mpWindowImpl->mbInitChildRegion = FALSE;
}

// -----------------------------------------------------------------------

Region* Window::ImplGetWinChildClipRegion()
{
    if ( mpWindowImpl->mbInitWinClipRegion )
        ImplInitWinClipRegion();
    if ( mpWindowImpl->mbInitChildRegion )
        ImplInitWinChildClipRegion();
    if ( mpWindowImpl->mpChildClipRegion )
        return mpWindowImpl->mpChildClipRegion;
    else
        return &mpWindowImpl->maWinClipRegion;
}

// -----------------------------------------------------------------------

void Window::ImplIntersectAndUnionOverlapWindows( const Region& rInterRegion, Region& rRegion )
{
    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
        {
            Region aTempRegion( rInterRegion );
            pWindow->ImplIntersectWindowRegion( aTempRegion );
            rRegion.Union( aTempRegion );
            pWindow->ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
        }

        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplIntersectAndUnionOverlapWindows2( const Region& rInterRegion, Region& rRegion )
{
    if ( mpWindowImpl->mbReallyVisible )
    {
        Region aTempRegion( rInterRegion );
        ImplIntersectWindowRegion( aTempRegion );
        rRegion.Union( aTempRegion );
    }

    ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
}

// -----------------------------------------------------------------------

void Window::ImplCalcOverlapRegionOverlaps( const Region& rInterRegion, Region& rRegion )
{
    // Clip Overlap Siblings
    Window* pStartOverlapWindow;
    if ( !ImplIsOverlapWindow() )
        pStartOverlapWindow = mpWindowImpl->mpOverlapWindow;
    else
        pStartOverlapWindow = this;
    while ( !pStartOverlapWindow->mpWindowImpl->mbFrame )
    {
        Window* pOverlapWindow = pStartOverlapWindow->mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
        while ( pOverlapWindow && (pOverlapWindow != pStartOverlapWindow) )
        {
            pOverlapWindow->ImplIntersectAndUnionOverlapWindows2( rInterRegion, rRegion );
            pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
        }
        pStartOverlapWindow = pStartOverlapWindow->mpWindowImpl->mpOverlapWindow;
    }

    // Clip Child Overlap Windows
    if ( !ImplIsOverlapWindow() )
        mpWindowImpl->mpOverlapWindow->ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
    else
        ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
}

// -----------------------------------------------------------------------

void Window::ImplCalcOverlapRegion( const Rectangle& rSourceRect, Region& rRegion,
                                    BOOL bChilds, BOOL bParent, BOOL bSiblings )
{
    Region  aRegion( rSourceRect );
    if ( mpWindowImpl->mbWinRegion )
        rRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
    Region  aTempRegion;
    Window* pWindow;

    ImplCalcOverlapRegionOverlaps( aRegion, rRegion );

    // Parent-Boundaries
    if ( bParent )
    {
        pWindow = this;
        if ( !ImplIsOverlapWindow() )
        {
            pWindow = ImplGetParent();
            do
            {
                aTempRegion = aRegion;
                pWindow->ImplExcludeWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
                if ( pWindow->ImplIsOverlapWindow() )
                    break;
                pWindow = pWindow->ImplGetParent();
            }
            while ( pWindow );
        }
        if ( !pWindow->mpWindowImpl->mbFrame )
        {
            aTempRegion = aRegion;
            aTempRegion.Exclude( Rectangle( Point( 0, 0 ), Size( mpWindowImpl->mpFrameWindow->mnOutWidth, mpWindowImpl->mpFrameWindow->mnOutHeight ) ) );
            rRegion.Union( aTempRegion );
        }
    }

    // Siblings
    if ( bSiblings && !ImplIsOverlapWindow() )
    {
        pWindow = mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild;
        do
        {
            if ( pWindow->mpWindowImpl->mbReallyVisible && (pWindow != this) )
            {
                aTempRegion = aRegion;
                pWindow->ImplIntersectWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
            }
            pWindow = pWindow->mpWindowImpl->mpNext;
        }
        while ( pWindow );
    }

    // Childs
    if ( bChilds )
    {
        pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            if ( pWindow->mpWindowImpl->mbReallyVisible )
            {
                aTempRegion = aRegion;
                pWindow->ImplIntersectWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
            }
            pWindow = pWindow->mpWindowImpl->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallPaint( const Region* pRegion, USHORT nPaintFlags )
{
    // call PrePaint. PrePaint may add to the invalidate region as well as
    // other parameters used below.
    PrePaint();

    mpWindowImpl->mbPaintFrame = FALSE;

    if ( nPaintFlags & IMPL_PAINT_PAINTALLCHILDS )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINT | IMPL_PAINT_PAINTALLCHILDS | (nPaintFlags & IMPL_PAINT_PAINTALL);
    if ( nPaintFlags & IMPL_PAINT_PAINTCHILDS )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTCHILDS;
    if ( nPaintFlags & IMPL_PAINT_ERASE )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_ERASE;
    if ( nPaintFlags & IMPL_PAINT_CHECKRTL )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_CHECKRTL;
    if ( !mpWindowImpl->mpFirstChild )
        mpWindowImpl->mnPaintFlags &= ~IMPL_PAINT_PAINTALLCHILDS;

    if ( mpWindowImpl->mbPaintDisabled )
    {
        if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
            Invalidate( INVALIDATE_NOCHILDREN | INVALIDATE_NOERASE | INVALIDATE_NOTRANSPARENT | INVALIDATE_NOCLIPCHILDREN );
        else if ( pRegion )
            Invalidate( *pRegion, INVALIDATE_NOCHILDREN | INVALIDATE_NOERASE | INVALIDATE_NOTRANSPARENT | INVALIDATE_NOCLIPCHILDREN );
        return;
    }

    nPaintFlags = mpWindowImpl->mnPaintFlags & ~(IMPL_PAINT_PAINT);

    Region* pChildRegion = NULL;
    Rectangle aSelectionRect;
    if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINT )
    {
        Region* pWinChildClipRegion = ImplGetWinChildClipRegion();
        if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
            mpWindowImpl->maInvalidateRegion = *pWinChildClipRegion;
        else
        {
            if ( pRegion )
                mpWindowImpl->maInvalidateRegion.Union( *pRegion );

            if( mpWindowImpl->mpWinData && mpWindowImpl->mbTrackVisible )
                /* #98602# need to repaint all children within the
               * tracking rectangle, so the following invert
               * operation takes places without traces of the previous
               * one.
               */
                mpWindowImpl->maInvalidateRegion.Union( *mpWindowImpl->mpWinData->mpTrackRect );

            if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDS )
                pChildRegion = new Region( mpWindowImpl->maInvalidateRegion );
            mpWindowImpl->maInvalidateRegion.Intersect( *pWinChildClipRegion );
        }
        mpWindowImpl->mnPaintFlags = 0;
        if ( !mpWindowImpl->maInvalidateRegion.IsEmpty() )
        {
            if ( mpWindowImpl->mpCursor )
                mpWindowImpl->mpCursor->ImplHide();

            mbInitClipRegion = TRUE;
            mpWindowImpl->mbInPaint = TRUE;

            // Paint-Region zuruecksetzen
            Region      aPaintRegion( mpWindowImpl->maInvalidateRegion );
            Rectangle   aPaintRect = aPaintRegion.GetBoundRect();

            // - RTL - re-mirror paint rect and region at this window
            if( ImplIsAntiparallel() )
            {
                ImplReMirror( aPaintRect );
                ImplReMirror( aPaintRegion );
            }
            aPaintRect = ImplDevicePixelToLogic( aPaintRect);
            mpWindowImpl->mpPaintRegion = &aPaintRegion;
            mpWindowImpl->maInvalidateRegion.SetEmpty();

            if ( (nPaintFlags & IMPL_PAINT_ERASE) && IsBackground() )
            {
                if ( IsClipRegion() )
                {
                    Region aOldRegion = GetClipRegion();
                    SetClipRegion();
                    Erase();
                    SetClipRegion( aOldRegion );
                }
                else
                    Erase();
            }

            // #98943# trigger drawing of toolbox selection after all childern are painted
            if( mpWindowImpl->mbDrawSelectionBackground )
                aSelectionRect = aPaintRect;

            Paint( aPaintRect );

            if ( mpWindowImpl->mpWinData )
            {
                if ( mpWindowImpl->mbFocusVisible )
                    ImplInvertFocus( *(mpWindowImpl->mpWinData->mpFocusRect) );
            }
            mpWindowImpl->mbInPaint = FALSE;
            mbInitClipRegion = TRUE;
            mpWindowImpl->mpPaintRegion = NULL;
            if ( mpWindowImpl->mpCursor )
                mpWindowImpl->mpCursor->ImplShow( FALSE );
        }
    }
    else
        mpWindowImpl->mnPaintFlags = 0;

    if ( nPaintFlags & (IMPL_PAINT_PAINTALLCHILDS | IMPL_PAINT_PAINTCHILDS) )
    {
        // die Childfenster ausgeben
        Window* pTempWindow = mpWindowImpl->mpFirstChild;
        while ( pTempWindow )
        {
            if ( pTempWindow->mpWindowImpl->mbVisible )
                pTempWindow->ImplCallPaint( pChildRegion, nPaintFlags );
            pTempWindow = pTempWindow->mpWindowImpl->mpNext;
        }
    }

    if ( mpWindowImpl->mpWinData && mpWindowImpl->mbTrackVisible && (mpWindowImpl->mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
        /* #98602# need to invert the tracking rect AFTER
        * the children have painted
        */
        InvertTracking( *(mpWindowImpl->mpWinData->mpTrackRect), mpWindowImpl->mpWinData->mnTrackFlags );

    // #98943# draw toolbox selection
    if( !aSelectionRect.IsEmpty() )
        DrawSelectionBackground( aSelectionRect, 3, FALSE, TRUE, FALSE );

    if ( pChildRegion )
        delete pChildRegion;
}

// -----------------------------------------------------------------------

void Window::ImplCallOverlapPaint()
{
    // Zuerst geben wir die ueberlappenden Fenster aus
    Window* pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->mpWindowImpl->mbReallyVisible )
            pTempWindow->ImplCallOverlapPaint();
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }

    // und dann erst uns selber
    if ( mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDS) )
    {
        // - RTL - notify ImplCallPaint to check for re-mirroring (CHECKRTL)
        //         because we were called from the Sal layer
        ImplCallPaint( NULL, mpWindowImpl->mnPaintFlags /*| IMPL_PAINT_CHECKRTL */);
    }
}

// -----------------------------------------------------------------------

void Window::ImplPostPaint()
{
    if ( !mpWindowImpl->mpFrameData->maPaintTimer.IsActive() )
        mpWindowImpl->mpFrameData->maPaintTimer.Start();
}

// -----------------------------------------------------------------------

IMPL_LINK( Window, ImplHandlePaintHdl, void*, EMPTYARG )
{
    // save paint events until resizing is done
    if( mpWindowImpl->mbFrame && mpWindowImpl->mpFrameData->maResizeTimer.IsActive() )
        mpWindowImpl->mpFrameData->maPaintTimer.Start();
    else if ( mpWindowImpl->mbReallyVisible )
        ImplCallOverlapPaint();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( Window, ImplHandleResizeTimerHdl, void*, EMPTYARG )
{
    if( mpWindowImpl->mbReallyVisible )
    {
        ImplCallResize();
        if( mpWindowImpl->mpFrameData->maPaintTimer.IsActive() )
        {
            mpWindowImpl->mpFrameData->maPaintTimer.Stop();
            mpWindowImpl->mpFrameData->maPaintTimer.GetTimeoutHdl().Call( NULL );
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

void Window::ImplInvalidateFrameRegion( const Region* pRegion, USHORT nFlags )
{
    // PAINTCHILDS bei allen Parent-Fenster bis zum ersten OverlapWindow
    // setzen
    if ( !ImplIsOverlapWindow() )
    {
        Window* pTempWindow = this;
        USHORT nTranspPaint = IsPaintTransparent() ? IMPL_PAINT_PAINT : 0;
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTCHILDS )
                break;
            pTempWindow->mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTCHILDS | nTranspPaint;
            if( ! pTempWindow->IsPaintTransparent() )
                nTranspPaint = 0;
        }
        while ( !pTempWindow->ImplIsOverlapWindow() );
    }

    // Paint-Flags setzen
    mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINT;
    if ( nFlags & INVALIDATE_CHILDREN )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTALLCHILDS;
    if ( !(nFlags & INVALIDATE_NOERASE) )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_ERASE;
    if ( !pRegion )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTALL;

    // Wenn nicht alles neu ausgegeben werden muss, dann die Region
    // dazupacken
    if ( !(mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL) )
        mpWindowImpl->maInvalidateRegion.Union( *pRegion );

    // Handle transparent windows correctly: invalidate must be done on the first opaque parent
    if( ((IsPaintTransparent() && !(nFlags & INVALIDATE_NOTRANSPARENT)) || (nFlags & INVALIDATE_TRANSPARENT) )
            && ImplGetParent() )
    {
        Window *pParent = ImplGetParent();
        while( pParent && pParent->IsPaintTransparent() )
            pParent = pParent->ImplGetParent();
        if( pParent )
        {
            Region *pChildRegion;
            if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
                // invalidate the whole child window region in the parent
                pChildRegion = ImplGetWinChildClipRegion();
            else
                // invalidate the same region in the parent that has to be repainted in the child
                pChildRegion = &mpWindowImpl->maInvalidateRegion;

            nFlags |= INVALIDATE_CHILDREN;  // paint should also be done on all children
            nFlags &= ~INVALIDATE_NOERASE;  // parent should paint and erase to create proper background
            pParent->ImplInvalidateFrameRegion( pChildRegion, nFlags );
        }
    }
    ImplPostPaint();
}

// -----------------------------------------------------------------------

void Window::ImplInvalidateOverlapFrameRegion( const Region& rRegion )
{
    Region aRegion = rRegion;

    ImplClipBoundaries( aRegion, TRUE, TRUE );
    if ( !aRegion.IsEmpty() )
        ImplInvalidateFrameRegion( &aRegion, INVALIDATE_CHILDREN );

    // Dann invalidieren wir die ueberlappenden Fenster
    Window* pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->IsVisible() )
            pTempWindow->ImplInvalidateOverlapFrameRegion( rRegion );

        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInvalidateParentFrameRegion( Region& rRegion )
{
    if ( mpWindowImpl->mbOverlapWin )
        mpWindowImpl->mpFrameWindow->ImplInvalidateOverlapFrameRegion( rRegion );
    else
    {
        if( ImplGetParent() )
            ImplGetParent()->ImplInvalidateFrameRegion( &rRegion, INVALIDATE_CHILDREN );
    }
}

// -----------------------------------------------------------------------

void Window::ImplInvalidate( const Region* pRegion, USHORT nFlags )
{

    // Hintergrund-Sicherung zuruecksetzen
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    // Feststellen, was neu ausgegeben werden muss
    BOOL bInvalidateAll = !pRegion;

    // Transparent-Invalidate beruecksichtigen
    Window* pOpaqueWindow = this;
    if ( (mpWindowImpl->mbPaintTransparent && !(nFlags & INVALIDATE_NOTRANSPARENT)) || (nFlags & INVALIDATE_TRANSPARENT) )
    {
        Window* pTempWindow = pOpaqueWindow->ImplGetParent();
        while ( pTempWindow )
        {
            if ( !pTempWindow->IsPaintTransparent() )
            {
                pOpaqueWindow = pTempWindow;
                nFlags |= INVALIDATE_CHILDREN;
                bInvalidateAll = FALSE;
                break;
            }

            if ( pTempWindow->ImplIsOverlapWindow() )
                break;

            pTempWindow = pTempWindow->ImplGetParent();
        }
    }

    // Region zusammenbauen
    USHORT nOrgFlags = nFlags;
    if ( !(nFlags & (INVALIDATE_CHILDREN | INVALIDATE_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= INVALIDATE_NOCHILDREN;
        else
            nFlags |= INVALIDATE_CHILDREN;
    }
    if ( (nFlags & INVALIDATE_NOCHILDREN) && mpWindowImpl->mpFirstChild )
        bInvalidateAll = FALSE;
    if ( bInvalidateAll )
        ImplInvalidateFrameRegion( NULL, nFlags );
    else
    {
        Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aRegion( aRect );
        if ( pRegion )
        {
            // --- RTL --- remirror region before intersecting it
            if ( ImplIsAntiparallel() )
            {
                Region aRgn( *pRegion );
                ImplReMirror( aRgn );
                aRegion.Intersect( aRgn );
            }
            else
                aRegion.Intersect( *pRegion );
        }
        ImplClipBoundaries( aRegion, TRUE, TRUE );
        if ( nFlags & INVALIDATE_NOCHILDREN )
        {
            nFlags &= ~INVALIDATE_CHILDREN;
            if ( !(nFlags & INVALIDATE_NOCLIPCHILDREN) )
            {
                if ( nOrgFlags & INVALIDATE_NOCHILDREN )
                    ImplClipAllChilds( aRegion );
                else
                {
                    if ( ImplClipChilds( aRegion ) )
                        nFlags |= INVALIDATE_CHILDREN;
                }
            }
        }
        if ( !aRegion.IsEmpty() )
            ImplInvalidateFrameRegion( &aRegion, nFlags );  // transparency is handled here, pOpaqueWindow not required
    }

    if ( nFlags & INVALIDATE_UPDATE )
        pOpaqueWindow->Update();        // start painting at the opaque parent
}

// -----------------------------------------------------------------------

void Window::ImplMoveInvalidateRegion( const Rectangle& rRect,
                                       long nHorzScroll, long nVertScroll,
                                       BOOL bChilds )
{
    if ( (mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTALL)) == IMPL_PAINT_PAINT )
    {
        Region aTempRegion = mpWindowImpl->maInvalidateRegion;
        aTempRegion.Intersect( rRect );
        aTempRegion.Move( nHorzScroll, nVertScroll );
        mpWindowImpl->maInvalidateRegion.Union( aTempRegion );
    }

    if ( bChilds && (mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTCHILDS) )
    {
        Window* pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            pWindow->ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, TRUE );
            pWindow = pWindow->mpWindowImpl->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplMoveAllInvalidateRegions( const Rectangle& rRect,
                                           long nHorzScroll, long nVertScroll,
                                           BOOL bChilds )
{
    // Paint-Region auch verschieben, wenn noch Paints anstehen
    ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, bChilds );
    // Paint-Region muss bei uns verschoben gesetzt werden, die durch
    // die Parents gezeichnet werden
    if ( !ImplIsOverlapWindow() )
    {
        Region  aPaintAllRegion;
        Window* pPaintAllWindow = this;
        do
        {
            pPaintAllWindow = pPaintAllWindow->ImplGetParent();
            if ( pPaintAllWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDS )
            {
                if ( pPaintAllWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
                {
                    aPaintAllRegion.SetEmpty();
                    break;
                }
                else
                    aPaintAllRegion.Union( pPaintAllWindow->mpWindowImpl->maInvalidateRegion );
            }
        }
        while ( !pPaintAllWindow->ImplIsOverlapWindow() );
        if ( !aPaintAllRegion.IsEmpty() )
        {
            aPaintAllRegion.Move( nHorzScroll, nVertScroll );
            USHORT nPaintFlags = 0;
            if ( bChilds )
                mpWindowImpl->mnPaintFlags |= INVALIDATE_CHILDREN;
            ImplInvalidateFrameRegion( &aPaintAllRegion, nPaintFlags );
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplValidateFrameRegion( const Region* pRegion, USHORT nFlags )
{
    if ( !pRegion )
        mpWindowImpl->maInvalidateRegion.SetEmpty();
    else
    {
        // Wenn alle Childfenster neu ausgegeben werden muessen,
        // dann invalidieren wir diese vorher
        if ( (mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDS) && mpWindowImpl->mpFirstChild )
        {
            Region aChildRegion = mpWindowImpl->maInvalidateRegion;
            if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
            {
                Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                aChildRegion = aRect;
            }
            Window* pChild = mpWindowImpl->mpFirstChild;
            while ( pChild )
            {
                pChild->Invalidate( aChildRegion, INVALIDATE_CHILDREN | INVALIDATE_NOTRANSPARENT );
                pChild = pChild->mpWindowImpl->mpNext;
            }
        }
        if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
        {
            Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
            mpWindowImpl->maInvalidateRegion = aRect;
        }
        mpWindowImpl->maInvalidateRegion.Exclude( *pRegion );
    }
    mpWindowImpl->mnPaintFlags &= ~IMPL_PAINT_PAINTALL;

    if ( nFlags & VALIDATE_CHILDREN )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->ImplValidateFrameRegion( pRegion, nFlags );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplValidate( const Region* pRegion, USHORT nFlags )
{
    // Region zusammenbauen
    BOOL    bValidateAll = !pRegion;
    USHORT  nOrgFlags = nFlags;
    if ( !(nFlags & (VALIDATE_CHILDREN | VALIDATE_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= VALIDATE_NOCHILDREN;
        else
            nFlags |= VALIDATE_CHILDREN;
    }
    if ( (nFlags & VALIDATE_NOCHILDREN) && mpWindowImpl->mpFirstChild )
        bValidateAll = FALSE;
    if ( bValidateAll )
        ImplValidateFrameRegion( NULL, nFlags );
    else
    {
        Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aRegion( aRect );
        if ( pRegion )
            aRegion.Intersect( *pRegion );
        ImplClipBoundaries( aRegion, TRUE, TRUE );
        if ( nFlags & VALIDATE_NOCHILDREN )
        {
            nFlags &= ~VALIDATE_CHILDREN;
            if ( nOrgFlags & VALIDATE_NOCHILDREN )
                ImplClipAllChilds( aRegion );
            else
            {
                if ( ImplClipChilds( aRegion ) )
                    nFlags |= VALIDATE_CHILDREN;
            }
        }
        if ( !aRegion.IsEmpty() )
            ImplValidateFrameRegion( &aRegion, nFlags );
    }
}

// -----------------------------------------------------------------------

void Window::ImplScroll( const Rectangle& rRect,
                         long nHorzScroll, long nVertScroll, USHORT nFlags )
{
    if ( !IsDeviceOutputNecessary() )
        return;

    nHorzScroll = ImplLogicWidthToDevicePixel( nHorzScroll );
    nVertScroll = ImplLogicHeightToDevicePixel( nVertScroll );

    if ( !nHorzScroll && !nVertScroll )
        return;

    // Hintergrund-Sicherung zuruecksetzen
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    if ( mpWindowImpl->mpCursor )
        mpWindowImpl->mpCursor->ImplHide();

    USHORT nOrgFlags = nFlags;
    if ( !(nFlags & (SCROLL_CHILDREN | SCROLL_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= SCROLL_NOCHILDREN;
        else
            nFlags |= SCROLL_CHILDREN;
    }

    Region  aInvalidateRegion;
    BOOL    bScrollChilds = (nFlags & SCROLL_CHILDREN) != 0;
    BOOL    bErase = (nFlags & SCROLL_NOERASE) == 0;

    if ( !mpWindowImpl->mpFirstChild )
        bScrollChilds = FALSE;

    // --- RTL --- check if this window requires special action
    BOOL bReMirror = ( ImplIsAntiparallel() );

    Rectangle aRectMirror( rRect );
    if( bReMirror )
    {
        // --- RTL --- make sure the invalidate region of this window is
        // computed in the same coordinate space as the one from the overlap windows
        ImplReMirror( aRectMirror );
    }

    // Paint-Bereiche anpassen
    ImplMoveAllInvalidateRegions( aRectMirror, nHorzScroll, nVertScroll, bScrollChilds );

    if ( !(nFlags & SCROLL_NOINVALIDATE) )
    {
        ImplCalcOverlapRegion( aRectMirror, aInvalidateRegion, !bScrollChilds, TRUE, FALSE );

        // --- RTL ---
        // if the scrolling on the device is performed in the opposite direction
        // then move the overlaps in that direction to compute the invalidate region
        // on the correct side, i.e., revert nHorzScroll

        if ( !aInvalidateRegion.IsEmpty() )
        {
            aInvalidateRegion.Move( bReMirror ? -nHorzScroll : nHorzScroll, nVertScroll );
            bErase = TRUE;
        }
        if ( !(nFlags & SCROLL_NOWINDOWINVALIDATE) )
        {
            Rectangle aDestRect( aRectMirror );
            aDestRect.Move( bReMirror ? -nHorzScroll : nHorzScroll, nVertScroll );
            Region aWinInvalidateRegion( aRectMirror );
            aWinInvalidateRegion.Exclude( aDestRect );

            aInvalidateRegion.Union( aWinInvalidateRegion );
        }
    }

    Point aPoint( mnOutOffX, mnOutOffY );
    Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
    if ( nFlags & SCROLL_CLIP )
        aRegion.Intersect( rRect );
    if ( mpWindowImpl->mbWinRegion )
        aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );

    aRegion.Exclude( aInvalidateRegion );

    ImplClipBoundaries( aRegion, FALSE, TRUE );
    if ( !bScrollChilds )
    {
        if ( nOrgFlags & SCROLL_NOCHILDREN )
            ImplClipAllChilds( aRegion );
        else
            ImplClipChilds( aRegion );
    }
    if ( mbClipRegion && (nFlags & SCROLL_USECLIPREGION) )
        aRegion.Intersect( maRegion );
    if ( !aRegion.IsEmpty() )
    {
        if ( mpWindowImpl->mpWinData )
        {
            if ( mpWindowImpl->mbFocusVisible )
                ImplInvertFocus( *(mpWindowImpl->mpWinData->mpFocusRect) );
            if ( mpWindowImpl->mbTrackVisible && (mpWindowImpl->mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
                InvertTracking( *(mpWindowImpl->mpWinData->mpTrackRect), mpWindowImpl->mpWinData->mnTrackFlags );
        }

        SalGraphics* pGraphics = ImplGetFrameGraphics();
        if ( pGraphics )
        {
            if( bReMirror )
            {
                // --- RTL --- frame coordinates require re-mirroring
                ImplReMirror( aRegion );
            }

            ImplSelectClipRegion( aRegion, pGraphics );
            pGraphics->CopyArea( rRect.Left()+nHorzScroll, rRect.Top()+nVertScroll,
                                 rRect.Left(), rRect.Top(),
                                 rRect.GetWidth(), rRect.GetHeight(),
                                 SAL_COPYAREA_WINDOWINVALIDATE, this );
        }

        if ( mpWindowImpl->mpWinData )
        {
            if ( mpWindowImpl->mbFocusVisible )
                ImplInvertFocus( *(mpWindowImpl->mpWinData->mpFocusRect) );
            if ( mpWindowImpl->mbTrackVisible && (mpWindowImpl->mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
                InvertTracking( *(mpWindowImpl->mpWinData->mpTrackRect), mpWindowImpl->mpWinData->mnTrackFlags );
        }
    }

    if ( !aInvalidateRegion.IsEmpty() )
    {
        // --- RTL --- the invalidate region for this windows is already computed in frame coordinates
        // so it has to be re-mirrored before calling the Paint-handler
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_CHECKRTL;

        USHORT nPaintFlags = INVALIDATE_CHILDREN;
        if ( !bErase )
            nPaintFlags |= INVALIDATE_NOERASE;
        if ( !bScrollChilds )
        {
            if ( nOrgFlags & SCROLL_NOCHILDREN )
                ImplClipAllChilds( aInvalidateRegion );
            else
                ImplClipChilds( aInvalidateRegion );
        }
        ImplInvalidateFrameRegion( &aInvalidateRegion, nPaintFlags );
    }

    if ( bScrollChilds )
    {
        Window* pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            Point aPos = pWindow->GetPosPixel();
            aPos += Point( nHorzScroll, nVertScroll );
            pWindow->SetPosPixel( aPos );

            pWindow = pWindow->mpWindowImpl->mpNext;
        }
    }

    if ( nFlags & SCROLL_UPDATE )
        Update();

    if ( mpWindowImpl->mpCursor )
        mpWindowImpl->mpCursor->ImplShow( FALSE );
}

// -----------------------------------------------------------------------

void Window::ImplUpdateAll( BOOL bOverlapWindows )
{
    if ( !mpWindowImpl->mbReallyVisible )
        return;

    BOOL bFlush = FALSE;
    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) )
            bFlush = TRUE;
    }

    // Ein Update wirkt immer auf das OverlapWindow, damit bei spaeteren
    // Paints nicht zuviel gemalt wird, wenn dort ALLCHILDREN usw. gesetzt
    // ist
    Window* pWindow = ImplGetFirstOverlapWindow();
    if ( bOverlapWindows )
        pWindow->ImplCallOverlapPaint();
    else
    {
        if ( pWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDS) )
            pWindow->ImplCallPaint( NULL, pWindow->mpWindowImpl->mnPaintFlags );
    }

    if ( bFlush )
        Flush();
}

// -----------------------------------------------------------------------

void Window::ImplUpdateWindowPtr( Window* pWindow )
{
    if ( mpWindowImpl->mpFrameWindow != pWindow->mpWindowImpl->mpFrameWindow )
    {
        // Graphic freigeben
        ImplReleaseGraphics();
    }

    mpWindowImpl->mpFrameData     = pWindow->mpWindowImpl->mpFrameData;
    mpWindowImpl->mpFrame         = pWindow->mpWindowImpl->mpFrame;
    mpWindowImpl->mpFrameWindow   = pWindow->mpWindowImpl->mpFrameWindow;
    if ( pWindow->ImplIsOverlapWindow() )
        mpWindowImpl->mpOverlapWindow = pWindow;
    else
        mpWindowImpl->mpOverlapWindow = pWindow->mpWindowImpl->mpOverlapWindow;

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateWindowPtr( pWindow );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateWindowPtr()
{
    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateWindowPtr( this );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateOverlapWindowPtr( BOOL bNewFrame )
{
    BOOL bVisible = IsVisible();
    Show( false );
    ImplRemoveWindow( bNewFrame );
    Window* pRealParent = mpWindowImpl->mpRealParent;
    ImplInsertWindow( ImplGetParent() );
    mpWindowImpl->mpRealParent = pRealParent;
    ImplUpdateWindowPtr();
    if ( ImplUpdatePos() )
        ImplUpdateSysObjPos();

    if ( bNewFrame )
    {
        Window* pOverlapWindow = mpWindowImpl->mpFirstOverlap;
        while ( pOverlapWindow )
        {
            Window* pNextOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
            pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
            pOverlapWindow = pNextOverlapWindow;
        }
    }

    if ( bVisible )
        Show( true );
}

// -----------------------------------------------------------------------

BOOL Window::ImplUpdatePos()
{
    BOOL bSysChild = FALSE;

    if ( ImplIsOverlapWindow() )
    {
        mnOutOffX  = mpWindowImpl->mnX;
        mnOutOffY  = mpWindowImpl->mnY;
    }
    else
    {
        Window* pParent = ImplGetParent();

        mnOutOffX  = mpWindowImpl->mnX + pParent->mnOutOffX;
        mnOutOffY  = mpWindowImpl->mnY + pParent->mnOutOffY;
    }

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        if ( pChild->ImplUpdatePos() )
            bSysChild = TRUE;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    if ( mpWindowImpl->mpSysObj )
        bSysChild = TRUE;

    return bSysChild;
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjPos()
{
    if ( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->SetPosSize( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight );

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateSysObjPos();
        pChild = pChild->mpWindowImpl->mpNext;
    }
}
// -----------------------------------------------------------------------

void Window::ImplPosSizeWindow( long nX, long nY,
                                long nWidth, long nHeight, USHORT nFlags )
{
    BOOL    bNewPos         = FALSE;
    BOOL    bNewSize        = FALSE;
    BOOL    bCopyBits       = FALSE;
    long    nOldOutOffX     = mnOutOffX;
    long    nOldOutOffY     = mnOutOffY;
    long    nOldOutWidth    = mnOutWidth;
    long    nOldOutHeight   = mnOutHeight;
    Region* pOverlapRegion  = NULL;
    Region* pOldRegion      = NULL;

    if ( IsReallyVisible() )
    {
        if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
            ImplInvalidateAllOverlapBackgrounds();

        Rectangle aOldWinRect( Point( nOldOutOffX, nOldOutOffY ),
                               Size( nOldOutWidth, nOldOutHeight ) );
        pOldRegion = new Region( aOldWinRect );
        if ( mpWindowImpl->mbWinRegion )
            pOldRegion->Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );

        if ( mnOutWidth && mnOutHeight && !mpWindowImpl->mbPaintTransparent &&
             !mpWindowImpl->mbInitWinClipRegion && !mpWindowImpl->maWinClipRegion.IsEmpty() &&
             !HasPaintEvent() )
            bCopyBits = TRUE;
    }

    BOOL bnXRecycled = FALSE; // avoid duplicate mirroring in RTL case
    if ( nFlags & WINDOW_POSSIZE_WIDTH )
    {
        if(!( nFlags & WINDOW_POSSIZE_X ))
        {
            nX = mpWindowImpl->mnX;
            nFlags |= WINDOW_POSSIZE_X;
            bnXRecycled = TRUE; // we're using a mnX which was already mirrored in RTL case
        }

        if ( nWidth < 0 )
            nWidth = 0;
        if ( nWidth != mnOutWidth )
        {
            mnOutWidth = nWidth;
            bNewSize = TRUE;
            bCopyBits = FALSE;
        }
    }
    if ( nFlags & WINDOW_POSSIZE_HEIGHT )
    {
        if ( nHeight < 0 )
            nHeight = 0;
        if ( nHeight != mnOutHeight )
        {
            mnOutHeight = nHeight;
            bNewSize = TRUE;
            bCopyBits = FALSE;
        }
    }

    if ( nFlags & WINDOW_POSSIZE_X )
    {
        long nOrgX = nX;
        // --- RTL ---  (compare the screen coordinates)
        Point aPtDev( Point( nX+mnOutOffX, 0 ) );
        if( ImplHasMirroredGraphics() )
        {
            mpGraphics->mirror( aPtDev.X(), this );

            // #106948# always mirror our pos if our parent is not mirroring, even
            // if we are also not mirroring
            // --- RTL --- check if parent is in different coordinates
            if( !bnXRecycled && mpWindowImpl->mpParent && !mpWindowImpl->mpParent->mpWindowImpl->mbFrame && mpWindowImpl->mpParent->ImplIsAntiparallel() )
            {
                // --- RTL --- (re-mirror at parent window)
                nX = mpWindowImpl->mpParent->mnOutWidth - mnOutWidth - nX;
            }
            /* #i99166# An LTR window in RTL UI that gets sized only would be
               expected to not moved its upper left point
            */
            if( bnXRecycled )
            {
                if( ImplIsAntiparallel() )
                {
                    aPtDev.X() = mpWindowImpl->mnAbsScreenX;
                    nOrgX = mpWindowImpl->maPos.X();
                }
            }
        }
        else if( !bnXRecycled && mpWindowImpl->mpParent && !mpWindowImpl->mpParent->mpWindowImpl->mbFrame && mpWindowImpl->mpParent->ImplIsAntiparallel() )
        {
            // mirrored window in LTR UI
            {
                // --- RTL --- (re-mirror at parent window)
                nX = mpWindowImpl->mpParent->mnOutWidth - mnOutWidth - nX;
            }
        }

        // check maPos as well, as it could have been changed for client windows (ImplCallMove())
        if ( mpWindowImpl->mnAbsScreenX != aPtDev.X() || nX != mpWindowImpl->mnX || nOrgX != mpWindowImpl->maPos.X() )
        {
            if ( bCopyBits && !pOverlapRegion )
            {
                pOverlapRegion = new Region();
                ImplCalcOverlapRegion( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                                  Size( mnOutWidth, mnOutHeight ) ),
                                       *pOverlapRegion, FALSE, TRUE, TRUE );
            }
            mpWindowImpl->mnX = nX;
            mpWindowImpl->maPos.X() = nOrgX;
            mpWindowImpl->mnAbsScreenX = aPtDev.X();    // --- RTL --- (store real screen pos)
            bNewPos = TRUE;
        }
    }
    if ( nFlags & WINDOW_POSSIZE_Y )
    {
        // check maPos as well, as it could have been changed for client windows (ImplCallMove())
        if ( nY != mpWindowImpl->mnY || nY != mpWindowImpl->maPos.Y() )
        {
            if ( bCopyBits && !pOverlapRegion )
            {
                pOverlapRegion = new Region();
                ImplCalcOverlapRegion( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                                  Size( mnOutWidth, mnOutHeight ) ),
                                       *pOverlapRegion, FALSE, TRUE, TRUE );
            }
            mpWindowImpl->mnY = nY;
            mpWindowImpl->maPos.Y() = nY;
            bNewPos = TRUE;
        }
    }

/*    if ( nFlags & (WINDOW_POSSIZE_X|WINDOW_POSSIZE_Y) )
    {
        POINT aPt;
        aPt.x = mpWindowImpl->maPos.X();
        aPt.y = mpWindowImpl->maPos.Y();
        ClientToScreen( mpWindowImpl->mpFrame->maFrameData.mhWnd , &aPt );
        mpWindowImpl->maPos.X() = aPt.x;
        mpWindowImpl->maPos.Y() = aPt.y;
    }
*/
    if ( bNewPos || bNewSize )
    {
        BOOL bUpdateSysObjPos = FALSE;
        if ( bNewPos )
            bUpdateSysObjPos = ImplUpdatePos();

        // the borderwindow always specifies the position for its client window
        if ( mpWindowImpl->mpBorderWindow )
            mpWindowImpl->maPos = mpWindowImpl->mpBorderWindow->mpWindowImpl->maPos;

        if ( mpWindowImpl->mpClientWindow )
        {
            mpWindowImpl->mpClientWindow->ImplPosSizeWindow( mpWindowImpl->mpClientWindow->mpWindowImpl->mnLeftBorder,
                                               mpWindowImpl->mpClientWindow->mpWindowImpl->mnTopBorder,
                                               mnOutWidth-mpWindowImpl->mpClientWindow->mpWindowImpl->mnLeftBorder-mpWindowImpl->mpClientWindow->mpWindowImpl->mnRightBorder,
                                               mnOutHeight-mpWindowImpl->mpClientWindow->mpWindowImpl->mnTopBorder-mpWindowImpl->mpClientWindow->mpWindowImpl->mnBottomBorder,
                                               WINDOW_POSSIZE_X | WINDOW_POSSIZE_Y |
                                               WINDOW_POSSIZE_WIDTH | WINDOW_POSSIZE_HEIGHT );
            // Wenn wir ein ClientWindow haben, dann hat dieses fuer die
            // Applikation auch die Position des FloatingWindows
            mpWindowImpl->mpClientWindow->mpWindowImpl->maPos = mpWindowImpl->maPos;
            if ( bNewPos )
            {
                if ( mpWindowImpl->mpClientWindow->IsVisible() )
                {
                    mpWindowImpl->mpClientWindow->ImplCallMove();
                }
                else
                {
                    mpWindowImpl->mpClientWindow->mpWindowImpl->mbCallMove = TRUE;
                }
            }
        }
//        else
//        {
//            if ( mpWindowImpl->mpBorderWindow )
//                mpWindowImpl->maPos = mpWindowImpl->mpBorderWindow->mpWindowImpl->maPos;
//        }

        // Move()/Resize() werden erst bei Show() gerufen, damit min. eins vor
        // einem Show() kommt
        if ( IsVisible() )
        {
            if ( bNewPos )
            {
                ImplCallMove();
            }
            if ( bNewSize )
            {
                ImplCallResize();
            }
        }
        else
        {
            if ( bNewPos )
                mpWindowImpl->mbCallMove = TRUE;
            if ( bNewSize )
                mpWindowImpl->mbCallResize = TRUE;
        }

        BOOL bUpdateSysObjClip = FALSE;
        if ( IsReallyVisible() )
        {
            if ( bNewPos || bNewSize )
            {
                // Hintergrund-Sicherung zuruecksetzen
                if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mpSaveBackDev )
                    ImplDeleteOverlapBackground();
                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                // Clip-Flag neu setzen
                bUpdateSysObjClip = !ImplSetClipFlag( TRUE );
            }

            // Fensterinhalt invalidieren ?
            if ( bNewPos || (mnOutWidth > nOldOutWidth) || (mnOutHeight > nOldOutHeight) )
            {
                if ( bNewPos )
                {
                    BOOL bInvalidate = FALSE;
                    BOOL bParentPaint = TRUE;
                    if ( !ImplIsOverlapWindow() )
                        bParentPaint = mpWindowImpl->mpParent->IsPaintEnabled();
                    if ( bCopyBits && bParentPaint && !HasPaintEvent() )
                    {
                        Point aPoint( mnOutOffX, mnOutOffY );
                        Region aRegion( Rectangle( aPoint,
                                                   Size( mnOutWidth, mnOutHeight ) ) );
                        if ( mpWindowImpl->mbWinRegion )
                            aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
                        ImplClipBoundaries( aRegion, FALSE, TRUE );
                        if ( !pOverlapRegion->IsEmpty() )
                        {
                            pOverlapRegion->Move( mnOutOffX-nOldOutOffX, mnOutOffY-nOldOutOffY );
                            aRegion.Exclude( *pOverlapRegion );
                        }
                        if ( !aRegion.IsEmpty() )
                        {
                            // Paint-Bereiche anpassen
                            ImplMoveAllInvalidateRegions( Rectangle( Point( nOldOutOffX, nOldOutOffY ),
                                                                     Size( nOldOutWidth, nOldOutHeight ) ),
                                                          mnOutOffX-nOldOutOffX, mnOutOffY-nOldOutOffY,
                                                          TRUE );
                            SalGraphics* pGraphics = ImplGetFrameGraphics();
                            if ( pGraphics )
                            {
                                const bool bSelectClipRegion = ImplSelectClipRegion( aRegion, pGraphics );
                                if ( bSelectClipRegion )
                                {
                                    pGraphics->CopyArea( mnOutOffX, mnOutOffY,
                                                         nOldOutOffX, nOldOutOffY,
                                                         nOldOutWidth, nOldOutHeight,
                                                         SAL_COPYAREA_WINDOWINVALIDATE, this );
                                }
                                else
                                    bInvalidate = TRUE;
                            }
                            else
                                bInvalidate = TRUE;
                            if ( !bInvalidate )
                            {
                                if ( !pOverlapRegion->IsEmpty() )
                                    ImplInvalidateFrameRegion( pOverlapRegion, INVALIDATE_CHILDREN );
                            }
                        }
                        else
                            bInvalidate = TRUE;
                    }
                    else
                        bInvalidate = TRUE;
                    if ( bInvalidate )
                        ImplInvalidateFrameRegion( NULL, INVALIDATE_CHILDREN );
                }
                else
                {
                    Point aPoint( mnOutOffX, mnOutOffY );
                    Region aRegion( Rectangle( aPoint,
                                               Size( mnOutWidth, mnOutHeight ) ) );
                    aRegion.Exclude( *pOldRegion );
                    if ( mpWindowImpl->mbWinRegion )
                        aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
                    ImplClipBoundaries( aRegion, FALSE, TRUE );
                    if ( !aRegion.IsEmpty() )
                        ImplInvalidateFrameRegion( &aRegion, INVALIDATE_CHILDREN );
                }
            }

            // Parent oder Overlaps invalidieren
            if ( bNewPos ||
                 (mnOutWidth < nOldOutWidth) || (mnOutHeight < nOldOutHeight) )
            {
                Region aRegion( *pOldRegion );
                if ( !mpWindowImpl->mbPaintTransparent )
                    ImplExcludeWindowRegion( aRegion );
                ImplClipBoundaries( aRegion, FALSE, TRUE );
                if ( !aRegion.IsEmpty() && !mpWindowImpl->mpBorderWindow )
                    ImplInvalidateParentFrameRegion( aRegion );
            }
        }

        // System-Objekte anpassen
        if ( bUpdateSysObjClip )
            ImplUpdateSysObjClip();
        if ( bUpdateSysObjPos )
            ImplUpdateSysObjPos();
        if ( bNewSize && mpWindowImpl->mpSysObj )
            mpWindowImpl->mpSysObj->SetPosSize( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight );
    }

    if ( pOverlapRegion )
        delete pOverlapRegion;
    if ( pOldRegion )
        delete pOldRegion;
}

// -----------------------------------------------------------------------

void Window::ImplToBottomChild()
{
    if ( !ImplIsOverlapWindow() && !mpWindowImpl->mbReallyVisible && (mpWindowImpl->mpParent->mpWindowImpl->mpLastChild != this) )
    {
        // Fenster an das Ende der Liste setzen
        if ( mpWindowImpl->mpPrev )
            mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
        else
            mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = mpWindowImpl->mpNext;
        mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
        mpWindowImpl->mpPrev = mpWindowImpl->mpParent->mpWindowImpl->mpLastChild;
        mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = this;
        mpWindowImpl->mpPrev->mpWindowImpl->mpNext = this;
        mpWindowImpl->mpNext = NULL;
    }
}

// -----------------------------------------------------------------------

void Window::ImplCalcToTop( ImplCalcToTopData* pPrevData )
{
    DBG_ASSERT( ImplIsOverlapWindow(), "Window::ImplCalcToTop(): Is not a OverlapWindow" );

    if ( !mpWindowImpl->mbFrame )
    {
        if ( IsReallyVisible() )
        {
            // Region berechnen, wo das Fenster mit anderen Fenstern ueberlappt
            Point aPoint( mnOutOffX, mnOutOffY );
            Region  aRegion( Rectangle( aPoint,
                                        Size( mnOutWidth, mnOutHeight ) ) );
            Region  aInvalidateRegion;
            ImplCalcOverlapRegionOverlaps( aRegion, aInvalidateRegion );

            if ( !aInvalidateRegion.IsEmpty() )
            {
                ImplCalcToTopData* pData    = new ImplCalcToTopData;
                pPrevData->mpNext           = pData;
                pData->mpNext               = NULL;
                pData->mpWindow             = this;
                pData->mpInvalidateRegion   = new Region( aInvalidateRegion );
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplCalcChildOverlapToTop( ImplCalcToTopData* pPrevData )
{
    DBG_ASSERT( ImplIsOverlapWindow(), "Window::ImplCalcChildOverlapToTop(): Is not a OverlapWindow" );

    ImplCalcToTop( pPrevData );
    if ( pPrevData->mpNext )
        pPrevData = pPrevData->mpNext;

    Window* pOverlap = mpWindowImpl->mpFirstOverlap;
    while ( pOverlap )
    {
        pOverlap->ImplCalcToTop( pPrevData );
        if ( pPrevData->mpNext )
            pPrevData = pPrevData->mpNext;
        pOverlap = pOverlap->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplToTop( USHORT nFlags )
{
    DBG_ASSERT( ImplIsOverlapWindow(), "Window::ImplToTop(): Is not a OverlapWindow" );

    if ( mpWindowImpl->mbFrame )
    {
        // Wenn in das externe Fenster geklickt wird, ist dieses
        // dafuer zustaendig dafuer zu sorgen, das unser Frame
        // nach vorne kommt
        if ( !mpWindowImpl->mpFrameData->mbHasFocus &&
             !mpWindowImpl->mpFrameData->mbSysObjFocus &&
             !mpWindowImpl->mpFrameData->mbInSysObjFocusHdl &&
             !mpWindowImpl->mpFrameData->mbInSysObjToTopHdl )
        {
            // do not bring floating windows on the client to top
            if( !ImplGetClientWindow() || !(ImplGetClientWindow()->GetStyle() & WB_SYSTEMFLOATWIN) )
            {
                USHORT nSysFlags = 0;
                if ( nFlags & TOTOP_RESTOREWHENMIN )
                    nSysFlags |= SAL_FRAME_TOTOP_RESTOREWHENMIN;
                if ( nFlags & TOTOP_FOREGROUNDTASK )
                    nSysFlags |= SAL_FRAME_TOTOP_FOREGROUNDTASK;
                if ( nFlags & TOTOP_GRABFOCUSONLY )
                    nSysFlags |= SAL_FRAME_TOTOP_GRABFOCUS_ONLY;
                mpWindowImpl->mpFrame->ToTop( nSysFlags );
            }
        }
    }
    else
    {
        if ( mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap != this )
        {
            // Fenster aus der Liste entfernen
            mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = mpWindowImpl->mpPrev;

            // AlwaysOnTop beruecksichtigen
            BOOL    bOnTop = IsAlwaysOnTopEnabled();
            Window* pNextWin = mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
            if ( !bOnTop )
            {
                while ( pNextWin )
                {
                    if ( !pNextWin->IsAlwaysOnTopEnabled() )
                        break;
                    pNextWin = pNextWin->mpWindowImpl->mpNext;
                }
            }

            // TopLevel abpruefen
            BYTE nTopLevel = mpWindowImpl->mpOverlapData->mnTopLevel;
            while ( pNextWin )
            {
                if ( (bOnTop != pNextWin->IsAlwaysOnTopEnabled()) ||
                     (nTopLevel <= pNextWin->mpWindowImpl->mpOverlapData->mnTopLevel) )
                    break;
                pNextWin = pNextWin->mpWindowImpl->mpNext;
            }

            // Fenster in die Liste wieder eintragen
            mpWindowImpl->mpNext = pNextWin;
            if ( pNextWin )
            {
                mpWindowImpl->mpPrev = pNextWin->mpWindowImpl->mpPrev;
                pNextWin->mpWindowImpl->mpPrev = this;
            }
            else
            {
                mpWindowImpl->mpPrev = mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap;
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = this;
            }
            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = this;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap = this;

            // ClipRegion muss von diesem Fenster und allen weiteren
            // ueberlappenden Fenstern neu berechnet werden.
            if ( IsReallyVisible() )
            {
                // Hintergrund-Sicherung zuruecksetzen
                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                mpWindowImpl->mpOverlapWindow->ImplSetClipFlagOverlapWindows();
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplStartToTop( USHORT nFlags )
{
    ImplCalcToTopData   aStartData;
    ImplCalcToTopData*  pCurData;
    ImplCalcToTopData*  pNextData;
    Window* pOverlapWindow;
    if ( ImplIsOverlapWindow() )
        pOverlapWindow = this;
    else
        pOverlapWindow = mpWindowImpl->mpOverlapWindow;

    // Zuerst die Paint-Bereiche berechnen
    Window* pTempOverlapWindow = pOverlapWindow;
    aStartData.mpNext = NULL;
    pCurData = &aStartData;
    do
    {
        pTempOverlapWindow->ImplCalcToTop( pCurData );
        if ( pCurData->mpNext )
            pCurData = pCurData->mpNext;
        pTempOverlapWindow = pTempOverlapWindow->mpWindowImpl->mpOverlapWindow;
    }
    while ( !pTempOverlapWindow->mpWindowImpl->mbFrame );
    // Dann die Paint-Bereiche der ChildOverlap-Windows berechnen
    pTempOverlapWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempOverlapWindow )
    {
        pTempOverlapWindow->ImplCalcToTop( pCurData );
        if ( pCurData->mpNext )
            pCurData = pCurData->mpNext;
        pTempOverlapWindow = pTempOverlapWindow->mpWindowImpl->mpNext;
    }

    // Dann die Fenster-Verkettung aendern
    pTempOverlapWindow = pOverlapWindow;
    do
    {
        pTempOverlapWindow->ImplToTop( nFlags );
        pTempOverlapWindow = pTempOverlapWindow->mpWindowImpl->mpOverlapWindow;
    }
    while ( !pTempOverlapWindow->mpWindowImpl->mbFrame );
    // Und zum Schluss invalidieren wir die ungueltigen Bereiche
    pCurData = aStartData.mpNext;
    while ( pCurData )
    {
        pCurData->mpWindow->ImplInvalidateFrameRegion( pCurData->mpInvalidateRegion, INVALIDATE_CHILDREN );
        pNextData = pCurData->mpNext;
        delete pCurData->mpInvalidateRegion;
        delete pCurData;
        pCurData = pNextData;
    }
}

// -----------------------------------------------------------------------

void Window::ImplFocusToTop( USHORT nFlags, BOOL bReallyVisible )
{
    // Soll Focus auch geholt werden?
    if ( !(nFlags & TOTOP_NOGRABFOCUS) )
    {
        // Erstes Fenster mit GrabFocus-Activate bekommt den Focus
        Window* pFocusWindow = this;
        while ( !pFocusWindow->ImplIsOverlapWindow() )
        {
            // Nur wenn Fenster kein Border-Fenster hat, da wir
            // immer das dazugehoerende BorderFenster finden wollen
            if ( !pFocusWindow->mpWindowImpl->mpBorderWindow )
            {
                if ( pFocusWindow->mpWindowImpl->mnActivateMode & ACTIVATE_MODE_GRABFOCUS )
                    break;
            }
            pFocusWindow = pFocusWindow->ImplGetParent();
        }
        if ( (pFocusWindow->mpWindowImpl->mnActivateMode & ACTIVATE_MODE_GRABFOCUS) &&
             !pFocusWindow->HasChildPathFocus( TRUE ) )
            pFocusWindow->GrabFocus();
    }

    if ( bReallyVisible )
        ImplGenerateMouseMove();
}

// -----------------------------------------------------------------------

void Window::ImplShowAllOverlaps()
{
    Window* pOverlapWindow = mpWindowImpl->mpFirstOverlap;
    while ( pOverlapWindow )
    {
        if ( pOverlapWindow->mpWindowImpl->mbOverlapVisible )
        {
            pOverlapWindow->Show( true, SHOW_NOACTIVATE );
            pOverlapWindow->mpWindowImpl->mbOverlapVisible = FALSE;
        }

        pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplHideAllOverlaps()
{
    Window* pOverlapWindow = mpWindowImpl->mpFirstOverlap;
    while ( pOverlapWindow )
    {
        if ( pOverlapWindow->IsVisible() )
        {
            pOverlapWindow->mpWindowImpl->mbOverlapVisible = TRUE;
            pOverlapWindow->Show( false );
        }

        pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallMouseMove( USHORT nMouseCode, BOOL bModChanged )
{
    if ( mpWindowImpl->mpFrameData->mbMouseIn && mpWindowImpl->mpFrameWindow->mpWindowImpl->mbReallyVisible )
    {
        ULONG   nTime   = Time::GetSystemTicks();
        long    nX      = mpWindowImpl->mpFrameData->mnLastMouseX;
        long    nY      = mpWindowImpl->mpFrameData->mnLastMouseY;
        USHORT  nCode   = nMouseCode;
        USHORT  nMode   = mpWindowImpl->mpFrameData->mnMouseMode;
        BOOL    bLeave;
        // Auf MouseLeave testen
        if ( ((nX < 0) || (nY < 0) ||
              (nX >= mpWindowImpl->mpFrameWindow->mnOutWidth) ||
              (nY >= mpWindowImpl->mpFrameWindow->mnOutHeight)) &&
             !ImplGetSVData()->maWinData.mpCaptureWin )
            bLeave = TRUE;
        else
            bLeave = FALSE;
        nMode |= MOUSE_SYNTHETIC;
        if ( bModChanged )
            nMode |= MOUSE_MODIFIERCHANGED;
        ImplHandleMouseEvent( mpWindowImpl->mpFrameWindow, EVENT_MOUSEMOVE, bLeave, nX, nY, nTime, nCode, nMode );
    }
}

// -----------------------------------------------------------------------

void Window::ImplGenerateMouseMove()
{
    if ( !mpWindowImpl->mpFrameData->mnMouseMoveId )
        Application::PostUserEvent( mpWindowImpl->mpFrameData->mnMouseMoveId, LINK( mpWindowImpl->mpFrameWindow, Window, ImplGenerateMouseMoveHdl ) );
}

// -----------------------------------------------------------------------

IMPL_LINK( Window, ImplGenerateMouseMoveHdl, void*, EMPTYARG )
{
    mpWindowImpl->mpFrameData->mnMouseMoveId = 0;
    Window* pCaptureWin = ImplGetSVData()->maWinData.mpCaptureWin;
    if( ! pCaptureWin ||
        (pCaptureWin->mpWindowImpl && pCaptureWin->mpWindowImpl->mpFrame == mpWindowImpl->mpFrame)
    )
    {
        ImplCallMouseMove( mpWindowImpl->mpFrameData->mnMouseCode );
    }
    return 0;
}

// -----------------------------------------------------------------------

void Window::ImplInvertFocus( const Rectangle& rRect )
{
    InvertTracking( rRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
}

// -----------------------------------------------------------------------

void Window::ImplCallFocusChangeActivate( Window* pNewOverlapWindow,
                                          Window* pOldOverlapWindow )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pNewRealWindow;
    Window*     pOldRealWindow;
    Window*     pLastRealWindow;
    BOOL        bCallActivate = TRUE;
    BOOL        bCallDeactivate = TRUE;

    pOldRealWindow = pOldOverlapWindow->ImplGetWindow();
    pNewRealWindow = pNewOverlapWindow->ImplGetWindow();
    if ( (pOldRealWindow->GetType() != WINDOW_FLOATINGWINDOW) ||
         pOldRealWindow->GetActivateMode() )
    {
        if ( (pNewRealWindow->GetType() == WINDOW_FLOATINGWINDOW) &&
             !pNewRealWindow->GetActivateMode() )
        {
            pSVData->maWinData.mpLastDeacWin = pOldOverlapWindow;
            bCallDeactivate = FALSE;
        }
    }
    else if ( (pNewRealWindow->GetType() != WINDOW_FLOATINGWINDOW) ||
              pNewRealWindow->GetActivateMode() )
    {
        if ( pSVData->maWinData.mpLastDeacWin )
        {
            if ( pSVData->maWinData.mpLastDeacWin == pNewOverlapWindow )
                bCallActivate = FALSE;
            else
            {
                pLastRealWindow = pSVData->maWinData.mpLastDeacWin->ImplGetWindow();
                pSVData->maWinData.mpLastDeacWin->mpWindowImpl->mbActive = FALSE;
                pSVData->maWinData.mpLastDeacWin->Deactivate();
                if ( pLastRealWindow != pSVData->maWinData.mpLastDeacWin )
                {
                    pLastRealWindow->mpWindowImpl->mbActive = TRUE;
                    pLastRealWindow->Activate();
                }
            }
            pSVData->maWinData.mpLastDeacWin = NULL;
        }
    }

    if ( bCallDeactivate )
    {
        if( pOldOverlapWindow->mpWindowImpl->mbActive )
        {
            pOldOverlapWindow->mpWindowImpl->mbActive = FALSE;
            pOldOverlapWindow->Deactivate();
        }
        if ( pOldRealWindow != pOldOverlapWindow )
        {
            if( pOldRealWindow->mpWindowImpl->mbActive )
            {
                pOldRealWindow->mpWindowImpl->mbActive = FALSE;
                pOldRealWindow->Deactivate();
            }
        }
    }
    if ( bCallActivate && ! pNewOverlapWindow->mpWindowImpl->mbActive )
    {
        if( ! pNewOverlapWindow->mpWindowImpl->mbActive )
        {
            pNewOverlapWindow->mpWindowImpl->mbActive = TRUE;
            pNewOverlapWindow->Activate();
        }
        if ( pNewRealWindow != pNewOverlapWindow )
        {
            if( ! pNewRealWindow->mpWindowImpl->mbActive )
            {
                pNewRealWindow->mpWindowImpl->mbActive = TRUE;
                pNewRealWindow->Activate();
            }
        }
    }
}

static bool IsWindowFocused(const WindowImpl& rWinImpl)
{
    if (rWinImpl.mpSysObj)
        return true;

    if (rWinImpl.mpFrameData->mbHasFocus)
        return true;

    if (rWinImpl.mbFakeFocusSet)
        return true;

    return false;
}

// -----------------------------------------------------------------------
void Window::ImplGrabFocus( USHORT nFlags )
{
    // #143570# no focus for destructing windows
    if( mpWindowImpl->mbInDtor )
        return;

    // some event listeners do really bad stuff
    // => prepare for the worst
    ImplDelData aDogTag( this );

    // Currently the client window should always get the focus
    // Should the border window at some point be focusable
    // we need to change all GrabFocus() instances in VCL,
    // e.g. in ToTop()

    if ( mpWindowImpl->mpClientWindow )
    {
        // For a lack of design we need a little hack here to
        // ensure that dialogs on close pass the focus back to
        // the correct window
        if ( mpWindowImpl->mpLastFocusWindow && (mpWindowImpl->mpLastFocusWindow != this) &&
             !(mpWindowImpl->mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) &&
             mpWindowImpl->mpLastFocusWindow->IsEnabled() &&
             mpWindowImpl->mpLastFocusWindow->IsInputEnabled() &&
             ! mpWindowImpl->mpLastFocusWindow->IsInModalMode()
             )
            mpWindowImpl->mpLastFocusWindow->GrabFocus();
        else
            mpWindowImpl->mpClientWindow->GrabFocus();
        return;
    }
    else if ( mpWindowImpl->mbFrame )
    {
        // For a lack of design we need a little hack here to
        // ensure that dialogs on close pass the focus back to
        // the correct window
        if ( mpWindowImpl->mpLastFocusWindow && (mpWindowImpl->mpLastFocusWindow != this) &&
             !(mpWindowImpl->mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) &&
             mpWindowImpl->mpLastFocusWindow->IsEnabled() &&
             mpWindowImpl->mpLastFocusWindow->IsInputEnabled() &&
             ! mpWindowImpl->mpLastFocusWindow->IsInModalMode()
             )
        {
            mpWindowImpl->mpLastFocusWindow->GrabFocus();
            return;
        }
    }

    // If the Window is disabled, then we don't change the focus
    if ( !IsEnabled() || !IsInputEnabled() || IsInModalMode() )
        return;

    // we only need to set the focus if it is not already set
    // note: if some other frame is waiting for an asynchrounous focus event
    // we also have to post an asynchronous focus event for this frame
    // which is done using ToTop
    ImplSVData* pSVData = ImplGetSVData();

    BOOL bAsyncFocusWaiting = FALSE;
    Window *pFrame = pSVData->maWinData.mpFirstFrame;
    while( pFrame  )
    {
        if( pFrame != mpWindowImpl->mpFrameWindow && pFrame->mpWindowImpl->mpFrameData->mnFocusId )
        {
            bAsyncFocusWaiting = TRUE;
            break;
        }
        pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
    }

    bool bHasFocus = IsWindowFocused(*mpWindowImpl);

    BOOL bMustNotGrabFocus = FALSE;
    // #100242#, check parent hierarchy if some floater prohibits grab focus

    Window *pParent = this;
    while( pParent )
    {
        // #102158#, ignore grabfocus only if the floating parent grabs keyboard focus by itself (GrabsFocus())
        // otherwise we cannot set the focus in a floating toolbox
        if( ( (pParent->mpWindowImpl->mbFloatWin && ((FloatingWindow*)pParent)->GrabsFocus()) || ( pParent->GetStyle() & WB_SYSTEMFLOATWIN ) ) && !( pParent->GetStyle() & WB_MOVEABLE ) )
        {
            bMustNotGrabFocus = TRUE;
            break;
        }
        pParent = pParent->mpWindowImpl->mpParent;
    }


    if ( ( pSVData->maWinData.mpFocusWin != this && ! mpWindowImpl->mbInDtor ) || ( bAsyncFocusWaiting && !bHasFocus && !bMustNotGrabFocus ) )
    {
        // EndExtTextInput if it is not the same window
        if ( pSVData->maWinData.mpExtTextInputWin &&
             (pSVData->maWinData.mpExtTextInputWin != this) )
            pSVData->maWinData.mpExtTextInputWin->EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );

        // Dieses Fenster als letztes FocusWindow merken
        Window* pOverlapWindow = ImplGetFirstOverlapWindow();
        pOverlapWindow->mpWindowImpl->mpLastFocusWindow = this;
        mpWindowImpl->mpFrameData->mpFocusWin = this;

        if( !bHasFocus )
        {
            // menue windows never get the system focus
            // the application will keep the focus
            if( bMustNotGrabFocus )
                return;
            else
            {
                // Hier setzen wir schon den Focus um, da ToTop() den Focus
                // nicht auf ein anderes Fenster setzen darf
                //DBG_WARNING( "Window::GrabFocus() - Frame doesn't have the focus" );
                mpWindowImpl->mpFrame->ToTop( SAL_FRAME_TOTOP_GRABFOCUS | SAL_FRAME_TOTOP_GRABFOCUS_ONLY );
                return;
            }
        }

        Window* pOldFocusWindow = pSVData->maWinData.mpFocusWin;
        ImplDelData aOldFocusDel( pOldFocusWindow );

        pSVData->maWinData.mpFocusWin = this;

        if ( pOldFocusWindow )
        {
            // Cursor hiden
            if ( pOldFocusWindow->mpWindowImpl->mpCursor )
                pOldFocusWindow->mpWindowImpl->mpCursor->ImplHide();
        }

        // !!!!! Wegen altem SV-Office Activate/Deavtivate Handling
        // !!!!! erstmal so wie frueher
        if ( pOldFocusWindow )
        {
            // Focus merken
            Window* pOldOverlapWindow = pOldFocusWindow->ImplGetFirstOverlapWindow();
            Window* pNewOverlapWindow = ImplGetFirstOverlapWindow();
            if ( pOldOverlapWindow != pNewOverlapWindow )
                ImplCallFocusChangeActivate( pNewOverlapWindow, pOldOverlapWindow );
        }
        else
        {
            Window* pNewOverlapWindow = ImplGetFirstOverlapWindow();
            Window* pNewRealWindow = pNewOverlapWindow->ImplGetWindow();
            pNewOverlapWindow->mpWindowImpl->mbActive = TRUE;
            pNewOverlapWindow->Activate();
            if ( pNewRealWindow != pNewOverlapWindow )
            {
                pNewRealWindow->mpWindowImpl->mbActive = TRUE;
                pNewRealWindow->Activate();
            }
        }

        // call Get- and LoseFocus
        if ( pOldFocusWindow && ! aOldFocusDel.IsDelete() )
        {
            if ( pOldFocusWindow->IsTracking() &&
                 (pSVData->maWinData.mnTrackFlags & STARTTRACK_FOCUSCANCEL) )
                pOldFocusWindow->EndTracking( ENDTRACK_CANCEL | ENDTRACK_FOCUS );
            NotifyEvent aNEvt( EVENT_LOSEFOCUS, pOldFocusWindow );
            if ( !ImplCallPreNotify( aNEvt ) )
                pOldFocusWindow->LoseFocus();
            pOldFocusWindow->ImplCallDeactivateListeners( this );
        }

        if ( pSVData->maWinData.mpFocusWin == this )
        {
            if ( mpWindowImpl->mpSysObj )
            {
                mpWindowImpl->mpFrameData->mpFocusWin = this;
                if ( !mpWindowImpl->mpFrameData->mbInSysObjFocusHdl )
                    mpWindowImpl->mpSysObj->GrabFocus();
            }

            if ( pSVData->maWinData.mpFocusWin == this )
            {
                if ( mpWindowImpl->mpCursor )
                    mpWindowImpl->mpCursor->ImplShow();
                mpWindowImpl->mbInFocusHdl = TRUE;
                mpWindowImpl->mnGetFocusFlags = nFlags;
                // if we're changing focus due to closing a popup floating window
                // notify the new focus window so it can restore the inner focus
                // eg, toolboxes can select their recent active item
                if( pOldFocusWindow &&
                    ! aOldFocusDel.IsDelete() &&
                    ( pOldFocusWindow->GetDialogControlFlags() & WINDOW_DLGCTRL_FLOATWIN_POPUPMODEEND_CANCEL ) )
                    mpWindowImpl->mnGetFocusFlags |= GETFOCUS_FLOATWIN_POPUPMODEEND_CANCEL;
                NotifyEvent aNEvt( EVENT_GETFOCUS, this );
                if ( !ImplCallPreNotify( aNEvt ) && !aDogTag.IsDelete() )
                    GetFocus();
                if( !aDogTag.IsDelete() )
                    ImplCallActivateListeners( (pOldFocusWindow && ! aOldFocusDel.IsDelete()) ? pOldFocusWindow : NULL );
                if( !aDogTag.IsDelete() )
                {
                    mpWindowImpl->mnGetFocusFlags = 0;
                    mpWindowImpl->mbInFocusHdl = FALSE;
                }
            }
        }

        GetpApp()->FocusChanged();
        ImplNewInputContext();
    }
}

// -----------------------------------------------------------------------

void Window::ImplNewInputContext()
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pFocusWin = pSVData->maWinData.mpFocusWin;
    if ( !pFocusWin )
        return;

    // Is InputContext changed?
    const InputContext& rInputContext = pFocusWin->GetInputContext();
    if ( rInputContext == pFocusWin->mpWindowImpl->mpFrameData->maOldInputContext )
        return;

    pFocusWin->mpWindowImpl->mpFrameData->maOldInputContext = rInputContext;

    SalInputContext         aNewContext;
    const Font&             rFont = rInputContext.GetFont();
    const XubString&        rFontName = rFont.GetName();
    ImplFontEntry*          pFontEntry = NULL;
    aNewContext.mpFont = NULL;
    if ( rFontName.Len() )
    {
        Size aSize = pFocusWin->ImplLogicToDevicePixel( rFont.GetSize() );
        if ( !aSize.Height() )
        {
            // Nur dann Defaultgroesse setzen, wenn Fonthoehe auch in logischen
            // Koordinaaten 0 ist
            if ( rFont.GetSize().Height() )
                aSize.Height() = 1;
            else
                aSize.Height() = (12*pFocusWin->mnDPIY)/72;
        }
        // TODO: No display device uses ImplDirectFontSubstitution thingy, right? => remove it
        ImplDirectFontSubstitution* pFontSubst = NULL;
        //if( pFocusWin->mpOutDevData )
        //    pFontSubst = &pFocusWin->mpOutDevData->maDevFontSubst;
        pFontEntry = pFocusWin->mpFontCache->GetFontEntry( pFocusWin->mpFontList,
                         rFont, aSize, static_cast<float>(aSize.Height()), pFontSubst );
        if ( pFontEntry )
            aNewContext.mpFont = &pFontEntry->maFontSelData;
    }
    aNewContext.meLanguage  = rFont.GetLanguage();
    aNewContext.mnOptions   = rInputContext.GetOptions();
    pFocusWin->ImplGetFrame()->SetInputContext( &aNewContext );

    if ( pFontEntry )
        pFocusWin->mpFontCache->Release( pFontEntry );
}

// -----------------------------------------------------------------------

Window::Window( WindowType nType )
{
    DBG_CTOR( Window, ImplDbgCheckWindow );

    ImplInitWindowData( nType );
}

// -----------------------------------------------------------------------

Window::Window( Window* pParent, WinBits nStyle )
{
    DBG_CTOR( Window, ImplDbgCheckWindow );

    ImplInitWindowData( WINDOW_WINDOW );
    ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

Window::Window( Window* pParent, const ResId& rResId )
{
    DBG_CTOR( Window, ImplDbgCheckWindow );

    ImplInitWindowData( WINDOW_WINDOW );
    rResId.SetRT( RSC_WINDOW );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle, NULL );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 0
namespace
{
    void lcl_appendWindowInfo( ByteString& io_rErrorString, const Window& i_rWindow )
    {
        // skip border windows, they don't carry information which helps diagnosing the problem
        const Window* pWindow( &i_rWindow );
        while ( pWindow && ( pWindow->GetType() == WINDOW_BORDERWINDOW ) )
            pWindow = pWindow->GetWindow( WINDOW_FIRSTCHILD );
        if ( !pWindow )
            pWindow = &i_rWindow;

        io_rErrorString += char(13);
        io_rErrorString += typeid( *pWindow ).name();
        io_rErrorString += " (window text: '";
        io_rErrorString += ByteString( pWindow->GetText(), RTL_TEXTENCODING_UTF8 );
        io_rErrorString += "')";
    }
}
#endif
// -----------------------------------------------------------------------

Window::~Window()
{
    vcl::LazyDeletor<Window>::Undelete( this );

    DBG_DTOR( Window, ImplDbgCheckWindow );
    DBG_ASSERT( !mpWindowImpl->mbInDtor, "~Window - already in DTOR!" );


    // remove Key and Mouse events issued by Application::PostKey/MouseEvent
    Application::RemoveMouseAndKeyEvents( this );

    // Dispose of the canvas implementation (which, currently, has an
    // own wrapper window as a child to this one.
    Reference< rendering::XCanvas > xCanvas( mpWindowImpl->mxCanvas );
    if( xCanvas.is() )
    {
        uno::Reference < lang::XComponent > xCanvasComponent( xCanvas,
                                                              uno::UNO_QUERY );
        if( xCanvasComponent.is() )
            xCanvasComponent->dispose();
    }

    mpWindowImpl->mbInDtor = TRUE;

    ImplCallEventListeners( VCLEVENT_OBJECT_DYING );

    // do not send child events for frames that were registered as native frames
    if( !ImplIsAccessibleNativeFrame() && mpWindowImpl->mbReallyVisible )
        if ( ImplIsAccessibleCandidate() && GetAccessibleParentWindow() )
            GetAccessibleParentWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_CHILDDESTROYED, this );

    // remove associated data structures from dockingmanager
    ImplGetDockingManager()->RemoveWindow( this );


    // remove ownerdraw decorated windows from list in the top-most frame window
    if( (GetStyle() & WB_OWNERDRAWDECORATION) && mpWindowImpl->mbFrame )
    {
        ::std::vector< Window* >& rList = ImplGetOwnerDrawList();
        ::std::vector< Window* >::iterator p;
        p = ::std::find( rList.begin(), rList.end(), this );
        if( p != rList.end() )
            rList.erase( p );
    }

    // shutdown drag and drop
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XComponent > xDnDComponent( mpWindowImpl->mxDNDListenerContainer, ::com::sun::star::uno::UNO_QUERY );

    if( xDnDComponent.is() )
        xDnDComponent->dispose();

    if( mpWindowImpl->mbFrame && mpWindowImpl->mpFrameData )
    {
        try
        {
            // deregister drop target listener
            if( mpWindowImpl->mpFrameData->mxDropTargetListener.is() )
            {
                Reference< XDragGestureRecognizer > xDragGestureRecognizer =
                    Reference< XDragGestureRecognizer > (mpWindowImpl->mpFrameData->mxDragSource, UNO_QUERY);
                if( xDragGestureRecognizer.is() )
                {
                    xDragGestureRecognizer->removeDragGestureListener(
                        Reference< XDragGestureListener > (mpWindowImpl->mpFrameData->mxDropTargetListener, UNO_QUERY));
                }

                mpWindowImpl->mpFrameData->mxDropTarget->removeDropTargetListener( mpWindowImpl->mpFrameData->mxDropTargetListener );
                mpWindowImpl->mpFrameData->mxDropTargetListener.clear();
            }

            // shutdown drag and drop for this frame window
            Reference< XComponent > xComponent( mpWindowImpl->mpFrameData->mxDropTarget, UNO_QUERY );

            // DNDEventDispatcher does not hold a reference of the DropTarget,
            // so it's ok if it does not support XComponent
            if( xComponent.is() )
                xComponent->dispose();
        }

        catch ( Exception exc )
        {
            // can be safely ignored here.
        }
    }

    UnoWrapperBase* pWrapper = Application::GetUnoWrapper( FALSE );
    if ( pWrapper )
        pWrapper->WindowDestroyed( this );

    // MT: Must be called after WindowDestroyed!
    // Otherwise, if the accessible is a VCLXWindow, it will try to destroy this window again!
    // But accessibility implementations from applications need this dispose.
    if ( mpWindowImpl->mxAccessible.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xC( mpWindowImpl->mxAccessible, ::com::sun::star::uno::UNO_QUERY );
        if ( xC.is() )
            xC->dispose();
    }

    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maHelpData.mpHelpWin && (pSVData->maHelpData.mpHelpWin->GetParent() == this) )
        ImplDestroyHelpWindow( true );

    DBG_ASSERT( pSVData->maWinData.mpTrackWin != this,
                "Window::~Window(): Window is in TrackingMode" );
    DBG_ASSERT( pSVData->maWinData.mpCaptureWin != this,
                "Window::~Window(): Window has the mouse captured" );
    // #103442# DefModalDialogParent is now determined on-the-fly, so this pointer is unimportant now
    //DBG_ASSERT( pSVData->maWinData.mpDefDialogParent != this,
    //            "Window::~Window(): Window is DefModalDialogParent" );

    // Wegen alter kompatibilitaet
    if ( pSVData->maWinData.mpTrackWin == this )
        EndTracking();
    if ( pSVData->maWinData.mpCaptureWin == this )
        ReleaseMouse();
    if ( pSVData->maWinData.mpDefDialogParent == this )
        pSVData->maWinData.mpDefDialogParent = NULL;

#ifdef DBG_UTIL
    if ( TRUE ) // always perform these tests in non-pro versions
    {
        ByteString  aErrorStr;
        BOOL        bError = FALSE;
        Window*     pTempWin = mpWindowImpl->mpFrameData->mpFirstOverlap;
        while ( pTempWin )
        {
            if ( ImplIsRealParentPath( pTempWin ) )
            {
                bError = TRUE;
                lcl_appendWindowInfo( aErrorStr, *pTempWin );
            }
            pTempWin = pTempWin->mpWindowImpl->mpNextOverlap;
        }
        if ( bError )
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") with living SystemWindow(s) destroyed: ";
            aTempStr += aErrorStr;
            DBG_ERROR( aTempStr.GetBuffer() );
            GetpApp()->Abort( String( aTempStr, RTL_TEXTENCODING_UTF8 ) );   // abort in non-pro version, this must be fixed!
        }

        bError = FALSE;
        pTempWin = pSVData->maWinData.mpFirstFrame;
        while ( pTempWin )
        {
            if ( ImplIsRealParentPath( pTempWin ) )
            {
                bError = TRUE;
                lcl_appendWindowInfo( aErrorStr, *pTempWin );
            }
            pTempWin = pTempWin->mpWindowImpl->mpFrameData->mpNextFrame;
        }
        if ( bError )
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") with living SystemWindow(s) destroyed: ";
            aTempStr += aErrorStr;
            DBG_ERROR( aTempStr.GetBuffer() );
            GetpApp()->Abort( String( aTempStr, RTL_TEXTENCODING_UTF8 ) );   // abort in non-pro version, this must be fixed!
        }

        if ( mpWindowImpl->mpFirstChild )
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") with living Child(s) destroyed: ";
            pTempWin = mpWindowImpl->mpFirstChild;
            while ( pTempWin )
            {
                lcl_appendWindowInfo( aTempStr, *pTempWin );
                pTempWin = pTempWin->mpWindowImpl->mpNext;
            }
            DBG_ERROR( aTempStr.GetBuffer() );
            GetpApp()->Abort( String( aTempStr, RTL_TEXTENCODING_UTF8 ) );   // abort in non-pro version, this must be fixed!
        }

        if ( mpWindowImpl->mpFirstOverlap )
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") with living SystemWindow(s) destroyed: ";
            pTempWin = mpWindowImpl->mpFirstOverlap;
            while ( pTempWin )
            {
                lcl_appendWindowInfo( aTempStr, *pTempWin );
                pTempWin = pTempWin->mpWindowImpl->mpNext;
            }
            DBG_ERROR( aTempStr.GetBuffer() );
            GetpApp()->Abort( String( aTempStr, RTL_TEXTENCODING_UTF8 ) );   // abort in non-pro version, this must be fixed!
        }

        Window* pMyParent = this;
        SystemWindow* pMySysWin = NULL;

        while ( pMyParent )
        {
            if ( pMyParent->IsSystemWindow() )
                pMySysWin = (SystemWindow*)pMyParent;
            pMyParent = pMyParent->GetParent();
        }
        if ( pMySysWin && pMySysWin->ImplIsInTaskPaneList( this ) )
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") still in TaskPanelList!";
            DBG_ERROR( aTempStr.GetBuffer() );
            GetpApp()->Abort( String( aTempStr, RTL_TEXTENCODING_UTF8 ) );   // abort in non-pro version, this must be fixed!
        }
    }
#endif

    if( mpWindowImpl->mbIsInTaskPaneList )
    {
        Window* pMyParent = this;
        SystemWindow* pMySysWin = NULL;

        while ( pMyParent )
        {
            if ( pMyParent->IsSystemWindow() )
                pMySysWin = (SystemWindow*)pMyParent;
            pMyParent = pMyParent->GetParent();
        }
        if ( pMySysWin && pMySysWin->ImplIsInTaskPaneList( this ) )
        {
            pMySysWin->GetTaskPaneList()->RemoveWindow( this );
        }
        else
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") not found in TaskPanelList!";
            DBG_ERROR( aTempStr.GetBuffer() );
        }
    }

    // Fenster hiden, um das entsprechende Paint-Handling auszuloesen
    Hide();

    // Mitteilen, das Fenster zerstoert wird
    {
    NotifyEvent aNEvt( EVENT_DESTROY, this );
    Notify( aNEvt );
    }

    // EndExtTextInputMode
    if ( pSVData->maWinData.mpExtTextInputWin == this )
    {
        EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );
        if ( pSVData->maWinData.mpExtTextInputWin == this )
            pSVData->maWinData.mpExtTextInputWin = NULL;
    }

    // check if the focus window is our child
    BOOL bHasFocussedChild = FALSE;
    if( pSVData->maWinData.mpFocusWin && ImplIsRealParentPath( pSVData->maWinData.mpFocusWin ) )
    {
        // #122232#, this must not happen and is an application bug ! but we try some cleanup to hopefully avoid crashes, see below
        bHasFocussedChild = TRUE;
#ifdef DBG_UTIL
        ByteString aTempStr( "Window (" );
        aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
        aTempStr += ") with focussed child window destroyed ! THIS WILL LEAD TO CRASHES AND MUST BE FIXED !";
        DBG_ERROR( aTempStr.GetBuffer() );
        GetpApp()->Abort( String( aTempStr, RTL_TEXTENCODING_UTF8 ) );   // abort in non-pro version, this must be fixed!
#endif
    }

    // Wenn wir den Focus haben, dann den Focus auf ein anderes Fenster setzen
    Window* pOverlapWindow = ImplGetFirstOverlapWindow();
    if ( pSVData->maWinData.mpFocusWin == this
        || bHasFocussedChild )  // #122232#, see above, try some cleanup
    {
        if ( mpWindowImpl->mbFrame )
        {
            pSVData->maWinData.mpFocusWin = NULL;
            pOverlapWindow->mpWindowImpl->mpLastFocusWindow = NULL;
            GetpApp()->FocusChanged();
        }
        else
        {
            Window* pParent = GetParent();
            Window* pBorderWindow = mpWindowImpl->mpBorderWindow;
            // Bei ueberlappenden Fenstern wird der Focus auf den
            // Parent vom naechsten FrameWindow gesetzt
            if ( pBorderWindow )
            {
                if ( pBorderWindow->ImplIsOverlapWindow() )
                    pParent = pBorderWindow->mpWindowImpl->mpOverlapWindow;
            }
            else if ( ImplIsOverlapWindow() )
                pParent = mpWindowImpl->mpOverlapWindow;

            if ( pParent && pParent->IsEnabled() && pParent->IsInputEnabled() && ! pParent->IsInModalMode() )
                pParent->GrabFocus();
            else
                mpWindowImpl->mpFrameWindow->GrabFocus();

            // If the focus was set back to 'this' set it to nothing
            if ( pSVData->maWinData.mpFocusWin == this )
            {
                pSVData->maWinData.mpFocusWin = NULL;
                pOverlapWindow->mpWindowImpl->mpLastFocusWindow = NULL;
                GetpApp()->FocusChanged();
            }
        }
    }


    if ( pOverlapWindow->mpWindowImpl->mpLastFocusWindow == this )
        pOverlapWindow->mpWindowImpl->mpLastFocusWindow = NULL;

    // reset hint for DefModalDialogParent
    if( pSVData->maWinData.mpActiveApplicationFrame == this )
        pSVData->maWinData.mpActiveApplicationFrame = NULL;

    // gemerkte Fenster zuruecksetzen
    if ( mpWindowImpl->mpFrameData->mpFocusWin == this )
        mpWindowImpl->mpFrameData->mpFocusWin = NULL;
    if ( mpWindowImpl->mpFrameData->mpMouseMoveWin == this )
        mpWindowImpl->mpFrameData->mpMouseMoveWin = NULL;
    if ( mpWindowImpl->mpFrameData->mpMouseDownWin == this )
        mpWindowImpl->mpFrameData->mpMouseDownWin = NULL;

    // Deactivate-Window zuruecksetzen
    if ( pSVData->maWinData.mpLastDeacWin == this )
        pSVData->maWinData.mpLastDeacWin = NULL;

    if ( mpWindowImpl->mbFrame )
    {
        if ( mpWindowImpl->mpFrameData->mnFocusId )
            Application::RemoveUserEvent( mpWindowImpl->mpFrameData->mnFocusId );
        if ( mpWindowImpl->mpFrameData->mnMouseMoveId )
            Application::RemoveUserEvent( mpWindowImpl->mpFrameData->mnMouseMoveId );
    }

    // Graphic freigeben
    ImplReleaseGraphics();

    // Evt. anderen Funktion mitteilen, das das Fenster geloescht
    // wurde
    ImplDelData* pDelData = mpWindowImpl->mpFirstDel;
    while ( pDelData )
    {
        pDelData->mbDel = TRUE;
        pDelData->mpWindow = NULL;  // #112873# pDel is not associated with a Window anymore
        pDelData = pDelData->mpNext;
    }

    // Fenster aus den Listen austragen
    ImplRemoveWindow( TRUE );

    // de-register as "top window child" at our parent, if necessary
    if ( mpWindowImpl->mbFrame )
    {
        BOOL bIsTopWindow = mpWindowImpl->mpWinData && ( mpWindowImpl->mpWinData->mnIsTopWindow == 1 );
        if ( mpWindowImpl->mpRealParent && bIsTopWindow )
        {
            ImplWinData* pParentWinData = mpWindowImpl->mpRealParent->ImplGetWinData();

            ::std::list< Window* >::iterator myPos = ::std::find( pParentWinData->maTopWindowChildren.begin(),
                pParentWinData->maTopWindowChildren.end(), this );
            DBG_ASSERT( myPos != pParentWinData->maTopWindowChildren.end(), "Window::~Window: inconsistency in top window chain!" );
            if ( myPos != pParentWinData->maTopWindowChildren.end() )
                pParentWinData->maTopWindowChildren.erase( myPos );
        }
    }

    // Extra Window Daten loeschen
    if ( mpWindowImpl->mpWinData )
    {
        if ( mpWindowImpl->mpWinData->mpExtOldText )
            delete mpWindowImpl->mpWinData->mpExtOldText;
        if ( mpWindowImpl->mpWinData->mpExtOldAttrAry )
            delete mpWindowImpl->mpWinData->mpExtOldAttrAry;
        if ( mpWindowImpl->mpWinData->mpCursorRect )
            delete mpWindowImpl->mpWinData->mpCursorRect;
        if ( mpWindowImpl->mpWinData->mpFocusRect )
            delete mpWindowImpl->mpWinData->mpFocusRect;
        if ( mpWindowImpl->mpWinData->mpTrackRect )
            delete mpWindowImpl->mpWinData->mpTrackRect;
        if ( mpWindowImpl->mpWinData->mpSmartHelpId )
            delete mpWindowImpl->mpWinData->mpSmartHelpId;
        if ( mpWindowImpl->mpWinData->mpSmartUniqueId )
            delete mpWindowImpl->mpWinData->mpSmartUniqueId;

        delete mpWindowImpl->mpWinData;
    }


    // Overlap-Window-Daten loeschen
    if ( mpWindowImpl->mpOverlapData )
    {
        delete mpWindowImpl->mpOverlapData;
    }

    // Evt. noch BorderWindow oder Frame zerstoeren
    if ( mpWindowImpl->mpBorderWindow )
        delete mpWindowImpl->mpBorderWindow;
    else if ( mpWindowImpl->mbFrame )
    {
        if ( pSVData->maWinData.mpFirstFrame == this )
            pSVData->maWinData.mpFirstFrame = mpWindowImpl->mpFrameData->mpNextFrame;
        else
        {
            Window* pSysWin = pSVData->maWinData.mpFirstFrame;
            while ( pSysWin->mpWindowImpl->mpFrameData->mpNextFrame != this )
                pSysWin = pSysWin->mpWindowImpl->mpFrameData->mpNextFrame;
            pSysWin->mpWindowImpl->mpFrameData->mpNextFrame = mpWindowImpl->mpFrameData->mpNextFrame;
        }
        mpWindowImpl->mpFrame->SetCallback( NULL, NULL );
        pSVData->mpDefInst->DestroyFrame( mpWindowImpl->mpFrame );
        delete mpWindowImpl->mpFrameData;
    }

    if ( mpWindowImpl->mpChildClipRegion )
        delete mpWindowImpl->mpChildClipRegion;

    delete mpWindowImpl->mpAccessibleInfos;
    delete mpWindowImpl->mpControlFont;

    // should be the last statements
    delete mpWindowImpl; mpWindowImpl = NULL;
}

// -----------------------------------------------------------------------
void Window::doLazyDelete()
{
    SystemWindow* pSysWin = dynamic_cast<SystemWindow*>(this);
    DockingWindow* pDockWin = dynamic_cast<DockingWindow*>(this);
    if( pSysWin || ( pDockWin && pDockWin->IsFloatingMode() ) )
    {
        Show( false );
        SetParent( ImplGetDefaultWindow() );
    }
    vcl::LazyDeletor<Window>::Delete( this );
}

USHORT Window::GetIndicatorState() const
{
    return mpWindowImpl->mpFrame->GetIndicatorState().mnState;
}

void Window::SimulateKeyPress( USHORT nKeyCode ) const
{
    mpWindowImpl->mpFrame->SimulateKeyPress(nKeyCode);
}

// -----------------------------------------------------------------------

void Window::MouseMove( const MouseEvent& rMEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_MOUSEMOVE, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseMove = TRUE;
}

// -----------------------------------------------------------------------

void Window::MouseButtonDown( const MouseEvent& rMEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_MOUSEBUTTONDOWN, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseButtonDown = TRUE;
}

// -----------------------------------------------------------------------

void Window::MouseButtonUp( const MouseEvent& rMEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_MOUSEBUTTONUP, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseButtonUp = TRUE;
}

// -----------------------------------------------------------------------

void Window::KeyInput( const KeyEvent& rKEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_KEYINPUT, this, &rKEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbKeyInput = TRUE;
}

// -----------------------------------------------------------------------

void Window::KeyUp( const KeyEvent& rKEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_KEYUP, this, &rKEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbKeyUp = TRUE;
}

// -----------------------------------------------------------------------

void Window::PrePaint()
{
}

// -----------------------------------------------------------------------

void Window::Paint( const Rectangle& rRect )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    ImplCallEventListeners( VCLEVENT_WINDOW_PAINT, (void*)&rRect );
}

// -----------------------------------------------------------------------

void Window::Draw( OutputDevice*, const Point&, const Size&, ULONG )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

void Window::Move()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

void Window::Resize()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

void Window::Activate()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

void Window::Deactivate()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

void Window::GetFocus()
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    if ( HasFocus() && mpWindowImpl->mpLastFocusWindow && !(mpWindowImpl->mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) )
    {
        ImplDelData aDogtag( this );
        mpWindowImpl->mpLastFocusWindow->GrabFocus();
        if( aDogtag.IsDelete() )
            return;
    }

    NotifyEvent aNEvt( EVENT_GETFOCUS, this );
    Notify( aNEvt );
}

// -----------------------------------------------------------------------

void Window::LoseFocus()
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_LOSEFOCUS, this );
    Notify( aNEvt );
}

// -----------------------------------------------------------------------

void Window::RequestHelp( const HelpEvent& rHEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    // Wenn Balloon-Help angefordert wird, dann den Balloon mit dem
    // gesetzten Hilfetext anzeigen
    if ( rHEvt.GetMode() & HELPMODE_BALLOON )
    {
        const XubString* pStr = &(GetHelpText());
        if ( !pStr->Len() )
            pStr = &(GetQuickHelpText());
        if ( !pStr->Len() && ImplGetParent() && !ImplIsOverlapWindow() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
            Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), *pStr );
    }
    else if ( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        const XubString* pStr = &(GetQuickHelpText());
        if ( !pStr->Len() && ImplGetParent() && !ImplIsOverlapWindow() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            Point aPos = GetPosPixel();
            if ( ImplGetParent() && !ImplIsOverlapWindow() )
                aPos = ImplGetParent()->OutputToScreenPixel( aPos );
            Rectangle   aRect( aPos, GetSizePixel() );
            String      aHelpText;
            if ( pStr->Len() )
                aHelpText = GetHelpText();
            Help::ShowQuickHelp( this, aRect, *pStr, aHelpText, QUICKHELP_CTRLTEXT );
        }
    }
    else
    {
        SmartId aSmartId = GetSmartHelpId();

        ULONG nNumHelpId = 0;
        String aStrHelpId;
        if( aSmartId.HasString() )
            aStrHelpId = aSmartId.GetStr();
        if( aSmartId.HasNumeric() )
            nNumHelpId = aSmartId.GetNum();

        if ( !nNumHelpId && aStrHelpId.Len() == 0 && ImplGetParent() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            if ( !nNumHelpId && aStrHelpId.Len() == 0 )
                nNumHelpId = OOO_HELP_INDEX;

            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                if( aStrHelpId.Len() > 0 )
                    pHelp->Start( aStrHelpId, this );
                else
                    pHelp->Start( nNumHelpId, this );
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::Command( const CommandEvent& rCEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    ImplCallEventListeners( VCLEVENT_WINDOW_COMMAND, (void*)&rCEvt );

    NotifyEvent aNEvt( EVENT_COMMAND, this, &rCEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbCommand = TRUE;
}

// -----------------------------------------------------------------------

void Window::Tracking( const TrackingEvent& rTEvt )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
        pWrapper->Tracking( rTEvt );
}

// -----------------------------------------------------------------------

void Window::UserEvent( ULONG, void* )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

void Window::StateChanged( StateChangedType )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

void Window::DataChanged( const DataChangedEvent& )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

void Window::ImplNotifyKeyMouseCommandEventListeners( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_COMMAND )
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        if ( pCEvt->GetCommand() != COMMAND_CONTEXTMENU )
            // non context menu events are not to be notified up the chain
            // so we return immediately
            return;

        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                // not interested in: The event listeners are already called in ::Command,
                // and calling them here a second time doesn't make sense
                ;
            else
            {
                CommandEvent aCommandEvent = ImplTranslateCommandEvent( *pCEvt, rNEvt.GetWindow(), this );
                ImplCallEventListeners( VCLEVENT_WINDOW_COMMAND, &aCommandEvent );
            }
        }
    }

    // #82968# notify event listeners for mouse and key events seperately and
    // not in PreNotify ( as for focus listeners )
    // this allows for procesing those events internally first and pass it to
    // the toolkit later

    ImplDelData aDelData;
    ImplAddDel( &aDelData );

    if( rNEvt.GetType() == EVENT_MOUSEMOVE )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEMOVE, (void*)rNEvt.GetMouseEvent() );
            else
            {
                MouseEvent aMouseEvent = ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this );
                ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEMOVE, &aMouseEvent );
            }
        }
    }
    else if( rNEvt.GetType() == EVENT_MOUSEBUTTONUP )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONUP, (void*)rNEvt.GetMouseEvent() );
            else
            {
                MouseEvent aMouseEvent = ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this );
                ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONUP, &aMouseEvent );
            }
        }
    }
    else if( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, (void*)rNEvt.GetMouseEvent() );
            else
            {
                MouseEvent aMouseEvent = ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this );
                ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, &aMouseEvent );
            }
        }
    }
    else if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
            ImplCallEventListeners( VCLEVENT_WINDOW_KEYINPUT, (void*)rNEvt.GetKeyEvent() );
    }
    else if( rNEvt.GetType() == EVENT_KEYUP )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
            ImplCallEventListeners( VCLEVENT_WINDOW_KEYUP, (void*)rNEvt.GetKeyEvent() );
    }

    if ( aDelData.IsDelete() )
        return;
    ImplRemoveDel( &aDelData );

    // #106721# check if we're part of a compound control and notify
    Window *pParent = ImplGetParent();
    while( pParent )
    {
        if( pParent->IsCompoundControl() )
        {
            pParent->ImplNotifyKeyMouseCommandEventListeners( rNEvt );
            break;
        }
        pParent = pParent->ImplGetParent();
    }
}

// -----------------------------------------------------------------------

long Window::PreNotify( NotifyEvent& rNEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    long bDone = FALSE;
    if ( mpWindowImpl->mpParent && !ImplIsOverlapWindow() )
        bDone = mpWindowImpl->mpParent->PreNotify( rNEvt );

    if ( !bDone )
    {
        if( rNEvt.GetType() == EVENT_GETFOCUS )
        {
            BOOL bCompoundFocusChanged = FALSE;
            if ( mpWindowImpl->mbCompoundControl && !mpWindowImpl->mbCompoundControlHasFocus && HasChildPathFocus() )
            {
                mpWindowImpl->mbCompoundControlHasFocus = TRUE;
                bCompoundFocusChanged = TRUE;
            }

            if ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) )
                ImplCallEventListeners( VCLEVENT_WINDOW_GETFOCUS );
        }
        else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
        {
            BOOL bCompoundFocusChanged = FALSE;
            if ( mpWindowImpl->mbCompoundControl && mpWindowImpl->mbCompoundControlHasFocus && !HasChildPathFocus() )
            {
                mpWindowImpl->mbCompoundControlHasFocus = FALSE ;
                bCompoundFocusChanged = TRUE;
            }

            if ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) )
                ImplCallEventListeners( VCLEVENT_WINDOW_LOSEFOCUS );
        }

        // #82968# mouse and key events will be notified after processing ( in ImplNotifyKeyMouseCommandEventListeners() )!
        //    see also ImplHandleMouseEvent(), ImplHandleKey()

    }

    return bDone;
}

// -----------------------------------------------------------------------

long Window::Notify( NotifyEvent& rNEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    long nRet = FALSE;

    // check for docking window
    // but do nothing if window is docked and locked
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper && !( !pWrapper->IsFloatingMode() && pWrapper->IsLocked() ) )
    {
        if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            BOOL bHit = pWrapper->GetDragArea().IsInside( pMEvt->GetPosPixel() );
            if ( pMEvt->IsLeft() )
            {
                if ( pMEvt->IsMod1() && (pMEvt->GetClicks() == 2) )
                {
                    // ctrl double click toggles floating mode
                    pWrapper->SetFloatingMode( !pWrapper->IsFloatingMode() );
                    return TRUE;
                }
                else if ( pMEvt->GetClicks() == 1 && bHit)
                {
                    // allow start docking during mouse move
                    pWrapper->ImplEnableStartDocking();
                    return TRUE;
                }
            }
        }
        else if ( rNEvt.GetType() == EVENT_MOUSEMOVE )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            BOOL bHit = pWrapper->GetDragArea().IsInside( pMEvt->GetPosPixel() );
            if ( pMEvt->IsLeft() )
            {
                // check if a single click initiated this sequence ( ImplStartDockingEnabled() )
                // check if window is docked and
                if( pWrapper->ImplStartDockingEnabled() && !pWrapper->IsFloatingMode() &&
                    !pWrapper->IsDocking() && bHit )
                {
                    Point   aPos = pMEvt->GetPosPixel();
                    Window* pWindow = rNEvt.GetWindow();
                    if ( pWindow != this )
                    {
                        aPos = pWindow->OutputToScreenPixel( aPos );
                        aPos = ScreenToOutputPixel( aPos );
                    }
                    pWrapper->ImplStartDocking( aPos );
                }
                return TRUE;
            }
        }
        else if( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            const KeyCode& rKey = rNEvt.GetKeyEvent()->GetKeyCode();
            if( rKey.GetCode() == KEY_F10 && rKey.GetModifier() &&
                rKey.IsShift() && rKey.IsMod1() )
            {
                pWrapper->SetFloatingMode( !pWrapper->IsFloatingMode() );
                /* At this point the floating toolbar frame does not have the
                 * input focus since these frames don't get the focus per default
                 * To enable keyboard handling of this toolbar set the input focus
                 * to the frame. This needs to be done with ToTop since GrabFocus
                 * would not notice any change since "this" already has the focus.
                 */
                if( pWrapper->IsFloatingMode() )
                    ToTop( TOTOP_GRABFOCUSONLY );
                return TRUE;
            }
        }
    }

    // Dialog-Steuerung
    if ( (GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL )
    {
        // Wenn Parent auch DialogSteuerung aktiviert hat, uebernimmt dieser die Steuerung
        if ( (rNEvt.GetType() == EVENT_KEYINPUT) || (rNEvt.GetType() == EVENT_KEYUP) )
        {
            if ( ImplIsOverlapWindow() ||
                 ((ImplGetParent()->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
            {
                nRet = ImplDlgCtrl( *rNEvt.GetKeyEvent(), rNEvt.GetType() == EVENT_KEYINPUT );
            }
        }
        else if ( (rNEvt.GetType() == EVENT_GETFOCUS) || (rNEvt.GetType() == EVENT_LOSEFOCUS) )
        {
            ImplDlgCtrlFocusChanged( rNEvt.GetWindow(), rNEvt.GetType() == EVENT_GETFOCUS );
            if ( (rNEvt.GetWindow() == this) && (rNEvt.GetType() == EVENT_GETFOCUS) &&
                 !(GetStyle() & WB_TABSTOP) && !(mpWindowImpl->mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) )
            {
                USHORT n = 0;
                Window* pFirstChild = ImplGetDlgWindow( n, DLGWINDOW_FIRST );
                if ( pFirstChild )
                    pFirstChild->ImplControlFocus();
            }
        }
    }

    if ( !nRet )
    {
        if ( mpWindowImpl->mpParent && !ImplIsOverlapWindow() )
            nRet = mpWindowImpl->mpParent->Notify( rNEvt );
    }

    return nRet;
}

// -----------------------------------------------------------------------

void Window::ImplCallEventListeners( ULONG nEvent, void* pData )
{
    // The implementation was moved to CallEventListeners(),
    // because derived classes in svtools must be able to
    // call the event listeners and ImplCallEventListeners()
    // is not exported.
    // TODO: replace ImplCallEventListeners() by CallEventListeners() in vcl

    CallEventListeners( nEvent, pData );
}

// -----------------------------------------------------------------------

void Window::CallEventListeners( ULONG nEvent, void* pData )
{
    VclWindowEvent aEvent( this, nEvent, pData );

    ImplDelData aDelData;
    ImplAddDel( &aDelData );

    ImplGetSVData()->mpApp->ImplCallEventListeners( &aEvent );

    if ( aDelData.IsDelete() )
        return;

    mpWindowImpl->maEventListeners.Call( &aEvent );

    if ( aDelData.IsDelete() )
        return;

    ImplRemoveDel( &aDelData );

    Window* pWindow = this;
    while ( pWindow )
    {
        pWindow->ImplAddDel( &aDelData );

        pWindow->mpWindowImpl->maChildEventListeners.Call( &aEvent );

        if ( aDelData.IsDelete() )
            return;

        pWindow->ImplRemoveDel( &aDelData );

        pWindow = pWindow->GetParent();
    }
}

void Window::FireVclEvent( VclSimpleEvent* pEvent )
{
    ImplGetSVData()->mpApp->ImplCallEventListeners(pEvent);
}

// -----------------------------------------------------------------------

void Window::AddEventListener( const Link& rEventListener )
{
    mpWindowImpl->maEventListeners.addListener( rEventListener );
}

// -----------------------------------------------------------------------

void Window::RemoveEventListener( const Link& rEventListener )
{
    mpWindowImpl->maEventListeners.removeListener( rEventListener );
}

// -----------------------------------------------------------------------

void Window::AddChildEventListener( const Link& rEventListener )
{
    mpWindowImpl->maChildEventListeners.addListener( rEventListener );
}

// -----------------------------------------------------------------------

void Window::RemoveChildEventListener( const Link& rEventListener )
{
    mpWindowImpl->maChildEventListeners.removeListener( rEventListener );
}

// -----------------------------------------------------------------------

ULONG Window::PostUserEvent( ULONG nEvent, void* pEventData )
{
    ULONG nEventId;
    PostUserEvent( nEventId, nEvent, pEventData );
    return nEventId;
}

// -----------------------------------------------------------------------

ULONG Window::PostUserEvent( const Link& rLink, void* pCaller )
{
    ULONG nEventId;
    PostUserEvent( nEventId, rLink, pCaller );
    return nEventId;
}

// -----------------------------------------------------------------------

BOOL Window::PostUserEvent( ULONG& rEventId, ULONG nEvent, void* pEventData )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mnEvent   = nEvent;
    pSVEvent->mpData    = pEventData;
    pSVEvent->mpLink    = NULL;
    pSVEvent->mpWindow  = this;
    pSVEvent->mbCall    = TRUE;
    ImplAddDel( &(pSVEvent->maDelData) );
    rEventId = (ULONG)pSVEvent;
    if ( mpWindowImpl->mpFrame->PostEvent( pSVEvent ) )
        return TRUE;
    else
    {
        rEventId = 0;
        ImplRemoveDel( &(pSVEvent->maDelData) );
        delete pSVEvent;
        return FALSE;
    }
}

// -----------------------------------------------------------------------

BOOL Window::PostUserEvent( ULONG& rEventId, const Link& rLink, void* pCaller )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mnEvent   = 0;
    pSVEvent->mpData    = pCaller;
    pSVEvent->mpLink    = new Link( rLink );
    pSVEvent->mpWindow  = this;
    pSVEvent->mbCall    = TRUE;
    ImplAddDel( &(pSVEvent->maDelData) );
    rEventId = (ULONG)pSVEvent;
    if ( mpWindowImpl->mpFrame->PostEvent( pSVEvent ) )
        return TRUE;
    else
    {
        rEventId = 0;
        ImplRemoveDel( &(pSVEvent->maDelData) );
        delete pSVEvent;
        return FALSE;
    }
}

// -----------------------------------------------------------------------

void Window::RemoveUserEvent( ULONG nUserEvent )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVEvent* pSVEvent = (ImplSVEvent*)nUserEvent;

    DBG_ASSERT( pSVEvent->mpWindow == this,
                "Window::RemoveUserEvent(): Event doesn't send to this window or is already removed" );
    DBG_ASSERT( pSVEvent->mbCall,
                "Window::RemoveUserEvent(): Event is already removed" );

    if ( pSVEvent->mpWindow )
    {
        pSVEvent->mpWindow->ImplRemoveDel( &(pSVEvent->maDelData) );
        pSVEvent->mpWindow = NULL;
    }

    pSVEvent->mbCall = FALSE;
}

// -----------------------------------------------------------------------

IMPL_LINK( Window, ImplAsyncStateChangedHdl, void*, pState )
{
    StateChanged( (StateChangedType)(ULONG)pState );
    return 0;
}

// -----------------------------------------------------------------------

void Window::PostStateChanged( StateChangedType nState )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    PostUserEvent( LINK( this, Window, ImplAsyncStateChangedHdl ), (void*)(ULONG)nState );
}

// -----------------------------------------------------------------------

BOOL Window::IsLocked( BOOL bChilds ) const
{
    if ( mpWindowImpl->mnLockCount != 0 )
        return TRUE;

    if ( bChilds || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            if ( pChild->IsLocked( TRUE ) )
                return TRUE;
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void Window::SetStyle( WinBits nStyle )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mnStyle != nStyle )
    {
        mpWindowImpl->mnPrevStyle = mpWindowImpl->mnStyle;
        mpWindowImpl->mnStyle = nStyle;
        StateChanged( STATE_CHANGE_STYLE );
    }
}

// -----------------------------------------------------------------------

void Window::SetExtendedStyle( WinBits nExtendedStyle )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mnExtendedStyle != nExtendedStyle )
    {
        Window* pWindow = ImplGetBorderWindow();
        if( ! pWindow )
            pWindow = this;
        if( pWindow->mpWindowImpl->mbFrame )
        {
            SalExtStyle nExt = 0;
            if( (nExtendedStyle & WB_EXT_DOCUMENT) )
                nExt |= SAL_FRAME_EXT_STYLE_DOCUMENT;
            if( (nExtendedStyle & WB_EXT_DOCMODIFIED) )
                nExt |= SAL_FRAME_EXT_STYLE_DOCMODIFIED;

            pWindow->ImplGetFrame()->SetExtendedFrameStyle( nExt );
        }
        mpWindowImpl->mnPrevExtendedStyle = mpWindowImpl->mnExtendedStyle;
        mpWindowImpl->mnExtendedStyle = nExtendedStyle;
        StateChanged( STATE_CHANGE_EXTENDEDSTYLE );
    }
}

// -----------------------------------------------------------------------

SystemWindow* Window::GetSystemWindow() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    const Window* pWin = this;
    while ( pWin && !pWin->IsSystemWindow() )
        pWin  = pWin->GetParent();
    return (SystemWindow*)pWin;
}

// -----------------------------------------------------------------------

void Window::SetBorderStyle( USHORT nBorderStyle )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
    {
        if( nBorderStyle == WINDOW_BORDER_REMOVEBORDER &&
            ! mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame &&
            mpWindowImpl->mpBorderWindow->mpWindowImpl->mpParent
            )
        {
            // this is a little awkward: some controls (e.g. svtools ProgressBar)
            // cannot avoid getting constructed with WB_BORDER but want to disable
            // borders in case of NWF drawing. So they need a method to remove their border window
            Window* pBorderWin = mpWindowImpl->mpBorderWindow;
            // remove us as border window's client
            pBorderWin->mpWindowImpl->mpClientWindow = NULL;
            mpWindowImpl->mpBorderWindow = NULL;
            mpWindowImpl->mpRealParent = pBorderWin->mpWindowImpl->mpParent;
            // reparent us above the border window
            SetParent( pBorderWin->mpWindowImpl->mpParent );
            // set us to the position and size of our previous border
            Point aBorderPos( pBorderWin->GetPosPixel() );
            Size aBorderSize( pBorderWin->GetSizePixel() );
            SetPosSizePixel( aBorderPos.X(), aBorderPos.Y(), aBorderSize.Width(), aBorderSize.Height() );
            // release border window
            delete pBorderWin;

            // set new style bits
            SetStyle( GetStyle() & (~WB_BORDER) );
        }
        else
        {
            if ( mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
                 ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetBorderStyle( nBorderStyle );
            else
                mpWindowImpl->mpBorderWindow->SetBorderStyle( nBorderStyle );
        }
    }
}

// -----------------------------------------------------------------------

USHORT Window::GetBorderStyle() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
    {
        if ( mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
            return ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->GetBorderStyle();
        else
            return mpWindowImpl->mpBorderWindow->GetBorderStyle();
    }

    return 0;
}

// -----------------------------------------------------------------------

long Window::CalcTitleWidth() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
    {
        if ( mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
            return ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->CalcTitleWidth();
        else
            return mpWindowImpl->mpBorderWindow->CalcTitleWidth();
    }
    else if ( mpWindowImpl->mbFrame && (mpWindowImpl->mnStyle & WB_MOVEABLE) )
    {
        // Fuer Frame-Fenster raten wir die Breite, da wir den Border fuer
        // externe Dialoge nicht kennen
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Font aFont = GetFont();
        ((Window*)this)->SetPointFont( rStyleSettings.GetTitleFont() );
        long nTitleWidth = GetTextWidth( GetText() );
        ((Window*)this)->SetFont( aFont );
        nTitleWidth += rStyleSettings.GetTitleHeight() * 3;
        nTitleWidth += rStyleSettings.GetBorderSize() * 2;
        nTitleWidth += 10;
        return nTitleWidth;
    }

    return 0;
}

// -----------------------------------------------------------------------

void Window::EnableClipSiblings( BOOL bClipSiblings )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->EnableClipSiblings( bClipSiblings );

    mpWindowImpl->mbClipSiblings = bClipSiblings;
}

// -----------------------------------------------------------------------

void Window::SetMouseTransparent( BOOL bTransparent )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetMouseTransparent( bTransparent );

    if( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->SetMouseTransparent( bTransparent );

    mpWindowImpl->mbMouseTransparent = bTransparent;
}

// -----------------------------------------------------------------------

void Window::SetPaintTransparent( BOOL bTransparent )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    // transparency is not useful for frames as the background would have to be provided by a different frame
    if( bTransparent && mpWindowImpl->mbFrame )
        return;

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetPaintTransparent( bTransparent );

    mpWindowImpl->mbPaintTransparent = bTransparent;
}

// -----------------------------------------------------------------------

void Window::SetInputContext( const InputContext& rInputContext )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mpWindowImpl->maInputContext = rInputContext;
    if ( !mpWindowImpl->mbInFocusHdl && HasFocus() )
        ImplNewInputContext();
}

// -----------------------------------------------------------------------

void Window::EndExtTextInput( USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mbExtTextInput )
        ImplGetFrame()->EndExtTextInput( nFlags );
}

// -----------------------------------------------------------------------

void Window::SetCursorRect( const Rectangle* pRect, long nExtTextInputWidth )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplWinData* pWinData = ImplGetWinData();
    if ( pWinData->mpCursorRect )
    {
        if ( pRect )
            *pWinData->mpCursorRect = *pRect;
        else
        {
            delete pWinData->mpCursorRect;
            pWinData->mpCursorRect = NULL;
        }
    }
    else
    {
        if ( pRect )
            pWinData->mpCursorRect = new Rectangle( *pRect );
    }

    pWinData->mnCursorExtWidth = nExtTextInputWidth;

}

// -----------------------------------------------------------------------

const Rectangle* Window::GetCursorRect() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplWinData* pWinData = ImplGetWinData();
    return pWinData->mpCursorRect;
}

// -----------------------------------------------------------------------

long Window::GetCursorExtTextInputWidth() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplWinData* pWinData = ImplGetWinData();
    return pWinData->mnCursorExtWidth;
}

// -----------------------------------------------------------------------
void Window::SetSettings( const AllSettings& rSettings )
{
    SetSettings( rSettings, FALSE );
}

void Window::SetSettings( const AllSettings& rSettings, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->SetSettings( rSettings, FALSE );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->SetSettings( rSettings, TRUE );
    }

    AllSettings aOldSettings = maSettings;
    OutputDevice::SetSettings( rSettings );
    ULONG nChangeFlags = aOldSettings.GetChangeFlags( rSettings );

    // AppFont-Aufloesung und DPI-Aufloesung neu berechnen
    ImplInitResolutionSettings();

    if ( nChangeFlags )
    {
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
        DataChanged( aDCEvt );
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->SetSettings( rSettings, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::UpdateSettings( const AllSettings& rSettings, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->UpdateSettings( rSettings, FALSE );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->UpdateSettings( rSettings, TRUE );
    }

    AllSettings aOldSettings = maSettings;
    ULONG nChangeFlags = maSettings.Update( maSettings.GetWindowUpdate(), rSettings );
    nChangeFlags |= SETTINGS_IN_UPDATE_SETTINGS; // Set this flag so the receiver of the data changed
                                                 // event can distinguish between the changing of global
                                                 // setting and a local change ( with SetSettings )

    // AppFont-Aufloesung und DPI-Aufloesung neu berechnen
    ImplInitResolutionSettings();

    /* #i73785#
    *  do not overwrite a WheelBehavior with false
    *  this looks kind of a hack, but WheelBehavior
    *  is always a local change, not a system property,
    *  so we can spare all our users the hassle of reacting on
    *  this in their respective DataChanged.
    */
    MouseSettings aSet( maSettings.GetMouseSettings() );
    aSet.SetWheelBehavior( aOldSettings.GetMouseSettings().GetWheelBehavior() );
    maSettings.SetMouseSettings( aSet );

    if( (nChangeFlags & SETTINGS_STYLE) && IsBackground() )
    {
        Wallpaper aWallpaper = GetBackground();
        if( !aWallpaper.IsBitmap() && !aWallpaper.IsGradient() )
        {
            if ( mpWindowImpl->mnStyle & WB_3DLOOK )
                SetBackground( Wallpaper( rSettings.GetStyleSettings().GetFaceColor() ) );
            else
                SetBackground( Wallpaper( rSettings.GetStyleSettings().GetWindowColor() ) );
        }
    }

    if ( nChangeFlags )
    {
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
        DataChanged( aDCEvt );
        // notify data change handler
        ImplCallEventListeners( VCLEVENT_WINDOW_DATACHANGED, &aDCEvt);
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->UpdateSettings( rSettings, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::NotifyAllChilds( DataChangedEvent& rDCEvt )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    DataChanged( rDCEvt );

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->NotifyAllChilds( rDCEvt );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::SetPointFont( const Font& rFont )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Font aFont = rFont;
    ImplPointToLogic( aFont );
    SetFont( aFont );
}

// -----------------------------------------------------------------------

Font Window::GetPointFont() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Font aFont = GetFont();
    ImplLogicToPoint( aFont );
    return aFont;
}

// -----------------------------------------------------------------------

void Window::SetParentClipMode( USHORT nMode )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetParentClipMode( nMode );
    else
    {
        if ( !ImplIsOverlapWindow() )
        {
            mpWindowImpl->mnParentClipMode = nMode;
            if ( nMode & PARENTCLIPMODE_CLIP )
                mpWindowImpl->mpParent->mpWindowImpl->mbClipChildren = TRUE;
        }
    }
}

// -----------------------------------------------------------------------

USHORT Window::GetParentClipMode() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        return mpWindowImpl->mpBorderWindow->GetParentClipMode();
    else
        return mpWindowImpl->mnParentClipMode;
}

// -----------------------------------------------------------------------

void Window::SetWindowRegionPixel()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetWindowRegionPixel();
    else if( mpWindowImpl->mbFrame )
    {
        mpWindowImpl->maWinRegion = Region( REGION_NULL);
        mpWindowImpl->mbWinRegion = FALSE;
        mpWindowImpl->mpFrame->ResetClipRegion();
    }
    else
    {
        if ( mpWindowImpl->mbWinRegion )
        {
            mpWindowImpl->maWinRegion = Region( REGION_NULL );
            mpWindowImpl->mbWinRegion = FALSE;
            ImplSetClipFlag();

            if ( IsReallyVisible() )
            {
                // Hintergrund-Sicherung zuruecksetzen
                if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mpSaveBackDev )
                    ImplDeleteOverlapBackground();
                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                Region      aRegion( aRect );
                ImplInvalidateParentFrameRegion( aRegion );
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::SetWindowRegionPixel( const Region& rRegion )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetWindowRegionPixel( rRegion );
    else if( mpWindowImpl->mbFrame )
    {
        if( rRegion.GetType() != REGION_NULL )
        {
            mpWindowImpl->maWinRegion = rRegion;
            mpWindowImpl->mbWinRegion = ! rRegion.IsEmpty();
            if( mpWindowImpl->mbWinRegion )
            {
                // ClipRegion setzen/updaten
                long                nX;
                long                nY;
                long                nWidth;
                long                nHeight;
                ULONG               nRectCount;
                ImplRegionInfo      aInfo;
                BOOL                bRegionRect;

                nRectCount = mpWindowImpl->maWinRegion.GetRectCount();
                mpWindowImpl->mpFrame->BeginSetClipRegion( nRectCount );
                bRegionRect = mpWindowImpl->maWinRegion.ImplGetFirstRect( aInfo, nX, nY, nWidth, nHeight );
                while ( bRegionRect )
                {
                    mpWindowImpl->mpFrame->UnionClipRegion( nX, nY, nWidth, nHeight );
                    bRegionRect = mpWindowImpl->maWinRegion.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
                }
                mpWindowImpl->mpFrame->EndSetClipRegion();
            }
            else
                SetWindowRegionPixel();
        }
        else
            SetWindowRegionPixel();
    }
    else
    {
        if ( rRegion.GetType() == REGION_NULL )
        {
            if ( mpWindowImpl->mbWinRegion )
            {
                mpWindowImpl->maWinRegion = Region( REGION_NULL );
                mpWindowImpl->mbWinRegion = FALSE;
                ImplSetClipFlag();
            }
        }
        else
        {
            mpWindowImpl->maWinRegion = rRegion;
            mpWindowImpl->mbWinRegion = TRUE;
            ImplSetClipFlag();
        }

        if ( IsReallyVisible() )
        {
            // Hintergrund-Sicherung zuruecksetzen
            if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mpSaveBackDev )
                ImplDeleteOverlapBackground();
            if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                ImplInvalidateAllOverlapBackgrounds();
            Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
            Region      aRegion( aRect );
            ImplInvalidateParentFrameRegion( aRegion );
        }
    }
}

// -----------------------------------------------------------------------

const Region& Window::GetWindowRegionPixel() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        return mpWindowImpl->mpBorderWindow->GetWindowRegionPixel();
    else
        return mpWindowImpl->maWinRegion;
}

// -----------------------------------------------------------------------

BOOL Window::IsWindowRegionPixel() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        return mpWindowImpl->mpBorderWindow->IsWindowRegionPixel();
    else
        return mpWindowImpl->mbWinRegion;
}

// -----------------------------------------------------------------------

Region Window::GetWindowClipRegionPixel( USHORT nFlags ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Region aWinClipRegion;

    if ( nFlags & WINDOW_GETCLIPREGION_NOCHILDREN )
    {
        if ( mpWindowImpl->mbInitWinClipRegion )
            ((Window*)this)->ImplInitWinClipRegion();
        aWinClipRegion = mpWindowImpl->maWinClipRegion;
    }
    else
    {
        Region* pWinChildClipRegion = ((Window*)this)->ImplGetWinChildClipRegion();
        aWinClipRegion = *pWinChildClipRegion;
        // --- RTL --- remirror clip region before passing it to somebody
        if( ImplIsAntiparallel() )
            ImplReMirror( aWinClipRegion );
    }

    if ( nFlags & WINDOW_GETCLIPREGION_NULL )
    {
        Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aWinRegion( aWinRect );

        if ( aWinRegion == aWinClipRegion )
            aWinClipRegion.SetNull();
    }

    aWinClipRegion.Move( -mnOutOffX, -mnOutOffY );

    return aWinClipRegion;
}

// -----------------------------------------------------------------------

Region Window::GetPaintRegion() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpPaintRegion )
    {
        Region aRegion = *mpWindowImpl->mpPaintRegion;
        aRegion.Move( -mnOutOffX, -mnOutOffY );
        return PixelToLogic( aRegion );
    }
    else
    {
        Region aPaintRegion( REGION_NULL );
        return aPaintRegion;
    }
}

// -----------------------------------------------------------------------

void Window::ExpandPaintClipRegion( const Region& rRegion )
{
    if( mpWindowImpl->mpPaintRegion )
    {
        Region aPixRegion = LogicToPixel( rRegion );
        Region aDevPixRegion = ImplPixelToDevicePixel( aPixRegion );

        Region aWinChildRegion = *ImplGetWinChildClipRegion();
        // --- RTL -- only this region is in frame coordinates, so re-mirror it
        if( ImplIsAntiparallel() )
            ImplReMirror( aWinChildRegion );
        aDevPixRegion.Intersect( aWinChildRegion );
        if( ! aDevPixRegion.IsEmpty() )
        {
            mpWindowImpl->mpPaintRegion->Union( aDevPixRegion );
            mbInitClipRegion = TRUE;
        }
    }
}

// -----------------------------------------------------------------------

static SystemWindow *ImplGetLastSystemWindow( Window *pWin )
{
    // get the most top-level system window, the one that contains the taskpanelist
    SystemWindow *pSysWin = NULL;
    if( !pWin )
        return pSysWin;
    Window *pMyParent = pWin;
    while ( pMyParent )
    {
        if ( pMyParent->IsSystemWindow() )
            pSysWin = (SystemWindow*)pMyParent;
        pMyParent = pMyParent->GetParent();
    }
    return pSysWin;
}

void Window::SetParent( Window* pNewParent )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    DBG_ASSERT( pNewParent, "Window::SetParent(): pParent == NULL" );
    DBG_ASSERT( pNewParent != this, "someone tried to reparent a window to itself" );

    if( pNewParent == this )
        return;

    // check if the taskpanelist would change and move the window pointer accordingly
    SystemWindow *pSysWin = ImplGetLastSystemWindow(this);
    SystemWindow *pNewSysWin = NULL;
    BOOL bChangeTaskPaneList = FALSE;
    if( pSysWin && pSysWin->ImplIsInTaskPaneList( this ) )
    {
        pNewSysWin = ImplGetLastSystemWindow( pNewParent );
        if( pNewSysWin && pNewSysWin != pSysWin )
        {
            bChangeTaskPaneList = TRUE;
            pSysWin->GetTaskPaneList()->RemoveWindow( this );
        }
    }
    // remove ownerdraw decorated windows from list in the top-most frame window
    if( (GetStyle() & WB_OWNERDRAWDECORATION) && mpWindowImpl->mbFrame )
    {
        ::std::vector< Window* >& rList = ImplGetOwnerDrawList();
        ::std::vector< Window* >::iterator p;
        p = ::std::find( rList.begin(), rList.end(), this );
        if( p != rList.end() )
            rList.erase( p );
    }

    ImplSetFrameParent( pNewParent );

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpRealParent = pNewParent;
        mpWindowImpl->mpBorderWindow->SetParent( pNewParent );
        return;
    }

    if ( mpWindowImpl->mpParent == pNewParent )
        return;

    if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetParent( pNewParent->mpWindowImpl->mpFrame );

    BOOL bVisible = IsVisible();
    Show( false, SHOW_NOFOCUSCHANGE );

    // Testen, ob sich das Overlap-Window aendert
    Window* pOldOverlapWindow;
    Window* pNewOverlapWindow = NULL;
    if ( ImplIsOverlapWindow() )
        pOldOverlapWindow = NULL;
    else
    {
        pNewOverlapWindow = pNewParent->ImplGetFirstOverlapWindow();
        if ( mpWindowImpl->mpOverlapWindow != pNewOverlapWindow )
            pOldOverlapWindow = mpWindowImpl->mpOverlapWindow;
        else
            pOldOverlapWindow = NULL;
    }

    // Fenster in der Hirachie umsetzen
    BOOL bFocusOverlapWin = HasChildPathFocus( TRUE );
    BOOL bFocusWin = HasChildPathFocus();
    BOOL bNewFrame = pNewParent->mpWindowImpl->mpFrameWindow != mpWindowImpl->mpFrameWindow;
    if ( bNewFrame )
    {
        if ( mpWindowImpl->mpFrameData->mpFocusWin )
        {
            if ( IsWindowOrChild( mpWindowImpl->mpFrameData->mpFocusWin ) )
                mpWindowImpl->mpFrameData->mpFocusWin = NULL;
        }
        if ( mpWindowImpl->mpFrameData->mpMouseMoveWin )
        {
            if ( IsWindowOrChild( mpWindowImpl->mpFrameData->mpMouseMoveWin ) )
                mpWindowImpl->mpFrameData->mpMouseMoveWin = NULL;
        }
        if ( mpWindowImpl->mpFrameData->mpMouseDownWin )
        {
            if ( IsWindowOrChild( mpWindowImpl->mpFrameData->mpMouseDownWin ) )
                mpWindowImpl->mpFrameData->mpMouseDownWin = NULL;
        }
    }
    ImplRemoveWindow( bNewFrame );
    ImplInsertWindow( pNewParent );
    if ( mpWindowImpl->mnParentClipMode & PARENTCLIPMODE_CLIP )
        pNewParent->mpWindowImpl->mbClipChildren = TRUE;
    ImplUpdateWindowPtr();
    if ( ImplUpdatePos() )
        ImplUpdateSysObjPos();

    // Wenn sich das Overlap-Window geaendert hat, dann muss getestet werden,
    // ob auch OverlapWindow die das Child-Fenster als Parent gehabt haben
    // in der Window-Hirachie umgesetzt werden muessen
    if ( ImplIsOverlapWindow() )
    {
        if ( bNewFrame )
        {
            Window* pOverlapWindow = mpWindowImpl->mpFirstOverlap;
            while ( pOverlapWindow )
            {
                Window* pNextOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
                pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
                pOverlapWindow = pNextOverlapWindow;
            }
        }
    }
    else if ( pOldOverlapWindow )
    {
        // Focus-Save zuruecksetzen
        if ( bFocusWin ||
             (pOldOverlapWindow->mpWindowImpl->mpLastFocusWindow &&
              IsWindowOrChild( pOldOverlapWindow->mpWindowImpl->mpLastFocusWindow )) )
            pOldOverlapWindow->mpWindowImpl->mpLastFocusWindow = NULL;

        Window* pOverlapWindow = pOldOverlapWindow->mpWindowImpl->mpFirstOverlap;
        while ( pOverlapWindow )
        {
            Window* pNextOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
            if ( ImplIsRealParentPath( pOverlapWindow->ImplGetWindow() ) )
                pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
            pOverlapWindow = pNextOverlapWindow;
        }

        // Activate-Status beim naechsten Overlap-Window updaten
        if ( HasChildPathFocus( TRUE ) )
            ImplCallFocusChangeActivate( pNewOverlapWindow, pOldOverlapWindow );
    }

    // Activate-Status mit umsetzen
    if ( bNewFrame )
    {
        if ( (GetType() == WINDOW_BORDERWINDOW) &&
             (ImplGetWindow()->GetType() == WINDOW_FLOATINGWINDOW) )
            ((ImplBorderWindow*)this)->SetDisplayActive( mpWindowImpl->mpFrameData->mbHasFocus );
    }

    // Focus evtl. auf den neuen Frame umsetzen, wenn FocusWindow mit
    // SetParent() umgesetzt wird
    if ( bFocusOverlapWin )
    {
        mpWindowImpl->mpFrameData->mpFocusWin = Application::GetFocusWindow();
        if ( !mpWindowImpl->mpFrameData->mbHasFocus )
        {
            mpWindowImpl->mpFrame->ToTop( 0 );
        }
    }

    // Assure DragSource and DropTarget members are created
    if ( bNewFrame )
    {
            GetDropTarget();
    }

    if( bChangeTaskPaneList )
        pNewSysWin->GetTaskPaneList()->AddWindow( this );

    if( (GetStyle() & WB_OWNERDRAWDECORATION) && mpWindowImpl->mbFrame )
        ImplGetOwnerDrawList().push_back( this );

    if ( bVisible )
        Show( true, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
}

// -----------------------------------------------------------------------

void Window::Show( BOOL bVisible, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mbVisible == bVisible )
        return;

    ImplDelData aDogTag( this );

    BOOL bRealVisibilityChanged = FALSE;
    mpWindowImpl->mbVisible = (bVisible != 0);

    if ( !bVisible )
    {
        ImplHideAllOverlaps();
        if( aDogTag.IsDelete() )
            return;

        if ( mpWindowImpl->mpBorderWindow )
        {
            bool bOldUpdate = mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate;
            if ( mpWindowImpl->mbNoParentUpdate )
                mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate = TRUE;
            mpWindowImpl->mpBorderWindow->Show( false, nFlags );
            mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate = bOldUpdate;
        }
        else if ( mpWindowImpl->mbFrame )
        {
            mpWindowImpl->mbSuppressAccessibilityEvents = TRUE;
            mpWindowImpl->mpFrame->Show( FALSE, FALSE );
        }

        StateChanged( STATE_CHANGE_VISIBLE );

        if ( mpWindowImpl->mbReallyVisible )
        {
            Region  aInvRegion( REGION_EMPTY );
            BOOL    bSaveBack = FALSE;

            if ( ImplIsOverlapWindow() && !mpWindowImpl->mbFrame )
            {
                if ( ImplRestoreOverlapBackground( aInvRegion ) )
                    bSaveBack = TRUE;
            }

            if ( !bSaveBack )
            {
                if ( mpWindowImpl->mbInitWinClipRegion )
                    ImplInitWinClipRegion();
                aInvRegion = mpWindowImpl->maWinClipRegion;
            }

            if( aDogTag.IsDelete() )
                return;

            bRealVisibilityChanged = mpWindowImpl->mbReallyVisible;
            ImplResetReallyVisible();
            ImplSetClipFlag();

            if ( ImplIsOverlapWindow() && !mpWindowImpl->mbFrame )
            {
                // Focus umsetzen
                if ( !(nFlags & SHOW_NOFOCUSCHANGE) && HasChildPathFocus() )
                {
                    if ( mpWindowImpl->mpOverlapWindow->IsEnabled() &&
                         mpWindowImpl->mpOverlapWindow->IsInputEnabled() &&
                         ! mpWindowImpl->mpOverlapWindow->IsInModalMode()
                         )
                        mpWindowImpl->mpOverlapWindow->GrabFocus();
                }
            }

            if ( !mpWindowImpl->mbFrame )
            {
                if( mpWindowImpl->mpWinData && mpWindowImpl->mpWinData->mbEnableNativeWidget )
                {
                    /*
                    * #i48371# native theming: some themes draw outside the control
                    * area we tell them to (bad thing, but we cannot do much about it ).
                    * On hiding these controls they get invalidated with their window rectangle
                    * which leads to the parts outside the control area being left and not
                    * invalidated. Workaround: invalidate an area on the parent, too
                    */
                    const int workaround_border = 5;
                    Rectangle aBounds( aInvRegion.GetBoundRect() );
                    aBounds.Left()      -= workaround_border;
                    aBounds.Top()       -= workaround_border;
                    aBounds.Right()     += workaround_border;
                    aBounds.Bottom()    += workaround_border;
                    aInvRegion = aBounds;
                }
                if ( !mpWindowImpl->mbNoParentUpdate && !(nFlags & SHOW_NOPARENTUPDATE) )
                {
                    if ( !aInvRegion.IsEmpty() )
                        ImplInvalidateParentFrameRegion( aInvRegion );
                }
                ImplGenerateMouseMove();
            }
        }
    }
    else
    {
        // inherit native widget flag for form controls
        // required here, because frames never show up in the child hierarchy - which should be fixed....
        // eg, the drop down of a combobox which is a system floating window
        if( mpWindowImpl->mbFrame && GetParent() && GetParent()->IsCompoundControl() &&
            GetParent()->IsNativeWidgetEnabled() != IsNativeWidgetEnabled() )
            EnableNativeWidget( GetParent()->IsNativeWidgetEnabled() );

        if ( mpWindowImpl->mbCallMove )
        {
            ImplCallMove();
        }
        if ( mpWindowImpl->mbCallResize )
        {
            ImplCallResize();
        }

        StateChanged( STATE_CHANGE_VISIBLE );

        Window* pTestParent;
        if ( ImplIsOverlapWindow() )
            pTestParent = mpWindowImpl->mpOverlapWindow;
        else
            pTestParent = ImplGetParent();
        if ( mpWindowImpl->mbFrame || pTestParent->mpWindowImpl->mbReallyVisible )
        {
            // Wenn ein Window gerade sichtbar wird, schicken wir allen
            // Child-Fenstern ein StateChanged, damit diese sich
            // initialisieren koennen
            ImplCallInitShow();

            // Wenn es ein SystemWindow ist, dann kommt es auch automatisch
            // nach vorne, wenn es gewuenscht ist
            if ( ImplIsOverlapWindow() && !(nFlags & SHOW_NOACTIVATE) )
            {
                ImplStartToTop(( nFlags & SHOW_FOREGROUNDTASK ) ? TOTOP_FOREGROUNDTASK : 0 );
                ImplFocusToTop( 0, FALSE );
            }

            // Hintergrund sichern
            if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mbSaveBack )
                ImplSaveOverlapBackground();
            // adjust mpWindowImpl->mbReallyVisible
            bRealVisibilityChanged = !mpWindowImpl->mbReallyVisible;
            ImplSetReallyVisible();

            // Dafuer sorgen, das Clip-Rechtecke neu berechnet werden
            ImplSetClipFlag();

            if ( !mpWindowImpl->mbFrame )
            {
                USHORT nInvalidateFlags = INVALIDATE_CHILDREN;
                if( ! IsPaintTransparent() )
                    nInvalidateFlags |= INVALIDATE_NOTRANSPARENT;
                ImplInvalidate( NULL, nInvalidateFlags );
                ImplGenerateMouseMove();
            }
        }

        if ( mpWindowImpl->mpBorderWindow )
            mpWindowImpl->mpBorderWindow->Show( true, nFlags );
        else if ( mpWindowImpl->mbFrame )
        {
            // #106431#, hide SplashScreen
            ImplSVData* pSVData = ImplGetSVData();
            if ( !pSVData->mpIntroWindow )
            {
                // The right way would be just to call this (not even in the 'if')
                GetpApp()->InitFinished();
            }
            else if ( !ImplIsWindowOrChild( pSVData->mpIntroWindow ) )
            {
                // ... but the VCL splash is broken, and it needs this
                // (for ./soffice slot:5500)
                pSVData->mpIntroWindow->Hide();
            }

            //DBG_ASSERT( !mpWindowImpl->mbSuppressAccessibilityEvents, "Window::Show() - Frame reactivated");
            mpWindowImpl->mbSuppressAccessibilityEvents = FALSE;

            mpWindowImpl->mbPaintFrame = TRUE;
            BOOL bNoActivate = (nFlags & (SHOW_NOACTIVATE|SHOW_NOFOCUSCHANGE)) ? TRUE : FALSE;
            mpWindowImpl->mpFrame->Show( TRUE, bNoActivate );
            if( aDogTag.IsDelete() )
                return;

            // Query the correct size of the window, if we are waiting for
            // a system resize
            if ( mpWindowImpl->mbWaitSystemResize )
            {
                long nOutWidth;
                long nOutHeight;
                mpWindowImpl->mpFrame->GetClientSize( nOutWidth, nOutHeight );
                ImplHandleResize( this, nOutWidth, nOutHeight );
            }
        }

        if( aDogTag.IsDelete() )
            return;

#ifdef DBG_UTIL
        if ( IsDialog() || (GetType() == WINDOW_TABPAGE) || (GetType() == WINDOW_DOCKINGWINDOW) )
        {
            DBG_DIALOGTEST( this );
        }
#endif

        ImplShowAllOverlaps();
    }

    if( aDogTag.IsDelete() )
        return;
    // invalidate all saved backgrounds
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    // the SHOW/HIDE events also serve as indicators to send child creation/destroy events to the access bridge
    // However, the access bridge only uses this event if the data member is not NULL (it's kind of a hack that
    // we re-use the SHOW/HIDE events this way, with this particular semantics).
    // Since #104887#, the notifications for the access bridge are done in Impl(Set|Reset)ReallyVisible. Here, we
    // now only notify with a NULL data pointer, for all other clients except the access bridge.
    if ( !bRealVisibilityChanged )
        ImplCallEventListeners( mpWindowImpl->mbVisible ? VCLEVENT_WINDOW_SHOW : VCLEVENT_WINDOW_HIDE, NULL );
    if( aDogTag.IsDelete() )
        return;

    // #107575#, if a floating windows is shown that grabs the focus, we have to notify the toolkit about it
    // ImplGrabFocus() is not called in this case
    // Because this might lead to problems the task will be shifted to 6.y
    // Note: top-level context menues are registered at the access bridge after being shown,
    // so this will probably not help here....
    /*
    if( mpWindowImpl->mbFloatWin && ((FloatingWindow*) this )->GrabsFocus() )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if( !mpWindowImpl->mbVisible )
        {
            ImplCallEventListeners( VCLEVENT_WINDOW_LOSEFOCUS );
            if( pSVData->maWinData.mpFocusWin )
                pSVData->maWinData.mpFocusWin->ImplCallEventListeners( VCLEVENT_WINDOW_GETFOCUS );
        }
        else
        {
            if( pSVData->maWinData.mpFocusWin )
                pSVData->maWinData.mpFocusWin->ImplCallEventListeners( VCLEVENT_WINDOW_LOSEFOCUS );
            ImplCallEventListeners( VCLEVENT_WINDOW_GETFOCUS );
        }
    }
    */
}

// -----------------------------------------------------------------------

Size Window::GetSizePixel() const
{
    // #i43257# trigger pending resize handler to assure correct window sizes
    if( mpWindowImpl->mpFrameData->maResizeTimer.IsActive() )
    {
        ImplDelData aDogtag( this );
        mpWindowImpl->mpFrameData->maResizeTimer.Stop();
        mpWindowImpl->mpFrameData->maResizeTimer.GetTimeoutHdl().Call( NULL );
        if( aDogtag.IsDelete() )
            return Size(0,0);
    }

    return Size( mnOutWidth+mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder,
                 mnOutHeight+mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder );
}

void Window::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                               sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    rLeftBorder     = mpWindowImpl->mnLeftBorder;
    rTopBorder      = mpWindowImpl->mnTopBorder;
    rRightBorder    = mpWindowImpl->mnRightBorder;
    rBottomBorder   = mpWindowImpl->mnBottomBorder;
}


// -----------------------------------------------------------------------

void Window::Enable( bool bEnable, bool bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !bEnable )
    {
        // Wenn ein Fenster disablte wird, wird automatisch der Tracking-Modus
        // beendet oder der Capture geklaut
        if ( IsTracking() )
            EndTracking( ENDTRACK_CANCEL );
        if ( IsMouseCaptured() )
            ReleaseMouse();
        // Wenn Fenster den Focus hat und in der Dialog-Steuerung enthalten,
        // wird versucht, den Focus auf das naechste Control weiterzuschalten
        // mpWindowImpl->mbDisabled darf erst nach Aufruf von ImplDlgCtrlNextWindow() gesetzt
        // werden. Ansonsten muss ImplDlgCtrlNextWindow() umgestellt werden
        if ( HasFocus() )
            ImplDlgCtrlNextWindow();
    }

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->Enable( bEnable, FALSE );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->Enable( bEnable, TRUE );
    }

    // #i56102# restore app focus win in case the
    // window was disabled when the frame focus changed
    ImplSVData* pSVData = ImplGetSVData();
    if( bEnable &&
        pSVData->maWinData.mpFocusWin == NULL &&
        mpWindowImpl->mpFrameData->mbHasFocus &&
        mpWindowImpl->mpFrameData->mpFocusWin == this )
        pSVData->maWinData.mpFocusWin = this;

    if ( mpWindowImpl->mbDisabled != !bEnable )
    {
        mpWindowImpl->mbDisabled = !bEnable;
        if ( mpWindowImpl->mpSysObj )
            mpWindowImpl->mpSysObj->Enable( bEnable && !mpWindowImpl->mbInputDisabled );
//      if ( mpWindowImpl->mbFrame )
//          mpWindowImpl->mpFrame->Enable( bEnable && !mpWindowImpl->mbInputDisabled );
        StateChanged( STATE_CHANGE_ENABLE );

        ImplCallEventListeners( bEnable ? VCLEVENT_WINDOW_ENABLED : VCLEVENT_WINDOW_DISABLED );
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->Enable( bEnable, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }

    if ( IsReallyVisible() )
        ImplGenerateMouseMove();
}

// -----------------------------------------------------------------------

void Window::SetCallHandlersOnInputDisabled( bool bCall )
{
    mpWindowImpl->mbCallHandlersDuringInputDisabled = bCall ? TRUE : FALSE;

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->SetCallHandlersOnInputDisabled( bCall );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

bool Window::IsCallHandlersOnInputDisabled() const
{
    return mpWindowImpl->mbCallHandlersDuringInputDisabled ? true : false;
}

// -----------------------------------------------------------------------

void Window::EnableInput( BOOL bEnable, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    BOOL bNotify = (bEnable != mpWindowImpl->mbInputDisabled);
    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->EnableInput( bEnable, FALSE );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->EnableInput( bEnable, TRUE );
    }

    if ( (! bEnable && mpWindowImpl->meAlwaysInputMode != AlwaysInputEnabled) ||
         (  bEnable && mpWindowImpl->meAlwaysInputMode != AlwaysInputDisabled) )
    {
        // Wenn ein Fenster disablte wird, wird automatisch der
        // Tracking-Modus beendet oder der Capture geklaut
        if ( !bEnable )
        {
            if ( IsTracking() )
                EndTracking( ENDTRACK_CANCEL );
            if ( IsMouseCaptured() )
                ReleaseMouse();
        }

        if ( mpWindowImpl->mbInputDisabled != !bEnable )
        {
            mpWindowImpl->mbInputDisabled = !bEnable;
            if ( mpWindowImpl->mpSysObj )
                mpWindowImpl->mpSysObj->Enable( !mpWindowImpl->mbDisabled && bEnable );
//          if ( mpWindowImpl->mbFrame )
//              mpWindowImpl->mpFrame->Enable( !mpWindowImpl->mbDisabled && bEnable );
        }
    }

    // #i56102# restore app focus win in case the
    // window was disabled when the frame focus changed
    ImplSVData* pSVData = ImplGetSVData();
    if( bEnable &&
        pSVData->maWinData.mpFocusWin == NULL &&
        mpWindowImpl->mpFrameData->mbHasFocus &&
        mpWindowImpl->mpFrameData->mpFocusWin == this )
        pSVData->maWinData.mpFocusWin = this;

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->EnableInput( bEnable, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }

    if ( IsReallyVisible() )
        ImplGenerateMouseMove();

    // #104827# notify parent
    if ( bNotify )
    {
        NotifyEvent aNEvt( bEnable ? EVENT_INPUTENABLE : EVENT_INPUTDISABLE, this );
        Notify( aNEvt );
    }
}

// -----------------------------------------------------------------------

void Window::EnableInput( BOOL bEnable, BOOL bChild, BOOL bSysWin,
                          const Window* pExcludeWindow )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    EnableInput( bEnable, bChild );
    if ( bSysWin )
    {
        // pExculeWindow is the first Overlap-Frame --> if this
        // shouldn't be the case, than this must be changed in dialog.cxx
        if( pExcludeWindow )
            pExcludeWindow = pExcludeWindow->ImplGetFirstOverlapWindow();
        Window* pSysWin = mpWindowImpl->mpFrameWindow->mpWindowImpl->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            // Is Window in the path from this window
            if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( pSysWin, TRUE ) )
            {
                // Is Window not in the exclude window path or not the
                // exclude window, than change the status
                if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( pSysWin, TRUE ) )
                    pSysWin->EnableInput( bEnable, bChild );
            }
            pSysWin = pSysWin->mpWindowImpl->mpNextOverlap;
        }

        // enable/disable floating system windows as well
        Window* pFrameWin = ImplGetSVData()->maWinData.mpFirstFrame;
        while ( pFrameWin )
        {
            if( pFrameWin->ImplIsFloatingWindow() )
            {
                // Is Window in the path from this window
                if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( pFrameWin, TRUE ) )
                {
                    // Is Window not in the exclude window path or not the
                    // exclude window, than change the status
                    if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( pFrameWin, TRUE ) )
                        pFrameWin->EnableInput( bEnable, bChild );
                }
            }
            pFrameWin = pFrameWin->mpWindowImpl->mpFrameData->mpNextFrame;
        }

        // the same for ownerdraw floating windows
        if( mpWindowImpl->mbFrame )
        {
            ::std::vector< Window* >& rList = mpWindowImpl->mpFrameData->maOwnerDrawList;
            ::std::vector< Window* >::iterator p = rList.begin();
            while( p != rList.end() )
            {
                // Is Window in the path from this window
                if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( (*p), TRUE ) )
                {
                    // Is Window not in the exclude window path or not the
                    // exclude window, than change the status
                    if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( (*p), TRUE ) )
                        (*p)->EnableInput( bEnable, bChild );
                }
                p++;
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::AlwaysEnableInput( BOOL bAlways, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->AlwaysEnableInput( bAlways, FALSE );

    if( bAlways && mpWindowImpl->meAlwaysInputMode != AlwaysInputEnabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputEnabled;

        if ( bAlways )
            EnableInput( TRUE, FALSE );
    }
    else if( ! bAlways && mpWindowImpl->meAlwaysInputMode == AlwaysInputEnabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputNone;
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->AlwaysEnableInput( bAlways, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::AlwaysDisableInput( BOOL bAlways, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->AlwaysDisableInput( bAlways, FALSE );

    if( bAlways && mpWindowImpl->meAlwaysInputMode != AlwaysInputDisabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputDisabled;

        if ( bAlways )
            EnableInput( FALSE, FALSE );
    }
    else if( ! bAlways && mpWindowImpl->meAlwaysInputMode == AlwaysInputDisabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputNone;
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->AlwaysDisableInput( bAlways, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::SetActivateMode( USHORT nMode )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetActivateMode( nMode );

    if ( mpWindowImpl->mnActivateMode != nMode )
    {
        mpWindowImpl->mnActivateMode = nMode;

        // Evtl. ein Decativate/Activate ausloesen
        if ( mpWindowImpl->mnActivateMode )
        {
            if ( (mpWindowImpl->mbActive || (GetType() == WINDOW_BORDERWINDOW)) &&
                 !HasChildPathFocus( TRUE ) )
            {
                mpWindowImpl->mbActive = FALSE;
                Deactivate();
            }
        }
        else
        {
            if ( !mpWindowImpl->mbActive || (GetType() == WINDOW_BORDERWINDOW) )
            {
                mpWindowImpl->mbActive = TRUE;
                Activate();
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::ToTop( USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplStartToTop( nFlags );
    ImplFocusToTop( nFlags, IsReallyVisible() );
}

// -----------------------------------------------------------------------

void Window::SetZOrder( Window* pRefWindow, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->SetZOrder( pRefWindow, nFlags );
        return;
    }

    if ( nFlags & WINDOW_ZORDER_FIRST )
    {
        if ( ImplIsOverlapWindow() )
            pRefWindow = mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
        else
            pRefWindow = mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild;
        nFlags |= WINDOW_ZORDER_BEFOR;
    }
    else if ( nFlags & WINDOW_ZORDER_LAST )
    {
        if ( ImplIsOverlapWindow() )
            pRefWindow = mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap;
        else
            pRefWindow = mpWindowImpl->mpParent->mpWindowImpl->mpLastChild;
        nFlags |= WINDOW_ZORDER_BEHIND;
    }

    while ( pRefWindow->mpWindowImpl->mpBorderWindow )
        pRefWindow = pRefWindow->mpWindowImpl->mpBorderWindow;
    if ( (pRefWindow == this) || mpWindowImpl->mbFrame )
        return;

    DBG_ASSERT( pRefWindow->mpWindowImpl->mpParent == mpWindowImpl->mpParent, "Window::SetZOrder() - pRefWindow has other parent" );
    if ( nFlags & WINDOW_ZORDER_BEFOR )
    {
        if ( pRefWindow->mpWindowImpl->mpPrev == this )
            return;

        if ( ImplIsOverlapWindow() )
        {
            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = mpWindowImpl->mpPrev;
            if ( !pRefWindow->mpWindowImpl->mpPrev )
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap = this;
        }
        else
        {
            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            else
                mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = mpWindowImpl->mpPrev;
            if ( !pRefWindow->mpWindowImpl->mpPrev )
                mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = this;
        }

        mpWindowImpl->mpPrev = pRefWindow->mpWindowImpl->mpPrev;
        mpWindowImpl->mpNext = pRefWindow;
        if ( mpWindowImpl->mpPrev )
            mpWindowImpl->mpPrev->mpWindowImpl->mpNext = this;
        mpWindowImpl->mpNext->mpWindowImpl->mpPrev = this;
    }
    else if ( nFlags & WINDOW_ZORDER_BEHIND )
    {
        if ( pRefWindow->mpWindowImpl->mpNext == this )
            return;

        if ( ImplIsOverlapWindow() )
        {
            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = mpWindowImpl->mpPrev;
            if ( !pRefWindow->mpWindowImpl->mpNext )
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = this;
        }
        else
        {
            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            else
                mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = mpWindowImpl->mpPrev;
            if ( !pRefWindow->mpWindowImpl->mpNext )
                mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = this;
        }

        mpWindowImpl->mpPrev = pRefWindow;
        mpWindowImpl->mpNext = pRefWindow->mpWindowImpl->mpNext;
        if ( mpWindowImpl->mpNext )
            mpWindowImpl->mpNext->mpWindowImpl->mpPrev = this;
        mpWindowImpl->mpPrev->mpWindowImpl->mpNext = this;
    }

    if ( IsReallyVisible() )
    {
        // Hintergrund-Sicherung zuruecksetzen
        if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
            ImplInvalidateAllOverlapBackgrounds();

        if ( mpWindowImpl->mbInitWinClipRegion || !mpWindowImpl->maWinClipRegion.IsEmpty() )
        {
            BOOL bInitWinClipRegion = mpWindowImpl->mbInitWinClipRegion;
            ImplSetClipFlag();

            // Wenn ClipRegion noch nicht initalisiert wurde, dann
            // gehen wir davon aus, das das Fenster noch nicht
            // ausgegeben wurde und loesen somit auch keine
            // Invalidates aus. Dies ist eine Optimierung fuer
            // HTML-Dokumenten mit vielen Controls. Wenn es mal
            // Probleme mit dieser Abfrage gibt, sollte man ein
            // Flag einfuehren, ob das Fenster nach Show schon
            // einmal ausgegeben wurde.
            if ( !bInitWinClipRegion )
            {
                // Alle nebeneinanderliegen Fenster invalidieren
                // Noch nicht komplett implementiert !!!
                Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                Window*     pWindow = NULL;
                if ( ImplIsOverlapWindow() )
                {
                    if ( mpWindowImpl->mpOverlapWindow )
                        pWindow = mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
                }
                else
                    pWindow = ImplGetParent()->mpWindowImpl->mpFirstChild;
                // Alle Fenster, die vor uns liegen und von uns verdeckt wurden,
                // invalidieren
                while ( pWindow )
                {
                    if ( pWindow == this )
                        break;
                    Rectangle aCompRect( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                                         Size( pWindow->mnOutWidth, pWindow->mnOutHeight ) );
                    if ( aWinRect.IsOver( aCompRect ) )
                        pWindow->Invalidate( INVALIDATE_CHILDREN | INVALIDATE_NOTRANSPARENT );
                    pWindow = pWindow->mpWindowImpl->mpNext;
                }
                // Wenn uns ein Fenster welches im Hinterund liegt verdeckt hat,
                // dann muessen wir uns neu ausgeben
                while ( pWindow )
                {
                    if ( pWindow != this )
                    {
                        Rectangle aCompRect( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                                             Size( pWindow->mnOutWidth, pWindow->mnOutHeight ) );
                        if ( aWinRect.IsOver( aCompRect ) )
                        {
                            Invalidate( INVALIDATE_CHILDREN | INVALIDATE_NOTRANSPARENT );
                            break;
                        }
                    }
                    pWindow = pWindow->mpWindowImpl->mpNext;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::EnableAlwaysOnTop( BOOL bEnable )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mpWindowImpl->mbAlwaysOnTop = bEnable;

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->EnableAlwaysOnTop( bEnable );
    else if ( bEnable && IsReallyVisible() )
        ToTop();

    if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetAlwaysOnTop( bEnable );
}

// -----------------------------------------------------------------------

void Window::SetPosSizePixel( long nX, long nY,
                              long nWidth, long nHeight, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    BOOL bHasValidSize = !mpWindowImpl->mbDefSize;

    if ( nFlags & WINDOW_POSSIZE_POS )
        mpWindowImpl->mbDefPos = FALSE;
    if ( nFlags & WINDOW_POSSIZE_SIZE )
        mpWindowImpl->mbDefSize = FALSE;

    // Oberstes BorderWindow ist das Window, welches positioniert werden soll
    Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;

    if ( pWindow->mpWindowImpl->mbFrame )
    {
        // Note: if we're positioning a frame, the coordinates are interpreted
        // as being the top-left corner of the window's client area and NOT
        // as the position of the border ! (due to limitations of several UNIX window managers)
        long nOldWidth  = pWindow->mnOutWidth;

        if ( !(nFlags & WINDOW_POSSIZE_WIDTH) )
            nWidth = pWindow->mnOutWidth;
        if ( !(nFlags & WINDOW_POSSIZE_HEIGHT) )
            nHeight = pWindow->mnOutHeight;


        USHORT nSysFlags=0;
        if( nFlags & WINDOW_POSSIZE_WIDTH )
            nSysFlags |= SAL_FRAME_POSSIZE_WIDTH;
        if( nFlags & WINDOW_POSSIZE_HEIGHT )
            nSysFlags |= SAL_FRAME_POSSIZE_HEIGHT;
        if( nFlags & WINDOW_POSSIZE_X )
        {
            nSysFlags |= SAL_FRAME_POSSIZE_X;
            if( pWindow->GetParent() && (pWindow->GetStyle() & WB_SYSTEMCHILDWINDOW) )
            {
                Window* pParent = pWindow->GetParent();
                nX += pParent->mnOutOffX;
            }
            if( GetParent() && GetParent()->ImplIsAntiparallel() )
            {
                // --- RTL --- (re-mirror at parent window)
                Rectangle aRect( Point ( nX, nY ), Size( nWidth, nHeight ) );
                GetParent()->ImplReMirror( aRect );
                nX = aRect.nLeft;
            }
        }
        if( !(nFlags & WINDOW_POSSIZE_X) && bHasValidSize && pWindow->mpWindowImpl->mpFrame->maGeometry.nWidth )
        {
            // --- RTL ---  make sure the old right aligned position is not changed
            //              system windows will always grow to the right
            if( pWindow->GetParent() && pWindow->GetParent()->ImplHasMirroredGraphics() )
            {
                long myWidth = nOldWidth;
                if( !myWidth )
                    myWidth = mpWindowImpl->mpFrame->GetUnmirroredGeometry().nWidth;
                if( !myWidth )
                    myWidth = nWidth;
                nFlags |= WINDOW_POSSIZE_X;
                nSysFlags |= SAL_FRAME_POSSIZE_X;
                nX = mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX - pWindow->GetParent()->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX -
                    mpWindowImpl->mpFrame->GetUnmirroredGeometry().nLeftDecoration;
                nX = pWindow->GetParent()->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX - mpWindowImpl->mpFrame->GetUnmirroredGeometry().nLeftDecoration +
                    pWindow->GetParent()->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nWidth - myWidth - 1 - mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX;
                if(!(nFlags & WINDOW_POSSIZE_Y))
                {
                    nFlags |= WINDOW_POSSIZE_Y;
                    nSysFlags |= SAL_FRAME_POSSIZE_Y;
                    nY = mpWindowImpl->mpFrame->GetUnmirroredGeometry().nY - pWindow->GetParent()->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nY -
                        mpWindowImpl->mpFrame->GetUnmirroredGeometry().nTopDecoration;
                }
            }
        }
        if( nFlags & WINDOW_POSSIZE_Y )
        {
            nSysFlags |= SAL_FRAME_POSSIZE_Y;
            if( pWindow->GetParent() && (pWindow->GetStyle() & WB_SYSTEMCHILDWINDOW) )
            {
                Window* pParent = pWindow->GetParent();
                nY += pParent->mnOutOffY;
            }
        }

        if( nSysFlags & (SAL_FRAME_POSSIZE_WIDTH|SAL_FRAME_POSSIZE_HEIGHT) )
        {
            // check for min/max client size and adjust size accordingly
            // otherwise it may happen that the resize event is ignored, i.e. the old size remains
            // unchanged but ImplHandleResize() is called with the wrong size
            SystemWindow *pSystemWindow = dynamic_cast< SystemWindow* >( pWindow );
            if( pSystemWindow )
            {
                Size aMinSize = pSystemWindow->GetMinOutputSizePixel();
                Size aMaxSize = pSystemWindow->GetMaxOutputSizePixel();
                if( nWidth < aMinSize.Width() )
                    nWidth = aMinSize.Width();
                if( nHeight < aMinSize.Height() )
                    nHeight = aMinSize.Height();

                if( nWidth > aMaxSize.Width() )
                    nWidth = aMaxSize.Width();
                if( nHeight > aMaxSize.Height() )
                    nHeight = aMaxSize.Height();
            }
        }

        pWindow->mpWindowImpl->mpFrame->SetPosSize( nX, nY, nWidth, nHeight, nSysFlags );

        // Resize should be called directly. If we havn't
        // set the correct size, we get a second resize from
        // the system with the correct size. This can be happend
        // if the size is to small or to large.
        ImplHandleResize( pWindow, nWidth, nHeight );
    }
    else
    {
        pWindow->ImplPosSizeWindow( nX, nY, nWidth, nHeight, nFlags );
        if ( IsReallyVisible() )
            ImplGenerateMouseMove();
    }
}

// -----------------------------------------------------------------------

Point Window::GetPosPixel() const
{
    return mpWindowImpl->maPos;
}

// -----------------------------------------------------------------------

Rectangle Window::GetDesktopRectPixel() const
{
    Rectangle rRect;
    mpWindowImpl->mpFrameWindow->mpWindowImpl->mpFrame->GetWorkArea( rRect );
    return rRect;
}

// -----------------------------------------------------------------------

Point Window::OutputToScreenPixel( const Point& rPos ) const
{
    // relative to top level parent
    return Point( rPos.X()+mnOutOffX, rPos.Y()+mnOutOffY );
}

// -----------------------------------------------------------------------

Point Window::ScreenToOutputPixel( const Point& rPos ) const
{
    // relative to top level parent
    return Point( rPos.X()-mnOutOffX, rPos.Y()-mnOutOffY );
}

// -----------------------------------------------------------------------

long Window::ImplGetUnmirroredOutOffX()
{
    // revert mnOutOffX changes that were potentially made in ImplPosSizeWindow
    long offx = mnOutOffX;
    if( ImplHasMirroredGraphics() )
    {
        if( mpWindowImpl->mpParent && !mpWindowImpl->mpParent->mpWindowImpl->mbFrame && mpWindowImpl->mpParent->ImplIsAntiparallel() )
        {
            if ( !ImplIsOverlapWindow() )
                offx -= mpWindowImpl->mpParent->mnOutOffX;

            offx = mpWindowImpl->mpParent->mnOutWidth - mnOutWidth - offx;

            if ( !ImplIsOverlapWindow() )
                offx += mpWindowImpl->mpParent->mnOutOffX;

        }
    }
    return offx;
}

// normalized screen pixel are independent of mirroring
Point Window::OutputToNormalizedScreenPixel( const Point& rPos ) const
{
    // relative to top level parent
    long offx = ((Window*) this)->ImplGetUnmirroredOutOffX();
    return Point( rPos.X()+offx, rPos.Y()+mnOutOffY );
}

Point Window::NormalizedScreenToOutputPixel( const Point& rPos ) const
{
    // relative to top level parent
    long offx = ((Window*) this)->ImplGetUnmirroredOutOffX();
    return Point( rPos.X()-offx, rPos.Y()-mnOutOffY );
}

// -----------------------------------------------------------------------

Point Window::OutputToAbsoluteScreenPixel( const Point& rPos ) const
{
    // relative to the screen
    Point p = OutputToScreenPixel( rPos );
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
    p.X() += g.nX;
    p.Y() += g.nY;
    return p;
}

// -----------------------------------------------------------------------

Point Window::AbsoluteScreenToOutputPixel( const Point& rPos ) const
{
    // relative to the screen
    Point p = ScreenToOutputPixel( rPos );
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
    p.X() -= g.nX;
    p.Y() -= g.nY;
    return p;
}

// -----------------------------------------------------------------------

Rectangle Window::ImplOutputToUnmirroredAbsoluteScreenPixel( const Rectangle &rRect ) const
{
    // this method creates unmirrored screen coordinates to be compared with the desktop
    // and is used for positioning of RTL popup windows correctly on the screen
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetUnmirroredGeometry();

    Point p1 = OutputToScreenPixel( rRect.TopRight() );
    p1.X() = g.nX+g.nWidth-p1.X();
    p1.Y() += g.nY;

    Point p2 = OutputToScreenPixel( rRect.BottomLeft() );
    p2.X() = g.nX+g.nWidth-p2.X();
    p2.Y() += g.nY;

    return Rectangle( p1, p2 );
}


// -----------------------------------------------------------------------

Rectangle Window::GetWindowExtentsRelative( Window *pRelativeWindow ) const
{
    // with decoration
    return ImplGetWindowExtentsRelative( pRelativeWindow, FALSE );
}

Rectangle Window::GetClientWindowExtentsRelative( Window *pRelativeWindow ) const
{
    // without decoration
    return ImplGetWindowExtentsRelative( pRelativeWindow, TRUE );
}

// -----------------------------------------------------------------------

Rectangle Window::ImplGetWindowExtentsRelative( Window *pRelativeWindow, BOOL bClientOnly ) const
{
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
    // make sure we use the extent of our border window,
    // otherwise we miss a few pixels
    const Window *pWin = (!bClientOnly && mpWindowImpl->mpBorderWindow) ? mpWindowImpl->mpBorderWindow : this;

    Point aPos( pWin->OutputToScreenPixel( Point(0,0) ) );
    aPos.X() += g.nX;
    aPos.Y() += g.nY;
    Size aSize ( pWin->GetSizePixel() );
    // #104088# do not add decoration to the workwindow to be compatible to java accessibility api
    if( !bClientOnly && (mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame && GetType() != WINDOW_WORKWINDOW)) )
    {
        aPos.X() -= g.nLeftDecoration;
        aPos.Y() -= g.nTopDecoration;
        aSize.Width() += g.nLeftDecoration + g.nRightDecoration;
        aSize.Height() += g.nTopDecoration + g.nBottomDecoration;
    }
    if( pRelativeWindow )
    {
        // #106399# express coordinates relative to borderwindow
        Window *pRelWin = (!bClientOnly && pRelativeWindow->mpWindowImpl->mpBorderWindow) ? pRelativeWindow->mpWindowImpl->mpBorderWindow : pRelativeWindow;
        aPos = pRelWin->AbsoluteScreenToOutputPixel( aPos );
    }
    return Rectangle( aPos, aSize );
}

// -----------------------------------------------------------------------

void Window::Scroll( long nHorzScroll, long nVertScroll, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplScroll( Rectangle( Point( mnOutOffX, mnOutOffY ),
                           Size( mnOutWidth, mnOutHeight ) ),
                nHorzScroll, nVertScroll, nFlags & ~SCROLL_CLIP );
}

// -----------------------------------------------------------------------

void Window::Scroll( long nHorzScroll, long nVertScroll,
                     const Rectangle& rRect, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Rectangle aRect = ImplLogicToDevicePixel( rRect );
    aRect.Intersection( Rectangle( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) ) );
    if ( !aRect.IsEmpty() )
        ImplScroll( aRect, nHorzScroll, nVertScroll, nFlags );
}

// -----------------------------------------------------------------------

void Window::Invalidate( USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    ImplInvalidate( NULL, nFlags );
}

// -----------------------------------------------------------------------

void Window::Invalidate( const Rectangle& rRect, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    Rectangle aRect = ImplLogicToDevicePixel( rRect );
    if ( !aRect.IsEmpty() )
    {
        Region aRegion( aRect );
        ImplInvalidate( &aRegion, nFlags );
    }
}

// -----------------------------------------------------------------------

void Window::Invalidate( const Region& rRegion, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    if ( rRegion.IsNull() )
        ImplInvalidate( NULL, nFlags );
    else
    {
        Region aRegion = ImplPixelToDevicePixel( LogicToPixel( rRegion ) );
        if ( !aRegion.IsEmpty() )
            ImplInvalidate( &aRegion, nFlags );
    }
}

// -----------------------------------------------------------------------

void Window::Validate( USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    ImplValidate( NULL, nFlags );
}

// -----------------------------------------------------------------------

void Window::Validate( const Rectangle& rRect, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    Rectangle aRect = ImplLogicToDevicePixel( rRect );
    if ( !aRect.IsEmpty() )
    {
        Region aRegion( aRect );
        ImplValidate( &aRegion, nFlags );
    }
}

// -----------------------------------------------------------------------

void Window::Validate( const Region& rRegion, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    if ( rRegion.IsNull() )
        ImplValidate( NULL, nFlags );
    else
    {
        Region aRegion = ImplPixelToDevicePixel( LogicToPixel( rRegion ) );
        if ( !aRegion.IsEmpty() )
            ImplValidate( &aRegion, nFlags );
    }
}

// -----------------------------------------------------------------------

BOOL Window::HasPaintEvent() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !mpWindowImpl->mbReallyVisible )
        return FALSE;

    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
        return TRUE;

    if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINT )
        return TRUE;

    if ( !ImplIsOverlapWindow() )
    {
        const Window* pTempWindow = this;
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINTCHILDS | IMPL_PAINT_PAINTALLCHILDS) )
                return TRUE;
        }
        while ( !pTempWindow->ImplIsOverlapWindow() );
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void Window::Update()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->Update();
        return;
    }

    if ( !mpWindowImpl->mbReallyVisible )
        return;

    BOOL bFlush = FALSE;
    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) )
            bFlush = TRUE;
    }

    // Zuerst muessen wir alle Fenster ueberspringen, die Paint-Transparent
    // sind
    Window* pUpdateWindow = this;
    Window* pWindow = pUpdateWindow;
    while ( !pWindow->ImplIsOverlapWindow() )
    {
        if ( !pWindow->mpWindowImpl->mbPaintTransparent )
        {
            pUpdateWindow = pWindow;
            break;
        }
        pWindow = pWindow->ImplGetParent();
    }
    // Ein Update wirkt immer auf das Window, wo PAINTALLCHILDS gesetzt
    // ist, damit nicht zuviel gemalt wird
    pWindow = pUpdateWindow;
    do
    {
        if ( pWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDS )
            pUpdateWindow = pWindow;
        if ( pWindow->ImplIsOverlapWindow() )
            break;
        pWindow = pWindow->ImplGetParent();
    }
    while ( pWindow );

    // Wenn es etwas zu malen gibt, dann ein Paint ausloesen
    if ( pUpdateWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDS) )
    {
         // und fuer alle ueber uns stehende System-Fenster auch ein Update
         // ausloesen, damit nicht die ganze Zeit luecken stehen bleiben
         Window* pUpdateOverlapWindow = ImplGetFirstOverlapWindow()->mpWindowImpl->mpFirstOverlap;
         while ( pUpdateOverlapWindow )
         {
             pUpdateOverlapWindow->Update();
             pUpdateOverlapWindow = pUpdateOverlapWindow->mpWindowImpl->mpNext;
         }

        pUpdateWindow->ImplCallPaint( NULL, pUpdateWindow->mpWindowImpl->mnPaintFlags );
    }

    if ( bFlush )
        Flush();
}

// -----------------------------------------------------------------------

void Window::Flush()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    const Rectangle aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
    mpWindowImpl->mpFrame->Flush( aWinRect );
}

// -----------------------------------------------------------------------

void Window::Sync()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mpWindowImpl->mpFrame->Sync();
}

// -----------------------------------------------------------------------

void Window::SetUpdateMode( BOOL bUpdate )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mpWindowImpl->mbNoUpdate = !bUpdate;
    StateChanged( STATE_CHANGE_UPDATEMODE );
}

// -----------------------------------------------------------------------

void Window::GrabFocus()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplGrabFocus( 0 );
}

// -----------------------------------------------------------------------

BOOL Window::HasFocus() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    // #107575# the first floating window always has the keyboard focus, see also winproc.cxx: ImplGetKeyInputWindow()
    //  task was shifted to 6.y, so its commented out
    /*
    Window* pFocusWin = ImplGetSVData()->maWinData.mpFirstFloat;
    if( pFocusWin && pFocusWin->mpWindowImpl->mbFloatWin && ((FloatingWindow *)pFocusWin)->GrabsFocus() )
        pFocusWin = pFocusWin->GetPreferredKeyInputWindow();
    else
        pFocusWin = ImplGetSVData()->maWinData.mpFocusWin;

    return (this == pFocusWin);
    */

    return (this == ImplGetSVData()->maWinData.mpFocusWin);
}

// -----------------------------------------------------------------------

void Window::GrabFocusToDocument()
{
    Window *pWin = this;
    while( pWin )
    {
        if( !pWin->GetParent() )
        {
            pWin->ImplGetFrameWindow()->GetWindow( WINDOW_CLIENT )->GrabFocus();
            return;
        }
        pWin = pWin->GetParent();
    }
}

void Window::SetFakeFocus( bool bFocus )
{
    ImplGetWindowImpl()->mbFakeFocusSet = bFocus;
}

// -----------------------------------------------------------------------

BOOL Window::HasChildPathFocus( BOOL bSystemWindow ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    // #107575#, the first floating window always has the keyboard focus, see also winproc.cxx: ImplGetKeyInputWindow()
    //  task was shifted to 6.y, so its commented out
    /*
    Window* pFocusWin = ImplGetSVData()->maWinData.mpFirstFloat;
    if( pFocusWin && pFocusWin->mpWindowImpl->mbFloatWin && ((FloatingWindow *)pFocusWin)->GrabsFocus() )
        pFocusWin = pFocusWin->GetPreferredKeyInputWindow();
    else
        pFocusWin = ImplGetSVData()->maWinData.mpFocusWin;
    */
    Window* pFocusWin = ImplGetSVData()->maWinData.mpFocusWin;
    if ( pFocusWin )
        return ImplIsWindowOrChild( pFocusWin, bSystemWindow );
    return FALSE;
}

// -----------------------------------------------------------------------

void Window::CaptureMouse()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVData* pSVData = ImplGetSVData();

    // Tracking evt. beenden
    if ( pSVData->maWinData.mpTrackWin != this )
    {
        if ( pSVData->maWinData.mpTrackWin )
            pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    }

    if ( pSVData->maWinData.mpCaptureWin != this )
    {
        pSVData->maWinData.mpCaptureWin = this;
        mpWindowImpl->mpFrame->CaptureMouse( TRUE );
    }
}

// -----------------------------------------------------------------------

void Window::ReleaseMouse()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVData* pSVData = ImplGetSVData();

    DBG_ASSERTWARNING( pSVData->maWinData.mpCaptureWin == this,
                       "Window::ReleaseMouse(): window doesn't have the mouse capture" );

    if ( pSVData->maWinData.mpCaptureWin == this )
    {
        pSVData->maWinData.mpCaptureWin = NULL;
        mpWindowImpl->mpFrame->CaptureMouse( FALSE );
        ImplGenerateMouseMove();
    }
}

// -----------------------------------------------------------------------

BOOL Window::IsMouseCaptured() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    return (this == ImplGetSVData()->maWinData.mpCaptureWin);
}

// -----------------------------------------------------------------------

void Window::SetPointer( const Pointer& rPointer )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->maPointer == rPointer )
        return;

    mpWindowImpl->maPointer   = rPointer;

    // Pointer evt. direkt umsetzen
    if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
}

// -----------------------------------------------------------------------

void Window::EnableChildPointerOverwrite( BOOL bOverwrite )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mbChildPtrOverwrite == bOverwrite )
        return;

    mpWindowImpl->mbChildPtrOverwrite  = bOverwrite;

    // Pointer evt. direkt umsetzen
    if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
}

// -----------------------------------------------------------------------

void Window::SetPointerPosPixel( const Point& rPos )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Point aPos = ImplOutputToFrame( rPos );
    if( ImplHasMirroredGraphics() )
    {
        if( !IsRTLEnabled() )
        {
            // --- RTL --- (re-mirror mouse pos at this window)
            ImplReMirror( aPos );
        }
        // mirroring is required here, SetPointerPos bypasses SalGraphics
        mpGraphics->mirror( aPos.X(), this );
    }
    else if( ImplIsAntiparallel() )
    {
        ImplReMirror( aPos );
    }
    mpWindowImpl->mpFrame->SetPointerPos( aPos.X(), aPos.Y() );
}

// -----------------------------------------------------------------------

Point Window::GetPointerPosPixel()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Point aPos( mpWindowImpl->mpFrameData->mnLastMouseX, mpWindowImpl->mpFrameData->mnLastMouseY );
    if( ImplIsAntiparallel() )
    {
        // --- RTL --- (re-mirror mouse pos at this window)
        ImplReMirror( aPos );
    }
    return ImplFrameToOutput( aPos );
}

// -----------------------------------------------------------------------

Point Window::GetLastPointerPosPixel()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Point aPos( mpWindowImpl->mpFrameData->mnBeforeLastMouseX, mpWindowImpl->mpFrameData->mnBeforeLastMouseY );
    if( ImplIsAntiparallel() )
    {
        // --- RTL --- (re-mirror mouse pos at this window)
        ImplReMirror( aPos );
    }
    return ImplFrameToOutput( aPos );
}

// -----------------------------------------------------------------------

void Window::ShowPointer( BOOL bVisible )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mbNoPtrVisible != !bVisible )
    {
        mpWindowImpl->mbNoPtrVisible = !bVisible;

        // Pointer evt. direkt umsetzen
        if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
            mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
    }
}

// -----------------------------------------------------------------------

Window::PointerState Window::GetPointerState()
{
    PointerState aState;
    aState.mnState = 0;

    if (mpWindowImpl->mpFrame)
    {
        SalFrame::SalPointerState aSalPointerState;

        aSalPointerState = mpWindowImpl->mpFrame->GetPointerState();
        if( ImplIsAntiparallel() )
        {
            // --- RTL --- (re-mirror mouse pos at this window)
            ImplReMirror( aSalPointerState.maPos );
        }
        aState.maPos = ImplFrameToOutput( aSalPointerState.maPos );
        aState.mnState = aSalPointerState.mnState;
    }
    return aState;
}

// -----------------------------------------------------------------------

BOOL Window::IsMouseOver()
{
    return ImplGetWinData()->mbMouseOver;
}

// -----------------------------------------------------------------------

void Window::EnterWait()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mpWindowImpl->mnWaitCount++;

    if ( mpWindowImpl->mnWaitCount == 1 )
    {
        // Pointer evt. direkt umsetzen
        if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
            mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
    }
}

// -----------------------------------------------------------------------

void Window::LeaveWait()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mnWaitCount )
    {
        mpWindowImpl->mnWaitCount--;

        if ( !mpWindowImpl->mnWaitCount )
        {
            // Pointer evt. direkt umsetzen
            if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
                mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
        }
    }
}

// -----------------------------------------------------------------------

void Window::SetCursor( Cursor* pCursor )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpCursor != pCursor )
    {
        if ( mpWindowImpl->mpCursor )
            mpWindowImpl->mpCursor->ImplHide();
        mpWindowImpl->mpCursor = pCursor;
        if ( pCursor )
            pCursor->ImplShow();
    }
}

// -----------------------------------------------------------------------

void Window::SetText( const XubString& rStr )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    String oldTitle( mpWindowImpl->maText );
    mpWindowImpl->maText = rStr;

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetText( rStr );
    else if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetTitle( rStr );

    ImplCallEventListeners( VCLEVENT_WINDOW_FRAMETITLECHANGED, &oldTitle );

    // #107247# needed for accessibility
    // The VCLEVENT_WINDOW_FRAMETITLECHANGED is (mis)used to notify accessible name changes.
    // Therefore a window, which is labeled by this window, must also notify an accessible
    // name change.
    if ( IsReallyVisible() )
    {
        Window* pWindow = GetLabelFor();
        if ( pWindow && pWindow != this )
            pWindow->ImplCallEventListeners( VCLEVENT_WINDOW_FRAMETITLECHANGED, &oldTitle );
    }

    StateChanged( STATE_CHANGE_TEXT );
}

// -----------------------------------------------------------------------

String Window::GetText() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    return mpWindowImpl->maText;
}

// -----------------------------------------------------------------------

String Window::GetDisplayText() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    return GetText();
}

// -----------------------------------------------------------------------

const Wallpaper& Window::GetDisplayBackground() const
{
    // FIXME: fix issue 52349, need to fix this really in
    // all NWF enabled controls
    const ToolBox* pTB = dynamic_cast<const ToolBox*>(this);
    if( pTB )
    {
        if( IsNativeWidgetEnabled() )
            return pTB->ImplGetToolBoxPrivateData()->maDisplayBackground;
    }

    if( !IsBackground() )
    {
        if( mpWindowImpl->mpParent )
            return mpWindowImpl->mpParent->GetDisplayBackground();
    }

    const Wallpaper& rBack = GetBackground();
    if( ! rBack.IsBitmap() &&
        ! rBack.IsGradient() &&
        rBack.GetColor().GetColor() == COL_TRANSPARENT &&
        mpWindowImpl->mpParent )
            return mpWindowImpl->mpParent->GetDisplayBackground();
    return rBack;
}

// -----------------------------------------------------------------------

const XubString& Window::GetHelpText() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    SmartId aSmartId = GetSmartHelpId();

    ULONG nNumHelpId = 0;
    String aStrHelpId;
    if( aSmartId.HasString() )
        aStrHelpId = aSmartId.GetStr();
    if( aSmartId.HasNumeric() )
        nNumHelpId = aSmartId.GetNum();
    bool bStrHelpId = (aStrHelpId.Len() > 0);

    if ( !mpWindowImpl->maHelpText.Len() && (nNumHelpId || bStrHelpId) )
    {
        if ( !IsDialog() && (mpWindowImpl->mnType != WINDOW_TABPAGE) && (mpWindowImpl->mnType != WINDOW_FLOATINGWINDOW) )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                if( bStrHelpId )
                    ((Window*)this)->mpWindowImpl->maHelpText = pHelp->GetHelpText( aStrHelpId, this );
                else
                    ((Window*)this)->mpWindowImpl->maHelpText = pHelp->GetHelpText( nNumHelpId, this );
                mpWindowImpl->mbHelpTextDynamic = FALSE;
            }
        }
    }
    else if( mpWindowImpl->mbHelpTextDynamic && (nNumHelpId || bStrHelpId) )
    {
        static const char* pEnv = getenv( "HELP_DEBUG" );
        if( pEnv && *pEnv )
        {
            rtl::OUStringBuffer aTxt( 64+mpWindowImpl->maHelpText.Len() );
            aTxt.append( mpWindowImpl->maHelpText );
            aTxt.appendAscii( "\n------------------\n" );
            if( bStrHelpId )
                aTxt.append( rtl::OUString( aStrHelpId ) );
            else
                aTxt.append( sal_Int32( nNumHelpId ) );
            mpWindowImpl->maHelpText = aTxt.makeStringAndClear();
        }
        mpWindowImpl->mbHelpTextDynamic = FALSE;
    }

    return mpWindowImpl->maHelpText;
}

// -----------------------------------------------------------------------

Window* Window::FindWindow( const Point& rPos ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Point aPos = OutputToScreenPixel( rPos );
    return ((Window*)this)->ImplFindWindow( aPos );
}

// -----------------------------------------------------------------------

USHORT Window::GetChildCount() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    USHORT  nChildCount = 0;
    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        nChildCount++;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    return nChildCount;
}

// -----------------------------------------------------------------------

Window* Window::GetChild( USHORT nChild ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    USHORT  nChildCount = 0;
    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        if ( nChild == nChildCount )
            return pChild;
        pChild = pChild->mpWindowImpl->mpNext;
        nChildCount++;
    }

    return NULL;
}

// -----------------------------------------------------------------------

Window* Window::GetWindow( USHORT nType ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    switch ( nType )
    {
        case WINDOW_PARENT:
            return mpWindowImpl->mpRealParent;

        case WINDOW_FIRSTCHILD:
            return mpWindowImpl->mpFirstChild;

        case WINDOW_LASTCHILD:
            return mpWindowImpl->mpLastChild;

        case WINDOW_PREV:
            return mpWindowImpl->mpPrev;

        case WINDOW_NEXT:
            return mpWindowImpl->mpNext;

        case WINDOW_FIRSTOVERLAP:
            return mpWindowImpl->mpFirstOverlap;

        case WINDOW_LASTOVERLAP:
            return mpWindowImpl->mpLastOverlap;

        case WINDOW_OVERLAP:
            if ( ImplIsOverlapWindow() )
                return (Window*)this;
            else
                return mpWindowImpl->mpOverlapWindow;

        case WINDOW_PARENTOVERLAP:
            if ( ImplIsOverlapWindow() )
                return mpWindowImpl->mpOverlapWindow;
            else
                return mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpOverlapWindow;

        case WINDOW_CLIENT:
            return ((Window*)this)->ImplGetWindow();

        case WINDOW_REALPARENT:
            return ImplGetParent();

        case WINDOW_FRAME:
            return mpWindowImpl->mpFrameWindow;

        case WINDOW_BORDER:
            if ( mpWindowImpl->mpBorderWindow )
                return mpWindowImpl->mpBorderWindow->GetWindow( WINDOW_BORDER );
            return (Window*)this;

        case WINDOW_FIRSTTOPWINDOWCHILD:
            return ImplGetWinData()->maTopWindowChildren.empty() ? NULL : *ImplGetWinData()->maTopWindowChildren.begin();

        case WINDOW_LASTTOPWINDOWCHILD:
            return ImplGetWinData()->maTopWindowChildren.empty() ? NULL : *ImplGetWinData()->maTopWindowChildren.rbegin();

        case WINDOW_PREVTOPWINDOWSIBLING:
        {
            if ( !mpWindowImpl->mpRealParent )
                return NULL;
            const ::std::list< Window* >& rTopWindows( mpWindowImpl->mpRealParent->ImplGetWinData()->maTopWindowChildren );
            ::std::list< Window* >::const_iterator myPos =
                ::std::find( rTopWindows.begin(), rTopWindows.end(), this );
            if ( myPos == rTopWindows.end() )
                return NULL;
            if ( myPos == rTopWindows.begin() )
                return NULL;
            return *--myPos;
        }

        case WINDOW_NEXTTOPWINDOWSIBLING:
        {
            if ( !mpWindowImpl->mpRealParent )
                return NULL;
            const ::std::list< Window* >& rTopWindows( mpWindowImpl->mpRealParent->ImplGetWinData()->maTopWindowChildren );
            ::std::list< Window* >::const_iterator myPos =
                ::std::find( rTopWindows.begin(), rTopWindows.end(), this );
            if ( ( myPos == rTopWindows.end() ) || ( ++myPos == rTopWindows.end() ) )
                return NULL;
            return *myPos;
        }

    }

    return NULL;
}

// -----------------------------------------------------------------------

BOOL Window::IsChild( const Window* pWindow, BOOL bSystemWindow ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    DBG_CHKOBJ( pWindow, Window, ImplDbgCheckWindow );

    do
    {
        if ( !bSystemWindow && pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();

        if ( pWindow == this )
            return TRUE;
    }
    while ( pWindow );

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Window::IsWindowOrChild( const Window* pWindow, BOOL bSystemWindow ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    DBG_CHKOBJ( pWindow, Window, ImplDbgCheckWindow );

    if ( this == pWindow )
        return TRUE;
    return ImplIsChild( pWindow, bSystemWindow );
}

// -----------------------------------------------------------------------

const SystemEnvData* Window::GetSystemData() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    return mpWindowImpl->mpFrame ? mpWindowImpl->mpFrame->GetSystemData() : NULL;
}

::com::sun::star::uno::Any Window::GetSystemDataAny() const
{
    ::com::sun::star::uno::Any aRet;
    const SystemEnvData* pSysData = GetSystemData();
    if( pSysData )
    {
        ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( (sal_Int8*)pSysData, pSysData->nSize );
        aRet <<= aSeq;
    }
    return aRet;
}

// -----------------------------------------------------------------------

void Window::SetWindowPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xPeer, VCLXWindow* pVCLXWindow  )
{
    // be safe against re-entrance: first clear the old ref, then assign the new one
    // #133706# / 2006-03-30 / frank.schoenheit@sun.com
    mpWindowImpl->mxWindowPeer.clear();
    mpWindowImpl->mxWindowPeer = xPeer;

    mpWindowImpl->mpVCLXWindow = pVCLXWindow;
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > Window::GetComponentInterface( BOOL bCreate )
{
    if ( !mpWindowImpl->mxWindowPeer.is() && bCreate )
    {
        UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
        if ( pWrapper )
            mpWindowImpl->mxWindowPeer = pWrapper->GetWindowInterface( this, sal_True );
    }
    return mpWindowImpl->mxWindowPeer;
}

// -----------------------------------------------------------------------

void Window::SetComponentInterface( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xIFace )
{
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    DBG_ASSERT( pWrapper, "SetComponentInterface: No Wrapper!" );
    if ( pWrapper )
        pWrapper->SetWindowInterface( this, xIFace );
}

// -----------------------------------------------------------------------

void Window::ImplCallDeactivateListeners( Window *pNew )
{
    // no deactivation if the the newly activated window is my child
    if ( !pNew || !ImplIsChild( pNew ) )
    {
        ImplDelData aDogtag( this );
        ImplCallEventListeners( VCLEVENT_WINDOW_DEACTIVATE );
        if( aDogtag.IsDelete() )
            return;

        // #100759#, avoid walking the wrong frame's hierarchy
        //           eg, undocked docking windows (ImplDockFloatWin)
        if ( ImplGetParent() && mpWindowImpl->mpFrameWindow == ImplGetParent()->mpWindowImpl->mpFrameWindow )
            ImplGetParent()->ImplCallDeactivateListeners( pNew );
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallActivateListeners( Window *pOld )
{
    // no activation if the the old active window is my child
    if ( !pOld || !ImplIsChild( pOld ) )
    {
        ImplDelData aDogtag( this );
        ImplCallEventListeners( VCLEVENT_WINDOW_ACTIVATE, pOld );
        if( aDogtag.IsDelete() )
            return;

        // #106298# revoke the change for 105369, because this change
        //          disabled the activate event for the parent,
        //          if the parent is a compound control
        //if( !GetParent() || !GetParent()->IsCompoundControl() )
        //{
            // #100759#, avoid walking the wrong frame's hierarchy
            //           eg, undocked docking windows (ImplDockFloatWin)
            // #104714#, revert the changes for 100759 because it has a side effect when pOld is a dialog
            //           additionally the gallery is not dockable anymore, so 100759 canot occur
            if ( ImplGetParent() ) /* && mpWindowImpl->mpFrameWindow == ImplGetParent()->mpWindowImpl->mpFrameWindow ) */
                ImplGetParent()->ImplCallActivateListeners( pOld );
            else if( (mpWindowImpl->mnStyle & WB_INTROWIN) == 0 )
            {
                // top level frame reached: store hint for DefModalDialogParent
                ImplGetSVData()->maWinData.mpActiveApplicationFrame = mpWindowImpl->mpFrameWindow;
            }
        //}
    }
}

// -----------------------------------------------------------------------

bool Window::ImplStopDnd()
{
    bool bRet = false;
    if( mpWindowImpl->mpFrameData && mpWindowImpl->mpFrameData->mxDropTargetListener.is() )
    {
        bRet = true;
        mpWindowImpl->mpFrameData->mxDropTarget.clear();
        mpWindowImpl->mpFrameData->mxDragSource.clear();
        mpWindowImpl->mpFrameData->mxDropTargetListener.clear();
    }

    return bRet;
}

// -----------------------------------------------------------------------

void Window::ImplStartDnd()
{
    GetDropTarget();
}

// -----------------------------------------------------------------------

Reference< XDropTarget > Window::GetDropTarget()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( ! mpWindowImpl->mxDNDListenerContainer.is() )
    {
        sal_Int8 nDefaultActions = 0;

        if( mpWindowImpl->mpFrameData )
        {
            if( ! mpWindowImpl->mpFrameData->mxDropTarget.is() )
            {
                // initialization is done in GetDragSource
                Reference< XDragSource > xDragSource = GetDragSource();
            }

            if( mpWindowImpl->mpFrameData->mxDropTarget.is() )
            {
                nDefaultActions = mpWindowImpl->mpFrameData->mxDropTarget->getDefaultActions();

                if( ! mpWindowImpl->mpFrameData->mxDropTargetListener.is() )
                {
                    mpWindowImpl->mpFrameData->mxDropTargetListener = new DNDEventDispatcher( mpWindowImpl->mpFrameWindow );

                    try
                    {
                        mpWindowImpl->mpFrameData->mxDropTarget->addDropTargetListener( mpWindowImpl->mpFrameData->mxDropTargetListener );

                        // register also as drag gesture listener if directly supported by drag source
                        Reference< XDragGestureRecognizer > xDragGestureRecognizer =
                            Reference< XDragGestureRecognizer > (mpWindowImpl->mpFrameData->mxDragSource, UNO_QUERY);

                        if( xDragGestureRecognizer.is() )
                        {
                            xDragGestureRecognizer->addDragGestureListener(
                                Reference< XDragGestureListener > (mpWindowImpl->mpFrameData->mxDropTargetListener, UNO_QUERY));
                        }
                        else
                            mpWindowImpl->mpFrameData->mbInternalDragGestureRecognizer = TRUE;

                    }

                    catch( RuntimeException exc )
                    {
                        // release all instances
                        mpWindowImpl->mpFrameData->mxDropTarget.clear();
                        mpWindowImpl->mpFrameData->mxDragSource.clear();
                    }
                }
            }

        }

        mpWindowImpl->mxDNDListenerContainer = static_cast < XDropTarget * > ( new DNDListenerContainer( nDefaultActions ) );
    }

    // this object is located in the same process, so there will be no runtime exception
    return Reference< XDropTarget > ( mpWindowImpl->mxDNDListenerContainer, UNO_QUERY );
}

// -----------------------------------------------------------------------

Reference< XDragSource > Window::GetDragSource()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxDragSource.is() )
        {
            try
            {
                Reference< XMultiServiceFactory > xFactory = vcl::unohelper::GetMultiServiceFactory();
                if ( xFactory.is() )
                {
                    const SystemEnvData * pEnvData = GetSystemData();

                    if( pEnvData )
                    {
                        Sequence< Any > aDragSourceAL( 2 ), aDropTargetAL( 2 );
                        OUString aDragSourceSN, aDropTargetSN;
#if defined WNT
                        aDragSourceSN = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.dnd.OleDragSource"));
                        aDropTargetSN = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.dnd.OleDropTarget"));
                        aDragSourceAL[ 1 ] = makeAny( (sal_uInt32) pEnvData->hWnd );
                        aDropTargetAL[ 0 ] = makeAny( (sal_uInt32) pEnvData->hWnd );
#elif defined QUARTZ
            /* FIXME: Mac OS X specific dnd interface does not exist! *
             * Using Windows based dnd as a temporary solution        */
                        aDragSourceSN = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.dnd.OleDragSource"));
                        aDropTargetSN = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.dnd.OleDropTarget"));
                        aDragSourceAL[ 1 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->pView) ) );
                        aDropTargetAL[ 0 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->pView) ) );
#elif defined UNX
                        aDropTargetAL.realloc( 3 );
                        aDragSourceAL.realloc( 3 );
                        aDragSourceSN = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.dnd.X11DragSource"));
                        aDropTargetSN = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.dnd.X11DropTarget"));

                        aDragSourceAL[ 0 ] = makeAny( Application::GetDisplayConnection() );
                        aDragSourceAL[ 2 ] = makeAny( vcl::createBmpConverter() );
                        aDropTargetAL[ 0 ] = makeAny( Application::GetDisplayConnection() );
                        aDropTargetAL[ 1 ] = makeAny( (sal_Size)(pEnvData->aShellWindow) );
                        aDropTargetAL[ 2 ] = makeAny( vcl::createBmpConverter() );
#endif
                        if( aDragSourceSN.getLength() )
                            mpWindowImpl->mpFrameData->mxDragSource = Reference< XDragSource > ( xFactory->createInstanceWithArguments( aDragSourceSN, aDragSourceAL ), UNO_QUERY );

                        if( aDropTargetSN.getLength() )
                            mpWindowImpl->mpFrameData->mxDropTarget = Reference< XDropTarget > ( xFactory->createInstanceWithArguments( aDropTargetSN, aDropTargetAL ), UNO_QUERY );
                    }
                }
            }

            // createInstance can throw any exception
            catch( Exception exc )
            {
                // release all instances
                mpWindowImpl->mpFrameData->mxDropTarget.clear();
                mpWindowImpl->mpFrameData->mxDragSource.clear();
            }
        }

        return mpWindowImpl->mpFrameData->mxDragSource;
    }

    return Reference< XDragSource > ();
}

// -----------------------------------------------------------------------

void Window::GetDragSourceDropTarget(Reference< XDragSource >& xDragSource, Reference< XDropTarget > &xDropTarget )
// only for RVP transmission
{
    if( mpWindowImpl->mpFrameData )
    {
        // initialization is done in GetDragSource
        xDragSource = GetDragSource();
        xDropTarget = mpWindowImpl->mpFrameData->mxDropTarget;
    }
    else
    {
        xDragSource.clear();
        xDropTarget.clear();
    }
}

// -----------------------------------------------------------------------

Reference< XDragGestureRecognizer > Window::GetDragGestureRecognizer()
{
    return Reference< XDragGestureRecognizer > ( GetDropTarget(), UNO_QUERY );
}

// -----------------------------------------------------------------------

Reference< XClipboard > Window::GetClipboard()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxClipboard.is() )
        {
            try
            {
                Reference< XMultiServiceFactory > xFactory( vcl::unohelper::GetMultiServiceFactory() );

                if( xFactory.is() )
                {
                    mpWindowImpl->mpFrameData->mxClipboard = Reference< XClipboard >( xFactory->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.clipboard.SystemClipboardExt")) ), UNO_QUERY );

                    if( !mpWindowImpl->mpFrameData->mxClipboard.is() )
                        mpWindowImpl->mpFrameData->mxClipboard = Reference< XClipboard >( xFactory->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.clipboard.SystemClipboard")) ), UNO_QUERY );

#if defined(UNX) && !defined(QUARTZ)          // unix clipboard needs to be initialized
                    if( mpWindowImpl->mpFrameData->mxClipboard.is() )
                    {
                        Reference< XInitialization > xInit = Reference< XInitialization >( mpWindowImpl->mpFrameData->mxClipboard, UNO_QUERY );

                        if( xInit.is() )
                        {
                            Sequence< Any > aArgumentList( 3 );
                            aArgumentList[ 0 ] = makeAny( Application::GetDisplayConnection() );
                            aArgumentList[ 1 ] = makeAny( OUString(RTL_CONSTASCII_USTRINGPARAM("CLIPBOARD")) );
                            aArgumentList[ 2 ] = makeAny( vcl::createBmpConverter() );

                            xInit->initialize( aArgumentList );
                        }
                    }
#endif
                }
            }

            // createInstance can throw any exception
            catch( Exception exc )
            {
                // release all instances
                mpWindowImpl->mpFrameData->mxClipboard.clear();
            }
        }

        return mpWindowImpl->mpFrameData->mxClipboard;
    }

    return static_cast < XClipboard * > (0);
}

// -----------------------------------------------------------------------

Reference< XClipboard > Window::GetPrimarySelection()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxSelection.is() )
        {
            try
            {
                Reference< XMultiServiceFactory > xFactory( vcl::unohelper::GetMultiServiceFactory() );

                if( xFactory.is() )
                {
#if defined(UNX) && !defined(QUARTZ)
                    Sequence< Any > aArgumentList( 3 );
                      aArgumentList[ 0 ] = makeAny( Application::GetDisplayConnection() );
                    aArgumentList[ 1 ] = makeAny( OUString(RTL_CONSTASCII_USTRINGPARAM("PRIMARY")) );
                    aArgumentList[ 2 ] = makeAny( vcl::createBmpConverter() );

                    mpWindowImpl->mpFrameData->mxSelection = Reference< XClipboard >( xFactory->createInstanceWithArguments(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.clipboard.SystemClipboard")), aArgumentList ), UNO_QUERY );
#       else
                    static Reference< XClipboard >      s_xSelection;

                    if ( !s_xSelection.is() )
                         s_xSelection = Reference< XClipboard >( xFactory->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.clipboard.GenericClipboardExt")) ), UNO_QUERY );

                    if ( !s_xSelection.is() )
                         s_xSelection = Reference< XClipboard >( xFactory->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.clipboard.GenericClipboard")) ), UNO_QUERY );

                    mpWindowImpl->mpFrameData->mxSelection = s_xSelection;
#       endif
                }
            }

            // createInstance can throw any exception
            catch( Exception exc )
            {
                // release all instances
                mpWindowImpl->mpFrameData->mxSelection.clear();
            }
        }

        return mpWindowImpl->mpFrameData->mxSelection;
    }

    return static_cast < XClipboard * > (0);
}

// -----------------------------------------------------------------------
// Accessibility
// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > Window::GetAccessible( BOOL bCreate )
{
    // do not optimize hierarchy for the top level border win (ie, when there is no parent)
    /* // do not optimize accessible hierarchy at all to better reflect real VCL hierarchy
    if ( GetParent() && ( GetType() == WINDOW_BORDERWINDOW ) && ( GetChildCount() == 1 ) )
    //if( !ImplIsAccessibleCandidate() )
    {
        Window* pChild = GetAccessibleChildWindow( 0 );
        if ( pChild )
            return pChild->GetAccessible();
    }
    */
    if ( !mpWindowImpl->mxAccessible.is() && bCreate )
        mpWindowImpl->mxAccessible = CreateAccessible();

    return mpWindowImpl->mxAccessible;
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > Window::CreateAccessible()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc( GetComponentInterface( TRUE ), ::com::sun::star::uno::UNO_QUERY );
    return xAcc;
}

void Window::SetAccessible( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > x )
{
    mpWindowImpl->mxAccessible = x;
}

// skip all border windows that are no top level frames
BOOL Window::ImplIsAccessibleCandidate() const
{
    if( !mpWindowImpl->mbBorderWin )
        return TRUE;
    else
        // #101741 do not check for WB_CLOSEABLE because undecorated floaters (like menues!) are closeable
        if( mpWindowImpl->mbFrame && mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE) )
            return TRUE;
        else
            return FALSE;
}

BOOL Window::ImplIsAccessibleNativeFrame() const
{
    if( mpWindowImpl->mbFrame )
        // #101741 do not check for WB_CLOSEABLE because undecorated floaters (like menues!) are closeable
        if( (mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE)) )
            return TRUE;
        else
            return FALSE;
    else
        return FALSE;
}

USHORT Window::ImplGetAccessibleCandidateChildWindowCount( USHORT nFirstWindowType ) const
{
    USHORT  nChildren = 0;
    Window* pChild = GetWindow( nFirstWindowType );
    while ( pChild )
    {
        if( pChild->ImplIsAccessibleCandidate() )
            nChildren++;
        else
            nChildren = sal::static_int_cast<USHORT>(nChildren + pChild->ImplGetAccessibleCandidateChildWindowCount( WINDOW_FIRSTCHILD ));
        pChild = pChild->mpWindowImpl->mpNext;
    }
    return nChildren;
}

Window* Window::ImplGetAccessibleCandidateChild( USHORT nChild, USHORT& rChildCount, USHORT nFirstWindowType, BOOL bTopLevel ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( bTopLevel )
        rChildCount = 0;

    Window* pChild = GetWindow( nFirstWindowType );
    while ( pChild )
    {
        Window *pTmpChild = pChild;

        if( !pChild->ImplIsAccessibleCandidate() )
            pTmpChild = pChild->ImplGetAccessibleCandidateChild( nChild, rChildCount, WINDOW_FIRSTCHILD, FALSE );

        if ( nChild == rChildCount )
            return pTmpChild;
        pChild = pChild->mpWindowImpl->mpNext;
        rChildCount++;
    }

    return NULL;
}

/*
Window* Window::GetAccessibleParentWindow() const
{
    Window* pParent = GetParent();
    while ( pParent )
        if( pParent->ImplIsAccessibleCandidate() )
            break;
        else
            pParent = pParent->GetParent();
    return pParent;
}
*/

Window* Window::GetAccessibleParentWindow() const
{
    if ( ImplIsAccessibleNativeFrame() )
        return NULL;

    Window* pParent = mpWindowImpl->mpParent;
    if( GetType() == WINDOW_MENUBARWINDOW )
    {
        // report the menubar as a child of THE workwindow
        Window *pWorkWin = GetParent()->mpWindowImpl->mpFirstChild;
        while( pWorkWin && (pWorkWin == this) )
            pWorkWin = pWorkWin->mpWindowImpl->mpNext;
        pParent = pWorkWin;
    }
    // If this a floating window which has a native boarder window, this one should be reported as
    // accessible parent
    else if( GetType() == WINDOW_FLOATINGWINDOW &&
        mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame)
    {
        pParent = mpWindowImpl->mpBorderWindow;
    }
    else if( pParent && !pParent->ImplIsAccessibleCandidate() )
    {
        pParent = pParent->mpWindowImpl->mpParent;
    }
    return pParent;
}

/*
USHORT Window::GetAccessibleChildWindowCount()
{
    USHORT nChildren = ImplGetAccessibleCandidateChildWindowCount( WINDOW_FIRSTCHILD );

    // Search also for SystemWindows.
    Window* pOverlap = GetWindow( WINDOW_OVERLAP );
    nChildren += pOverlap->ImplGetAccessibleCandidateChildWindowCount( WINDOW_FIRSTOVERLAP );

    return nChildren;
}
*/

USHORT Window::GetAccessibleChildWindowCount()
{
    USHORT nChildren = 0;
    Window* pChild = mpWindowImpl->mpFirstChild;
    while( pChild )
    {
        if( pChild->IsVisible() )
            nChildren++;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    // #107176# ignore overlapwindows
    // this only affects non-system floating windows
    // which are either not accessible (like the HelpAgent) or should be changed to system windows anyway
    /*
    if( ImplIsOverlapWindow() )
    {
        Window* pOverlap = GetWindow( WINDOW_FIRSTOVERLAP );
        while ( pOverlap )
        {
            if( pOverlap->IsVisible() )
                nChildren++;
            pOverlap = pOverlap->GetWindow( WINDOW_NEXT );
        }
    }
    */

    // report the menubarwindow as a child of THE workwindow
    if( GetType() == WINDOW_BORDERWINDOW )
    {
        if( ((ImplBorderWindow *) this)->mpMenuBarWindow &&
            ((ImplBorderWindow *) this)->mpMenuBarWindow->IsVisible()
            )
            --nChildren;
    }
    else if( GetType() == WINDOW_WORKWINDOW )
    {
        if( ((WorkWindow *) this)->GetMenuBar() &&
            ((WorkWindow *) this)->GetMenuBar()->GetWindow() &&
            ((WorkWindow *) this)->GetMenuBar()->GetWindow()->IsVisible()
            )
            ++nChildren;
    }

    return nChildren;
}

/*
Window* Window::GetAccessibleChildWindow( USHORT n )
{
    USHORT nChildCount; // will be set in ImplGetAccessibleCandidateChild(...)
    Window* pChild = ImplGetAccessibleCandidateChild( n, nChildCount, WINDOW_FIRSTCHILD, TRUE );
    if ( !pChild && ( n >= nChildCount ) )
    {
        Window* pOverlap = GetWindow( WINDOW_OVERLAP );
        pChild = pOverlap->ImplGetAccessibleCandidateChild( n, nChildCount, WINDOW_FIRSTOVERLAP, FALSE );
    }

    return pChild;
}
*/

Window* Window::GetAccessibleChildWindow( USHORT n )
{
    // report the menubarwindow as a the first child of THE workwindow
    if( GetType() == WINDOW_WORKWINDOW && ((WorkWindow *) this)->GetMenuBar() )
    {
        if( n == 0 )
        {
            MenuBar *pMenuBar = ((WorkWindow *) this)->GetMenuBar();
            if( pMenuBar->GetWindow() && pMenuBar->GetWindow()->IsVisible() )
                return pMenuBar->GetWindow();
        }
        else
            --n;
    }

    // transform n to child number including invisible children
    USHORT nChildren = n;
    Window* pChild = mpWindowImpl->mpFirstChild;
    while( pChild )
    {
        if( pChild->IsVisible() )
        {
            if( ! nChildren )
                break;
            nChildren--;
        }
        pChild = pChild->mpWindowImpl->mpNext;
    }

    if( GetType() == WINDOW_BORDERWINDOW && pChild && pChild->GetType() == WINDOW_MENUBARWINDOW )
    {
        do pChild = pChild->mpWindowImpl->mpNext; while( pChild && ! pChild->IsVisible() );
        DBG_ASSERT( pChild, "GetAccessibleChildWindow(): wrong index in border window");
    }
    if ( !pChild )
    {
        // #107176# ignore overlapwindows
        /*
        if( ImplIsOverlapWindow() )
        {
            Window* pOverlap = GetWindow( WINDOW_FIRSTOVERLAP );
            while ( !pChild && pOverlap )
            {
                if ( !nChildren && pOverlap->IsVisible() )
                {
                    pChild = pOverlap;
                    break;
                }
                pOverlap = pOverlap->GetWindow( WINDOW_NEXT );
                if( pOverlap && pOverlap->IsVisible() )
                    nChildren--;
            }
        }
        */

    }
    if ( pChild && ( pChild->GetType() == WINDOW_BORDERWINDOW ) && ( pChild->GetChildCount() == 1 ) )
    {
        pChild = pChild->GetChild( 0 );
    }
    return pChild;
}


void Window::SetAccessibleRole( USHORT nRole )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;

    DBG_ASSERT( mpWindowImpl->mpAccessibleInfos->nAccessibleRole == 0xFFFF, "AccessibleRole already set!" );
    mpWindowImpl->mpAccessibleInfos->nAccessibleRole = nRole;
}

USHORT Window::GetAccessibleRole() const
{
    using namespace ::com::sun::star;

    USHORT nRole = mpWindowImpl->mpAccessibleInfos ? mpWindowImpl->mpAccessibleInfos->nAccessibleRole : 0xFFFF;
    if ( nRole == 0xFFFF )
    {
        switch ( GetType() )
        {
            case WINDOW_MESSBOX:    // MT: Would be nice to have special roles!
            case WINDOW_INFOBOX:
            case WINDOW_WARNINGBOX:
            case WINDOW_ERRORBOX:
            case WINDOW_QUERYBOX: nRole = accessibility::AccessibleRole::ALERT; break;

            case WINDOW_MODELESSDIALOG:
            case WINDOW_MODALDIALOG:
            case WINDOW_SYSTEMDIALOG:
            case WINDOW_PRINTERSETUPDIALOG:
            case WINDOW_PRINTDIALOG:
            case WINDOW_TABDIALOG:
            case WINDOW_BUTTONDIALOG:
            case WINDOW_DIALOG: nRole = accessibility::AccessibleRole::DIALOG; break;

            case WINDOW_PUSHBUTTON:
            case WINDOW_OKBUTTON:
            case WINDOW_CANCELBUTTON:
            case WINDOW_HELPBUTTON:
            case WINDOW_IMAGEBUTTON:
            case WINDOW_MENUBUTTON:
            case WINDOW_MOREBUTTON:
            case WINDOW_SPINBUTTON:
            case WINDOW_BUTTON: nRole = accessibility::AccessibleRole::PUSH_BUTTON; break;

            case WINDOW_PATHDIALOG: nRole = accessibility::AccessibleRole::DIRECTORY_PANE; break;
            case WINDOW_FILEDIALOG: nRole = accessibility::AccessibleRole::FILE_CHOOSER; break;
            case WINDOW_COLORDIALOG: nRole = accessibility::AccessibleRole::COLOR_CHOOSER; break;
            case WINDOW_FONTDIALOG: nRole = accessibility::AccessibleRole::FONT_CHOOSER; break;

            case WINDOW_IMAGERADIOBUTTON:
            case WINDOW_RADIOBUTTON: nRole = accessibility::AccessibleRole::RADIO_BUTTON; break;
            case WINDOW_TRISTATEBOX:
            case WINDOW_CHECKBOX: nRole = accessibility::AccessibleRole::CHECK_BOX; break;

            case WINDOW_MULTILINEEDIT: nRole = accessibility::AccessibleRole::SCROLL_PANE; break;

            case WINDOW_PATTERNFIELD:
            case WINDOW_NUMERICFIELD:
            case WINDOW_METRICFIELD:
            case WINDOW_CURRENCYFIELD:
            case WINDOW_LONGCURRENCYFIELD:
            case WINDOW_EDIT: nRole = ( GetStyle() & WB_PASSWORD ) ? (accessibility::AccessibleRole::PASSWORD_TEXT) : (accessibility::AccessibleRole::TEXT); break;

            case WINDOW_PATTERNBOX:
            case WINDOW_NUMERICBOX:
            case WINDOW_METRICBOX:
            case WINDOW_CURRENCYBOX:
            case WINDOW_LONGCURRENCYBOX:
            case WINDOW_COMBOBOX: nRole = accessibility::AccessibleRole::COMBO_BOX; break;

            case WINDOW_LISTBOX:
            case WINDOW_MULTILISTBOX: nRole = accessibility::AccessibleRole::LIST; break;

            case WINDOW_TREELISTBOX: nRole = accessibility::AccessibleRole::TREE; break;

            case WINDOW_FIXEDTEXT: nRole = accessibility::AccessibleRole::LABEL; break;
            case WINDOW_FIXEDBORDER:
            case WINDOW_FIXEDLINE: nRole = accessibility::AccessibleRole::SEPARATOR; break;
            case WINDOW_FIXEDBITMAP:
            case WINDOW_FIXEDIMAGE: nRole = accessibility::AccessibleRole::ICON; break;
            case WINDOW_GROUPBOX: nRole = accessibility::AccessibleRole::GROUP_BOX; break;
            case WINDOW_SCROLLBAR: nRole = accessibility::AccessibleRole::SCROLL_BAR; break;

            case WINDOW_SLIDER:
            case WINDOW_SPLITTER:
            case WINDOW_SPLITWINDOW: nRole = accessibility::AccessibleRole::SPLIT_PANE; break;

            case WINDOW_DATEBOX:
            case WINDOW_TIMEBOX:
            case WINDOW_DATEFIELD:
            case WINDOW_TIMEFIELD: nRole = accessibility::AccessibleRole::DATE_EDITOR; break;

            case WINDOW_SPINFIELD: nRole = accessibility::AccessibleRole::SPIN_BOX; break;

            case WINDOW_TOOLBOX: nRole = accessibility::AccessibleRole::TOOL_BAR; break;
            case WINDOW_STATUSBAR: nRole = accessibility::AccessibleRole::STATUS_BAR; break;

            case WINDOW_TABPAGE: nRole = accessibility::AccessibleRole::PANEL; break;
            case WINDOW_TABCONTROL: nRole = accessibility::AccessibleRole::PAGE_TAB_LIST; break;

            case WINDOW_DOCKINGWINDOW:
            case WINDOW_SYSWINDOW:      nRole = (mpWindowImpl->mbFrame) ? accessibility::AccessibleRole::FRAME :
                                                                          accessibility::AccessibleRole::PANEL; break;

            case WINDOW_FLOATINGWINDOW: nRole = ( mpWindowImpl->mbFrame ||
                                                 (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) ||
                                                 (GetStyle() & WB_OWNERDRAWDECORATION) ) ? accessibility::AccessibleRole::FRAME :
                                                                                           accessibility::AccessibleRole::WINDOW; break;

            case WINDOW_WORKWINDOW: nRole = accessibility::AccessibleRole::ROOT_PANE; break;


            case WINDOW_SCROLLBARBOX: nRole = accessibility::AccessibleRole::FILLER; break;

            case WINDOW_HELPTEXTWINDOW: nRole = accessibility::AccessibleRole::TOOL_TIP; break;

            case WINDOW_WINDOW:
            case WINDOW_CONTROL:
            case WINDOW_BORDERWINDOW:
            case WINDOW_SYSTEMCHILDWINDOW:
            default:
                if (ImplIsAccessibleNativeFrame() )
                    nRole = accessibility::AccessibleRole::FRAME;
                else if( IsScrollable() )
                    nRole = accessibility::AccessibleRole::SCROLL_PANE;
                else if( ((Window*)this)->ImplGetWindow()->IsMenuFloatingWindow() )
                    nRole = accessibility::AccessibleRole::WINDOW;      // #106002#, contextmenues are windows (i.e. toplevel)
                else
                    // #104051# WINDOW seems to be a bad default role, use LAYEREDPANE instead
                    // a WINDOW is interpreted as a top-level window, which is typically not the case
                    //nRole = accessibility::AccessibleRole::WINDOW;
                    nRole = accessibility::AccessibleRole::PANEL;
        }
    }
    return nRole;
}

void Window::SetAccessibleName( const String& rName )
{
   if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;

    DBG_ASSERT( !mpWindowImpl->mpAccessibleInfos->pAccessibleName, "AccessibleName already set!" );
    delete mpWindowImpl->mpAccessibleInfos->pAccessibleName;
    mpWindowImpl->mpAccessibleInfos->pAccessibleName = new String( rName );
}

String Window::GetAccessibleName() const
{
    String aAccessibleName;
    if ( mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pAccessibleName )
    {
        aAccessibleName = *mpWindowImpl->mpAccessibleInfos->pAccessibleName;
    }
    else
    {
        switch ( GetType() )
        {
//            case WINDOW_IMAGERADIOBUTTON:
//            case WINDOW_RADIOBUTTON:
//            case WINDOW_TRISTATEBOX:
//            case WINDOW_CHECKBOX:

            case WINDOW_MULTILINEEDIT:
            case WINDOW_PATTERNFIELD:
            case WINDOW_NUMERICFIELD:
            case WINDOW_METRICFIELD:
            case WINDOW_CURRENCYFIELD:
            case WINDOW_LONGCURRENCYFIELD:
            case WINDOW_EDIT:

            case WINDOW_DATEBOX:
            case WINDOW_TIMEBOX:
            case WINDOW_CURRENCYBOX:
            case WINDOW_LONGCURRENCYBOX:
            case WINDOW_DATEFIELD:
            case WINDOW_TIMEFIELD:
            case WINDOW_SPINFIELD:

            case WINDOW_COMBOBOX:
            case WINDOW_LISTBOX:
            case WINDOW_MULTILISTBOX:
            case WINDOW_TREELISTBOX:

            {
                Window *pLabel = GetLabeledBy();
                if ( pLabel && pLabel != this )
                    aAccessibleName = pLabel->GetText();
            }
            break;

            case WINDOW_IMAGEBUTTON:
            case WINDOW_PUSHBUTTON:
                aAccessibleName = GetText();
                if ( !aAccessibleName.Len() )
                {
                    aAccessibleName = GetQuickHelpText();
                    if ( !aAccessibleName.Len() )
                        aAccessibleName = GetHelpText();
                }
            break;

            default:
                aAccessibleName = GetText();
                break;
        }

        aAccessibleName = GetNonMnemonicString( aAccessibleName );
    }

    return aAccessibleName;
}

void Window::SetAccessibleDescription( const String& rDescription )
{
   if ( ! mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;

    DBG_ASSERT( !mpWindowImpl->mpAccessibleInfos->pAccessibleDescription, "AccessibleDescription already set!" );
    delete mpWindowImpl->mpAccessibleInfos->pAccessibleDescription;
    mpWindowImpl->mpAccessibleInfos->pAccessibleDescription = new String( rDescription );
}

String Window::GetAccessibleDescription() const
{
    String aAccessibleDescription;
    if ( mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pAccessibleDescription )
    {
        aAccessibleDescription = *mpWindowImpl->mpAccessibleInfos->pAccessibleDescription;
    }
    else
    {
        // Special code for help text windows. ZT asks the border window for the
        // description so we have to forward this request to our inner window.
        const Window* pWin = ((Window *)this)->ImplGetWindow();
        if ( pWin->GetType() == WINDOW_HELPTEXTWINDOW )
            aAccessibleDescription = pWin->GetHelpText();
        else
            aAccessibleDescription = GetHelpText();
    }

    return aAccessibleDescription;
}

BOOL Window::IsAccessibilityEventsSuppressed( BOOL bTraverseParentPath )
{
    if( !bTraverseParentPath )
        return mpWindowImpl->mbSuppressAccessibilityEvents;
    else
    {
        Window *pParent = this;
        while ( pParent && pParent->mpWindowImpl)
        {
            if( pParent->mpWindowImpl->mbSuppressAccessibilityEvents )
                return TRUE;
            else
                pParent = pParent->mpWindowImpl->mpParent; // do not use GetParent() to find borderwindows that are frames
        }
        return FALSE;
    }
}

void Window::RecordLayoutData( vcl::ControlLayoutData* pLayout, const Rectangle& rRect )
{
    if( ! mpOutDevData )
        ImplInitOutDevData();
    mpOutDevData->mpRecordLayout = pLayout;
    mpOutDevData->maRecordRect = rRect;
    Paint( rRect );
    mpOutDevData->mpRecordLayout = NULL;
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------


// returns background color used in this control
// false: could not determine color
BOOL Window::ImplGetCurrentBackgroundColor( Color& rCol )
{
    BOOL bRet = TRUE;

    switch ( GetType() )
    {
        // peform special handling here
        case WINDOW_PUSHBUTTON:
        case WINDOW_OKBUTTON:
        case WINDOW_CANCELBUTTON:
        // etc.
        default:
            if( IsControlBackground() )
                rCol = GetControlBackground();
            else if( IsBackground() )
                {
                    Wallpaper aWall = GetBackground();
                    if( !aWall.IsGradient() && !aWall.IsBitmap() )
                        rCol = aWall.GetColor();
                    else
                        bRet = FALSE;
                }
            else
                rCol = GetSettings().GetStyleSettings().GetFaceColor();
            break;
    }
    return bRet;
}

void Window::DrawSelectionBackground( const Rectangle& rRect, USHORT highlight, BOOL bChecked, BOOL bDrawBorder, BOOL bDrawExtBorderOnly )
{
    DrawSelectionBackground( rRect, highlight, bChecked, bDrawBorder, bDrawExtBorderOnly, 0, NULL, NULL );
}

void Window::DrawSelectionBackground( const Rectangle& rRect, USHORT highlight, BOOL bChecked, BOOL bDrawBorder, BOOL bDrawExtBorderOnly, Color* pSelectionTextColor )
{
    DrawSelectionBackground( rRect, highlight, bChecked, bDrawBorder, bDrawExtBorderOnly, 0, pSelectionTextColor, NULL );
}

void Window::DrawSelectionBackground( const Rectangle& rRect,
                                      USHORT highlight,
                                      BOOL bChecked,
                                      BOOL bDrawBorder,
                                      BOOL bDrawExtBorderOnly,
                                      long nCornerRadius,
                                      Color* pSelectionTextColor,
                                      Color* pPaintColor
                                      )
{
    if( rRect.IsEmpty() )
        return;

    bool bRoundEdges = nCornerRadius > 0;

    const StyleSettings& rStyles = GetSettings().GetStyleSettings();


    // colors used for item highlighting
    Color aSelectionBorderCol( pPaintColor ? *pPaintColor : rStyles.GetHighlightColor() );
    Color aSelectionFillCol( aSelectionBorderCol );

    BOOL bDark = rStyles.GetFaceColor().IsDark();
    BOOL bBright = ( rStyles.GetFaceColor() == Color( COL_WHITE ) );

    int c1 = aSelectionBorderCol.GetLuminance();
    int c2 = GetDisplayBackground().GetColor().GetLuminance();

    if( !bDark && !bBright && abs( c2-c1 ) < (pPaintColor ? 40 : 75) )
    {
        // constrast too low
        USHORT h,s,b;
        aSelectionFillCol.RGBtoHSB( h, s, b );
        if( b > 50 )    b -= 40;
        else            b += 40;
        aSelectionFillCol.SetColor( Color::HSBtoRGB( h, s, b ) );
        aSelectionBorderCol = aSelectionFillCol;
    }

    if( bRoundEdges )
    {
        if( aSelectionBorderCol.IsDark() )
            aSelectionBorderCol.IncreaseLuminance( 128 );
        else
            aSelectionBorderCol.DecreaseLuminance( 128 );
    }

    Rectangle aRect( rRect );
    if( bDrawExtBorderOnly )
    {
        aRect.nLeft     -= 1;
        aRect.nTop      -= 1;
        aRect.nRight    += 1;
        aRect.nBottom   += 1;
    }
    Color oldFillCol = GetFillColor();
    Color oldLineCol = GetLineColor();

    if( bDrawBorder )
        SetLineColor( bDark ? Color(COL_WHITE) : ( bBright ? Color(COL_BLACK) : aSelectionBorderCol ) );
    else
        SetLineColor();

    USHORT nPercent = 0;
    if( !highlight )
    {
        if( bDark )
            aSelectionFillCol = COL_BLACK;
        else
            nPercent = bRoundEdges ? 90 : 80;  // just checked (light)
    }
    else
    {
        if( bChecked && highlight == 2 )
        {
            if( bDark )
                aSelectionFillCol = COL_LIGHTGRAY;
            else if ( bBright )
            {
                aSelectionFillCol = COL_BLACK;
                SetLineColor( COL_BLACK );
                nPercent = 0;
            }
            else
                nPercent = bRoundEdges ? 50 : 20;          // selected, pressed or checked ( very dark )
        }
        else if( bChecked || highlight == 1 )
        {
            if( bDark )
                aSelectionFillCol = COL_GRAY;
            else if ( bBright )
            {
                aSelectionFillCol = COL_BLACK;
                SetLineColor( COL_BLACK );
                nPercent = 0;
            }
            else
                nPercent = bRoundEdges ? 70 : 35;          // selected, pressed or checked ( very dark )
        }
        else
        {
            if( bDark )
                aSelectionFillCol = COL_LIGHTGRAY;
            else if ( bBright )
            {
                aSelectionFillCol = COL_BLACK;
                SetLineColor( COL_BLACK );
                if( highlight == 3 )
                    nPercent = 80;
                else
                    nPercent = 0;
            }
            else
                nPercent = bRoundEdges ? 80 : 70;          // selected ( dark )
        }
    }

    if( bDark && bDrawExtBorderOnly )
    {
        SetFillColor();
        if( pSelectionTextColor )
            *pSelectionTextColor = rStyles.GetHighlightTextColor();
    }
    else
    {
        SetFillColor( aSelectionFillCol );
        if( pSelectionTextColor )
        {
            Color aTextColor = IsControlBackground() ? GetControlForeground() : rStyles.GetButtonTextColor();
            Color aHLTextColor = rStyles.GetHighlightTextColor();
            int nTextDiff = abs(aSelectionFillCol.GetLuminance() - aTextColor.GetLuminance());
            int nHLDiff = abs(aSelectionFillCol.GetLuminance() - aHLTextColor.GetLuminance());
            *pSelectionTextColor = (nHLDiff >= nTextDiff) ? aHLTextColor : aTextColor;
        }
    }


    if( bDark )
    {
        DrawRect( aRect );
    }
    else
    {
        if( bRoundEdges )
        {
            Polygon aPoly( aRect, nCornerRadius, nCornerRadius );
            PolyPolygon aPolyPoly( aPoly );
            DrawTransparent( aPolyPoly, nPercent );
        }
        else
        {
            Polygon aPoly( aRect );
            PolyPolygon aPolyPoly( aPoly );
            DrawTransparent( aPolyPoly, nPercent );
        }
    }

    SetFillColor( oldFillCol );
    SetLineColor( oldLineCol );
}

/*
void Window::DbgAssertNoEventListeners()
{
    VclWindowEvent aEvent( this, 0, NULL );
    DBG_ASSERT( mpWindowImpl->maEventListeners.empty(), "Eventlistener: Who is still listening???" )
    if ( !mpWindowImpl->maEventListeners.empty() )
        mpWindowImpl->maEventListeners.Call( &aEvent );

    DBG_ASSERT( mpWindowImpl->maChildEventListeners.empty(), "ChildEventlistener: Who is still listening???" )
    if ( !mpWindowImpl->maChildEventListeners.empty() )
        mpWindowImpl->maChildEventListeners.Call( &aEvent );
}
*/

// controls should return the window that gets the
// focus by default, so keyevents can be sent to that window directly
Window* Window::GetPreferredKeyInputWindow()
{
    return this;
}


BOOL Window::IsScrollable() const
{
    // check for scrollbars
    Window *pChild = mpWindowImpl->mpFirstChild;
    while( pChild )
    {
        if( pChild->GetType() == WINDOW_SCROLLBAR )
            return true;
        else
            pChild = pChild->mpWindowImpl->mpNext;
    }
    return false;
}

BOOL Window::IsTopWindow() const
{
    if ( mpWindowImpl->mbInDtor )
        return FALSE;

    // topwindows must be frames or they must have a borderwindow which is a frame
    if( !mpWindowImpl->mbFrame && (!mpWindowImpl->mpBorderWindow || (mpWindowImpl->mpBorderWindow && !mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) ) )
        return FALSE;

    ImplGetWinData();
    if( mpWindowImpl->mpWinData->mnIsTopWindow == (USHORT)~0)    // still uninitialized
    {
        // #113722#, cache result of expensive queryInterface call
        Window *pThisWin = (Window*)this;
        Reference< XTopWindow > xTopWindow( pThisWin->GetComponentInterface(), UNO_QUERY );
        pThisWin->mpWindowImpl->mpWinData->mnIsTopWindow = xTopWindow.is() ? 1 : 0;
    }
    return mpWindowImpl->mpWinData->mnIsTopWindow == 1 ? TRUE : FALSE;
}

void Window::ImplMirrorFramePos( Point &pt ) const
{
    pt.X() = mpWindowImpl->mpFrame->maGeometry.nWidth-1-pt.X();
}

// frame based modal counter (dialogs are not modal to the whole application anymore)
BOOL Window::IsInModalMode() const
{
    return (mpWindowImpl->mpFrameWindow->mpWindowImpl->mpFrameData->mnModalMode != 0);
}
void Window::ImplIncModalCount()
{
    Window* pFrameWindow = mpWindowImpl->mpFrameWindow;
    Window* pParent = pFrameWindow;
    while( pFrameWindow )
    {
        pFrameWindow->mpWindowImpl->mpFrameData->mnModalMode++;
        while( pParent && pParent->mpWindowImpl->mpFrameWindow == pFrameWindow )
        {
            pParent = pParent->GetParent();
        }
        pFrameWindow = pParent ? pParent->mpWindowImpl->mpFrameWindow : NULL;
    }
}
void Window::ImplDecModalCount()
{
    Window* pFrameWindow = mpWindowImpl->mpFrameWindow;
    Window* pParent = pFrameWindow;
    while( pFrameWindow )
    {
        pFrameWindow->mpWindowImpl->mpFrameData->mnModalMode--;
        while( pParent && pParent->mpWindowImpl->mpFrameWindow == pFrameWindow )
        {
            pParent = pParent->GetParent();
        }
        pFrameWindow = pParent ? pParent->mpWindowImpl->mpFrameWindow : NULL;
    }
}
BOOL Window::ImplIsInTaskPaneList()
{
    return mpWindowImpl->mbIsInTaskPaneList;
}
void Window::ImplIsInTaskPaneList( BOOL mbIsInTaskList )
{
    mpWindowImpl->mbIsInTaskPaneList = mbIsInTaskList;
}

void Window::ImplNotifyIconifiedState( BOOL bIconified )
{
    mpWindowImpl->mpFrameWindow->ImplCallEventListeners( bIconified ? VCLEVENT_WINDOW_MINIMIZE : VCLEVENT_WINDOW_NORMALIZE );
    // #109206# notify client window as well to have toolkit topwindow listeners notified
    if( mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow && mpWindowImpl->mpFrameWindow != mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow )
        mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow->ImplCallEventListeners( bIconified ? VCLEVENT_WINDOW_MINIMIZE : VCLEVENT_WINDOW_NORMALIZE );
}

BOOL Window::HasActiveChildFrame()
{
    BOOL bRet = FALSE;
    Window *pFrameWin = ImplGetSVData()->maWinData.mpFirstFrame;
    while( pFrameWin )
    {
        if( pFrameWin != mpWindowImpl->mpFrameWindow )
        {
            BOOL bDecorated = FALSE;
            Window *pChildFrame = pFrameWin->ImplGetWindow();
            // #i15285# unfortunately WB_MOVEABLE is the same as WB_TABSTOP which can
            // be removed for ToolBoxes to influence the keyboard accessibility
            // thus WB_MOVEABLE is no indicator for decoration anymore
            // but FloatingWindows carry this information in their TitleType...
            // TODO: avoid duplicate WinBits !!!
            if( pChildFrame && pChildFrame->ImplIsFloatingWindow() )
                bDecorated = ((FloatingWindow*) pChildFrame)->GetTitleType() != FLOATWIN_TITLE_NONE;
            if( bDecorated || (pFrameWin->mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE) ) )
                if( pChildFrame && pChildFrame->IsVisible() && pChildFrame->IsActive() )
                {
                    if( ImplIsChild( pChildFrame, TRUE ) )
                    {
                        bRet = TRUE;
                        break;
                    }
                }
        }
        pFrameWin = pFrameWin->mpWindowImpl->mpFrameData->mpNextFrame;
    }
    return bRet;
}

LanguageType Window::GetInputLanguage() const
{
    return mpWindowImpl->mpFrame->GetInputLanguage();
}

void Window::EnableNativeWidget( BOOL bEnable )
{
    static const char* pNoNWF = getenv( "SAL_NO_NWF" );
    if( pNoNWF && *pNoNWF )
        bEnable = FALSE;

    if( bEnable != ImplGetWinData()->mbEnableNativeWidget )
    {
        ImplGetWinData()->mbEnableNativeWidget = bEnable;

        // send datachanged event to allow for internal changes required for NWF
        // like clipmode, transparency, etc.
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &maSettings, SETTINGS_STYLE );
        DataChanged( aDCEvt );

        // sometimes the borderwindow is queried, so keep it in sync
        if( mpWindowImpl->mpBorderWindow )
            mpWindowImpl->mpBorderWindow->ImplGetWinData()->mbEnableNativeWidget = bEnable;
    }

    // push down, useful for compound controls
    Window *pChild = mpWindowImpl->mpFirstChild;
    while( pChild )
    {
        pChild->EnableNativeWidget( bEnable );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

BOOL Window::IsNativeWidgetEnabled() const
{
    return ImplGetWinData()->mbEnableNativeWidget;
}

#ifdef WNT // see #140456#
#include <salframe.h>
#endif

Reference< rendering::XCanvas > Window::ImplGetCanvas( const Size& rFullscreenSize,
                                                       bool        bFullscreen,
                                                       bool        bSpriteCanvas ) const
{
    // try to retrieve hard reference from weak member
    Reference< rendering::XCanvas > xCanvas( mpWindowImpl->mxCanvas );

    // canvas still valid? Then we're done.
    if( xCanvas.is() )
        return xCanvas;

    Sequence< Any > aArg(6);

    // Feed any with operating system's window handle
    // ==============================================

    // common: first any is VCL pointer to window (for VCL canvas)
    aArg[ 0 ] = makeAny( reinterpret_cast<sal_Int64>(this) );

    // TODO(Q1): Make GetSystemData method virtual

    // check whether we're a SysChild: have to fetch system data
    // directly from SystemChildWindow, because the GetSystemData
    // method is unfortunately not virtual
    const SystemChildWindow* pSysChild = dynamic_cast< const SystemChildWindow* >( this );
    if( pSysChild )
    {
        aArg[ 1 ] = pSysChild->GetSystemDataAny();
        aArg[ 5 ] = pSysChild->GetSystemGfxDataAny();
    }
    else
    {
        aArg[ 1 ] = GetSystemDataAny();
        aArg[ 5 ] = GetSystemGfxDataAny();
    }

    if( bFullscreen )
        aArg[ 2 ] = makeAny( ::com::sun::star::awt::Rectangle( 0, 0,
                                                               rFullscreenSize.Width(),
                                                               rFullscreenSize.Height() ) );
    else
        aArg[ 2 ] = makeAny( ::com::sun::star::awt::Rectangle( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight ) );

    aArg[ 3 ] = makeAny( mpWindowImpl->mbAlwaysOnTop ? sal_True : sal_False );
    aArg[ 4 ] = makeAny( Reference< awt::XWindow >(
                             const_cast<Window*>(this)->GetComponentInterface(),
                             uno::UNO_QUERY ));

    Reference< XMultiServiceFactory > xFactory = vcl::unohelper::GetMultiServiceFactory();

    // Create canvas instance with window handle
    // =========================================
    if ( xFactory.is() )
    {
        static Reference<lang::XMultiServiceFactory> xCanvasFactory(
            xFactory->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                              "com.sun.star."
                              "rendering.CanvasFactory") ) ), UNO_QUERY );
        if(xCanvasFactory.is())
        {
#ifdef WNT
            // see #140456# - if we're running on a multiscreen setup,
            // request special, multi-screen safe sprite canvas
            // implementation (not DX5 canvas, as it cannot cope with
            // surfaces spanning multiple displays). Note: canvas
            // (without sprite) stays the same)
            const sal_uInt32 nDisplay = static_cast< WinSalFrame* >( mpWindowImpl->mpFrame )->mnDisplay;
            if( (nDisplay >= Application::GetScreenCount()) )
            {
                xCanvas.set( xCanvasFactory->createInstanceWithArguments(
                                 bSpriteCanvas ?
                                 OUString( RTL_CONSTASCII_USTRINGPARAM(
                                               "com.sun.star.rendering.SpriteCanvas.MultiScreen" )) :
                                 OUString( RTL_CONSTASCII_USTRINGPARAM(
                                               "com.sun.star.rendering.Canvas" )),
                                 aArg ),
                             UNO_QUERY );

            }
            else
            {
#endif
                xCanvas.set( xCanvasFactory->createInstanceWithArguments(
                                 bSpriteCanvas ?
                                 OUString( RTL_CONSTASCII_USTRINGPARAM(
                                               "com.sun.star.rendering.SpriteCanvas" )) :
                                 OUString( RTL_CONSTASCII_USTRINGPARAM(
                                               "com.sun.star.rendering.Canvas" )),
                                 aArg ),
                             UNO_QUERY );

#ifdef WNT
            }
#endif

            mpWindowImpl->mxCanvas = xCanvas;
        }
    }

    // no factory??? Empty reference, then.
    return xCanvas;
}

Reference< rendering::XCanvas > Window::GetCanvas() const
{
    return ImplGetCanvas( Size(), false, false );
}

Reference< rendering::XSpriteCanvas > Window::GetSpriteCanvas() const
{
    Reference< rendering::XSpriteCanvas > xSpriteCanvas(
        ImplGetCanvas( Size(), false, true ), uno::UNO_QUERY );
    return xSpriteCanvas;
}

Reference< ::com::sun::star::rendering::XSpriteCanvas > Window::GetFullscreenSpriteCanvas( const Size& rFullscreenSize ) const
{
    Reference< rendering::XSpriteCanvas > xSpriteCanvas(
        ImplGetCanvas( rFullscreenSize, true, true ), uno::UNO_QUERY );
    return xSpriteCanvas;
}

void Window::ImplPaintToDevice( OutputDevice* i_pTargetOutDev, const Point& i_rPos )
{
    BOOL bRVisible = mpWindowImpl->mbReallyVisible;
    mpWindowImpl->mbReallyVisible = mpWindowImpl->mbVisible;
    BOOL bDevOutput = mbDevOutput;
    mbDevOutput = TRUE;

    long nOldDPIX = ImplGetDPIX();
    long nOldDPIY = ImplGetDPIY();
    mnDPIX = i_pTargetOutDev->ImplGetDPIX();
    mnDPIY = i_pTargetOutDev->ImplGetDPIY();
    BOOL bOutput = IsOutputEnabled();
    EnableOutput();

    DBG_ASSERT( GetMapMode().GetMapUnit() == MAP_PIXEL, "MapMode must be PIXEL based" );
    if ( GetMapMode().GetMapUnit() != MAP_PIXEL )
        return;

    // preserve graphicsstate
    Push();
    Region aClipRegion( GetClipRegion() );
    SetClipRegion();

    GDIMetaFile* pOldMtf = GetConnectMetaFile();
    GDIMetaFile aMtf;
    SetConnectMetaFile( &aMtf );

    // put a push action to metafile
    Push();
    // copy graphics state to metafile
    Font aCopyFont = GetFont();
    if( nOldDPIX != mnDPIX || nOldDPIY != mnDPIY )
    {
        aCopyFont.SetHeight( aCopyFont.GetHeight() * mnDPIY / nOldDPIY );
        aCopyFont.SetWidth( aCopyFont.GetWidth() * mnDPIX / nOldDPIX );
    }
    SetFont( aCopyFont );
    SetTextColor( GetTextColor() );
    if( IsLineColor() )
        SetLineColor( GetLineColor() );
    else
        SetLineColor();
    if( IsFillColor() )
        SetFillColor( GetFillColor() );
    else
        SetFillColor();
    if( IsTextLineColor() )
        SetTextLineColor( GetTextLineColor() );
    else
        SetTextLineColor();
    if( IsOverlineColor() )
        SetOverlineColor( GetOverlineColor() );
    else
        SetOverlineColor();
    if( IsTextFillColor() )
        SetTextFillColor( GetTextFillColor() );
    else
        SetTextFillColor();
    SetTextAlign( GetTextAlign() );
    SetRasterOp( GetRasterOp() );
    if( IsRefPoint() )
        SetRefPoint( GetRefPoint() );
    else
        SetRefPoint();
    SetLayoutMode( GetLayoutMode() );
    SetDigitLanguage( GetDigitLanguage() );
    Rectangle aPaintRect( Point( 0, 0 ), GetOutputSizePixel() );
    aClipRegion.Intersect( aPaintRect );
    SetClipRegion( aClipRegion );

    // do the actual paint

    // background
    if( ! IsPaintTransparent() && IsBackground() && ! (GetParentClipMode() & PARENTCLIPMODE_NOCLIP ) )
        Erase();
    // foreground
    Paint( aPaintRect );
    // put a pop action to metafile
    Pop();

    SetConnectMetaFile( pOldMtf );
    EnableOutput( bOutput );
    mpWindowImpl->mbReallyVisible = bRVisible;

    // paint metafile to VDev
    VirtualDevice* pMaskedDevice = new VirtualDevice( *i_pTargetOutDev, 0, 0 );
    pMaskedDevice->SetOutputSizePixel( GetOutputSizePixel() );
    pMaskedDevice->EnableRTL( IsRTLEnabled() );
    aMtf.WindStart();
    aMtf.Play( pMaskedDevice );
    BitmapEx aBmpEx( pMaskedDevice->GetBitmapEx( Point( 0, 0 ), pMaskedDevice->GetOutputSizePixel() ) );
    i_pTargetOutDev->DrawBitmapEx( i_rPos, aBmpEx );
    // get rid of virtual device now so they don't pile up during recursive calls
    delete pMaskedDevice, pMaskedDevice = NULL;


    for( Window* pChild = mpWindowImpl->mpFirstChild; pChild; pChild = pChild->mpWindowImpl->mpNext )
    {
        if( pChild->mpWindowImpl->mpFrame == mpWindowImpl->mpFrame && pChild->IsVisible() )
        {
            long nDeltaX = pChild->mnOutOffX - mnOutOffX;
            if( ImplHasMirroredGraphics() )
                nDeltaX = mnOutWidth - nDeltaX - pChild->mnOutWidth;
            long nDeltaY = pChild->GetOutOffYPixel() - GetOutOffYPixel();
            Point aPos( i_rPos );
            Point aDelta( nDeltaX, nDeltaY );
            aPos += aDelta;
            pChild->ImplPaintToDevice( i_pTargetOutDev, aPos );
        }
    }

    // restore graphics state
    Pop();

    EnableOutput( bOutput );
    mpWindowImpl->mbReallyVisible = bRVisible;
    mbDevOutput = bDevOutput;
    mnDPIX = nOldDPIX;
    mnDPIY = nOldDPIY;
}

void Window::PaintToDevice( OutputDevice* pDev, const Point& rPos, const Size& /*rSize*/ )
{
    // FIXME: scaling: currently this is for pixel copying only

    DBG_ASSERT( ! pDev->ImplHasMirroredGraphics(), "PaintToDevice to mirroring graphics" );
    DBG_ASSERT( ! pDev->IsRTLEnabled(), "PaintToDevice to mirroring device" );

    Window* pRealParent = NULL;
    if( ! mpWindowImpl->mbVisible )
    {
        Window* pTempParent = ImplGetDefaultWindow();
        if( pTempParent )
            pTempParent->EnableChildTransparentMode();
        pRealParent = GetParent();
        SetParent( pTempParent );
        // trigger correct visibility flags for children
        Show();
        Hide();
    }

    BOOL bVisible = mpWindowImpl->mbVisible;
    mpWindowImpl->mbVisible = TRUE;

    if( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->ImplPaintToDevice( pDev, rPos );
    else
        ImplPaintToDevice( pDev, rPos );

    mpWindowImpl->mbVisible = bVisible;

    if( pRealParent )
        SetParent( pRealParent );
}

XubString Window::GetSurroundingText() const
{
  return XubString::EmptyString();
}

Selection Window::GetSurroundingTextSelection() const
{
  return Selection( 0, 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
