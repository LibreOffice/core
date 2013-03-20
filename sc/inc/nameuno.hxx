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
#if 1

#include <svl/lstner.hxx>
#include "address.hxx"
#include "formula/grammar.hxx"
#include <com/sun/star/sheet/XLabelRange.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase6.hxx>
#include <rtl/ref.hxx>

class ScDocShell;
class ScRangeName;
class ScRangeData;
class ScTokenArray;
class ScNamedRangesObj;

class ScNamedRangeObj : public ::cppu::WeakImplHelper6<
                            ::com::sun::star::sheet::XNamedRange,
                            ::com::sun::star::sheet::XFormulaTokens,
                            ::com::sun::star::sheet::XCellRangeReferrer,
                            ::com::sun::star::beans::XPropertySet,
                            ::com::sun::star::lang::XUnoTunnel,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    rtl::Reference< ScNamedRangesObj > mxParent;
    ScDocShell*             pDocShell;
    String                  aName;
    com::sun::star::uno::Reference< com::sun::star::container::XNamed > mxSheet;

private:
    ScRangeData*            GetRangeData_Impl();
    void                    Modify_Impl( const String* pNewName,
                                        const ScTokenArray* pNewTokens, const String* pNewContent,
                                        const ScAddress* pNewPos, const sal_uInt16* pNewType,
                                        const formula::FormulaGrammar::Grammar eGrammar );
    SCTAB                   GetTab_Impl();

public:
                            ScNamedRangeObj( rtl::Reference< ScNamedRangesObj > xParent, ScDocShell* pDocSh, const String& rNm,
                                    com::sun::star::uno::Reference< com::sun::star::container::XNamed > xSheet = com::sun::star::uno::Reference< com::sun::star::container::XNamed > ());
    virtual                 ~ScNamedRangeObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XNamedRange
    virtual ::rtl::OUString SAL_CALL getContent() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setContent( const ::rtl::OUString& aContent )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellAddress SAL_CALL getReferencePosition()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setReferencePosition(
                                const ::com::sun::star::table::CellAddress& aReferencePosition )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32       SAL_CALL getType() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setType( sal_Int32 nType ) throw(::com::sun::star::uno::RuntimeException);

                            // XFormulaTokens
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > SAL_CALL getTokens()
                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setTokens( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::sheet::FormulaToken >& aTokens )
                                throw (::com::sun::star::uno::RuntimeException);

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XCellRangeReferrer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getReferredCells() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
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

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScNamedRangesObj : public ::cppu::WeakImplHelper6<
                            ::com::sun::star::sheet::XNamedRanges,
                            ::com::sun::star::container::XEnumerationAccess,
                            ::com::sun::star::container::XIndexAccess,
                            ::com::sun::star::beans::XPropertySet,
                            ::com::sun::star::document::XActionLockable,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:

     // if true, adding new name or modifying existing one will set the
     // document 'modified' and broadcast the change.  We turn this off during
     // import.
    sal_Bool                mbModifyAndBroadcast;

    virtual ScNamedRangeObj* GetObjectByIndex_Impl(sal_uInt16 nIndex) = 0;
    virtual ScNamedRangeObj* GetObjectByName_Impl(const ::rtl::OUString& aName) = 0;

    virtual ScRangeName*    GetRangeName_Impl() = 0;
    virtual SCTAB           GetTab_Impl() = 0;

protected:

    ScDocShell*             pDocShell;
    /** called from the XActionLockable interface methods on initial locking */
    virtual void            lock();

    /** called from the XActionLockable interface methods on final unlock */
    virtual void            unlock();

public:
                            ScNamedRangesObj(ScDocShell* pDocSh);
    virtual                 ~ScNamedRangesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    bool                    IsModifyAndBroadcast() const;

                            // XNamedRanges
    virtual void SAL_CALL   addNewByName( const ::rtl::OUString& aName, const ::rtl::OUString& aContent,
                                const ::com::sun::star::table::CellAddress& aPosition, sal_Int32 nType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addNewFromTitles( const ::com::sun::star::table::CellRangeAddress& aSource,
                                ::com::sun::star::sheet::Border aBorder )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   outputList( const ::com::sun::star::table::CellAddress& aOutputPosition )
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

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
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

                            // XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addActionLock() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeActionLock() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActionLocks( sal_Int16 nLock ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL resetActionLocks() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};

class ScGlobalNamedRangesObj: public ScNamedRangesObj
{
private:

    virtual ScNamedRangeObj* GetObjectByIndex_Impl(sal_uInt16 nIndex);
    virtual ScNamedRangeObj* GetObjectByName_Impl(const ::rtl::OUString& aName);

    virtual ScRangeName*    GetRangeName_Impl();
    virtual SCTAB           GetTab_Impl();

public:
                            ScGlobalNamedRangesObj(ScDocShell* pDocSh);
    virtual                 ~ScGlobalNamedRangesObj();
};

class ScLocalNamedRangesObj: public ScNamedRangesObj
{
private:

    virtual ScNamedRangeObj* GetObjectByIndex_Impl(sal_uInt16 nIndex);
    virtual ScNamedRangeObj* GetObjectByName_Impl(const ::rtl::OUString& aName);

    virtual ScRangeName*    GetRangeName_Impl();
    virtual SCTAB           GetTab_Impl();

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > mxSheet;
public:
                            ScLocalNamedRangesObj(ScDocShell* pDocSh, ::com::sun::star::uno::Reference< com::sun::star::container::XNamed > xNamed );
    virtual                 ~ScLocalNamedRangesObj();
};


class ScLabelRangeObj : public ::cppu::WeakImplHelper2<
                            ::com::sun::star::sheet::XLabelRange,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    sal_Bool                bColumn;
    ScRange                 aRange;         // criterion to find range

private:
    ScRangePair*            GetData_Impl();
    void                    Modify_Impl( const ScRange* pLabel, const ScRange* pData );

public:
                            ScLabelRangeObj(ScDocShell* pDocSh, sal_Bool bCol, const ScRange& rR);
    virtual                 ~ScLabelRangeObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XLabelRange
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getLabelArea()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setLabelArea( const ::com::sun::star::table::CellRangeAddress& aLabelArea )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getDataArea()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setDataArea( const ::com::sun::star::table::CellRangeAddress& aDataArea )
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
    ScDocShell*             pDocShell;
    sal_Bool                    bColumn;

    ScLabelRangeObj*        GetObjectByIndex_Impl(size_t nIndex);

public:
                            ScLabelRangesObj(ScDocShell* pDocSh, sal_Bool bCol);
    virtual                 ~ScLabelRangesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XLabelRanges
    virtual void SAL_CALL   addNew( const ::com::sun::star::table::CellRangeAddress& aLabelArea,
                                const ::com::sun::star::table::CellRangeAddress& aDataArea )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex )
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




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
