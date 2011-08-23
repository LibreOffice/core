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

#ifndef SC_NAMEUNO_HXX
#define SC_NAMEUNO_HXX

#include <bf_svtools/lstner.hxx>

#include "global.hxx"	// ScRange

#include <com/sun/star/sheet/XLabelRange.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>
namespace binfilter {

class ScDocShell;
class ScRangeData;


class ScNamedRangeObj : public ::cppu::WeakImplHelper4<
                            ::com::sun::star::sheet::XNamedRange,
                            ::com::sun::star::sheet::XCellRangeReferrer,
                            ::com::sun::star::beans::XPropertySet,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*				pDocShell;
    String					aName;

private:
    ScRangeData*			GetRangeData_Impl();
    void					Modify_Impl( const String* pNewName, const String* pNewContent,
                                        const ScAddress* pNewPos, const sal_uInt16* pNewType );

public:
                            ScNamedRangeObj(ScDocShell* pDocSh, const String& rNm);
    virtual					~ScNamedRangeObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XNamedRange
    virtual ::rtl::OUString SAL_CALL getContent() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setContent( const ::rtl::OUString& aContent )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellAddress SAL_CALL getReferencePosition()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setReferencePosition(
                                const ::com::sun::star::table::CellAddress& aReferencePosition )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32		SAL_CALL getType() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setType( sal_Int32 nType ) throw(::com::sun::star::uno::RuntimeException);

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XCellRangeReferrer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getReferredCells() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
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
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScNamedRangesObj : public ::cppu::WeakImplHelper4<
                            ::com::sun::star::sheet::XNamedRanges,
                            ::com::sun::star::container::XEnumerationAccess,
                            ::com::sun::star::container::XIndexAccess,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*				pDocShell;

    ScNamedRangeObj*		GetObjectByIndex_Impl(sal_uInt16 nIndex);
    ScNamedRangeObj*		GetObjectByName_Impl(const ::rtl::OUString& aName);

public:
                            ScNamedRangesObj(ScDocShell* pDocSh);
    virtual					~ScNamedRangesObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XNamedRanges
    virtual void SAL_CALL	addNewByName( const ::rtl::OUString& aName, const ::rtl::OUString& aContent,
                                const ::com::sun::star::table::CellAddress& aPosition, sal_Int32 nType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addNewFromTitles( const ::com::sun::star::table::CellRangeAddress& aSource,
                                ::com::sun::star::sheet::Border aBorder )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	outputList( const ::com::sun::star::table::CellAddress& aOutputPosition )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScLabelRangeObj : public ::cppu::WeakImplHelper2<
                            ::com::sun::star::sheet::XLabelRange,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*				pDocShell;
    sal_Bool				bColumn;
    ScRange					aRange;			// Kriterium um Bereich zu finden

private:
    ScRangePair*			GetData_Impl();
    void					Modify_Impl( const ScRange* pLabel, const ScRange* pData );

public:
                            ScLabelRangeObj(ScDocShell* pDocSh, sal_Bool bCol, const ScRange& rR);
    virtual					~ScLabelRangeObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XLabelRange
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getLabelArea()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setLabelArea( const ::com::sun::star::table::CellRangeAddress& aLabelArea )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getDataArea()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setDataArea( const ::com::sun::star::table::CellRangeAddress& aDataArea )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScLabelRangesObj : public ::cppu::WeakImplHelper3<
                            ::com::sun::star::sheet::XLabelRanges,
                            ::com::sun::star::container::XEnumerationAccess,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*				pDocShell;
    sal_Bool					bColumn;

    ScLabelRangeObj*		GetObjectByIndex_Impl(sal_uInt16 nIndex);

public:
                            ScLabelRangesObj(ScDocShell* pDocSh, sal_Bool bCol);
    virtual					~ScLabelRangesObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XLabelRanges
    virtual void SAL_CALL	addNew( const ::com::sun::star::table::CellRangeAddress& aLabelArea,
                                const ::com::sun::star::table::CellRangeAddress& aDataArea )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeByIndex( sal_Int32 nIndex )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};




} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
