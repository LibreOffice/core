/*************************************************************************
 *
 *  $RCSfile: xipivot.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:33:48 $
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

#ifndef SC_XIPIVOT_HXX
#define SC_XIPIVOT_HXX

#ifndef SC_XLPIVOT_HXX
#include "xlpivot.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif

class ScDPSaveData;
class ScDPSaveDimension;

typedef ::std::vector< String > XclImpPTNameVec;

// ============================================================================
// Pivot cache
// ============================================================================

class XclImpPivotCache;

// ============================================================================

/** Represents a data item in a pivot cache. */
class XclImpPCItem : public XclPCItem
{
public:
    explicit            XclImpPCItem( XclImpStream& rStrm );

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

// ============================================================================

struct ScDPNumGroupInfo;

/** Represents a field in a pivot cache (a column of data items in the source area). */
class XclImpPCField : public XclPCField
{
public:
    /** Creates a pivot cache field by reading an SXFIELD record. */
    explicit            XclImpPCField( XclImpStream& rStrm, XclImpPivotCache& rPCache, sal_uInt16 nFieldIdx );
                        ~XclImpPCField();

    // general field/item access ----------------------------------------------

    /** Returns the name of the field, uses the passed visible name if supported. */
    const String&       GetFieldName( const XclImpPTNameVec& rVisNames ) const;

    /** Returns the base field if this is a grouping field. */
    const XclImpPCField* GetGroupBaseField() const;

    /** Returns the number of items of this field. */
    sal_uInt16          GetItemCount() const;
    /** Returns the item at the specified position or 0 on error. */
    const XclImpPCItem* GetItem( sal_uInt16 nItemIdx ) const;
    /** Returns the item representing a limit value in numeric/date/time grouping fields.
        @param nItemIdx  One of EXC_SXFIELD_INDEX_MIN, EXC_SXFIELD_INDEX_MAX, or EXC_SXFIELD_INDEX_STEP. */
    const XclImpPCItem* GetLimitItem( sal_uInt16 nItemIdx ) const;

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
    void                ApplyGroupField( ScDPSaveData& rSaveData, const XclImpPTNameVec& rVisNames ) const;

    // ------------------------------------------------------------------------
private:
    /** Inserts standard grouping information of this field into the passed ScDPSaveData. */
    void                ApplyStdGroupField( ScDPSaveData& rSaveData, const XclImpPTNameVec& rVisNames ) const;
    /** Inserts numeric grouping information of this field into the passed ScDPSaveData. */
    void                ApplyNumGroupField( ScDPSaveData& rSaveData, const XclImpPTNameVec& rVisNames ) const;
    /** Inserts date grouping information of this field into the passed ScDPSaveData. */
    void                ApplyDateGroupField( ScDPSaveData& rSaveData, const XclImpPTNameVec& rVisNames ) const;

    /** Returns a Calc struct with numeric grouping data. */
    ScDPNumGroupInfo    GetScNumGroupInfo() const;
    /** Returns a Calc struct with date grouping data. */
    ScDPNumGroupInfo    GetScDateGroupInfo() const;

    /** Returns a limit value for numeric grouping fields. */
    const double*       GetNumGroupLimit( sal_uInt16 nLimitIdx ) const;
    /** Returns a limit value for date grouping fields (minimum/maximum only). */
    const double*       GetDateGroupLimit( sal_uInt16 nLimitIdx ) const;
    /** Returns the step value for date grouping fields. */
    const sal_Int16*    GetDateGroupStep() const;

private:
    typedef ScfDelList< XclImpPCItem > XclImpPCItemList;

    XclImpPivotCache&   mrPCache;           /// Parent pivot cache containing this field.
    XclImpPCItemList    maItemList;         /// List of all original data items.
    size_t              mnTotalItemCount;   /// Total count of read items.
    XclImpPCItemList    maNumGroupItemList; /// List of items containing numeric grouping limits.
    bool                mbNumGroupInfoRead; /// true = Numeric grouping info read (SXNUMGROUP record).
};

// ============================================================================

class XclImpPivotCache : protected XclImpRoot
{
public:
    explicit            XclImpPivotCache( const XclImpRoot& rRoot, sal_uInt16 nStrmId );
                        ~XclImpPivotCache();

    // data access ------------------------------------------------------------

    /** Returns true, if this cache represents data from the own document. */
    bool                IsSelfRef() const;

    inline const ScRange& GetSourceRange() const { return maSrcRange; }
    inline const String& GetUrl() const { return maUrl; }
    inline const String& GetTabName() const { return maTabName; }

    /** Returns the number of pivot cache fields. */
    sal_uInt16          GetFieldCount() const;
    /** Returns read-only access to a pivot cache field. */
    const XclImpPCField* GetField( sal_uInt16 nFieldIdx ) const;

    // records ----------------------------------------------------------------

    /** Reads a DCONREF record containing the source range of the pivot cache. */
    void                ReadDconref( XclImpStream& rStrm );
    /** Reads an SXVS record containing the source type of the pivot cache. */
    void                ReadSxvs( XclImpStream& rStrm );

    // ------------------------------------------------------------------------
private:
    void                ReadPivotCacheStream( XclImpStream& rStrm );

private:
    typedef ScfDelList< XclImpPCField > XclImpPCFieldList;

