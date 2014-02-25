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

#ifndef SOLVER_HXX
#define SOLVER_HXX

#include <com/sun/star/sheet/XSolver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>

typedef cppu::WeakImplHelper3<
                com::sun::star::sheet::XSolver,
                com::sun::star::sheet::XSolverDescription,
                com::sun::star::lang::XServiceInfo >
        SolverComponent_Base;

class SolverComponent : public comphelper::OMutexAndBroadcastHelper,
                        public comphelper::OPropertyContainer,
                        public comphelper::OPropertyArrayUsageHelper< SolverComponent >,
                        public SolverComponent_Base
{
    // settings
    com::sun::star::uno::Reference< com::sun::star::sheet::XSpreadsheetDocument > mxDoc;
    com::sun::star::table::CellAddress                                            maObjective;
    com::sun::star::uno::Sequence< com::sun::star::table::CellAddress >           maVariables;
    com::sun::star::uno::Sequence< com::sun::star::sheet::SolverConstraint >      maConstraints;
    sal_Bool                                                                      mbMaximize;
    // set via XPropertySet
    sal_Bool                                                                      mbNonNegative;
    sal_Bool                                                                      mbInteger;
    sal_Int32                                                                     mnTimeout;
    sal_Int32                                                                     mnEpsilonLevel;
    sal_Bool                                                                      mbLimitBBDepth;
    // results
    sal_Bool                                                                      mbSuccess;
    double                                                                        mfResultValue;
    com::sun::star::uno::Sequence< double >                                       maSolution;
    OUString                                                                 maStatus;

public:
                            SolverComponent( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XComponentContext >& rxMSF );
    virtual                 ~SolverComponent();

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
                                throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();     // from OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;    // from OPropertyArrayUsageHelper

                            // XSolver
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument > SAL_CALL getDocument()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   setDocument( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::sheet::XSpreadsheetDocument >& _document )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::table::CellAddress SAL_CALL getObjective() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   setObjective( const ::com::sun::star::table::CellAddress& _objective )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellAddress > SAL_CALL getVariables()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   setVariables( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::table::CellAddress >& _variables )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::SolverConstraint > SAL_CALL getConstraints()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   setConstraints( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::sheet::SolverConstraint >& _constraints )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::sal_Bool SAL_CALL getMaximize() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   setMaximize( ::sal_Bool _maximize ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    virtual ::sal_Bool SAL_CALL getSuccess() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual double SAL_CALL getResultValue() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getSolution()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);

    virtual void SAL_CALL solve() throw(::com::sun::star::uno::RuntimeException, std::exception);

                            // XSolverDescription
    virtual OUString SAL_CALL getComponentDescription() throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getStatusDescription() throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getPropertyDescription( const OUString& aPropertyName )
                                throw (::com::sun::star::uno::RuntimeException, std::exception);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
