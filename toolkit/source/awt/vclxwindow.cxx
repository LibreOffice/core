/*************************************************************************
 *
 *  $RCSfile: vclxwindow.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mt $ $Date: 2001-02-16 11:15:57 $
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


#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_STYLE_HPP_
#include <com/sun/star/awt/Style.hpp>
#endif

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxpointer.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/property.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <tools/color.hxx>

//  ----------------------------------------------------
//  class VCLXWindow
//  ----------------------------------------------------
VCLXWindow::VCLXWindow()
    : maEventListeners( *this ),
      maFocusListeners( *this ),
      maWindowListeners( *this ),
      maKeyListeners( *this ),
      maMouseListeners( *this ),
      maMouseMotionListeners( *this ),
      maPaintListeners( *this ),
      maContainerListeners( *this ),
      maTopWindowListeners( *this )
{
    mbDisposing = sal_False;
    mbDesignMode = sal_False;
}

VCLXWindow::~VCLXWindow()
{
    if ( GetWindow() )
        GetWindow()->SetWindowPeer( NULL, NULL );
}

void VCLXWindow::SetWindow( Window* pWindow )
{
    SetOutputDevice( pWindow );
}

Size VCLXWindow::ImplCalcWindowSize( const Size& rOutSz ) const
{
    Size aSz = rOutSz;

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        sal_Int32 nLeft, nTop, nRight, nBottom;
        pWindow->GetBorder( nLeft, nTop, nRight, nBottom );
        aSz.Width() += nLeft+nRight;
        aSz.Height() += nTop+nBottom;
    }
    return aSz;
}


// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXWindow::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XComponent*, (::com::sun::star::awt::XWindow*)this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XWindow*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XWindowPeer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XVclWindowPeer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XLayoutConstrains*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XView*, this ) );
    return (aRet.hasValue() ? aRet : VCLXDevice::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXWindow )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXWindow )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView>* ) NULL ),
    VCLXDevice::getTypes()
IMPL_XTYPEPROVIDER_END


// ::com::sun::star::lang::Component
void VCLXWindow::dispose(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    mxViewGraphics = NULL;

    if ( GetWindow() && !mbDisposing )
    {
        mbDisposing = sal_True;

        ::com::sun::star::lang::EventObject aObj;
        aObj.Source = (::cppu::OWeakObject*)this;

        maEventListeners.disposeAndClear( aObj );
        maFocusListeners.disposeAndClear( aObj );
        maWindowListeners.disposeAndClear( aObj );
        maKeyListeners.disposeAndClear( aObj );
        maMouseListeners.disposeAndClear( aObj );
        maMouseMotionListeners.disposeAndClear( aObj );
        maPaintListeners.disposeAndClear( aObj );
        maContainerListeners.disposeAndClear( aObj );
        maTopWindowListeners.disposeAndClear( aObj );

        OutputDevice* pOutDev = GetOutputDevice();
        SetWindow( NULL );  // Damit ggf. Handler abgemeldet werden (virtuell).
        SetOutputDevice( pOutDev );
        DestroyOutputDevice();

        mbDisposing = sal_False;
    }
}

void VCLXWindow::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetEventListeners().addInterface( rxListener );
}

void VCLXWindow::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetEventListeners().removeInterface( rxListener );
}


// ::com::sun::star::awt::XWindow
void VCLXWindow::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->SetPosSizePixel( X, Y, Width, Height, Flags );
}

::com::sun::star::awt::Rectangle VCLXWindow::getPosSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Rectangle aBounds;
    if ( GetWindow() )
        aBounds = AWTRectangle( Rectangle( GetWindow()->GetPosPixel(), GetWindow()->GetSizePixel() ) );

    return aBounds;
}

void VCLXWindow::setVisible( sal_Bool bVisible ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
/*
        if ( bVisible )
        {
            // #57167# TopWindows mit unsichtbaren Parent anzeigen...
            ::com::sun::star::uno::Any aTest = queryInterface( ::getCppuType(  (const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindow >*)  0 ) );
            if ( aTest.hasValue() )
            {
                Window* pParent = pWindow->GetWindow( WINDOW_PARENTOVERLAP );
                if ( pParent && !pParent->IsReallyVisible() )
                    pWindow->SetParent( pWindow->GetWindow( WINDOW_FRAME ) );
            }
        }
*/
        pWindow->Show( bVisible );
    }
}

