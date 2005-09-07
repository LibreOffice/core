/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: waitsymbol.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:33:17 $
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

#include "waitsymbol.hxx"
#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "canvas/canvastools.hxx"
#include <algorithm>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace presentation {
namespace internal {

const sal_Int32 LEFT_BORDER_SPACE  = 10;
const sal_Int32 LOWER_BORDER_SPACE = 10;

    WaitSymbol::WaitSymbol( Reference<rendering::XBitmap> const &   xBitmap,
                            EventMultiplexer&                       rEventMultiplexer ) :
        m_xBitmap(xBitmap),
        m_views(),
        mrEventMultiplexer( rEventMultiplexer ),
        m_bVisible(false)
{
}

void WaitSymbol::setVisible( const bool bVisible )
{
    if (m_bVisible != bVisible)
    {
        m_bVisible = bVisible;
        for_each_sprite( boost::bind( bVisible ? &cppcanvas::Sprite::show
                                               : &cppcanvas::Sprite::hide,
                                      _1 ) );

        // force screen update, this implementation bypasses the layer
        // manager.
        mrEventMultiplexer.updateScreenContent( true );
    }
}

// Disposable:
void WaitSymbol::dispose()
{
    m_xBitmap.clear();
    ViewsVecT().swap( m_views );
}

basegfx::B2DPoint WaitSymbol::calcSpritePos(
    UnoViewSharedPtr const & rView ) const
{
    const Reference<rendering::XBitmap> xBitmap(
        rView->getCanvas()->getUNOCanvas(), UNO_QUERY_THROW );
    const geometry::IntegerSize2D realSize( xBitmap->getSize() );
    return basegfx::B2DPoint(
        std::min<sal_Int32>( realSize.Width, LEFT_BORDER_SPACE ),
        std::max<sal_Int32>( 0, realSize.Height - m_xBitmap->getSize().Height
                                                - LOWER_BORDER_SPACE ) );
}

void WaitSymbol::addView( UnoViewSharedPtr const & rView )
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

    const geometry::IntegerSize2D spriteSize( m_xBitmap->getSize() );
    cppcanvas::CustomSpriteSharedPtr sprite(
        rView->createSprite( basegfx::B2DSize( spriteSize.Width,
                                               spriteSize.Height ) ) );

    try {
        rendering::ViewState viewState;
        canvas::tools::initViewState( viewState );
        rendering::RenderState renderState;
        canvas::tools::initRenderState( renderState );
        sprite->getContentCanvas()->getUNOCanvas()->drawBitmap(
            m_xBitmap, viewState, renderState );
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (Exception &) {
        OSL_ENSURE( false,
                    rtl::OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
    }

    sprite->setAlpha( 0.9 );
    sprite->movePixel( calcSpritePos( rView ) );
    m_views.push_back( ViewsVecT::value_type( rView, sprite ) );
    if (m_bVisible)
        sprite->show();
}

void WaitSymbol::removeView( UnoViewSharedPtr const & rView )
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

void WaitSymbol::notifyViewChange()
{
    if (! m_views.empty()) {
        // reposition all sprites:
        const basegfx::B2DPoint spritePos(
            calcSpritePos( m_views.begin()->first ) );
        for_each_sprite( boost::bind( &cppcanvas::Sprite::movePixel,
                                      _1, boost::cref(spritePos) ) );
    }
}

} // namespace internal
} // namespace presentation
