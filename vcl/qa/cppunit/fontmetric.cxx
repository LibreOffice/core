/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <osl/file.hxx>
#include <osl/process.h>

#include <vcl/metric.hxx>

#include "impfont.hxx"

class VclFontMetricTest : public test::BootstrapFixture
{
public:
    VclFontMetricTest() : BootstrapFixture(true, false) {}

    void testScalableFlag();
    void testEqualityOperator();

    CPPUNIT_TEST_SUITE(VclFontMetricTest);
    CPPUNIT_TEST(testScalableFlag);
    CPPUNIT_TEST(testEqualityOperator);
    CPPUNIT_TEST_SUITE_END();
};

void VclFontMetricTest::testScalableFlag()
{
    // default constructor should set scalable flag to false
    FontMetric aFontMetric;

    CPPUNIT_ASSERT_MESSAGE( "Scalable flag should be false after default constructor called", !aFontMetric.IsScalable() );

    aFontMetric.SetScalableFlag(true);

    CPPUNIT_ASSERT_MESSAGE( "Scalable flag should be true", aFontMetric.IsScalable() );
}

void VclFontMetricTest::testEqualityOperator()
{
    // default constructor should set scalable flag to false
    FontMetric aLhs, aRhs;

    aLhs.SetScalableFlag(true);
    aRhs.SetScalableFlag(true);

    CPPUNIT_ASSERT_MESSAGE( "Scalable flag set same", aLhs == aRhs );
}


CPPUNIT_TEST_SUITE_REGISTRATION(VclFontMetricTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
