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

#ifndef SC_DPOBJECT_HXX
#define SC_DPOBJECT_HXX

#include "scdllapi.h"
#include "global.hxx"
#include "address.hxx"
#include "dpoutput.hxx"
#include "dptypes.hxx"
#include "pivot.hxx"
#include "dpmacros.hxx"

#include <com/sun/star/sheet/XDimensionsSupplier.hpp>

#include <set>
#include <vector>

#include <boost/ptr_container/ptr_list.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/shared_ptr.hpp>

namespace com { namespace sun { namespace star {

    namespace container {
        class XIndexAccess;
    }

    namespace sdbc {
        class XRowSet;
    }

    namespace sheet {
        struct DataPilotTablePositionData;
        struct DataPilotTableHeaderData;
        struct DataPilotFieldFilter;
    }
}}}

class Rectangle;
class ScDPSaveData;
class ScDPOutput;
struct ScPivotParam;
struct ScImportSourceDesc;
class ScSheetSourceDesc;
struct PivotField;
class ScDPTableData;
class ScDPDimensionSaveData;

struct ScDPServiceDesc
{
    ::rtl::OUString aServiceName;
    ::rtl::OUString aParSource;
    ::rtl::OUString aParName;
    ::rtl::OUString aParUser;
    ::rtl::OUString aParPass;

    ScDPServiceDesc( const ::rtl::OUString& rServ, const ::rtl::OUString& rSrc, const ::rtl::OUString& rNam,
                     const ::rtl::OUString& rUser, const ::rtl::OUString& rPass );

    bool operator== ( const ScDPServiceDesc& rOther ) const;
};


class SC_DLLPUBLIC ScDPObject
{
private:
    ScDocument*             pDoc;
                                            // settings
    ScDPSaveData*           pSaveData;
    ::rtl::OUString aTableName;
    ::rtl::OUString aTableTag;
    ScRange                 aOutRange;
    ScSheetSourceDesc*      pSheetDesc;     //  for sheet data
    ScImportSourceDesc*     pImpDesc;       //  for database data
    ScDPServiceDesc*        pServDesc;      //  for external service
    ::boost::shared_ptr<ScDPTableData>  mpTableData;
                                            // cached data
    com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier> xSource;
    ScDPOutput*             pOutput;
    sal_uInt16              mnAutoFormatIndex;
    long                    nHeaderRows;    // page fields plus filter button
    bool                    mbHeaderLayout:1;  // true : grid, false : standard
    bool                    bAllowMove:1;
    bool                    bAlive:1;         // false if only used to hold settings
    bool                    bSettingsChanged:1;
    bool                    mbEnableGetPivotData:1;

    SC_DLLPRIVATE ScDPTableData*    GetTableData();
    SC_DLLPRIVATE void              CreateObjects();
    SC_DLLPRIVATE void              CreateOutput();
    SC_DLLPRIVATE void ClearSource();
    SC_DLLPRIVATE bool FillLabelDataForDimension(
        const com::sun::star::uno::Reference<
            com::sun::star::container::XIndexAccess>& xDims,
        sal_Int32 nDim, ScDPLabelData& rLabelData);

public:
    ScDPObject(ScDocument* pD);
    ScDPObject(const ScDPObject& r);
    ~ScDPObject();

    void EnableGetPivotData(bool b);

    /**
     * When a DP object is "alive", it has table output on a sheet.  This flag
     * doesn't really change the behavior of the object, but is used only for
     * testing purposes.
     */
    void                SetAlive(bool bSet);
    void                SetAllowMove(bool bSet);

    void                InvalidateData();
    void ClearTableData();
    void ReloadGroupTableData();

    void                Output( const ScAddress& rPos );
    ScRange             GetNewOutputRange( bool& rOverflow );
    const ScRange       GetOutputRangeByType( sal_Int32 nType );

