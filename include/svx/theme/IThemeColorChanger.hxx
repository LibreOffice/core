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
#include <svx/ColorSets.hxx>

namespace svx
{
class SVX_DLLPUBLIC IThemeColorChanger
{
public:
    virtual ~IThemeColorChanger() = default;
    virtual void apply(svx::ColorSet const& rColorSet) = 0;
};

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
