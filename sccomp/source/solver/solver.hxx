/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: solver.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2008-02-15 15:19:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SOLVER_HXX
#define SOLVER_HXX

#ifndef _COM_SUN_STAR_SHEET_XSOLVER_HPP_
#include <com/sun/star/sheet/XSolver.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSOLVERDESCRIPTION_HPP_
#include <com/sun/star/sheet/XSolverDescription.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif

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

