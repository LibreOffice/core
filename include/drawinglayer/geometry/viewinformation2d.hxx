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

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#define INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/config.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <o3tl/cow_wrapper.hxx>


// predefines

namespace drawinglayer { namespace geometry {
    class ImpViewInformation2D;
}}

namespace basegfx {
    class B2DHomMatrix;
    class B2DRange;
}



namespace drawinglayer
{
    namespace geometry
    {
        /** ViewInformation2D class

            This class holds all view-relevant information for a 2d geometry. It works
            together with UNO API definitions and supports holding a sequence of PropertyValues.
            The most used data is for convenience offered directly using basegfx tooling classes.
            It is an implementation to support the sequence of PropertyValues used in a
            css::graphic::XPrimitive2D for C++ implementations working with those
        */
        class DRAWINGLAYER_DLLPUBLIC ViewInformation2D
        {
        public:
            typedef o3tl::cow_wrapper< ImpViewInformation2D, o3tl::ThreadSafeRefCountingPolicy > ImplType;

        private:
            /// pointer to private implementation class
            ImplType mpViewInformation2D;

        public:
            /** Constructor: Create a ViewInformation2D

                @param rObjectTransformation
                The Transformation from Object to World coordinates (normally logic coordinates).

                @param rViewTransformation
                The Transformation from World to View coordinates (normally logic coordinates
                to discrete units, e.g. pixels).

                @param rViewport
                The visible part of the view in World coordinates. If empty (getViewport().isEmpty())
                everything is visible. The data is in World coordinates.

                @param rxDrawPage
                The currently displayed page. This information is needed e.g. due to existing PageNumber
                fields which need to be interpreted.

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
            ViewInformation2D(
                const basegfx::B2DHomMatrix& rObjectTransformation,
                const basegfx::B2DHomMatrix& rViewTransformation,
                const basegfx::B2DRange& rViewport,
                const css::uno::Reference< css::drawing::XDrawPage >& rxDrawPage,
                double fViewTime,
                const css::uno::Sequence< css::beans::PropertyValue >& rExtendedParameters);

            /** Constructor: Create a ViewInformation2D

                @param rViewParameters
                A sequence of property values which allows holding any combination of local and various
                other parameters. This constructor is feeded completely with a sequence of PropertyValues
                which will be parsed to be able to offer the most used ones in a convenient way.
            */
            explicit ViewInformation2D(const css::uno::Sequence< css::beans::PropertyValue >& rViewParameters);

            /// default (empty) constructor
            ViewInformation2D();

            /// copy constructor
            ViewInformation2D(const ViewInformation2D& rCandidate);

            /// destructor
            ~ViewInformation2D();

            /// assignment operator
            ViewInformation2D& operator=(const ViewInformation2D& rCandidate);

            /// compare operators
            bool operator==(const ViewInformation2D& rCandidate) const;
            bool operator!=(const ViewInformation2D& rCandidate) const { return !operator==(rCandidate); }

            /// data access
            const basegfx::B2DHomMatrix& getObjectTransformation() const;
            const basegfx::B2DHomMatrix& getViewTransformation() const;
            const basegfx::B2DRange& getViewport() const;
            double getViewTime() const;
            const css::uno::Reference< css::drawing::XDrawPage >& getVisualizedPage() const;

            /// On-demand prepared Object to View transformation and its inverse for convenience
            const basegfx::B2DHomMatrix& getObjectToViewTransformation() const;
            const basegfx::B2DHomMatrix& getInverseObjectToViewTransformation() const;

            /// On-demand prepared Viewport in discrete units for convenience
            const basegfx::B2DRange& getDiscreteViewport() const;

            /** support reduced DisplayQuality, PropertyName is 'ReducedDisplayQuality'. This
                is used e.g. to allow to lower display quality for OverlayPrimitives and
                may lead to simpler decompositions in the local create2DDecomposition
                implementations of the primitives
             */
            bool getReducedDisplayQuality() const;

            /** Get the uno::Sequence< beans::PropertyValue > which contains all ViewInformation

                Use this call if You need to extract all contained ViewInformation. The ones
                directly supported for convenience will be added to the ones only available
                as PropertyValues. This set completely describes this ViewInformation2D and
                can be used for complete information transport over UNO API.
            */
            const css::uno::Sequence< css::beans::PropertyValue >& getViewInformationSequence() const;

            /** Get the uno::Sequence< beans::PropertyValue > which contains only ViewInformation
                not offered directly

                Use this call if You only need ViewInformation which is not offered conveniently,
                but only exists as PropertyValue. This is e.g. used to create partially updated
                incarnations of ViewInformation2D without losing the only with PropertyValues
                defined data. It does not contain a complete description.
            */
            const css::uno::Sequence< css::beans::PropertyValue >& getExtendedInformationSequence() const;
        };
    } // end of namespace geometry
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
