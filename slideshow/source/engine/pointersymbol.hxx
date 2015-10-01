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
#include <cppcanvas/customsprite.hxx>

#include "com/sun/star/uno/Reference.hxx"

#include "vieweventhandler.hxx"
#include "screenupdater.hxx"
#include "eventmultiplexer.hxx"
#include "unoview.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

class EventMultiplexer;
typedef boost::shared_ptr<class PointerSymbol> PointerSymbolSharedPtr;

/// On-screen 'laser pointer' from the Impress remote control
class PointerSymbol : public ViewEventHandler,
                      private ::boost::noncopyable
{
public:
    static PointerSymbolSharedPtr create( const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::rendering::XBitmap>& xBitmap,
                                          ScreenUpdater&                               rScreenUpdater,
                                          EventMultiplexer&                            rEventMultiplexer,
                                          const UnoViewContainer&                      rViewContainer );

    /** Use this method to update the pointer's position
    */
    void setVisible( const bool bVisible );
    void viewsChanged(const ::com::sun::star::geometry::RealPoint2D pos);

private:
    PointerSymbol( const ::com::sun::star::uno::Reference<
                         ::com::sun::star::rendering::XBitmap>& xBitmap,
                   ScreenUpdater&                               rScreenUpdater,
                   const UnoViewContainer&                      rViewContainer );

    // ViewEventHandler
    virtual void viewAdded( const UnoViewSharedPtr& rView ) SAL_OVERRIDE;
    virtual void viewRemoved( const UnoViewSharedPtr& rView ) SAL_OVERRIDE;
    virtual void viewChanged( const UnoViewSharedPtr& rView ) SAL_OVERRIDE;
    virtual void viewsChanged() SAL_OVERRIDE;

    ::basegfx::B2DPoint calcSpritePos( UnoViewSharedPtr const & rView ) const;

    typedef ::std::vector<
        ::std::pair<UnoViewSharedPtr,
                    cppcanvas::CustomSpriteSharedPtr> > ViewsVecT;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::rendering::XBitmap>  mxBitmap;

    ViewsVecT                                  maViews;
    ScreenUpdater&                             mrScreenUpdater;
    ::com::sun::star::geometry::RealPoint2D    maPos;
    bool                                       mbVisible;
};

} // namespace internal
} // namespace presentation

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
