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
            OSL_TRACE( "ERROR: failed to load image: `%s'", pName );
        }
    }
};

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

void Button::Click()
{
}

IMPL_GET_IMPL( Button );

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

void PushButton::SetToggleHdl( const Link& link )
{
    if (&getImpl () && getImpl().mxButton.is ())
        getImpl().SetToggleHdl( link );
}

IMPL_GET_IMPL( PushButton );

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

BUTTON_IMPL( CancelButton, PushButton, BUTTONID_CANCEL );
BUTTON_IMPL( YesButton, PushButton, BUTTONID_YES );
BUTTON_IMPL( NoButton, PushButton, BUTTONID_NO );
BUTTON_IMPL( RetryButton, PushButton, BUTTONID_RETRY );
BUTTON_IMPL( IgnoreButton, PushButton, BUTTONID_IGNORE );
BUTTON_IMPL( HelpButton, PushButton, BUTTONID_HELP );

CancelButton::CancelButton ( Context *context, char const* pId, sal_uInt32 nId )
    : PushButton( new CancelButtonImpl( context, context->GetPeerHandle( pId, nId ), this ) )
{
    Window *parent = dynamic_cast<Window*> (context);
    if (parent)
        SetParent (parent);
}

YesButton::YesButton ( Context *context, char const* pId, sal_uInt32 nId )
    : PushButton( new YesButtonImpl( context, context->GetPeerHandle( pId, nId ), this ) )
{
    Window *parent = dynamic_cast<Window*> (context);
    if (parent)
        SetParent (parent);
}

NoButton::NoButton ( Context *context, char const* pId, sal_uInt32 nId )
    : PushButton( new NoButtonImpl( context, context->GetPeerHandle( pId, nId ), this ) )
{
    Window *parent = dynamic_cast<Window*> (context);
    if (parent)
        SetParent (parent);
}

RetryButton::RetryButton ( Context *context, char const* pId, sal_uInt32 nId )
    : PushButton( new RetryButtonImpl( context, context->GetPeerHandle( pId, nId ), this ) )
{
    Window *parent = dynamic_cast<Window*> (context);
    if (parent)
        SetParent (parent);
}

IgnoreButton::IgnoreButton ( Context *context, char const* pId, sal_uInt32 nId )
    : PushButton( new IgnoreButtonImpl( context, context->GetPeerHandle( pId, nId ), this ) )
{
    Window *parent = dynamic_cast<Window*> (context);
    if (parent)
        SetParent (parent);
}

HelpButton::HelpButton ( Context *context, char const* pId, sal_uInt32 nId )
    : PushButton( new HelpButtonImpl( context, context->GetPeerHandle( pId, nId ), this ) )
{
    Window *parent = dynamic_cast<Window*> (context);
    if (parent)
        SetParent (parent);
}

} // namespace layout

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
