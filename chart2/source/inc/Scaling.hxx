/*************************************************************************
 *
 *  $RCSfile: Scaling.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART2_SCALING_HXX
#define _CHART2_SCALING_HXX

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XSCALING_HPP_
#include <drafts/com/sun/star/chart2/XScaling.hpp>
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
        ::drafts::com::sun::star::chart2::XScaling,
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
        ::drafts::com::sun::star::chart2::XScaling > SAL_CALL
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
        ::drafts::com::sun::star::chart2::XScaling,
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
        ::drafts::com::sun::star::chart2::XScaling > SAL_CALL
    getInverseScaling() throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    const double m_fBase;
};

// ----------------------------------------

class LinearScaling : public ::cppu::WeakImplHelper2 <
        ::drafts::com::sun::star::chart2::XScaling,
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
        ::drafts::com::sun::star::chart2::XScaling > SAL_CALL
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
        ::drafts::com::sun::star::chart2::XScaling,
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
        ::drafts::com::sun::star::chart2::XScaling > SAL_CALL
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

