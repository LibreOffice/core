/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "unitsimpl.hxx"

#include "helper/qahelper.hxx"

using namespace sc::units;

// In order to be able to access the private members of UnitsImpl for
// testing, we need to be a friend of UnitsImpl. For this to work
// UnitsTest can't be a member of the anonymous namespace hence the
// need to use a namespace here.
namespace sc {
namespace units {
namespace test {

class UnitsTest:
    public ::test::BootstrapFixture
{
public:
    UnitsTest() {};
    virtual ~UnitsTest() {};

    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    ::boost::shared_ptr< UnitsImpl > mpUnitsImpl;

    void testStringExtraction();

    CPPUNIT_TEST_SUITE(UnitsTest);
    CPPUNIT_TEST(testStringExtraction);
    CPPUNIT_TEST_SUITE_END();
};

void UnitsTest::setUp() {
    BootstrapFixture::setUp();

    mpUnitsImpl = UnitsImpl::GetUnits();
}

void UnitsTest::tearDown() {
    BootstrapFixture::tearDown();
}

void UnitsTest::testStringExtraction() {
    CPPUNIT_ASSERT(mpUnitsImpl->extractUnitStringFromFormat("\"weight: \"0.0\"kg\"") == "kg");
    CPPUNIT_ASSERT(mpUnitsImpl->extractUnitStringFromFormat("#\"cm\"") == "cm");
}

CPPUNIT_TEST_SUITE_REGISTRATION(UnitsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

}}} // namespace sc::units::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
