/*************************************************************************
 *
 *  $RCSfile: xlpivot.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 12:55:50 $
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

// ============================================================================

#ifndef SC_XLPIVOT_HXX
#define SC_XLPIVOT_HXX

#include <vector>

#ifndef _COM_SUN_STAR_SHEET_GENERALFUNCTION_HPP_
#include <com/sun/star/sheet/GeneralFunction.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDORIENTATION_HPP_
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDSORTMODE_HPP_
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDSHOWITEMSMODE_HPP_
#include <com/sun/star/sheet/DataPilotFieldShowItemsMode.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDLAYOUTMODE_HPP_
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDREFERENCETYPE_HPP_
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATAPILOTFIELDREFERENCEITEMTYPE_HPP_
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

// Constants and Enumerations =================================================

// misc -----------------------------------------------------------------------

const sal_uInt16 EXC_PC_MAXFIELDCOUNT       = 0xFFFE;
const sal_uInt16 EXC_PC_MAXITEMCOUNT        = 32500;

const sal_uInt16 EXC_PT_MAXFIELDCOUNT       = 0xFFFE;
const sal_uInt16 EXC_PT_MAXROWCOLCOUNT      = EXC_PT_MAXFIELDCOUNT;
const sal_uInt16 EXC_PT_MAXPAGECOUNT        = 256;
const sal_uInt16 EXC_PT_MAXDATACOUNT        = 256;
const sal_uInt16 EXC_PT_MAXITEMCOUNT        = 32500;

const sal_uInt16 EXC_PT_NOSTRING            = 0xFFFF;
const sal_uInt16 EXC_PT_MAXSTRLEN           = 0xFFFE;

/** Data type of a pivot cache item. */
enum XclPCItemType
{
    EXC_PCITEM_INVALID,         /// Special state, not used in Excel files.
    EXC_PCITEM_EMPTY,           /// Empty cell.
    EXC_PCITEM_TEXT,            /// String data.
    EXC_PCITEM_VALUE,           /// Floating-point value.
    EXC_PCITEM_DATE,            /// Date/time.
    EXC_PCITEM_BOOL,            /// Boolean value.
    EXC_PCITEM_ERROR            /// Error code.
};

// (0x0051) DCONREF -----------------------------------------------------------

const sal_uInt16 EXC_ID_DCONREF             = 0x0051;

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
const sal_uInt16 EXC_SXVD_AXIS_RC_MASK      = EXC_SXVD_AXIS_ROW | EXC_SXVD_AXIS_COL;
const sal_uInt16 EXC_SXVD_AXIS_RCP_MASK     = EXC_SXVD_AXIS_RC_MASK | EXC_SXVD_AXIS_PAGE;

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

const sal_uInt16 EXC_SXVI_DEFAULT_CACHE     = 0xFFFF;

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

const sal_uInt16 EXC_SXFIELD_POSTPONE       = 0x0002;
const sal_uInt16 EXC_SXFIELD_16BIT          = 0x0200;
const sal_uInt16 EXC_SXFIELD_DEFAULTFLAGS   = 0x0001;

// (0x00C8) SXIDARRAY ---------------------------------------------------------

const sal_uInt16 EXC_ID_SXIDARRAY           = 0x00C8;

// (0x00C9) SXDOUBLE ----------------------------------------------------------

const sal_uInt16 EXC_ID_SXDOUBLE            = 0x00C9;

// (0x00CA) SXBOOLEAN ---------------------------------------------------------

const sal_uInt16 EXC_ID_SXBOOLEAN           = 0x00CA;

// (0x00CB) SXERROR -----------------------------------------------------------

const sal_uInt16 EXC_ID_SXERROR             = 0x00CB;

// (0x00CD) SXSTRING ----------------------------------------------------------

const sal_uInt16 EXC_ID_SXSTRING            = 0x00CD;

// (0x00CE) SXDATETIME --------------------------------------------------------

const sal_uInt16 EXC_ID_SXDATETIME          = 0x00CE;

// (0x00CF) SXEMPTY -----------------------------------------------------------

const sal_uInt16 EXC_ID_SXEMPTY             = 0x00CF;

// (0x00D5) SXIDSTM -----------------------------------------------------------

const sal_uInt16 EXC_ID_SXIDSTM             = 0x00D5;

// (0x00E3) SXVS --------------------------------------------------------------

const sal_uInt16 EXC_ID_SXVS                = 0x00E3;

const sal_uInt16 EXC_SXVS_UNKNOWN           = 0x0000;
const sal_uInt16 EXC_SXVS_SHEET             = 0x0001;
const sal_uInt16 EXC_SXVS_EXTERN            = 0x0002;
const sal_uInt16 EXC_SXVS_CONSOLID          = 0x0004;
const sal_uInt16 EXC_SXVS_PIVOTTAB          = 0x0008;
const sal_uInt16 EXC_SXVS_SCENARIO          = 0x0010;

// (0x00F1) SXEX --------------------------------------------------------------

const sal_uInt16 EXC_ID_SXEX                = 0x00F1;

const sal_uInt32 EXC_SXEX_DRILLDOWN         = 0x00020000;
const sal_uInt32 EXC_SXEX_DEFAULTFLAGS      = 0x004F0200;

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

// (0x0122) SXDBEX ---------------------------------------------------------

const sal_uInt16 EXC_ID_SXDBEX              = 0x0122;
const double EXC_SXDBEX_CREATION_DATE       = 51901.029652778;

// (0x01BB) SXFDBTYPE ---------------------------------------------------------

