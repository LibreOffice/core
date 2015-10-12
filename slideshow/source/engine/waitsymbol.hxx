/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_WAITSYMBOL_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_WAITSYMBOL_HXX

#include <com/sun/star/rendering/XBitmap.hpp>
#include <cppcanvas/customsprite.hxx>

#include "vieweventhandler.hxx"
#include "screenupdater.hxx"
#include "eventmultiplexer.hxx"
#include "unoview.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

namespace slideshow {
namespace internal {

class EventMultiplexer;
typedef boost::shared_ptr<class WaitSymbol> WaitSymbolSharedPtr;

/// On-screen 'hour glass' for when slideshow is unresponsive
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
    virtual void viewAdded( const UnoViewSharedPtr& rView ) override;
    virtual void viewRemoved( const UnoViewSharedPtr& rView ) override;
    virtual void viewChanged( const UnoViewSharedPtr& rView ) override;
    virtual void viewsChanged() override;

    void setVisible( const bool bVisible );
    ::basegfx::B2DPoint calcSpritePos( UnoViewSharedPtr const & rView ) const;

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
