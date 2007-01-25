/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dptabdat.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-25 11:03:15 $
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

#ifndef SC_DPTABDAT_HXX
#define SC_DPTABDAT_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

class TypedStrCollection;

// -----------------------------------------------------------------------

#define SC_DAPI_DATE_HIERARCHIES    3

#define SC_DAPI_HIERARCHY_FLAT      0
#define SC_DAPI_HIERARCHY_QUARTER   1
#define SC_DAPI_HIERARCHY_WEEK      2

#define SC_DAPI_FLAT_LEVELS     1       // single level for flat dates
#define SC_DAPI_QUARTER_LEVELS  4       // levels in year/quarter/month/day hierarchy
#define SC_DAPI_WEEK_LEVELS     3       // levels in year/week/day hierarchy

#define SC_DAPI_LEVEL_YEAR      0
#define SC_DAPI_LEVEL_QUARTER   1
#define SC_DAPI_LEVEL_MONTH     2
#define SC_DAPI_LEVEL_DAY       3
#define SC_DAPI_LEVEL_WEEK      1
#define SC_DAPI_LEVEL_WEEKDAY   2

// --------------------------------------------------------------------
//
//  base class ScDPTableData to allow implementation with tabular data
//  by deriving only of this
//

struct ScDPItemData
{
    String  aString;
    double  fValue;
    BOOL    bHasValue;

                ScDPItemData() : fValue(0.0), bHasValue(FALSE) {}
                ScDPItemData( const String& rS, double fV = 0.0, BOOL bHV = FALSE ) :
                    aString(rS), fValue(fV), bHasValue( bHV ) {}

    void        SetString( const String& rS ) { aString = rS; bHasValue = FALSE; }
    BOOL        IsCaseInsEqual( const ScDPItemData& r ) const;

    size_t      Hash() const;

    // exact equality
    BOOL        operator==( const ScDPItemData& r ) const;
    // case insensitive equality
    static sal_Int32    Compare( const ScDPItemData& rA, const ScDPItemData& rB );
};

#define SC_VALTYPE_EMPTY    0
#define SC_VALTYPE_VALUE    1
#define SC_VALTYPE_STRING   2
#define SC_VALTYPE_ERROR    3

struct ScDPValueData
{
    double  fValue;
    BYTE    nType;

    void    Set( double fV, BYTE nT ) { fValue = fV; nType = nT; }
};


struct ScDPTableIteratorParam
{
    //  all pointers are just copied

    SCSIZE          nColCount;
    const long*     pCols;
    ScDPItemData*   pColData;
    SCSIZE          nRowCount;
    const long*     pRows;
    ScDPItemData*   pRowData;
    SCSIZE          nPageCount;
    const long*     pPages;
    ScDPItemData*   pPageData;
    SCSIZE          nDatCount;
    const long*     pDats;
    ScDPValueData*  pValues;

    ScDPTableIteratorParam( long nCCount, const long* pC, ScDPItemData* pCDat,
                            long nRCount, const long* pR, ScDPItemData* pRDat,
                            long nPCount, const long* pP, ScDPItemData* pPDat,
                            long nDCount, const long* pD, ScDPValueData* pV );
};

class ScDPTableData
{
    //  cached data for GetDatePart
    long    nLastDateVal;
    long    nLastHier;
    long    nLastLevel;
    long    nLastRet;

public:
                ScDPTableData();
    virtual     ~ScDPTableData();

    long        GetDatePart( long nDateVal, long nHierarchy, long nLevel );

                //! use (new) typed collection instead of StrCollection
                //! or separate Str and ValueCollection

    virtual long                    GetColumnCount() = 0;
    virtual const TypedStrCollection&   GetColumnEntries(long nColumn) = 0;
    virtual String                  getDimensionName(long nColumn) = 0;
    virtual BOOL                    getIsDataLayoutDimension(long nColumn) = 0;
    virtual BOOL                    IsDateDimension(long nDim) = 0;
    virtual UINT32                  GetNumberFormat(long nDim);
    virtual void                    DisposeData() = 0;
    virtual void                    SetEmptyFlags( BOOL bIgnoreEmptyRows, BOOL bRepeatIfEmpty ) = 0;

    virtual void                    ResetIterator() = 0;
    virtual BOOL                    GetNextRow( const ScDPTableIteratorParam& rParam ) = 0;

                                    // overloaded in ScDPGroupTableData:
    virtual BOOL                    IsBaseForGroup(long nDim) const;
    virtual long                    GetGroupBase(long nGroupDim) const;
    virtual BOOL                    IsNumOrDateGroup(long nDim) const;
    virtual BOOL                    IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                               const ScDPItemData& rBaseData, long nBaseIndex ) const;
    virtual BOOL                    HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                                      const ScDPItemData& rSecondData, long nSecondIndex ) const;
};


#endif

