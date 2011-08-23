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

#ifndef SC_DPTABDAT_HXX
#define SC_DPTABDAT_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
namespace binfilter {

class TypedStrCollection;

// -----------------------------------------------------------------------

#define SC_DAPI_DATE_HIERARCHIES	3

#define SC_DAPI_HIERARCHY_FLAT		0
#define SC_DAPI_HIERARCHY_QUARTER	1
#define SC_DAPI_HIERARCHY_WEEK		2

#define SC_DAPI_FLAT_LEVELS		1		// single level for flat dates
#define SC_DAPI_QUARTER_LEVELS	4		// levels in year/quarter/month/day hierarchy
#define SC_DAPI_WEEK_LEVELS		3		// levels in year/week/day hierarchy

#define SC_DAPI_LEVEL_YEAR		0
#define SC_DAPI_LEVEL_QUARTER	1
#define SC_DAPI_LEVEL_MONTH		2
#define SC_DAPI_LEVEL_DAY		3
#define SC_DAPI_LEVEL_WEEK		1
#define SC_DAPI_LEVEL_WEEKDAY	2

// --------------------------------------------------------------------
//
//	base class ScDPTableData to allow implementation with tabular data
//	by deriving only of this
//

struct ScDPItemData
{
    String	aString;
    double	fValue;
    BOOL	bHasValue;

                ScDPItemData() : fValue(0.0), bHasValue(FALSE) {}
                ScDPItemData( const String& rS, double fV = 0.0, BOOL bHV = FALSE ) :
                    aString(rS), fValue(fV), bHasValue( bHV ) {}

    void		SetString( const String& rS ) { aString = rS; bHasValue = FALSE; }

};

#define SC_VALTYPE_EMPTY	0
#define SC_VALTYPE_VALUE	1
#define SC_VALTYPE_STRING	2
#define SC_VALTYPE_ERROR	3

struct ScDPValueData
{
    double	fValue;
    BYTE	nType;

    void	Set( double fV, BYTE nT ) { fValue = fV; nType = nT; }
};


struct ScDPTableIteratorParam
{
    //	all pointers are just copied

    USHORT			nColCount;
    const long*		pCols;
    ScDPItemData*	pColData;
    USHORT			nRowCount;
    const long*		pRows;
    ScDPItemData*	pRowData;
    USHORT			nDatCount;
    const long*		pDats;
    ScDPValueData*	pValues;

};

class ScDPTableData
{
    //	cached data for GetDatePart
    long 	nLastDateVal;
    long 	nLastHier;
    long 	nLastLevel;
    long 	nLastRet;

public:
                ScDPTableData();
    virtual		~ScDPTableData();


                //!	use (new) typed collection instead of StrCollection
                //!	or separate Str and ValueCollection

    virtual long					GetColumnCount() = 0;
    virtual const TypedStrCollection&	GetColumnEntries(long nColumn) = 0;
    virtual String					getDimensionName(long nColumn) = 0;
    virtual BOOL					getIsDataLayoutDimension(long nColumn) = 0;
    virtual BOOL					IsDateDimension(long nDim) = 0;
    virtual void					DisposeData() = 0;
    virtual void					SetEmptyFlags( BOOL bIgnoreEmptyRows, BOOL bRepeatIfEmpty ) = 0;

    virtual void					ResetIterator() = 0;
    virtual BOOL					GetNextRow( const ScDPTableIteratorParam& rParam ) = 0;
};


} //namespace binfilter
#endif

