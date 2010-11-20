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

#ifndef SC_XIPIVOT_HXX
#define SC_XIPIVOT_HXX

#include <list>
#include "xlpivot.hxx"
#include "xiroot.hxx"
#include <boost/shared_ptr.hpp>

class ScDPSaveData;
class ScDPSaveDimension;

// ============================================================================
// Pivot cache
// ============================================================================

/** Represents a data item in a pivot cache. */
class XclImpPCItem : public XclPCItem
{
public:
    explicit            XclImpPCItem( XclImpStream& rStrm );

    /** Inserts the item data into the passed document. */
    void                WriteToSource( const XclImpRoot& rRoot, const ScAddress& rScPos ) const;

private:
    /** Reads an SXDOUBLE record describing a floating-point item. */
    void                ReadSxdouble( XclImpStream& rStrm );
    /** Reads an SXBOOLEAN record describing a boolean item. */
    void                ReadSxboolean( XclImpStream& rStrm );
    /** Reads an SXERROR record describing an error code item. */
    void                ReadSxerror( XclImpStream& rStrm );
    /** Reads an SXINTEGER record describing an integer item. */
    void                ReadSxinteger( XclImpStream& rStrm );
    /** Reads an SXSTRING record describing a text item. */
    void                ReadSxstring( XclImpStream& rStrm );
    /** Reads an SXDATETIME record describing a date/time item. */
    void                ReadSxdatetime( XclImpStream& rStrm );
    /** Reads an SXEMPTY record describing an empty item. */
    void                ReadSxempty( XclImpStream& rStrm );
};

typedef boost::shared_ptr< XclImpPCItem > XclImpPCItemRef;

// ============================================================================

struct ScDPNumGroupInfo;
class XclImpPivotCache;

/** Represents a field in a pivot cache (a column of data items in the source area). */
class XclImpPCField : public XclPCField, protected XclImpRoot
{
public:
    /** Creates a pivot cache field by reading an SXFIELD record. */
    explicit            XclImpPCField( const XclImpRoot& rRoot,
                            XclImpPivotCache& rPCache, sal_uInt16 nFieldIdx );
    virtual             ~XclImpPCField();

    // general field/item access ----------------------------------------------

    /** Returns the name of the field, uses the passed visible name if supported. */
    const String&       GetFieldName( const ScfStringVec& rVisNames ) const;

    /** Returns the base field if this is a grouping field. */
    const XclImpPCField* GetGroupBaseField() const;

    /** Returns the number of items of this field. */
    sal_uInt16          GetItemCount() const;
    /** Returns the item at the specified position or 0 on error. */
    const XclImpPCItem* GetItem( sal_uInt16 nItemIdx ) const;
    /** Returns the item representing a limit value in numeric/date/time grouping fields.
        @param nItemIdx  One of EXC_SXFIELD_INDEX_MIN, EXC_SXFIELD_INDEX_MAX, or EXC_SXFIELD_INDEX_STEP. */
    const XclImpPCItem* GetLimitItem( sal_uInt16 nItemIdx ) const;

    /** Inserts the field name into the document. */
    void                WriteFieldNameToSource( SCCOL nScCol, SCTAB nScTab ) const;
    /** Inserts the specified item data into the document. */
    void                WriteOrigItemToSource( SCROW nScRow, SCTAB nScTab, sal_uInt16 nItemIdx ) const;
    /** Inserts the data of the last inserted item into the document. */
    void                WriteLastOrigItemToSource( SCROW nScRow, SCTAB nScTab ) const;

    // records ----------------------------------------------------------------

    /** Reads the SXFIELD record describing the field. */
    void                ReadSxfield( XclImpStream& rStrm );
    /** Reads an item data record describing a new item. */
    void                ReadItem( XclImpStream& rStrm );
    /** Reads the SXNUMGROUP record describing numeric grouping fields. */
    void                ReadSxnumgroup( XclImpStream& rStrm );
    /** Reads the SXGROUPINFO record describing the item order in grouping fields. */
    void                ReadSxgroupinfo( XclImpStream& rStrm );

