/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xddelinks.hxx>

#include <com/sun/star/sheet/DDELinkMode.hpp>
#include <com/sun/star/sheet/XDDELink.hpp>
#include <com/sun/star/sheet/XDDELinks.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XDDELinks::testAddDDELink()
{
    uno::Reference<sheet::XDDELinks> xLinks(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::XDDELink> xLink = xLinks->addDDELink(
        u"soffice"_ustr, m_aTopic, u"Sheet1.A1"_ustr, sheet::DDELinkMode_DEFAULT);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to execute getApplication()", u"soffice"_ustr,
                                 xLink->getApplication());
    CPPUNIT_ASSERT_MESSAGE("Unable to execute getTopic()",
                           xLink->getTopic().endsWith("ScDDELinksObj.ods"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to execute getItem()", u"Sheet1.A1"_ustr,
                                 xLink->getItem());
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
