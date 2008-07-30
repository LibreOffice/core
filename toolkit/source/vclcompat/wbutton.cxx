/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wbutton.cxx,v $
 *
 * $Revision: 1.3 $
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

#include "wrapper.hxx"

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <cppuhelper/implbase1.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/button.hxx>
#include <vcl/event.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <list>

#include "layout/layoutcore.hxx"

using namespace ::com::sun::star;

namespace layout
{

class ImageImpl
{
  public:
    css::uno::Reference< css::graphic::XGraphic > mxGraphic;
    ImageImpl( const char *pName )
        : mxGraphic( layoutimpl::loadGraphic( pName ) )
    {
        if ( !mxGraphic.is() )
        {
            DBG_ERROR1( "ERROR: failed to load image: `%s'\n", pName );
        }
    }
};

Image::Image( const char *pName )
    : pImpl( new ImageImpl( pName ) )
{
}

Image::~Image()
{
    delete pImpl;
}

class ButtonImpl : public ControlImpl
                 , public ::cppu::WeakImplHelper1< awt::XActionListener >
                 , public ::cppu::WeakImplHelper1< awt::XItemListener >
{
    Link maClickHdl;
protected:
    // we add toggle hooks here to cut on code
    Link maToggleHdl;
public:
    uno::Reference< awt::XButton > mxButton;
    ButtonImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ControlImpl( pCtx, xPeer, pWindow )
        , mxButton( xPeer, uno::UNO_QUERY )
    {
        mxButton->addActionListener( this );
    }

    ~ButtonImpl()
    {
    }

    virtual void Click() { /* make me pure virtual? */ };

    void SetClickHdl( const Link& rLink )
    {
        maClickHdl = rLink;
        if ( !rLink )
            mxButton->removeActionListener( this );
        else
            mxButton->addActionListener( this );
    }

    virtual void SAL_CALL disposing( const css::lang::EventObject& /* Source */ )
        throw (css::uno::RuntimeException)
    { mxButton.clear(); }

    virtual void SAL_CALL actionPerformed( const css::awt::ActionEvent& /* rEvent */ )
        throw (css::uno::RuntimeException)
    {
        if ( !maClickHdl )
            Click();
        else
            maClickHdl.Call( static_cast<Window *>( mpWindow ) );
    }

    virtual  // HACK: doesn't need to be virtual... remove in future...
    void SAL_CALL itemStateChanged( const css::awt::ItemEvent& /* rEvent */ )
        throw (css::uno::RuntimeException)
    {
        maToggleHdl.Call( static_cast<Window *>( mpWindow ) );
    }

    BOOL SetModeImage( css::uno::Reference< css::graphic::XGraphic > xGraph )
    {
        setProperty( "Graphic", uno::Any( xGraph ) );
        return true;
    }
};

void Button::SetText( const String& rStr )
{
    if ( !getImpl().mxButton.is() )
        return;
    getImpl().mxButton->setLabel( rStr );
}

void Button::SetClickHdl( const Link& rLink )
{
    getImpl().SetClickHdl( rLink );
}

BOOL Button::SetModeImage( const Image& rImage )
{
    return getImpl().SetModeImage( rImage.getImpl().mxGraphic );
}

void Button::SetImageAlign( ImageAlign eAlign )
{
    getImpl().setProperty( "ImageAlign", uno::Any( (sal_Int16) eAlign ) );
}

void Button::Click()
{
}

DECL_GET_IMPL_IMPL( Button );
DECL_CONSTRUCTOR_IMPLS( Button, Control, "button" );

class PushButtonImpl : public ButtonImpl
{
  public:
    PushButtonImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ButtonImpl( pCtx, xPeer, pWindow ) {}

    void SetToggleHdl( const Link& rLink )
    {
        // XButton doesn't have an explicit event for Toggle. Anyway, it is a
        // superset of the clicks: all clicks, and explicit toggles
        maToggleHdl = rLink;
        if ( !rLink )
            mxButton->removeActionListener( this );
        else
            mxButton->addActionListener( this );
    }

    virtual void SAL_CALL actionPerformed( const css::awt::ActionEvent& rEvent )
        throw (css::uno::RuntimeException)
    {
        ButtonImpl::actionPerformed( rEvent );
        fireToggle();
    }

