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

#ifndef SC_DPTABSRC_HXX
#define SC_DPTABSRC_HXX

#include <tools/string.hxx>
#include "global.hxx"       // enum ScSubTotalFunc
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
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase6.hxx>
#include "rtl/ref.hxx"

#include "dptabdat.hxx"
#include "dpglobal.hxx"

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <list>

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

//
//  implementation of DataPilotSource using ScDPTableData
//


class ScDPDimensions;
class ScDPDimension;
class ScDPHierarchies;
class ScDPHierarchy;
class ScDPLevels;
class ScDPLevel;
class ScDPMembers;
class ScDPMember;


class ScDPSource : public cppu::WeakImplHelper6<
                            com::sun::star::sheet::XDimensionsSupplier,
                            com::sun::star::sheet::XDataPilotResults,
                            com::sun::star::util::XRefreshable,
                            com::sun::star::sheet::XDrillDownDataSupplier,
                            com::sun::star::beans::XPropertySet,
                            com::sun::star::lang::XServiceInfo >
{
private:
    ScDPTableData*          pData;              // data source (ScDPObject manages its life time)
    ScDPDimensions*         pDimensions;        // api objects
                                                // settings:
    long                    nColDims[SC_DP_MAX_FIELDS];
    long                    nRowDims[SC_DP_MAX_FIELDS];
    long                    nDataDims[SC_DP_MAX_FIELDS];
    long                    nPageDims[SC_DP_MAX_FIELDS];
    long                    nColDimCount;
    long                    nRowDimCount;
    long                    nDataDimCount;
    long                    nPageDimCount;
    bool                    bColumnGrand;
    bool                    bRowGrand;
    bool                    bIgnoreEmptyRows;
    bool                    bRepeatIfEmpty;

    long                    nDupCount;

                                                // results:
    ScDPResultData*         pResData;           // keep the rest in this!
    ScDPResultMember*       pColResRoot;
    ScDPResultMember*       pRowResRoot;
    com::sun::star::uno::Sequence<com::sun::star::sheet::MemberResult>* pColResults;
    com::sun::star::uno::Sequence<com::sun::star::sheet::MemberResult>* pRowResults;
    std::vector<ScDPLevel*> aColLevelList;
    std::vector<ScDPLevel*> aRowLevelList;
    bool                    bResultOverflow;
    bool                    bPageFiltered;      // set if page field filters have been applied to cache table

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<rtl::OUString> mpGrandTotalName;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    void                    CreateRes_Impl();
    void                    FillMemberResults();
    void                    FillLevelList( sal_uInt16 nOrientation, std::vector<ScDPLevel*> &rList );
    void                    FillCalcInfo(bool bIsRow, ScDPTableData::CalcInfo& rInfo, bool &bHasAutoShow);

    /**
     * Compile a list of dimension indices that are either, column, row or
     * page dimensions (i.e. all but data dimensions).
     */
    void                    GetCategoryDimensionIndices(::boost::unordered_set<sal_Int32>& rCatDims);

    /**
     * Set visibilities of individual rows in the cache table based on the
     * page field data.
     */
    void FilterCacheByPageDimensions();

    void                    SetDupCount( long nNew );

    rtl::OUString getDataDescription();       //! ???

    void setIgnoreEmptyRows(bool bSet);
    void setRepeatIfEmpty(bool bSet);

    void disposeData();

public:
                                ScDPSource( ScDPTableData* pD );
    virtual                     ~ScDPSource();

    ScDPTableData*          GetData()       { return pData; }
    const ScDPTableData*    GetData() const { return pData; }

    const ::rtl::OUString*  GetGrandTotalName() const;

    sal_uInt16                  GetOrientation(long nColumn);
    void                    SetOrientation(long nColumn, sal_uInt16 nNew);
    long                    GetPosition(long nColumn);

    long                    GetDataDimensionCount();
    ScDPDimension*          GetDataDimension(long nIndex);
    rtl::OUString GetDataDimName(long nIndex);
    const ScDPCache* GetCache();
    const ScDPItemData*            GetItemDataById( long nDim, long nId );
    long                                       GetDataLayoutDim(){ return pData->GetColumnCount(); }
    SCROW                                GetMemberId(  long  nDim, const ScDPItemData& rData );
    sal_Bool                    IsDataLayoutDimension(long nDim);
    sal_uInt16                  GetDataLayoutOrientation();

    sal_Bool                    IsDateDimension(long nDim);

    sal_Bool                    SubTotalAllowed(long nColumn);      //! move to ScDPResultData

    ScDPDimension* AddDuplicated(long nSource, const rtl::OUString& rNewName);
    long                    GetDupCount() const { return nDupCount; }

    long                    GetSourceDim(long nDim);

    const com::sun::star::uno::Sequence<com::sun::star::sheet::MemberResult>*
                            GetMemberResults( ScDPLevel* pLevel );

    ScDPDimensions*         GetDimensionsObject();

                            // XDimensionsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                            SAL_CALL getDimensions(  )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XDataPilotResults
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence<
                            ::com::sun::star::sheet::DataResult > > SAL_CALL getResults(  )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XRefreshable
    virtual void SAL_CALL   refresh() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addRefreshListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XRefreshListener >& l )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeRefreshListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XRefreshListener >& l )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XDrillDownDataSupplier
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >
        SAL_CALL getDrillDownData(const ::com::sun::star::uno::Sequence<
                                      ::com::sun::star::sheet::DataPilotFieldFilter >& aFilters )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};

