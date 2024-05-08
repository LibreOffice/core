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

#include "errorbroadcaster.hxx"
#include <FormComponent.hxx>
#include <formcontrolfont.hxx>
#include <InterfaceContainer.hxx>

#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/sdb/XRowSetSupplier.hpp>
#include <com/sun/star/sdb/XRowSetChangeBroadcaster.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/interfacecontainer3.hxx>
#include <cppuhelper/implbase7.hxx>


namespace frm
{

class OGridColumn;

// OGridControlModel

typedef ::cppu::ImplHelper7 <   css::awt::XControlModel
                            ,   css::form::XGridColumnFactory
                            ,   css::form::XReset
                            ,   css::view::XSelectionSupplier
                            ,   css::sdb::XSQLErrorListener
                            ,   css::sdb::XRowSetSupplier
                            ,   css::sdb::XRowSetChangeBroadcaster
                            >   OGridControlModel_BASE;

class OGridControlModel final :public OControlModel
                        ,public OInterfaceContainer
                        ,public OErrorBroadcaster
                        ,public FontControlModel
                        ,public OGridControlModel_BASE
{
    ::comphelper::OInterfaceContainerHelper3<css::view::XSelectionChangeListener> m_aSelectListeners;
    ::comphelper::OInterfaceContainerHelper3<css::form::XResetListener> m_aResetListeners;
    ::comphelper::OInterfaceContainerHelper3<css::sdb::XRowSetChangeListener> m_aRowSetChangeListeners;

// [properties]
    css::uno::Any                    m_aRowHeight; // Row height
    css::uno::Any                    m_aTabStop;
    css::uno::Any                    m_aBackgroundColor;
    css::uno::Any                    m_aCursorColor; // transient
    css::uno::Any                    m_aBorderColor;
    OUString                         m_aDefaultControl;
    OUString                         m_sHelpText;
// [properties]

    css::uno::Reference< css::beans::XPropertySet >       m_xSelection;

// [properties]
    OUString                m_sHelpURL;                 // URL
    sal_Int16               m_nBorder;
    sal_Int16               m_nWritingMode;
    sal_Int16               m_nContextWritingMode;
    bool                    m_bEnableVisible    : 1;
    bool                    m_bEnable           : 1;
    bool                    m_bNavigation       : 1;
    bool                    m_bRecordMarker     : 1;
    bool                    m_bPrintable        : 1;
    bool                    m_bAlwaysShowCursor : 1;    // transient
    bool                    m_bDisplaySynchron  : 1;    // transient
// [properties]

    void _reset();

public:
    OGridControlModel(
        const css::uno::Reference< css::uno::XComponentContext>& _rxFactory
    );
    OGridControlModel(
        const OGridControlModel* _pOriginal,
        const css::uno::Reference< css::uno::XComponentContext>& _rxFactory
    );
    virtual ~OGridControlModel() override;

    // UNO Binding
    DECLARE_UNO3_AGG_DEFAULTS(OGridControlModel, OControlModel)
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) override;

    // XChild
    virtual void SAL_CALL setParent(const css::uno::Reference<css::uno::XInterface>& Parent) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override
    { return u"com.sun.star.form.OGridControlModel"_ustr; }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& _rSource) override;

    // XReset
    virtual void SAL_CALL reset() override;
    virtual void SAL_CALL addResetListener(const css::uno::Reference< css::form::XResetListener>& _rxListener) override;
    virtual void SAL_CALL removeResetListener(const css::uno::Reference< css::form::XResetListener>& _rxListener) override;

    // XSelectionSupplier
    virtual sal_Bool SAL_CALL select(const css::uno::Any& aElement) override;
    virtual css::uno::Any SAL_CALL getSelection() override;
    virtual void SAL_CALL addSelectionChangeListener(const css::uno::Reference< css::view::XSelectionChangeListener >& xListener) override;
    virtual void SAL_CALL removeSelectionChangeListener(const css::uno::Reference< css::view::XSelectionChangeListener >& xListener) override;

    // XGridColumnFactory
    virtual css::uno::Reference< css::beans::XPropertySet> SAL_CALL createColumn(const OUString& ColumnType) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getColumnTypes() override;

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() override;
    virtual void SAL_CALL write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) override;
    virtual void SAL_CALL read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) override;

    // XPropertySet
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle ) const override;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(css::uno::Any& rConvertedValue, css::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const css::uno::Any& rValue ) override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const css::uno::Any& rValue) override;

    // XPropertyState
    virtual css::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const override;

    // XSQLErrorListener
    virtual void SAL_CALL errorOccured( const css::sdb::SQLErrorEvent& _rEvent ) override;

    // XRowSetSupplier
    virtual css::uno::Reference< css::sdbc::XRowSet > SAL_CALL getRowSet(  ) override;
    virtual void SAL_CALL setRowSet( const css::uno::Reference< css::sdbc::XRowSet >& xDataSource ) override;

    // XRowSetChangeBroadcaster
    virtual void SAL_CALL addRowSetChangeListener( const css::uno::Reference< css::sdb::XRowSetChangeListener >& i_Listener ) override;
    virtual void SAL_CALL removeRowSetChangeListener( const css::uno::Reference< css::sdb::XRowSetChangeListener >& i_Listener ) override;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const override;

    // prevent method hiding
    using OControlModel::disposing;
    using OControlModel::getFastPropertyValue;

private:
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;

    virtual void approveNewElement(
            const css::uno::Reference< css::beans::XPropertySet >& _rxObject,
            ElementDescription* _pElement
        ) override;

    css::uno::Reference< css::beans::XPropertySet>  createColumnById(sal_Int32 nTypeId) const;

    virtual ElementDescription* createElementMetaData( ) override;

    virtual void implRemoved(const css::uno::Reference<css::uno::XInterface>& _rxObject) override;
    virtual void implInserted( const ElementDescription* _pElement ) override;
    virtual void impl_replacedElement(
                    const css::container::ContainerEvent& _rEvent,
                    ::osl::ClearableMutexGuard& _rInstanceLock
                ) override;

    void gotColumn(const css::uno::Reference< css::uno::XInterface >& _rxColumn);
    void lostColumn(const css::uno::Reference< css::uno::XInterface >& _rxColumn);

    void cloneColumns( const OGridControlModel* _pOriginalContainer );
};


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
