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

#include "global.hxx"
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XMembersSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotResults.hpp>
#include <com/sun/star/sheet/XDataPilotMemberResults.hpp>
#include <com/sun/star/sheet/MemberResult.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataPilotFieldAutoShowInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortInfo.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/sheet/XDrillDownDataSupplier.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

#include "dptabdat.hxx"
#include "dpglobal.hxx"
#include "dpresfilter.hxx"

#include <boost/noncopyable.hpp>
#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace com { namespace sun { namespace star {
    namespace sheet {
        struct DataPilotFieldFilter;
    }
    namespace table {
        struct CellAddress;
    }
}}}

class ScDPResultMember;
class ScDPResultData;
class ScDPItemData;
class ScDPTableData;

//  implementation of DataPilotSource using ScDPTableData

class ScDPDimensions;
class ScDPDimension;
class ScDPHierarchies;
class ScDPHierarchy;
class ScDPLevels;
class ScDPLevel;
class ScDPMembers;
class ScDPMember;

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
    ScDPDimensions*         pDimensions;        // api objects
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
    ScDPResultData*         pResData;           // keep the rest in this!
    ScDPResultMember*       pColResRoot;
    ScDPResultMember*       pRowResRoot;
    css::uno::Sequence<css::sheet::MemberResult>* pColResults;
    css::uno::Sequence<css::sheet::MemberResult>* pRowResults;
    std::vector<ScDPLevel*> aColLevelList;
    std::vector<ScDPLevel*> aRowLevelList;
    bool                    bResultOverflow;
    bool                    bPageFiltered;      // set if page field filters have been applied to cache table

    ::std::unique_ptr<OUString> mpGrandTotalName;

    void                    CreateRes_Impl();
    void                    FillMemberResults();
    void                    FillLevelList( sal_uInt16 nOrientation, std::vector<ScDPLevel*> &rList );
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
    virtual                     ~ScDPSource();

    ScDPTableData*          GetData()       { return pData; }
    const ScDPTableData*    GetData() const { return pData; }

    const OUString*  GetGrandTotalName() const;

    sal_uInt16                  GetOrientation(long nColumn);
    void                    SetOrientation(long nColumn, sal_uInt16 nNew);
    long                    GetPosition(long nColumn);

    long                    GetDataDimensionCount();
    ScDPDimension*          GetDataDimension(long nIndex);
    OUString GetDataDimName(long nIndex);
    const ScDPCache* GetCache();
    const ScDPItemData*         GetItemDataById( long nDim, long nId );
    SCROW                       GetMemberId(  long  nDim, const ScDPItemData& rData );
    bool                        IsDataLayoutDimension(long nDim);
    sal_uInt16                  GetDataLayoutOrientation();

    bool                        IsDateDimension(long nDim);

    bool                        SubTotalAllowed(long nColumn);      //! move to ScDPResultData

    ScDPDimension* AddDuplicated(long nSource, const OUString& rNewName);
    long                    GetDupCount() const { return nDupCount; }

    long                    GetSourceDim(long nDim);

    const css::uno::Sequence<css::sheet::MemberResult>*
                            GetMemberResults( ScDPLevel* pLevel );

    ScDPDimensions*         GetDimensionsObject();

                            // XDimensionsSupplier
    virtual css::uno::Reference< css::container::XNameAccess >
                            SAL_CALL getDimensions(  )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XDataPilotResults
    virtual css::uno::Sequence< css::uno::Sequence< css::sheet::DataResult > > SAL_CALL getResults(  )
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence<double> SAL_CALL
        getFilteredResults(
            const css::uno::Sequence<css::sheet::DataPilotFieldFilter>& aFilters )
                throw (css::uno::RuntimeException, std::exception) override;

                            // XRefreshable
    virtual void SAL_CALL   refresh() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XDrillDownDataSupplier
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > >
        SAL_CALL getDrillDownData(const css::uno::Sequence<
                                      css::sheet::DataPilotFieldFilter >& aFilters )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

#if DEBUG_PIVOT_TABLE
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
    ScDPDimension**     ppDims;

public:
                            ScDPDimensions( ScDPSource* pSrc );
    virtual                 ~ScDPDimensions();

    void                    CountChanged();

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    long            getCount() const;
    ScDPDimension*  getByIndex(long nIndex) const;
};