    void                SetSaveData(const ScDPSaveData& rData);
    ScDPSaveData*       GetSaveData() const     { return pSaveData; }

    void                SetOutRange(const ScRange& rRange);
    const ScRange&      GetOutRange() const     { return aOutRange; }

    void                SetHeaderLayout(bool bUseGrid);
    bool                GetHeaderLayout() const;

    void                SetSheetDesc(const ScSheetSourceDesc& rDesc);
    void                SetImportDesc(const ScImportSourceDesc& rDesc);
    void                SetServiceData(const ScDPServiceDesc& rDesc);

    void                WriteSourceDataTo( ScDPObject& rDest ) const;
    void                WriteTempDataTo( ScDPObject& rDest ) const;

    const ScSheetSourceDesc* GetSheetDesc() const   { return pSheetDesc; }
    const ScImportSourceDesc* GetImportSourceDesc() const   { return pImpDesc; }
    const ScDPServiceDesc* GetDPServiceDesc() const { return pServDesc; }

    com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier> GetSource();

    bool                IsSheetData() const;
    bool                IsImportData() const { return(pImpDesc != NULL); }
    bool                IsServiceData() const { return(pServDesc != NULL); }

    void SetName(const ::rtl::OUString& rNew);
    const ::rtl::OUString& GetName() const { return aTableName; }
    void SetTag(const ::rtl::OUString& rNew);
    const ::rtl::OUString& GetTag() const { return aTableTag; }

    /**
     *  Data description cell displays the description of a data dimension if
     *  and only if there is only one data dimension.  It's usually located at
     *  the upper-left corner of the table output.
     */
    bool                IsDataDescriptionCell(const ScAddress& rPos);

    bool                IsDimNameInUse(const ::rtl::OUString& rName) const;
    ::rtl::OUString GetDimName( long nDim, bool& rIsDataLayout, sal_Int32* pFlags = NULL );
    bool                IsDuplicated( long nDim );
    long                GetDimCount();
    void                GetHeaderPositionData(const ScAddress& rPos, ::com::sun::star::sheet::DataPilotTableHeaderData& rData);
    long                GetHeaderDim( const ScAddress& rPos, sal_uInt16& rOrient );
    bool                GetHeaderDrag( const ScAddress& rPos, bool bMouseLeft, bool bMouseTop,
                                       long nDragDim,
                                       Rectangle& rPosRect, sal_uInt16& rOrient, long& rDimPos );
    bool                IsFilterButton( const ScAddress& rPos );

    bool                GetPivotData( ScDPGetPivotDataField& rTarget, /* returns result */
                                      const std::vector< ScDPGetPivotDataField >& rFilters );
    bool                ParseFilters( ScDPGetPivotDataField& rTarget,
                                      std::vector< ScDPGetPivotDataField >& rFilters,
                                      const ::rtl::OUString& rFilterList );

    void GetMemberResultNames(ScDPUniqueStringSet& rNames, long nDimension);

    void FillPageList( std::vector<rtl::OUString>& rStrings, long nField );

    void                ToggleDetails(const ::com::sun::star::sheet::DataPilotTableHeaderData& rElemDesc, ScDPObject* pDestObj);

    bool                FillOldParam(ScPivotParam& rParam) const;
    bool                FillLabelData(sal_Int32 nDim, ScDPLabelData& Labels);
    bool                FillLabelData(ScPivotParam& rParam);

    bool                GetHierarchiesNA( sal_Int32 nDim, com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& xHiers );
    bool                GetHierarchies( sal_Int32 nDim, com::sun::star::uno::Sequence< rtl::OUString >& rHiers );

    sal_Int32           GetUsedHierarchy( sal_Int32 nDim );

    bool                GetMembersNA( sal_Int32 nDim, com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& xMembers );
    bool                GetMembersNA( sal_Int32 nDim, sal_Int32 nHier, com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& xMembers );

