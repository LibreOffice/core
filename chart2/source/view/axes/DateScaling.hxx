/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _CHART2_DATESCALING_HXX
#define _CHART2_DATESCALING_HXX
#include "ServiceMacros.hxx"
#include <com/sun/star/chart2/XScaling.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase3.hxx>
#include <tools/date.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class DateScaling :
        public ::cppu::WeakImplHelper3 <
        ::com::sun::star::chart2::XScaling,
        ::com::sun::star::lang::XServiceName,
        ::com::sun::star::lang::XServiceInfo
        >
{
public:
    DateScaling( const Date& rNullDate, sal_Int32 nTimeUnit, bool bShifted );
    virtual ~DateScaling();

    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

    // ____ XScaling ____
    virtual double SAL_CALL doScaling( double value )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XScaling > SAL_CALL
        getInverseScaling() throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const Date m_aNullDate;
    const sal_Int32 m_nTimeUnit;
    const bool m_bShifted;
};

class InverseDateScaling :
        public ::cppu::WeakImplHelper3 <
        ::com::sun::star::chart2::XScaling,
        ::com::sun::star::lang::XServiceName,
        ::com::sun::star::lang::XServiceInfo
        >
{
public:
    InverseDateScaling( const Date& rNullDate, sal_Int32 nTimeUnit, bool bShifted );
    virtual ~InverseDateScaling();

    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

    // ____ XScaling ____
    virtual double SAL_CALL doScaling( double value )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XScaling > SAL_CALL
        getInverseScaling() throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const Date m_aNullDate;
    const sal_Int32 m_nTimeUnit;
    const bool m_bShifted;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

