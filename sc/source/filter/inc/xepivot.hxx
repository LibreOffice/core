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

#pragma once

#include "xerecord.hxx"
#include "xlpivot.hxx"
#include "xeroot.hxx"

class ScDPObject;
class ScDPSaveData;
class ScDPSaveDimension;
class ScDPSaveMember;
class ScDPSaveGroupDimension;
struct ScDPNumGroupInfo;

// Pivot cache

/** Represents a data item in a pivot cache containing data of any type. */
class XclExpPCItem : public XclExpRecord, public XclPCItem
{
public:
    explicit            XclExpPCItem( const OUString& rText );
    explicit            XclExpPCItem( double fValue, const OUString& rText = OUString() );
    explicit            XclExpPCItem( const DateTime& rDateTime, const OUString& rText = OUString() );
    explicit            XclExpPCItem( sal_Int16 nValue );
    explicit            XclExpPCItem( bool bValue, const OUString& rText );

    sal_uInt16   GetTypeFlag() const { return mnTypeFlag; }

    bool                EqualsText( std::u16string_view rText ) const;
    bool                EqualsDouble( double fValue ) const;
    bool                EqualsDateTime( const DateTime& rDateTime ) const;
    bool                EqualsBool( bool bValue ) const;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    sal_uInt16          mnTypeFlag;         /// Data type flag.
};

class XclExpPCField : public XclExpRecord, public XclPCField, protected XclExpRoot
{
public:
    /** Creates a standard pivot cache field, filled from sheet source data. */
    explicit            XclExpPCField( const XclExpRoot& rRoot,
                            sal_uInt16 nFieldIdx,
                            const ScDPObject& rDPObj, const ScRange& rRange );
    /** Creates a child grouping pivot cache field, filled from the passed grouping info. */
    explicit            XclExpPCField( const XclExpRoot& rRoot,
                            sal_uInt16 nFieldIdx,
                            const ScDPObject& rDPObj, const ScDPSaveGroupDimension& rGroupDim,
                            const XclExpPCField& rBaseField );
    virtual             ~XclExpPCField() override;

    /** Sets the passed field as direct grouping child field of this field. */
    void                SetGroupChildField( const XclExpPCField& rChildField );

    /** Returns the name of this cache field. */
    const OUString& GetFieldName() const { return maFieldInfo.maName; }

    /** Returns the number of visible items of this field. */
    sal_uInt16          GetItemCount() const;
    /** Returns the specified pivot cache item (returns visible items in groupings). */
    const XclExpPCItem* GetItem( sal_uInt16 nItemIdx ) const;
    /** Returns the index of a pivot cache item, or EXC_PC_NOITEM on error. */
    sal_uInt16          GetItemIndex( std::u16string_view rItemName ) const;

    /** Returns the size an item index needs to write out. */
    std::size_t         GetIndexSize() const;
    /** Writes the item index at the passed source row position as part of the SXINDEXLIST record. */
    void                WriteIndex( XclExpStream& rStrm, sal_uInt32 nSrcRow ) const;

    /** Writes the pivot cache field and all items and other related records. */
    virtual void        Save( XclExpStream& rStrm ) override;

private:
    typedef XclExpRecordList< XclExpPCItem >    XclExpPCItemList;

    /** Returns the item list that contains the visible items.
        @descr  Visible items are equal to source items in standard fields,
            but are generated items in grouping and calculated fields. */
    const XclExpPCItemList& GetVisItemList() const;

    /** Initializes a standard field. Inserts all original source items. */
    void                InitStandardField( const ScRange& rRange );
    /** Initializes a standard grouping field. Inserts all visible grouping items. */
    void                InitStdGroupField( const XclExpPCField& rBaseField, const ScDPSaveGroupDimension& rGroupDim );
    /** Initializes a numeric grouping field. Inserts all visible grouping items and the limit settings. */
    void                InitNumGroupField( const ScDPObject& rDPObj, const ScDPNumGroupInfo& rNumInfo );
    /** Initializes a date grouping field. Inserts all visible grouping items and the limit settings. */
    void                InitDateGroupField( const ScDPObject& rDPObj, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nDatePart );

