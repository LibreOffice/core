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

#ifndef INCLUDED_SC_INC_DPTABSRC_HXX
#define INCLUDED_SC_INC_DPTABSRC_HXX

#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XMembersSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotResults.hpp>
#include <com/sun/star/sheet/XDataPilotMemberResults.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/sheet/XDrillDownDataSupplier.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

#include "dptabdat.hxx"
#include "dpresfilter.hxx"

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/optional.hpp>

namespace com { namespace sun { namespace star {
    namespace sheet {
        struct DataPilotFieldFilter;
        struct MemberResult;
    }
}}}

class ScDPResultMember;
class ScDPResultData;
class ScDPItemData;
class ScDPDimensions;
class ScDPDimension;
class ScDPHierarchies;
class ScDPHierarchy;
class ScDPLevels;
class ScDPLevel;
class ScDPMembers;
class ScDPMember;
enum class ScGeneralFunction;

//  implementation of DataPilotSource using ScDPTableData

class ScDPSource : public cppu::WeakImplHelper<
                            css::sheet::XDimensionsSupplier,
                            css::sheet::XDataPilotResults,
                            css::util::XRefreshable,
                            css::sheet::XDrillDownDataSupplier,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >
{
private:
    ScDPTableData*          pData;              // data source (ScDPObject manages its life time)
    rtl::Reference<ScDPDimensions> pDimensions; // api objects
                                                // settings:

    std::vector<long> maColDims;
    std::vector<long> maRowDims;
    std::vector<long> maDataDims;
    std::vector<long> maPageDims;
    ScDPResultTree maResFilterSet;

    bool                    bColumnGrand;
    bool                    bRowGrand;
    bool                    bIgnoreEmptyRows;
    bool                    bRepeatIfEmpty;

    long                    nDupCount;

                                                // results:
    std::unique_ptr<ScDPResultData>   pResData;           // keep the rest in this!
    std::unique_ptr<ScDPResultMember> pColResRoot;
    std::unique_ptr<ScDPResultMember> pRowResRoot;
    std::unique_ptr<css::uno::Sequence<css::sheet::MemberResult>[]> pColResults;
    std::unique_ptr<css::uno::Sequence<css::sheet::MemberResult>[]> pRowResults;
    std::vector<ScDPLevel*> aColLevelList;
    std::vector<ScDPLevel*> aRowLevelList;
    bool                    bResultOverflow;
    bool                    bPageFiltered;      // set if page field filters have been applied to cache table

    boost::optional<OUString> mpGrandTotalName;

    void                    CreateRes_Impl();
    void                    FillMemberResults();
    void                    FillLevelList( css::sheet::DataPilotFieldOrientation nOrientation, std::vector<ScDPLevel*> &rList );
    void                    FillCalcInfo(bool bIsRow, ScDPTableData::CalcInfo& rInfo, bool &bHasAutoShow);

    /**
     * Compile a list of dimension indices that are either, column, row or
     * page dimensions (i.e. all but data dimensions).
     */
    void                    GetCategoryDimensionIndices(std::unordered_set<sal_Int32>& rCatDims);

    /**
     * Set visibilities of individual rows in the cache table based on the
     * page field data.
     */
    void FilterCacheByPageDimensions();

    void                    SetDupCount( long nNew );

    OUString getDataDescription();       //! ???

    void setIgnoreEmptyRows(bool bSet);
    void setRepeatIfEmpty(bool bSet);

    void disposeData();

public:
                                ScDPSource( ScDPTableData* pD );
    virtual                     ~ScDPSource() override;

    ScDPTableData*          GetData()       { return pData; }
    const ScDPTableData*    GetData() const { return pData; }

    const boost::optional<OUString> &
                            GetGrandTotalName() const;

    css::sheet::DataPilotFieldOrientation
                            GetOrientation(long nColumn);
    void                    SetOrientation(long nColumn, css::sheet::DataPilotFieldOrientation nNew);
    long                    GetPosition(long nColumn);

    long                    GetDataDimensionCount() const;
    ScDPDimension*          GetDataDimension(long nIndex);
    OUString GetDataDimName(long nIndex);
    const ScDPCache* GetCache();
    const ScDPItemData*         GetItemDataById( long nDim, long nId );
    bool                        IsDataLayoutDimension(long nDim);
    css::sheet::DataPilotFieldOrientation
                                GetDataLayoutOrientation();

    bool                        IsDateDimension(long nDim);

    bool                        SubTotalAllowed(long nColumn);      //! move to ScDPResultData

    ScDPDimension* AddDuplicated(const OUString& rNewName);
    long                    GetDupCount() const { return nDupCount; }

    long                    GetSourceDim(long nDim);

    const css::uno::Sequence<css::sheet::MemberResult>*
                            GetMemberResults( const ScDPLevel* pLevel );

    ScDPDimensions*         GetDimensionsObject();

                            // XDimensionsSupplier
    virtual css::uno::Reference< css::container::XNameAccess >
                            SAL_CALL getDimensions(  ) override;

                            // XDataPilotResults
    virtual css::uno::Sequence< css::uno::Sequence< css::sheet::DataResult > > SAL_CALL getResults(  ) override;

    virtual css::uno::Sequence<double> SAL_CALL
        getFilteredResults(
            const css::uno::Sequence<css::sheet::DataPilotFieldFilter>& aFilters ) override;

                            // XRefreshable
    virtual void SAL_CALL   refresh() override;
    virtual void SAL_CALL   addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l ) override;
    virtual void SAL_CALL   removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l ) override;

                            // XDrillDownDataSupplier
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > >
        SAL_CALL getDrillDownData(const css::uno::Sequence<
                                      css::sheet::DataPilotFieldFilter >& aFilters ) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

