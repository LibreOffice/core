/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
    css::uno::Reference<css::container::XNamed> m_xSheet;
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
    ScSolverSettings(ScDocShell* pDocSh, css::uno::Reference<css::container::XNamed> xSheet);
    ~ScSolverSettings();

    // XSolverSettings attributes
    virtual sal_Int8 SAL_CALL getObjectiveType() override;
    virtual void SAL_CALL setObjectiveType(sal_Int8 aObjType) override;
    virtual css::uno::Any SAL_CALL getObjectiveCell() override;
    virtual void SAL_CALL setObjectiveCell(const css::uno::Any& aValue) override;
    virtual css::uno::Any SAL_CALL getGoalValue() override;
    virtual void SAL_CALL setGoalValue(const css::uno::Any& aValue) override;
    virtual OUString SAL_CALL getEngine() override;
    virtual void SAL_CALL setEngine(const OUString& sEngine) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getAvailableEngines() override;
    virtual css::uno::Sequence<css::uno::Any> SAL_CALL getVariableCells() override;
    virtual void SAL_CALL
    setVariableCells(const css::uno::Sequence<css::uno::Any>& aRanges) override;
    virtual css::uno::Sequence<css::sheet::ModelConstraint> SAL_CALL getConstraints() override;
    virtual void SAL_CALL
    setConstraints(const css::uno::Sequence<css::sheet::ModelConstraint>& aConstraints) override;
    virtual sal_Int32 SAL_CALL getConstraintCount() override;
    virtual css::uno::Sequence<css::beans::PropertyValue> SAL_CALL getEngineOptions() override;
    virtual void SAL_CALL
    setEngineOptions(const css::uno::Sequence<css::beans::PropertyValue>& rProps) override;
    virtual sal_Int8 SAL_CALL getStatus() override;
    virtual OUString SAL_CALL getErrorMessage() override;
    virtual sal_Bool SAL_CALL getSuppressDialog() override;
    virtual void SAL_CALL setSuppressDialog(sal_Bool bSuppress) override;

    // XSolverSettings methods
    virtual void SAL_CALL reset() override;
    virtual void SAL_CALL solve() override;
    virtual void SAL_CALL saveToFile() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};
