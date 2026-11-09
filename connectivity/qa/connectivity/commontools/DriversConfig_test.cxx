/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <connectivity/DriversConfig.hxx>

namespace connectivity::commontools
{
class DriversConfigTest : public test::BootstrapFixture
{
public:
    DriversConfigTest()
        : test::BootstrapFixture(false, false)
    {
    }

    void testTdf174405MysqlNamedParameterFeature();

    CPPUNIT_TEST_SUITE(DriversConfigTest);
    CPPUNIT_TEST(testTdf174405MysqlNamedParameterFeature);
    CPPUNIT_TEST_SUITE_END();
};

// tdf#174405: the MySQL drivers must offer "Replace named parameters with '?'"
// in Advanced Settings, otherwise a document with the setting off cannot
// enable named parameters again
void DriversConfigTest::testTdf174405MysqlNamedParameterFeature()
{
    const DriversConfig aConfig(getComponentContext());
    const auto check = [&aConfig](std::u16string_view rUrl, const char* pMsg) {
        CPPUNIT_ASSERT_MESSAGE(
            pMsg, aConfig.getFeatures(rUrl).getOrDefault(u"ParameterNameSubstitution"_ustr, false));
    };
    check(u"sdbc:mysql:mysqlc:localhost/test",
          "mysqlc driver has no ParameterNameSubstitution feature");
    check(u"sdbc:mysql:jdbc:localhost/test",
          "mysql_jdbc driver has no ParameterNameSubstitution feature");
    check(u"sdbc:mysql:odbc:localhost/test",
          "MySQL ODBC driver has no ParameterNameSubstitution feature");
}

CPPUNIT_TEST_SUITE_REGISTRATION(DriversConfigTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
