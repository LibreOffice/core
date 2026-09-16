/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/XSolverSettings.hpp>
#include <com/sun/star/sheet/SolverObjectiveType.hpp>
#include <com/sun/star/sheet/SolverStatus.hpp>
#include <com/sun/star/sheet/SolverConstraint.hpp>

namespace com::sun::star::container
{
class XNamed;
}

namespace sc
{
class SolverSettings;
}

class ScDocument;
class ScDocShell;
class ScTable;
class ScRange;
class ScRangeList;

class ScSolverSettings final
    : public ::cppu::WeakImplHelper<css::sheet::XSolverSettings, css::lang::XServiceInfo>
{
private:
    ScDocShell* m_pDocShell;
    ScDocument& m_rDoc;
    cpo::uno::Reference<css::container::XNamed> m_xSheet;
    // Status uses constants in css::uno::sheet::SolverStatus
    sal_Int8 m_nStatus;
    bool m_bSuppressDialog;
    OUString m_sErrorMessage;
    ScTable* m_pTable;
    std::shared_ptr<sc::SolverSettings> m_pSettings;

    // Parses a reference string (named ranges are also parsed)
    // If bAllowRange is "false" then only single cell ranges are acceptable,
    // which is the case of the objective cell
    bool ParseRef(ScRange& rRange, const OUString& rInput, bool bAllowRange);

    // Parses a reference string composed of various ranges
    bool ParseWithNames(ScRangeList& rRanges, std::u16string_view rInput);

    static void ShowErrorMessage(const OUString& rMessage);

public:
    ScSolverSettings(ScDocShell* pDocSh, cpo::uno::Reference<css::container::XNamed> xSheet);
    ~ScSolverSettings();

    // XSolverSettings attributes
    virtual sal_Int8 getObjectiveType() override;
    virtual void setObjectiveType(sal_Int8 aObjType) override;
    virtual cpo::uno::Any getObjectiveCell() override;
    virtual void setObjectiveCell(const cpo::uno::Any& aValue) override;
    virtual cpo::uno::Any getGoalValue() override;
    virtual void setGoalValue(const cpo::uno::Any& aValue) override;
    virtual OUString getEngine() override;
    virtual void setEngine(const OUString& sEngine) override;
    virtual cpo::uno::Sequence<OUString> getAvailableEngines() override;
    virtual cpo::uno::Sequence<cpo::uno::Any> getVariableCells() override;
    virtual void
    setVariableCells(const cpo::uno::Sequence<cpo::uno::Any>& aRanges) override;
    virtual cpo::uno::Sequence<css::sheet::ModelConstraint> getConstraints() override;
    virtual void
    setConstraints(const cpo::uno::Sequence<css::sheet::ModelConstraint>& aConstraints) override;
    virtual sal_Int32 getConstraintCount() override;
    virtual cpo::uno::Sequence<css::beans::PropertyValue> getEngineOptions() override;
    virtual void
    setEngineOptions(const cpo::uno::Sequence<css::beans::PropertyValue>& rProps) override;
    virtual sal_Int8 getStatus() override;
    virtual OUString getErrorMessage() override;
    virtual bool getSuppressDialog() override;
    virtual void setSuppressDialog(bool bSuppress) override;

    // XSolverSettings methods
    virtual void reset() override;
    virtual void solve() override;
    virtual void saveToFile() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};
