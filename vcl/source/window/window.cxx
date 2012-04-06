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

#include <i18npool/mslangid.hxx>

#include "tools/time.hxx"
#include "tools/debug.hxx"
#include "tools/rc.h"

#include "unotools/fontcfg.hxx"
#include "unotools/confignode.hxx"

#include "vcl/unohelp.hxx"
#include "vcl/layout.hxx"
#include "vcl/salgtype.hxx"
#include "vcl/event.hxx"
#include "vcl/help.hxx"
#include "vcl/cursor.hxx"
#include "vcl/svapp.hxx"
#include "vcl/window.hxx"
#include "vcl/syswin.hxx"
#include "vcl/syschild.hxx"
#include "vcl/dockwin.hxx"
#include "vcl/menu.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/wall.hxx"
#include "vcl/gradient.hxx"
#include "vcl/salctype.hxx"
#include "vcl/button.hxx"
#include "vcl/taskpanelist.hxx"
#include "vcl/dialog.hxx"
#include "vcl/unowrap.hxx"
#include "vcl/gdimtf.hxx"
#include "vcl/pdfextoutdevdata.hxx"
#include "vcl/lazydelete.hxx"
#include "vcl/virdev.hxx"

// declare system types in sysdata.hxx
#include "svsys.h"
#include "vcl/sysdata.hxx"

#include "salframe.hxx"
#include "salobj.hxx"
#include "salinst.hxx"
#include "salgdi.hxx"
#include "svdata.hxx"
#include "dbggui.hxx"
#include "outfont.hxx"
#include "window.h"
#include "toolbox.h"
#include "outdev.h"
#include "region.h"
#include "brdwin.hxx"
#include "helpwin.hxx"
#include "sallayout.hxx"
#include "dndlcon.hxx"
#include "dndevdis.hxx"

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

#include <sal/macros.h>
#include <rtl/strbuf.hxx>

#include <set>
#include <typeinfo>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::com::sun::star;
using namespace com::sun;

using ::rtl::OUString;

using ::com::sun::star::awt::XTopWindow;

// =======================================================================

DBG_NAME( Window )

// =======================================================================

#define IMPL_PAINT_PAINT            ((sal_uInt16)0x0001)
#define IMPL_PAINT_PAINTALL         ((sal_uInt16)0x0002)
#define IMPL_PAINT_PAINTALLCHILDREN   ((sal_uInt16)0x0004)
#define IMPL_PAINT_PAINTCHILDREN      ((sal_uInt16)0x0008)
#define IMPL_PAINT_ERASE            ((sal_uInt16)0x0010)
#define IMPL_PAINT_CHECKRTL         ((sal_uInt16)0x0020)

// -----------------------------------------------------------------------

struct ImplCalcToTopData
{
    ImplCalcToTopData*  mpNext;
    Window*             mpWindow;
    Region*             mpInvalidateRegion;
};

ImplAccessibleInfos::ImplAccessibleInfos()
{
    nAccessibleRole = 0xFFFF;
    pAccessibleName = NULL;
    pAccessibleDescription = NULL;
    pLabeledByWindow = NULL;
    pLabelForWindow = NULL;
    pMemberOfWindow = NULL;
}

ImplAccessibleInfos::~ImplAccessibleInfos()
{
    delete pAccessibleName;
    delete pAccessibleDescription;
}

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

    // check window-chain
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
    long nTextWidth = pWindow->GetTextWidth(rtl::OUString("aemnnxEM"));
    long nSymHeight = nTextHeight*4;
    // Make the basis wider if the font is too narrow
    // such that the dialog looks symmetrical and does not become too narrow.
    // Add some extra space when the dialog has the same width,
    // as a little more space is better.
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
        // try to find out whether there is a large correction
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

    // create a text string using the localized text of important buttons
    String aTestText;
    static const StandardButtonType aTestButtons[] =
    {
        BUTTON_OK, BUTTON_CANCEL, BUTTON_CLOSE, BUTTON_ABORT,
        BUTTON_YES, BUTTON_NO, BUTTON_MORE, BUTTON_IGNORE,
        BUTTON_RETRY, BUTTON_HELP
    };

    const int nTestButtonCount = SAL_N_ELEMENTS(aTestButtons);
    for( int n = 0; n < nTestButtonCount; ++n )
    {
        String aButtonStr = Button::GetStandardText( aTestButtons[n] );
        // #i115432# ignore mnemonic+accelerator part of each string
        // TODO: use a string filtering method when it becomes available
        const int nLen = aButtonStr.Len();
        bool bInside = false;
        for( int i = 0; i < nLen; ++i ) {
            const sal_Unicode c = aButtonStr.GetChar( i );
            if( (c == '('))
                bInside = true;
            if( (c == ')'))
                bInside = false;
            if( (c == '~')
            ||  (c == '(') || (c == ')')
            || ((c >= 'A') && (c <= 'Z') && bInside) )
                aButtonStr.SetChar( i, ' ' );
        }
        // append sanitized button text to test string
        aTestText.Append( aButtonStr );
    }

    const int nFirstChar = HasGlyphs( rFont, aTestText );
    const bool bUIFontOk = (nFirstChar >= aTestText.Len());
    return bUIFontOk;
}

// -----------------------------------------------------------------------

void Window::ImplUpdateGlobalSettings( AllSettings& rSettings, sal_Bool bCallHdl )
{
    StyleSettings aTmpSt( rSettings.GetStyleSettings() );
    aTmpSt.SetHighContrastMode( sal_False );
    rSettings.SetStyleSettings( aTmpSt );
    ImplGetFrame()->UpdateSettings( rSettings );
    // reset default border width for layouters
    ImplGetSVData()->maAppData.mnDefaultLayoutBorder = -1;

    // Verify availability of the configured UI font, otherwise choose "Andale Sans UI"
    String aUserInterfaceFont;
    bool bUseSystemFont = rSettings.GetStyleSettings().GetUseSystemUIFonts();

    // check whether system UI font can display a typical UI text
    if( bUseSystemFont )
        bUseSystemFont = ImplCheckUIFont( rSettings.GetStyleSettings().GetAppFont() );

    if ( !bUseSystemFont )
    {
        ImplInitFontList();
        String aConfigFont = utl::DefaultFontConfiguration::get().getUserInterfaceFont( rSettings.GetUILocale() );
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
            String aFallbackFont ("Andale Sans UI" );
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
    bool bBrokenLangFontHeight = MsLangId::isCJK(Application::GetSettings().GetUILanguage());
    if (bBrokenLangFontHeight)
        defFontheight = Max(9, defFontheight);

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
        Gradient aGrad( GradientStyle_LINEAR, DEFAULT_WORKSPACE_GRADIENT_START_COLOR, DEFAULT_WORKSPACE_GRADIENT_END_COLOR );
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
            OUString("org.openoffice.Office.Common/Accessibility") );    // note: case sensitive !
        if ( aNode.isValid() )
        {
            ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString("AutoDetectSystemHC") );
            if( aValue >>= bTmp )
                bAutoHCMode = bTmp;
        }
        if( bAutoHCMode )
        {
            if( rSettings.GetStyleSettings().GetFaceColor().IsDark()
             || rSettings.GetStyleSettings().GetWindowColor().IsDark() )
            {
                aStyleSettings = rSettings.GetStyleSettings();
                aStyleSettings.SetHighContrastMode( sal_True );
                aStyleSettings.SetSymbolsStyle( STYLE_SYMBOLS_HICONTRAST );
                rSettings.SetStyleSettings( aStyleSettings );
            }
        }
    }

    static const char* pEnvHC = getenv( "SAL_FORCE_HC" );
    if( pEnvHC && *pEnvHC )
    {
        aStyleSettings.SetHighContrastMode( sal_True );
        aStyleSettings.SetSymbolsStyle( STYLE_SYMBOLS_HICONTRAST );
        rSettings.SetStyleSettings( aStyleSettings );
    }