class ScDPDimensions : public cppu::WeakImplHelper2<
                            com::sun::star::container::XNameAccess,
                            com::sun::star::lang::XServiceInfo >
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
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

    virtual long            getCount() const;
    virtual ScDPDimension*  getByIndex(long nIndex) const;
};

class ScDPDimension : boost::noncopyable, public cppu::WeakImplHelper5<
                            com::sun::star::sheet::XHierarchiesSupplier,
                            com::sun::star::container::XNamed,
                            com::sun::star::util::XCloneable,
                            com::sun::star::beans::XPropertySet,
                            com::sun::star::lang::XServiceInfo >
{
    ScDPSource*         pSource;
    long                nDim;               // dimension index (== column ID)
    ScDPHierarchies*    pHierarchies;
    long                nUsedHier;
    sal_uInt16              nFunction;          // enum GeneralFunction
    rtl::OUString       aName;              // if empty, take from source
    boost::scoped_ptr<rtl::OUString> mpLayoutName;
    boost::scoped_ptr<rtl::OUString> mpSubtotalName;
    long                nSourceDim;         // >=0 if dup'ed
    ::com::sun::star::sheet::DataPilotFieldReference
                        aReferenceValue;    // settings for "show data as" / "displayed value"
    bool                bHasSelectedPage;
    rtl::OUString       aSelectedPage;
    ScDPItemData*       pSelectedData;      // internal, temporary, created from aSelectedPage
    bool                mbHasHiddenMember;

public:
                            ScDPDimension( ScDPSource* pSrc, long nD );
    virtual                 ~ScDPDimension();

    long                    GetDimension() const    { return nDim; }        // dimension index in source
    long                    GetSourceDim() const    { return nSourceDim; }  // >=0 if dup'ed

    ScDPDimension*          CreateCloneObject();
    ScDPHierarchies*        GetHierarchiesObject();

    SC_DLLPUBLIC const ::rtl::OUString*  GetLayoutName() const;
    const ::rtl::OUString*  GetSubtotalName() const;

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XHierarchiesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getHierarchies() throw(::com::sun::star::uno::RuntimeException);

                            // XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL
                            createClone() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

    sal_uInt16 getOrientation() const;
    void setOrientation(sal_uInt16 nNew);
    long getPosition() const;
    bool getIsDataLayoutDimension() const;
    sal_uInt16 getFunction() const;
    void setFunction(sal_uInt16 nNew);       // for data dimension
    long getUsedHierarchy() const;
    void setUsedHierarchy(long nNew);

    bool                        HasSelectedPage() const     { return bHasSelectedPage; }
    const ScDPItemData&         GetSelectedData();

    const ::com::sun::star::sheet::DataPilotFieldReference& GetReferenceValue() const;
};

class ScDPHierarchies : public cppu::WeakImplHelper2<
                            com::sun::star::container::XNameAccess,
                            com::sun::star::lang::XServiceInfo >
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
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

    virtual long            getCount() const;
    virtual ScDPHierarchy*  getByIndex(long nIndex) const;
};

class ScDPHierarchy : public cppu::WeakImplHelper3<
                            com::sun::star::sheet::XLevelsSupplier,
                            com::sun::star::container::XNamed,
                            com::sun::star::lang::XServiceInfo >
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
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XLevelsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getLevels() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};

class ScDPLevels : public cppu::WeakImplHelper2<
                            com::sun::star::container::XNameAccess,
                            com::sun::star::lang::XServiceInfo >
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
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

    virtual long            getCount() const;
    virtual ScDPLevel*      getByIndex(long nIndex) const;
};

