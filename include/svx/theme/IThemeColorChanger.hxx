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
#include <docmodel/theme/ColorSet.hxx>
#include "ThemeColorChangerCommon.hxx"

namespace svx
{
class SVXCORE_DLLPUBLIC IThemeColorChanger
{
public:
    virtual ~IThemeColorChanger() = default;
    void apply(std::shared_ptr<model::ColorSet> const& pColorSet)
    {
        doApply(pColorSet);
        svx::theme::notifyLOK(pColorSet, std::set<Color>());
    }

private:
    virtual void doApply(std::shared_ptr<model::ColorSet> const& pColorSet) = 0;
};

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
