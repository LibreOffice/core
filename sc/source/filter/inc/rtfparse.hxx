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
#if 1

#include "eeparser.hxx"

#ifdef SC_RTFPARSE_CXX
#include <boost/ptr_container/ptr_vector.hpp>
#include <o3tl/sorted_vector.hxx>

struct ScRTFCellDefault
{
    SfxItemSet          aItemSet;
    SCCOL               nCol;
    sal_uInt16              nTwips;         // rechter Rand der Zelle
    SCCOL               nColOverlap;    // MergeCell wenn >1, merged cells wenn 0

                        ScRTFCellDefault( SfxItemPool* pPool ) :
                            aItemSet( *pPool ), nColOverlap(1) {}
};

class ScRTFColTwips : public o3tl::sorted_vector<sal_uLong> {};

#else       // SC_RTFPARSE_CXX

struct ScRTFCellDefault;
class ScRTFColTwips;

#endif      // SC_RTFPARSE_CXX


class EditEngine;

class ScRTFParser : public ScEEParser
{
private:
    typedef boost::ptr_vector<ScRTFCellDefault> DefaultList;

    DefaultList maDefaultList;
    size_t mnCurPos;

    ScRTFColTwips*      pColTwips;
    ScRTFCellDefault*   pInsDefault;
    ScRTFCellDefault*   pActDefault;
    ScRTFCellDefault*   pDefMerge;
    sal_uLong               nStartAdjust;
    sal_uInt16              nLastWidth;
    sal_Bool                bNewDef;

    DECL_LINK( RTFImportHdl, ImportInfo* );
    inline void         NextRow();
    void                EntryEnd( ScEEParseEntry*, const ESelection& );
    void                ProcToken( ImportInfo* );
    void                ColAdjust();
    sal_Bool                SeekTwips( sal_uInt16 nTwips, SCCOL* pCol );
    void                NewCellRow( ImportInfo* );

public:
                        ScRTFParser( EditEngine* );
    virtual             ~ScRTFParser();
    virtual sal_uLong       Read( SvStream&, const String& rBaseURL );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
