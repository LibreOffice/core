/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XLPIVOT_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XLPIVOT_HXX

#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldShowItemsMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#include <tools/datetime.hxx>
#include "ftools.hxx"
#include "xladdress.hxx"
#include <dpobject.hxx>

#include <memory>
#include <boost/optional.hpp>

class XclImpStream;
class XclExpStream;
enum class ScGeneralFunction;

// Constants and Enumerations =================================================

// misc -----------------------------------------------------------------------

#define EXC_STORAGE_PTCACHE         "_SX_DB_CUR"

// strings
const sal_uInt16 EXC_PT_NOSTRING            = 0xFFFF;
const sal_uInt16 EXC_PT_MAXSTRLEN           = 0xFFFE;

// pivot cache fields
const size_t EXC_PC_MAXFIELDCOUNT           = 0xFFFE;
const sal_uInt16 EXC_PC_NOFIELD             = 0xFFFF;
const sal_Int32 EXC_PC_MAXSTRLEN           = 255;

// pivot cache items
const size_t EXC_PC_MAXITEMCOUNT            = 32500;
const sal_uInt16 EXC_PC_NOITEM              = 0xFFFF;

// pivot table fields
const sal_uInt16 EXC_PT_MAXFIELDCOUNT       = 0xFFFE;
const sal_uInt16 EXC_PT_MAXROWCOLCOUNT      = EXC_PT_MAXFIELDCOUNT;
const sal_uInt16 EXC_PT_MAXPAGECOUNT        = 256;
const sal_uInt16 EXC_PT_MAXDATACOUNT        = 256;

// pivot table items
const sal_uInt16 EXC_PT_MAXITEMCOUNT        = 32500;

const sal_uInt16 EXC_PT_AUTOFMT_HEADER      = 0x810;
const sal_uInt16 EXC_PT_AUTOFMT_ZERO        = 0;
const sal_uInt32 EXC_PT_AUTOFMT_FLAGS       = 0x20;

/** Data type of a pivot cache item. */
enum XclPCItemType
{
    EXC_PCITEM_INVALID,         /// Special state, not used in Excel files.
    EXC_PCITEM_EMPTY,           /// Empty cell.
    EXC_PCITEM_TEXT,            /// String data.
    EXC_PCITEM_DOUBLE,          /// Floating-point value.
    EXC_PCITEM_DATETIME,        /// Date/time.
    EXC_PCITEM_INTEGER,         /// 16-bit integer value.
    EXC_PCITEM_BOOL,            /// Boolean value.
    EXC_PCITEM_ERROR            /// Error code.
};

/** Specifies the type of a pivot cache field. */
enum XclPCFieldType
{
    EXC_PCFIELD_STANDARD,       /// Standard field without grouping.
    EXC_PCFIELD_STDGROUP,       /// Standard grouping field.
    EXC_PCFIELD_NUMGROUP,       /// Numeric grouping field.
    EXC_PCFIELD_DATEGROUP,      /// First date grouping field (opt. with child grouping field).
    EXC_PCFIELD_DATECHILD,      /// Additional date grouping field.
    EXC_PCFIELD_CALCED,         /// Calculated field.
    EXC_PCFIELD_UNKNOWN         /// Unknown field state, handled like standard field.
};

// (0x0051,0x0052) DCONREF, DCONNAME ------------------------------------------

const sal_uInt16 EXC_ID_DCONREF             = 0x0051;
const sal_uInt16 EXC_ID_DCONNAME            = 0x0052;

// (0x00B0) SXVIEW ------------------------------------------------------------

const sal_uInt16 EXC_ID_SXVIEW              = 0x00B0;

const sal_uInt16 EXC_SXVIEW_ROWGRAND        = 0x0001;
const sal_uInt16 EXC_SXVIEW_COLGRAND        = 0x0002;
const sal_uInt16 EXC_SXVIEW_DEFAULTFLAGS    = 0x0208;

const sal_uInt16 EXC_SXVIEW_DATALAST        = 0xFFFF;
const sal_uInt16 EXC_SXVIEW_AUTOFMT         = 0x0001;

// (0x00B1) SXVD --------------------------------------------------------------
const sal_uInt16 EXC_ID_SXVD                = 0x00B1;

const sal_uInt16 EXC_SXVD_AXIS_NONE         = 0x0000;
const sal_uInt16 EXC_SXVD_AXIS_ROW          = 0x0001;
const sal_uInt16 EXC_SXVD_AXIS_COL          = 0x0002;
const sal_uInt16 EXC_SXVD_AXIS_PAGE         = 0x0004;
const sal_uInt16 EXC_SXVD_AXIS_DATA         = 0x0008;
const sal_uInt16 EXC_SXVD_AXIS_ROWCOL       = EXC_SXVD_AXIS_ROW | EXC_SXVD_AXIS_COL;
const sal_uInt16 EXC_SXVD_AXIS_ROWCOLPAGE   = EXC_SXVD_AXIS_ROWCOL | EXC_SXVD_AXIS_PAGE;

