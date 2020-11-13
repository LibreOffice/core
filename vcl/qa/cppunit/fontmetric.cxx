/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <cppunit/TestAssert.h>

#include <vcl/metric.hxx>

class VclFontMetricTest : public test::BootstrapFixture
{
public:
    VclFontMetricTest() : BootstrapFixture(true, false) {}

    void testFullstopCenteredFlag();
    void testSpacings();
    void testSlant();
    void testBulletOffset();
    void testEqualityOperator();

    CPPUNIT_TEST_SUITE(VclFontMetricTest);
    CPPUNIT_TEST(testFullstopCenteredFlag);
    CPPUNIT_TEST(testSpacings);
    CPPUNIT_TEST(testSlant);
    CPPUNIT_TEST(testBulletOffset);
    CPPUNIT_TEST(testEqualityOperator);
    CPPUNIT_TEST_SUITE_END();
};

void VclFontMetricTest::testFullstopCenteredFlag()
{
    // default constructor should set scalable flag to false
    FontMetric aFontMetric;

    CPPUNIT_ASSERT_MESSAGE( "Fullstop centered flag should be false after default constructor called", !aFontMetric.IsFullstopCentered() );

    aFontMetric.SetFullstopCenteredFlag(true);

    CPPUNIT_ASSERT_MESSAGE( "Fullstop centered flag should be true", aFontMetric.IsFullstopCentered() );
}

void VclFontMetricTest::testSpacings()
{
    // default constructor should set scalable flag to false
    FontMetric aFontMetric;

    CPPUNIT_ASSERT_EQUAL(tools::Long(0), aFontMetric.GetAscent());
    CPPUNIT_ASSERT_EQUAL(tools::Long(0), aFontMetric.GetDescent());
    CPPUNIT_ASSERT_EQUAL(tools::Long(0), aFontMetric.GetExternalLeading());
    CPPUNIT_ASSERT_EQUAL(tools::Long(0), aFontMetric.GetInternalLeading());
    CPPUNIT_ASSERT_EQUAL(tools::Long(0), aFontMetric.GetLineHeight());


    aFontMetric.SetAscent( 100 );
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aFontMetric.GetAscent());

    aFontMetric.SetDescent( 100 );
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aFontMetric.GetDescent());

    aFontMetric.SetExternalLeading( 100 );
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aFontMetric.GetExternalLeading());

    aFontMetric.SetInternalLeading( 100 );
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aFontMetric.GetInternalLeading());

    aFontMetric.SetLineHeight( 100 );
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aFontMetric.GetLineHeight());
}

void VclFontMetricTest::testSlant()
{
    // default constructor should set scalable flag to false
    FontMetric aFontMetric;

    CPPUNIT_ASSERT_EQUAL(tools::Long(0), aFontMetric.GetSlant());

    aFontMetric.SetSlant( 45 );
    CPPUNIT_ASSERT_EQUAL(tools::Long(45), aFontMetric.GetSlant());
}

void VclFontMetricTest::testBulletOffset()
{
    // default constructor should set scalable flag to false
    FontMetric aFontMetric;

    CPPUNIT_ASSERT_EQUAL(tools::Long(0), aFontMetric.GetBulletOffset());

    aFontMetric.SetBulletOffset( 45 );
    CPPUNIT_ASSERT_EQUAL(tools::Long(45), aFontMetric.GetBulletOffset());
}

void VclFontMetricTest::testEqualityOperator()
{
    // default constructor should set scalable flag to false
    FontMetric aLhs, aRhs;

    aLhs.SetFullstopCenteredFlag(true);
    aRhs.SetFullstopCenteredFlag(true);
    CPPUNIT_ASSERT_MESSAGE( "Fullstop centered flag set same, aLhs == aRhs failed", aLhs.operator ==(aRhs) );
    CPPUNIT_ASSERT_MESSAGE( "Fullstop centered flag set same, aLhs != aRhs succeeded", !aLhs.operator !=(aRhs) );

    aLhs.SetExternalLeading(10);
    aRhs.SetExternalLeading(10);
    CPPUNIT_ASSERT_MESSAGE( "External leading set same, aLHS == aRhs failed", aLhs.operator ==(aRhs) );
    CPPUNIT_ASSERT_MESSAGE( "External leading set same, aLHS != aRhs succeeded", !aLhs.operator !=(aRhs) );

    aLhs.SetInternalLeading(10);
    aRhs.SetInternalLeading(10);
    CPPUNIT_ASSERT_MESSAGE( "Internal leading set same, aLHS == aRhs failed", aLhs.operator ==(aRhs) );
    CPPUNIT_ASSERT_MESSAGE( "Internal leading set same, aLHS != aRhs succeeded", !aLhs.operator !=(aRhs) );

    aLhs.SetAscent( 100 );
    aRhs.SetAscent( 100 );
    CPPUNIT_ASSERT_MESSAGE( "Ascent set same, aLHS == aRhs failed", aLhs.operator ==(aRhs) );
    CPPUNIT_ASSERT_MESSAGE( "Ascent set same, aLHS != aRhs succeeded", !aLhs.operator !=(aRhs) );

    aLhs.SetDescent( 100 );
    aRhs.SetDescent( 100 );
    CPPUNIT_ASSERT_MESSAGE( "Descent set same, aLHS == aRhs failed", aLhs.operator ==(aRhs));
    CPPUNIT_ASSERT_MESSAGE( "Descent set same, aLHS != aRhs succeeded", !aLhs.operator !=(aRhs) );

    aLhs.SetSlant( 100 );
    aRhs.SetSlant( 100 );
    CPPUNIT_ASSERT_MESSAGE( "Slant set same, aLHS == aRhs failed", aLhs.operator ==(aRhs));
    CPPUNIT_ASSERT_MESSAGE( "Slant set same, aLHS != aRhs succeeded", !aLhs.operator !=(aRhs) );
}


CPPUNIT_TEST_SUITE_REGISTRATION(VclFontMetricTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
