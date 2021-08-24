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

#include <sal/config.h>
#include <tools/stream.hxx>
#include <tools/XmlWriter.hxx>

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>

namespace svg
{
enum class SvgVersion
{
    v1_1,
    v2
};

class DRAWINGLAYER_DLLPUBLIC SvgWriter
{
private:
    SvStream& mrStream;
    tools::XmlWriter maWriter;
    SvgVersion meSvgVersion;

    void
    decomposeAndWrite(const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence);

public:
    SvgWriter(SvStream& rStream, SvgVersion eSvgVersion = SvgVersion::v1_1);
    bool write(const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
