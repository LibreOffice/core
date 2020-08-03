/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <basegfx/DrawCommands.hxx>
#include <memory>
#include "svgnode.hxx"

namespace svgio::svgreader
{
class SvgDrawVisitor final : public Visitor
{
private:
    std::shared_ptr<gfx::DrawRoot> mpDrawRoot;
    std::shared_ptr<gfx::DrawBase> mpCurrent;

public:
    SvgDrawVisitor();

    void visit(svgio::svgreader::SvgNode const& rNode) override;
    void goToChildren(svgio::svgreader::SvgNode const& rNode);

    std::shared_ptr<gfx::DrawRoot> const& getDrawRoot() const { return mpDrawRoot; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
