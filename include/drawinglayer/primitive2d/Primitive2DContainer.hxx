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

#include "rtl/ustring.hxx"
#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/CommonTypes.hxx>
#include <drawinglayer/primitive2d/Primitive2DVisitor.hxx>

#include <basegfx/range/b2drange.hxx>
#include <deque>

namespace drawinglayer::geometry
{
class ViewInformation2D;
}

namespace drawinglayer::primitive2d
{
class SAL_WARN_UNUSED DRAWINGLAYERCORE_DLLPUBLIC Primitive2DContainer final
    : public std::deque<Primitive2DReference>,
      public Primitive2DDecompositionVisitor
{
public:
    // use zero because we allocate a lot of empty containers
    explicit Primitive2DContainer()
        : deque(0)
    {
    }
    explicit Primitive2DContainer(size_type count)
        : deque(count)
    {
    }
    virtual ~Primitive2DContainer() override;
    Primitive2DContainer(const Primitive2DContainer& other)
        : deque(other)
    {
    }
    Primitive2DContainer(Primitive2DContainer&& other) noexcept
        : deque(std::move(other))
    {
    }
    Primitive2DContainer(std::initializer_list<Primitive2DReference> init)
        : deque(init)
    {
    }
    Primitive2DContainer(
        const css::uno::Sequence<css::uno::Reference<css::graphic::XPrimitive2D>>&);
    Primitive2DContainer(const std::deque<css::uno::Reference<css::graphic::XPrimitive2D>>&);

    virtual void visit(const Primitive2DReference& rSource) override { append(rSource); }
    virtual void visit(const Primitive2DContainer& rSource) override { append(rSource); }
    virtual void visit(Primitive2DContainer&& rSource) override { append(std::move(rSource)); }

    void append(const Primitive2DReference&);
    void append(const Primitive2DContainer& rSource);
    void append(Primitive2DContainer&& rSource);
    Primitive2DContainer& operator=(const Primitive2DContainer& r)
    {
        deque::operator=(r);
        return *this;
    }
    Primitive2DContainer& operator=(Primitive2DContainer&& r) noexcept
    {
        deque::operator=(std::move(r));
        return *this;
    }
    bool operator==(const Primitive2DContainer& rB) const;
    bool operator!=(const Primitive2DContainer& rB) const { return !operator==(rB); }
    basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& aViewInformation) const;
    Primitive2DContainer maybeInvert(bool bInvert = false);

    css::uno::Sequence<css::uno::Reference<css::graphic::XPrimitive2D>> toSequence() const;
    void dumpAsXml(const char* pFileLocation = nullptr) const;
};

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
