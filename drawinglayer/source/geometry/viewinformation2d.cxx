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

#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <rtl/instance.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace geometry
    {
        class ImpViewInformation2D
        {
        private:
            // ViewInformation2D implementation can change refcount, so we have only
            // two memory regions for pairs of ViewInformation2D/ImpViewInformation2D
            friend class ::drawinglayer::geometry::ViewInformation2D;

        protected:
            // the object transformation
            basegfx::B2DHomMatrix                       maObjectTransformation;

            // the view transformation
            basegfx::B2DHomMatrix                       maViewTransformation;

            // the ObjectToView and it's inverse, both on demand from ObjectTransformation
            // and ViewTransformation
            basegfx::B2DHomMatrix                       maObjectToViewTransformation;
            basegfx::B2DHomMatrix                       maInverseObjectToViewTransformation;

            // the visible range and the on-demand one in ViewCoordinates
            basegfx::B2DRange                           maViewport;
            basegfx::B2DRange                           maDiscreteViewport;

            // the DrawPage which is target of visualisation. This is needed e.g. for
            // the view-dependent decomposition of PageNumber TextFields.
            // This parameter is buffered here, but mainly resides in mxExtendedInformation,
            // so it will be interpreted, but held there. It will also not be added
            // to mxExtendedInformation in impFillViewInformationFromContent (it's there already)
            uno::Reference< drawing::XDrawPage >        mxVisualizedPage;

            // the point in time
            double                                      mfViewTime;

            // bitfield
            bool                                        mbReducedDisplayQuality : 1;

            // the complete PropertyValue representation (if already created)
            uno::Sequence< beans::PropertyValue >       mxViewInformation;

            // the extra PropertyValues; not represented by ViewTransformation,
            // Viewport, VisualizedPage or ViewTime
            uno::Sequence< beans::PropertyValue >       mxExtendedInformation;

            // the local UNO API strings
            const OUString& getNamePropertyObjectTransformation()
            {
                static OUString s_sNameProperty("ObjectTransformation");
                return s_sNameProperty;
            }

            const OUString& getNamePropertyViewTransformation()
            {
                static OUString s_sNameProperty("ViewTransformation");
                return s_sNameProperty;
            }

            const OUString& getNamePropertyViewport()
            {
                static OUString s_sNameProperty("Viewport");
                return s_sNameProperty;
            }

            const OUString& getNamePropertyTime()
            {
                static OUString s_sNameProperty("Time");
                return s_sNameProperty;
            }

            const OUString& getNamePropertyVisualizedPage()
            {
                static OUString s_sNameProperty("VisualizedPage");
                return s_sNameProperty;
            }

            const OUString& getNamePropertyReducedDisplayQuality()
            {
                static OUString s_sNameProperty("ReducedDisplayQuality");
                return s_sNameProperty;
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

                        if(rProp.Name == getNamePropertyReducedDisplayQuality())
                        {
                            // extra information; add to filtered information
                            mxExtendedInformation[nExtendedInsert++] = rProp;

                            // for performance reasons, also cache content locally
                            sal_Bool bSalBool(false);
                            rProp.Value >>= bSalBool;
                            mbReducedDisplayQuality = bSalBool;
                        }
                        else if(rProp.Name == getNamePropertyObjectTransformation())
                        {
                            com::sun::star::geometry::AffineMatrix2D aAffineMatrix2D;
                            rProp.Value >>= aAffineMatrix2D;
                            basegfx::unotools::homMatrixFromAffineMatrix(maObjectTransformation, aAffineMatrix2D);
                        }
                        else if(rProp.Name == getNamePropertyViewTransformation())
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
                const bool bObjectTransformationUsed(!maObjectTransformation.isIdentity());
                const bool bViewTransformationUsed(!maViewTransformation.isIdentity());
                const bool bViewportUsed(!maViewport.isEmpty());
                const bool bTimeUsed(0.0 < mfViewTime);
                const bool bVisualizedPageUsed(mxVisualizedPage.is());
                const bool bReducedDisplayQualityUsed(true == mbReducedDisplayQuality);
                const bool bExtraInformation(mxExtendedInformation.hasElements());
                sal_uInt32 nIndex(0);
                const sal_uInt32 nCount(
                    (bObjectTransformationUsed ? 1 : 0) +
                    (bViewTransformationUsed ? 1 : 0) +
                    (bViewportUsed ? 1 : 0) +
                    (bTimeUsed ? 1 : 0) +
                    (bVisualizedPageUsed ? 1 : 0) +
                    (bReducedDisplayQualityUsed ? 1 : 0) +
                    (bExtraInformation ? mxExtendedInformation.getLength() : 0));

                mxViewInformation.realloc(nCount);

                if(bObjectTransformationUsed)
                {
                    com::sun::star::geometry::AffineMatrix2D aAffineMatrix2D;
                    basegfx::unotools::affineMatrixFromHomMatrix(aAffineMatrix2D, maObjectTransformation);
                    mxViewInformation[nIndex].Name = getNamePropertyObjectTransformation();
                    mxViewInformation[nIndex].Value <<= aAffineMatrix2D;
                    nIndex++;
                }

                if(bViewTransformationUsed)
                {
                    com::sun::star::geometry::AffineMatrix2D aAffineMatrix2D;
                    basegfx::unotools::affineMatrixFromHomMatrix(aAffineMatrix2D, maViewTransformation);
                    mxViewInformation[nIndex].Name = getNamePropertyViewTransformation();
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
                const basegfx::B2DHomMatrix& rObjectTransformation,
                const basegfx::B2DHomMatrix& rViewTransformation,
                const basegfx::B2DRange& rViewport,
                const uno::Reference< drawing::XDrawPage >& rxDrawPage,
                double fViewTime,
                const uno::Sequence< beans::PropertyValue >& rExtendedParameters)
            :   maObjectTransformation(rObjectTransformation),
                maViewTransformation(rViewTransformation),
                maObjectToViewTransformation(),
                maInverseObjectToViewTransformation(),
                maViewport(rViewport),
                maDiscreteViewport(),
                mxVisualizedPage(rxDrawPage),
                mfViewTime(fViewTime),
                mbReducedDisplayQuality(false),
                mxViewInformation(),
                mxExtendedInformation()
            {
                impInterpretPropertyValues(rExtendedParameters);
            }

            explicit ImpViewInformation2D(const uno::Sequence< beans::PropertyValue >& rViewParameters)
            :   maObjectTransformation(),
                maViewTransformation(),
                maObjectToViewTransformation(),
                maInverseObjectToViewTransformation(),
                maViewport(),
                maDiscreteViewport(),
                mxVisualizedPage(),
                mfViewTime(),
                mbReducedDisplayQuality(false),
                mxViewInformation(rViewParameters),
                mxExtendedInformation()
            {
                impInterpretPropertyValues(rViewParameters);
            }

            ImpViewInformation2D()
            :   maObjectTransformation(),
                maViewTransformation(),
                maObjectToViewTransformation(),
                maInverseObjectToViewTransformation(),
                maViewport(),
                maDiscreteViewport(),
                mxVisualizedPage(),
                mfViewTime(),
                mbReducedDisplayQuality(false),
                mxViewInformation(),
                mxExtendedInformation()
            {
            }

            const basegfx::B2DHomMatrix& getObjectTransformation() const
            {
                return maObjectTransformation;
            }

            const basegfx::B2DHomMatrix& getViewTransformation() const
            {
                return maViewTransformation;
            }

            const basegfx::B2DRange& getViewport() const
            {
                return maViewport;
            }

            const basegfx::B2DRange& getDiscreteViewport() const
            {
                if(maDiscreteViewport.isEmpty() && !maViewport.isEmpty())
                {
                    basegfx::B2DRange aDiscreteViewport(maViewport);
                    aDiscreteViewport.transform(getViewTransformation());
                    const_cast< ImpViewInformation2D* >(this)->maDiscreteViewport = aDiscreteViewport;
                }

                return maDiscreteViewport;
            }

            const basegfx::B2DHomMatrix& getObjectToViewTransformation() const
            {
                if(maObjectToViewTransformation.isIdentity() &&
                    (!maObjectTransformation.isIdentity() || !maViewTransformation.isIdentity()))
                {
                    basegfx::B2DHomMatrix aObjectToView(maViewTransformation * maObjectTransformation);
                    const_cast< ImpViewInformation2D* >(this)->maObjectToViewTransformation = aObjectToView;
                }

                return maObjectToViewTransformation;
            }

            const basegfx::B2DHomMatrix& getInverseObjectToViewTransformation() const
            {
                if(maInverseObjectToViewTransformation.isIdentity() &&
                    (!maObjectTransformation.isIdentity() || !maViewTransformation.isIdentity()))
                {
                    basegfx::B2DHomMatrix aInverseObjectToView(maViewTransformation * maObjectTransformation);
                    aInverseObjectToView.invert();
                    const_cast< ImpViewInformation2D* >(this)->maInverseObjectToViewTransformation = aInverseObjectToView;
                }

                return maInverseObjectToViewTransformation;
            }

            double getViewTime() const
            {
                return mfViewTime;
            }

            const uno::Reference< drawing::XDrawPage >& getVisualizedPage() const
            {
                return mxVisualizedPage;
            }

            bool getReducedDisplayQuality() const
            {
                return mbReducedDisplayQuality;
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

            bool operator==(const ImpViewInformation2D& rCandidate) const
            {
                return (maObjectTransformation == rCandidate.maObjectTransformation
                    && maViewTransformation == rCandidate.maViewTransformation
                    && maViewport == rCandidate.maViewport
                    && mxVisualizedPage == rCandidate.mxVisualizedPage
                    && mfViewTime == rCandidate.mfViewTime
                    && mxExtendedInformation == rCandidate.mxExtendedInformation);
            }
        };
    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace geometry
    {
        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< ViewInformation2D::ImplType, theGlobalDefault > {};
        }

        ViewInformation2D::ViewInformation2D(
            const basegfx::B2DHomMatrix& rObjectTransformation,
            const basegfx::B2DHomMatrix& rViewTransformation,
            const basegfx::B2DRange& rViewport,
            const uno::Reference< drawing::XDrawPage >& rxDrawPage,
            double fViewTime,
            const uno::Sequence< beans::PropertyValue >& rExtendedParameters)
        :   mpViewInformation2D(ImpViewInformation2D(
                rObjectTransformation,
                rViewTransformation,
                rViewport,
                rxDrawPage,
                fViewTime,
                rExtendedParameters))
        {
        }

        ViewInformation2D::ViewInformation2D(const uno::Sequence< beans::PropertyValue >& rViewParameters)
        :   mpViewInformation2D(ImpViewInformation2D(rViewParameters))
        {
        }

        ViewInformation2D::ViewInformation2D()
        :   mpViewInformation2D(theGlobalDefault::get())
        {
        }

        ViewInformation2D::ViewInformation2D(const ViewInformation2D& rCandidate)
        :   mpViewInformation2D(rCandidate.mpViewInformation2D)
        {
        }

        ViewInformation2D::~ViewInformation2D()
        {
        }

        ViewInformation2D& ViewInformation2D::operator=(const ViewInformation2D& rCandidate)
        {
            mpViewInformation2D = rCandidate.mpViewInformation2D;
            return *this;
        }

        bool ViewInformation2D::operator==(const ViewInformation2D& rCandidate) const
        {
            return rCandidate.mpViewInformation2D == mpViewInformation2D;
        }

        const basegfx::B2DHomMatrix& ViewInformation2D::getObjectTransformation() const
        {
            return mpViewInformation2D->getObjectTransformation();
        }

        const basegfx::B2DHomMatrix& ViewInformation2D::getViewTransformation() const
        {
            return mpViewInformation2D->getViewTransformation();
        }

        const basegfx::B2DRange& ViewInformation2D::getViewport() const
        {
            return mpViewInformation2D->getViewport();
        }

        double ViewInformation2D::getViewTime() const
        {
            return mpViewInformation2D->getViewTime();
        }

        const uno::Reference< drawing::XDrawPage >& ViewInformation2D::getVisualizedPage() const
        {
            return mpViewInformation2D->getVisualizedPage();
        }

        const basegfx::B2DHomMatrix& ViewInformation2D::getObjectToViewTransformation() const
        {
            return mpViewInformation2D->getObjectToViewTransformation();
        }

        const basegfx::B2DHomMatrix& ViewInformation2D::getInverseObjectToViewTransformation() const
        {
            return mpViewInformation2D->getInverseObjectToViewTransformation();
        }

        const basegfx::B2DRange& ViewInformation2D::getDiscreteViewport() const
        {
            return mpViewInformation2D->getDiscreteViewport();
        }

        bool ViewInformation2D::getReducedDisplayQuality() const
        {
            return mpViewInformation2D->getReducedDisplayQuality();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
