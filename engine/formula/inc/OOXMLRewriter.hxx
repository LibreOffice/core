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

#include <formula/formuladllapi.h>

#include <memory>

namespace formula
{
class FormulaTokenArray;

/** Rewrites the token array to make it compatible with OOXML.

    OOXML writes @ and # as calls and a reference union as a parenthesised list. Those call
    names and parentheses become tokens here, so writing the result is a plain walk. The #
    also moves in front of its operand and becomes ocAnchorArray.

    Operator extents come from the token order, so this restates the precedence the parser
    applied. Change one and the other needs it too.
 */
class FORMULA_DLLPUBLIC OOXMLRewriter
{
public:
    explicit OOXMLRewriter(FormulaTokenArray const& rTokens);

    /** The OOXML compatible token array, or nullptr. The caller takes ownership. */
    std::unique_ptr<FormulaTokenArray> releaseTokens()
    {
        return std::move(mpRewritten);
    }
private:
    std::unique_ptr<FormulaTokenArray> mpRewritten;
};

} // namespace formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