const sal_uInt16 EXC_SXVD_SUBT_NONE         = 0x0000;
const sal_uInt16 EXC_SXVD_SUBT_DEFAULT      = 0x0001;
const sal_uInt16 EXC_SXVD_SUBT_SUM          = 0x0002;
const sal_uInt16 EXC_SXVD_SUBT_COUNT        = 0x0004;
const sal_uInt16 EXC_SXVD_SUBT_AVERAGE      = 0x0008;
const sal_uInt16 EXC_SXVD_SUBT_MAX          = 0x0010;
const sal_uInt16 EXC_SXVD_SUBT_MIN          = 0x0020;
const sal_uInt16 EXC_SXVD_SUBT_PROD         = 0x0040;
const sal_uInt16 EXC_SXVD_SUBT_COUNTNUM     = 0x0080;
const sal_uInt16 EXC_SXVD_SUBT_STDDEV       = 0x0100;
const sal_uInt16 EXC_SXVD_SUBT_STDDEVP      = 0x0200;
const sal_uInt16 EXC_SXVD_SUBT_VAR          = 0x0400;
const sal_uInt16 EXC_SXVD_SUBT_VARP         = 0x0800;

const sal_uInt16 EXC_SXVD_DEFAULT_CACHE     = EXC_PC_NOFIELD;

// (0x00B2) SXVI --------------------------------------------------------------
const sal_uInt16 EXC_ID_SXVI                = 0x00B2;

const sal_uInt16 EXC_SXVI_TYPE_PAGE         = 0x00FE;
const sal_uInt16 EXC_SXVI_TYPE_NULL         = 0x00FF;
const sal_uInt16 EXC_SXVI_TYPE_DATA         = 0x0000;
const sal_uInt16 EXC_SXVI_TYPE_DEFAULT      = 0x0001;
const sal_uInt16 EXC_SXVI_TYPE_SUM          = 0x0002;
const sal_uInt16 EXC_SXVI_TYPE_COUNT        = 0x0003;
const sal_uInt16 EXC_SXVI_TYPE_AVERAGE      = 0x0004;
const sal_uInt16 EXC_SXVI_TYPE_MAX          = 0x0005;
const sal_uInt16 EXC_SXVI_TYPE_MIN          = 0x0006;
const sal_uInt16 EXC_SXVI_TYPE_PROD         = 0x0007;
const sal_uInt16 EXC_SXVI_TYPE_COUNTNUM     = 0x0008;
const sal_uInt16 EXC_SXVI_TYPE_STDDEV       = 0x0009;
const sal_uInt16 EXC_SXVI_TYPE_STDDEVP      = 0x000A;
const sal_uInt16 EXC_SXVI_TYPE_VAR          = 0x000B;
const sal_uInt16 EXC_SXVI_TYPE_VARP         = 0x000C;
const sal_uInt16 EXC_SXVI_TYPE_GRAND        = 0x000D;

const sal_uInt16 EXC_SXVI_DEFAULTFLAGS      = 0x0000;
const sal_uInt16 EXC_SXVI_HIDDEN            = 0x0001;
const sal_uInt16 EXC_SXVI_HIDEDETAIL        = 0x0002;
const sal_uInt16 EXC_SXVI_FORMULA           = 0x0004;
const sal_uInt16 EXC_SXVI_MISSING           = 0x0008;

const sal_uInt16 EXC_SXVI_DEFAULT_CACHE     = EXC_PC_NOFIELD;

// (0x00B4) SXIVD -------------------------------------------------------------
const sal_uInt16 EXC_ID_SXIVD               = 0x00B4;
const sal_uInt16 EXC_SXIVD_DATA             = 0xFFFE;

// (0x00B5) SXLI --------------------------------------------------------------
const sal_uInt16 EXC_ID_SXLI                = 0x00B5;
const sal_uInt16 EXC_SXLI_DEFAULTFLAGS      = 0x0000;

// (0x00B6) SXPI --------------------------------------------------------------
const sal_uInt16 EXC_ID_SXPI                = 0x00B6;
const sal_uInt16 EXC_SXPI_ALLITEMS          = 0x7FFD;

// (0x00C5) SXDI --------------------------------------------------------------
const sal_uInt16 EXC_ID_SXDI                = 0x00C5;

