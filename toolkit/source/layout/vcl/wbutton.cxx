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

#include "wrapper.hxx"

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <cppuhelper/implbase1.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/button.hxx>
#include <vcl/event.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <list>

#include <layout/core/helper.hxx>

using namespace ::com::sun::star;

using rtl::OUString;

namespace layout
{

class ImageImpl
{
  public:
    uno::Reference< graphic::XGraphic > mxGraphic;
    ImageImpl( const char *pName )
        : mxGraphic( layoutimpl::loadGraphic( pName ) )
    {
        if ( !mxGraphic.is() )
        {
            OSL_TRACE( "ERROR: failed to load image: `%s'\n", pName );
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
{
    Link maClickHdl;

public:
    uno::Reference< awt::XButton > mxButton;
    ButtonImpl( Context *context, const PeerHandle &peer, Window *window )
        : ControlImpl( context, peer, window )
        , mxButton( peer, uno::UNO_QUERY )
    {
        /* We have default action when clicked, always listen. */
        mxButton->addActionListener( this );
    }

    ~ButtonImpl()
    {
    }

    virtual void Click() { /* make me pure virtual? */ };

    Link& GetClickHdl ()
    {
        return maClickHdl;
    }

    virtual void SetClickHdl( Link const& link )
    {
        maClickHdl = link;
    }

    void SAL_CALL disposing( lang::EventObject const& e )
        throw (uno::RuntimeException)
    {
        mxButton->removeActionListener( this );
        ControlImpl::disposing (e);
        mxButton.clear ();
    }

    virtual void SAL_CALL actionPerformed( const awt::ActionEvent& )
        throw (uno::RuntimeException)
    {
        if ( !maClickHdl )
            Click();
        else
            maClickHdl.Call( static_cast<Window *>( mpWindow ) );
    }

    bool SetModeImage( uno::Reference< graphic::XGraphic > xGraph )
    {
        setProperty( "Graphic", uno::Any( xGraph ) );
        return true;
    }
};

Button::~Button ()
{
}

String Button::GetStandardText (sal_uInt16 button_type)
{
    return ::Button::GetStandardText (button_type);
}

void Button::SetText( OUString const& rStr )
{
    if ( !getImpl().mxButton.is() )
        return;
    getImpl().mxButton->setLabel( rStr );
}

void Button::SetClickHdl( const Link& link )
{
    if (&getImpl () && getImpl().mxButton.is ())
        getImpl().SetClickHdl( link );
}

Link& Button::GetClickHdl ()
{
    return getImpl().GetClickHdl ();
}

bool Button::SetModeImage (Image const& image)
{
    return getImpl().SetModeImage (image.getImpl().mxGraphic);
}

bool Button::SetModeImage (::Image const& image)
{
    return GetButton ()->SetModeImage (image);
}

void Button::SetImageAlign( ImageAlign eAlign )
{
    getImpl().setProperty( "ImageAlign", uno::Any( (sal_Int16) eAlign ) );
}

void Button::Click()
{
}

IMPL_GET_IMPL( Button );
IMPL_CONSTRUCTORS( Button, Control, "button" );
IMPL_GET_WINDOW (Button);

class PushButtonImpl : public ButtonImpl
                 , public ::cppu::WeakImplHelper1< awt::XItemListener >
{
    Link maToggleHdl;
public:
    PushButtonImpl( Context *context, const PeerHandle &peer, Window *window )
        : ButtonImpl( context, peer, window )
    {
    }

    void SetToggleHdl( const Link& link )
    {
        // XButton doesn't have an explicit event for Toggle. Anyway, it is a
        // superset of the clicks: all clicks, and explicit toggles
        if (!link && !!maToggleHdl)
            mxButton->removeActionListener( this );
        else if (!!link && !maToggleHdl)
            mxButton->addActionListener( this );
        maToggleHdl = link;
    }
    void SAL_CALL disposing( lang::EventObject const& e )
        throw (uno::RuntimeException)
    {
        ButtonImpl::disposing (e);
    }
    virtual void SAL_CALL actionPerformed( awt::ActionEvent const& e )
        throw (uno::RuntimeException)
    {
        ButtonImpl::actionPerformed( e );
        fireToggle();
    }
    virtual void SAL_CALL itemStateChanged( const awt::ItemEvent& )
        throw (uno::RuntimeException)
    {
        maToggleHdl.Call( static_cast<Window *>( mpWindow ) );
    }
    void fireToggle()
    {
        maToggleHdl.Call(  static_cast<Window *>( mpWindow ) );
    }

};

PushButton::~PushButton ()
{
    SetToggleHdl (Link ());
}

void PushButton::Check( bool bCheck )
{
    getImpl().setProperty( "State", uno::Any( (sal_Int16) !!bCheck ) );
    // XButton doesn't have explicit toggle event
    getImpl().fireToggle();
}

bool PushButton::IsChecked() const
{
    return !!( getImpl().getProperty( "State" ).get< sal_Int16 >() );
}

void PushButton::Toggle()
{
    Check( true );
}

void PushButton::SetToggleHdl( const Link& link )
{
    if (&getImpl () && getImpl().mxButton.is ())
        getImpl().SetToggleHdl( link );
}

IMPL_GET_IMPL( PushButton );
IMPL_CONSTRUCTORS( PushButton, Button, "pushbutton" );
IMPL_GET_WINDOW (PushButton);

class RadioButtonImpl : public ButtonImpl
                      , public ::cppu::WeakImplHelper1< awt::XItemListener >
{
    Link maToggleHdl;
public:
    uno::Reference< awt::XRadioButton > mxRadioButton;
    RadioButtonImpl( Context *context, const PeerHandle &peer, Window *window )
        : ButtonImpl( context, peer, window )
        , mxRadioButton( peer, uno::UNO_QUERY )
    {
    }

    void Check( bool bCheck )
    {
        if ( !mxRadioButton.is() )
            return;

#if 1
        // Have setState fire item event for
        // RadioGroups::RadioGroup::itemStateChanged ()
        ::RadioButton *r = static_cast<RadioButton*>(mpWindow)->GetRadioButton ();
        bool state = r->IsRadioCheckEnabled ();
        r->EnableRadioCheck();
        mxRadioButton->setState( !!bCheck );
        r->EnableRadioCheck (state);
#else
        mxRadioButton->setState( !!bCheck );
#endif
        fireToggle();
    }

    bool IsChecked()
    {
        if ( !mxRadioButton.is() )
            return false;
        return mxRadioButton->getState();
    }

    void SetToggleHdl( const Link& link )
    {
        if (!link && !!maToggleHdl)
            mxRadioButton->removeItemListener( this );
        else if (!!link && !maToggleHdl)
            mxRadioButton->addItemListener( this );
        maToggleHdl = link;
    }

    inline void fireToggle()
    {
        maToggleHdl.Call(  static_cast<Window *>( mpWindow ) );
    }

    virtual void SetClickHdl( const Link& link )
    {
        // Keep RadioGroups::RadioGroup's actionListener at HEAD
        // of list.  This way, it can handle RadioGroup's button
        // states before all other callbacks and make sure the
        // client code has the right state.

        // IWBN to add an XRadioButton2 and layout::VCLXRadioButton
        // with {get,set}RadioGroup() (and a "radiogroup" property
        // even) and handle the grouping here in RadioButtonImpl.
        uno::Reference< uno::XInterface > x = static_cast<VCLXRadioButton*> (mpWindow->GetVCLXWindow ())->getFirstActionListener ();
        uno::Reference< awt::XActionListener > a = uno::Reference< awt::XActionListener> (x ,uno::UNO_QUERY );
        mxButton->removeActionListener (a);
        ButtonImpl::SetClickHdl (link);
        mxButton->addActionListener (a);
    }

    void SAL_CALL disposing( lang::EventObject const& e )
        throw (uno::RuntimeException)
    {
        ButtonImpl::disposing (e);
    }

    virtual void SAL_CALL itemStateChanged( const awt::ItemEvent& )
        throw (uno::RuntimeException)
    {
        maToggleHdl.Call( static_cast<Window *>( mpWindow ) );
    }
};

RadioButton::~RadioButton ()
{
    SetToggleHdl (Link ());
}

void RadioButton::Check( bool bCheck )
{
    getImpl().Check( bCheck );
}

bool RadioButton::IsChecked() const
{
    return getImpl().IsChecked();
}

void RadioButton::SetToggleHdl( const Link& link )
{
    if (&getImpl () && getImpl().mxRadioButton.is ())
        getImpl().SetToggleHdl( link );
}

IMPL_GET_IMPL( RadioButton );
IMPL_GET_WINDOW( RadioButton );
IMPL_GET_VCLXWINDOW( RadioButton );
IMPL_CONSTRUCTORS( RadioButton, Button, "radiobutton" );

class CheckBoxImpl : public ButtonImpl
                 , public ::cppu::WeakImplHelper1< awt::XItemListener >
{
    Link maToggleHdl;
  public:
    uno::Reference< awt::XCheckBox > mxCheckBox;
    CheckBoxImpl( Context *context, const PeerHandle &peer, Window *window )
        : ButtonImpl( context, peer, window )
        , mxCheckBox( peer, uno::UNO_QUERY )
    {
    }

    void SetToggleHdl( const Link& link )
    {
        if (!link && !!maToggleHdl)
            mxCheckBox->removeItemListener( this );
        else if (!!link && !maToggleHdl)
            mxCheckBox->addItemListener( this );
        maToggleHdl = link;
    }
    void SAL_CALL disposing( lang::EventObject const& e )
        throw (uno::RuntimeException)
    {
        ButtonImpl::disposing (e);
    }
    virtual void SAL_CALL itemStateChanged( const awt::ItemEvent& )
        throw (uno::RuntimeException)
    {
        maToggleHdl.Call( static_cast<Window *>( mpWindow ) );
    }
};

CheckBox::~CheckBox ()
{
    SetToggleHdl (Link ());
}

void CheckBox::Check( bool bCheck )
{
    if ( !getImpl().mxCheckBox.is() )
        return;
    getImpl().mxCheckBox->setState( !!bCheck );
}

bool CheckBox::IsChecked() const
{
    if ( !getImpl().mxCheckBox.is() )
        return false;
    return getImpl().mxCheckBox->getState() != 0;
}

void CheckBox::SetToggleHdl( const Link& link )
{
    if (&getImpl () && getImpl().mxCheckBox.is ())
        getImpl().SetToggleHdl( link );
}

IMPL_GET_IMPL( CheckBox );
IMPL_CONSTRUCTORS( CheckBox, Button, "checkbox" );

#define BUTTON_IMPL(t, parent, response) \
    class t##Impl : public parent##Impl \
    { \
    public: \
        t##Impl( Context *context, PeerHandle const& peer, Window *window ) \
            : parent##Impl( context, peer, window ) \
        { \
        } \
        void Click() \
        { \
            if (Dialog *d = static_cast<Dialog *> (mpCtx)) \
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

IMPL_CONSTRUCTORS( OKButton, PushButton, "okbutton" );
IMPL_CONSTRUCTORS( CancelButton, PushButton, "cancelbutton" );
IMPL_CONSTRUCTORS( YesButton, PushButton, "yesbutton" );
IMPL_CONSTRUCTORS( NoButton, PushButton, "nobutton" );
IMPL_CONSTRUCTORS( RetryButton, PushButton, "retrybutton" );
IMPL_CONSTRUCTORS( IgnoreButton, PushButton, "ignorebutton" );
IMPL_CONSTRUCTORS( ResetButton, PushButton, "resetbutton" );
IMPL_CONSTRUCTORS( ApplyButton, PushButton, "applybutton" );  /* Deprecated? */
IMPL_CONSTRUCTORS( HelpButton, PushButton, "helpbutton" );

IMPL_IMPL (ImageButton, PushButton)


IMPL_CONSTRUCTORS( ImageButton, PushButton, "imagebutton" );

class AdvancedButtonImpl : public PushButtonImpl
{
protected:
    bool bAdvancedMode;
    std::list< Window*> maAdvanced;
    std::list< Window*> maSimple;

public:
    rtl::OUString mAdvancedLabel;
    rtl::OUString mSimpleLabel;

protected:
    Window* Remove( std::list< Window*> lst, Window* w )
    {
        for ( std::list< Window*>::iterator it = maAdvanced.begin();
              it != maAdvanced.end(); ++it )
            if ( *it == w )
            {
                lst.erase( it );
                return *it;
            }
        return 0;
    }

public:
    AdvancedButtonImpl( Context *context, PeerHandle const& peer, Window *window )
        : PushButtonImpl( context, peer, window )
        , bAdvancedMode( false )
          // TODO: i18n
          // Button::GetStandardText( BUTTON_ADVANCED );
          // Button::GetStandardText( BUTTON_SIMPLE );
        , mAdvancedLabel( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Advanced...")) )
        , mSimpleLabel( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Simple...")) )
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
    void setAlign ()
    {
        ::PushButton *b = static_cast<PushButton*> (mpWindow)->GetPushButton ();
        b->SetSymbolAlign (SYMBOLALIGN_RIGHT);
        b->SetSmallSymbol ();
        //mpWindow->SetStyle (mpWindow->GetStyle() | WB_CENTER);
    }
    void advancedMode()
    {
        ::PushButton *b = static_cast<PushButton*> (mpWindow)->GetPushButton ();
        b->SetSymbol (SYMBOL_PAGEUP);
        setAlign ();
        if (mSimpleLabel.getLength ())
            b->SetText (mSimpleLabel);
        for ( std::list< Window*>::iterator it = maAdvanced.begin();
              it != maAdvanced.end(); ++it )
            ( *it )->Show();
        for ( std::list< Window*>::iterator it = maSimple.begin();
              it != maSimple.end(); ++it )
            ( *it )->Hide();

        redraw ();
    }
    void simpleMode()
    {
        //mxButton->setLabel( mSimpleLabel );
        ::PushButton *b = static_cast<PushButton*> (mpWindow)->GetPushButton ();
        b->SetSymbol (SYMBOL_PAGEDOWN);
        if (mAdvancedLabel.getLength ())
            b->SetText (mAdvancedLabel);
        setAlign ();
        for ( std::list< Window*>::iterator it = maAdvanced.begin();
              it != maAdvanced.end(); ++it )
            ( *it )->Hide();
        for ( std::list< Window*>::iterator it = maSimple.begin();
              it != maSimple.end(); ++it )
            ( *it )->Show();

        redraw (true);
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

void AdvancedButton::SetAdvancedText (rtl::OUString const& text)
{
    if (text.getLength ())
        getImpl ().mAdvancedLabel = text;
}

void AdvancedButton::SetSimpleText (rtl::OUString const& text)
{
    if (text.getLength ())
        getImpl ().mSimpleLabel = text;
}

rtl::OUString AdvancedButton::GetAdvancedText () const
{
    return getImpl ().mAdvancedLabel;
}

rtl::OUString AdvancedButton::GetSimpleText () const
{
    return getImpl ().mSimpleLabel;
}

void AdvancedButton::SetDelta (int)
{
}

IMPL_CONSTRUCTORS_BODY( AdvancedButton, PushButton, "advancedbutton", getImpl().simpleMode () );
IMPL_GET_IMPL( AdvancedButton );


class MoreButtonImpl : public AdvancedButtonImpl
{
public:
    MoreButtonImpl( Context *context, PeerHandle const& peer, Window *window )
        : AdvancedButtonImpl( context, peer, window)
    {
        mSimpleLabel = Button::GetStandardText( BUTTON_MORE );
        mAdvancedLabel = Button::GetStandardText( BUTTON_LESS );
    }
    void AddWindow( Window* w ) { AddAdvanced( w ); }
    void RemoveWindow( Window* w ) { RemoveAdvanced( w ); }
};

// TODO
//BUTTON_IMPL( MoreButton, PushButton, 0 );
IMPL_CONSTRUCTORS_BODY( MoreButton, AdvancedButton, "morebutton", getImpl().simpleMode () );
IMPL_GET_IMPL( MoreButton );

void MoreButton::AddWindow( Window* w )
{
    getImpl().AddWindow( w );
}

void MoreButton::RemoveWindow( Window* w )
{
    getImpl().RemoveWindow( w );
}

void MoreButton::SetMoreText (rtl::OUString const& text)
{
    SetAdvancedText (text);
}

void MoreButton::SetLessText (rtl::OUString const& text)
{
    SetSimpleText (text);
}

rtl::OUString MoreButton::GetMoreText () const
{
    return GetAdvancedText ();
}

rtl::OUString MoreButton::GetLessText () const
{
    return GetSimpleText ();
}

} // namespace layout

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
