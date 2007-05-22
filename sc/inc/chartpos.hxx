/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chartpos.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:38:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_CHARTPOS_HXX
#define SC_CHARTPOS_HXX

// -----------------------------------------------------------------------

#ifndef SC_COLLECT_HXX
#include "collect.hxx"
#endif
#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif


class ScAddress;
class Table;

class ScChartPositionMap
{
    friend class ScChartPositioner;

            ScAddress**         ppData;
            ScAddress**         ppColHeader;
            ScAddress**         ppRowHeader;
            ULONG               nCount;
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

            ULONG               GetCount() const { return nCount; }
            SCCOL               GetColCount() const { return nColCount; }
            SCROW               GetRowCount() const { return nRowCount; }

            BOOL                IsValid( SCCOL nCol, SCROW nRow ) const
                                    { return nCol < nColCount && nRow < nRowCount; }
                                // Daten spaltenweise
            ULONG               GetIndex( SCCOL nCol, SCROW nRow ) const
                                    { return (ULONG) nCol * nRowCount + nRow; }

            const ScAddress*    GetPosition( ULONG nIndex ) const
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
            ScRangeListRef      GetColRanges( SCCOL nChartCol ) const;
            ScRangeListRef      GetRowRanges( SCROW nChartRow ) const;
};


enum ScChartGlue {
    SC_CHARTGLUE_NA,
    SC_CHARTGLUE_NONE,      // alte Mimik
    SC_CHARTGLUE_COLS,      // alte Mimik
    SC_CHARTGLUE_ROWS,
    SC_CHARTGLUE_BOTH
};

class ScDocument;
class ScMultipleReadHeader;

class ScChartPositioner             // nur noch Parameter-Struct
{
    ScRangeListRef  aRangeListRef;
    ScDocument* pDocument;
    ScChartPositionMap* pPositionMap;
    ScChartGlue eGlue;
    SCCOL       nStartCol;
    SCROW       nStartRow;
    BOOL        bColHeaders;
    BOOL        bRowHeaders;
    BOOL        bDummyUpperLeft;

private:
    void        CheckColRowHeaders();

    void        GlueState();        // zusammengefasste Bereiche
    void        CreatePositionMap();

public:
    ScChartPositioner( ScDocument* pDoc );
    ScChartPositioner( ScDocument* pDoc, SCTAB nTab,
                    SCCOL nStartColP, SCROW nStartRowP,
                    SCCOL nEndColP, SCROW nEndRowP );
    ScChartPositioner( ScDocument* pDoc, const ScRangeListRef& rRangeList );
    ScChartPositioner( const ScChartPositioner& rPositioner );

    virtual ~ScChartPositioner();

    String  ReadStream( SvStream& rStream, ScMultipleReadHeader& rHdr );

    const ScRangeListRef&   GetRangeList() const { return aRangeListRef; }
    void    SetRangeList( const ScRangeListRef& rNew ) { aRangeListRef = rNew; }
    void    SetRangeList( const ScRange& rNew );
    void    AddToRangeList( const ScRange& rRange );
    void    AddToRangeList( const ScRangeListRef& rAdd );

    void    SetHeaders(BOOL bCol, BOOL bRow) { bColHeaders=bCol; bRowHeaders=bRow; }
    BOOL    HasColHeaders() const            { return bColHeaders; }
    BOOL    HasRowHeaders() const            { return bRowHeaders; }
    void    SetDummyUpperLeft(BOOL bNew) { bDummyUpperLeft = bNew; }
    void    SeteGlue(ScChartGlue eNew) { eGlue = eNew; }
    void    SetStartCol(SCCOL nNew) { nStartCol = nNew; }
    void    SetStartRow(SCROW nNew) { nStartRow = nNew; }

    BOOL    IsAtCursor(const ScAddress& rPos) const;

    BOOL    operator==(const ScChartPositioner& rCmp) const;

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

