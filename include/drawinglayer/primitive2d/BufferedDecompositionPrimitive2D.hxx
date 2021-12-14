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

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

namespace drawinglayer::geometry
{
class ViewInformation2D;
}

namespace drawinglayer::primitive2d
{
/** BufferedDecompositionPrimitive2D class

    Baseclass for all C++ implementations of css::graphic::XPrimitive2D
    which want to buffer the decomposition result

    Buffering the decomposition is the most-used buffering and is thus used my most
    primitive implementations which support a decomposition as base class.

    The buffering is done by holding the last decomposition in the local parameter
    maBuffered2DDecomposition. The default implementation of get2DDecomposition checks
    if maBuffered2DDecomposition is empty. If yes, it uses create2DDecomposition
    to create the content. In all cases, maBuffered2DDecomposition is returned.

    For view-dependent primitives derived from Primitive2DBufferDecomposition more needs
    to be done when the decomposition depends on parts of the parameter ViewInformation2D.
    This defines a standard method for processing these:

    Implement a view-dependent get2DDecomposition doing the following steps:
    (a) Locally extract needed parameters from ViewInformation2D to new, local parameters
        (this may be a complete local copy of ViewInformation2D)
    (b) If a buffered decomposition exists, ckeck if one of the new local parameters
        differs from the corresponding locally remembered (as member) ones. If yes,
        clear maBuffered2DDecomposition
    (d) call baseclass::get2DDecomposition which will use create2DDecomposition
        to fill maBuffered2DDecomposition if it's empty
    (e) copy the new local parameters to the corresponding locally remembered ones
        to identify if a new decomposition is needed at the next call
    (f) return maBuffered2DDecomposition
 */
class DRAWINGLAYER_DLLPUBLIC BufferedDecompositionPrimitive2D : public BasePrimitive2D
{
private:
    /// a sequence used for buffering the last create2DDecomposition() result
    Primitive2DContainer maBuffered2DDecomposition;

    /// When a shadow wraps a list of primitives, this primitive wants to influence the transparency
    /// of the shadow.
    sal_uInt16 mnTransparenceForShadow = 0;

protected:
    /** access methods to maBuffered2DDecomposition. The usage of this methods may allow
        later thread-safe stuff to be added if needed. Only to be used by getDecomposition()
        implementations for buffering the last decomposition.
     */
    const Primitive2DContainer& getBuffered2DDecomposition() const
    {
        return maBuffered2DDecomposition;
    }
    void setBuffered2DDecomposition(Primitive2DContainer&& rNew)
    {
        maBuffered2DDecomposition = std::move(rNew);
    }

    /** method which is to be used to implement the local decomposition of a 2D primitive. */
    virtual void
    create2DDecomposition(Primitive2DContainer& rContainer,
                          const geometry::ViewInformation2D& rViewInformation) const = 0;

public:
    // constructor/destructor
    BufferedDecompositionPrimitive2D();

    /** The getDecomposition default implementation will on demand use create2DDecomposition() if
        maBuffered2DDecomposition is empty. It will set maBuffered2DDecomposition to this obtained decomposition
        to buffer it. If the decomposition is also ViewInformation2D-dependent, this method needs to be
        overridden and the ViewInformation2D for the last decomposition need to be remembered, too, and
        be used in the next call to decide if the buffered decomposition may be reused or not.
     */
    virtual void
    get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                       const geometry::ViewInformation2D& rViewInformation) const override;

    void setTransparenceForShadow(sal_uInt16 nTransparenceForShadow)
    {
        mnTransparenceForShadow = nTransparenceForShadow;
    }

    sal_uInt16 getTransparenceForShadow() const { return mnTransparenceForShadow; }
};

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