const sal_uInt16 EXC_SXDI_FUNC_SUM          = 0x0000;
const sal_uInt16 EXC_SXDI_FUNC_COUNT        = 0x0001;
const sal_uInt16 EXC_SXDI_FUNC_AVERAGE      = 0x0002;
const sal_uInt16 EXC_SXDI_FUNC_MAX          = 0x0003;
const sal_uInt16 EXC_SXDI_FUNC_MIN          = 0x0004;
const sal_uInt16 EXC_SXDI_FUNC_PRODUCT      = 0x0005;
const sal_uInt16 EXC_SXDI_FUNC_COUNTNUM     = 0x0006;
const sal_uInt16 EXC_SXDI_FUNC_STDDEV       = 0x0007;
const sal_uInt16 EXC_SXDI_FUNC_STDDEVP      = 0x0008;
const sal_uInt16 EXC_SXDI_FUNC_VAR          = 0x0009;
const sal_uInt16 EXC_SXDI_FUNC_VARP         = 0x000A;

const sal_uInt16 EXC_SXDI_REF_NORMAL        = 0x0000;
const sal_uInt16 EXC_SXDI_REF_DIFF          = 0x0001;
const sal_uInt16 EXC_SXDI_REF_PERC          = 0x0002;
const sal_uInt16 EXC_SXDI_REF_PERC_DIFF     = 0x0003;
const sal_uInt16 EXC_SXDI_REF_RUN_TOTAL     = 0x0004;
const sal_uInt16 EXC_SXDI_REF_PERC_ROW      = 0x0005;
const sal_uInt16 EXC_SXDI_REF_PERC_COL      = 0x0006;
const sal_uInt16 EXC_SXDI_REF_PERC_TOTAL    = 0x0007;
const sal_uInt16 EXC_SXDI_REF_INDEX         = 0x0008;

const sal_uInt16 EXC_SXDI_PREVITEM          = 0x7FFB;
const sal_uInt16 EXC_SXDI_NEXTITEM          = 0x7FFC;

// (0x00C6) SXDB --------------------------------------------------------------
const sal_uInt16 EXC_ID_SXDB                = 0x00C6;

const sal_uInt16 EXC_SXDB_SAVEDATA          = 0x0001;
const sal_uInt16 EXC_SXDB_INVALID           = 0x0002;
const sal_uInt16 EXC_SXDB_REFRESH_LOAD      = 0x0004;
const sal_uInt16 EXC_SXDB_OPT_CACHE         = 0x0008;
const sal_uInt16 EXC_SXDB_BG_QUERY          = 0x0010;
const sal_uInt16 EXC_SXDB_ENABLE_REFRESH    = 0x0020;
const sal_uInt16 EXC_SXDB_DEFAULTFLAGS      = EXC_SXDB_SAVEDATA | EXC_SXDB_ENABLE_REFRESH;

const sal_uInt16 EXC_SXDB_BLOCKRECS         = 0x1FFF;

const sal_uInt16 EXC_SXDB_SRC_SHEET         = 0x0001;
const sal_uInt16 EXC_SXDB_SRC_EXTERN        = 0x0002;
const sal_uInt16 EXC_SXDB_SRC_CONSOLID      = 0x0004;
const sal_uInt16 EXC_SXDB_SRC_SCENARIO      = 0x0008;

// (0x00C7) SXFIELD -----------------------------------------------------------
const sal_uInt16 EXC_ID_SXFIELD             = 0x00C7;

const sal_uInt16 EXC_SXFIELD_HASITEMS       = 0x0001;
const sal_uInt16 EXC_SXFIELD_POSTPONE       = 0x0002;
const sal_uInt16 EXC_SXFIELD_CALCED         = 0x0004;
const sal_uInt16 EXC_SXFIELD_HASCHILD       = 0x0008;
const sal_uInt16 EXC_SXFIELD_NUMGROUP       = 0x0010;
const sal_uInt16 EXC_SXFIELD_16BIT          = 0x0200;

const sal_uInt16 EXC_SXFIELD_DATA_MASK      = 0x0DE0;
// known data types
const sal_uInt16 EXC_SXFIELD_DATA_NONE      = 0x0000;   /// Special state for groupings.
const sal_uInt16 EXC_SXFIELD_DATA_STR       = 0x0480;   /// Only strings, nothing else.
const sal_uInt16 EXC_SXFIELD_DATA_INT       = 0x0520;   /// Only integers, opt. with doubles.
const sal_uInt16 EXC_SXFIELD_DATA_DBL       = 0x0560;   /// Only doubles, nothing else.
const sal_uInt16 EXC_SXFIELD_DATA_STR_INT   = 0x05A0;   /// Only strings and integers, opt. with doubles.
const sal_uInt16 EXC_SXFIELD_DATA_STR_DBL   = 0x05E0;   /// Only strings and doubles, nothing else.
const sal_uInt16 EXC_SXFIELD_DATA_DATE      = 0x0900;   /// Only dates, nothing else.
const sal_uInt16 EXC_SXFIELD_DATA_DATE_EMP  = 0x0980;   /// Dates and empty strings, nothing else (?).
const sal_uInt16 EXC_SXFIELD_DATA_DATE_NUM  = 0x0D00;   /// Dates with integers or doubles without strings.
const sal_uInt16 EXC_SXFIELD_DATA_DATE_STR  = 0x0D80;   /// Dates and strings, opt. with integers or doubles.