    bool                GetMemberNames( sal_Int32 nDim, ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames );
    bool                GetMembers( sal_Int32 nDim, sal_Int32 nHier, ::std::vector<ScDPLabelData::Member>& rMembers );

    void                UpdateReference( UpdateRefMode eUpdateRefMode,
                                         const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    bool                RefsEqual( const ScDPObject& r ) const;
    void                WriteRefsTo( ScDPObject& r ) const;

    void                GetPositionData(const ScAddress& rPos, ::com::sun::star::sheet::DataPilotTablePositionData& rPosData);

    bool                GetDataFieldPositionData(const ScAddress& rPos,
                                                 ::com::sun::star::uno::Sequence<
                                                    ::com::sun::star::sheet::DataPilotFieldFilter >& rFilters);

    void                GetDrillDownData(const ScAddress& rPos,
                                         ::com::sun::star::uno::Sequence<
                                            ::com::sun::star::uno::Sequence<
                                                ::com::sun::star::uno::Any > >& rTableData);

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
    static com::sun::star::uno::Sequence<rtl::OUString> GetRegisteredSources();
    static com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier>
                        CreateSource( const ScDPServiceDesc& rDesc );

    static void ConvertOrientation(
        ScDPSaveData& rSaveData,
        const ::std::vector<PivotField>& rFields, sal_uInt16 nOrient,
        const com::sun::star::uno::Reference<
            com::sun::star::sheet::XDimensionsSupplier>& xSource,
        const ScDPLabelDataVec& rLabels,
        std::vector<PivotField>* pRefColFields = NULL,
        std::vector<PivotField>* pRefRowFields = NULL,
        std::vector<PivotField>* pRefPageFields = NULL );

    static bool         IsOrientationAllowed( sal_uInt16 nOrient, sal_Int32 nDimFlags );

#if DEBUG_PIVOT_TABLE
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
        typedef boost::ptr_map<size_t, ScDPCache> CachesType;
        typedef std::vector<ScRange> RangeIndexType;
        CachesType maCaches;
        RangeIndexType maRanges;
        ScDocument* mpDoc;
    public:
        SheetCaches(ScDocument* pDoc);
        bool hasCache(const ScRange& rRange) const;
        const ScDPCache* getCache(const ScRange& rRange, const ScDPDimensionSaveData* pDimData);
        size_t size() const;

        void updateReference(
            UpdateRefMode eMode, const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz);

    private:
        ScDPCache* getExistingCache(const ScRange& rRange);

        void updateCache(const ScRange& rRange, const ScDPDimensionSaveData* pDimData, std::set<ScDPObject*>& rRefs);
        bool remove(const ScDPCache* p);
    };

    /**
     * Data caches for range name based source data.
     */
    class NameCaches
    {
        friend class ScDPCollection;
        typedef ::boost::ptr_map<rtl::OUString, ScDPCache> CachesType;
        CachesType maCaches;
        ScDocument* mpDoc;
    public:
        NameCaches(ScDocument* pDoc);
        bool hasCache(const rtl::OUString& rName) const;
        const ScDPCache* getCache(
            const ::rtl::OUString& rName, const ScRange& rRange, const ScDPDimensionSaveData* pDimData);
        size_t size() const;
    private:
        ScDPCache* getExistingCache(const rtl::OUString& rName);

        void updateCache(
            const rtl::OUString& rName, const ScRange& rRange,
            const ScDPDimensionSaveData* pDimData, std::set<ScDPObject*>& rRefs);
        bool remove(const ScDPCache* p);
    };

    /**
     * Defines connection type to external data source.  Used as a key to look
     * up database cache.
     */
    struct DBType
    {
        sal_Int32 mnSdbType;
        ::rtl::OUString maDBName;
        ::rtl::OUString maCommand;
        DBType(sal_Int32 nSdbType, const ::rtl::OUString& rDBName, const ::rtl::OUString& rCommand);

