/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef POINTERSYMBOL_HXX_INCLUDED
#define POINTERSYMBOL_HXX_INCLUDED

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
typedef boost::shared_ptr<class PointerSymbol> PointerSymbolSharedPtr;

class PointerSymbol : public ViewEventHandler,
                   private ::boost::noncopyable
{
public:
    static PointerSymbolSharedPtr create( const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::rendering::XBitmap>& xBitmap,
                                       ScreenUpdater&                               rScreenUpdater,
                                       EventMultiplexer&                            rEventMultiplexer,
                                       const UnoViewContainer&                      rViewContainer );

    /** Shows the pointer symbol.
     */
    void show() { setVisible(true); }

    /** Hides the pointer symbol.
     */
    void hide() { setVisible(false); }

    virtual void viewsChanged(const ::com::sun::star::geometry::RealPoint2D& pos);

private:
    PointerSymbol( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::rendering::XBitmap>& xBitmap,
                ScreenUpdater&                               rScreenUpdater,
                const UnoViewContainer&                      rViewContainer );

    // ViewEventHandler
    virtual void viewAdded( const UnoViewSharedPtr& rView );
    virtual void viewRemoved( const UnoViewSharedPtr& rView );
    virtual void viewChanged( const UnoViewSharedPtr& rView );
    virtual void viewsChanged();

    void setVisible( const bool bVisible );

    basegfx::B2DPoint calcSpritePos( UnoViewSharedPtr const & rView, const ::com::sun::star::geometry::RealPoint2D& pos = ::com::sun::star::geometry::RealPoint2D(0, 0) ) const;

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
