/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <svx/svxdllapi.h>
#include <drawinglayer/tools/primitive2dxmldump.hxx>

namespace svx
{
/**
 * Extends the normal dumper to include svx primitives
 */
class SVXCORE_DLLPUBLIC ExtendedPrimitive2dXmlDump : public drawinglayer::Primitive2dXmlDump
{
public:
    ExtendedPrimitive2dXmlDump();
    virtual ~ExtendedPrimitive2dXmlDump();
    virtual bool
    decomposeAndWrite(const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive2DSequence,
                      ::tools::XmlWriter& rWriter);
};

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
