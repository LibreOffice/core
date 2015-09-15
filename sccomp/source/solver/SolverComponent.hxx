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
                com::sun::star::sheet::XSolver,
                com::sun::star::sheet::XSolverDescription,
                com::sun::star::lang::XServiceInfo >
        SolverComponent_Base;

class SolverComponent : public comphelper::OMutexAndBroadcastHelper,
                        public comphelper::OPropertyContainer,
                        public comphelper::OPropertyArrayUsageHelper< SolverComponent >,
                        public SolverComponent_Base
{
protected:
    static ResMgr* pSolverResMgr;

    // settings
    com::sun::star::uno::Reference< com::sun::star::sheet::XSpreadsheetDocument > mxDoc;
    com::sun::star::table::CellAddress                                            maObjective;
    com::sun::star::uno::Sequence< com::sun::star::table::CellAddress >           maVariables;
    com::sun::star::uno::Sequence< com::sun::star::sheet::SolverConstraint >      maConstraints;
    bool                                                                      mbMaximize;
    // set via XPropertySet
    bool                                                                      mbNonNegative;
    bool                                                                      mbInteger;
    sal_Int32                                                                     mnTimeout;
    sal_Int32                                                                     mnEpsilonLevel;
    bool                                                                      mbLimitBBDepth;
    // results
    bool                                                                      mbSuccess;
    double                                                                        mfResultValue;
    com::sun::star::uno::Sequence< double >                                       maSolution;
    OUString                                                                 maStatus;

    static OUString GetResourceString( sal_uInt32 nId );
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
    virtual                 ~SolverComponent();

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;     // from OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const SAL_OVERRIDE;    // from OPropertyArrayUsageHelper

                            // XSolver
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument > SAL_CALL getDocument()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setDocument( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::sheet::XSpreadsheetDocument >& _document )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::table::CellAddress SAL_CALL getObjective() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setObjective( const ::com::sun::star::table::CellAddress& _objective )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellAddress > SAL_CALL getVariables()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setVariables( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::table::CellAddress >& _variables )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::SolverConstraint > SAL_CALL getConstraints()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setConstraints( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::sheet::SolverConstraint >& _constraints )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getMaximize() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setMaximize( sal_Bool _maximize ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL getSuccess() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getResultValue() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getSolution()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL solve() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;

                            // XSolverDescription
    virtual OUString SAL_CALL getComponentDescription() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;
    virtual OUString SAL_CALL getStatusDescription() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getPropertyDescription( const OUString& aPropertyName )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
