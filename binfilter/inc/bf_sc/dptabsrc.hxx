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



#ifndef SC_SCGLOB_HXX
#include "global.hxx"		// enum ScSubTotalFunc
#endif

#ifndef _COM_SUN_STAR_SHEET_XDIMENSIONSSUPPLIER_HPP_
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XHIERARCHIESSUPPLIER_HPP_
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XLEVELSSUPPLIER_HPP_
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XMEMBERSSUPPLIER_HPP_
#include <com/sun/star/sheet/XMembersSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATAPILOTRESULTS_HPP_
#include <com/sun/star/sheet/XDataPilotResults.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATAPILOTMEMBERRESULTS_HPP_
#include <com/sun/star/sheet/XDataPilotMemberResults.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_GENERALFUNCTION_HPP_
#include <com/sun/star/sheet/GeneralFunction.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
namespace binfilter {


class ScDPResultMember;
class ScDPResultData;
struct ScDPItemData;
class ScDPTableData;

// --------------------------------------------------------------------

//	should be dynamic!
#define SC_DAPI_MAXFIELDS 	256


// --------------------------------------------------------------------
//
//	implementation of DataPilotSource using ScDPTableData
//


class ScDPDimensions;
class ScDPDimension;
class ScDPHierarchies;
class ScDPHierarchy;
class ScDPLevels;
class ScDPLevel;
class ScDPMembers;
class ScDPMember;


class ScDPSource : public cppu::WeakImplHelper5<
                            ::com::sun::star::sheet::XDimensionsSupplier,
                            ::com::sun::star::sheet::XDataPilotResults,
                            ::com::sun::star::util::XRefreshable,
                            ::com::sun::star::beans::XPropertySet,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    ScDPTableData*			pData;				// data source
    ScDPDimensions*			pDimensions;		// api objects
                                                // settings:
    long					nColDims[SC_DAPI_MAXFIELDS];
    long					nRowDims[SC_DAPI_MAXFIELDS];
    long					nDataDims[SC_DAPI_MAXFIELDS];
    long					nPageDims[SC_DAPI_MAXFIELDS];
    long					nColDimCount;
    long					nRowDimCount;
    long					nDataDimCount;
    long					nPageDimCount;
    BOOL					bColumnGrand;
    BOOL					bRowGrand;
    BOOL					bIgnoreEmptyRows;
    BOOL					bRepeatIfEmpty;

    long					nDupCount;

                                                // results:
    ScDPResultData*			pResData;			// keep the rest in this!
    ScDPResultMember*		pColResRoot;
    ScDPResultMember*		pRowResRoot;
    ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::MemberResult>* pColResults;
    ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::MemberResult>* pRowResults;
    List					aColLevelList;
    List					aRowLevelList;
    ScSubTotalFunc			eDataFunctions[SC_DAPI_MAXFIELDS];
    BOOL					bResultOverflow;



public:
                                ScDPSource( ScDPTableData* pD );	// TableData is deleted by Source
    virtual 					~ScDPSource();

    ScDPTableData*			GetData()		{ return pData; }
    const ScDPTableData*	GetData() const	{ return pData; }

    USHORT					GetOrientation(long nColumn);
    void					SetOrientation(long nColumn, USHORT nNew);
    long					GetPosition(long nColumn);

    BOOL					IsDataLayoutDimension(long nDim);

    BOOL					IsDateDimension(long nDim);

    BOOL					SubTotalAllowed(long nColumn);		//! move to ScDPResultData

    long					GetDupCount() const	{ return nDupCount; }

    long					GetSourceDim(long nDim);


    ScDPDimensions*			GetDimensionsObject();

                            // XDimensionsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                            SAL_CALL getDimensions(  )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XDataPilotResults
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence<
                            ::com::sun::star::sheet::DataResult > > SAL_CALL getResults(  )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XRefreshable
    virtual void SAL_CALL	refresh() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addRefreshListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XRefreshListener >& l )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeRefreshListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XRefreshListener >& l )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPropertyValue( const ::rtl::OUString& aPropertyName,
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
    virtual void SAL_CALL	addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
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


    virtual BOOL				getColumnGrand() const;
    virtual void				setColumnGrand(BOOL bSet);
    virtual BOOL				getRowGrand() const;
    virtual void				setRowGrand(BOOL bSet);

    virtual BOOL				getIgnoreEmptyRows() const;
    virtual void				setIgnoreEmptyRows(BOOL bSet);
    virtual BOOL				getRepeatIfEmpty() const;
    virtual void				setRepeatIfEmpty(BOOL bSet);

};