void VCLXWindow::setEnable( sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->Enable( bEnable );
}

void VCLXWindow::setFocus(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->GrabFocus();
}

void VCLXWindow::addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetWindowListeners().addInterface( rxListener );
}

void VCLXWindow::removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetWindowListeners().removeInterface( rxListener );
}

void VCLXWindow::addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetFocusListeners().addInterface( rxListener );
}

void VCLXWindow::removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetFocusListeners().removeInterface( rxListener );
}

void VCLXWindow::addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetKeyListeners().addInterface( rxListener );
}

void VCLXWindow::removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetKeyListeners().removeInterface( rxListener );
}

void VCLXWindow::addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetMouseListeners().addInterface( rxListener );
}

void VCLXWindow::removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetMouseListeners().removeInterface( rxListener );
}

void VCLXWindow::addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetMouseMotionListeners().addInterface( rxListener );
}

void VCLXWindow::removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetMouseMotionListeners().removeInterface( rxListener );
}

void VCLXWindow::addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetPaintListeners().addInterface( rxListener );
}

void VCLXWindow::removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetPaintListeners().removeInterface( rxListener );
}

// ::com::sun::star::awt::XWindowPeer
::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > VCLXWindow::getToolkit(  ) throw(::com::sun::star::uno::RuntimeException)
{
    // no guard. nothing to guard here.
    // 82463 - 12/21/00 - fs
    return Application::GetVCLToolkit();
}

void VCLXWindow::setPointer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer >& rxPointer ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    VCLXPointer* pPointer = VCLXPointer::GetImplementation( rxPointer );
    if ( pPointer )
    {
        mxPointer = rxPointer;
        if ( GetWindow() )
            GetWindow()->SetPointer( pPointer->GetPointer() );
    }
}

void VCLXWindow::setBackground( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        Color aColor( (sal_uInt32)nColor );
        GetWindow()->SetBackground( aColor );
        GetWindow()->SetControlBackground( aColor );

        WindowType eWinType = GetWindow()->GetType();
        if ( ( eWinType == WINDOW_WINDOW ) ||
             ( eWinType == WINDOW_WORKWINDOW ) ||
             ( eWinType == WINDOW_FLOATINGWINDOW ) )
        {
            GetWindow()->Invalidate();
        }
    }
}

void VCLXWindow::invalidate( sal_Int16 nFlags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->Invalidate( (sal_uInt16) nFlags );
}

void VCLXWindow::invalidateRect( const ::com::sun::star::awt::Rectangle& rRect, sal_Int16 nFlags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->Invalidate( VCLRectangle(rRect), (sal_uInt16) nFlags );
}


// ::com::sun::star::awt::XVclWindowPeer
sal_Bool VCLXWindow::isChild( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rxPeer ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Bool bIsChild = sal_False;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        Window* pPeerWindow = VCLUnoHelper::GetWindow( rxPeer );
        bIsChild = pPeerWindow && pWindow->IsChild( pPeerWindow );
    }

    return bIsChild;
}

void VCLXWindow::setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    mbDesignMode = bOn;
}

sal_Bool VCLXWindow::isDesignMode(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    return mbDesignMode;
}

void VCLXWindow::enableClipSiblings( sal_Bool bClip ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->EnableClipSiblings( bClip );
}

void VCLXWindow::setForeground( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        Color aColor( (sal_uInt32)nColor );
        GetWindow()->SetControlForeground( aColor );
    }
}

void VCLXWindow::setControlFont( const ::com::sun::star::awt::FontDescriptor& rFont ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->SetControlFont( VCLUnoHelper::CreateFont( rFont, GetWindow()->GetControlFont() ) );
}

void VCLXWindow::getStyles( sal_Int16 nType, ::com::sun::star::awt::FontDescriptor& Font, sal_Int32& ForegroundColor, sal_Int32& BackgroundColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        const StyleSettings& rStyleSettings = GetWindow()->GetSettings().GetStyleSettings();

        switch ( nType )
        {
            case ::com::sun::star::awt::Style::FRAME:
            {
                Font = VCLUnoHelper::CreateFontDescriptor( rStyleSettings.GetAppFont() );
                ForegroundColor = rStyleSettings.GetWindowTextColor().GetColor();
                BackgroundColor = rStyleSettings.GetWindowColor().GetColor();
            }
            break;
            case ::com::sun::star::awt::Style::DIALOG:
            {
                Font = VCLUnoHelper::CreateFontDescriptor( rStyleSettings.GetAppFont() );
                ForegroundColor = rStyleSettings.GetDialogTextColor().GetColor();
                BackgroundColor = rStyleSettings.GetDialogColor().GetColor();
            }
            break;
            default: DBG_ERROR( "VCLWindow::getStyles() - unknown Type" );
        }

    }
}

