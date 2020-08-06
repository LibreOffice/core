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

#include "animationbasenode.hxx"
#include <com/sun/star/animations/XAnimateMotion.hpp>

namespace slideshow
{
namespace internal
{
class AnimationPhysicsNode : public AnimationBaseNode
{
public:
    AnimationPhysicsNode(const css::uno::Reference<css::animations::XAnimationNode>& xNode,
                         const BaseContainerNodeSharedPtr& rParent, const NodeContext& rContext)
        : AnimationBaseNode(xNode, rParent, rContext)
        , mxPhysicsMotionNode(xNode, css::uno::UNO_QUERY_THROW)
    {
    }

#if defined(DBG_UTIL)
    virtual const char* getDescription() const override { return "AnimationPhysicsNode"; }
#endif

protected:
    virtual void dispose() override;

private:
    virtual AnimationActivitySharedPtr createActivity() const override;
    virtual bool enqueueActivity() const override;

    css::uno::Reference<css::animations::XAnimateMotion> mxPhysicsMotionNode;
};

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
