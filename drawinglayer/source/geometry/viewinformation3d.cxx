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

#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <basegfx/range/b3drange.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <com/sun/star/geometry/AffineMatrix3D.hpp>
#include <com/sun/star/geometry/RealRectangle3D.hpp>
#include <basegfx/tools/canvastools.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace geometry
    {
        /** Implementation class for ViewInformation3D
        */
        class ImpViewInformation3D
        {
        private:
            // ViewInformation3D implementation can change refcount, so we have only
            // two memory regions for pairs of ViewInformation3D/ImpViewInformation3D
            friend class ::drawinglayer::geometry::ViewInformation3D;

            // the refcounter. 0 means exclusively used
            sal_uInt32                                  mnRefCount;

            // the 3D transformations
            // Object to World. This may change and being adapted when entering 3D transformation
            // groups
            basegfx::B3DHomMatrix                       maObjectTransformation;

            // World to Camera. This includes VRP, VPN and VUV camera coordinate system
            basegfx::B3DHomMatrix                       maOrientation;

            // Camera to Device with X,Y and Z [-1.0 .. 1.0]. This is the
            // 3D to 2D projection which may be parallell or perspective. When it is perspective,
            // the last line of the homogen matrix will NOT be unused
            basegfx::B3DHomMatrix                       maProjection;

            // Device to View with X,Y and Z [0.0 .. 1.0]. This converts from -1 to 1 coordinates
            // in camera coordinate system to 0 to 1 in unit 2D coordinates. This way it stays
            // view-independent. To get discrete coordinates, the 2D transformation of a scene
            // as 2D object needs to be involved
            basegfx::B3DHomMatrix                       maDeviceToView;

            // Object to View is the linear combination of all four transformations. It's
            // buffered to avoid too much matrix multiplying and created on demand
            basegfx::B3DHomMatrix                       maObjectToView;

            // the point in time
            double                                      mfViewTime;

            // the complete PropertyValue representation (if already created)
            uno::Sequence< beans::PropertyValue >       mxViewInformation;

            // the extra PropertyValues; does not contain the transformations
            uno::Sequence< beans::PropertyValue >       mxExtendedInformation;

            // the local UNO API strings
            const ::rtl::OUString& getNamePropertyObjectTransformation()
            {
                static ::rtl::OUString s_sNameProperty(RTL_CONSTASCII_USTRINGPARAM("ObjectTransformation"));
                return s_sNameProperty;
            }

            const ::rtl::OUString& getNamePropertyOrientation()
            {
                static ::rtl::OUString s_sNameProperty(RTL_CONSTASCII_USTRINGPARAM("Orientation"));
                return s_sNameProperty;
            }

            const ::rtl::OUString& getNamePropertyProjection()
            {
                static ::rtl::OUString s_sNameProperty(RTL_CONSTASCII_USTRINGPARAM("Projection"));
                return s_sNameProperty;
            }

            const ::rtl::OUString& getNamePropertyProjection_30()
            {
                static ::rtl::OUString s_sNameProperty(RTL_CONSTASCII_USTRINGPARAM("Projection30"));
                return s_sNameProperty;
            }

            const ::rtl::OUString& getNamePropertyProjection_31()
            {
                static ::rtl::OUString s_sNameProperty(RTL_CONSTASCII_USTRINGPARAM("Projection31"));
                return s_sNameProperty;
            }

            const ::rtl::OUString& getNamePropertyProjection_32()
            {
                static ::rtl::OUString s_sNameProperty(RTL_CONSTASCII_USTRINGPARAM("Projection32"));
                return s_sNameProperty;
            }

            const ::rtl::OUString& getNamePropertyProjection_33()
            {
                static ::rtl::OUString s_sNameProperty(RTL_CONSTASCII_USTRINGPARAM("Projection33"));
                return s_sNameProperty;
            }

            const ::rtl::OUString& getNamePropertyDeviceToView()
            {
                static ::rtl::OUString s_sNameProperty(RTL_CONSTASCII_USTRINGPARAM("DeviceToView"));
                return s_sNameProperty;
            }

            const ::rtl::OUString& getNamePropertyTime()
            {
                static ::rtl::OUString s_sNamePropertyTime(RTL_CONSTASCII_USTRINGPARAM("Time"));
                return s_sNamePropertyTime;
            }

            // a central PropertyValue parsing method to allow transportatin of
            // all ViewParameters using UNO API
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

                        if(rProp.Name == getNamePropertyObjectTransformation())
                        {
                            com::sun::star::geometry::AffineMatrix3D aAffineMatrix3D;
                            rProp.Value >>= aAffineMatrix3D;
                            maObjectTransformation = basegfx::unotools::homMatrixFromAffineMatrix3D(aAffineMatrix3D);
                        }
                        else if(rProp.Name == getNamePropertyOrientation())
                        {
                            com::sun::star::geometry::AffineMatrix3D aAffineMatrix3D;
                            rProp.Value >>= aAffineMatrix3D;
                            maOrientation = basegfx::unotools::homMatrixFromAffineMatrix3D(aAffineMatrix3D);
                        }
                        else if(rProp.Name == getNamePropertyProjection())
                        {
                            // projection may be defined using a frustum in which case the last line of
                            // the 4x4 matrix is not (0,0,0,1). Since AffineMatrix3D does not support that,
                            // these four values need to be treated extra
                            const double f_30(maProjection.get(3, 0));
                            const double f_31(maProjection.get(3, 1));
                            const double f_32(maProjection.get(3, 2));
                            const double f_33(maProjection.get(3, 3));

                            com::sun::star::geometry::AffineMatrix3D aAffineMatrix3D;
                            rProp.Value >>= aAffineMatrix3D;
                            maProjection = basegfx::unotools::homMatrixFromAffineMatrix3D(aAffineMatrix3D);

                            maProjection.set(3, 0, f_30);
                            maProjection.set(3, 1, f_31);
                            maProjection.set(3, 2, f_32);
                            maProjection.set(3, 3, f_33);
                        }
                        else if(rProp.Name == getNamePropertyProjection_30())
                        {
                            double f_30(0.0);
                            rProp.Value >>= f_30;
                            maProjection.set(3, 0, f_30);
                        }
                        else if(rProp.Name == getNamePropertyProjection_31())
                        {
                            double f_31(0.0);
                            rProp.Value >>= f_31;
                            maProjection.set(3, 1, f_31);
                        }
                        else if(rProp.Name == getNamePropertyProjection_32())
                        {
                            double f_32(0.0);
                            rProp.Value >>= f_32;
                            maProjection.set(3, 2, f_32);
                        }
                        else if(rProp.Name == getNamePropertyProjection_33())
                        {
                            double f_33(1.0);
                            rProp.Value >>= f_33;
                            maProjection.set(3, 3, f_33);
                        }
                        else if(rProp.Name == getNamePropertyDeviceToView())
                        {
                            com::sun::star::geometry::AffineMatrix3D aAffineMatrix3D;
                            rProp.Value >>= aAffineMatrix3D;
                            maDeviceToView = basegfx::unotools::homMatrixFromAffineMatrix3D(aAffineMatrix3D);
                        }
                        else if(rProp.Name == getNamePropertyTime())
                        {
                            rProp.Value >>= mfViewTime;
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

            // central method to create a Sequence of PropertyValues containing he complete
            // data set
            void impFillViewInformationFromContent()
            {
                uno::Sequence< beans::PropertyValue > xRetval;
                const bool bObjectTransformationUsed(!maObjectTransformation.isIdentity());
                const bool bOrientationUsed(!maOrientation.isIdentity());
                const bool bProjectionUsed(!maProjection.isIdentity());
                const bool bDeviceToViewUsed(!maDeviceToView.isIdentity());
                const bool bTimeUsed(0.0 < mfViewTime);
                const bool bExtraInformation(mxExtendedInformation.hasElements());

                // projection may be defined using a frustum in which case the last line of
                // the 4x4 matrix is not (0,0,0,1). Since AffineMatrix3D does not support that,
                // these four values need to be treated extra
                const bool bProjectionUsed_30(bProjectionUsed && !basegfx::fTools::equalZero(maProjection.get(3, 0)));
                const bool bProjectionUsed_31(bProjectionUsed && !basegfx::fTools::equalZero(maProjection.get(3, 1)));
                const bool bProjectionUsed_32(bProjectionUsed && !basegfx::fTools::equalZero(maProjection.get(3, 2)));
                const bool bProjectionUsed_33(bProjectionUsed && !basegfx::fTools::equal(maProjection.get(3, 3), 1.0));

                sal_uInt32 nIndex(0);
                const sal_uInt32 nCount(
                    (bObjectTransformationUsed ? 1 : 0) +
                    (bOrientationUsed ? 1 : 0) +
                    (bProjectionUsed ? 1 : 0) +
                    (bProjectionUsed_30 ? 1 : 0) +
                    (bProjectionUsed_31 ? 1 : 0) +
                    (bProjectionUsed_32 ? 1 : 0) +
                    (bProjectionUsed_33 ? 1 : 0) +
                    (bDeviceToViewUsed ? 1 : 0) +
                    (bTimeUsed ? 1 : 0) +
                    (bExtraInformation ? mxExtendedInformation.getLength() : 0));

                mxViewInformation.realloc(nCount);

                if(bObjectTransformationUsed)
                {
                    com::sun::star::geometry::AffineMatrix3D aAffineMatrix3D;
                    basegfx::unotools::affineMatrixFromHomMatrix3D(aAffineMatrix3D, maObjectTransformation);
                    mxViewInformation[nIndex].Name = getNamePropertyObjectTransformation();
                    mxViewInformation[nIndex].Value <<= aAffineMatrix3D;
                    nIndex++;
                }

                if(bOrientationUsed)
                {
                    com::sun::star::geometry::AffineMatrix3D aAffineMatrix3D;
                    basegfx::unotools::affineMatrixFromHomMatrix3D(aAffineMatrix3D, maOrientation);
                    mxViewInformation[nIndex].Name = getNamePropertyOrientation();
                    mxViewInformation[nIndex].Value <<= aAffineMatrix3D;
                    nIndex++;
                }

                if(bProjectionUsed)
                {
                    com::sun::star::geometry::AffineMatrix3D aAffineMatrix3D;
                    basegfx::unotools::affineMatrixFromHomMatrix3D(aAffineMatrix3D, maProjection);
                    mxViewInformation[nIndex].Name = getNamePropertyProjection();
                    mxViewInformation[nIndex].Value <<= aAffineMatrix3D;
                    nIndex++;
                }

                if(bProjectionUsed_30)
                {
                    mxViewInformation[nIndex].Name = getNamePropertyProjection_30();
                    mxViewInformation[nIndex].Value <<= maProjection.get(3, 0);
                    nIndex++;
                }

                if(bProjectionUsed_31)
                {
                    mxViewInformation[nIndex].Name = getNamePropertyProjection_31();
                    mxViewInformation[nIndex].Value <<= maProjection.get(3, 1);
                    nIndex++;
                }

                if(bProjectionUsed_32)
                {
                    mxViewInformation[nIndex].Name = getNamePropertyProjection_32();
                    mxViewInformation[nIndex].Value <<= maProjection.get(3, 2);
                    nIndex++;
                }

                if(bProjectionUsed_33)
                {
                    mxViewInformation[nIndex].Name = getNamePropertyProjection_33();
                    mxViewInformation[nIndex].Value <<= maProjection.get(3, 3);
                    nIndex++;
                }

                if(bDeviceToViewUsed)
                {
                    com::sun::star::geometry::AffineMatrix3D aAffineMatrix3D;
                    basegfx::unotools::affineMatrixFromHomMatrix3D(aAffineMatrix3D, maDeviceToView);
                    mxViewInformation[nIndex].Name = getNamePropertyDeviceToView();
                    mxViewInformation[nIndex].Value <<= aAffineMatrix3D;
                    nIndex++;
                }

                if(bTimeUsed)
                {
                    mxViewInformation[nIndex].Name = getNamePropertyTime();
                    mxViewInformation[nIndex].Value <<= mfViewTime;
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
            ImpViewInformation3D(
                const basegfx::B3DHomMatrix& rObjectTransformation,
                const basegfx::B3DHomMatrix& rOrientation,
                const basegfx::B3DHomMatrix& rProjection,
                const basegfx::B3DHomMatrix& rDeviceToView,
                double fViewTime,
                const uno::Sequence< beans::PropertyValue >& rExtendedParameters)
            :   mnRefCount(0),
                maObjectTransformation(rObjectTransformation),
                maOrientation(rOrientation),
                maProjection(rProjection),
                maDeviceToView(rDeviceToView),
                mfViewTime(fViewTime),
                mxViewInformation(),
                mxExtendedInformation()
            {
                impInterpretPropertyValues(rExtendedParameters);
            }

            explicit ImpViewInformation3D(const uno::Sequence< beans::PropertyValue >& rViewParameters)
            :   mnRefCount(0),
                maObjectTransformation(),
                maOrientation(),
                maProjection(),
                maDeviceToView(),
                mfViewTime(),
                mxViewInformation(rViewParameters),
                mxExtendedInformation()
            {
                impInterpretPropertyValues(rViewParameters);
            }

            ImpViewInformation3D()
            :   mnRefCount(0),
                maObjectTransformation(),
                maOrientation(),
                maProjection(),
                maDeviceToView(),
                mfViewTime(),
                mxViewInformation(),
                mxExtendedInformation()
            {
            }

            const basegfx::B3DHomMatrix& getObjectTransformation() const { return maObjectTransformation; }
            const basegfx::B3DHomMatrix& getOrientation() const { return maOrientation; }
            const basegfx::B3DHomMatrix& getProjection() const { return maProjection; }
            const basegfx::B3DHomMatrix& getDeviceToView() const { return maDeviceToView; }
            double getViewTime() const { return mfViewTime; }

            const basegfx::B3DHomMatrix& getObjectToView() const
            {
                // on demand WorldToView creation
                ::osl::Mutex m_mutex;

                if(maObjectToView.isIdentity())
                {
                    const_cast< ImpViewInformation3D* >(this)->maObjectToView = maDeviceToView * maProjection * maOrientation * maObjectTransformation;
                }

                return maObjectToView;
            }

            const uno::Sequence< beans::PropertyValue >& getViewInformationSequence() const
            {
                ::osl::Mutex m_mutex;

                if(!mxViewInformation.hasElements())
                {
                    const_cast< ImpViewInformation3D* >(this)->impFillViewInformationFromContent();
                }

                return mxViewInformation;
            }

            const uno::Sequence< beans::PropertyValue >& getExtendedInformationSequence() const
            {
                return mxExtendedInformation;
            }

            bool operator==(const ImpViewInformation3D& rCandidate) const
            {
                return (maObjectTransformation == rCandidate.maObjectTransformation
                    && maOrientation == rCandidate.maOrientation
                    && maProjection == rCandidate.maProjection
                    && maDeviceToView == rCandidate.maDeviceToView
                    && mfViewTime == rCandidate.mfViewTime
                    && mxExtendedInformation == rCandidate.mxExtendedInformation);
            }

            static ImpViewInformation3D* get_global_default()
            {
                static ImpViewInformation3D* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpViewInformation3D();

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };
    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace geometry
    {
        ViewInformation3D::ViewInformation3D(
            const basegfx::B3DHomMatrix& rObjectObjectTransformation,
            const basegfx::B3DHomMatrix& rOrientation,
            const basegfx::B3DHomMatrix& rProjection,
            const basegfx::B3DHomMatrix& rDeviceToView,
            double fViewTime,
            const uno::Sequence< beans::PropertyValue >& rExtendedParameters)
        :   mpViewInformation3D(new ImpViewInformation3D(
                rObjectObjectTransformation, rOrientation, rProjection,
                rDeviceToView, fViewTime, rExtendedParameters))
        {
        }

        ViewInformation3D::ViewInformation3D(const uno::Sequence< beans::PropertyValue >& rViewParameters)
        :   mpViewInformation3D(new ImpViewInformation3D(rViewParameters))
        {
        }

        ViewInformation3D::ViewInformation3D()
        :   mpViewInformation3D(ImpViewInformation3D::get_global_default())
        {
            mpViewInformation3D->mnRefCount++;
        }

        ViewInformation3D::ViewInformation3D(const ViewInformation3D& rCandidate)
        :   mpViewInformation3D(rCandidate.mpViewInformation3D)
        {
            ::osl::Mutex m_mutex;
            mpViewInformation3D->mnRefCount++;
        }

        ViewInformation3D::~ViewInformation3D()
        {
            ::osl::Mutex m_mutex;

            if(mpViewInformation3D->mnRefCount)
            {
                mpViewInformation3D->mnRefCount--;
            }
            else
            {
                delete mpViewInformation3D;
            }
        }

        bool ViewInformation3D::isDefault() const
        {
            return mpViewInformation3D == ImpViewInformation3D::get_global_default();
        }

        ViewInformation3D& ViewInformation3D::operator=(const ViewInformation3D& rCandidate)
        {
            ::osl::Mutex m_mutex;

            if(mpViewInformation3D->mnRefCount)
            {
                mpViewInformation3D->mnRefCount--;
            }
            else
            {
                delete mpViewInformation3D;
            }

            mpViewInformation3D = rCandidate.mpViewInformation3D;
            mpViewInformation3D->mnRefCount++;

            return *this;
        }

        bool ViewInformation3D::operator==(const ViewInformation3D& rCandidate) const
        {
            if(rCandidate.mpViewInformation3D == mpViewInformation3D)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpViewInformation3D == *mpViewInformation3D);
        }

        const basegfx::B3DHomMatrix& ViewInformation3D::getObjectTransformation() const
        {
            return mpViewInformation3D->getObjectTransformation();
        }

        const basegfx::B3DHomMatrix& ViewInformation3D::getOrientation() const
        {
            return mpViewInformation3D->getOrientation();
        }

        const basegfx::B3DHomMatrix& ViewInformation3D::getProjection() const
        {
            return mpViewInformation3D->getProjection();
        }

        const basegfx::B3DHomMatrix& ViewInformation3D::getDeviceToView() const
        {
            return mpViewInformation3D->getDeviceToView();
        }

        const basegfx::B3DHomMatrix& ViewInformation3D::getObjectToView() const
        {
            return mpViewInformation3D->getObjectToView();
        }

        double ViewInformation3D::getViewTime() const
        {
            return mpViewInformation3D->getViewTime();
        }

        const uno::Sequence< beans::PropertyValue >& ViewInformation3D::getViewInformationSequence() const
        {
            return mpViewInformation3D->getViewInformationSequence();
        }

        const uno::Sequence< beans::PropertyValue >& ViewInformation3D::getExtendedInformationSequence() const
        {
            return mpViewInformation3D->getExtendedInformationSequence();
        }
    } // end of namespace geometry
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
