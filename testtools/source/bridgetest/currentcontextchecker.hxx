/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: currentcontextchecker.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:45:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_TESTTOOLS_SOURCE_BRIDGETEST_CURRENTCONTEXTCHECKER_HXX
#define INCLUDED_TESTTOOLS_SOURCE_BRIDGETEST_CURRENTCONTEXTCHECKER_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include "com/sun/star/uno/RuntimeException.hpp"
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include "cppuhelper/implbase1.hxx"
#endif
#ifndef OSL_DIAGNOSE_HXX_INCLUDED
#include "osl/diagnose.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _TEST_TESTTOOLS_BRIDGETEST_XCURRENTCONTEXTCHECKER_HPP_
#include "test/testtools/bridgetest/XCurrentContextChecker.hpp"
#endif

namespace testtools { namespace bridgetest {

class CurrentContextChecker:
    private ::osl::DebugBase< CurrentContextChecker >,
    public ::cppu::WeakImplHelper1<
        ::test::testtools::bridgetest::XCurrentContextChecker >
{
public:
    CurrentContextChecker();

    virtual ~CurrentContextChecker();

    virtual ::sal_Bool SAL_CALL perform(
        ::com::sun::star::uno::Reference<
            ::test::testtools::bridgetest::XCurrentContextChecker > const &
            other,
        ::sal_Int32 setSteps, ::sal_Int32 checkSteps)
        throw (::com::sun::star::uno::RuntimeException);

private:
    CurrentContextChecker(CurrentContextChecker &); // not defined
    void operator =(CurrentContextChecker &); // not defined

    bool performCheck(
        ::com::sun::star::uno::Reference<
            ::test::testtools::bridgetest::XCurrentContextChecker > const &
            other,
        ::sal_Int32 setSteps, ::sal_Int32 checkSteps);
};

} }

#endif
