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

#include <swdllapi.h>
#include <swrect.hxx>

#include <optional>

class SwViewOption;
class SwViewShell;

namespace sw::vector
{
/** What a decomposition of the layout into primitives is for.

    The primitives go either to any client or to one view. For any client the text in the
    automatic color keeps that color, and the view options are the same whichever view is the
    current one. For one view the options of that view apply, and they decide the text colors.
 */
class SW_DLLPUBLIC Context
{
public:
    /// For any client.
    Context() = default;

    /// For the view of rShell only.
    explicit Context(const SwViewShell& rShell)
        : mpShell(&rShell)
    {
    }

    /// Whether the primitives are drawn for one view only.
    bool isForView() const { return mpShell != nullptr; }

    /// The shell of the view the primitives are for, or nullptr when they are for any client.
    const SwViewShell* shell() const { return mpShell; }

    /// The view options the text is drawn with: the options of the view, or the fixed ones for
    /// any client.
    const SwViewOption& options() const;

    /// Limits the decomposition to what reaches into rArea, in document twips.
    void setArea(const SwRect& rArea) { moArea = rArea; }

    /// Whether what is drawn within rBounds, in document twips, belongs to the decomposition.
    bool wants(const SwRect& rBounds) const { return !moArea || moArea->Overlaps(rBounds); }

private:
    const SwViewShell* mpShell = nullptr;
    /// The area the decomposition is limited to, in document twips. None for the whole layout.
    std::optional<SwRect> moArea;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
