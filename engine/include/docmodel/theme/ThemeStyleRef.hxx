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

#pragma once

#include <docmodel/dllapi.h>
#include <docmodel/color/ComplexColor.hxx>
#include <sal/types.h>

namespace model
{
/** One of the styles a theme holds, as a shape points at it.

    A theme keeps a list of each kind of style, and a shape says which of them it is drawn with
    by the place the style has in that list, counted from one. The colour goes with the pointing
    rather than with the style: a style is written in terms of a colour it is given, and the
    shape is what gives it. */
struct DOCMODEL_DLLPUBLIC ThemeStyleRef
{
    sal_Int32 mnIndex = 0;
    ComplexColor maColor;

    bool isStated() const;

    bool operator==(ThemeStyleRef const& rOther) const;
};

/** Which of the two fonts of a theme the text of a shape is set in. */
enum class ThemeFontRefType
{
    Unset,
    Minor,
    Major
};

/** The font of a theme that the text of a shape is set in, with the colour that text is given. */
struct DOCMODEL_DLLPUBLIC ThemeFontRef
{
    ThemeFontRefType meType = ThemeFontRefType::Unset;
    ComplexColor maColor;

    bool isStated() const;

    bool operator==(ThemeFontRef const& rOther) const;
};

/** What a shape takes from the styles of the theme behind it: the line it is drawn with, the
    fill it stands on, the effects it is given and the font its text is set in. A shape that
    takes none of them states none. */
struct DOCMODEL_DLLPUBLIC ThemeStyleRefs
{
    ThemeStyleRef maLine;
    ThemeStyleRef maFill;
    ThemeStyleRef maEffect;
    ThemeFontRef maFont;

    bool isStated() const;

    bool operator==(ThemeStyleRefs const& rOther) const;
};

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
