/*************************************************************************
 *
 *  $RCSfile: xepivot.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:33:35 $
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

#ifndef SC_XEPIVOT_HXX
#define SC_XEPIVOT_HXX

#include <map>

#ifndef SC_XLPIVOT_HXX
#include "xlpivot.hxx"
#endif
#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif
#ifndef SC_XERECORD_HXX
#include "xerecord.hxx"
#endif

class ScDPObject;
class ScDPSaveData;
class ScDPSaveDimension;
class ScDPSaveMember;
class ScDPDimensionSaveData;
class ScDPSaveGroupDimension;
class ScDPSaveNumGroupDimension;
struct ScDPNumGroupInfo;

// ============================================================================
// Pivot cache
// ============================================================================

/** Represents a data item in a pivot cache containing data of any type. */
class XclExpPCItem : public XclExpRecord, public XclPCItem
{
public:
    explicit            XclExpPCItem( const String& rText );
    explicit            XclExpPCItem( double fValue, bool bDate );
    explicit            XclExpPCItem( sal_Int16 nValue );
    explicit            XclExpPCItem( bool bValue );

    inline sal_uInt16   GetTypeFlag() const { return mnTypeFlag; }

    bool                EqualsText( const String& rText ) const;
    bool                EqualsDouble( double fValue ) const;
    bool                EqualsDate( double fDate ) const;
    bool                EqualsBool( bool bValue ) const;

private:
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    sal_uInt16          mnTypeFlag;         /// Data type flag.
};

// ============================================================================

class XclExpPivotCache;

class XclExpPCField : public XclExpRecord, public XclPCField, protected XclExpRoot
{
public:
    /** Creates a standard pivot cache field, filled from sheet source data. */
    explicit            XclExpPCField( const XclExpRoot& rRoot,
                            const XclExpPivotCache& rPCache, sal_uInt16 nFieldIdx,
                            const ScDPObject& rDPObj, const ScRange& rRange );
    /** Creates a child grouping pivot cache field, filled from the passed grouping info. */
    explicit            XclExpPCField( const XclExpRoot& rRoot,
                            const XclExpPivotCache& rPCache, sal_uInt16 nFieldIdx,
                            const ScDPObject& rDPObj, const ScDPSaveGroupDimension& rGroupDim,
                            const XclExpPCField& rBaseField );

    /** Sets the passed field as direct grouping child field of this field. */
    void                SetGroupChildField( const XclExpPCField& rChildField );
    /** Converts this standard field into a numeric grouping field. */
    void                ConvertToNumGroup( const ScDPObject& rDPObj, const ScDPSaveNumGroupDimension& rNumGroupDim );

    /** Returns the name of this cache field. */
    inline const String& GetFieldName() const { return maFieldInfo.maName; }

    /** Returns the number of visible items of this field. */
    sal_uInt16          GetItemCount() const;
    /** Returns the specified pivot cache item (returns visible items in groupings). */
    const XclExpPCItem* GetItem( sal_uInt16 nItemIdx ) const;
    /** Returns the index of a pivot cache item, or EXC_PC_NOITEM on error. */
    sal_uInt16          GetItemIndex( const String& rItemName ) const;

    /** Returns the size an item index needs to write out. */
    sal_uInt32          GetIndexSize() const;
    /** Writes the item index at the passed source row position as part of the SXIDARRAY record. */
    void                WriteIndex( XclExpStream& rStrm, sal_uInt32 nSrcRow ) const;

    /** Writes the pivot cache field and all items and other related records. */
    virtual void        Save( XclExpStream& rStrm );

private:
    typedef XclExpRecordList< XclExpPCItem > XclExpPCItemList;

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
    void                InsertItemArrayIndex( ULONG nListPos );
    /** Inserts an original source item. Updates item index array. */
    void                InsertOrigItem( XclExpPCItem* pNewItem );
    /** Inserts an original text item, if it is not contained already. */
    void                InsertOrigTextItem( const String& rText );
    /** Inserts an original value item, if it is not contained already. */
    void                InsertOrigDoubleItem( double fValue );
    /** Inserts an original date/time item, if it is not contained already. */
    void                InsertOrigDateItem( double fDate );
    /** Inserts an original boolean item, if it is not contained already. */
    void                InsertOrigBoolItem( bool bValue );

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
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    const XclExpPivotCache& mrPCache;       /// Parent pivot cache containing this field.
    XclExpPCItemList    maOrigItemList;     /// List with original items.
    XclExpPCItemList    maGroupItemList;    /// List with grouping items.
    ScfUInt16Vec        maIndexVec;         /// Indexes into maItemList.
    XclExpPCItemList    maNumGroupLimits;   /// List with limit values for numeric grouping.
    sal_uInt16          mnTypeFlags;        /// Collected item data type flags.
};

