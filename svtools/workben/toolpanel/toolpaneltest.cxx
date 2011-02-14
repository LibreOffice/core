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

#include "ctrlbox.hxx"
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
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/wrkwin.hxx>

namespace svt { namespace toolpanel
{

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::accessibility::XAccessible;

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
    ColoredPanelWindow( Window& i_rParent, const Color& i_rColor, const String& i_rTitle )
        :Window( &i_rParent )
        ,m_aEdit( this, WB_BORDER )
        ,m_aTabControl( this )
        ,m_sTitle( i_rTitle )
    {
        SetLineColor();
        SetFillColor( i_rColor );

        m_aEdit.Show();
        m_aTabControl.Show();

        const sal_Char* pTabTitles[] =
        {
            "This", "is a", "Tab", "Control", "intended", "for", "comparison"
        };
        for ( size_t i=0; i < sizeof( pTabTitles ) / sizeof( pTabTitles[0] ); ++i )
        {
            String sText( String::CreateFromAscii( pTabTitles[i] ) );
            m_aTabControl.InsertPage( i + 1, sText );
        }
    }

    virtual void Paint( const Rectangle& /*i_rRect*/ )
    {
        const Size aOutputSize( GetOutputSizePixel() );
        const Rectangle aTitleRect( Point( 10, 10 ), Size( aOutputSize.Width() - 20, 20 ) );
        DrawRect( aTitleRect );
        SetTextColor( GetFillColor().IsDark() ? COL_WHITE : COL_BLACK );
        DrawText( aTitleRect, m_sTitle, TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER );
    }

    virtual void GetFocus()
    {
        m_aEdit.GrabFocus();
    }

    virtual void Resize()
    {
        const Size aOutputSize( GetOutputSizePixel() );
        m_aEdit.SetPosSizePixel(
            Point( 20, 40 ),
            Size( aOutputSize.Width() - 40, 20 )
        );
        m_aTabControl.SetPosSizePixel(
            Point( 20, 70 ),
            Size( aOutputSize.Width() - 40, 150 )
        );
    }

private:
    Edit        m_aEdit;
    TabControl  m_aTabControl;
    String      m_sTitle;
};

//=============================================================================
//= ColoredPanel
//=============================================================================
class ColoredPanel : public IToolPanel
{
public:
    ColoredPanel( Window& i_rParent, const Color& i_rColor, const sal_Char* i_pAsciiPanelName );
    ColoredPanel( Window& i_rParent, const Color& i_rColor, const String& i_rPanelName );
    ~ColoredPanel();

    // IToolPanel
    virtual ::rtl::OUString GetDisplayName() const;
    virtual Image GetImage() const;
    virtual rtl::OString GetHelpID() const;
    virtual void Activate( Window& i_rParentWindow );
    virtual void Deactivate();
    virtual void SetSizePixel( const Size& i_rPanelWindowSize );
    virtual void GrabFocus();
    virtual void Dispose();
    virtual Reference< XAccessible > CreatePanelAccessible( const Reference< XAccessible >& i_rParentAccessible );

