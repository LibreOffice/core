/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the LibreOffice contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/types.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <limits>
#include <vector>

#include <CorrelationCoefficient.hxx>

namespace
{
constexpr double fTolerance = 1e-12;

const double fNotANumber = std::numeric_limits<double>::quiet_NaN();

std::optional<double> lclCoefficientOf(const std::vector<double>& rFirst,
                                       const std::vector<double>& rSecond)
{
    return chart::calculateCorrelationCoefficient(rFirst, rSecond);
}

} // anonymous namespace

class CorrelationCoefficientTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(CorrelationCoefficientTest);
    CPPUNIT_TEST(testValuesThatRiseTogether);
    CPPUNIT_TEST(testValuesThatMoveApart);
    CPPUNIT_TEST(testValuesWithoutARelation);
    CPPUNIT_TEST(testMissingValuesAreLeftOut);
    CPPUNIT_TEST(testRunsOfDifferentLength);
    CPPUNIT_TEST(testAnswerlessInput);
    CPPUNIT_TEST_SUITE_END();

    void testValuesThatRiseTogether();
    void testValuesThatMoveApart();
    void testValuesWithoutARelation();
    void testMissingValuesAreLeftOut();
    void testRunsOfDifferentLength();
    void testAnswerlessInput();
};

// Values that rise together, whatever their scale and offset, are perfectly
// related.
void CorrelationCoefficientTest::testValuesThatRiseTogether()
{
    std::optional<double> oCoefficient
        = lclCoefficientOf({ 1.0, 2.0, 3.0, 4.0 }, { 11.0, 13.0, 15.0, 17.0 });
    CPPUNIT_ASSERT(oCoefficient.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, *oCoefficient, fTolerance);
}

void CorrelationCoefficientTest::testValuesThatMoveApart()
{
    std::optional<double> oCoefficient
        = lclCoefficientOf({ 1.0, 2.0, 3.0, 4.0 }, { 8.0, 6.0, 4.0, 2.0 });
    CPPUNIT_ASSERT(oCoefficient.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, *oCoefficient, fTolerance);
}

// Uncorrelated sequences
void CorrelationCoefficientTest::testValuesWithoutARelation()
{
    std::optional<double> oCoefficient
        = lclCoefficientOf({ 1.0, 2.0, 3.0, 4.0 }, { 1.0, -1.0, -1.0, 1.0 });
    CPPUNIT_ASSERT(oCoefficient.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, *oCoefficient, fTolerance);
}

// The instances where one side is missing count for nothing, so the answer
// matches the one for the pairs that are left.
void CorrelationCoefficientTest::testMissingValuesAreLeftOut()
{
    std::optional<double> oWithGaps = lclCoefficientOf({ 1.0, fNotANumber, 2.0, 3.0, 4.0 },
                                                       { 8.0, 100.0, 6.0, fNotANumber, 2.0 });
    std::optional<double> oWithoutGaps = lclCoefficientOf({ 1.0, 2.0, 4.0 }, { 8.0, 6.0, 2.0 });
    CPPUNIT_ASSERT(oWithGaps.has_value());
    CPPUNIT_ASSERT(oWithoutGaps.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(*oWithoutGaps, *oWithGaps, fTolerance);
}

// The shorter run says how many instances there are to compare.
void CorrelationCoefficientTest::testRunsOfDifferentLength()
{
    std::optional<double> oCoefficient
        = lclCoefficientOf({ 1.0, 2.0, 3.0 }, { 2.0, 4.0, 6.0, 8.0, 10.0 });
    CPPUNIT_ASSERT(oCoefficient.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, *oCoefficient, fTolerance);
}

// A singleton pair, empty pair, and a run that holds one value throughout all
// leave the coefficient undefined.
void CorrelationCoefficientTest::testAnswerlessInput()
{
    CPPUNIT_ASSERT(!lclCoefficientOf({ 1.0 }, { 2.0 }).has_value());
    CPPUNIT_ASSERT(!lclCoefficientOf({}, {}).has_value());
    CPPUNIT_ASSERT(!lclCoefficientOf({ 1.0, fNotANumber }, { 2.0, 3.0 }).has_value());
    CPPUNIT_ASSERT(!lclCoefficientOf({ 5.0, 5.0, 5.0 }, { 1.0, 2.0, 3.0 }).has_value());
    CPPUNIT_ASSERT(!lclCoefficientOf({ 1.0, 2.0, 3.0 }, { 5.0, 5.0, 5.0 }).has_value());
}

CPPUNIT_TEST_SUITE_REGISTRATION(CorrelationCoefficientTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
