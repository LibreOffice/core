/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wrapper.hxx,v $
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

#ifndef WRAPPER_HXX
#define WRAPPER_HXX

#include <layout/layout.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>

#include <cstring>

namespace css = com::sun::star;

namespace layout
{

class WindowImpl
{
  public:
    Window  *mpWindow;
    Context *mpCtx;
    css::uno::Reference< css::awt::XWindow >        mxWindow;
    css::uno::Reference< css::awt::XVclWindowPeer > mxVclPeer;

    WindowImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : mpWindow ( pWindow )
        , mpCtx ( pCtx )
        , mxWindow ( xPeer, css::uno::UNO_QUERY )
        , mxVclPeer( xPeer, css::uno::UNO_QUERY )
    {
    }

    virtual ~WindowImpl()
    {
    }

    void wrapperGone()
    {
        mpWindow = NULL;
        mpCtx = NULL;
        if ( mxWindow.is() )
        {
            css::uno::Reference< css::lang::XComponent > xComp( mxWindow, css::uno::UNO_QUERY );
            mxWindow.clear();
            if ( xComp.is() )
                xComp->dispose();
        }
    }

    css::uno::Any getProperty( const char *pName )
    {
        if ( !this || !mxVclPeer.is() )
            return css::uno::Any();
        return mxVclPeer->getProperty
            ( rtl::OUString( pName, strlen( pName ), RTL_TEXTENCODING_ASCII_US ) );
    }
    void setProperty( const char *pName, css::uno::Any aAny )
    {
        if ( !this || !mxVclPeer.is() )
            return;
        mxVclPeer->setProperty
            ( rtl::OUString( pName, strlen( pName ), RTL_TEXTENCODING_ASCII_US ), aAny );
    }
};

class ControlImpl : public WindowImpl
{
  public:
    ControlImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : WindowImpl( pCtx, xPeer, pWindow ) {}
};

inline WindowImpl &Window::getImpl() const
{ return *(static_cast< WindowImpl * >( mpImpl )); }

// Helpers for defining boiler-plate constructors ...
// Could in-line in top-level but not with safe static_casts.
#define DECL_GET_IMPL_IMPL(t) \
    inline t##Impl &t::getImpl() const \
    { \
        return *(static_cast<t##Impl *>(mpImpl)); \
    }
#define DECL_CONSTRUCTOR_IMPLS(t,par,unoName) \
    t::t( Context *pCtx, const char *pId, sal_uInt32 nId ) \
        : par( new t##Impl( pCtx, pCtx->GetPeerHandle( pId, nId ), this ) ) \
    { \
    } \
    t::t( Window *pParent, WinBits nBits) \
        : par( new t##Impl( pParent->getContext(), Window::CreatePeer( pParent, nBits, unoName ), this ) ) \
    { \
    }
#define DECL_CONSTRUCTOR_IMPLS_2(t,win_par,other_par,unoName) \
    t::t( Context *pCtx, const char *pId, sal_uInt32 nId ) \
        : win_par( new t##Impl( pCtx, pCtx->GetPeerHandle( pId, nId ), this ) ) \
        , other_par( new other_par##Impl( Window::GetPeer() ) ) \
    { \
    } \
    t::t( Window *pParent, WinBits nBits ) \
        : win_par( new t##Impl( pParent->getContext(), Window::CreatePeer( pParent, nBits, unoName ), this ) ) \
        , other_par( new other_par##Impl( Window::GetPeer() ) ) \
    { \
    }

} // namespace layout

#endif // WRAPPER_HXX
