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

class ScViewData;

namespace tools
{
class JsonWriter;
}

namespace sc
{
/**
 * Write the cell cursor and the marked ranges of the view, on its selected
 * sheet, as cell indexes.
 */
void putCellCursorState(tools::JsonWriter& rWriter, const ScViewData& rViewData);

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
