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
#include "collect.hxx"
#include "dpoutput.hxx"
#include "pivot.hxx"
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>

#include <boost/ptr_container/ptr_list.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>

//------------------------------------------------------------------

namespace com { namespace sun { namespace star { namespace sheet {

    struct DataPilotTablePositionData;
    struct DataPilotTableHeaderData;

}}}}

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
}}}}

class Rectangle;
class SvStream;
class ScDPSaveData;
class ScDPOutput;
class ScPivot;
class ScPivotCollection;
struct ScPivotParam;
struct ScImportSourceDesc;
struct ScSheetSourceDesc;
class ScStrCollection;
class TypedScStrCollection;
struct PivotField;
class ScDPCacheTable;
class ScDPTableData;

struct ScDPServiceDesc
{
    String  aServiceName;
    String  aParSource;
    String  aParName;
    String  aParUser;
    String  aParPass;

    ScDPServiceDesc( const String& rServ, const String& rSrc, const String& rNam,
                        const String& rUser, const String& rPass ) :
        aServiceName( rServ ), aParSource( rSrc ), aParName( rNam ),
        aParUser( rUser ), aParPass( rPass ) {  }

    BOOL operator== ( const ScDPServiceDesc& rOther ) const
        { return aServiceName == rOther.aServiceName &&
                 aParSource   == rOther.aParSource &&
                 aParName     == rOther.aParName &&
                 aParUser     == rOther.aParUser &&
                 aParPass     == rOther.aParPass; }
};


class SC_DLLPUBLIC ScDPObject
{
private:
    ScDocument*             pDoc;
                                            // settings
    ScDPSaveData*           pSaveData;
    String                  aTableName;
    String                  aTableTag;
    ScRange                 aOutRange;
    ScSheetSourceDesc*      pSheetDesc;     //  for sheet data
    ScImportSourceDesc*     pImpDesc;       //  for database data
    ScDPServiceDesc*        pServDesc;      //  for external service
    ::boost::shared_ptr<ScDPTableData>  mpTableData;
                                            // cached data
    com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier> xSource;
    ScDPOutput*             pOutput;
    BOOL                    bSettingsChanged;
    BOOL                    bAlive;         // FALSE if only used to hold settings
    sal_uInt16              mnAutoFormatIndex;
    BOOL                    bAllowMove;
    long                    nHeaderRows;    // page fields plus filter button
    bool                    mbHeaderLayout;  // TRUE : grid, FALSE : standard


    SC_DLLPRIVATE ScDPTableData*    GetTableData();
    SC_DLLPRIVATE void              CreateObjects();
    SC_DLLPRIVATE void              CreateOutput();
    BOOL                    bRefresh;
    long                        mnCacheId;

public:
    inline void SetRefresh() { bRefresh = TRUE; }
                ScDPObject( ScDocument* pD );
                ScDPObject(const ScDPObject& r);
                ~ScDPObject();

    /**
     * When a DP object is "alive", it has table output on a sheet.  This flag
     * doesn't really change the behavior of the object, but is used only for
     * testing purposes.
     */
    void                SetAlive(BOOL bSet);
    void                SetAllowMove(BOOL bSet);

    void                InvalidateData();
    void                InvalidateSource();


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

    BOOL                IsSheetData() const;
    BOOL                IsImportData() const { return(pImpDesc != NULL); }
    BOOL                IsServiceData() const { return(pServDesc != NULL); }

    void                SetName(const String& rNew);
    const String&       GetName() const                 { return aTableName; }
    void                SetTag(const String& rNew);
    const String&       GetTag() const                  { return aTableTag; }

    /**
     *  Data description cell displays the description of a data dimension if
     *  and only if there is only one data dimension.  It's usually located at
     *  the upper-left corner of the table output.
     */
    bool                IsDataDescriptionCell(const ScAddress& rPos);