    /** Inserts the passed index into the item index array of original items. */
    void                InsertItemArrayIndex( size_t nListPos );
    /** Inserts an original source item. Updates item index array. */
    void                InsertOrigItem( XclExpPCItem* pNewItem );
    /** Inserts an original text item, if it is not contained already. */
    void                InsertOrigTextItem( const OUString& rText );
    /** Inserts an original value item, if it is not contained already. */
    void                InsertOrigDoubleItem( double fValue, const OUString& rText );
    /** Inserts an original date/time item, if it is not contained already. */
    void                InsertOrigDateTimeItem( const DateTime& rDateTime, const OUString& rText );
    /** Inserts an original boolean item, if it is not contained already. */
    void                InsertOrigBoolItem( bool bValue, const OUString& rText );

    /** Inserts an item into the grouping item list. Does not change anything else.
        @return  The list index of the new item. */
    sal_uInt16          InsertGroupItem( XclExpPCItem* pNewItem );
    /** Generates and inserts all visible items for numeric or date grouping. */
    void                InsertNumDateGroupItems( const ScDPObject& rDPObj, const ScDPNumGroupInfo& rNumInfo, sal_Int32 nDatePart = 0 );

    /** Inserts the SXDOUBLE items that specify the limits for a numeric grouping. */
    void                SetNumGroupLimit( const ScDPNumGroupInfo& rNumInfo );
    /** Inserts the SXDATETIME/SXINTEGER items that specify the limits for a date grouping.
        @param bUseStep  true = Insert the passed step value; false = always insert 1. */
    void                SetDateGroupLimit( const ScDPNumGroupInfo& rDateInfo, bool bUseStep );

    /** Initializes flags and item count fields. */
    void                Finalize();

    /** Writes an SXNUMGROUP record and the additional items for a numeric grouping field. */
    void                WriteSxnumgroup( XclExpStream& rStrm );
    /** Writes an SXGROUPINFO record describing the item order in grouping fields. */
    void                WriteSxgroupinfo( XclExpStream& rStrm );

    /** Writes the contents of the SXFIELD record for this field. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclExpPCItemList    maOrigItemList;     /// List with original items.
    XclExpPCItemList    maGroupItemList;    /// List with grouping items.
    ScfUInt16Vec        maIndexVec;         /// Indexes into maItemList.
    XclExpPCItemList    maNumGroupLimits;   /// List with limit values for numeric grouping.
    sal_uInt16          mnTypeFlags;        /// Collected item data type flags.
};

class XclExpPivotCache : public salhelper::SimpleReferenceObject, protected XclExpRoot
{
public:
    explicit            XclExpPivotCache( const XclExpRoot& rRoot,
                            const ScDPObject& rDPObj, sal_uInt16 nListIdx );

    /** Returns true, if the cache has been constructed successfully. */
    bool         IsValid() const { return mbValid; }
    /** Returns true, if the item index list will be written. */
    bool                HasItemIndexList() const;

    /** Returns the list index of the cache used in pivot table records. */
    sal_uInt16   GetCacheIndex() const { return mnListIdx; }

    /** Returns the number of pivot cache fields. */
    sal_uInt16          GetFieldCount() const;
    /** Returns the specified pivot cache field. */
    const XclExpPCField* GetField( sal_uInt16 nFieldIdx ) const;
    /** Returns true, if this pivot cache contains non-standard fields (e.g. grouping fields). */
    bool                HasAddFields() const;

    /** Returns true, if the passed DP object has the same data source as this cache. */
    bool                HasEqualDataSource( const ScDPObject& rDPObj ) const;

    /** Writes related records into Workbook stream and creates the pivot cache storage stream. */
    void        Save( XclExpStream& rStrm );
    static void SaveXml( XclExpXmlStream& rStrm );

private:
    /** Adds all pivot cache fields. */
    void                AddFields( const ScDPObject& rDPObj );