    inline void fireToggle()
    {
        maToggleHdl.Call(  static_cast<Window *>( mpWindow ) );
    }
};

void PushButton::Check( BOOL bCheck )
{
    getImpl().setProperty( "State", uno::Any( (sal_Int16) !!bCheck ) );
    // XButton doesn't have explicit toggle event
    getImpl().fireToggle();
}

BOOL PushButton::IsChecked() const
{
    return !!( getImpl().getProperty( "State" ).get< sal_Int16 >() );
}

void PushButton::Toggle()
{
    Check( true );
}

void PushButton::SetToggleHdl( const Link& rLink )
{
    getImpl().SetToggleHdl( rLink );
}

DECL_GET_IMPL_IMPL( PushButton );
DECL_CONSTRUCTOR_IMPLS( PushButton, Button, "pushbutton" );

 // HACK: put every radio button into a group :/
static std::list< RadioButtonImpl*> mpRadioGroup;

class RadioButtonImpl : public ButtonImpl
{
public:
    uno::Reference< awt::XRadioButton > mxRadioButton;
    RadioButtonImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ButtonImpl( pCtx, xPeer, pWindow )
        , mxRadioButton( xPeer, uno::UNO_QUERY )
    {
        // TEMP:
        mpRadioGroup.push_back( this );
        mxRadioButton->addItemListener( this );
    }

    ~RadioButtonImpl()
    {
        //mxRadioButton->removeItemListener( this );
        mpRadioGroup.pop_front();
    }

    void Check( BOOL bCheck )
    {
        if ( !mxRadioButton.is() )
            return;
        mxRadioButton->setState( !!bCheck );
        fireToggle();

        if ( bCheck )
            unsetOthersGroup( this );
    }

    BOOL IsChecked()
    {
        if ( !mxRadioButton.is() )
            return FALSE;
        return mxRadioButton->getState();
    }

    static void unsetOthersGroup( RadioButtonImpl* current )
    {
        // set all others to false
        for ( std::list< RadioButtonImpl*>::iterator i = mpRadioGroup.begin();
              i != mpRadioGroup.end(); i++ )
        {
            if ( *i != current )
                ( *i )->Check( false );
        }
    }

    void SetToggleHdl( const Link& rLink )
    {
        maToggleHdl = rLink;
#if 0
        if ( !rLink )
            mxRadioButton->removeItemListener( this );
        else
            mxRadioButton->addItemListener( this );
#endif
    }

    inline void fireToggle()
    {
        maToggleHdl.Call(  static_cast<Window *>( mpWindow ) );
    }

//HACK: temp
    virtual void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent )
        throw (css::uno::RuntimeException)
    {
        if ( !!maToggleHdl )
        ButtonImpl::itemStateChanged( rEvent );

        if ( IsChecked() )
            unsetOthersGroup( this );
    }
};

void RadioButton::Check( BOOL bCheck )
{
    getImpl().Check( bCheck );
}

BOOL RadioButton::IsChecked() const
{
    return getImpl().IsChecked();
}

void RadioButton::SetToggleHdl( const Link& rLink )
{
    getImpl().SetToggleHdl( rLink );
}

DECL_GET_IMPL_IMPL( RadioButton );
#if 1
DECL_CONSTRUCTOR_IMPLS( RadioButton, Button, "radiobutton" );
#else //debugging aid
RadioButton::RadioButton( Context *pCtx, const char *pId, sal_uInt32 nId )
    : Button( new RadioButtonImpl( pCtx, pCtx->GetPeerHandle( pId, nId ), this ) )
{
    printf( "%s: name=%s\n", __PRETTY_FUNCTION__, pId );
}

RadioButton::RadioButton( Window *pParent, WinBits nBits )
    : Button( new RadioButtonImpl( pParent->getContext(), Window::CreatePeer( pParent, nBits, "radiobutton" ), this ) )
{
}
#endif

class CheckBoxImpl : public ButtonImpl
{
  public:
    uno::Reference< awt::XCheckBox > mxCheckBox;
    CheckBoxImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ButtonImpl( pCtx, xPeer, pWindow )
        , mxCheckBox( xPeer, uno::UNO_QUERY ) {}

    void SetToggleHdl( const Link& rLink )
    {
        maToggleHdl = rLink;
        if ( !rLink )
            mxCheckBox->removeItemListener( this );
        else
            mxCheckBox->addItemListener( this );
    }
};