const sal_uInt16 EXC_SXFIELD_INDEX_MIN      = 0;        /// List index for minimum item in groupings.
const sal_uInt16 EXC_SXFIELD_INDEX_MAX      = 1;        /// List index for maximum item in groupings.
const sal_uInt16 EXC_SXFIELD_INDEX_STEP     = 2;        /// List index for step item in groupings.

// (0x00C8) SXINDEXLIST -------------------------------------------------------
const sal_uInt16 EXC_ID_SXINDEXLIST         = 0x00C8;

// (0x00C9) SXDOUBLE ----------------------------------------------------------
const sal_uInt16 EXC_ID_SXDOUBLE            = 0x00C9;

// (0x00CA) SXBOOLEAN ---------------------------------------------------------
const sal_uInt16 EXC_ID_SXBOOLEAN           = 0x00CA;

// (0x00CB) SXERROR -----------------------------------------------------------
const sal_uInt16 EXC_ID_SXERROR             = 0x00CB;

// (0x00CC) SXINTEGER ---------------------------------------------------------
const sal_uInt16 EXC_ID_SXINTEGER           = 0x00CC;

// (0x00CD) SXSTRING ----------------------------------------------------------
const sal_uInt16 EXC_ID_SXSTRING            = 0x00CD;

// (0x00CE) SXDATETIME --------------------------------------------------------
const sal_uInt16 EXC_ID_SXDATETIME          = 0x00CE;

// (0x00CF) SXEMPTY -----------------------------------------------------------
const sal_uInt16 EXC_ID_SXEMPTY             = 0x00CF;

// (0x00D5) SXIDSTM -----------------------------------------------------------
const sal_uInt16 EXC_ID_SXIDSTM             = 0x00D5;

// (0x00D8) SXNUMGROUP --------------------------------------------------------
const sal_uInt16 EXC_ID_SXNUMGROUP          = 0x00D8;

const sal_uInt16 EXC_SXNUMGROUP_AUTOMIN     = 0x0001;
const sal_uInt16 EXC_SXNUMGROUP_AUTOMAX     = 0x0002;

const sal_uInt16 EXC_SXNUMGROUP_TYPE_SEC    = 1;
const sal_uInt16 EXC_SXNUMGROUP_TYPE_MIN    = 2;
const sal_uInt16 EXC_SXNUMGROUP_TYPE_HOUR   = 3;
const sal_uInt16 EXC_SXNUMGROUP_TYPE_DAY    = 4;
const sal_uInt16 EXC_SXNUMGROUP_TYPE_MONTH  = 5;
const sal_uInt16 EXC_SXNUMGROUP_TYPE_QUART  = 6;
const sal_uInt16 EXC_SXNUMGROUP_TYPE_YEAR   = 7;
const sal_uInt16 EXC_SXNUMGROUP_TYPE_NUM    = 8;

// (0x00D9) SXGROUPINFO -------------------------------------------------------
const sal_uInt16 EXC_ID_SXGROUPINFO         = 0x00D9;

// (0x00DC) SXEXT -------------------------------------------------------------
const sal_uInt16 EXC_ID_SXEXT               = 0x00DC;

// (0x00E3) SXVS --------------------------------------------------------------
const sal_uInt16 EXC_ID_SXVS                = 0x00E3;

const sal_uInt16 EXC_SXVS_UNKNOWN           = 0x0000;
const sal_uInt16 EXC_SXVS_SHEET             = 0x0001;
const sal_uInt16 EXC_SXVS_EXTERN            = 0x0002;
const sal_uInt16 EXC_SXVS_CONSOLID          = 0x0004;
const sal_uInt16 EXC_SXVS_PIVOTTAB          = 0x0008;
const sal_uInt16 EXC_SXVS_SCENARIO          = 0x0010;

// (0x00F0) SXRULE ------------------------------------------------------------
const sal_uInt16 EXC_ID_SXRULE              = 0x00F0;

// (0x00F1) SXEX --------------------------------------------------------------
const sal_uInt16 EXC_ID_SXEX                = 0x00F1;

const sal_uInt32 EXC_SXEX_DRILLDOWN         = 0x00020000;
const sal_uInt32 EXC_SXEX_DEFAULTFLAGS      = 0x004F0200;

// (0x00F2) SXFILT ------------------------------------------------------------
const sal_uInt16 EXC_ID_SXFILT              = 0x00F2;

// (0x00F5) -------------------------------------------------------------------
const sal_uInt16 EXC_ID_00F5                = 0x00F5;   /// Unknown record

// (0x00F6) SXNAME ------------------------------------------------------------
const sal_uInt16 EXC_ID_SXNAME              = 0x00F6;