// ============================================================================

class XclExpPivotCache : protected XclExpRoot
{
public:
    explicit            XclExpPivotCache( const XclExpRoot& rRoot,
                            const ScDPObject& rDPObj, sal_uInt16 nListIdx );

    /** Returns true, if the cache has been constructed successfully. */
    inline bool         IsValid() const { return mbValid; }
    /** Returns true, if the item index list will be written. */
    bool                HasItemIndexList() const;

    /** Returns the stream identifier used to create the cache stream. */
    inline sal_uInt16   GetStreamId() const { return maPCInfo.mnStrmId; }
    /** Returns the list index of the cache used in pivot table records. */
    inline sal_uInt16   GetCacheIndex() const { return mnListIdx; }

    /** Returns the number of pivot cache fields. */
    sal_uInt16          GetFieldCount() const;
    /** Returns the specified pivot cache field. */
    const XclExpPCField* GetField( sal_uInt16 nFieldIdx ) const;
    /** Returns a pivot cache field by its name. */
    const XclExpPCField* GetField( const String& rFieldName ) const;
    /** Returns true, if this pivot cache contains non-standard fields (i.e. grouping fields). */
    bool                HasAddFields() const;

    /** Returns true, if the passed DP object has the same data source as this cache. */
    bool                HasEqualDataSource( const ScDPObject& rDPObj ) const;

    /** Writes related records into Workbook stream and creates the pivot cache storage stream. */
    virtual void        Save( XclExpStream& rStrm );

private:
    /** Returns read/write access to a pivot cache field. */
    XclExpPCField*      GetFieldAcc( sal_uInt16 nFieldIdx );
    /** Returns read/write access to a pivot cache field. */
    XclExpPCField*      GetFieldAcc( const String& rFieldName );

    /** Adds all pivot cache fields. */
    void                AddFields( const ScDPObject& rDPObj );

    /** Adds all standard pivot cache fields based on source data. */
    void                AddStdFields( const ScDPObject& rDPObj );
    /** Adds all grouping pivot cache fields. */
    void                AddGroupFields( const ScDPObject& rDPObj );
    /** Adds all calculated pivot cache fields. */
    void                AddCalcFields( const ScDPObject& rDPObj );

    /** Writes the DCONREF record containing the source range. */
    void                WriteDconref( XclExpStream& rStrm ) const;

    /** Creates the pivot cache storage stream and writes the cache. */
    void                WriteCacheStream();
    /** Writes the SXDB record. */
    void                WriteSxdb( XclExpStream& rStrm ) const;
    /** Writes the SXDBEX record. */
    void                WriteSxdbex( XclExpStream& rStrm ) const;
    /** Writes the SXIDARRAY record list containing the item index table. */
    void                WriteSxidarrayList( XclExpStream& rStrm ) const;

private:
    typedef XclExpRecordList< XclExpPCField > XclExpPCFieldList;

    XclPCInfo           maPCInfo;           /// Pivot cache settings (SXDB record).
    XclExpPCFieldList   maFieldList;        /// List of all pivot cache fields.
    String              maTabName;          /// Name of source data sheet.
    ScRange             maOrigSrcRange;     /// The original sheet source range.
    ScRange             maExpSrcRange;      /// The exported sheet source range.
    ScRange             maDocSrcRange;      /// The range used to build the cache fields and items.
    sal_uInt16          mnListIdx;          /// List index in pivot cache buffer.
    bool                mbValid;            /// true = The cache is valid for export.
};

// ============================================================================
// Pivot table
// ============================================================================

