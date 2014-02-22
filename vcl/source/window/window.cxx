/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <config_features.h>

#include <i18nlangtag/mslangid.hxx>

#include "tools/time.hxx"
#include "tools/debug.hxx"
#include "tools/rc.h"

#include "unotools/fontcfg.hxx"
#include "unotools/confignode.hxx"

#include "vcl/layout.hxx"
#include "vcl/salgtype.hxx"
#include "vcl/event.hxx"
#include "vcl/fixed.hxx"
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
#include "vcl/button.hxx"
#include "vcl/taskpanelist.hxx"
#include "vcl/dialog.hxx"
#include "vcl/unowrap.hxx"
#include "vcl/gdimtf.hxx"
#include "vcl/pdfextoutdevdata.hxx"
#include "vcl/popupmenuwindow.hxx"
#include "vcl/lazydelete.hxx"
#include "vcl/virdev.hxx"
#include "vcl/settings.hxx"


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
#include "brdwin.hxx"
#include "helpwin.hxx"
#include "sallayout.hxx"
#include "dndlcon.hxx"
#include "dndevdis.hxx"

#include "com/sun/star/accessibility/XAccessible.hpp"
#include "com/sun/star/accessibility/AccessibleRole.hpp"
#include "com/sun/star/awt/XWindowPeer.hpp"
#include "com/sun/star/awt/XTopWindow.hpp"
#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/awt/XDisplayConnection.hpp"
#include "com/sun/star/datatransfer/dnd/XDragSource.hpp"
#include "com/sun/star/datatransfer/dnd/XDropTarget.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboard.hpp"
#include "com/sun/star/datatransfer/clipboard/SystemClipboard.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XServiceName.hpp"
#include "com/sun/star/rendering/CanvasFactory.hpp"
#include "com/sun/star/rendering/XCanvas.hpp"
#include "com/sun/star/rendering/XSpriteCanvas.hpp"
#include "comphelper/processfactory.hxx"

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


using ::com::sun::star::awt::XTopWindow;



#define IMPL_PAINT_PAINT            ((sal_uInt16)0x0001)
#define IMPL_PAINT_PAINTALL         ((sal_uInt16)0x0002)
#define IMPL_PAINT_PAINTALLCHILDREN   ((sal_uInt16)0x0004)
#define IMPL_PAINT_PAINTCHILDREN      ((sal_uInt16)0x0008)
#define IMPL_PAINT_ERASE            ((sal_uInt16)0x0010)
#define IMPL_PAINT_CHECKRTL         ((sal_uInt16)0x0020)



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



WindowImpl::WindowImpl( WindowType nType )
{
    maZoom              = Fraction( 1, 1 );
    maWinRegion         = Region(true);
    maWinClipRegion     = Region(true);
    mpWinData           = NULL;         
    mpOverlapData       = NULL;         
    mpFrameData         = NULL;         
    mpFrame             = NULL;         
    mpSysObj            = NULL;
    mpFrameWindow       = NULL;         
    mpOverlapWindow     = NULL;         
    mpBorderWindow      = NULL;         
    mpClientWindow      = NULL;         
    mpParent            = NULL;         
    mpRealParent        = NULL;         
    mpFirstChild        = NULL;         
    mpLastChild         = NULL;         
    mpFirstOverlap      = NULL;         
    mpLastOverlap       = NULL;         
    mpPrev              = NULL;         
    mpNext              = NULL;         
    mpNextOverlap       = NULL;         
    mpLastFocusWindow   = NULL;         
    mpDlgCtrlDownWindow = NULL;         
    mpFirstDel          = NULL;         
    mpUserData          = NULL;         
    mpCursor            = NULL;         
    mpControlFont       = NULL;         
    mpVCLXWindow        = NULL;
    mpAccessibleInfos   = NULL;
    maControlForeground = Color( COL_TRANSPARENT );     
    maControlBackground = Color( COL_TRANSPARENT );     
    mnLeftBorder        = 0;            
    mnTopBorder         = 0;            
    mnRightBorder       = 0;            
    mnBottomBorder      = 0;            
    mnWidthRequest      = -1;           
    mnHeightRequest     = -1;           
    mnX                 = 0;            
    mnY                 = 0;            
    mnAbsScreenX        = 0;            
    mpChildClipRegion   = NULL;         
    mpPaintRegion       = NULL;         
    mnStyle             = 0;            
    mnPrevStyle         = 0;            
    mnExtendedStyle     = 0;            
    mnPrevExtendedStyle = 0;            
    mnType              = nType;        
    mnGetFocusFlags     = 0;            
    mnWaitCount         = 0;            
    mnPaintFlags        = 0;            
    mnParentClipMode    = 0;            
    mnActivateMode      = 0;            
    mnDlgCtrlFlags      = 0;            
    mnLockCount         = 0;            
    meAlwaysInputMode   = AlwaysInputNone; 
    meHalign            = VCL_ALIGN_FILL;
    meValign            = VCL_ALIGN_FILL;
    mePackType          = VCL_PACK_START;
    mnPadding           = 0;
    mnGridHeight        = 1;
    mnGridLeftAttach    = -1;
    mnGridTopAttach     = -1;
    mnGridWidth         = 1;
    mnBorderWidth       = 0;
    mnMarginLeft        = 0;
    mnMarginRight       = 0;
    mnMarginTop         = 0;
    mnMarginBottom      = 0;
    mbFrame             = false;        
    mbBorderWin         = false;        
    mbOverlapWin        = false;        
    mbSysWin            = false;        
    mbDialog            = false;        
    mbDockWin           = false;        
    mbFloatWin          = false;        
    mbPushButton        = false;        
    mbToolBox           = false;      
    mbMenuFloatingWindow= false;      
    mbToolbarFloatingWindow= false;       
    mbSplitter          = false;      
    mbVisible           = false;        
    mbOverlapVisible    = false;        
    mbDisabled          = false;        
    mbInputDisabled     = false;        
    mbDropDisabled      = false;        
    mbNoUpdate          = false;        
    mbNoParentUpdate    = false;        
    mbActive            = false;        
    mbParentActive      = false;        
    mbReallyVisible     = false;        
    mbReallyShown       = false;        
    mbInInitShow        = false;        
    mbChildNotify       = false;        
    mbChildPtrOverwrite = false;        
    mbNoPtrVisible      = false;        
    mbMouseMove         = false;        
    mbPaintFrame        = false;        
    mbInPaint           = false;        
    mbMouseButtonDown   = false;        
    mbMouseButtonUp     = false;        
    mbKeyInput          = false;        
    mbKeyUp             = false;        
    mbCommand           = false;        
    mbDefPos            = true;         
    mbDefSize           = true;         
    mbCallMove          = true;         
    mbCallResize        = true;         
    mbWaitSystemResize  = true;         
    mbInitWinClipRegion = true;         
    mbInitChildRegion   = false;        
    mbWinRegion         = false;        
    mbClipChildren      = false;        
    mbClipSiblings      = false;        
    mbChildTransparent  = false;        
    mbPaintTransparent  = false;        
    mbMouseTransparent  = false;        
    mbDlgCtrlStart      = false;        
    mbFocusVisible      = false;        
    mbUseNativeFocus    = false;
    mbNativeFocusVisible= false;        
    mbInShowFocus       = false;        
    mbInHideFocus       = false;        
    mbTrackVisible      = false;        
    mbControlForeground = false;        
    mbControlBackground = false;        
    mbAlwaysOnTop       = false;        
    mbCompoundControl   = false;        
    mbCompoundControlHasFocus = false;  
    mbPaintDisabled     = false;        
    mbAllResize         = false;        
    mbInDtor            = false;        
    mbExtTextInput      = false;        
    mbInFocusHdl        = false;        
    mbCreatedWithToolkit = false;
    mbSuppressAccessibilityEvents = false; 
    mbDrawSelectionBackground = false;    
    mbIsInTaskPaneList = false;           
    mnNativeBackground  = 0;              
    mbCallHandlersDuringInputDisabled = false; 
    mbHelpTextDynamic = false;          
    mbFakeFocusSet = false; 
    mbHexpand = false;
    mbVexpand = false;
    mbExpand = false;
    mbFill = true;
    mbSecondary = false;
    mbNonHomogeneous = false;
}

WindowImpl::~WindowImpl()
{
    delete mpChildClipRegion;
    delete mpAccessibleInfos;
    delete mpControlFont;
}





void ImplDelData::AttachToWindow( const Window* pWindow )
{
    if( pWindow )
        const_cast<Window*>(pWindow)->ImplAddDel( this );
}




ImplDelData::~ImplDelData()
{
    
    
    if( !mbDel && mpWindow )
    {
        
        const_cast<Window*>(mpWindow)->ImplRemoveDel( this );
        mpWindow = NULL;
    }
}



#ifdef DBG_UTIL
const char* ImplDbgCheckWindow( const void* pObj )
{
    DBG_TESTSOLARMUTEX();

    const Window* pWindow = (Window*)pObj;

    if ( (pWindow->GetType() < WINDOW_FIRST) || (pWindow->GetType() > WINDOW_LAST) )
        return "Window data overwrite";

    
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



bool Window::HasMirroredGraphics() const
{
    const OutputDevice* pOutDev = GetOutDev();
    return pOutDev->OutputDevice::HasMirroredGraphics();
}

void Window::ImplInitAppFontData( Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();
    long nTextHeight = pWindow->GetTextHeight();
    long nTextWidth = pWindow->approximate_char_width() * 8;
    long nSymHeight = nTextHeight*4;
    
    
    
    
    if ( nSymHeight > nTextWidth )
        nTextWidth = nSymHeight;
    else if ( nSymHeight+5 > nTextWidth )
        nTextWidth = nSymHeight+5;
    pSVData->maGDIData.mnAppFontX = nTextWidth * 10 / 8;
    pSVData->maGDIData.mnAppFontY = nTextHeight * 10;

    
    
    if( pSVData->maNWFData.mbNoFocusRects )
    {
        
        
        
        ImplControlValue aControlValue;
        Rectangle aCtrlRegion( Point(), Size( nTextWidth < 10 ? 10 : nTextWidth, nTextHeight < 10 ? 10 : nTextHeight ) );
        Rectangle aBoundingRgn( aCtrlRegion );
        Rectangle aContentRgn( aCtrlRegion );
        if( pWindow->GetNativeControlRegion( CTRL_EDITBOX, PART_ENTIRE_CONTROL, aCtrlRegion,
                                             CTRL_STATE_ENABLED, aControlValue, OUString(),
                                             aBoundingRgn, aContentRgn ) )
        {
            
            
            if( aContentRgn.GetHeight() - nTextHeight > (nTextHeight+4)/4 )
                pSVData->maGDIData.mnAppFontY = (aContentRgn.GetHeight()-4) * 10;
        }
    }


    pSVData->maGDIData.mnRealAppFontX = pSVData->maGDIData.mnAppFontX;
    if ( pSVData->maAppData.mnDialogScaleX )
        pSVData->maGDIData.mnAppFontX += (pSVData->maGDIData.mnAppFontX*pSVData->maAppData.mnDialogScaleX)/100;
}



bool Window::ImplCheckUIFont( const Font& rFont )
{
    if( ImplGetSVData()->maGDIData.mbNativeFontConfig )
        return true;

    
    OUString aTestText;
    static const StandardButtonType aTestButtons[] =
    {
        BUTTON_OK, BUTTON_CANCEL, BUTTON_CLOSE, BUTTON_ABORT,
        BUTTON_YES, BUTTON_NO, BUTTON_MORE, BUTTON_IGNORE,
        BUTTON_RETRY, BUTTON_HELP
    };

    const int nTestButtonCount = SAL_N_ELEMENTS(aTestButtons);
    for( int n = 0; n < nTestButtonCount; ++n )
    {
        OUString aButtonStr = Button::GetStandardText( aTestButtons[n] );
        
        
        const sal_Int32 nLen = aButtonStr.getLength();
        bool bInside = false;
        for( int i = 0; i < nLen; ++i ) {
            const sal_Unicode c = aButtonStr[ i ];
            if( (c == '('))
                bInside = true;
            if( (c == ')'))
                bInside = false;
            if( (c == '~')
            ||  (c == '(') || (c == ')')
            || ((c >= 'A') && (c <= 'Z') && bInside) )
                aButtonStr = aButtonStr.replaceAt( i, 1, " " );
        }
        
        aTestText += aButtonStr;
    }

    const bool bUIFontOk = ( HasGlyphs( rFont, aTestText ) == -1 );
    return bUIFontOk;
}



void Window::ImplUpdateGlobalSettings( AllSettings& rSettings, bool bCallHdl )
{
    StyleSettings aTmpSt( rSettings.GetStyleSettings() );
    aTmpSt.SetHighContrastMode( false );
    rSettings.SetStyleSettings( aTmpSt );
    ImplGetFrame()->UpdateSettings( rSettings );
    
    ImplGetSVData()->maAppData.mnDefaultLayoutBorder = -1;

    

    

    OUString aUserInterfaceFont;
    bool bUseSystemFont = rSettings.GetStyleSettings().GetUseSystemUIFonts();

    
    if( bUseSystemFont )
        bUseSystemFont = ImplCheckUIFont( rSettings.GetStyleSettings().GetAppFont() );

    if ( !bUseSystemFont )
    {
        OutputDevice *pOutDev = GetOutDev();
        pOutDev->ImplInitFontList();
        OUString aConfigFont = utl::DefaultFontConfiguration::get().getUserInterfaceFont( rSettings.GetUILanguageTag() );
        sal_Int32 nIndex = 0;
        while( nIndex != -1 )
        {
            OUString aName( aConfigFont.getToken( 0, ';', nIndex ) );
            if ( !aName.isEmpty() && mpWindowImpl->mpFrameData->mpFontList->FindFontFamily( aName ) )
            {
                aUserInterfaceFont = aConfigFont;
                break;
            }
        }

        if ( aUserInterfaceFont.isEmpty() )
        {
            OUString aFallbackFont ("Andale Sans UI" );
            if ( mpWindowImpl->mpFrameData->mpFontList->FindFontFamily( aFallbackFont ) )
                aUserInterfaceFont = aFallbackFont;
        }
    }

    if ( !bUseSystemFont && !aUserInterfaceFont.isEmpty() )
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
    
    
    
    
    int maxFontheight = 9; 
    if( GetDesktopRectPixel().getHeight() > 600 )
        maxFontheight = (int) ((( 8.0 * (double) GetDesktopRectPixel().getHeight()) / 600.0) + 1.5);

    Font aFont = aStyleSettings.GetMenuFont();
    int defFontheight = aFont.GetHeight();
    if( defFontheight > maxFontheight )
        defFontheight = maxFontheight;

    
    
    
    bool bBrokenLangFontHeight = MsLangId::isCJK(Application::GetSettings().GetUILanguageTag().getLanguageType());
    if (bBrokenLangFontHeight)
        defFontheight = std::max(9, defFontheight);

    
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

    
    if( aStyleSettings.GetWindowColor().IsDark() )
        aStyleSettings.SetWorkspaceGradient( Wallpaper( Color( COL_BLACK ) ) );
    else
    {
        Gradient aGrad( GradientStyle_LINEAR, DEFAULT_WORKSPACE_GRADIENT_START_COLOR, DEFAULT_WORKSPACE_GRADIENT_END_COLOR );
        aStyleSettings.SetWorkspaceGradient( Wallpaper( aGrad ) );
    }

    rSettings.SetStyleSettings( aStyleSettings );

    bool bForceHCMode = false;

    
    
    if( !rSettings.GetStyleSettings().GetHighContrastMode() )
    {
        bool bTmp = false, bAutoHCMode = true;
        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithComponentContext(
            comphelper::getProcessComponentContext(),
            OUString("org.openoffice.Office.Common/Accessibility") );    
        if ( aNode.isValid() )
        {
            ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString("AutoDetectSystemHC") );
            if( aValue >>= bTmp )
                bAutoHCMode = bTmp;
        }
        if( bAutoHCMode )
        {
            if( rSettings.GetStyleSettings().GetFaceColor().IsDark() ||
                rSettings.GetStyleSettings().GetWindowColor().IsDark() )
                bForceHCMode = true;
        }
    }

    static const char* pEnvHC = getenv( "SAL_FORCE_HC" );
    if( pEnvHC && *pEnvHC )
        bForceHCMode = true;

    if( bForceHCMode )
    {
        aStyleSettings = rSettings.GetStyleSettings();
        aStyleSettings.SetHighContrastMode( true );
        rSettings.SetStyleSettings( aStyleSettings );
    }

#if defined(DBG_UTIL)
    
    
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
        GetpApp()->OverrideSystemSettings( rSettings );
}



MouseEvent ImplTranslateMouseEvent( const MouseEvent& rE, Window* pSource, Window* pDest )
{
    Point aPos = pSource->OutputToScreenPixel( rE.GetPosPixel() );
    aPos = pDest->ScreenToOutputPixel( aPos );
    return MouseEvent( aPos, rE.GetClicks(), rE.GetMode(), rE.GetButtons(), rE.GetModifier() );
}



CommandEvent ImplTranslateCommandEvent( const CommandEvent& rCEvt, Window* pSource, Window* pDest )
{
    if ( !rCEvt.IsMouseEvent() )
        return rCEvt;

    Point aPos = pSource->OutputToScreenPixel( rCEvt.GetMousePosPixel() );
    aPos = pDest->ScreenToOutputPixel( aPos );
    return CommandEvent( aPos, rCEvt.GetCommand(), rCEvt.IsMouseEvent(), rCEvt.GetData() );
}



void Window::ImplInitWindowData( WindowType nType )
{
    
    
    
    mpOutputDevice = (OutputDevice*)this;


    mpWindowImpl = new WindowImpl( nType );

    meOutDevType        = OUTDEV_WINDOW;


    mbEnableRTL         = Application::GetSettings().GetLayoutRTL();         
}



void Window::ImplInit( Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData )
{
    DBG_ASSERT( mpWindowImpl->mbFrame || pParent, "Window::Window(): pParent == NULL" );

    ImplSVData* pSVData = ImplGetSVData();
    Window*     pRealParent = pParent;

    
    if ( !mpWindowImpl->mbOverlapWin && pParent && (pParent->GetStyle() & WB_3DLOOK) )
        nStyle |= WB_3DLOOK;

    
    if ( !mpWindowImpl->mbFrame && !mpWindowImpl->mbBorderWin && !mpWindowImpl->mpBorderWindow
         && (nStyle & (WB_BORDER | WB_SYSTEMCHILDWINDOW) ) )
    {
        sal_uInt16 nBorderTypeStyle = 0;
        if( (nStyle & WB_SYSTEMCHILDWINDOW) )
        {
            
            
            
            
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
        mpWindowImpl->mbOverlapWin  = true;
        mpWindowImpl->mbFrame = true;
    }

    
    ImplInsertWindow( pParent );
    mpWindowImpl->mnStyle = nStyle;

    
    if ( mpWindowImpl->mbOverlapWin )
    {
        mpWindowImpl->mpOverlapData                   = new ImplOverlapData;
        mpWindowImpl->mpOverlapData->mpSaveBackDev    = NULL;
        mpWindowImpl->mpOverlapData->mpSaveBackRgn    = NULL;
        mpWindowImpl->mpOverlapData->mpNextBackWin    = NULL;
        mpWindowImpl->mpOverlapData->mnSaveBackSize   = 0;
        mpWindowImpl->mpOverlapData->mbSaveBack       = false;
        mpWindowImpl->mpOverlapData->mnTopLevel       = 1;
    }

    if( pParent && ! mpWindowImpl->mbFrame )
        mbEnableRTL = Application::GetSettings().GetLayoutRTL();

    
    if ( mpWindowImpl->mbFrame )
    {
        
        sal_uLong nFrameStyle = 0;

        if ( nStyle & WB_MOVEABLE )
            nFrameStyle |= SAL_FRAME_STYLE_MOVEABLE;
        if ( nStyle & WB_SIZEABLE )
            nFrameStyle |= SAL_FRAME_STYLE_SIZEABLE;
        if ( nStyle & WB_CLOSEABLE )
            nFrameStyle |= SAL_FRAME_STYLE_CLOSEABLE;
        if ( nStyle & WB_APP )
            nFrameStyle |= SAL_FRAME_STYLE_DEFAULT;
        
        if( 
            ( !(nFrameStyle & ~SAL_FRAME_STYLE_CLOSEABLE) &&
            ( mpWindowImpl->mbFloatWin || ((GetType() == WINDOW_BORDERWINDOW) && ((ImplBorderWindow*)this)->mbFloatWindow) || (nStyle & WB_SYSTEMFLOATWIN) ) ) ||
            
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
            
            throw ::com::sun::star::uno::RuntimeException(
                OUString( "Could not create system window!"  ),
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );
        }

        pFrame->SetCallback( this, ImplWindowFrameProc );

        
        mpWindowImpl->mpFrameData     = new ImplFrameData;
        mpWindowImpl->mpFrame         = pFrame;
        mpWindowImpl->mpFrameWindow   = this;
        mpWindowImpl->mpOverlapWindow = this;

        
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
        mpWindowImpl->mpFrameData->mbHasFocus         = false;
        mpWindowImpl->mpFrameData->mbInMouseMove      = false;
        mpWindowImpl->mpFrameData->mbMouseIn          = false;
        mpWindowImpl->mpFrameData->mbStartDragCalled  = false;
        mpWindowImpl->mpFrameData->mbNeedSysWindow    = false;
        mpWindowImpl->mpFrameData->mbMinimized        = false;
        mpWindowImpl->mpFrameData->mbStartFocusState  = false;
        mpWindowImpl->mpFrameData->mbInSysObjFocusHdl = false;
        mpWindowImpl->mpFrameData->mbInSysObjToTopHdl = false;
        mpWindowImpl->mpFrameData->mbSysObjFocus      = false;
        mpWindowImpl->mpFrameData->maPaintTimer.SetTimeout( 30 );
        mpWindowImpl->mpFrameData->maPaintTimer.SetTimeoutHdl( LINK( this, Window, ImplHandlePaintHdl ) );
        mpWindowImpl->mpFrameData->maResizeTimer.SetTimeout( 50 );
        mpWindowImpl->mpFrameData->maResizeTimer.SetTimeoutHdl( LINK( this, Window, ImplHandleResizeTimerHdl ) );
        mpWindowImpl->mpFrameData->mbInternalDragGestureRecognizer = false;

        if ( pRealParent && IsTopWindow() )
        {
            ImplWinData* pParentWinData = pRealParent->ImplGetWinData();
            pParentWinData->maTopWindowChildren.push_back( this );
        }
    }

    
    mpWindowImpl->mpRealParent = pRealParent;

    
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
            OutputDevice *pOutDev = GetOutDev();
            if ( pOutDev->ImplGetGraphics() )
            {
                mpGraphics->GetResolution( mpWindowImpl->mpFrameData->mnDPIX, mpWindowImpl->mpFrameData->mnDPIY );
            }
        }

        
        
        if( nStyle & WB_OWNERDRAWDECORATION )
            ImplGetOwnerDrawList().push_back( this );

        
        
        if ( !pSVData->maAppData.mbSettingsInit &&
             ! (nStyle & (WB_INTROWIN|WB_DEFAULTWIN))
             )
        {
            
            ImplUpdateGlobalSettings( *pSVData->maAppData.mpSettings );
            OutputDevice::SetSettings( *pSVData->maAppData.mpSettings );
            pSVData->maAppData.mbSettingsInit = true;
        }

        
        
        
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

    
    mnDPIScaleFactor = std::max((sal_Int32)1, (mpWindowImpl->mpFrameData->mnDPIY + 48) / 96);

    const StyleSettings& rStyleSettings = mxSettings->GetStyleSettings();
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

    
    if ( mpWindowImpl->mbFrame && !pSVData->maGDIData.mnAppFontX && ! (nStyle & (WB_INTROWIN|WB_DEFAULTWIN)) )
        ImplInitAppFontData( this );

    if ( GetAccessibleParentWindow()  && GetParent() != Application::GetDefDialogParent() )
        GetAccessibleParentWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_CHILDCREATED, this );
}



