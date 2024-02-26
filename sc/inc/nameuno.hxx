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
    css::uno::Reference< css::container::XNamed > mxSheet;

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
                                    css::uno::Reference< css::container::XNamed > const & xSheet = css::uno::Reference< css::container::XNamed > ());
    virtual                 ~ScNamedRangeObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XNamedRange
    virtual OUString SAL_CALL getContent() override;
    virtual void SAL_CALL   setContent( const OUString& aContent ) override;
    virtual css::table::CellAddress SAL_CALL getReferencePosition() override;
    virtual void SAL_CALL   setReferencePosition(
                                const css::table::CellAddress& aReferencePosition ) override;
    virtual sal_Int32       SAL_CALL getType() override;
    virtual void SAL_CALL   setType( sal_Int32 nType ) override;

                            /// XFormulaTokens
    virtual css::uno::Sequence< css::sheet::FormulaToken > SAL_CALL getTokens() override;
    virtual void SAL_CALL   setTokens( const css::uno::Sequence< css::sheet::FormulaToken >& aTokens ) override;

                            /// XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL   setName( const OUString& aName ) override;

                            /// XCellRangeReferrer
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getReferredCells() override;

                            /// XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    virtual void SAL_CALL   addNewByName( const OUString& aName, const OUString& aContent,
                                const css::table::CellAddress& aPosition, sal_Int32 nType ) override;
    virtual void SAL_CALL   addNewFromTitles( const css::table::CellRangeAddress& aSource,
                                css::sheet::Border aBorder ) override;
    virtual void SAL_CALL   removeByName( const OUString& aName ) override;
    virtual void SAL_CALL   outputList( const css::table::CellAddress& aOutputPosition ) override;

                            /// XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            /// XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            /// XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
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

                            /// XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked() override;
    virtual void SAL_CALL addActionLock() override;
    virtual void SAL_CALL removeActionLock() override;
    virtual void SAL_CALL setActionLocks( sal_Int16 nLock ) override;
    virtual sal_Int16 SAL_CALL resetActionLocks() override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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

    css::uno::Reference< css::container::XNamed > mxSheet;
public:
                            ScLocalNamedRangesObj(ScDocShell* pDocSh, css::uno::Reference< css::container::XNamed > xNamed );
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
    virtual css::table::CellRangeAddress SAL_CALL getLabelArea() override;
    virtual void SAL_CALL   setLabelArea( const css::table::CellRangeAddress& aLabelArea ) override;
    virtual css::table::CellRangeAddress SAL_CALL getDataArea() override;
    virtual void SAL_CALL   setDataArea( const css::table::CellRangeAddress& aDataArea ) override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    virtual void SAL_CALL   addNew( const css::table::CellRangeAddress& aLabelArea,
                                const css::table::CellRangeAddress& aDataArea ) override;
    virtual void SAL_CALL   removeByIndex( sal_Int32 nIndex ) override;

                            /// XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            /// XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            /// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
