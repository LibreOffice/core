/*************************************************************************
 *
 *  $RCSfile: chartarr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
            ULONG               nCount;
            USHORT              nColCount;
            USHORT              nRowCount;

                                ScChartPositionMap( USHORT nChartCols, USHORT nChartRows,
                                    USHORT nColAdd,     // Header-Spalten
                                    USHORT nRowAdd,     // Header-Zeilen
                                    Table& rCols        // Table mit Col-Tables mit Address*
                                    );
                                ~ScChartPositionMap();  //! deletes all ScAddress*

            const ScAddress*    GetPosition( ULONG nIndex ) const
                                    {
                                        if ( nIndex < nCount )
                                            return ppData[ nIndex ];
                                        return NULL;
                                    }

                                // not implemented
                                ScChartPositionMap( const ScChartPositionMap& );
            ScChartPositionMap& operator=( const ScChartPositionMap& );

public:

            ULONG               GetCount() const { return nCount; }
            USHORT              GetColCount() const { return nColCount; }
            USHORT              GetRowCount() const { return nRowCount; }

            BOOL                IsValid( USHORT nCol, USHORT nRow ) const
                                    { return nCol < nColCount && nRow < nRowCount; }
                                // Daten spaltenweise
            ULONG               GetIndex( USHORT nCol, USHORT nRow ) const
                                    { return (ULONG) nCol * nRowCount + nRow; }

                                //! kann NULL sein und damit "kein Wert"
            const ScAddress*    GetPosition( USHORT nChartCol, USHORT nChartRow ) const
                                    {
                                        if ( IsValid( nChartCol, nChartRow ) )
                                            return ppData[ GetIndex( nChartCol, nChartRow ) ];
                                        return NULL;
                                    }
            const ScAddress*    GetColHeaderPosition( USHORT nChartCol ) const
                                    {
                                        if ( nChartCol < nColCount )
                                            return ppColHeader[ nChartCol ];
                                        return NULL;
                                    }
            const ScAddress*    GetRowHeaderPosition( USHORT nChartRow ) const
                                    {
                                        if ( nChartRow < nRowCount )
                                            return ppRowHeader[ nChartRow ];
                                        return NULL;
                                    }
            ScRangeListRef      GetColRanges( USHORT nChartCol ) const;
            ScRangeListRef      GetRowRanges( USHORT nChartRow ) const;
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
    USHORT      nStartCol;
    USHORT      nStartRow;
    BOOL        bColHeaders;
    BOOL        bRowHeaders;
    BOOL        bDummyUpperLeft;
    BOOL        bValid;             // fuer Erzeugung aus SchMemChart

private:
    void        CheckColRowHeaders();
    SchMemChart* CreateMemChartSingle() const;
    SchMemChart* CreateMemChartMulti();

    void        GlueState();        // zusammengefasste Bereiche
    void        CreatePositionMap();

public:
    ScChartArray( ScDocument* pDoc, USHORT nTab,
                    USHORT nStartCol, USHORT nStartRow,
                    USHORT nEndCol, USHORT nEndRow,
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
    void        SetExtraStrings(SchMemChart& rMem) const;

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

