/*************************************************************************
 *
 *  $RCSfile: window.cxx,v $
 *
 *  $Revision: 1.194 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:32:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALOBJ_HXX
#include <salobj.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALGTYPE_HXX
#include <salgtype.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALCTRLHANDLE_HXX
#include <salctrlhandle.hxx>
#endif

#include <unohelp.hxx>

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_WINDATA_HXX
#include <windata.hxx>
#endif
#ifndef _SV_DBGGUI_HXX
#include <dbggui.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_REGION_H
#include <region.h>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif
#ifndef _SV_CURSOR_HXX
#include <cursor.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_SYSWIN_HXX
#include <syswin.hxx>
#endif
#ifndef _SV_BRDWIN_HXX
#include <brdwin.hxx>
#endif
#ifndef _SV_HELPWIN_HXX
#include <helpwin.hxx>
#endif
#ifndef _SV_DOCKWIN_HXX
#include <dockwin.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_WALL_HXX
#include <wall.hxx>
#endif
#ifndef _VCL_FONTCFG_HXX
#include <fontcfg.hxx>
#endif
#include <sysdata.hxx>
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <button.hxx> // Button::GetStandardText
#endif
#include <com/sun/star/awt/XWindowPeer.hpp>

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XCANVAS_HPP_
#include <drafts/com/sun/star/rendering/XCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XCANVAS_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCE_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGET_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDISPLAYCONNECTION_HPP_
#include <com/sun/star/awt/XDisplayConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif

#include <unowrap.hxx>
#include <dndlcon.hxx>
#include <dndevdis.hxx>
#include <impbmpconv.hxx>

#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif



using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace com::sun;

using ::com::sun::star::awt::XTopWindow;

// =======================================================================

DBG_NAME( Window );

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

// define dtor for ImplDelData
ImplDelData::~ImplDelData()
{
    // #112873# auto remove of ImplDelData
    // due to this code actively calling ImplRemoveDel() is not mandatory anymore
    if( !mbDel && mpWindow )
    {
        // the window still exists but we were not removed
        mpWindow->ImplRemoveDel( this );
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
    Window* pChild = pWindow->mpFirstChild;
    while ( pChild )
    {
        if ( pChild->mpParent != pWindow )
            return "Child-Window-Parent wrong";
        pChild = pChild->mpNext;
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

    pSVData->maGDIData.mnRealAppFontX = pSVData->maGDIData.mnAppFontX;
    if ( pSVData->maAppData.mnDialogScaleX )
        pSVData->maGDIData.mnAppFontX += (pSVData->maGDIData.mnAppFontX*pSVData->maAppData.mnDialogScaleX)/100;
}

// -----------------------------------------------------------------------

bool Window::ImplCheckUIFont( const Font& rFont )
{
    ImplInitFontList();

    String aTestText;
    aTestText.Append( Button::GetStandardText( BUTTON_OK ) );
    aTestText.Append( Button::GetStandardText( BUTTON_CANCEL ) );
    aTestText.Append( Button::GetStandardText( BUTTON_YES ) );
    aTestText.Append( Button::GetStandardText( BUTTON_NO ) );
    aTestText.Append( Button::GetStandardText( BUTTON_RETRY ) );
    aTestText.Append( Button::GetStandardText( BUTTON_HELP ) );
    aTestText.Append( Button::GetStandardText( BUTTON_CLOSE ) );
    aTestText.Append( Button::GetStandardText( BUTTON_MORE ) );
    aTestText.Append( Button::GetStandardText( BUTTON_ABORT ) );

    return HasGlyphs( rFont, aTestText ) >= aTestText.Len();
}

// -----------------------------------------------------------------------

void Window::ImplUpdateGlobalSettings( AllSettings& rSettings, BOOL bCallHdl )
{
    // Verify availability of the configured UI font, otherwise choose "Andale Sans UI"
    String aUserInterfaceFont;
    bool bUseSystemFont = rSettings.GetStyleSettings().GetUseSystemUIFonts();

    // check whether system UI font can display a typical UI text
    if( bUseSystemFont )
        bUseSystemFont = ImplCheckUIFont( rSettings.GetStyleSettings().GetAppFont() );

    if ( !bUseSystemFont )
    {
        ImplInitFontList();
        String aConfigFont = vcl::DefaultFontConfigItem::get()->getUserInterfaceFont( rSettings.GetUILocale() );
        xub_StrLen nIndex = 0;
        while( nIndex != STRING_NOTFOUND )
        {
            String aName( aConfigFont.GetToken( 0, ';', nIndex ) );
            if ( aName.Len() && mpFrameData->mpFontList->FindFontFamily( aName ) )
            {
                aUserInterfaceFont = aConfigFont;
                break;
            }
        }

        if ( ! aUserInterfaceFont.Len() )
        {
            String aFallbackFont (RTL_CONSTASCII_USTRINGPARAM( "Andale Sans UI" ));
            if ( mpFrameData->mpFontList->FindFontFamily( aFallbackFont ) )
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
        maxFontheight = (int) ((( 8 * (double) GetDesktopRectPixel().getHeight()) / 600.) + 0.5);

    Font aFont = aStyleSettings.GetMenuFont();
    int defFontheight = aFont.GetHeight();
    if( defFontheight > maxFontheight )
        defFontheight = maxFontheight;

    // if the UI is korean, always use 9pt
    LanguageType aLang = Application::GetSettings().GetUILanguage();
    if( aLang == LANGUAGE_KOREAN || aLang == LANGUAGE_KOREAN_JOHAB )
        defFontheight = Max(9, defFontheight);

    // i22098, toolfont will be scaled differently to avoid bloated rulers and status bars for big fonts
    int toolfontheight = defFontheight;
    if( toolfontheight > 9 )
        toolfontheight = (defFontheight+8) / 2;

    aFont = aStyleSettings.GetAppFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetAppFont( aFont );
    //aFont = aStyleSettings.GetHelpFont();
    //aFont.SetHeight( defFontheight );
    //aStyleSettings.SetHelpFont( aFont );
    aFont = aStyleSettings.GetTitleFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetTitleFont( aFont );
    aFont = aStyleSettings.GetFloatTitleFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetFloatTitleFont( aFont );
    //aFont = aStyleSettings.GetMenuFont();
    //aFont.SetHeight( defFontheight );
    //aStyleSettings.SetMenuFont( aFont );

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

    rSettings.SetStyleSettings( aStyleSettings );


    // #104427# auto detect HC mode ?
    if( !rSettings.GetStyleSettings().GetHighContrastMode() )
    {
        sal_Bool bTmp, bAutoHCMode = sal_True;
        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithServiceFactory(
            vcl::unohelper::GetMultiServiceFactory(),
            OUString::createFromAscii( "org.openoffice.Office.Common/Accessibility" ) );    // note: case sensisitive !
        if ( aNode.isValid() )
        {
            ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString::createFromAscii( "AutoDetectSystemHC" ) );
            if( aValue >>= bTmp )
                bAutoHCMode = bTmp;
        }
        if( bAutoHCMode )
        {
            if( rSettings.GetStyleSettings().GetFaceColor().IsDark()
             || rSettings.GetStyleSettings().GetWindowColor().IsDark() )
            {
                StyleSettings aStyleSettings = rSettings.GetStyleSettings();
                aStyleSettings.SetHighContrastMode( TRUE );
                rSettings.SetStyleSettings( aStyleSettings );
            }
        }
    }

    // Detect if images in menus are allowed or not
    {
        sal_Bool bTmp, bUseImagesInMenus = sal_True;
        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithServiceFactory(
            vcl::unohelper::GetMultiServiceFactory(),
            OUString::createFromAscii( "org.openoffice.Office.Common/View/Menu" ) );    // note: case sensisitive !
        if ( aNode.isValid() )
        {
            ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString::createFromAscii( "ShowIconsInMenues" ) );
            if( aValue >>= bTmp )
                bUseImagesInMenus = bTmp;
        }

        StyleSettings aStyleSettings = rSettings.GetStyleSettings();
        aStyleSettings.SetUseImagesInMenus( bUseImagesInMenus );
        rSettings.SetStyleSettings( aStyleSettings );
    }

#ifdef DBG_UTIL
    // Evt. AppFont auf Fett schalten, damit man feststellen kann,
    // ob fuer die Texte auf anderen Systemen genuegend Platz
    // vorhanden ist
    if ( DbgIsBoldAppFont() )
    {
        StyleSettings aStyleSettings = rSettings.GetStyleSettings();
        Font aFont = aStyleSettings.GetAppFont();
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

void Window::ImplInitData( WindowType nType )
{
    meOutDevType        = OUTDEV_WINDOW;

    mpWinData           = NULL;         // Extra Window Data, that we dont need for all windows
    mpOverlapData       = NULL;         // Overlap Data
    mpFrameData         = NULL;         // Frame Data
    mpFrame             = NULL;         // Pointer to frame window
    mpSysObj            = NULL;
    mpFrameWindow       = NULL;         // window to top level parent (same as frame window)
    mpOverlapWindow     = NULL;         // first overlap parent
    mpBorderWindow      = NULL;         // Border-Window
    mpClientWindow      = NULL;         // Client-Window of a FrameWindow
    mpParent            = NULL;         // parent (inkl. BorderWindow)
    mpRealParent        = NULL;         // real parent (exkl. BorderWindow)
    mpFirstChild        = NULL;         // first child window
    mpLastChild         = NULL;         // last child window
    mpFirstOverlap      = NULL;         // first overlap window (only set in overlap windows)
    mpLastOverlap       = NULL;         // last overlap window (only set in overlap windows)
    mpPrev              = NULL;         // prev window
    mpNext              = NULL;         // next window
    mpNextOverlap       = NULL;         // next overlap window of frame
    mpLastFocusWindow   = NULL;         // window for focus restore
    mpDlgCtrlDownWindow = NULL;         // window for dialog control
    mpFirstDel          = NULL;         // Dtor notification list
    mpUserData          = NULL;         // user data
    mpCursor            = NULL;         // cursor
    mpControlFont       = NULL;         // font propertie
    mpVCLXWindow        = NULL;
    mpAccessibleInfos   = NULL;
    maControlForeground = Color( COL_TRANSPARENT );     // kein Foreground gesetzt
    maControlBackground = Color( COL_TRANSPARENT );     // kein Background gesetzt
    mnLeftBorder        = 0;            // left border
    mnTopBorder         = 0;            // top border
    mnRightBorder       = 0;            // right border
    mnBottomBorder      = 0;            // bottom border
    mnX                 = 0;            // X-Position to Parent
    mnY                 = 0;            // Y-Position to Parent
    mnAbsScreenX        = 0;            // absolute X-position on screen, used for RTL window positioning
    mnHelpId            = 0;            // help id
    mnUniqId            = 0;            // unique id
    mpChildClipRegion   = NULL;         // Child-Clip-Region when ClipChildren
    mpPaintRegion       = NULL;         // Paint-ClipRegion
    mnStyle             = 0;            // style (init in ImplInitWindow)
    mnPrevStyle         = 0;            // prevstyle (set in SetStyle)
    mnExtendedStyle     = 0;            // extended style (init in ImplInitWindow)
    mnPrevExtendedStyle = 0;            // prevstyle (set in SetExtendedStyle)
    mnType              = nType;        // type
    mnGetFocusFlags     = 0;            // Flags fuer GetFocus()-Aufruf
    mnWaitCount         = 0;            // Wait-Count (>1 == Warte-MousePointer)
    mnPaintFlags        = 0;            // Flags for ImplCallPaint
    mnParentClipMode    = 0;            // Flags for Parent-ClipChildren-Mode
    mnActivateMode      = 0;            // Wird bei System/Overlap-Windows umgesetzt
    mnDlgCtrlFlags      = 0;            // DialogControl-Flags
    mnLockCount         = 0;            // LockCount
    mbFrame             = FALSE;        // TRUE: Window is a frame window
    mbBorderWin         = FALSE;        // TRUE: Window is a border window
    mbOverlapWin        = FALSE;        // TRUE: Window is a overlap window
    mbSysWin            = FALSE;        // TRUE: SystemWindow is the base class
    mbDialog            = FALSE;        // TRUE: Dialog is the base class
    mbDockWin           = FALSE;        // TRUE: DockingWindow is the base class
    mbFloatWin          = FALSE;        // TRUE: FloatingWindow is the base class
    mbPushButton        = FALSE;        // TRUE: PushButton is the base class
    mbToolBox           = FALSE;        // TRUE: ToolBox is the base class
    mbMenuFloatingWindow= FALSE;        // TRUE: MenuFloatingWindow is the base class
    mbSplitter          = FALSE;        // TRUE: Splitter is the base class
    mbVisible           = FALSE;        // TRUE: Show( TRUE ) called
    mbOverlapVisible    = FALSE;        // TRUE: Hide called for visible window from ImplHideAllOverlapWindow()
    mbDisabled          = FALSE;        // TRUE: Enable( FALSE ) called
    mbInputDisabled     = FALSE;        // TRUE: EnableInput( FALSE ) called
    mbAlwaysEnableInput = FALSE;        // TRUE: AlwaysEnableInput( TRUE ) called
    mbDropDisabled      = FALSE;        // TRUE: Drop is enabled
    mbNoUpdate          = FALSE;        // TRUE: SetUpdateMode( FALSE ) called
    mbNoParentUpdate    = FALSE;        // TRUE: SetParentUpdateMode( FALSE ) called
    mbActive            = FALSE;        // TRUE: Window Active
    mbParentActive      = FALSE;        // TRUE: OverlapActive from Parent
    mbReallyVisible     = FALSE;        // TRUE: this and all parents to an overlaped window are visible
    mbReallyShown       = FALSE;        // TRUE: this and all parents to an overlaped window are shown
    mbInInitShow        = FALSE;        // TRUE: we are in InitShow
    mbChildNotify       = FALSE;        // TRUE: ChildNotify
    mbChildPtrOverwrite = FALSE;        // TRUE: PointerStyle overwrites Child-Pointer
    mbNoPtrVisible      = FALSE;        // TRUE: ShowPointer( FALSE ) called
    mbMouseMove         = FALSE;        // TRUE: BaseMouseMove called
    mbPaintFrame        = FALSE;        // TRUE: Paint is visible, but not painted
    mbInPaint           = FALSE;        // TRUE: Inside PaintHdl
    mbMouseButtonDown   = FALSE;        // TRUE: BaseMouseButtonDown called
    mbMouseButtonUp     = FALSE;        // TRUE: BaseMouseButtonUp called
    mbKeyInput          = FALSE;        // TRUE: BaseKeyInput called
    mbKeyUp             = FALSE;        // TRUE: BaseKeyUp called
    mbCommand           = FALSE;        // TRUE: BaseCommand called
    mbDefPos            = TRUE;         // TRUE: Position is not Set
    mbDefSize           = TRUE;         // TRUE: Size is not Set
    mbCallMove          = TRUE;         // TRUE: Move must be called by Show
    mbCallResize        = TRUE;         // TRUE: Resize must be called by Show
    mbWaitSystemResize  = TRUE;         // TRUE: Wait for System-Resize
    mbInitWinClipRegion = TRUE;         // TRUE: Calc Window Clip Region
    mbInitChildRegion   = FALSE;        // TRUE: InitChildClipRegion
    mbWinRegion         = FALSE;        // TRUE: Window Region
    mbClipChildren      = FALSE;        // TRUE: Child-Fenster muessen evt. geclippt werden
    mbClipSiblings      = FALSE;        // TRUE: Nebeneinanderliegende Child-Fenster muessen evt. geclippt werden
    mbChildTransparent  = FALSE;        // TRUE: Child-Fenster duerfen transparent einschalten (inkl. Parent-CLIPCHILDREN)
    mbPaintTransparent  = FALSE;        // TRUE: Paints muessen auf Parent ausgeloest werden
    mbMouseTransparent  = FALSE;        // TRUE: Window is transparent for Mouse
    mbDlgCtrlStart      = FALSE;        // TRUE: Ab hier eigenes Dialog-Control
    mbFocusVisible      = FALSE;        // TRUE: Focus Visible
    mbTrackVisible      = FALSE;        // TRUE: Tracking Visible
    mbControlForeground = FALSE;        // TRUE: Foreground-Property set
    mbControlBackground = FALSE;        // TRUE: Background-Property set
    mbAlwaysOnTop       = FALSE;        // TRUE: immer vor allen anderen normalen Fenstern sichtbar
    mbCompoundControl   = FALSE;        // TRUE: Zusammengesetztes Control => Listener...
    mbCompoundControlHasFocus = FALSE;  // TRUE: Zusammengesetztes Control hat irgendwo den Focus
    mbPaintDisabled     = FALSE;        // TRUE: Paint soll nicht ausgefuehrt werden
    mbAllResize         = FALSE;        // TRUE: Auch ResizeEvents mit 0,0 schicken
    mbInDtor            = FALSE;        // TRUE: Wir befinden uns im Window-Dtor
    mbExtTextInput      = FALSE;        // TRUE: ExtTextInput-Mode is active
    mbInFocusHdl        = FALSE;        // TRUE: Innerhalb vom GetFocus-Handler
    mbCreatedWithToolkit = FALSE;
    mbSuppressAccessibilityEvents = FALSE; // TRUE: do not send any accessibility events
    mbEnableRTL         = TRUE;         // TRUE: this outdev will be mirrored if RTL window layout (UI mirroring) is globally active
    mbDrawSelectionBackground = FALSE;  // TRUE: draws transparent window background to indicate (toolbox) selection
}

// -----------------------------------------------------------------------

void Window::ImplInit( Window* pParent, WinBits nStyle, const ::com::sun::star::uno::Any& aSystemWorkWindowToken )
{
    ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

void Window::ImplInit( Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData )
{
    DBG_ASSERT( mbFrame || pParent, "Window::Window(): pParent == NULL" );

    ImplSVData* pSVData = ImplGetSVData();
    Window*     pRealParent = pParent;

    // 3D-Look vererben
    if ( !mbOverlapWin && (pParent->GetStyle() & WB_3DLOOK) )
        nStyle |= WB_3DLOOK;

    // Wenn wir einen Border haben, muessen wir ein BorderWindow anlegen
    if ( !mbFrame && !mbBorderWin && !mpBorderWindow && (nStyle & WB_BORDER) )
    {
        ImplBorderWindow* pBorderWin = new ImplBorderWindow( pParent, nStyle & (WB_BORDER | WB_DIALOGCONTROL | WB_NODIALOGCONTROL) );
        ((Window*)pBorderWin)->mpClientWindow = this;
        pBorderWin->GetBorder( mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder );
        mpBorderWindow  = pBorderWin;
        pParent = mpBorderWindow;
    }

    // insert window in list
    ImplInsertWindow( pParent );
    mnStyle = nStyle;

    // Overlap-Window-Daten
    if ( mbOverlapWin )
    {
        mpOverlapData                   = new ImplOverlapData;
        mpOverlapData->mpSaveBackDev    = NULL;
        mpOverlapData->mpSaveBackRgn    = NULL;
        mpOverlapData->mpNextBackWin    = NULL;
        mpOverlapData->mnSaveBackSize   = 0;
        mpOverlapData->mbSaveBack       = FALSE;
        mpOverlapData->mnTopLevel       = 1;
    }

    // test for frame creation
    if ( mbFrame )
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
        if( ! (nFrameStyle & ~SAL_FRAME_STYLE_CLOSEABLE) && // closeable only is ok, useful for undecorated floaters
            ( mbFloatWin || ((GetType() == WINDOW_BORDERWINDOW) && ((ImplBorderWindow*)this)->mbFloatWindow) || (nStyle & WB_SYSTEMFLOATWIN) )
            )
            nFrameStyle = SAL_FRAME_STYLE_FLOAT; // hmmm, was '0' before ????
        else if( mbFloatWin )
            nFrameStyle |= SAL_FRAME_STYLE_TOOLWINDOW;

        if( nStyle & WB_INTROWIN )
            nFrameStyle |= SAL_FRAME_STYLE_INTRO;
        if( nStyle & WB_TOOLTIPWIN )
            nFrameStyle |= SAL_FRAME_STYLE_TOOLTIP;

        if( nStyle & WB_NOSHADOW )
            nFrameStyle |= SAL_FRAME_STYLE_NOSHADOW;

        SalFrame* pParentFrame = NULL;
        if ( pParent )
            pParentFrame = pParent->mpFrame;
        SalFrame* pFrame;
        if ( pSystemParentData )
            pFrame = pSVData->mpDefInst->CreateChildFrame( pSystemParentData, nFrameStyle | SAL_FRAME_STYLE_CHILD );
        else
            pFrame = pSVData->mpDefInst->CreateFrame( pParentFrame, nFrameStyle );
        if ( !pFrame )
            GetpApp()->Exception( EXC_SYSOBJNOTCREATED );
        pFrame->SetCallback( this, ImplWindowFrameProc );

        // set window frame data
        mpFrameData     = new ImplFrameData;
        mpFrame         = pFrame;
        mpFrameWindow   = this;
        mpOverlapWindow = this;

        // set frame data
        mpFrameData->mpNextFrame        = pSVData->maWinData.mpFirstFrame;
        pSVData->maWinData.mpFirstFrame = this;
        mpFrameData->mpFirstOverlap     = NULL;
        mpFrameData->mpFocusWin         = NULL;
        mpFrameData->mpMouseMoveWin     = NULL;
        mpFrameData->mpMouseDownWin     = NULL;
        mpFrameData->mpFirstBackWin     = NULL;
        mpFrameData->mpFontList         = pSVData->maGDIData.mpScreenFontList;
        mpFrameData->mpFontCache        = pSVData->maGDIData.mpScreenFontCache;
        mpFrameData->mnAllSaveBackSize  = 0;
        mpFrameData->mnFocusId          = 0;
        mpFrameData->mnMouseMoveId      = 0;
        mpFrameData->mnLastMouseX       = -1;
        mpFrameData->mnLastMouseY       = -1;
        mpFrameData->mnBeforeLastMouseX = -1;
        mpFrameData->mnBeforeLastMouseY = -1;
        mpFrameData->mnFirstMouseX      = -1;
        mpFrameData->mnFirstMouseY      = -1;
        mpFrameData->mnLastMouseWinX    = -1;
        mpFrameData->mnLastMouseWinY    = -1;
        mpFrameData->mnModalMode        = 0;
        mpFrameData->mnMouseDownTime    = 0;
        mpFrameData->mnClickCount       = 0;
        mpFrameData->mnFirstMouseCode   = 0;
        mpFrameData->mnMouseCode        = 0;
        mpFrameData->mnMouseMode        = 0;
        mpFrameData->meMapUnit          = MAP_PIXEL;
        mpFrameData->mbHasFocus         = FALSE;
        mpFrameData->mbInMouseMove      = FALSE;
        mpFrameData->mbMouseIn          = FALSE;
        mpFrameData->mbStartDragCalled  = FALSE;
        mpFrameData->mbNeedSysWindow    = FALSE;
        mpFrameData->mbMinimized        = FALSE;
        mpFrameData->mbStartFocusState  = FALSE;
        mpFrameData->mbInSysObjFocusHdl = FALSE;
        mpFrameData->mbInSysObjToTopHdl = FALSE;
        mpFrameData->mbSysObjFocus      = FALSE;
        mpFrameData->maPaintTimer.SetTimeout( 30 );
        mpFrameData->maPaintTimer.SetTimeoutHdl( LINK( this, Window, ImplHandlePaintHdl ) );
        mpFrameData->mbInternalDragGestureRecognizer = FALSE;
        mpFrameData->mbTriggerHangulHanja = FALSE;
    }

    // init data
    mpRealParent = pRealParent;

    // #99318: make sure fontcache and list is available before call to SetSettings
    mpFontList      = mpFrameData->mpFontList;
    mpFontCache     = mpFrameData->mpFontCache;

    if ( mbFrame )
    {
        if ( pParent )
        {
            mpFrameData->mnDPIX     = pParent->mpFrameData->mnDPIX;
            mpFrameData->mnDPIY     = pParent->mpFrameData->mnDPIY;
            mpFrameData->mnFontDPIX = pParent->mpFrameData->mnFontDPIX;
            mpFrameData->mnFontDPIY = pParent->mpFrameData->mnFontDPIY;
        }
        else
        {
            if ( ImplGetGraphics() )
            {
                mpGraphics->GetResolution( mpFrameData->mnDPIX, mpFrameData->mnDPIY );
                mpGraphics->GetScreenFontResolution( mpFrameData->mnFontDPIX, mpFrameData->mnFontDPIY );
            }
        }

        // delay settings initialization until first "real" frame
        // this relies on the IntroWindow not needing any system settings
        if ( !pSVData->maAppData.mbSettingsInit &&
             ! (nStyle & WB_INTROWIN)
             )
        {
            mpFrame->UpdateSettings( *pSVData->maAppData.mpSettings );
            ImplUpdateGlobalSettings( *pSVData->maAppData.mpSettings );
            OutputDevice::SetSettings( *pSVData->maAppData.mpSettings );
            pSVData->maAppData.mbSettingsInit = TRUE;
        }

        // If we create a Window with default size, query this
        // size directly, because we want resize all Controls to
        // the correct size before we display the window
        if ( nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_APP) )
            mpFrame->GetClientSize( mnOutWidth, mnOutHeight );
    }
    else
    {
        if ( pParent )
        {
            if ( !ImplIsOverlapWindow() )
            {
                mbDisabled          = pParent->mbDisabled;
                mbInputDisabled     = pParent->mbInputDisabled;
                mbAlwaysEnableInput = pParent->mbAlwaysEnableInput;
            }

            OutputDevice::SetSettings( pParent->GetSettings() );
        }
    }

    const StyleSettings& rStyleSettings = maSettings.GetStyleSettings();
    USHORT nScreenZoom = rStyleSettings.GetScreenZoom();
    mnDPIX          = (mpFrameData->mnDPIX*nScreenZoom)/100;
    mnDPIY          = (mpFrameData->mnDPIY*nScreenZoom)/100;
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

    // calculate app font res
    if ( mbFrame && !pSVData->maGDIData.mnAppFontX && ! (nStyle & WB_INTROWIN) )
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
            DBG_ASSERT( mpFrame != pFrameWindow->mpFrame, "SetFrameParent to own" );
            DBG_ASSERT( mpFrame, "no frame" );
            SalFrame* pParentFrame = pParent ? pParent->mpFrame : NULL;
            pFrameWindow->mpFrame->SetParent( pParentFrame );
        }
        pFrameWindow = pFrameWindow->mpFrameData->mpNextFrame;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInsertWindow( Window* pParent )
{
    mpParent            = pParent;
    mpRealParent        = pParent;

    if ( pParent && !mbFrame )
    {
        // search frame window and set window frame data
        Window* pFrameParent = pParent->mpFrameWindow;
        mpFrameData     = pFrameParent->mpFrameData;
        mpFrame         = pFrameParent->mpFrame;
        mpFrameWindow   = pFrameParent;
        mbFrame         = FALSE;

        // search overlap window and insert window in list
        if ( ImplIsOverlapWindow() )
        {
            Window* pFirstOverlapParent = pParent;
            while ( !pFirstOverlapParent->ImplIsOverlapWindow() )
                pFirstOverlapParent = pFirstOverlapParent->ImplGetParent();
            mpOverlapWindow = pFirstOverlapParent;

            mpNextOverlap = mpFrameData->mpFirstOverlap;
            mpFrameData->mpFirstOverlap = this;

            // Overlap-Windows sind per default die obersten
            mpNext = pFirstOverlapParent->mpFirstOverlap;
            pFirstOverlapParent->mpFirstOverlap = this;
            if ( !pFirstOverlapParent->mpLastOverlap )
                pFirstOverlapParent->mpLastOverlap = this;
            else
                mpNext->mpPrev = this;
        }
        else
        {
            if ( pParent->ImplIsOverlapWindow() )
                mpOverlapWindow = pParent;
            else
                mpOverlapWindow = pParent->mpOverlapWindow;
            mpPrev = pParent->mpLastChild;
            pParent->mpLastChild = this;
            if ( !pParent->mpFirstChild )
                pParent->mpFirstChild = this;
            else
                mpPrev->mpNext = this;
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplRemoveWindow( BOOL bRemoveFrameData )
{
    // Fenster aus den Listen austragen
    if ( !mbFrame )
    {
        if ( ImplIsOverlapWindow() )
        {
            if ( mpFrameData->mpFirstOverlap == this )
                mpFrameData->mpFirstOverlap = mpNextOverlap;
            else
            {
                Window* pTempWin = mpFrameData->mpFirstOverlap;
                while ( pTempWin->mpNextOverlap != this )
                    pTempWin = pTempWin->mpNextOverlap;
                pTempWin->mpNextOverlap = mpNextOverlap;
            }

            if ( mpPrev )
                mpPrev->mpNext = mpNext;
            else
                mpOverlapWindow->mpFirstOverlap = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpOverlapWindow->mpLastOverlap = mpPrev;
        }
        else
        {
            if ( mpPrev )
                mpPrev->mpNext = mpNext;
            else
                mpParent->mpFirstChild = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpParent->mpLastChild = mpPrev;
        }

        mpPrev = NULL;
        mpNext = NULL;
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
    mbCallResize = FALSE;
    Resize();

    // Forward size to embedded java frame. As stated below, derived
    // classes don't always call our Resize, thus this extra step here
    // is required.
    if( mxCanvasWindow.is() )
        mxCanvasWindow->setPosSize( mnOutOffX, mnOutOffX,
                                    mnOutWidth, mnOutHeight, 0 );

    // #88419# Most classes don't call the base class in Resize() and Move(),
    // => Call ImpleResize/Move instead of Resize/Move directly...
    ImplCallEventListeners( VCLEVENT_WINDOW_RESIZE );
}

// -----------------------------------------------------------------------

void Window::ImplCallMove()
{
    mbCallMove = FALSE;

    if( mbFrame )
    {
        // update frame position
        SalFrame *mpParentFrame = NULL;
        Window *pParent = ImplGetParent();
        while( pParent )
        {
            if( pParent->mpFrame != mpFrame )
            {
                mpParentFrame = pParent->mpFrame;
                break;
            }
            pParent = pParent->GetParent();
        }

        SalFrameGeometry g = mpFrame->GetGeometry();
        maPos = Point( g.nX, g.nY );
        if( mpParentFrame )
        {
            g = mpParentFrame->GetGeometry();
            maPos -= Point( g.nX, g.nY );
        }
    }

    Move();

    // Forward physical position to embedded java frame. As stated
    // above, derived classes don't always call our Resize, thus this
    // extra step here is required.
    if( mxCanvasWindow.is() )
        mxCanvasWindow->setPosSize( mnOutOffX, mnOutOffX,
                                    mnOutWidth, mnOutHeight, 0 );

    ImplCallEventListeners( VCLEVENT_WINDOW_MOVE );
}

// -----------------------------------------------------------------------

static ULONG ImplAutoHelpID()
{
    if ( !Application::IsAutoHelpIdEnabled() )
        return 0;

    ULONG nHID = 0;

    ResMgr *pResMgr = Resource::GetResManager();

    DBG_ASSERT( pResMgr,          "MM hat gesagt, dass wir immer einen haben" );
    DBG_ASSERT( pResMgr->nTopRes, "MM hat gesagt, dass der Stack nie leer ist" );
    if( !pResMgr || pResMgr->nTopRes < 1 || pResMgr->nTopRes > 2 )
        return 0;

    const ImpRCStack *pRC = pResMgr->StackTop( pResMgr->nTopRes==1 ? 0 : 1 );

    DBG_ASSERT( pRC->pResource, "MM hat gesagt, dass der immer einen hat" );
    ULONG nGID = pRC->pResource->GetId();

    if( !nGID || nGID > 32767 )
        return 0;

    // GGGg gggg::gggg gggg::ggLL LLLl::llll llll
    switch( pRC->pResource->GetRT() ) { // maximal 7
        case RSC_DOCKINGWINDOW:
            nHID += 0x20000000L;
        case RSC_WORKWIN:
            nHID += 0x20000000L;
        case RSC_MODELESSDIALOG:
            nHID += 0x20000000L;
        case RSC_FLOATINGWINDOW:
            nHID += 0x20000000L;
        case RSC_MODALDIALOG:
            nHID += 0x20000000L;
        case RSC_TABPAGE:
            nHID += 0x20000000L;

            if( pResMgr->nTopRes == 2 ) {
                pRC = pResMgr->StackTop();
                ULONG nLID = pRC->pResource->GetId();

                if( !nLID || nLID > 511 )
                    return 0;

                switch( pRC->pResource->GetRT() ) { // maximal 32
                    case RSC_TABCONTROL:        nHID |= 0x0000; break;
                    case RSC_RADIOBUTTON:       nHID |= 0x0200; break;
                    case RSC_CHECKBOX:          nHID |= 0x0400; break;
                    case RSC_TRISTATEBOX:       nHID |= 0x0600; break;
                    case RSC_EDIT:              nHID |= 0x0800; break;
                    case RSC_MULTILINEEDIT:     nHID |= 0x0A00; break;
                    case RSC_MULTILISTBOX:      nHID |= 0x0C00; break;
                    case RSC_LISTBOX:           nHID |= 0x0E00; break;
                    case RSC_COMBOBOX:          nHID |= 0x1000; break;
                    case RSC_PUSHBUTTON:        nHID |= 0x1200; break;
                    case RSC_SPINFIELD:         nHID |= 0x1400; break;
                    case RSC_PATTERNFIELD:      nHID |= 0x1600; break;
                    case RSC_NUMERICFIELD:      nHID |= 0x1800; break;
                    case RSC_METRICFIELD:       nHID |= 0x1A00; break;
                    case RSC_CURRENCYFIELD:     nHID |= 0x1C00; break;
                    case RSC_DATEFIELD:         nHID |= 0x1E00; break;
                    case RSC_TIMEFIELD:         nHID |= 0x2000; break;
                    case RSC_IMAGERADIOBUTTON:  nHID |= 0x2200; break;
                    case RSC_NUMERICBOX:        nHID |= 0x2400; break;
                    case RSC_METRICBOX:         nHID |= 0x2600; break;
                    case RSC_CURRENCYBOX:       nHID |= 0x2800; break;
                    case RSC_DATEBOX:           nHID |= 0x2A00; break;
                    case RSC_TIMEBOX:           nHID |= 0x2C00; break;
                    case RSC_IMAGEBUTTON:       nHID |= 0x2E00; break;
                    case RSC_MENUBUTTON:        nHID |= 0x3000; break;
                    case RSC_MOREBUTTON:        nHID |= 0x3200; break;
                    default:
                        return 0;
                } // of switch
                nHID |= nLID;
            } // of if
            break;
        default:
            return 0;
    } // of switch
    nHID |= nGID << 14;
    return nHID;
}

// -----------------------------------------------------------------------

WinBits Window::ImplInitRes( const ResId& rResId )
{
    GetRes( rResId );

    char* pRes = (char*)GetClassRes();
    pRes += 4;
    ULONG nStyle = GetLongRes( (void*)pRes );
    ((ResId&)rResId).aWinBits = nStyle;
    return nStyle;
}

// -----------------------------------------------------------------------

void Window::ImplLoadRes( const ResId& rResId )
{
    // newer move this line after IncrementRes
    char* pRes = (char*)GetClassRes();
    pRes += 8;
    ULONG nHelpId = (ULONG)GetLongRes( (void*)pRes );
    if ( !nHelpId )
        nHelpId = ImplAutoHelpID();
    SetHelpId( nHelpId );

    USHORT nObjMask = (USHORT)ReadShortRes();

    // ResourceStyle
    USHORT nRSStyle = ReadShortRes();
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
            ePosMap = (MapUnit)(USHORT)ReadShortRes();
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
            eSizeMap = (MapUnit)(USHORT)ReadShortRes();
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
        SetHelpText( ReadStringRes() );
    if ( nObjMask & WINDOW_QUICKTEXT )
        SetQuickHelpText( ReadStringRes() );
    if ( nObjMask & WINDOW_EXTRALONG )
        SetData( (void*)ReadLongRes() );
    if ( nObjMask & WINDOW_UNIQUEID )
        SetUniqueId( (ULONG)ReadLongRes() );
}

// -----------------------------------------------------------------------

ImplWinData* Window::ImplGetWinData() const
{
    if ( !mpWinData )
    {
        static const char* pNoNWF = getenv( "SAL_NO_NWF" );

        ((Window*)this)->mpWinData = new ImplWinData;
        mpWinData->mpExtOldText     = NULL;
        mpWinData->mpExtOldAttrAry  = NULL;
        mpWinData->mpCursorRect     = 0;
        mpWinData->mnCursorExtWidth = 0;
        mpWinData->mpFocusRect      = NULL;
        mpWinData->mpTrackRect      = NULL;
        mpWinData->mnTrackFlags     = 0;
        mpWinData->mnIsTopWindow    = (USHORT) ~0;  // not initialized yet, 0/1 will indicate TopWindow (see IsTopWindow())
        mpWinData->mbMouseOver      = FALSE;
        mpWinData->mbEnableNativeWidget = (pNoNWF && *pNoNWF) ? FALSE : TRUE; // TRUE: try to draw this control with native theme API
        mpWinData->mpSalControlHandle  = NULL;
   }

    return mpWinData;
}

// -----------------------------------------------------------------------

SalGraphics* Window::ImplGetFrameGraphics() const
{
    if ( mpFrameWindow->mpGraphics )
        mpFrameWindow->mbInitClipRegion = TRUE;
    else
        mpFrameWindow->ImplGetGraphics();
    mpFrameWindow->mpGraphics->ResetClipRegion();
    return mpFrameWindow->mpGraphics;
}

// -----------------------------------------------------------------------

Window* Window::ImplFindWindow( const Point& rFramePos )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Window* pTempWindow;
    Window* pFindWindow;

    // Zuerst alle ueberlappenden Fenster ueberpruefen
    pTempWindow = mpFirstOverlap;
    while ( pTempWindow )
    {
        pFindWindow = pTempWindow->ImplFindWindow( rFramePos );
        if ( pFindWindow )
            return pFindWindow;
        pTempWindow = pTempWindow->mpNext;
    }

    // dann testen wir unser Fenster
    if ( !mbVisible )
        return NULL;

    USHORT nHitTest = ImplHitTest( rFramePos );
    if ( nHitTest & WINDOW_HITTEST_INSIDE )
    {
        // und danach gehen wir noch alle Child-Fenster durch
        pTempWindow = mpFirstChild;
        while ( pTempWindow )
        {
            pFindWindow = pTempWindow->ImplFindWindow( rFramePos );
            if ( pFindWindow )
                return pFindWindow;
            pTempWindow = pTempWindow->mpNext;
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
    if( ImplHasMirroredGraphics() && !IsRTLEnabled() )
    {
        // - RTL - re-mirror frame pos at this window
        ImplReMirror( aFramePos );
    }
    Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
    if ( !aRect.IsInside( aFramePos ) )
        return 0;
    if ( mbWinRegion )
    {
        Point aTempPos = aFramePos;
        aTempPos.X() -= mnOutOffX;
        aTempPos.Y() -= mnOutOffY;
        if ( !maWinRegion.IsInside( aTempPos ) )
            return 0;
    }

    USHORT nHitTest = WINDOW_HITTEST_INSIDE;
    if ( mbMouseTransparent )
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
    if ( mpFrameWindow != pWindow->mpFrameWindow )
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

    if ( IsEnabled() && IsInputEnabled() )
        ePointerStyle = GetPointer().GetStyle();
    else
        ePointerStyle = POINTER_ARROW;

    const Window* pWindow = this;
    do
    {
        // Wenn Pointer nicht sichtbar, dann wird suche abgebrochen, da
        // dieser Status nicht ueberschrieben werden darf
        if ( pWindow->mbNoPtrVisible )
            return POINTER_NULL;

        if ( !bWait )
        {
            if ( pWindow->mnWaitCount )
            {
                ePointerStyle = POINTER_WAIT;
                bWait = TRUE;
            }
            else
            {
                if ( pWindow->mbChildPtrOverwrite )
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
    mbDevOutput     = FALSE;
    mbReallyVisible = FALSE;
    mbReallyShown   = FALSE;

    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mbReallyVisible )
            pWindow->ImplResetReallyVisible();
        pWindow = pWindow->mpNext;
    }

    pWindow = mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mbReallyVisible )
            pWindow->ImplResetReallyVisible();
        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplSetReallyVisible()
{
    mbDevOutput     = TRUE;
    mbReallyVisible = TRUE;
    mbReallyShown   = TRUE;

    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mbVisible )
            pWindow->ImplSetReallyVisible();
        pWindow = pWindow->mpNext;
    }

    pWindow = mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mbVisible )
            pWindow->ImplSetReallyVisible();
        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallInitShow()
{
    mbReallyShown   = TRUE;
    mbInInitShow    = TRUE;
    StateChanged( STATE_CHANGE_INITSHOW );
    mbInInitShow    = FALSE;

    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mbVisible )
            pWindow->ImplCallInitShow();
        pWindow = pWindow->mpNext;
    }

    pWindow = mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mbVisible )
            pWindow->ImplCallInitShow();
        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplAddDel( ImplDelData* pDel )
{
    DBG_ASSERT( !pDel->mpWindow, "Window::ImplAddDel(): cannot add ImplDelData twice !" );
    if( !pDel->mpWindow )
    {
        pDel->mpWindow = this;  // #112873# store ref to this window, so pDel can remove itself
        pDel->mpNext = mpFirstDel;
        mpFirstDel = pDel;
    }
}

// -----------------------------------------------------------------------

void Window::ImplRemoveDel( ImplDelData* pDel )
{
    pDel->mpWindow = NULL;      // #112873# pDel is not associated with a Window anymore
    if ( mpFirstDel == pDel )
        mpFirstDel = pDel->mpNext;
    else
    {
        ImplDelData* pData = mpFirstDel;
        while ( pData->mpNext != pDel )
            pData = pData->mpNext;
        pData->mpNext = pDel->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInitResolutionSettings()
{
    // AppFont-Aufloesung und DPI-Aufloesung neu berechnen
    if ( mbFrame )
    {
        const StyleSettings& rStyleSettings = maSettings.GetStyleSettings();
        USHORT nScreenZoom = rStyleSettings.GetScreenZoom();
        mnDPIX = (mpFrameData->mnDPIX*nScreenZoom)/100;
        mnDPIY = (mpFrameData->mnDPIY*nScreenZoom)/100;
        SetPointFont( rStyleSettings.GetAppFont() );
    }
    else if ( mpParent )
    {
        mnDPIX  = mpParent->mnDPIX;
        mnDPIY  = mpParent->mnDPIY;
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
        aSize.Width() *= mpFrameData->mnFontDPIX;
        aSize.Width() += 72/2;
        aSize.Width() /= 72;
        aSize.Width() *= nScreenFontZoom;
        aSize.Width() /= 100;
    }
    aSize.Height() *= mpFrameData->mnFontDPIY;
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
        aSize.Width() += mpFrameData->mnFontDPIX/2;
        aSize.Width() /= mpFrameData->mnFontDPIX;
    }
    aSize.Height() *= 100;
    aSize.Height() /= nScreenFontZoom;
    aSize.Height() *= 72;
    aSize.Height() += mpFrameData->mnFontDPIY/2;
    aSize.Height() /= mpFrameData->mnFontDPIY;

    rFont.SetSize( aSize );
}

// -----------------------------------------------------------------------

BOOL Window::ImplSysObjClip( const Region* pOldRegion )
{
    BOOL bUpdate = TRUE;

    if ( mpSysObj )
    {
        BOOL bVisibleState = mbReallyVisible;

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

                if ( mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();

                Region      aRegion = *pWinChildClipRegion;
                Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                Region      aWinRectRegion( aWinRect );
                USHORT      nClipFlags = mpSysObj->GetClipRegionType();

                if ( aRegion == aWinRectRegion )
                    mpSysObj->ResetClipRegion();
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
                    mpSysObj->BeginSetClipRegion( nRectCount );
                    bRegionRect = aRegion.ImplGetFirstRect( aInfo, nX, nY, nWidth, nHeight );
                    while ( bRegionRect )
                    {
                        mpSysObj->UnionClipRegion( nX, nY, nWidth, nHeight );
                        bRegionRect = aRegion.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
                    }
                    mpSysObj->EndSetClipRegion();
                }
            }
            else
                bVisibleState = FALSE;
        }

        // Visible-Status updaten
        mpSysObj->Show( bVisibleState );
    }

    return bUpdate;
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjChildsClip()
{
    if ( mpSysObj && mbInitWinClipRegion )
        ImplSysObjClip( NULL );

    Window* pWindow = mpFirstChild;
    while ( pWindow )
    {
        pWindow->ImplUpdateSysObjChildsClip();
        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjOverlapsClip()
{
    ImplUpdateSysObjChildsClip();

    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        pWindow->ImplUpdateSysObjOverlapsClip();
        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjClip()
{
    if ( !ImplIsOverlapWindow() )
    {
        ImplUpdateSysObjChildsClip();

        // Schwestern muessen ihre ClipRegion auch neu berechnen
        if ( mbClipSiblings )
        {
            Window* pWindow = mpNext;
            while ( pWindow )
            {
                pWindow->ImplUpdateSysObjChildsClip();
                pWindow = pWindow->mpNext;
            }
        }
    }
    else
        mpFrameWindow->ImplUpdateSysObjOverlapsClip();
}

// -----------------------------------------------------------------------

BOOL Window::ImplSetClipFlagChilds( BOOL bSysObjOnlySmaller )
{
    BOOL bUpdate = TRUE;
    if ( mpSysObj )
    {
        Region* pOldRegion = NULL;
        if ( bSysObjOnlySmaller && !mbInitWinClipRegion )
            pOldRegion = new Region( maWinClipRegion );

        mbInitClipRegion = TRUE;
        mbInitWinClipRegion = TRUE;

        Window* pWindow = mpFirstChild;
        while ( pWindow )
        {
            if ( !pWindow->ImplSetClipFlagChilds( bSysObjOnlySmaller ) )
                bUpdate = FALSE;
            pWindow = pWindow->mpNext;
        }

        if ( !ImplSysObjClip( pOldRegion ) )
        {
            mbInitClipRegion = TRUE;
            mbInitWinClipRegion = TRUE;
            bUpdate = FALSE;
        }

        if ( pOldRegion )
            delete pOldRegion;
    }
    else
    {
    mbInitClipRegion = TRUE;
    mbInitWinClipRegion = TRUE;

    Window* pWindow = mpFirstChild;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagChilds( bSysObjOnlySmaller ) )
            bUpdate = FALSE;
        pWindow = pWindow->mpNext;
    }
    }
    return bUpdate;
}

// -----------------------------------------------------------------------

BOOL Window::ImplSetClipFlagOverlapWindows( BOOL bSysObjOnlySmaller )
{
    BOOL bUpdate = ImplSetClipFlagChilds( bSysObjOnlySmaller );

    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller ) )
            bUpdate = FALSE;
        pWindow = pWindow->mpNext;
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
             ((pParent->GetStyle() & WB_CLIPCHILDREN) || (mnParentClipMode & PARENTCLIPMODE_CLIP)) )
        {
            pParent->mbInitClipRegion = TRUE;
            pParent->mbInitChildRegion = TRUE;
        }

        // Schwestern muessen ihre ClipRegion auch neu berechnen
        if ( mbClipSiblings )
        {
            Window* pWindow = mpNext;
            while ( pWindow )
            {
                if ( !pWindow->ImplSetClipFlagChilds( bSysObjOnlySmaller ) )
                    bUpdate = FALSE;
                pWindow = pWindow->mpNext;
            }
        }

        return bUpdate;
    }
    else
        return mpFrameWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller );
}

// -----------------------------------------------------------------------

void Window::ImplIntersectWindowClipRegion( Region& rRegion )
{
    if ( mbInitWinClipRegion )
        ImplInitWinClipRegion();

    rRegion.Intersect( maWinClipRegion );
}

// -----------------------------------------------------------------------

void Window::ImplIntersectWindowRegion( Region& rRegion )
{
    rRegion.Intersect( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                  Size( mnOutWidth, mnOutHeight ) ) );
    if ( mbWinRegion )
        rRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );
}

// -----------------------------------------------------------------------

void Window::ImplExcludeWindowRegion( Region& rRegion )
{
    if ( mbWinRegion )
    {
        Point aPoint( mnOutOffX, mnOutOffY );
        Region aRegion( Rectangle( aPoint,
                                   Size( mnOutWidth, mnOutHeight ) ) );
        aRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );
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
    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mbReallyVisible )
        {
            pWindow->ImplExcludeWindowRegion( rRegion );
            pWindow->ImplExcludeOverlapWindows( rRegion );
        }

        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplExcludeOverlapWindows2( Region& rRegion )
{
    if ( mbReallyVisible )
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
        if ( !mbFrame )
            rRegion.Intersect( Rectangle( Point( 0, 0 ), Size( mpFrameWindow->mnOutWidth, mpFrameWindow->mnOutHeight ) ) );

        if ( bOverlaps && !rRegion.IsEmpty() )
        {
            // Clip Overlap Siblings
            Window* pStartOverlapWindow = this;
            while ( !pStartOverlapWindow->mbFrame )
            {
                Window* pOverlapWindow = pStartOverlapWindow->mpOverlapWindow->mpFirstOverlap;
                while ( pOverlapWindow && (pOverlapWindow != pStartOverlapWindow) )
                {
                    pOverlapWindow->ImplExcludeOverlapWindows2( rRegion );
                    pOverlapWindow = pOverlapWindow->mpNext;
                }
                pStartOverlapWindow = pStartOverlapWindow->mpOverlapWindow;
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
    Window* pWindow = mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mbReallyVisible )
        {
            // ParentClipMode-Flags auswerten
            USHORT nClipMode = pWindow->GetParentClipMode();
            if ( !(nClipMode & PARENTCLIPMODE_NOCLIP) &&
                 ((nClipMode & PARENTCLIPMODE_CLIP) || (GetStyle() & WB_CLIPCHILDREN)) )
                pWindow->ImplExcludeWindowRegion( rRegion );
            else
                bOtherClip = TRUE;
        }

        pWindow = pWindow->mpNext;
    }

    return bOtherClip;
}

// -----------------------------------------------------------------------

void Window::ImplClipAllChilds( Region& rRegion )
{
    Window* pWindow = mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mbReallyVisible )
            pWindow->ImplExcludeWindowRegion( rRegion );
        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplClipSiblings( Region& rRegion )
{
    Window* pWindow = ImplGetParent()->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow == this )
            break;

        if ( pWindow->mbReallyVisible )
            pWindow->ImplExcludeWindowRegion( rRegion );

        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInitWinClipRegion()
{
    // Build Window Region
    maWinClipRegion = Rectangle( Point( mnOutOffX, mnOutOffY ),
                                 Size( mnOutWidth, mnOutHeight ) );
    if ( mbWinRegion )
        maWinClipRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );

    // ClipSiblings
    if ( mbClipSiblings && !ImplIsOverlapWindow() )
        ImplClipSiblings( maWinClipRegion );

    // Clip Parent Boundaries
    ImplClipBoundaries( maWinClipRegion, FALSE, TRUE );

    // Clip Children
    if ( (GetStyle() & WB_CLIPCHILDREN) || mbClipChildren )
        mbInitChildRegion = TRUE;

    mbInitWinClipRegion = FALSE;
}

// -----------------------------------------------------------------------

void Window::ImplInitWinChildClipRegion()
{
    if ( !mpFirstChild )
    {
        if ( mpChildClipRegion )
        {
            delete mpChildClipRegion;
            mpChildClipRegion = NULL;
        }
    }
    else
    {
        if ( !mpChildClipRegion )
            mpChildClipRegion = new Region( maWinClipRegion );
        else
            *mpChildClipRegion = maWinClipRegion;

        ImplClipChilds( *mpChildClipRegion );
    }

    mbInitChildRegion = FALSE;
}

// -----------------------------------------------------------------------

Region* Window::ImplGetWinChildClipRegion()
{
    if ( mbInitWinClipRegion )
        ImplInitWinClipRegion();
    if ( mbInitChildRegion )
        ImplInitWinChildClipRegion();
    if ( mpChildClipRegion )
        return mpChildClipRegion;
    else
        return &maWinClipRegion;
}

// -----------------------------------------------------------------------

void Window::ImplIntersectAndUnionOverlapWindows( const Region& rInterRegion, Region& rRegion )
{
    Window* pWindow = mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mbReallyVisible )
        {
            Region aTempRegion( rInterRegion );
            pWindow->ImplIntersectWindowRegion( aTempRegion );
            rRegion.Union( aTempRegion );
            pWindow->ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
        }

        pWindow = pWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplIntersectAndUnionOverlapWindows2( const Region& rInterRegion, Region& rRegion )
{
    if ( mbReallyVisible )
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
        pStartOverlapWindow = mpOverlapWindow;
    else
        pStartOverlapWindow = this;
    while ( !pStartOverlapWindow->mbFrame )
    {
        Window* pOverlapWindow = pStartOverlapWindow->mpOverlapWindow->mpFirstOverlap;
        while ( pOverlapWindow && (pOverlapWindow != pStartOverlapWindow) )
        {
            pOverlapWindow->ImplIntersectAndUnionOverlapWindows2( rInterRegion, rRegion );
            pOverlapWindow = pOverlapWindow->mpNext;
        }
        pStartOverlapWindow = pStartOverlapWindow->mpOverlapWindow;
    }

    // Clip Child Overlap Windows
    if ( !ImplIsOverlapWindow() )
        mpOverlapWindow->ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
    else
        ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
}

// -----------------------------------------------------------------------

void Window::ImplCalcOverlapRegion( const Rectangle& rSourceRect, Region& rRegion,
                                    BOOL bChilds, BOOL bParent, BOOL bSiblings )
{
    Region  aRegion( rSourceRect );
    if ( mbWinRegion )
        rRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );
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
        if ( !pWindow->mbFrame )
        {
            aTempRegion = aRegion;
            aTempRegion.Exclude( Rectangle( Point( 0, 0 ), Size( mpFrameWindow->mnOutWidth, mpFrameWindow->mnOutHeight ) ) );
            rRegion.Union( aTempRegion );
        }
    }

    // Siblings
    if ( bSiblings && !ImplIsOverlapWindow() )
    {
        pWindow = mpParent->mpFirstChild;
        do
        {
            if ( pWindow->mbReallyVisible && (pWindow != this) )
            {
                aTempRegion = aRegion;
                pWindow->ImplIntersectWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
            }
            pWindow = pWindow->mpNext;
        }
        while ( pWindow );
    }

    // Childs
    if ( bChilds )
    {
        pWindow = mpFirstChild;
        while ( pWindow )
        {
            if ( pWindow->mbReallyVisible )
            {
                aTempRegion = aRegion;
                pWindow->ImplIntersectWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
            }
            pWindow = pWindow->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallPaint( const Region* pRegion, USHORT nPaintFlags )
{
    mbPaintFrame = FALSE;

    if ( nPaintFlags & IMPL_PAINT_PAINTALLCHILDS )
        mnPaintFlags |= IMPL_PAINT_PAINT | IMPL_PAINT_PAINTALLCHILDS | (nPaintFlags & IMPL_PAINT_PAINTALL);
    if ( nPaintFlags & IMPL_PAINT_PAINTCHILDS )
        mnPaintFlags |= IMPL_PAINT_PAINTCHILDS;
    if ( nPaintFlags & IMPL_PAINT_ERASE )
        mnPaintFlags |= IMPL_PAINT_ERASE;
    if ( nPaintFlags & IMPL_PAINT_CHECKRTL )
        mnPaintFlags |= IMPL_PAINT_CHECKRTL;
    if ( !mpFirstChild )
        mnPaintFlags &= ~IMPL_PAINT_PAINTALLCHILDS;

    if ( mbPaintDisabled )
    {
        if ( mnPaintFlags & IMPL_PAINT_PAINTALL )
            Invalidate( INVALIDATE_NOCHILDREN | INVALIDATE_NOERASE | INVALIDATE_NOTRANSPARENT | INVALIDATE_NOCLIPCHILDREN );
        else if ( pRegion )
            Invalidate( *pRegion, INVALIDATE_NOCHILDREN | INVALIDATE_NOERASE | INVALIDATE_NOTRANSPARENT | INVALIDATE_NOCLIPCHILDREN );
        return;
    }

    nPaintFlags = mnPaintFlags & ~(IMPL_PAINT_PAINT);

    Region* pChildRegion = NULL;
    Rectangle aSelectionRect;
    if ( mnPaintFlags & IMPL_PAINT_PAINT )
    {
        Region* pWinChildClipRegion = ImplGetWinChildClipRegion();
        if ( mnPaintFlags & IMPL_PAINT_PAINTALL )
            maInvalidateRegion = *pWinChildClipRegion;
        else
        {
            if ( pRegion )
                maInvalidateRegion.Union( *pRegion );

            if( mpWinData && mbTrackVisible )
                /* #98602# need to repaint all children within the
               * tracking rectangle, so the following invert
               * operation takes places without traces of the previous
               * one.
               */
                maInvalidateRegion.Union( *mpWinData->mpTrackRect );

            if ( mnPaintFlags & IMPL_PAINT_PAINTALLCHILDS )
                pChildRegion = new Region( maInvalidateRegion );
            maInvalidateRegion.Intersect( *pWinChildClipRegion );
        }
        mnPaintFlags = 0;
        if ( !maInvalidateRegion.IsEmpty() )
        {
            if ( mpCursor )
                mpCursor->ImplHide();

            mbInitClipRegion = TRUE;
            mbInPaint = TRUE;

            // Paint-Region zuruecksetzen
            Region      aPaintRegion( maInvalidateRegion );
            Rectangle   aPaintRect = aPaintRegion.GetBoundRect();

            // - RTL - re-mirror paint rect and region at this window
            if( ImplHasMirroredGraphics() && !IsRTLEnabled() )
            {
                ImplReMirror( aPaintRect );
                ImplReMirror( aPaintRegion );
            }
            aPaintRect = ImplDevicePixelToLogic( aPaintRect);
            mpPaintRegion = &aPaintRegion;
            maInvalidateRegion.SetEmpty();

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
            if( mbDrawSelectionBackground )
                aSelectionRect = aPaintRect;

            Paint( aPaintRect );

            if ( mpWinData )
            {
                if ( mbFocusVisible )
                    ImplInvertFocus( *(mpWinData->mpFocusRect) );
            }
            mbInPaint = FALSE;
            mbInitClipRegion = TRUE;
            mpPaintRegion = NULL;
            if ( mpCursor )
                mpCursor->ImplShow( FALSE );
        }
    }
    else
        mnPaintFlags = 0;

    if ( nPaintFlags & (IMPL_PAINT_PAINTALLCHILDS | IMPL_PAINT_PAINTCHILDS) )
    {
        // die Childfenster ausgeben
        Window* pTempWindow = mpFirstChild;
        while ( pTempWindow )
        {
            if ( pTempWindow->mbVisible )
                pTempWindow->ImplCallPaint( pChildRegion, nPaintFlags );
            pTempWindow = pTempWindow->mpNext;
        }
    }

    if ( mpWinData && mbTrackVisible && (mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
        /* #98602# need to invert the tracking rect AFTER
        * the children have painted
        */
        InvertTracking( *(mpWinData->mpTrackRect), mpWinData->mnTrackFlags );

    // #98943# draw toolbox selection
    if( !aSelectionRect.IsEmpty() )
        DrawSelectionBackground( aSelectionRect, 2, FALSE, TRUE, FALSE );

    if ( pChildRegion )
        delete pChildRegion;
}

// -----------------------------------------------------------------------

void Window::ImplCallOverlapPaint()
{
    // Zuerst geben wir die ueberlappenden Fenster aus
    Window* pTempWindow = mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->mbReallyVisible )
            pTempWindow->ImplCallOverlapPaint();
        pTempWindow = pTempWindow->mpNext;
    }

    // und dann erst uns selber
    if ( mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDS) )
    {
        // - RTL - notify ImplCallPaint to check for re-mirroring (CHECKRTL)
        //         because we were called from the Sal layer
        ImplCallPaint( NULL, mnPaintFlags /*| IMPL_PAINT_CHECKRTL */);
    }
}