#if defined(DBG_UTIL)
    // If needed, set AppFont to bold, in order to check
    // if there is enough space available for texts on other systems
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
    mpWindowImpl->mpExtImpl           = NULL;         // extended implementation data
    mpWindowImpl->mpCursor            = NULL;         // cursor
    mpWindowImpl->mpControlFont       = NULL;         // font propertie
    mpWindowImpl->mpVCLXWindow        = NULL;
    mpWindowImpl->mpAccessibleInfos   = NULL;
    mpWindowImpl->maControlForeground = Color( COL_TRANSPARENT );     // no foreground set
    mpWindowImpl->maControlBackground = Color( COL_TRANSPARENT );     // no background set
    mpWindowImpl->mnLeftBorder        = 0;            // left border
    mpWindowImpl->mnTopBorder         = 0;            // top border
    mpWindowImpl->mnRightBorder       = 0;            // right border
    mpWindowImpl->mnBottomBorder      = 0;            // bottom border
    mpWindowImpl->mnX                 = 0;            // X-Position to Parent
    mpWindowImpl->mnY                 = 0;            // Y-Position to Parent
    mpWindowImpl->mnAbsScreenX        = 0;            // absolute X-position on screen, used for RTL window positioning
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
    mpWindowImpl->mnActivateMode      = 0;            // Will be converted in System/Overlap-Windows
    mpWindowImpl->mnDlgCtrlFlags      = 0;            // DialogControl-Flags
    mpWindowImpl->mnLockCount         = 0;            // LockCount
    mpWindowImpl->meAlwaysInputMode   = AlwaysInputNone; // neither AlwaysEnableInput nor AlwaysDisableInput called
    mpWindowImpl->mbFrame             = sal_False;        // sal_True: Window is a frame window
    mpWindowImpl->mbBorderWin         = sal_False;        // sal_True: Window is a border window
    mpWindowImpl->mbOverlapWin        = sal_False;        // sal_True: Window is a overlap window
    mpWindowImpl->mbSysWin            = sal_False;        // sal_True: SystemWindow is the base class
    mpWindowImpl->mbDialog            = sal_False;        // sal_True: Dialog is the base class
    mpWindowImpl->mbDockWin           = sal_False;        // sal_True: DockingWindow is the base class
    mpWindowImpl->mbFloatWin          = sal_False;        // sal_True: FloatingWindow is the base class
    mpWindowImpl->mbPushButton        = sal_False;        // sal_True: PushButton is the base class
    mpWindowImpl->mbToolBox           = sal_False;      // sal_True: ToolBox is the base class
    mpWindowImpl->mbMenuFloatingWindow= sal_False;      // sal_True: MenuFloatingWindow is the base class
    mpWindowImpl->mbToolbarFloatingWindow= sal_False;       // sal_True: ImplPopupFloatWin is the base class, used for subtoolbars
    mpWindowImpl->mbSplitter          = sal_False;      // sal_True: Splitter is the base class
    mpWindowImpl->mbVisible           = sal_False;        // sal_True: Show( sal_True ) called
    mpWindowImpl->mbOverlapVisible    = sal_False;        // sal_True: Hide called for visible window from ImplHideAllOverlapWindow()
    mpWindowImpl->mbDisabled          = sal_False;        // sal_True: Enable( sal_False ) called
    mpWindowImpl->mbInputDisabled     = sal_False;        // sal_True: EnableInput( sal_False ) called
    mpWindowImpl->mbDropDisabled      = sal_False;        // sal_True: Drop is enabled
    mpWindowImpl->mbNoUpdate          = sal_False;        // sal_True: SetUpdateMode( sal_False ) called
    mpWindowImpl->mbNoParentUpdate    = sal_False;        // sal_True: SetParentUpdateMode( sal_False ) called
    mpWindowImpl->mbActive            = sal_False;        // sal_True: Window Active
    mpWindowImpl->mbParentActive      = sal_False;        // sal_True: OverlapActive from Parent
    mpWindowImpl->mbReallyVisible     = sal_False;        // sal_True: this and all parents to an overlaped window are visible
    mpWindowImpl->mbReallyShown       = sal_False;        // sal_True: this and all parents to an overlaped window are shown
    mpWindowImpl->mbInInitShow        = sal_False;        // sal_True: we are in InitShow
    mpWindowImpl->mbChildNotify       = sal_False;        // sal_True: ChildNotify
    mpWindowImpl->mbChildPtrOverwrite = sal_False;        // sal_True: PointerStyle overwrites Child-Pointer
    mpWindowImpl->mbNoPtrVisible      = sal_False;        // sal_True: ShowPointer( sal_False ) called
    mpWindowImpl->mbMouseMove         = sal_False;        // sal_True: BaseMouseMove called
    mpWindowImpl->mbPaintFrame        = sal_False;        // sal_True: Paint is visible, but not painted
    mpWindowImpl->mbInPaint           = sal_False;        // sal_True: Inside PaintHdl
    mpWindowImpl->mbMouseButtonDown   = sal_False;        // sal_True: BaseMouseButtonDown called
    mpWindowImpl->mbMouseButtonUp     = sal_False;        // sal_True: BaseMouseButtonUp called
    mpWindowImpl->mbKeyInput          = sal_False;        // sal_True: BaseKeyInput called
    mpWindowImpl->mbKeyUp             = sal_False;        // sal_True: BaseKeyUp called
    mpWindowImpl->mbCommand           = sal_False;        // sal_True: BaseCommand called
    mpWindowImpl->mbDefPos            = sal_True;         // sal_True: Position is not Set
    mpWindowImpl->mbDefSize           = sal_True;         // sal_True: Size is not Set
    mpWindowImpl->mbCallMove          = sal_True;         // sal_True: Move must be called by Show
    mpWindowImpl->mbCallResize        = sal_True;         // sal_True: Resize must be called by Show
    mpWindowImpl->mbWaitSystemResize  = sal_True;         // sal_True: Wait for System-Resize
    mpWindowImpl->mbInitWinClipRegion = sal_True;         // sal_True: Calc Window Clip Region
    mpWindowImpl->mbInitChildRegion   = sal_False;        // sal_True: InitChildClipRegion
    mpWindowImpl->mbWinRegion         = sal_False;        // sal_True: Window Region
    mpWindowImpl->mbClipChildren      = sal_False;        // sal_True: Child-window should be clipped
    mpWindowImpl->mbClipSiblings      = sal_False;        // sal_True: Adjacent Child-window should be clipped
    mpWindowImpl->mbChildTransparent  = sal_False;        // sal_True: Child-windows are allowed to switch to transparent (incl. Parent-CLIPCHILDREN)
    mpWindowImpl->mbPaintTransparent  = sal_False;        // sal_True: Paints should be executed on the Parent
    mpWindowImpl->mbMouseTransparent  = sal_False;        // sal_True: Window is transparent for Mouse
    mpWindowImpl->mbDlgCtrlStart      = sal_False;        // sal_True: From here on own Dialog-Control
    mpWindowImpl->mbFocusVisible      = sal_False;        // sal_True: Focus Visible
    mpWindowImpl->mbUseNativeFocus    = sal_False;
    mpWindowImpl->mbNativeFocusVisible= sal_False;        // sal_True: native Focus Visible
    mpWindowImpl->mbInShowFocus       = sal_False;        // prevent recursion
    mpWindowImpl->mbInHideFocus       = sal_False;        // prevent recursion
    mpWindowImpl->mbTrackVisible      = sal_False;        // sal_True: Tracking Visible
    mpWindowImpl->mbControlForeground = sal_False;        // sal_True: Foreground-Property set
    mpWindowImpl->mbControlBackground = sal_False;        // sal_True: Background-Property set
    mpWindowImpl->mbAlwaysOnTop       = sal_False;        // sal_True: always visible for all others windows
    mpWindowImpl->mbCompoundControl   = sal_False;        // sal_True: Composite Control => Listener...
    mpWindowImpl->mbCompoundControlHasFocus = sal_False;  // sal_True: Composite Control has focus somewhere
    mpWindowImpl->mbPaintDisabled     = sal_False;        // sal_True: Paint should not be executed
    mpWindowImpl->mbAllResize         = sal_False;        // sal_True: Also sent ResizeEvents with 0,0
    mpWindowImpl->mbInDtor            = sal_False;        // sal_True: We're still in Window-Dtor
    mpWindowImpl->mbExtTextInput      = sal_False;        // sal_True: ExtTextInput-Mode is active
    mpWindowImpl->mbInFocusHdl        = sal_False;        // sal_True: Within GetFocus-Handler
    mpWindowImpl->mbCreatedWithToolkit = sal_False;
    mpWindowImpl->mbSuppressAccessibilityEvents = sal_False; // sal_True: do not send any accessibility events
    mpWindowImpl->mbDrawSelectionBackground = sal_False;    // sal_True: draws transparent window background to indicate (toolbox) selection
    mpWindowImpl->mbIsInTaskPaneList = sal_False;           // sal_True: window was added to the taskpanelist in the topmost system window
    mpWindowImpl->mnNativeBackground  = 0;              // initialize later, depends on type
    mpWindowImpl->mbCallHandlersDuringInputDisabled = sal_False; // sal_True: call event handlers even if input is disabled
    mpWindowImpl->mbDisableAccessibleLabelForRelation = sal_False; // sal_True: do not set LabelFor relation on accessible objects
    mpWindowImpl->mbDisableAccessibleLabeledByRelation = sal_False; // sal_True: do not set LabeledBy relation on accessible objects
    mpWindowImpl->mbHelpTextDynamic = sal_False;          // sal_True: append help id in HELP_DEBUG case
    mpWindowImpl->mbFakeFocusSet = sal_False; // sal_True: pretend as if the window has focus.

    mbEnableRTL         = Application::GetSettings().GetLayoutRTL();         // sal_True: this outdev will be mirrored if RTL window layout (UI mirroring) is globally active
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
        sal_uInt16 nBorderTypeStyle = 0;
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
        mpWindowImpl->mbOverlapWin  = sal_True;
        mpWindowImpl->mbFrame = sal_True;
    }

    // insert window in list
    ImplInsertWindow( pParent );
    mpWindowImpl->mnStyle = nStyle;

    // Overlap-Window-Data
    if ( mpWindowImpl->mbOverlapWin )
    {
        mpWindowImpl->mpOverlapData                   = new ImplOverlapData;
        mpWindowImpl->mpOverlapData->mpSaveBackDev    = NULL;
        mpWindowImpl->mpOverlapData->mpSaveBackRgn    = NULL;
        mpWindowImpl->mpOverlapData->mpNextBackWin    = NULL;
        mpWindowImpl->mpOverlapData->mnSaveBackSize   = 0;
        mpWindowImpl->mpOverlapData->mbSaveBack       = sal_False;
        mpWindowImpl->mpOverlapData->mnTopLevel       = 1;
    }

    if( pParent && ! mpWindowImpl->mbFrame )
        mbEnableRTL = pParent->mbEnableRTL;

    // test for frame creation
    if ( mpWindowImpl->mbFrame )
    {
        // create frame
        sal_uLong nFrameStyle = 0;

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
                OUString( "Could not create system window!"  ),
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
        mpWindowImpl->mpFrameData->mbHasFocus         = sal_False;
        mpWindowImpl->mpFrameData->mbInMouseMove      = sal_False;
        mpWindowImpl->mpFrameData->mbMouseIn          = sal_False;
        mpWindowImpl->mpFrameData->mbStartDragCalled  = sal_False;
        mpWindowImpl->mpFrameData->mbNeedSysWindow    = sal_False;
        mpWindowImpl->mpFrameData->mbMinimized        = sal_False;
        mpWindowImpl->mpFrameData->mbStartFocusState  = sal_False;
        mpWindowImpl->mpFrameData->mbInSysObjFocusHdl = sal_False;
        mpWindowImpl->mpFrameData->mbInSysObjToTopHdl = sal_False;
        mpWindowImpl->mpFrameData->mbSysObjFocus      = sal_False;
        mpWindowImpl->mpFrameData->maPaintTimer.SetTimeout( 30 );
        mpWindowImpl->mpFrameData->maPaintTimer.SetTimeoutHdl( LINK( this, Window, ImplHandlePaintHdl ) );
        mpWindowImpl->mpFrameData->maResizeTimer.SetTimeout( 50 );
        mpWindowImpl->mpFrameData->maResizeTimer.SetTimeoutHdl( LINK( this, Window, ImplHandleResizeTimerHdl ) );
        mpWindowImpl->mpFrameData->mbInternalDragGestureRecognizer = sal_False;

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
            pSVData->maAppData.mbSettingsInit = sal_True;
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
    sal_uInt16 nScreenZoom = rStyleSettings.GetScreenZoom();
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
        mpWindowImpl->mbFrame         = sal_False;

        // search overlap window and insert window in list
        if ( ImplIsOverlapWindow() )
        {
            Window* pFirstOverlapParent = pParent;
            while ( !pFirstOverlapParent->ImplIsOverlapWindow() )
                pFirstOverlapParent = pFirstOverlapParent->ImplGetParent();
            mpWindowImpl->mpOverlapWindow = pFirstOverlapParent;

            mpWindowImpl->mpNextOverlap = mpWindowImpl->mpFrameData->mpFirstOverlap;
            mpWindowImpl->mpFrameData->mpFirstOverlap = this;

            // Overlap-Windows are by default the uppermost
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

void Window::ImplRemoveWindow( sal_Bool bRemoveFrameData )
{
    // remove window from the lists
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
        // release the graphic
        ImplReleaseGraphics();
    }
}

void Window::reorderWithinParent(sal_uInt16 nNewPosition)
{
    sal_uInt16 nChildCount = 0;
    Window *pSource = mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild;
    while (pSource)
    {
        if (nChildCount == nNewPosition)
            break;
        pSource = pSource->mpWindowImpl->mpNext;
        nChildCount++;
    }

    if (pSource == this) //already at the right place
        return;

    ImplRemoveWindow(false);

    if (pSource)
    {
        mpWindowImpl->mpNext = pSource;
        mpWindowImpl->mpPrev = pSource->mpWindowImpl->mpPrev;
        pSource->mpWindowImpl->mpPrev = this;
    }
    else
        mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = this;

    if (mpWindowImpl->mpPrev)
        mpWindowImpl->mpPrev->mpWindowImpl->mpNext = this;
    else
        mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = this;
}

// -----------------------------------------------------------------------

void Window::ImplCallResize()
{
    mpWindowImpl->mbCallResize = sal_False;

    if( GetBackground().IsGradient() )
        Invalidate();

    Resize();

    // #88419# Most classes don't call the base class in Resize() and Move(),
    // => Call ImpleResize/Move instead of Resize/Move directly...
    ImplCallEventListeners( VCLEVENT_WINDOW_RESIZE );

    ImplExtResize();
}

// -----------------------------------------------------------------------

void Window::ImplCallMove()
{
    mpWindowImpl->mbCallMove = sal_False;

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

static rtl::OString ImplAutoHelpID( ResMgr* pResMgr )
{
    rtl::OString aRet;

    if( pResMgr && Application::IsAutoHelpIdEnabled() )
        aRet = pResMgr->GetAutoHelpId();

    return aRet;
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
    sal_uLong nObjMask = ReadLongRes();

    // we need to calculate auto helpids before the resource gets closed
    // if the resource  only contains flags, it will be closed before we try to read a help id
    // so we always create an auto help id that might be overwritten later
    // HelpId
    rtl::OString aHelpId = ImplAutoHelpID( rResId.GetResMgr() );

    // ResourceStyle
    sal_uLong nRSStyle = ReadLongRes();
    // WinBits
    ReadLongRes();

    if( nObjMask & WINDOW_HELPID )
        aHelpId = ReadByteStringRes();

    SetHelpId( aHelpId );

    sal_Bool  bPos  = sal_False;
    sal_Bool  bSize = sal_False;
    Point aPos;
    Size  aSize;

    if ( nObjMask & (WINDOW_XYMAPMODE | WINDOW_X | WINDOW_Y) )
    {
        // use size as per resource
        MapUnit ePosMap = MAP_PIXEL;

        bPos = sal_True;

        if ( nObjMask & WINDOW_XYMAPMODE )
            ePosMap = (MapUnit)ReadLongRes();
        if ( nObjMask & WINDOW_X )
            aPos.X() = ImplLogicUnitToPixelX( ReadLongRes(), ePosMap );
        if ( nObjMask & WINDOW_Y )
            aPos.Y() = ImplLogicUnitToPixelY( ReadLongRes(), ePosMap );
    }

    if ( nObjMask & (WINDOW_WHMAPMODE | WINDOW_WIDTH | WINDOW_HEIGHT) )
    {
        // use size as per resource
        MapUnit eSizeMap = MAP_PIXEL;

        bSize = sal_True;

        if ( nObjMask & WINDOW_WHMAPMODE )
            eSizeMap = (MapUnit)ReadLongRes();
        if ( nObjMask & WINDOW_WIDTH )
            aSize.Width() = ImplLogicUnitToPixelX( ReadLongRes(), eSizeMap );
        if ( nObjMask & WINDOW_HEIGHT )
            aSize.Height() = ImplLogicUnitToPixelY( ReadLongRes(), eSizeMap );
    }

    // looks bad due to optimisation
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
        Enable( sal_False );

    if ( nObjMask & WINDOW_TEXT )
        SetText( ReadStringRes() );
    if ( nObjMask & WINDOW_HELPTEXT )
    {
        SetHelpText( ReadStringRes() );
        mpWindowImpl->mbHelpTextDynamic = sal_True;
    }
    if ( nObjMask & WINDOW_QUICKTEXT )
        SetQuickHelpText( ReadStringRes() );
    if ( nObjMask & WINDOW_EXTRALONG )
    {
        sal_uIntPtr nRes = ReadLongRes();
        SetData( (void*)nRes );
    }
    if ( nObjMask & WINDOW_UNIQUEID )
        SetUniqueId( ReadByteStringRes() );

    if ( nObjMask & WINDOW_BORDER_STYLE )
    {
        sal_uInt16 nBorderStyle = (sal_uInt16)ReadLongRes();
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
        mpWindowImpl->mpWinData->mnIsTopWindow  = (sal_uInt16) ~0;  // not initialized yet, 0/1 will indicate TopWindow (see IsTopWindow())
        mpWindowImpl->mpWinData->mbMouseOver      = sal_False;
        mpWindowImpl->mpWinData->mbEnableNativeWidget = (pNoNWF && *pNoNWF) ? sal_False : sal_True; // sal_True: try to draw this control with native theme API
   }

    return mpWindowImpl->mpWinData;
}

// -----------------------------------------------------------------------

SalGraphics* Window::ImplGetFrameGraphics() const
{
    if ( mpWindowImpl->mpFrameWindow->mpGraphics )
        mpWindowImpl->mpFrameWindow->mbInitClipRegion = sal_True;
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

    // first check all overlapping windows
    pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        pFindWindow = pTempWindow->ImplFindWindow( rFramePos );
        if ( pFindWindow )
            return pFindWindow;
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }

    // then we check our window
    if ( !mpWindowImpl->mbVisible )
        return NULL;

    sal_uInt16 nHitTest = ImplHitTest( rFramePos );
    if ( nHitTest & WINDOW_HITTEST_INSIDE )
    {
        // and then we check all child windows
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

sal_uInt16 Window::ImplHitTest( const Point& rFramePos )
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

    sal_uInt16 nHitTest = WINDOW_HITTEST_INSIDE;
    if ( mpWindowImpl->mbMouseTransparent )
        nHitTest |= WINDOW_HITTEST_TRANSPARENT;
    return nHitTest;
}

// -----------------------------------------------------------------------

sal_Bool Window::ImplIsRealParentPath( const Window* pWindow ) const
{
    pWindow = pWindow->GetParent();
    while ( pWindow )
    {
        if ( pWindow == this )
            return sal_True;
        pWindow = pWindow->GetParent();
    }

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Window::ImplIsChild( const Window* pWindow, sal_Bool bSystemWindow ) const
{
    do
    {
        if ( !bSystemWindow && pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();

        if ( pWindow == this )
            return sal_True;
    }
    while ( pWindow );

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Window::ImplIsWindowOrChild( const Window* pWindow, sal_Bool bSystemWindow ) const
{
    if ( this == pWindow )
        return sal_True;
    return ImplIsChild( pWindow, bSystemWindow );
}

// -----------------------------------------------------------------------

int Window::ImplTestMousePointerSet()
{
    // as soon as mouse is captured, switch mouse-pointer
    if ( IsMouseCaptured() )
        return sal_True;

    // if the mouse is over the window, switch it
    Rectangle aClientRect( Point( 0, 0 ), GetOutputSizePixel() );
    if ( aClientRect.IsInside( GetPointerPosPixel() ) )
        return sal_True;

    return sal_False;
}

// -----------------------------------------------------------------------

PointerStyle Window::ImplGetMousePointer() const
{
    PointerStyle    ePointerStyle;
    sal_Bool            bWait = sal_False;

    if ( IsEnabled() && IsInputEnabled() && ! IsInModalMode() )
        ePointerStyle = GetPointer().GetStyle();
    else
        ePointerStyle = POINTER_ARROW;

    const Window* pWindow = this;
    do
    {
        // when the pointer is not visible stop the search, as
        // this status should not be overwritten
        if ( pWindow->mpWindowImpl->mbNoPtrVisible )
            return POINTER_NULL;

        if ( !bWait )
        {
            if ( pWindow->mpWindowImpl->mnWaitCount )
            {
                ePointerStyle = POINTER_WAIT;
                bWait = sal_True;
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
    sal_Bool bBecameReallyInvisible = mpWindowImpl->mbReallyVisible;

    mbDevOutput     = sal_False;
    mpWindowImpl->mbReallyVisible = sal_False;
    mpWindowImpl->mbReallyShown   = sal_False;

    // the SHOW/HIDE events serve as indicators to send child creation/destroy events to the access bridge.
    // For this, the data member of the event must not be NULL.
    // Previously, we did this in Window::Show, but there some events got lost in certain situations.
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

    sal_Bool bBecameReallyVisible = !mpWindowImpl->mbReallyVisible;

    mbDevOutput     = sal_True;
    mpWindowImpl->mbReallyVisible = sal_True;
    mpWindowImpl->mbReallyShown   = sal_True;

    // the SHOW/HIDE events serve as indicators to send child creation/destroy events to the access bridge.
    // For this, the data member of the event must not be NULL.
    // Previously, we did this in Window::Show, but there some events got lost in certain situations. Now
    // we're doing it when the visibility really changes
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
    mpWindowImpl->mbReallyShown   = sal_True;
    mpWindowImpl->mbInInitShow    = sal_True;
    StateChanged( STATE_CHANGE_INITSHOW );
    mpWindowImpl->mbInInitShow    = sal_False;

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
    // recalculate AppFont-resolution and DPI-resolution
    if ( mpWindowImpl->mbFrame )
    {
        const StyleSettings& rStyleSettings = maSettings.GetStyleSettings();
        sal_uInt16 nScreenZoom = rStyleSettings.GetScreenZoom();
        mnDPIX = (mpWindowImpl->mpFrameData->mnDPIX*nScreenZoom)/100;
        mnDPIY = (mpWindowImpl->mpFrameData->mnDPIY*nScreenZoom)/100;
        SetPointFont( rStyleSettings.GetAppFont() );
    }
    else if ( mpWindowImpl->mpParent )
    {
        mnDPIX  = mpWindowImpl->mpParent->mnDPIX;
        mnDPIY  = mpWindowImpl->mpParent->mnDPIY;
    }

    // update the recalculated values for logical units
    // and also tools belonging to the values
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
    sal_uInt16  nScreenFontZoom = maSettings.GetStyleSettings().GetScreenFontZoom();

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
    sal_uInt16  nScreenFontZoom = maSettings.GetStyleSettings().GetScreenFontZoom();

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

sal_Bool Window::ImplSysObjClip( const Region* pOldRegion )
{
    sal_Bool bUpdate = sal_True;

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
                sal_uInt16      nClipFlags = mpWindowImpl->mpSysObj->GetClipRegionType();

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
                    sal_uLong               nRectCount;
                    ImplRegionInfo      aInfo;
                    sal_Bool                bRegionRect;

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
                bVisibleState = sal_False;
        }

        // Visible-Status updaten
        mpWindowImpl->mpSysObj->Show( bVisibleState );
    }

    return bUpdate;
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjChildrenClip()
{
    if ( mpWindowImpl->mpSysObj && mpWindowImpl->mbInitWinClipRegion )
        ImplSysObjClip( NULL );

    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        pWindow->ImplUpdateSysObjChildrenClip();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjOverlapsClip()
{
    ImplUpdateSysObjChildrenClip();

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
        ImplUpdateSysObjChildrenClip();

        // siblings should recalculate their clip region
        if ( mpWindowImpl->mbClipSiblings )
        {
            Window* pWindow = mpWindowImpl->mpNext;
            while ( pWindow )
            {
                pWindow->ImplUpdateSysObjChildrenClip();
                pWindow = pWindow->mpWindowImpl->mpNext;
            }
        }
    }
    else
        mpWindowImpl->mpFrameWindow->ImplUpdateSysObjOverlapsClip();
}

// -----------------------------------------------------------------------

sal_Bool Window::ImplSetClipFlagChildren( sal_Bool bSysObjOnlySmaller )
{
    sal_Bool bUpdate = sal_True;
    if ( mpWindowImpl->mpSysObj )
    {
        Region* pOldRegion = NULL;
        if ( bSysObjOnlySmaller && !mpWindowImpl->mbInitWinClipRegion )
            pOldRegion = new Region( mpWindowImpl->maWinClipRegion );

        mbInitClipRegion = sal_True;
        mpWindowImpl->mbInitWinClipRegion = sal_True;

        Window* pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            if ( !pWindow->ImplSetClipFlagChildren( bSysObjOnlySmaller ) )
                bUpdate = sal_False;
            pWindow = pWindow->mpWindowImpl->mpNext;
        }

        if ( !ImplSysObjClip( pOldRegion ) )
        {
            mbInitClipRegion = sal_True;
            mpWindowImpl->mbInitWinClipRegion = sal_True;
            bUpdate = sal_False;
        }

        delete pOldRegion;
    }
    else
    {
    mbInitClipRegion = sal_True;
    mpWindowImpl->mbInitWinClipRegion = sal_True;

    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagChildren( bSysObjOnlySmaller ) )
            bUpdate = sal_False;
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
    }
    return bUpdate;
}

// -----------------------------------------------------------------------

sal_Bool Window::ImplSetClipFlagOverlapWindows( sal_Bool bSysObjOnlySmaller )
{
    sal_Bool bUpdate = ImplSetClipFlagChildren( bSysObjOnlySmaller );

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller ) )
            bUpdate = sal_False;
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    return bUpdate;
}

// -----------------------------------------------------------------------

sal_Bool Window::ImplSetClipFlag( sal_Bool bSysObjOnlySmaller )
{
    if ( !ImplIsOverlapWindow() )
    {
        sal_Bool bUpdate = ImplSetClipFlagChildren( bSysObjOnlySmaller );

        Window* pParent = ImplGetParent();
        if ( pParent &&
             ((pParent->GetStyle() & WB_CLIPCHILDREN) || (mpWindowImpl->mnParentClipMode & PARENTCLIPMODE_CLIP)) )
        {
            pParent->mbInitClipRegion = sal_True;
            pParent->mpWindowImpl->mbInitChildRegion = sal_True;
        }

        // siblings should recalculate their clip region
        if ( mpWindowImpl->mbClipSiblings )
        {
            Window* pWindow = mpWindowImpl->mpNext;
            while ( pWindow )
            {
                if ( !pWindow->ImplSetClipFlagChildren( bSysObjOnlySmaller ) )
                    bUpdate = sal_False;
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

void Window::ImplClipBoundaries( Region& rRegion, sal_Bool bThis, sal_Bool bOverlaps )
{
    if ( bThis )
        ImplIntersectWindowClipRegion( rRegion );
    else if ( ImplIsOverlapWindow() )
    {
        // clip to frame if required
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

sal_Bool Window::ImplClipChildren( Region& rRegion )
{
    sal_Bool    bOtherClip = sal_False;
    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
        {
            // read-out ParentClipMode-Flags
            sal_uInt16 nClipMode = pWindow->GetParentClipMode();
            if ( !(nClipMode & PARENTCLIPMODE_NOCLIP) &&
                 ((nClipMode & PARENTCLIPMODE_CLIP) || (GetStyle() & WB_CLIPCHILDREN)) )
                pWindow->ImplExcludeWindowRegion( rRegion );
            else
                bOtherClip = sal_True;
        }

        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    return bOtherClip;
}

// -----------------------------------------------------------------------

void Window::ImplClipAllChildren( Region& rRegion )
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
    ImplClipBoundaries( mpWindowImpl->maWinClipRegion, sal_False, sal_True );

    // Clip Children
    if ( (GetStyle() & WB_CLIPCHILDREN) || mpWindowImpl->mbClipChildren )
        mpWindowImpl->mbInitChildRegion = sal_True;

    mpWindowImpl->mbInitWinClipRegion = sal_False;
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

        ImplClipChildren( *mpWindowImpl->mpChildClipRegion );
    }

    mpWindowImpl->mbInitChildRegion = sal_False;
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
                                    sal_Bool bChildren, sal_Bool bParent, sal_Bool bSiblings )
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

    if ( bChildren )
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

void Window::ImplCallPaint( const Region* pRegion, sal_uInt16 nPaintFlags )
{
    // call PrePaint. PrePaint may add to the invalidate region as well as
    // other parameters used below.
    PrePaint();

    mpWindowImpl->mbPaintFrame = sal_False;

    if ( nPaintFlags & IMPL_PAINT_PAINTALLCHILDREN )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINT | IMPL_PAINT_PAINTALLCHILDREN | (nPaintFlags & IMPL_PAINT_PAINTALL);
    if ( nPaintFlags & IMPL_PAINT_PAINTCHILDREN )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTCHILDREN;
    if ( nPaintFlags & IMPL_PAINT_ERASE )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_ERASE;
    if ( nPaintFlags & IMPL_PAINT_CHECKRTL )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_CHECKRTL;
    if ( !mpWindowImpl->mpFirstChild )
        mpWindowImpl->mnPaintFlags &= ~IMPL_PAINT_PAINTALLCHILDREN;

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

            if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDREN )
                pChildRegion = new Region( mpWindowImpl->maInvalidateRegion );
            mpWindowImpl->maInvalidateRegion.Intersect( *pWinChildClipRegion );
        }
        mpWindowImpl->mnPaintFlags = 0;
        if ( !mpWindowImpl->maInvalidateRegion.IsEmpty() )
        {
            bool bRestoreCursor = false;
            if ( mpWindowImpl->mpCursor )
                bRestoreCursor = mpWindowImpl->mpCursor->ImplSuspend();

            mbInitClipRegion = sal_True;
            mpWindowImpl->mbInPaint = sal_True;

            // restore Paint-Region
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
            mpWindowImpl->mbInPaint = sal_False;
            mbInitClipRegion = sal_True;
            mpWindowImpl->mpPaintRegion = NULL;
            if ( mpWindowImpl->mpCursor )
                mpWindowImpl->mpCursor->ImplResume( bRestoreCursor );
        }
    }
    else
        mpWindowImpl->mnPaintFlags = 0;

    if ( nPaintFlags & (IMPL_PAINT_PAINTALLCHILDREN | IMPL_PAINT_PAINTCHILDREN) )
    {
        // Paint from the bottom child window and frontward.
        Window* pTempWindow = mpWindowImpl->mpLastChild;
        while ( pTempWindow )
        {
            if ( pTempWindow->mpWindowImpl->mbVisible )
                pTempWindow->ImplCallPaint( pChildRegion, nPaintFlags );
            pTempWindow = pTempWindow->mpWindowImpl->mpPrev;
        }
    }

    if ( mpWindowImpl->mpWinData && mpWindowImpl->mbTrackVisible && (mpWindowImpl->mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
        /* #98602# need to invert the tracking rect AFTER
        * the children have painted
        */
        InvertTracking( *(mpWindowImpl->mpWinData->mpTrackRect), mpWindowImpl->mpWinData->mnTrackFlags );

    // #98943# draw toolbox selection
    if( !aSelectionRect.IsEmpty() )
        DrawSelectionBackground( aSelectionRect, 3, sal_False, sal_True, sal_False );

    delete pChildRegion;
}

// -----------------------------------------------------------------------

void Window::ImplCallOverlapPaint()
{
    // emit overlapping windows first
    Window* pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->mpWindowImpl->mbReallyVisible )
            pTempWindow->ImplCallOverlapPaint();
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }

    // only then ourself
    if ( mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDREN) )
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

IMPL_LINK_NOARG(Window, ImplHandlePaintHdl)
{
    // save paint events until layout is done
    if (IsDialog() && static_cast<const Dialog*>(this)->hasPendingLayout())
    {
        mpWindowImpl->mpFrameData->maPaintTimer.Start();
        return 0;
    }

    // save paint events until resizing is done
    if( mpWindowImpl->mbFrame && mpWindowImpl->mpFrameData->maResizeTimer.IsActive() )
        mpWindowImpl->mpFrameData->maPaintTimer.Start();
    else if ( mpWindowImpl->mbReallyVisible )
        ImplCallOverlapPaint();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(Window, ImplHandleResizeTimerHdl)
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

void Window::ImplInvalidateFrameRegion( const Region* pRegion, sal_uInt16 nFlags )
{
    // set PAINTCHILDREN for all parent windows till the first OverlapWindow
    if ( !ImplIsOverlapWindow() )
    {
        Window* pTempWindow = this;
        sal_uInt16 nTranspPaint = IsPaintTransparent() ? IMPL_PAINT_PAINT : 0;
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTCHILDREN )
                break;
            pTempWindow->mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTCHILDREN | nTranspPaint;
            if( ! pTempWindow->IsPaintTransparent() )
                nTranspPaint = 0;
        }
        while ( !pTempWindow->ImplIsOverlapWindow() );
    }

    // set Paint-Flags
    mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINT;
    if ( nFlags & INVALIDATE_CHILDREN )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTALLCHILDREN;
    if ( !(nFlags & INVALIDATE_NOERASE) )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_ERASE;
    if ( !pRegion )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTALL;

    // if not everything has to be redrawn, add the region to it
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

    ImplClipBoundaries( aRegion, sal_True, sal_True );
    if ( !aRegion.IsEmpty() )
        ImplInvalidateFrameRegion( &aRegion, INVALIDATE_CHILDREN );

    // now we invalidate the overlapping windows
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

void Window::ImplInvalidate( const Region* pRegion, sal_uInt16 nFlags )
{

    // reset background storage
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    // check what has to be redrawn
    sal_Bool bInvalidateAll = !pRegion;

    // take Transparent-Invalidate into account
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
                bInvalidateAll = sal_False;
                break;
            }

            if ( pTempWindow->ImplIsOverlapWindow() )
                break;

            pTempWindow = pTempWindow->ImplGetParent();
        }
    }

    // assemble region
    sal_uInt16 nOrgFlags = nFlags;
    if ( !(nFlags & (INVALIDATE_CHILDREN | INVALIDATE_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= INVALIDATE_NOCHILDREN;
        else
            nFlags |= INVALIDATE_CHILDREN;
    }
    if ( (nFlags & INVALIDATE_NOCHILDREN) && mpWindowImpl->mpFirstChild )
        bInvalidateAll = sal_False;
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
        ImplClipBoundaries( aRegion, sal_True, sal_True );
        if ( nFlags & INVALIDATE_NOCHILDREN )
        {
            nFlags &= ~INVALIDATE_CHILDREN;
            if ( !(nFlags & INVALIDATE_NOCLIPCHILDREN) )
            {
                if ( nOrgFlags & INVALIDATE_NOCHILDREN )
                    ImplClipAllChildren( aRegion );
                else
                {
                    if ( ImplClipChildren( aRegion ) )
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
                                       sal_Bool bChildren )
{
    if ( (mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTALL)) == IMPL_PAINT_PAINT )
    {
        Region aTempRegion = mpWindowImpl->maInvalidateRegion;
        aTempRegion.Intersect( rRect );
        aTempRegion.Move( nHorzScroll, nVertScroll );
        mpWindowImpl->maInvalidateRegion.Union( aTempRegion );
    }

    if ( bChildren && (mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTCHILDREN) )
    {
        Window* pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            pWindow->ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, sal_True );
            pWindow = pWindow->mpWindowImpl->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplMoveAllInvalidateRegions( const Rectangle& rRect,
                                           long nHorzScroll, long nVertScroll,
                                           sal_Bool bChildren )
{
    // also shift Paint-Region when paints need processing
    ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, bChildren );
    // Paint-Region should be shifted, as drawn by the parents
    if ( !ImplIsOverlapWindow() )
    {
        Region  aPaintAllRegion;
        Window* pPaintAllWindow = this;
        do
        {
            pPaintAllWindow = pPaintAllWindow->ImplGetParent();
            if ( pPaintAllWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDREN )
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
            sal_uInt16 nPaintFlags = 0;
            if ( bChildren )
                mpWindowImpl->mnPaintFlags |= INVALIDATE_CHILDREN;
            ImplInvalidateFrameRegion( &aPaintAllRegion, nPaintFlags );
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplValidateFrameRegion( const Region* pRegion, sal_uInt16 nFlags )
{
    if ( !pRegion )
        mpWindowImpl->maInvalidateRegion.SetEmpty();
    else
    {
        // when all child windows have to be drawn we need to invalidate them before doing so
        if ( (mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDREN) && mpWindowImpl->mpFirstChild )
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

void Window::ImplValidate( const Region* pRegion, sal_uInt16 nFlags )
{
    // assemble region
    sal_Bool    bValidateAll = !pRegion;
    sal_uInt16  nOrgFlags = nFlags;
    if ( !(nFlags & (VALIDATE_CHILDREN | VALIDATE_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= VALIDATE_NOCHILDREN;
        else
            nFlags |= VALIDATE_CHILDREN;
    }
    if ( (nFlags & VALIDATE_NOCHILDREN) && mpWindowImpl->mpFirstChild )
        bValidateAll = sal_False;
    if ( bValidateAll )
        ImplValidateFrameRegion( NULL, nFlags );
    else
    {
        Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aRegion( aRect );
        if ( pRegion )
            aRegion.Intersect( *pRegion );
        ImplClipBoundaries( aRegion, sal_True, sal_True );
        if ( nFlags & VALIDATE_NOCHILDREN )
        {
            nFlags &= ~VALIDATE_CHILDREN;
            if ( nOrgFlags & VALIDATE_NOCHILDREN )
                ImplClipAllChildren( aRegion );
            else
            {
                if ( ImplClipChildren( aRegion ) )
                    nFlags |= VALIDATE_CHILDREN;
            }
        }
        if ( !aRegion.IsEmpty() )
            ImplValidateFrameRegion( &aRegion, nFlags );
    }
}

// -----------------------------------------------------------------------

void Window::ImplScroll( const Rectangle& rRect,
                         long nHorzScroll, long nVertScroll, sal_uInt16 nFlags )
{
    if ( !IsDeviceOutputNecessary() )
        return;

    nHorzScroll = ImplLogicWidthToDevicePixel( nHorzScroll );
    nVertScroll = ImplLogicHeightToDevicePixel( nVertScroll );

    if ( !nHorzScroll && !nVertScroll )
        return;

    // restore background storage
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    if ( mpWindowImpl->mpCursor )
        mpWindowImpl->mpCursor->ImplSuspend();

    sal_uInt16 nOrgFlags = nFlags;
    if ( !(nFlags & (SCROLL_CHILDREN | SCROLL_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= SCROLL_NOCHILDREN;
        else
            nFlags |= SCROLL_CHILDREN;
    }

    Region  aInvalidateRegion;
    sal_Bool    bScrollChildren = (nFlags & SCROLL_CHILDREN) != 0;
    sal_Bool    bErase = (nFlags & SCROLL_NOERASE) == 0;

    if ( !mpWindowImpl->mpFirstChild )
        bScrollChildren = sal_False;

    // --- RTL --- check if this window requires special action
    sal_Bool bReMirror = ( ImplIsAntiparallel() );

    Rectangle aRectMirror( rRect );
    if( bReMirror )
    {
        // --- RTL --- make sure the invalidate region of this window is
        // computed in the same coordinate space as the one from the overlap windows
        ImplReMirror( aRectMirror );
    }

    // adapt paint areas
    ImplMoveAllInvalidateRegions( aRectMirror, nHorzScroll, nVertScroll, bScrollChildren );

    if ( !(nFlags & SCROLL_NOINVALIDATE) )
    {
        ImplCalcOverlapRegion( aRectMirror, aInvalidateRegion, !bScrollChildren, sal_True, sal_False );

        // --- RTL ---
        // if the scrolling on the device is performed in the opposite direction
        // then move the overlaps in that direction to compute the invalidate region
        // on the correct side, i.e., revert nHorzScroll

        if ( !aInvalidateRegion.IsEmpty() )
        {
            aInvalidateRegion.Move( bReMirror ? -nHorzScroll : nHorzScroll, nVertScroll );
            bErase = sal_True;
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

    ImplClipBoundaries( aRegion, sal_False, sal_True );
    if ( !bScrollChildren )
    {
        if ( nOrgFlags & SCROLL_NOCHILDREN )
            ImplClipAllChildren( aRegion );
        else
            ImplClipChildren( aRegion );
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

        sal_uInt16 nPaintFlags = INVALIDATE_CHILDREN;
        if ( !bErase )
            nPaintFlags |= INVALIDATE_NOERASE;
        if ( !bScrollChildren )
        {
            if ( nOrgFlags & SCROLL_NOCHILDREN )
                ImplClipAllChildren( aInvalidateRegion );
            else
                ImplClipChildren( aInvalidateRegion );
        }
        ImplInvalidateFrameRegion( &aInvalidateRegion, nPaintFlags );
    }

    if ( bScrollChildren )
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
        mpWindowImpl->mpCursor->ImplResume();
}

// -----------------------------------------------------------------------

void Window::ImplUpdateAll( sal_Bool bOverlapWindows )
{
    if ( !mpWindowImpl->mbReallyVisible )
        return;

    sal_Bool bFlush = sal_False;
    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) )
            bFlush = sal_True;
    }

    // an update changes the OverlapWindow, such that for later paints
    // not too much has to be drawn, if ALLCHILDREN etc. is set
    Window* pWindow = ImplGetFirstOverlapWindow();
    if ( bOverlapWindows )
        pWindow->ImplCallOverlapPaint();
    else
    {
        if ( pWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDREN) )
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
        // release graphic
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

void Window::ImplUpdateOverlapWindowPtr( sal_Bool bNewFrame )
{
    sal_Bool bVisible = IsVisible();
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

sal_Bool Window::ImplUpdatePos()
{
    sal_Bool bSysChild = sal_False;

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
            bSysChild = sal_True;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    if ( mpWindowImpl->mpSysObj )
        bSysChild = sal_True;

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
                                long nWidth, long nHeight, sal_uInt16 nFlags )
{
    sal_Bool    bNewPos         = sal_False;
    sal_Bool    bNewSize        = sal_False;
    sal_Bool    bCopyBits       = sal_False;
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
            bCopyBits = sal_True;
    }

    sal_Bool bnXRecycled = sal_False; // avoid duplicate mirroring in RTL case
    if ( nFlags & WINDOW_POSSIZE_WIDTH )
    {
        if(!( nFlags & WINDOW_POSSIZE_X ))
        {
            nX = mpWindowImpl->mnX;
            nFlags |= WINDOW_POSSIZE_X;
            bnXRecycled = sal_True; // we're using a mnX which was already mirrored in RTL case
        }

        if ( nWidth < 0 )
            nWidth = 0;
        if ( nWidth != mnOutWidth )
        {
            mnOutWidth = nWidth;
            bNewSize = sal_True;
            bCopyBits = sal_False;
        }
    }
    if ( nFlags & WINDOW_POSSIZE_HEIGHT )
    {
        if ( nHeight < 0 )
            nHeight = 0;
        if ( nHeight != mnOutHeight )
        {
            mnOutHeight = nHeight;
            bNewSize = sal_True;
            bCopyBits = sal_False;
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
                                       *pOverlapRegion, sal_False, sal_True, sal_True );
            }
            mpWindowImpl->mnX = nX;
            mpWindowImpl->maPos.X() = nOrgX;
            mpWindowImpl->mnAbsScreenX = aPtDev.X();    // --- RTL --- (store real screen pos)
            bNewPos = sal_True;
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
                                       *pOverlapRegion, sal_False, sal_True, sal_True );
            }
            mpWindowImpl->mnY = nY;
            mpWindowImpl->maPos.Y() = nY;
            bNewPos = sal_True;
        }
    }

    if ( bNewPos || bNewSize )
    {
        sal_Bool bUpdateSysObjPos = sal_False;
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
                    mpWindowImpl->mpClientWindow->mpWindowImpl->mbCallMove = sal_True;
                }
            }
        }

        // Move()/Resize() will be called only for Show(), such that
        // at least one is called before Show()
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
                mpWindowImpl->mbCallMove = sal_True;
            if ( bNewSize )
                mpWindowImpl->mbCallResize = sal_True;
        }

        sal_Bool bUpdateSysObjClip = sal_False;
        if ( IsReallyVisible() )
        {
            if ( bNewPos || bNewSize )
            {
                // reset background storage
                if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mpSaveBackDev )
                    ImplDeleteOverlapBackground();
                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                // set Clip-Flag
                bUpdateSysObjClip = !ImplSetClipFlag( sal_True );
            }

            // invalidate window content ?
            if ( bNewPos || (mnOutWidth > nOldOutWidth) || (mnOutHeight > nOldOutHeight) )
            {
                if ( bNewPos )
                {
                    sal_Bool bInvalidate = sal_False;
                    sal_Bool bParentPaint = sal_True;
                    if ( !ImplIsOverlapWindow() )
                        bParentPaint = mpWindowImpl->mpParent->IsPaintEnabled();
                    if ( bCopyBits && bParentPaint && !HasPaintEvent() )
                    {
                        Point aPoint( mnOutOffX, mnOutOffY );
                        Region aRegion( Rectangle( aPoint,
                                                   Size( mnOutWidth, mnOutHeight ) ) );
                        if ( mpWindowImpl->mbWinRegion )
                            aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
                        ImplClipBoundaries( aRegion, sal_False, sal_True );
                        if ( !pOverlapRegion->IsEmpty() )
                        {
                            pOverlapRegion->Move( mnOutOffX-nOldOutOffX, mnOutOffY-nOldOutOffY );
                            aRegion.Exclude( *pOverlapRegion );
                        }
                        if ( !aRegion.IsEmpty() )
                        {
                            // adapt Paint areas
                            ImplMoveAllInvalidateRegions( Rectangle( Point( nOldOutOffX, nOldOutOffY ),
                                                                     Size( nOldOutWidth, nOldOutHeight ) ),
                                                          mnOutOffX-nOldOutOffX, mnOutOffY-nOldOutOffY,
                                                          sal_True );
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
                                    bInvalidate = sal_True;
                            }
                            else
                                bInvalidate = sal_True;
                            if ( !bInvalidate )
                            {
                                if ( !pOverlapRegion->IsEmpty() )
                                    ImplInvalidateFrameRegion( pOverlapRegion, INVALIDATE_CHILDREN );
                            }
                        }
                        else
                            bInvalidate = sal_True;
                    }
                    else
                        bInvalidate = sal_True;
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
                    ImplClipBoundaries( aRegion, sal_False, sal_True );
                    if ( !aRegion.IsEmpty() )
                        ImplInvalidateFrameRegion( &aRegion, INVALIDATE_CHILDREN );
                }
            }

            // invalidate Parent or Overlaps
            if ( bNewPos ||
                 (mnOutWidth < nOldOutWidth) || (mnOutHeight < nOldOutHeight) )
            {
                Region aRegion( *pOldRegion );
                if ( !mpWindowImpl->mbPaintTransparent )
                    ImplExcludeWindowRegion( aRegion );
                ImplClipBoundaries( aRegion, sal_False, sal_True );
                if ( !aRegion.IsEmpty() && !mpWindowImpl->mpBorderWindow )
                    ImplInvalidateParentFrameRegion( aRegion );
            }
        }

        // adapt system objects
        if ( bUpdateSysObjClip )
            ImplUpdateSysObjClip();
        if ( bUpdateSysObjPos )
            ImplUpdateSysObjPos();
        if ( bNewSize && mpWindowImpl->mpSysObj )
            mpWindowImpl->mpSysObj->SetPosSize( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight );
    }

    delete pOverlapRegion;
    delete pOldRegion;
}

// -----------------------------------------------------------------------

void Window::ImplToBottomChild()
{
    if ( !ImplIsOverlapWindow() && !mpWindowImpl->mbReallyVisible && (mpWindowImpl->mpParent->mpWindowImpl->mpLastChild != this) )
    {
        // put the window to the end of the list
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
            // calculate region, where the window overlaps with other windows
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

void Window::ImplToTop( sal_uInt16 nFlags )
{
    DBG_ASSERT( ImplIsOverlapWindow(), "Window::ImplToTop(): Is not a OverlapWindow" );

    if ( mpWindowImpl->mbFrame )
    {
        // on a mouse click in the external window, it is the latter's
        // responsibility to assure our frame is put in front
        if ( !mpWindowImpl->mpFrameData->mbHasFocus &&
             !mpWindowImpl->mpFrameData->mbSysObjFocus &&
             !mpWindowImpl->mpFrameData->mbInSysObjFocusHdl &&
             !mpWindowImpl->mpFrameData->mbInSysObjToTopHdl )
        {
            // do not bring floating windows on the client to top
            if( !ImplGetClientWindow() || !(ImplGetClientWindow()->GetStyle() & WB_SYSTEMFLOATWIN) )
            {
                sal_uInt16 nSysFlags = 0;
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
            // remove window from the list
            mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = mpWindowImpl->mpPrev;

            // take AlwaysOnTop into account
            sal_Bool    bOnTop = IsAlwaysOnTopEnabled();
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

            // check TopLevel
            sal_uInt8 nTopLevel = mpWindowImpl->mpOverlapData->mnTopLevel;
            while ( pNextWin )
            {
                if ( (bOnTop != pNextWin->IsAlwaysOnTopEnabled()) ||
                     (nTopLevel <= pNextWin->mpWindowImpl->mpOverlapData->mnTopLevel) )
                    break;
                pNextWin = pNextWin->mpWindowImpl->mpNext;
            }

            // add the window to the list again
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

            // recalculate ClipRegion of this and all overlapping windows
            if ( IsReallyVisible() )
            {
                // reset background storage
                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                mpWindowImpl->mpOverlapWindow->ImplSetClipFlagOverlapWindows();
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplStartToTop( sal_uInt16 nFlags )
{
    ImplCalcToTopData   aStartData;
    ImplCalcToTopData*  pCurData;
    ImplCalcToTopData*  pNextData;
    Window* pOverlapWindow;
    if ( ImplIsOverlapWindow() )
        pOverlapWindow = this;
    else
        pOverlapWindow = mpWindowImpl->mpOverlapWindow;

    // first calculate paint areas
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
    // next calculate the paint areas of the ChildOverlap windows
    pTempOverlapWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempOverlapWindow )
    {
        pTempOverlapWindow->ImplCalcToTop( pCurData );
        if ( pCurData->mpNext )
            pCurData = pCurData->mpNext;
        pTempOverlapWindow = pTempOverlapWindow->mpWindowImpl->mpNext;
    }

    // and next change the windows list
    pTempOverlapWindow = pOverlapWindow;
    do
    {
        pTempOverlapWindow->ImplToTop( nFlags );
        pTempOverlapWindow = pTempOverlapWindow->mpWindowImpl->mpOverlapWindow;
    }
    while ( !pTempOverlapWindow->mpWindowImpl->mbFrame );
    // as last step invalidate the invalid areas
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

void Window::ImplFocusToTop( sal_uInt16 nFlags, sal_Bool bReallyVisible )
{
    // do we need to fetch the focus?
    if ( !(nFlags & TOTOP_NOGRABFOCUS) )
    {
        // first window with GrabFocus-Activate gets the focus
        Window* pFocusWindow = this;
        while ( !pFocusWindow->ImplIsOverlapWindow() )
        {
            // if the window has no BorderWindow, we
            // should always find the belonging BorderWindow
            if ( !pFocusWindow->mpWindowImpl->mpBorderWindow )
            {
                if ( pFocusWindow->mpWindowImpl->mnActivateMode & ACTIVATE_MODE_GRABFOCUS )
                    break;
            }
            pFocusWindow = pFocusWindow->ImplGetParent();
        }
        if ( (pFocusWindow->mpWindowImpl->mnActivateMode & ACTIVATE_MODE_GRABFOCUS) &&
             !pFocusWindow->HasChildPathFocus( sal_True ) )
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
            pOverlapWindow->mpWindowImpl->mbOverlapVisible = sal_False;
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
            pOverlapWindow->mpWindowImpl->mbOverlapVisible = sal_True;
            pOverlapWindow->Show( false );
        }

        pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallMouseMove( sal_uInt16 nMouseCode, sal_Bool bModChanged )
{
    if ( mpWindowImpl->mpFrameData->mbMouseIn && mpWindowImpl->mpFrameWindow->mpWindowImpl->mbReallyVisible )
    {
        sal_uLong   nTime   = Time::GetSystemTicks();
        long    nX      = mpWindowImpl->mpFrameData->mnLastMouseX;
        long    nY      = mpWindowImpl->mpFrameData->mnLastMouseY;
        sal_uInt16  nCode   = nMouseCode;
        sal_uInt16  nMode   = mpWindowImpl->mpFrameData->mnMouseMode;
        sal_Bool    bLeave;
        // check for MouseLeave
        if ( ((nX < 0) || (nY < 0) ||
              (nX >= mpWindowImpl->mpFrameWindow->mnOutWidth) ||
              (nY >= mpWindowImpl->mpFrameWindow->mnOutHeight)) &&
             !ImplGetSVData()->maWinData.mpCaptureWin )
            bLeave = sal_True;
        else
            bLeave = sal_False;
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

IMPL_LINK_NOARG(Window, ImplGenerateMouseMoveHdl)
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
    sal_Bool        bCallActivate = sal_True;
    sal_Bool        bCallDeactivate = sal_True;

    pOldRealWindow = pOldOverlapWindow->ImplGetWindow();
    pNewRealWindow = pNewOverlapWindow->ImplGetWindow();
    if ( (pOldRealWindow->GetType() != WINDOW_FLOATINGWINDOW) ||
         pOldRealWindow->GetActivateMode() )
    {
        if ( (pNewRealWindow->GetType() == WINDOW_FLOATINGWINDOW) &&
             !pNewRealWindow->GetActivateMode() )
        {
            pSVData->maWinData.mpLastDeacWin = pOldOverlapWindow;
            bCallDeactivate = sal_False;
        }
    }
    else if ( (pNewRealWindow->GetType() != WINDOW_FLOATINGWINDOW) ||
              pNewRealWindow->GetActivateMode() )
    {
        if ( pSVData->maWinData.mpLastDeacWin )
        {
            if ( pSVData->maWinData.mpLastDeacWin == pNewOverlapWindow )
                bCallActivate = sal_False;
            else
            {
                pLastRealWindow = pSVData->maWinData.mpLastDeacWin->ImplGetWindow();
                pSVData->maWinData.mpLastDeacWin->mpWindowImpl->mbActive = sal_False;
                pSVData->maWinData.mpLastDeacWin->Deactivate();
                if ( pLastRealWindow != pSVData->maWinData.mpLastDeacWin )
                {
                    pLastRealWindow->mpWindowImpl->mbActive = sal_True;
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
            pOldOverlapWindow->mpWindowImpl->mbActive = sal_False;
            pOldOverlapWindow->Deactivate();
        }
        if ( pOldRealWindow != pOldOverlapWindow )
        {
            if( pOldRealWindow->mpWindowImpl->mbActive )
            {
                pOldRealWindow->mpWindowImpl->mbActive = sal_False;
                pOldRealWindow->Deactivate();
            }
        }
    }
    if ( bCallActivate && ! pNewOverlapWindow->mpWindowImpl->mbActive )
    {
        if( ! pNewOverlapWindow->mpWindowImpl->mbActive )
        {
            pNewOverlapWindow->mpWindowImpl->mbActive = sal_True;
            pNewOverlapWindow->Activate();
        }
        if ( pNewRealWindow != pNewOverlapWindow )
        {
            if( ! pNewRealWindow->mpWindowImpl->mbActive )
            {
                pNewRealWindow->mpWindowImpl->mbActive = sal_True;
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
void Window::ImplGrabFocus( sal_uInt16 nFlags )
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

    sal_Bool bAsyncFocusWaiting = sal_False;
    Window *pFrame = pSVData->maWinData.mpFirstFrame;
    while( pFrame  )
    {
        if( pFrame != mpWindowImpl->mpFrameWindow && pFrame->mpWindowImpl->mpFrameData->mnFocusId )
        {
            bAsyncFocusWaiting = sal_True;
            break;
        }
        pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
    }

    bool bHasFocus = IsWindowFocused(*mpWindowImpl);

    sal_Bool bMustNotGrabFocus = sal_False;
    // #100242#, check parent hierarchy if some floater prohibits grab focus

    Window *pParent = this;
    while( pParent )
    {
        // #102158#, ignore grabfocus only if the floating parent grabs keyboard focus by itself (GrabsFocus())
        // otherwise we cannot set the focus in a floating toolbox
        if( ( (pParent->mpWindowImpl->mbFloatWin && ((FloatingWindow*)pParent)->GrabsFocus()) || ( pParent->GetStyle() & WB_SYSTEMFLOATWIN ) ) && !( pParent->GetStyle() & WB_MOVEABLE ) )
        {
            bMustNotGrabFocus = sal_True;
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

        // mark this windows as the last FocusWindow
        Window* pOverlapWindow = ImplGetFirstOverlapWindow();
        pOverlapWindow->mpWindowImpl->mpLastFocusWindow = this;
        mpWindowImpl->mpFrameData->mpFocusWin = this;

        if( !bHasFocus )
        {
            // menu windows never get the system focus
            // the application will keep the focus
            if( bMustNotGrabFocus )
                return;
            else
            {
                // here we already switch focus as ToTop()
                // should not give focus to another window
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

        // !!!!! due to old SV-Office Activate/Deactivate handling
        // !!!!! first as before
        if ( pOldFocusWindow )
        {
            // remember Focus
            Window* pOldOverlapWindow = pOldFocusWindow->ImplGetFirstOverlapWindow();
            Window* pNewOverlapWindow = ImplGetFirstOverlapWindow();
            if ( pOldOverlapWindow != pNewOverlapWindow )
                ImplCallFocusChangeActivate( pNewOverlapWindow, pOldOverlapWindow );
        }
        else
        {
            Window* pNewOverlapWindow = ImplGetFirstOverlapWindow();
            Window* pNewRealWindow = pNewOverlapWindow->ImplGetWindow();
            pNewOverlapWindow->mpWindowImpl->mbActive = sal_True;
            pNewOverlapWindow->Activate();
            if ( pNewRealWindow != pNewOverlapWindow )
            {
                pNewRealWindow->mpWindowImpl->mbActive = sal_True;
                pNewRealWindow->Activate();
            }
        }

        // call Get- and LoseFocus
        if ( pOldFocusWindow && ! aOldFocusDel.IsDead() )
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
                mpWindowImpl->mbInFocusHdl = sal_True;
                mpWindowImpl->mnGetFocusFlags = nFlags;
                // if we're changing focus due to closing a popup floating window
                // notify the new focus window so it can restore the inner focus
                // eg, toolboxes can select their recent active item
                if( pOldFocusWindow &&
                    ! aOldFocusDel.IsDead() &&
                    ( pOldFocusWindow->GetDialogControlFlags() & WINDOW_DLGCTRL_FLOATWIN_POPUPMODEEND_CANCEL ) )
                    mpWindowImpl->mnGetFocusFlags |= GETFOCUS_FLOATWIN_POPUPMODEEND_CANCEL;
                NotifyEvent aNEvt( EVENT_GETFOCUS, this );
                if ( !ImplCallPreNotify( aNEvt ) && !aDogTag.IsDead() )
                    GetFocus();
                if( !aDogTag.IsDead() )
                    ImplCallActivateListeners( (pOldFocusWindow && ! aOldFocusDel.IsDead()) ? pOldFocusWindow : NULL );
                if( !aDogTag.IsDead() )
                {
                    mpWindowImpl->mnGetFocusFlags = 0;
                    mpWindowImpl->mbInFocusHdl = sal_False;
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
            // only set default sizes if the font height in logical
            // coordinates equals 0
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
    rtl::OString lcl_createWindowInfo(const Window& i_rWindow)
    {
        // skip border windows, they don't carry information which helps diagnosing the problem
        const Window* pWindow( &i_rWindow );
        while ( pWindow && ( pWindow->GetType() == WINDOW_BORDERWINDOW ) )
            pWindow = pWindow->GetWindow( WINDOW_FIRSTCHILD );
        if ( !pWindow )
            pWindow = &i_rWindow;

        rtl::OStringBuffer aErrorString;
        aErrorString.append(char(13));
        aErrorString.append(typeid( *pWindow ).name());
        aErrorString.append(" (window text: '");
        aErrorString.append(rtl::OUStringToOString(pWindow->GetText(), RTL_TEXTENCODING_UTF8));
        aErrorString.append("')");
        return aErrorString.makeStringAndClear();
    }
}
#endif
// -----------------------------------------------------------------------

Window::~Window()
{
    ImplFreeExtWindowImpl();

    vcl::LazyDeletor<Window>::Undelete( this );

    DBG_DTOR( Window, ImplDbgCheckWindow );
    DBG_ASSERT( !mpWindowImpl->mbInDtor, "~Window - already in DTOR!" );


    // remove Key and Mouse events issued by Application::PostKey/MouseEvent
    Application::RemoveMouseAndKeyEvents( this );

    // Dispose of the canvas implementation (which, currently, has an
    // own wrapper window as a child to this one.
    uno::Reference< rendering::XCanvas > xCanvas( mpWindowImpl->mxCanvas );
    if( xCanvas.is() )
    {
        uno::Reference < lang::XComponent > xCanvasComponent( xCanvas,
                                                              uno::UNO_QUERY );
        if( xCanvasComponent.is() )
            xCanvasComponent->dispose();
    }

    mpWindowImpl->mbInDtor = sal_True;

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
                uno::Reference< XDragGestureRecognizer > xDragGestureRecognizer =
                    uno::Reference< XDragGestureRecognizer > (mpWindowImpl->mpFrameData->mxDragSource, UNO_QUERY);
                if( xDragGestureRecognizer.is() )
                {
                    xDragGestureRecognizer->removeDragGestureListener(
                        uno::Reference< XDragGestureListener > (mpWindowImpl->mpFrameData->mxDropTargetListener, UNO_QUERY));
                }

                mpWindowImpl->mpFrameData->mxDropTarget->removeDropTargetListener( mpWindowImpl->mpFrameData->mxDropTargetListener );
                mpWindowImpl->mpFrameData->mxDropTargetListener.clear();
            }

            // shutdown drag and drop for this frame window
            uno::Reference< XComponent > xComponent( mpWindowImpl->mpFrameData->mxDropTarget, UNO_QUERY );

            // DNDEventDispatcher does not hold a reference of the DropTarget,
            // so it's ok if it does not support XComponent
            if( xComponent.is() )
                xComponent->dispose();
        }
        catch (const Exception&)
        {
            // can be safely ignored here.
        }
    }

    UnoWrapperBase* pWrapper = Application::GetUnoWrapper( sal_False );
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

    // due to old compatibility
    if ( pSVData->maWinData.mpTrackWin == this )
        EndTracking();
    if ( pSVData->maWinData.mpCaptureWin == this )
        ReleaseMouse();
    if ( pSVData->maWinData.mpDefDialogParent == this )
        pSVData->maWinData.mpDefDialogParent = NULL;

#if OSL_DEBUG_LEVEL > 0
    if ( sal_True ) // always perform these tests in non-pro versions
    {
        rtl::OStringBuffer aErrorStr;
        sal_Bool        bError = sal_False;
        Window*     pTempWin = mpWindowImpl->mpFrameData->mpFirstOverlap;
        while ( pTempWin )
        {
            if ( ImplIsRealParentPath( pTempWin ) )
            {
                bError = sal_True;
                aErrorStr.append(lcl_createWindowInfo(*pTempWin));
            }
            pTempWin = pTempWin->mpWindowImpl->mpNextOverlap;
        }
        if ( bError )
        {
            rtl::OStringBuffer aTempStr;
            aTempStr.append(RTL_CONSTASCII_STRINGPARAM("Window ("));
            aTempStr.append(rtl::OUStringToOString(GetText(),
                RTL_TEXTENCODING_UTF8));
            aTempStr.append(RTL_CONSTASCII_STRINGPARAM(
                ") with living SystemWindow(s) destroyed: "));
            aTempStr.append(aErrorStr.toString());
            OSL_FAIL(aTempStr.getStr());
            // abort in non-pro version, this must be fixed!
            GetpApp()->Abort(rtl::OStringToOUString(
                aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));
        }

        bError = sal_False;
        pTempWin = pSVData->maWinData.mpFirstFrame;
        while ( pTempWin )
        {
            if ( ImplIsRealParentPath( pTempWin ) )
            {
                bError = sal_True;
                aErrorStr.append(lcl_createWindowInfo(*pTempWin));
            }
            pTempWin = pTempWin->mpWindowImpl->mpFrameData->mpNextFrame;
        }
        if ( bError )
        {
            rtl::OStringBuffer aTempStr( "Window (" );
            aTempStr.append(rtl::OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") with living SystemWindow(s) destroyed: ");
            aTempStr.append(aErrorStr.toString());
            OSL_FAIL( aTempStr.getStr() );
            GetpApp()->Abort(rtl::OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   // abort in non-pro version, this must be fixed!
        }

        if ( mpWindowImpl->mpFirstChild )
        {
            rtl::OStringBuffer aTempStr("Window (");
            aTempStr.append(rtl::OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") with living Child(s) destroyed: ");
            pTempWin = mpWindowImpl->mpFirstChild;
            while ( pTempWin )
            {
                aTempStr.append(lcl_createWindowInfo(*pTempWin));
                pTempWin = pTempWin->mpWindowImpl->mpNext;
            }
            OSL_FAIL( aTempStr.getStr() );
            GetpApp()->Abort(rtl::OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   // abort in non-pro version, this must be fixed!
        }

        if ( mpWindowImpl->mpFirstOverlap )
        {
            rtl::OStringBuffer aTempStr("Window (");
            aTempStr.append(rtl::OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") with living SystemWindow(s) destroyed: ");
            pTempWin = mpWindowImpl->mpFirstOverlap;
            while ( pTempWin )
            {
                aTempStr.append(lcl_createWindowInfo(*pTempWin));
                pTempWin = pTempWin->mpWindowImpl->mpNext;
            }
            OSL_FAIL( aTempStr.getStr() );
            GetpApp()->Abort(rtl::OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   // abort in non-pro version, this must be fixed!
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
            rtl::OStringBuffer aTempStr("Window (");
            aTempStr.append(rtl::OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") still in TaskPanelList!");
            OSL_FAIL( aTempStr.getStr() );
            GetpApp()->Abort(rtl::OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   // abort in non-pro version, this must be fixed!
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
            rtl::OStringBuffer aTempStr("Window (");
            aTempStr.append(rtl::OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") not found in TaskPanelList!");
            OSL_FAIL( aTempStr.getStr() );
        }
    }

    // hide window in order to trigger the Paint-Handling
    Hide();

    // announce the window is to be destroyed
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
    sal_Bool bHasFocussedChild = sal_False;
    if( pSVData->maWinData.mpFocusWin && ImplIsRealParentPath( pSVData->maWinData.mpFocusWin ) )
    {
        // #122232#, this must not happen and is an application bug ! but we try some cleanup to hopefully avoid crashes, see below
        bHasFocussedChild = sal_True;
#if OSL_DEBUG_LEVEL > 0
        rtl::OStringBuffer aTempStr("Window (");
        aTempStr.append(rtl::OUStringToOString(GetText(),
            RTL_TEXTENCODING_UTF8)).
                append(") with focussed child window destroyed ! THIS WILL LEAD TO CRASHES AND MUST BE FIXED !");
        OSL_FAIL( aTempStr.getStr() );
        GetpApp()->Abort(rtl::OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ));   // abort in non-pro version, this must be fixed!
#endif
    }

    // if we get focus pass focus to another window
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
	    // when windows overlap, give focus to the parent
	    // of the next FrameWindow
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

    // reset marked windows
    if ( mpWindowImpl->mpFrameData->mpFocusWin == this )
        mpWindowImpl->mpFrameData->mpFocusWin = NULL;
    if ( mpWindowImpl->mpFrameData->mpMouseMoveWin == this )
        mpWindowImpl->mpFrameData->mpMouseMoveWin = NULL;
    if ( mpWindowImpl->mpFrameData->mpMouseDownWin == this )
        mpWindowImpl->mpFrameData->mpMouseDownWin = NULL;

    // reset Deactivate-Window
    if ( pSVData->maWinData.mpLastDeacWin == this )
        pSVData->maWinData.mpLastDeacWin = NULL;

    if ( mpWindowImpl->mbFrame )
    {
        if ( mpWindowImpl->mpFrameData->mnFocusId )
            Application::RemoveUserEvent( mpWindowImpl->mpFrameData->mnFocusId );
        if ( mpWindowImpl->mpFrameData->mnMouseMoveId )
            Application::RemoveUserEvent( mpWindowImpl->mpFrameData->mnMouseMoveId );
    }

    // release Graphic
    ImplReleaseGraphics();

    // if appropriate announce the window has been deleted
    ImplDelData* pDelData = mpWindowImpl->mpFirstDel;
    while ( pDelData )
    {
        pDelData->mbDel = sal_True;
        pDelData->mpWindow = NULL;  // #112873# pDel is not associated with a Window anymore
        pDelData = pDelData->mpNext;
    }

    // remove window from the lists
    ImplRemoveWindow( sal_True );

    // de-register as "top window child" at our parent, if necessary
    if ( mpWindowImpl->mbFrame )
    {
        sal_Bool bIsTopWindow = mpWindowImpl->mpWinData && ( mpWindowImpl->mpWinData->mnIsTopWindow == 1 );
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

    // delete extra window data
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

        delete mpWindowImpl->mpWinData;
    }


    // delete Overlap-Window data
    if ( mpWindowImpl->mpOverlapData )
    {
        delete mpWindowImpl->mpOverlapData;
    }

    // delete BorderWindow or Frame if required
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

sal_uInt16 Window::GetIndicatorState() const
{
    return mpWindowImpl->mpFrame->GetIndicatorState().mnState;
}

void Window::SimulateKeyPress( sal_uInt16 nKeyCode ) const
{
    mpWindowImpl->mpFrame->SimulateKeyPress(nKeyCode);
}

// -----------------------------------------------------------------------

void Window::MouseMove( const MouseEvent& rMEvt )
{
    { // Parentheses, as in this handler the window can be destroyed
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_MOUSEMOVE, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseMove = sal_True;
}

// -----------------------------------------------------------------------

void Window::MouseButtonDown( const MouseEvent& rMEvt )
{
    { // Parentheses, as in this handler the window can be destroyed
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_MOUSEBUTTONDOWN, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseButtonDown = sal_True;
}

// -----------------------------------------------------------------------

void Window::MouseButtonUp( const MouseEvent& rMEvt )
{
    { // Parentheses, as in this handler the window can be destroyed
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_MOUSEBUTTONUP, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseButtonUp = sal_True;
}

// -----------------------------------------------------------------------

void Window::KeyInput( const KeyEvent& rKEvt )
{
    { // Parentheses, as in this handler the window can be destroyed
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_KEYINPUT, this, &rKEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbKeyInput = sal_True;
}

// -----------------------------------------------------------------------

void Window::KeyUp( const KeyEvent& rKEvt )
{
    { // Parentheses, as in this handler the window can be destroyed
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_KEYUP, this, &rKEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbKeyUp = sal_True;
}

// -----------------------------------------------------------------------

void Window::PrePaint()
{
}

// -----------------------------------------------------------------------

void Window::Paint( const Rectangle& rRect )
{
    { // Parentheses, as in this handler the window can be destroyed
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    ImplCallEventListeners( VCLEVENT_WINDOW_PAINT, (void*)&rRect );
}

// -----------------------------------------------------------------------

void Window::PostPaint()
{
}

// -----------------------------------------------------------------------

void Window::Draw( OutputDevice*, const Point&, const Size&, sal_uLong )
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
    { // Parentheses, as in this handler the window can be destroyed
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    if ( HasFocus() && mpWindowImpl->mpLastFocusWindow && !(mpWindowImpl->mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) )
    {
        ImplDelData aDogtag( this );
        mpWindowImpl->mpLastFocusWindow->GrabFocus();
        if( aDogtag.IsDead() )
            return;
    }

    NotifyEvent aNEvt( EVENT_GETFOCUS, this );
    Notify( aNEvt );
}

// -----------------------------------------------------------------------

void Window::LoseFocus()
{
    { // Parentheses, as in this handler the window can be destroyed
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_LOSEFOCUS, this );
    Notify( aNEvt );
}

// -----------------------------------------------------------------------

void Window::RequestHelp( const HelpEvent& rHEvt )
{
    { // Parentheses, as in this handler the window can be destroyed
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    // if Balloon-Help is requested, show the balloon
    // with help text set
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
        String aStrHelpId( rtl::OStringToOUString( GetHelpId(), RTL_TEXTENCODING_UTF8 ) );
        if ( aStrHelpId.Len() == 0 && ImplGetParent() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                if( aStrHelpId.Len() > 0 )
                    pHelp->Start( aStrHelpId, this );
                else
                    pHelp->Start( rtl::OUString( OOO_HELP_INDEX  ), this );
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::Command( const CommandEvent& rCEvt )
{
    { // Parentheses, as in this handler the window can be destroyed
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    ImplCallEventListeners( VCLEVENT_WINDOW_COMMAND, (void*)&rCEvt );

    NotifyEvent aNEvt( EVENT_COMMAND, this, &rCEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbCommand = sal_True;
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

void Window::UserEvent( sal_uLong, void* )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
}

// -----------------------------------------------------------------------

void Window::StateChanged( StateChangedType )
{
    queue_resize();
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

    if ( aDelData.IsDead() )
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
    { // Parentheses, as in this handler the window can be destroyed
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    long bDone = sal_False;
    if ( mpWindowImpl->mpParent && !ImplIsOverlapWindow() )
        bDone = mpWindowImpl->mpParent->PreNotify( rNEvt );

    if ( !bDone )
    {
        if( rNEvt.GetType() == EVENT_GETFOCUS )
        {
            sal_Bool bCompoundFocusChanged = sal_False;
            if ( mpWindowImpl->mbCompoundControl && !mpWindowImpl->mbCompoundControlHasFocus && HasChildPathFocus() )
            {
                mpWindowImpl->mbCompoundControlHasFocus = sal_True;
                bCompoundFocusChanged = sal_True;
            }

            if ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) )
                ImplCallEventListeners( VCLEVENT_WINDOW_GETFOCUS );
        }
        else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
        {
            sal_Bool bCompoundFocusChanged = sal_False;
            if ( mpWindowImpl->mbCompoundControl && mpWindowImpl->mbCompoundControlHasFocus && !HasChildPathFocus() )
            {
                mpWindowImpl->mbCompoundControlHasFocus = sal_False ;
                bCompoundFocusChanged = sal_True;
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
    { // Parentheses, as in this handler the window can be destroyed
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    long nRet = sal_False;

    // check for docking window
    // but do nothing if window is docked and locked
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper && !( !pWrapper->IsFloatingMode() && pWrapper->IsLocked() ) )
    {
        if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            sal_Bool bHit = pWrapper->GetDragArea().IsInside( pMEvt->GetPosPixel() );
            if ( pMEvt->IsLeft() )
            {
                if ( pMEvt->IsMod1() && (pMEvt->GetClicks() == 2) )
                {
                    // ctrl double click toggles floating mode
                    pWrapper->SetFloatingMode( !pWrapper->IsFloatingMode() );
                    return sal_True;
                }
                else if ( pMEvt->GetClicks() == 1 && bHit)
                {
                    // allow start docking during mouse move
                    pWrapper->ImplEnableStartDocking();
                    return sal_True;
                }
            }
        }
        else if ( rNEvt.GetType() == EVENT_MOUSEMOVE )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            sal_Bool bHit = pWrapper->GetDragArea().IsInside( pMEvt->GetPosPixel() );
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
                return sal_True;
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
                return sal_True;
            }
        }
    }

    // manage the dialogs
    if ( (GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL )
    {
        // if the parent also has dialog control activated, the parent takes over control
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
                sal_uInt16 n = 0;
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

void Window::ImplCallEventListeners( sal_uLong nEvent, void* pData )
{
    // The implementation was moved to CallEventListeners(),
    // because derived classes in svtools must be able to
    // call the event listeners and ImplCallEventListeners()
    // is not exported.
    // TODO: replace ImplCallEventListeners() by CallEventListeners() in vcl

    CallEventListeners( nEvent, pData );
}

// -----------------------------------------------------------------------

void Window::CallEventListeners( sal_uLong nEvent, void* pData )
{
    VclWindowEvent aEvent( this, nEvent, pData );

    ImplDelData aDelData;
    ImplAddDel( &aDelData );

    ImplGetSVData()->mpApp->ImplCallEventListeners( &aEvent );

    if ( aDelData.IsDead() )
        return;

    mpWindowImpl->maEventListeners.Call( &aEvent );

    if ( aDelData.IsDead() )
        return;

    ImplRemoveDel( &aDelData );

    Window* pWindow = this;
    while ( pWindow )
    {
        pWindow->ImplAddDel( &aDelData );

        pWindow->mpWindowImpl->maChildEventListeners.Call( &aEvent );

        if ( aDelData.IsDead() )
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

sal_uLong Window::PostUserEvent( const Link& rLink, void* pCaller )
{
    sal_uLong nEventId;
    PostUserEvent( nEventId, rLink, pCaller );
    return nEventId;
}

// -----------------------------------------------------------------------

sal_Bool Window::PostUserEvent( sal_uLong& rEventId, sal_uLong nEvent, void* pEventData )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mnEvent   = nEvent;
    pSVEvent->mpData    = pEventData;
    pSVEvent->mpLink    = NULL;
    pSVEvent->mpWindow  = this;
    pSVEvent->mbCall    = sal_True;
    ImplAddDel( &(pSVEvent->maDelData) );
    rEventId = (sal_uLong)pSVEvent;
    if ( mpWindowImpl->mpFrame->PostEvent( pSVEvent ) )
        return sal_True;
    else
    {
        rEventId = 0;
        ImplRemoveDel( &(pSVEvent->maDelData) );
        delete pSVEvent;
        return sal_False;
    }
}

// -----------------------------------------------------------------------

sal_Bool Window::PostUserEvent( sal_uLong& rEventId, const Link& rLink, void* pCaller )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mnEvent   = 0;
    pSVEvent->mpData    = pCaller;
    pSVEvent->mpLink    = new Link( rLink );
    pSVEvent->mpWindow  = this;
    pSVEvent->mbCall    = sal_True;
    ImplAddDel( &(pSVEvent->maDelData) );
    rEventId = (sal_uLong)pSVEvent;
    if ( mpWindowImpl->mpFrame->PostEvent( pSVEvent ) )
        return sal_True;
    else
    {
        rEventId = 0;
        ImplRemoveDel( &(pSVEvent->maDelData) );
        delete pSVEvent;
        return sal_False;
    }
}

// -----------------------------------------------------------------------

void Window::RemoveUserEvent( sal_uLong nUserEvent )
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

    pSVEvent->mbCall = sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Window::IsLocked( sal_Bool bChildren ) const
{
    if ( mpWindowImpl->mnLockCount != 0 )
        return sal_True;

    if ( bChildren || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            if ( pChild->IsLocked( sal_True ) )
                return sal_True;
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }

    return sal_False;
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

void Window::SetBorderStyle( sal_uInt16 nBorderStyle )
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

sal_uInt16 Window::GetBorderStyle() const
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
        // we guess the width for frame windows as we do not know the
        // border of external dialogs
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

void Window::EnableClipSiblings( sal_Bool bClipSiblings )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->EnableClipSiblings( bClipSiblings );

    mpWindowImpl->mbClipSiblings = bClipSiblings;
}

// -----------------------------------------------------------------------

void Window::SetMouseTransparent( sal_Bool bTransparent )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetMouseTransparent( bTransparent );

    if( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->SetMouseTransparent( bTransparent );

    mpWindowImpl->mbMouseTransparent = bTransparent;
}

// -----------------------------------------------------------------------

void Window::SetPaintTransparent( sal_Bool bTransparent )
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

void Window::EndExtTextInput( sal_uInt16 nFlags )
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
    SetSettings( rSettings, sal_False );
}

void Window::SetSettings( const AllSettings& rSettings, sal_Bool bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->SetSettings( rSettings, sal_False );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->SetSettings( rSettings, sal_True );
    }

    AllSettings aOldSettings = maSettings;
    OutputDevice::SetSettings( rSettings );
    sal_uLong nChangeFlags = aOldSettings.GetChangeFlags( rSettings );

    // recalculate AppFont-resolution and DPI-resolution
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

void Window::UpdateSettings( const AllSettings& rSettings, sal_Bool bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->UpdateSettings( rSettings, sal_False );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->UpdateSettings( rSettings, sal_True );
    }

    AllSettings aOldSettings = maSettings;
    sal_uLong nChangeFlags = maSettings.Update( maSettings.GetWindowUpdate(), rSettings );
    nChangeFlags |= SETTINGS_IN_UPDATE_SETTINGS; // Set this flag so the receiver of the data changed
                                                 // event can distinguish between the changing of global
                                                 // setting and a local change ( with SetSettings )

    // recalculate AppFont-resolution and DPI-resolution
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

void Window::NotifyAllChildren( DataChangedEvent& rDCEvt )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    DataChanged( rDCEvt );

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->NotifyAllChildren( rDCEvt );
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

void Window::SetParentClipMode( sal_uInt16 nMode )
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
                mpWindowImpl->mpParent->mpWindowImpl->mbClipChildren = sal_True;
        }
    }
}

// -----------------------------------------------------------------------

sal_uInt16 Window::GetParentClipMode() const
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
        mpWindowImpl->mbWinRegion = sal_False;
        mpWindowImpl->mpFrame->ResetClipRegion();
    }
    else
    {
        if ( mpWindowImpl->mbWinRegion )
        {
            mpWindowImpl->maWinRegion = Region( REGION_NULL );
            mpWindowImpl->mbWinRegion = sal_False;
            ImplSetClipFlag();

            if ( IsReallyVisible() )
            {
                // restore background storage
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
                // set/update ClipRegion
                long                nX;
                long                nY;
                long                nWidth;
                long                nHeight;
                sal_uLong               nRectCount;
                ImplRegionInfo      aInfo;
                sal_Bool                bRegionRect;

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
                mpWindowImpl->mbWinRegion = sal_False;
                ImplSetClipFlag();
            }
        }
        else
        {
            mpWindowImpl->maWinRegion = rRegion;
            mpWindowImpl->mbWinRegion = sal_True;
            ImplSetClipFlag();
        }

        if ( IsReallyVisible() )
        {
            // restore background storage
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

sal_Bool Window::IsWindowRegionPixel() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        return mpWindowImpl->mpBorderWindow->IsWindowRegionPixel();
    else
        return mpWindowImpl->mbWinRegion;
}

// -----------------------------------------------------------------------

Region Window::GetWindowClipRegionPixel( sal_uInt16 nFlags ) const
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
            mbInitClipRegion = sal_True;
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
    sal_Bool bChangeTaskPaneList = sal_False;
    if( pSysWin && pSysWin->ImplIsInTaskPaneList( this ) )
    {
        pNewSysWin = ImplGetLastSystemWindow( pNewParent );
        if( pNewSysWin && pNewSysWin != pSysWin )
        {
            bChangeTaskPaneList = sal_True;
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

    sal_Bool bVisible = IsVisible();
    Show( false, SHOW_NOFOCUSCHANGE );

    // check if the overlap window changes
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

    // convert windows in the hierarchy
    sal_Bool bFocusOverlapWin = HasChildPathFocus( sal_True );
    sal_Bool bFocusWin = HasChildPathFocus();
    sal_Bool bNewFrame = pNewParent->mpWindowImpl->mpFrameWindow != mpWindowImpl->mpFrameWindow;
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
        pNewParent->mpWindowImpl->mbClipChildren = sal_True;
    ImplUpdateWindowPtr();
    if ( ImplUpdatePos() )
        ImplUpdateSysObjPos();

    // If the Overlap-Window has changed, we need to test whether
    // OverlapWindows that had the Child window as their parent
    // need to be put into the window hierarchy.
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
        // reset Focus-Save
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

        // update activate-status at next overlap window
        if ( HasChildPathFocus( sal_True ) )
            ImplCallFocusChangeActivate( pNewOverlapWindow, pOldOverlapWindow );
    }

    // also convert Activate-Status
    if ( bNewFrame )
    {
        if ( (GetType() == WINDOW_BORDERWINDOW) &&
             (ImplGetWindow()->GetType() == WINDOW_FLOATINGWINDOW) )
            ((ImplBorderWindow*)this)->SetDisplayActive( mpWindowImpl->mpFrameData->mbHasFocus );
    }

    // when required give focus to new frame if
    // FocusWindow is changed with SetParent()
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

void Window::Show( sal_Bool bVisible, sal_uInt16 nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mbVisible == bVisible )
        return;

    ImplDelData aDogTag( this );

    sal_Bool bRealVisibilityChanged = sal_False;
    mpWindowImpl->mbVisible = (bVisible != 0);

    if ( !bVisible )
    {
        ImplHideAllOverlaps();
        if( aDogTag.IsDead() )
            return;

        if ( mpWindowImpl->mpBorderWindow )
        {
            bool bOldUpdate = mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate;
            if ( mpWindowImpl->mbNoParentUpdate )
                mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate = sal_True;
            mpWindowImpl->mpBorderWindow->Show( false, nFlags );
            mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate = bOldUpdate;
        }
        else if ( mpWindowImpl->mbFrame )
        {
            mpWindowImpl->mbSuppressAccessibilityEvents = sal_True;
            mpWindowImpl->mpFrame->Show( sal_False, sal_False );
        }

        StateChanged( STATE_CHANGE_VISIBLE );

        if ( mpWindowImpl->mbReallyVisible )
        {
            Region  aInvRegion( REGION_EMPTY );
            sal_Bool    bSaveBack = sal_False;

            if ( ImplIsOverlapWindow() && !mpWindowImpl->mbFrame )
            {
                if ( ImplRestoreOverlapBackground( aInvRegion ) )
                    bSaveBack = sal_True;
            }

            if ( !bSaveBack )
            {
                if ( mpWindowImpl->mbInitWinClipRegion )
                    ImplInitWinClipRegion();
                aInvRegion = mpWindowImpl->maWinClipRegion;
            }

            if( aDogTag.IsDead() )
                return;

            bRealVisibilityChanged = mpWindowImpl->mbReallyVisible;
            ImplResetReallyVisible();
            ImplSetClipFlag();

            if ( ImplIsOverlapWindow() && !mpWindowImpl->mbFrame )
            {
                // convert focus
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
            // if a window becomes visible, send all child windows a StateChange,
            // such that these can initialise themselves
            ImplCallInitShow();

            // If it is a SystemWindow it automatically pops up on top of
            // all other windows if needed.
            if ( ImplIsOverlapWindow() && !(nFlags & SHOW_NOACTIVATE) )
            {
                ImplStartToTop(( nFlags & SHOW_FOREGROUNDTASK ) ? TOTOP_FOREGROUNDTASK : 0 );
                ImplFocusToTop( 0, sal_False );
            }

            // save background
            if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mbSaveBack )
                ImplSaveOverlapBackground();
            // adjust mpWindowImpl->mbReallyVisible
            bRealVisibilityChanged = !mpWindowImpl->mbReallyVisible;
            ImplSetReallyVisible();

            // assure clip rectangles will be recalculated
            ImplSetClipFlag();

            if ( !mpWindowImpl->mbFrame )
            {
                sal_uInt16 nInvalidateFlags = INVALIDATE_CHILDREN;
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
            mpWindowImpl->mbSuppressAccessibilityEvents = sal_False;

            mpWindowImpl->mbPaintFrame = sal_True;
            sal_Bool bNoActivate = (nFlags & (SHOW_NOACTIVATE|SHOW_NOFOCUSCHANGE)) ? sal_True : sal_False;
            mpWindowImpl->mpFrame->Show( sal_True, bNoActivate );
            if( aDogTag.IsDead() )
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

        if( aDogTag.IsDead() )
            return;

#if OSL_DEBUG_LEVEL > 0
        if ( IsDialog() || (GetType() == WINDOW_TABPAGE) || (GetType() == WINDOW_DOCKINGWINDOW) )
        {
            DBG_DIALOGTEST( this );
        }
#endif

        ImplShowAllOverlaps();
    }

    if( aDogTag.IsDead() )
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
    if( aDogTag.IsDead() )
        return;

    // #107575#, if a floating windows is shown that grabs the focus, we have to notify the toolkit about it
    // ImplGrabFocus() is not called in this case
    // Because this might lead to problems the task will be shifted to 6.y
    // Note: top-level context menus are registered at the access bridge after being shown,
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
        if( aDogtag.IsDead() )
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
        // the tracking mode will be stopped or the capture will be stolen
        // when a window is disabled,
        if ( IsTracking() )
            EndTracking( ENDTRACK_CANCEL );
        if ( IsMouseCaptured() )
            ReleaseMouse();
        // try to pass focus to the next control
        // if the window has focus and is contained in the dialog control
        // mpWindowImpl->mbDisabled should only be set after a call of ImplDlgCtrlNextWindow().
        // Otherwise ImplDlgCtrlNextWindow() should be used
        if ( HasFocus() )
            ImplDlgCtrlNextWindow();
    }

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->Enable( bEnable, sal_False );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->Enable( bEnable, sal_True );
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
    mpWindowImpl->mbCallHandlersDuringInputDisabled = bCall ? sal_True : sal_False;

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

void Window::EnableInput( sal_Bool bEnable, sal_Bool bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    sal_Bool bNotify = (bEnable != mpWindowImpl->mbInputDisabled);
    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->EnableInput( bEnable, sal_False );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->EnableInput( bEnable, sal_True );
    }

    if ( (! bEnable && mpWindowImpl->meAlwaysInputMode != AlwaysInputEnabled) ||
         (  bEnable && mpWindowImpl->meAlwaysInputMode != AlwaysInputDisabled) )
    {
        // automatically stop the tracking mode or steal capture
        // if the window is disabled
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

void Window::EnableInput( sal_Bool bEnable, sal_Bool bChild, sal_Bool bSysWin,
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
            if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( pSysWin, sal_True ) )
            {
                // Is Window not in the exclude window path or not the
                // exclude window, than change the status
                if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( pSysWin, sal_True ) )
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
                if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( pFrameWin, sal_True ) )
                {
                    // Is Window not in the exclude window path or not the
                    // exclude window, than change the status
                    if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( pFrameWin, sal_True ) )
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
                if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( (*p), sal_True ) )
                {
                    // Is Window not in the exclude window path or not the
                    // exclude window, than change the status
                    if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( (*p), sal_True ) )
                        (*p)->EnableInput( bEnable, bChild );
                }
                ++p;
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::AlwaysEnableInput( sal_Bool bAlways, sal_Bool bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->AlwaysEnableInput( bAlways, sal_False );

    if( bAlways && mpWindowImpl->meAlwaysInputMode != AlwaysInputEnabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputEnabled;

        if ( bAlways )
            EnableInput( sal_True, sal_False );
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

void Window::AlwaysDisableInput( sal_Bool bAlways, sal_Bool bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->AlwaysDisableInput( bAlways, sal_False );

    if( bAlways && mpWindowImpl->meAlwaysInputMode != AlwaysInputDisabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputDisabled;

        if ( bAlways )
            EnableInput( sal_False, sal_False );
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

void Window::SetActivateMode( sal_uInt16 nMode )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetActivateMode( nMode );

    if ( mpWindowImpl->mnActivateMode != nMode )
    {
        mpWindowImpl->mnActivateMode = nMode;

        // possibly trigger Decativate/Activate
        if ( mpWindowImpl->mnActivateMode )
        {
            if ( (mpWindowImpl->mbActive || (GetType() == WINDOW_BORDERWINDOW)) &&
                 !HasChildPathFocus( sal_True ) )
            {
                mpWindowImpl->mbActive = sal_False;
                Deactivate();
            }
        }
        else
        {
            if ( !mpWindowImpl->mbActive || (GetType() == WINDOW_BORDERWINDOW) )
            {
                mpWindowImpl->mbActive = sal_True;
                Activate();
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::ToTop( sal_uInt16 nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplStartToTop( nFlags );
    ImplFocusToTop( nFlags, IsReallyVisible() );
}

// -----------------------------------------------------------------------

void Window::SetZOrder( Window* pRefWindow, sal_uInt16 nFlags )
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
        // restore background storage
        if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
            ImplInvalidateAllOverlapBackgrounds();

        if ( mpWindowImpl->mbInitWinClipRegion || !mpWindowImpl->maWinClipRegion.IsEmpty() )
        {
            sal_Bool bInitWinClipRegion = mpWindowImpl->mbInitWinClipRegion;
            ImplSetClipFlag();

            // When ClipRegion was not initialised, assume
            // the window has not been sent, therefore do not
            // trigger any Invalidates. This is an optimisation
            // for HTML documents with many controls. If this
            // check gives problems, a flag should be introduced
            // which tracks whether the window has already been
            // emitted after Show
            if ( !bInitWinClipRegion )
            {
                // Invalidate all windows which are next to each other
                // Is INCOMPLETE !!!
                Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                Window*     pWindow = NULL;
                if ( ImplIsOverlapWindow() )
                {
                    if ( mpWindowImpl->mpOverlapWindow )
                        pWindow = mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
                }
                else
                    pWindow = ImplGetParent()->mpWindowImpl->mpFirstChild;
                // Invalidate all windows in front of us and which are covered by us
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

                // If we are covered by a window in the background
                // we should redraw it
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

void Window::EnableAlwaysOnTop( sal_Bool bEnable )
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
                              long nWidth, long nHeight, sal_uInt16 nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    sal_Bool bHasValidSize = !mpWindowImpl->mbDefSize;

    if ( nFlags & WINDOW_POSSIZE_POS )
        mpWindowImpl->mbDefPos = sal_False;
    if ( nFlags & WINDOW_POSSIZE_SIZE )
        mpWindowImpl->mbDefSize = sal_False;

    // The top BorderWindow is the window which is to be positioned
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


        sal_uInt16 nSysFlags=0;
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
    return ImplGetWindowExtentsRelative( pRelativeWindow, sal_False );
}

Rectangle Window::GetClientWindowExtentsRelative( Window *pRelativeWindow ) const
{
    // without decoration
    return ImplGetWindowExtentsRelative( pRelativeWindow, sal_True );
}

// -----------------------------------------------------------------------

Rectangle Window::ImplGetWindowExtentsRelative( Window *pRelativeWindow, sal_Bool bClientOnly ) const
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

void Window::Scroll( long nHorzScroll, long nVertScroll, sal_uInt16 nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplScroll( Rectangle( Point( mnOutOffX, mnOutOffY ),
                           Size( mnOutWidth, mnOutHeight ) ),
                nHorzScroll, nVertScroll, nFlags & ~SCROLL_CLIP );
}

// -----------------------------------------------------------------------

void Window::Scroll( long nHorzScroll, long nVertScroll,
                     const Rectangle& rRect, sal_uInt16 nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Rectangle aRect = ImplLogicToDevicePixel( rRect );
    aRect.Intersection( Rectangle( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) ) );
    if ( !aRect.IsEmpty() )
        ImplScroll( aRect, nHorzScroll, nVertScroll, nFlags );
}

// -----------------------------------------------------------------------

void Window::Invalidate( sal_uInt16 nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    ImplInvalidate( NULL, nFlags );
}

// -----------------------------------------------------------------------

void Window::Invalidate( const Rectangle& rRect, sal_uInt16 nFlags )
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

void Window::Invalidate( const Region& rRegion, sal_uInt16 nFlags )
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

void Window::Validate( sal_uInt16 nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    ImplValidate( NULL, nFlags );
}

// -----------------------------------------------------------------------

sal_Bool Window::HasPaintEvent() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !mpWindowImpl->mbReallyVisible )
        return sal_False;

    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
        return sal_True;

    if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINT )
        return sal_True;

    if ( !ImplIsOverlapWindow() )
    {
        const Window* pTempWindow = this;
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINTCHILDREN | IMPL_PAINT_PAINTALLCHILDREN) )
                return sal_True;
        }
        while ( !pTempWindow->ImplIsOverlapWindow() );
    }

    return sal_False;
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

    sal_Bool bFlush = sal_False;
    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) )
            bFlush = sal_True;
    }

    // First we should skip all windows which are Paint-Transparent
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
    // In order to limit drawing, an update only draws the window which
    // has PAINTALLCHILDREN set
    pWindow = pUpdateWindow;
    do
    {
        if ( pWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDREN )
            pUpdateWindow = pWindow;
        if ( pWindow->ImplIsOverlapWindow() )
            break;
        pWindow = pWindow->ImplGetParent();
    }
    while ( pWindow );

    // if there is something to paint, trigger a Paint
    if ( pUpdateWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDREN) )
    {
        // trigger an update also for system windows on top of us,
        // otherwise holes would remain
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

void Window::SetUpdateMode( sal_Bool bUpdate )
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

sal_Bool Window::HasFocus() const
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

sal_Bool Window::HasChildPathFocus( sal_Bool bSystemWindow ) const
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
    return sal_False;
}

// -----------------------------------------------------------------------

void Window::CaptureMouse()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVData* pSVData = ImplGetSVData();

    // possibly stop tracking
    if ( pSVData->maWinData.mpTrackWin != this )
    {
        if ( pSVData->maWinData.mpTrackWin )
            pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    }

    if ( pSVData->maWinData.mpCaptureWin != this )
    {
        pSVData->maWinData.mpCaptureWin = this;
        mpWindowImpl->mpFrame->CaptureMouse( sal_True );
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
        mpWindowImpl->mpFrame->CaptureMouse( sal_False );
        ImplGenerateMouseMove();
    }
}

// -----------------------------------------------------------------------

sal_Bool Window::IsMouseCaptured() const
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

    // possibly immediately move pointer
    if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
}

// -----------------------------------------------------------------------

void Window::EnableChildPointerOverwrite( sal_Bool bOverwrite )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mbChildPtrOverwrite == bOverwrite )
        return;

    mpWindowImpl->mbChildPtrOverwrite  = bOverwrite;

    // possibly immediately move pointer
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

void Window::ShowPointer( sal_Bool bVisible )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mbNoPtrVisible != !bVisible )
    {
        mpWindowImpl->mbNoPtrVisible = !bVisible;

        // possibly immediately move pointer
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

sal_Bool Window::IsMouseOver()
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
        // possibly immediately move pointer
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
            // possibly immediately move pointer
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
        Window* pWindow = GetAccessibleRelationLabelFor();
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

    String aStrHelpId( rtl::OStringToOUString( GetHelpId(), RTL_TEXTENCODING_UTF8 ) );
    bool bStrHelpId = (aStrHelpId.Len() > 0);

    if ( !mpWindowImpl->maHelpText.Len() && bStrHelpId )
    {
        if ( !IsDialog() && (mpWindowImpl->mnType != WINDOW_TABPAGE) && (mpWindowImpl->mnType != WINDOW_FLOATINGWINDOW) )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                ((Window*)this)->mpWindowImpl->maHelpText = pHelp->GetHelpText( aStrHelpId, this );
                mpWindowImpl->mbHelpTextDynamic = sal_False;
            }
        }
    }
    else if( mpWindowImpl->mbHelpTextDynamic && bStrHelpId )
    {
        static const char* pEnv = getenv( "HELP_DEBUG" );
        if( pEnv && *pEnv )
        {
            rtl::OUStringBuffer aTxt( 64+mpWindowImpl->maHelpText.Len() );
            aTxt.append( mpWindowImpl->maHelpText );
            aTxt.appendAscii( "\n------------------\n" );
            aTxt.append( rtl::OUString( aStrHelpId ) );
            mpWindowImpl->maHelpText = aTxt.makeStringAndClear();
        }
        mpWindowImpl->mbHelpTextDynamic = sal_False;
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

sal_uInt16 Window::GetChildCount() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    sal_uInt16  nChildCount = 0;
    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        nChildCount++;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    return nChildCount;
}

// -----------------------------------------------------------------------

Window* Window::GetChild( sal_uInt16 nChild ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    sal_uInt16  nChildCount = 0;
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

Window* Window::GetWindow( sal_uInt16 nType ) const
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

sal_Bool Window::IsChild( const Window* pWindow, sal_Bool bSystemWindow ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    DBG_CHKOBJ( pWindow, Window, ImplDbgCheckWindow );

    do
    {
        if ( !bSystemWindow && pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();

        if ( pWindow == this )
            return sal_True;
    }
    while ( pWindow );

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Window::IsWindowOrChild( const Window* pWindow, sal_Bool bSystemWindow ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    DBG_CHKOBJ( pWindow, Window, ImplDbgCheckWindow );

    if ( this == pWindow )
        return sal_True;
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

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > Window::GetComponentInterface( sal_Bool bCreate )
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
        if( aDogtag.IsDead() )
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
        if( aDogtag.IsDead() )
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

uno::Reference< XDropTarget > Window::GetDropTarget()
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
                uno::Reference< XDragSource > xDragSource = GetDragSource();
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
                        uno::Reference< XDragGestureRecognizer > xDragGestureRecognizer =
                            uno::Reference< XDragGestureRecognizer > (mpWindowImpl->mpFrameData->mxDragSource, UNO_QUERY);

                        if( xDragGestureRecognizer.is() )
                        {
                            xDragGestureRecognizer->addDragGestureListener(
                                uno::Reference< XDragGestureListener > (mpWindowImpl->mpFrameData->mxDropTargetListener, UNO_QUERY));
                        }
                        else
                            mpWindowImpl->mpFrameData->mbInternalDragGestureRecognizer = sal_True;

                    }
                    catch (const RuntimeException&)
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
    return uno::Reference< XDropTarget > ( mpWindowImpl->mxDNDListenerContainer, UNO_QUERY );
}

// -----------------------------------------------------------------------

uno::Reference< XDragSource > Window::GetDragSource()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxDragSource.is() )
        {
            try
            {
                uno::Reference< XMultiServiceFactory > xFactory = vcl::unohelper::GetMultiServiceFactory();
                if ( xFactory.is() )
                {
                    const SystemEnvData * pEnvData = GetSystemData();

                    if( pEnvData )
                    {
                        Sequence< Any > aDragSourceAL( 2 ), aDropTargetAL( 2 );
                        OUString aDragSourceSN, aDropTargetSN;
#if defined WNT
                        aDragSourceSN = OUString("com.sun.star.datatransfer.dnd.OleDragSource");
                        aDropTargetSN = OUString("com.sun.star.datatransfer.dnd.OleDropTarget");
                        aDragSourceAL[ 1 ] = makeAny( (sal_uInt32) pEnvData->hWnd );
                        aDropTargetAL[ 0 ] = makeAny( (sal_uInt32) pEnvData->hWnd );
#elif defined QUARTZ
            /* FIXME: Mac OS X specific dnd interface does not exist! *
             * Using Windows based dnd as a temporary solution        */
                        aDragSourceSN = OUString("com.sun.star.datatransfer.dnd.OleDragSource");
                        aDropTargetSN = OUString("com.sun.star.datatransfer.dnd.OleDropTarget");
                        aDragSourceAL[ 1 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->pView) ) );
                        aDropTargetAL[ 0 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->pView) ) );
#elif defined IOS
            /* What does LibreOffice's use of DND concepts mean on
             * iOS, huh, is this both inter-app DND (which clearly is
             * meaningless), or intra-app? Anyway, use the same code
             * as for MacOSX for now, even if meaningless...
             */
                        aDragSourceSN = OUString("com.sun.star.datatransfer.dnd.OleDragSource");
                        aDropTargetSN = OUString("com.sun.star.datatransfer.dnd.OleDropTarget");
                        aDragSourceAL[ 1 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->pView) ) );
                        aDropTargetAL[ 0 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->pView) ) );
#elif defined UNX
                        aDropTargetAL.realloc( 3 );
                        aDragSourceAL.realloc( 3 );
                        aDragSourceSN = OUString("com.sun.star.datatransfer.dnd.X11DragSource");
                        aDropTargetSN = OUString("com.sun.star.datatransfer.dnd.X11DropTarget");

                        aDragSourceAL[ 0 ] = makeAny( Application::GetDisplayConnection() );
                        aDragSourceAL[ 2 ] = makeAny( vcl::createBmpConverter() );
                        aDropTargetAL[ 0 ] = makeAny( Application::GetDisplayConnection() );
                        aDropTargetAL[ 1 ] = makeAny( (sal_Size)(pEnvData->aShellWindow) );
                        aDropTargetAL[ 2 ] = makeAny( vcl::createBmpConverter() );
#endif
                        if( !aDragSourceSN.isEmpty() )
                            mpWindowImpl->mpFrameData->mxDragSource = uno::Reference< XDragSource > ( xFactory->createInstanceWithArguments( aDragSourceSN, aDragSourceAL ), UNO_QUERY );

                        if( !aDropTargetSN.isEmpty() )
                            mpWindowImpl->mpFrameData->mxDropTarget = uno::Reference< XDropTarget > ( xFactory->createInstanceWithArguments( aDropTargetSN, aDropTargetAL ), UNO_QUERY );
                    }
                }
            }

            // createInstance can throw any exception
            catch (const Exception&)
            {
                // release all instances
                mpWindowImpl->mpFrameData->mxDropTarget.clear();
                mpWindowImpl->mpFrameData->mxDragSource.clear();
            }
        }

        return mpWindowImpl->mpFrameData->mxDragSource;
    }

    return uno::Reference< XDragSource > ();
}

// -----------------------------------------------------------------------

uno::Reference< XDragGestureRecognizer > Window::GetDragGestureRecognizer()
{
    return uno::Reference< XDragGestureRecognizer > ( GetDropTarget(), UNO_QUERY );
}

// -----------------------------------------------------------------------

uno::Reference< XClipboard > Window::GetClipboard()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxClipboard.is() )
        {
            try
            {
                uno::Reference< XMultiServiceFactory > xFactory( vcl::unohelper::GetMultiServiceFactory() );

                if( xFactory.is() )
                {
                    mpWindowImpl->mpFrameData->mxClipboard = uno::Reference< XClipboard >( xFactory->createInstance( OUString("com.sun.star.datatransfer.clipboard.SystemClipboardExt") ), UNO_QUERY );

                    if( !mpWindowImpl->mpFrameData->mxClipboard.is() )
                        mpWindowImpl->mpFrameData->mxClipboard = uno::Reference< XClipboard >( xFactory->createInstance( OUString("com.sun.star.datatransfer.clipboard.SystemClipboard") ), UNO_QUERY );

#if defined(UNX) && !defined(QUARTZ)          // unix clipboard needs to be initialized
                    if( mpWindowImpl->mpFrameData->mxClipboard.is() )
                    {
                        uno::Reference< XInitialization > xInit = uno::Reference< XInitialization >( mpWindowImpl->mpFrameData->mxClipboard, UNO_QUERY );

                        if( xInit.is() )
                        {
                            Sequence< Any > aArgumentList( 3 );
                            aArgumentList[ 0 ] = makeAny( Application::GetDisplayConnection() );
                            aArgumentList[ 1 ] = makeAny( OUString("CLIPBOARD") );
                            aArgumentList[ 2 ] = makeAny( vcl::createBmpConverter() );

                            xInit->initialize( aArgumentList );
                        }
                    }
#endif
                }
            }

            // createInstance can throw any exception
            catch (const Exception&)
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

uno::Reference< XClipboard > Window::GetPrimarySelection()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxSelection.is() )
        {
            try
            {
                uno::Reference< XMultiServiceFactory > xFactory( vcl::unohelper::GetMultiServiceFactory() );

                if( xFactory.is() )
                {
#if defined(UNX) && !defined(QUARTZ)
                    Sequence< Any > aArgumentList( 3 );
                      aArgumentList[ 0 ] = makeAny( Application::GetDisplayConnection() );
                    aArgumentList[ 1 ] = makeAny( OUString("PRIMARY") );
                    aArgumentList[ 2 ] = makeAny( vcl::createBmpConverter() );

                    mpWindowImpl->mpFrameData->mxSelection = uno::Reference< XClipboard >( xFactory->createInstanceWithArguments(
                    OUString("com.sun.star.datatransfer.clipboard.SystemClipboard"), aArgumentList ), UNO_QUERY );
#       else
                    static uno::Reference< XClipboard > s_xSelection;

                    if ( !s_xSelection.is() )
                         s_xSelection = uno::Reference< XClipboard >( xFactory->createInstance( OUString("com.sun.star.datatransfer.clipboard.GenericClipboardExt") ), UNO_QUERY );

                    if ( !s_xSelection.is() )
                         s_xSelection = uno::Reference< XClipboard >( xFactory->createInstance( OUString("com.sun.star.datatransfer.clipboard.GenericClipboard") ), UNO_QUERY );

                    mpWindowImpl->mpFrameData->mxSelection = s_xSelection;
#       endif
                }
            }

            // createInstance can throw any exception
            catch (const Exception&)
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

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > Window::GetAccessible( sal_Bool bCreate )
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
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc( GetComponentInterface( sal_True ), ::com::sun::star::uno::UNO_QUERY );
    return xAcc;
}

void Window::SetAccessible( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > x )
{
    mpWindowImpl->mxAccessible = x;
}

// skip all border windows that are no top level frames
sal_Bool Window::ImplIsAccessibleCandidate() const
{
    if( !mpWindowImpl->mbBorderWin )
        return sal_True;
    else
        // #101741 do not check for WB_CLOSEABLE because undecorated floaters (like menus!) are closeable
        if( mpWindowImpl->mbFrame && mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE) )
            return sal_True;
        else
            return sal_False;
}

sal_Bool Window::ImplIsAccessibleNativeFrame() const
{
    if( mpWindowImpl->mbFrame )
        // #101741 do not check for WB_CLOSEABLE because undecorated floaters (like menus!) are closeable
        if( (mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE)) )
            return sal_True;
        else
            return sal_False;
    else
        return sal_False;
}

sal_uInt16 Window::ImplGetAccessibleCandidateChildWindowCount( sal_uInt16 nFirstWindowType ) const
{
    sal_uInt16  nChildren = 0;
    Window* pChild = GetWindow( nFirstWindowType );
    while ( pChild )
    {
        if( pChild->ImplIsAccessibleCandidate() )
            nChildren++;
        else
            nChildren = sal::static_int_cast<sal_uInt16>(nChildren + pChild->ImplGetAccessibleCandidateChildWindowCount( WINDOW_FIRSTCHILD ));
        pChild = pChild->mpWindowImpl->mpNext;
    }
    return nChildren;
}

Window* Window::ImplGetAccessibleCandidateChild( sal_uInt16 nChild, sal_uInt16& rChildCount, sal_uInt16 nFirstWindowType, sal_Bool bTopLevel ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( bTopLevel )
        rChildCount = 0;

    Window* pChild = GetWindow( nFirstWindowType );
    while ( pChild )
    {
        Window *pTmpChild = pChild;

        if( !pChild->ImplIsAccessibleCandidate() )
            pTmpChild = pChild->ImplGetAccessibleCandidateChild( nChild, rChildCount, WINDOW_FIRSTCHILD, sal_False );

        if ( nChild == rChildCount )
            return pTmpChild;
        pChild = pChild->mpWindowImpl->mpNext;
        rChildCount++;
    }

    return NULL;
}

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

sal_uInt16 Window::GetAccessibleChildWindowCount()
{
    sal_uInt16 nChildren = 0;
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

Window* Window::GetAccessibleChildWindow( sal_uInt16 n )
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
    sal_uInt16 nChildren = n;
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


void Window::SetAccessibleRole( sal_uInt16 nRole )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;

    DBG_ASSERT( mpWindowImpl->mpAccessibleInfos->nAccessibleRole == 0xFFFF, "AccessibleRole already set!" );
    mpWindowImpl->mpAccessibleInfos->nAccessibleRole = nRole;
}

sal_uInt16 Window::GetAccessibleRole() const
{
    using namespace ::com::sun::star;

    sal_uInt16 nRole = mpWindowImpl->mpAccessibleInfos ? mpWindowImpl->mpAccessibleInfos->nAccessibleRole : 0xFFFF;
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
            case WINDOW_CALCINPUTLINE:
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

            case WINDOW_RULER:          nRole = accessibility::AccessibleRole::RULER; break;
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
                    nRole = accessibility::AccessibleRole::WINDOW;      // #106002#, contextmenus are windows (i.e. toplevel)
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

    DBG_ASSERT( !mpWindowImpl->mpAccessibleInfos->pAccessibleName || !rName.Len(), "AccessibleName already set!" );
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
            case WINDOW_MULTILINEEDIT:
            case WINDOW_PATTERNFIELD:
            case WINDOW_NUMERICFIELD:
            case WINDOW_METRICFIELD:
            case WINDOW_CURRENCYFIELD:
            case WINDOW_LONGCURRENCYFIELD:
            case WINDOW_CALCINPUTLINE:
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
            case WINDOW_METRICBOX:
            {
                Window *pLabel = GetAccessibleRelationLabeledBy();
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

void Window::SetAccessibleRelationLabeledBy( Window* pLabeledBy )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;
    mpWindowImpl->mpAccessibleInfos->pLabeledByWindow = pLabeledBy;
}

void Window::SetAccessibleRelationMemberOf( Window* pMemberOfWin )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;
    mpWindowImpl->mpAccessibleInfos->pMemberOfWindow = pMemberOfWin;
}

sal_Bool Window::IsAccessibilityEventsSuppressed( sal_Bool bTraverseParentPath )
{
    if( !bTraverseParentPath )
        return mpWindowImpl->mbSuppressAccessibilityEvents;
    else
    {
        Window *pParent = this;
        while ( pParent && pParent->mpWindowImpl)
        {
            if( pParent->mpWindowImpl->mbSuppressAccessibilityEvents )
                return sal_True;
            else
                pParent = pParent->mpWindowImpl->mpParent; // do not use GetParent() to find borderwindows that are frames
        }
        return sal_False;
    }
}

void Window::SetAccessibilityEventsSuppressed(sal_Bool bSuppressed)
{
    mpWindowImpl->mbSuppressAccessibilityEvents = bSuppressed;
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

void Window::DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, sal_Bool bChecked, sal_Bool bDrawBorder, sal_Bool bDrawExtBorderOnly )
{
    DrawSelectionBackground( rRect, highlight, bChecked, bDrawBorder, bDrawExtBorderOnly, 0, NULL, NULL );
}

void Window::DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, sal_Bool bChecked, sal_Bool bDrawBorder, sal_Bool bDrawExtBorderOnly, Color* pSelectionTextColor )
{
    DrawSelectionBackground( rRect, highlight, bChecked, bDrawBorder, bDrawExtBorderOnly, 0, pSelectionTextColor, NULL );
}

void Window::DrawSelectionBackground( const Rectangle& rRect,
                                      sal_uInt16 highlight,
                                      sal_Bool bChecked,
                                      sal_Bool bDrawBorder,
                                      sal_Bool bDrawExtBorderOnly,
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

    sal_Bool bDark = rStyles.GetFaceColor().IsDark();
    sal_Bool bBright = ( rStyles.GetFaceColor() == Color( COL_WHITE ) );

    int c1 = aSelectionBorderCol.GetLuminance();
    int c2 = GetDisplayBackground().GetColor().GetLuminance();

    if( !bDark && !bBright && abs( c2-c1 ) < (pPaintColor ? 40 : 75) )
    {
        // constrast too low
        sal_uInt16 h,s,b;
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

    sal_uInt16 nPercent = 0;
    if( !highlight )
    {
        if( bDark )
            aSelectionFillCol = COL_BLACK;
        else
            nPercent = 80;  // just checked (light)
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
                nPercent = bRoundEdges ? 40 : 20;          // selected, pressed or checked ( very dark )
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
                nPercent = bRoundEdges ? 60 : 35;          // selected, pressed or checked ( very dark )
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
                nPercent = 70;          // selected ( dark )
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

// controls should return the window that gets the
// focus by default, so keyevents can be sent to that window directly
Window* Window::GetPreferredKeyInputWindow()
{
    return this;
}


sal_Bool Window::IsScrollable() const
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

sal_Bool Window::IsTopWindow() const
{
    if ( mpWindowImpl->mbInDtor )
        return sal_False;

    // topwindows must be frames or they must have a borderwindow which is a frame
    if( !mpWindowImpl->mbFrame && (!mpWindowImpl->mpBorderWindow || (mpWindowImpl->mpBorderWindow && !mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) ) )
        return sal_False;

    ImplGetWinData();
    if( mpWindowImpl->mpWinData->mnIsTopWindow == (sal_uInt16)~0)    // still uninitialized
    {
        // #113722#, cache result of expensive queryInterface call
        Window *pThisWin = (Window*)this;
        uno::Reference< XTopWindow > xTopWindow( pThisWin->GetComponentInterface(), UNO_QUERY );
        pThisWin->mpWindowImpl->mpWinData->mnIsTopWindow = xTopWindow.is() ? 1 : 0;
    }
    return mpWindowImpl->mpWinData->mnIsTopWindow == 1 ? sal_True : sal_False;
}

void Window::ImplMirrorFramePos( Point &pt ) const
{
    pt.X() = mpWindowImpl->mpFrame->maGeometry.nWidth-1-pt.X();
}

// frame based modal counter (dialogs are not modal to the whole application anymore)
sal_Bool Window::IsInModalMode() const
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

void Window::ImplIsInTaskPaneList( sal_Bool mbIsInTaskList )
{
    mpWindowImpl->mbIsInTaskPaneList = mbIsInTaskList;
}

void Window::ImplNotifyIconifiedState( sal_Bool bIconified )
{
    mpWindowImpl->mpFrameWindow->ImplCallEventListeners( bIconified ? VCLEVENT_WINDOW_MINIMIZE : VCLEVENT_WINDOW_NORMALIZE );
    // #109206# notify client window as well to have toolkit topwindow listeners notified
    if( mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow && mpWindowImpl->mpFrameWindow != mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow )
        mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow->ImplCallEventListeners( bIconified ? VCLEVENT_WINDOW_MINIMIZE : VCLEVENT_WINDOW_NORMALIZE );
}

sal_Bool Window::HasActiveChildFrame()
{
    sal_Bool bRet = sal_False;
    Window *pFrameWin = ImplGetSVData()->maWinData.mpFirstFrame;
    while( pFrameWin )
    {
        if( pFrameWin != mpWindowImpl->mpFrameWindow )
        {
            sal_Bool bDecorated = sal_False;
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
                    if( ImplIsChild( pChildFrame, sal_True ) )
                    {
                        bRet = sal_True;
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

void Window::EnableNativeWidget( sal_Bool bEnable )
{
    static const char* pNoNWF = getenv( "SAL_NO_NWF" );
    if( pNoNWF && *pNoNWF )
        bEnable = sal_False;

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

sal_Bool Window::IsNativeWidgetEnabled() const
{
    return ImplGetWinData()->mbEnableNativeWidget;
}

#ifdef WNT // see #140456#
#include <win/salframe.h>
#endif

uno::Reference< rendering::XCanvas > Window::ImplGetCanvas( const Size& rFullscreenSize,
                                                       bool        bFullscreen,
                                                       bool        bSpriteCanvas ) const
{
    // try to retrieve hard reference from weak member
    uno::Reference< rendering::XCanvas > xCanvas( mpWindowImpl->mxCanvas );

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
    aArg[ 4 ] = makeAny( uno::Reference< awt::XWindow >(
                             const_cast<Window*>(this)->GetComponentInterface(),
                             uno::UNO_QUERY ));

    uno::Reference< XMultiServiceFactory > xFactory = vcl::unohelper::GetMultiServiceFactory();

    // Create canvas instance with window handle
    // =========================================
    if ( xFactory.is() )
    {
        static ::vcl::DeleteUnoReferenceOnDeinit<lang::XMultiServiceFactory> xStaticCanvasFactory(
            uno::Reference<lang::XMultiServiceFactory>(
                xFactory->createInstance(
                    OUString( "com.sun.star.rendering.CanvasFactory" ) ),
                UNO_QUERY ));
        uno::Reference<lang::XMultiServiceFactory> xCanvasFactory(xStaticCanvasFactory.get());

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
                                 OUString( "com.sun.star.rendering.SpriteCanvas.MultiScreen" ) :
                                 OUString( "com.sun.star.rendering.Canvas" ),
                                 aArg ),
                             UNO_QUERY );

            }
            else
            {
#endif
                xCanvas.set( xCanvasFactory->createInstanceWithArguments(
                                 bSpriteCanvas ?
                                 OUString( "com.sun.star.rendering.SpriteCanvas" ) :
                                 OUString( "com.sun.star.rendering.Canvas" ),
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

uno::Reference< rendering::XCanvas > Window::GetCanvas() const
{
    return ImplGetCanvas( Size(), false, false );
}

uno::Reference< rendering::XSpriteCanvas > Window::GetSpriteCanvas() const
{
    uno::Reference< rendering::XSpriteCanvas > xSpriteCanvas(
        ImplGetCanvas( Size(), false, true ), uno::UNO_QUERY );
    return xSpriteCanvas;
}

void Window::ImplPaintToDevice( OutputDevice* i_pTargetOutDev, const Point& i_rPos )
{
    sal_Bool bRVisible = mpWindowImpl->mbReallyVisible;
    mpWindowImpl->mbReallyVisible = mpWindowImpl->mbVisible;
    sal_Bool bDevOutput = mbDevOutput;
    mbDevOutput = sal_True;

    long nOldDPIX = ImplGetDPIX();
    long nOldDPIY = ImplGetDPIY();
    mnDPIX = i_pTargetOutDev->ImplGetDPIX();
    mnDPIY = i_pTargetOutDev->ImplGetDPIY();
    sal_Bool bOutput = IsOutputEnabled();
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

    sal_Bool bVisible = mpWindowImpl->mbVisible;
    mpWindowImpl->mbVisible = sal_True;

    if( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->ImplPaintToDevice( pDev, rPos );
    else
        ImplPaintToDevice( pDev, rPos );

    mpWindowImpl->mbVisible = bVisible;

    if( pRealParent )
        SetParent( pRealParent );
}

rtl::OUString Window::GetSurroundingText() const
{
  return rtl::OUString();
}

Selection Window::GetSurroundingTextSelection() const
{
  return Selection( 0, 0 );
}

//Poor man's equivalent, when widget wants to renegotiate
//size, get parent dialog and call resize on it
void Window::queue_resize()
{
    Dialog *pParent = GetParentDialog();
    if (!pParent)
        return;
    if (pParent->IsInClose())
        return;
    if (!pParent->isLayoutEnabled())
        return;
    if (!pParent->IsReallyShown())
    {
        //resize dialog to fit requisition
        const Window *pContainer = pParent->GetChild(0);
        Size aSize = pContainer->get_preferred_size();

        Size aMax = pParent->GetOptimalSize(WINDOWSIZE_MAXIMUM);
        aSize.Width() = std::min(aMax.Width(), aSize.Width());
        aSize.Height() = std::min(aMax.Height(), aSize.Height());

        pParent->SetMinOutputSizePixel(aSize);
        pParent->SetSizePixel(aSize);
    }
    if (pParent->hasPendingLayout())
        return;
    pParent->Resize();
}

void Window::setChildAnyProperty(const rtl::OString &rString, const Any &rValue)
{
    m_aWidgetProperties[rString] <<= rValue;
}

uno::Any Window::getWidgetAnyProperty(const rtl::OString &rString) const
{
    uno::Any aAny;
    ChildPropertyMap::const_iterator aI = m_aWidgetProperties.find(rString);
    if (aI != m_aWidgetProperties.end())
        aAny = aI->second;
    return aAny;
}

Size Window::get_preferred_size() const
{
    rtl::OString sWidthRequest(RTL_CONSTASCII_STRINGPARAM("width-request"));
    rtl::OString sHeightRequest(RTL_CONSTASCII_STRINGPARAM("height-request"));
    sal_Int32 nWidth = getWidgetProperty<sal_Int32>(sWidthRequest, -1);
    sal_Int32 nHeight = getWidgetProperty<sal_Int32>(sHeightRequest, -1);
    Size aRet(nWidth, nHeight);
    if (aRet.Width() == -1 || aRet.Height() == -1)
    {
        Size aOptimal = GetOptimalSize(WINDOWSIZE_PREFERRED);
        if (aRet.Width() == -1)
            aRet.Width() = aOptimal.Width();
        if (aRet.Height() == -1)
            aRet.Height() = aOptimal.Height();
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
