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

#include "scdllapi.h"
#include "global.hxx"
#include "address.hxx"
#include "dpcache.hxx"
#include "dptypes.hxx"
#include "pivot.hxx"
#include "calcmacros.hxx"

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <o3tl/sorted_vector.hxx>

#include <memory>
#include <vector>
#include <map>

namespace com::sun::star {

    namespace container {
        class XIndexAccess;
        class XNameAccess;
    }

    namespace sdbc {
        class XRowSet;
    }

    namespace sheet {
        class XMembersAccess;
        class XDimensionsSupplier;
        struct DataPilotTablePositionData;
        struct DataPilotTableHeaderData;
        struct DataPilotFieldFilter;
    }
}

namespace tools { class Rectangle; }
class ScDPSaveData;
class ScDPOutput;
struct ScImportSourceDesc;
class ScSheetSourceDesc;
class ScDPTableData;
class ScDPDimensionSaveData;
class ScRangeList;
class ScDPCache;
class ScDocument;

struct ScDPServiceDesc
{
    OUString aServiceName;
    OUString aParSource;
    OUString aParName;
    OUString aParUser;
    OUString aParPass;

    ScDPServiceDesc( const OUString& rServ, const OUString& rSrc, const OUString& rNam,
                     const OUString& rUser, const OUString& rPass );

    bool operator== ( const ScDPServiceDesc& rOther ) const;
};

class SC_DLLPUBLIC ScDPObject
{
private:
    ScDocument*             pDoc;
                                            // settings
    std::unique_ptr<ScDPSaveData> pSaveData;
    OUString aTableName;
    OUString aTableTag;
    ScRange                 aOutRange;
    std::unique_ptr<ScSheetSourceDesc>  pSheetDesc;     //  for sheet data
    std::unique_ptr<ScImportSourceDesc> pImpDesc;       //  for database data
    std::unique_ptr<ScDPServiceDesc> pServDesc;      //  for external service
    std::shared_ptr<ScDPTableData>  mpTableData;
                                            // cached data
    css::uno::Reference<css::sheet::XDimensionsSupplier> xSource;
    std::unique_ptr<ScDPOutput> pOutput;

    // name -> sequence of sequences of css::xml::FastAttribute or css::xml::Attribute
    // see PivotTable::putToInteropGrabBag in sc/source/filter/oox/pivottablebuffer.cxx for details
    std::map<OUString, css::uno::Any> maInteropGrabBag;

    sal_Int32               nHeaderRows;    // page fields plus filter button
    bool                    mbHeaderLayout:1;  // true : grid, false : standard
    bool                    bAllowMove:1;
    bool                    bSettingsChanged:1;
    bool                    mbEnableGetPivotData:1;

    SAL_DLLPRIVATE ScDPTableData*    GetTableData();
    SAL_DLLPRIVATE void              CreateObjects();
    SAL_DLLPRIVATE void              CreateOutput();
    SAL_DLLPRIVATE void ClearSource();
    SAL_DLLPRIVATE void FillLabelDataForDimension(
        const css::uno::Reference< css::container::XIndexAccess>& xDims,
        sal_Int32 nDim, ScDPLabelData& rLabelData);

public:
    ScDPObject(ScDocument* pD);
    ScDPObject(const ScDPObject& r);
    ~ScDPObject();

    ScDPObject& operator= (const ScDPObject& r);

    void EnableGetPivotData(bool b);

    void                SetAllowMove(bool bSet);

    void                InvalidateData();
    void Clear();
    void ClearTableData();
    void ReloadGroupTableData();

    void                Output( const ScAddress& rPos );
    ScRange             GetNewOutputRange( bool& rOverflow );

    ScRange GetOutputRangeByType( sal_Int32 nType );
    ScRange GetOutputRangeByType( sal_Int32 nType ) const;

    void                SetSaveData(const ScDPSaveData& rData);
    ScDPSaveData*       GetSaveData() const     { return pSaveData.get(); }

    void                SetOutRange(const ScRange& rRange);
    const ScRange&      GetOutRange() const;

    void                SetHeaderLayout(bool bUseGrid);
    bool                GetHeaderLayout() const { return mbHeaderLayout;}

    void                SetSheetDesc(const ScSheetSourceDesc& rDesc);
    void                SetImportDesc(const ScImportSourceDesc& rDesc);
    void                SetServiceData(const ScDPServiceDesc& rDesc);