// -----------------------------------------------------------------------

void Window::ImplPostPaint()
{
    if ( !mpFrameData->maPaintTimer.IsActive() )
        mpFrameData->maPaintTimer.Start();
}

// -----------------------------------------------------------------------

IMPL_LINK( Window, ImplHandlePaintHdl, void*, EMPTYARG )
{
    if ( mbReallyVisible )
        ImplCallOverlapPaint();
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
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mnPaintFlags & IMPL_PAINT_PAINTCHILDS )
                break;
            pTempWindow->mnPaintFlags |= IMPL_PAINT_PAINTCHILDS;
        }
        while ( !pTempWindow->ImplIsOverlapWindow() );
    }

    // Paint-Flags setzen
    mnPaintFlags |= IMPL_PAINT_PAINT;
    if ( nFlags & INVALIDATE_CHILDREN )
        mnPaintFlags |= IMPL_PAINT_PAINTALLCHILDS;
    if ( !(nFlags & INVALIDATE_NOERASE) )
        mnPaintFlags |= IMPL_PAINT_ERASE;
    if ( !pRegion )
        mnPaintFlags |= IMPL_PAINT_PAINTALL;

    // Wenn nicht alles neu ausgegeben werden muss, dann die Region
    // dazupacken
    if ( !(mnPaintFlags & IMPL_PAINT_PAINTALL) )
        maInvalidateRegion.Union( *pRegion );

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
    Window* pTempWindow = mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->IsVisible() )
            pTempWindow->ImplInvalidateOverlapFrameRegion( rRegion );

        pTempWindow = pTempWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInvalidateParentFrameRegion( Region& rRegion )
{
    if ( mbOverlapWin )
        mpFrameWindow->ImplInvalidateOverlapFrameRegion( rRegion );
    else
        ImplGetParent()->ImplInvalidateFrameRegion( &rRegion, INVALIDATE_CHILDREN );
}