void CheckBox::Check( BOOL bCheck )
{
    if ( !getImpl().mxCheckBox.is() )
        return;
    getImpl().mxCheckBox->setState( !!bCheck );
}

BOOL CheckBox::IsChecked() const
{
    if ( !getImpl().mxCheckBox.is() )
        return FALSE;
    return getImpl().mxCheckBox->getState() != 0;
}

void CheckBox::SetToggleHdl( const Link& rLink )
{
    getImpl().SetToggleHdl( rLink );
}

DECL_GET_IMPL_IMPL( CheckBox );
DECL_CONSTRUCTOR_IMPLS( CheckBox, Button, "checkbox" );

#define BUTTON_IMPL(t, parent, response) \
    class t##Impl : public parent##Impl \
    { \
    public: \
        t##Impl( Context *pCtx, PeerHandle const& xPeer, Window *pWindow ) \
            : parent##Impl( pCtx, xPeer, pWindow ) \
        { \
        } \
        void Click() \
        { \
            Dialog *d = static_cast<Dialog *>( mpCtx ); \
            d->EndDialog( response ); \
        } \
    }

/* Common button types currently unavailable in OOo: */
/* mpReset */
/* mpApply */
/* mpAction */
#define RET_RESET 6
#define RET_APPLY 7
#define BUTTONID_RESET RET_RESET
#define BUTTONID_APPLY RET_APPLY

BUTTON_IMPL( OKButton, PushButton, BUTTONID_OK );
BUTTON_IMPL( CancelButton, PushButton, BUTTONID_CANCEL );
BUTTON_IMPL( YesButton, PushButton, BUTTONID_YES );
BUTTON_IMPL( NoButton, PushButton, BUTTONID_NO );
BUTTON_IMPL( RetryButton, PushButton, BUTTONID_RETRY );
BUTTON_IMPL( IgnoreButton, PushButton, BUTTONID_IGNORE );
BUTTON_IMPL( ResetButton, PushButton, BUTTONID_RESET );
BUTTON_IMPL( ApplyButton, PushButton, BUTTONID_APPLY ); /* Deprecated? */
BUTTON_IMPL( HelpButton, PushButton, BUTTONID_HELP );

DECL_CONSTRUCTOR_IMPLS( OKButton, PushButton, "okbutton" );
DECL_CONSTRUCTOR_IMPLS( CancelButton, PushButton, "cancelbutton" );
DECL_CONSTRUCTOR_IMPLS( YesButton, PushButton, "yesbutton" );
DECL_CONSTRUCTOR_IMPLS( NoButton, PushButton, "nobutton" );
DECL_CONSTRUCTOR_IMPLS( RetryButton, PushButton, "retrybutton" );
DECL_CONSTRUCTOR_IMPLS( IgnoreButton, PushButton, "ignorebutton" );
DECL_CONSTRUCTOR_IMPLS( ResetButton, PushButton, "resetbutton" );
DECL_CONSTRUCTOR_IMPLS( ApplyButton, PushButton, "applybutton" );  /* Deprecated? */
DECL_CONSTRUCTOR_IMPLS( HelpButton, PushButton, "helpbutton" );

class AdvancedButtonImpl : public PushButtonImpl
{
protected:
    bool bAdvancedMode;
    std::list< Window*> maAdvanced;
    std::list< Window*> maSimple;
    rtl::OUString mSimpleLabel;
    rtl::OUString mAdvancedLabel;