// (0x00F8) SXPAIR ------------------------------------------------------------
const sal_uInt16 EXC_ID_SXPAIR              = 0x00F8;

// (0x00F9) SXFMLA ------------------------------------------------------------
const sal_uInt16 EXC_ID_SXFMLA              = 0x00F9;

// (0x0100) SXVDEX ------------------------------------------------------------
const sal_uInt16 EXC_ID_SXVDEX              = 0x0100;

const sal_uInt32 EXC_SXVDEX_SHOWALL         = 0x00000001;
const sal_uInt32 EXC_SXVDEX_SORT            = 0x00000200;
const sal_uInt32 EXC_SXVDEX_SORT_ASC        = 0x00000400;
const sal_uInt32 EXC_SXVDEX_AUTOSHOW        = 0x00000800;
const sal_uInt32 EXC_SXVDEX_AUTOSHOW_ASC    = 0x00001000;
const sal_uInt32 EXC_SXVDEX_LAYOUT_REPORT   = 0x00200000;
const sal_uInt32 EXC_SXVDEX_LAYOUT_BLANK    = 0x00400000;
const sal_uInt32 EXC_SXVDEX_LAYOUT_TOP      = 0x00800000;
const sal_uInt32 EXC_SXVDEX_DEFAULTFLAGS    = 0x0A00001E | EXC_SXVDEX_SORT_ASC | EXC_SXVDEX_AUTOSHOW_ASC;

const sal_uInt16 EXC_SXVDEX_SORT_OWN        = 0xFFFF;
const sal_uInt16 EXC_SXVDEX_SHOW_NONE       = 0xFFFF;
const sal_uInt16 EXC_SXVDEX_FORMAT_NONE     = 0x0000;

// (0x0103) SXFORMULA ---------------------------------------------------------
const sal_uInt16 EXC_ID_SXFORMULA           = 0x0103;

// (0x0122) SXDBEX ------------------------------------------------------------
const sal_uInt16 EXC_ID_SXDBEX              = 0x0122;
const double EXC_SXDBEX_CREATION_DATE       = 51901.029652778;

// (0x01BB) SXFDBTYPE ---------------------------------------------------------
const sal_uInt16 EXC_ID_SXFDBTYPE           = 0x01BB;
const sal_uInt16 EXC_SXFDBTYPE_DEFAULT      = 0x0000;

// (0x0810) SXVIEWEX9 ---------------------------------------------------------
const sal_uInt16 EXC_ID_SXVIEWEX9       = 0x0810;

// (0x0864) SXADDL ("Pivot Table Additional Info") ----------------------------
const sal_uInt16 EXC_ID_SXADDL       = 0x0864;

// Pivot cache

/** Represents a data item of any type in a pivot cache. Supposed as base class for import and export. */
class XclPCItem
{
public:
    explicit            XclPCItem();
    virtual             ~XclPCItem();

    XclPCItem(XclPCItem const &) = default;
    XclPCItem(XclPCItem &&) = default;
    XclPCItem & operator =(XclPCItem const &) = default;
    XclPCItem & operator =(XclPCItem &&) = default;

    /** Sets the item to 'empty' type. */
    void                SetEmpty();
    /** Sets the item to 'text' type and adds the passed text. */
    void                SetText( const OUString& rText );
    /** Sets the item to 'double' type and adds the passed value. */
    void                SetDouble( double fValue, const OUString& rText = OUString() );
    /** Sets the item to 'date/time' type and adds the passed date. */
    void                SetDateTime( const DateTime& rDateTime, const OUString& rText = OUString() );
    /** Sets the item to 'integer' type and adds the passed value. */
    void                SetInteger( sal_Int16 nValue );
    /** Sets the item to 'error' type and adds the passed Excel error code. */
    void                SetError( sal_uInt16 nError );
    /** Sets the item to 'boolean' type and adds the passed Boolean value. */
    void                SetBool( bool bValue, const OUString& rText = OUString() );

    /** Returns the text representation of the item. */
    const OUString& ConvertToText() const { return maText; }

    /** Returns true, if the passed term equals this item. */
    bool                IsEqual( const XclPCItem& rItem ) const;

    /** Returns true, if the item type is 'empty'. */
    bool                IsEmpty() const;
    /** Returns pointer to text, if the item type is 'text', otherwise 0. */
    const OUString* GetText() const;
    /** Returns pointer to value, if the item type is 'double', otherwise 0. */
    const double*       GetDouble() const;
    /** Returns pointer to date, if the item type is 'date/time', otherwise 0. */
    const DateTime*     GetDateTime() const;
    /** Returns pointer to integer, if the item type is 'integer', otherwise 0. */
    const sal_Int16*    GetInteger() const;
    /** Returns pointer to error code, if the item type is 'error', otherwise 0. */
    const sal_uInt16*   GetError() const;
    /** Returns pointer to Boolean value, if the item type is 'boolean', otherwise 0. */
    const bool*         GetBool() const;