void Window::ImplSetFrameParent( const Window* pParent )
{
    Window* pFrameWindow = ImplGetSVData()->maWinData.mpFirstFrame;
    while( pFrameWindow )
    {
        
        
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



void Window::ImplInsertWindow( Window* pParent )
{
    mpWindowImpl->mpParent            = pParent;
    mpWindowImpl->mpRealParent        = pParent;

    if ( pParent && !mpWindowImpl->mbFrame )
    {
        
        Window* pFrameParent = pParent->mpWindowImpl->mpFrameWindow;
        mpWindowImpl->mpFrameData     = pFrameParent->mpWindowImpl->mpFrameData;
        mpWindowImpl->mpFrame         = pFrameParent->mpWindowImpl->mpFrame;
        mpWindowImpl->mpFrameWindow   = pFrameParent;
        mpWindowImpl->mbFrame         = false;

        
        if ( ImplIsOverlapWindow() )
        {
            Window* pFirstOverlapParent = pParent;
            while ( !pFirstOverlapParent->ImplIsOverlapWindow() )
                pFirstOverlapParent = pFirstOverlapParent->ImplGetParent();
            mpWindowImpl->mpOverlapWindow = pFirstOverlapParent;

            mpWindowImpl->mpNextOverlap = mpWindowImpl->mpFrameData->mpFirstOverlap;
            mpWindowImpl->mpFrameData->mpFirstOverlap = this;

            
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



void Window::ImplRemoveWindow( bool bRemoveFrameData )
{
    
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
            else if ( mpWindowImpl->mpParent )
                mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else if ( mpWindowImpl->mpParent )
                mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = mpWindowImpl->mpPrev;
        }

        mpWindowImpl->mpPrev = NULL;
        mpWindowImpl->mpNext = NULL;
    }

    if ( bRemoveFrameData )
    {
        
        OutputDevice *pOutDev = GetOutDev();
        pOutDev->ImplReleaseGraphics();
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

    if (pSource == this) 
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



void Window::ImplCallResize()
{
    mpWindowImpl->mbCallResize = false;

    if( GetBackground().IsGradient() )
        Invalidate();

    Resize();

    
    
    ImplCallEventListeners( VCLEVENT_WINDOW_RESIZE );
}



void Window::ImplCallMove()
{
    mpWindowImpl->mbCallMove = false;

    if( mpWindowImpl->mbFrame )
    {
        
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



static OString ImplAutoHelpID( ResMgr* pResMgr )
{
    OString aRet;

    if( pResMgr && Application::IsAutoHelpIdEnabled() )
        aRet = pResMgr->GetAutoHelpId();

    return aRet;
}



WinBits Window::ImplInitRes( const ResId& rResId )
{
    GetRes( rResId );

    char* pRes = (char*)GetClassRes();
    pRes += 8;
    sal_uInt32 nStyle = (sal_uInt32)GetLongRes( (void*)pRes );
    rResId.SetWinBits( nStyle );
    return nStyle;
}



WindowResHeader Window::ImplLoadResHeader( const ResId& rResId )
{
    WindowResHeader aHeader;

    aHeader.nObjMask = ReadLongRes();

    
    
    
    
    aHeader.aHelpId = ImplAutoHelpID( rResId.GetResMgr() );

    
    aHeader.nRSStyle = ReadLongRes();
    
    ReadLongRes();

    if( aHeader.nObjMask & WINDOW_HELPID )
        aHeader.aHelpId = ReadByteStringRes();

    return aHeader;
}

void Window::ImplLoadRes( const ResId& rResId )
{
    WindowResHeader aHeader = ImplLoadResHeader( rResId );

    SetHelpId( aHeader.aHelpId );

    sal_uLong nObjMask = aHeader.nObjMask;

    bool  bPos  = false;
    bool  bSize = false;
    Point aPos;
    Size  aSize;

    if ( nObjMask & (WINDOW_XYMAPMODE | WINDOW_X | WINDOW_Y) )
    {
        
        MapUnit ePosMap = MAP_PIXEL;

        bPos = true;

        if ( nObjMask & WINDOW_XYMAPMODE )
            ePosMap = (MapUnit)ReadLongRes();
        if ( nObjMask & WINDOW_X )
            aPos.X() = ImplLogicUnitToPixelX( ReadLongRes(), ePosMap );
        if ( nObjMask & WINDOW_Y )
            aPos.Y() = ImplLogicUnitToPixelY( ReadLongRes(), ePosMap );
    }

    if ( nObjMask & (WINDOW_WHMAPMODE | WINDOW_WIDTH | WINDOW_HEIGHT) )
    {
        
        MapUnit eSizeMap = MAP_PIXEL;

        bSize = true;

        if ( nObjMask & WINDOW_WHMAPMODE )
            eSizeMap = (MapUnit)ReadLongRes();
        if ( nObjMask & WINDOW_WIDTH )
            aSize.Width() = ImplLogicUnitToPixelX( ReadLongRes(), eSizeMap );
        if ( nObjMask & WINDOW_HEIGHT )
            aSize.Height() = ImplLogicUnitToPixelY( ReadLongRes(), eSizeMap );
    }

    sal_uLong nRSStyle = aHeader.nRSStyle;

    
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
        Enable( false );

    if ( nObjMask & WINDOW_TEXT )
        SetText( ReadStringRes() );
    if ( nObjMask & WINDOW_HELPTEXT )
    {
        SetHelpText( ReadStringRes() );
        mpWindowImpl->mbHelpTextDynamic = true;
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



ImplWinData* Window::ImplGetWinData() const
{
    if ( !mpWindowImpl->mpWinData )
    {
        static const char* pNoNWF = getenv( "SAL_NO_NWF" );

        ((Window*)this)->mpWindowImpl->mpWinData = new ImplWinData;
        mpWindowImpl->mpWinData->mpExtOldText     = NULL;
        mpWindowImpl->mpWinData->mpExtOldAttrAry  = NULL;
        mpWindowImpl->mpWinData->mpCursorRect     = NULL;
        mpWindowImpl->mpWinData->mnCursorExtWidth = 0;
        mpWindowImpl->mpWinData->mpCompositionCharRects = NULL;
        mpWindowImpl->mpWinData->mnCompositionCharRects = 0;
        mpWindowImpl->mpWinData->mpFocusRect      = NULL;
        mpWindowImpl->mpWinData->mpTrackRect      = NULL;
        mpWindowImpl->mpWinData->mnTrackFlags     = 0;
        mpWindowImpl->mpWinData->mnIsTopWindow  = (sal_uInt16) ~0;  
        mpWindowImpl->mpWinData->mbMouseOver      = false;
        mpWindowImpl->mpWinData->mbEnableNativeWidget = (pNoNWF && *pNoNWF) ? sal_False : sal_True; 
   }

    return mpWindowImpl->mpWinData;
}



SalGraphics* Window::ImplGetFrameGraphics() const
{
    if ( mpWindowImpl->mpFrameWindow->mpGraphics )
    {
        mpWindowImpl->mpFrameWindow->mbInitClipRegion = true;
    }
    else
    {
        OutputDevice *pFrameWinOutDev = mpWindowImpl->mpFrameWindow;
        pFrameWinOutDev->ImplGetGraphics();
    }
    mpWindowImpl->mpFrameWindow->mpGraphics->ResetClipRegion();
    return mpWindowImpl->mpFrameWindow->mpGraphics;
}



Window* Window::ImplFindWindow( const Point& rFramePos )
{
    Window* pTempWindow;
    Window* pFindWindow;

    
    pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        pFindWindow = pTempWindow->ImplFindWindow( rFramePos );
        if ( pFindWindow )
            return pFindWindow;
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }

    
    if ( !mpWindowImpl->mbVisible )
        return NULL;

    sal_uInt16 nHitTest = ImplHitTest( rFramePos );
    if ( nHitTest & WINDOW_HITTEST_INSIDE )
    {
        
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



sal_uInt16 Window::ImplHitTest( const Point& rFramePos )
{
    Point aFramePos( rFramePos );
    if( ImplIsAntiparallel() )
    {
        
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aFramePos );
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



bool Window::ImplIsRealParentPath( const Window* pWindow ) const
{
    pWindow = pWindow->GetParent();
    while ( pWindow )
    {
        if ( pWindow == this )
            return true;
        pWindow = pWindow->GetParent();
    }

    return false;
}



bool Window::ImplIsChild( const Window* pWindow, bool bSystemWindow ) const
{
    do
    {
        if ( !bSystemWindow && pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();

        if ( pWindow == this )
            return true;
    }
    while ( pWindow );

    return false;
}



bool Window::ImplIsWindowOrChild( const Window* pWindow, bool bSystemWindow ) const
{
    if ( this == pWindow )
        return true;
    return ImplIsChild( pWindow, bSystemWindow );
}



int Window::ImplTestMousePointerSet()
{
    
    if ( IsMouseCaptured() )
        return sal_True;

    
    Rectangle aClientRect( Point( 0, 0 ), GetOutputSizePixel() );
    if ( aClientRect.IsInside( GetPointerPosPixel() ) )
        return sal_True;

    return sal_False;
}



PointerStyle Window::ImplGetMousePointer() const
{
    PointerStyle    ePointerStyle;
    bool            bWait = false;

    if ( IsEnabled() && IsInputEnabled() && ! IsInModalMode() )
        ePointerStyle = GetPointer().GetStyle();
    else
        ePointerStyle = POINTER_ARROW;

    const Window* pWindow = this;
    do
    {
        
        
        if ( pWindow->mpWindowImpl->mbNoPtrVisible )
            return POINTER_NULL;

        if ( !bWait )
        {
            if ( pWindow->mpWindowImpl->mnWaitCount )
            {
                ePointerStyle = POINTER_WAIT;
                bWait = true;
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



void Window::ImplResetReallyVisible()
{
    bool bBecameReallyInvisible = mpWindowImpl->mbReallyVisible;

    mbDevOutput     = false;
    mpWindowImpl->mbReallyVisible = false;
    mpWindowImpl->mbReallyShown   = false;

    
    
    
    if( bBecameReallyInvisible && ImplIsAccessibleCandidate() )
        ImplCallEventListeners( VCLEVENT_WINDOW_HIDE, this );
        
        

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



void Window::ImplSetReallyVisible()
{
    
    
    
    if( !mpWindowImpl->mbReallyShown )
        ImplCallInitShow();

    bool bBecameReallyVisible = !mpWindowImpl->mbReallyVisible;

    mbDevOutput     = true;
    mpWindowImpl->mbReallyVisible = true;
    mpWindowImpl->mbReallyShown   = true;

    
    
    
    
    if( bBecameReallyVisible && ImplIsAccessibleCandidate() )
        ImplCallEventListeners( VCLEVENT_WINDOW_SHOW, this );
        
        

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



void Window::ImplCallInitShow()
{
    mpWindowImpl->mbReallyShown   = true;
    mpWindowImpl->mbInInitShow    = true;
    StateChanged( STATE_CHANGE_INITSHOW );
    mpWindowImpl->mbInInitShow    = false;

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



void Window::ImplAddDel( ImplDelData* pDel ) 
{
    DBG_ASSERT( !pDel->mpWindow, "Window::ImplAddDel(): cannot add ImplDelData twice !" );
    if( !pDel->mpWindow )
    {
        pDel->mpWindow = this;  
        pDel->mpNext = mpWindowImpl->mpFirstDel;
        mpWindowImpl->mpFirstDel = pDel;
    }
}



void Window::ImplRemoveDel( ImplDelData* pDel ) 
{
    pDel->mpWindow = NULL;      
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



void Window::ImplInitResolutionSettings()
{
    
    if ( mpWindowImpl->mbFrame )
    {
        const StyleSettings& rStyleSettings = mxSettings->GetStyleSettings();
        sal_uInt16 nScreenZoom = rStyleSettings.GetScreenZoom();
        mnDPIX = (mpWindowImpl->mpFrameData->mnDPIX*nScreenZoom)/100;
        mnDPIY = (mpWindowImpl->mpFrameData->mnDPIY*nScreenZoom)/100;

        
        mnDPIScaleFactor = std::max((sal_Int32)1, (mpWindowImpl->mpFrameData->mnDPIY + 48) / 96);

        SetPointFont( rStyleSettings.GetAppFont() );
    }
    else if ( mpWindowImpl->mpParent )
    {
        mnDPIX  = mpWindowImpl->mpParent->mnDPIX;
        mnDPIY  = mpWindowImpl->mpParent->mnDPIY;
        mnDPIScaleFactor = mpWindowImpl->mpParent->mnDPIScaleFactor;
    }

    
    
    if ( IsMapMode() )
    {
        MapMode aMapMode = GetMapMode();
        SetMapMode();
        SetMapMode( aMapMode );
    }
}



void Window::ImplPointToLogic( Font& rFont ) const
{
    Size    aSize = rFont.GetSize();
    sal_uInt16  nScreenFontZoom = mxSettings->GetStyleSettings().GetScreenFontZoom();

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



void Window::ImplLogicToPoint( Font& rFont ) const
{
    Size    aSize = rFont.GetSize();
    sal_uInt16  nScreenFontZoom = mxSettings->GetStyleSettings().GetScreenFontZoom();

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



bool Window::ImplSysObjClip( const Region* pOldRegion )
{
    bool bUpdate = true;

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

                    
                    RectangleVector aRectangles;
                    aRegion.GetRegionRectangles(aRectangles);
                    mpWindowImpl->mpSysObj->BeginSetClipRegion(aRectangles.size());

                    for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
                    {
                        mpWindowImpl->mpSysObj->UnionClipRegion(
                            aRectIter->Left(),
                            aRectIter->Top(),
                            aRectIter->GetWidth(),   
                            aRectIter->GetHeight()); 
                    }

                    mpWindowImpl->mpSysObj->EndSetClipRegion();

                    
                    
                    
                    
                    
                    
                    
                    //
                    
                    
                    
                    
                    
                    
                    
                    
                    
                }
            }
            else
                bVisibleState = false;
        }

        
        mpWindowImpl->mpSysObj->Show( bVisibleState );
    }

    return bUpdate;
}



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



void Window::ImplUpdateSysObjClip()
{
    if ( !ImplIsOverlapWindow() )
    {
        ImplUpdateSysObjChildrenClip();

        
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



bool Window::ImplSetClipFlagChildren( bool bSysObjOnlySmaller )
{
    bool bUpdate = true;
    if ( mpWindowImpl->mpSysObj )
    {
        Region* pOldRegion = NULL;
        if ( bSysObjOnlySmaller && !mpWindowImpl->mbInitWinClipRegion )
            pOldRegion = new Region( mpWindowImpl->maWinClipRegion );

        mbInitClipRegion = true;
        mpWindowImpl->mbInitWinClipRegion = true;

        Window* pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            if ( !pWindow->ImplSetClipFlagChildren( bSysObjOnlySmaller ) )
                bUpdate = false;
            pWindow = pWindow->mpWindowImpl->mpNext;
        }

        if ( !ImplSysObjClip( pOldRegion ) )
        {
            mbInitClipRegion = true;
            mpWindowImpl->mbInitWinClipRegion = true;
            bUpdate = false;
        }

        delete pOldRegion;
    }
    else
    {
    mbInitClipRegion = true;
    mpWindowImpl->mbInitWinClipRegion = true;

    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagChildren( bSysObjOnlySmaller ) )
            bUpdate = false;
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
    }
    return bUpdate;
}



bool Window::ImplSetClipFlagOverlapWindows( bool bSysObjOnlySmaller )
{
    bool bUpdate = ImplSetClipFlagChildren( bSysObjOnlySmaller );

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller ) )
            bUpdate = false;
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    return bUpdate;
}



bool Window::ImplSetClipFlag( bool bSysObjOnlySmaller )
{
    if ( !ImplIsOverlapWindow() )
    {
        bool bUpdate = ImplSetClipFlagChildren( bSysObjOnlySmaller );

        Window* pParent = ImplGetParent();
        if ( pParent &&
             ((pParent->GetStyle() & WB_CLIPCHILDREN) || (mpWindowImpl->mnParentClipMode & PARENTCLIPMODE_CLIP)) )
        {
            pParent->mbInitClipRegion = true;
            pParent->mpWindowImpl->mbInitChildRegion = true;
        }

        
        if ( mpWindowImpl->mbClipSiblings )
        {
            Window* pWindow = mpWindowImpl->mpNext;
            while ( pWindow )
            {
                if ( !pWindow->ImplSetClipFlagChildren( bSysObjOnlySmaller ) )
                    bUpdate = false;
                pWindow = pWindow->mpWindowImpl->mpNext;
            }
        }

        return bUpdate;
    }
    else
        return mpWindowImpl->mpFrameWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller );
}



void Window::ImplIntersectWindowClipRegion( Region& rRegion )
{
    if ( mpWindowImpl->mbInitWinClipRegion )
        ImplInitWinClipRegion();

    rRegion.Intersect( mpWindowImpl->maWinClipRegion );
}



void Window::ImplIntersectWindowRegion( Region& rRegion )
{
    rRegion.Intersect( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                  Size( mnOutWidth, mnOutHeight ) ) );
    if ( mpWindowImpl->mbWinRegion )
        rRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
}



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



void Window::ImplExcludeOverlapWindows2( Region& rRegion )
{
    if ( mpWindowImpl->mbReallyVisible )
        ImplExcludeWindowRegion( rRegion );

    ImplExcludeOverlapWindows( rRegion );
}



void Window::ImplClipBoundaries( Region& rRegion, bool bThis, bool bOverlaps )
{
    if ( bThis )
        ImplIntersectWindowClipRegion( rRegion );
    else if ( ImplIsOverlapWindow() )
    {
        
        if ( !mpWindowImpl->mbFrame )
            rRegion.Intersect( Rectangle( Point( 0, 0 ), Size( mpWindowImpl->mpFrameWindow->mnOutWidth, mpWindowImpl->mpFrameWindow->mnOutHeight ) ) );

        if ( bOverlaps && !rRegion.IsEmpty() )
        {
            
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

            
            ImplExcludeOverlapWindows( rRegion );
        }
    }
    else
        ImplGetParent()->ImplIntersectWindowClipRegion( rRegion );
}



bool Window::ImplClipChildren( Region& rRegion )
{
    bool    bOtherClip = false;
    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
        {
            
            sal_uInt16 nClipMode = pWindow->GetParentClipMode();
            if ( !(nClipMode & PARENTCLIPMODE_NOCLIP) &&
                 ((nClipMode & PARENTCLIPMODE_CLIP) || (GetStyle() & WB_CLIPCHILDREN)) )
                pWindow->ImplExcludeWindowRegion( rRegion );
            else
                bOtherClip = true;
        }

        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    return bOtherClip;
}



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



void Window::ImplInitWinClipRegion()
{
    
    mpWindowImpl->maWinClipRegion = Rectangle( Point( mnOutOffX, mnOutOffY ),
                                 Size( mnOutWidth, mnOutHeight ) );
    if ( mpWindowImpl->mbWinRegion )
        mpWindowImpl->maWinClipRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );

    
    if ( mpWindowImpl->mbClipSiblings && !ImplIsOverlapWindow() )
        ImplClipSiblings( mpWindowImpl->maWinClipRegion );

    
    ImplClipBoundaries( mpWindowImpl->maWinClipRegion, false, true );

    
    if ( (GetStyle() & WB_CLIPCHILDREN) || mpWindowImpl->mbClipChildren )
        mpWindowImpl->mbInitChildRegion = true;

    mpWindowImpl->mbInitWinClipRegion = false;
}



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

    mpWindowImpl->mbInitChildRegion = false;
}



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



void Window::ImplCalcOverlapRegionOverlaps( const Region& rInterRegion, Region& rRegion )
{
    
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

    
    if ( !ImplIsOverlapWindow() )
        mpWindowImpl->mpOverlapWindow->ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
    else
        ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
}



void Window::ImplCalcOverlapRegion( const Rectangle& rSourceRect, Region& rRegion,
                                    bool bChildren, bool bParent, bool bSiblings )
{
    Region  aRegion( rSourceRect );
    if ( mpWindowImpl->mbWinRegion )
        rRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
    Region  aTempRegion;
    Window* pWindow;

    ImplCalcOverlapRegionOverlaps( aRegion, rRegion );

    
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
        if ( pWindow && !pWindow->mpWindowImpl->mbFrame )
        {
            aTempRegion = aRegion;
            aTempRegion.Exclude( Rectangle( Point( 0, 0 ), Size( mpWindowImpl->mpFrameWindow->mnOutWidth, mpWindowImpl->mpFrameWindow->mnOutHeight ) ) );
            rRegion.Union( aTempRegion );
        }
    }

    
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



void Window::ImplCallPaint( const Region* pRegion, sal_uInt16 nPaintFlags )
{
    Exception aException;
    bool bExceptionCaught(false);

    
    
    PrePaint();

    mpWindowImpl->mbPaintFrame = false;

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

            mbInitClipRegion = true;
            mpWindowImpl->mbInPaint = true;

            
            Region      aPaintRegion( mpWindowImpl->maInvalidateRegion );
            Rectangle   aPaintRect = aPaintRegion.GetBoundRect();

            
            if( ImplIsAntiparallel() )
            {
                const OutputDevice *pOutDev = GetOutDev();
                pOutDev->ReMirror( aPaintRect );
                pOutDev->ReMirror( aPaintRegion );
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

            
            if( mpWindowImpl->mbDrawSelectionBackground )
                aSelectionRect = aPaintRect;

            
            
            
            
            try
            {
                Paint( aPaintRect );
            }
            catch(Exception& rException)
            {
                aException = rException;
                bExceptionCaught = true;
            }

            if ( mpWindowImpl->mpWinData )
            {
                if ( mpWindowImpl->mbFocusVisible )
                    ImplInvertFocus( *(mpWindowImpl->mpWinData->mpFocusRect) );
            }
            mpWindowImpl->mbInPaint = false;
            mbInitClipRegion = true;
            mpWindowImpl->mpPaintRegion = NULL;
            if ( mpWindowImpl->mpCursor )
                mpWindowImpl->mpCursor->ImplResume( bRestoreCursor );
        }
    }
    else
        mpWindowImpl->mnPaintFlags = 0;

    if ( nPaintFlags & (IMPL_PAINT_PAINTALLCHILDREN | IMPL_PAINT_PAINTCHILDREN) )
    {
        
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

    
    if( !aSelectionRect.IsEmpty() )
        DrawSelectionBackground( aSelectionRect, 3, false, true, false );

    delete pChildRegion;

    if(bExceptionCaught)
    {
        throw(aException);
    }
}



void Window::ImplCallOverlapPaint()
{
    
    Window* pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->mpWindowImpl->mbReallyVisible )
            pTempWindow->ImplCallOverlapPaint();
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }

    
    if ( mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDREN) )
    {
        
        
        ImplCallPaint( NULL, mpWindowImpl->mnPaintFlags /*| IMPL_PAINT_CHECKRTL */);
    }
}



void Window::ImplPostPaint()
{
    if ( !mpWindowImpl->mpFrameData->maPaintTimer.IsActive() )
        mpWindowImpl->mpFrameData->maPaintTimer.Start();
}



IMPL_LINK_NOARG(Window, ImplHandlePaintHdl)
{
    
    if (IsDialog() && static_cast<const Dialog*>(this)->hasPendingLayout())
    {
        mpWindowImpl->mpFrameData->maPaintTimer.Start();
        return 0;
    }

    
    if( mpWindowImpl->mbFrame && mpWindowImpl->mpFrameData->maResizeTimer.IsActive() )
        mpWindowImpl->mpFrameData->maPaintTimer.Start();
    else if ( mpWindowImpl->mbReallyVisible )
        ImplCallOverlapPaint();
    return 0;
}



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



void Window::ImplInvalidateFrameRegion( const Region* pRegion, sal_uInt16 nFlags )
{
    
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

    
    mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINT;
    if ( nFlags & INVALIDATE_CHILDREN )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTALLCHILDREN;
    if ( !(nFlags & INVALIDATE_NOERASE) )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_ERASE;
    if ( !pRegion )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTALL;

    
    if ( !(mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL) )
        mpWindowImpl->maInvalidateRegion.Union( *pRegion );

    
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
                
                pChildRegion = ImplGetWinChildClipRegion();
            else
                
                pChildRegion = &mpWindowImpl->maInvalidateRegion;

            nFlags |= INVALIDATE_CHILDREN;  
            nFlags &= ~INVALIDATE_NOERASE;  
            pParent->ImplInvalidateFrameRegion( pChildRegion, nFlags );
        }
    }
    ImplPostPaint();
}



void Window::ImplInvalidateOverlapFrameRegion( const Region& rRegion )
{
    Region aRegion = rRegion;

    ImplClipBoundaries( aRegion, true, true );
    if ( !aRegion.IsEmpty() )
        ImplInvalidateFrameRegion( &aRegion, INVALIDATE_CHILDREN );

    
    Window* pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->IsVisible() )
            pTempWindow->ImplInvalidateOverlapFrameRegion( rRegion );

        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }
}



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



void Window::ImplInvalidate( const Region* pRegion, sal_uInt16 nFlags )
{

    
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    
    bool bInvalidateAll = !pRegion;

    
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
                bInvalidateAll = false;
                break;
            }

            if ( pTempWindow->ImplIsOverlapWindow() )
                break;

            pTempWindow = pTempWindow->ImplGetParent();
        }
    }

    
    sal_uInt16 nOrgFlags = nFlags;
    if ( !(nFlags & (INVALIDATE_CHILDREN | INVALIDATE_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= INVALIDATE_NOCHILDREN;
        else
            nFlags |= INVALIDATE_CHILDREN;
    }
    if ( (nFlags & INVALIDATE_NOCHILDREN) && mpWindowImpl->mpFirstChild )
        bInvalidateAll = false;
    if ( bInvalidateAll )
        ImplInvalidateFrameRegion( NULL, nFlags );
    else
    {
        Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aRegion( aRect );
        if ( pRegion )
        {
            
            if ( ImplIsAntiparallel() )
            {
                const OutputDevice *pOutDev = GetOutDev();

                Region aRgn( *pRegion );
                pOutDev->ReMirror( aRgn );
                aRegion.Intersect( aRgn );
            }
            else
                aRegion.Intersect( *pRegion );
        }
        ImplClipBoundaries( aRegion, true, true );
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
            ImplInvalidateFrameRegion( &aRegion, nFlags );  
    }

    if ( nFlags & INVALIDATE_UPDATE )
        pOpaqueWindow->Update();        
}



void Window::ImplMoveInvalidateRegion( const Rectangle& rRect,
                                       long nHorzScroll, long nVertScroll,
                                       bool bChildren )
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
            pWindow->ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, true );
            pWindow = pWindow->mpWindowImpl->mpNext;
        }
    }
}



