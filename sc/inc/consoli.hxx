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

#ifndef SC_CONSOLI_HXX
#define SC_CONSOLI_HXX

#include "global.hxx"
#include "address.hxx"

class ScDocument;

struct ScReferenceEntry             // without constructor !
{
    SCCOL   nCol;
    SCROW   nRow;
    SCTAB   nTab;
};


//!     Use delta value for data?

class ScReferenceList           // without constructor !
{
private:
    SCSIZE              nCount;
    SCSIZE              nFullSize;          // incl. fill entries
    ScReferenceEntry*   pData;

public:
    void                    Init()                      { nCount=0; nFullSize=0; pData=NULL; }
    void                    Clear()                     { delete[] pData; }

    SCSIZE                  GetCount()                  { return nCount; }
    const ScReferenceEntry& GetEntry( SCSIZE nPos )     { return pData[nPos]; }
    void                    SetFullSize( SCSIZE nNew )  { nFullSize = nNew; }

    void                    AddEntry( SCCOL nCol, SCROW nRow, SCTAB nTab );
};

//
//  Sequence:
//      1)  create ScConsData
//      2)  Parameter (Size/Flags)
//      3)  AddFields for all areas (only needed for bColByName or bRowByName)
//      4)  DoneFields              (       "                       "        )
//      5)  AddData for all areas
//          perhaps AddName after each area
//      6)  OutputToDocument
//

//! Use structure ScDocument if a certain size is exceeded?

class ScConsData
{
private:
    ScSubTotalFunc      eFunction;
    sal_Bool                bReference;
    sal_Bool                bColByName;
    sal_Bool                bRowByName;
    SCSIZE              nColCount;
    SCSIZE              nRowCount;
    sal_Bool**              ppUsed;
    double**            ppSum;
    double**            ppCount;
    double**            ppSumSqr;
    ScReferenceList**   ppRefs;
    ::std::vector<OUString> maColHeaders;
    ::std::vector<OUString> maRowHeaders;
    ::std::vector<OUString> maTitles;
    SCSIZE              nDataCount;
    SCSIZE**            ppTitlePos;
    sal_Bool                bCornerUsed;
    OUString            aCornerText;        // only for bColByName && bRowByName

public:
                ScConsData();
                ~ScConsData();

    void        SetSize( SCCOL nCols, SCROW nRows );
    void        SetFlags( ScSubTotalFunc eFunc, sal_Bool bColName, sal_Bool bRowName, sal_Bool bRef );

    void        InitData(sal_Bool bDelete=sal_True);
    void        DeleteData();

    void        AddFields( ScDocument* pSrcDoc, SCTAB nTab,
                            SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    void        DoneFields();

    void        AddData( ScDocument* pSrcDoc, SCTAB nTab,
                            SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    void        AddName( const OUString& rName );

    void        OutputToDocument( ScDocument* pDestDoc, SCCOL nCol, SCROW nRow, SCTAB nTab );

    void        GetSize( SCCOL& rCols, SCROW& rRows ) const;
    SCROW       GetInsertCount() const;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
