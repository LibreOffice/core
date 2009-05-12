/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: solver.hxx,v $
 * $Revision: 1.2 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    rtl::OUString                                                                 maStatus;

public:
                            SolverComponent( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XComponentContext >& rxMSF );
    virtual                 ~SolverComponent();

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
                                throw (::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();     // from OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;    // from OPropertyArrayUsageHelper

                            // XSolver
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument > SAL_CALL getDocument()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setDocument( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::sheet::XSpreadsheetDocument >& _document )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellAddress SAL_CALL getObjective() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setObjective( const ::com::sun::star::table::CellAddress& _objective )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellAddress > SAL_CALL getVariables()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setVariables( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::table::CellAddress >& _variables )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::SolverConstraint > SAL_CALL getConstraints()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setConstraints( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::sheet::SolverConstraint >& _constraints )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getMaximize() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setMaximize( ::sal_Bool _maximize ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL getSuccess() throw(::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getResultValue() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getSolution()
                                throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL solve() throw(::com::sun::star::uno::RuntimeException);

                            // XSolverDescription
    virtual ::rtl::OUString SAL_CALL getComponentDescription() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getStatusDescription() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getPropertyDescription( const ::rtl::OUString& aPropertyName )
                                throw (::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};

#endif