void Window::ImplMoveAllInvalidateRegions( const Rectangle& rRect,
                                           long nHorzScroll, long nVertScroll,
                                           bool bChildren )
{
    
    ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, bChildren );
    
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



void Window::ImplValidateFrameRegion( const Region* pRegion, sal_uInt16 nFlags )
{
    if ( !pRegion )
        mpWindowImpl->maInvalidateRegion.SetEmpty();
    else
    {
        
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



void Window::ImplValidate( const Region* pRegion, sal_uInt16 nFlags )
{
    
    bool    bValidateAll = !pRegion;
    sal_uInt16  nOrgFlags = nFlags;
    if ( !(nFlags & (VALIDATE_CHILDREN | VALIDATE_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= VALIDATE_NOCHILDREN;
        else
            nFlags |= VALIDATE_CHILDREN;
    }
    if ( (nFlags & VALIDATE_NOCHILDREN) && mpWindowImpl->mpFirstChild )
        bValidateAll = false;
    if ( bValidateAll )
        ImplValidateFrameRegion( NULL, nFlags );
    else
    {
        Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aRegion( aRect );
        if ( pRegion )
            aRegion.Intersect( *pRegion );
        ImplClipBoundaries( aRegion, true, true );
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



void Window::ImplScroll( const Rectangle& rRect,
                         long nHorzScroll, long nVertScroll, sal_uInt16 nFlags )
{
    if ( !IsDeviceOutputNecessary() )
        return;

    nHorzScroll = ImplLogicWidthToDevicePixel( nHorzScroll );
    nVertScroll = ImplLogicHeightToDevicePixel( nVertScroll );

    if ( !nHorzScroll && !nVertScroll )
        return;

    
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
    bool    bScrollChildren = (nFlags & SCROLL_CHILDREN) != 0;
    bool    bErase = (nFlags & SCROLL_NOERASE) == 0;

    if ( !mpWindowImpl->mpFirstChild )
        bScrollChildren = false;

    OutputDevice *pOutDev = GetOutDev();

    
    bool bReMirror = ( ImplIsAntiparallel() );

    Rectangle aRectMirror( rRect );
    if( bReMirror )
    {
        
        
        pOutDev->ReMirror( aRectMirror );
    }

    
    ImplMoveAllInvalidateRegions( aRectMirror, nHorzScroll, nVertScroll, bScrollChildren );

    if ( !(nFlags & SCROLL_NOINVALIDATE) )
    {
        ImplCalcOverlapRegion( aRectMirror, aInvalidateRegion, !bScrollChildren, true, false );

        
        
        
        

        if ( !aInvalidateRegion.IsEmpty() )
        {
            aInvalidateRegion.Move( bReMirror ? -nHorzScroll : nHorzScroll, nVertScroll );
            bErase = true;
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

    ImplClipBoundaries( aRegion, false, true );
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
                
                pOutDev->ReMirror( aRegion );
            }

            pOutDev->ImplSelectClipRegion( aRegion, pGraphics );
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



void Window::ImplUpdateAll( bool bOverlapWindows )
{
    if ( !mpWindowImpl->mbReallyVisible )
        return;

    bool bFlush = false;
    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) )
            bFlush = true;
    }

    
    
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



void Window::ImplUpdateWindowPtr( Window* pWindow )
{
    if ( mpWindowImpl->mpFrameWindow != pWindow->mpWindowImpl->mpFrameWindow )
    {
        
        OutputDevice *pOutDev = GetOutDev();
        pOutDev->ImplReleaseGraphics();
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



void Window::ImplUpdateWindowPtr()
{
    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateWindowPtr( this );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}



void Window::ImplUpdateOverlapWindowPtr( bool bNewFrame )
{
    bool bVisible = IsVisible();
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



bool Window::ImplUpdatePos()
{
    bool bSysChild = false;

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
            bSysChild = true;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    if ( mpWindowImpl->mpSysObj )
        bSysChild = true;

    return bSysChild;
}



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


void Window::ImplPosSizeWindow( long nX, long nY,
                                long nWidth, long nHeight, sal_uInt16 nFlags )
{
    bool    bNewPos         = false;
    bool    bNewSize        = false;
    bool    bCopyBits       = false;
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
            bCopyBits = true;
    }

    bool bnXRecycled = false; 
    if ( nFlags & WINDOW_POSSIZE_WIDTH )
    {
        if(!( nFlags & WINDOW_POSSIZE_X ))
        {
            nX = mpWindowImpl->mnX;
            nFlags |= WINDOW_POSSIZE_X;
            bnXRecycled = true; 
        }

        if ( nWidth < 0 )
            nWidth = 0;
        if ( nWidth != mnOutWidth )
        {
            mnOutWidth = nWidth;
            bNewSize = true;
            bCopyBits = false;
        }
    }
    if ( nFlags & WINDOW_POSSIZE_HEIGHT )
    {
        if ( nHeight < 0 )
            nHeight = 0;
        if ( nHeight != mnOutHeight )
        {
            mnOutHeight = nHeight;
            bNewSize = true;
            bCopyBits = false;
        }
    }

    if ( nFlags & WINDOW_POSSIZE_X )
    {
        long nOrgX = nX;
        
        Point aPtDev( Point( nX+mnOutOffX, 0 ) );
        OutputDevice *pOutDev = GetOutDev();
        if( pOutDev->HasMirroredGraphics() )
        {
            mpGraphics->mirror( aPtDev.X(), this );

            
            
            
            if( !bnXRecycled && mpWindowImpl->mpParent && !mpWindowImpl->mpParent->mpWindowImpl->mbFrame && mpWindowImpl->mpParent->ImplIsAntiparallel() )
            {
                
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
            
            {
                
                nX = mpWindowImpl->mpParent->mnOutWidth - mnOutWidth - nX;
            }
        }

        
        if ( mpWindowImpl->mnAbsScreenX != aPtDev.X() || nX != mpWindowImpl->mnX || nOrgX != mpWindowImpl->maPos.X() )
        {
            if ( bCopyBits && !pOverlapRegion )
            {
                pOverlapRegion = new Region();
                ImplCalcOverlapRegion( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                                  Size( mnOutWidth, mnOutHeight ) ),
                                       *pOverlapRegion, false, true, true );
            }
            mpWindowImpl->mnX = nX;
            mpWindowImpl->maPos.X() = nOrgX;
            mpWindowImpl->mnAbsScreenX = aPtDev.X();    
            bNewPos = true;
        }
    }
    if ( nFlags & WINDOW_POSSIZE_Y )
    {
        
        if ( nY != mpWindowImpl->mnY || nY != mpWindowImpl->maPos.Y() )
        {
            if ( bCopyBits && !pOverlapRegion )
            {
                pOverlapRegion = new Region();
                ImplCalcOverlapRegion( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                                  Size( mnOutWidth, mnOutHeight ) ),
                                       *pOverlapRegion, false, true, true );
            }
            mpWindowImpl->mnY = nY;
            mpWindowImpl->maPos.Y() = nY;
            bNewPos = true;
        }
    }

    if ( bNewPos || bNewSize )
    {
        bool bUpdateSysObjPos = false;
        if ( bNewPos )
            bUpdateSysObjPos = ImplUpdatePos();

        
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
            
            
            mpWindowImpl->mpClientWindow->mpWindowImpl->maPos = mpWindowImpl->maPos;
            if ( bNewPos )
            {
                if ( mpWindowImpl->mpClientWindow->IsVisible() )
                {
                    mpWindowImpl->mpClientWindow->ImplCallMove();
                }
                else
                {
                    mpWindowImpl->mpClientWindow->mpWindowImpl->mbCallMove = true;
                }
            }
        }

        
        
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
                mpWindowImpl->mbCallMove = true;
            if ( bNewSize )
                mpWindowImpl->mbCallResize = true;
        }

        bool bUpdateSysObjClip = false;
        if ( IsReallyVisible() )
        {
            if ( bNewPos || bNewSize )
            {
                
                if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mpSaveBackDev )
                    ImplDeleteOverlapBackground();
                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                
                bUpdateSysObjClip = !ImplSetClipFlag( true );
            }

            
            if ( bNewPos || (mnOutWidth > nOldOutWidth) || (mnOutHeight > nOldOutHeight) )
            {
                if ( bNewPos )
                {
                    bool bInvalidate = false;
                    bool bParentPaint = true;
                    if ( !ImplIsOverlapWindow() )
                        bParentPaint = mpWindowImpl->mpParent->IsPaintEnabled();
                    if ( bCopyBits && bParentPaint && !HasPaintEvent() )
                    {
                        Point aPoint( mnOutOffX, mnOutOffY );
                        Region aRegion( Rectangle( aPoint,
                                                   Size( mnOutWidth, mnOutHeight ) ) );
                        if ( mpWindowImpl->mbWinRegion )
                            aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
                        ImplClipBoundaries( aRegion, false, true );
                        if ( !pOverlapRegion->IsEmpty() )
                        {
                            pOverlapRegion->Move( mnOutOffX-nOldOutOffX, mnOutOffY-nOldOutOffY );
                            aRegion.Exclude( *pOverlapRegion );
                        }
                        if ( !aRegion.IsEmpty() )
                        {
                            
                            ImplMoveAllInvalidateRegions( Rectangle( Point( nOldOutOffX, nOldOutOffY ),
                                                                     Size( nOldOutWidth, nOldOutHeight ) ),
                                                          mnOutOffX-nOldOutOffX, mnOutOffY-nOldOutOffY,
                                                          true );
                            SalGraphics* pGraphics = ImplGetFrameGraphics();
                            if ( pGraphics )
                            {

                                OutputDevice *pOutDev = GetOutDev();
                                const bool bSelectClipRegion = pOutDev->ImplSelectClipRegion( aRegion, pGraphics );
                                if ( bSelectClipRegion )
                                {
                                    pGraphics->CopyArea( mnOutOffX, mnOutOffY,
                                                         nOldOutOffX, nOldOutOffY,
                                                         nOldOutWidth, nOldOutHeight,
                                                         SAL_COPYAREA_WINDOWINVALIDATE, this );
                                }
                                else
                                    bInvalidate = true;
                            }
                            else
                                bInvalidate = true;
                            if ( !bInvalidate )
                            {
                                if ( !pOverlapRegion->IsEmpty() )
                                    ImplInvalidateFrameRegion( pOverlapRegion, INVALIDATE_CHILDREN );
                            }
                        }
                        else
                            bInvalidate = true;
                    }
                    else
                        bInvalidate = true;
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
                    ImplClipBoundaries( aRegion, false, true );
                    if ( !aRegion.IsEmpty() )
                        ImplInvalidateFrameRegion( &aRegion, INVALIDATE_CHILDREN );
                }
            }

            
            if ( bNewPos ||
                 (mnOutWidth < nOldOutWidth) || (mnOutHeight < nOldOutHeight) )
            {
                Region aRegion( *pOldRegion );
                if ( !mpWindowImpl->mbPaintTransparent )
                    ImplExcludeWindowRegion( aRegion );
                ImplClipBoundaries( aRegion, false, true );
                if ( !aRegion.IsEmpty() && !mpWindowImpl->mpBorderWindow )
                    ImplInvalidateParentFrameRegion( aRegion );
            }
        }

        
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



void Window::ImplToBottomChild()
{
    if ( !ImplIsOverlapWindow() && !mpWindowImpl->mbReallyVisible && (mpWindowImpl->mpParent->mpWindowImpl->mpLastChild != this) )
    {
        
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



void Window::ImplCalcToTop( ImplCalcToTopData* pPrevData )
{
    DBG_ASSERT( ImplIsOverlapWindow(), "Window::ImplCalcToTop(): Is not a OverlapWindow" );

    if ( !mpWindowImpl->mbFrame )
    {
        if ( IsReallyVisible() )
        {
            
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



void Window::ImplToTop( sal_uInt16 nFlags )
{
    DBG_ASSERT( ImplIsOverlapWindow(), "Window::ImplToTop(): Is not a OverlapWindow" );

    if ( mpWindowImpl->mbFrame )
    {
        
        
        if ( !mpWindowImpl->mpFrameData->mbHasFocus &&
             !mpWindowImpl->mpFrameData->mbSysObjFocus &&
             !mpWindowImpl->mpFrameData->mbInSysObjFocusHdl &&
             !mpWindowImpl->mpFrameData->mbInSysObjToTopHdl )
        {
            
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
            
            mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = mpWindowImpl->mpPrev;

            
            bool    bOnTop = IsAlwaysOnTopEnabled();
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

            
            sal_uInt8 nTopLevel = mpWindowImpl->mpOverlapData->mnTopLevel;
            while ( pNextWin )
            {
                if ( (bOnTop != pNextWin->IsAlwaysOnTopEnabled()) ||
                     (nTopLevel <= pNextWin->mpWindowImpl->mpOverlapData->mnTopLevel) )
                    break;
                pNextWin = pNextWin->mpWindowImpl->mpNext;
            }

            
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

            
            if ( IsReallyVisible() )
            {
                
                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                mpWindowImpl->mpOverlapWindow->ImplSetClipFlagOverlapWindows();
            }
        }
    }
}



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
    
    pTempOverlapWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempOverlapWindow )
    {
        pTempOverlapWindow->ImplCalcToTop( pCurData );
        if ( pCurData->mpNext )
            pCurData = pCurData->mpNext;
        pTempOverlapWindow = pTempOverlapWindow->mpWindowImpl->mpNext;
    }

    
    pTempOverlapWindow = pOverlapWindow;
    do
    {
        pTempOverlapWindow->ImplToTop( nFlags );
        pTempOverlapWindow = pTempOverlapWindow->mpWindowImpl->mpOverlapWindow;
    }
    while ( !pTempOverlapWindow->mpWindowImpl->mbFrame );
    
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



void Window::ImplFocusToTop( sal_uInt16 nFlags, bool bReallyVisible )
{
    
    if ( !(nFlags & TOTOP_NOGRABFOCUS) )
    {
        
        Window* pFocusWindow = this;
        while ( !pFocusWindow->ImplIsOverlapWindow() )
        {
            
            
            if ( !pFocusWindow->mpWindowImpl->mpBorderWindow )
            {
                if ( pFocusWindow->mpWindowImpl->mnActivateMode & ACTIVATE_MODE_GRABFOCUS )
                    break;
            }
            pFocusWindow = pFocusWindow->ImplGetParent();
        }
        if ( (pFocusWindow->mpWindowImpl->mnActivateMode & ACTIVATE_MODE_GRABFOCUS) &&
             !pFocusWindow->HasChildPathFocus( true ) )
            pFocusWindow->GrabFocus();
    }

    if ( bReallyVisible )
        ImplGenerateMouseMove();
}



void Window::ImplShowAllOverlaps()
{
    Window* pOverlapWindow = mpWindowImpl->mpFirstOverlap;
    while ( pOverlapWindow )
    {
        if ( pOverlapWindow->mpWindowImpl->mbOverlapVisible )
        {
            pOverlapWindow->Show( true, SHOW_NOACTIVATE );
            pOverlapWindow->mpWindowImpl->mbOverlapVisible = false;
        }

        pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
    }
}



void Window::ImplHideAllOverlaps()
{
    Window* pOverlapWindow = mpWindowImpl->mpFirstOverlap;
    while ( pOverlapWindow )
    {
        if ( pOverlapWindow->IsVisible() )
        {
            pOverlapWindow->mpWindowImpl->mbOverlapVisible = true;
            pOverlapWindow->Show( false );
        }

        pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
    }
}



void Window::ImplCallMouseMove( sal_uInt16 nMouseCode, bool bModChanged )
{
    if ( mpWindowImpl->mpFrameData->mbMouseIn && mpWindowImpl->mpFrameWindow->mpWindowImpl->mbReallyVisible )
    {
        sal_uLong   nTime   = Time::GetSystemTicks();
        long    nX      = mpWindowImpl->mpFrameData->mnLastMouseX;
        long    nY      = mpWindowImpl->mpFrameData->mnLastMouseY;
        sal_uInt16  nCode   = nMouseCode;
        sal_uInt16  nMode   = mpWindowImpl->mpFrameData->mnMouseMode;
        bool    bLeave;
        
        if ( ((nX < 0) || (nY < 0) ||
              (nX >= mpWindowImpl->mpFrameWindow->mnOutWidth) ||
              (nY >= mpWindowImpl->mpFrameWindow->mnOutHeight)) &&
             !ImplGetSVData()->maWinData.mpCaptureWin )
            bLeave = true;
        else
            bLeave = false;
        nMode |= MOUSE_SYNTHETIC;
        if ( bModChanged )
            nMode |= MOUSE_MODIFIERCHANGED;
        ImplHandleMouseEvent( mpWindowImpl->mpFrameWindow, EVENT_MOUSEMOVE, bLeave, nX, nY, nTime, nCode, nMode );
    }
}



void Window::ImplGenerateMouseMove()
{
    if ( !mpWindowImpl->mpFrameData->mnMouseMoveId )
        Application::PostUserEvent( mpWindowImpl->mpFrameData->mnMouseMoveId, LINK( mpWindowImpl->mpFrameWindow, Window, ImplGenerateMouseMoveHdl ) );
}



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



void Window::ImplInvertFocus( const Rectangle& rRect )
{
    InvertTracking( rRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
}



void Window::ImplCallFocusChangeActivate( Window* pNewOverlapWindow,
                                          Window* pOldOverlapWindow )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pNewRealWindow;
    Window*     pOldRealWindow;
    bool bCallActivate = true;
    bool bCallDeactivate = true;

    pOldRealWindow = pOldOverlapWindow->ImplGetWindow();
    pNewRealWindow = pNewOverlapWindow->ImplGetWindow();
    if ( (pOldRealWindow->GetType() != WINDOW_FLOATINGWINDOW) ||
         pOldRealWindow->GetActivateMode() )
    {
        if ( (pNewRealWindow->GetType() == WINDOW_FLOATINGWINDOW) &&
             !pNewRealWindow->GetActivateMode() )
        {
            pSVData->maWinData.mpLastDeacWin = pOldOverlapWindow;
            bCallDeactivate = false;
        }
    }
    else if ( (pNewRealWindow->GetType() != WINDOW_FLOATINGWINDOW) ||
              pNewRealWindow->GetActivateMode() )
    {
        if ( pSVData->maWinData.mpLastDeacWin )
        {
            if ( pSVData->maWinData.mpLastDeacWin == pNewOverlapWindow )
                bCallActivate = false;
            else
            {
                Window* pLastRealWindow = pSVData->maWinData.mpLastDeacWin->ImplGetWindow();
                pSVData->maWinData.mpLastDeacWin->mpWindowImpl->mbActive = false;
                pSVData->maWinData.mpLastDeacWin->Deactivate();
                if ( pLastRealWindow != pSVData->maWinData.mpLastDeacWin )
                {
                    pLastRealWindow->mpWindowImpl->mbActive = true;
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
            pOldOverlapWindow->mpWindowImpl->mbActive = false;
            pOldOverlapWindow->Deactivate();
        }
        if ( pOldRealWindow != pOldOverlapWindow )
        {
            if( pOldRealWindow->mpWindowImpl->mbActive )
            {
                pOldRealWindow->mpWindowImpl->mbActive = false;
                pOldRealWindow->Deactivate();
            }
        }
    }
    if ( bCallActivate && ! pNewOverlapWindow->mpWindowImpl->mbActive )
    {
        if( ! pNewOverlapWindow->mpWindowImpl->mbActive )
        {
            pNewOverlapWindow->mpWindowImpl->mbActive = true;
            pNewOverlapWindow->Activate();
        }
        if ( pNewRealWindow != pNewOverlapWindow )
        {
            if( ! pNewRealWindow->mpWindowImpl->mbActive )
            {
                pNewRealWindow->mpWindowImpl->mbActive = true;
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


void Window::ImplGrabFocus( sal_uInt16 nFlags )
{
    
    if( mpWindowImpl->mbInDtor )
        return;

    
    
    ImplDelData aDogTag( this );

    
    
    
    

    if ( mpWindowImpl->mpClientWindow )
    {
        
        
        
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

    
    if ( !IsEnabled() || !IsInputEnabled() || IsInModalNonRefMode() )
        return;

    
    
    
    
    ImplSVData* pSVData = ImplGetSVData();

    bool bAsyncFocusWaiting = false;
    Window *pFrame = pSVData->maWinData.mpFirstFrame;
    while( pFrame  )
    {
        if( pFrame != mpWindowImpl->mpFrameWindow && pFrame->mpWindowImpl->mpFrameData->mnFocusId )
        {
            bAsyncFocusWaiting = true;
            break;
        }
        pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
    }

    bool bHasFocus = IsWindowFocused(*mpWindowImpl);

    bool bMustNotGrabFocus = false;
    

    Window *pParent = this;
    while( pParent )
    {
        
        
        if( ( (pParent->mpWindowImpl->mbFloatWin && ((FloatingWindow*)pParent)->GrabsFocus()) || ( pParent->GetStyle() & WB_SYSTEMFLOATWIN ) ) && !( pParent->GetStyle() & WB_MOVEABLE ) )
        {
            bMustNotGrabFocus = true;
            break;
        }
        pParent = pParent->mpWindowImpl->mpParent;
    }


    if ( ( pSVData->maWinData.mpFocusWin != this && ! mpWindowImpl->mbInDtor ) || ( bAsyncFocusWaiting && !bHasFocus && !bMustNotGrabFocus ) )
    {
        
        if ( pSVData->maWinData.mpExtTextInputWin &&
             (pSVData->maWinData.mpExtTextInputWin != this) )
            pSVData->maWinData.mpExtTextInputWin->EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );

        
        Window* pOverlapWindow = ImplGetFirstOverlapWindow();
        pOverlapWindow->mpWindowImpl->mpLastFocusWindow = this;
        mpWindowImpl->mpFrameData->mpFocusWin = this;

        if( !bHasFocus )
        {
            
            
            if( bMustNotGrabFocus )
                return;
            else
            {
                
                
                
                mpWindowImpl->mpFrame->ToTop( SAL_FRAME_TOTOP_GRABFOCUS | SAL_FRAME_TOTOP_GRABFOCUS_ONLY );
                return;
            }
        }

        Window* pOldFocusWindow = pSVData->maWinData.mpFocusWin;
        ImplDelData aOldFocusDel( pOldFocusWindow );

        pSVData->maWinData.mpFocusWin = this;

        if ( pOldFocusWindow )
        {
            
            if ( pOldFocusWindow->mpWindowImpl->mpCursor )
                pOldFocusWindow->mpWindowImpl->mpCursor->ImplHide( true );
        }

        
        
        if ( pOldFocusWindow )
        {
            
            Window* pOldOverlapWindow = pOldFocusWindow->ImplGetFirstOverlapWindow();
            Window* pNewOverlapWindow = ImplGetFirstOverlapWindow();
            if ( pOldOverlapWindow != pNewOverlapWindow )
                ImplCallFocusChangeActivate( pNewOverlapWindow, pOldOverlapWindow );
        }
        else
        {
            Window* pNewOverlapWindow = ImplGetFirstOverlapWindow();
            Window* pNewRealWindow = pNewOverlapWindow->ImplGetWindow();
            pNewOverlapWindow->mpWindowImpl->mbActive = true;
            pNewOverlapWindow->Activate();
            if ( pNewRealWindow != pNewOverlapWindow )
            {
                pNewRealWindow->mpWindowImpl->mbActive = true;
                pNewRealWindow->Activate();
            }
        }

        
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
                mpWindowImpl->mbInFocusHdl = true;
                mpWindowImpl->mnGetFocusFlags = nFlags;
                
                
                
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
                    mpWindowImpl->mbInFocusHdl = false;
                }
            }
        }

        GetpApp()->FocusChanged();
        ImplNewInputContext();
    }
}

void Window::ImplGrabFocusToDocument( sal_uInt16 nFlags )
{
    Window *pWin = this;
    while( pWin )
    {
        if( !pWin->GetParent() )
        {
            pWin->ImplGetFrameWindow()->GetWindow( WINDOW_CLIENT )->ImplGrabFocus(nFlags);
            return;
        }
        pWin = pWin->GetParent();
    }
}



void Window::ImplNewInputContext()
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pFocusWin = pSVData->maWinData.mpFocusWin;
    if ( !pFocusWin )
        return;

    
    const InputContext& rInputContext = pFocusWin->GetInputContext();
    if ( rInputContext == pFocusWin->mpWindowImpl->mpFrameData->maOldInputContext )
        return;

    pFocusWin->mpWindowImpl->mpFrameData->maOldInputContext = rInputContext;

    SalInputContext         aNewContext;
    const Font&             rFont = rInputContext.GetFont();
    const OUString&         rFontName = rFont.GetName();
    ImplFontEntry*          pFontEntry = NULL;
    aNewContext.mpFont = NULL;
    if (!rFontName.isEmpty())
    {
        OutputDevice *pFocusWinOutDev = pFocusWin->GetOutDev();
        Size aSize = pFocusWinOutDev->ImplLogicToDevicePixel( rFont.GetSize() );
        if ( !aSize.Height() )
        {
            
            
            if ( rFont.GetSize().Height() )
                aSize.Height() = 1;
            else
                aSize.Height() = (12*pFocusWin->mnDPIY)/72;
        }
        pFontEntry = pFocusWin->mpFontCache->GetFontEntry( pFocusWin->mpFontList,
                         rFont, aSize, static_cast<float>(aSize.Height()) );
        if ( pFontEntry )
            aNewContext.mpFont = &pFontEntry->maFontSelData;
    }
    aNewContext.meLanguage  = rFont.GetLanguage();
    aNewContext.mnOptions   = rInputContext.GetOptions();
    pFocusWin->ImplGetFrame()->SetInputContext( &aNewContext );

    if ( pFontEntry )
        pFocusWin->mpFontCache->Release( pFontEntry );
}



Window::Window( WindowType nType )
{
    ImplInitWindowData( nType );
}



Window::Window( Window* pParent, WinBits nStyle )
{

    ImplInitWindowData( WINDOW_WINDOW );
    ImplInit( pParent, nStyle, NULL );
}



Window::Window( Window* pParent, const ResId& rResId )
    : mpWindowImpl(NULL)
{
    rResId.SetRT( RSC_WINDOW );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInitWindowData( WINDOW_WINDOW );
    ImplInit( pParent, nStyle, NULL );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}


#if OSL_DEBUG_LEVEL > 0
namespace
{
    OString lcl_createWindowInfo(const Window& i_rWindow)
    {
        
        const Window* pWindow( &i_rWindow );
        while ( pWindow && ( pWindow->GetType() == WINDOW_BORDERWINDOW ) )
            pWindow = pWindow->GetWindow( WINDOW_FIRSTCHILD );
        if ( !pWindow )
            pWindow = &i_rWindow;

        OStringBuffer aErrorString;
        aErrorString.append(' ');
        aErrorString.append(typeid( *pWindow ).name());
        aErrorString.append(" (");
        aErrorString.append(OUStringToOString(pWindow->GetText(), RTL_TEXTENCODING_UTF8));
        aErrorString.append(")");
        return aErrorString.makeStringAndClear();
    }
}
#endif


Window::~Window()
{
    vcl::LazyDeletor<Window>::Undelete( this );

    DBG_ASSERT( !mpWindowImpl->mbInDtor, "~Window - already in DTOR!" );


    
    Application::RemoveMouseAndKeyEvents( this );

    
    
    uno::Reference< rendering::XCanvas > xCanvas( mpWindowImpl->mxCanvas );
    if( xCanvas.is() )
    {
        uno::Reference < lang::XComponent > xCanvasComponent( xCanvas,
                                                              uno::UNO_QUERY );
        if( xCanvasComponent.is() )
            xCanvasComponent->dispose();
    }

    mpWindowImpl->mbInDtor = true;

    ImplCallEventListeners( VCLEVENT_OBJECT_DYING );

    
    if( !ImplIsAccessibleNativeFrame() && mpWindowImpl->mbReallyVisible )
        if ( ImplIsAccessibleCandidate() && GetAccessibleParentWindow() )
            GetAccessibleParentWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_CHILDDESTROYED, this );

    
    ImplGetDockingManager()->RemoveWindow( this );


    
    if( (GetStyle() & WB_OWNERDRAWDECORATION) && mpWindowImpl->mbFrame )
    {
        ::std::vector< Window* >& rList = ImplGetOwnerDrawList();
        ::std::vector< Window* >::iterator p;
        p = ::std::find( rList.begin(), rList.end(), this );
        if( p != rList.end() )
            rList.erase( p );
    }

    
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XComponent > xDnDComponent( mpWindowImpl->mxDNDListenerContainer, ::com::sun::star::uno::UNO_QUERY );

    if( xDnDComponent.is() )
        xDnDComponent->dispose();

    if( mpWindowImpl->mbFrame && mpWindowImpl->mpFrameData )
    {
        try
        {
            
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

            
            uno::Reference< XComponent > xComponent( mpWindowImpl->mpFrameData->mxDropTarget, UNO_QUERY );

            
            
            if( xComponent.is() )
                xComponent->dispose();
        }
        catch (const Exception&)
        {
            
        }
    }

    UnoWrapperBase* pWrapper = Application::GetUnoWrapper( false );
    if ( pWrapper )
        pWrapper->WindowDestroyed( this );

    
    
    
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
    
    
    

    
    if ( pSVData->maWinData.mpTrackWin == this )
        EndTracking();
    if ( pSVData->maWinData.mpCaptureWin == this )
        ReleaseMouse();
    if ( pSVData->maWinData.mpDefDialogParent == this )
        pSVData->maWinData.mpDefDialogParent = NULL;

#if OSL_DEBUG_LEVEL > 0
    if ( true ) 
    {
        OStringBuffer aErrorStr;
        bool        bError = false;
        Window*     pTempWin;
        if (mpWindowImpl->mpFrameData != 0)
        {
            pTempWin = mpWindowImpl->mpFrameData->mpFirstOverlap;
            while ( pTempWin )
            {
                if ( ImplIsRealParentPath( pTempWin ) )
                {
                    bError = true;
                    aErrorStr.append(lcl_createWindowInfo(*pTempWin));
                }
                pTempWin = pTempWin->mpWindowImpl->mpNextOverlap;
            }
            if ( bError )
            {
                OStringBuffer aTempStr;
                aTempStr.append("Window (");
                aTempStr.append(OUStringToOString(GetText(),
                                                       RTL_TEXTENCODING_UTF8));
                aTempStr.append(") with live SystemWindows destroyed: ");
                aTempStr.append(aErrorStr.toString());
                OSL_FAIL(aTempStr.getStr());
                
                GetpApp()->Abort(OStringToOUString(
                                     aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));
            }
        }

        bError = false;
        pTempWin = pSVData->maWinData.mpFirstFrame;
        while ( pTempWin )
        {
            if ( ImplIsRealParentPath( pTempWin ) )
            {
                bError = true;
                aErrorStr.append(lcl_createWindowInfo(*pTempWin));
            }
            pTempWin = pTempWin->mpWindowImpl->mpFrameData->mpNextFrame;
        }
        if ( bError )
        {
            OStringBuffer aTempStr( "Window (" );
            aTempStr.append(OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") with live SystemWindows destroyed: ");
            aTempStr.append(aErrorStr.toString());
            OSL_FAIL( aTempStr.getStr() );
            GetpApp()->Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   
        }

        if ( mpWindowImpl->mpFirstChild )
        {
            OStringBuffer aTempStr("Window (");
            aTempStr.append(OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") with live children destroyed: ");
            pTempWin = mpWindowImpl->mpFirstChild;
            while ( pTempWin )
            {
                aTempStr.append(lcl_createWindowInfo(*pTempWin));
                pTempWin = pTempWin->mpWindowImpl->mpNext;
            }
            OSL_FAIL( aTempStr.getStr() );
            GetpApp()->Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   
        }

        if ( mpWindowImpl->mpFirstOverlap )
        {
            OStringBuffer aTempStr("Window (");
            aTempStr.append(OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") with live SystemWindows destroyed: ");
            pTempWin = mpWindowImpl->mpFirstOverlap;
            while ( pTempWin )
            {
                aTempStr.append(lcl_createWindowInfo(*pTempWin));
                pTempWin = pTempWin->mpWindowImpl->mpNext;
            }
            OSL_FAIL( aTempStr.getStr() );
            GetpApp()->Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   
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
            OStringBuffer aTempStr("Window (");
            aTempStr.append(OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") still in TaskPanelList!");
            OSL_FAIL( aTempStr.getStr() );
            GetpApp()->Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   
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
            OStringBuffer aTempStr("Window (");
            aTempStr.append(OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") not found in TaskPanelList!");
            OSL_FAIL( aTempStr.getStr() );
        }
    }

    
    remove_from_all_size_groups();

    
    std::vector<FixedText*> aMnemonicLabels(list_mnemonic_labels());
    for (std::vector<FixedText*>::iterator aI = aMnemonicLabels.begin();
        aI != aMnemonicLabels.end(); ++aI)
    {
        remove_mnemonic_label(*aI);
    }

    
    Hide();

    
    {
        NotifyEvent aNEvt( EVENT_DESTROY, this );
        Notify( aNEvt );
    }

    
    if ( pSVData->maWinData.mpExtTextInputWin == this )
    {
        EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );
        if ( pSVData->maWinData.mpExtTextInputWin == this )
            pSVData->maWinData.mpExtTextInputWin = NULL;
    }

    
    bool bHasFocussedChild = false;
    if( pSVData->maWinData.mpFocusWin && ImplIsRealParentPath( pSVData->maWinData.mpFocusWin ) )
    {
        
        bHasFocussedChild = true;
#if OSL_DEBUG_LEVEL > 0
        OStringBuffer aTempStr("Window (");
        aTempStr.append(OUStringToOString(GetText(),
            RTL_TEXTENCODING_UTF8)).
                append(") with focussed child window destroyed ! THIS WILL LEAD TO CRASHES AND MUST BE FIXED !");
        OSL_FAIL( aTempStr.getStr() );
        GetpApp()->Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ));   
#endif
    }

    
    Window* pOverlapWindow = ImplGetFirstOverlapWindow();
    if ( pSVData->maWinData.mpFocusWin == this
        || bHasFocussedChild )  
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

            
            if ( pSVData->maWinData.mpFocusWin == this )
            {
                pSVData->maWinData.mpFocusWin = NULL;
                pOverlapWindow->mpWindowImpl->mpLastFocusWindow = NULL;
                GetpApp()->FocusChanged();
            }
        }
    }


    if ( pOverlapWindow != 0 &&
         pOverlapWindow->mpWindowImpl->mpLastFocusWindow == this )
        pOverlapWindow->mpWindowImpl->mpLastFocusWindow = NULL;

    
    if( pSVData->maWinData.mpActiveApplicationFrame == this )
        pSVData->maWinData.mpActiveApplicationFrame = NULL;

    
    if ( mpWindowImpl->mpFrameData != 0 )
    {
        if ( mpWindowImpl->mpFrameData->mpFocusWin == this )
            mpWindowImpl->mpFrameData->mpFocusWin = NULL;
        if ( mpWindowImpl->mpFrameData->mpMouseMoveWin == this )
            mpWindowImpl->mpFrameData->mpMouseMoveWin = NULL;
        if ( mpWindowImpl->mpFrameData->mpMouseDownWin == this )
            mpWindowImpl->mpFrameData->mpMouseDownWin = NULL;
    }

    
    if ( pSVData->maWinData.mpLastDeacWin == this )
        pSVData->maWinData.mpLastDeacWin = NULL;

    if ( mpWindowImpl->mbFrame && mpWindowImpl->mpFrameData )
    {
        if ( mpWindowImpl->mpFrameData->mnFocusId )
            Application::RemoveUserEvent( mpWindowImpl->mpFrameData->mnFocusId );
        if ( mpWindowImpl->mpFrameData->mnMouseMoveId )
            Application::RemoveUserEvent( mpWindowImpl->mpFrameData->mnMouseMoveId );
    }

    
    OutputDevice *pOutDev = GetOutDev();
    pOutDev->ImplReleaseGraphics();

    
    ImplDelData* pDelData = mpWindowImpl->mpFirstDel;
    while ( pDelData )
    {
        pDelData->mbDel = true;
        pDelData->mpWindow = NULL;  
        pDelData = pDelData->mpNext;
    }

    
    ImplRemoveWindow( true );

    
    if ( mpWindowImpl->mbFrame )
    {
        bool bIsTopWindow = mpWindowImpl->mpWinData && ( mpWindowImpl->mpWinData->mnIsTopWindow == 1 );
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

    
    if ( mpWindowImpl->mpWinData )
    {
        if ( mpWindowImpl->mpWinData->mpExtOldText )
            delete mpWindowImpl->mpWinData->mpExtOldText;
        if ( mpWindowImpl->mpWinData->mpExtOldAttrAry )
            delete mpWindowImpl->mpWinData->mpExtOldAttrAry;
        if ( mpWindowImpl->mpWinData->mpCursorRect )
            delete mpWindowImpl->mpWinData->mpCursorRect;
        if ( mpWindowImpl->mpWinData->mpCompositionCharRects)
            delete[] mpWindowImpl->mpWinData->mpCompositionCharRects;
        if ( mpWindowImpl->mpWinData->mpFocusRect )
            delete mpWindowImpl->mpWinData->mpFocusRect;
        if ( mpWindowImpl->mpWinData->mpTrackRect )
            delete mpWindowImpl->mpWinData->mpTrackRect;

        delete mpWindowImpl->mpWinData;
    }

    
    if ( mpWindowImpl->mpOverlapData )
        delete mpWindowImpl->mpOverlapData;

    
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

    
    delete mpWindowImpl; mpWindowImpl = NULL;
}


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



void Window::MouseMove( const MouseEvent& rMEvt )
{
    NotifyEvent aNEvt( EVENT_MOUSEMOVE, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseMove = true;
}



void Window::MouseButtonDown( const MouseEvent& rMEvt )
{
    NotifyEvent aNEvt( EVENT_MOUSEBUTTONDOWN, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseButtonDown = true;
}



void Window::MouseButtonUp( const MouseEvent& rMEvt )
{
    NotifyEvent aNEvt( EVENT_MOUSEBUTTONUP, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseButtonUp = true;
}



void Window::KeyInput( const KeyEvent& rKEvt )
{
    NotifyEvent aNEvt( EVENT_KEYINPUT, this, &rKEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbKeyInput = true;
}



void Window::KeyUp( const KeyEvent& rKEvt )
{
    NotifyEvent aNEvt( EVENT_KEYUP, this, &rKEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbKeyUp = true;
}



void Window::PrePaint()
{
}



void Window::Paint( const Rectangle& rRect )
{
    ImplCallEventListeners( VCLEVENT_WINDOW_PAINT, (void*)&rRect );
}



void Window::PostPaint()
{
}



void Window::Draw( OutputDevice*, const Point&, const Size&, sal_uLong )
{
}

void Window::Move() {}

void Window::Resize() {}

void Window::Activate() {}

void Window::Deactivate() {}

void Window::GetFocus()
{
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



void Window::LoseFocus()
{
    NotifyEvent aNEvt( EVENT_LOSEFOCUS, this );
    Notify( aNEvt );
}



void Window::RequestHelp( const HelpEvent& rHEvt )
{
    
    
    if ( rHEvt.GetMode() & HELPMODE_BALLOON )
    {
        OUString rStr = GetHelpText();
        if ( rStr.isEmpty() )
            rStr = GetQuickHelpText();
        if ( rStr.isEmpty() && ImplGetParent() && !ImplIsOverlapWindow() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
            Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), rStr );
    }
    else if ( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        const OUString& rStr = GetQuickHelpText();
        if ( rStr.isEmpty() && ImplGetParent() && !ImplIsOverlapWindow() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            Point aPos = GetPosPixel();
            if ( ImplGetParent() && !ImplIsOverlapWindow() )
                aPos = ImplGetParent()->OutputToScreenPixel( aPos );
            Rectangle   aRect( aPos, GetSizePixel() );
            OUString      aHelpText;
            if ( !rStr.isEmpty() )
                aHelpText = GetHelpText();
            Help::ShowQuickHelp( this, aRect, rStr, aHelpText, QUICKHELP_CTRLTEXT );
        }
    }
    else
    {
        OUString aStrHelpId( OStringToOUString( GetHelpId(), RTL_TEXTENCODING_UTF8 ) );
        if ( aStrHelpId.isEmpty() && ImplGetParent() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                if( !aStrHelpId.isEmpty() )
                    pHelp->Start( aStrHelpId, this );
                else
                    pHelp->Start( OUString( OOO_HELP_INDEX  ), this );
            }
        }
    }
}



void Window::Command( const CommandEvent& rCEvt )
{
    ImplCallEventListeners( VCLEVENT_WINDOW_COMMAND, (void*)&rCEvt );

    NotifyEvent aNEvt( EVENT_COMMAND, this, &rCEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbCommand = true;
}



void Window::Tracking( const TrackingEvent& rTEvt )
{

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
        pWrapper->Tracking( rTEvt );
}



void Window::UserEvent( sal_uLong, void* )
{
}



void Window::StateChanged( StateChangedType eType )
{
    switch (eType)
    {
        
        case STATE_CHANGE_CONTROLFOREGROUND:
        case STATE_CHANGE_CONTROLBACKGROUND:
        case STATE_CHANGE_TRANSPARENT:
        case STATE_CHANGE_UPDATEMODE:
        case STATE_CHANGE_READONLY:
        case STATE_CHANGE_ENABLE:
        case STATE_CHANGE_STATE:
        case STATE_CHANGE_DATA:
        case STATE_CHANGE_INITSHOW:
        case STATE_CHANGE_CONTROL_FOCUS:
            break;
        
        default:
            queue_resize();
            break;
    }
}



void Window::DataChanged( const DataChangedEvent& )
{
}



void Window::ImplNotifyKeyMouseCommandEventListeners( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_COMMAND )
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        if ( pCEvt->GetCommand() != COMMAND_CONTEXTMENU )
            
            
            return;

        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                
                
                ;
            else
            {
                CommandEvent aCommandEvent = ImplTranslateCommandEvent( *pCEvt, rNEvt.GetWindow(), this );
                ImplCallEventListeners( VCLEVENT_WINDOW_COMMAND, &aCommandEvent );
            }
        }
    }

    
    
    
    

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



bool Window::PreNotify( NotifyEvent& rNEvt )
{
    bool bDone = false;
    if ( mpWindowImpl->mpParent && !ImplIsOverlapWindow() )
        bDone = mpWindowImpl->mpParent->PreNotify( rNEvt );

    if ( !bDone )
    {
        if( rNEvt.GetType() == EVENT_GETFOCUS )
        {
            bool bCompoundFocusChanged = false;
            if ( mpWindowImpl->mbCompoundControl && !mpWindowImpl->mbCompoundControlHasFocus && HasChildPathFocus() )
            {
                mpWindowImpl->mbCompoundControlHasFocus = true;
                bCompoundFocusChanged = true;
            }

            if ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) )
                ImplCallEventListeners( VCLEVENT_WINDOW_GETFOCUS );
        }
        else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
        {
            bool bCompoundFocusChanged = false;
            if ( mpWindowImpl->mbCompoundControl && mpWindowImpl->mbCompoundControlHasFocus && !HasChildPathFocus() )
            {
                mpWindowImpl->mbCompoundControlHasFocus = false ;
                bCompoundFocusChanged = true;
            }

            if ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) )
                ImplCallEventListeners( VCLEVENT_WINDOW_LOSEFOCUS );
        }

        
        

    }

    return bDone;
}



bool Window::Notify( NotifyEvent& rNEvt )
{
    bool nRet = false;

    
    
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper && !( !pWrapper->IsFloatingMode() && pWrapper->IsLocked() ) )
    {
        if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            bool bHit = pWrapper->GetDragArea().IsInside( pMEvt->GetPosPixel() );
            if ( pMEvt->IsLeft() )
            {
                if ( pMEvt->IsMod1() && (pMEvt->GetClicks() == 2) )
                {
                    
                    pWrapper->SetFloatingMode( !pWrapper->IsFloatingMode() );
                    return true;
                }
                else if ( pMEvt->GetClicks() == 1 && bHit)
                {
                    
                    pWrapper->ImplEnableStartDocking();
                    return true;
                }
            }
        }
        else if ( rNEvt.GetType() == EVENT_MOUSEMOVE )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            bool bHit = pWrapper->GetDragArea().IsInside( pMEvt->GetPosPixel() );
            if ( pMEvt->IsLeft() )
            {
                
                
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
                return true;
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
                return true;
            }
        }
    }

    
    if ( (GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL )
    {
        
        if ( (rNEvt.GetType() == EVENT_KEYINPUT) || (rNEvt.GetType() == EVENT_KEYUP) )
        {
            if ( ImplIsOverlapWindow() ||
                 ((getNonLayoutRealParent(this)->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
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



void Window::ImplCallEventListeners( sal_uLong nEvent, void* pData )
{
    
    
    
    
    

    CallEventListeners( nEvent, pData );
}



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



void Window::AddEventListener( const Link& rEventListener )
{
    mpWindowImpl->maEventListeners.addListener( rEventListener );
}



void Window::RemoveEventListener( const Link& rEventListener )
{
    mpWindowImpl->maEventListeners.removeListener( rEventListener );
}



void Window::AddChildEventListener( const Link& rEventListener )
{
    mpWindowImpl->maChildEventListeners.addListener( rEventListener );
}



void Window::RemoveChildEventListener( const Link& rEventListener )
{
    mpWindowImpl->maChildEventListeners.removeListener( rEventListener );
}



sal_uLong Window::PostUserEvent( const Link& rLink, void* pCaller )
{
    sal_uLong nEventId;
    PostUserEvent( nEventId, rLink, pCaller );
    return nEventId;
}





bool Window::PostUserEvent( sal_uLong& rEventId, const Link& rLink, void* pCaller )
{

    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mnEvent   = 0;
    pSVEvent->mpData    = pCaller;
    pSVEvent->mpLink    = new Link( rLink );
    pSVEvent->mpWindow  = this;
    pSVEvent->mbCall    = true;
    ImplAddDel( &(pSVEvent->maDelData) );
    rEventId = (sal_uLong)pSVEvent;
    if ( mpWindowImpl->mpFrame->PostEvent( pSVEvent ) )
        return true;
    else
    {
        rEventId = 0;
        ImplRemoveDel( &(pSVEvent->maDelData) );
        delete pSVEvent;
        return false;
    }
}



void Window::RemoveUserEvent( sal_uLong nUserEvent )
{

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

    pSVEvent->mbCall = false;
}



bool Window::IsLocked( bool bChildren ) const
{
    if ( mpWindowImpl->mnLockCount != 0 )
        return true;

    if ( bChildren || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            if ( pChild->IsLocked( true ) )
                return true;
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }

    return false;
}



void Window::SetStyle( WinBits nStyle )
{

    if ( mpWindowImpl->mnStyle != nStyle )
    {
        mpWindowImpl->mnPrevStyle = mpWindowImpl->mnStyle;
        mpWindowImpl->mnStyle = nStyle;
        StateChanged( STATE_CHANGE_STYLE );
    }
}



void Window::SetExtendedStyle( WinBits nExtendedStyle )
{

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



SystemWindow* Window::GetSystemWindow() const
{

    const Window* pWin = this;
    while ( pWin && !pWin->IsSystemWindow() )
        pWin  = pWin->GetParent();
    return (SystemWindow*)pWin;
}



void Window::SetBorderStyle( sal_uInt16 nBorderStyle )
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        if( nBorderStyle == WINDOW_BORDER_REMOVEBORDER &&
            ! mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame &&
            mpWindowImpl->mpBorderWindow->mpWindowImpl->mpParent
            )
        {
            
            
            
            Window* pBorderWin = mpWindowImpl->mpBorderWindow;
            
            pBorderWin->mpWindowImpl->mpClientWindow = NULL;
            mpWindowImpl->mpBorderWindow = NULL;
            mpWindowImpl->mpRealParent = pBorderWin->mpWindowImpl->mpParent;
            
            SetParent( pBorderWin->mpWindowImpl->mpParent );
            
            Point aBorderPos( pBorderWin->GetPosPixel() );
            Size aBorderSize( pBorderWin->GetSizePixel() );
            setPosSizePixel( aBorderPos.X(), aBorderPos.Y(), aBorderSize.Width(), aBorderSize.Height() );
            
            delete pBorderWin;

            
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



sal_uInt16 Window::GetBorderStyle() const
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        if ( mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
            return ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->GetBorderStyle();
        else
            return mpWindowImpl->mpBorderWindow->GetBorderStyle();
    }

    return 0;
}



long Window::CalcTitleWidth() const
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        if ( mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
            return ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->CalcTitleWidth();
        else
            return mpWindowImpl->mpBorderWindow->CalcTitleWidth();
    }
    else if ( mpWindowImpl->mbFrame && (mpWindowImpl->mnStyle & WB_MOVEABLE) )
    {
        
        
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



void Window::EnableClipSiblings( bool bClipSiblings )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->EnableClipSiblings( bClipSiblings );

    mpWindowImpl->mbClipSiblings = bClipSiblings;
}



void Window::SetMouseTransparent( bool bTransparent )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetMouseTransparent( bTransparent );

    if( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->SetMouseTransparent( bTransparent );

    mpWindowImpl->mbMouseTransparent = bTransparent;
}



void Window::SetPaintTransparent( bool bTransparent )
{

    
    if( bTransparent && mpWindowImpl->mbFrame )
        return;

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetPaintTransparent( bTransparent );

    mpWindowImpl->mbPaintTransparent = bTransparent;
}



void Window::SetInputContext( const InputContext& rInputContext )
{

    mpWindowImpl->maInputContext = rInputContext;
    if ( !mpWindowImpl->mbInFocusHdl && HasFocus() )
        ImplNewInputContext();
}



void Window::EndExtTextInput( sal_uInt16 nFlags )
{

    if ( mpWindowImpl->mbExtTextInput )
        ImplGetFrame()->EndExtTextInput( nFlags );
}



void Window::SetCursorRect( const Rectangle* pRect, long nExtTextInputWidth )
{

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



const Rectangle* Window::GetCursorRect() const
{

    ImplWinData* pWinData = ImplGetWinData();
    return pWinData->mpCursorRect;
}



long Window::GetCursorExtTextInputWidth() const
{

    ImplWinData* pWinData = ImplGetWinData();
    return pWinData->mnCursorExtWidth;
}



void Window::SetCompositionCharRect( const Rectangle* pRect, long nCompositionLength, bool bVertical ) {

    ImplWinData* pWinData = ImplGetWinData();
    delete[] pWinData->mpCompositionCharRects;
    pWinData->mbVertical = bVertical;
    pWinData->mpCompositionCharRects = NULL;
    pWinData->mnCompositionCharRects = nCompositionLength;
    if ( pRect && (nCompositionLength > 0) )
    {
        pWinData->mpCompositionCharRects = new Rectangle[nCompositionLength];
        for (long i = 0; i < nCompositionLength; ++i)
            pWinData->mpCompositionCharRects[i] = pRect[i];
    }
}


void Window::SetSettings( const AllSettings& rSettings )
{
    SetSettings( rSettings, sal_False );
}

void Window::SetSettings( const AllSettings& rSettings, sal_Bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->SetSettings( rSettings, sal_False );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->SetSettings( rSettings, sal_True );
    }

    AllSettings aOldSettings(*mxSettings);
    OutputDevice::SetSettings( rSettings );
    sal_uLong nChangeFlags = aOldSettings.GetChangeFlags( rSettings );

    
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



void Window::UpdateSettings( const AllSettings& rSettings, bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->UpdateSettings( rSettings, false );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->UpdateSettings( rSettings, true );
    }

    AllSettings aOldSettings(*mxSettings);
    sal_uLong nChangeFlags = mxSettings->Update( mxSettings->GetWindowUpdate(), rSettings );
    nChangeFlags |= SETTINGS_IN_UPDATE_SETTINGS; 
                                                 
                                                 

    
    ImplInitResolutionSettings();

    /* #i73785#
    *  do not overwrite a WheelBehavior with false
    *  this looks kind of a hack, but WheelBehavior
    *  is always a local change, not a system property,
    *  so we can spare all our users the hassle of reacting on
    *  this in their respective DataChanged.
    */
    MouseSettings aSet( mxSettings->GetMouseSettings() );
    aSet.SetWheelBehavior( aOldSettings.GetMouseSettings().GetWheelBehavior() );
    mxSettings->SetMouseSettings( aSet );

    if( (nChangeFlags & SETTINGS_STYLE) && IsBackground() )
    {
        Wallpaper aWallpaper = GetBackground();
        if( !aWallpaper.IsBitmap() && !aWallpaper.IsGradient() )
        {
            if ( mpWindowImpl->mnStyle & WB_3DLOOK )
            {
                if (aOldSettings.GetStyleSettings().GetFaceColor() != rSettings.GetStyleSettings().GetFaceColor())
                    SetBackground( Wallpaper( rSettings.GetStyleSettings().GetFaceColor() ) );
            }
            else
            {
                if (aOldSettings.GetStyleSettings().GetWindowColor() != rSettings.GetStyleSettings().GetWindowColor())
                    SetBackground( Wallpaper( rSettings.GetStyleSettings().GetWindowColor() ) );
            }
        }
    }

    if ( nChangeFlags )
    {
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
        DataChanged( aDCEvt );
        
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



void Window::NotifyAllChildren( DataChangedEvent& rDCEvt )
{

    DataChanged( rDCEvt );

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->NotifyAllChildren( rDCEvt );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}



void Window::SetPointFont( const Font& rFont )
{

    Font aFont = rFont;
    ImplPointToLogic( aFont );
    SetFont( aFont );
}



Font Window::GetPointFont() const
{

    Font aFont = GetFont();
    ImplLogicToPoint( aFont );
    return aFont;
}



void Window::SetParentClipMode( sal_uInt16 nMode )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetParentClipMode( nMode );
    else
    {
        if ( !ImplIsOverlapWindow() )
        {
            mpWindowImpl->mnParentClipMode = nMode;
            if ( nMode & PARENTCLIPMODE_CLIP )
                mpWindowImpl->mpParent->mpWindowImpl->mbClipChildren = true;
        }
    }
}



sal_uInt16 Window::GetParentClipMode() const
{

    if ( mpWindowImpl->mpBorderWindow )
        return mpWindowImpl->mpBorderWindow->GetParentClipMode();
    else
        return mpWindowImpl->mnParentClipMode;
}



void Window::SetWindowRegionPixel()
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetWindowRegionPixel();
    else if( mpWindowImpl->mbFrame )
    {
        mpWindowImpl->maWinRegion = Region(true);
        mpWindowImpl->mbWinRegion = false;
        mpWindowImpl->mpFrame->ResetClipRegion();
    }
    else
    {
        if ( mpWindowImpl->mbWinRegion )
        {
            mpWindowImpl->maWinRegion = Region(true);
            mpWindowImpl->mbWinRegion = false;
            ImplSetClipFlag();

            if ( IsReallyVisible() )
            {
                
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



void Window::SetWindowRegionPixel( const Region& rRegion )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetWindowRegionPixel( rRegion );
    else if( mpWindowImpl->mbFrame )
    {
        if( !rRegion.IsNull() )
        {
            mpWindowImpl->maWinRegion = rRegion;
            mpWindowImpl->mbWinRegion = ! rRegion.IsEmpty();

            if( mpWindowImpl->mbWinRegion )
            {
                
                RectangleVector aRectangles;
                mpWindowImpl->maWinRegion.GetRegionRectangles(aRectangles);
                mpWindowImpl->mpFrame->BeginSetClipRegion(aRectangles.size());

                for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
                {
                    mpWindowImpl->mpFrame->UnionClipRegion(
                        aRectIter->Left(),
                        aRectIter->Top(),
                        aRectIter->GetWidth(),       
                        aRectIter->GetHeight());     
                }

                mpWindowImpl->mpFrame->EndSetClipRegion();

                
                
                
                
                
                
                
                //
                
                
                
                
                
                
                
                
                
            }
            else
                SetWindowRegionPixel();
        }
        else
            SetWindowRegionPixel();
    }
    else
    {
        if ( rRegion.IsNull() )
        {
            if ( mpWindowImpl->mbWinRegion )
            {
                mpWindowImpl->maWinRegion = Region(true);
                mpWindowImpl->mbWinRegion = false;
                ImplSetClipFlag();
            }
        }
        else
        {
            mpWindowImpl->maWinRegion = rRegion;
            mpWindowImpl->mbWinRegion = true;
            ImplSetClipFlag();
        }

        if ( IsReallyVisible() )
        {
            
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



const Region& Window::GetWindowRegionPixel() const
{

    if ( mpWindowImpl->mpBorderWindow )
        return mpWindowImpl->mpBorderWindow->GetWindowRegionPixel();
    else
        return mpWindowImpl->maWinRegion;
}



bool Window::IsWindowRegionPixel() const
{

    if ( mpWindowImpl->mpBorderWindow )
        return mpWindowImpl->mpBorderWindow->IsWindowRegionPixel();
    else
        return mpWindowImpl->mbWinRegion;
}



Region Window::GetWindowClipRegionPixel( sal_uInt16 nFlags ) const
{

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
        
        if( ImplIsAntiparallel() )
        {
            const OutputDevice *pOutDev = GetOutDev();
            pOutDev->ReMirror( aWinClipRegion );
        }
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



Region Window::GetPaintRegion() const
{

    if ( mpWindowImpl->mpPaintRegion )
    {
        Region aRegion = *mpWindowImpl->mpPaintRegion;
        aRegion.Move( -mnOutOffX, -mnOutOffY );
        return PixelToLogic( aRegion );
    }
    else
    {
        Region aPaintRegion(true);
        return aPaintRegion;
    }
}



void Window::ExpandPaintClipRegion( const Region& rRegion )
{
    if( mpWindowImpl->mpPaintRegion )
    {
        Region aPixRegion = LogicToPixel( rRegion );
        Region aDevPixRegion = ImplPixelToDevicePixel( aPixRegion );

        Region aWinChildRegion = *ImplGetWinChildClipRegion();
        
        if( ImplIsAntiparallel() )
        {
            const OutputDevice *pOutDev = GetOutDev();
            pOutDev->ReMirror( aWinChildRegion );
        }

        aDevPixRegion.Intersect( aWinChildRegion );
        if( ! aDevPixRegion.IsEmpty() )
        {
            mpWindowImpl->mpPaintRegion->Union( aDevPixRegion );
            mbInitClipRegion = true;
        }
    }
}



static SystemWindow *ImplGetLastSystemWindow( Window *pWin )
{
    
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
    DBG_ASSERT( pNewParent, "Window::SetParent(): pParent == NULL" );
    DBG_ASSERT( pNewParent != this, "someone tried to reparent a window to itself" );

    if( pNewParent == this )
        return;

    
    SystemWindow *pSysWin = ImplGetLastSystemWindow(this);
    SystemWindow *pNewSysWin = NULL;
    bool bChangeTaskPaneList = false;
    if( pSysWin && pSysWin->ImplIsInTaskPaneList( this ) )
    {
        pNewSysWin = ImplGetLastSystemWindow( pNewParent );
        if( pNewSysWin && pNewSysWin != pSysWin )
        {
            bChangeTaskPaneList = true;
            pSysWin->GetTaskPaneList()->RemoveWindow( this );
        }
    }
    
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

    bool bVisible = IsVisible();
    Show( false, SHOW_NOFOCUSCHANGE );

    
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

    
    bool bFocusOverlapWin = HasChildPathFocus( true );
    bool bFocusWin = HasChildPathFocus();
    bool bNewFrame = pNewParent->mpWindowImpl->mpFrameWindow != mpWindowImpl->mpFrameWindow;
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
        pNewParent->mpWindowImpl->mbClipChildren = true;
    ImplUpdateWindowPtr();
    if ( ImplUpdatePos() )
        ImplUpdateSysObjPos();

    
    
    
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

        
        if ( HasChildPathFocus( true ) )
            ImplCallFocusChangeActivate( pNewOverlapWindow, pOldOverlapWindow );
    }

    
    if ( bNewFrame )
    {
        if ( (GetType() == WINDOW_BORDERWINDOW) &&
             (ImplGetWindow()->GetType() == WINDOW_FLOATINGWINDOW) )
            ((ImplBorderWindow*)this)->SetDisplayActive( mpWindowImpl->mpFrameData->mbHasFocus );
    }

    
    
    if ( bFocusOverlapWin )
    {
        mpWindowImpl->mpFrameData->mpFocusWin = Application::GetFocusWindow();
        if ( !mpWindowImpl->mpFrameData->mbHasFocus )
        {
            mpWindowImpl->mpFrame->ToTop( 0 );
        }
    }

    
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



void Window::Show( bool bVisible, sal_uInt16 nFlags )
{

    if ( mpWindowImpl->mbVisible == bVisible )
        return;

    ImplDelData aDogTag( this );

    bool bRealVisibilityChanged = false;
    mpWindowImpl->mbVisible = bVisible;

    if ( !bVisible )
    {
        ImplHideAllOverlaps();
        if( aDogTag.IsDead() )
            return;

        if ( mpWindowImpl->mpBorderWindow )
        {
            bool bOldUpdate = mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate;
            if ( mpWindowImpl->mbNoParentUpdate )
                mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate = true;
            mpWindowImpl->mpBorderWindow->Show( false, nFlags );
            mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate = bOldUpdate;
        }
        else if ( mpWindowImpl->mbFrame )
        {
            mpWindowImpl->mbSuppressAccessibilityEvents = true;
            mpWindowImpl->mpFrame->Show( sal_False, sal_False );
        }

        StateChanged( STATE_CHANGE_VISIBLE );

        if ( mpWindowImpl->mbReallyVisible )
        {
            Region  aInvRegion;
            bool    bSaveBack = false;

            if ( ImplIsOverlapWindow() && !mpWindowImpl->mbFrame )
            {
                if ( ImplRestoreOverlapBackground( aInvRegion ) )
                    bSaveBack = true;
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
            
            
            ImplCallInitShow();

            
            
            if ( ImplIsOverlapWindow() && !(nFlags & SHOW_NOACTIVATE) )
            {
                ImplStartToTop(( nFlags & SHOW_FOREGROUNDTASK ) ? TOTOP_FOREGROUNDTASK : 0 );
                ImplFocusToTop( 0, false );
            }

            
            if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mbSaveBack )
                ImplSaveOverlapBackground();
            
            bRealVisibilityChanged = !mpWindowImpl->mbReallyVisible;
            ImplSetReallyVisible();

            
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
            
            ImplSVData* pSVData = ImplGetSVData();
            if ( !pSVData->mpIntroWindow )
            {
                
                GetpApp()->InitFinished();
            }
            else if ( !ImplIsWindowOrChild( pSVData->mpIntroWindow ) )
            {
                
                
                pSVData->mpIntroWindow->Hide();
            }

            
            mpWindowImpl->mbSuppressAccessibilityEvents = false;

            mpWindowImpl->mbPaintFrame = true;
            bool bNoActivate = (nFlags & (SHOW_NOACTIVATE|SHOW_NOFOCUSCHANGE)) ? sal_True : sal_False;
            mpWindowImpl->mpFrame->Show( sal_True, bNoActivate );
            if( aDogTag.IsDead() )
                return;

            
            
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
    
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    
    
    
    
    
    if ( !bRealVisibilityChanged )
        ImplCallEventListeners( mpWindowImpl->mbVisible ? VCLEVENT_WINDOW_SHOW : VCLEVENT_WINDOW_HIDE, NULL );
    if( aDogTag.IsDead() )
        return;

    
    
    
    
    
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



Size Window::GetSizePixel() const
{
    if (!mpWindowImpl)
    {
        SAL_WARN("vcl.layout", "WTF no windowimpl");
        return Size(0,0);
    }

    
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




void Window::Enable( bool bEnable, bool bChild )
{

    if ( !bEnable )
    {
        
        
        if ( IsTracking() )
            EndTracking( ENDTRACK_CANCEL );
        if ( IsMouseCaptured() )
            ReleaseMouse();
        
        
        
        
        if ( HasFocus() )
            ImplDlgCtrlNextWindow();
    }

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->Enable( bEnable, false );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->Enable( bEnable, true );
    }

    
    
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



bool Window::IsCallHandlersOnInputDisabled() const
{
    return mpWindowImpl->mbCallHandlersDuringInputDisabled ? true : false;
}



void Window::EnableInput( bool bEnable, bool bChild )
{

    bool bNotify = (bEnable != mpWindowImpl->mbInputDisabled);
    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->EnableInput( bEnable, false );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->EnableInput( bEnable, true );
    }

    if ( (! bEnable && mpWindowImpl->meAlwaysInputMode != AlwaysInputEnabled) ||
         (  bEnable && mpWindowImpl->meAlwaysInputMode != AlwaysInputDisabled) )
    {
        
        
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

    
    if ( bNotify )
    {
        NotifyEvent aNEvt( bEnable ? EVENT_INPUTENABLE : EVENT_INPUTDISABLE, this );
        Notify( aNEvt );
    }
}



void Window::EnableInput( bool bEnable, bool bChild, bool bSysWin,
                          const Window* pExcludeWindow )
{

    EnableInput( bEnable, bChild );
    if ( bSysWin )
    {
        
        
        if( pExcludeWindow )
            pExcludeWindow = pExcludeWindow->ImplGetFirstOverlapWindow();
        Window* pSysWin = mpWindowImpl->mpFrameWindow->mpWindowImpl->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            
            if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( pSysWin, true ) )
            {
                
                
                if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( pSysWin, true ) )
                    pSysWin->EnableInput( bEnable, bChild );
            }
            pSysWin = pSysWin->mpWindowImpl->mpNextOverlap;
        }

        
        Window* pFrameWin = ImplGetSVData()->maWinData.mpFirstFrame;
        while ( pFrameWin )
        {
            if( pFrameWin->ImplIsFloatingWindow() )
            {
                
                if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( pFrameWin, true ) )
                {
                    
                    
                    if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( pFrameWin, true ) )
                        pFrameWin->EnableInput( bEnable, bChild );
                }
            }
            pFrameWin = pFrameWin->mpWindowImpl->mpFrameData->mpNextFrame;
        }

        
        if( mpWindowImpl->mbFrame )
        {
            ::std::vector< Window* >& rList = mpWindowImpl->mpFrameData->maOwnerDrawList;
            ::std::vector< Window* >::iterator p = rList.begin();
            while( p != rList.end() )
            {
                
                if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( (*p), true ) )
                {
                    
                    
                    if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( (*p), true ) )
                        (*p)->EnableInput( bEnable, bChild );
                }
                ++p;
            }
        }
    }
}



void Window::AlwaysEnableInput( bool bAlways, bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->AlwaysEnableInput( bAlways, false );

    if( bAlways && mpWindowImpl->meAlwaysInputMode != AlwaysInputEnabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputEnabled;

        if ( bAlways )
            EnableInput( true, false );
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



void Window::AlwaysDisableInput( bool bAlways, bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->AlwaysDisableInput( bAlways, false );

    if( bAlways && mpWindowImpl->meAlwaysInputMode != AlwaysInputDisabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputDisabled;

        if ( bAlways )
            EnableInput( false, false );
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



void Window::SetActivateMode( sal_uInt16 nMode )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetActivateMode( nMode );

    if ( mpWindowImpl->mnActivateMode != nMode )
    {
        mpWindowImpl->mnActivateMode = nMode;

        
        if ( mpWindowImpl->mnActivateMode )
        {
            if ( (mpWindowImpl->mbActive || (GetType() == WINDOW_BORDERWINDOW)) &&
                 !HasChildPathFocus( true ) )
            {
                mpWindowImpl->mbActive = false;
                Deactivate();
            }
        }
        else
        {
            if ( !mpWindowImpl->mbActive || (GetType() == WINDOW_BORDERWINDOW) )
            {
                mpWindowImpl->mbActive = true;
                Activate();
            }
        }
    }
}



void Window::ToTop( sal_uInt16 nFlags )
{

    ImplStartToTop( nFlags );
    ImplFocusToTop( nFlags, IsReallyVisible() );
}



void Window::SetZOrder( Window* pRefWindow, sal_uInt16 nFlags )
{

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
        
        if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
            ImplInvalidateAllOverlapBackgrounds();

        if ( mpWindowImpl->mbInitWinClipRegion || !mpWindowImpl->maWinClipRegion.IsEmpty() )
        {
            bool bInitWinClipRegion = mpWindowImpl->mbInitWinClipRegion;
            ImplSetClipFlag();

            
            
            
            
            
            
            
            if ( !bInitWinClipRegion )
            {
                
                
                Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                Window*     pWindow = NULL;
                if ( ImplIsOverlapWindow() )
                {
                    if ( mpWindowImpl->mpOverlapWindow )
                        pWindow = mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
                }
                else
                    pWindow = ImplGetParent()->mpWindowImpl->mpFirstChild;
                
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



void Window::EnableAlwaysOnTop( bool bEnable )
{

    mpWindowImpl->mbAlwaysOnTop = bEnable;

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->EnableAlwaysOnTop( bEnable );
    else if ( bEnable && IsReallyVisible() )
        ToTop();

    if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetAlwaysOnTop( bEnable );
}



void Window::setPosSizePixel( long nX, long nY,
                              long nWidth, long nHeight, sal_uInt16 nFlags )
{

    bool bHasValidSize = !mpWindowImpl->mbDefSize;

    if ( nFlags & WINDOW_POSSIZE_POS )
        mpWindowImpl->mbDefPos = false;
    if ( nFlags & WINDOW_POSSIZE_SIZE )
        mpWindowImpl->mbDefSize = false;

    
    Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;

    if ( pWindow->mpWindowImpl->mbFrame )
    {
        
        
        
        long nOldWidth  = pWindow->mnOutWidth;

        if ( !(nFlags & WINDOW_POSSIZE_WIDTH) )
            nWidth = pWindow->mnOutWidth;
        if ( !(nFlags & WINDOW_POSSIZE_HEIGHT) )
            nHeight = pWindow->mnOutHeight;


        sal_uInt16 nSysFlags=0;
        Window *pParent = GetParent();

        if( nFlags & WINDOW_POSSIZE_WIDTH )
            nSysFlags |= SAL_FRAME_POSSIZE_WIDTH;
        if( nFlags & WINDOW_POSSIZE_HEIGHT )
            nSysFlags |= SAL_FRAME_POSSIZE_HEIGHT;
        if( nFlags & WINDOW_POSSIZE_X )
        {
            nSysFlags |= SAL_FRAME_POSSIZE_X;
            if( pParent && (pWindow->GetStyle() & WB_SYSTEMCHILDWINDOW) )
            {
                nX += pParent->mnOutOffX;
            }
            if( pParent && pParent->ImplIsAntiparallel() )
            {
                
                Rectangle aRect( Point ( nX, nY ), Size( nWidth, nHeight ) );
                const OutputDevice *pParentOutDev = pParent->GetOutDev();
                pParentOutDev->ReMirror( aRect );
                nX = aRect.Left();
            }
        }
        if( !(nFlags & WINDOW_POSSIZE_X) && bHasValidSize && pWindow->mpWindowImpl->mpFrame->maGeometry.nWidth )
        {
            
            
            if ( pParent )
            {
                OutputDevice *pParentOutDev = pParent->GetOutDev();
                if( pParentOutDev->HasMirroredGraphics() )
                {
                    long myWidth = nOldWidth;
                    if( !myWidth )
                        myWidth = mpWindowImpl->mpFrame->GetUnmirroredGeometry().nWidth;
                    if( !myWidth )
                        myWidth = nWidth;
                    nFlags |= WINDOW_POSSIZE_X;
                    nSysFlags |= SAL_FRAME_POSSIZE_X;
                    nX = mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX - pParent->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX -
                        mpWindowImpl->mpFrame->GetUnmirroredGeometry().nLeftDecoration;
                    nX = pParent->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX - mpWindowImpl->mpFrame->GetUnmirroredGeometry().nLeftDecoration +
                        pParent->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nWidth - myWidth - 1 - mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX;
                    if(!(nFlags & WINDOW_POSSIZE_Y))
                    {
                        nFlags |= WINDOW_POSSIZE_Y;
                        nSysFlags |= SAL_FRAME_POSSIZE_Y;
                        nY = mpWindowImpl->mpFrame->GetUnmirroredGeometry().nY - pWindow->GetParent()->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nY -
                            mpWindowImpl->mpFrame->GetUnmirroredGeometry().nTopDecoration;
                    }
                }
            }
        }
        if( nFlags & WINDOW_POSSIZE_Y )
        {
            nSysFlags |= SAL_FRAME_POSSIZE_Y;
            if( pParent && (pWindow->GetStyle() & WB_SYSTEMCHILDWINDOW) )
            {
                nY += pParent->mnOutOffY;
            }
        }

        if( nSysFlags & (SAL_FRAME_POSSIZE_WIDTH|SAL_FRAME_POSSIZE_HEIGHT) )
        {
            
            
            
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

        
        
        
        
        ImplHandleResize( pWindow, nWidth, nHeight );
    }
    else
    {
        pWindow->ImplPosSizeWindow( nX, nY, nWidth, nHeight, nFlags );
        if ( IsReallyVisible() )
            ImplGenerateMouseMove();
    }
}



Point Window::GetPosPixel() const
{
    return mpWindowImpl->maPos;
}



Rectangle Window::GetDesktopRectPixel() const
{
    Rectangle rRect;
    mpWindowImpl->mpFrameWindow->mpWindowImpl->mpFrame->GetWorkArea( rRect );
    return rRect;
}



Point Window::OutputToScreenPixel( const Point& rPos ) const
{
    
    return Point( rPos.X()+mnOutOffX, rPos.Y()+mnOutOffY );
}



Point Window::ScreenToOutputPixel( const Point& rPos ) const
{
    
    return Point( rPos.X()-mnOutOffX, rPos.Y()-mnOutOffY );
}



long Window::ImplGetUnmirroredOutOffX()
{
    
    long offx = mnOutOffX;
    OutputDevice *pOutDev = GetOutDev();
    if( pOutDev->HasMirroredGraphics() )
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


Point Window::OutputToNormalizedScreenPixel( const Point& rPos ) const
{
    
    long offx = ((Window*) this)->ImplGetUnmirroredOutOffX();
    return Point( rPos.X()+offx, rPos.Y()+mnOutOffY );
}

Point Window::NormalizedScreenToOutputPixel( const Point& rPos ) const
{
    
    long offx = ((Window*) this)->ImplGetUnmirroredOutOffX();
    return Point( rPos.X()-offx, rPos.Y()-mnOutOffY );
}



Point Window::OutputToAbsoluteScreenPixel( const Point& rPos ) const
{
    
    Point p = OutputToScreenPixel( rPos );
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
    p.X() += g.nX;
    p.Y() += g.nY;
    return p;
}



Point Window::AbsoluteScreenToOutputPixel( const Point& rPos ) const
{
    
    Point p = ScreenToOutputPixel( rPos );
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
    p.X() -= g.nX;
    p.Y() -= g.nY;
    return p;
}



Rectangle Window::ImplOutputToUnmirroredAbsoluteScreenPixel( const Rectangle &rRect ) const
{
    
    
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetUnmirroredGeometry();

    Point p1 = OutputToScreenPixel( rRect.TopRight() );
    p1.X() = g.nX+g.nWidth-p1.X();
    p1.Y() += g.nY;

    Point p2 = OutputToScreenPixel( rRect.BottomLeft() );
    p2.X() = g.nX+g.nWidth-p2.X();
    p2.Y() += g.nY;

    return Rectangle( p1, p2 );
}




Rectangle Window::GetWindowExtentsRelative( Window *pRelativeWindow ) const
{
    
    return ImplGetWindowExtentsRelative( pRelativeWindow, false );
}

Rectangle Window::GetClientWindowExtentsRelative( Window *pRelativeWindow ) const
{
    
    return ImplGetWindowExtentsRelative( pRelativeWindow, true );
}



Rectangle Window::ImplGetWindowExtentsRelative( Window *pRelativeWindow, bool bClientOnly ) const
{
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
    
    
    const Window *pWin = (!bClientOnly && mpWindowImpl->mpBorderWindow) ? mpWindowImpl->mpBorderWindow : this;

    Point aPos( pWin->OutputToScreenPixel( Point(0,0) ) );
    aPos.X() += g.nX;
    aPos.Y() += g.nY;
    Size aSize ( pWin->GetSizePixel() );
    
    if( !bClientOnly && (mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame && GetType() != WINDOW_WORKWINDOW)) )
    {
        aPos.X() -= g.nLeftDecoration;
        aPos.Y() -= g.nTopDecoration;
        aSize.Width() += g.nLeftDecoration + g.nRightDecoration;
        aSize.Height() += g.nTopDecoration + g.nBottomDecoration;
    }
    if( pRelativeWindow )
    {
        
        Window *pRelWin = (!bClientOnly && pRelativeWindow->mpWindowImpl->mpBorderWindow) ? pRelativeWindow->mpWindowImpl->mpBorderWindow : pRelativeWindow;
        aPos = pRelWin->AbsoluteScreenToOutputPixel( aPos );
    }
    return Rectangle( aPos, aSize );
}



void Window::Scroll( long nHorzScroll, long nVertScroll, sal_uInt16 nFlags )
{

    ImplScroll( Rectangle( Point( mnOutOffX, mnOutOffY ),
                           Size( mnOutWidth, mnOutHeight ) ),
                nHorzScroll, nVertScroll, nFlags & ~SCROLL_CLIP );
}



void Window::Scroll( long nHorzScroll, long nVertScroll,
                     const Rectangle& rRect, sal_uInt16 nFlags )
{

    OutputDevice *pOutDev = GetOutDev();
    Rectangle aRect = pOutDev->ImplLogicToDevicePixel( rRect );
    aRect.Intersection( Rectangle( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) ) );
    if ( !aRect.IsEmpty() )
        ImplScroll( aRect, nHorzScroll, nVertScroll, nFlags );
}



void Window::Invalidate( sal_uInt16 nFlags )
{

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    ImplInvalidate( NULL, nFlags );
}



void Window::Invalidate( const Rectangle& rRect, sal_uInt16 nFlags )
{

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;


    OutputDevice *pOutDev = GetOutDev();
    Rectangle aRect = pOutDev->ImplLogicToDevicePixel( rRect );
    if ( !aRect.IsEmpty() )
    {
        Region aRegion( aRect );
        ImplInvalidate( &aRegion, nFlags );
    }
}



void Window::Invalidate( const Region& rRegion, sal_uInt16 nFlags )
{

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



void Window::Validate( sal_uInt16 nFlags )
{

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    ImplValidate( NULL, nFlags );
}



bool Window::HasPaintEvent() const
{

    if ( !mpWindowImpl->mbReallyVisible )
        return false;

    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
        return true;

    if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINT )
        return true;

    if ( !ImplIsOverlapWindow() )
    {
        const Window* pTempWindow = this;
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINTCHILDREN | IMPL_PAINT_PAINTALLCHILDREN) )
                return true;
        }
        while ( !pTempWindow->ImplIsOverlapWindow() );
    }

    return false;
}



void Window::Update()
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->Update();
        return;
    }

    if ( !mpWindowImpl->mbReallyVisible )
        return;

    bool bFlush = false;
    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) )
            bFlush = true;
    }

    
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

    
    if ( pUpdateWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDREN) )
    {
        
        
         Window* pUpdateOverlapWindow = ImplGetFirstOverlapWindow()->mpWindowImpl->mpFirstOverlap;
         while ( pUpdateOverlapWindow )
         {
             pUpdateOverlapWindow->Update();
             pUpdateOverlapWindow = pUpdateOverlapWindow->mpWindowImpl->mpNext;
         }

        pUpdateWindow->ImplCallPaint( NULL, pUpdateWindow->mpWindowImpl->mnPaintFlags );
        bFlush = true;
    }

    if ( bFlush )
        Flush();
}



void Window::Flush()
{

    const Rectangle aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
    mpWindowImpl->mpFrame->Flush( aWinRect );
}



void Window::Sync()
{

    mpWindowImpl->mpFrame->Sync();
}



void Window::SetUpdateMode( bool bUpdate )
{

    mpWindowImpl->mbNoUpdate = !bUpdate;
    StateChanged( STATE_CHANGE_UPDATEMODE );
}



void Window::GrabFocus()
{

    ImplGrabFocus( 0 );
}



bool Window::HasFocus() const
{

    
    
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



void Window::GrabFocusToDocument()
{
    ImplGrabFocusToDocument(0);
}

void Window::SetFakeFocus( bool bFocus )
{
    ImplGetWindowImpl()->mbFakeFocusSet = bFocus;
}



bool Window::HasChildPathFocus( bool bSystemWindow ) const
{

    
    
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
    return false;
}



void Window::CaptureMouse()
{

    ImplSVData* pSVData = ImplGetSVData();

    
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



void Window::ReleaseMouse()
{

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



bool Window::IsMouseCaptured() const
{

    return (this == ImplGetSVData()->maWinData.mpCaptureWin);
}



void Window::SetPointer( const Pointer& rPointer )
{

    if ( mpWindowImpl->maPointer == rPointer )
        return;

    mpWindowImpl->maPointer   = rPointer;

    
    if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
}



void Window::EnableChildPointerOverwrite( bool bOverwrite )
{

    if ( mpWindowImpl->mbChildPtrOverwrite == bOverwrite )
        return;

    mpWindowImpl->mbChildPtrOverwrite  = bOverwrite;

    
    if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
}



void Window::SetPointerPosPixel( const Point& rPos )
{
    Point aPos = ImplOutputToFrame( rPos );
    const OutputDevice *pOutDev = GetOutDev();
    if( pOutDev->HasMirroredGraphics() )
    {
        if( !IsRTLEnabled() )
        {
            
            pOutDev->ReMirror( aPos );
        }
        
        mpGraphics->mirror( aPos.X(), this );
    }
    else if( ImplIsAntiparallel() )
    {
        pOutDev->ReMirror( aPos );
    }
    mpWindowImpl->mpFrame->SetPointerPos( aPos.X(), aPos.Y() );
}



Point Window::GetPointerPosPixel()
{

    Point aPos( mpWindowImpl->mpFrameData->mnLastMouseX, mpWindowImpl->mpFrameData->mnLastMouseY );
    if( ImplIsAntiparallel() )
    {
        
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aPos );
    }
    return ImplFrameToOutput( aPos );
}



Point Window::GetLastPointerPosPixel()
{

    Point aPos( mpWindowImpl->mpFrameData->mnBeforeLastMouseX, mpWindowImpl->mpFrameData->mnBeforeLastMouseY );
    if( ImplIsAntiparallel() )
    {
        
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aPos );
    }
    return ImplFrameToOutput( aPos );
}



void Window::ShowPointer( bool bVisible )
{

    if ( mpWindowImpl->mbNoPtrVisible != !bVisible )
    {
        mpWindowImpl->mbNoPtrVisible = !bVisible;

        
        if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
            mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
    }
}



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
            
            const OutputDevice *pOutDev = GetOutDev();
            pOutDev->ReMirror( aSalPointerState.maPos );
        }
        aState.maPos = ImplFrameToOutput( aSalPointerState.maPos );
        aState.mnState = aSalPointerState.mnState;
    }
    return aState;
}



bool Window::IsMouseOver()
{
    return ImplGetWinData()->mbMouseOver;
}



void Window::EnterWait()
{

    mpWindowImpl->mnWaitCount++;

    if ( mpWindowImpl->mnWaitCount == 1 )
    {
        
        if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
            mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
    }
}



void Window::LeaveWait()
{

    if ( mpWindowImpl->mnWaitCount )
    {
        mpWindowImpl->mnWaitCount--;

        if ( !mpWindowImpl->mnWaitCount )
        {
            
            if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
                mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
        }
    }
}



void Window::SetCursor( Cursor* pCursor )
{

    if ( mpWindowImpl->mpCursor != pCursor )
    {
        if ( mpWindowImpl->mpCursor )
            mpWindowImpl->mpCursor->ImplHide( true );
        mpWindowImpl->mpCursor = pCursor;
        if ( pCursor )
            pCursor->ImplShow();
    }
}



void Window::SetText( const OUString& rStr )
{
    if (rStr == mpWindowImpl->maText)
        return;


    OUString oldTitle( mpWindowImpl->maText );
    mpWindowImpl->maText = rStr;

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetText( rStr );
    else if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetTitle( rStr );

    ImplCallEventListeners( VCLEVENT_WINDOW_FRAMETITLECHANGED, &oldTitle );

    
    
    
    
    if ( IsReallyVisible() )
    {
        Window* pWindow = GetAccessibleRelationLabelFor();
        if ( pWindow && pWindow != this )
            pWindow->ImplCallEventListeners( VCLEVENT_WINDOW_FRAMETITLECHANGED, &oldTitle );
    }

    StateChanged( STATE_CHANGE_TEXT );
}



OUString Window::GetText() const
{

    return mpWindowImpl->maText;
}



OUString Window::GetDisplayText() const
{

    return GetText();
}



const Wallpaper& Window::GetDisplayBackground() const
{
    
    
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



const OUString& Window::GetHelpText() const
{

    OUString aStrHelpId( OStringToOUString( GetHelpId(), RTL_TEXTENCODING_UTF8 ) );
    bool bStrHelpId = !aStrHelpId.isEmpty();

    if ( !mpWindowImpl->maHelpText.getLength() && bStrHelpId )
    {
        if ( !IsDialog() && (mpWindowImpl->mnType != WINDOW_TABPAGE) && (mpWindowImpl->mnType != WINDOW_FLOATINGWINDOW) )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                ((Window*)this)->mpWindowImpl->maHelpText = pHelp->GetHelpText( aStrHelpId, this );
                mpWindowImpl->mbHelpTextDynamic = false;
            }
        }
    }
    else if( mpWindowImpl->mbHelpTextDynamic && bStrHelpId )
    {
        static const char* pEnv = getenv( "HELP_DEBUG" );
        if( pEnv && *pEnv )
        {
            OUStringBuffer aTxt( 64+mpWindowImpl->maHelpText.getLength() );
            aTxt.append( mpWindowImpl->maHelpText );
            aTxt.appendAscii( "\n------------------\n" );
            aTxt.append( OUString( aStrHelpId ) );
            mpWindowImpl->maHelpText = aTxt.makeStringAndClear();
        }
        mpWindowImpl->mbHelpTextDynamic = false;
    }

    return mpWindowImpl->maHelpText;
}



Window* Window::FindWindow( const Point& rPos ) const
{

    Point aPos = OutputToScreenPixel( rPos );
    return ((Window*)this)->ImplFindWindow( aPos );
}



sal_uInt16 Window::GetChildCount() const
{

    sal_uInt16  nChildCount = 0;
    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        nChildCount++;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    return nChildCount;
}



Window* Window::GetChild( sal_uInt16 nChild ) const
{

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



Window* Window::GetWindow( sal_uInt16 nType ) const
{

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



bool Window::IsChild( const Window* pWindow, bool bSystemWindow ) const
{

    do
    {
        if ( !bSystemWindow && pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();

        if ( pWindow == this )
            return true;
    }
    while ( pWindow );

    return false;
}



bool Window::IsWindowOrChild( const Window* pWindow, bool bSystemWindow ) const
{

    if ( this == pWindow )
        return true;
    return ImplIsChild( pWindow, bSystemWindow );
}



const SystemEnvData* Window::GetSystemData() const
{

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



void Window::SetWindowPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xPeer, VCLXWindow* pVCLXWindow  )
{
    
    
    mpWindowImpl->mxWindowPeer.clear();
    mpWindowImpl->mxWindowPeer = xPeer;

    mpWindowImpl->mpVCLXWindow = pVCLXWindow;
}



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



void Window::SetComponentInterface( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xIFace )
{
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    DBG_ASSERT( pWrapper, "SetComponentInterface: No Wrapper!" );
    if ( pWrapper )
        pWrapper->SetWindowInterface( this, xIFace );
}



void Window::ImplCallDeactivateListeners( Window *pNew )
{
    
    if ( !pNew || !ImplIsChild( pNew ) )
    {
        ImplDelData aDogtag( this );
        ImplCallEventListeners( VCLEVENT_WINDOW_DEACTIVATE );
        if( aDogtag.IsDead() )
            return;

        
        
        if ( ImplGetParent() && mpWindowImpl->mpFrameWindow == ImplGetParent()->mpWindowImpl->mpFrameWindow )
            ImplGetParent()->ImplCallDeactivateListeners( pNew );
    }
}



void Window::ImplCallActivateListeners( Window *pOld )
{
    
    if ( !pOld || !ImplIsChild( pOld ) )
    {
        ImplDelData aDogtag( this );
        ImplCallEventListeners( VCLEVENT_WINDOW_ACTIVATE, pOld );
        if( aDogtag.IsDead() )
            return;

        
        
        
        
        
            
            
            
            
        if ( ImplGetParent() ) /* && mpWindowImpl->mpFrameWindow == ImplGetParent()->mpWindowImpl->mpFrameWindow ) */
            ImplGetParent()->ImplCallActivateListeners( pOld );
        else if( (mpWindowImpl->mnStyle & WB_INTROWIN) == 0 )
        {
            
            ImplGetSVData()->maWinData.mpActiveApplicationFrame = mpWindowImpl->mpFrameWindow;
        }
        
    }
}



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



void Window::ImplStartDnd()
{
    GetDropTarget();
}



uno::Reference< XDropTarget > Window::GetDropTarget()
{

    if( ! mpWindowImpl->mxDNDListenerContainer.is() )
    {
        sal_Int8 nDefaultActions = 0;

        if( mpWindowImpl->mpFrameData )
        {
            if( ! mpWindowImpl->mpFrameData->mxDropTarget.is() )
            {
                
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

                        
                        uno::Reference< XDragGestureRecognizer > xDragGestureRecognizer =
                            uno::Reference< XDragGestureRecognizer > (mpWindowImpl->mpFrameData->mxDragSource, UNO_QUERY);

                        if( xDragGestureRecognizer.is() )
                        {
                            xDragGestureRecognizer->addDragGestureListener(
                                uno::Reference< XDragGestureListener > (mpWindowImpl->mpFrameData->mxDropTargetListener, UNO_QUERY));
                        }
                        else
                            mpWindowImpl->mpFrameData->mbInternalDragGestureRecognizer = true;

                    }
                    catch (const RuntimeException&)
                    {
                        
                        mpWindowImpl->mpFrameData->mxDropTarget.clear();
                        mpWindowImpl->mpFrameData->mxDragSource.clear();
                    }
                }
            }

        }

        mpWindowImpl->mxDNDListenerContainer = static_cast < XDropTarget * > ( new DNDListenerContainer( nDefaultActions ) );
    }

    
    return uno::Reference< XDropTarget > ( mpWindowImpl->mxDNDListenerContainer, UNO_QUERY );
}



uno::Reference< XDragSource > Window::GetDragSource()
{

#if HAVE_FEATURE_DESKTOP

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxDragSource.is() )
        {
            try
            {
                uno::Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
                const SystemEnvData * pEnvData = GetSystemData();

                if( pEnvData )
                {
                    Sequence< Any > aDragSourceAL( 2 ), aDropTargetAL( 2 );
                    OUString aDragSourceSN, aDropTargetSN;
#if defined WNT
                    aDragSourceSN = "com.sun.star.datatransfer.dnd.OleDragSource";
                    aDropTargetSN = "com.sun.star.datatransfer.dnd.OleDropTarget";
                    aDragSourceAL[ 1 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->hWnd) ) );
                    aDropTargetAL[ 0 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->hWnd) ) );
#elif defined MACOSX
            /* FIXME: Mac OS X specific dnd interface does not exist! *
             * Using Windows based dnd as a temporary solution        */
                    aDragSourceSN = "com.sun.star.datatransfer.dnd.OleDragSource";
                    aDropTargetSN = "com.sun.star.datatransfer.dnd.OleDropTarget";
                    aDragSourceAL[ 1 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->mpNSView) ) );
                    aDropTargetAL[ 0 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->mpNSView) ) );
#elif HAVE_FEATURE_X11
                    aDragSourceSN = "com.sun.star.datatransfer.dnd.X11DragSource";
                    aDropTargetSN = "com.sun.star.datatransfer.dnd.X11DropTarget";

                    aDragSourceAL[ 0 ] = makeAny( Application::GetDisplayConnection() );
                    aDropTargetAL[ 0 ] = makeAny( Application::GetDisplayConnection() );
                    aDropTargetAL[ 1 ] = makeAny( (sal_Size)(pEnvData->aShellWindow) );
#endif
                    if( !aDragSourceSN.isEmpty() )
                        mpWindowImpl->mpFrameData->mxDragSource.set(
                            xContext->getServiceManager()->createInstanceWithArgumentsAndContext( aDragSourceSN, aDragSourceAL, xContext ),
                            UNO_QUERY );

                    if( !aDropTargetSN.isEmpty() )
                        mpWindowImpl->mpFrameData->mxDropTarget.set(
                           xContext->getServiceManager()->createInstanceWithArgumentsAndContext( aDropTargetSN, aDropTargetAL, xContext ),
                           UNO_QUERY );
                }
            }

            
            catch (const Exception&)
            {
                
                mpWindowImpl->mpFrameData->mxDropTarget.clear();
                mpWindowImpl->mpFrameData->mxDragSource.clear();
            }
        }

        return mpWindowImpl->mpFrameData->mxDragSource;
    }
#endif
    return uno::Reference< XDragSource > ();
}