class ScDPDimensions : public cppu::WeakImplHelper2<
                            ::com::sun::star::container::XNameAccess,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    ScDPSource*			pSource;
    long				nDimCount;
    ScDPDimension**		ppDims;

public:
                            ScDPDimensions( ScDPSource* pSrc );
    virtual 				~ScDPDimensions();


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

    virtual	long			getCount() const;
    virtual ScDPDimension*	getByIndex(long nIndex) const;
};

class ScDPDimension : public cppu::WeakImplHelper5<
                            ::com::sun::star::sheet::XHierarchiesSupplier,
                            ::com::sun::star::container::XNamed,
                            ::com::sun::star::util::XCloneable,
                            ::com::sun::star::beans::XPropertySet,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    ScDPSource*			pSource;
    long				nDim;
    ScDPHierarchies*	pHierarchies;
    long				nUsedHier;
    USHORT				nFunction;			// enum GeneralFunction
    String				aName;				// if empty, take from source
    long				nSourceDim;			// >=0 if dup'ed

public:
                            ScDPDimension( ScDPSource* pSrc, long nD );
    virtual 				~ScDPDimension();

    long					GetSourceDim() const	{ return nSourceDim; }

    ScDPHierarchies*		GetHierarchiesObject();

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setName( const ::rtl::OUString& aName )
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
    virtual void SAL_CALL	setPropertyValue( const ::rtl::OUString& aPropertyName,
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
    virtual void SAL_CALL	addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
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

    virtual USHORT				getOrientation() const;
    virtual void				setOrientation(USHORT nNew);
    virtual long				getPosition() const;
    virtual	BOOL				getIsDataLayoutDimension() const;
    virtual USHORT				getFunction() const;
    virtual void				setFunction(USHORT nNew);		// for data dimension
    virtual long				getUsedHierarchy() const;
};

class ScDPHierarchies : public cppu::WeakImplHelper2<
                            ::com::sun::star::container::XNameAccess,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    ScDPSource*			pSource;
    long				nDim;
    long				nHierCount;
    ScDPHierarchy**		ppHiers;

public:
                            ScDPHierarchies( ScDPSource* pSrc, long nD );
    virtual 				~ScDPHierarchies();

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

    virtual	long			getCount() const;
    virtual ScDPHierarchy*	getByIndex(long nIndex) const;
};

class ScDPHierarchy : public cppu::WeakImplHelper3<
                            ::com::sun::star::sheet::XLevelsSupplier,
                            ::com::sun::star::container::XNamed,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    ScDPSource*		pSource;
    long			nDim;
    long			nHier;
    ScDPLevels*		pLevels;

public:
                            ScDPHierarchy( ScDPSource* pSrc, long nD, long nH );
    virtual 				~ScDPHierarchy();

    ScDPLevels*				GetLevelsObject();

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setName( const ::rtl::OUString& aName )
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
                            ::com::sun::star::container::XNameAccess,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    ScDPSource*		pSource;
    long			nDim;
    long			nHier;
    long			nLevCount;
    ScDPLevel**		ppLevs;

public:
                            ScDPLevels( ScDPSource* pSrc, long nD, long nH );
    virtual 				~ScDPLevels();

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

    virtual	long			getCount() const;
    virtual ScDPLevel*		getByIndex(long nIndex) const;
};

class ScDPLevel : public cppu::WeakImplHelper5<
                            ::com::sun::star::sheet::XMembersSupplier,
                            ::com::sun::star::container::XNamed,
                            ::com::sun::star::sheet::XDataPilotMemberResults,
                            ::com::sun::star::beans::XPropertySet,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    ScDPSource*					pSource;
    long						nDim;
    long						nHier;
    long						nLev;
    ScDPMembers*				pMembers;
    ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::GeneralFunction> aSubTotals;
    BOOL						bShowEmpty;

public:
                            ScDPLevel( ScDPSource* pSrc, long nD, long nH, long nL );
    virtual 				~ScDPLevel();


                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setName( const ::rtl::OUString& aName )
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
    virtual void SAL_CALL	setPropertyValue( const ::rtl::OUString& aPropertyName,
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
    virtual void SAL_CALL	addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
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

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::GeneralFunction> getSubTotals() const;
    virtual void			setShowEmpty(BOOL bSet);

    //!	number format (for data fields and date fields)
};




} //namespace binfilter
#endif

