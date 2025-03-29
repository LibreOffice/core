/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "OPropertySet.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/chart2/XDataTable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include "ModifyListenerHelper.hxx"

namespace chart
{
typedef cppu::WeakImplHelper<css::chart2::XDataTable, css::lang::XServiceInfo,
                             css::util::XCloneable, css::util::XModifyBroadcaster,
                             css::util::XModifyListener>
    DataTable_Base;

/** Data table implementation */
class DataTable final : public DataTable_Base, public ::property::OPropertySet
{
public:
    explicit DataTable();
    virtual ~DataTable() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    /// merge XInterface implementations
    DECLARE_XINTERFACE()

    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

    explicit DataTable(DataTable const& rOther);

private:
    // ____ OPropertySet ____
    virtual void GetDefaultValue(sal_Int32 nHandle, css::uno::Any& rAny) const override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

public:
    // ____ XPropertySet ____
    virtual css::uno::Reference<css::beans::XPropertySetInfo>
        SAL_CALL getPropertySetInfo() override;

    // ____ XCloneable ____
    virtual css::uno::Reference<css::util::XCloneable> SAL_CALL createClone() override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL
    addModifyListener(const css::uno::Reference<css::util::XModifyListener>& aListener) override;
    virtual void SAL_CALL
    removeModifyListener(const css::uno::Reference<css::util::XModifyListener>& aListener) override;

private:
    // ____ XModifyListener ____
    virtual void SAL_CALL modified(const css::lang::EventObject& aEvent) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;
    using OPropertySet::disposing;

    rtl::Reference<ModifyEventForwarder> m_xModifyEventForwarder;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