// -----------------------------------------------------------------------

void Window::ImplInvalidate( const Region* pRegion, USHORT nFlags )
{

    // Hintergrund-Sicherung zuruecksetzen
    if ( mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    // Feststellen, was neu ausgegeben werden muss
    BOOL bInvalidateAll = !pRegion;

    // Transparent-Invalidate beruecksichtigen
    Window* pWindow = this;
    if ( (mbPaintTransparent && !(nFlags & INVALIDATE_NOTRANSPARENT)) || (nFlags & INVALIDATE_TRANSPARENT) )
    {
        Window* pTempWindow = pWindow->ImplGetParent();
        while ( pTempWindow )
        {
            if ( !pTempWindow->IsPaintTransparent() )
            {
                pWindow = pTempWindow;
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
        if ( pWindow->GetStyle() & WB_CLIPCHILDREN )
            nFlags |= INVALIDATE_NOCHILDREN;
        else
            nFlags |= INVALIDATE_CHILDREN;
    }
    if ( (nFlags & INVALIDATE_NOCHILDREN) && pWindow->mpFirstChild )
        bInvalidateAll = FALSE;
    if ( bInvalidateAll )
        pWindow->ImplInvalidateFrameRegion( NULL, nFlags );
    else
    {
        Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aRegion( aRect );
        if ( pRegion )
        {
            // --- RTL --- remirror region before intersecting it
            if ( ImplHasMirroredGraphics() && !IsRTLEnabled() )
            {
                Region aRgn( *pRegion );
                ImplReMirror( aRgn );
                aRegion.Intersect( aRgn );
            }
            else
                aRegion.Intersect( *pRegion );
        }
        pWindow->ImplClipBoundaries( aRegion, TRUE, TRUE );
        if ( nFlags & INVALIDATE_NOCHILDREN )
        {
            nFlags &= ~INVALIDATE_CHILDREN;
            if ( !(nFlags & INVALIDATE_NOCLIPCHILDREN) )
            {
                if ( nOrgFlags & INVALIDATE_NOCHILDREN )
                    pWindow->ImplClipAllChilds( aRegion );
                else
                {
                    if ( pWindow->ImplClipChilds( aRegion ) )
                        nFlags |= INVALIDATE_CHILDREN;
                }
            }
        }
        if ( !aRegion.IsEmpty() )
            pWindow->ImplInvalidateFrameRegion( &aRegion, nFlags );
    }

    if ( nFlags & INVALIDATE_UPDATE )
        pWindow->Update();
}

// -----------------------------------------------------------------------

void Window::ImplMoveInvalidateRegion( const Rectangle& rRect,
                                       long nHorzScroll, long nVertScroll,
                                       BOOL bChilds )
{
    if ( (mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTALL)) == IMPL_PAINT_PAINT )
    {
        Region aTempRegion = maInvalidateRegion;
        aTempRegion.Intersect( rRect );
        aTempRegion.Move( nHorzScroll, nVertScroll );
        maInvalidateRegion.Union( aTempRegion );
    }

    if ( bChilds && (mnPaintFlags & IMPL_PAINT_PAINTCHILDS) )
    {
        Window* pWindow = mpFirstChild;
        while ( pWindow )
        {
            pWindow->ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, TRUE );
            pWindow = pWindow->mpNext;
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
            if ( pPaintAllWindow->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDS )
            {
                if ( pPaintAllWindow->mnPaintFlags & IMPL_PAINT_PAINTALL )
                {
                    aPaintAllRegion.SetEmpty();
                    break;
                }
                else
                    aPaintAllRegion.Union( pPaintAllWindow->maInvalidateRegion );
            }
        }
        while ( !pPaintAllWindow->ImplIsOverlapWindow() );
        if ( !aPaintAllRegion.IsEmpty() )
        {
            aPaintAllRegion.Move( nHorzScroll, nVertScroll );
            USHORT nPaintFlags = 0;
            if ( bChilds )
                mnPaintFlags |= INVALIDATE_CHILDREN;
            ImplInvalidateFrameRegion( &aPaintAllRegion, nPaintFlags );
        }
    }
}

// -----------------------------------------------------------------------

void Window::ImplValidateFrameRegion( const Region* pRegion, USHORT nFlags )
{
    if ( !pRegion )
        maInvalidateRegion.SetEmpty();
    else
    {
        // Wenn alle Childfenster neu ausgegeben werden muessen,
        // dann invalidieren wir diese vorher
        if ( (mnPaintFlags & IMPL_PAINT_PAINTALLCHILDS) && mpFirstChild )
        {
            Region aChildRegion = maInvalidateRegion;
            if ( mnPaintFlags & IMPL_PAINT_PAINTALL )
            {
                Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                aChildRegion = aRect;
            }
            Window* pChild = mpFirstChild;
            while ( pChild )
            {
                pChild->Invalidate( aChildRegion, INVALIDATE_CHILDREN | INVALIDATE_NOTRANSPARENT );
                pChild = pChild->mpNext;
            }
        }
        if ( mnPaintFlags & IMPL_PAINT_PAINTALL )
        {
            Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
            maInvalidateRegion = aRect;
        }
        maInvalidateRegion.Exclude( *pRegion );
    }
    mnPaintFlags &= ~IMPL_PAINT_PAINTALL;

    if ( nFlags & VALIDATE_CHILDREN )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            pChild->ImplValidateFrameRegion( pRegion, nFlags );
            pChild = pChild->mpNext;
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
    if ( (nFlags & VALIDATE_NOCHILDREN) && mpFirstChild )
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
    if ( mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    if ( mpCursor )
        mpCursor->ImplHide();

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

    if ( !mpFirstChild )
        bScrollChilds = FALSE;

    // --- RTL --- check if this window requires special action
    BOOL bReMirror = ( ImplHasMirroredGraphics() && !IsRTLEnabled() );

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
    if ( mbWinRegion )
        aRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );

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
        if ( mpWinData )
        {
            if ( mbFocusVisible )
                ImplInvertFocus( *(mpWinData->mpFocusRect) );
            if ( mbTrackVisible && (mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
                InvertTracking( *(mpWinData->mpTrackRect), mpWinData->mnTrackFlags );
        }

        SalGraphics* pGraphics = ImplGetFrameGraphics();
        if ( pGraphics )
        {
            if( bReMirror )
            {
                // --- RTL --- frame coordinates require re-mirroring
                ImplReMirror( aRegion );
            }

            ImplSelectClipRegion( pGraphics, aRegion, this );
            pGraphics->CopyArea( rRect.Left()+nHorzScroll, rRect.Top()+nVertScroll,
                                 rRect.Left(), rRect.Top(),
                                 rRect.GetWidth(), rRect.GetHeight(),
                                 SAL_COPYAREA_WINDOWINVALIDATE, this );
        }

        if ( mpWinData )
        {
            if ( mbFocusVisible )
                ImplInvertFocus( *(mpWinData->mpFocusRect) );
            if ( mbTrackVisible && (mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
                InvertTracking( *(mpWinData->mpTrackRect), mpWinData->mnTrackFlags );
        }
    }

    if ( !aInvalidateRegion.IsEmpty() )
    {
        // --- RTL --- the invalidate region for this windows is already computed in frame coordinates
        // so it has to be re-mirrored before calling the Paint-handler
        mnPaintFlags |= IMPL_PAINT_CHECKRTL;

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
        Rectangle aDestRect( rRect );
        Window* pWindow = mpFirstChild;
        while ( pWindow )
        {
            Rectangle aWinRect( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                                Size( pWindow->mnOutWidth, pWindow->mnOutHeight ) );
            if ( aDestRect.IsOver( aWinRect ) )
            {
                pWindow->mnX        += nHorzScroll;
                pWindow->maPos.X()  += nHorzScroll;
                pWindow->mnY        += nVertScroll;
                pWindow->maPos.Y()  += nVertScroll;
                if ( pWindow->ImplUpdatePos() )
                    pWindow->ImplUpdateSysObjPos();
                if ( pWindow->IsReallyVisible() )
                    pWindow->ImplSetClipFlag();
                if ( pWindow->mpClientWindow )
                    pWindow->mpClientWindow->maPos = pWindow->maPos;

                if ( pWindow->IsVisible() )
                {
                    pWindow->ImplCallMove();
                }
                else
                {
                    pWindow->mbCallMove = TRUE;
                }
            }
            pWindow = pWindow->mpNext;
        }
    }

    if ( nFlags & SCROLL_UPDATE )
        Update();

    if ( mpCursor )
        mpCursor->ImplShow( FALSE );
}

// -----------------------------------------------------------------------

void Window::ImplUpdateAll( BOOL bOverlapWindows )
{
    if ( !mbReallyVisible )
        return;

    BOOL bFlush = FALSE;
    if ( mpFrameWindow->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mbFrame || (mpBorderWindow && mpBorderWindow->mbFrame) )
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
        if ( pWindow->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDS) )
            pWindow->ImplCallPaint( NULL, pWindow->mnPaintFlags );
    }

    if ( bFlush )
        Flush();
}

// -----------------------------------------------------------------------

void Window::ImplUpdateWindowPtr( Window* pWindow )
{
    if ( mpFrameWindow != pWindow->mpFrameWindow )
    {
        // Graphic freigeben
        ImplReleaseGraphics();
    }

    mpFrameData     = pWindow->mpFrameData;
    mpFrame         = pWindow->mpFrame;
    mpFrameWindow   = pWindow->mpFrameWindow;
    if ( pWindow->ImplIsOverlapWindow() )
        mpOverlapWindow = pWindow;
    else
        mpOverlapWindow = pWindow->mpOverlapWindow;

    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateWindowPtr( pWindow );
        pChild = pChild->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateWindowPtr()
{
    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateWindowPtr( this );
        pChild = pChild->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplUpdateOverlapWindowPtr( BOOL bNewFrame )
{
    BOOL bVisible = IsVisible();
    Show( FALSE );
    ImplRemoveWindow( bNewFrame );
    Window* pRealParent = mpRealParent;
    ImplInsertWindow( ImplGetParent() );
    mpRealParent = pRealParent;
    ImplUpdateWindowPtr();
    if ( ImplUpdatePos() )
        ImplUpdateSysObjPos();

    if ( bNewFrame )
    {
        Window* pOverlapWindow = mpFirstOverlap;
        while ( pOverlapWindow )
        {
            Window* pNextOverlapWindow = pOverlapWindow->mpNext;
            pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
            pOverlapWindow = pNextOverlapWindow;
        }
    }

    if ( bVisible )
        Show( TRUE );
}

// -----------------------------------------------------------------------

BOOL Window::ImplUpdatePos()
{
    BOOL bSysChild = FALSE;

    if ( ImplIsOverlapWindow() )
    {
        mnOutOffX  = mnX;
        mnOutOffY  = mnY;
    }
    else
    {
        Window* pParent = ImplGetParent();

        mnOutOffX  = mnX + pParent->mnOutOffX;
        mnOutOffY  = mnY + pParent->mnOutOffY;
    }

    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        if ( pChild->ImplUpdatePos() )
            bSysChild = TRUE;
        pChild = pChild->mpNext;
    }

    if ( mpSysObj )
        bSysChild = TRUE;

    return bSysChild;
}

// -----------------------------------------------------------------------

void Window::ImplUpdateSysObjPos()
{
    if ( mpSysObj )
        mpSysObj->SetPosSize( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight );

    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateSysObjPos();
        pChild = pChild->mpNext;
    }
}
// -----------------------------------------------------------------------

void Window::ImplPosSizeWindow( long nX, long nY,
                                long nWidth, long nHeight, USHORT nFlags )
{
    BOOL    bNewPos         = FALSE;
    BOOL    bNewSize        = FALSE;
    BOOL    bNewWidth       = FALSE;
    BOOL    bCopyBits       = FALSE;
    long    nOldOutOffX     = mnOutOffX;
    long    nOldOutOffY     = mnOutOffY;
    long    nOldOutWidth    = mnOutWidth;
    long    nOldOutHeight   = mnOutHeight;
    Region* pOverlapRegion  = NULL;
    Region* pOldRegion      = NULL;

    if ( IsReallyVisible() )
    {
        if ( mpFrameData->mpFirstBackWin )
            ImplInvalidateAllOverlapBackgrounds();

        Rectangle aOldWinRect( Point( nOldOutOffX, nOldOutOffY ),
                               Size( nOldOutWidth, nOldOutHeight ) );
        pOldRegion = new Region( aOldWinRect );
        if ( mbWinRegion )
            pOldRegion->Intersect( ImplPixelToDevicePixel( maWinRegion ) );

        if ( mnOutWidth && mnOutHeight && !mbPaintTransparent &&
             !mbInitWinClipRegion && !maWinClipRegion.IsEmpty() &&
             !HasPaintEvent() )
            bCopyBits = TRUE;
    }

    BOOL bnXRecycled = FALSE; // avoid duplicate mirroring in RTL case
    if ( nFlags & WINDOW_POSSIZE_WIDTH )
    {
        if(!( nFlags & WINDOW_POSSIZE_X ))
        {
            nX = mnX;
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
            bNewWidth = TRUE;
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
        //if ( nX != mnX )
        // --- RTL ---  (compare the screen coordinates)
        Point aPtDev( Point( nX+mnOutOffX, 0 ) );
        if( ImplHasMirroredGraphics() )
        {
            mpGraphics->mirror( aPtDev.X(), this );

            // #106948# always mirror our pos if our parent is not mirroring, even
            // if we are also not mirroring
            // --- RTL --- check if parent is in different coordinates
            if( !bnXRecycled && mpParent && !mpParent->mbFrame && mpParent->ImplHasMirroredGraphics() && !mpParent->IsRTLEnabled() )
            {
                // --- RTL --- (re-mirror at parent window)
                nX = mpParent->mnOutWidth - mnOutWidth - nX;
            }
        }
        if ( mnAbsScreenX != aPtDev.X() || nX != mnX )
        {
            if ( bCopyBits && !pOverlapRegion )
            {
                pOverlapRegion = new Region();
                ImplCalcOverlapRegion( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                                  Size( mnOutWidth, mnOutHeight ) ),
                                       *pOverlapRegion, FALSE, TRUE, TRUE );
            }
            mnX = nX;
            maPos.X() = nOrgX;
            mnAbsScreenX = aPtDev.X();  // --- RTL --- (store real screen pos)
            bNewPos = TRUE;
        }
    }
    if ( nFlags & WINDOW_POSSIZE_Y )
    {
        if ( nY != mnY )
        {
            if ( bCopyBits && !pOverlapRegion )
            {
                pOverlapRegion = new Region();
                ImplCalcOverlapRegion( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                                  Size( mnOutWidth, mnOutHeight ) ),
                                       *pOverlapRegion, FALSE, TRUE, TRUE );
            }
            mnY = nY;
            maPos.Y() = nY;
            bNewPos = TRUE;
        }
    }

/*    if ( nFlags & (WINDOW_POSSIZE_X|WINDOW_POSSIZE_Y) )
    {
        POINT aPt;
        aPt.x = maPos.X();
        aPt.y = maPos.Y();
        ClientToScreen( mpFrame->maFrameData.mhWnd , &aPt );
        maPos.X() = aPt.x;
        maPos.Y() = aPt.y;
    }
*/
    if ( bNewPos || bNewSize )
    {
        BOOL bUpdateSysObjPos = FALSE;
        if ( bNewPos )
            bUpdateSysObjPos = ImplUpdatePos();

        if ( mpClientWindow )
        {
            mpClientWindow->ImplPosSizeWindow( mpClientWindow->mnLeftBorder,
                                               mpClientWindow->mnTopBorder,
                                               mnOutWidth-mpClientWindow->mnLeftBorder-mpClientWindow->mnRightBorder,
                                               mnOutHeight-mpClientWindow->mnTopBorder-mpClientWindow->mnBottomBorder,
                                               WINDOW_POSSIZE_X | WINDOW_POSSIZE_Y |
                                               WINDOW_POSSIZE_WIDTH | WINDOW_POSSIZE_HEIGHT );
            // Wenn wir ein ClientWindow haben, dann hat dieses fuer die
            // Applikation auch die Position des FloatingWindows
            mpClientWindow->maPos = maPos;
            if ( bNewPos )
            {
                if ( mpClientWindow->IsVisible() )
                {
                    mpClientWindow->ImplCallMove();
                }
                else
                {
                    mpClientWindow->mbCallMove = TRUE;
                }
            }
        }
        else
        {
            if ( mpBorderWindow )
                maPos = mpBorderWindow->maPos;
        }

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
                mbCallMove = TRUE;
            if ( bNewSize )
                mbCallResize = TRUE;
        }

        BOOL bUpdateSysObjClip = FALSE;
        if ( IsReallyVisible() )
        {
            if ( bNewPos || bNewSize )
            {
                // Hintergrund-Sicherung zuruecksetzen
                if ( mpOverlapData && mpOverlapData->mpSaveBackDev )
                    ImplDeleteOverlapBackground();
                if ( mpFrameData->mpFirstBackWin )
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
                        bParentPaint = mpParent->IsPaintEnabled();
                    if ( bCopyBits && bParentPaint && !HasPaintEvent() )
                    {
                        Point aPoint( mnOutOffX, mnOutOffY );
                        Region aRegion( Rectangle( aPoint,
                                                   Size( mnOutWidth, mnOutHeight ) ) );
                        if ( mbWinRegion )
                            aRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );
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
                                BOOL bSelectClipRegion = ImplSelectClipRegion( pGraphics, aRegion, this );
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
                    if ( mbWinRegion )
                        aRegion.Intersect( ImplPixelToDevicePixel( maWinRegion ) );
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
                if ( !mbPaintTransparent )
                    ImplExcludeWindowRegion( aRegion );
                ImplClipBoundaries( aRegion, FALSE, TRUE );
                if ( !aRegion.IsEmpty() && !mpBorderWindow )
                    ImplInvalidateParentFrameRegion( aRegion );
            }
        }

        // System-Objekte anpassen
        if ( bUpdateSysObjClip )
            ImplUpdateSysObjClip();
        if ( bUpdateSysObjPos )
            ImplUpdateSysObjPos();
        if ( bNewSize && mpSysObj )
            mpSysObj->SetPosSize( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight );
    }

    if ( pOverlapRegion )
        delete pOverlapRegion;
    if ( pOldRegion )
        delete pOldRegion;
}

