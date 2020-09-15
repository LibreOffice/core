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

#ifndef INCLUDED_SC_INC_CONSOLI_HXX
#define INCLUDED_SC_INC_CONSOLI_HXX

#include "global.hxx"
#include "address.hxx"
#include <vector>

class ScDocument;
class ScFunctionData;

//  Sequence:
//      1)  create ScConsData
//      2)  Parameter (Size/Flags)
//      3)  AddFields for all areas (only needed for bColByName or bRowByName)
//      4)  DoneFields              (       "                       "        )
//      5)  AddData for all areas
//          perhaps AddName after each area
//      6)  OutputToDocument

//! Use structure ScDocument if a certain size is exceeded?

class ScConsData
{
private:
    struct ScReferenceEntry             // without constructor !
    {
        SCCOL   nCol;
        SCROW   nRow;
        SCTAB   nTab;
    };
    typedef std::vector<ScReferenceEntry> ScReferenceList;
    ScSubTotalFunc      eFunction;
    bool                bReference;
    bool                bColByName;
    bool                bRowByName;
    SCSIZE              nColCount;
    SCSIZE              nRowCount;
    std::unique_ptr<std::unique_ptr<bool[]>[]> ppUsed;
    std::unique_ptr<std::unique_ptr<ScFunctionData[]>[]> ppFunctionData;
    std::unique_ptr<std::unique_ptr<ScReferenceList[]>[]> ppRefs;
    ::std::vector<OUString> maColHeaders;
    ::std::vector<OUString> maRowHeaders;
    ::std::vector<OUString> maTitles;
    SCSIZE              nDataCount;
    std::unique_ptr<std::unique_ptr<SCSIZE[]>[]> ppTitlePos;
    bool                bCornerUsed;
    OUString            aCornerText;        // only for bColByName && bRowByName

public:
                ScConsData();
                ~ScConsData();

    void        SetSize( SCCOL nCols, SCROW nRows );
    void        SetFlags( ScSubTotalFunc eFunc, bool bColName, bool bRowName, bool bRef );

    void        InitData();
    void        DeleteData();

    void        AddFields( const ScDocument* pSrcDoc, SCTAB nTab,
                           SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    void        DoneFields();

    void        AddData( ScDocument* pSrcDoc, SCTAB nTab,
                            SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    void        AddName( const OUString& rName );

    void        OutputToDocument( ScDocument& rDestDoc, SCCOL nCol, SCROW nRow, SCTAB nTab );

    void        GetSize( SCCOL& rCols, SCROW& rRows ) const;
    SCROW       GetInsertCount() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
