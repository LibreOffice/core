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
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <salhelper/timer.hxx>

namespace drawinglayer::primitive2d
{
/** BufferedDecompositionGroupPrimitive2D class

    Baseclass for all C++ implementations which are derived from GroupPrimitive2D
    which want to buffer the decomposition result

    For discussion please refer to BufferedDecompositionPrimitive2D, this is the same
    but for GroupPrimitive2D which want to buffer their decomposition
 */
class UNLESS_MERGELIBS(DRAWINGLAYER_DLLPUBLIC) BufferedDecompositionGroupPrimitive2D
    : public GroupPrimitive2D
{
private:
    // exclusive helper for Primitive2DFlusher
    friend void flushBufferedDecomposition(BufferedDecompositionGroupPrimitive2D&);

    /// a sequence used for buffering the last create2DDecomposition() result
    Primitive2DContainer maBuffered2DDecomposition;

    /// offer callback mechanism to flush buffered content timer-based
    ::rtl::Reference<::salhelper::Timer> maCallbackTimer;
    mutable std::mutex maCallbackLock;
    sal_uInt16 maCallbackSeconds;

protected:
    /// identical to BufferedDecompositionPrimitive2D, see there please
    bool hasBuffered2DDecomposition() const;
    void setBuffered2DDecomposition(Primitive2DContainer&& rNew);

    /// method which is to be used to implement the local decomposition of a 2D group primitive.
    virtual void
    create2DDecomposition(Primitive2DContainer& rContainer,
                          const geometry::ViewInformation2D& rViewInformation) const = 0;

    // when changing from null (which is inactive) to a count of seconds, the
    // callback mechanism to flush buffered content timer-based will be activated.
    // it is protected since the idea is that this gets called in the constructor
    // of derived classes.
    void setCallbackSeconds(sal_uInt16 nNew) { maCallbackSeconds = nNew; }

public:
    /// constructor/destructor. For GroupPrimitive2D we need the child parameter, too.
    BufferedDecompositionGroupPrimitive2D(Primitive2DContainer&& aChildren);
    virtual ~BufferedDecompositionGroupPrimitive2D();

    /// identical to BufferedDecompositionPrimitive2D, see there please
    virtual void
    get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                       const geometry::ViewInformation2D& rViewInformation) const override;
};

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
