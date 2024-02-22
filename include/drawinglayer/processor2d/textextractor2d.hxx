/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>

namespace drawinglayer::processor2d
{
/** TextExtractor2D class

        This processor extracts text in the fed primitives and calls
        processTextPrimitive2D for those while calling processBasePrimitive2D
        for everything else.
     */
class DRAWINGLAYER_DLLPUBLIC TextExtractor2D : public BaseProcessor2D
{
public:
    explicit TextExtractor2D(const geometry::ViewInformation2D& rViewInformation);
    void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate) override;
    virtual void processTextPrimitive2D(const primitive2d::BasePrimitive2D& rCandidate) = 0;
    virtual ~TextExtractor2D() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
