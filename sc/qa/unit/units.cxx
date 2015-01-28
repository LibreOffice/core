/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "units.hxx"

#include "helper/qahelper.hxx"

class UnitsTest:
    public test::BootstrapFixture
{
public:
    UnitsTest() {};
    virtual ~UnitsTest() {};

    void testStringExtraction();

    CPPUNIT_TEST_SUITE(UnitsTest);
    CPPUNIT_TEST(testStringExtraction);
    CPPUNIT_TEST_SUITE_END();
};

void UnitsTest::testStringExtraction() {
    CPPUNIT_ASSERT(extractUnitStringFromFormat("\"weight: \"0.0\"kg\"") == "kg");
    CPPUNIT_ASSERT(extractUnitStringFromFormat("#\"cm\"") == "cm");
}

CPPUNIT_TEST_SUITE_REGISTRATION(UnitsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
