/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_CONSOLI_HXX
#define SC_CONSOLI_HXX

#include "global.hxx"
#include "address.hxx"

class ScDocument;

// -----------------------------------------------------------------------

struct ScReferenceEntry             // ohne Constructor !
{
    SCCOL   nCol;
    SCROW   nRow;
    SCTAB   nTab;
};


//!     Delta-Wert fuer Daten benutzen?

class ScReferenceList           // ohne Constructor !
{
private:
    SCSIZE              nCount;
    SCSIZE              nFullSize;          // inkl. Fuell-Eintraege
    ScReferenceEntry*   pData;

public:
    void                    Init()                      { nCount=0; nFullSize=0; pData=NULL; }
    void                    Clear()                     { delete[] pData; }

    SCSIZE                  GetCount()                  { return nCount; }
    const ScReferenceEntry& GetEntry( SCSIZE nPos )     { return pData[nPos]; }
    void                    SetFullSize( SCSIZE nNew )  { nFullSize = nNew; }

    void                    AddEntry( SCCOL nCol, SCROW nRow, SCTAB nTab );
};

// -----------------------------------------------------------------------


//
//  Reihenfolge:
//      1)  ScConsData anlegen
//      2)  Parameter (Size/Flags)
//      3)  AddFields fuer alle Bereiche (nur noetig bei bColByName oder bRowByName)
//      4)  DoneFields                   (      "                       "          )
//      5)  AddData fuer alle Bereiche
//          evtl. AddName nach jedem Bereich
//      6)  OutputToDocument
//

//! ab bestimmter Groesse ScDocument Struktur benutzen?


class ScConsData
{
private:
    ScSubTotalFunc      eFunction;
    sal_Bool                bReference;
    sal_Bool                bColByName;
    sal_Bool                bRowByName;
    sal_Bool                bSubTitles;
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
    String              aCornerText;        // nur bei bColByName && bRowByName

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