#if DUMP_PIVOT_TABLE
    void DumpResults() const;
#endif
};

class ScDPDimensions : public cppu::WeakImplHelper<
                            css::container::XNameAccess,
                            css::lang::XServiceInfo >
{
private:
    ScDPSource*         pSource;
    long                nDimCount;
    std::unique_ptr<rtl::Reference<ScDPDimension>[]>
                        ppDims;

public:
                            ScDPDimensions( ScDPSource* pSrc );
    virtual                 ~ScDPDimensions() override;

    void                    CountChanged();

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    long            getCount() const;
    ScDPDimension*  getByIndex(long nIndex) const;
};

class ScDPDimension : public cppu::WeakImplHelper<
                            css::sheet::XHierarchiesSupplier,
                            css::container::XNamed,
                            css::util::XCloneable,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >
{
    ScDPSource*         pSource;
    long const          nDim;               // dimension index (== column ID)
    rtl::Reference<ScDPHierarchies> mxHierarchies;
    ScGeneralFunction   nFunction;
    OUString            aName;              // if empty, take from source
    boost::optional<OUString> mpLayoutName;
    boost::optional<OUString> mpSubtotalName;
    long                nSourceDim;         // >=0 if dup'ed
    css::sheet::DataPilotFieldReference
                        aReferenceValue;    // settings for "show data as" / "displayed value"
    bool                bHasSelectedPage;
    OUString            aSelectedPage;
    std::unique_ptr<ScDPItemData>
                        pSelectedData;      // internal, temporary, created from aSelectedPage
    bool                mbHasHiddenMember;

public:
                            ScDPDimension( ScDPSource* pSrc, long nD );
    virtual                 ~ScDPDimension() override;
                            ScDPDimension(const ScDPDimension&) = delete;
    ScDPDimension&          operator=(const ScDPDimension&) = delete;

    long                    GetDimension() const    { return nDim; }        // dimension index in source
    long                    GetSourceDim() const    { return nSourceDim; }  // >=0 if dup'ed

    ScDPDimension*          CreateCloneObject();
    ScDPHierarchies*        GetHierarchiesObject();

    SC_DLLPUBLIC const boost::optional<OUString> & GetLayoutName() const;
    const boost::optional<OUString> & GetSubtotalName() const;

                            // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL   setName( const OUString& aName ) override;

                            // XHierarchiesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
                            getHierarchies() override;

                            // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL
                            createClone() override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    css::sheet::DataPilotFieldOrientation getOrientation() const;
    bool getIsDataLayoutDimension() const;
    ScGeneralFunction getFunction() const { return nFunction;}
    void setFunction(ScGeneralFunction nNew);       // for data dimension
    static long getUsedHierarchy() { return 0;}

    bool                        HasSelectedPage() const     { return bHasSelectedPage; }
    const ScDPItemData&         GetSelectedData();

    const css::sheet::DataPilotFieldReference& GetReferenceValue() const { return aReferenceValue;}
};

