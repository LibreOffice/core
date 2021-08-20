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

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/config.h>
#include <o3tl/cow_wrapper.hxx>

// predefines

namespace drawinglayer::geometry
{
class ImpViewInformation2D;
}

namespace basegfx
{
class B2DHomMatrix;
class B2DRange;
}

namespace com::sun::star::beans
{
struct PropertyValue;
}

namespace com::sun::star::drawing
{
class XDrawPage;
}

namespace com::sun::star::uno
{
template <class interface_type> class Reference;
template <typename> class Sequence;
}

namespace drawinglayer::geometry
{
/** ViewInformation2D class

    This class holds all view-relevant information for a 2d geometry.
    The most used data is for convenience offered directly using basegfx tooling classes.
*/
class DRAWINGLAYER_DLLPUBLIC ViewInformation2D
{
public:
    typedef o3tl::cow_wrapper<ImpViewInformation2D, o3tl::ThreadSafeRefCountingPolicy> ImplType;

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

        @param rReducedDisplayQuality
    */
    ViewInformation2D(const basegfx::B2DHomMatrix& rObjectTransformation,
                      const basegfx::B2DHomMatrix& rViewTransformation,
                      const basegfx::B2DRange& rViewport,
                      const css::uno::Reference<css::drawing::XDrawPage>& rxDrawPage,
                      double fViewTime, bool bReducedDisplayQuality = false);

    /// default (empty) constructor
    ViewInformation2D();

    /// copy constructor
    ViewInformation2D(const ViewInformation2D&);

    ViewInformation2D(ViewInformation2D&&);

    /// destructor
    ~ViewInformation2D();

    /// assignment operator
    ViewInformation2D& operator=(const ViewInformation2D&);
    ViewInformation2D& operator=(ViewInformation2D&&);

    /// compare operators
    bool operator==(const ViewInformation2D& rCandidate) const;
    bool operator!=(const ViewInformation2D& rCandidate) const { return !operator==(rCandidate); }

    /// data access
    const basegfx::B2DHomMatrix& getObjectTransformation() const;
    const basegfx::B2DHomMatrix& getViewTransformation() const;
    const basegfx::B2DRange& getViewport() const;
    double getViewTime() const;
    const css::uno::Reference<css::drawing::XDrawPage>& getVisualizedPage() const;

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
};

DRAWINGLAYER_DLLPUBLIC ViewInformation2D
createViewInformation2D(const css::uno::Sequence<css::beans::PropertyValue>& rViewParameters);

} // end of namespace drawinglayer::geometry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
