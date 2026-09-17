/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <test/bootstrapfixture.hxx>

#include <drawinglayer/attribute/strokeattribute.hxx>

using namespace drawinglayer::attribute;

namespace
{
class StrokeAttributeTest : public test::BootstrapFixture
{
};

CPPUNIT_TEST_FIXTURE(StrokeAttributeTest, tdf152997_odd_length_dash_array)
{
    std::vector<double> aDashArray{ 6.0 };
    StrokeAttribute aStroke(std::move(aDashArray));

    // Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 1
    // i.e. the odd-length array was not duplicated
    const std::vector<double> aExpected{ 6.0, 6.0 };
    CPPUNIT_ASSERT_EQUAL(aExpected.size(), aStroke.getDotDashArray().size());
    CPPUNIT_ASSERT_EQUAL(12.0, aStroke.getFullDotDashLen());
    for (size_t i = 0; i < aExpected.size(); ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aExpected[i], aStroke.getDotDashArray()[i], 1E-12);
}

CPPUNIT_TEST_FIXTURE(StrokeAttributeTest, tdf152997_odd_length_dash_array_multiple_values)
{
    std::vector<double> aDashArray{ 5.0, 3.0, 2.0 };
    StrokeAttribute aStroke(std::move(aDashArray));

    // Without the fix in place, this test would have failed with
    // - Expected: 6
    // - Actual  : 3
    // i.e. the odd-length array was not duplicated
    const std::vector<double> aExpected{ 5.0, 3.0, 2.0, 5.0, 3.0, 2.0 };
    CPPUNIT_ASSERT_EQUAL(aExpected.size(), aStroke.getDotDashArray().size());
    for (size_t i = 0; i < aExpected.size(); ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aExpected[i], aStroke.getDotDashArray()[i], 1E-12);
}

CPPUNIT_TEST_FIXTURE(StrokeAttributeTest, tdf152997_even_length_dash_array)
{
    std::vector<double> aDashArray{ 4.0, 2.0 };
    StrokeAttribute aStroke(std::move(aDashArray), 6.0);

    // An even-length dash array should not be changed
    const std::vector<double> aExpected{ 4.0, 2.0 };
    CPPUNIT_ASSERT_EQUAL(aExpected.size(), aStroke.getDotDashArray().size());
    CPPUNIT_ASSERT_EQUAL(6.0, aStroke.getFullDotDashLen());
    for (size_t i = 0; i < aExpected.size(); ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aExpected[i], aStroke.getDotDashArray()[i], 1E-12);
}

CPPUNIT_TEST_FIXTURE(StrokeAttributeTest, tdf152997_empty_dash_array)
{
    std::vector<double> aDashArray;
    StrokeAttribute aStroke(std::move(aDashArray));

    // An empty dash array should not be changed
    CPPUNIT_ASSERT(aStroke.getDotDashArray().empty());
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
