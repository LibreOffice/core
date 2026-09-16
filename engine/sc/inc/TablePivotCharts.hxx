/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    virtual void addNewByName(OUString const & aName,
                                       const css::awt::Rectangle& aRect,
                                       OUString const & aDataPilotName) override;
    virtual void removeByName(OUString const & aName) override;

    // XNameAccess
    virtual cpo::uno::Any getByName(OUString const & aName) override;
    virtual cpo::uno::Sequence<OUString> getElementNames() override;
    virtual bool hasByName(OUString const & aName) override;

    // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex(sal_Int32 nIndex) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(OUString const & ServiceName) override;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
