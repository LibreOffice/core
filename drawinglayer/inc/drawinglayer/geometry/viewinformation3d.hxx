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

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION3D_HXX
#define INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

// the solaris compiler defines 'sun' as '1'. To avoid that (and to allow
// pre-declarations of com/sun/star namespace), include sal/config.h here
// where sun is redefined as 'sun' (so i guess the problem is known).
#include <sal/config.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace geometry {
    class ImpViewInformation3D;
}}

namespace basegfx {
    class B3DHomMatrix;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace geometry
    {
        /** ViewInformation3D class

            This class holds all view-relevant information for a 3d geometry. It works
            together with UNO API definitions and supports holding a sequence of PropertyValues.
            The most used data is for convenience offered directly using basegfx tooling classes.
            It is an implementation to support the sequence of PropertyValues used in a
            ::com::sun::star::graphic::XPrimitive3D for C++ implementations working with those
        */
        class DRAWINGLAYER_DLLPUBLIC ViewInformation3D
        {
        private:
            /// pointer to private implementation class
            ImpViewInformation3D*                   mpViewInformation3D;

        public:
            /** Constructor: Create a ViewInformation3D

                @param rObjectTransformation
                The Transformation from Object to World coordinates (normally logic coordinates).

                @param rOrientation
                A part of the 3D ViewTransformation, the World to Camera coordinates transformation
                which holds the camera coordinate system.

                @param rProjection
                A part of the 3D ViewTransformation, the Camera to Device transformation which
                transforms coordinates to a [0.0 .. 1.0] device range in X,Y and Z. Z may be used
                as source for for Z-Buffers. This transformation may be e.g. a parallell projection,
                but also a perspective one and thus may use the last line of the matrix.

                @param rDeviceToView
                A part of the 3D ViewTransformation, the Device to View transformation which normally
                translates and scales from [0.0 .. 1.0] range in X,Y and Z to discrete position and
                size.

                rOrientation, rProjection and rDeviceToView define the 3D transformation pipeline
                and are normally used multiplied together to have a direct transformation from
                World to View coordinates

                @param fViewTime
                The time the view is defined for. Default is 0.0. This parameter is used e.g. for
                animated objects

                @param rExtendedParameters
                A sequence of property values which allows holding various other parameters besides
                the obvious and needed ones above. For this constructor none of the other parameters
                should be added as data. The constructor will parse the given parameters and if
                data for the other parameters is given, the value in rExtendedParameters will
                be preferred and overwrite the given parameter
            */
            ViewInformation3D(
                const basegfx::B3DHomMatrix& rObjectTransformation,
                const basegfx::B3DHomMatrix& rOrientation,
                const basegfx::B3DHomMatrix& rProjection,
                const basegfx::B3DHomMatrix& rDeviceToView,
                double fViewTime,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rExtendedParameters);

            /** Constructor: Create a ViewInformation3D

                @param rViewParameters
                A sequence of property values which allows holding any combination of local and various
                other parameters. This constructor is feeded completely with a sequence of PropertyValues
                which will be parsed to be able to offer the most used ones in a convenient way.
            */
            explicit ViewInformation3D(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rViewParameters);

            /// default (empty) constructor
            ViewInformation3D();

            /// copy constructor
            ViewInformation3D(const ViewInformation3D& rCandidate);

            /// destructor
            ~ViewInformation3D();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            /// assignment operator
            ViewInformation3D& operator=(const ViewInformation3D& rCandidate);

            /// compare operators
            bool operator==(const ViewInformation3D& rCandidate) const;
            bool operator!=(const ViewInformation3D& rCandidate) const { return !operator==(rCandidate); }

            /// data access
            const basegfx::B3DHomMatrix& getObjectTransformation() const;
            const basegfx::B3DHomMatrix& getOrientation() const;
            const basegfx::B3DHomMatrix& getProjection() const;
            const basegfx::B3DHomMatrix& getDeviceToView() const;
            double getViewTime() const;

            /// for convenience, the linear combination of the above four transformations is offered
            const basegfx::B3DHomMatrix& getObjectToView() const;

            /** Get the uno::Sequence< beans::PropertyValue > which contains all ViewInformation

                Use this call if You need to extract all contained ViewInformation. The ones
                directly supported for convenience will be added to the ones only available
                as PropertyValues. This set completely describes this ViewInformation3D and
                can be used for complete information transport over UNO API.
            */
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& getViewInformationSequence() const;

            /** Get the uno::Sequence< beans::PropertyValue > which contains only ViewInformation
                not offered directly

                Use this call if You only need ViewInformation which is not offered conveniently,
                but only exists as PropertyValue. This is e.g. used to create partially updated
                incarnations of ViewInformation3D without losing the only with PropertyValues
                defined data. It does not contain a complete description.
            */
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& getExtendedInformationSequence() const;
        };
    } // end of namespace geometry
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
