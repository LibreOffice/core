/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <config_features.h>
#include <vcl/dllapi.h>

#include <TypeSerializer.hxx>

namespace
{
class TypeSerializerTest : public CppUnit::TestFixture
{
    void testGradient();

    CPPUNIT_TEST_SUITE(TypeSerializerTest);
    CPPUNIT_TEST(testGradient);
    CPPUNIT_TEST_SUITE_END();
};

void TypeSerializerTest::testGradient()
{
    Gradient aGradient(GradientStyle::Radial, Color(0xFF, 0x00, 0x00), Color(0x00, 0xFF, 0x00));
    aGradient.SetAngle(900);
    aGradient.SetBorder(5);
    aGradient.SetOfsX(11);
    aGradient.SetOfsY(12);
    aGradient.SetStartIntensity(21);
    aGradient.SetEndIntensity(22);
    aGradient.SetSteps(30);

    SvMemoryStream aStream;
    TypeSerializer aSerializer(aStream);
    aSerializer.writeGradient(aGradient);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    Gradient aReadGradient;
    aSerializer.readGradient(aReadGradient);
    CPPUNIT_ASSERT_EQUAL(GradientStyle::Radial, aReadGradient.GetStyle());
    CPPUNIT_ASSERT_EQUAL(Color(0xFF, 0x00, 0x00), aReadGradient.GetStartColor());
    CPPUNIT_ASSERT_EQUAL(Color(0x00, 0xFF, 0x00), aReadGradient.GetEndColor());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(900), aReadGradient.GetAngle());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), aReadGradient.GetBorder());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(11), aReadGradient.GetOfsX());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(12), aReadGradient.GetOfsY());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(21), aReadGradient.GetStartIntensity());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(22), aReadGradient.GetEndIntensity());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(30), aReadGradient.GetSteps());
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(TypeSerializerTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
