/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <o3tl/any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>

namespace oox::core
{
struct Cond
{
    OString msDelay;
    const char* mpEvent;
    css::uno::Reference<css::drawing::XShape> mxShape;
    css::uno::Reference<css::animations::XAnimationNode> mxNode;

    Cond(const css::uno::Any& rAny, bool bIsMainSeqChild);

    bool isValid() const { return msDelay.getLength() || mpEvent; }
    const char* getDelay() const { return msDelay.getLength() ? msDelay.getStr() : nullptr; }
};
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
