/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: null_spritecanvashelper.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <comphelper/scopeguard.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <boost/cast.hpp>

#include "null_spritecanvashelper.hxx"
#include "null_canvascustomsprite.hxx"


using namespace ::com::sun::star;

namespace nullcanvas
{
    SpriteCanvasHelper::SpriteCanvasHelper() :
        mpRedrawManager( NULL )
    {
    }

    void SpriteCanvasHelper::init( ::canvas::SpriteRedrawManager& rManager,
                                   SpriteCanvas&                  rDevice,
                                   const ::basegfx::B2ISize&      rSize,
                                   bool                           bHasAlpha )
    {
        mpRedrawManager = &rManager;

        CanvasHelper::init( rSize, rDevice, bHasAlpha );
    }

    void SpriteCanvasHelper::disposing()
    {
        mpRedrawManager = NULL;

        // forward to base
        CanvasHelper::disposing();
    }

    uno::Reference< rendering::XAnimatedSprite > SpriteCanvasHelper::createSpriteFromAnimation(
        const uno::Reference< rendering::XAnimation >& /*animation*/ )
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XAnimatedSprite > SpriteCanvasHelper::createSpriteFromBitmaps(
        const uno::Sequence< uno::Reference< rendering::XBitmap > >& /*animationBitmaps*/,
        sal_Int8                                                     /*interpolationMode*/ )
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XCustomSprite > SpriteCanvasHelper::createCustomSprite( const geometry::RealSize2D& spriteSize )
    {
        if( !mpRedrawManager )
            return uno::Reference< rendering::XCustomSprite >(); // we're disposed

        return uno::Reference< rendering::XCustomSprite >(
            new CanvasCustomSprite( spriteSize,
                                    mpDevice ) );
    }

    uno::Reference< rendering::XSprite > SpriteCanvasHelper::createClonedSprite( const uno::Reference< rendering::XSprite >& /*original*/ )
    {
        return uno::Reference< rendering::XSprite >();
    }

    sal_Bool SpriteCanvasHelper::updateScreen( const ::basegfx::B2IRange& /*rCurrArea*/,
                                               sal_Bool                   /*bUpdateAll*/,
                                               bool&                      /*io_bSurfaceDirty*/ )
    {
        // TODO
        return sal_True;
    }

    void SpriteCanvasHelper::backgroundPaint( const ::basegfx::B2DRange& /*rUpdateRect*/ )
    {
        // TODO
    }

    void SpriteCanvasHelper::scrollUpdate( const ::basegfx::B2DRange&                       /*rMoveStart*/,
                                           const ::basegfx::B2DRange&                       /*rMoveEnd*/,
                                           const ::canvas::SpriteRedrawManager::UpdateArea& /*rUpdateArea*/ )
    {
        // TODO
    }

    void SpriteCanvasHelper::opaqueUpdate( const ::canvas::SpriteRedrawManager::UpdateArea& /*rUpdateArea*/ )
    {
        // TODO
    }

    void SpriteCanvasHelper::genericUpdate( const ::canvas::SpriteRedrawManager::UpdateArea& /*rUpdateArea*/ )
    {
        // TODO
    }

}
