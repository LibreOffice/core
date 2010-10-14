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

#ifndef LAYOUT_VCL_WRAPPER_HXX
#define LAYOUT_VCL_WRAPPER_HXX

#include <layout/layout.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <cppuhelper/implbase1.hxx>

#include <cstring>

namespace layout
{

namespace css = com::sun::star;

class WindowImpl
{
public:
    Window *mpWindow;
    Context *mpCtx;
    css::uno::Reference< css::awt::XWindow > mxWindow;
    css::uno::Reference< css::awt::XVclWindowPeer > mxVclPeer;
    ::Window *mvclWindow;
    bool bFirstTimeVisible;

    WindowImpl (Context *context, PeerHandle const &peer, Window *window);
    virtual ~WindowImpl ();

    void wrapperGone();
    css::uno::Any getProperty (char const *name);
    void setProperty (char const *name, css::uno::Any any);
    void redraw (bool resize=false);

    // XFocusListener
    virtual void SAL_CALL disposing (css::lang::EventObject const&) throw (css::uno::RuntimeException);
};

class ControlImpl : public WindowImpl
                  , public ::cppu::WeakImplHelper1 <css::awt::XFocusListener>
{
public:
    Link mGetFocusHdl;
    Link mLoseFocusHdl;

    ControlImpl( Context *context, PeerHandle const& peer, Window *window );
    ~ControlImpl ();

    virtual void SetGetFocusHdl (Link const& link);
    Link& GetGetFocusHdl ();
    virtual void SetLoseFocusHdl (Link const& link);
    Link& GetLoseFocusHdl ();
    virtual void UpdateListening (Link const& link);

    // XFocusListener
    virtual void SAL_CALL disposing (css::lang::EventObject const&) throw (css::uno::RuntimeException);
    void SAL_CALL focusGained (css::awt::FocusEvent const& e) throw (css::uno::RuntimeException);
    void SAL_CALL focusLost (css::awt::FocusEvent const& e) throw (css::uno::RuntimeException);
};

inline WindowImpl &Window::getImpl() const{ return *(static_cast< WindowImpl * >( mpImpl )); }

// Helpers for defining boiler-plate constructors ...
// Could in-line in top-level but not with safe static_casts.
#define IMPL_GET_IMPL(t) \
    inline t##Impl &t::getImpl() const \
    { \
        return *(static_cast<t##Impl *>(mpImpl)); \
    }
#define IMPL_CONSTRUCTORS_BODY(t,par,unoName,body) \
    t::t( Context *context, const char *pId, sal_uInt32 nId ) \
        : par( new t##Impl( context, context->GetPeerHandle( pId, nId ), this ) ) \
    { \
        Window *parent = dynamic_cast<Window*> (context);\
        body;\
        if (parent)\
            SetParent (parent);\
    } \
    t::t( Window *parent, WinBits bits) \
        : par( new t##Impl( parent->getContext(), Window::CreatePeer( parent, bits, unoName ), this ) ) \
    { \
        body;\
        if ( parent )\
            SetParent (parent);\
    } \
    t::t( Window *parent, ResId const& res) \
        : par( new t##Impl( parent->getContext(), Window::CreatePeer( parent, 0, unoName ), this ) ) \
    { \
        body;\
        setRes (res);\
        if (parent)\
            SetParent (parent);\
    }
#define IMPL_CONSTRUCTORS(t,par,unoName) IMPL_CONSTRUCTORS_BODY(t, par, unoName, )
#define IMPL_CONSTRUCTORS_2(t,win_par,other_par,unoName) \
    t::t( Context *context, const char *pId, sal_uInt32 nId ) \
        : win_par( new t##Impl( context, context->GetPeerHandle( pId, nId ), this ) ) \
        , other_par( new other_par##Impl( Window::GetPeer() ) ) \
    { \
    } \
    t::t( Window *parent, WinBits bits) \
        : win_par( new t##Impl( parent->getContext(), Window::CreatePeer( parent, bits, unoName ), this ) ) \
        , other_par( new other_par##Impl( Window::GetPeer() ) ) \
    { \
    }

#define IMPL_IMPL(t, parent) \
    class t##Impl : public parent##Impl \
    { \
    public: \
        t##Impl( Context *context, PeerHandle const& peer, Window *window ) \
            : parent##Impl( context, peer, window ) \
        { \
        } \
    };


} // namespace layout

#endif /* LAYOUT_VCL_WRAPPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
