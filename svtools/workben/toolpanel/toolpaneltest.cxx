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
#include <vcl/fixed.hxx>
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
    DECL_LINK( OnRadioToggled, RadioButton* );

private:
    FixedLine       m_aAlignmentHeader;
    RadioButton     m_aAlignLeft;
    RadioButton     m_aAlignRight;
    FixedLine       m_aTabItemContent;
    RadioButton     m_aImagesAndText;
    RadioButton     m_aImagesOnly;
    RadioButton     m_aTextOnly;
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
    void SetTabItemContent( const TabItemContent i_eItemContent );

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
    ,m_aAlignmentHeader( this )
    ,m_aAlignLeft( this, WB_GROUP )
    ,m_aAlignRight( this, 0 )
    ,m_aTabItemContent( this )
    ,m_aImagesAndText( this )
    ,m_aImagesOnly( this )
    ,m_aTextOnly( this )
{
    SetBorderStyle( WINDOW_BORDER_MONO );
    const Color aFaceColor( GetSettings().GetStyleSettings().GetFaceColor() );
    SetBackground( aFaceColor );

    Window* pControls[] =
    {
        &m_aAlignmentHeader, &m_aAlignLeft, &m_aAlignRight, &m_aTabItemContent, &m_aImagesAndText, &m_aImagesOnly, &m_aTextOnly
    };
    const sal_Char* pTexts[] =
    {
        "Tab Bar Alignment", "Left", "Right", "Tab Items", "Images and Text", "Images only", "Text only"
    };
    for ( size_t i=0; i < sizeof( pControls ) / sizeof( pControls[0] ); ++i )
    {
        pControls[i]->SetText( String::CreateFromAscii( pTexts[i] ) );
        pControls[i]->SetControlBackground( aFaceColor );
        pControls[i]->Show();

        if ( pControls[i]->GetType() == WINDOW_RADIOBUTTON )
            static_cast< RadioButton* >( pControls[i] )->SetToggleHdl( LINK( this, OptionsWindow, OnRadioToggled ) );
    }

    m_aAlignRight.Check();
    m_aImagesAndText.Check();

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

    const Size aOutputSize( GetOutputSizePixel() );

    const Size aSpacing( LogicToPixel( Size( 3, 3 ), MAP_APPFONT ) );
    const long nIndent( LogicToPixel( Size( 6, 9 ), MAP_APPFONT ).Width() );
    const long nLineHeight( LogicToPixel( Size( 0, 8 ), MAP_APPFONT ).Height() );

    const Size aSuperordinateSize( aOutputSize.Width() - 2 * aSpacing.Width(), nLineHeight );
    const long nSuperordinateX = aSpacing.Width();

    const Size aSubordinateSize( aOutputSize.Width() - 2 * aSpacing.Width() - nIndent, nLineHeight );
    const long nSubordinateX = aSpacing.Width() + nIndent;

    Point aItemPos( nSuperordinateX, aSpacing.Height() );

    struct ControlRow
    {
        Window* pWindow;
        bool    bSubordinate;
        ControlRow( Window& i_rWindow, const bool i_bSubordinate ) : pWindow( &i_rWindow ), bSubordinate( i_bSubordinate ) { }
    };
    ControlRow aControlRows[] =
    {
        ControlRow( m_aAlignmentHeader, false ),
        ControlRow( m_aAlignLeft,       true ),
        ControlRow( m_aAlignRight,      true ),
        ControlRow( m_aTabItemContent,  false ),
        ControlRow( m_aImagesAndText,   true ),
        ControlRow( m_aImagesOnly,      true ),
        ControlRow( m_aTextOnly,        true )
    };
    bool bPreviousWasSubordinate = false;
    for ( size_t i=0; i < sizeof( aControlRows ) / sizeof( aControlRows[0] ); ++i )
    {
        aItemPos.X() = ( aControlRows[i].bSubordinate ) ? nSubordinateX : nSuperordinateX;

        if ( bPreviousWasSubordinate && !aControlRows[i].bSubordinate )
            aItemPos.Y() += aSpacing.Height();
        bPreviousWasSubordinate = aControlRows[i].bSubordinate;

        const Size& rControlSize = ( aControlRows[i].bSubordinate ) ? aSubordinateSize : aSuperordinateSize;
        aControlRows[i].pWindow->SetPosSizePixel( aItemPos, rControlSize );

        aItemPos.Move( 0, rControlSize.Height() + aSpacing.Height() );
    }
}

//-----------------------------------------------------------------------------
IMPL_LINK( OptionsWindow, OnRadioToggled, RadioButton*, i_pRadioButton )
{
    PanelDemoMainWindow& rController( dynamic_cast< PanelDemoMainWindow& >( *GetParent() ) );

    if ( i_pRadioButton->IsChecked() )
    {
        if ( i_pRadioButton == &m_aAlignLeft )
        {
            rController.AlignTabs( TABS_LEFT );
        }
        else if ( i_pRadioButton == &m_aAlignRight )
        {
            rController.AlignTabs( TABS_RIGHT );
        }
        else if ( i_pRadioButton == &m_aImagesAndText )
        {
            rController.SetTabItemContent( TABITEM_IMAGE_AND_TEXT );
        }
        else if ( i_pRadioButton == &m_aImagesOnly )
        {
            rController.SetTabItemContent( TABITEM_IMAGE_ONLY );
        }
        else if ( i_pRadioButton == &m_aTextOnly )
        {
            rController.SetTabItemContent( TABITEM_TEXT_ONLY );
        }
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
    aSize.Width() -= 190;
    aSize.Height() -= 40;
    m_aToolPanelDeck.SetPosSizePixel( Point( 20, 20 ), aSize );

    m_aDemoOptions.SetPosSizePixel(
        Point( 20 + aSize.Width(), 20 ),
        Size( 150, aSize.Height() )
    );
}

//-----------------------------------------------------------------------------
void PanelDemoMainWindow::AlignTabs( const ::svt::TabAlignment i_eAlignment )
{
    TabItemContent eCurrentItemContent( TABITEM_IMAGE_AND_TEXT );
    TabDeckLayouter* pLayouter = dynamic_cast< TabDeckLayouter* >( m_aToolPanelDeck.GetLayouter().get() );
    OSL_ENSURE( pLayouter, "PanelDemoMainWindow::AlignTabs: wrong layouter!" );
    if ( pLayouter )
        eCurrentItemContent = pLayouter->GetTabItemContent();

    m_aToolPanelDeck.SetLayouter( PDeckLayouter( new TabDeckLayouter( m_aToolPanelDeck, i_eAlignment, eCurrentItemContent ) ) );
}

//-----------------------------------------------------------------------------
void PanelDemoMainWindow::SetTabItemContent( const TabItemContent i_eItemContent )
{
    TabDeckLayouter* pLayouter = dynamic_cast< TabDeckLayouter* >( m_aToolPanelDeck.GetLayouter().get() );
    OSL_ENSURE( pLayouter, "PanelDemoMainWindow::SetTabItemContent: wrong layouter!" );
        // we currently use tab layouters only ...
    if ( !pLayouter )
        return;

    pLayouter->SetTabItemContent( i_eItemContent );
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
