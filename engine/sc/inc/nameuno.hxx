/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <svl/lstner.hxx>
#include "address.hxx"
#include "rangenam.hxx"
#include <formula/grammar.hxx>
#include <com/sun/star/sheet/XLabelRange.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

namespace com::sun::star::container { class XNamed; }

class ScDocShell;
class ScNamedRangesObj;

class SAL_DLLPUBLIC_RTTI ScNamedRangeObj final : public ::cppu::WeakImplHelper<
                            css::sheet::XNamedRange,
                            css::sheet::XFormulaTokens,
                            css::sheet::XCellRangeReferrer,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    rtl::Reference< ScNamedRangesObj > mxParent;
    ScDocShell*             pDocShell;
    OUString                aName;
    cpo::uno::Reference< css::container::XNamed > mxSheet;

private:
friend class ScVbaName;
    SC_DLLPUBLIC ScRangeData* GetRangeData_Impl();
    void                    Modify_Impl( const OUString* pNewName,
                                        const ScTokenArray* pNewTokens, const OUString* pNewContent,
                                        const ScAddress* pNewPos, const ScRangeData::Type* pNewType,
                                        const formula::FormulaGrammar::Grammar eGrammar );
    SCTAB                   GetTab_Impl();

public:
                            ScNamedRangeObj( rtl::Reference< ScNamedRangesObj > xParent, ScDocShell* pDocSh, OUString aNm,
                                    cpo::uno::Reference< css::container::XNamed > const & xSheet = cpo::uno::Reference< css::container::XNamed > ());
    virtual                 ~ScNamedRangeObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XNamedRange
    virtual OUString getContent() override;
    virtual void   setContent( const OUString& aContent ) override;
    virtual css::table::CellAddress getReferencePosition() override;
    virtual void   setReferencePosition(
                                const css::table::CellAddress& aReferencePosition ) override;
    virtual sal_Int32       getType() override;
    virtual void   setType( sal_Int32 nType ) override;

                            /// XFormulaTokens
    virtual cpo::uno::Sequence< css::sheet::FormulaToken > getTokens() override;
    virtual void   setTokens( const cpo::uno::Sequence< css::sheet::FormulaToken >& aTokens ) override;

                            /// XNamed
    virtual OUString getName() override;
    virtual void   setName( const OUString& aName ) override;

                            /// XCellRangeReferrer
    virtual cpo::uno::Reference< css::table::XCellRange >
                            getReferredCells() override;

                            /// XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class ScNamedRangesObj : public ::cppu::WeakImplHelper<
                            css::sheet::XNamedRanges,
                            css::container::XEnumerationAccess,
                            css::container::XIndexAccess,
                            css::beans::XPropertySet,
                            css::document::XActionLockable,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:

     /** if true, adding new name or modifying existing one will set the
         document 'modified' and broadcast the change.  We turn this off during
         import. */
    bool                    mbModifyAndBroadcast;

    virtual rtl::Reference<ScNamedRangeObj> GetObjectByIndex_Impl(sal_uInt16 nIndex) = 0;
    virtual rtl::Reference<ScNamedRangeObj> GetObjectByName_Impl(const OUString& aName) = 0;

    virtual ScRangeName*    GetRangeName_Impl() = 0;
    virtual SCTAB           GetTab_Impl() = 0;

protected:

    ScDocShell*             pDocShell;
    /** called from the XActionLockable interface methods on initial locking */
    void            lock();

    /** called from the XActionLockable interface methods on final unlock */
    void            unlock();

public:
                            ScNamedRangesObj(ScDocShell* pDocSh);
    virtual                 ~ScNamedRangesObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    bool                    IsModifyAndBroadcast() const { return mbModifyAndBroadcast;}

                            /// XNamedRanges
    virtual void   addNewByName( const OUString& aName, const OUString& aContent,
                                const css::table::CellAddress& aPosition, sal_Int32 nType ) override;
    virtual void   addNewFromTitles( const css::table::CellRangeAddress& aSource,
                                css::sheet::Border aBorder ) override;
    virtual void   removeByName( const OUString& aName ) override;
    virtual void   outputList( const css::table::CellAddress& aOutputPosition ) override;

                            /// XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

                            /// XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            /// XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            /// XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            /// XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            /// XActionLockable
    virtual bool isActionLocked() override;
    virtual void addActionLock() override;
    virtual void removeActionLock() override;
    virtual void setActionLocks( sal_Int16 nLock ) override;
    virtual sal_Int16 resetActionLocks() override;

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class ScGlobalNamedRangesObj final : public ScNamedRangesObj
{
private:

    virtual rtl::Reference<ScNamedRangeObj> GetObjectByIndex_Impl(sal_uInt16 nIndex) override;
    virtual rtl::Reference<ScNamedRangeObj> GetObjectByName_Impl(const OUString& aName) override;

    virtual ScRangeName*    GetRangeName_Impl() override;
    virtual SCTAB           GetTab_Impl() override;

public:
                            ScGlobalNamedRangesObj(ScDocShell* pDocSh);
    virtual                 ~ScGlobalNamedRangesObj() override;
};

class ScLocalNamedRangesObj final : public ScNamedRangesObj
{
private:

    virtual rtl::Reference<ScNamedRangeObj> GetObjectByIndex_Impl(sal_uInt16 nIndex) override;
    virtual rtl::Reference<ScNamedRangeObj> GetObjectByName_Impl(const OUString& aName) override;

    virtual ScRangeName*    GetRangeName_Impl() override;
    virtual SCTAB           GetTab_Impl() override;

    cpo::uno::Reference< css::container::XNamed > mxSheet;
public:
                            ScLocalNamedRangesObj(ScDocShell* pDocSh, cpo::uno::Reference< css::container::XNamed > xNamed );
    virtual                 ~ScLocalNamedRangesObj() override;
};

class ScLabelRangeObj final : public ::cppu::WeakImplHelper<
                            css::sheet::XLabelRange,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    bool                    bColumn;
    ScRange                 aRange;         ///< criterion to find range

private:
    ScRangePair*            GetData_Impl();
    void                    Modify_Impl( const ScRange* pLabel, const ScRange* pData );

public:
                            ScLabelRangeObj(ScDocShell* pDocSh, bool bCol, const ScRange& rR);
    virtual                 ~ScLabelRangeObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XLabelRange
    virtual css::table::CellRangeAddress getLabelArea() override;
    virtual void   setLabelArea( const css::table::CellRangeAddress& aLabelArea ) override;
    virtual css::table::CellRangeAddress getDataArea() override;
    virtual void   setDataArea( const css::table::CellRangeAddress& aDataArea ) override;

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

class ScLabelRangesObj final : public ::cppu::WeakImplHelper<
                            css::sheet::XLabelRanges,
                            css::container::XEnumerationAccess,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    bool                    bColumn;

    rtl::Reference<ScLabelRangeObj> GetObjectByIndex_Impl(size_t nIndex);

public:
                            ScLabelRangesObj(ScDocShell* pDocSh, bool bCol);
    virtual                 ~ScLabelRangesObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XLabelRanges
    virtual void   addNew( const css::table::CellRangeAddress& aLabelArea,
                                const css::table::CellRangeAddress& aDataArea ) override;
    virtual void   removeByIndex( sal_Int32 nIndex ) override;

                            /// XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            /// XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            /// XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