    /** Returns the type of the item */
    XclPCItemType GetType() const;

private:
    XclPCItemType       meType;         /// Type of the item.
    OUString       maText;         /// Text representation of the item.
    DateTime            maDateTime;     /// Value of a date/time item.
    union
    {
        double              mfValue;        /// Value of a floating-point item.
        sal_Int16           mnValue;        /// Value of an integer item.
        sal_uInt16          mnError;        /// Error code of an error item.
        bool                mbValue;        /// Value of a boolean item.
    };
};

inline bool operator==( const XclPCItem& rLeft, const XclPCItem& rRight ) { return rLeft.IsEqual( rRight ); }
inline bool operator!=( const XclPCItem& rLeft, const XclPCItem& rRight ) { return !(rLeft == rRight); }

// Field settings =============================================================

/** Contains data for a pivot cache field (SXFIELD record). */
struct XclPCFieldInfo
{
    OUString       maName;         /// Name of the pivot cache field.
    sal_uInt16          mnFlags;        /// Various flags.
    sal_uInt16          mnGroupChild;   /// Field containing grouping info for this field.
    sal_uInt16          mnGroupBase;    /// Base field if this field contains grouping info.
    sal_uInt16          mnVisItems;     /// Number of visible items for this field.
    sal_uInt16          mnGroupItems;   /// Number of special items in a grouping field.
    sal_uInt16          mnBaseItems;    /// Number of items in the base field.
    sal_uInt16          mnOrigItems;    /// Number of original source data items.

    explicit            XclPCFieldInfo();
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPCFieldInfo& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPCFieldInfo& rInfo );

// Numeric grouping field settings ============================================

/** Contains data for a numeric grouping field (SXNUMGROUP record). */
struct XclPCNumGroupInfo
{
    sal_uInt16          mnFlags;        /// Various flags.

    explicit            XclPCNumGroupInfo();

    void                SetNumType();

    sal_Int32           GetScDateType() const;
    void                SetScDateType( sal_Int32 nScType );

    sal_uInt16          GetXclDataType() const;
    void                SetXclDataType( sal_uInt16 nXclType );
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPCNumGroupInfo& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPCNumGroupInfo& rInfo );

// Base class for pivot cache fields ==========================================

/** Represents a field in a pivot cache. Supposed as base class for import and export. */
class XclPCField
{
public:
    explicit            XclPCField( XclPCFieldType eFieldType, sal_uInt16 nFieldIdx );
    virtual             ~XclPCField();

    /** Returns the index of this field in the containing pivot cache. */
    sal_uInt16   GetFieldIndex() const { return mnFieldIdx; }

    /** Returns true, if the type of the field is supported by Calc. */
    bool                IsSupportedField() const;

    /** Returns true, if this is a standard field build directly from source data. */
    bool                IsStandardField() const;

    /** Returns true, if this field is a grouping field. */
    bool                IsStdGroupField() const;
    /** Returns true, if this field is a numeric grouping field. */
    bool                IsNumGroupField() const;
    /** Returns true, if this field is a date/time grouping field. */
    bool                IsDateGroupField() const;
    /** Returns true, if this field is a grouping field of any type. */
    bool                IsGroupField() const;

    /** Returns true, if this field has a child field in a grouping. */
    bool                IsGroupBaseField() const;
    /** Returns true, if this field is a child field in a grouping (it has a base field). */
    bool                IsGroupChildField() const;

    /** Returns true, if the field is based on a column in the source data area. */
    bool                HasOrigItems() const;
    /** Returns true, if any items are stored after the SXFIELD record. */
    bool                HasInlineItems() const;
    /** Returns true, if the items are stored separately after the last field. */
    bool                HasPostponedItems() const;
    /** Returns true, if the item indexes in the SXINDEXLIST record are stored as 16-bit values. */
    bool                Has16BitIndexes() const;

protected:
    XclPCFieldInfo      maFieldInfo;        /// Pivot cache field info (SXFIELD record).
    XclPCFieldType      meFieldType;        /// Type of this pivot cache field.
    sal_uInt16          mnFieldIdx;         /// Own field index in pivot cache.
    ScfUInt16Vec        maGroupOrder;       /// Order of items in a grouping field (SXGROUPINFO record).
    XclPCNumGroupInfo   maNumGroupInfo;     /// Info for numeric grouping (SXNUMGROUP record).
};

// Pivot cache settings =======================================================

/** Contains data for a pivot cache (SXDB record). */
struct XclPCInfo
{
    sal_uInt32          mnSrcRecs;      /// Records in source database.
    sal_uInt16          mnStrmId;       /// Stream identifier.
    sal_uInt16          mnFlags;        /// Flags for the cache.
    sal_uInt16          mnBlockRecs;    /// Records in a source database block.
    sal_uInt16          mnStdFields;    /// Number of standard pivot cache fields.
    sal_uInt16          mnTotalFields;  /// Number of all fields (standard, grouped, calculated).
    sal_uInt16          mnSrcType;      /// Database type.
    OUString       maUserName;     /// Name of user who last modified the cache.