    void                WriteSourceDataTo( ScDPObject& rDest ) const;
    void                WriteTempDataTo( ScDPObject& rDest ) const;

    const ScSheetSourceDesc* GetSheetDesc() const   { return pSheetDesc.get(); }
    const ScImportSourceDesc* GetImportSourceDesc() const   { return pImpDesc.get(); }
    const ScDPServiceDesc* GetDPServiceDesc() const { return pServDesc.get(); }

    css::uno::Reference<css::sheet::XDimensionsSupplier> const & GetSource();

    bool                IsSheetData() const;
    bool                IsImportData() const { return(pImpDesc != nullptr); }
    bool                IsServiceData() const { return(pServDesc != nullptr); }

    void SetName(const OUString& rNew);
    const OUString& GetName() const { return aTableName; }
    void SetTag(const OUString& rNew);
    const OUString& GetTag() const { return aTableTag; }

    /**
     *  Data description cell displays the description of a data dimension if
     *  and only if there is only one data dimension.  It's usually located at
     *  the upper-left corner of the table output.
     */
    bool                IsDataDescriptionCell(const ScAddress& rPos);

    bool                IsDimNameInUse(std::u16string_view rName) const;
    OUString GetDimName( tools::Long nDim, bool& rIsDataLayout, sal_Int32* pFlags = nullptr );
    bool                IsDuplicated( tools::Long nDim );
    tools::Long                GetDimCount();
    void                GetHeaderPositionData(const ScAddress& rPos, css::sheet::DataPilotTableHeaderData& rData);
    tools::Long                GetHeaderDim( const ScAddress& rPos, css::sheet::DataPilotFieldOrientation& rOrient );
    bool                GetHeaderDrag( const ScAddress& rPos, bool bMouseLeft, bool bMouseTop,
                                       tools::Long nDragDim,
                                       tools::Rectangle& rPosRect, css::sheet::DataPilotFieldOrientation& rOrient, tools::Long& rDimPos );
    bool                IsFilterButton( const ScAddress& rPos );

    OUString            GetFormattedString( std::u16string_view rDimName, const double fValue );

    double GetPivotData(
        const OUString& rDataFieldName,
        std::vector<css::sheet::DataPilotFieldFilter>& rFilters);

    bool ParseFilters(
        OUString& rDataFieldName,
        std::vector<css::sheet::DataPilotFieldFilter>& rFilters,
        std::vector<sal_Int16>& rFilterFuncs,
        std::u16string_view rFilterList );

    void GetMemberResultNames(ScDPUniqueStringSet& rNames, tools::Long nDimension);

    void                ToggleDetails(const css::sheet::DataPilotTableHeaderData& rElemDesc, ScDPObject* pDestObj);

    void                FillOldParam(ScPivotParam& rParam) const;
    void                FillLabelData(sal_Int32 nDim, ScDPLabelData& Labels);
    void                FillLabelData(ScPivotParam& rParam);

    bool                GetHierarchiesNA( sal_Int32 nDim, css::uno::Reference< css::container::XNameAccess >& xHiers );
    void                GetHierarchies( sal_Int32 nDim, css::uno::Sequence< OUString >& rHiers );

    sal_Int32           GetUsedHierarchy( sal_Int32 nDim );

    bool                GetMembersNA( sal_Int32 nDim, css::uno::Reference< css::sheet::XMembersAccess >& xMembers );
    bool                GetMembersNA( sal_Int32 nDim, sal_Int32 nHier, css::uno::Reference< css::sheet::XMembersAccess >& xMembers );

    bool                GetMemberNames( sal_Int32 nDim, css::uno::Sequence< OUString >& rNames );
    bool                GetMembers( sal_Int32 nDim, sal_Int32 nHier, ::std::vector<ScDPLabelData::Member>& rMembers );

    void                UpdateReference( UpdateRefMode eUpdateRefMode,
                                         const ScRange& r, SCCOL nDx, SCROW nDy, SCTAB nDz );
    bool                RefsEqual( const ScDPObject& r ) const;
    void                WriteRefsTo( ScDPObject& r ) const;

    void                GetPositionData(const ScAddress& rPos, css::sheet::DataPilotTablePositionData& rPosData);

    bool                GetDataFieldPositionData(const ScAddress& rPos,
                                                 css::uno::Sequence<
                                                    css::sheet::DataPilotFieldFilter >& rFilters);

