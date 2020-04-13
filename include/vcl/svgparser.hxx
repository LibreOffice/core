/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vcl/dllapi.h>
#include <com/sun/star/uno/Reference.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <com/sun/star/io/XInputStream.hpp>

namespace gfx
{
class DrawRoot;
}

namespace vcl
{
/** This is here to break a circular dependency between svgio and vcl */
class VCL_DLLPUBLIC AbstractSvgParser
{
public:
    virtual ~AbstractSvgParser();

    virtual drawinglayer::primitive2d::Primitive2DContainer
    getDecomposition(const css::uno::Reference<css::io::XInputStream>& xSVGStream,
                     const OUString& aAbsolutePath)
        = 0;

    virtual gfx::DrawRoot*
    getDrawCommands(css::uno::Reference<css::io::XInputStream> const& xSvgStream,
                    const OUString& aAbsolutePath)
        = 0;
};

VCL_DLLPUBLIC std::unique_ptr<AbstractSvgParser> loadSvgParser();
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