    // grouping ---------------------------------------------------------------

    /** Inserts grouping information of this field into the passed ScDPSaveData. */
    void                ConvertGroupField( ScDPSaveData& rSaveData, const ScfStringVec& rVisNames ) const;

    // ------------------------------------------------------------------------
private:
    /** Inserts standard grouping information of this field into the passed ScDPSaveData. */
    void                ConvertStdGroupField( ScDPSaveData& rSaveData, const ScfStringVec& rVisNames ) const;
    /** Inserts numeric grouping information of this field into the passed ScDPSaveData. */
    void                ConvertNumGroupField( ScDPSaveData& rSaveData, const ScfStringVec& rVisNames ) const;
    /** Inserts date grouping information of this field into the passed ScDPSaveData. */
    void                ConvertDateGroupField( ScDPSaveData& rSaveData, const ScfStringVec& rVisNames ) const;

    /** Returns a Calc struct with numeric grouping data. */
    ScDPNumGroupInfo    GetScNumGroupInfo() const;
    /** Returns a Calc struct with date grouping data. */
    ScDPNumGroupInfo    GetScDateGroupInfo() const;

    /** Returns a limit value for numeric grouping fields. */
    const double*       GetNumGroupLimit( sal_uInt16 nLimitIdx ) const;
    /** Returns a limit value for date grouping fields (minimum/maximum only). */
    const DateTime*     GetDateGroupLimit( sal_uInt16 nLimitIdx ) const;
    /** Returns the step value for date grouping fields. */
    const sal_Int16*    GetDateGroupStep() const;

private:
    typedef ::std::vector< XclImpPCItemRef > XclImpPCItemVec;

    XclImpPivotCache&   mrPCache;           /// Parent pivot cache containing this field.
    XclImpPCItemVec     maItems;            /// List of all displayed data items.
    XclImpPCItemVec     maOrigItems;        /// List of all source data items.
    XclImpPCItemVec     maNumGroupItems;    /// List of items containing numeric grouping limits.
    mutable SCCOL       mnSourceScCol;      /// Column index of source data for this field.
    bool                mbNumGroupInfoRead; /// true = Numeric grouping info read (SXNUMGROUP record).
};

typedef boost::shared_ptr< XclImpPCField > XclImpPCFieldRef;

// ============================================================================

class XclImpPivotCache : protected XclImpRoot
{
public:
    explicit            XclImpPivotCache( const XclImpRoot& rRoot );
    virtual             ~XclImpPivotCache();

    // data access ------------------------------------------------------------

    /** Returns the data source range read from the DCONREF record. */
    inline const ScRange& GetSourceRange() const { return maSrcRange; }

    /** Returns the number of pivot cache fields. */
    sal_uInt16          GetFieldCount() const;
    /** Returns read-only access to a pivot cache field. */
    const XclImpPCField* GetField( sal_uInt16 nFieldIdx ) const;

    // records ----------------------------------------------------------------

    /** Reads an SXIDSTM record containing a pivot cache stream identifier and the pivot cache. */
    void                ReadSxidstm( XclImpStream& rStrm );
    /** Reads an SXVS record containing the source type of the pivot cache. */
    void                ReadSxvs( XclImpStream& rStrm );
    /** Reads a DCONREF record containing the source range of the pivot cache. */
    void                ReadDconref( XclImpStream& rStrm );
    /** Reads the entire pivot cache stream. Uses decrypter from passed stream. */
    void                ReadPivotCacheStream( XclImpStream& rStrm );

    bool                IsRefreshOnLoad() const;

private:
    typedef ::std::vector< XclImpPCFieldRef > XclImpPCFieldVec;