    void                GetDrillDownData(const ScAddress& rPos,
                                         css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& rTableData);

    // apply drop-down attribute, initialize nHeaderRows, without accessing the source
    // (button attribute must be present)
    void                RefreshAfterLoad();

    void                BuildAllDimensionMembers();

    /**
     * Remove in the save data entries for members that don't exist anymore.
     * This is called during pivot table refresh.
     */
    bool SyncAllDimensionMembers();

    static bool         HasRegisteredSources();
    static std::vector<OUString> GetRegisteredSources();
    static css::uno::Reference<css::sheet::XDimensionsSupplier>
                        CreateSource( const ScDPServiceDesc& rDesc );

    static void ConvertOrientation(
        ScDPSaveData& rSaveData,
        const ScPivotFieldVector& rFields, css::sheet::DataPilotFieldOrientation nOrient,
        const css::uno::Reference< css::sheet::XDimensionsSupplier>& xSource,
        const ScDPLabelDataVector& rLabels,
        const ScPivotFieldVector* pRefColFields = nullptr,
        const ScPivotFieldVector* pRefRowFields = nullptr,
        const ScPivotFieldVector* pRefPageFields = nullptr );

    static bool         IsOrientationAllowed( css::sheet::DataPilotFieldOrientation nOrient, sal_Int32 nDimFlags );

    void PutInteropGrabBag(std::map<OUString, css::uno::Any>&& val)
    {
        maInteropGrabBag = std::move(val);
    }
    std::pair<bool, css::uno::Any> GetInteropGrabBagValue(const OUString& sName) const
    {
        if (const auto it = maInteropGrabBag.find(sName); it != maInteropGrabBag.end())
            return { true, it->second };

        return { false, css::uno::Any() };
    }

#if DUMP_PIVOT_TABLE
    void Dump() const;
    void DumpCache() const;
#endif
};

class ScDPCollection
{
    friend class ScDPCache;
public:

    /**
     * Stores and manages all caches from internal sheets.
     */
    class SheetCaches
    {
        friend class ScDPCollection;
        typedef std::map<size_t, std::unique_ptr<ScDPCache>> CachesType;
        typedef std::vector<ScRange> RangeIndexType;
        CachesType m_Caches;
        RangeIndexType maRanges;
        ScDocument& mrDoc;
    public:
        SheetCaches(ScDocument& rDoc);
        bool hasCache(const ScRange& rRange) const;
        const ScDPCache* getCache(const ScRange& rRange, const ScDPDimensionSaveData* pDimData);
        SC_DLLPUBLIC size_t size() const;

        void updateReference(
            UpdateRefMode eMode, const ScRange& r, SCCOL nDx, SCROW nDy, SCTAB nDz);

        SC_DLLPUBLIC ScDPCache* getExistingCache(const ScRange& rRange);
        SC_DLLPUBLIC const ScDPCache* getExistingCache(const ScRange& rRange) const;

        void updateCache(const ScRange& rRange, o3tl::sorted_vector<ScDPObject*>& rRefs);
        bool remove(const ScDPCache* p);

        SC_DLLPUBLIC const std::vector<ScRange>& getAllRanges() const;
    };

    /**
     * Data caches for range name based source data.
     */
    class NameCaches
    {
        friend class ScDPCollection;
        typedef ::std::map<OUString, std::unique_ptr<ScDPCache>> CachesType;
        CachesType m_Caches;
        ScDocument& mrDoc;
    public:
        NameCaches(ScDocument& rDoc);
        bool hasCache(const OUString& rName) const;
        const ScDPCache* getCache(
            const OUString& rName, const ScRange& rRange, const ScDPDimensionSaveData* pDimData);
        size_t size() const;
    private:
        ScDPCache* getExistingCache(const OUString& rName);

        void updateCache(
            const OUString& rName, const ScRange& rRange, o3tl::sorted_vector<ScDPObject*>& rRefs);
        bool remove(const ScDPCache* p);
    };

    /**
     * Defines connection type to external data source.  Used as a key to look
     * up database cache.
     */
    struct DBType
    {
        sal_Int32 mnSdbType;
        OUString maDBName;
        OUString maCommand;
        DBType(sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand);

        struct less
        {
            bool operator() (const DBType& left, const DBType& right) const;
        };
    };