// -----------------------------------------------------------------------

void Window::ImplToBottomChild()
{
    if ( !ImplIsOverlapWindow() && !mbReallyVisible && (mpParent->mpLastChild != this) )
    {
        // Fenster an das Ende der Liste setzen
        if ( mpPrev )
            mpPrev->mpNext = mpNext;
        else
            mpParent->mpFirstChild = mpNext;
        mpNext->mpPrev = mpPrev;
        mpPrev = mpParent->mpLastChild;
        mpParent->mpLastChild = this;
        mpPrev->mpNext = this;
        mpNext = NULL;
    }
}

// -----------------------------------------------------------------------

void Window::ImplCalcToTop( ImplCalcToTopData* pPrevData )
{
    DBG_ASSERT( ImplIsOverlapWindow(), "Window::ImplCalcToTop(): Is not a OverlapWindow" );

    if ( !mbFrame )
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

    Window* pOverlap = mpFirstOverlap;
    while ( pOverlap )
    {
        pOverlap->ImplCalcToTop( pPrevData );
        if ( pPrevData->mpNext )
            pPrevData = pPrevData->mpNext;
        pOverlap = pOverlap->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplToTop( USHORT nFlags )
{
    DBG_ASSERT( ImplIsOverlapWindow(), "Window::ImplToTop(): Is not a OverlapWindow" );

    if ( mbFrame )
    {
        // Wenn in das externe Fenster geklickt wird, ist dieses
        // dafuer zustaendig dafuer zu sorgen, das unser Frame
        // nach vorne kommt
        if ( !mpFrameData->mbHasFocus &&
             !mpFrameData->mbSysObjFocus &&
             !mpFrameData->mbInSysObjFocusHdl &&
             !mpFrameData->mbInSysObjToTopHdl )
        {
            // do not bring floating windows on the client to top
            if( !ImplGetClientWindow() || !(ImplGetClientWindow()->GetStyle() & WB_SYSTEMFLOATWIN) )
            {
                USHORT nSysFlags = 0;
                if ( nFlags & TOTOP_RESTOREWHENMIN )
                    nSysFlags = SAL_FRAME_TOTOP_RESTOREWHENMIN;
                if ( nFlags & TOTOP_FOREGROUNDTASK )
                    nSysFlags = SAL_FRAME_TOTOP_FOREGROUNDTASK;
                if ( nFlags & TOTOP_GRABFOCUSONLY )
                    nSysFlags = SAL_FRAME_TOTOP_GRABFOCUS_ONLY;
                mpFrame->ToTop( nSysFlags );
            }
        }
    }
    else
    {
        if ( mpOverlapWindow->mpFirstOverlap != this )
        {
            // Fenster aus der Liste entfernen
            mpPrev->mpNext = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpOverlapWindow->mpLastOverlap = mpPrev;

            // AlwaysOnTop beruecksichtigen
            BOOL    bOnTop = IsAlwaysOnTopEnabled();
            Window* pNextWin = mpOverlapWindow->mpFirstOverlap;
            if ( !bOnTop )
            {
                while ( pNextWin )
                {
                    if ( !pNextWin->IsAlwaysOnTopEnabled() )
                        break;
                    pNextWin = pNextWin->mpNext;
                }
            }

            // TopLevel abpruefen
            BYTE nTopLevel = mpOverlapData->mnTopLevel;
            while ( pNextWin )
            {
                if ( (bOnTop != pNextWin->IsAlwaysOnTopEnabled()) ||
                     (nTopLevel <= pNextWin->mpOverlapData->mnTopLevel) )
                    break;
                pNextWin = pNextWin->mpNext;
            }

            // Fenster in die Liste wieder eintragen
            mpNext = pNextWin;
            if ( pNextWin )
            {
                mpPrev = pNextWin->mpPrev;
                pNextWin->mpPrev = this;
            }
            else
            {
                mpPrev = mpOverlapWindow->mpLastOverlap;
                mpOverlapWindow->mpLastOverlap = this;
            }
            if ( mpPrev )
                mpPrev->mpNext = this;
            else
                mpOverlapWindow->mpFirstOverlap = this;

            // ClipRegion muss von diesem Fenster und allen weiteren
            // ueberlappenden Fenstern neu berechnet werden.
            if ( IsReallyVisible() )
            {
                // Hintergrund-Sicherung zuruecksetzen
                if ( mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                mpOverlapWindow->ImplSetClipFlagOverlapWindows();
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
        pOverlapWindow = mpOverlapWindow;

    // Zuerst die Paint-Bereiche berechnen
    Window* pTempOverlapWindow = pOverlapWindow;
    aStartData.mpNext = NULL;
    pCurData = &aStartData;
    do
    {
        pTempOverlapWindow->ImplCalcToTop( pCurData );
        if ( pCurData->mpNext )
            pCurData = pCurData->mpNext;
        pTempOverlapWindow = pTempOverlapWindow->mpOverlapWindow;
    }
    while ( !pTempOverlapWindow->mbFrame );
    // Dann die Paint-Bereiche der ChildOverlap-Windows berechnen
    pTempOverlapWindow = mpFirstOverlap;
    while ( pTempOverlapWindow )
    {
        pTempOverlapWindow->ImplCalcToTop( pCurData );
        if ( pCurData->mpNext )
            pCurData = pCurData->mpNext;
        pTempOverlapWindow = pTempOverlapWindow->mpNext;
    }

    // Dann die Fenster-Verkettung aendern
    pTempOverlapWindow = pOverlapWindow;
    do
    {
        pTempOverlapWindow->ImplToTop( nFlags );
        pTempOverlapWindow = pTempOverlapWindow->mpOverlapWindow;
    }
    while ( !pTempOverlapWindow->mbFrame );
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
            if ( !pFocusWindow->mpBorderWindow )
            {
                if ( pFocusWindow->mnActivateMode & ACTIVATE_MODE_GRABFOCUS )
                    break;
            }
            pFocusWindow = pFocusWindow->ImplGetParent();
        }
        if ( (pFocusWindow->mnActivateMode & ACTIVATE_MODE_GRABFOCUS) &&
             !pFocusWindow->HasChildPathFocus( TRUE ) )
            pFocusWindow->GrabFocus();
    }

    if ( bReallyVisible )
        ImplGenerateMouseMove();
}

// -----------------------------------------------------------------------

void Window::ImplShowAllOverlaps()
{
    Window* pOverlapWindow = mpFirstOverlap;
    while ( pOverlapWindow )
    {
        if ( pOverlapWindow->mbOverlapVisible )
        {
            pOverlapWindow->Show( TRUE, SHOW_NOACTIVATE );
            pOverlapWindow->mbOverlapVisible = FALSE;
        }

        pOverlapWindow = pOverlapWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplHideAllOverlaps()
{
    Window* pOverlapWindow = mpFirstOverlap;
    while ( pOverlapWindow )
    {
        if ( pOverlapWindow->IsVisible() )
        {
            pOverlapWindow->mbOverlapVisible = TRUE;
            pOverlapWindow->Show( FALSE );
        }

        pOverlapWindow = pOverlapWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallMouseMove( USHORT nMouseCode, BOOL bModChanged )
{
    if ( mpFrameData->mbMouseIn && mpFrameWindow->mbReallyVisible )
    {
        ULONG   nTime   = Time::GetSystemTicks();
        long    nX      = mpFrameData->mnLastMouseX;
        long    nY      = mpFrameData->mnLastMouseY;
        USHORT  nCode   = nMouseCode;
        USHORT  nMode   = mpFrameData->mnMouseMode;
        BOOL    bLeave;
        // Auf MouseLeave testen
        if ( ((nX < 0) || (nY < 0) ||
              (nX >= mpFrameWindow->mnOutWidth) ||
              (nY >= mpFrameWindow->mnOutHeight)) &&
             !ImplGetSVData()->maWinData.mpCaptureWin )
            bLeave = TRUE;
        else
            bLeave = FALSE;
        nMode |= MOUSE_SYNTHETIC;
        if ( bModChanged )
            nMode |= MOUSE_MODIFIERCHANGED;
        ImplHandleMouseEvent( mpFrameWindow, EVENT_MOUSEMOVE, bLeave, nX, nY, nTime, nCode, nMode );
    }
}

// -----------------------------------------------------------------------

void Window::ImplGenerateMouseMove()
{
    if ( !mpFrameData->mnMouseMoveId )
        Application::PostUserEvent( mpFrameData->mnMouseMoveId, LINK( mpFrameWindow, Window, ImplGenerateMouseMoveHdl ) );
}

// -----------------------------------------------------------------------

IMPL_LINK( Window, ImplGenerateMouseMoveHdl, void*, EMPTYARG )
{
    mpFrameData->mnMouseMoveId = 0;
    ImplCallMouseMove( mpFrameData->mnMouseCode );
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
                pSVData->maWinData.mpLastDeacWin->mbActive = FALSE;
                pSVData->maWinData.mpLastDeacWin->Deactivate();
                if ( pLastRealWindow != pSVData->maWinData.mpLastDeacWin )
                {
                    pLastRealWindow->mbActive = TRUE;
                    pLastRealWindow->Activate();
                }
            }
            pSVData->maWinData.mpLastDeacWin = NULL;
        }
    }

    if ( bCallDeactivate )
    {
        if( pOldOverlapWindow->mbActive )
        {
            pOldOverlapWindow->mbActive = FALSE;
            pOldOverlapWindow->Deactivate();
        }
        if ( pOldRealWindow != pOldOverlapWindow )
        {
            if( pOldRealWindow->mbActive )
            {
                pOldRealWindow->mbActive = FALSE;
                pOldRealWindow->Deactivate();
            }
        }
    }
    if ( bCallActivate && ! pNewOverlapWindow->mbActive )
    {
        if( ! pNewOverlapWindow->mbActive )
        {
            pNewOverlapWindow->mbActive = TRUE;
            pNewOverlapWindow->Activate();
        }
        if ( pNewRealWindow != pNewOverlapWindow )
        {
            if( ! pNewRealWindow->mbActive )
            {
                pNewRealWindow->mbActive = TRUE;
                pNewRealWindow->Activate();
            }
        }
    }
}

// -----------------------------------------------------------------------
void Window::ImplGrabFocus( USHORT nFlags )
{
    // Es soll immer das Client-Fenster den Focus bekommen. Falls
    // wir mal auch Border-Fenstern den Focus geben wollen,
    // muessten wir bei allen GrabFocus()-Aufrufen in VCL dafuer
    // sorgen, das es an der Stelle gemacht wird. Dies wuerde
    // beispielsweise bei ToTop() der Fall sein.

    if ( mpClientWindow )
    {
        // Wegen nicht ganz durchdachtem Konzept muessen wir hier
        // leider noch einen Hack einbauen, damit wenn Dialoge
        // geschlossen werden der Focus wieder auf das richtige
        // Fenster zurueckgesetzt wird
        if ( mpLastFocusWindow && (mpLastFocusWindow != this) &&
             !(mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) &&
             mpLastFocusWindow->IsEnabled() && mpLastFocusWindow->IsInputEnabled() )
            mpLastFocusWindow->GrabFocus();
        else
            mpClientWindow->GrabFocus();
        return;
    }
    else if ( mbFrame )
    {
        // Wegen nicht ganz durchdachtem Konzept muessen wir hier
        // leider noch einen Hack einbauen, damit wenn Dialoge
        // geschlossen werden der Focus wieder auf das richtige
        // Fenster zurueckgesetzt wird
        if ( mpLastFocusWindow && (mpLastFocusWindow != this) &&
             !(mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) &&
             mpLastFocusWindow->IsEnabled() && mpLastFocusWindow->IsInputEnabled() )
        {
            mpLastFocusWindow->GrabFocus();
            return;
        }
    }

    // If the Windows is disabled, then we doesn't change the focus
    if ( !IsEnabled() || !IsInputEnabled() )
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
        if( pFrame != mpFrameWindow && pFrame->mpFrameData->mnFocusId )
        {
            bAsyncFocusWaiting = TRUE;
            break;
        }
        pFrame = pFrame->mpFrameData->mpNextFrame;
    }

    BOOL bHasFocus = TRUE;
        if ( !mpSysObj && !mpFrameData->mbHasFocus )
            bHasFocus = FALSE;

    BOOL bMustNotGrabFocus = FALSE;
    // #100242#, check parent hierarchy if some floater prohibits grab focus

    Window *pParent = this;
    while( pParent )
    {
        // #102158#, ignore grabfocus only if the floating parent grabs keyboard focus by itself (GrabsFocus())
        // otherwise we cannot set the focus in a floating toolbox
        if( ( (pParent->mbFloatWin && ((FloatingWindow*)pParent)->GrabsFocus()) || ( pParent->GetStyle() & WB_SYSTEMFLOATWIN ) ) && !( pParent->GetStyle() & WB_MOVEABLE ) )
        {
            bMustNotGrabFocus = TRUE;
            break;
        }
        pParent = pParent->mpParent;
    }


    if ( ( pSVData->maWinData.mpFocusWin != this && ! mbInDtor ) || ( bAsyncFocusWaiting && !bHasFocus && !bMustNotGrabFocus ) )
    {
        // EndExtTextInput if it is not the same window
        if ( pSVData->maWinData.mpExtTextInputWin &&
             (pSVData->maWinData.mpExtTextInputWin != this) )
            pSVData->maWinData.mpExtTextInputWin->EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );

        // Dieses Fenster als letztes FocusWindow merken
        Window* pOverlapWindow = ImplGetFirstOverlapWindow();
        pOverlapWindow->mpLastFocusWindow = this;
        mpFrameData->mpFocusWin = this;

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
                mpFrame->ToTop( SAL_FRAME_TOTOP_GRABFOCUS | SAL_FRAME_TOTOP_GRABFOCUS_ONLY );
                return;
            }
        }

        Window* pOldFocusWindow = pSVData->maWinData.mpFocusWin;

        pSVData->maWinData.mpFocusWin = this;

        if ( pOldFocusWindow )
        {
            // Cursor hiden
            if ( pOldFocusWindow->mpCursor )
                pOldFocusWindow->mpCursor->ImplHide();
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
            pNewOverlapWindow->mbActive = TRUE;
            pNewOverlapWindow->Activate();
            if ( pNewRealWindow != pNewOverlapWindow )
            {
                pNewRealWindow->mbActive = TRUE;
                pNewRealWindow->Activate();
            }
        }
/*
        // call Deactivate and Activate
        Window* pDeactivateParent;
        Window* pActivateParent;
        Window* pParent;
        Window* pLastParent;
        pDeactivateParent = pOldFocusWindow;
        while ( pDeactivateParent )
        {
            pParent = pDeactivateParent;
            if ( pParent->ImplIsChild( this ) )
                break;

            if ( pDeactivateParent->ImplIsOverlapWindow() )
            {
                if ( !pDeactivateParent->mbParentActive )
                    break;
            }

            pDeactivateParent = pDeactivateParent->ImplGetParent();
        }
        if ( pOldFocusWindow )
        {
            pActivateParent = this;
            while ( pActivateParent )
            {
                pParent = pActivateParent;
                if ( pParent->ImplIsChild( pOldFocusWindow ) )
                    break;

                if ( pActivateParent->ImplIsOverlapWindow() )
                {
                    if ( !pActivateParent->mbParentActive )
                        break;
                }

                pActivateParent = pActivateParent->ImplGetParent();
            }
        }
        else
        {
            if ( ImplIsOverlapWindow() )
                pActivateParent = this;
            else
                pActivateParent = mpOverlapWindow;
            while ( pActivateParent )
            {
                if ( pActivateParent->ImplIsOverlapWindow() )
                {
                    if ( !pActivateParent->mbParentActive )
                        break;
                }

                pActivateParent = pActivateParent->ImplGetParent();
            }
        }
        if ( pDeactivateParent )
        {
            do
            {
                pLastParent = pOldFocusWindow;
                if ( pLastParent != pDeactivateParent )
                {
                    pParent = pLastParent->ImplGetParent();
                    while ( pParent )
                    {
                        if ( pParent == pDeactivateParent )
                            break;
                        pLastParent = pParent;
                        pParent = pParent->ImplGetParent();
                    }
                }
                else
                    pParent = pLastParent;

                pParent->mbActive = FALSE;
                pParent->Deactivate();
                pDeactivateParent = pLastParent;
            }
            while ( pDeactivateParent != pOldFocusWindow );
        }
        do
        {
            pLastParent = this;
            if ( pLastParent != pActivateParent )
            {
                pParent = pLastParent->ImplGetParent();
                while ( pParent )
                {
                    if ( pParent == pActivateParent )
                        break;
                    pLastParent = pParent;
                    pParent = pParent->ImplGetParent();
                }
            }
            else
                pParent = pLastParent;

            pParent->mbActive = TRUE;
            pParent->Activate();
            pActivateParent = pLastParent;
        }
        while ( pActivateParent != this );
*/
        // call Get- and LoseFocus
        if ( pOldFocusWindow )
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
            if ( mpSysObj )
            {
                mpFrameData->mpFocusWin = this;
                if ( !mpFrameData->mbInSysObjFocusHdl )
                    mpSysObj->GrabFocus();
            }

            if ( pSVData->maWinData.mpFocusWin == this )
            {
                if ( mpCursor )
                    mpCursor->ImplShow();
                mbInFocusHdl = TRUE;
                mnGetFocusFlags = nFlags;
                // if we're changing focus due to closing a popup floating window
                // notify the new focus window so it can restore the inner focus
                // eg, toolboxes can select their recent active item
                if( pOldFocusWindow &&
                    ( pOldFocusWindow->GetDialogControlFlags() & WINDOW_DLGCTRL_FLOATWIN_POPUPMODEEND_CANCEL ) )
                    mnGetFocusFlags |= GETFOCUS_FLOATWIN_POPUPMODEEND_CANCEL;
                NotifyEvent aNEvt( EVENT_GETFOCUS, this );
                if ( !ImplCallPreNotify( aNEvt ) )
                    GetFocus();
                ImplCallActivateListeners( pOldFocusWindow );
                mnGetFocusFlags = 0;
                mbInFocusHdl = FALSE;
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
    if ( rInputContext == pFocusWin->mpFrameData->maOldInputContext )
        return;

    pFocusWin->mpFrameData->maOldInputContext = rInputContext;

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
        pFontEntry = pFocusWin->mpFontCache->Get( pFocusWin->mpFontList, rFont, aSize, pFocusWin->mpOutDevData ? pFocusWin->mpOutDevData->mpFirstFontSubstEntry : NULL );
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

Window::Window( WindowType nType ) :
    maZoom( 1, 1 ),
    maWinRegion( REGION_NULL ),
    maWinClipRegion( REGION_NULL )
{
    DBG_CTOR( Window, ImplDbgCheckWindow );

    ImplInitData( nType );
}

// -----------------------------------------------------------------------

Window::Window( Window* pParent, WinBits nStyle ) :
    maZoom( 1, 1 ),
    maWinRegion( REGION_NULL ),
    maWinClipRegion( REGION_NULL )
{
    DBG_CTOR( Window, ImplDbgCheckWindow );

    ImplInitData( WINDOW_WINDOW );
    ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

Window::Window( Window* pParent, const ResId& rResId ) :
    maZoom( 1, 1 ),
    maWinRegion( REGION_NULL ),
    maWinClipRegion( REGION_NULL )
{
    DBG_CTOR( Window, ImplDbgCheckWindow );

    ImplInitData( WINDOW_WINDOW );
    rResId.SetRT( RSC_WINDOW );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle, NULL );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

Window::~Window()
{
    DBG_DTOR( Window, ImplDbgCheckWindow );
    DBG_ASSERT( !mbInDtor, "~Window - already in DTOR!" );

    // Dispose of the canvas implementation (which, currently, has an
    // own wrapper window as a child to this one.
    if( mxCanvas.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::lang::XComponent >
              xCanvasComponent( mxCanvas,
                                ::com::sun::star::uno::UNO_QUERY );
        xCanvasComponent->dispose();
    }
    mxCanvas = NULL;
    mxCanvasWindow = NULL;

    mbInDtor = TRUE;

    ImplCallEventListeners( VCLEVENT_OBJECT_DYING );

    // do not send child events for frames that were registered as native frames
    if( !ImplIsAccessibleNativeFrame() && mbReallyVisible )
        if ( ImplIsAccessibleCandidate() && GetAccessibleParentWindow() )
            GetAccessibleParentWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_CHILDDESTROYED, this );

    // remove associated data structures from dockingmanager
    ImplGetDockingManager()->RemoveWindow( this );

    // shutdown drag and drop
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XComponent > xComponent( mxDNDListenerContainer, ::com::sun::star::uno::UNO_QUERY );

    if( xComponent.is() )
        xComponent->dispose();

    if( mbFrame && mpFrameData )
    {
        try
        {
            // deregister drop target listener
            if( mpFrameData->mxDropTargetListener.is() )
            {
                OSL_TRACE( "removing drop target listener" );

                Reference< XDragGestureRecognizer > xDragGestureRecognizer =
                    Reference< XDragGestureRecognizer > (mpFrameData->mxDragSource, UNO_QUERY);
                if( xDragGestureRecognizer.is() )
                {
                    xDragGestureRecognizer->removeDragGestureListener(
                        Reference< XDragGestureListener > (mpFrameData->mxDropTargetListener, UNO_QUERY));
                }

                mpFrameData->mxDropTarget->removeDropTargetListener( mpFrameData->mxDropTargetListener );
                mpFrameData->mxDropTargetListener.clear();
            }

            // shutdown drag and drop for this frame window
            Reference< XComponent > xComponent( mpFrameData->mxDropTarget, UNO_QUERY );

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
    if ( mxAccessible.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xC( mxAccessible, ::com::sun::star::uno::UNO_QUERY );
        if ( xC.is() )
            xC->dispose();
    }

    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maHelpData.mpHelpWin && (pSVData->maHelpData.mpHelpWin->GetParent() == this) )
        ImplDestroyHelpWindow( FALSE );

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
    if ( DbgIsAssert() )
    {
        ByteString  aErrorStr;
        BOOL        bError = FALSE;
        Window*     pTempWin = mpFrameData->mpFirstOverlap;
        while ( pTempWin )
        {
            if ( ImplIsRealParentPath( pTempWin ) )
            {
                bError = TRUE;
                if ( aErrorStr.Len() )
                    aErrorStr += "; ";
                aErrorStr += ByteString( pTempWin->GetText(), RTL_TEXTENCODING_UTF8 );
            }
            pTempWin = pTempWin->mpNextOverlap;
        }
        if ( bError )
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") with living SystemWindow(s) destroyed: ";
            aTempStr += aErrorStr;
            DBG_ERROR( aTempStr.GetBuffer() );
        }

        bError = FALSE;
        pTempWin = pSVData->maWinData.mpFirstFrame;
        while ( pTempWin )
        {
            if ( ImplIsRealParentPath( pTempWin ) )
            {
                bError = TRUE;
                if ( aErrorStr.Len() )
                    aErrorStr += "; ";
                aErrorStr += ByteString( pTempWin->GetText(), RTL_TEXTENCODING_UTF8 );
            }
            pTempWin = pTempWin->mpFrameData->mpNextFrame;
        }
        if ( bError )
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") with living SystemWindow(s) destroyed: ";
            aTempStr += aErrorStr;
            DBG_ERROR( aTempStr.GetBuffer() );
        }

        if ( mpFirstChild )
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") with living Child(s) destroyed: ";
            Window* pTempWin = mpFirstChild;
            while ( pTempWin )
            {
                aTempStr += ByteString( pTempWin->GetText(), RTL_TEXTENCODING_UTF8 );
                pTempWin = pTempWin->mpNext;
                if ( pTempWin )
                    aTempStr += "; ";
            }
            DBG_ERROR( aTempStr.GetBuffer() );
        }

        if ( mpFirstOverlap )
        {
            ByteString aTempStr( "Window (" );
            aTempStr += ByteString( GetText(), RTL_TEXTENCODING_UTF8 );
            aTempStr += ") with living SystemWindow(s) destroyed: ";
            Window* pTempWin = mpFirstOverlap;
            while ( pTempWin )
            {
                aTempStr += ByteString( pTempWin->GetText(), RTL_TEXTENCODING_UTF8 );
                pTempWin = pTempWin->mpNext;
                if ( pTempWin )
                    aTempStr += "; ";
            }
            DBG_ERROR( aTempStr.GetBuffer() );
        }
    }
#endif

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

    // Wenn wir den Focus haben, dann den Focus auf ein anderes Fenster setzen
    Window* pOverlapWindow = ImplGetFirstOverlapWindow();
    if ( pSVData->maWinData.mpFocusWin == this )
    {
        if ( mbFrame )
        {
            pSVData->maWinData.mpFocusWin = NULL;
            pOverlapWindow->mpLastFocusWindow = NULL;
            GetpApp()->FocusChanged();
        }
        else
        {
            Window* pParent = GetParent();
            Window* pBorderWindow = mpBorderWindow;
            // Bei ueberlappenden Fenstern wird der Focus auf den
            // Parent vom naechsten FrameWindow gesetzt
            if ( pBorderWindow )
            {
                if ( pBorderWindow->ImplIsOverlapWindow() )
                    pParent = pBorderWindow->mpOverlapWindow;
            }
            else if ( ImplIsOverlapWindow() )
                pParent = mpOverlapWindow;

            if ( pParent && pParent->IsEnabled() && pParent->IsInputEnabled() )
                pParent->GrabFocus();
            else
                mpFrameWindow->GrabFocus();

            // If the focus was set back to 'this' set it to nothing
            if ( pSVData->maWinData.mpFocusWin == this )
            {
                pSVData->maWinData.mpFocusWin = NULL;
                pOverlapWindow->mpLastFocusWindow = NULL;
                GetpApp()->FocusChanged();
            }
        }
    }
    if ( pOverlapWindow->mpLastFocusWindow == this )
        pOverlapWindow->mpLastFocusWindow = NULL;

    // reset hint for DefModalDialogParent
    if( pSVData->maWinData.mpActiveApplicationFrame == this )
        pSVData->maWinData.mpActiveApplicationFrame = NULL;

    // gemerkte Fenster zuruecksetzen
    if ( mpFrameData->mpFocusWin == this )
        mpFrameData->mpFocusWin = NULL;
    if ( mpFrameData->mpMouseMoveWin == this )
        mpFrameData->mpMouseMoveWin = NULL;
    if ( mpFrameData->mpMouseDownWin == this )
        mpFrameData->mpMouseDownWin = NULL;

    // Deactivate-Window zuruecksetzen
    if ( pSVData->maWinData.mpLastDeacWin == this )
        pSVData->maWinData.mpLastDeacWin = NULL;

    if ( mbFrame )
    {
        if ( mpFrameData->mnFocusId )
            Application::RemoveUserEvent( mpFrameData->mnFocusId );
        if ( mpFrameData->mnMouseMoveId )
            Application::RemoveUserEvent( mpFrameData->mnMouseMoveId );
    }

    // Graphic freigeben
    ImplReleaseGraphics();

    // Evt. anderen Funktion mitteilen, das das Fenster geloescht
    // wurde
    ImplDelData* pDelData = mpFirstDel;
    while ( pDelData )
    {
        pDelData->mbDel = TRUE;
        pDelData->mpWindow = NULL;  // #112873# pDel is not associated with a Window anymore
        pDelData = pDelData->mpNext;
    }

    // Fenster aus den Listen austragen
    ImplRemoveWindow( TRUE );

    // Extra Window Daten loeschen
    if ( mpWinData )
    {
        if ( mpWinData->mpExtOldText )
            delete mpWinData->mpExtOldText;
        if ( mpWinData->mpExtOldAttrAry )
            delete mpWinData->mpExtOldAttrAry;
        if ( mpWinData->mpCursorRect )
            delete mpWinData->mpCursorRect;
        if ( mpWinData->mpFocusRect )
            delete mpWinData->mpFocusRect;
        if ( mpWinData->mpTrackRect )
            delete mpWinData->mpTrackRect;
        // Native widget support
        delete mpWinData->mpSalControlHandle;
        mpWinData->mpSalControlHandle = NULL;
        delete mpWinData;
    }


    // Overlap-Window-Daten loeschen
    if ( mpOverlapData )
    {
        delete mpOverlapData;
    }

    // Evt. noch BorderWindow oder Frame zerstoeren
    if ( mpBorderWindow )
        delete mpBorderWindow;
    else if ( mbFrame )
    {
        if ( pSVData->maWinData.mpFirstFrame == this )
            pSVData->maWinData.mpFirstFrame = mpFrameData->mpNextFrame;
        else
        {
            Window* pSysWin = pSVData->maWinData.mpFirstFrame;
            while ( pSysWin->mpFrameData->mpNextFrame != this )
                pSysWin = pSysWin->mpFrameData->mpNextFrame;
            pSysWin->mpFrameData->mpNextFrame = mpFrameData->mpNextFrame;
        }
        mpFrame->SetCallback( NULL, NULL );
        pSVData->mpDefInst->DestroyFrame( mpFrame );
        delete mpFrameData;
    }

    if ( mpChildClipRegion )
        delete mpChildClipRegion;

    delete mpAccessibleInfos;
}

// -----------------------------------------------------------------------

void Window::MouseMove( const MouseEvent& rMEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_MOUSEMOVE, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mbMouseMove = TRUE;
}

// -----------------------------------------------------------------------

void Window::MouseButtonDown( const MouseEvent& rMEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_MOUSEBUTTONDOWN, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mbMouseButtonDown = TRUE;
}

// -----------------------------------------------------------------------

void Window::MouseButtonUp( const MouseEvent& rMEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_MOUSEBUTTONUP, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mbMouseButtonUp = TRUE;
}

// -----------------------------------------------------------------------

void Window::KeyInput( const KeyEvent& rKEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_KEYINPUT, this, &rKEvt );
    if ( !Notify( aNEvt ) )
        mbKeyInput = TRUE;
}

// -----------------------------------------------------------------------

void Window::KeyUp( const KeyEvent& rKEvt )
{
    { // Klammerung, da in diesem Handler das Window zerstoert werden darf
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    }

    NotifyEvent aNEvt( EVENT_KEYUP, this, &rKEvt );
    if ( !Notify( aNEvt ) )
        mbKeyUp = TRUE;
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

    // Forward physical position to embedded java frame
    if( mxCanvasWindow.is() )
        mxCanvasWindow->setPosSize( mnOutOffX, mnOutOffX,
                                    mnOutWidth, mnOutHeight, 0 );
}

// -----------------------------------------------------------------------

void Window::Resize()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    // Forward size to embedded java frame
    if( mxCanvasWindow.is() )
        mxCanvasWindow->setPosSize( mnOutOffX, mnOutOffX,
                                    mnOutWidth, mnOutHeight, 0 );
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

    if ( HasFocus() && mpLastFocusWindow && !(mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) )
        mpLastFocusWindow->GrabFocus();

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
        ULONG nStartHelpId = GetHelpId();

        if ( !nStartHelpId && ImplGetParent() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            if ( !nStartHelpId )
                nStartHelpId = HELP_INDEX;

            Help* pHelp = Application::GetHelp();
            if ( pHelp )
                pHelp->Start( nStartHelpId, this );
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
        mbCommand = TRUE;
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

void Window::StateChanged( StateChangedType nType )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    // Forward visible state to embedded java frame
    if( mxCanvasWindow.is() )
    {
        switch( nType )
        {
            case STATE_CHANGE_VISIBLE:
                mxCanvasWindow->setVisible( mbVisible );
                break;

            case STATE_CHANGE_ENABLE:
                mxCanvasWindow->setEnable( !mbDisabled );
                break;
        }
    }
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

        if ( mbCompoundControl || ( rNEvt.GetWindow() == this ) )
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
        if ( mbCompoundControl || ( rNEvt.GetWindow() == this ) )
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
        if ( mbCompoundControl || ( rNEvt.GetWindow() == this ) )
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
        if ( mbCompoundControl || ( rNEvt.GetWindow() == this ) )
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
        if ( mbCompoundControl || ( rNEvt.GetWindow() == this ) )
            ImplCallEventListeners( VCLEVENT_WINDOW_KEYINPUT, (void*)rNEvt.GetKeyEvent() );
    }
    else if( rNEvt.GetType() == EVENT_KEYUP )
    {
        if ( mbCompoundControl || ( rNEvt.GetWindow() == this ) )
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
    if ( mpParent && !ImplIsOverlapWindow() )
        bDone = mpParent->PreNotify( rNEvt );

    if ( !bDone )
    {
        if( rNEvt.GetType() == EVENT_GETFOCUS )
        {
            BOOL bCompoundFocusChanged = FALSE;
            if ( mbCompoundControl && !mbCompoundControlHasFocus && HasChildPathFocus() )
            {
                mbCompoundControlHasFocus = TRUE;
                bCompoundFocusChanged = TRUE;
            }

            if ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) )
                ImplCallEventListeners( VCLEVENT_WINDOW_GETFOCUS );
        }
        else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
        {
            BOOL bCompoundFocusChanged = FALSE;
            if ( mbCompoundControl && mbCompoundControlHasFocus && !HasChildPathFocus() )
            {
                mbCompoundControlHasFocus = FALSE ;
                bCompoundFocusChanged = TRUE;
            }

            if ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) )
                ImplCallEventListeners( VCLEVENT_WINDOW_LOSEFOCUS );
        }

        // #82968# mouse and key events will be notified after processing ( in ImplNotifyKeyMouseCommandEventListeners() )!
        //    see also ImplHandleMouseEvent(), ImplHandleKey()

        /*
        else if( rNEvt.GetType() == EVENT_MOUSEMOVE )
        {
            if ( mbCompoundControl || ( rNEvt.GetWindow() == this ) )
            {
                if ( rNEvt.GetWindow() == this )
                    ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEMOVE, (void*)rNEvt.GetMouseEvent() );
                else
                    ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEMOVE, &ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this ) );
            }
        }
        else if( rNEvt.GetType() == EVENT_MOUSEBUTTONUP )
        {
            if ( mbCompoundControl || ( rNEvt.GetWindow() == this ) )
            {
                if ( rNEvt.GetWindow() == this )
                    ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONUP, (void*)rNEvt.GetMouseEvent() );
                else
                    ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONUP, &ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this ) );
            }
        }
        else if( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
        {
            if ( mbCompoundControl || ( rNEvt.GetWindow() == this ) )
            {
                if ( rNEvt.GetWindow() == this )
                    ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, (void*)rNEvt.GetMouseEvent() );
                else
                    ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, &ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this ) );
            }
        }
        else if( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            if ( mbCompoundControl || ( rNEvt.GetWindow() == this ) )
                ImplCallEventListeners( VCLEVENT_WINDOW_KEYINPUT, (void*)rNEvt.GetKeyEvent() );
        }
        else if( rNEvt.GetType() == EVENT_KEYUP )
        {
            if ( mbCompoundControl || ( rNEvt.GetWindow() == this ) )
                ImplCallEventListeners( VCLEVENT_WINDOW_KEYUP, (void*)rNEvt.GetKeyEvent() );
        }
        */
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
                if ( pMEvt->GetClicks() == 2 )
                {
                    // double click toggles floating mode
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
                 !(GetStyle() & WB_TABSTOP) && !(mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) )
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
        if ( mpParent && !ImplIsOverlapWindow() )
            nRet = mpParent->Notify( rNEvt );
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

    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpApp->ImplCallEventListeners( &aEvent );

    if ( aDelData.IsDelete() )
        return;

    if ( !maEventListeners.empty() )
        maEventListeners.Call( &aEvent );

    if ( aDelData.IsDelete() )
        return;

    ImplRemoveDel( &aDelData );

    Window* pWindow = this;
    while ( pWindow )
    {
        pWindow->ImplAddDel( &aDelData );

        if ( !pWindow->maChildEventListeners.empty() )
            pWindow->maChildEventListeners.Call( &aEvent );

        if ( aDelData.IsDelete() )
            return;

        pWindow->ImplRemoveDel( &aDelData );

        pWindow = pWindow->GetParent();
    }
}

