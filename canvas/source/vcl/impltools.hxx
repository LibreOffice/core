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

#pragma once

#include <osl/mutex.hxx>

#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <com/sun/star/uno/Reference.hxx>

#include "outdevprovider.hxx"


class OutputDevice;
class Point;
class Size;

namespace basegfx
{
    namespace matrix
    {
        class B2DHomMatrix;
    }
}

namespace com::sun::star::awt
{
    struct Point;
    struct Size;
    struct Rectangle;
}

namespace com::sun::star::drawing
{
    struct HomogenMatrix3;
}

namespace com::sun::star::geometry
{
    struct RealPoint2D;
    struct RealSize2D;
    struct RealRectangle2D;
}

namespace com::sun::star::rendering
{
    struct RenderState;
    struct ViewState;
    class  XBitmap;
}


namespace vclcanvas
{
    namespace tools
    {
        ::BitmapEx
        bitmapExFromXBitmap( const css::uno::Reference<
                             css::rendering::XBitmap >& );

        /** Setup VCL font and output position

            @returns false, if no text output should happen
         */
        bool setupFontTransform( ::Point&                              o_rPoint,
                                 vcl::Font&                            io_rVCLFont,
                                 const css::rendering::ViewState&      viewState,
                                 const css::rendering::RenderState&    renderState,
                                 ::OutputDevice const &                rOutDev );

        /** Predicate, to determine whether polygon is actually an axis-aligned rectangle

            @return true, if the polygon is a rectangle.
         */
        bool isRectangle( const ::tools::PolyPolygon& rPolyPoly );


        // Little helper to encapsulate locking into policy class
        class LocalGuard
        {
        public:
            LocalGuard() :
                aSolarGuard()
            {
            }

            /// To be compatible with CanvasBase mutex concept
            explicit LocalGuard( const ::osl::Mutex& ) :
                aSolarGuard()
            {
            }

        private:
            SolarMutexGuard aSolarGuard;
        };

        class OutDevStateKeeper
        {
        public:
            explicit OutDevStateKeeper( OutputDevice& rOutDev ) :
                mpOutDev( &rOutDev ),
                mbMappingWasEnabled( mpOutDev->IsMapModeEnabled() ),
                mnAntiAliasing( mpOutDev->GetAntialiasing() )
            {
                init();
            }

            explicit OutDevStateKeeper( const OutDevProviderSharedPtr& rOutDev ) :
                mpOutDev( rOutDev ? &(rOutDev->getOutDev()) : nullptr ),
                mbMappingWasEnabled( mpOutDev && mpOutDev->IsMapModeEnabled() ),
                mnAntiAliasing( mpOutDev ? mpOutDev->GetAntialiasing() : AntialiasingFlags::NONE )
            {
                init();
            }

            ~OutDevStateKeeper()
            {
                if( mpOutDev )
                {
                    mpOutDev->EnableMapMode( mbMappingWasEnabled );
                    mpOutDev->SetAntialiasing( mnAntiAliasing );

                    mpOutDev->Pop();
                }
            }

        private:
            void init()
            {
                if( mpOutDev )
                {
                    mpOutDev->Push();
                    mpOutDev->EnableMapMode(false);
                    mpOutDev->SetAntialiasing( AntialiasingFlags::Enable );
                }
            }

            VclPtr<OutputDevice>    mpOutDev;
            const bool              mbMappingWasEnabled;
            const AntialiasingFlags mnAntiAliasing;
        };

        ::Point mapRealPoint2D( const css::geometry::RealPoint2D&  rPoint,
                                const css::rendering::ViewState&   rViewState,
                                const css::rendering::RenderState& rRenderState );

        ::tools::PolyPolygon mapPolyPolygon( const ::basegfx::B2DPolyPolygon&                          rPoly,
                                      const css::rendering::ViewState&     rViewState,
                                      const css::rendering::RenderState&   rRenderState );

        ::BitmapEx transformBitmap( const BitmapEx&                                     rBitmap,
                                    const ::basegfx::B2DHomMatrix&                      rTransform );

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
