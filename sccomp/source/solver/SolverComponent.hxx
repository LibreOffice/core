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

#ifndef INCLUDED_SCCOMP_SOURCE_SOLVER_SOLVERCOMPONENT_HXX
#define INCLUDED_SCCOMP_SOURCE_SOLVER_SOLVERCOMPONENT_HXX

#include <com/sun/star/sheet/XSolver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>

#include <unordered_map>

class ResMgr;

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

typedef cppu::WeakImplHelper<
                css::sheet::XSolver,
                css::sheet::XSolverDescription,
                css::lang::XServiceInfo >
        SolverComponent_Base;

class SolverComponent : public comphelper::OMutexAndBroadcastHelper,
                        public comphelper::OPropertyContainer,
                        public comphelper::OPropertyArrayUsageHelper< SolverComponent >,
                        public SolverComponent_Base
{
protected:
    static ResMgr* pSolverResMgr;

    // settings
    css::uno::Reference< css::sheet::XSpreadsheetDocument > mxDoc;
    css::table::CellAddress                                 maObjective;
    css::uno::Sequence< css::table::CellAddress >           maVariables;
    css::uno::Sequence< css::sheet::SolverConstraint >      maConstraints;
    bool                                                    mbMaximize;
    // set via XPropertySet
    bool                                                    mbNonNegative;
    bool                                                    mbInteger;
    sal_Int32                                               mnTimeout;
    sal_Int32                                               mnEpsilonLevel;
    bool                                                    mbLimitBBDepth;
    // results
    bool                                                    mbSuccess;
    double                                                  mfResultValue;
    css::uno::Sequence< double >                            maSolution;
    OUString                                                maStatus;

    static OUString GetResourceString(const char* pId);
    static css::uno::Reference<css::table::XCell> GetCell(
            const css::uno::Reference<css::sheet::XSpreadsheetDocument>& xDoc,
            const css::table::CellAddress& rPos );
    static void SetValue(
            const css::uno::Reference<css::sheet::XSpreadsheetDocument>& xDoc,
            const css::table::CellAddress& rPos, double fValue );
    static double GetValue(
            const css::uno::Reference<css::sheet::XSpreadsheetDocument>& xDoc,
            const css::table::CellAddress& rPos );

public:
                            SolverComponent();
    virtual                 ~SolverComponent() override;

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;     // from OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;    // from OPropertyArrayUsageHelper

                            // XSolver
    virtual css::uno::Reference< css::sheet::XSpreadsheetDocument > SAL_CALL getDocument() override;
    virtual void SAL_CALL   setDocument( const css::uno::Reference<
                                    css::sheet::XSpreadsheetDocument >& _document ) override;
    virtual css::table::CellAddress SAL_CALL getObjective() override;
    virtual void SAL_CALL   setObjective( const css::table::CellAddress& _objective ) override;
    virtual css::uno::Sequence< css::table::CellAddress > SAL_CALL getVariables() override;
    virtual void SAL_CALL   setVariables( const css::uno::Sequence<
                                    css::table::CellAddress >& _variables ) override;
    virtual css::uno::Sequence< css::sheet::SolverConstraint > SAL_CALL getConstraints() override;
    virtual void SAL_CALL   setConstraints( const css::uno::Sequence<
                                    css::sheet::SolverConstraint >& _constraints ) override;
    virtual sal_Bool SAL_CALL getMaximize() override;
    virtual void SAL_CALL   setMaximize( sal_Bool _maximize ) override;

    virtual sal_Bool SAL_CALL getSuccess() override;
    virtual double SAL_CALL getResultValue() override;
    virtual css::uno::Sequence< double > SAL_CALL getSolution() override;

    virtual void SAL_CALL solve() override = 0;

                            // XSolverDescription
    virtual OUString SAL_CALL getComponentDescription() override = 0;
    virtual OUString SAL_CALL getStatusDescription() override;
    virtual OUString SAL_CALL getPropertyDescription( const OUString& aPropertyName ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override = 0;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