// -----------------------------------------------------------------------

void Window::AddEventListener( const Link& rEventListener )
{
    maEventListeners.push_back( rEventListener );
}

// -----------------------------------------------------------------------

void Window::RemoveEventListener( const Link& rEventListener )
{
    maEventListeners.remove( rEventListener );
}

// -----------------------------------------------------------------------

void Window::AddChildEventListener( const Link& rEventListener )
{
    maChildEventListeners.push_back( rEventListener );
}

// -----------------------------------------------------------------------

void Window::RemoveChildEventListener( const Link& rEventListener )
{
    maChildEventListeners.remove( rEventListener );
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
    if ( mpFrame->PostEvent( pSVEvent ) )
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
    if ( mpFrame->PostEvent( pSVEvent ) )
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
    if ( mnLockCount != 0 )
        return TRUE;

    if ( bChilds || mbChildNotify )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            if ( pChild->IsLocked( TRUE ) )
                return TRUE;
            pChild = pChild->mpNext;
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Window::IsUICaptured( BOOL bChilds ) const
{
    return Application::IsUICaptured();
}

// -----------------------------------------------------------------------

BOOL Window::IsUserActive( USHORT nTest, BOOL bChilds ) const
{
    return Application::IsUserActive( nTest );
}

// -----------------------------------------------------------------------

ULONG Window::GetLastInputInterval() const
{
    return Application::GetLastInputInterval();
}

// -----------------------------------------------------------------------

void Window::SetStyle( WinBits nStyle )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mnStyle != nStyle )
    {
        mnPrevStyle = mnStyle;
        mnStyle = nStyle;
        StateChanged( STATE_CHANGE_STYLE );
    }
}

