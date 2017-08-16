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
#include <cppunit/TestFixture.h>

#include <osl/file.hxx>
#include <osl/process.h>

#include <vcl/metric.hxx>

#include "impfont.hxx"

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

    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetAscent(), 0L );
    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetDescent(), 0L );
    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetExternalLeading(), 0L );
    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetInternalLeading(), 0L );
    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetLineHeight(), 0L );


    aFontMetric.SetAscent( 100 );
    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetAscent(), 100L );

    aFontMetric.SetDescent( 100 );
    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetDescent(), 100L );

    aFontMetric.SetExternalLeading( 100 );
    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetExternalLeading(), 100L );

    aFontMetric.SetInternalLeading( 100 );
    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetInternalLeading(), 100L );

    aFontMetric.SetLineHeight( 100 );
    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetLineHeight(), 100L );
}

void VclFontMetricTest::testSlant()
{
    // default constructor should set scalable flag to false
    FontMetric aFontMetric;

    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetSlant(), 0L );

    aFontMetric.SetSlant( 45 );
    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetSlant(), 45L );
}

void VclFontMetricTest::testBulletOffset()
{
    // default constructor should set scalable flag to false
    FontMetric aFontMetric;

    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetBulletOffset(), 0L );

    aFontMetric.SetBulletOffset( 45 );
    CPPUNIT_ASSERT_EQUAL( aFontMetric.GetBulletOffset(), 45L );
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
