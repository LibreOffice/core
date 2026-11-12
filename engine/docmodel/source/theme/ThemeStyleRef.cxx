/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <docmodel/theme/ThemeStyleRef.hxx>

namespace model
{
bool ThemeStyleRef::isStated() const { return mnIndex != 0; }

bool ThemeStyleRef::operator==(ThemeStyleRef const& rOther) const
{
    return mnIndex == rOther.mnIndex && maColor == rOther.maColor;
}

bool ThemeFontRef::isStated() const { return meType != ThemeFontRefType::Unset; }

bool ThemeFontRef::operator==(ThemeFontRef const& rOther) const
{
    return meType == rOther.meType && maColor == rOther.maColor;
}

bool ThemeStyleRefs::isStated() const
{
    return maLine.isStated() || maFill.isStated() || maEffect.isStated() || maFont.isStated();
}

bool ThemeStyleRefs::operator==(ThemeStyleRefs const& rOther) const
{
    return maLine == rOther.maLine && maFill == rOther.maFill && maEffect == rOther.maEffect
           && maFont == rOther.maFont;
}

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
