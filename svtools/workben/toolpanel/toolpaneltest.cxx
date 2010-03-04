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
#include "svtools/toolpanel/tablayouter.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <tools/diagnose_ex.h>
#include <ucbhelper/contentbroker.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/wrkwin.hxx>

namespace svt { namespace toolpanel
{

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
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
        ,m_aEdit( this, WB_BORDER )
    {
        SetLineColor();
        SetFillColor( i_rColor );

        m_aEdit.Show();
    }

    virtual void Paint( const Rectangle& i_rRect )
    {
        DrawRect( i_rRect );
    }

    virtual void GetFocus()
    {
        m_aEdit.GrabFocus();
    }

    virtual void Resize()
    {
        const Size aOutputSize( GetOutputSizePixel() );
        m_aEdit.SetPosSizePixel(
            Point( 20, 20 ),
            Size( aOutputSize.Width() - 40, 20 )
        );
    }

private:
    Edit    m_aEdit;
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
    virtual ::rtl::OUString GetDisplayName() const;
    virtual Image GetImage() const;
    virtual void Show();
    virtual void Hide();
    virtual void SetPosSizePixel( const Rectangle& i_rPanelPlayground );
    virtual void GrabFocus();
    virtual bool HasFocus() const;

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
void ColoredPanel::GrabFocus()
{
    m_aWindow.GrabFocus();
}

//-----------------------------------------------------------------------------
bool ColoredPanel::HasFocus() const
{
    return m_aWindow.HasChildPathFocus();
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
//= OptionsWindow
//=============================================================================
class PanelDemoMainWindow;
class OptionsWindow : public Window
{
public:
    OptionsWindow( PanelDemoMainWindow& i_rParent );

    virtual void Resize();
    virtual void GetFocus();

private:
    DECL_LINK( OnAlignmentChanged, void* );

private:
    RadioButton     m_aAlignLeft;
    RadioButton     m_aAlignRight;
};

//=============================================================================
//= PanelDemoMainWindow
//=============================================================================
class PanelDemoMainWindow : public WorkWindow
{
public:
                    PanelDemoMainWindow();
                    ~PanelDemoMainWindow();

    // window overridables
    virtual void    Resize();

public:
    // operations
    void AlignTabs( const ::svt::TabAlignment i_eAlignment );

protected:
    virtual void    GetFocus();

private:
    ToolPanelDeck   m_aToolPanelDeck;
    OptionsWindow   m_aDemoOptions;
};

//=============================================================================
//= PanelDemoMainWindow - implementation
//=============================================================================
//-----------------------------------------------------------------------------
OptionsWindow::OptionsWindow( PanelDemoMainWindow& i_rParent )
    :Window( &i_rParent, WB_BORDER | WB_DIALOGCONTROL )
    ,m_aAlignLeft( this, WB_GROUP )
    ,m_aAlignRight( this, 0 )
{
    SetBorderStyle( WINDOW_BORDER_MONO );
    const Color aFaceColor( GetSettings().GetStyleSettings().GetFaceColor() );
    SetBackground( aFaceColor );

    RadioButton* pRadios[] =
    {
        &m_aAlignLeft, &m_aAlignRight
    };
    const sal_Char* pTexts[] =
    {
        "Left", "Right"
    };
    for ( size_t i=0; i < sizeof( pRadios ) / sizeof( pRadios[0] ); ++i )
    {
        pRadios[i]->SetText( String::CreateFromAscii( pTexts[i] ) );
        pRadios[i]->SetControlBackground( aFaceColor );
        pRadios[i]->Show();
        pRadios[i]->SetToggleHdl( LINK( this, OptionsWindow, OnAlignmentChanged ) );
    }

    m_aAlignRight.Check();

    Show();
}

//-----------------------------------------------------------------------------
void OptionsWindow::GetFocus()
{
    Window::GetFocus();
    RadioButton* pRadios[] =
    {
        &m_aAlignLeft, &m_aAlignRight
    };
    for ( size_t i=0; i < sizeof( pRadios ) / sizeof( pRadios[0] ); ++i )
    {
        if ( pRadios[i]->IsChecked() )
        {
            pRadios[i]->GrabFocus();
            break;
        }
    }
}

//-----------------------------------------------------------------------------
void OptionsWindow::Resize()
{
    Window::Resize();

    const Size aSpacing( LogicToPixel( Size( 3, 3 ), MAP_APPFONT ) );
    const Size aOutputSize( GetOutputSizePixel() );

    const Size aRadioSize(
        aOutputSize.Width() - 2 * aSpacing.Width(),
        LogicToPixel( Size( 0, 8 ), MAP_APPFONT ).Height()
    );

    Point aRadioPos( aSpacing.Width(), aSpacing.Height() );
    RadioButton* pRadios[] =
    {
        &m_aAlignLeft, &m_aAlignRight
    };
    for ( size_t i=0; i < sizeof( pRadios ) / sizeof( pRadios[0] ); ++i )
    {
        pRadios[i]->SetPosSizePixel( aRadioPos, aRadioSize );
        aRadioPos.Move( 0, aRadioSize.Height() + aSpacing.Height() );
    }
}

//-----------------------------------------------------------------------------
IMPL_LINK( OptionsWindow, OnAlignmentChanged, void*, /**/ )
{
    PanelDemoMainWindow& rController( dynamic_cast< PanelDemoMainWindow& >( *GetParent() ) );
    if ( m_aAlignLeft.IsChecked() )
    {
        rController.AlignTabs( TABS_LEFT );
    }
    else if ( m_aAlignRight.IsChecked() )
    {
        rController.AlignTabs( TABS_RIGHT );
    }
    return 0L;
}
//=============================================================================
//= PanelDemoMainWindow - implementation
//=============================================================================
//-----------------------------------------------------------------------------
PanelDemoMainWindow::PanelDemoMainWindow()
    :WorkWindow( NULL, WB_APP | WB_STDWORK | WB_CLIPCHILDREN )
    ,m_aToolPanelDeck( *this, WB_BORDER )
    ,m_aDemoOptions( *this )
{
    const Color aFaceColor( GetSettings().GetStyleSettings().GetFaceColor() );

    SetBackground( aFaceColor );

    m_aToolPanelDeck.SetPosSizePixel( Point( 20, 20 ), Size( 500, 300 ) );
    m_aToolPanelDeck.SetBorderStyle( WINDOW_BORDER_MONO );

    PToolPanelContainer pPanels( m_aToolPanelDeck.GetPanels() );
    pPanels->InsertPanel( PToolPanel( new ColoredPanel( m_aToolPanelDeck, Color( COL_RED ), "Red" ) ), pPanels->GetPanelCount() );
    pPanels->InsertPanel( PToolPanel( new ColoredPanel( m_aToolPanelDeck, Color( COL_GREEN ), "Some flavor of Green" ) ), pPanels->GetPanelCount() );
    pPanels->InsertPanel( PToolPanel( new ColoredPanel( m_aToolPanelDeck, RGB_COLORDATA( 255, 255, 0 ), "Yellow is ugly" ) ), pPanels->GetPanelCount() );

    m_aToolPanelDeck.ActivatePanel( 0 );
    m_aToolPanelDeck.Show();

    SetText( String::CreateFromAscii( "ToolPanelDeck Demo Application" ) );
    Show();

    Help::EnableQuickHelp();

    GetSystemWindow()->GetTaskPaneList()->AddWindow( &m_aToolPanelDeck );
    GetSystemWindow()->GetTaskPaneList()->AddWindow( &m_aDemoOptions );
}

//-----------------------------------------------------------------------------
PanelDemoMainWindow::~PanelDemoMainWindow()
{
    GetSystemWindow()->GetTaskPaneList()->RemoveWindow( &m_aDemoOptions );
    GetSystemWindow()->GetTaskPaneList()->RemoveWindow( &m_aToolPanelDeck );
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
    aSize.Width() -= 140;
    aSize.Height() -= 40;
    m_aToolPanelDeck.SetPosSizePixel( Point( 20, 20 ), aSize );

    m_aDemoOptions.SetPosSizePixel(
        Point( 20 + aSize.Width(), 20 ),
        Size( 100, aSize.Height() )
    );
}

//-----------------------------------------------------------------------------
void PanelDemoMainWindow::AlignTabs( const ::svt::TabAlignment i_eAlignment )
{
    m_aToolPanelDeck.SetLayouter( PDeckLayouter( new TabDeckLayouter( i_eAlignment, m_aToolPanelDeck ) ) );
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
    // create service factory
    Reference< XMultiServiceFactory >  xSMgr = createApplicationServiceManager();
    ::comphelper::setProcessServiceFactory( xSMgr );

    // initialize the UCB
    Sequence< Any > aArgs(2);
    aArgs[0] <<= rtl::OUString::createFromAscii( "Local" );
    aArgs[1] <<= rtl::OUString::createFromAscii( "Office" );
    ::ucbhelper::ContentBroker::initialize( xSMgr, aArgs );

    // run the application
    PanelDemoMainWindow aWindow;
    Execute();
}

PanelDemo aTheApplication;

} } // namespace ::svt::toolpanel
