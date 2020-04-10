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

#include <drawinglayer/primitive2d/CommonTypes.hxx>
#include <drawinglayer/primitive2d/Primitive2DVisitor.hxx>
#include <functional>
#include <memory>

namespace drawinglayer::primitive2d
{
typedef std::function<void(const Primitive2DReference)> Primitive2DProcessingCallback;

class DRAWINGLAYER_DLLPUBLIC BasePrimitive2DContainer : public Primitive2DDecompositionVisitor
{
public:
    virtual ~BasePrimitive2DContainer() {}
    virtual void processAll(Primitive2DProcessingCallback aCallback) const = 0;
};

typedef std::shared_ptr<BasePrimitive2DContainer> BasePrimitive2DContainerPtr;
} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
