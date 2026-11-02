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

#include <address.hxx>
#include <editeng/svxenum.hxx>
#include <rtl/ustring.hxx>

#include <string_view>

class ScDocument;
class ScPatternAttr;
class SfxItemSet;

namespace sc
{
/** Get left, right or centered alignment from the right-to-left context. Standard, block and repeat
    resolve to the contextual left or right.
 */
SvxCellHorJustify getAlignmentFromContext(SvxCellHorJustify eInHorJust, bool bCellIsValue,
                                          bool bCellIsCallable, const OUString& rText,
                                          const ScPatternAttr& rPattern, const SfxItemSet* pCondSet,
                                          const ScDocument& rDocument, SCTAB nTab,
                                          bool bNumberFormatIsText);

/// Whether the number format of the cell is a text format.
bool isNumberFormatText(const ScDocument& rDocument, SCCOL nColumn, SCROW nRow, SCTAB nTab);

/** Whether the text holds characters that only the EditEngine lays out: soft hyphen, zero width
    space, direction marks, no-break hyphen and word joiner. The no-break space counts only without
    a repeat character, because it is the thousands separator of some number formats.
 */
bool hasEditCharacters(std::u16string_view rText, bool bHasRepeatCharacter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