    explicit            XclPCInfo();
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPCInfo& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPCInfo& rInfo );

// Pivot table

// cached name ================================================================

/** A name for various pivot table info structs. Includes 'use cache' state. */
struct XclPTCachedName
{
    OUString       maName;         /// The visible name, if used.
    bool                mbUseCache;     /// true = Use name in cache instead of maName.

    explicit     XclPTCachedName() : mbUseCache( true ) {}
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPTCachedName& rCachedName );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPTCachedName& rCachedName );

/** Base struct for named info structs. Supports explicit naming and using the cache. */
struct XclPTVisNameInfo
{
    XclPTCachedName     maVisName;      /// The displayed name of the item.

    /** Returns true, if the name is set explicitly (maVisName.mbUseCache is false). */
    bool         HasVisName() const { return !maVisName.mbUseCache; }
    /** Returns the name, if set explicitly (maVisName.mbUseCache is false). */
    const OUString* GetVisName() const;
    /** Sets the visible name and enables usage of cache if name is empty. */
    void                SetVisName( const OUString& rName );
};

// Field item settings ========================================================

/** Contains data for a pivot table data item (SXVI record). */
struct XclPTItemInfo : public XclPTVisNameInfo
{
    sal_uInt16          mnType;         /// Type of the item (e.g. data, function, grand total).
    sal_uInt16          mnFlags;        /// Several flags.
    sal_uInt16          mnCacheIdx;     /// Index into cache for item name.

    explicit            XclPTItemInfo();
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPTItemInfo& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPTItemInfo& rInfo );

// General field settings =====================================================

typedef ::std::vector< ScGeneralFunction > XclPTSubtotalVec;

/** Contains data for a pivot table field (SXVD record). */
struct XclPTFieldInfo : public XclPTVisNameInfo
{
    sal_uInt16          mnAxes;         /// Flags for axes this field is part of.
    sal_uInt16          mnSubtCount;    /// Number of subtotal functions.
    sal_uInt16          mnSubtotals;    /// Bitfield for subtotal functions.
    sal_uInt16          mnItemCount;    /// Number of items of this field.
    sal_uInt16          mnCacheIdx;     /// Index into cache for field name (not part of record).

    explicit            XclPTFieldInfo();

    /** Returns the API enum representing the orientation (first of row/col/page/data).
        @param nMask  Restricts the axes taken into account.
        @return  The first found axis orientation, that is allowed in nMask parameter. */
    css::sheet::DataPilotFieldOrientation GetApiOrient( sal_uInt16 nMask ) const;
    /** Adds the axis orientation represented by the passed API enum. */
    void                AddApiOrient( css::sheet::DataPilotFieldOrientation eOrient );

    /** Returns a vector of all set subtotal functions. */
    void                GetSubtotals( XclPTSubtotalVec& rSubtotals ) const;
    /** Sets the subtotal functions contained in the passed sequence. */
    void                SetSubtotals( const XclPTSubtotalVec& rSubtotals );
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPTFieldInfo& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPTFieldInfo& rInfo );

// Extended field settings ====================================================

/** Contains extended data for a pivot table field (SXVDEX record). */
struct XclPTFieldExtInfo
{
    sal_uInt32          mnFlags;        /// Several flags and number of items for AutoShow.
    sal_uInt16          mnSortField;    /// Index to data field sorting bases on.
    sal_uInt16          mnShowField;    /// Index to data field AutoShow bases on.
    sal_uInt16          mnNumFmt;
    boost::optional<OUString> mpFieldTotalName;

    explicit            XclPTFieldExtInfo();

    /** Returns the API constant representing the sorting mode. */
    sal_Int32           GetApiSortMode() const;
    /** Sets the sorting mode represented by the passed API constant. */
    void                SetApiSortMode( sal_Int32 nSortMode );

    /** Returns the API constant representing the AutoShow mode. */
    sal_Int32           GetApiAutoShowMode() const;
    /** Sets the AutoShow mode represented by the passed API constant. */
    void                SetApiAutoShowMode( sal_Int32 nShowMode );

    /** Returns the number of items to be shown in AutoShow mode. */
    sal_Int32           GetApiAutoShowCount() const;
    /** Sets the number of items to be shown in AutoShow mode. */
    void                SetApiAutoShowCount( sal_Int32 nShowCount );

    /** Returns the API constant representing the layout mode. */
    sal_Int32           GetApiLayoutMode() const;
    /** Sets the layout mode represented by the passed API constant. */
    void                SetApiLayoutMode( sal_Int32 nLayoutMode );
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPTFieldExtInfo& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPTFieldExtInfo& rInfo );

