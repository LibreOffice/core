/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/bitmapex.hxx>
#include <vcl/animate/Animation.hxx>

class VCL_DLLPUBLIC ImportOutput
{
public:
    bool mbIsAnimated = false;
    std::optional<BitmapEx> moBitmap;
    std::optional<Animation> moAnimation;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