    // IReference
    virtual oslInterlockedCount SAL_CALL acquire();
    virtual oslInterlockedCount SAL_CALL release();

private:
    oslInterlockedCount m_refCount;
    ::std::auto_ptr< ColoredPanelWindow >
                        m_pWindow;
    ::rtl::OUString     m_aPanelName;
    BitmapEx            m_aPanelIcon;
};

//=============================================================================
//= ColoredPanel
//=============================================================================
//-----------------------------------------------------------------------------
ColoredPanel::ColoredPanel( Window& i_rParent, const Color& i_rColor, const sal_Char* i_pAsciiPanelName )
    :m_refCount(0)
    ,m_pWindow( new ColoredPanelWindow( i_rParent, i_rColor, ::rtl::OUString::createFromAscii( i_pAsciiPanelName ) ) )
    ,m_aPanelName( ::rtl::OUString::createFromAscii( i_pAsciiPanelName ) )
    ,m_aPanelIcon()
{
    Bitmap aBitmap( Size( 16, 16 ), 8 );
    m_aPanelIcon = BitmapEx( aBitmap );
    m_aPanelIcon.Erase( i_rColor );
}

//-----------------------------------------------------------------------------
ColoredPanel::ColoredPanel( Window& i_rParent, const Color& i_rColor, const String& i_rPanelName )
    :m_refCount(0)
    ,m_pWindow( new ColoredPanelWindow( i_rParent, i_rColor, i_rPanelName ) )
    ,m_aPanelName( i_rPanelName )
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
void ColoredPanel::Activate( Window& i_rParentWindow )
{
    ENSURE_OR_RETURN_VOID( m_pWindow.get(), "disposed!" );
    OSL_ENSURE( &i_rParentWindow == m_pWindow->GetParent(), "ColoredPanel::Activate: unexpected new parent window!" );
        // the documentation of IToolPanel::Activate says it is guaranteed that the parent window is
        // always the same ...
    m_pWindow->SetPosSizePixel( Point(), i_rParentWindow.GetSizePixel() );
    m_pWindow->Show();
}

//-----------------------------------------------------------------------------
void ColoredPanel::Deactivate()
{
    ENSURE_OR_RETURN_VOID( m_pWindow.get(), "disposed!" );
    m_pWindow->Hide();
}

//-----------------------------------------------------------------------------
void ColoredPanel::SetSizePixel( const Size& i_rPanelWindowSize )
{
    ENSURE_OR_RETURN_VOID( m_pWindow.get(), "disposed!" );
    m_pWindow->SetSizePixel( i_rPanelWindowSize );
}

//-----------------------------------------------------------------------------
void ColoredPanel::GrabFocus()
{
    ENSURE_OR_RETURN_VOID( m_pWindow.get(), "disposed!" );
    m_pWindow->GrabFocus();
}

//-----------------------------------------------------------------------------
void ColoredPanel::Dispose()
{
    ENSURE_OR_RETURN_VOID( m_pWindow.get(), "disposed!" );
    m_pWindow.reset();
}

//-----------------------------------------------------------------------------
Reference< XAccessible > ColoredPanel::CreatePanelAccessible( const Reference< XAccessible >& i_rParentAccessible )
{
    ENSURE_OR_RETURN( m_pWindow.get(), "disposed!", NULL );
    (void)i_rParentAccessible;
    return m_pWindow->GetAccessible();
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

//-----------------------------------------------------------------------------
rtl::OString ColoredPanel::GetHelpID() const
{
    return rtl::OString();
}

//=============================================================================
//= OptionsWindow
//=============================================================================
class PanelDemoMainWindow;
class OptionsWindow :public Window
                    ,public ::svt::IToolPanelDeckListener
{
public:
    OptionsWindow( PanelDemoMainWindow& i_rParent );
    ~OptionsWindow();

    // Window overridables
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    StateChanged( StateChangedType i_nStateChange );

    // IToolPanelDeckListener
    virtual void PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition );
    virtual void PanelRemoved( const size_t i_nPosition );
    virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
    virtual void LayouterChanged( const PDeckLayouter& i_rNewLayouter );
    virtual void Dying();

private:
    DECL_LINK( OnRadioToggled, RadioButton* );
    DECL_LINK( OnListEntrySelected, ListBox* );
    DECL_LINK( OnListEntryDoubleClicked, ListBox* );
    DECL_LINK( OnButtonClicked, PushButton* );
    DECL_LINK( OnEditModified, Edit* );

    void    impl_initPanelList();
    void    impl_updateRemoveButton();
    void    impl_updateInsertButton();

private:
    FixedLine       m_aAlignmentHeader;
    RadioButton     m_aAlignLeft;
    RadioButton     m_aAlignRight;
    RadioButton     m_aAlignTop;
    RadioButton     m_aAlignBottom;
    FixedLine       m_aTabItemContent;
    RadioButton     m_aImagesAndText;
    RadioButton     m_aImagesOnly;
    RadioButton     m_aTextOnly;
    RadioButton     m_aAutomaticContent;

    FixedLine       m_aPanelsHeader;
    ListBox         m_aPanelList;
    PushButton      m_aRemovePanel;
    ColorListBox    m_aColors;
    Edit            m_aNewPanelName;
    PushButton      m_aInsertPanel;
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

    // member access
    IToolPanelDeck& GetToolPanelDeck();
    PToolPanel      CreateToolPanel( const Color& i_rColor, const String& i_rPanelName );

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
    ,m_aAlignTop( this, 0 )
    ,m_aAlignBottom( this, 0 )
    ,m_aTabItemContent( this )
    ,m_aImagesAndText( this )
    ,m_aImagesOnly( this )
    ,m_aTextOnly( this )
    ,m_aAutomaticContent( this )
    ,m_aPanelsHeader( this )
    ,m_aPanelList( this )
    ,m_aRemovePanel( this )
    ,m_aColors( this, WB_DROPDOWN )
    ,m_aNewPanelName( this, WB_BORDER )
    ,m_aInsertPanel( this )
{
    SetBorderStyle( WINDOW_BORDER_MONO );

    m_aColors.InsertEntry( Color( COL_BLACK ),         String( RTL_CONSTASCII_USTRINGPARAM( "Black" ) ) );
    m_aColors.InsertEntry( Color( COL_BLUE ),          String( RTL_CONSTASCII_USTRINGPARAM( "Blue" ) ) );
    m_aColors.InsertEntry( Color( COL_GREEN ),         String( RTL_CONSTASCII_USTRINGPARAM( "Green" ) ) );
    m_aColors.InsertEntry( Color( COL_CYAN ),          String( RTL_CONSTASCII_USTRINGPARAM( "Cyan" ) ) );
    m_aColors.InsertEntry( Color( COL_RED ),           String( RTL_CONSTASCII_USTRINGPARAM( "Red" ) ) );
    m_aColors.InsertEntry( Color( COL_MAGENTA ),       String( RTL_CONSTASCII_USTRINGPARAM( "Magenta" ) ) );
    m_aColors.InsertEntry( Color( COL_BROWN ),         String( RTL_CONSTASCII_USTRINGPARAM( "Brown" ) ) );
    m_aColors.InsertEntry( Color( COL_GRAY ),          String( RTL_CONSTASCII_USTRINGPARAM( "Gray" ) ) );
    m_aColors.InsertEntry( Color( COL_LIGHTGRAY ),     String( RTL_CONSTASCII_USTRINGPARAM( "Light Gray" ) ) );
    m_aColors.InsertEntry( Color( COL_LIGHTBLUE ),     String( RTL_CONSTASCII_USTRINGPARAM( "Light Blue" ) ) );
    m_aColors.InsertEntry( Color( COL_LIGHTGREEN ),    String( RTL_CONSTASCII_USTRINGPARAM( "Light Green" ) ) );
    m_aColors.InsertEntry( Color( COL_LIGHTCYAN ),     String( RTL_CONSTASCII_USTRINGPARAM( "Light Cyan" ) ) );
    m_aColors.InsertEntry( Color( COL_LIGHTRED ),      String( RTL_CONSTASCII_USTRINGPARAM( "Light Red" ) ) );
    m_aColors.InsertEntry( Color( COL_LIGHTMAGENTA ),  String( RTL_CONSTASCII_USTRINGPARAM( "Light Magenta" ) ) );
    m_aColors.InsertEntry( Color( COL_YELLOW ),        String( RTL_CONSTASCII_USTRINGPARAM( "Yellow" ) ) );
    m_aColors.InsertEntry( Color( COL_WHITE ),         String( RTL_CONSTASCII_USTRINGPARAM( "White" ) ) );
    m_aColors.SetDropDownLineCount( 16 );

    Window* pControls[] =
    {
        &m_aAlignmentHeader, &m_aAlignLeft, &m_aAlignRight, &m_aAlignTop, &m_aAlignBottom, &m_aTabItemContent,
        &m_aImagesAndText, &m_aImagesOnly, &m_aTextOnly, &m_aAutomaticContent, &m_aPanelsHeader, &m_aPanelList,
        &m_aRemovePanel, &m_aColors, &m_aNewPanelName, &m_aInsertPanel
    };
    const sal_Char* pTexts[] =
    {
        "Tab Bar Alignment", "Left", "Right", "Top", "Bottom", "Tab Items", "Images and Text", "Images only",
        "Text only", "Automatic", "Panels", "", "Remove Panel", "", "", "Insert Panel"
    };
    for ( size_t i=0; i < sizeof( pControls ) / sizeof( pControls[0] ); ++i )
    {
        const WindowType eWindowType = pControls[i]->GetType();

        pControls[i]->SetText( String::CreateFromAscii( pTexts[i] ) );
        pControls[i]->Show();

        if ( eWindowType == WINDOW_RADIOBUTTON )
            static_cast< RadioButton* >( pControls[i] )->SetToggleHdl( LINK( this, OptionsWindow, OnRadioToggled ) );

        if  ( eWindowType == WINDOW_LISTBOX )
        {
            static_cast< ListBox* >( pControls[i] )->SetSelectHdl( LINK( this, OptionsWindow, OnListEntrySelected ) );
            static_cast< ListBox* >( pControls[i] )->SetDoubleClickHdl( LINK( this, OptionsWindow, OnListEntryDoubleClicked ) );
        }

        if ( eWindowType == WINDOW_PUSHBUTTON )
        {
            static_cast< PushButton* >( pControls[i] )->SetClickHdl( LINK( this, OptionsWindow, OnButtonClicked ) );
        }

        if ( eWindowType == WINDOW_EDIT )
        {
            static_cast< Edit* >( pControls[i] )->SetModifyHdl( LINK( this, OptionsWindow, OnEditModified ) );
        }
    }

    m_aAlignRight.Check();
    m_aImagesAndText.Check();

    Show();
}

//-----------------------------------------------------------------------------
OptionsWindow::~OptionsWindow()
{
}

//-----------------------------------------------------------------------------
void OptionsWindow::impl_updateInsertButton()
{
    m_aInsertPanel.Enable( ( m_aColors.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND ) && ( m_aNewPanelName.GetText().Len() > 0 ) );
}

//-----------------------------------------------------------------------------
void OptionsWindow::impl_updateRemoveButton()
{
    m_aRemovePanel.Enable( m_aPanelList.GetSelectEntryCount() > 0 );
}

//-----------------------------------------------------------------------------
void OptionsWindow::impl_initPanelList()
{
    m_aPanelList.Clear();

    PanelDemoMainWindow& rController( dynamic_cast< PanelDemoMainWindow& >( *GetParent() ) );
    IToolPanelDeck& rPanelDeck( rController.GetToolPanelDeck() );

    for ( size_t i=0; i<rPanelDeck.GetPanelCount(); ++i )
    {
        PToolPanel pPanel = rPanelDeck.GetPanel( i );
        m_aPanelList.InsertEntry( pPanel->GetDisplayName(), pPanel->GetImage() );
    }
    ActivePanelChanged( ::boost::optional< size_t >(), rPanelDeck.GetActivePanel() );

    impl_updateRemoveButton();
    impl_updateInsertButton();

    rPanelDeck.AddListener( *this );
}

//-----------------------------------------------------------------------------
void OptionsWindow::StateChanged( StateChangedType i_nStateChange )
{
    Window::StateChanged( i_nStateChange );

    if ( i_nStateChange == STATE_CHANGE_INITSHOW )
    {
        impl_initPanelList();
    }
}

//-----------------------------------------------------------------------------
void OptionsWindow::GetFocus()
{
    Window::GetFocus();
    RadioButton* pRadios[] =
    {
        &m_aAlignLeft, &m_aAlignRight, &m_aAlignTop, &m_aAlignBottom
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
    const long nFixedLineHeight( LogicToPixel( Size( 0, 8 ), MAP_APPFONT ).Height() );
    const long nEditLineHeight( LogicToPixel( Size( 0, 12 ), MAP_APPFONT ).Height() );
    const long nButtonLineHeight( LogicToPixel( Size( 0, 14 ), MAP_APPFONT ).Height() );

    const long nSuperordinateWidth = aOutputSize.Width() - 2 * aSpacing.Width();
    const long nSuperordinateX = aSpacing.Width();

    const long nSubordinateWidth = aOutputSize.Width() - 2 * aSpacing.Width() - nIndent;
    const long nSubordinateX = aSpacing.Width() + nIndent;

    Point aItemPos( nSuperordinateX, aSpacing.Height() );

    struct ControlRow
    {
        Window* pWindow;
        bool    bSubordinate;
        size_t  nRows;

        ControlRow( Window& i_rWindow, const bool i_bSubordinate, const size_t i_nRows = 1 )
            :pWindow( &i_rWindow )
            ,bSubordinate( i_bSubordinate )
            ,nRows( i_nRows )
        {
        }
    };
    ControlRow aControlRows[] =
    {
        ControlRow( m_aAlignmentHeader,     false ),
        ControlRow( m_aAlignLeft,           true ),
        ControlRow( m_aAlignRight,          true ),
        ControlRow( m_aAlignTop,            true ),
        ControlRow( m_aAlignBottom,         true ),
        ControlRow( m_aTabItemContent,      false ),
        ControlRow( m_aImagesAndText,       true ),
        ControlRow( m_aImagesOnly,          true ),
        ControlRow( m_aTextOnly,            true ),
        ControlRow( m_aAutomaticContent,    true ),
        ControlRow( m_aPanelsHeader,        false ),
        ControlRow( m_aPanelList,           true, 6 ),
        ControlRow( m_aRemovePanel,         true ),
        ControlRow( m_aColors,              true ),
        ControlRow( m_aNewPanelName,        true ),
        ControlRow( m_aInsertPanel,         true )
    };
    bool bPreviousWasSubordinate = false;
    for ( size_t i=0; i < sizeof( aControlRows ) / sizeof( aControlRows[0] ); ++i )
    {
        aItemPos.X() = ( aControlRows[i].bSubordinate ) ? nSubordinateX : nSuperordinateX;

        if ( bPreviousWasSubordinate && !aControlRows[i].bSubordinate )
            aItemPos.Y() += aSpacing.Height();
        bPreviousWasSubordinate = aControlRows[i].bSubordinate;

        // height depends on the window type
        const WindowType eWindowType = aControlRows[i].pWindow->GetType();
        long nControlHeight( nFixedLineHeight );
        if  (   ( eWindowType == WINDOW_EDIT )
            ||  ( eWindowType == WINDOW_LISTBOX )
            )
        {
            nControlHeight = nEditLineHeight;
        }
        else
        if  (   ( eWindowType == WINDOW_PUSHBUTTON )
            )
        {
            nControlHeight = nButtonLineHeight;
        }

        Size aControlSize(
            aControlRows[i].bSubordinate ? nSubordinateWidth : nSuperordinateWidth,
            nControlHeight * aControlRows[i].nRows
        );
        aControlRows[i].pWindow->SetPosSizePixel( aItemPos, aControlSize );

        aItemPos.Move( 0, aControlSize.Height() + aSpacing.Height() );
    }
}

//-----------------------------------------------------------------------------
void OptionsWindow::PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition )
{
    m_aPanelList.InsertEntry( i_pPanel->GetDisplayName(), i_pPanel->GetImage(), sal_uInt16( i_nPosition ) );
}

//-----------------------------------------------------------------------------
void OptionsWindow::PanelRemoved( const size_t i_nPosition )
{
    m_aPanelList.RemoveEntry( sal_uInt16( i_nPosition ) );
    impl_updateRemoveButton();
}

//-----------------------------------------------------------------------------
void OptionsWindow::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
{
    (void)i_rOldActive;

    if ( !i_rNewActive )
        m_aPanelList.SetNoSelection();
    else
        m_aPanelList.SelectEntryPos( sal_uInt16( *i_rNewActive ) );
}

//-----------------------------------------------------------------------------
void OptionsWindow::LayouterChanged( const PDeckLayouter& i_rNewLayouter )
{
    (void)i_rNewLayouter;
    // not interested in
}

//-----------------------------------------------------------------------------
void OptionsWindow::Dying()
{
    // not interested in
}

//-----------------------------------------------------------------------------
IMPL_LINK( OptionsWindow, OnListEntrySelected, ListBox*, i_pListBox )
{
    if ( i_pListBox == &m_aColors )
    {
        m_aNewPanelName.SetText( m_aColors.GetEntry( m_aColors.GetSelectEntryPos()  ) );
        impl_updateInsertButton();
    }
    else if ( i_pListBox == &m_aPanelList )
    {
        impl_updateRemoveButton();
    }
    return 0L;
}

//-----------------------------------------------------------------------------
IMPL_LINK( OptionsWindow, OnListEntryDoubleClicked, ListBox*, i_pListBox )
{
    PanelDemoMainWindow& rController( dynamic_cast< PanelDemoMainWindow& >( *GetParent() ) );

    if ( i_pListBox == &m_aPanelList )
    {
        size_t nActivatePanel = size_t( m_aPanelList.GetSelectEntryPos() );
        rController.GetToolPanelDeck().ActivatePanel( nActivatePanel );
    }

    return 0L;
}

//-----------------------------------------------------------------------------
IMPL_LINK( OptionsWindow, OnEditModified, Edit*, i_pEdit )
{
    if ( i_pEdit && &m_aNewPanelName )
    {
        impl_updateInsertButton();
    }

    return 0L;
}

//-----------------------------------------------------------------------------
IMPL_LINK( OptionsWindow, OnButtonClicked, PushButton*, i_pPushButton )
{
    PanelDemoMainWindow& rController( dynamic_cast< PanelDemoMainWindow& >( *GetParent() ) );

    if ( i_pPushButton == &m_aRemovePanel )
    {
        rController.GetToolPanelDeck().RemovePanel( size_t( m_aPanelList.GetSelectEntryPos() ) );
    }
    else if ( i_pPushButton == &m_aInsertPanel )
    {
        PToolPanel pNewPanel( rController.CreateToolPanel( m_aColors.GetEntryColor( m_aColors.GetSelectEntryPos() ), m_aNewPanelName.GetText() ) );

        ::boost::optional< size_t > aActivePanel( rController.GetToolPanelDeck().GetActivePanel() );
        size_t nNewPanelPos = !aActivePanel ? rController.GetToolPanelDeck().GetPanelCount() : *aActivePanel + 1;

        rController.GetToolPanelDeck().InsertPanel( pNewPanel, nNewPanelPos );
    }
    return 0L;
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
        else if ( i_pRadioButton == &m_aAlignTop )
        {
            rController.AlignTabs( TABS_TOP );
        }
        else if ( i_pRadioButton == &m_aAlignBottom )
        {
            rController.AlignTabs( TABS_BOTTOM );
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
        else if ( i_pRadioButton == &m_aAutomaticContent )
        {
            rController.SetTabItemContent( TABITEM_AUTO );
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
    m_aToolPanelDeck.SetPosSizePixel( Point( 20, 20 ), Size( 500, 300 ) );
    m_aToolPanelDeck.SetBorderStyle( WINDOW_BORDER_MONO );

    m_aToolPanelDeck.InsertPanel( PToolPanel( new ColoredPanel( m_aToolPanelDeck.GetPanelWindowAnchor(), Color( COL_RED ), "Red" ) ), m_aToolPanelDeck.GetPanelCount() );
    m_aToolPanelDeck.InsertPanel( PToolPanel( new ColoredPanel( m_aToolPanelDeck.GetPanelWindowAnchor(), Color( COL_GREEN ), "Some flavor of Green" ) ), m_aToolPanelDeck.GetPanelCount() );
    m_aToolPanelDeck.InsertPanel( PToolPanel( new ColoredPanel( m_aToolPanelDeck.GetPanelWindowAnchor(), RGB_COLORDATA( 255, 255, 0 ), "Yellow is ugly" ) ), m_aToolPanelDeck.GetPanelCount() );
    m_aToolPanelDeck.InsertPanel( PToolPanel( new ColoredPanel( m_aToolPanelDeck.GetPanelWindowAnchor(), RGB_COLORDATA( 0, 0, 128 ), "Blue is the Color" ) ), m_aToolPanelDeck.GetPanelCount() );

    m_aToolPanelDeck.ActivatePanel( size_t( 0 ) );
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
    aSize.Width() -= 240;
    aSize.Height() -= 40;
    m_aToolPanelDeck.SetPosSizePixel( Point( 20, 20 ), aSize );

    m_aDemoOptions.SetPosSizePixel(
        Point( 20 + aSize.Width(), 20 ),
        Size( 200, aSize.Height() )
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

    m_aToolPanelDeck.SetLayouter( PDeckLayouter( new TabDeckLayouter( m_aToolPanelDeck, m_aToolPanelDeck, i_eAlignment, eCurrentItemContent ) ) );
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

//-----------------------------------------------------------------------------
IToolPanelDeck& PanelDemoMainWindow::GetToolPanelDeck()
{
    return m_aToolPanelDeck;
}

//-----------------------------------------------------------------------------
PToolPanel PanelDemoMainWindow::CreateToolPanel( const Color& i_rColor, const String& i_rPanelName )
{
    return PToolPanel( new ColoredPanel( m_aToolPanelDeck.GetPanelWindowAnchor(), i_rColor, i_rPanelName ) );
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