// -----------------------------------------------------------------------

void Window::SetExtendedStyle( WinBits nExtendedStyle )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mnExtendedStyle != nExtendedStyle )
    {
        mnPrevExtendedStyle = mnExtendedStyle;
        mnExtendedStyle = nExtendedStyle;
        StateChanged( STATE_CHANGE_EXTENDEDSTYLE );
    }
}

// -----------------------------------------------------------------------

SystemWindow* Window::GetSystemWindow() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    const Window* pWin = this;
    while ( !pWin->IsSystemWindow() )
        pWin  = pWin->GetParent();
    return (SystemWindow*)pWin;
}

// -----------------------------------------------------------------------

void Window::SetBorderStyle( USHORT nBorderStyle )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        if ( mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
             ((ImplBorderWindow*)mpBorderWindow)->SetBorderStyle( nBorderStyle );
        else
            mpBorderWindow->SetBorderStyle( nBorderStyle );
    }
}

// -----------------------------------------------------------------------

USHORT Window::GetBorderStyle() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        if ( mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
            return ((ImplBorderWindow*)mpBorderWindow)->GetBorderStyle();
        else
            return mpBorderWindow->GetBorderStyle();
    }

    return 0;
}

// -----------------------------------------------------------------------

long Window::CalcTitleWidth() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        if ( mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
            return ((ImplBorderWindow*)mpBorderWindow)->CalcTitleWidth();
        else
            return mpBorderWindow->CalcTitleWidth();
    }
    else if ( mbFrame && (mnStyle & WB_MOVEABLE) )
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

    if ( mpBorderWindow )
        mpBorderWindow->EnableClipSiblings( bClipSiblings );

    mbClipSiblings = bClipSiblings;
}

// -----------------------------------------------------------------------

void Window::SetMouseTransparent( BOOL bTransparent )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->SetMouseTransparent( bTransparent );

    mbMouseTransparent = bTransparent;
}

// -----------------------------------------------------------------------

void Window::SetPaintTransparent( BOOL bTransparent )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->SetPaintTransparent( bTransparent );

    mbPaintTransparent = bTransparent;
}

// -----------------------------------------------------------------------

void Window::SetInputContext( const InputContext& rInputContext )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    maInputContext = rInputContext;
    if ( !mbInFocusHdl && HasFocus() )
        ImplNewInputContext();
}

// -----------------------------------------------------------------------

void Window::EndExtTextInput( USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mbExtTextInput )
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