    XclPCInfo           maPCInfo;           /// Pivot cache settings (SXDB record).
    XclImpPCFieldVec    maFields;           /// List of pivot cache fields.
    ScRange             maSrcRange;         /// Source range in the spreadsheet.
    String              maUrl;              /// URL of the source data.
    String              maTabName;          /// Sheet name of the source data.
    sal_uInt16          mnStrmId;           /// Pivot cache stream identifier.
    sal_uInt16          mnSrcType;          /// Source data type.
    bool                mbSelfRef;          /// true = Source data from own document.
};

typedef boost::shared_ptr< XclImpPivotCache > XclImpPivotCacheRef;

// ============================================================================
// Pivot table
// ============================================================================

class XclImpPivotTable;

// ============================================================================

class XclImpPTItem
{
public:
    explicit            XclImpPTItem( const XclImpPCField* pCacheField );

    /** Returns the internal name of the item or 0, if no name could be found. */
    const String*       GetItemName() const;
    /** Returns the displayed name of the item or 0, if no name could be found. */
    const String*       GetVisItemName() const;

    /** Reads an SXVI record containing data of this item. */
    void                ReadSxvi( XclImpStream& rStrm );

    /** Inserts this item into the passed ScDPSaveDimension. */
    void                ConvertItem( ScDPSaveDimension& rSaveDim ) const;

private:
    XclPTItemInfo       maItemInfo;         /// General data for this item.
    const XclImpPCField* mpCacheField;      /// Corresponding pivot cache field.
};

typedef boost::shared_ptr< XclImpPTItem > XclImpPTItemRef;

// ============================================================================

class XclImpPTField
{
public:
    explicit            XclImpPTField( const XclImpPivotTable& rPTable, sal_uInt16 nCacheIdx );

    // general field/item access ----------------------------------------------

    /** Returns the corresponding pivot cache field of this field. */
    const XclImpPCField* GetCacheField() const;
    /** Returns the name of this field that is used to create the Calc dimensions. */
    const String&       GetFieldName() const;
    /** Returns the internally set visible name of this field. */
    const String&       GetVisFieldName() const;

    /** Returns the specified item. */
    const XclImpPTItem* GetItem( sal_uInt16 nItemIdx ) const;
    /** Returns the internal name of the specified item. */
    const String*       GetItemName( sal_uInt16 nItemIdx ) const;
    /** Returns the displayed name of the specified item. */
    const String*       GetVisItemName( sal_uInt16 nItemIdx ) const;

    /** Returns the flags of the axes this field is part of. */
    inline sal_uInt16   GetAxes() const { return maFieldInfo.mnAxes; }
    /** Sets the flags of the axes this field is part of. */
    inline void         SetAxes( sal_uInt16 nAxes ) { maFieldInfo.mnAxes = nAxes; }

    // records ----------------------------------------------------------------

    /** Reads an SXVD record describing the field. */
    void                ReadSxvd( XclImpStream& rStrm );
    /** Reads an SXVDEX record describing extended options of the field. */
    void                ReadSxvdex( XclImpStream& rStrm );
    /** Reads an SXVI record describing a new item of this field. */
    void                ReadSxvi( XclImpStream& rStrm );

    // row/column fields ------------------------------------------------------

    void                ConvertRowColField( ScDPSaveData& rSaveData ) const;

    // page fields ------------------------------------------------------------

    void                SetPageFieldInfo( const XclPTPageFieldInfo& rPageInfo );
    void                ConvertPageField( ScDPSaveData& rSaveData ) const;

    // hidden fields ----------------------------------------------------------

    void                ConvertHiddenField( ScDPSaveData& rSaveData ) const;

    // data fields ------------------------------------------------------------

    bool                HasDataFieldInfo() const;
    void                AddDataFieldInfo( const XclPTDataFieldInfo& rDataInfo );
    void                ConvertDataField( ScDPSaveData& rSaveData ) const;

    // ------------------------------------------------------------------------
private:
    ScDPSaveDimension*  ConvertRCPField( ScDPSaveData& rSaveData ) const;
    void                ConvertFieldInfo( ScDPSaveDimension& rSaveDim ) const;

