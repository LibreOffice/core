/*************************************************************************
 *
 *  $RCSfile: impltools.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 17:13:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _VCLCANVAS_TOOLS_HXX
#define _VCLCANVAS_TOOLS_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

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

namespace drafts { namespace com { namespace sun { namespace star { namespace geometry
{
    struct RealPoint2D;
    struct RealSize2D;
    struct RealRectangle2D;
} } } } }

namespace drafts { namespace com { namespace sun { namespace star { namespace rendering
{
    struct RenderState;
    struct ViewState;
    class  XCanvas;
    class  XBitmap;
    class  XPolyPolygon2D;
} } } } }


namespace vclcanvas
{
    namespace tools
    {
        ::basegfx::B2DPolyPolygon
        polyPolygonFromXPolyPolygon2D( const ::com::sun::star::uno::Reference<
                                       ::drafts::com::sun::star::rendering::XPolyPolygon2D >& );

        ::BitmapEx
        bitmapExFromXBitmap( const ::com::sun::star::uno::Reference<
                             ::drafts::com::sun::star::rendering::XBitmap >& );

        /** Setup VCL font and output position

            @returns false, if no text output should happen
         */
        bool setupFontTransform( ::Point&                                                   o_rPoint,
                                 ::Font&                                                    io_rVCLFont,
                                 const ::drafts::com::sun::star::rendering::ViewState&      viewState,
                                 const ::drafts::com::sun::star::rendering::RenderState&    renderState,
                                 ::OutputDevice&                                            rOutDev );


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
            typedef ::boost::shared_ptr< canvas::vcltools::VCLObject< OutputDevice > >  OutputDeviceSharedPtr;

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

        ::Point mapRealPoint2D( const ::drafts::com::sun::star::geometry::RealPoint2D&  rPoint,
                                const ::drafts::com::sun::star::rendering::ViewState&   rViewState,
                                const ::drafts::com::sun::star::rendering::RenderState& rRenderState );

        ::PolyPolygon mapPolyPolygon( const ::basegfx::B2DPolyPolygon&                          rPoly,
                                      const ::drafts::com::sun::star::rendering::ViewState&     rViewState,
                                      const ::drafts::com::sun::star::rendering::RenderState&   rRenderState );

        enum ModulationMode
        {
            MODULATE_NONE,
            MODULATE_WITH_DEVICECOLOR
        };

        ::BitmapEx transformBitmap( const BitmapEx&                                         rBitmap,
                                    const ::drafts::com::sun::star::rendering::ViewState&   rViewState,
                                    const ::drafts::com::sun::star::rendering::RenderState& rRenderState,
                                    ModulationMode                                          eModulationMode=MODULATE_NONE );

    }
}

#endif /* _VCLCANVAS_TOOLS_HXX */
