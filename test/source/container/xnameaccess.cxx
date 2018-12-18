/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/container/xnameaccess.hxx>
#include <rtl/string.hxx>

#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XNameAccess::testGetByName()
{
    uno::Reference<container::XNameAccess> xNA(init(), uno::UNO_QUERY_THROW);

    // test with existing name
    CPPUNIT_ASSERT_NO_THROW(xNA->getByName(m_aName));
    // test with non-existing name
    CPPUNIT_ASSERT_THROW(xNA->getByName(m_aName + "UnitTest"), container::NoSuchElementException);
}

void XNameAccess::testGetElementNames()
{
    uno::Reference<container::XNameAccess> xNA(init(), uno::UNO_QUERY_THROW);
    uno::Sequence<OUString> aNames = xNA->getElementNames();

    CPPUNIT_ASSERT(aNames.getLength());
}

void XNameAccess::testHasByName()
{
    uno::Reference<container::XNameAccess> xNA(init(), uno::UNO_QUERY_THROW);

    // test with existing name
    CPPUNIT_ASSERT(xNA->hasByName(m_aName));
    // test with non-existing name
    CPPUNIT_ASSERT(!xNA->hasByName(m_aName + "UnitTest"));
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
