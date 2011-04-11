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
                                    SCCOL nColAdd,      // header columns
                                    SCROW nRowAdd,      // header rows
                                    Table& rCols        // table with col tables with address*
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
                                // data column by column
            sal_uLong               GetIndex( SCCOL nCol, SCROW nRow ) const
                                    { return (sal_uLong) nCol * nRowCount + nRow; }

            const ScAddress*    GetPosition( sal_uLong nIndex ) const
                                    {
                                        if ( nIndex < nCount )
                                            return ppData[ nIndex ];
                                        return NULL;
                                    }

                                    //! might be NULL indicating "no value"
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
};


enum ScChartGlue {
    SC_CHARTGLUE_NA,
    SC_CHARTGLUE_NONE,      // old mimic
    SC_CHARTGLUE_COLS,      // old mimic
    SC_CHARTGLUE_ROWS,
    SC_CHARTGLUE_BOTH
};

class ScDocument;

class ScChartPositioner             // only parameter struct
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

    void        GlueState();        // summarised areas
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