    Window* Remove( std::list< Window*> lst, Window* w )
    {
        for ( std::list< Window*>::iterator it = maAdvanced.begin();
              it != maAdvanced.end(); it++ )
            if ( *it == w )
            {
                lst.erase( it );
                return *it;
            }
        return 0;
    }
    void redraw()
    {
        uno::Reference <awt::XWindow> ref( mxWindow, uno::UNO_QUERY );
        ::Window* window = VCLXWindow::GetImplementation( ref )->GetWindow();
        ::Window* parent = window->GetParent();

        ::Rectangle r = Rectangle( parent->GetPosPixel(),
                                   parent->GetSizePixel() );

        parent->Invalidate( r, INVALIDATE_CHILDREN | INVALIDATE_NOCHILDREN );
        parent->SetPosSizePixel( 0, 0, r.nRight - r.nLeft, r.nBottom - r.nTop,
                                 awt::PosSize::SIZE );
    }

public:
    AdvancedButtonImpl( Context *pCtx, PeerHandle const& xPeer, Window *pWindow )
        : PushButtonImpl( pCtx, xPeer, pWindow )
        , bAdvancedMode( false )
          // TODO: i18n
          // Button::GetStandardText( BUTTON_ADVANCED );
          // Button::GetStandardText( BUTTON_SIMPLE );
        , mSimpleLabel( rtl::OUString::createFromAscii( "Advanced..." ) )
        , mAdvancedLabel( rtl::OUString::createFromAscii( "Simple..." ) )
    {
    }
    void Click()
    {
        bAdvancedMode = !bAdvancedMode;
        if ( bAdvancedMode )
            advancedMode();
        else
            simpleMode();
    }
    void advancedMode()
    {
        // TODO: set symbol/image?
        // SYMBOL_PAGEUP, SYMBOL_PAGEDOWN
#if 0
        // How to set images from here?
        // XImageConsumer looks like a disaster
        // Should move all this switching functionality to VCLXAdvancedButton?
        /biek/home/janneke/vc/layout-cws/default_images/res/commandimagelist/
            sc_arrowshapes_down.png
            sch_flowchartshapes.flowchart-extract.png
            sch_flowchartshapes.flowchart-merge.png
#endif
        mxButton->setLabel( mAdvancedLabel );
        for ( std::list< Window*>::iterator it = maAdvanced.begin();
              it != maAdvanced.end(); it++ )
            ( *it )->Show();
        for ( std::list< Window*>::iterator it = maSimple.begin();
              it != maSimple.end(); it++ )
            ( *it )->Hide();

        redraw();
    }

    void simpleMode()
    {
        mxButton->setLabel( mSimpleLabel );
        for ( std::list< Window*>::iterator it = maAdvanced.begin();
              it != maAdvanced.end(); it++ )
            ( *it )->Hide();
        for ( std::list< Window*>::iterator it = maSimple.begin();
              it != maSimple.end(); it++ )
            ( *it )->Show();

        redraw();
    }
    void AddAdvanced( Window* w )
    {
        maAdvanced.push_back( w );
        if ( !bAdvancedMode )
            w->Hide();
    }
    void AddSimple( Window* w )
    {
        maSimple.push_back( w );
        if ( bAdvancedMode )
            w->Hide();
    }
    void RemoveAdvanced( Window* w )
    {
        Remove( maAdvanced, w );
    }
    void RemoveSimple( Window* w )
    {
        Remove( maSimple, w );
    }
};

void AdvancedButton::AddAdvanced( Window* w )
{
    getImpl().AddAdvanced( w );
}

void AdvancedButton::AddSimple( Window* w )
{
    getImpl().AddSimple( w );
}

void AdvancedButton::RemoveAdvanced( Window* w )
{
    getImpl().RemoveAdvanced( w );
}

void AdvancedButton::RemoveSimple( Window* w )
{
    getImpl().RemoveSimple( w );
}

DECL_CONSTRUCTOR_IMPLS( AdvancedButton, PushButton, "advancedbutton" );
DECL_GET_IMPL_IMPL( AdvancedButton );


class MoreButtonImpl : public AdvancedButtonImpl
{
public:
    MoreButtonImpl( Context *pCtx, PeerHandle const& xPeer, Window *pWindow )
        : AdvancedButtonImpl( pCtx, xPeer, pWindow)
    {
        mAdvancedLabel = ::Button::GetStandardText( BUTTON_LESS );
        mSimpleLabel = ::Button::GetStandardText( BUTTON_MORE );
    }
    void AddWindow( Window* w ) { AddAdvanced( w ); }
    void RemoveWindow( Window* w ) { RemoveAdvanced( w ); }
};

// TODO
//BUTTON_IMPL( MoreButton, PushButton, 0 );
DECL_CONSTRUCTOR_IMPLS( MoreButton, AdvancedButton, "morebutton" );
DECL_GET_IMPL_IMPL( MoreButton );

void MoreButton::AddWindow( Window* w )
{
    getImpl().AddWindow( w );
}

void MoreButton::RemoveWindow( Window* w )
{
    getImpl().RemoveWindow( w );
}

} // namespace layout