    /** Adds all standard pivot cache fields based on source data. */
    void                AddStdFields( const ScDPObject& rDPObj );
    /** Adds all grouping pivot cache fields. */
    void                AddGroupFields( const ScDPObject& rDPObj );

    /** Writes the DCONREF record containing the source range. */
    void                WriteDconref( XclExpStream& rStrm ) const;
    /** DCONNAME record contains range name source. */
    void                WriteDConName( XclExpStream& rStrm ) const;

    /** Creates the pivot cache storage stream and writes the cache. */
    void                WriteCacheStream();
    /** Writes the SXDB record. */
    void                WriteSxdb( XclExpStream& rStrm ) const;
    /** Writes the SXDBEX record. */
    static void         WriteSxdbex( XclExpStream& rStrm );
    /** Writes the SXINDEXLIST record list containing the item index table. */
    void                WriteSxindexlistList( XclExpStream& rStrm ) const;

private:
    typedef XclExpRecordList< XclExpPCField >   XclExpPCFieldList;
    typedef XclExpPCFieldList::RecordRefType    XclExpPCFieldRef;

    XclPCInfo           maPCInfo;           /// Pivot cache settings (SXDB record).
    XclExpPCFieldList   maFieldList;        /// List of all pivot cache fields.
    OUString       maTabName;          /// Name of source data sheet.
    OUString       maSrcRangeName;     /// Range name for source data.
    ScRange             maOrigSrcRange;     /// The original sheet source range.
    ScRange             maExpSrcRange;      /// The exported sheet source range.
    ScRange             maDocSrcRange;      /// The range used to build the cache fields and items.
    sal_uInt16          mnListIdx;          /// List index in pivot cache buffer.
    bool                mbValid;            /// true = The cache is valid for export.
};

typedef rtl::Reference<XclExpPivotCache>       XclExpPivotCacheRef;

// Pivot table

class XclExpPivotTable;

/** Data field position specifying the pivot table field index (first) and data info index (second). */
typedef ::std::pair< sal_uInt16, sal_uInt16 > XclPTDataFieldPos;

class XclExpPTItem : public XclExpRecord
{
public:
    explicit            XclExpPTItem( const XclExpPCField& rCacheField, sal_uInt16 nCacheIdx );
    explicit            XclExpPTItem( sal_uInt16 nItemType, sal_uInt16 nCacheIdx );

    /** Returns the internal name of this item. */
    OUString       GetItemName() const;

    /** Fills this item with properties from the passed save member. */
    void                SetPropertiesFromMember( const ScDPSaveMember& rSaveMem );

private:
    /** Writes the SXVI record body describing the pivot table item. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    const XclExpPCItem* mpCacheItem;        /// The referred pivot cache item.
    XclPTItemInfo       maItemInfo;         /// General data for this item.
};

class XclExpPTField : public XclExpRecordBase
{
public:
    explicit            XclExpPTField( const XclExpPivotTable& rPTable, sal_uInt16 nCacheIdx );

    // data access ------------------------------------------------------------

    /** Returns the name of this field. */
    OUString       GetFieldName() const;
    /** Returns the pivot table field list index of this field.
      * The field index is always equal to cache index.
      */
    sal_uInt16          GetFieldIndex() const { return maFieldInfo.mnCacheIdx; }

    /** Returns the index of the last inserted data info struct. */
    sal_uInt16          GetLastDataInfoIndex() const;

    /** Returns the list index of an item by its name.
        @param nDefaultIdx  This value will be returned, if the item could not be found. */
    sal_uInt16          GetItemIndex( std::u16string_view rName, sal_uInt16 nDefaultIdx ) const;

    // fill data --------------------------------------------------------------

    /** Fills this field with row/column/page properties from the passed save dimension. */
    void                SetPropertiesFromDim( const ScDPSaveDimension& rSaveDim );
    /** Fills this field with data field properties from the passed save dimension. */
    void                SetDataPropertiesFromDim( const ScDPSaveDimension& rSaveDim );

