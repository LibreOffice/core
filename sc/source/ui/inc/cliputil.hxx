/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_CLIPUTIL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CLIPUTIL_HXX

#include <types.hxx>

class ScViewData;
class ScTabViewShell;
class ScDocument;
class ScMarkData;
class ScRangeList;

namespace ScClipUtil
{
    void PasteFromClipboard( ScViewData& rViewData, ScTabViewShell* pTabViewShell, bool bShowDialog );

    bool CheckDestRanges(
        const ScDocument& rDoc, SCCOL nSrcCols, SCROW nSrcRows, const ScMarkData& rMark,
        const ScRangeList& rDest);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
