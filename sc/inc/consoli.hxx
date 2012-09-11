/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    String**            ppColHeaders;
    String**            ppRowHeaders;
    SCSIZE              nDataCount;
    SCSIZE              nTitleCount;
    String**            ppTitles;
    SCSIZE**            ppTitlePos;
    sal_Bool                bCornerUsed;
    String              aCornerText;        // only for bColByName && bRowByName

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
    void        AddName( const String& rName );

    void        OutputToDocument( ScDocument* pDestDoc, SCCOL nCol, SCROW nRow, SCTAB nTab );

    void        GetSize( SCCOL& rCols, SCROW& rRows ) const;
    SCROW       GetInsertCount() const;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
