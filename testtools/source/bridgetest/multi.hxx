/*************************************************************************
 *
 *  $RCSfile: multi.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-18 13:28:47 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef INCLUDED_testtools_source_bridgetest_multi_hxx
#define INCLUDED_testtools_source_bridgetest_multi_hxx

#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "cppuhelper/implbase1.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "test/testtools/bridgetest/XMulti.hpp"

namespace testtools { namespace bridgetest {

class Multi: public cppu::WeakImplHelper1< test::testtools::bridgetest::XMulti >
{
public:
    Multi(): m_attribute1(0.0), m_attribute3(0.0) {}

    virtual double SAL_CALL getatt1()
        throw (com::sun::star::uno::RuntimeException)
    { return m_attribute1; }

    virtual void SAL_CALL setatt1(double value)
        throw (com::sun::star::uno::RuntimeException)
    { m_attribute1 = value; }

    virtual sal_Int32 SAL_CALL fn11(sal_Int32 arg)
        throw (com::sun::star::uno::RuntimeException)
    { return 11 * arg; }

    virtual rtl::OUString SAL_CALL fn12(rtl::OUString const & arg)
        throw (com::sun::star::uno::RuntimeException)
    { return rtl::OUString::createFromAscii("12") + arg; }

    virtual sal_Int32 SAL_CALL fn21(sal_Int32 arg)
        throw (com::sun::star::uno::RuntimeException)
    { return 21 * arg; }

    virtual rtl::OUString SAL_CALL fn22(rtl::OUString const & arg)
        throw (com::sun::star::uno::RuntimeException)
    { return rtl::OUString::createFromAscii("22") + arg; }

    virtual double SAL_CALL getatt3()
        throw (com::sun::star::uno::RuntimeException)
    { return m_attribute3; }

    virtual void SAL_CALL setatt3(double value)
        throw (com::sun::star::uno::RuntimeException)
    { m_attribute3 = value; }

    virtual sal_Int32 SAL_CALL fn31(sal_Int32 arg)
        throw (com::sun::star::uno::RuntimeException)
    { return 31 * arg; }

    virtual rtl::OUString SAL_CALL fn32(rtl::OUString const & arg)
        throw (com::sun::star::uno::RuntimeException)
    { return rtl::OUString::createFromAscii("32") + arg; }

    virtual sal_Int32 SAL_CALL fn33()
        throw (com::sun::star::uno::RuntimeException)
    { return 33; }

    virtual sal_Int32 SAL_CALL fn41(sal_Int32 arg)
        throw (com::sun::star::uno::RuntimeException)
    { return 41 * arg; }

    virtual sal_Int32 SAL_CALL fn61(sal_Int32 arg)
        throw (com::sun::star::uno::RuntimeException)
    { return 61 * arg; }

    virtual rtl::OUString SAL_CALL fn62(rtl::OUString const & arg)
        throw (com::sun::star::uno::RuntimeException)
    { return rtl::OUString::createFromAscii("62") + arg; }

    virtual sal_Int32 SAL_CALL fn71(sal_Int32 arg)
        throw (com::sun::star::uno::RuntimeException)
    { return 71 * arg; }

    virtual rtl::OUString SAL_CALL fn72(rtl::OUString const & arg)
        throw (com::sun::star::uno::RuntimeException)
    { return rtl::OUString::createFromAscii("72") + arg; }

    virtual sal_Int32 SAL_CALL fn73()
        throw (com::sun::star::uno::RuntimeException)
    { return 73; }

private:
    double m_attribute1;
    double m_attribute3;
};

rtl::OUString testMulti(
    com::sun::star::uno::Reference< test::testtools::bridgetest::XMulti >
    const & multi);

} }

#endif
