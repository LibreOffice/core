/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <cppunit/TestAssert.h>

#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>

class VclFontMetricTest : public test::BootstrapFixture
{
    OUString maDataUrl = u"/vcl/qa/cppunit/data/"_ustr;

    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }

protected:
    bool addFont(OutputDevice* pOutDev, std::u16string_view sFileName,
                 std::u16string_view sFamilyName)
    {
        OutputDevice::ImplClearAllFontData(true);
        bool bAdded = pOutDev->AddTempDevFont(getFullUrl(sFileName), OUString(sFamilyName));
        OutputDevice::ImplRefreshAllFontData(true);
        return bAdded;
    }

public:
    VclFontMetricTest() : BootstrapFixture(true, false) {}

    void testFullstopCenteredFlag();
    void testSpacings();
    void testSlant();
    void testBulletOffset();
    void testEqualityOperator();
    void testHheaMatchingWinMetrics();

    CPPUNIT_TEST_SUITE(VclFontMetricTest);
    CPPUNIT_TEST(testFullstopCenteredFlag);
    CPPUNIT_TEST(testSpacings);
    CPPUNIT_TEST(testSlant);
    CPPUNIT_TEST(testBulletOffset);
    CPPUNIT_TEST(testEqualityOperator);
    CPPUNIT_TEST(testHheaMatchingWinMetrics);
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

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aFontMetric.GetAscent());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aFontMetric.GetDescent());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aFontMetric.GetExternalLeading());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aFontMetric.GetInternalLeading());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aFontMetric.GetLineHeight());


    aFontMetric.SetAscent( 100 );
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aFontMetric.GetAscent());

    aFontMetric.SetDescent( 100 );
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aFontMetric.GetDescent());

    aFontMetric.SetExternalLeading( 100 );
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aFontMetric.GetExternalLeading());

    aFontMetric.SetInternalLeading( 100 );
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aFontMetric.GetInternalLeading());

    aFontMetric.SetLineHeight( 100 );
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), aFontMetric.GetLineHeight());
}

void VclFontMetricTest::testSlant()
{
    // default constructor should set scalable flag to false
    FontMetric aFontMetric;

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aFontMetric.GetSlant());

    aFontMetric.SetSlant( 45 );
    CPPUNIT_ASSERT_EQUAL(sal_Int32(45), aFontMetric.GetSlant());
}

void VclFontMetricTest::testBulletOffset()
{
    // default constructor should set scalable flag to false
    FontMetric aFontMetric;

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aFontMetric.GetBulletOffset());

    aFontMetric.SetBulletOffset( 45 );
    CPPUNIT_ASSERT_EQUAL(sal_Int32(45), aFontMetric.GetBulletOffset());
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


void VclFontMetricTest::testHheaMatchingWinMetrics()
{
    // Two fonts with different hhea tables but identical OS/2 Win and Typo metrics:
    // - TestMetricsA: hhea ascent/descent differ from Win
    // - TestMetricsB: hhea ascent/descent match Win exactly

    // Both should produce the same ascent, descent, and external leading,
    // because when hhea differs from Win but the total line height matches,
    // the code should prefer Win metrics for compatibility.

    ScopedVclPtrInstance<VirtualDevice> pOutDev;

    if (!addFont(pOutDev, u"TestFont_HheaDiffersWin.ttf", u"TestMetricsA"))
        return; // skip if fonts cannot be loaded

    if (!addFont(pOutDev, u"TestFont_HheaEqualsWin.ttf", u"TestMetricsB"))
        return; // skip if fonts cannot be loaded

    // Measure font A (hhea != Win)
    vcl::Font aFontA(u"TestMetricsA"_ustr, u"Regular"_ustr, Size(0, 2048));
    pOutDev->SetFont(aFontA);
    FontMetric aMetricA = pOutDev->GetFontMetric();

    // Measure font B (hhea == Win)
    vcl::Font aFontB(u"TestMetricsB"_ustr, u"Regular"_ustr, Size(0, 2048));
    pOutDev->SetFont(aFontB);
    FontMetric aMetricB = pOutDev->GetFontMetric();

    // Both fonts should have the same metrics
    CPPUNIT_ASSERT_EQUAL(aMetricA.GetAscent(), aMetricB.GetAscent());
    CPPUNIT_ASSERT_EQUAL(aMetricA.GetDescent(), aMetricB.GetDescent());
    CPPUNIT_ASSERT_EQUAL(aMetricA.GetExternalLeading(), aMetricB.GetExternalLeading());
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclFontMetricTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