uno::Reference< XDragGestureRecognizer > Window::GetDragGestureRecognizer()
{
    return uno::Reference< XDragGestureRecognizer > ( GetDropTarget(), UNO_QUERY );
}



uno::Reference< XClipboard > Window::GetClipboard()
{

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxClipboard.is() )
        {
            try
            {
                mpWindowImpl->mpFrameData->mxClipboard
                    = css::datatransfer::clipboard::SystemClipboard::create(
                        comphelper::getProcessComponentContext());
            }
            catch (css::uno::DeploymentException & e)
            {
                SAL_WARN(
                    "vcl.window",
                    "ignoring DeploymentException \"" << e.Message << "\"");
            }
        }

        return mpWindowImpl->mpFrameData->mxClipboard;
    }

    return static_cast < XClipboard * > (0);
}



uno::Reference< XClipboard > Window::GetPrimarySelection()
{

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxSelection.is() )
        {
            try
            {
                uno::Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );

#if HAVE_FEATURE_X11
                
                
                css::uno::Sequence<css::uno::Any> args(1);
                args[0] <<= OUString("PRIMARY");
                mpWindowImpl->mpFrameData->mxSelection.set(
                    (xContext->getServiceManager()->
                     createInstanceWithArgumentsAndContext(
                         "com.sun.star.datatransfer.clipboard.SystemClipboard",
                         args, xContext)),
                    css::uno::UNO_QUERY_THROW);
#       else
                static uno::Reference< XClipboard > s_xSelection(
                    xContext->getServiceManager()->createInstanceWithContext( "com.sun.star.datatransfer.clipboard.GenericClipboard", xContext ), UNO_QUERY );

                mpWindowImpl->mpFrameData->mxSelection = s_xSelection;
#       endif
            }
            catch (css::uno::RuntimeException & e)
            {
                SAL_WARN(
                    "vcl.window",
                    "ignoring RuntimeException \"" << e.Message << "\"");
            }
        }

        return mpWindowImpl->mpFrameData->mxSelection;
    }

    return static_cast < XClipboard * > (0);
}





