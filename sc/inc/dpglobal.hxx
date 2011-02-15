/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dpglobal.hxx,v $
 * $Revision: 1.0 $
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
// Wang Xu Ming - DataPilot migration
// Buffer&&Performance
//
#ifndef _SC_DPGLOBAL_HXX
#define _SC_DPGLOBAL_HXX

#include <algorithm>
#include <list>
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

#define DBG_TRACESTR( x )  \
    {\
        ByteString aTemp( x , RTL_TEXTENCODING_UTF8 ); \
        DBG_TRACE( aTemp.GetBuffer() );\
    }

class TypedStrData;
class ScDPObject;

class SC_DLLPUBLIC ScDPItemData
{
public:
    enum { MK_VAL = 0x01, MK_DATA = MK_VAL<<1, MK_ERR = MK_DATA<<1, MK_DATE = MK_ERR<<1, MK_DATEPART = MK_DATE<<1 };
private:
    union
    {
        sal_uLong   nNumFormat;
        sal_Int32 mnDatePart;
    };

    String  aString;
    double  fValue;
    sal_uInt8   mbFlag;
    //sal_Bool  bHasValue: 1 ;
    //sal_Bool  bHasData: 1;
    //sal_Bool  bErr: 1;

    friend class ScDPTableDataCache;
public:
    ScDPItemData() : nNumFormat( 0 ), fValue(0.0), mbFlag( 0 ){}
    ScDPItemData( sal_uLong nNF, const String & rS, double fV, sal_uInt8 bF ):nNumFormat(nNF), aString(rS), fValue(fV), mbFlag( bF ){}
    ScDPItemData( const String& rS, double fV = 0.0, sal_Bool bHV = sal_False, const sal_uLong nNumFormat = 0 , sal_Bool bData = sal_True) ;
    ScDPItemData( ScDocument* pDoc, SCROW nRow, sal_uInt16 nCol, sal_uInt16 nDocTab );

    void        SetString( const String& rS ) { aString = rS; mbFlag &= ~(MK_VAL|MK_DATE); nNumFormat = 0; mbFlag |= MK_DATA; }
//  void        SetValue ( double value , sal_uLong nNumFormat = 0 ) { bHasValue = sal_True; nNumFormat = 0;bHasData = sal_True; bDate = sal_False; fValue = value ;}
    sal_Bool        IsCaseInsEqual( const ScDPItemData& r ) const;

    size_t      Hash() const;

    // exact equality
    sal_Bool        operator==( const ScDPItemData& r ) const;
    // case insensitive equality
    static sal_Int32    Compare( const ScDPItemData& rA, const ScDPItemData& rB );

#ifdef DEBUG
    void    dump() const;
#endif

public:
    sal_Bool IsHasData() const ;
    sal_Bool IsHasErr() const ;
    sal_Bool IsValue() const;
    String  GetString() const ;
    double  GetValue() const ;
    sal_uLong    GetNumFormat() const ;
    sal_Bool HasStringData() const ;
    sal_Bool IsDate() const;
    sal_Bool HasDatePart() const;
    void SetDate( sal_Bool b ) ;

    TypedStrData*  CreateTypeString( );
    sal_uInt8    GetType() const;
    sal_uInt8 & GetFlag() throw() { return mbFlag; }
    const sal_uInt8 & GetFlag() const throw() { return const_cast<ScDPItemData*>(this)->GetFlag(); }
};

class SC_DLLPUBLIC ScDPItemDataPool
{
public:
    // construct
    ScDPItemDataPool(void);
    ScDPItemDataPool(const ScDPItemDataPool& r);

    virtual ~ScDPItemDataPool(void);
    virtual const ScDPItemData* getData( sal_Int32 nId  );
    virtual sal_Int32 getDataId( const ScDPItemData& aData );
    virtual sal_Int32 insertData( const ScDPItemData& aData );
protected:
    struct DataHashFunc : public std::unary_function< const ScDPItemData &, size_t >
    {
        size_t operator() (const ScDPItemData &rData) const { return rData.Hash(); }
    };

    typedef ::std::hash_multimap< ScDPItemData, sal_Int32, DataHashFunc > DataHash;

    ::std::vector< ScDPItemData > maItems;
    DataHash  maItemIds;
};

class ScDPInfoWnd;
class ScDocShell;
class ScTabViewShell;
namespace ScDPGlobal
{
// used for core data
    String GetFieldFuncString( const String& rSourceName, sal_uInt16 &rFuncMask, sal_Bool bIsValue );
    String GetFuncString( const String &rString, const sal_uInt16 nIndex );
    com::sun::star::uno::Reference<com::sun::star::container::XNameAccess> DP_GetMembers( const com::sun::star::uno::Reference<
                                                                                      com::sun::star::sheet::XDimensionsSupplier>&rSrc, long nField );
// common operation
    String operator + ( const String & rL, const String &rR );
    Rectangle operator *( const Rectangle &rLeft, const std::pair<double,double> & rRight );
// used for  DataPilot Panel
    ScDPInfoWnd* GetDPInfoWnd( ScTabViewShell *pViewShell );
   bool ChkDPTableOverlap( ScDocument *pDestDoc, std::list<ScDPObject> & rClipboard, SCCOL nClipStartCol, SCROW nClipStartRow, SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab, sal_uInt16 nEndTab, sal_Bool bExcludeClip = sal_False );

}
#endif
