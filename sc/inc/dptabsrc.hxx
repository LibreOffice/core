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

#include <vector>
#include <hash_map>
// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
#include <list>
// End Comments
#include <memory>
#include <tools/string.hxx>
#include <tools/list.hxx>
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
// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
#include "dpglobal.hxx"
// End Comments
#include "dptabdat.hxx"

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
// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
class ScDPItemData;
// End Comments
class ScDPTableData;

// ------------------------------------------------------------------------


//  should be dynamic!
#define SC_DAPI_MAXFIELDS   256


// --------------------------------------------------------------------
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
    long                    nColDims[SC_DAPI_MAXFIELDS];
    long                    nRowDims[SC_DAPI_MAXFIELDS];
    long                    nDataDims[SC_DAPI_MAXFIELDS];
    long                    nPageDims[SC_DAPI_MAXFIELDS];
    long                    nColDimCount;
    long                    nRowDimCount;
    long                    nDataDimCount;
    long                    nPageDimCount;
    sal_Bool                    bColumnGrand;
    sal_Bool                    bRowGrand;
    sal_Bool                    bIgnoreEmptyRows;
    sal_Bool                    bRepeatIfEmpty;

    long                    nDupCount;

                                                // results:
    ScDPResultData*         pResData;           // keep the rest in this!
    ScDPResultMember*       pColResRoot;
    ScDPResultMember*       pRowResRoot;
    com::sun::star::uno::Sequence<com::sun::star::sheet::MemberResult>* pColResults;
    com::sun::star::uno::Sequence<com::sun::star::sheet::MemberResult>* pRowResults;
    List                    aColLevelList;
    List                    aRowLevelList;
    sal_Bool                    bResultOverflow;

    ::std::auto_ptr<rtl::OUString> mpGrandTotalName;

    void                    CreateRes_Impl();
    void                    FillMemberResults();
    void                    FillLevelList( sal_uInt16 nOrientation, List& rList );
    void                    FillCalcInfo(bool bIsRow, ScDPTableData::CalcInfo& rInfo, bool &bHasAutoShow);

    /**
     * Compile a list of dimension indices that are either, column, row or
     * page dimensions (i.e. all but data dimensions).
     */
    void                    GetCategoryDimensionIndices(::std::hash_set<sal_Int32>& rCatDims);

    /**
     * Set visibilities of individual rows in the cache table based on the
     * page field data.
     */
    void                    FilterCacheTableByPageDimensions();

    void                    SetDupCount( long nNew );

public:
                                ScDPSource( ScDPTableData* pD );    // TableData is deleted by Source
    virtual                     ~ScDPSource();

    ScDPTableData*          GetData()       { return pData; }
    const ScDPTableData*    GetData() const { return pData; }

    void                    SetGrandTotalName(const ::rtl::OUString& rName);
    const ::rtl::OUString*  GetGrandTotalName() const;

    sal_uInt16                  GetOrientation(long nColumn);
    void                    SetOrientation(long nColumn, sal_uInt16 nNew);
    long                    GetPosition(long nColumn);

    long                    GetDataDimensionCount();
    ScDPDimension*          GetDataDimension(long nIndex);
    String                  GetDataDimName(long nIndex);
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    ScDPTableDataCache*         GetCache();
    const ScDPItemData*            GetItemDataById( long nDim, long nId );
    long                                       GetDataLayoutDim(){ return pData->GetColumnCount(); }
    SCROW                                GetMemberId(  long  nDim, const ScDPItemData& rData );
    // End Comments
    sal_Bool                    IsDataLayoutDimension(long nDim);
    sal_uInt16                  GetDataLayoutOrientation();

    sal_Bool                    IsDateDimension(long nDim);
    sal_uInt32                  GetNumberFormat(long nDim);

    sal_Bool                    SubTotalAllowed(long nColumn);      //! move to ScDPResultData

    ScDPDimension*          AddDuplicated(long nSource, const String& rNewName);
    long                    GetDupCount() const { return nDupCount; }

    long                    GetSourceDim(long nDim);

    const com::sun::star::uno::Sequence<com::sun::star::sheet::MemberResult>*
                            GetMemberResults( ScDPLevel* pLevel );

    ScDPDimensions*         GetDimensionsObject();

