/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_POINTERSYMBOL_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_POINTERSYMBOL_HXX

#include <com/sun/star/rendering/XBitmap.hpp>
#include <cppcanvas/sprite.hxx>

#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <vieweventhandler.hxx>
#include <screenupdater.hxx>
#include <eventmultiplexer.hxx>
#include <unoview.hxx>

#include <vector>

using namespace com::sun::star;

namespace slideshow::internal {

class EventMultiplexer;
typedef std::shared_ptr<class PointerSymbol> PointerSymbolSharedPtr;

/// On-screen 'laser pointer' from the Impress remote control
class PointerSymbol : public ViewEventHandler
{
public:
    static PointerSymbolSharedPtr create( const css::uno::Reference<css::rendering::XBitmap>& xBitmap,
                                          ScreenUpdater&                               rScreenUpdater,
                                          EventMultiplexer&                            rEventMultiplexer,
                                          const UnoViewContainer&                      rViewContainer );

    /** Use this method to update the pointer's position
    */
    void setVisible( const bool bVisible );
    void viewsChanged(const css::geometry::RealPoint2D pos);

private:
    PointerSymbol( const css::uno::Reference<css::rendering::XBitmap>& xBitmap,
                   ScreenUpdater&                               rScreenUpdater,
                   const UnoViewContainer&                      rViewContainer );

    // ViewEventHandler
    virtual void viewAdded( const UnoViewSharedPtr& rView ) override;
    virtual void viewRemoved( const UnoViewSharedPtr& rView ) override;
    virtual void viewChanged( const UnoViewSharedPtr& rView ) override;
    virtual void viewsChanged() override;

    ::basegfx::B2DPoint calcSpritePos( UnoViewSharedPtr const & rView ) const;

    typedef ::std::vector<
        ::std::pair<UnoViewSharedPtr,
                    cppcanvas::CustomSpriteSharedPtr> > ViewsVecT;

    css::uno::Reference<css::rendering::XBitmap>  mxBitmap;

    ViewsVecT                                     maViews;
    ScreenUpdater&                                mrScreenUpdater;
    css::geometry::RealPoint2D                    maPos;
    bool                                          mbVisible;
};

} // namespace presentation::internal

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
