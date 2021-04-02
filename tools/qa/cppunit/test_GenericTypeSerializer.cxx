/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/types.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <tools/GenericTypeSerializer.hxx>
#include <tools/stream.hxx>
#include <tools/gen.hxx>

namespace tools
{
class GenericTypeSerializerTest : public CppUnit::TestFixture
{
public:
    void testRoundtripPoint()
    {
        Point aPoint(20, 50);
        SvMemoryStream aStream;
        GenericTypeSerializer aSerializer(aStream);
        aSerializer.writePoint(aPoint);
        aStream.Seek(STREAM_SEEK_TO_BEGIN);
        Point aReadPoint;
        aSerializer.readPoint(aReadPoint);
        CPPUNIT_ASSERT_EQUAL(aPoint, aReadPoint);
    }

    void testRoundtripSize()
    {
        Size aSize(40, 80);
        SvMemoryStream aStream;
        GenericTypeSerializer aSerializer(aStream);
        aSerializer.writeSize(aSize);
        aStream.Seek(STREAM_SEEK_TO_BEGIN);
        Size aReadSize;
        aSerializer.readSize(aReadSize);
        CPPUNIT_ASSERT_EQUAL(aSize, aReadSize);
    }

    void testRoundtripRectangle()
    {
        {
            Rectangle aRectangle;
            CPPUNIT_ASSERT(aRectangle.IsEmpty());
            SvMemoryStream aStream;
            aStream.Seek(STREAM_SEEK_TO_BEGIN);
            GenericTypeSerializer aSerializer(aStream);
            aSerializer.writeRectangle(aRectangle);
            aStream.Seek(STREAM_SEEK_TO_BEGIN);
            // Need to set the rectangle to non-empty, so it will be set to empty later
            Rectangle aReadRectangle(Point(20, 50), Size(10, 30));
            aSerializer.readRectangle(aReadRectangle);
            CPPUNIT_ASSERT(aRectangle.IsEmpty());
        }

        {
            Rectangle aRectangle(Point(20, 50), Size(10, 30));
            SvMemoryStream aStream;
            aStream.Seek(STREAM_SEEK_TO_BEGIN);
            GenericTypeSerializer aSerializer(aStream);
            aSerializer.writeRectangle(aRectangle);
            aStream.Seek(STREAM_SEEK_TO_BEGIN);
            Rectangle aReadRectangle;
            aSerializer.readRectangle(aReadRectangle);
            CPPUNIT_ASSERT_EQUAL(aRectangle.Top(), aReadRectangle.Top());
            CPPUNIT_ASSERT_EQUAL(aRectangle.Left(), aReadRectangle.Left());
            CPPUNIT_ASSERT_EQUAL(aRectangle.Right(), aReadRectangle.Right());
            CPPUNIT_ASSERT_EQUAL(aRectangle.Bottom(), aReadRectangle.Bottom());
        }
    }

    void testRoundtripFraction()
    {
        {
            Fraction aFraction(2, 5);
            CPPUNIT_ASSERT_EQUAL(true, aFraction.IsValid());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aFraction.GetNumerator());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aFraction.GetDenominator());

            SvMemoryStream aStream;
            aStream.Seek(STREAM_SEEK_TO_BEGIN);
            GenericTypeSerializer aSerializer(aStream);
            aSerializer.writeFraction(aFraction);

            aStream.Seek(STREAM_SEEK_TO_BEGIN);

            Fraction aReadFraction(1, 2);
            aSerializer.readFraction(aReadFraction);
            CPPUNIT_ASSERT_EQUAL(true, aReadFraction.IsValid());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aReadFraction.GetNumerator());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aReadFraction.GetDenominator());
        }
        {
            Fraction aFraction(1, 0);
            CPPUNIT_ASSERT_EQUAL(false, aFraction.IsValid());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aFraction.GetNumerator());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aFraction.GetDenominator());

            SvMemoryStream aStream;
            aStream.Seek(STREAM_SEEK_TO_BEGIN);
            GenericTypeSerializer aSerializer(aStream);
            aSerializer.writeFraction(aFraction);

            aStream.Seek(STREAM_SEEK_TO_BEGIN);

            Fraction aReadFraction(1, 2);
            aSerializer.readFraction(aReadFraction);
            CPPUNIT_ASSERT_EQUAL(false, aReadFraction.IsValid());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aReadFraction.GetNumerator());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aReadFraction.GetDenominator());
        }
    }

    CPPUNIT_TEST_SUITE(GenericTypeSerializerTest);
    CPPUNIT_TEST(testRoundtripPoint);
    CPPUNIT_TEST(testRoundtripSize);
    CPPUNIT_TEST(testRoundtripRectangle);
    CPPUNIT_TEST(testRoundtripFraction);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GenericTypeSerializerTest);

} // namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
