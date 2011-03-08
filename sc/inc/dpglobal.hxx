/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
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

#ifndef _SC_DPGLOBAL_HXX
#define _SC_DPGLOBAL_HXX

#include <algorithm>
#include <list>
#include <vector>
#include <tools/gen.hxx>
#include <tools/debug.hxx>
#include <global.hxx>

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>

#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotTableHeaderData.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionData.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionType.hpp>
#include <com/sun/star/sheet/DataPilotTableResultData.hpp>
#include <com/sun/star/sheet/DataResultFlags.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>
#include <com/sun/star/sheet/TableFilterField.hpp>
#include <com/sun/star/sheet/XDataPilotMemberResults.hpp>
#include <com/sun/star/sheet/XDataPilotResults.hpp>
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>


// moved from fieldwnd.hxx, see also SC_DAPI_MAXFIELDS
#define MAX_LABELS  256

#define     PIVOT_MAXFUNC           11
#define     PIVOT_FUNC_NONE         0x0000
#define     PIVOT_FUNC_SUM          0x0001
#define     PIVOT_FUNC_COUNT        0x0002
#define     PIVOT_FUNC_AVERAGE      0x0004
#define     PIVOT_FUNC_MAX          0x0008
#define     PIVOT_FUNC_MIN          0x0010
#define     PIVOT_FUNC_PRODUCT      0x0020
#define     PIVOT_FUNC_COUNT_NUM    0x0040
#define     PIVOT_FUNC_STD_DEV      0x0080
#define     PIVOT_FUNC_STD_DEVP     0x0100
#define     PIVOT_FUNC_STD_VAR      0x0200
#define     PIVOT_FUNC_STD_VARP     0x0400
#define     PIVOT_FUNC_AUTO         0x1000

#define DATA_RENAME_SEPARATOR       "_"
#define __MAX_NUM_LEN 64
#define __DECIMALPLACE  18

#define DP_PROP_COLUMNGRAND         "ColumnGrand"
#define DP_PROP_FUNCTION            "Function"
#define DP_PROP_IGNOREEMPTY         "IgnoreEmptyRows"
#define DP_PROP_ISDATALAYOUT        "IsDataLayoutDimension"
#define DP_PROP_ISVISIBLE           "IsVisible"
#define DP_PROP_ORIENTATION         "Orientation"
#define DP_PROP_REPEATIFEMPTY       "RepeatIfEmpty"
#define DP_PROP_ROWGRAND            "RowGrand"
#define DP_PROP_SHOWDETAILS         "ShowDetails"
#define DP_PROP_SHOWEMPTY           "ShowEmpty"
#define DP_PROP_SUBTOTALS           "SubTotals"
#define DP_PROP_USEDHIERARCHY       "UsedHierarchy"
#define DP_PROP_FILTER              "Filter"
#define DP_PROP_POSITION            "Position"

class TypedStrData;
class ScDPObject;
class ScDPInfoWnd;
class ScDocShell;
class ScTabViewShell;

class SC_DLLPUBLIC ScDPItemData
{
public:
    enum {
        MK_VAL      = 0x01,
        MK_DATA     = 0x02,
        MK_ERR      = 0x04,
        MK_DATE     = 0x08,
        MK_DATEPART = 0x10
    };
private:
    union
    {
        ULONG   nNumFormat;
        sal_Int32 mnDatePart;
    };

    String  aString;
    double  fValue;
    BYTE    mbFlag;

    friend class ScDPCache;
public:
    ScDPItemData() : nNumFormat( 0 ), fValue(0.0), mbFlag( 0 ){}
    ScDPItemData( ULONG nNF, const String & rS, double fV, BYTE bF ):nNumFormat(nNF), aString(rS), fValue(fV), mbFlag( bF ){}
    ScDPItemData( const String& rS, double fV = 0.0, bool bHV = FALSE, const ULONG nNumFormat = 0 , bool bData = TRUE) ;
    ScDPItemData( ScDocument* pDoc, SCROW nRow, USHORT nCol, USHORT nDocTab );

    void        SetString( const String& rS ) { aString = rS; mbFlag &= ~(MK_VAL|MK_DATE); nNumFormat = 0; mbFlag |= MK_DATA; }
    bool        IsCaseInsEqual( const ScDPItemData& r ) const;

    size_t      Hash() const;

    // exact equality
    bool        operator==( const ScDPItemData& r ) const;
    // case insensitive equality
    static sal_Int32    Compare( const ScDPItemData& rA, const ScDPItemData& rB );

#if OSL_DEBUG_LEVEL > 1
    void    dump() const;
#endif

public:
    bool IsHasData() const ;
    bool IsHasErr() const ;
    bool IsValue() const;
    String  GetString() const ;
    double  GetValue() const ;
    ULONG    GetNumFormat() const ;
    bool HasStringData() const ;
    bool IsDate() const;
    bool HasDatePart() const;
    void SetDate( bool b ) ;

    TypedStrData*  CreateTypeString( );
    sal_uInt8    GetType() const;
    BYTE & GetFlag() throw() { return mbFlag; }
    const BYTE & GetFlag() const throw() { return const_cast<ScDPItemData*>(this)->GetFlag(); }
};

class SC_DLLPUBLIC ScDPItemDataPool
{
public:
    ScDPItemDataPool();
    ScDPItemDataPool(const ScDPItemDataPool& r);

    virtual ~ScDPItemDataPool();
    virtual const ScDPItemData* getData( sal_Int32 nId  );
    virtual sal_Int32 getDataId( const ScDPItemData& aData );
    virtual sal_Int32 insertData( const ScDPItemData& aData );
protected:
    struct DataHashFunc : public std::unary_function< const ScDPItemData &, size_t >
    {
        size_t operator() (const ScDPItemData &rData) const { return rData.Hash(); }
    };

    typedef ::boost::unordered_multimap< ScDPItemData, sal_Int32, DataHashFunc > DataHash;

    ::std::vector< ScDPItemData > maItems;
    DataHash  maItemIds;
};

namespace ScDPGlobal
{
// common operation
    String operator + ( const String & rL, const String &rR );
    Rectangle operator *( const Rectangle &rLeft, const std::pair<double,double> & rRight );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
