/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rehearsetimingsactivity.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2005-10-11 08:34:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#if ! defined(REHEARSETIMINGSACTIVITY_HXX_INCLUDED)
#define REHEARSETIMINGSACTIVITY_HXX_INCLUDED

#include "activity.hxx"
#include "wakeupevent.hxx"
#include "canvas/elapsedtime.hxx"
#include "cppcanvas/customsprite.hxx"
#include "basegfx/range/b2drectangle.hxx"
#include "vcl/font.hxx"
#include "boost/shared_ptr.hpp"
#include "boost/bind.hpp"
#include "boost/utility.hpp" // for noncopyable
#include <vector>
#include <algorithm>

namespace presentation {
namespace internal {

class EventQueue;
class ActivitiesQueue;
class EventMultiplexer;

class RehearseTimingsActivity : public Activity, private ::boost::noncopyable
{
public:
    /** Creates the activity.
     */
    static boost::shared_ptr<RehearseTimingsActivity> create(
        EventQueue & rEventQueue,
        EventMultiplexer & rEventMultiplexer,
        ActivitiesQueue & rActivitiesQueue );

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

    /** Adds a view for timer display.
     */
    void addView( UnoViewSharedPtr const & rView );

    void removeView( UnoViewSharedPtr const & rView );

    void notifyViewChange();

    // Disposable:
    virtual void dispose();
    // Activity:
    virtual double calcTimeLag() const;
    virtual bool perform();
    virtual bool isActive() const;
    virtual bool needsScreenUpdate() const;
    virtual void dequeued();
    virtual void end();

private:
    RehearseTimingsActivity(
        EventQueue & rEventQueue,
        EventMultiplexer & rEventMultiplexer,
        ActivitiesQueue & rActivitiesQueue );

    void paint( ::cppcanvas::CanvasSharedPtr const & canvas ) const;
    void paintAllSprites() const;

    class MouseHandler;
    friend class MouseHandler;

    EventQueue & m_rEventQueue;
    EventMultiplexer & m_rEventMultiplexer;
    ActivitiesQueue & m_rActivitiesQueue;
    boost::shared_ptr<RehearseTimingsActivity> m_this;
    canvas::tools::ElapsedTime m_elapsedTime;

    typedef ::std::vector<
        ::std::pair<UnoViewSharedPtr,
                    cppcanvas::CustomSpriteSharedPtr> > ViewsVecT;
    ViewsVecT m_views;

    template <typename func_type>
    void for_each_sprite( func_type const & func ) const
    {
        ViewsVecT::const_iterator iPos( m_views.begin() );
        const ViewsVecT::const_iterator iEnd( m_views.end() );
        for ( ; iPos != iEnd; ++iPos )
            func( iPos->second );
    }

    ::basegfx::B2DRectangle m_spriteRectangle;
    ::basegfx::B2DRectangle calcSpriteRectangle(
        UnoViewSharedPtr const & rView ) const;

    Font m_font;
    WakeupEventSharedPtr m_wakeUpEvent;
    boost::shared_ptr<MouseHandler> m_mouseHandler;
    long m_nYOffset;
    Size m_spriteSizePixel;
    bool m_bActive;
    bool m_drawPressed;
};

} // namespace internal
} // namespace presentation

#endif
