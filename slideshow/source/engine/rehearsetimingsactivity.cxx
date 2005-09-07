/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rehearsetimingsactivity.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:28:14 $
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

#include "rtl/ustrbuf.hxx"
#include "vcl/svapp.hxx"
#include "vcl/gdimtf.hxx"
#include "vcl/virdev.hxx"
#include "vcl/metric.hxx"
#include "cppcanvas/vclfactory.hxx"
#include "cppcanvas/basegfxfactory.hxx"
#include "eventqueue.hxx"
#include "eventmultiplexer.hxx"
#include "activitiesqueue.hxx"
#include "mouseeventhandler.hxx"
#include "rehearsetimingsactivity.hxx"
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include "boost/bind.hpp"
#include <algorithm>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace presentation {
namespace internal {

const sal_Int32 LEFT_BORDER_SPACE  = 10;
const sal_Int32 LOWER_BORDER_SPACE = 30;

RehearseTimingsActivity::RehearseTimingsActivity(
    EventQueue & rEventQueue,
    EventMultiplexer & rEventMultiplexer,
    ActivitiesQueue & rActivitiesQueue )
    : m_rEventQueue(rEventQueue),
      m_rEventMultiplexer(rEventMultiplexer),
      m_rActivitiesQueue(rActivitiesQueue),
      m_this(),
      m_elapsedTime( rEventQueue.getTimer() ),
      m_views(),
      m_spriteRectangle(),
      m_font( Application::GetSettings().GetStyleSettings().GetInfoFont() ),
      m_wakeUpEvent( new WakeupEvent( rEventQueue.getTimer(),
                                      rActivitiesQueue ) ),
      m_mouseHandler(),
      m_bActive(false),
      m_drawPressed(false)
{
//     m_font.SetOutline(true);
    m_font.SetHeight( m_font.GetHeight() * 2 );
    m_font.SetWidth( m_font.GetWidth() * 2 );
    m_font.SetAlign( ALIGN_BASELINE );
//     m_font.SetColor( COL_WHITE );
    m_font.SetColor( COL_BLACK );

    // determine sprite size (in pixel):
    VirtualDevice blackHole;
    blackHole.EnableOutput(false);
    blackHole.SetFont( m_font );
    blackHole.SetMapMode( MAP_PIXEL );
    Rectangle rect;
    const FontMetric metric( blackHole.GetFontMetric() );
    blackHole.GetTextBoundRect(
        rect, String(RTL_CONSTASCII_USTRINGPARAM("XX:XX:XX")) );
    m_spriteSizePixel = Size(
        rect.getWidth() * 12 / 10, metric.GetLineHeight() * 11 / 10 );
    m_nYOffset = (metric.GetAscent() + (metric.GetLineHeight() / 20));
}

boost::shared_ptr<RehearseTimingsActivity> RehearseTimingsActivity::create(
    EventQueue & rEventQueue,
    EventMultiplexer & rEventMultiplexer,
    ActivitiesQueue & rActivitiesQueue )
{
    boost::shared_ptr<RehearseTimingsActivity> activity(
        new RehearseTimingsActivity( rEventQueue,
                                     rEventMultiplexer,
                                     rActivitiesQueue ) );
    activity->m_this = activity;
    activity->m_mouseHandler.reset( new MouseHandler(activity) );
    activity->m_wakeUpEvent->setActivity(activity);
    return activity;
}

void RehearseTimingsActivity::start()
{
    m_elapsedTime.reset();
    m_drawPressed = false;
    m_bActive = true;

    // paint and show all sprites:
    paintAllSprites();
    for_each_sprite( boost::bind( &cppcanvas::Sprite::show, _1 ) );

    m_rActivitiesQueue.addActivity( m_this );

    m_mouseHandler->reset();
    m_rEventMultiplexer.addClickHandler(
        m_mouseHandler, 42 /* highest prio of all, > 3.0 */ );
    m_rEventMultiplexer.addMouseMoveHandler(
        m_mouseHandler, 42 /* highest prio of all, > 3.0 */ );
}

double RehearseTimingsActivity::stop()
{
    m_rEventMultiplexer.removeMouseMoveHandler( m_mouseHandler );
    m_rEventMultiplexer.removeClickHandler( m_mouseHandler );
    m_bActive = false; // will be removed from queue
    for_each_sprite( boost::bind( &cppcanvas::Sprite::hide, _1 ) );
    return m_elapsedTime.getElapsedTime();
}

bool RehearseTimingsActivity::hasBeenClicked() const
{
    if (m_mouseHandler.get() != 0)
        return m_mouseHandler->hasBeenClicked();
    return false;
}

// Disposable:
void RehearseTimingsActivity::dispose()
{
    stop();
    if (m_wakeUpEvent.get() != 0) {
        m_wakeUpEvent->dispose();
        m_wakeUpEvent.reset();
    }
    if (m_mouseHandler.get() != 0) {
        m_mouseHandler->dispose();
        m_mouseHandler.reset();
    }
    ViewsVecT().swap( m_views );
    m_this.reset();
}

// Activity:
double RehearseTimingsActivity::calcTimeLag() const
{
    return 0.0;
}

bool RehearseTimingsActivity::perform()
{
    if (! isActive())
        return false;
    OSL_ENSURE( m_wakeUpEvent.get() != 0, "### no wake-up event!" );
    if (m_wakeUpEvent.get() == 0)
        return false;

    m_wakeUpEvent->start();
    m_wakeUpEvent->setNextTimeout( 0.5 /* secs */ );
    m_rEventQueue.addEvent( m_wakeUpEvent );

    paintAllSprites();

    return false; // don't reinsert, WakeupEvent will perform
                  // that after the given timeout
}

bool RehearseTimingsActivity::isActive() const
{
    return m_bActive;
}

bool RehearseTimingsActivity::needsScreenUpdate() const
{
    return isActive();
}

void RehearseTimingsActivity::dequeued()
{
    // not used here
}

basegfx::B2DRectangle RehearseTimingsActivity::calcSpriteRectangle(
    UnoViewSharedPtr const & rView ) const
{
    const Reference<rendering::XBitmap> xBitmap(
        rView->getCanvas()->getUNOCanvas(), UNO_QUERY_THROW );
    const geometry::IntegerSize2D realSize( xBitmap->getSize() );
    // pixel:
    basegfx::B2DPoint spritePos(
        std::min<sal_Int32>( realSize.Width, LEFT_BORDER_SPACE ),
        std::max<sal_Int32>( 0, realSize.Height - m_spriteSizePixel.getHeight()
                                                - LOWER_BORDER_SPACE ) );
    basegfx::B2DHomMatrix transformation( rView->getTransformation() );
    transformation.invert();
    spritePos *= transformation;
    basegfx::B2DSize spriteSize( m_spriteSizePixel.getWidth(),
                                 m_spriteSizePixel.getHeight() );
    spriteSize *= transformation;
    return basegfx::B2DRectangle(
        spritePos.getX(), spritePos.getY(),
        spritePos.getX() + spriteSize.getX(),
        spritePos.getY() + spriteSize.getY() );
}

void RehearseTimingsActivity::addView( UnoViewSharedPtr const & rView )
{
    const ViewsVecT::iterator iEnd( m_views.end() );
    if (std::find_if(
            m_views.begin(), iEnd,
            boost::bind(
                std::equal_to<UnoViewSharedPtr>(),
                rView,
                // select view:
                boost::bind( std::select1st<ViewsVecT::value_type>(), _1 ) ) )!=
        iEnd)
        return; // already added

    cppcanvas::CustomSpriteSharedPtr sprite(
        rView->createSprite( basegfx::B2DSize(
                                 m_spriteSizePixel.getWidth(),
                                 m_spriteSizePixel.getHeight() ) ) );
    sprite->setAlpha( 0.9 );
    const basegfx::B2DRectangle spriteRectangle( calcSpriteRectangle( rView ) );
    sprite->move( basegfx::B2DPoint(
                      spriteRectangle.getMinX(), spriteRectangle.getMinY() ) );
    m_views.push_back( ViewsVecT::value_type( rView, sprite ) );
    if (isActive())
        sprite->show();
}

void RehearseTimingsActivity::removeView( UnoViewSharedPtr const & rView )
{
    m_views.erase(
        std::remove_if(
            m_views.begin(), m_views.end(),
            boost::bind(
                std::equal_to<UnoViewSharedPtr>(),
                rView,
                // select view:
                boost::bind( std::select1st<ViewsVecT::value_type>(), _1 ) ) ),
        m_views.end() );
}

void RehearseTimingsActivity::notifyViewChange()
{
    if (! m_views.empty()) {
        // new sprite pos, transformation might have changed:
        m_spriteRectangle = calcSpriteRectangle( m_views.begin()->first );

        // reposition all sprites:
        const basegfx::B2DPoint spritePos(
            m_spriteRectangle.getMinX(), m_spriteRectangle.getMinY() );
        for_each_sprite( boost::bind( &cppcanvas::Sprite::move,
                                      _1, boost::cref(spritePos) ) );
    }
}

void RehearseTimingsActivity::paintAllSprites() const
{
    for_each_sprite(
        boost::bind( &RehearseTimingsActivity::paint, this,
                     // call getContentCanvas() on each sprite:
                     boost::bind(
                         &cppcanvas::CustomSprite::getContentCanvas, _1 ) ) );
}

void RehearseTimingsActivity::paint(
    cppcanvas::CanvasSharedPtr const & canvas ) const
{
    // build timer string:
    const sal_Int32 nTimeSecs =
        static_cast<sal_Int32>(m_elapsedTime.getElapsedTime());
    rtl::OUStringBuffer buf;
    sal_Int32 n = (nTimeSecs / 3600);
    if (n < 10)
        buf.append( static_cast<sal_Unicode>('0') );
    buf.append( n );
    buf.append( static_cast<sal_Unicode>(':') );
    n = ((nTimeSecs % 3600) / 60);
    if (n < 10)
        buf.append( static_cast<sal_Unicode>('0') );
    buf.append( n );
    buf.append( static_cast<sal_Unicode>(':') );
    n = (nTimeSecs % 60);
    if (n < 10)
        buf.append( static_cast<sal_Unicode>('0') );
    buf.append( n );
    const rtl::OUString time = buf.makeStringAndClear();

    cppcanvas::BaseGfxFactory & factory =
        cppcanvas::BaseGfxFactory::getInstance();

    // create the MetaFile:
    GDIMetaFile metaFile;
    VirtualDevice blackHole;
    metaFile.Record( &blackHole );
    metaFile.SetPrefSize( Size( 1, 1 ) );
    blackHole.EnableOutput(false);
    blackHole.SetMapMode( MAP_PIXEL );
    blackHole.SetFont( m_font );
    Rectangle rect = Rectangle( Point(), m_spriteSizePixel );
    if (m_drawPressed) {
        blackHole.SetTextColor( COL_BLACK );
        blackHole.SetFillColor( COL_LIGHTGRAY );
        blackHole.SetLineColor( COL_GRAY );
    }
    else {
        blackHole.SetTextColor( COL_BLACK );
        blackHole.SetFillColor( COL_WHITE );
        blackHole.SetLineColor( COL_GRAY );
    }
    blackHole.DrawRect( rect );
    blackHole.GetTextBoundRect( rect, time );
    blackHole.DrawText(
        Point( (m_spriteSizePixel.getWidth() - rect.getWidth()) / 2,
               m_nYOffset ), time );

    metaFile.Stop();
    metaFile.WindStart();

    cppcanvas::RendererSharedPtr renderer(
        cppcanvas::VCLFactory::getInstance().createRenderer(
            canvas, metaFile, cppcanvas::Renderer::Parameters() ) );
    const bool succ = renderer->draw();
    OSL_ASSERT( succ ); succ;
}


RehearseTimingsActivity::MouseHandler::MouseHandler(
    boost::shared_ptr<RehearseTimingsActivity> const & rta )
    : m_rta(rta),
      m_hasBeenClicked(false),
      m_mouseStartedInArea(false)
{
}

void RehearseTimingsActivity::MouseHandler::reset()
{
    m_hasBeenClicked = false;
    m_mouseStartedInArea = false;
}

// Disposable:
void RehearseTimingsActivity::MouseHandler::dispose()
{
    m_rta.reset();
}

bool RehearseTimingsActivity::MouseHandler::isInArea(
    awt::MouseEvent const & evt ) const
{
    if (m_rta.get() != 0)
        return m_rta->m_spriteRectangle.isInside(
            basegfx::B2DPoint( evt.X, evt.Y ) );
    return false;
}

void RehearseTimingsActivity::MouseHandler::updatePressedState(
    const bool pressedState ) const
{
    if (pressedState != m_rta->m_drawPressed)
    {
        m_rta->m_drawPressed = pressedState;
        m_rta->paintAllSprites();

        // update screen immediately (cannot wait for next
        // ActivitiesQueue loop)
        m_rta->m_rEventMultiplexer.updateScreenContent( true );
    }
}

// MouseEventHandler
bool RehearseTimingsActivity::MouseHandler::handleMousePressed(
    awt::MouseEvent const & evt )
{
    if (!isDisposed() && isInArea(evt)) {
        m_mouseStartedInArea = true;
        updatePressedState(true);
        return true; // consume event
    }
    return false;
}

bool RehearseTimingsActivity::MouseHandler::handleMouseReleased(
    awt::MouseEvent const & evt )
{
    if (!isDisposed() && m_mouseStartedInArea) {
        m_hasBeenClicked = isInArea(evt); // fini if in
        m_mouseStartedInArea = false;
        updatePressedState(false);
        if (! m_hasBeenClicked)
            return true; // consume event, else next slide (manual advance)
    }
    return false;
}

bool RehearseTimingsActivity::MouseHandler::handleMouseEntered(
    awt::MouseEvent const & evt )
{
    return false;
}

bool RehearseTimingsActivity::MouseHandler::handleMouseExited(
    awt::MouseEvent const & evt )
{
    return false;
}

bool RehearseTimingsActivity::MouseHandler::handleMouseDragged(
    awt::MouseEvent const & evt )
{
    if (!isDisposed() && m_mouseStartedInArea)
        updatePressedState( isInArea(evt) );
    return false;
}

bool RehearseTimingsActivity::MouseHandler::handleMouseMoved(
    awt::MouseEvent const & evt )
{
    return false;
}

} // namespace internal
} // namespace presentation
