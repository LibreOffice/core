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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include <boost/current_function.hpp>
#include <canvas/canvastools.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>

#include "waitsymbol.hxx"
#include "eventmultiplexer.hxx"

#include <o3tl/compat_functional.hxx>
#include <algorithm>


using namespace com::sun::star;

namespace slideshow {
namespace internal {

const sal_Int32 LEFT_BORDER_SPACE  = 10;
const sal_Int32 LOWER_BORDER_SPACE = 10;

WaitSymbolSharedPtr WaitSymbol::create( const uno::Reference<rendering::XBitmap>& xBitmap,
                                        ScreenUpdater&                            rScreenUpdater,
                                        EventMultiplexer&                         rEventMultiplexer,
                                        const UnoViewContainer&                   rViewContainer )
{
    WaitSymbolSharedPtr pRet(
        new WaitSymbol( xBitmap,
                        rScreenUpdater,
                        rViewContainer ));

    rEventMultiplexer.addViewHandler( pRet );

    return pRet;
}

WaitSymbol::WaitSymbol( uno::Reference<rendering::XBitmap> const &   xBitmap,
                        ScreenUpdater&                               rScreenUpdater,
                        const UnoViewContainer&                      rViewContainer ) :
    mxBitmap(xBitmap),
    maViews(),
    mrScreenUpdater( rScreenUpdater ),
    mbVisible(false)
{
    std::for_each( rViewContainer.begin(),
                   rViewContainer.end(),
                   boost::bind( &WaitSymbol::viewAdded,
                                this,
                                _1 ));
}

void WaitSymbol::setVisible( const bool bVisible )
{
    if( mbVisible != bVisible )
    {
        mbVisible = bVisible;

        ViewsVecT::const_iterator       aIter( maViews.begin() );
        ViewsVecT::const_iterator const aEnd ( maViews.end() );
        while( aIter != aEnd )
        {
            if( aIter->second )
            {
                if( bVisible )
                    aIter->second->show();
                else
                    aIter->second->hide();
            }

            ++aIter;
        }

        // sprites changed, need a screen update for this frame.
        mrScreenUpdater.requestImmediateUpdate();
    }
}

basegfx::B2DPoint WaitSymbol::calcSpritePos(
    UnoViewSharedPtr const & rView ) const
{
    const uno::Reference<rendering::XBitmap> xBitmap( rView->getCanvas()->getUNOCanvas(),
                                                      uno::UNO_QUERY_THROW );
    const geometry::IntegerSize2D realSize( xBitmap->getSize() );
    return basegfx::B2DPoint(
        std::min<sal_Int32>( realSize.Width, LEFT_BORDER_SPACE ),
        std::max<sal_Int32>( 0, realSize.Height - mxBitmap->getSize().Height
                                                - LOWER_BORDER_SPACE ) );
}

void WaitSymbol::viewAdded( const UnoViewSharedPtr& rView )
{
    cppcanvas::CustomSpriteSharedPtr sprite;

    try
    {
        const geometry::IntegerSize2D spriteSize( mxBitmap->getSize() );
        sprite = rView->createSprite( basegfx::B2DVector( spriteSize.Width,
                                                          spriteSize.Height ),
                                      1000.0 ); // sprite should be in front of all
                                                // other sprites
        rendering::ViewState viewState;
        canvas::tools::initViewState( viewState );
        rendering::RenderState renderState;
        canvas::tools::initRenderState( renderState );
        sprite->getContentCanvas()->getUNOCanvas()->drawBitmap(
            mxBitmap, viewState, renderState );

        sprite->setAlpha( 0.9 );
        sprite->movePixel( calcSpritePos( rView ) );
        if( mbVisible )
            sprite->show();
    }
    catch( uno::Exception& )
    {
        OSL_FAIL( rtl::OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
    }

    maViews.push_back( ViewsVecT::value_type( rView, sprite ) );
}

void WaitSymbol::viewRemoved( const UnoViewSharedPtr& rView )
{
    maViews.erase(
        std::remove_if(
            maViews.begin(), maViews.end(),
            boost::bind(
                std::equal_to<UnoViewSharedPtr>(),
                rView,
                // select view:
                boost::bind( o3tl::select1st<ViewsVecT::value_type>(), _1 ) ) ),
        maViews.end() );
}

void WaitSymbol::viewChanged( const UnoViewSharedPtr& rView )
{
    // find entry corresponding to modified view
    ViewsVecT::iterator aModifiedEntry(
        std::find_if(
            maViews.begin(),
            maViews.end(),
            boost::bind(
                std::equal_to<UnoViewSharedPtr>(),
                rView,
                // select view:
                boost::bind( o3tl::select1st<ViewsVecT::value_type>(), _1 ))));

    OSL_ASSERT( aModifiedEntry != maViews.end() );
    if( aModifiedEntry == maViews.end() )
        return;

    if( aModifiedEntry->second )
        aModifiedEntry->second->movePixel(
            calcSpritePos(aModifiedEntry->first) );
}

void WaitSymbol::viewsChanged()
{
    // reposition sprites on all views
    ViewsVecT::const_iterator       aIter( maViews.begin() );
    ViewsVecT::const_iterator const aEnd ( maViews.end() );
    while( aIter != aEnd )
    {
        if( aIter->second )
            aIter->second->movePixel(
                calcSpritePos( aIter->first ));
        ++aIter;
    }
}

} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