    bool                IsDimNameInUse(const ::rtl::OUString& rName) const;
    String              GetDimName( long nDim, BOOL& rIsDataLayout, sal_Int32* pFlags = NULL );
    BOOL                IsDuplicated( long nDim );
    long                GetDimCount();
    void                GetHeaderPositionData(const ScAddress& rPos, ::com::sun::star::sheet::DataPilotTableHeaderData& rData);
    long                GetHeaderDim( const ScAddress& rPos, USHORT& rOrient );
    BOOL                GetHeaderDrag( const ScAddress& rPos, BOOL bMouseLeft, BOOL bMouseTop,
                                        long nDragDim,
                                        Rectangle& rPosRect, USHORT& rOrient, long& rDimPos );
    BOOL                IsFilterButton( const ScAddress& rPos );

    BOOL                GetPivotData( ScDPGetPivotDataField& rTarget, /* returns result */
                                      const std::vector< ScDPGetPivotDataField >& rFilters );
    BOOL                ParseFilters( ScDPGetPivotDataField& rTarget,
                                      std::vector< ScDPGetPivotDataField >& rFilters,
                                      const String& rFilterList );

    void                GetMemberResultNames( ScStrCollection& rNames, long nDimension );

    void                FillPageList( TypedScStrCollection& rStrings, long nField );

    void                ToggleDetails(const ::com::sun::star::sheet::DataPilotTableHeaderData& rElemDesc, ScDPObject* pDestObj);

    BOOL                FillOldParam(ScPivotParam& rParam, BOOL bForFile) const;
    BOOL                FillLabelData(ScPivotParam& rParam);
    void                InitFromOldPivot(const ScPivot& rOld, ScDocument* pDoc, BOOL bSetSource);

    BOOL                GetHierarchiesNA( sal_Int32 nDim, com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& xHiers );
    BOOL                GetHierarchies( sal_Int32 nDim, com::sun::star::uno::Sequence< rtl::OUString >& rHiers );

    sal_Int32           GetUsedHierarchy( sal_Int32 nDim );

    BOOL                GetMembersNA( sal_Int32 nDim, com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& xMembers );
    BOOL                GetMembersNA( sal_Int32 nDim, sal_Int32 nHier, com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& xMembers );

    bool                GetMemberNames( sal_Int32 nDim, ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames );
    bool                GetMembers( sal_Int32 nDim, sal_Int32 nHier, ::std::vector<ScDPLabelData::Member>& rMembers );

    void                UpdateReference( UpdateRefMode eUpdateRefMode,
                                         const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    BOOL                RefsEqual( const ScDPObject& r ) const;
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

    static BOOL         HasRegisteredSources();
    static com::sun::star::uno::Sequence<rtl::OUString> GetRegisteredSources();
    static com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier>
                        CreateSource( const ScDPServiceDesc& rDesc );

    static void         ConvertOrientation( ScDPSaveData& rSaveData,
                            const ::std::vector<PivotField>& rFields, USHORT nOrient,
                            const com::sun::star::uno::Reference<
                                com::sun::star::sheet::XDimensionsSupplier>& xSource,
                            ::std::vector<PivotField>* pRefColFields = NULL,
                            ::std::vector<PivotField>* pRefRowFields = NULL,
                            ::std::vector<PivotField>* pRefPageFields = NULL );

    static bool         IsOrientationAllowed( USHORT nOrient, sal_Int32 nDimFlags );
};


// ============================================================================

class ScDPCollection
{
private:
    ScDocument* pDoc;

    typedef ::boost::ptr_list<ScDPTableDataCache>   DataCachesType;
    typedef ::boost::ptr_vector<ScDPObject>         TablesType;

    TablesType      maTables;
    DataCachesType  maDPDataCaches;

public:
    ScDPCollection(ScDocument* pDocument);
    ScDPCollection(const ScDPCollection& r);
    ~ScDPCollection();

    SC_DLLPUBLIC size_t GetCount() const;
    SC_DLLPUBLIC ScDPObject* operator[](size_t nIndex);
    SC_DLLPUBLIC const ScDPObject* operator[](size_t nIndex) const;

    const ScDPObject* GetByName(const String& rName) const;

    void DeleteOnTab( SCTAB nTab );
    void UpdateReference( UpdateRefMode eUpdateRefMode,
                          const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

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
    String CreateNewName( USHORT nMin = 1 ) const;

    void FreeTable(ScDPObject* pDPObj);
    SC_DLLPUBLIC bool InsertNewTable(ScDPObject* pDPObj);

    bool HasDPTable(SCCOL nCol, SCROW nRow, SCTAB nTab) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
