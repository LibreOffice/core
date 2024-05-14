/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/container/xnamed.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XNamed::testGetName()
{
    uno::Reference<container::XNamed> xNamed(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(m_aTestName, xNamed->getName());
}

void XNamed::testSetName()
{
    uno::Reference<container::XNamed> xNamed(init(), UNO_QUERY_THROW);
    OUString aTestName(u"NewName"_ustr);

    xNamed->setName(aTestName);
    CPPUNIT_ASSERT_EQUAL(aTestName, xNamed->getName());

    // restore old name
    xNamed->setName(m_aTestName);
    CPPUNIT_ASSERT_EQUAL(m_aTestName, xNamed->getName());
}

void XNamed::testSetNameByScSheetLinkObj()
{
    uno::Reference<container::XNamed> xNamed(init(), uno::UNO_QUERY_THROW);
    OUString aTestName(m_aTestName.replaceAll("ScSheetLinkObj", "NewScSheetLinkObj"));

    xNamed->setName(aTestName);
    CPPUNIT_ASSERT_EQUAL(aTestName, xNamed->getName());

    // restore old name
    xNamed->setName(m_aTestName);
    CPPUNIT_ASSERT_EQUAL(m_aTestName, xNamed->getName());
}

void XNamed::testSetNameThrowsException()
{
    uno::Reference<container::XNamed> xNamed(init(), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_THROW(xNamed->setName(u"NewName"_ustr), uno::RuntimeException);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