class XclExpPivotTable;

/** Data field position specifying the pivot table field index (first) and data info index (second). */
typedef ::std::pair< sal_uInt16, sal_uInt16 > XclPTDataFieldPos;

// ============================================================================

class XclExpPTItem : public XclExpRecord
{
public:
    explicit            XclExpPTItem( const XclExpPCField& rCacheField, sal_uInt16 nCacheIdx );
    explicit            XclExpPTItem( sal_uInt16 nItemType, sal_uInt16 nCacheIdx, bool bUseCache );

    /** Returns the internal name of this item. */
    const String&       GetItemName() const;

    /** Fills this item with properties from the passed save member. */
    void                SetPropertiesFromMember( const ScDPSaveMember& rSaveMem );

private:
    /** Writes the SXVI record body describing the pivot table item. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    const XclExpPCItem* mpCacheItem;        /// The referred pivot cache item.
    XclPTItemInfo       maItemInfo;         /// General data for this item.
};

// ============================================================================

class XclExpPTField : public XclExpRecordBase
{
public:
    explicit            XclExpPTField( const XclExpPivotTable& rPTable, sal_uInt16 nCacheIdx );

    // data access ------------------------------------------------------------

    /** Returns the name of this field. */
    const String&       GetFieldName() const;
    /** Returns the pivot table field list index of this field. */
    sal_uInt16          GetFieldIndex() const;

    /** Returns the index of the last inserted data info struct. */
    sal_uInt16          GetLastDataInfoIndex() const;

    /** Returns an item by its name. */
    const XclExpPTItem* GetItem( const String& rName ) const;
    /** Returns the list index of an item by its name.
        @param nDefaultIdx  This value will be returned, if the item could not be found. */
    sal_uInt16          GetItemIndex( const String& rName, sal_uInt16 nDefaultIdx ) const;

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
    virtual void        Save( XclExpStream& rStrm );

    // ------------------------------------------------------------------------
private:
    /** Returns an item by its name. */
    XclExpPTItem*       GetItemAcc( const String& rName );

    /** Appends a special item describing a field subtotal entry. */
    void                AppendSubtotalItem( sal_uInt16 nItemType );

    /** Writes the SXVD record introducing the field. */
    void                WriteSxvd( XclExpStream& rStrm ) const;
    /** Writes the SXVDEX record containing additional settings. */
    void                WriteSxvdex( XclExpStream& rStrm ) const;

private:
    typedef ::std::vector< XclPTDataFieldInfo > XclPTDataFieldInfoVec;
    typedef XclExpRecordList< XclExpPTItem >    XclExpPTItemList;

    const XclExpPivotTable& mrPTable;       /// Parent pivot table containing this field.
    const XclExpPCField* mpCacheField;      /// The referred pivot cache field.
    XclPTFieldInfo      maFieldInfo;        /// General field info (SXVD record).
    XclPTFieldExtInfo   maFieldExtInfo;     /// Extended field info (SXVDEX record).
    XclPTPageFieldInfo  maPageInfo;         /// Page field info (entry in SXPI record).
    XclPTDataFieldInfoVec maDataInfoVec;    /// List of extended data field info (SXDI records).
    XclExpPTItemList    maItemList;         /// List of all items of this field.
};

// ============================================================================

class XclExpPivotTable : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpPivotTable( const XclExpRoot& rRoot,
                            const ScDPObject& rDPObj, const XclExpPivotCache& rPCache );

    /** Returns a pivot cache field. */
    const XclExpPCField* GetCacheField( sal_uInt16 nCacheIdx ) const;

    /** Returns the output range of the pivot table. */
    SCTAB               GetScTab() const;

    /** Returns a pivot table field by its name. */
    const XclExpPTField* GetField( sal_uInt16 nFieldIdx ) const;
    /** Returns a pivot table field by its name. */
    const XclExpPTField* GetField( const String& rName ) const;

    /** Returns the data-field-only index of the first data field with the passed name.
        @param nDefaultIdx  This value will be returned, if the field could not be found. */
    sal_uInt16          GetDataFieldIndex( const String& rName, sal_uInt16 nDefaultIdx ) const;

    /** Writes the entire pivot table. */
    virtual void        Save( XclExpStream& rStrm );

    // ------------------------------------------------------------------------
