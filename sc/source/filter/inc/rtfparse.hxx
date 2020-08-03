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

#pragma once

#include "eeparser.hxx"

#include <vector>
#include <memory>
#include <o3tl/sorted_vector.hxx>

struct ScRTFCellDefault
{
    SfxItemSet          aItemSet;
    SCCOL               nCol;
    sal_uInt16          nTwips;         // right border of cell
    SCCOL               nColOverlap;    // MergeCell if >1, merged cells if 0

    ScRTFCellDefault( SfxItemPool* pPool )
        : aItemSet(*pPool)
        , nCol(0)
        , nTwips(0)
        , nColOverlap(1)
    {
    }
};

class ScRTFColTwips : public o3tl::sorted_vector<sal_uLong> {};

class EditEngine;

class ScRTFParser : public ScEEParser
{
private:
    typedef std::vector< std::unique_ptr<ScRTFCellDefault> > DefaultList;

    DefaultList maDefaultList;
    size_t mnCurPos;

    ScRTFColTwips       aColTwips;
    std::unique_ptr<ScRTFCellDefault> pInsDefault;
    ScRTFCellDefault*   pActDefault;
    ScRTFCellDefault*   pDefMerge;
    sal_uLong           nStartAdjust;
    sal_uInt16          nLastWidth;
    bool                bNewDef;

    DECL_LINK( RTFImportHdl, RtfImportInfo&, void );
    inline void         NextRow();
    void                EntryEnd( ScEEParseEntry*, const ESelection& );
    void                ProcToken( RtfImportInfo* );
    void                ColAdjust();
    bool                SeekTwips( sal_uInt16 nTwips, SCCOL* pCol );
    void                NewCellRow();

public:
                        ScRTFParser( EditEngine* );
    virtual             ~ScRTFParser() override;
    virtual ErrCode     Read( SvStream&, const OUString& rBaseURL ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
