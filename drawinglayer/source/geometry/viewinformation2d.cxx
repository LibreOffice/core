/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewinformation2d.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:40:02 $
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

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef _COM_SUN_STAR_GRAPHIC_PRIMITIVE2DPARAMETERS_HPP_
#include "com/sun/star/graphic/Primitive2DParameters.hpp"
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace
    {
        class ImpViewInformation2D
        {
        private:
            // ViewInformation2D implementation can change refcount, so we have only
            // two memory regions for pairs of ViewInformation2D/ImpViewInformation2D
            friend class ::drawinglayer::geometry::ViewInformation2D;

            // the refcounter
            sal_uInt32                                  mnRefCount;

        protected:
            // the view transformation
            ::basegfx::B2DHomMatrix                     maViewTransformation;
            ::basegfx::B2DHomMatrix                     maInverseViewTransformation;

            // the visible ranges
            ::basegfx::B2DRange                         maViewport;
            ::basegfx::B2DRange                         maDiscreteViewport;

            // the point in time
            double                                      mfViewTime;

            // bitfield
            unsigned                                    mbInverseValid : 1;
            unsigned                                    mbDiscreteViewportValid : 1;

        public:
            ImpViewInformation2D(
                const ::basegfx::B2DHomMatrix& rViewTransformation,
                const ::basegfx::B2DRange& rViewport,
                double fViewTime)
            :   mnRefCount(0L),
                maViewTransformation(rViewTransformation),
                maInverseViewTransformation(maViewTransformation),
                maViewport(rViewport),
                maDiscreteViewport(),
                mfViewTime(fViewTime),
                mbInverseValid(false),
                mbDiscreteViewportValid(false)
            {
            }

            ImpViewInformation2D(
                const ::com::sun::star::graphic::Primitive2DParameters& rParams)
            :   mnRefCount(0L),
                maViewTransformation(basegfx::unotools::homMatrixFromAffineMatrix(rParams.ViewTransformation)),
                maInverseViewTransformation(maViewTransformation),
                maViewport(basegfx::unotools::b2DRectangleFromRealRectangle2D(rParams.Viewport)),
                maDiscreteViewport(),
                mfViewTime(rParams.Time),
                mbInverseValid(false),
                mbDiscreteViewportValid(false)
            {
            }

            bool operator==(const ImpViewInformation2D& rCandidate) const
            {
                return (maViewTransformation == rCandidate.maViewTransformation
                    && mfViewTime == rCandidate.mfViewTime
                    && maViewport == rCandidate.maViewport);
            }

            const ::basegfx::B2DHomMatrix& getViewTransformation() const { return maViewTransformation; }

            const ::basegfx::B2DRange& getViewport() const { return maViewport; }

            const ::basegfx::B2DRange& getDiscreteViewport() const
            {
                ::osl::Mutex m_mutex;

                if(!mbDiscreteViewportValid)
                {
                    ::basegfx::B2DRange aDiscreteViewport(maViewport);
                    aDiscreteViewport.transform(getViewTransformation());
                    const_cast< ImpViewInformation2D* >(this)->maDiscreteViewport = aDiscreteViewport;
                    const_cast< ImpViewInformation2D* >(this)->mbDiscreteViewportValid = true;
                }

                return maDiscreteViewport;
            }

            const ::basegfx::B2DHomMatrix& getInverseViewTransformation() const
            {
                ::osl::Mutex m_mutex;

                if(!mbInverseValid)
                {
                    const_cast< ImpViewInformation2D* >(this)->maInverseViewTransformation.invert();
                    const_cast< ImpViewInformation2D* >(this)->mbInverseValid = true;
                }

                return maInverseViewTransformation;
            }

            double getViewTime() const { return mfViewTime; }
        };
    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace geometry
    {
        ViewInformation2D::ViewInformation2D(
            const ::basegfx::B2DHomMatrix& rViewTransformation,
            const ::basegfx::B2DRange& rViewport,
            double fViewTime)
        :   mpViewInformation2D(new ImpViewInformation2D(rViewTransformation, rViewport, fViewTime))
        {
        }

        ViewInformation2D::ViewInformation2D(
            const ::com::sun::star::graphic::Primitive2DParameters& rParams)
        :   mpViewInformation2D(new ImpViewInformation2D(rParams))
        {
        }

        ViewInformation2D::ViewInformation2D(
            const ViewInformation2D& rCandidate)
        :   mpViewInformation2D(rCandidate.mpViewInformation2D)
        {
            ::osl::Mutex m_mutex;
            mpViewInformation2D->mnRefCount++;
        }

        ViewInformation2D::~ViewInformation2D()
        {
            ::osl::Mutex m_mutex;

            if(mpViewInformation2D->mnRefCount)
            {
                mpViewInformation2D->mnRefCount--;
            }
            else
            {
                delete mpViewInformation2D;
            }
        }

        ViewInformation2D& ViewInformation2D::operator=(const ViewInformation2D& rCandidate)
        {
            ::osl::Mutex m_mutex;

            if(rCandidate.mpViewInformation2D != mpViewInformation2D)
            {
                if(mpViewInformation2D->mnRefCount)
                {
                    mpViewInformation2D->mnRefCount--;
                }
                else
                {
                    delete mpViewInformation2D;
                }

                mpViewInformation2D = rCandidate.mpViewInformation2D;
                mpViewInformation2D->mnRefCount++;
            }

            return *this;
        }

        bool ViewInformation2D::operator==(const ViewInformation2D& rCandidate) const
        {
            if(rCandidate.mpViewInformation2D == mpViewInformation2D)
            {
                return true;
            }

            return (*rCandidate.mpViewInformation2D == *mpViewInformation2D);
        }

        const ::basegfx::B2DHomMatrix& ViewInformation2D::getViewTransformation() const
        {
            return mpViewInformation2D->getViewTransformation();
        }

        const ::basegfx::B2DRange& ViewInformation2D::getViewport() const
        {
            return mpViewInformation2D->getViewport();
        }

        const ::basegfx::B2DRange& ViewInformation2D::getDiscreteViewport() const
        {
            return mpViewInformation2D->getDiscreteViewport();
        }

        const ::basegfx::B2DHomMatrix& ViewInformation2D::getInverseViewTransformation() const
        {
            return mpViewInformation2D->getInverseViewTransformation();
        }

        double ViewInformation2D::getViewTime() const
        {
            return mpViewInformation2D->getViewTime();
        }
    } // end of namespace geometry
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
