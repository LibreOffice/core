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
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/geometry/AffineMatrix3D.hpp>
#include <basegfx/utils/canvastools.hxx>
#include <rtl/instance.hxx>
#include <com/sun/star/uno/Sequence.hxx>


using namespace com::sun::star;


namespace drawinglayer::geometry
{
        /** Implementation class for ViewInformation3D
        */
        class ImpViewInformation3D
        {
        private:
            // ViewInformation3D implementation can change refcount, so we have only
            // two memory regions for pairs of ViewInformation3D/ImpViewInformation3D
            friend class ::drawinglayer::geometry::ViewInformation3D;

            // the 3D transformations
            // Object to World. This may change and being adapted when entering 3D transformation
            // groups
            basegfx::B3DHomMatrix                       maObjectTransformation;

            // World to Camera. This includes VRP, VPN and VUV camera coordinate system
            basegfx::B3DHomMatrix                       maOrientation;

            // Camera to Device with X,Y and Z [-1.0 .. 1.0]. This is the
            // 3D to 2D projection which may be parallel or perspective. When it is perspective,
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

            // the extra PropertyValues; does not contain the transformations
            uno::Sequence< beans::PropertyValue >       mxExtendedInformation;

            // the local UNO API strings
            static OUString getNamePropertyObjectTransformation()
            {
                return "ObjectTransformation";
            }

            static OUString getNamePropertyOrientation()
            {
                return "Orientation";
            }

            static OUString getNamePropertyProjection()
            {
                return "Projection";
            }

            static OUString getNamePropertyProjection_30()
            {
                return "Projection30";
            }

            static OUString getNamePropertyProjection_31()
            {
                return "Projection31";
            }

            static OUString getNamePropertyProjection_32()
            {
                return "Projection32";
            }

            static OUString getNamePropertyProjection_33()
            {
                return "Projection33";
            }

            static OUString getNamePropertyDeviceToView()
            {
                return "DeviceToView";
            }

            static OUString getNamePropertyTime()
            {
                return "Time";
            }

            // a central PropertyValue parsing method to allow transportation of
            // all ViewParameters using UNO API
            void impInterpretPropertyValues(const uno::Sequence< beans::PropertyValue >& rViewParameters)
            {
                if(!rViewParameters.hasElements())
                    return;

                const sal_Int32 nCount(rViewParameters.getLength());
                sal_Int32 nExtendedInsert(0);

                // prepare extended information for filtering. Maximum size is nCount
                auto pExtendedInformation = mxExtendedInformation.realloc(nCount);

                for(sal_Int32 a(0); a < nCount; a++)
                {
                    const beans::PropertyValue& rProp = rViewParameters[a];

                    if(rProp.Name == getNamePropertyObjectTransformation())
                    {
                        css::geometry::AffineMatrix3D aAffineMatrix3D;
                        rProp.Value >>= aAffineMatrix3D;
                        maObjectTransformation = basegfx::unotools::homMatrixFromAffineMatrix3D(aAffineMatrix3D);
                    }
                    else if(rProp.Name == getNamePropertyOrientation())
                    {
                        css::geometry::AffineMatrix3D aAffineMatrix3D;
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

                        css::geometry::AffineMatrix3D aAffineMatrix3D;
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
                        css::geometry::AffineMatrix3D aAffineMatrix3D;
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
                        pExtendedInformation[nExtendedInsert++] = rProp;
                    }
                }

                // extra information size is now known; realloc to final size
                mxExtendedInformation.realloc(nExtendedInsert);
            }

        public:
            ImpViewInformation3D(
                const basegfx::B3DHomMatrix& rObjectTransformation,
                const basegfx::B3DHomMatrix& rOrientation,
                const basegfx::B3DHomMatrix& rProjection,
                const basegfx::B3DHomMatrix& rDeviceToView,
                double fViewTime,
                const uno::Sequence< beans::PropertyValue >& rExtendedParameters)
            :   maObjectTransformation(rObjectTransformation),
                maOrientation(rOrientation),
                maProjection(rProjection),
                maDeviceToView(rDeviceToView),
                mfViewTime(fViewTime)
            {
                impInterpretPropertyValues(rExtendedParameters);
            }

            explicit ImpViewInformation3D(const uno::Sequence< beans::PropertyValue >& rViewParameters)
            :   mfViewTime()
            {
                impInterpretPropertyValues(rViewParameters);
            }

            ImpViewInformation3D()
            :   mfViewTime()
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

                if(maObjectToView.isIdentity())
                {
                    const_cast< ImpViewInformation3D* >(this)->maObjectToView = maDeviceToView * maProjection * maOrientation * maObjectTransformation;
                }

                return maObjectToView;
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
        };
} // end of namespace drawinglayer::geometry


namespace drawinglayer::geometry
{
        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< ViewInformation3D::ImplType, theGlobalDefault > {};
        }

        ViewInformation3D::ViewInformation3D(
            const basegfx::B3DHomMatrix& rObjectObjectTransformation,
            const basegfx::B3DHomMatrix& rOrientation,
            const basegfx::B3DHomMatrix& rProjection,
            const basegfx::B3DHomMatrix& rDeviceToView,
            double fViewTime,
            const uno::Sequence< beans::PropertyValue >& rExtendedParameters)
        :   mpViewInformation3D(ImpViewInformation3D(
                rObjectObjectTransformation, rOrientation, rProjection,
                rDeviceToView, fViewTime, rExtendedParameters))
        {
        }

        ViewInformation3D::ViewInformation3D(const uno::Sequence< beans::PropertyValue >& rViewParameters)
        :   mpViewInformation3D(ImpViewInformation3D(rViewParameters))
        {
        }

        ViewInformation3D::ViewInformation3D()
        :   mpViewInformation3D(theGlobalDefault::get())
        {
        }

        ViewInformation3D::ViewInformation3D(const ViewInformation3D&) = default;

        ViewInformation3D::ViewInformation3D(ViewInformation3D&&) = default;

        ViewInformation3D::~ViewInformation3D() = default;

        bool ViewInformation3D::isDefault() const
        {
            return mpViewInformation3D.same_object(theGlobalDefault::get());
        }

        ViewInformation3D& ViewInformation3D::operator=(const ViewInformation3D&) = default;

        ViewInformation3D& ViewInformation3D::operator=(ViewInformation3D&&) = default;

        bool ViewInformation3D::operator==(const ViewInformation3D& rCandidate) const
        {
            return rCandidate.mpViewInformation3D == mpViewInformation3D;
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

        const uno::Sequence< beans::PropertyValue >& ViewInformation3D::getExtendedInformationSequence() const
        {
            return mpViewInformation3D->getExtendedInformationSequence();
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
