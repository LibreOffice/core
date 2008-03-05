/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewinformation2d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:42 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

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

#ifndef _COM_SUN_STAR_GEOMETRY_AFFINEMATRIX2D_HPP_
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#endif

#ifndef _COM_SUN_STAR_GEOMETRY_REALRECTANGLE2D_HPP__
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

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
            // the view transformation and the on-demand inverse of it
            basegfx::B2DHomMatrix                       maViewTransformation;
            basegfx::B2DHomMatrix                       maInverseViewTransformation;

            // the visible range and the on-demand one in ViewCoordinates
            basegfx::B2DRange                           maViewport;
            basegfx::B2DRange                           maDiscreteViewport;

            // the DrawPage which is target of visualisation. This is needed e.g. for
            // the view-dependent decomposition of PageNumber TextFields
            uno::Reference< drawing::XDrawPage > mxVisualizedPage;

            // the point in time
            double                                      mfViewTime;

            // the complete PropertyValue representation (if already created)
            uno::Sequence< beans::PropertyValue >       mxViewInformation;

            // the extra PropertyValues; not represented by ViewTransformation,
            // Viewport or ViewTime
            uno::Sequence< beans::PropertyValue >       mxExtendedInformation;

            // bitfield
            unsigned                                    mbInverseValid : 1;
            unsigned                                    mbDiscreteViewportValid : 1;

            // the local UNO API strings
            const ::rtl::OUString& getNamePropertyTransformation()
            {
                static ::rtl::OUString s_sNamePropertyTransformation(RTL_CONSTASCII_USTRINGPARAM("Transformation"));
                return s_sNamePropertyTransformation;
            }

            const ::rtl::OUString& getNamePropertyViewport()
            {
                static ::rtl::OUString s_sNamePropertyViewport(RTL_CONSTASCII_USTRINGPARAM("Viewport"));
                return s_sNamePropertyViewport;
            }

            const ::rtl::OUString& getNamePropertyTime()
            {
                static ::rtl::OUString s_sNamePropertyTime(RTL_CONSTASCII_USTRINGPARAM("Time"));
                return s_sNamePropertyTime;
            }

            const ::rtl::OUString& getNamePropertyVisualizedPage()
            {
                static ::rtl::OUString s_sNamePropertyXDrawPage(RTL_CONSTASCII_USTRINGPARAM("VisualizedPage"));
                return s_sNamePropertyXDrawPage;
            }

            void impInterpretPropertyValues(const uno::Sequence< beans::PropertyValue >& rViewParameters)
            {
                if(rViewParameters.hasElements())
                {
                    const sal_Int32 nCount(rViewParameters.getLength());
                    sal_Int32 nExtendedInsert(0);

                    // prepare extended information for filtering. Maximum size is nCount
                    mxExtendedInformation.realloc(nCount);

                    for(sal_Int32 a(0); a < nCount; a++)
                    {
                        const beans::PropertyValue& rProp = rViewParameters[a];

                        if(rProp.Name == getNamePropertyTransformation())
                        {
                            com::sun::star::geometry::AffineMatrix2D aAffineMatrix2D;
                            rProp.Value >>= aAffineMatrix2D;
                            basegfx::unotools::homMatrixFromAffineMatrix(maViewTransformation, aAffineMatrix2D);
                        }
                        else if(rProp.Name == getNamePropertyViewport())
                        {
                            com::sun::star::geometry::RealRectangle2D aViewport;
                            rProp.Value >>= aViewport;
                            maViewport = basegfx::unotools::b2DRectangleFromRealRectangle2D(aViewport);
                        }
                        else if(rProp.Name == getNamePropertyTime())
                        {
                            rProp.Value >>= mfViewTime;
                        }
                        else if(rProp.Name == getNamePropertyVisualizedPage())
                        {
                            rProp.Value >>= mxVisualizedPage;
                        }
                        else
                        {
                            // extra information; add to filtered information
                            mxExtendedInformation[nExtendedInsert++] = rProp;
                        }
                    }

                    // extra information size is now known; realloc to final size
                    mxExtendedInformation.realloc(nExtendedInsert);
                }
            }

            void impFillViewInformationFromContent()
            {
                uno::Sequence< beans::PropertyValue > xRetval;
                const bool bViewTransformationUsed(!maViewTransformation.isIdentity());
                const bool bViewportUsed(!maViewport.isEmpty());
                const bool bTimeUsed(0.0 < mfViewTime);
                const bool bVisualizedPageUsed(mxVisualizedPage.is());
                const bool bExtraInformation(mxExtendedInformation.hasElements());
                sal_uInt32 nIndex(0);
                const sal_uInt32 nCount(
                    (bViewTransformationUsed ? 1 : 0) +
                    (bViewportUsed ? 1 : 0) +
                    (bTimeUsed ? 1 : 0) +
                    (bVisualizedPageUsed ? 1 : 0) +
                    (bExtraInformation ? mxExtendedInformation.getLength() : 0));

                mxViewInformation.realloc(nCount);

                if(bViewTransformationUsed)
                {
                    com::sun::star::geometry::AffineMatrix2D aAffineMatrix2D;
                    basegfx::unotools::affineMatrixFromHomMatrix(aAffineMatrix2D, maViewTransformation);
                    mxViewInformation[nIndex].Name = getNamePropertyTransformation();
                    mxViewInformation[nIndex].Value <<= aAffineMatrix2D;
                    nIndex++;
                }

                if(bViewportUsed)
                {
                    const com::sun::star::geometry::RealRectangle2D aViewport(basegfx::unotools::rectangle2DFromB2DRectangle(maViewport));
                    mxViewInformation[nIndex].Name = getNamePropertyViewport();
                    mxViewInformation[nIndex].Value <<= aViewport;
                    nIndex++;
                }

                if(bTimeUsed)
                {
                    mxViewInformation[nIndex].Name = getNamePropertyTime();
                    mxViewInformation[nIndex].Value <<= mfViewTime;
                    nIndex++;
                }

                if(bVisualizedPageUsed)
                {
                    mxViewInformation[nIndex].Name = getNamePropertyVisualizedPage();
                    mxViewInformation[nIndex].Value <<= mxVisualizedPage;
                    nIndex++;
                }

                if(bExtraInformation)
                {
                    const sal_Int32 nExtra(mxExtendedInformation.getLength());

                    for(sal_Int32 a(0); a < nExtra; a++)
                    {
                        mxViewInformation[nIndex++] = mxExtendedInformation[a];
                    }
                }
            }

        public:
            ImpViewInformation2D(
                const basegfx::B2DHomMatrix& rViewTransformation,
                const basegfx::B2DRange& rViewport,
                const uno::Reference< drawing::XDrawPage >& rxDrawPage,
                double fViewTime,
                const uno::Sequence< beans::PropertyValue >& rExtendedParameters)
            :   mnRefCount(0),
                maViewTransformation(rViewTransformation),
                maInverseViewTransformation(maViewTransformation),
                maViewport(rViewport),
                maDiscreteViewport(),
                mxVisualizedPage(rxDrawPage),
                mfViewTime(fViewTime),
                mxViewInformation(),
                mxExtendedInformation(),
                mbInverseValid(false),
                mbDiscreteViewportValid(false)
            {
                impInterpretPropertyValues(rExtendedParameters);
            }

            ImpViewInformation2D(const uno::Sequence< beans::PropertyValue >& rViewParameters)
            :   mnRefCount(0),
                maViewTransformation(),
                maInverseViewTransformation(),
                maViewport(),
                maDiscreteViewport(),
                mxVisualizedPage(),
                mfViewTime(),
                mxViewInformation(rViewParameters),
                mxExtendedInformation(),
                mbInverseValid(false),
                mbDiscreteViewportValid(false)
            {
                impInterpretPropertyValues(rViewParameters);
            }

            const basegfx::B2DHomMatrix& getViewTransformation() const { return maViewTransformation; }

            const basegfx::B2DRange& getViewport() const { return maViewport; }

            const basegfx::B2DRange& getDiscreteViewport() const
            {
                ::osl::Mutex m_mutex;

                if(!mbDiscreteViewportValid)
                {
                    if(!maViewport.isEmpty())
                    {
                        basegfx::B2DRange aDiscreteViewport(maViewport);
                        aDiscreteViewport.transform(getViewTransformation());
                        const_cast< ImpViewInformation2D* >(this)->maDiscreteViewport = aDiscreteViewport;
                    }

                    const_cast< ImpViewInformation2D* >(this)->mbDiscreteViewportValid = true;
                }

                return maDiscreteViewport;
            }

            const basegfx::B2DHomMatrix& getInverseViewTransformation() const
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

            const uno::Reference< drawing::XDrawPage >& getVisualizedPage() const
            {
                return mxVisualizedPage;
            }

            const uno::Sequence< beans::PropertyValue >& getViewInformationSequence() const
            {
                if(!mxViewInformation.hasElements())
                {
                    const_cast< ImpViewInformation2D* >(this)->impFillViewInformationFromContent();
                }

                return mxViewInformation;
            }

            const uno::Sequence< beans::PropertyValue >& getExtendedInformationSequence() const
            {
                return mxExtendedInformation;
            }
        };
    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace geometry
    {
        ViewInformation2D::ViewInformation2D(
            const basegfx::B2DHomMatrix& rViewTransformation,
            const basegfx::B2DRange& rViewport,
            const uno::Reference< drawing::XDrawPage >& rxDrawPage,
            double fViewTime,
            const uno::Sequence< beans::PropertyValue >& rExtendedParameters)
        :   mpViewInformation2D(new ImpViewInformation2D(rViewTransformation, rViewport, rxDrawPage, fViewTime, rExtendedParameters))
        {
        }

        ViewInformation2D::ViewInformation2D(const uno::Sequence< beans::PropertyValue >& rViewParameters)
        :   mpViewInformation2D(new ImpViewInformation2D(rViewParameters))
        {
        }

        ViewInformation2D::ViewInformation2D(const ViewInformation2D& rCandidate)
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

            return *this;
        }

        const basegfx::B2DHomMatrix& ViewInformation2D::getViewTransformation() const
        {
            return mpViewInformation2D->getViewTransformation();
        }

        const basegfx::B2DRange& ViewInformation2D::getViewport() const
        {
            return mpViewInformation2D->getViewport();
        }

        const basegfx::B2DRange& ViewInformation2D::getDiscreteViewport() const
        {
            return mpViewInformation2D->getDiscreteViewport();
        }

        const basegfx::B2DHomMatrix& ViewInformation2D::getInverseViewTransformation() const
        {
            return mpViewInformation2D->getInverseViewTransformation();
        }

        double ViewInformation2D::getViewTime() const
        {
            return mpViewInformation2D->getViewTime();
        }

        const uno::Reference< drawing::XDrawPage >& ViewInformation2D::getVisualizedPage() const
        {
            return mpViewInformation2D->getVisualizedPage();
        }

        const uno::Sequence< beans::PropertyValue >& ViewInformation2D::getViewInformationSequence() const
        {
            return mpViewInformation2D->getViewInformationSequence();
        }

        const uno::Sequence< beans::PropertyValue >& ViewInformation2D::getExtendedInformationSequence() const
        {
            return mpViewInformation2D->getExtendedInformationSequence();
        }
    } // end of namespace geometry
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
