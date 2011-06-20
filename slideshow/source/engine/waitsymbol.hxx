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

#if ! defined(WAITSYMBOL_HXX_INCLUDED)
#define WAITSYMBOL_HXX_INCLUDED

#include <com/sun/star/rendering/XBitmap.hpp>
#include <cppcanvas/customsprite.hxx>

#include "vieweventhandler.hxx"
#include "screenupdater.hxx"
#include "eventmultiplexer.hxx"
#include "unoview.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp> // for noncopyable
#include <vector>

namespace slideshow {
namespace internal {

class EventMultiplexer;
typedef boost::shared_ptr<class WaitSymbol> WaitSymbolSharedPtr;

class WaitSymbol : public ViewEventHandler,
                   private ::boost::noncopyable
{
public:
    static WaitSymbolSharedPtr create( const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::rendering::XBitmap>& xBitmap,
                                       ScreenUpdater&                               rScreenUpdater,
                                       EventMultiplexer&                            rEventMultiplexer,
                                       const UnoViewContainer&                      rViewContainer );

    /** Shows the wait symbol.
     */
    void show() { setVisible(true); }

    /** Hides the wait symbol.
     */
    void hide() { setVisible(false); }

private:
    WaitSymbol( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::rendering::XBitmap>& xBitmap,
                ScreenUpdater&                               rScreenUpdater,
                const UnoViewContainer&                      rViewContainer );

    // ViewEventHandler
    virtual void viewAdded( const UnoViewSharedPtr& rView );
    virtual void viewRemoved( const UnoViewSharedPtr& rView );
    virtual void viewChanged( const UnoViewSharedPtr& rView );
    virtual void viewsChanged();

    void setVisible( const bool bVisible );
    ::basegfx::B2DPoint calcSpritePos( UnoViewSharedPtr const & rView ) const;

    template <typename func_type>
    void for_each_sprite( func_type const & func ) const
    {
        ViewsVecT::const_iterator iPos( maViews.begin() );
        const ViewsVecT::const_iterator iEnd( maViews.end() );
        for ( ; iPos != iEnd; ++iPos )
            if( iPos->second )
                func( iPos->second );
    }

    typedef ::std::vector<
        ::std::pair<UnoViewSharedPtr,
                    cppcanvas::CustomSpriteSharedPtr> > ViewsVecT;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::rendering::XBitmap>  mxBitmap;

    ViewsVecT                                  maViews;
    ScreenUpdater&                             mrScreenUpdater;
    bool                                       mbVisible;
};

} // namespace internal
} // namespace presentation

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
