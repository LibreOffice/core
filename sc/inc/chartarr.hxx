/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chartarr.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:25:57 $
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

#ifndef SC_CHARTARR_HXX
#define SC_CHARTARR_HXX

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
    friend class ScChartArray;

            ScAddress**         ppData;
            ScAddress**         ppColHeader;
            ScAddress**         ppRowHeader;
            SCSIZE              nCount;
            SCSIZE              nColCount;
            SCSIZE              nRowCount;

                                ScChartPositionMap( SCSIZE nChartCols, SCSIZE nChartRows,
                                    SCSIZE nColAdd,     // Header-Spalten
                                    SCSIZE nRowAdd,     // Header-Zeilen
                                    Table& rCols        // Table mit Col-Tables mit Address*
                                    );
                                ~ScChartPositionMap();  //! deletes all ScAddress*

            const ScAddress*    GetPosition( SCSIZE nIndex ) const
                                    {
                                        if ( nIndex < nCount )
                                            return ppData[ nIndex ];
                                        return NULL;
                                    }

                                // not implemented
                                ScChartPositionMap( const ScChartPositionMap& );
            ScChartPositionMap& operator=( const ScChartPositionMap& );

public:

            SCSIZE              GetCount() const { return nCount; }
            SCSIZE              GetColCount() const { return nColCount; }
            SCSIZE              GetRowCount() const { return nRowCount; }

            BOOL                IsValid( SCSIZE nCol, SCSIZE nRow ) const
                                    { return nCol < nColCount && nRow < nRowCount; }
                                // Daten spaltenweise
            SCSIZE              GetIndex( SCSIZE nCol, SCSIZE nRow ) const
                                    { return nCol * nRowCount + nRow; }

                                //! kann NULL sein und damit "kein Wert"
            const ScAddress*    GetPosition( SCSIZE nChartCol, SCSIZE nChartRow ) const
                                    {
                                        if ( IsValid( nChartCol, nChartRow ) )
                                            return ppData[ GetIndex( nChartCol, nChartRow ) ];
                                        return NULL;
                                    }
            const ScAddress*    GetColHeaderPosition( SCSIZE nChartCol ) const
                                    {
                                        if ( nChartCol < nColCount )
                                            return ppColHeader[ nChartCol ];
                                        return NULL;
                                    }
            const ScAddress*    GetRowHeaderPosition( SCSIZE nChartRow ) const
                                    {
                                        if ( nChartRow < nRowCount )
                                            return ppRowHeader[ nChartRow ];
                                        return NULL;
                                    }
            ScRangeListRef      GetColRanges( SCSIZE nChartCol ) const;
            ScRangeListRef      GetRowRanges( SCSIZE nChartRow ) const;
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
class SchMemChart;

class ScChartArray : public DataObject              // nur noch Parameter-Struct
{
    ScRangeListRef  aRangeListRef;
    String      aName;
    ScDocument* pDocument;
    ScChartPositionMap* pPositionMap;
    ScChartGlue eGlue;
    SCCOL       nStartCol;
    SCROW       nStartRow;
    BOOL        bColHeaders;
    BOOL        bRowHeaders;
    BOOL        bDummyUpperLeft;
    BOOL        bValid;             // fuer Erzeugung aus SchMemChart

private:
    void        CheckColRowHeaders();
    SchMemChart* CreateMemChartSingle();
    SchMemChart* CreateMemChartMulti();

    void        GlueState();        // zusammengefasste Bereiche
    void        CreatePositionMap();

public:
    ScChartArray( ScDocument* pDoc, SCTAB nTab,
                    SCCOL nStartCol, SCROW nStartRow,
                    SCCOL nEndCol, SCROW nEndRow,
                    const String& rChartName );
    ScChartArray( ScDocument* pDoc, const ScRangeListRef& rRangeList,
                    const String& rChartName );
    ScChartArray( const ScChartArray& rArr );
    ScChartArray( ScDocument* pDoc, SvStream& rStream, ScMultipleReadHeader& rHdr );
    ScChartArray( ScDocument* pDoc, const SchMemChart& rData );

    virtual ~ScChartArray();
    virtual DataObject* Clone() const;

    const ScRangeListRef&   GetRangeList() const { return aRangeListRef; }
    void    SetRangeList( const ScRangeListRef& rNew ) { aRangeListRef = rNew; }
    void    SetRangeList( const ScRange& rNew );
    void    AddToRangeList( const ScRange& rRange );
    void    AddToRangeList( const ScRangeListRef& rAdd );

    void    SetHeaders(BOOL bCol, BOOL bRow) { bColHeaders=bCol; bRowHeaders=bRow; }
    BOOL    HasColHeaders() const            { return bColHeaders; }
    BOOL    HasRowHeaders() const            { return bRowHeaders; }
    BOOL    IsValid() const                  { return bValid; }
    void    SetName(const String& rNew)      { aName = rNew; }
    const String& GetName() const            { return aName; }

    BOOL    IsAtCursor(const ScAddress& rPos) const;

    BOOL    operator==(const ScChartArray& rCmp) const;

    SchMemChart* CreateMemChart();
    void        SetExtraStrings( SchMemChart& rMem );

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

    static void CopySettings( SchMemChart& rDest, const SchMemChart& rSource );
};

class ScChartCollection : public Collection
{
public:
    ScChartCollection() : Collection( 4,4 ) {}
    ScChartCollection( const ScChartCollection& rColl ):
            Collection( rColl ) {}

    virtual DataObject* Clone() const;
    ScChartArray*       operator[](USHORT nIndex) const
                        { return (ScChartArray*)At(nIndex); }

    BOOL    operator==(const ScChartCollection& rCmp) const;

    BOOL    Load( ScDocument* pDoc, SvStream& rStream );
};



#endif

