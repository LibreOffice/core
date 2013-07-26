/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_CLIPUTIL_HXX__
#define __SC_CLIPUTIL_HXX__

#include "address.hxx"

class ScViewData;
class ScTabViewShell;
class ScDocument;
class ScMarkData;
class ScRangeList;

class ScClipUtil
{
    ScClipUtil();
    ~ScClipUtil();
public:
    static void PasteFromClipboard( ScViewData* pViewData, ScTabViewShell* pTabViewShell, bool bShowDialog );

    static bool CheckDestRanges(
        ScDocument* pDoc, SCCOL nSrcCols, SCROW nSrcRows, const ScMarkData& rMark,
        const ScRangeList& rDest);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