//UNUSED2009-05 void                    DumpState( ScDocument* pDoc, const ScAddress& rPos );

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

    virtual String              getDataDescription();       //! ???

    virtual sal_Bool                getColumnGrand() const;
    virtual void                setColumnGrand(sal_Bool bSet);
    virtual sal_Bool                getRowGrand() const;
    virtual void                setRowGrand(sal_Bool bSet);

    virtual sal_Bool                getIgnoreEmptyRows() const;
    virtual void                setIgnoreEmptyRows(sal_Bool bSet);
    virtual sal_Bool                getRepeatIfEmpty() const;
    virtual void                setRepeatIfEmpty(sal_Bool bSet);

    virtual void                validate();     //! ???
    virtual void                disposeData();
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

class ScDPDimension : public cppu::WeakImplHelper5<
                            com::sun::star::sheet::XHierarchiesSupplier,
                            com::sun::star::container::XNamed,
                            com::sun::star::util::XCloneable,
                            com::sun::star::beans::XPropertySet,
                            com::sun::star::lang::XServiceInfo >
{
private:
    ScDPSource*         pSource;
    long                nDim;               // dimension index (== column ID)
    ScDPHierarchies*    pHierarchies;
    long                nUsedHier;
    sal_uInt16              nFunction;          // enum GeneralFunction
    String              aName;              // if empty, take from source
    ::std::auto_ptr<rtl::OUString> mpLayoutName;
    ::std::auto_ptr<rtl::OUString> mpSubtotalName;
    long                nSourceDim;         // >=0 if dup'ed
    ::com::sun::star::sheet::DataPilotFieldReference
                        aReferenceValue;    // settings for "show data as" / "displayed value"
    sal_Bool                bHasSelectedPage;
    String              aSelectedPage;
    ScDPItemData*       pSelectedData;      // internal, temporary, created from aSelectedPage
    sal_Bool            mbHasHiddenMember;

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

    virtual sal_uInt16              getOrientation() const;
    virtual void                setOrientation(sal_uInt16 nNew);
    virtual long                getPosition() const;
    virtual void                setPosition(long nNew);
    virtual sal_Bool                getIsDataLayoutDimension() const;
    virtual sal_uInt16              getFunction() const;
    virtual void                setFunction(sal_uInt16 nNew);       // for data dimension
    virtual long                getUsedHierarchy() const;
    virtual void                setUsedHierarchy(long nNew);
    virtual sal_Bool                isDuplicated() const;

    sal_Bool                        HasSelectedPage() const     { return bHasSelectedPage; }
    const ScDPItemData&         GetSelectedData();

    const ::com::sun::star::sheet::DataPilotFieldReference& GetReferenceValue() const;

//UNUSED2009-05 sal_Bool                        IsValidPage( const ScDPItemData& rData );
// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
    sal_Bool                      IsVisible( const ScDPItemData& rData );
// End Comments
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
    sal_Bool                        bShowEmpty;
    ::com::sun::star::sheet::DataPilotFieldSortInfo     aSortInfo;      // stored user settings
    ::com::sun::star::sheet::DataPilotFieldAutoShowInfo aAutoShowInfo;  // stored user settings
    ::com::sun::star::sheet::DataPilotFieldLayoutInfo   aLayoutInfo;    // stored user settings
                                                    // valid only from result calculation:
    ::std::vector<sal_Int32>    aGlobalOrder;       // result of sorting by name or position
    long                        nSortMeasure;       // measure (index of data dimension) to sort by
    long                        nAutoMeasure;       // measure (index of data dimension) for AutoShow
    sal_Bool                        bEnableLayout;      // enabled only for row fields, not for the innermost one

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

    virtual com::sun::star::uno::Sequence<com::sun::star::sheet::GeneralFunction> getSubTotals() const;
    virtual void            setSubTotals(const com::sun::star::uno::Sequence<
                                            com::sun::star::sheet::GeneralFunction>& rNew);
    virtual sal_Bool            getShowEmpty() const;
    virtual void            setShowEmpty(sal_Bool bSet);

    const ::com::sun::star::sheet::DataPilotFieldSortInfo& GetSortInfo() const      { return aSortInfo; }
    const ::com::sun::star::sheet::DataPilotFieldAutoShowInfo& GetAutoShow() const  { return aAutoShowInfo; }

    void                    EvaluateSortOrder();
    void                    SetEnableLayout( sal_Bool bSet );

    const ::std::vector<sal_Int32>& GetGlobalOrder() const      { return aGlobalOrder; }
    ::std::vector<sal_Int32>&  GetGlobalOrder()                 { return aGlobalOrder; }
    long                    GetSortMeasure() const              { return nSortMeasure; }
    long                    GetAutoMeasure() const              { return nAutoMeasure; }

    sal_Bool                    IsOutlineLayout() const
                            { return bEnableLayout &&
                               aLayoutInfo.LayoutMode !=
                                ::com::sun::star::sheet::DataPilotFieldLayoutMode::TABULAR_LAYOUT; }

    sal_Bool                    IsSubtotalsAtTop() const
                            { return bEnableLayout &&
                               aLayoutInfo.LayoutMode ==
                                ::com::sun::star::sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_TOP; }

    sal_Bool                    IsAddEmpty() const          { return bEnableLayout && aLayoutInfo.AddEmptyLines; }

    //! number format (for data fields and date fields)
};