void VCLXWindow::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        WindowType eWinType = pWindow->GetType();
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_ENABLED:
            {
                sal_Bool b;
                if ( Value >>= b )
                    pWindow->Enable( b );
            }
            break;
            case BASEPROPERTY_TEXT:
            case BASEPROPERTY_LABEL:
            case BASEPROPERTY_TITLE:
            {
                ::rtl::OUString aText;
                if ( Value >>= aText )
                    pWindow->SetText( aText );
            }
            break;
            case BASEPROPERTY_HELPURL:
            {
                ::rtl::OUString aURL;
                if ( Value >>= aURL )
                {
                    String aHelpURL(  aURL );
                    String aPattern( RTL_CONSTASCII_USTRINGPARAM( "HID:" ) );
                    if ( aHelpURL.CompareIgnoreCaseToAscii( aPattern, aPattern.Len() ) == COMPARE_EQUAL )
                    {
                        String aID = aHelpURL.Copy( aPattern.Len() );
                        pWindow->SetHelpId( aID.ToInt32() );
                    }
                }
            }
            break;
            case BASEPROPERTY_FONTDESCRIPTOR:
            {
                if ( bVoid )
                    pWindow->SetControlFont( Font() );
                else
                {
                    ::com::sun::star::awt::FontDescriptor aFont;
                    if ( Value >>= aFont )
                        pWindow->SetControlFont( VCLUnoHelper::CreateFont( aFont, pWindow->GetControlFont() ) );
                }
            }
            break;
            case BASEPROPERTY_BACKGROUNDCOLOR:
                if ( bVoid )
                {
                    pWindow->SetControlBackground();
                }
                else
                {
                    sal_Int32 nColor;
                    if ( Value >>= nColor )
                    {
                        Color aColor( nColor );
                        pWindow->SetBackground( aColor );
                        pWindow->SetControlBackground( aColor );
                        pWindow->Invalidate();  // Falls das Control nicht drauf reagiert
                    }
                }
            break;
            case BASEPROPERTY_TEXTCOLOR:
                if ( bVoid )
                {
                    pWindow->SetControlForeground();
                }
                else
                {
                    sal_Int32 nColor;
                    if ( Value >>= nColor )
                    {
                        Color aColor( nColor );
                        pWindow->SetTextColor( aColor );
                        pWindow->SetControlForeground( aColor );
                    }
                }
            break;
            case BASEPROPERTY_FILLCOLOR:
                if ( bVoid )
                    pWindow->SetFillColor();
                else
                {
                    sal_Int32 nColor;
                    if ( Value >>= nColor )
                    {
                        Color aColor( nColor );
                        pWindow->SetFillColor( aColor );
                    }
                }
            break;
            case BASEPROPERTY_LINECOLOR:
                if ( bVoid )
                    pWindow->SetLineColor();
                else
                {
                    sal_Int32 nColor;
                    if ( Value >>= nColor )
                    {
                        Color aColor( nColor );
                        pWindow->SetLineColor( aColor );
                    }
                }
            break;
            case BASEPROPERTY_BORDER:
            {
                WinBits nStyle = pWindow->GetStyle();
                sal_uInt16 nBorder;
                Value >>= nBorder;
                if ( !nBorder )
                {
                    pWindow->SetStyle( nStyle & ~WB_BORDER );
                }
                else
                {
                    pWindow->SetStyle( nStyle | WB_BORDER );
                    pWindow->SetBorderStyle( nBorder );
                }
            }
            break;
            case BASEPROPERTY_TABSTOP:
            {
                WinBits nStyle = pWindow->GetStyle() & ~WB_TABSTOP;
                if ( !bVoid )
                {
                    sal_Bool bTab;
                    Value >>= bTab;
                    if ( bTab )
                        nStyle |= WB_TABSTOP;
                    else
                        nStyle |= WB_NOTABSTOP;
                }
                pWindow->SetStyle( nStyle );
            }
            break;
            case BASEPROPERTY_ALIGN:
            {
                switch ( eWinType )
                {
                    case WINDOW_FIXEDTEXT:
                    case WINDOW_EDIT:
                    {
                        WinBits nStyle = pWindow->GetStyle();
                        nStyle &= ~(WB_LEFT|WB_CENTER|WB_RIGHT);
                        sal_uInt16 nAlign = 0;
                        if ( !bVoid )
                            Value >>= nAlign;
                        if ( nAlign == PROPERTY_ALIGN_LEFT )
                            nStyle |= WB_LEFT;
                        else if ( nAlign == PROPERTY_ALIGN_CENTER )
                            nStyle |= WB_CENTER;
                        else
                            nStyle |= WB_RIGHT;
                        pWindow->SetStyle( nStyle );
                    }
                    break;
                }
            }
            break;
            case BASEPROPERTY_MULTILINE:
            {
                if ( eWinType == WINDOW_FIXEDTEXT )
                {
                    WinBits nStyle = pWindow->GetStyle();
                    sal_Bool bMulti;
                    Value >>= bMulti;
                    if ( bMulti )
                        nStyle |= WB_WORDBREAK;
                    else
                        nStyle &= ~WB_WORDBREAK;
                    pWindow->SetStyle( nStyle );
                }
            }
            break;
        }
    }
}