const sal_uInt16 EXC_ID_SXFDBTYPE           = 0x01BB;
const sal_uInt16 EXC_SXFDBTYPE_DEFAULT      = 0x0000;

// ============================================================================
// Pivot cache
// ============================================================================

class XclPCItem
{
public:
    explicit            XclPCItem();

    void                SetEmpty();
    void                SetText( const String& rText );
    void                SetValue( double fValue );
    void                SetDate( double fDate );
    void                SetError( sal_uInt16 nError );
    void                SetBool( bool bValue );

    inline XclPCItemType GetType() const { return meType; }

    bool                IsEmpty() const;
    const String*       GetText() const;
    const double*       GetValue() const;
    const double*       GetDate() const;
    const sal_uInt16*   GetError() const;
    const bool*         GetBool() const;

    inline const String& ConvertToText() const { return maText; }
    inline double       ConvertToValue() const { return mfValue; }
    inline sal_uInt16   ConvertToError() const { return mnError; }
    inline bool         ConvertToBool() const { return mbValue; }

private:
    XclPCItemType       meType;             /// Type of the item.
    String              maText;             /// Text data of a text item.
    double              mfValue;            /// Value of a floating-point or date item.
    sal_uInt16          mnError;            /// Error code of an error item.
    bool                mbValue;            /// Value of a boolean item.
};

// ============================================================================
// Pivot table
// ============================================================================

class XclImpStream;
class XclExpStream;

// Field item settings ========================================================

/** General information about a data item (SXVI record). */
struct XclPTItemInfo
{
    String              maName;         /// Name of the item (if cache is not used).
    sal_uInt16          mnType;         /// Type of the item (i.e. data, function, grand total).
    sal_uInt16          mnFlags;        /// Several flags.
    sal_uInt16          mnCacheIdx;     /// Index into cache for item name.
    bool                mbUseCache;     /// true = Use name in cache.

    explicit            XclPTItemInfo();
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPTItemInfo& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPTItemInfo& rInfo );

// General field settings =====================================================

typedef ::std::vector< USHORT > XclPTSubtotalVec;

/** General information about a field (SXVD record). */
struct XclPTFieldInfo
{
    String              maVisName;      /// Visible name of the field.
    sal_uInt16          mnAxes;         /// Flags for axes this field is part of.
    sal_uInt16          mnSubtCount;    /// Number of subtotal functions.
    sal_uInt16          mnSubtotals;    /// Bitfield for subtotal functions.
    sal_uInt16          mnItemCount;    /// Number of items of this field.
    sal_uInt16          mnCacheIdx;     /// Index into cache for field name (not part of record).

    explicit            XclPTFieldInfo();

    /** Returns the API enum representing the orientation (first of row/col/page/data). */
    ::com::sun::star::sheet::DataPilotFieldOrientation GetApiOrient() const;
    /** Adds the axis orientation represented by the passed API enum. */
    void                AddApiOrient( ::com::sun::star::sheet::DataPilotFieldOrientation eOrient );

    /** Returns a vector of all set subtotal functions. */
    void                GetSubtotals( XclPTSubtotalVec& rSubtotals ) const;
    /** Sets the subtotal functions contained in the passed sequence. */
    void                SetSubtotals( const XclPTSubtotalVec& rSubtotals );
};

XclImpStream& operator>>( XclImpStream& rStrm, XclPTFieldInfo& rInfo );
XclExpStream& operator<<( XclExpStream& rStrm, const XclPTFieldInfo& rInfo );

// Extended field settings ====================================================

/** Extended information about a field (SXVDEX record). */
struct XclPTFieldExtInfo
{
    sal_uInt32          mnFlags;        /// Several flags and number of items for AutoShow.
    sal_uInt16          mnSortField;    /// Index to data field sorting bases on.
    sal_uInt16          mnShowField;    /// Index to data field AutoShow bases on.

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

/** Additional information about a data field (SXDI record). */
struct XclPTDataFieldInfo
{
    String              maVisName;      /// Visible name of the data field (i.e. "Sum of xyz").
    sal_uInt16          mnField;        /// Base field for this data info.
    sal_uInt16          mnAggFunc;      /// Data aggregation function.
    sal_uInt16          mnRefType;      /// Result reference type.
    sal_uInt16          mnRefField;     /// Index to SXVD of referred field used for the results.
    sal_uInt16          mnRefItem;      /// Index to SXVI of referred item of the used field.
    sal_uInt16          mnNumFmt;       /// Number format of the results.

    explicit            XclPTDataFieldInfo();

    /** Returns the API enum representing the aggregation function. */
    ::com::sun::star::sheet::GeneralFunction GetApiAggFunc() const;
    /** Sets the aggregation function represented by the passed API enum. */
    void                SetApiAggFunc( ::com::sun::star::sheet::GeneralFunction eAggFunc );

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

/** general information about a pivot table (SXVIEW record). */
struct XclPTInfo
{
    String              maTableName;        /// The name of the pivot table.
    String              maDataName;         /// The visible name of the data field.
    sal_uInt16          mnFirstRow;         /// First row of output range.
    sal_uInt16          mnLastRow;          /// Last row of output range.
    sal_uInt16          mnFirstCol;         /// First column of output range.
    sal_uInt16          mnLastCol;          /// Last column of output range.
    sal_uInt16          mnFirstHeadRow;     /// First heading row.
    sal_uInt16          mnFirstDataRow;     /// First row containing data.
    sal_uInt16          mnFirstDataCol;     /// First column containing data.
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

// ============================================================================

#endif