    XclPCInfo           maPCInfo;           /// Pivot cache settings (SXDB record).
    XclImpPCFieldList   maFieldList;        /// List of pivot cache fields.
    ScRange             maSrcRange;         /// Source range in the spreadsheet.
    sal_uInt16          mnSrcType;          /// Source data type.
    String              maUrl;              /// URL of the source data.
    String              maTabName;          /// Sheet name of the source data.
    bool                mbSelf;             /// true = Source data from own document.
};

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
    void                ApplyItem( ScDPSaveDimension& rSaveDim ) const;

private:
    XclPTItemInfo       maItemInfo;         /// General data for this item.
    const XclImpPCField* mpCacheField;      /// Corresponding pivot cache field.
};

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

    void                ApplyRowColField( ScDPSaveData& rSaveData ) const;

    // page fields ------------------------------------------------------------

    void                SetPageFieldInfo( const XclPTPageFieldInfo& rPageInfo );
    void                ApplyPageField( ScDPSaveData& rSaveData ) const;

    // hidden fields ----------------------------------------------------------

    void                ApplyHiddenField( ScDPSaveData& rSaveData ) const;

    // data fields ------------------------------------------------------------

    bool                HasDataFieldInfo() const;
    void                AddDataFieldInfo( const XclPTDataFieldInfo& rDataInfo );
    void                ApplyDataField( ScDPSaveData& rSaveData ) const;

    // ------------------------------------------------------------------------
private:
    ScDPSaveDimension*  ApplyRCPField( ScDPSaveData& rSaveData ) const;
    void                ApplyFieldInfo( ScDPSaveDimension& rSaveDim ) const;

    void                ApplyDataField( ScDPSaveDimension& rSaveDim, const XclPTDataFieldInfo& rDataInfo ) const;
    void                ApplyDataFieldInfo( ScDPSaveDimension& rSaveDim, const XclPTDataFieldInfo& rDataInfo ) const;
    void                ApplyItems( ScDPSaveDimension& rSaveDim ) const;

private:
    typedef ::std::list< XclPTDataFieldInfo >   XclPTDataFieldInfoList;
    typedef ScfDelList< XclImpPTItem >          XclImpPTItemList;

    const XclImpPivotTable& mrPTable;       /// Parent pivot table containing this field.
    XclPTFieldInfo      maFieldInfo;        /// General field info (SXVD record).
    XclPTFieldExtInfo   maFieldExtInfo;     /// Extended field info (SXVDEX record).
    XclPTPageFieldInfo  maPageInfo;         /// Page field info (entry from SXPI record).
    XclPTDataFieldInfoList maDataInfoList;  /// List of extended data field info (SXDI records).
    XclImpPTItemList    maItemList;         /// List of all items of this field.
};

// ============================================================================

class XclImpPivotTable : protected XclImpRoot
{
public:
    explicit            XclImpPivotTable( const XclImpRoot& rRoot );
                        ~XclImpPivotTable();

    // cache/field access, misc. ----------------------------------------------

    inline const XclImpPivotCache* GetPivotCache() const { return mpPCache; }
    inline const XclImpPTNameVec& GetVisFieldNames() const { return maVisFieldNames; }

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

    // ------------------------------------------------------------------------

    /** Inserts the pivot table into the Calc document. */
    void                Apply() const;

    // ------------------------------------------------------------------------
private:
    typedef ScfDelList< XclImpPTField > XclImpPTFieldList;

    const XclImpPivotCache* mpPCache;       /// Pivot cache containing field/item names.

    XclPTInfo           maPTInfo;           /// General info about the pivot table (SXVIEW record).
    XclPTExtInfo        maPTExtInfo;        /// Extended info about the pivot table (SXEX record).
    XclImpPTFieldList   maFieldList;        /// List containing all fields.
    XclImpPTNameVec     maVisFieldNames;    /// Vector containing all visible field names.
    ScfUInt16Vec        maRowFields;        /// Row field indexes.
    ScfUInt16Vec        maColFields;        /// Column field indexes.
    ScfUInt16Vec        maPageFields;       /// Page field indexes.
    ScfUInt16Vec        maOrigDataFields;   /// Original data field indexes.
    ScfUInt16Vec        maFiltDataFields;   /// Filtered data field indexes.
    XclImpPTField       maDataOrientField;  /// Special data field orientation field.
    ScRange             maOutputRange;      /// Output range in the Calc document.
    XclImpPTField*      mpCurrField;        /// Current field for importing additional info.
};

typedef ScfDelList< XclImpPivotTable > XclImpPivotTableList;

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
                        ~XclImpPivotTableManager();

    // access functions -------------------------------------------------------

    /** Returns the pivot cache with the specified 0-based index. */
    const XclImpPivotCache* GetPivotCache( sal_uInt16 nCacheIdx ) const;

    // pivot cache records ----------------------------------------------------

    /** Reads an SXIDSTM record containing a pivot cache stream identifier and the pivot cache. */
    void                ReadSxidstm( XclImpStream& rStrm );
    /** Reads a DCONREF record containing the source range of a pivot cache. */
    void                ReadDconref( XclImpStream& rStrm );
    /** Reads an SXVS record containing the source type of a pivot cache. */
    void                ReadSxvs( XclImpStream& rStrm );

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

    // ------------------------------------------------------------------------

    /** Inserts all pivot tables into the Calc document. */
    void                Apply() const;

private:
    typedef ScfDelList< XclImpPivotCache > XclImpPivotCacheList;

    XclImpPivotCacheList maPCacheList;      /// List of all pivot caches.
    XclImpPivotTableList maPTableList;      /// List of all pivot tables.
};

// ============================================================================

#endif