private:
    /** Returns a pivot table field by its name. */
    XclExpPTField*      GetFieldAcc( const String& rName );
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
    void                WriteSxivd( XclExpStream& rStrm, const ScfUInt16Vec& rFields ) const;
    /** Writes the SXPI record containing page field info. */
    void                WriteSxpi( XclExpStream& rStrm ) const;
    /** Writes all SXDI records containing info about the data fields. */
    void                WriteSxdiList( XclExpStream& rStrm ) const;
    /** Writes a dummy SXLI records containing item layout info. */
    void                WriteSxli( XclExpStream& rStrm, sal_uInt16 nLineCount, sal_uInt16 nIndexCount ) const;
    /** Writes the SXEX records containing additional pivot table info. */
    void                WriteSxex( XclExpStream& rStrm ) const;

    // ------------------------------------------------------------------------
private:
    typedef XclExpRecordList< XclExpPTField >   XclExpPTFieldList;
    typedef ::std::vector< XclPTDataFieldPos >  XclPTDataFieldPosVec;

    const XclExpPivotCache& mrPCache;       /// The pivot cache this pivot table bases on.
    XclPTInfo           maPTInfo;           /// Info about the pivot table (SXVIEW record).
    XclPTExtInfo        maPTExtInfo;        /// Extended info about the pivot table (SXEX record).
    XclExpPTFieldList   maFieldList;        /// All fields in pivot cache order.
    ScfUInt16Vec        maRowFields;        /// Row field indexes.
    ScfUInt16Vec        maColFields;        /// Column field indexes.
    ScfUInt16Vec        maPageFields;       /// Page field indexes.
    XclPTDataFieldPosVec maDataFields;      /// Data field indexes.
    XclExpPTField       maDataOrientField;  /// Special data field orientation field.
    ScRange             maOutputRange;      /// Output range in the Calc document.
    bool                mbValid;            /// true = The pivot table is valid for export.
    bool                mbFilterBtn;        /// true = DataPilot has filter button.
};

// ============================================================================

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

    /** Writes all pivot caches (all Workbook records and cache streams). */
    void                WritePivotCaches( XclExpStream& rStrm );
    /** Writes all pivot tables of the specified Calc sheet. */
    void                WritePivotTables( XclExpStream& rStrm, SCTAB nScTab );

private:
    /** Finds an existing (if enabled in mbShareCaches) or creates a new pivot cache.
        @return  Pointer to the pivot cache or 0, if the passed source range was invalid. */
    const XclExpPivotCache* CreatePivotCache( const ScDPObject& rDPObj );

private:
    typedef XclExpRecordList< XclExpPivotCache >    XclExpPivotCacheList;
    typedef XclExpRecordList< XclExpPivotTable >    XclExpPivotTableList;

    XclExpPivotCacheList maPCacheList;      /// List of all pivot caches.
    XclExpPivotTableList maPTableList;      /// List of all pivot tables.
    bool                mbShareCaches;      /// true = Tries to share caches between tables.
};

// ============================================================================
// Reference record classes
// ============================================================================

/** Placeholder record to write the entire pivot cache list. */
class XclExpPivotCacheRefRecord : public XclExpRecordBase
{
public:
    explicit            XclExpPivotCacheRefRecord( const XclExpRoot& rRoot );

    virtual void        Save( XclExpStream& rStrm );

private:
    XclExpPivotTableManager& mrPTManager;   /// The pivot table manager.
};

// ----------------------------------------------------------------------------

/** Placeholder record to write all pivot tables of a specific sheet. */
class XclExpPivotTablesRefRecord : public XclExpRecordBase
{
public:
    /** Refers to the current Calc sheet contained in the root data. */
    explicit            XclExpPivotTablesRefRecord( const XclExpRoot& rRoot );

    virtual void        Save( XclExpStream& rStrm );

private:
    XclExpPivotTableManager& mrPTManager;   /// The pivot table manager.
    SCTAB               mnScTab;            /// The Calc sheet index for the pivot tables.
};

// ============================================================================

#endif