    /** Appends special items describing the field subtotal entries. */
    void                AppendSubtotalItems();

    // records ----------------------------------------------------------------

    /** Writes an entry for an SXPI record containing own page field info. */
    void                WriteSxpiEntry( XclExpStream& rStrm ) const;
    /** Writes an SXDI records containing info about a data field. */
    void                WriteSxdi( XclExpStream& rStrm, sal_uInt16 nDataInfoIdx ) const;

    /** Writes the entire pivot table field. */
    virtual void        Save( XclExpStream& rStrm ) override;

private:
    /** Returns an item by its name. */
    XclExpPTItem*       GetItemAcc( std::u16string_view rName );

    /** Appends a special item describing a field subtotal entry. */
    void                AppendSubtotalItem( sal_uInt16 nItemType );

    /** Writes the SXVD record introducing the field. */
    void                WriteSxvd( XclExpStream& rStrm ) const;
    /** Writes the SXVDEX record containing additional settings. */
    void                WriteSxvdex( XclExpStream& rStrm ) const;

private:

    const XclExpPivotTable& mrPTable;       /// Parent pivot table containing this field.
    const XclExpPCField* mpCacheField;      /// The referred pivot cache field.
    XclPTFieldInfo      maFieldInfo;        /// General field info (SXVD record).
    XclPTFieldExtInfo   maFieldExtInfo;     /// Extended field info (SXVDEX record).
    XclPTPageFieldInfo  maPageInfo;         /// Page field info (entry in SXPI record).
    std::vector< XclPTDataFieldInfo >
                        maDataInfoVec;    /// List of extended data field info (SXDI records).
    XclExpRecordList< XclExpPTItem >
                        maItemList;         /// List of all items of this field.
};

class XclExpPivotTable : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpPivotTable( const XclExpRoot& rRoot,
                            const ScDPObject& rDPObj, const XclExpPivotCache& rPCache );

    /** Returns a pivot cache field. */
    const XclExpPCField* GetCacheField( sal_uInt16 nCacheIdx ) const;

    /** Returns the output range of the pivot table. */
    SCTAB         GetScTab() const { return mnOutScTab; }

    /** Returns a pivot table field by its name. */
    const XclExpPTField* GetField( sal_uInt16 nFieldIdx ) const;
    /** Returns a pivot table field by its name. */
    const XclExpPTField* GetField( std::u16string_view rName ) const;

    /** Returns the data-field-only index of the first data field with the passed name.
        @param nDefaultIdx  This value will be returned, if the field could not be found. */
    sal_uInt16          GetDataFieldIndex( const OUString& rName, sal_uInt16 nDefaultIdx ) const;

    /** Writes the entire pivot table. */
    virtual void        Save( XclExpStream& rStrm ) override;

private:
    /** Returns a pivot table field by its name. */
    XclExpPTField*      GetFieldAcc( std::u16string_view rName );
    /** Returns a pivot table field corresponding to the passed save dimension. */
    XclExpPTField*      GetFieldAcc( const ScDPSaveDimension& rSaveDim );

    // fill data --------------------------------------------------------------

    /** Fills internal members with all properties from the passed save data. */
    void                SetPropertiesFromDP( const ScDPSaveData& rSaveData );
    /** Fills a pivot table field with all properties from the passed save dimension. */
    void                SetFieldPropertiesFromDim( const ScDPSaveDimension& rSaveDim );
    /** Fills a pivot table data field with all properties from the passed save dimension. */
    void                SetDataFieldPropertiesFromDim( const ScDPSaveDimension& rSaveDim );

    /** Initializes any data after processing the entire source DataPilot. */
    void                Finalize();

    // records ----------------------------------------------------------------

    /** Writes the SXVIEW record starting the pivot table. */
    void                WriteSxview( XclExpStream& rStrm ) const;
    /** Writes an SXIVD record for row field or column field order. */
    static void         WriteSxivd( XclExpStream& rStrm, const ScfUInt16Vec& rFields );
    /** Writes the SXPI record containing page field info. */
    void                WriteSxpi( XclExpStream& rStrm ) const;
    /** Writes all SXDI records containing info about the data fields. */
    void                WriteSxdiList( XclExpStream& rStrm ) const;
    /** Writes a dummy SXLI records containing item layout info. */
    static void         WriteSxli( XclExpStream& rStrm, sal_uInt16 nLineCount, sal_uInt16 nIndexCount );
    /** Writes the SXEX records containing additional pivot table info. */
    void                WriteSxex( XclExpStream& rStrm ) const;

    void                WriteQsiSxTag( XclExpStream& rStrm ) const;
    /** Writes the SX_AUTOFORMAT records with the autoformat id and header layout */
    void                WriteSxViewEx9( XclExpStream& rStrm ) const;

