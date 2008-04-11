/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impltools.hxx,v $
 * $Revision: 1.8 $
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

#ifndef _VCLCANVAS_TOOLS_HXX
#define _VCLCANVAS_TOOLS_HXX

#include <osl/mutex.hxx>
#include <vos/mutex.hxx>

#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <canvas/vclwrapper.hxx>
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
    namespace vector
    {
        class B2DVector;
    }
    namespace point
    {
        class B2DPoint;
    }
}

namespace com { namespace sun { namespace star { namespace awt
{
    struct Point;
    struct Size;
    struct Rectangle;
} } } }

namespace com { namespace sun { namespace star { namespace drawing
{
    struct HomogenMatrix3;
} } } }

namespace com { namespace sun { namespace star { namespace geometry
{
    struct RealPoint2D;
    struct RealSize2D;
    struct RealRectangle2D;
} } } }

namespace com { namespace sun { namespace star { namespace rendering
{
    struct RenderState;
    struct ViewState;
    class  XCanvas;
    class  XBitmap;
    class  XPolyPolygon2D;
} } } }


namespace vclcanvas
{
    namespace tools
    {
        ::BitmapEx
        bitmapExFromXBitmap( const ::com::sun::star::uno::Reference<
                             ::com::sun::star::rendering::XBitmap >& );

        /** Setup VCL font and output position

            @returns false, if no text output should happen
         */
        bool setupFontTransform( ::Point&                                                   o_rPoint,
                                 ::Font&                                                    io_rVCLFont,
                                 const ::com::sun::star::rendering::ViewState&      viewState,
                                 const ::com::sun::star::rendering::RenderState&    renderState,
                                 ::OutputDevice&                                            rOutDev );

        /** Predicate, to determine whether polygon is actually an axis-aligned rectangle

            @return true, if the polygon is a rectangle.
         */
        bool isRectangle( const PolyPolygon& rPolyPoly );


        // Little helper to encapsulate locking into policy class
        class LocalGuard
        {
        public:
            LocalGuard() :
                aGuard( Application::GetSolarMutex() )
            {
            }

            /// To be compatible with CanvasBase mutex concept
            LocalGuard( const ::osl::Mutex& ) :
                aGuard( Application::GetSolarMutex() )
            {
            }

        private:
            ::vos::OGuard aGuard;
        };

        class OutDevStateKeeper
        {
        public:
            explicit OutDevStateKeeper( OutputDevice& rOutDev ) :
                mpOutDev( &rOutDev ),
                mbMappingWasEnabled( mpOutDev->IsMapModeEnabled() )
            {
                init();
            }

            explicit OutDevStateKeeper( const OutDevProviderSharedPtr& rOutDev ) :
                mpOutDev( rOutDev.get() ? &(rOutDev->getOutDev()) : NULL ),
                mbMappingWasEnabled( mpOutDev ? mpOutDev->IsMapModeEnabled() : false )
            {
                init();
            }

            ~OutDevStateKeeper()
            {
                if( mpOutDev )
                {
                    mpOutDev->EnableMapMode( mbMappingWasEnabled );
                    mpOutDev->Pop();
                }
            }

        private:
            void init()
            {
                if( mpOutDev )
                {
                    mpOutDev->Push();
                    mpOutDev->EnableMapMode(FALSE);
                }
            }

            OutputDevice*   mpOutDev;
            const bool      mbMappingWasEnabled;
        };

        ::Point mapRealPoint2D( const ::com::sun::star::geometry::RealPoint2D&  rPoint,
                                const ::com::sun::star::rendering::ViewState&   rViewState,
                                const ::com::sun::star::rendering::RenderState& rRenderState );

        ::PolyPolygon mapPolyPolygon( const ::basegfx::B2DPolyPolygon&                          rPoly,
                                      const ::com::sun::star::rendering::ViewState&     rViewState,
                                      const ::com::sun::star::rendering::RenderState&   rRenderState );

        enum ModulationMode
        {
            MODULATE_NONE,
            MODULATE_WITH_DEVICECOLOR
        };

        ::BitmapEx transformBitmap( const BitmapEx&                                     rBitmap,
                                    const ::basegfx::B2DHomMatrix&                      rTransform,
                                    const ::com::sun::star::uno::Sequence< double >&    rDeviceColor,
                                    ModulationMode                                      eModulationMode );

    }
}

#endif /* _VCLCANVAS_TOOLS_HXX */