// hash map from name to index in the member array, for fast name access
typedef ::std::hash_map< ::rtl::OUString, sal_Int32, ::rtl::OUStringHash > ScDPMembersHashMap;

class ScDPMembers : public cppu::WeakImplHelper2<
                            com::sun::star::container::XNameAccess,
                            com::sun::star::lang::XServiceInfo >
{
private:
    ScDPSource*     pSource;
    long            nDim;
    long            nHier;
    long            nLev;
    long            nMbrCount;
    ScDPMember**    ppMbrs;
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

    virtual long            getCount() const;
    virtual ScDPMember*     getByIndex(long nIndex) const;

    long                    getMinMembers() const;

    sal_Int32               GetIndexFromName( const ::rtl::OUString& rName ) const;     // <0 if not found
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    const std::vector<sal_Int32>&    GetGlobalOrder();
    const ScDPItemData*               GetSrcItemDataByIndex(  SCROW nIndex);
    SCROW                                   GetSrcItemsCount();
    // End Comments
};

class ScDPMember : public cppu::WeakImplHelper3<
                            com::sun::star::container::XNamed,
                            com::sun::star::beans::XPropertySet,
                            com::sun::star::lang::XServiceInfo >
{
private:
    ScDPSource*     pSource;
    long            nDim;
    long            nHier;
    long            nLev;

    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    SCROW       mnDataId;
    // End Comments
//  String          aCaption;           // visible name (changeable by user)
    ::std::auto_ptr<rtl::OUString> mpLayoutName;

    sal_Int32       nPosition;          // manual sorting
    sal_Bool            bVisible;
    sal_Bool            bShowDet;

public:
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    ScDPMember( ScDPSource* pSrc, long nD, long nH, long nL,
        SCROW nIndex /*const String& rN, double fV, sal_Bool bHV */);
    // End Comments
    virtual                 ~ScDPMember();

    sal_Bool                    IsNamedItem( const ScDPItemData& r ) const;
    String                  GetNameStr() const;
    void                    FillItemData( ScDPItemData& rData ) const;
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    //  const ScDPItemData&  GetItemData() const{ return maData; }
    const ScDPItemData&  GetItemData() const;
    inline SCROW               GetItemDataId() const { return mnDataId; }
    sal_Bool                           IsNamedItem( SCROW    nIndex  ) const;
    // End Comments

    SC_DLLPUBLIC const ::rtl::OUString*  GetLayoutName() const;

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

    virtual sal_Bool            getIsVisible() const;
    virtual void            setIsVisible(sal_Bool bSet);
    virtual sal_Bool            getShowDetails() const;
    virtual void            setShowDetails(sal_Bool bSet);

    sal_Int32               getPosition() const;
    void                    setPosition(sal_Int32 nNew);
};


#endif

