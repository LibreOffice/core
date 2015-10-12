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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_REHEARSETIMINGSACTIVITY_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_REHEARSETIMINGSACTIVITY_HXX

#include "activity.hxx"

#include <basegfx/range/b2drange.hxx>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

#include <vector>
#include <utility>

namespace vcl { class Font; }
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
    virtual void viewAdded( const UnoViewSharedPtr& rView ) override;
    virtual void viewRemoved( const UnoViewSharedPtr& rView ) override;
    virtual void viewChanged( const UnoViewSharedPtr& rView ) override;
    virtual void viewsChanged() override;

    // Disposable:
    virtual void dispose() override;
    // Activity:
    virtual double calcTimeLag() const override;
    virtual bool perform() override;
    virtual bool isActive() const override;
    virtual void dequeued() override;
    virtual void end() override;

private:
    class WakeupEvent;

    explicit RehearseTimingsActivity( const SlideShowContext& rContext );

    void paint( ::cppcanvas::CanvasSharedPtr const & canvas ) const;
    void paintAllSprites() const;

    class MouseHandler;
    friend class MouseHandler;

    typedef std::vector<
        std::pair<UnoViewSharedPtr,
                    std::shared_ptr<cppcanvas::CustomSprite> > > ViewsVecT;

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

    vcl::Font                       maFont;
    boost::shared_ptr<WakeupEvent>  mpWakeUpEvent;
    boost::shared_ptr<MouseHandler> mpMouseHandler;
    ::basegfx::B2IVector            maSpriteSizePixel;
    sal_Int32                       mnYOffset;
    bool                            mbActive;
    bool                            mbDrawPressed;
};

} // namespace internal
} // namespace presentation

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_REHEARSETIMINGSACTIVITY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