::com::sun::star::uno::Any VCLXWindow::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    if ( GetWindow() )
    {
        WindowType eWinType = GetWindow()->GetType();
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_ENABLED:
                aProp <<= (sal_Bool) GetWindow()->IsEnabled();
            break;
            case BASEPROPERTY_TEXT:
            case BASEPROPERTY_LABEL:
            case BASEPROPERTY_TITLE:
            {
                ::rtl::OUString aText = GetWindow()->GetText();
                aProp <<= aText;
            }
            break;
            case BASEPROPERTY_FONTDESCRIPTOR:
            {
                Font aFont = GetWindow()->GetControlFont();
                ::com::sun::star::awt::FontDescriptor aFD = VCLUnoHelper::CreateFontDescriptor( aFont );
                aProp <<= aFD;
            }
            break;
            case BASEPROPERTY_BACKGROUNDCOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetControlBackground().GetColor();
            break;
            case BASEPROPERTY_TEXTCOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetControlForeground().GetColor();
            break;
            case BASEPROPERTY_FILLCOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetFillColor().GetColor();
            break;
            case BASEPROPERTY_LINECOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetLineColor().GetColor();
            break;
            case BASEPROPERTY_BORDER:
            {
                sal_uInt16 nBorder = 0;
                if ( GetWindow()->GetStyle() & WB_BORDER )
                    nBorder = GetWindow()->GetBorderStyle();
                aProp <<= nBorder;
            }
            break;
            case BASEPROPERTY_TABSTOP:
                aProp <<= (sal_Bool) ( GetWindow()->GetStyle() & WB_TABSTOP ) ? sal_True : sal_False;
            break;
            case BASEPROPERTY_ALIGN:
            {
                switch ( eWinType )
                {
                    case WINDOW_FIXEDTEXT:
                    case WINDOW_EDIT:
                    {
                        WinBits nStyle = GetWindow()->GetStyle();
                        if ( nStyle & WB_LEFT )
                            aProp <<= (sal_Int16) PROPERTY_ALIGN_LEFT;
                        else if ( nStyle & WB_CENTER )
                            aProp <<= (sal_Int16) PROPERTY_ALIGN_CENTER;
                        else if ( nStyle & WB_RIGHT )
                            aProp <<= (sal_Int16) PROPERTY_ALIGN_RIGHT;
                    }
                    break;
                }
            }
            case BASEPROPERTY_MULTILINE:
            {
                if ( eWinType == WINDOW_FIXEDTEXT )
                    aProp <<= (sal_Bool) ( GetWindow()->GetStyle() & WB_WORDBREAK ) ? sal_True : sal_False;
            }
            break;
        }
    }
    return aProp;
}


