/*************************************************************************
 *
 *  $RCSfile: dtint.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:44:37 $
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <salunx.h>
#include <X11/Xatom.h>

#ifdef USE_CDE
#include <cdeint.hxx>
#endif
#include <kdeint.hxx>
#include <gnomeint.hxx>
#include <saldisp.hxx>
#include <saldata.hxx>
#include <wmadaptor.hxx>

#include <svapp.hxx>
#include <dtsetenum.hxx>

#include <osl/file.h>
#include <osl/process.h>

#include <set>
#include <stdio.h>

// NETBSD has no RTLD_GLOBAL
#ifndef RTLD_GLOBAL
#define DLOPEN_MODE (RTLD_LAZY)
#else
#define DLOPEN_MODE (RTLD_GLOBAL | RTLD_LAZY)
#endif


using namespace rtl;
using namespace vcl_sal;

BOOL bSymbolLoadFailed = FALSE;

DtIntegratorList DtIntegrator::aIntegratorList;
String DtIntegrator::aHomeDir;

DtIntegrator::DtIntegrator() :
        meType( DtGeneric ),
        mnRefCount( 0 ),
        mnSystemLookCommandProcess( -1 )
{
    mpSalDisplay = GetSalData()->GetDefDisp();
    mpDisplay = mpSalDisplay->GetDisplay();
    aIntegratorList.Insert( this, LIST_APPEND );
    static const char* pHome = getenv( "HOME" );
    aHomeDir = String( pHome, osl_getThreadTextEncoding() );
}

DtIntegrator::~DtIntegrator()
{
}

DtIntegrator* DtIntegrator::CreateDtIntegrator()
{
    SalDisplay* pSalDisplay = GetSalData()->GetDefDisp();
    Display* pDisplay = pSalDisplay->GetDisplay();

    for( unsigned int i = 0; i < aIntegratorList.Count(); i++ )
    {
        DtIntegrator* pIntegrator = aIntegratorList.GetObject( i );
        if( pIntegrator->mpDisplay == pDisplay )
            return pIntegrator;
    }

    Atom nDtAtom = None;

#ifdef USE_CDE
    void* pLibrary = NULL;

    // check dt type
    // CDE
    nDtAtom = XInternAtom( pDisplay, "_DT_WM_READY", True );
    if( nDtAtom && ( pLibrary = dlopen( "libDtSvc.so", DLOPEN_MODE ) ) )
    {
        dlclose( pLibrary );
        return new CDEIntegrator();
    }
#endif

    if( pSalDisplay->getWMAdaptor()->getWindowManagerName().EqualsAscii( "KWin" ) )
        return new KDEIntegrator();

    // actually this is not that good an indicator for a GNOME running
    // but there currently does not seem to be a better one
    nDtAtom = XInternAtom( pDisplay, "GNOME_SM_PROXY", True );
    Atom nDtAtom2 = XInternAtom( pDisplay, "NAUTILUS_DESKTOP_WINDOW_ID", True );
    if( nDtAtom || nDtAtom2 )
    {
        int nProperties = 0;
        Atom* pProperties = XListProperties( pDisplay, pSalDisplay->GetRootWindow(), &nProperties );
        if( pProperties && nProperties )
        {
            for( int i = 0; i < nProperties; i++ )
                if( pProperties[ i ] == nDtAtom ||
                    pProperties[ i ] == nDtAtom2 )
                {
                    XFree( pProperties );
                    return new GNOMEIntegrator();
                }
            XFree( pProperties );
        }
    }

    // default: generic implementation
    return new DtIntegrator();
}

void DtIntegrator::GetSystemLook( AllSettings& rSettings )
{
}

Color DtIntegrator::parseColor( const ByteString& rLine )
{
    Color aColor;

    xub_StrLen nPos1 = rLine.Search( '(' );
    xub_StrLen nPos2 = rLine.Search( ')', nPos1 != STRING_NOTFOUND ? nPos1 : 0 );
    if( nPos1 != STRING_NOTFOUND && nPos2 != STRING_NOTFOUND
        && (int)nPos2 - (int)nPos1 > 5 )
    {
        ByteString aTriple( rLine.Copy( nPos1+1, nPos2-nPos1-1 ) );
        xub_StrLen nIndex = 0;
        int nRed = 0, nGreen = 0, nBlue = 0, nColor = 0;
        do
        {
            switch( nColor )
            {
                case 0: nRed = aTriple.GetToken( 0, ',', nIndex ).ToInt32();break;
                case 1: nGreen = aTriple.GetToken( 0, ',', nIndex ).ToInt32();break;
                case 2: nBlue = aTriple.GetToken( 0, ',', nIndex ).ToInt32();break;
            }
            nColor++;
        } while(  nIndex != STRING_NOTFOUND );
        if( nColor == 3 )
        {
            aColor = Color( nRed, nGreen, nBlue );
        }
    }
    return aColor;
}

Font DtIntegrator::parseFont( const ByteString& rLine )
{
    Font aFont;

    xub_StrLen nPos1 = rLine.Search( '"' );
    xub_StrLen nPos2 = rLine.Search( '"', nPos1 != STRING_NOTFOUND ? nPos1+1 : 0 );
    if( nPos1 != STRING_NOTFOUND && nPos2 != STRING_NOTFOUND
        && (int)nPos2 - (int)nPos1 > 1 )
    {
        aFont.SetName( String( rLine.Copy( nPos1+1, nPos2-nPos1-1 ), osl_getThreadTextEncoding() ) );
        xub_StrLen nIndex = nPos2;
        ByteString aToken = rLine.GetToken( 1, ',', nIndex );
        if( aToken.Len() )
        {
            int nPixelHeight = aToken.ToInt32();
            long nDPIX, nDPIY;
            long nDispDPIY = mpSalDisplay->GetResolution().B();
            mpSalDisplay->GetScreenFontResolution( nDPIX, nDPIY );
            int nHeight = nPixelHeight * nDispDPIY / nDPIY;
            // allow for rounding in back conversion (at SetFont)
            while( (nHeight * nDPIY / nDispDPIY) > nPixelHeight )
                nHeight--;
            while( (nHeight * nDPIY / nDispDPIY) < nPixelHeight )
                nHeight++;
            aFont.SetHeight( nHeight );
        }
        while( nIndex != STRING_NOTFOUND )
        {
            aToken = rLine.GetToken( 0, ',', nIndex );
            if( aToken.Equals( "bold" ) )
                aFont.SetWeight( WEIGHT_BOLD );
            else if( aToken.Equals( "light" ) )
                aFont.SetWeight( WEIGHT_LIGHT );
            else if( aToken.Equals( "italic" ) )
                aFont.SetItalic( ITALIC_NORMAL );
        }
    }
    return aFont;
}

bool DtIntegrator::StartSystemLookProcess( const char* pCommand )
{
    ByteString aCommand, aArgWindow;
    if( pCommand[0] == '/' )
        aCommand = pCommand;
    else
    {
        OUString aExec;
        OUString aSys;
        osl_getExecutableFile( &aExec.pData );
        if( osl_File_E_None != osl_getSystemPathFromFileURL( aExec.pData, &aSys.pData ) )
            return false; // huh ?
        aCommand = ByteString( String( aSys ), osl_getThreadTextEncoding() );
        aCommand.Erase( aCommand.SearchBackward( '/' )+1 );
        aCommand.Append( pCommand );
    }
    aArgWindow = ByteString::CreateFromInt32( mpSalDisplay->GetDrawable() );
    mnSystemLookCommandProcess = fork();
    if( mnSystemLookCommandProcess == 0 )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "exec( \"%s --vcl-system-settings-window %s\" )\n", aCommand.GetBuffer(), aArgWindow.GetBuffer() );
#endif
        int nRedirect = open( "/dev/null", O_WRONLY );
        if( nRedirect != -1 )
        {
            dup2( nRedirect, STDOUT_FILENO );
            dup2( nRedirect, STDERR_FILENO );
        }
        execl( aCommand.GetBuffer(), aCommand.GetBuffer(), "--vcl-system-settings-window", aArgWindow.GetBuffer(), NULL );
        _exit(1);
    }
    return mnSystemLookCommandProcess != -1;
}

void DtIntegrator::GetSystemLook( const char* pCommand, AllSettings& rSettings )
{
    BOOL bRet = FALSE;
    std::list< ByteString > aLines;

    if( pCommand )
    {
        char pBuffer[1024];
        FILE* pOutput = NULL;

        ByteString aCommand;
        if( pCommand[0] == '/' )
            aCommand = pCommand;
        else
        {
            OUString aExec;
            OUString aSys;
            osl_getExecutableFile( &aExec.pData );
            if( osl_File_E_None != osl_getSystemPathFromFileURL( aExec.pData, &aSys.pData ) )
                return; // huh ?
            aCommand = ByteString( String( aSys ), osl_getThreadTextEncoding() );
            aCommand.Erase( aCommand.SearchBackward( '/' )+1 );
            aCommand.Append( pCommand );
        }
#if OSL_DEBUG_LEVEL < 2
        aCommand.Append( " 2>/dev/null" );
#endif
        pOutput = popen( aCommand.GetBuffer(), "r" );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "popen( \"%s\", \"r\" ) = %p\n", aCommand.GetBuffer(), pOutput );
#endif
        while( fgets( pBuffer, sizeof( pBuffer ), pOutput ) )
        {
            aLines.push_back( ByteString( pBuffer ) );
        }
        pclose( pOutput );
    }
    else if( ! pCommand && mnSystemLookCommandProcess != -1 )
    {
        // get the lines from the VCL_SYSTEM_SETTINGS property
        Atom nType = 0;
        int nFormat = 0;
        unsigned long nItems = 0, nBytes = 0;
        char* pData = NULL;
        // query size of the property
        XGetWindowProperty( mpSalDisplay->GetDisplay(),
                            mpSalDisplay->GetDrawable(),
                            mpSalDisplay->getWMAdaptor()->getAtom( WMAdaptor::VCL_SYSTEM_SETTINGS ),
                            0, 0,
                            False,
                            XA_STRING,
                            &nType,
                            &nFormat,
                            &nItems,
                            &nBytes,
                            (unsigned char**)&pData );
        if( nBytes )
        {
            // query the actual data
            XGetWindowProperty( mpSalDisplay->GetDisplay(),
                                mpSalDisplay->GetDrawable(),
                                mpSalDisplay->getWMAdaptor()->getAtom( WMAdaptor::VCL_SYSTEM_SETTINGS ),
                                0, (nBytes+3)/4,
                                False,
                                XA_STRING,
                                &nType,
                                &nFormat,
                                &nItems,
                                &nBytes,
                                (unsigned char**)&pData );
            // sanity check
            if( nFormat == 8 && nType == XA_STRING && nItems )
            {
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "got %ld data items:\n%.*s", nItems, (int)nItems, pData );
#endif
                // fill in the lines
                char* pRun = pData;
                char* pLastLine = pData;
                while( (pRun-pData) <= (int)nItems )
                {
                    if( *pRun == '\n' )
                    {
                        if( pRun-pLastLine > 1 )
                        {
                            aLines.push_back( ByteString( pLastLine, pRun - pLastLine ) );
                            pLastLine = pRun+1;
                        }
                    }
                    pRun++;
                }
            }
#if OSL_DEBUG_LEVEL > 1
            else
                fprintf( stderr, "query of data failed with nFormat = %d, nType = %d, nItems = %ld\n", nFormat, (int)nType, nItems );
#endif
            XFree( pData );
        }
#if OSL_DEBUG_LEVEL > 1
        else
            fprintf( stderr, "query of bytes failed\n" );
#endif
    }

    // nothing to do ?
    if( aLines.begin() == aLines.end() )
        return;

    StyleSettings aStyleSettings( rSettings.GetStyleSettings() );
    MouseSettings aMouseSettings( rSettings.GetMouseSettings() );
    KeyboardSettings aKeyboardSettings( rSettings.GetKeyboardSettings() );

    std::set< DtSetEnum > aItemSet;

    while( aLines.begin() != aLines.end() )
    {
        ByteString aLine( aLines.front() );
        aLines.pop_front();
        if( aLine.GetChar( aLine.Len()-1 ) == '\n' )
            aLine.Erase( aLine.Len()-1 ); // cut newline

        USHORT nPos = aLine.Search( '=' );
        if( nPos == STRING_NOTFOUND )
            continue;

        DtSetEnum aToken = (DtSetEnum)aLine.Copy( 0, nPos ).ToInt32();
        aLine.Erase( 0, nPos+1 );

        if( aToken )
        {
            bRet = TRUE;
            aItemSet.insert( aToken );
        }

        switch( aToken )
        {
            case MouseOptions:          aMouseSettings.SetOptions( aLine.ToInt32() );break;
            case DoubleClickTime:       aMouseSettings.SetDoubleClickTime( aLine.ToInt32() );break;
            case DoubleClickWidth:      aMouseSettings.SetDoubleClickWidth( aLine.ToInt32() );break;
            case DoubleClickHeight:     aMouseSettings.SetDoubleClickHeight( aLine.ToInt32() );break;
            case StartDragWidth:        aMouseSettings.SetStartDragWidth( aLine.ToInt32() );break;
            case StartDragHeight:       aMouseSettings.SetStartDragHeight( aLine.ToInt32() );break;
            case DragMoveCode:          aMouseSettings.SetDragMoveCode( aLine.ToInt32() );break;
            case DragCopyCode:          aMouseSettings.SetDragCopyCode( aLine.ToInt32() );break;
            case DragLinkCode:          aMouseSettings.SetDragLinkCode( aLine.ToInt32() );break;
            case ContextMenuCode:       aMouseSettings.SetContextMenuCode( aLine.ToInt32() );break;
            case ContextMenuClicks:     aMouseSettings.SetContextMenuClicks( aLine.ToInt32() );break;
            case ContextMenuDown:       aMouseSettings.SetContextMenuDown( aLine.ToInt32() );break;
            case ScrollRepeat:          aMouseSettings.SetScrollRepeat( aLine.ToInt32() );break;
            case ButtonStartRepeat:     aMouseSettings.SetButtonStartRepeat( aLine.ToInt32() );break;
            case ButtonRepeat:          aMouseSettings.SetButtonRepeat( aLine.ToInt32() );break;
            case ActionDelay:           aMouseSettings.SetActionDelay( aLine.ToInt32() );break;
            case MenuDelay:             aMouseSettings.SetMenuDelay( aLine.ToInt32() );break;
            case Follow:                aMouseSettings.SetFollow( aLine.ToInt32() );break;
            case MiddleButtonAction:    aMouseSettings.SetMiddleButtonAction( aLine.ToInt32() );break;
            case KeyboardOptions:       aKeyboardSettings.SetOptions( aLine.ToInt32() );break;
            case StyleOptions:          aStyleSettings.SetOptions( aLine.ToInt32() );break;
            case BorderSize:            aStyleSettings.SetBorderSize( aLine.ToInt32() );break;
            case TitleHeight:           aStyleSettings.SetTitleHeight( aLine.ToInt32() );break;
            case FloatTitleHeight:      aStyleSettings.SetFloatTitleHeight( aLine.ToInt32() );break;
            case TearOffTitleHeight:    aStyleSettings.SetTearOffTitleHeight( aLine.ToInt32() );break;
            case MenuBarHeight:         aStyleSettings.SetMenuBarHeight( aLine.ToInt32() );break;
            case ScrollBarSize:         aStyleSettings.SetScrollBarSize( aLine.ToInt32() );break;
            case SpinSize:              aStyleSettings.SetSpinSize( aLine.ToInt32() );break;
            case SplitSize:             aStyleSettings.SetSplitSize( aLine.ToInt32() );break;
            case IconHorzSpace:         aStyleSettings.SetIconHorzSpace( aLine.ToInt32() );break;
            case IconVertSpace:         aStyleSettings.SetIconVertSpace( aLine.ToInt32() );break;
            case CursorSize:            aStyleSettings.SetCursorSize( aLine.ToInt32() );break;
            case CursorBlinkTime:       aStyleSettings.SetCursorBlinkTime( aLine.ToInt32() );break;
            case ScreenZoom:            aStyleSettings.SetScreenZoom( aLine.ToInt32() );break;
            case ScreenFontZoom:        aStyleSettings.SetScreenFontZoom( aLine.ToInt32() );break;
            case LogoDisplayTime:       aStyleSettings.SetLogoDisplayTime( aLine.ToInt32() );break;
            case DragFullOptions:       aStyleSettings.SetDragFullOptions( aLine.ToInt32() );break;
            case AnimationOptions:      aStyleSettings.SetAnimationOptions( aLine.ToInt32() );break;
            case SelectionOptions:      aStyleSettings.SetSelectionOptions( aLine.ToInt32() );break;
            case DisplayOptions:        aStyleSettings.SetDisplayOptions( aLine.ToInt32() );break;
            case AntialiasingMinPixelHeight: aStyleSettings.SetAntialiasingMinPixelHeight( aLine.ToInt32() );break;
            case ThreeDColor:           aStyleSettings.Set3DColors( parseColor( aLine ) );break;
            case FaceColor:             aStyleSettings.SetFaceColor( parseColor( aLine ) );break;
            case CheckedColor:          aStyleSettings.SetCheckedColor( parseColor( aLine ) );break;
            case LightColor:            aStyleSettings.SetLightColor( parseColor( aLine ) );break;
            case LightBorderColor:      aStyleSettings.SetLightBorderColor( parseColor( aLine ) );break;
            case ShadowColor:           aStyleSettings.SetShadowColor( parseColor( aLine ) );break;
            case DarkShadowColor:       aStyleSettings.SetDarkShadowColor( parseColor( aLine ) );break;
            case ButtonTextColor:       aStyleSettings.SetButtonTextColor( parseColor( aLine ) );break;
            case RadioCheckTextColor:   aStyleSettings.SetRadioCheckTextColor( parseColor( aLine ) );break;
            case GroupTextColor:        aStyleSettings.SetGroupTextColor( parseColor( aLine ) );break;
            case LabelTextColor:        aStyleSettings.SetLabelTextColor( parseColor(aLine ) );break;
            case InfoTextColor:         aStyleSettings.SetInfoTextColor( parseColor( aLine ) );break;
            case WindowColor:           aStyleSettings.SetWindowColor( parseColor( aLine ) );break;
            case WindowTextColor:       aStyleSettings.SetWindowTextColor( parseColor( aLine ) );break;
            case DialogColor:           aStyleSettings.SetDialogColor( parseColor( aLine ) );break;
            case DialogTextColor:       aStyleSettings.SetDialogTextColor( parseColor( aLine ) );break;
            case WorkspaceColor:        aStyleSettings.SetWorkspaceColor( parseColor( aLine ) );break;
            case FieldColor:            aStyleSettings.SetFieldColor( parseColor( aLine ) );break;
            case FieldTextColor:        aStyleSettings.SetFieldTextColor( parseColor( aLine ) );break;
            case ActiveColor:           aStyleSettings.SetActiveColor( parseColor( aLine ) );break;
            case ActiveColor2:          aStyleSettings.SetActiveColor2( parseColor( aLine ) );break;
            case ActiveTextColor:       aStyleSettings.SetActiveTextColor( parseColor( aLine ) );break;
            case ActiveBorderColor:     aStyleSettings.SetActiveBorderColor( parseColor( aLine ) );break;
            case DeactiveColor:         aStyleSettings.SetDeactiveColor( parseColor( aLine ) );break;
            case DeactiveColor2:        aStyleSettings.SetDeactiveColor2( parseColor( aLine ) );break;
            case DeactiveTextColor:     aStyleSettings.SetDeactiveTextColor( parseColor( aLine ) );break;
            case DeactiveBorderColor:   aStyleSettings.SetDeactiveBorderColor( parseColor( aLine ) );break;
            case HighlightColor:        aStyleSettings.SetHighlightColor( parseColor( aLine ) );break;
            case HighlightTextColor:    aStyleSettings.SetHighlightTextColor( parseColor( aLine ) );break;
            case DisableColor:          aStyleSettings.SetDisableColor( parseColor( aLine ) );break;
            case HelpColor:             aStyleSettings.SetHelpColor( parseColor( aLine ) );break;
            case HelpTextColor:         aStyleSettings.SetHelpTextColor( parseColor( aLine ) );break;
            case MenuColor:             aStyleSettings.SetMenuColor( parseColor( aLine ) );break;
            case MenuBarColor:          aStyleSettings.SetMenuBarColor( parseColor( aLine ) );break;
            case MenuTextColor:         aStyleSettings.SetMenuTextColor( parseColor( aLine ) );break;
            case MenuHighlightColor:    aStyleSettings.SetMenuHighlightColor( parseColor( aLine ) );break;
            case MenuHighlightTextColor: aStyleSettings.SetMenuHighlightTextColor( parseColor( aLine ) );break;
            case LinkColor:             aStyleSettings.SetLinkColor( parseColor( aLine ) );break;
            case VisitedLinkColor:      aStyleSettings.SetVisitedLinkColor( parseColor( aLine ) );break;
            case HighlightLinkColor:    aStyleSettings.SetHighlightLinkColor( parseColor( aLine ) );break;
            case HighContrastMode:      aStyleSettings.SetHighContrastMode( aLine.ToInt32() );break;
            case AppFont:               aStyleSettings.SetAppFont( parseFont( aLine ) );break;
            case HelpFont:              aStyleSettings.SetHelpFont( parseFont( aLine ) );break;
            case TitleFont:             aStyleSettings.SetTitleFont( parseFont( aLine ) );break;
            case FloatTitleFont:        aStyleSettings.SetFloatTitleFont( parseFont( aLine ) );break;
            case MenuFont:              aStyleSettings.SetMenuFont( parseFont( aLine ) );break;
            case ToolFont:              aStyleSettings.SetToolFont( parseFont( aLine ) );break;
            case GroupFont:             aStyleSettings.SetGroupFont( parseFont( aLine ) );break;
            case LabelFont:             aStyleSettings.SetLabelFont( parseFont( aLine ) );break;
            case InfoFont:              aStyleSettings.SetInfoFont( parseFont( aLine ) );break;
            case RadioCheckFont:        aStyleSettings.SetRadioCheckFont( parseFont( aLine ) );break;
            case PushButtonFont:        aStyleSettings.SetPushButtonFont( parseFont( aLine ) );break;
            case FieldFont:             aStyleSettings.SetFieldFont( parseFont( aLine ) );break;
            case IconFont:              aStyleSettings.SetIconFont( parseFont( aLine ) );break;
            case RadioButtonStyle:      aStyleSettings.SetRadioButtonStyle( aLine.ToInt32() );break;
            case CheckBoxStyle:         aStyleSettings.SetCheckBoxStyle( aLine.ToInt32() );break;
            case PushButtonStyle:       aStyleSettings.SetPushButtonStyle( aLine.ToInt32() );break;
            case TabControlStyle:       aStyleSettings.SetTabControlStyle( aLine.ToInt32() );
                break;

            case AllTextColors:
            {
                Color aTextColor = parseColor( aLine );
                aStyleSettings.SetDialogTextColor( aTextColor );
                aStyleSettings.SetMenuTextColor( aTextColor );
                aStyleSettings.SetButtonTextColor( aTextColor );
                aStyleSettings.SetRadioCheckTextColor( aTextColor );
                aStyleSettings.SetGroupTextColor( aTextColor );
                aStyleSettings.SetLabelTextColor( aTextColor );
                aStyleSettings.SetInfoTextColor( aTextColor );
                aStyleSettings.SetWindowTextColor( aTextColor );
                aStyleSettings.SetFieldTextColor( aTextColor );
            }
            break;
            case AllBackColors:
            {
                Color aBack = parseColor( aLine );
                aStyleSettings.Set3DColors( aBack );
                aStyleSettings.SetFaceColor( aBack );
                aStyleSettings.SetDialogColor( aBack );
                aStyleSettings.SetMenuColor( aBack );
                aStyleSettings.SetMenuBarColor( aBack );
                aStyleSettings.SetWindowColor( aBack );
                aStyleSettings.SetFieldColor( aBack );
                if( aBack == COL_LIGHTGRAY )
                    aStyleSettings.SetCheckedColor( Color( 0xCC, 0xCC, 0xCC ) );
                else
                {
                    Color aColor2 = aStyleSettings.GetLightColor();
                    aStyleSettings.
                        SetCheckedColor( Color(
                                               (BYTE)(((USHORT)aBack.GetRed()+(USHORT)aColor2.GetRed())/2),
                                               (BYTE)(((USHORT)aBack.GetGreen()+(USHORT)aColor2.GetGreen())/2),
                                               (BYTE)(((USHORT)aBack.GetBlue()+(USHORT)aColor2.GetBlue())/2)
                                               ) );
                }
            }
            break;
            case UIFont:
                if( aStyleSettings.GetUseSystemUIFonts() )
                {
                    Font aFont = parseFont( aLine );
                    aStyleSettings.SetAppFont( aFont );
                    aStyleSettings.SetHelpFont( aFont );
                    aStyleSettings.SetMenuFont( aFont );
                    aStyleSettings.SetToolFont( aFont );
                    aStyleSettings.SetLabelFont( aFont );
                    aStyleSettings.SetInfoFont( aFont );
                    aStyleSettings.SetRadioCheckFont( aFont );
                    aStyleSettings.SetPushButtonFont( aFont );
                    aStyleSettings.SetFieldFont( aFont );
                    aStyleSettings.SetIconFont( aFont );
                    aStyleSettings.SetGroupFont( aFont );
                }
                break;
            case ToolbarIconSize:
                if( aLine.EqualsIgnoreCaseAscii( "large" ) )
                    aStyleSettings.SetToolbarIconSize( STYLE_TOOLBAR_ICONSIZE_LARGE );
                else if( aLine.EqualsIgnoreCaseAscii( "small" ) )
                    aStyleSettings.SetToolbarIconSize( STYLE_TOOLBAR_ICONSIZE_SMALL );
                else
                    aStyleSettings.SetToolbarIconSize( STYLE_TOOLBAR_ICONSIZE_UNKNOWN );
                break;
#if OSL_DEBUG_LEVEL > 1
            default:
                fprintf( stderr, "don't understand \"%d=%s\"\n", aToken, aLine.GetBuffer() );
                break;
#endif
        }
    }
    if( bRet )
    {
        rSettings.SetKeyboardSettings( aKeyboardSettings );
        rSettings.SetMouseSettings( aMouseSettings );
        rSettings.SetStyleSettings( aStyleSettings );
    }
}
