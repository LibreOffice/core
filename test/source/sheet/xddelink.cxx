/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xddelink.hxx>

#include <com/sun/star/sheet/XDDELink.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XDDELink::testGetApplication()
{
    uno::Reference<sheet::XDDELink> xLink(init(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to execute getApplication()", OUString("soffice"),
                                 xLink->getApplication());
}
void XDDELink::testGetItem()
{
    uno::Reference<sheet::XDDELink> xLink(init(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to execute getItem()", OUString("Sheet1.A1"),
                                 xLink->getItem());
}
void XDDELink::testGetTopic()
{
    uno::Reference<sheet::XDDELink> xLink(init(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to execute getTopic()", m_URL, xLink->getTopic());
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
