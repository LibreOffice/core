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

#include <com/sun/star/sheet/XSolver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>
#include <com/sun/star/sheet/SensitivityReport.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/propcontainerimplhelper.hxx>
#include <unotools/resmgr.hxx>

#include <unordered_map>

namespace com::sun::star::table { class XCell; }

class ScDocument;

// hash map for the coefficients of a dependent cell (objective or constraint)
// The size of each vector is the number of columns (variable cells) plus one, first entry is initial value.

struct ScSolverCellHash
{
    size_t operator()( const css::table::CellAddress& rAddress ) const;
};

inline bool AddressEqual( const css::table::CellAddress& rAddr1, const css::table::CellAddress& rAddr2 )
{
    return rAddr1.Sheet == rAddr2.Sheet && rAddr1.Column == rAddr2.Column && rAddr1.Row == rAddr2.Row;
}

struct ScSolverCellEqual
{
    bool operator()( const css::table::CellAddress& rAddr1, const css::table::CellAddress& rAddr2 ) const;
};

typedef std::unordered_map< css::table::CellAddress, std::vector<double>, ScSolverCellHash, ScSolverCellEqual > ScSolverCellHashMap;

class SolverComponent
    : public comphelper::OPropertyContainerImplHelper<
          comphelper::WeakImplHelper<
              css::sheet::XSolver,
              css::sheet::XSolverDescription,
              css::lang::XServiceInfo>,
          SolverComponent>
{
protected:
    // settings
    cpo::uno::Reference< css::sheet::XSpreadsheetDocument > mxDoc;
    // The ScDocument behind mxDoc, recovered through the UNO tunnel. Null until
    // setDocument runs.
    ScDocument* mpDocument = nullptr;
    css::table::CellAddress                                 maObjective;
    cpo::uno::Sequence< css::table::CellAddress >           maVariables;
    cpo::uno::Sequence< css::sheet::SolverConstraint >      maConstraints;
    bool                                                    mbMaximize;
    // set via XPropertySet
    bool                                                    mbNonNegative;
    bool                                                    mbInteger;
    sal_Int32                                               mnTimeout;
    sal_Int32                                               mnEpsilonLevel;
    bool                                                    mbLimitBBDepth;
    bool                                                    mbGenSensitivity;
    // results
    bool                                                    mbSuccess;
    double                                                  mfResultValue;
    cpo::uno::Sequence< double >                            maSolution;
    OUString                                                maStatus;

    // Sensitivity report
    css::sheet::SensitivityReport m_aSensitivityReport;

    // Direct cell access through mpDocument.
    void SetValue(const css::table::CellAddress& rPosition, double fValue);
    double GetValue(const css::table::CellAddress& rPosition);

public:
                            SolverComponent();
    virtual                 ~SolverComponent() override;

    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;    // from OPropertyArrayUsageHelper

                            // XSolver
    virtual cpo::uno::Reference< css::sheet::XSpreadsheetDocument > getDocument() override;
    virtual void   setDocument( const cpo::uno::Reference<
                                    css::sheet::XSpreadsheetDocument >& _document ) override;
    virtual css::table::CellAddress getObjective() override;
    virtual void   setObjective( const css::table::CellAddress& _objective ) override;
    virtual cpo::uno::Sequence< css::table::CellAddress > getVariables() override;
    virtual void   setVariables( const cpo::uno::Sequence<
                                    css::table::CellAddress >& _variables ) override;
    virtual cpo::uno::Sequence< css::sheet::SolverConstraint > getConstraints() override;
    virtual void   setConstraints( const cpo::uno::Sequence<
                                    css::sheet::SolverConstraint >& _constraints ) override;
    virtual bool getMaximize() override;
    virtual void   setMaximize( bool _maximize ) override;

    virtual bool getSuccess() override;
    virtual double getResultValue() override;
    virtual cpo::uno::Sequence< double > getSolution() override;

    virtual void solve() override = 0;

                            // XSolverDescription
    virtual OUString getComponentDescription() override = 0;
    virtual OUString getStatusDescription() override;
    virtual OUString getPropertyDescription( const OUString& aPropertyName ) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override = 0;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
