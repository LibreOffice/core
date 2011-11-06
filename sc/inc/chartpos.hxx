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



#ifndef SC_CHARTPOS_HXX
#define SC_CHARTPOS_HXX

// -----------------------------------------------------------------------

#include "collect.hxx"
#include "rangelst.hxx"


class ScAddress;
class Table;

class ScChartPositionMap
{
    friend class ScChartPositioner;

            ScAddress**         ppData;
            ScAddress**         ppColHeader;
            ScAddress**         ppRowHeader;
            sal_uLong               nCount;
            SCCOL               nColCount;
            SCROW               nRowCount;

                                ScChartPositionMap( SCCOL nChartCols, SCROW nChartRows,
                                    SCCOL nColAdd,      // Header-Spalten
                                    SCROW nRowAdd,      // Header-Zeilen
                                    Table& rCols        // Table mit Col-Tables mit Address*
                                    );
                                ~ScChartPositionMap();  //! deletes all ScAddress*

                                // not implemented
                                ScChartPositionMap( const ScChartPositionMap& );
            ScChartPositionMap& operator=( const ScChartPositionMap& );

public:

            sal_uLong               GetCount() const { return nCount; }
            SCCOL               GetColCount() const { return nColCount; }
            SCROW               GetRowCount() const { return nRowCount; }

            sal_Bool                IsValid( SCCOL nCol, SCROW nRow ) const
                                    { return nCol < nColCount && nRow < nRowCount; }
                                // Daten spaltenweise
            sal_uLong               GetIndex( SCCOL nCol, SCROW nRow ) const
                                    { return (sal_uLong) nCol * nRowCount + nRow; }

            const ScAddress*    GetPosition( sal_uLong nIndex ) const
                                    {
                                        if ( nIndex < nCount )
                                            return ppData[ nIndex ];
                                        return NULL;
                                    }

                                    //! kann NULL sein und damit "kein Wert"
            const ScAddress*    GetPosition( SCCOL nChartCol, SCROW nChartRow ) const
                                    {
                                        if ( IsValid( nChartCol, nChartRow ) )
                                            return ppData[ GetIndex( nChartCol, nChartRow ) ];
                                        return NULL;
                                    }
            const ScAddress*    GetColHeaderPosition( SCCOL nChartCol ) const
                                    {
                                        if ( nChartCol < nColCount )
                                            return ppColHeader[ nChartCol ];
                                        return NULL;
                                    }
            const ScAddress*    GetRowHeaderPosition( SCROW nChartRow ) const
                                    {
                                        if ( nChartRow < nRowCount )
                                            return ppRowHeader[ nChartRow ];
                                        return NULL;
                                    }
//UNUSED2009-05 ScRangeListRef      GetColRanges( SCCOL nChartCol ) const;
//UNUSED2009-05 ScRangeListRef      GetRowRanges( SCROW nChartRow ) const;
};


enum ScChartGlue {
    SC_CHARTGLUE_NA,
    SC_CHARTGLUE_NONE,      // alte Mimik
    SC_CHARTGLUE_COLS,      // alte Mimik
    SC_CHARTGLUE_ROWS,
    SC_CHARTGLUE_BOTH
};

class ScDocument;

class ScChartPositioner             // nur noch Parameter-Struct
{
    ScRangeListRef  aRangeListRef;
    ScDocument* pDocument;
    ScChartPositionMap* pPositionMap;
    ScChartGlue eGlue;
    SCCOL       nStartCol;
    SCROW       nStartRow;
    sal_Bool        bColHeaders;
    sal_Bool        bRowHeaders;
    sal_Bool        bDummyUpperLeft;

private:
    void        CheckColRowHeaders();

    void        GlueState();        // zusammengefasste Bereiche
    void        CreatePositionMap();

public:
    ScChartPositioner( ScDocument* pDoc, SCTAB nTab,
                    SCCOL nStartColP, SCROW nStartRowP,
                    SCCOL nEndColP, SCROW nEndRowP );
    ScChartPositioner( ScDocument* pDoc, const ScRangeListRef& rRangeList );
    ScChartPositioner( const ScChartPositioner& rPositioner );

    virtual ~ScChartPositioner();

    const ScRangeListRef&   GetRangeList() const { return aRangeListRef; }
    void    SetRangeList( const ScRangeListRef& rNew ) { aRangeListRef = rNew; }
    void    SetRangeList( const ScRange& rNew );

    void    SetHeaders(sal_Bool bCol, sal_Bool bRow) { bColHeaders=bCol; bRowHeaders=bRow; }
    sal_Bool    HasColHeaders() const            { return bColHeaders; }
    sal_Bool    HasRowHeaders() const            { return bRowHeaders; }
    void    SetDummyUpperLeft(sal_Bool bNew) { bDummyUpperLeft = bNew; }
    void    SeteGlue(ScChartGlue eNew) { eGlue = eNew; }
    void    SetStartCol(SCCOL nNew) { nStartCol = nNew; }
    void    SetStartRow(SCROW nNew) { nStartRow = nNew; }

    sal_Bool    operator==(const ScChartPositioner& rCmp) const;

    void                    InvalidateGlue()
                                {
                                    eGlue = SC_CHARTGLUE_NA;
                                    if ( pPositionMap )
                                    {
                                        delete pPositionMap;
                                        pPositionMap = NULL;
                                    }
                                }
    const ScChartPositionMap*   GetPositionMap();
};


#endif

