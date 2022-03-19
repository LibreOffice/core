/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <types.hxx>
#include <scdllapi.h>

class ScViewData;
class ScTabViewShell;
class ScDocument;
class ScMarkData;
class ScRangeList;

namespace ScClipUtil
{

SC_DLLPUBLIC void PasteFromClipboard( ScViewData& rViewData, ScTabViewShell* pTabViewShell, bool bShowDialog );

bool CheckDestRanges(
        const ScDocument& rDoc, SCCOL nSrcCols, SCROW nSrcRows, const ScMarkData& rMark,
        const ScRangeList& rDest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