::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > Window::GetAccessible( bool bCreate )
{
    
    /* 
    if ( GetParent() && ( GetType() == WINDOW_BORDERWINDOW ) && ( GetChildCount() == 1 ) )
    
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


bool Window::ImplIsAccessibleCandidate() const
{
    if( !mpWindowImpl->mbBorderWin )
        return true;
    else
        
        if( mpWindowImpl->mbFrame && mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE) )
            return true;
        else
            return false;
}

bool Window::ImplIsAccessibleNativeFrame() const
{
    if( mpWindowImpl->mbFrame )
        
        if( (mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE)) )
            return true;
        else
            return false;
    else
        return false;
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

Window* Window::ImplGetAccessibleCandidateChild( sal_uInt16 nChild, sal_uInt16& rChildCount, sal_uInt16 nFirstWindowType, bool bTopLevel ) const
{

    if( bTopLevel )
        rChildCount = 0;

    Window* pChild = GetWindow( nFirstWindowType );
    while ( pChild )
    {
        Window *pTmpChild = pChild;

        if( !pChild->ImplIsAccessibleCandidate() )
            pTmpChild = pChild->ImplGetAccessibleCandidateChild( nChild, rChildCount, WINDOW_FIRSTCHILD, false );

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
        
        Window *pWorkWin = GetParent()->mpWindowImpl->mpFirstChild;
        while( pWorkWin && (pWorkWin == this) )
            pWorkWin = pWorkWin->mpWindowImpl->mpNext;
        pParent = pWorkWin;
    }
    
    
    //
    
    
    
    else if( GetType() == WINDOW_FLOATINGWINDOW &&
        mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame &&
        !PopupMenuFloatingWindow::isPopupMenu(this))
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

sal_uInt16 Window::getDefaultAccessibleRole() const
{
    sal_uInt16 nRole = 0xFFFF;
    switch ( GetType() )
    {
        case WINDOW_MESSBOX:    
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
        case WINDOW_MOREBUTTON:
        case WINDOW_SPINBUTTON:
        case WINDOW_BUTTON: nRole = accessibility::AccessibleRole::PUSH_BUTTON; break;
        case WINDOW_MENUBUTTON: nRole = accessibility::AccessibleRole::BUTTON_MENU; break;

        case WINDOW_PATHDIALOG: nRole = accessibility::AccessibleRole::DIRECTORY_PANE; break;
        case WINDOW_FILEDIALOG: nRole = accessibility::AccessibleRole::FILE_CHOOSER; break;
        case WINDOW_COLORDIALOG: nRole = accessibility::AccessibleRole::COLOR_CHOOSER; break;
        case WINDOW_FONTDIALOG: nRole = accessibility::AccessibleRole::FONT_CHOOSER; break;

        case WINDOW_RADIOBUTTON: nRole = accessibility::AccessibleRole::RADIO_BUTTON; break;
        case WINDOW_TRISTATEBOX:
        case WINDOW_CHECKBOX: nRole = accessibility::AccessibleRole::CHECK_BOX; break;

        case WINDOW_MULTILINEEDIT: nRole = accessibility::AccessibleRole::SCROLL_PANE; break;

        case WINDOW_PATTERNFIELD:
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
        case WINDOW_FIXEDLINE:
            if( !GetText().isEmpty() )
                nRole = accessibility::AccessibleRole::LABEL;
            else
                nRole = accessibility::AccessibleRole::SEPARATOR;
            break;

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

        case WINDOW_NUMERICFIELD:
        case WINDOW_METRICFIELD:
        case WINDOW_CURRENCYFIELD:
        case WINDOW_LONGCURRENCYFIELD:
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

        case WINDOW_RULER: nRole = accessibility::AccessibleRole::RULER; break;

        case WINDOW_SCROLLWINDOW: nRole = accessibility::AccessibleRole::SCROLL_PANE; break;

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
                nRole = accessibility::AccessibleRole::WINDOW;      
            else
                
                
                
                nRole = accessibility::AccessibleRole::PANEL;
    }
    return nRole;
}

sal_uInt16 Window::GetAccessibleRole() const
{
    using namespace ::com::sun::star;

    sal_uInt16 nRole = mpWindowImpl->mpAccessibleInfos ? mpWindowImpl->mpAccessibleInfos->nAccessibleRole : 0xFFFF;
    if ( nRole == 0xFFFF )
        nRole = getDefaultAccessibleRole();
    return nRole;
}

void Window::SetAccessibleName( const OUString& rName )
{
   if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;

    OUString oldName = GetAccessibleName();

    delete mpWindowImpl->mpAccessibleInfos->pAccessibleName;
    mpWindowImpl->mpAccessibleInfos->pAccessibleName = new OUString( rName );

    ImplCallEventListeners( VCLEVENT_WINDOW_FRAMETITLECHANGED, &oldName );
}

OUString Window::GetAccessibleName() const
{
    if (mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pAccessibleName)
        return *mpWindowImpl->mpAccessibleInfos->pAccessibleName;
    return getDefaultAccessibleName();
}

OUString Window::getDefaultAccessibleName() const
{
    OUString aAccessibleName;
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
            if (aAccessibleName.isEmpty())
                aAccessibleName = GetQuickHelpText();
        }
        break;

        case WINDOW_IMAGEBUTTON:
        case WINDOW_PUSHBUTTON:
            aAccessibleName = GetText();
            if (aAccessibleName.isEmpty())
            {
                aAccessibleName = GetQuickHelpText();
                if (aAccessibleName.isEmpty())
                    aAccessibleName = GetHelpText();
            }
        break;

        case WINDOW_TOOLBOX:
            aAccessibleName = GetText();
            if( aAccessibleName.isEmpty() )
                aAccessibleName = "Tool Bar";
            break;

        case WINDOW_MOREBUTTON:
            aAccessibleName = mpWindowImpl->maText;
            break;

        default:
            aAccessibleName = GetText();
            break;
    }

    return GetNonMnemonicString( aAccessibleName );
}

void Window::SetAccessibleDescription( const OUString& rDescription )
{
   if ( ! mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;

    DBG_ASSERT( !mpWindowImpl->mpAccessibleInfos->pAccessibleDescription, "AccessibleDescription already set!" );
    delete mpWindowImpl->mpAccessibleInfos->pAccessibleDescription;
    mpWindowImpl->mpAccessibleInfos->pAccessibleDescription = new OUString( rDescription );
}

OUString Window::GetAccessibleDescription() const
{
    OUString aAccessibleDescription;
    if ( mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pAccessibleDescription )
    {
        aAccessibleDescription = *mpWindowImpl->mpAccessibleInfos->pAccessibleDescription;
    }
    else
    {
        
        
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

void Window::SetAccessibleRelationLabelFor( Window* pLabelFor )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;
    mpWindowImpl->mpAccessibleInfos->pLabelForWindow = pLabelFor;
}

void Window::SetAccessibleRelationMemberOf( Window* pMemberOfWin )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;
    mpWindowImpl->mpAccessibleInfos->pMemberOfWindow = pMemberOfWin;
}

Window* Window::GetAccessibleRelationMemberOf() const
{
    if (mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pMemberOfWindow)
        return mpWindowImpl->mpAccessibleInfos->pMemberOfWindow;

    if (!isContainerWindow(this) && !isContainerWindow(GetParent()))
        return getLegacyNonLayoutAccessibleRelationMemberOf();

    return NULL;
}

Window* Window::getAccessibleRelationLabelFor() const
{
    if (mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pLabelForWindow)
        return mpWindowImpl->mpAccessibleInfos->pLabelForWindow;

    return NULL;
}

Window* Window::GetAccessibleRelationLabelFor() const
{
    Window* pWindow = getAccessibleRelationLabelFor();

    if (pWindow)
        return pWindow;

    if (!isContainerWindow(this) && !isContainerWindow(GetParent()))
        return getLegacyNonLayoutAccessibleRelationLabelFor();

    return NULL;
}

Window* Window::GetAccessibleRelationLabeledBy() const
{
    if (mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pLabeledByWindow)
        return mpWindowImpl->mpAccessibleInfos->pLabeledByWindow;

    std::vector<FixedText*> aMnemonicLabels(list_mnemonic_labels());
    if (!aMnemonicLabels.empty())
    {
        
        for (std::vector<FixedText*>::iterator
            aI = aMnemonicLabels.begin(), aEnd = aMnemonicLabels.end(); aI != aEnd; ++aI)
        {
            Window *pCandidate = *aI;
            if (pCandidate->IsVisible())
                return pCandidate;
        }
        return aMnemonicLabels[0];
    }

    if (!isContainerWindow(this) && !isContainerWindow(GetParent()))
        return getLegacyNonLayoutAccessibleRelationLabeledBy();

    return NULL;
}

bool Window::IsAccessibilityEventsSuppressed( bool bTraverseParentPath )
{
    if( !bTraverseParentPath )
        return mpWindowImpl->mbSuppressAccessibilityEvents;
    else
    {
        Window *pParent = this;
        while ( pParent && pParent->mpWindowImpl)
        {
            if( pParent->mpWindowImpl->mbSuppressAccessibilityEvents )
                return true;
            else
                pParent = pParent->mpWindowImpl->mpParent; 
        }
        return false;
    }
}

void Window::SetAccessibilityEventsSuppressed(bool bSuppressed)
{
    mpWindowImpl->mbSuppressAccessibilityEvents = bSuppressed;
}

void Window::RecordLayoutData( vcl::ControlLayoutData* pLayout, const Rectangle& rRect )
{
    if( ! mpOutDevData )
    {
        OutputDevice *pOutDev = GetOutDev();
        pOutDev->ImplInitOutDevData();
    }
    assert(mpOutDevData);
    mpOutDevData->mpRecordLayout = pLayout;
    mpOutDevData->maRecordRect = rRect;
    Paint( rRect );
    mpOutDevData->mpRecordLayout = NULL;
}



void Window::DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, bool bChecked, bool bDrawBorder, bool bDrawExtBorderOnly )
{
    DrawSelectionBackground( rRect, highlight, bChecked, bDrawBorder, bDrawExtBorderOnly, 0, NULL, NULL );
}

void Window::DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, bool bChecked, bool bDrawBorder, bool bDrawExtBorderOnly, Color* pSelectionTextColor )
{
    DrawSelectionBackground( rRect, highlight, bChecked, bDrawBorder, bDrawExtBorderOnly, 0, pSelectionTextColor, NULL );
}

void Window::DrawSelectionBackground( const Rectangle& rRect,
                                      sal_uInt16 highlight,
                                      bool bChecked,
                                      bool bDrawBorder,
                                      bool bDrawExtBorderOnly,
                                      long nCornerRadius,
                                      Color* pSelectionTextColor,
                                      Color* pPaintColor
                                      )
{
    if( rRect.IsEmpty() )
        return;

    bool bRoundEdges = nCornerRadius > 0;

    const StyleSettings& rStyles = GetSettings().GetStyleSettings();


    
    Color aSelectionBorderCol( pPaintColor ? *pPaintColor : rStyles.GetHighlightColor() );
    Color aSelectionFillCol( aSelectionBorderCol );

    bool bDark = rStyles.GetFaceColor().IsDark();
    bool bBright = ( rStyles.GetFaceColor() == Color( COL_WHITE ) );

    int c1 = aSelectionBorderCol.GetLuminance();
    int c2 = GetDisplayBackground().GetColor().GetLuminance();

    if( !bDark && !bBright && abs( c2-c1 ) < (pPaintColor ? 40 : 75) )
    {
        
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
        --aRect.Left();
        --aRect.Top();
        ++aRect.Right();
        ++aRect.Bottom();
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
            nPercent = 80;  
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
                nPercent = bRoundEdges ? 40 : 20;          
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
                nPercent = bRoundEdges ? 60 : 35;          
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
                nPercent = 70;          
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



Window* Window::GetPreferredKeyInputWindow()
{
    return this;
}


bool Window::IsScrollable() const
{
    
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

bool Window::IsTopWindow() const
{
    if ( mpWindowImpl->mbInDtor )
        return false;

    
    if( !mpWindowImpl->mbFrame && (!mpWindowImpl->mpBorderWindow || (mpWindowImpl->mpBorderWindow && !mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) ) )
        return false;

    ImplGetWinData();
    if( mpWindowImpl->mpWinData->mnIsTopWindow == (sal_uInt16)~0)    
    {
        
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


bool Window::IsInModalMode() const
{
    return (mpWindowImpl->mpFrameWindow->mpWindowImpl->mpFrameData->mnModalMode != 0);
}

bool Window::IsInModalNonRefMode() const
{
    if(mpWindowImpl->mnStyle & WB_REFMODE)
        return false;

    return IsInModalMode();
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

void Window::ImplIsInTaskPaneList( bool mbIsInTaskList )
{
    mpWindowImpl->mbIsInTaskPaneList = mbIsInTaskList;
}

void Window::ImplNotifyIconifiedState( bool bIconified )
{
    mpWindowImpl->mpFrameWindow->ImplCallEventListeners( bIconified ? VCLEVENT_WINDOW_MINIMIZE : VCLEVENT_WINDOW_NORMALIZE );
    
    if( mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow && mpWindowImpl->mpFrameWindow != mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow )
        mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow->ImplCallEventListeners( bIconified ? VCLEVENT_WINDOW_MINIMIZE : VCLEVENT_WINDOW_NORMALIZE );
}

bool Window::HasActiveChildFrame()
{
    bool bRet = false;
    Window *pFrameWin = ImplGetSVData()->maWinData.mpFirstFrame;
    while( pFrameWin )
    {
        if( pFrameWin != mpWindowImpl->mpFrameWindow )
        {
            bool bDecorated = false;
            Window *pChildFrame = pFrameWin->ImplGetWindow();
            
            
            
            
            
            if( pChildFrame && pChildFrame->ImplIsFloatingWindow() )
                bDecorated = ((FloatingWindow*) pChildFrame)->GetTitleType() != FLOATWIN_TITLE_NONE;
            if( bDecorated || (pFrameWin->mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE) ) )
                if( pChildFrame && pChildFrame->IsVisible() && pChildFrame->IsActive() )
                {
                    if( ImplIsChild( pChildFrame, true ) )
                    {
                        bRet = true;
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

void Window::EnableNativeWidget( bool bEnable )
{
    static const char* pNoNWF = getenv( "SAL_NO_NWF" );
    if( pNoNWF && *pNoNWF )
        bEnable = false;

    if( bEnable != ImplGetWinData()->mbEnableNativeWidget )
    {
        ImplGetWinData()->mbEnableNativeWidget = bEnable;

        
        
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, mxSettings.get(), SETTINGS_STYLE );
        DataChanged( aDCEvt );

        
        if( mpWindowImpl->mpBorderWindow )
            mpWindowImpl->mpBorderWindow->ImplGetWinData()->mbEnableNativeWidget = bEnable;
    }

    
    Window *pChild = mpWindowImpl->mpFirstChild;
    while( pChild )
    {
        pChild->EnableNativeWidget( bEnable );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

bool Window::IsNativeWidgetEnabled() const
{
    return ImplGetWinData()->mbEnableNativeWidget;
}

#ifdef WNT 
#include <win/salframe.h>
#endif

uno::Reference< rendering::XCanvas > Window::ImplGetCanvas( const Size& rFullscreenSize,
                                                       bool        bFullscreen,
                                                       bool        bSpriteCanvas ) const
{
    
    uno::Reference< rendering::XCanvas > xCanvas( mpWindowImpl->mxCanvas );

    
    if( xCanvas.is() )
        return xCanvas;

    Sequence< Any > aArg(6);

    
    

    
    aArg[ 0 ] = makeAny( reinterpret_cast<sal_Int64>(this) );

    

    
    
    
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

    uno::Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();

    
    
    static ::vcl::DeleteUnoReferenceOnDeinit<lang::XMultiComponentFactory> xStaticCanvasFactory(
        rendering::CanvasFactory::create( xContext ) );
    uno::Reference<lang::XMultiComponentFactory> xCanvasFactory(xStaticCanvasFactory.get());

    if(xCanvasFactory.is())
    {
#ifdef WNT
        
        
        
        
        
        const sal_uInt32 nDisplay = static_cast< WinSalFrame* >( mpWindowImpl->mpFrame )->mnDisplay;
        if( (nDisplay >= Application::GetScreenCount()) )
        {
            xCanvas.set( xCanvasFactory->createInstanceWithArgumentsAndContext(
                                 bSpriteCanvas ?
                                 OUString( "com.sun.star.rendering.SpriteCanvas.MultiScreen" ) :
                                 OUString( "com.sun.star.rendering.Canvas.MultiScreen" ),
                                 aArg,
                                 xContext ),
                             UNO_QUERY );

        }
        else
        {
#endif
            xCanvas.set( xCanvasFactory->createInstanceWithArgumentsAndContext(
                             bSpriteCanvas ?
                             OUString( "com.sun.star.rendering.SpriteCanvas" ) :
                             OUString( "com.sun.star.rendering.Canvas" ),
                             aArg,
                             xContext ),
                         UNO_QUERY );

#ifdef WNT
        }
#endif
        mpWindowImpl->mxCanvas = xCanvas;
    }

    
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
    bool bRVisible = mpWindowImpl->mbReallyVisible;
    mpWindowImpl->mbReallyVisible = mpWindowImpl->mbVisible;
    bool bDevOutput = mbDevOutput;
    mbDevOutput = true;

    const OutputDevice *pOutDev = GetOutDev();
    long nOldDPIX = pOutDev->ImplGetDPIX();
    long nOldDPIY = pOutDev->ImplGetDPIY();
    mnDPIX = i_pTargetOutDev->ImplGetDPIX();
    mnDPIY = i_pTargetOutDev->ImplGetDPIY();
    bool bOutput = IsOutputEnabled();
    EnableOutput();

    DBG_ASSERT( GetMapMode().GetMapUnit() == MAP_PIXEL, "MapMode must be PIXEL based" );
    if ( GetMapMode().GetMapUnit() != MAP_PIXEL )
        return;

    
    Push();
    Region aClipRegion( GetClipRegion() );
    SetClipRegion();

    GDIMetaFile* pOldMtf = GetConnectMetaFile();
    GDIMetaFile aMtf;
    SetConnectMetaFile( &aMtf );

    
    Push();
    
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

    

    
    if( ! IsPaintTransparent() && IsBackground() && ! (GetParentClipMode() & PARENTCLIPMODE_NOCLIP ) )
        Erase();
    
    Paint( aPaintRect );
    
    Pop();

    SetConnectMetaFile( pOldMtf );
    EnableOutput( bOutput );
    mpWindowImpl->mbReallyVisible = bRVisible;

    
    VirtualDevice* pMaskedDevice = new VirtualDevice( *i_pTargetOutDev, 0, 0 );
    pMaskedDevice->SetOutputSizePixel( GetOutputSizePixel() );
    pMaskedDevice->EnableRTL( IsRTLEnabled() );
    aMtf.WindStart();
    aMtf.Play( pMaskedDevice );
    BitmapEx aBmpEx( pMaskedDevice->GetBitmapEx( Point( 0, 0 ), pMaskedDevice->GetOutputSizePixel() ) );
    i_pTargetOutDev->DrawBitmapEx( i_rPos, aBmpEx );
    
    delete pMaskedDevice, pMaskedDevice = NULL;


    for( Window* pChild = mpWindowImpl->mpFirstChild; pChild; pChild = pChild->mpWindowImpl->mpNext )
    {
        if( pChild->mpWindowImpl->mpFrame == mpWindowImpl->mpFrame && pChild->IsVisible() )
        {
            long nDeltaX = pChild->mnOutOffX - mnOutOffX;

            if( pOutDev->HasMirroredGraphics() )
                nDeltaX = mnOutWidth - nDeltaX - pChild->mnOutWidth;
            long nDeltaY = pChild->GetOutOffYPixel() - GetOutOffYPixel();
            Point aPos( i_rPos );
            Point aDelta( nDeltaX, nDeltaY );
            aPos += aDelta;
            pChild->ImplPaintToDevice( i_pTargetOutDev, aPos );
        }
    }

    
    Pop();

    EnableOutput( bOutput );
    mpWindowImpl->mbReallyVisible = bRVisible;
    mbDevOutput = bDevOutput;
    mnDPIX = nOldDPIX;
    mnDPIY = nOldDPIY;
}

void Window::PaintToDevice( OutputDevice* pDev, const Point& rPos, const Size& /*rSize*/ )
{
    

    DBG_ASSERT( ! pDev->HasMirroredGraphics(), "PaintToDevice to mirroring graphics" );
    DBG_ASSERT( ! pDev->IsRTLEnabled(), "PaintToDevice to mirroring device" );

    Window* pRealParent = NULL;
    if( ! mpWindowImpl->mbVisible )
    {
        Window* pTempParent = ImplGetDefaultWindow();
        if( pTempParent )
            pTempParent->EnableChildTransparentMode();
        pRealParent = GetParent();
        SetParent( pTempParent );
        
        Show();
        Hide();
    }

    bool bVisible = mpWindowImpl->mbVisible;
    mpWindowImpl->mbVisible = true;

    if( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->ImplPaintToDevice( pDev, rPos );
    else
        ImplPaintToDevice( pDev, rPos );

    mpWindowImpl->mbVisible = bVisible;

    if( pRealParent )
        SetParent( pRealParent );
}

OUString Window::GetSurroundingText() const
{
  return OUString();
}

Selection Window::GetSurroundingTextSelection() const
{
  return Selection( 0, 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
