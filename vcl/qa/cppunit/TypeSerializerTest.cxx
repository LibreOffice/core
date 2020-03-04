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
    Gradient aGradient(GradientStyle::Linear, Color(0xFF, 0x00, 0x00), Color(0x00, 0xFF, 0x00));
    aGradient.SetAngle(900);

    SvMemoryStream aStream;
    TypeSerializer aSerializer(aStream);
    aSerializer.writeGradient(aGradient);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    Gradient aReadGradient;
    aSerializer.readGradient(aReadGradient);
    CPPUNIT_ASSERT(aReadGradient == aGradient);
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(TypeSerializerTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