    void                ConvertDataField( ScDPSaveDimension& rSaveDim, const XclPTDataFieldInfo& rDataInfo ) const;
    void                ConvertDataFieldInfo( ScDPSaveDimension& rSaveDim, const XclPTDataFieldInfo& rDataInfo ) const;
    void                ConvertItems( ScDPSaveDimension& rSaveDim ) const;

private:
    typedef ::std::list< XclPTDataFieldInfo >   XclPTDataFieldInfoList;
    typedef ::std::vector< XclImpPTItemRef >    XclImpPTItemVec;

    const XclImpPivotTable& mrPTable;       /// Parent pivot table containing this field.
    XclPTFieldInfo      maFieldInfo;        /// General field info (SXVD record).
    XclPTFieldExtInfo   maFieldExtInfo;     /// Extended field info (SXVDEX record).
    XclPTPageFieldInfo  maPageInfo;         /// Page field info (entry from SXPI record).
    XclPTDataFieldInfoList maDataInfoList;  /// List of extended data field info (SXDI records).
    XclImpPTItemVec     maItems;            /// List of all items of this field.
};

typedef boost::shared_ptr< XclImpPTField > XclImpPTFieldRef;

// ============================================================================

class XclImpPivotTable : protected XclImpRoot
{
public:
    explicit            XclImpPivotTable( const XclImpRoot& rRoot );
    virtual             ~XclImpPivotTable();

    // cache/field access, misc. ----------------------------------------------

    inline XclImpPivotCacheRef GetPivotCache() const { return mxPCache; }
    inline const ScfStringVec& GetVisFieldNames() const { return maVisFieldNames; }

    sal_uInt16          GetFieldCount() const;
    const XclImpPTField* GetField( sal_uInt16 nFieldIdx ) const;
    XclImpPTField*      GetFieldAcc( sal_uInt16 nFieldIdx );
    const String&       GetFieldName( sal_uInt16 nFieldIdx ) const;

    const XclImpPTField* GetDataField( sal_uInt16 nDataFieldIdx ) const;
    const String&       GetDataFieldName( sal_uInt16 nDataFieldIdx ) const;

    // records ----------------------------------------------------------------

    /** Reads an SXVIEW record starting a new pivot table. */
    void                ReadSxview( XclImpStream& rStrm );
    /** Reads an SXVD record describing a new field. */
    void                ReadSxvd( XclImpStream& rStrm );
    /** Reads an SXVI record describing a new item of the current field. */
    void                ReadSxvi( XclImpStream& rStrm );
    /** Reads an SXVDEX record describing extended options of the current field. */
    void                ReadSxvdex( XclImpStream& rStrm );
    /** Reads an SXIVD record containing the row field or column field order. */
    void                ReadSxivd( XclImpStream& rStrm );
    /** Reads an SXPI record containing page field data. */
    void                ReadSxpi( XclImpStream& rStrm );
    /** Reads an SXDI record containing data field data. */
    void                ReadSxdi( XclImpStream& rStrm );
    /** Reads an SXEX record containing additional settings for the pivot table. */
    void                ReadSxex( XclImpStream& rStrm );
    /** Reads an SXVIEWEX9 record that specifies the pivot tables
     *  autoformat. */
    void                ReadSxViewEx9( XclImpStream& rStrm );

    // ------------------------------------------------------------------------

    /** Inserts the pivot table into the Calc document. */
    void                Convert();

    void                MaybeRefresh();

    void                ApplyMergeFlags(const ScRange& rOutRange, const ScDPSaveData& rSaveData);

    // ------------------------------------------------------------------------
private:
    typedef ::std::vector< XclImpPTFieldRef > XclImpPTFieldVec;

    XclImpPivotCacheRef mxPCache;           /// Pivot cache containing field/item names.

