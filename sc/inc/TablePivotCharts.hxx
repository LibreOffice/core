/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/table/XTablePivotCharts.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <svl/lstner.hxx>
#include <cppuhelper/implbase.hxx>

#include "types.hxx"

class ScDocShell;

namespace sc
{
typedef cppu::WeakImplHelper<css::table::XTablePivotCharts,
                             css::container::XIndexAccess,
                             css::lang::XServiceInfo>
        TablePivotCharts_Base;

class TablePivotCharts final : public TablePivotCharts_Base, public SfxListener
{
private:
    ScDocShell* m_pDocShell;
    SCTAB m_nTab;

public:
    TablePivotCharts(ScDocShell* pDocShell, SCTAB nTab);

    virtual ~TablePivotCharts() override;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // XTablePivotCharts
    virtual void SAL_CALL addNewByName(OUString const & aName,
                                       const css::awt::Rectangle& aRect,
                                       OUString const & aDataPilotName) override;
    virtual void SAL_CALL removeByName(OUString const & aName) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName(OUString const & aName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(OUString const & aName) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