        struct less : public ::std::binary_function<DBType, DBType, bool>
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
        typedef ::boost::ptr_map<DBType, ScDPCache, DBType::less> CachesType;
        CachesType maCaches;
        ScDocument* mpDoc;
    public:
        DBCaches(ScDocument* pDoc);
        bool hasCache(sal_Int32 nSdbType, const rtl::OUString& rDBName, const rtl::OUString& rCommand) const;
        const ScDPCache* getCache(
            sal_Int32 nSdbType, const ::rtl::OUString& rDBName, const ::rtl::OUString& rCommand,
            const ScDPDimensionSaveData* pDimData);

    private:
        ScDPCache* getExistingCache(
            sal_Int32 nSdbType, const ::rtl::OUString& rDBName, const ::rtl::OUString& rCommand);

        com::sun::star::uno::Reference<com::sun::star::sdbc::XRowSet> createRowSet(
            sal_Int32 nSdbType, const ::rtl::OUString& rDBName, const ::rtl::OUString& rCommand);

        void updateCache(sal_Int32 nSdbType, const ::rtl::OUString& rDBName, const ::rtl::OUString& rCommand,
                         const ScDPDimensionSaveData* pDimData, std::set<ScDPObject*>& rRefs);
        bool remove(const ScDPCache* p);
    };

    ScDPCollection(ScDocument* pDocument);
    ScDPCollection(const ScDPCollection& r);
    ~ScDPCollection();

    sal_uLong ReloadCache(ScDPObject* pDPObj, std::set<ScDPObject*>& rRefs);
    bool ReloadGroupsInCache(ScDPObject* pDPObj, std::set<ScDPObject*>& rRefs);

    SC_DLLPUBLIC size_t GetCount() const;
    SC_DLLPUBLIC ScDPObject* operator[](size_t nIndex);
    SC_DLLPUBLIC const ScDPObject* operator[](size_t nIndex) const;

    const ScDPObject* GetByName(const ::rtl::OUString& rName) const;

    void DeleteOnTab( SCTAB nTab );
    void UpdateReference( UpdateRefMode eUpdateRefMode,
                          const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void CopyToTab( SCTAB nOld, SCTAB nNew );
    bool RefsEqual( const ScDPCollection& r ) const;
    void WriteRefsTo( ScDPCollection& r ) const;

    /**
     * Create a new name that's not yet used by any existing data pilot
     * objects.  All data pilot names are 'DataPilot' + <num>, and the nMin
     * specifies the minimum number allowed.
     *
     * @param nMin minimum number allowed.
     *
     * @return new name for data pilot object.
     */
    ::rtl::OUString CreateNewName( sal_uInt16 nMin = 1 ) const;

    void FreeTable(ScDPObject* pDPObj);
    SC_DLLPUBLIC bool InsertNewTable(ScDPObject* pDPObj);

    bool HasDPTable(SCCOL nCol, SCROW nRow, SCTAB nTab) const;

    SheetCaches& GetSheetCaches();
    NameCaches& GetNameCaches();
    DBCaches& GetDBCaches();

private:
    /** Only to be called from ScDPCache::RemoveReference(). */
    void RemoveCache(const ScDPCache* pCache);

    void GetAllTables(const ScRange& rSrcRange, std::set<ScDPObject*>& rRefs) const;
    void GetAllTables(const rtl::OUString& rSrcName, std::set<ScDPObject*>& rRefs) const;
    void GetAllTables(
        sal_Int32 nSdbType, const ::rtl::OUString& rDBName, const ::rtl::OUString& rCommand,
        std::set<ScDPObject*>& rRefs) const;

private:
    typedef ::boost::ptr_vector<ScDPObject> TablesType;

    ScDocument* mpDoc;
    TablesType maTables;
    SheetCaches maSheetCaches;
    NameCaches maNameCaches;
    DBCaches maDBCaches;
};

bool operator<(const ScDPCollection::DBType& left, const ScDPCollection::DBType& right);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