    XclPTInfo           maPTInfo;           /// General info about the pivot table (SXVIEW record).
    XclPTExtInfo        maPTExtInfo;        /// Extended info about the pivot table (SXEX record).
    XclPTViewEx9Info    maPTViewEx9Info;     /// (SXVIEWEX9 record)
    XclImpPTFieldVec    maFields;           /// Vector containing all fields.
    XclImpPTFieldRef    mxCurrField;        /// Current field for importing additional info.
    ScfStringVec        maVisFieldNames;    /// Vector containing all visible field names.
    ScfUInt16Vec        maRowFields;        /// Row field indexes.
    ScfUInt16Vec        maColFields;        /// Column field indexes.
    ScfUInt16Vec        maPageFields;       /// Page field indexes.
    ScfUInt16Vec        maOrigDataFields;   /// Original data field indexes.
    ScfUInt16Vec        maFiltDataFields;   /// Filtered data field indexes.
    XclImpPTField       maDataOrientField;  /// Special data field orientation field.
    ScRange             maOutScRange;       /// Output range in the Calc document.
    ScDPObject*         mpDPObj;
};

typedef boost::shared_ptr< XclImpPivotTable > XclImpPivotTableRef;

// ============================================================================
// ============================================================================

/** The main class for pivot table import.

    This class contains functions to read all records related to pivot tables
    and pivot caches.
 */
class XclImpPivotTableManager : protected XclImpRoot
{
public:
    explicit            XclImpPivotTableManager( const XclImpRoot& rRoot );
    virtual             ~XclImpPivotTableManager();

    // pivot cache records ----------------------------------------------------

    /** Returns the pivot cache with the specified 0-based index. */
    XclImpPivotCacheRef GetPivotCache( sal_uInt16 nCacheIdx );

    /** Reads an SXIDSTM record containing a pivot cache stream identifier and the pivot cache. */
    void                ReadSxidstm( XclImpStream& rStrm );
    /** Reads an SXVS record containing the source type of a pivot cache. */
    void                ReadSxvs( XclImpStream& rStrm );
    /** Reads a DCONREF record containing the source range of a pivot cache. */
    void                ReadDconref( XclImpStream& rStrm );

    // pivot table records ----------------------------------------------------

    /** Reads an SXVIEW record describing a new pivot table. */
    void                ReadSxview( XclImpStream& rStrm );
    /** Reads an SXVD record describing a new field. */
    void                ReadSxvd( XclImpStream& rStrm );
    /** Reads an SXVDEX record describing extended options of a field. */
    void                ReadSxvdex( XclImpStream& rStrm );
    /** Reads an SXIVD record containing the row field or column field order. */
    void                ReadSxivd( XclImpStream& rStrm );
    /** Reads an SXPI record containing page field data. */
    void                ReadSxpi( XclImpStream& rStrm );
    /** Reads an SXDI record containing data field data. */
    void                ReadSxdi( XclImpStream& rStrm );
    /** Reads an SXVI record describing a new item of the current field. */
    void                ReadSxvi( XclImpStream& rStrm );
    /** Reads an SXEX record containing additional settings for a pivot table. */
    void                ReadSxex( XclImpStream& rStrm );
    /** Reads an SXVIEWEX9 record that specifies the pivot tables
     *  autoformat. */
    void                ReadSxViewEx9( XclImpStream& rStrm );

    // ------------------------------------------------------------------------

    /** Reads all used pivot caches and creates additional sheets for external data sources. */
    void                ReadPivotCaches( XclImpStream& rStrm );
    /** Inserts all pivot tables into the Calc document. */
    void                ConvertPivotTables();

    void                MaybeRefreshPivotTables();

private:
    typedef ::std::vector< XclImpPivotCacheRef >    XclImpPivotCacheVec;
    typedef ::std::vector< XclImpPivotTableRef >    XclImpPivotTableVec;

    XclImpPivotCacheVec maPCaches;          /// List of all pivot caches.
    XclImpPivotTableVec maPTables;          /// List of all pivot tables.
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