class ScDPLevel : public cppu::WeakImplHelper5<
                            com::sun::star::sheet::XMembersSupplier,
                            com::sun::star::container::XNamed,
                            com::sun::star::sheet::XDataPilotMemberResults,
                            com::sun::star::beans::XPropertySet,
                            com::sun::star::lang::XServiceInfo >
{
private:
    ScDPSource*                 pSource;
    long                        nDim;
    long                        nHier;
    long                        nLev;
    ScDPMembers*                pMembers;
    com::sun::star::uno::Sequence<com::sun::star::sheet::GeneralFunction> aSubTotals;
    ::com::sun::star::sheet::DataPilotFieldSortInfo     aSortInfo;      // stored user settings
    ::com::sun::star::sheet::DataPilotFieldAutoShowInfo aAutoShowInfo;  // stored user settings
    ::com::sun::star::sheet::DataPilotFieldLayoutInfo   aLayoutInfo;    // stored user settings
                                                    // valid only from result calculation:
    ::std::vector<sal_Int32>    aGlobalOrder;       // result of sorting by name or position
    long                        nSortMeasure;       // measure (index of data dimension) to sort by
    long                        nAutoMeasure;       // measure (index of data dimension) for AutoShow
    bool                        bShowEmpty:1;
    bool                        bEnableLayout:1;      // enabled only for row fields, not for the innermost one

public:
                            ScDPLevel( ScDPSource* pSrc, long nD, long nH, long nL );
    virtual                 ~ScDPLevel();

    ScDPMembers*            GetMembersObject();

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XMembersSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getMembers() throw(::com::sun::star::uno::RuntimeException);

                            // XDataPilotMemberResults
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::MemberResult > SAL_CALL
                            getResults() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

    com::sun::star::uno::Sequence<com::sun::star::sheet::GeneralFunction> getSubTotals() const;
    bool getShowEmpty() const;

    const ::com::sun::star::sheet::DataPilotFieldSortInfo& GetSortInfo() const      { return aSortInfo; }
    const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo& GetAutoShow() const  { return aAutoShowInfo; }

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
            ::com::sun::star::sheet::DataPilotFieldLayoutMode::TABULAR_LAYOUT;
    }

    bool IsSubtotalsAtTop() const
    {
        return bEnableLayout &&
            aLayoutInfo.LayoutMode ==
            ::com::sun::star::sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_TOP;
    }

    bool IsAddEmpty() const
    {
        return bEnableLayout && aLayoutInfo.AddEmptyLines;
    }

    //! number format (for data fields and date fields)
};

// hash map from name to index in the member array, for fast name access
typedef ::boost::unordered_map< ::rtl::OUString, sal_Int32, ::rtl::OUStringHash > ScDPMembersHashMap;

class ScDPMembers : public cppu::WeakImplHelper2<
                            com::sun::star::container::XNameAccess,
                            com::sun::star::lang::XServiceInfo >
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
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

    long getCount() const;
    ScDPMember* getByIndex(long nIndex) const;

    long                    getMinMembers() const;

    sal_Int32               GetIndexFromName( const ::rtl::OUString& rName ) const;     // <0 if not found
    const std::vector<sal_Int32>&    GetGlobalOrder();
    const ScDPItemData*               GetSrcItemDataByIndex(  SCROW nIndex);
    SCROW                                   GetSrcItemsCount();
};

class ScDPMember : boost::noncopyable, public cppu::WeakImplHelper3<
                            com::sun::star::container::XNamed,
                            com::sun::star::beans::XPropertySet,
                            com::sun::star::lang::XServiceInfo >
{
private:
    ScDPSource*     pSource;
    long            nDim;
    long            nHier;
    long            nLev;

    SCROW       mnDataId;
    boost::scoped_ptr<rtl::OUString> mpLayoutName;

    sal_Int32       nPosition;          // manual sorting
    bool            bVisible;
    bool            bShowDet;

public:
    ScDPMember(ScDPSource* pSrc, long nD, long nH, long nL, SCROW nIndex);
    virtual                 ~ScDPMember();

    rtl::OUString GetNameStr() const;
    void                    FillItemData( ScDPItemData& rData ) const;
    const ScDPItemData*  GetItemData() const;
    SCROW GetItemDataId() const { return mnDataId; }
    bool IsNamedItem(SCROW nIndex) const;

    SC_DLLPUBLIC const ::rtl::OUString* GetLayoutName() const;
    long GetDim() const;

    sal_Int32               Compare( const ScDPMember& rOther ) const;      // visible order

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

    bool isVisible() const;
    bool getShowDetails() const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
