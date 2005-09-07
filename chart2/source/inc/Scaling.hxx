/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Scaling.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:46:15 $
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
#ifndef _CHART2_SCALING_HXX
#define _CHART2_SCALING_HXX

#ifndef _COM_SUN_STAR_CHART2_XSCALING_HPP_
#include <com/sun/star/chart2/XScaling.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class LogarithmicScaling : public ::cppu::WeakImplHelper2 <
        ::com::sun::star::chart2::XScaling,
        ::com::sun::star::lang::XServiceName
        >
{
public:
    LogarithmicScaling();
    explicit LogarithmicScaling( double fBase );
    virtual ~LogarithmicScaling();

    // ____ XScaling ____
        virtual double SAL_CALL
    doScaling( double value )
        throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XScaling > SAL_CALL
    getInverseScaling() throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fBase;
    const double m_fLogOfBase;
};

// ----------------------------------------

class ExponentialScaling : public ::cppu::WeakImplHelper2 <
        ::com::sun::star::chart2::XScaling,
        ::com::sun::star::lang::XServiceName
        >
{
public:
    ExponentialScaling();
    explicit ExponentialScaling( double fBase );
    virtual ~ExponentialScaling();

    // ____ XScaling ____
        virtual double SAL_CALL
    doScaling( double value )
        throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XScaling > SAL_CALL
    getInverseScaling() throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fBase;
};

// ----------------------------------------

class LinearScaling : public ::cppu::WeakImplHelper2 <
        ::com::sun::star::chart2::XScaling,
        ::com::sun::star::lang::XServiceName
        >
{
public:
    /// y(x) = fSlope * x + fOffset
    LinearScaling( double fSlope, double fOffset );
    virtual ~LinearScaling();

    // ____ XScaling ____
        virtual double SAL_CALL
    doScaling( double value )
        throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XScaling > SAL_CALL
    getInverseScaling() throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fSlope;
    const double m_fOffset;
};

// ----------------------------------------

class PowerScaling : public ::cppu::WeakImplHelper2 <
        ::com::sun::star::chart2::XScaling,
        ::com::sun::star::lang::XServiceName
        >
{
public:
    PowerScaling( double fExponent );
    virtual ~PowerScaling();

    // ____ XScaling ____
        virtual double SAL_CALL
    doScaling( double value )
        throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XScaling > SAL_CALL
    getInverseScaling() throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fExponent;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

