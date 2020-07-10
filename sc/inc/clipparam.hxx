/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_INC_CLIPPARAM_HXX
#define INCLUDED_SC_INC_CLIPPARAM_HXX

#include "rangelst.hxx"
#include "charthelper.hxx"

/**
 * This struct stores general clipboard parameters associated with a
 * ScDocument instance created in clipboard mode.
 */
struct SC_DLLPUBLIC ScClipParam
{
    enum Direction { Unspecified, Column, Row };

    ScRangeList         maRanges;
    Direction           meDirection;
    bool                mbCutMode;
    sal_uInt32          mnSourceDocID;
    ScRangeListVector   maProtectedChartRangesVector;

    ScClipParam();
    ScClipParam(const ScRange& rRange, bool bCutMode);

    bool isMultiRange() const;

    /**
     * Get the column size of a pasted range.  Note that when the range is
     * non-contiguous, we first compress all individual ranges into a single
     * range, and the size of that compressed range is returned.
     */
    SCCOL getPasteColSize();

    /**
     * Same as the above method, but returns the row size of the compressed
     * range.
     */
    SCROW getPasteRowSize();

    /**
     * Return a single range that encompasses all individual ranges.
     */
    ScRange getWholeRange() const;

    void transpose();

    sal_uInt32 getSourceDocID() const { return mnSourceDocID; }
    void setSourceDocID( sal_uInt32 nVal ) { mnSourceDocID = nVal; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