// ::com::sun::star::awt::XLayoutConstrains
::com::sun::star::awt::Size VCLXWindow::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    // Diese Methode sollte nur fuer Componenten gerufen werden, die zwar
    // ueber das ::com::sun::star::awt::Toolkit erzeugt werden koennen, aber fuer die es
    // kein Interface gibt.

    Size aSz;
    if ( GetWindow() )
    {
        WindowType nWinType = GetWindow()->GetType();
        WinBits nStyle = GetWindow()->GetStyle();
        switch ( nWinType )
        {
            case WINDOW_MESSBOX:
            case WINDOW_INFOBOX:
            case WINDOW_WARNINGBOX:
            case WINDOW_ERRORBOX:
            case WINDOW_QUERYBOX:
                aSz = Size( 250, 100 );
            break;

            case WINDOW_CONTROL:
                aSz.Width() = GetWindow()->GetTextWidth( GetWindow()->GetText() )+2*12;
                aSz.Height() = GetWindow()->GetTextHeight()+2*6;
            break;

            case WINDOW_PATTERNBOX:
            case WINDOW_NUMERICBOX:
            case WINDOW_METRICBOX:
            case WINDOW_CURRENCYBOX:
            case WINDOW_DATEBOX:
            case WINDOW_TIMEBOX:
            case WINDOW_LONGCURRENCYBOX:
                aSz.Width() = GetWindow()->GetTextWidth( GetWindow()->GetText() )+2*2;
                aSz.Height() = GetWindow()->GetTextHeight()+2*2;
            break;
            case WINDOW_SCROLLBARBOX:
            case WINDOW_SCROLLBAR:
            {
                long n = GetWindow()->GetSettings().GetStyleSettings().GetScrollBarSize();
                aSz = Size( n, n );
            }
            break;
            default:    DBG_ERROR( "getMinimumSize: Unknown Type" );
        }
    }

    return ::com::sun::star::awt::Size( aSz.Width(), aSz.Height() );
}

::com::sun::star::awt::Size VCLXWindow::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXWindow::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Size aNewSize( rNewSize );
    ::com::sun::star::awt::Size aMinSize = getMinimumSize();

    if ( aNewSize.Width < aMinSize.Width )
        aNewSize.Width = aMinSize.Width;
    if ( aNewSize.Height < aMinSize.Height )
        aNewSize.Height = aMinSize.Height;

    return aNewSize;
}


// ::com::sun::star::awt::XView
sal_Bool VCLXWindow::setGraphics( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& rxDevice ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( VCLUnoHelper::GetOutputDevice( rxDevice ) )
        mxViewGraphics = rxDevice;
    else
        mxViewGraphics = NULL;

    return mxViewGraphics.is();
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > VCLXWindow::getGraphics(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    return mxViewGraphics;
}

::com::sun::star::awt::Size VCLXWindow::getSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    if ( GetWindow() )
        aSz = GetWindow()->GetSizePixel();
    return ::com::sun::star::awt::Size( aSz.Width(), aSz.Height() );
}

void VCLXWindow::draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( mxViewGraphics );
        Point aPos( nX, nY );

        if ( !pDev )
            pDev = pWindow->GetParent();

        if ( pWindow->GetParent() && !pWindow->IsVisible()
                && !pWindow->IsSystemWindow() && ( pWindow->GetParent() == pDev ) )
        {
            Point aOldPos( pWindow->GetPosPixel() );
            pWindow->SetPosPixel( aPos );

            // Erstmal ein Update auf den Parent, damit nicht beim Update
            // auf dieses Fenster noch ein Paint vom Parent abgearbeitet wird,
            // wo dann ggf. dieses Fenster sofort wieder gehidet wird.
            if( pWindow->GetParent() )
                pWindow->GetParent()->Update();

            pWindow->Show();
            pWindow->Update();
            pWindow->SetParentUpdateMode( sal_False );
            pWindow->Hide();
            pWindow->SetParentUpdateMode( sal_True );

            pWindow->SetPosPixel( aOldPos );
        }
        else if ( pDev )
        {
            Size aSz = pWindow->GetSizePixel();
            aSz = pDev->PixelToLogic( aSz );
            Point aP = pDev->PixelToLogic( aPos );
            pWindow->Draw( pDev, aP, aSz, WINDOW_DRAW_NOCONTROLS );
        }
    }
}

void VCLXWindow::setZoom( float fZoomX, float fZoomY ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->SetZoom( Fraction( fZoomX ) );
}