class ScDPHierarchies : public cppu::WeakImplHelper<
                            css::container::XNameAccess,
                            css::lang::XServiceInfo >
{
private:
    ScDPSource* const   pSource;
    long const          nDim;
    //  date columns have 3 hierarchies (flat/quarter/week), other columns only one
    // #i52547# don't offer the incomplete date hierarchy implementation
    static const long   nHierCount = 1;
    std::unique_ptr<rtl::Reference<ScDPHierarchy>[]>
                        ppHiers;

public:
                            ScDPHierarchies( ScDPSource* pSrc, long nD );
    virtual                 ~ScDPHierarchies() override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static long     getCount();
    ScDPHierarchy*  getByIndex(long nIndex) const;
};

class ScDPHierarchy : public cppu::WeakImplHelper<
                            css::sheet::XLevelsSupplier,
                            css::container::XNamed,
                            css::lang::XServiceInfo >
{
private:
    ScDPSource* const     pSource;
    long const            nDim;
    long const            nHier;
    rtl::Reference<ScDPLevels> mxLevels;

public:
                            ScDPHierarchy( ScDPSource* pSrc, long nD, long nH );
    virtual                 ~ScDPHierarchy() override;

    ScDPLevels*             GetLevelsObject();

                            // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL   setName( const OUString& aName ) override;

                            // XLevelsSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
                            getLevels() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScDPLevels : public cppu::WeakImplHelper<
                            css::container::XNameAccess,
                            css::lang::XServiceInfo >
{
private:
    ScDPSource*     pSource;
    long const      nDim;
    long const      nHier;
    long            nLevCount;
    std::unique_ptr<rtl::Reference<ScDPLevel>[]>
                    ppLevs;

public:
                            ScDPLevels( ScDPSource* pSrc, long nD, long nH );
    virtual                 ~ScDPLevels() override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    long            getCount() const;
    ScDPLevel*      getByIndex(long nIndex) const;
};

class ScDPLevel : public cppu::WeakImplHelper<
                            css::sheet::XMembersSupplier,
                            css::container::XNamed,
                            css::sheet::XDataPilotMemberResults,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >
{
private:
    ScDPSource*                 pSource;
    long const                        nDim;
    long const                        nHier;
    long const                        nLev;
    rtl::Reference<ScDPMembers> mxMembers;
    css::uno::Sequence<sal_Int16> aSubTotals;
    css::sheet::DataPilotFieldSortInfo     aSortInfo;      // stored user settings
    css::sheet::DataPilotFieldAutoShowInfo aAutoShowInfo;  // stored user settings
    css::sheet::DataPilotFieldLayoutInfo   aLayoutInfo;    // stored user settings
                                                    // valid only from result calculation:
    ::std::vector<sal_Int32>    aGlobalOrder;       // result of sorting by name or position
    long                        nSortMeasure;       // measure (index of data dimension) to sort by
    long                        nAutoMeasure;       // measure (index of data dimension) for AutoShow
    bool                        bShowEmpty:1;
    bool                        bEnableLayout:1;      // enabled only for row fields, not for the innermost one
    bool                        bRepeatItemLabels:1;

public:
                            ScDPLevel( ScDPSource* pSrc, long nD, long nH, long nL );
    virtual                 ~ScDPLevel() override;

    ScDPMembers*            GetMembersObject();

                            // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL   setName( const OUString& aName ) override;

                            // XMembersSupplier
    virtual css::uno::Reference< css::sheet::XMembersAccess > SAL_CALL
                            getMembers() override;

                            // XDataPilotMemberResults
    virtual css::uno::Sequence< css::sheet::MemberResult > SAL_CALL
                            getResults() override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    css::uno::Sequence<sal_Int16> getSubTotals() const;
    bool getShowEmpty() const { return bShowEmpty;}
    bool getRepeatItemLabels() const { return bRepeatItemLabels; }

    const css::sheet::DataPilotFieldSortInfo& GetSortInfo() const      { return aSortInfo; }
    const css::sheet::DataPilotFieldAutoShowInfo& GetAutoShow() const  { return aAutoShowInfo; }

    void EvaluateSortOrder();
    void SetEnableLayout(bool bSet);

    const ::std::vector<sal_Int32>& GetGlobalOrder() const      { return aGlobalOrder; }
    ::std::vector<sal_Int32>&  GetGlobalOrder()                 { return aGlobalOrder; }
    long                    GetSortMeasure() const              { return nSortMeasure; }
    long                    GetAutoMeasure() const              { return nAutoMeasure; }

    bool IsOutlineLayout() const
    {
        return bEnableLayout &&
            aLayoutInfo.LayoutMode !=
            css::sheet::DataPilotFieldLayoutMode::TABULAR_LAYOUT;
    }

    bool IsSubtotalsAtTop() const
    {
        return bEnableLayout &&
            aLayoutInfo.LayoutMode ==
            css::sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_TOP;
    }

    bool IsAddEmpty() const
    {
        return bEnableLayout && aLayoutInfo.AddEmptyLines;
    }

    //! number format (for data fields and date fields)
};

// hash map from name to index in the member array, for fast name access
typedef std::unordered_map< OUString, sal_Int32 > ScDPMembersHashMap;

class ScDPMembers : public cppu::WeakImplHelper<
                            css::sheet::XMembersAccess,
                            css::lang::XServiceInfo >
{
private:
    typedef std::vector<rtl::Reference<ScDPMember> > MembersType;
    ScDPSource*     pSource;
    long            nDim;
    long            nHier;
    long const      nLev;
    long            nMbrCount;
    mutable MembersType maMembers;
    mutable ScDPMembersHashMap aHashMap;

public:
                            ScDPMembers( ScDPSource* pSrc, long nD, long nH, long nL );
    virtual                 ~ScDPMembers() override;

                            // XMembersAccess
    virtual css::uno::Sequence< OUString > SAL_CALL getLocaleIndependentElementNames() override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    long                    getCount() const { return nMbrCount;}
    ScDPMember*             getByIndex(long nIndex) const;

    long                    getMinMembers() const;

    sal_Int32               GetIndexFromName( const OUString& rName ) const;     // <0 if not found
    const ScDPItemData*     GetSrcItemDataByIndex(  SCROW nIndex);

private:
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< OUString > getElementNames( bool bLocaleIndependent ) const;
};

class ScDPMember : public cppu::WeakImplHelper<
                            css::container::XNamed,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >
{
private:
    ScDPSource*     pSource;
    long            nDim;
    long const      nHier;
    long const      nLev;

    SCROW const     mnDataId;
    boost::optional<OUString> mpLayoutName;

    sal_Int32       nPosition;          // manual sorting
    bool            bVisible;
    bool            bShowDet;

public:
    ScDPMember(ScDPSource* pSrc, long nD, long nH, long nL, SCROW nIndex);
    virtual                 ~ScDPMember() override;
    ScDPMember(const ScDPMember&) = delete;
    ScDPMember& operator=(const ScDPMember&) = delete;

    OUString GetNameStr( bool bLocaleIndependent ) const;
    ScDPItemData FillItemData() const;
    const ScDPItemData*  GetItemData() const;
    SCROW GetItemDataId() const { return mnDataId; }
    bool IsNamedItem(SCROW nIndex) const;

    SC_DLLPUBLIC const boost::optional<OUString> & GetLayoutName() const;
    long GetDim() const { return nDim;}

    sal_Int32               Compare( const ScDPMember& rOther ) const;      // visible order

                            // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL   setName( const OUString& aName ) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    bool isVisible() const { return bVisible;}
    bool getShowDetails() const { return bShowDet;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