class ScDPDimension : boost::noncopyable, public cppu::WeakImplHelper<
                            css::sheet::XHierarchiesSupplier,
                            css::container::XNamed,
                            css::util::XCloneable,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >
{
    ScDPSource*         pSource;
    long                nDim;               // dimension index (== column ID)
    ScDPHierarchies*    pHierarchies;
    long                nUsedHier;
    sal_uInt16          nFunction;          // enum GeneralFunction
    OUString            aName;              // if empty, take from source
    std::unique_ptr<OUString> mpLayoutName;
    std::unique_ptr<OUString> mpSubtotalName;
    long                nSourceDim;         // >=0 if dup'ed
    css::sheet::DataPilotFieldReference
                        aReferenceValue;    // settings for "show data as" / "displayed value"
    bool                bHasSelectedPage;
    OUString            aSelectedPage;
    ScDPItemData*       pSelectedData;      // internal, temporary, created from aSelectedPage
    bool                mbHasHiddenMember;

public:
                            ScDPDimension( ScDPSource* pSrc, long nD );
    virtual                 ~ScDPDimension();

    long                    GetDimension() const    { return nDim; }        // dimension index in source
    long                    GetSourceDim() const    { return nSourceDim; }  // >=0 if dup'ed

    ScDPDimension*          CreateCloneObject();
    ScDPHierarchies*        GetHierarchiesObject();

    SC_DLLPUBLIC const OUString*  GetLayoutName() const;
    const OUString*  GetSubtotalName() const;

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XHierarchiesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
                            getHierarchies() throw(css::uno::RuntimeException, std::exception) override;

                            // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL
                            createClone() throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    sal_uInt16 getOrientation() const;
    void setOrientation(sal_uInt16 nNew);
    long getPosition() const;
    bool getIsDataLayoutDimension() const;
    sal_uInt16 getFunction() const { return nFunction;}
    void setFunction(sal_uInt16 nNew);       // for data dimension
    long getUsedHierarchy() const { return nUsedHier;}

    bool                        HasSelectedPage() const     { return bHasSelectedPage; }
    const ScDPItemData&         GetSelectedData();

    const css::sheet::DataPilotFieldReference& GetReferenceValue() const { return aReferenceValue;}
};

class ScDPHierarchies : public cppu::WeakImplHelper<
                            css::container::XNameAccess,
                            css::lang::XServiceInfo >
{
private:
    ScDPSource*         pSource;
    long                nDim;
    long                nHierCount;
    ScDPHierarchy**     ppHiers;

public:
                            ScDPHierarchies( ScDPSource* pSrc, long nD );
    virtual                 ~ScDPHierarchies();

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    long            getCount() const;
    ScDPHierarchy*  getByIndex(long nIndex) const;
};

class ScDPHierarchy : public cppu::WeakImplHelper<
                            css::sheet::XLevelsSupplier,
                            css::container::XNamed,
                            css::lang::XServiceInfo >
{
private:
    ScDPSource*     pSource;
    long            nDim;
    long            nHier;
    ScDPLevels*     pLevels;

public:
                            ScDPHierarchy( ScDPSource* pSrc, long nD, long nH );
    virtual                 ~ScDPHierarchy();

    ScDPLevels*             GetLevelsObject();

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XLevelsSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
                            getLevels() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScDPLevels : public cppu::WeakImplHelper<
                            css::container::XNameAccess,
                            css::lang::XServiceInfo >
{
private:
    ScDPSource*     pSource;
    long            nDim;
    long            nHier;
    long            nLevCount;
    ScDPLevel**     ppLevs;

public:
                            ScDPLevels( ScDPSource* pSrc, long nD, long nH );
    virtual                 ~ScDPLevels();

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

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
    long                        nDim;
    long                        nHier;
    long                        nLev;
    ScDPMembers*                pMembers;
    css::uno::Sequence<css::sheet::GeneralFunction> aSubTotals;
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
    virtual                 ~ScDPLevel();

    ScDPMembers*            GetMembersObject();

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XMembersSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
                            getMembers() throw(css::uno::RuntimeException, std::exception) override;

                            // XDataPilotMemberResults
    virtual css::uno::Sequence< css::sheet::MemberResult > SAL_CALL
                            getResults() throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<css::sheet::GeneralFunction> getSubTotals() const;
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
typedef std::unordered_map< OUString, sal_Int32, OUStringHash > ScDPMembersHashMap;

class ScDPMembers : public cppu::WeakImplHelper<
                            css::container::XNameAccess,
                            css::lang::XServiceInfo >
{
private:
    typedef std::vector<rtl::Reference<ScDPMember> > MembersType;
    ScDPSource*     pSource;
    long            nDim;
    long            nHier;
    long            nLev;
    long            nMbrCount;
    mutable MembersType maMembers;
    mutable ScDPMembersHashMap aHashMap;

public:
                            ScDPMembers( ScDPSource* pSrc, long nD, long nH, long nL );
    virtual                 ~ScDPMembers();

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    long                    getCount() const { return nMbrCount;}
    ScDPMember*             getByIndex(long nIndex) const;

    long                    getMinMembers() const;

    sal_Int32               GetIndexFromName( const OUString& rName ) const;     // <0 if not found
    const ScDPItemData*     GetSrcItemDataByIndex(  SCROW nIndex);
    SCROW                   GetSrcItemsCount();
};

class ScDPMember : boost::noncopyable, public cppu::WeakImplHelper<
                            css::container::XNamed,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >
{
private:
    ScDPSource*     pSource;
    long            nDim;
    long            nHier;
    long            nLev;

    SCROW       mnDataId;
    std::unique_ptr<OUString> mpLayoutName;

    sal_Int32       nPosition;          // manual sorting
    bool            bVisible;
    bool            bShowDet;

public:
    ScDPMember(ScDPSource* pSrc, long nD, long nH, long nL, SCROW nIndex);
    virtual                 ~ScDPMember();

    OUString GetNameStr() const;
    void                    FillItemData( ScDPItemData& rData ) const;
    const ScDPItemData*  GetItemData() const;
    SCROW GetItemDataId() const { return mnDataId; }
    bool IsNamedItem(SCROW nIndex) const;

    SC_DLLPUBLIC const OUString* GetLayoutName() const;
    long GetDim() const { return nDim;}

    sal_Int32               Compare( const ScDPMember& rOther ) const;      // visible order

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    bool isVisible() const { return bVisible;}
    bool getShowDetails() const { return bShowDet;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