void Window::SetSettings( const AllSettings& rSettings, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        mpBorderWindow->SetSettings( rSettings, FALSE );
        if ( (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow->SetSettings( rSettings, TRUE );
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

    if ( bChild || mbChildNotify )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            pChild->SetSettings( rSettings, bChild );
            pChild = pChild->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::UpdateSettings( const AllSettings& rSettings, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
    {
        mpBorderWindow->UpdateSettings( rSettings, FALSE );
        if ( (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow->UpdateSettings( rSettings, TRUE );
    }

    AllSettings aOldSettings = maSettings;
    ULONG nChangeFlags = maSettings.Update( maSettings.GetWindowUpdate(), rSettings );

    // AppFont-Aufloesung und DPI-Aufloesung neu berechnen
    ImplInitResolutionSettings();

    if( nChangeFlags & SETTINGS_STYLE )
    {
        if ( mnStyle & WB_3DLOOK )
            SetBackground( Wallpaper( rSettings.GetStyleSettings().GetFaceColor() ) );
        else
            SetBackground( Wallpaper( rSettings.GetStyleSettings().GetWindowColor() ) );
    }

    if ( nChangeFlags )
    {
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
        DataChanged( aDCEvt );
    }

    if ( bChild || mbChildNotify )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            pChild->UpdateSettings( rSettings, bChild );
            pChild = pChild->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::NotifyAllChilds( DataChangedEvent& rDCEvt )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    DataChanged( rDCEvt );

    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        pChild->NotifyAllChilds( rDCEvt );
        pChild = pChild->mpNext;
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

void Window::GetFontResolution( sal_Int32& nDPIX, sal_Int32& nDPIY ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    nDPIX = mpFrameData->mnFontDPIX;
    nDPIY = mpFrameData->mnFontDPIY;
}

// -----------------------------------------------------------------------

void Window::SetParentClipMode( USHORT nMode )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->SetParentClipMode( nMode );
    else
    {
        if ( !ImplIsOverlapWindow() )
        {
            mnParentClipMode = nMode;
            if ( nMode & PARENTCLIPMODE_CLIP )
                mpParent->mbClipChildren = TRUE;
        }
    }
}

// -----------------------------------------------------------------------

USHORT Window::GetParentClipMode() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        return mpBorderWindow->GetParentClipMode();
    else
        return mnParentClipMode;
}

// -----------------------------------------------------------------------

void Window::SetWindowRegionPixel()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->SetWindowRegionPixel();
    else
    {
        if ( mbWinRegion )
        {
            maWinRegion = Region( REGION_NULL );
            mbWinRegion = FALSE;
            ImplSetClipFlag();

            if ( IsReallyVisible() )
            {
                // Hintergrund-Sicherung zuruecksetzen
                if ( mpOverlapData && mpOverlapData->mpSaveBackDev )
                    ImplDeleteOverlapBackground();
                if ( mpFrameData->mpFirstBackWin )
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

    if ( mpBorderWindow )
        mpBorderWindow->SetWindowRegionPixel( rRegion );
    else
    {
        BOOL bInvalidate = FALSE;

        if ( rRegion.GetType() == REGION_NULL )
        {
            if ( mbWinRegion )
            {
                maWinRegion = Region( REGION_NULL );
                mbWinRegion = FALSE;
                ImplSetClipFlag();
                bInvalidate = TRUE;
            }
        }
        else
        {
            maWinRegion = rRegion;
            mbWinRegion = TRUE;
            ImplSetClipFlag();
            bInvalidate = TRUE;
        }

        if ( IsReallyVisible() )
        {
            // Hintergrund-Sicherung zuruecksetzen
            if ( mpOverlapData && mpOverlapData->mpSaveBackDev )
                ImplDeleteOverlapBackground();
            if ( mpFrameData->mpFirstBackWin )
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

    if ( mpBorderWindow )
        return mpBorderWindow->GetWindowRegionPixel();
    else
        return maWinRegion;
}

// -----------------------------------------------------------------------

BOOL Window::IsWindowRegionPixel() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        return mpBorderWindow->IsWindowRegionPixel();
    else
        return mbWinRegion;
}

// -----------------------------------------------------------------------

Region Window::GetWindowClipRegionPixel( USHORT nFlags ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Region aWinClipRegion;

    if ( nFlags & WINDOW_GETCLIPREGION_NOCHILDREN )
    {
        if ( mbInitWinClipRegion )
            ((Window*)this)->ImplInitWinClipRegion();
        aWinClipRegion = maWinClipRegion;
    }
    else
    {
        Region* pWinChildClipRegion = ((Window*)this)->ImplGetWinChildClipRegion();
        aWinClipRegion = *pWinChildClipRegion;
        // --- RTL --- remirror clip region before passing it to somebody
        if( ((Window*)this)->ImplHasMirroredGraphics() && !IsRTLEnabled() )
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

    if ( mpPaintRegion )
    {
        Region aRegion = *mpPaintRegion;
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
    if( mpPaintRegion )
    {
        Region aPixRegion = LogicToPixel( rRegion );
        Region aDevPixRegion = ImplPixelToDevicePixel( aPixRegion );

        Region aWinChildRegion = *ImplGetWinChildClipRegion();
        // --- RTL -- only this region is in frame coordinates, so re-mirror it
        if( ImplHasMirroredGraphics() && !IsRTLEnabled() )
            ImplReMirror( aWinChildRegion );
        aDevPixRegion.Intersect( aWinChildRegion );
        if( ! aDevPixRegion.IsEmpty() )
        {
            mpPaintRegion->Union( aDevPixRegion );
            mbInitClipRegion = TRUE;
        }
    }
}

// -----------------------------------------------------------------------

void Window::SetParent( Window* pNewParent )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    DBG_ASSERT( pNewParent, "Window::SetParent(): pParent == NULL" );

    ImplSetFrameParent( pNewParent );

    if ( mbFrame )
        return;

    if ( mpBorderWindow )
    {
        mpRealParent = pNewParent;
        mpBorderWindow->SetParent( pNewParent );
        return;
    }

    if ( mpParent == pNewParent )
        return;

    BOOL bVisible = IsVisible();
    Show( FALSE, SHOW_NOFOCUSCHANGE );

    // Testen, ob sich das Overlap-Window aendert
    Window* pOldOverlapWindow;
    Window* pNewOverlapWindow;
    if ( ImplIsOverlapWindow() )
        pOldOverlapWindow = NULL;
    else
    {
        pNewOverlapWindow = pNewParent->ImplGetFirstOverlapWindow();
        if ( mpOverlapWindow != pNewOverlapWindow )
            pOldOverlapWindow = mpOverlapWindow;
        else
            pOldOverlapWindow = NULL;
    }

    // Fenster in der Hirachie umsetzen
    BOOL bFocusOverlapWin = HasChildPathFocus( TRUE );
    BOOL bFocusWin = HasChildPathFocus();
    BOOL bNewFrame = pNewParent->mpFrameWindow != mpFrameWindow;
    if ( bNewFrame )
    {
        if ( mpFrameData->mpFocusWin )
        {
            if ( IsWindowOrChild( mpFrameData->mpFocusWin ) )
                mpFrameData->mpFocusWin = NULL;
        }
        if ( mpFrameData->mpMouseMoveWin )
        {
            if ( IsWindowOrChild( mpFrameData->mpMouseMoveWin ) )
                mpFrameData->mpMouseMoveWin = NULL;
        }
        if ( mpFrameData->mpMouseDownWin )
        {
            if ( IsWindowOrChild( mpFrameData->mpMouseDownWin ) )
                mpFrameData->mpMouseDownWin = NULL;
        }
    }
    ImplRemoveWindow( bNewFrame );
    ImplInsertWindow( pNewParent );
    if ( mnParentClipMode & PARENTCLIPMODE_CLIP )
        pNewParent->mbClipChildren = TRUE;
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
            Window* pOverlapWindow = mpFirstOverlap;
            while ( pOverlapWindow )
            {
                Window* pNextOverlapWindow = pOverlapWindow->mpNext;
                pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
                pOverlapWindow = pNextOverlapWindow;
            }
        }
    }
    else if ( pOldOverlapWindow )
    {
        // Focus-Save zuruecksetzen
        if ( bFocusWin ||
             (pOldOverlapWindow->mpLastFocusWindow &&
              IsWindowOrChild( pOldOverlapWindow->mpLastFocusWindow )) )
            pOldOverlapWindow->mpLastFocusWindow = NULL;

        Window* pOverlapWindow = pOldOverlapWindow->mpFirstOverlap;
        while ( pOverlapWindow )
        {
            Window* pNextOverlapWindow = pOverlapWindow->mpNext;
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
            ((ImplBorderWindow*)this)->SetDisplayActive( mpFrameData->mbHasFocus );
    }

    // Focus evtl. auf den neuen Frame umsetzen, wenn FocusWindow mit
    // SetParent() umgesetzt wird
    if ( bFocusOverlapWin )
    {
        mpFrameData->mpFocusWin = Application::GetFocusWindow();
        if ( !mpFrameData->mbHasFocus )
        {
            mpFrame->ToTop( 0 );
        }
    }

    // Assure DragSource and DropTarget members are created
    if ( bNewFrame )
    {
            GetDropTarget();
    }

    if ( bVisible )
        Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
}

// -----------------------------------------------------------------------

void Window::Show( BOOL bVisible, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    BOOL bNativeFrameRegistered = FALSE;

    if ( mbVisible == bVisible )
        return;

    mbVisible = bVisible != 0;

    if ( !bVisible )
    {
        ImplHideAllOverlaps();

        if ( mpBorderWindow )
        {
            BOOL bOldUpdate = mpBorderWindow->mbNoParentUpdate;
            if ( mbNoParentUpdate )
                mpBorderWindow->mbNoParentUpdate = TRUE;
            mpBorderWindow->Show( FALSE, nFlags );
            mpBorderWindow->mbNoParentUpdate = bOldUpdate;
        }
        else if ( mbFrame )
        {
            mbSuppressAccessibilityEvents = TRUE;
            mpFrame->Show( FALSE, FALSE );
        }

        StateChanged( STATE_CHANGE_VISIBLE );

        if ( mbReallyVisible )
        {
            Region  aInvRegion( REGION_EMPTY );
            BOOL    bSaveBack = FALSE;

            if ( ImplIsOverlapWindow() && !mbFrame )
            {
                if ( ImplRestoreOverlapBackground( aInvRegion ) )
                    bSaveBack = TRUE;
            }

            if ( !bSaveBack )
            {
                if ( mbInitWinClipRegion )
                    ImplInitWinClipRegion();
                aInvRegion = maWinClipRegion;
            }

            ImplResetReallyVisible();
            ImplSetClipFlag();

            if ( ImplIsOverlapWindow() && !mbFrame )
            {
                // Focus umsetzen
                if ( !(nFlags & SHOW_NOFOCUSCHANGE) && HasChildPathFocus() )
                {
                    if ( mpOverlapWindow->IsEnabled() && mpOverlapWindow->IsInputEnabled() )
                        mpOverlapWindow->GrabFocus();
                }
            }

            if ( !mbFrame )
            {
                if ( !mbNoParentUpdate && !(nFlags & SHOW_NOPARENTUPDATE) )
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
        if( mbFrame && GetParent() && GetParent()->IsCompoundControl() &&
            GetParent()->IsNativeWidgetEnabled() != IsNativeWidgetEnabled() )
            EnableNativeWidget( GetParent()->IsNativeWidgetEnabled() );

        if ( mbCallMove )
        {
            ImplCallMove();
        }
        if ( mbCallResize )
        {
            ImplCallResize();
        }

        StateChanged( STATE_CHANGE_VISIBLE );

        Window* pTestParent;
        if ( ImplIsOverlapWindow() )
            pTestParent = mpOverlapWindow;
        else
            pTestParent = ImplGetParent();
        if ( mbFrame || pTestParent->mbReallyVisible )
        {
            // Wenn ein Window gerade sichtbar wird, schicken wir allen
            // Child-Fenstern ein StateChanged, damit diese sich
            // initialisieren koennen
            ImplCallInitShow();

            // Wenn es ein SystemWindow ist, dann kommt es auch automatisch
            // nach vorne, wenn es gewuenscht ist
            if ( ImplIsOverlapWindow() && !(nFlags & SHOW_NOACTIVATE) )
            {
                ImplStartToTop( 0 );
                ImplFocusToTop( 0, FALSE );
            }

            // Hintergrund sichern
            if ( mpOverlapData && mpOverlapData->mbSaveBack )
                ImplSaveOverlapBackground();
            // Dafuer sorgen, das Clip-Rechtecke neu berechnet werden
            ImplSetReallyVisible();
            ImplSetClipFlag();

            if ( !mbFrame )
            {
                ImplInvalidate( NULL, INVALIDATE_NOTRANSPARENT | INVALIDATE_CHILDREN );
                ImplGenerateMouseMove();
            }
        }

        if ( mpBorderWindow )
            mpBorderWindow->Show( TRUE, nFlags );
        else if ( mbFrame )
        {
            ImplSVData* pSVData = ImplGetSVData();
            // #106431#, hide SplashScreen
            if( pSVData->mpIntroWindow && !ImplIsWindowOrChild( pSVData->mpIntroWindow ) )
                pSVData->mpIntroWindow->Hide();

            //DBG_ASSERT( !mbSuppressAccessibilityEvents, "Window::Show() - Frame reactivated");
            mbSuppressAccessibilityEvents = FALSE;

            mbPaintFrame = TRUE;
            BOOL bNoActivate = nFlags & (SHOW_NOACTIVATE|SHOW_NOFOCUSCHANGE);
            mpFrame->Show( TRUE, bNoActivate );

            // Query the correct size of the window, if we are waiting for
            // a system resize
            if ( mbWaitSystemResize )
            {
                long nOutWidth;
                long nOutHeight;
                mpFrame->GetClientSize( nOutWidth, nOutHeight );
                ImplHandleResize( this, nOutWidth, nOutHeight );
            }
        }

#ifdef DBG_UTIL
        if ( IsDialog() || (GetType() == WINDOW_TABPAGE) || (GetType() == WINDOW_DOCKINGWINDOW) )
        {
            DBG_DIALOGTEST( this );
        }
#endif

        ImplShowAllOverlaps();
    }

    // Hintergrund-Sicherung zuruecksetzen
    if ( mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    // the SHOW/HIDE events also serve as indicators to send child creation/destroy events to the access bridge
    // to avoid creation event to be send twice a NULL handle is passed if this (frame)window was already registered
    // a NULL handle is also passed if:
    // the window is hidden
    // the window is not really visible (ie, all parents must be visible)
    void *pData = this;
    if( mbVisible )
    {
        // show
        if( bNativeFrameRegistered || !ImplIsAccessibleCandidate() || !mbReallyVisible )
            pData = NULL;
    }
    else
    {
        // hide
        if( !ImplIsAccessibleCandidate() )
            pData = NULL;
    }
    ImplCallEventListeners( mbVisible ? VCLEVENT_WINDOW_SHOW : VCLEVENT_WINDOW_HIDE, pData );

    // #107575#, if a floating windows is shown that grabs the focus, we have to notify the toolkit about it
    // ImplGrabFocus() is not called in this case
    // Because this might lead to problems the task will be shifted to 6.y
    // Note: top-level context menues are registered at the access bridge after being shown,
    // so this will probably not help here....
    /*
    if( mbFloatWin && ((FloatingWindow*) this )->GrabsFocus() )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if( !mbVisible )
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
    return Size( mnOutWidth+mnLeftBorder+mnRightBorder,
                 mnOutHeight+mnTopBorder+mnBottomBorder );
}

void Window::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                               sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    rLeftBorder     = mnLeftBorder;
    rTopBorder      = mnTopBorder;
    rRightBorder    = mnRightBorder;
    rBottomBorder   = mnBottomBorder;
}


// -----------------------------------------------------------------------

void Window::Enable( BOOL bEnable, BOOL bChild )
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
        // mbDisabled darf erst nach Aufruf von ImplDlgCtrlNextWindow() gesetzt
        // werden. Ansonsten muss ImplDlgCtrlNextWindow() umgestellt werden
        if ( HasFocus() )
            ImplDlgCtrlNextWindow();
    }

    if ( mpBorderWindow )
    {
        mpBorderWindow->Enable( bEnable, FALSE );
        if ( (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow->Enable( bEnable, TRUE );
    }

    if ( mbDisabled != !bEnable )
    {
        mbDisabled = !bEnable;
        if ( mpSysObj )
            mpSysObj->Enable( bEnable && !mbInputDisabled );
//      if ( mbFrame )
//          mpFrame->Enable( bEnable && !mbInputDisabled );
        StateChanged( STATE_CHANGE_ENABLE );

        ImplCallEventListeners( bEnable ? VCLEVENT_WINDOW_ENABLED : VCLEVENT_WINDOW_DISABLED );
    }

    if ( bChild || mbChildNotify )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            pChild->Enable( bEnable, bChild );
            pChild = pChild->mpNext;
        }
    }

    if ( IsReallyVisible() )
        ImplGenerateMouseMove();
}

// -----------------------------------------------------------------------

void Window::EnableInput( BOOL bEnable, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    BOOL bNotify = (bEnable != mbInputDisabled);
    if ( mpBorderWindow )
    {
        mpBorderWindow->EnableInput( bEnable, FALSE );
        if ( (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpBorderWindow)->mpMenuBarWindow->EnableInput( bEnable, TRUE );
    }

    if ( !mbAlwaysEnableInput || bEnable )
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

        if ( mbInputDisabled != !bEnable )
        {
            mbInputDisabled = !bEnable;
            if ( mpSysObj )
                mpSysObj->Enable( !mbDisabled && bEnable );
//          if ( mbFrame )
//              mpFrame->Enable( !mbDisabled && bEnable );
        }
    }

    if ( bChild || mbChildNotify )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            pChild->EnableInput( bEnable, bChild );
            pChild = pChild->mpNext;
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
        Window* pSysWin = mpFrameWindow->mpFrameData->mpFirstOverlap;
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
            pSysWin = pSysWin->mpNextOverlap;
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
            pFrameWin = pFrameWin->mpFrameData->mpNextFrame;
        }


    }
}

// -----------------------------------------------------------------------

void Window::AlwaysEnableInput( BOOL bAlways, BOOL bChild )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->AlwaysEnableInput( bAlways, FALSE );

    if ( mbAlwaysEnableInput != bAlways )
    {
        mbAlwaysEnableInput = bAlways;

        if ( bAlways )
            EnableInput( TRUE, FALSE );
    }

    if ( bChild || mbChildNotify )
    {
        Window* pChild = mpFirstChild;
        while ( pChild )
        {
            pChild->AlwaysEnableInput( bAlways, bChild );
            pChild = pChild->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void Window::SetActivateMode( USHORT nMode )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpBorderWindow )
        mpBorderWindow->SetActivateMode( nMode );

    if ( mnActivateMode != nMode )
    {
        mnActivateMode = nMode;

        // Evtl. ein Decativate/Activate ausloesen
        if ( mnActivateMode )
        {
            if ( (mbActive || (GetType() == WINDOW_BORDERWINDOW)) &&
                 !HasChildPathFocus( TRUE ) )
            {
                mbActive = FALSE;
                Deactivate();
            }
        }
        else
        {
            if ( !mbActive || (GetType() == WINDOW_BORDERWINDOW) )
            {
                mbActive = TRUE;
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

    if ( mpBorderWindow )
    {
        mpBorderWindow->SetZOrder( pRefWindow, nFlags );
        return;
    }

    if ( nFlags & WINDOW_ZORDER_FIRST )
    {
        if ( ImplIsOverlapWindow() )
            pRefWindow = mpOverlapWindow->mpFirstOverlap;
        else
            pRefWindow = mpParent->mpFirstChild;
        nFlags |= WINDOW_ZORDER_BEFOR;
    }
    else if ( nFlags & WINDOW_ZORDER_LAST )
    {
        if ( ImplIsOverlapWindow() )
            pRefWindow = mpOverlapWindow->mpLastOverlap;
        else
            pRefWindow = mpParent->mpLastChild;
        nFlags |= WINDOW_ZORDER_BEHIND;
    }

    while ( pRefWindow->mpBorderWindow )
        pRefWindow = pRefWindow->mpBorderWindow;
    if ( (pRefWindow == this) || mbFrame )
        return;

    DBG_ASSERT( pRefWindow->mpParent == mpParent, "Window::SetZOrder() - pRefWindow has other parent" );
    if ( nFlags & WINDOW_ZORDER_BEFOR )
    {
        if ( pRefWindow->mpPrev == this )
            return;

        if ( ImplIsOverlapWindow() )
        {
            if ( mpPrev )
                mpPrev->mpNext = mpNext;
            else
                mpOverlapWindow->mpFirstOverlap = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpOverlapWindow->mpLastOverlap = mpPrev;
            if ( !pRefWindow->mpPrev )
                mpOverlapWindow->mpFirstOverlap = this;
        }
        else
        {
            if ( mpPrev )
                mpPrev->mpNext = mpNext;
            else
                mpParent->mpFirstChild = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpParent->mpLastChild = mpPrev;
            if ( !pRefWindow->mpPrev )
                mpParent->mpFirstChild = this;
        }

        mpPrev = pRefWindow->mpPrev;
        mpNext = pRefWindow;
        if ( mpPrev )
            mpPrev->mpNext = this;
        mpNext->mpPrev = this;
    }
    else if ( nFlags & WINDOW_ZORDER_BEHIND )
    {
        if ( pRefWindow->mpNext == this )
            return;

        if ( ImplIsOverlapWindow() )
        {
            if ( mpPrev )
                mpPrev->mpNext = mpNext;
            else
                mpOverlapWindow->mpFirstOverlap = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpOverlapWindow->mpLastOverlap = mpPrev;
            if ( !pRefWindow->mpNext )
                mpOverlapWindow->mpLastOverlap = this;
        }
        else
        {
            if ( mpPrev )
                mpPrev->mpNext = mpNext;
            else
                mpParent->mpFirstChild = mpNext;
            if ( mpNext )
                mpNext->mpPrev = mpPrev;
            else
                mpParent->mpLastChild = mpPrev;
            if ( !pRefWindow->mpNext )
                mpParent->mpLastChild = this;
        }

        mpPrev = pRefWindow;
        mpNext = pRefWindow->mpNext;
        if ( mpNext )
            mpNext->mpPrev = this;
        mpPrev->mpNext = this;
    }

    if ( IsReallyVisible() )
    {
        // Hintergrund-Sicherung zuruecksetzen
        if ( mpFrameData->mpFirstBackWin )
            ImplInvalidateAllOverlapBackgrounds();

        if ( mbInitWinClipRegion || !maWinClipRegion.IsEmpty() )
        {
            BOOL bInitWinClipRegion = mbInitWinClipRegion;
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
                    if ( mpOverlapWindow )
                        pWindow = mpOverlapWindow->mpFirstOverlap;
                }
                else
                    pWindow = ImplGetParent()->mpFirstChild;
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
                    pWindow = pWindow->mpNext;
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
                    pWindow = pWindow->mpNext;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

void Window::EnableAlwaysOnTop( BOOL bEnable )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mbAlwaysOnTop = bEnable;

    if ( mpBorderWindow )
        mpBorderWindow->EnableAlwaysOnTop( bEnable );
    else if ( bEnable && IsReallyVisible() )
        ToTop();

    if ( mbFrame )
        mpFrame->SetAlwaysOnTop( bEnable );
}

// -----------------------------------------------------------------------

void Window::SetPosSizePixel( long nX, long nY,
                              long nWidth, long nHeight, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    BOOL bHasValidSize = !mbDefSize;

    if ( nFlags & WINDOW_POSSIZE_POS )
        mbDefPos = FALSE;
    if ( nFlags & WINDOW_POSSIZE_SIZE )
        mbDefSize = FALSE;

    // Oberstes BorderWindow ist das Window, welches positioniert werden soll
    Window* pWindow = this;
    while ( pWindow->mpBorderWindow )
        pWindow = pWindow->mpBorderWindow;

    if ( pWindow->mbFrame )
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
            if( GetParent() && GetParent()->ImplHasMirroredGraphics() && !GetParent()->IsRTLEnabled() )
            {
                // --- RTL --- (re-mirror at parent window)
                Rectangle aRect( Point ( nX, nY ), Size( nWidth, nHeight ) );
                GetParent()->ImplReMirror( aRect );
                nX = aRect.nLeft;
            }
        }
        if( !(nFlags & WINDOW_POSSIZE_X) && bHasValidSize && pWindow->mpFrame->maGeometry.nWidth )
        {
            // --- RTL ---  make sure the old right aligned position is not changed
            //              system windows will always grow to the right
            if( pWindow->GetParent() && pWindow->GetParent()->ImplHasMirroredGraphics() )
            {
                long myWidth = nOldWidth;
                if( !myWidth )
                    myWidth = mpFrame->GetUnmirroredGeometry().nWidth;
                if( !myWidth )
                    myWidth = nWidth;
                nFlags |= WINDOW_POSSIZE_X;
                nSysFlags |= SAL_FRAME_POSSIZE_X;
                nX = mpFrame->GetUnmirroredGeometry().nX - pWindow->GetParent()->mpFrame->GetUnmirroredGeometry().nX -
                    mpFrame->GetUnmirroredGeometry().nLeftDecoration;
                nX = pWindow->GetParent()->mpFrame->GetUnmirroredGeometry().nX - mpFrame->GetUnmirroredGeometry().nLeftDecoration +
                    pWindow->GetParent()->mpFrame->GetUnmirroredGeometry().nWidth - myWidth - 1 - mpFrame->GetUnmirroredGeometry().nX;
                if(!(nFlags & WINDOW_POSSIZE_Y))
                {
                    nFlags |= WINDOW_POSSIZE_Y;
                    nSysFlags |= SAL_FRAME_POSSIZE_Y;
                    nY = mpFrame->GetUnmirroredGeometry().nY - pWindow->GetParent()->mpFrame->GetUnmirroredGeometry().nY -
                        mpFrame->GetUnmirroredGeometry().nTopDecoration;
                }
            }
        }
        if( nFlags & WINDOW_POSSIZE_Y )
            nSysFlags |= SAL_FRAME_POSSIZE_Y;
        pWindow->mpFrame->SetPosSize( nX, nY, nWidth, nHeight, nSysFlags );

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
    return maPos;
}

// -----------------------------------------------------------------------

Rectangle Window::GetDesktopRectPixel() const
{
    Rectangle rRect;
    mpFrameWindow->mpFrame->GetWorkArea( rRect );
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
        if( mpParent && !mpParent->mbFrame && mpParent->ImplHasMirroredGraphics() && !mpParent->IsRTLEnabled() )
        {
            if ( !ImplIsOverlapWindow() )
                offx -= mpParent->mnOutOffX;

            offx = mpParent->mnOutWidth - mnOutWidth - offx;

            if ( !ImplIsOverlapWindow() )
                offx += mpParent->mnOutOffX;

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
    SalFrameGeometry g = mpFrame->GetGeometry();
    p.X() += g.nX;
    p.Y() += g.nY;
    return p;
}

// -----------------------------------------------------------------------

Point Window::AbsoluteScreenToOutputPixel( const Point& rPos ) const
{
    // relative to the screen
    Point p = ScreenToOutputPixel( rPos );
    SalFrameGeometry g = mpFrame->GetGeometry();
    p.X() -= g.nX;
    p.Y() -= g.nY;
    return p;
}

// -----------------------------------------------------------------------

Rectangle Window::ImplOutputToUnmirroredAbsoluteScreenPixel( const Rectangle &rRect ) const
{
    // this method creates unmirrored screen coordinates to be compared with the desktop
    // and is used for positioning of RTL popup windows correctly on the screen
    SalFrameGeometry g = mpFrame->GetUnmirroredGeometry();

    Point p1 = OutputToScreenPixel( rRect.TopRight() );
    p1.X() = g.nX+g.nWidth-p1.X();
    p1.Y() += g.nY;

    Point p2 = OutputToScreenPixel( rRect.BottomLeft() );
    p2.X() = g.nX+g.nWidth-p2.X();
    p2.Y() += g.nY;

    return Rectangle( p1, p2 );
}


// -----------------------------------------------------------------------

Rectangle Window::GetWindowExtentsRelative( Window *pRelativeWindow )
{
    // with decoration
    return ImplGetWindowExtentsRelative( pRelativeWindow, FALSE );
}

Rectangle Window::GetClientWindowExtentsRelative( Window *pRelativeWindow )
{
    // without decoration
    return ImplGetWindowExtentsRelative( pRelativeWindow, TRUE );
}

// -----------------------------------------------------------------------

Rectangle Window::ImplGetWindowExtentsRelative( Window *pRelativeWindow, BOOL bClientOnly )
{
    SalFrameGeometry g = mpFrame->GetGeometry();
    // make sure we use the extent of our border window,
    // otherwise we miss a few pixels
    Window *pWin = (!bClientOnly && mpBorderWindow) ? mpBorderWindow : this;

    Point aPos( pWin->OutputToScreenPixel( Point(0,0) ) );
    aPos.X() += g.nX;
    aPos.Y() += g.nY;
    Size aSize ( pWin->GetSizePixel() );
    // #104088# do not add decoration to the workwindow to be compatible to java accessibility api
    if( !bClientOnly && (mbFrame || (mpBorderWindow && mpBorderWindow->mbFrame && GetType() != WINDOW_WORKWINDOW)) )
    {
        aPos.X() -= g.nLeftDecoration;
        aPos.Y() -= g.nTopDecoration;
        aSize.Width() += g.nLeftDecoration + g.nRightDecoration;
        aSize.Height() += g.nTopDecoration + g.nBottomDecoration;
    }
    if( pRelativeWindow )
    {
        // #106399# express coordinates relative to borderwindow
        Window *pRelWin = (!bClientOnly && pRelativeWindow->mpBorderWindow) ? pRelativeWindow->mpBorderWindow : pRelativeWindow;
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

    if ( !mbReallyVisible )
        return FALSE;

    if ( mpFrameWindow->mbPaintFrame )
        return TRUE;

    if ( mnPaintFlags & IMPL_PAINT_PAINT )
        return TRUE;

    if ( !ImplIsOverlapWindow() )
    {
        const Window* pTempWindow = this;
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mnPaintFlags & (IMPL_PAINT_PAINTCHILDS | IMPL_PAINT_PAINTALLCHILDS) )
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

    if ( mpBorderWindow )
    {
        mpBorderWindow->Update();
        return;
    }

    if ( !mbReallyVisible )
        return;

    BOOL bFlush = FALSE;
    if ( mpFrameWindow->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mbFrame || (mpBorderWindow && mpBorderWindow->mbFrame) )
            bFlush = TRUE;
    }

    // Zuerst muessen wir alle Fenster ueberspringen, die Paint-Transparent
    // sind
    Window* pUpdateWindow = this;
    Window* pWindow = pUpdateWindow;
    while ( !pWindow->ImplIsOverlapWindow() )
    {
        if ( !pWindow->mbPaintTransparent )
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
        if ( pWindow->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDS )
            pUpdateWindow = pWindow;
        if ( pWindow->ImplIsOverlapWindow() )
            break;
        pWindow = pWindow->ImplGetParent();
    }
    while ( pWindow );

    // Wenn es etwas zu malen gibt, dann ein Paint ausloesen
    if ( pUpdateWindow->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDS) )
    {
         // und fuer alle ueber uns stehende System-Fenster auch ein Update
         // ausloesen, damit nicht die ganze Zeit luecken stehen bleiben
         Window* pUpdateOverlapWindow = ImplGetFirstOverlapWindow()->mpFirstOverlap;
         while ( pUpdateOverlapWindow )
         {
             pUpdateOverlapWindow->Update();
             pUpdateOverlapWindow = pUpdateOverlapWindow->mpNext;
         }

        pUpdateWindow->ImplCallPaint( NULL, pUpdateWindow->mnPaintFlags );
    }

    if ( bFlush )
        Flush();
}

// -----------------------------------------------------------------------

void Window::Flush()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    mpFrame->Flush();
}

// -----------------------------------------------------------------------

void Window::Sync()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mpFrame->Sync();
}

// -----------------------------------------------------------------------

void Window::SetUpdateMode( BOOL bUpdate )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mbNoUpdate = !bUpdate;
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
    if( pFocusWin && pFocusWin->mbFloatWin && ((FloatingWindow *)pFocusWin)->GrabsFocus() )
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

// -----------------------------------------------------------------------

BOOL Window::HasChildPathFocus( BOOL bSystemWindow ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    // #107575#, the first floating window always has the keyboard focus, see also winproc.cxx: ImplGetKeyInputWindow()
    //  task was shifted to 6.y, so its commented out
    /*
    Window* pFocusWin = ImplGetSVData()->maWinData.mpFirstFloat;
    if( pFocusWin && pFocusWin->mbFloatWin && ((FloatingWindow *)pFocusWin)->GrabsFocus() )
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
        mpFrame->CaptureMouse( TRUE );
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
        mpFrame->CaptureMouse( FALSE );
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

    if ( maPointer == rPointer )
        return;

    maPointer   = rPointer;

    // Pointer evt. direkt umsetzen
    if ( !mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpFrame->SetPointer( ImplGetMousePointer() );
}

// -----------------------------------------------------------------------

void Window::EnableChildPointerOverwrite( BOOL bOverwrite )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mbChildPtrOverwrite == bOverwrite )
        return;

    mbChildPtrOverwrite  = bOverwrite;

    // Pointer evt. direkt umsetzen
    if ( !mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpFrame->SetPointer( ImplGetMousePointer() );
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
    mpFrame->SetPointerPos( aPos.X(), aPos.Y() );
}

// -----------------------------------------------------------------------

Point Window::GetPointerPosPixel()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Point aPos( mpFrameData->mnLastMouseX, mpFrameData->mnLastMouseY );
    if( ImplHasMirroredGraphics() && !IsRTLEnabled() )
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

    Point aPos( mpFrameData->mnBeforeLastMouseX, mpFrameData->mnBeforeLastMouseY );
#ifndef REMOTE_APPSERVER
    if( ImplHasMirroredGraphics() && !IsRTLEnabled() )
    {
        // --- RTL --- (re-mirror mouse pos at this window)
        ImplReMirror( aPos );
    }
#endif
    return ImplFrameToOutput( aPos );
}

// -----------------------------------------------------------------------

void Window::ShowPointer( BOOL bVisible )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mbNoPtrVisible != !bVisible )
    {
        mbNoPtrVisible = !bVisible;

        // Pointer evt. direkt umsetzen
        if ( !mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
            mpFrame->SetPointer( ImplGetMousePointer() );
    }
}

// -----------------------------------------------------------------------

Window::PointerState Window::GetPointerState()
{
    PointerState aState;
    aState.mnState = 0;

    if (mpFrame)
    {
        SalFrame::SalPointerState aSalPointerState;

        aSalPointerState = mpFrame->GetPointerState();
        if( ImplHasMirroredGraphics() && !IsRTLEnabled() )
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

    mnWaitCount++;

    if ( mnWaitCount == 1 )
    {
        // Pointer evt. direkt umsetzen
        if ( !mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
            mpFrame->SetPointer( ImplGetMousePointer() );
    }
}

// -----------------------------------------------------------------------

void Window::LeaveWait()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mnWaitCount )
    {
        mnWaitCount--;

        if ( !mnWaitCount )
        {
            // Pointer evt. direkt umsetzen
            if ( !mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
                mpFrame->SetPointer( ImplGetMousePointer() );
        }
    }
}

// -----------------------------------------------------------------------

void Window::SetCursor( Cursor* pCursor )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpCursor != pCursor )
    {
        if ( mpCursor )
            mpCursor->ImplHide();
        mpCursor = pCursor;
        if ( pCursor )
            pCursor->ImplShow();
    }
}

// -----------------------------------------------------------------------

void Window::SetText( const XubString& rStr )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    String oldTitle( maText );
    maText = rStr;

    if ( mpBorderWindow )
        mpBorderWindow->SetText( rStr );
    else if ( mbFrame )
        mpFrame->SetTitle( rStr );

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

    return maText;
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
    if( !IsBackground() )
    {
        if( mpParent )
            return mpParent->GetDisplayBackground();
    }

    const Wallpaper& rBack = GetBackground();
    if( ! rBack.IsBitmap() &&
        ! rBack.IsGradient() &&
        rBack.GetColor().GetColor() == COL_TRANSPARENT &&
        mpParent )
            return mpParent->GetDisplayBackground();
    return rBack;
}

// -----------------------------------------------------------------------

const XubString& Window::GetHelpText() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !maHelpText.Len() && mnHelpId )
    {
        if ( !IsDialog() && (mnType != WINDOW_TABPAGE) && (mnType != WINDOW_FLOATINGWINDOW) )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
                ((Window*)this)->maHelpText = pHelp->GetHelpText( GetHelpId(), this );
        }
    }

    return maHelpText;
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
    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        nChildCount++;
        pChild = pChild->mpNext;
    }

    return nChildCount;
}

// -----------------------------------------------------------------------

Window* Window::GetChild( USHORT nChild ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    USHORT  nChildCount = 0;
    Window* pChild = mpFirstChild;
    while ( pChild )
    {
        if ( nChild == nChildCount )
            return pChild;
        pChild = pChild->mpNext;
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
            return mpRealParent;

        case WINDOW_FIRSTCHILD:
            return mpFirstChild;

        case WINDOW_LASTCHILD:
            return mpLastChild;

        case WINDOW_PREV:
            return mpPrev;

        case WINDOW_NEXT:
            return mpNext;

        case WINDOW_FIRSTOVERLAP:
            return mpFirstOverlap;

        case WINDOW_LASTOVERLAP:
            return mpLastOverlap;

        case WINDOW_OVERLAP:
            if ( ImplIsOverlapWindow() )
                return (Window*)this;
            else
                return mpOverlapWindow;

        case WINDOW_PARENTOVERLAP:
            if ( ImplIsOverlapWindow() )
                return mpOverlapWindow;
            else
                return mpOverlapWindow->mpOverlapWindow;

        case WINDOW_CLIENT:
            return ((Window*)this)->ImplGetWindow();

        case WINDOW_REALPARENT:
            return ImplGetParent();

        case WINDOW_FRAME:
            return mpFrameWindow;

        case WINDOW_BORDER:
            if ( mpBorderWindow )
                return mpBorderWindow->GetWindow( WINDOW_BORDER );
            return (Window*)this;
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

    return mpFrame ? mpFrame->GetSystemData() : NULL;
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
    mxWindowPeer = xPeer;
    mpVCLXWindow = pVCLXWindow;
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > Window::GetComponentInterface( BOOL bCreate )
{
    if ( !mxWindowPeer.is() && bCreate )
    {
        UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
        if ( pWrapper )
            mxWindowPeer = pWrapper->GetWindowInterface( this, sal_True );
    }
    return mxWindowPeer;
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
        ImplCallEventListeners( VCLEVENT_WINDOW_DEACTIVATE );
        // #100759#, avoid walking the wrong frame's hierarchy
        //           eg, undocked docking windows (ImplDockFloatWin)
        if ( ImplGetParent() && mpFrameWindow == ImplGetParent()->mpFrameWindow )
            ImplGetParent()->ImplCallDeactivateListeners( pNew );
    }
}

// -----------------------------------------------------------------------

void Window::ImplCallActivateListeners( Window *pOld )
{
    // no activation if the the old active window is my child
    if ( !pOld || !ImplIsChild( pOld ) )
    {
        ImplCallEventListeners( VCLEVENT_WINDOW_ACTIVATE, pOld );
        // #106298# revoke the change for 105369, because this change
        //          disabled the activate event for the parent,
        //          if the parent is a compound control
        //if( !GetParent() || !GetParent()->IsCompoundControl() )
        //{
            // #100759#, avoid walking the wrong frame's hierarchy
            //           eg, undocked docking windows (ImplDockFloatWin)
            // #104714#, revert the changes for 100759 because it has a side effect when pOld is a dialog
            //           additionally the gallery is not dockable anymore, so 100759 canot occur
            if ( ImplGetParent() ) /* && mpFrameWindow == ImplGetParent()->mpFrameWindow ) */
                ImplGetParent()->ImplCallActivateListeners( pOld );
            else
            {
                // top level frame reached: store hint for DefModalDialogParent
                ImplGetSVData()->maWinData.mpActiveApplicationFrame = mpFrameWindow;
            }
        //}
    }
}

// -----------------------------------------------------------------------

bool Window::ImplStopDnd()
{
    bool bRet = false;
    if( mpFrameData && mpFrameData->mxDropTargetListener.is() )
    {
        bRet = true;
        mpFrameData->mxDropTarget.clear();
        mpFrameData->mxDragSource.clear();
        mpFrameData->mxDropTargetListener.clear();
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

    if( ! mxDNDListenerContainer.is() )
    {
        sal_Int8 nDefaultActions = 0;

        if( mpFrameData )
        {
            if( ! mpFrameData->mxDropTarget.is() )
            {
                // initialization is done in GetDragSource
                Reference< XDragSource > xDragSource = GetDragSource();
            }

            if( mpFrameData->mxDropTarget.is() )
            {
                nDefaultActions = mpFrameData->mxDropTarget->getDefaultActions();

                if( ! mpFrameData->mxDropTargetListener.is() )
                {
                    mpFrameData->mxDropTargetListener = new DNDEventDispatcher( mpFrameWindow );

                    try
                    {
//                        OSL_TRACE( "adding droptarget listener" );
                        mpFrameData->mxDropTarget->addDropTargetListener( mpFrameData->mxDropTargetListener );

                        // register also as drag gesture listener if directly supported by drag source
                        Reference< XDragGestureRecognizer > xDragGestureRecognizer =
                            Reference< XDragGestureRecognizer > (mpFrameData->mxDragSource, UNO_QUERY);

                        if( xDragGestureRecognizer.is() )
                        {
                            xDragGestureRecognizer->addDragGestureListener(
                                Reference< XDragGestureListener > (mpFrameData->mxDropTargetListener, UNO_QUERY));
                        }
                        else
                            mpFrameData->mbInternalDragGestureRecognizer = TRUE;

                    }

                    catch( RuntimeException exc )
                    {
                        // release all instances
                        mpFrameData->mxDropTarget.clear();
                        mpFrameData->mxDragSource.clear();
                    }
                }
            }

        }

        mxDNDListenerContainer = static_cast < XDropTarget * > ( new DNDListenerContainer( nDefaultActions ) );
    }

    // this object is located in the same process, so there will be no runtime exception
    return Reference< XDropTarget > ( mxDNDListenerContainer, UNO_QUERY );
}

// -----------------------------------------------------------------------

Reference< XDragSource > Window::GetDragSource()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mpFrameData )
    {
        if( ! mpFrameData->mxDragSource.is() )
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
                        aDragSourceSN = OUString::createFromAscii( "com.sun.star.datatransfer.dnd.OleDragSource" );
                        aDropTargetSN = OUString::createFromAscii( "com.sun.star.datatransfer.dnd.OleDropTarget" );
                        aDragSourceAL[ 1 ] = makeAny( (sal_uInt32) pEnvData->hWnd );
                        aDropTargetAL[ 0 ] = makeAny( (sal_uInt32) pEnvData->hWnd );
#elif defined UNX
                        aDropTargetAL.realloc( 3 );
                        aDragSourceAL.realloc( 3 );
                        aDragSourceSN = OUString::createFromAscii( "com.sun.star.datatransfer.dnd.X11DragSource" );
                        aDropTargetSN = OUString::createFromAscii( "com.sun.star.datatransfer.dnd.X11DropTarget" );

                        aDragSourceAL[ 0 ] = makeAny( Application::GetDisplayConnection() );
                        aDragSourceAL[ 2 ] = makeAny( vcl::createBmpConverter() );
                        aDropTargetAL[ 0 ] = makeAny( Application::GetDisplayConnection() );
                        aDropTargetAL[ 1 ] = makeAny( (sal_Size)(pEnvData->aShellWindow) );
                        aDropTargetAL[ 2 ] = makeAny( vcl::createBmpConverter() );
#endif
                        if( aDragSourceSN.getLength() )
                            mpFrameData->mxDragSource = Reference< XDragSource > ( xFactory->createInstanceWithArguments( aDragSourceSN, aDragSourceAL ), UNO_QUERY );

                        if( aDropTargetSN.getLength() )
                            mpFrameData->mxDropTarget = Reference< XDropTarget > ( xFactory->createInstanceWithArguments( aDropTargetSN, aDropTargetAL ), UNO_QUERY );
                    }
                }
            }

            // createInstance can throw any exception
            catch( Exception exc )
            {
                // release all instances
                mpFrameData->mxDropTarget.clear();
                mpFrameData->mxDragSource.clear();
            }
        }

        return mpFrameData->mxDragSource;
    }

    return Reference< XDragSource > ();
}

// -----------------------------------------------------------------------

void Window::GetDragSourceDropTarget(Reference< XDragSource >& xDragSource, Reference< XDropTarget > &xDropTarget )
// only for RVP transmission
{
    if( mpFrameData )
    {
        // initialization is done in GetDragSource
        xDragSource = GetDragSource();
        xDropTarget = mpFrameData->mxDropTarget;
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

    if( mpFrameData )
    {
        if( ! mpFrameData->mxClipboard.is() )
        {
            try
            {
                Reference< XMultiServiceFactory > xFactory( vcl::unohelper::GetMultiServiceFactory() );

                if( xFactory.is() )
                {
                    mpFrameData->mxClipboard = Reference< XClipboard >( xFactory->createInstance( OUString::createFromAscii( "com.sun.star.datatransfer.clipboard.SystemClipboard" ) ), UNO_QUERY );

#ifdef UNX          // unix clipboard needs to be initialized
                    if( mpFrameData->mxClipboard.is() )
                    {
                        Reference< XInitialization > xInit = Reference< XInitialization >( mpFrameData->mxClipboard, UNO_QUERY );

                        if( xInit.is() )
                        {
                            Sequence< Any > aArgumentList( 3 );
                            aArgumentList[ 0 ] = makeAny( Application::GetDisplayConnection() );
                            aArgumentList[ 1 ] = makeAny( OUString::createFromAscii( "CLIPBOARD" ) );
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
                mpFrameData->mxClipboard.clear();
            }
        }

        return mpFrameData->mxClipboard;
    }

    return static_cast < XClipboard * > (0);
}

// -----------------------------------------------------------------------

Reference< XClipboard > Window::GetSelection()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mpFrameData )
    {
        if( ! mpFrameData->mxSelection.is() )
        {
            try
            {
                Reference< XMultiServiceFactory > xFactory( vcl::unohelper::GetMultiServiceFactory() );

                if( xFactory.is() )
                {
#   ifdef UNX
                    Sequence< Any > aArgumentList( 3 );
                      aArgumentList[ 0 ] = makeAny( Application::GetDisplayConnection() );
                    aArgumentList[ 1 ] = makeAny( OUString::createFromAscii( "PRIMARY" ) );
                    aArgumentList[ 2 ] = makeAny( vcl::createBmpConverter() );

                    mpFrameData->mxSelection = Reference< XClipboard >( xFactory->createInstanceWithArguments(
                    OUString::createFromAscii( "com.sun.star.datatransfer.clipboard.SystemClipboard" ), aArgumentList ), UNO_QUERY );
#   else
                    static Reference< XClipboard >  s_xSelection;

                    if ( !s_xSelection.is() )
                        s_xSelection = Reference< XClipboard >( xFactory->createInstance( OUString::createFromAscii( "com.sun.star.datatransfer.clipboard.GenericClipboard" ) ), UNO_QUERY );

                    mpFrameData->mxSelection = s_xSelection;
#   endif
                }
            }

            // createInstance can throw any exception
            catch( Exception exc )
            {
                // release all instances
                mpFrameData->mxSelection.clear();
            }
        }

        return mpFrameData->mxSelection;
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
    if ( !mxAccessible.is() && bCreate )
        mxAccessible = CreateAccessible();

    return mxAccessible;
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > Window::CreateAccessible()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc( GetComponentInterface( TRUE ), ::com::sun::star::uno::UNO_QUERY );
    return xAcc;
}

void Window::SetAccessible( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > x )
{
    mxAccessible = x;
}

// skip all border windows that are no top level frames
BOOL Window::ImplIsAccessibleCandidate() const
{
    if( !mbBorderWin )
        return TRUE;
    else
        // #101741 do not check for WB_CLOSEABLE because undecorated floaters (like menues!) are closeable
        if( mbFrame && mnStyle & (WB_MOVEABLE | WB_SIZEABLE) )
            return TRUE;
        else
            return FALSE;
}

BOOL Window::ImplIsAccessibleNativeFrame() const
{
    if( mbFrame )
        // #101741 do not check for WB_CLOSEABLE because undecorated floaters (like menues!) are closeable
        if( (mnStyle & (WB_MOVEABLE | WB_SIZEABLE)) )
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
            nChildren += pChild->ImplGetAccessibleCandidateChildWindowCount( WINDOW_FIRSTCHILD );
        pChild = pChild->mpNext;
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
        pChild = pChild->mpNext;
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

    Window* pParent = mpParent;
    if( GetType() == WINDOW_MENUBARWINDOW )
    {
        // report the menubar as a child of THE workwindow
        Window *pWorkWin = GetParent()->mpFirstChild;
        while( pWorkWin && (pWorkWin == this) )
            pWorkWin = pWorkWin->mpNext;
        pParent = pWorkWin;
    }
    // If this a floating window which has a native boarder window, this one should be reported as
    // accessible parent
    else if( GetType() == WINDOW_FLOATINGWINDOW &&
        mpBorderWindow && mpBorderWindow->mbFrame)
    {
        pParent = mpBorderWindow;
    }
    else if( pParent && !pParent->ImplIsAccessibleCandidate() )
    {
        pParent = pParent->mpParent;
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
    Window* pChild = mpFirstChild;
    while( pChild )
    {
        if( pChild->IsVisible() )
            nChildren++;
        pChild = pChild->mpNext;
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
    Window* pChild = mpFirstChild;
    while( pChild )
    {
        if( pChild->IsVisible() )
        {
            if( ! nChildren )
                break;
            nChildren--;
        }
        pChild = pChild->mpNext;
    }

    if( GetType() == WINDOW_BORDERWINDOW && pChild && pChild->GetType() == WINDOW_MENUBARWINDOW )
    {
        do pChild = pChild->mpNext; while( pChild && ! pChild->IsVisible() );
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
    if ( !mpAccessibleInfos )
        mpAccessibleInfos = new ImplAccessibleInfos;

    DBG_ASSERT( mpAccessibleInfos->nAccessibleRole == 0xFFFF, "AccessibleRole already set!" );
    mpAccessibleInfos->nAccessibleRole = nRole;
}

USHORT Window::GetAccessibleRole() const
{
    using namespace ::com::sun::star;

    USHORT nRole = mpAccessibleInfos ? mpAccessibleInfos->nAccessibleRole : 0xFFFF;
    if ( nRole == 0xFFFF )
    {
        switch ( GetType() )
        {
            case WINDOW_MESSBOX:    // MT: Would be nice to have special roles!
            case WINDOW_INFOBOX:
            case WINDOW_WARNINGBOX:
            case WINDOW_ERRORBOX:
            case WINDOW_QUERYBOX: nRole = accessibility::AccessibleRole::DIALOG; break; // #i12331, DIALOG must be used to
                                                                                        // to allow activation, those are frames!
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
            case WINDOW_SYSWINDOW:
            case WINDOW_FLOATINGWINDOW: nRole = mbFrame ? accessibility::AccessibleRole::FRAME :
                                                            accessibility::AccessibleRole::PANEL; break;

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
   if ( !mpAccessibleInfos )
        mpAccessibleInfos = new ImplAccessibleInfos;

    DBG_ASSERT( !mpAccessibleInfos->pAccessibleName, "AccessibleName already set!" );
    delete mpAccessibleInfos->pAccessibleName;
    mpAccessibleInfos->pAccessibleName = new String( rName );
}

String Window::GetAccessibleName() const
{
    String aAccessibleName;
    if ( mpAccessibleInfos && mpAccessibleInfos->pAccessibleName )
    {
        aAccessibleName = *mpAccessibleInfos->pAccessibleName;
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
   if ( ! mpAccessibleInfos )
        mpAccessibleInfos = new ImplAccessibleInfos;

    DBG_ASSERT( !mpAccessibleInfos->pAccessibleDescription, "AccessibleDescription already set!" );
    delete mpAccessibleInfos->pAccessibleDescription;
    mpAccessibleInfos->pAccessibleDescription = new String( rDescription );
}

String Window::GetAccessibleDescription() const
{
    String aAccessibleDescription;
    if ( mpAccessibleInfos && mpAccessibleInfos->pAccessibleDescription )
    {
        aAccessibleDescription = *mpAccessibleInfos->pAccessibleDescription;
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
        return mbSuppressAccessibilityEvents;
    else
    {
        Window *pParent = this;
        while ( pParent )
        {
            if( pParent->mbSuppressAccessibilityEvents )
                return TRUE;
            else
                pParent = pParent->mpParent; // do not use GetParent() to find borderwindows that are frames
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
    extern void ImplRGBtoHSB( const Color& rColor, USHORT& nHue, USHORT& nSat, USHORT& nBri );
    extern Color ImplHSBtoRGB( USHORT nHue, USHORT nSat, USHORT nBri );

    if( rRect.IsEmpty() )
        return;

    // colors used for item highlighting
    Color aSelectionBorderCol( GetSettings().GetStyleSettings().GetHighlightColor() );
    Color aSelectionFillCol( aSelectionBorderCol );

    BOOL bDark = GetSettings().GetStyleSettings().GetFaceColor().IsDark();
    BOOL bBright = GetSettings().GetStyleSettings().GetFaceColor().IsBright();

    int c1 = aSelectionBorderCol.GetLuminance();
    int c2 = GetDisplayBackground().GetColor().GetLuminance();

    if( !bDark && !bBright && abs( c2-c1 ) < 75 )
    {
        // constrast too low
        USHORT h,s,b;
        ImplRGBtoHSB( aSelectionFillCol, h, s, b );
        if( b > 50 )    b -= 40;
        else            b += 40;
        aSelectionFillCol = ImplHSBtoRGB( h, s, b );
        aSelectionBorderCol = aSelectionFillCol;
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

    USHORT nPercent;
    if( !highlight )
    {
        if( bDark )
            aSelectionFillCol = COL_BLACK;
        else
            nPercent = 80;              // just checked (light)
    }
    else
    {
        if( bChecked || highlight == 1 )
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
                nPercent = 35;          // selected, pressed or checked ( very dark )
        }
        else
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
                nPercent = 70;          // selected ( dark )
        }
    }

    if( bDark && bDrawExtBorderOnly )
        SetFillColor();
    else
        SetFillColor( aSelectionFillCol );


    if( bDark )
    {
        DrawRect( aRect );
    }
    else
    {
        Polygon aPoly( aRect );
        PolyPolygon aPolyPoly( aPoly );
        DrawTransparent( aPolyPoly, nPercent );
    }

    SetFillColor( oldFillCol );
    SetLineColor( oldLineCol );
}

/*
void Window::DbgAssertNoEventListeners()
{
    VclWindowEvent aEvent( this, 0, NULL );
    DBG_ASSERT( maEventListeners.empty(), "Eventlistener: Who is still listening???" )
    if ( !maEventListeners.empty() )
        maEventListeners.Call( &aEvent );

    DBG_ASSERT( maChildEventListeners.empty(), "ChildEventlistener: Who is still listening???" )
    if ( !maChildEventListeners.empty() )
        maChildEventListeners.Call( &aEvent );
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
    Window *pChild = mpFirstChild;
    while( pChild )
    {
        if( pChild->GetType() == WINDOW_SCROLLBAR )
            return true;
        else
            pChild = pChild->mpNext;
    }
    return false;
}

BOOL Window::IsTopWindow() const
{
    if ( mbInDtor )
        return FALSE;

    // topwindows must be frames or they must have a borderwindow which is a frame
    if( !mbFrame && (!mpBorderWindow || (mpBorderWindow && !mpBorderWindow->mbFrame) ) )
        return FALSE;

    ImplGetWinData();
    if( mpWinData->mnIsTopWindow == (USHORT)~0)    // still uninitialized
    {
        // #113722#, cache result of expensive queryInterface call
        Window *pThisWin = (Window*)this;
        Reference< XTopWindow > xTopWindow( pThisWin->GetComponentInterface(), UNO_QUERY );
        pThisWin->mpWinData->mnIsTopWindow = xTopWindow.is() ? 1 : 0;
    }
    return mpWinData->mnIsTopWindow == 1 ? TRUE : FALSE;
}

void Window::ImplMirrorFramePos( Point &pt ) const
{
    pt.X() = mpFrame->maGeometry.nWidth-1-pt.X();
}

// frame based modal counter (dialogs are not modal to the whole application anymore)
BOOL Window::IsInModalMode() const
{
    return (mpFrameWindow->mpFrameData->mnModalMode != 0);
}
void Window::ImplIncModalCount()
{
    mpFrameWindow->mpFrameData->mnModalMode++;
}
void Window::ImplDecModalCount()
{
    mpFrameWindow->mpFrameData->mnModalMode--;
}
void Window::ImplNotifyIconifiedState( BOOL bIconified )
{
    mpFrameWindow->ImplCallEventListeners( bIconified ? VCLEVENT_WINDOW_MINIMIZE : VCLEVENT_WINDOW_NORMALIZE );
    // #109206# notify client window as well to have toolkit topwindow listeners notified
    if( mpFrameWindow->mpClientWindow && mpFrameWindow != mpFrameWindow->mpClientWindow )
        mpFrameWindow->mpClientWindow->ImplCallEventListeners( bIconified ? VCLEVENT_WINDOW_MINIMIZE : VCLEVENT_WINDOW_NORMALIZE );
}

BOOL Window::HasActiveChildFrame()
{
    BOOL bRet = FALSE;
    Window *pFrameWin = ImplGetSVData()->maWinData.mpFirstFrame;
    while( pFrameWin )
    {
        if( pFrameWin != mpFrameWindow )
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
            if( bDecorated || (pFrameWin->mnStyle & (WB_MOVEABLE | WB_SIZEABLE) ) )
                if( pChildFrame && pChildFrame->IsVisible() && pChildFrame->IsActive() )
                {
                    if( ImplIsChild( pChildFrame, TRUE ) )
                    {
                        bRet = TRUE;
                        break;
                    }
                }
        }
        pFrameWin = pFrameWin->mpFrameData->mpNextFrame;
    }
    return bRet;
}

LanguageType Window::GetInputLanguage() const
{
    return mpFrame->GetInputLanguage();
}

void Window::EnableNativeWidget( BOOL bEnable )
{
    if( bEnable != ImplGetWinData()->mbEnableNativeWidget )
    {
        ImplGetWinData()->mbEnableNativeWidget = bEnable;
        // sometimes the borderwindow is queried, so keep it in sync
        if( mpBorderWindow )
            mpBorderWindow->ImplGetWinData()->mbEnableNativeWidget = bEnable;
    }

    // push down, useful for compound controls
    Window *pChild = mpFirstChild;
    while( pChild )
    {
        pChild->EnableNativeWidget( bEnable );
        pChild = pChild->mpNext;
    }
}

BOOL Window::IsNativeWidgetEnabled() const
{
    return ImplGetWinData()->mbEnableNativeWidget;
}

Reference< ::drafts::com::sun::star::rendering::XCanvas > Window::GetCanvas() const
{
    if( mxCanvas.is() )
        return mxCanvas;

    Sequence< Any > aArg( 4 );

    // Feed any with operating system's window handle
    // ==============================================

    // common: first any is VCL pointer to window (for VCL canvas)
    aArg[ 0 ] = makeAny( reinterpret_cast<sal_Int64>(this) );

#if defined( WIN ) || defined( WNT )
    // take HWND for Windows
    aArg[ 1 ] = makeAny( reinterpret_cast<sal_Int32>(mpFrame->GetSystemData()->hWnd) );
#elif defined( UNX )
    // take XLIB window for X11, and fake a motif widget ID from
    // that.

    // feed the motif widget ID to canvas
    //aArg[ 0 ] = makeAny( vcl::createMotifHandle( mpFrame->maFrameData.GetWindow() ) );

    // feed the X11 window handle to canvas
    aArg[ 1 ] = makeAny( static_cast<sal_Int32>(mpFrame->GetSystemData()->aWindow) );
#else
# error Please forward window handle to canvas for your OS
#endif

    aArg[ 2 ] = makeAny( ::com::sun::star::awt::Rectangle( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight ) );

    aArg[ 3 ] = makeAny( mbAlwaysOnTop ? sal_True : sal_False );

    Reference< XMultiServiceFactory > xFactory = vcl::unohelper::GetMultiServiceFactory();

    // Create canvas instance with window handle
    // =========================================
    if ( xFactory.is() )
    {
        mxCanvas = Reference< ::drafts::com::sun::star::rendering::XCanvas >(
            xFactory->createInstanceWithArguments( OUString(RTL_CONSTASCII_USTRINGPARAM("drafts.com.sun.star.rendering.Canvas")),
                                                   aArg ),
            UNO_QUERY );

        // now, try to retrieve an XWindow interface from the canvas
        // (this is an implementation detail of the Java interface,
        // which might vanish without notice).
        mxCanvasWindow = Reference< ::com::sun::star::awt::XWindow >(mxCanvas, UNO_QUERY );
    }

    // no factory??? Empty reference, then.
    return mxCanvas;
}

Reference< ::drafts::com::sun::star::rendering::XCanvas > Window::GetFullscreenCanvas( const Size& rFullscreenSize ) const
{
    if( mxCanvas.is() )
        return mxCanvas;

    Sequence< Any > aArg( 4 );

    // Feed any with operating system's window handle
    // ==============================================

    // common: first any is VCL pointer to window (for VCL canvas)
    aArg[ 0 ] = makeAny( reinterpret_cast<sal_Int64>(this) );

#if defined( WIN ) || defined( WNT )
    // take HWND for Windows
    aArg[ 1 ] = makeAny( reinterpret_cast<sal_Int32>(mpFrame->GetSystemData()->hWnd) );
#elif defined( UNX )
    // take XLIB window for X11, and fake a motif widget ID from
    // that.

    // feed the motif widget ID to canvas
    //aArg[ 0 ] = makeAny( vcl::createMotifHandle( mpFrame->maFrameData.GetWindow() ) );

    // feed the X11 window handle to canvas
    aArg[ 1 ] = makeAny( static_cast<sal_Int32>(mpFrame->GetSystemData()->aWindow) );
#else
# error Please forward window handle to canvas for your OS
#endif

    aArg[ 2 ] = makeAny( ::com::sun::star::awt::Rectangle( 0, 0,
                                                           rFullscreenSize.Width(),
                                                           rFullscreenSize.Height() ) );

    aArg[ 3 ] = makeAny( sal_True );

    Reference< XMultiServiceFactory > xFactory = vcl::unohelper::GetMultiServiceFactory();

    // Create canvas instance with window handle
    // =========================================
    if ( xFactory.is() )
    {
        mxCanvas = Reference< ::drafts::com::sun::star::rendering::XCanvas >(
            xFactory->createInstanceWithArguments( OUString(RTL_CONSTASCII_USTRINGPARAM("drafts.com.sun.star.rendering.Canvas")),
                                                   aArg ),
            UNO_QUERY );

        // now, try to retrieve an XWindow interface from the canvas
        // (this is an implementation detail of the Java interface,
        // which might vanish without notice).
        mxCanvasWindow = Reference< ::com::sun::star::awt::XWindow >(mxCanvas, UNO_QUERY );
    }

    // no factory??? Empty reference, then.
    return mxCanvas;
}