// Page field settings ========================================================

/** Contains data for a pivot table page field (part of SXPI record). */
struct XclPTPageFieldInfo
{
    sal_uInt16          mnField;        /// Base field for this page info.
    sal_uInt16          mnSelItem;      /// Index to selected item.
    sal_uInt16          mnObjId;        /// Escher object ID of dropdown listbox.

    explicit            XclPTPageFieldInfo();
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPTPageFieldInfo& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPTPageFieldInfo& rInfo );

// Data field settings ========================================================

/** Contains data for a pivot table data field (SXDI record). */
struct XclPTDataFieldInfo : public XclPTVisNameInfo
{
    sal_uInt16          mnField;        /// Base field for this data info.
    sal_uInt16          mnAggFunc;      /// Data aggregation function.
    sal_uInt16          mnRefType;      /// Result reference type.
    sal_uInt16          mnRefField;     /// Index to SXVD of referred field used for the results.
    sal_uInt16          mnRefItem;      /// Index to SXVI of referred item of the used field.
    sal_uInt16          mnNumFmt;       /// Number format of the results.

    explicit            XclPTDataFieldInfo();

    /** Returns the API enum representing the aggregation function. */
    ScGeneralFunction GetApiAggFunc() const;
    /** Sets the aggregation function represented by the passed API enum. */
    void                SetApiAggFunc( ScGeneralFunction eAggFunc );

    /** Returns the API constant representing the result reference type. */
    sal_Int32           GetApiRefType() const;
    /** Sets the result reference type represented by the passed API constant. */
    void                SetApiRefType( sal_Int32 nRefType );

    /** Returns the API constant representing the result reference item type. */
    sal_Int32           GetApiRefItemType() const;
    /** Sets the result reference item type represented by the passed API constant. */
    void                SetApiRefItemType( sal_Int32 nRefItemType );
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPTDataFieldInfo& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPTDataFieldInfo& rInfo );

// Pivot table settings =======================================================

/** Contains data for a pivot table (SXVIEW record). */
struct XclPTInfo
{
    OUString       maTableName;        /// The name of the pivot table.
    OUString       maDataName;         /// The visible name of the data field.
    XclRange            maOutXclRange;      /// Output range.
    XclAddress          maDataXclPos;       /// First cell containing data.
    sal_uInt16          mnFirstHeadRow;     /// First heading row.
    sal_uInt16          mnCacheIdx;         /// 0-based index of the pivot cache.
    sal_uInt16          mnDataAxis;         /// Orientation of data fields.
    sal_uInt16          mnDataPos;          /// Position of data fields.
    sal_uInt16          mnFields;           /// Number of all fields.
    sal_uInt16          mnRowFields;        /// Number of row fields.
    sal_uInt16          mnColFields;        /// Number of column fields.
    sal_uInt16          mnPageFields;       /// Number of page fields.
    sal_uInt16          mnDataFields;       /// Number of data fields.
    sal_uInt16          mnDataRows;         /// Number of rows containing data.
    sal_uInt16          mnDataCols;         /// Number of columns containing data.
    sal_uInt16          mnFlags;            /// Flags for the entire pivot table.
    sal_uInt16          mnAutoFmtIdx;       /// Index to pivot table autoformat.

    explicit            XclPTInfo();
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPTInfo& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPTInfo& rInfo );

// Extended pivot table settings ==============================================

/** Extended information about a pivot table (SXEX record). */
struct XclPTExtInfo
{
    sal_uInt16          mnSxformulaRecs;    /// Number of SXFORMULA records.
    sal_uInt16          mnSxselectRecs;     /// Number of SXSELECT records.
    sal_uInt16          mnPagePerRow;       /// Number of page fields per row.
    sal_uInt16          mnPagePerCol;       /// Number of page fields per column.
    sal_uInt32          mnFlags;            /// Flags for the entire pivot table.

    explicit            XclPTExtInfo();
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPTExtInfo& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPTExtInfo& rInfo );

// Pivot table autoformat settings ==============================================

/** Pivot table autoformat settings (SXVIEWEX9 record). */
struct XclPTViewEx9Info
{
    sal_uInt32          mbReport;           /// 2 for report* fmts ?
    sal_uInt8           mnAutoFormat;       /// AutoFormat ID
    sal_uInt8           mnGridLayout;       /// 0 == gridlayout, 0x10 == modern
    OUString       maGrandTotalName;

    explicit            XclPTViewEx9Info();
    void                Init( const ScDPObject& rDPObj );
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPTViewEx9Info& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPTViewEx9Info& rInfo );

/** Additional pivot table settings (SXADDL record). */
struct XclPTAddl
{
    bool          mbCompactMode;
    explicit      XclPTAddl();
};

XclImpStream& operator>>(XclImpStream& rStrm, XclPTAddl& rInfo);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