    /**
     * Data caches for external database sources.
     */
    class DBCaches
    {
        friend class ScDPCollection;
        typedef ::std::map<DBType, std::unique_ptr<ScDPCache>, DBType::less> CachesType;
        CachesType m_Caches;
        ScDocument& mrDoc;
    public:
        DBCaches(ScDocument& rDoc);
        bool hasCache(sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand) const;
        const ScDPCache* getCache(
            sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand,
            const ScDPDimensionSaveData* pDimData);

    private:
        ScDPCache* getExistingCache(
            sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand);

        static css::uno::Reference<css::sdbc::XRowSet> createRowSet(
            sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand);

        void updateCache(
            sal_Int32 nSdbType, const OUString& rDBName, const OUString& rCommand,
            o3tl::sorted_vector<ScDPObject*>& rRefs);
        bool remove(const ScDPCache* p);
    };

    ScDPCollection(ScDocument& rDocument);
    ScDPCollection(const ScDPCollection& r);
    ~ScDPCollection();

    const char* ReloadCache(const ScDPObject* pDPObj, o3tl::sorted_vector<ScDPObject*>& rRefs);
    bool ReloadGroupsInCache(const ScDPObject* pDPObj, o3tl::sorted_vector<ScDPObject*>& rRefs);
    SC_DLLPUBLIC bool GetReferenceGroups(const ScDPObject& rDPObj, const ScDPDimensionSaveData** pGroups) const;

    SC_DLLPUBLIC size_t GetCount() const;
    SC_DLLPUBLIC ScDPObject& operator[](size_t nIndex);
    SC_DLLPUBLIC const ScDPObject& operator[](size_t nIndex) const;

    ScDPObject* GetByName(std::u16string_view rName) const;

    void DeleteOnTab( SCTAB nTab );
    void UpdateReference( UpdateRefMode eUpdateRefMode,
                          const ScRange& r, SCCOL nDx, SCROW nDy, SCTAB nDz );
    void CopyToTab( SCTAB nOld, SCTAB nNew );
    bool RefsEqual( const ScDPCollection& r ) const;
    void WriteRefsTo( ScDPCollection& r ) const;

    /**
     * Create a new name that's not yet used by any existing data pilot
     * objects.  All data pilot names are 'DataPilot' + <num>
     *
     * @return new name for data pilot object.
     */
    OUString CreateNewName() const;

    void FreeTable(const ScDPObject* pDPObj);
    SC_DLLPUBLIC ScDPObject* InsertNewTable(std::unique_ptr<ScDPObject> pDPObj);
    SC_DLLPUBLIC bool HasTable(const ScDPObject* pDPObj) const;

    SC_DLLPUBLIC SheetCaches& GetSheetCaches();
    SC_DLLPUBLIC const SheetCaches& GetSheetCaches() const;
    NameCaches& GetNameCaches();
    SC_DLLPUBLIC const NameCaches& GetNameCaches() const;
    DBCaches& GetDBCaches();
    SC_DLLPUBLIC const DBCaches& GetDBCaches() const;

    ScRangeList GetAllTableRanges( SCTAB nTab ) const;
    bool IntersectsTableByColumns( SCCOL nCol1, SCCOL nCol2, SCROW nRow, SCTAB nTab ) const;
    bool IntersectsTableByRows( SCCOL nCol, SCROW nRow1, SCROW nRow2, SCTAB nTab ) const;
    bool HasTable( const ScRange& rRange ) const;

#if DEBUG_PIVOT_TABLE
    void DumpTables() const;
#endif

private:
    /** Only to be called from ScDPCache::RemoveReference(). */
    void RemoveCache(const ScDPCache* pCache);

    void GetAllTables(const ScRange& rSrcRange, o3tl::sorted_vector<ScDPObject*>& rRefs) const;
    void GetAllTables(std::u16string_view rSrcName, o3tl::sorted_vector<ScDPObject*>& rRefs) const;
    void GetAllTables(
        sal_Int32 nSdbType, std::u16string_view rDBName, std::u16string_view rCommand,
        o3tl::sorted_vector<ScDPObject*>& rRefs) const;

private:
    typedef std::vector< std::unique_ptr<ScDPObject> > TablesType;

    ScDocument& mrDoc;
    TablesType maTables;
    SheetCaches maSheetCaches;
    NameCaches maNameCaches;
    DBCaches maDBCaches;
};

bool operator<(const ScDPCollection::DBType& left, const ScDPCollection::DBType& right);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