private:
    typedef XclExpRecordList< XclExpPTField >   XclExpPTFieldList;
    typedef XclExpPTFieldList::RecordRefType    XclExpPTFieldRef;

    const XclExpPivotCache& mrPCache;       /// The pivot cache this pivot table bases on.
    XclPTInfo           maPTInfo;           /// Info about the pivot table (SXVIEW record).
    XclPTExtInfo        maPTExtInfo;        /// Extended info about the pivot table (SXEX record).
    XclPTViewEx9Info    maPTViewEx9Info;    /// The selected autoformat (SXVIEWEX9)
    XclExpPTFieldList   maFieldList;        /// All fields in pivot cache order.
    ScfUInt16Vec        maRowFields;        /// Row field indexes.
    ScfUInt16Vec        maColFields;        /// Column field indexes.
    ScfUInt16Vec        maPageFields;       /// Page field indexes.
    std::vector< XclPTDataFieldPos >
                        maDataFields;      /// Data field indexes.
    XclExpPTField       maDataOrientField;  /// Special data field orientation field.
    SCTAB               mnOutScTab;         /// Sheet index of the output range.
    bool                mbValid;            /// true = The pivot table is valid for export.
    bool                mbFilterBtn;        /// true = DataPilot has filter button.
};

/** The main class for pivot table export.

    This class contains all pivot caches and pivot tables in a Calc document.
    It creates the pivot cache streams and pivot table records in the main
    workbook stream. It supports sharing of pivot caches between multiple pivot
    tables to decrease file size.
 */
class XclExpPivotTableManager : protected XclExpRoot
{
public:
    explicit            XclExpPivotTableManager( const XclExpRoot& rRoot );

    /** Creates all pivot tables and caches from the Calc DataPilot objects. */
    void                CreatePivotTables();

    /** Creates a record wrapper for exporting all pivot caches. */
    XclExpRecordRef     CreatePivotCachesRecord();
    /** Creates a record wrapper for exporting all pivot tables of the specified sheet. */
    XclExpRecordRef     CreatePivotTablesRecord( SCTAB nScTab );

    /** Writes all pivot caches (all Workbook records and cache streams). */
    void                WritePivotCaches( XclExpStream& rStrm );
    /** Writes all pivot tables of the specified Calc sheet. */
    void                WritePivotTables( XclExpStream& rStrm, SCTAB nScTab );

private:
    /** Finds an existing (if enabled in mbShareCaches) or creates a new pivot cache.
        @return  Pointer to the pivot cache or 0, if the passed source range was invalid. */
    const XclExpPivotCache* CreatePivotCache( const ScDPObject& rDPObj );

private:
    typedef XclExpRecordList< XclExpPivotTable >    XclExpPivotTableList;
    typedef XclExpPivotTableList::RecordRefType     XclExpPivotTableRef;

    XclExpRecordList< XclExpPivotCache > maPCacheList;      /// List of all pivot caches.
    XclExpPivotTableList                 maPTableList;      /// List of all pivot tables.
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
