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

class VclFontMetricTest : public test::BootstrapFixture
{
public:
    VclFontMetricTest() : BootstrapFixture(true, false) {}

    void testScalableFlag();

    CPPUNIT_TEST_SUITE(VclFontMetricTest);
    CPPUNIT_TEST(testScalableFlag);
    CPPUNIT_TEST_SUITE_END();
};

void VclFontMetricTest::testScalableFlag()
{
    // default constructor should set scalable flag to false
    FontMetric aFontMetric;

    CPPUNIT_ASSERT( !aFontMetric.IsScalable() );

    aFontMetric.SetScalableFlag(true);

    CPPUNIT_ASSERT( aFontMetric.IsScalable() );
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclFontMetricTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
