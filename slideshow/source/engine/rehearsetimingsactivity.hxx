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

#ifndef INCLUDED_SLIDESHOW_REHEARSETIMINGSACTIVITY_HXX
#define INCLUDED_SLIDESHOW_REHEARSETIMINGSACTIVITY_HXX

#include "activity.hxx"

#include <basegfx/range/b2drange.hxx>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

#include <vector>
#include <utility>

class Font;
namespace canvas{ namespace tools{ class ElapsedTime; }}
namespace cppcanvas{ class CustomSprite; }
namespace basegfx
{
    class B2IVector;
    class B2DRange;
}

namespace slideshow {
namespace internal {

struct SlideShowContext;
class EventMultiplexer;
class ScreenUpdater;
class RehearseTimingsActivity : public Activity,
                                public ViewEventHandler,
                                public boost::enable_shared_from_this<RehearseTimingsActivity>,
                                private ::boost::noncopyable
{
public:
    /** Creates the activity.
     */
    static boost::shared_ptr<RehearseTimingsActivity> create(
        const SlideShowContext& rContext );

    virtual ~RehearseTimingsActivity();

    /** Starts and shows the timer; adds to activity queue.
     */
    void start();

    /** Stops and hides the timer.
        @return elapsed time
     */
    double stop();

    /** Determines whether the timer button has been clicked.
     */
    bool hasBeenClicked() const;

    // ViewEventHandler interface
    virtual void viewAdded( const UnoViewSharedPtr& rView );
    virtual void viewRemoved( const UnoViewSharedPtr& rView );
    virtual void viewChanged( const UnoViewSharedPtr& rView );
    virtual void viewsChanged();

    // Disposable:
    virtual void dispose();
    // Activity:
    virtual double calcTimeLag() const;
    virtual bool perform();
    virtual bool isActive() const;
    virtual void dequeued();
    virtual void end();

private:
    class WakeupEvent;

    explicit RehearseTimingsActivity( const SlideShowContext& rContext );

    void paint( ::cppcanvas::CanvasSharedPtr const & canvas ) const;
    void paintAllSprites() const;

    class MouseHandler;
    friend class MouseHandler;

    typedef ::std::vector<
        ::std::pair<UnoViewSharedPtr,
                    boost::shared_ptr<cppcanvas::CustomSprite> > > ViewsVecT;

    template <typename func_type>
    void for_each_sprite( func_type const & func ) const
    {
        ViewsVecT::const_iterator iPos( maViews.begin() );
        const ViewsVecT::const_iterator iEnd( maViews.end() );
        for ( ; iPos != iEnd; ++iPos )
            func( iPos->second );
    }

    ::basegfx::B2DRange calcSpriteRectangle(
        UnoViewSharedPtr const & rView ) const;

    EventQueue&                     mrEventQueue;
    ScreenUpdater&                  mrScreenUpdater;
    EventMultiplexer&               mrEventMultiplexer;
    ActivitiesQueue&                mrActivitiesQueue;
    canvas::tools::ElapsedTime      maElapsedTime;

    ViewsVecT                       maViews;

    /// screen rect of sprite (in view coordinates!)
    ::basegfx::B2DRange             maSpriteRectangle;

    Font                            maFont;
    boost::shared_ptr<WakeupEvent>  mpWakeUpEvent;
    boost::shared_ptr<MouseHandler> mpMouseHandler;
    ::basegfx::B2IVector            maSpriteSizePixel;
    sal_Int32                       mnYOffset;
    bool                            mbActive;
    bool                            mbDrawPressed;
};

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_REHEARSETIMINGSACTIVITY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
