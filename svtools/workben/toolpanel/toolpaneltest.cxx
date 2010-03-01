/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "svtools/toolpanel/toolpaneldeck.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

namespace svt { namespace toolpanel
{

using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::uno::XComponentContext;

//=============================================================================
//= PanelDemo
//=============================================================================
class PanelDemo : public Application
{
public:
    virtual void Main();

private:
    static Reference< XMultiServiceFactory > createApplicationServiceManager();
};

//=============================================================================
//= ColoredPanelWindow
//=============================================================================
class ColoredPanelWindow : public Window
{
public:
    ColoredPanelWindow( Window& i_rParent, const Color& i_rColor )
        :Window( &i_rParent )
    {
        SetLineColor();
        SetFillColor( i_rColor );
    }

    virtual void Paint( const Rectangle& i_rRect )
    {
        DrawRect( i_rRect );
    }
};

//=============================================================================
//= ColoredPanel
//=============================================================================
class ColoredPanel : public IToolPanel
{
public:
    ColoredPanel( Window& i_rParent, const Color& i_rColor, const sal_Char* i_pAsciiPanelName );
    ~ColoredPanel();

    // IToolPanel
    virtual void Show();
    virtual void Hide();
    virtual void SetPosSizePixel( const Rectangle& i_rPanelPlayground );
    virtual ::rtl::OUString GetDisplayName() const;
    virtual Image GetImage() const;

    // IReference
    virtual oslInterlockedCount SAL_CALL acquire();
    virtual oslInterlockedCount SAL_CALL release();

private:
    oslInterlockedCount m_refCount;
    ColoredPanelWindow  m_aWindow;
    ::rtl::OUString     m_aPanelName;
    BitmapEx            m_aPanelIcon;
};

//=============================================================================
//= ColoredPanel
//=============================================================================
//-----------------------------------------------------------------------------
ColoredPanel::ColoredPanel( Window& i_rParent, const Color& i_rColor, const sal_Char* i_pAsciiPanelName )
    :m_refCount(0)
    ,m_aWindow( i_rParent, i_rColor )
    ,m_aPanelName( ::rtl::OUString::createFromAscii( i_pAsciiPanelName ) )
    ,m_aPanelIcon()
{
    Bitmap aBitmap( Size( 16, 16 ), 8 );
    m_aPanelIcon = BitmapEx( aBitmap );
    m_aPanelIcon.Erase( i_rColor );
}

//-----------------------------------------------------------------------------
ColoredPanel::~ColoredPanel()
{
}

//-----------------------------------------------------------------------------
oslInterlockedCount SAL_CALL ColoredPanel::acquire()
{
    return osl_incrementInterlockedCount( &m_refCount );
}

//-----------------------------------------------------------------------------
oslInterlockedCount SAL_CALL ColoredPanel::release()
{
    oslInterlockedCount newCount = osl_decrementInterlockedCount( &m_refCount );
    if ( 0 == newCount )
        delete this;
    return newCount;
}

//-----------------------------------------------------------------------------
void ColoredPanel::Show()
{
    m_aWindow.Show();
}

//-----------------------------------------------------------------------------
void ColoredPanel::Hide()
{
    m_aWindow.Hide();
}

//-----------------------------------------------------------------------------
void ColoredPanel::SetPosSizePixel( const Rectangle& i_rPanelPlayground )
{
    m_aWindow.SetPosSizePixel( i_rPanelPlayground.TopLeft(), i_rPanelPlayground.GetSize() );
}

//-----------------------------------------------------------------------------
::rtl::OUString ColoredPanel::GetDisplayName() const
{
    return m_aPanelName;
}

//-----------------------------------------------------------------------------
Image ColoredPanel::GetImage() const
{
    return Image( m_aPanelIcon );
}

//=============================================================================
//= PanelDemoMainWindow
//=============================================================================
class PanelDemoMainWindow : public WorkWindow
{
private:
    ToolPanelDeck   m_aToolPanelDeck;

protected:
    virtual void    GetFocus();

public:
                    PanelDemoMainWindow();
                    ~PanelDemoMainWindow();

    virtual void    Resize();
};

//=============================================================================
//= PanelDemoMainWindow
//=============================================================================
//-----------------------------------------------------------------------------
PanelDemoMainWindow::PanelDemoMainWindow()
    :WorkWindow( NULL, WB_APP | WB_STDWORK | WB_CLIPCHILDREN )
    ,m_aToolPanelDeck( *this, WB_BORDER )
{
    m_aToolPanelDeck.SetPosSizePixel( Point( 20, 20 ), Size( 500, 300 ) );
    m_aToolPanelDeck.SetBorderStyle( WINDOW_BORDER_MONO );

    PToolPanelContainer pPanels( m_aToolPanelDeck.GetPanels() );
    pPanels->InsertPanel( PToolPanel( new ColoredPanel( m_aToolPanelDeck, Color( COL_RED ), "Red" ) ), pPanels->GetPanelCount() );
    pPanels->InsertPanel( PToolPanel( new ColoredPanel( m_aToolPanelDeck, Color( COL_GREEN ), "Some flavor of Green" ) ), pPanels->GetPanelCount() );
    pPanels->InsertPanel( PToolPanel( new ColoredPanel( m_aToolPanelDeck, RGB_COLORDATA( 255, 255, 0 ), "Yellow is ugly" ) ), pPanels->GetPanelCount() );

    m_aToolPanelDeck.ActivatePanel( 0 );
    m_aToolPanelDeck.Show();

    SetBackground( Color( COL_LIGHTGRAY ) );

    SetText( String::CreateFromAscii( "ToolPanelDeck Demo Application" ) );
    Show();

    Help::EnableQuickHelp();
}

//-----------------------------------------------------------------------------
PanelDemoMainWindow::~PanelDemoMainWindow()
{
}

//-----------------------------------------------------------------------------
void PanelDemoMainWindow::GetFocus()
{
    WorkWindow::GetFocus();
    m_aToolPanelDeck.GrabFocus();
}

//-----------------------------------------------------------------------------
void PanelDemoMainWindow::Resize()
{
    WorkWindow::Resize();
    Size aSize( GetOutputSizePixel() );
    aSize.Width() -= 40;
    aSize.Height() -= 40;
    m_aToolPanelDeck.SetPosSizePixel( Point( 20, 20 ), aSize );
}

//=============================================================================
//= PanelDemo
//=============================================================================
//-----------------------------------------------------------------------------
Reference< XMultiServiceFactory > PanelDemo::createApplicationServiceManager()
{
    Reference< XMultiServiceFactory > xMS;
    try
    {
        Reference< XComponentContext >  xComponentContext = ::cppu::defaultBootstrap_InitialComponentContext();
        if ( xComponentContext.is() )
            xMS = xMS.query( xComponentContext->getServiceManager() );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return xMS;
}

//-----------------------------------------------------------------------------
void __EXPORT PanelDemo::Main()
{
    Reference< XMultiServiceFactory >  xSMgr = createApplicationServiceManager();
    ::comphelper::setProcessServiceFactory( xSMgr );

    PanelDemoMainWindow aWindow;
    Execute();
}

PanelDemo aTheApplication;

} } // namespace ::svt::toolpanel
