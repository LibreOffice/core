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

#include <string_view>

class SfxViewShell;

namespace kitcommand
{
// Puts the caret of the running text edit at the object, paragraph and index that a
// .uno:SetTextCursor?object=...&paragraph=...&index=... command names, with the text from the
// optional anchorparagraph and anchorindex selected. A command without the three parameters
// does nothing.
void setTextCursor(SfxViewShell& rViewShell, std::u16string_view aCommand);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
