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
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/base64.hxx>

using namespace css;

namespace
{
class Base64Test : public CppUnit::TestFixture
{
public:
    void testBase64Encode();
    void testBase64Decode();
    void testBase64EncodeForOStringBuffer();

    CPPUNIT_TEST_SUITE(Base64Test);
    CPPUNIT_TEST(testBase64Encode);
    CPPUNIT_TEST(testBase64Decode);
    CPPUNIT_TEST(testBase64EncodeForOStringBuffer);
    CPPUNIT_TEST_SUITE_END();
};

void Base64Test::testBase64Encode()
{
    OUStringBuffer aBuffer(32);
    uno::Sequence<sal_Int8> inputSequence;

    inputSequence = { 0, 0, 0, 0, 0, 1, 2, 3 };
    comphelper::Base64::encode(aBuffer, inputSequence);
    CPPUNIT_ASSERT_EQUAL(OUString("AAAAAAABAgM="), aBuffer.toString());
    aBuffer.setLength(0);

    inputSequence = { 5, 2, 3, 0, 0, 1, 2, 3 };
    comphelper::Base64::encode(aBuffer, inputSequence);
    CPPUNIT_ASSERT_EQUAL(OUString("BQIDAAABAgM="), aBuffer.toString());
    aBuffer.setLength(0);

    inputSequence = { sal_Int8(sal_uInt8(200)), 31, 77, 111, 0, 1, 2, 3 };
    comphelper::Base64::encode(aBuffer, inputSequence);
    CPPUNIT_ASSERT_EQUAL(OUString("yB9NbwABAgM="), aBuffer.makeStringAndClear());
}

void Base64Test::testBase64Decode()
{
    uno::Sequence<sal_Int8> decodedSequence;

    uno::Sequence<sal_Int8> expectedSequence = { 0, 0, 0, 0, 0, 1, 2, 3 };
    comphelper::Base64::decode(decodedSequence, "AAAAAAABAgM=");
    CPPUNIT_ASSERT(
        std::equal(expectedSequence.begin(), expectedSequence.end(), decodedSequence.begin()));

    expectedSequence = { 5, 2, 3, 0, 0, 1, 2, 3 };
    comphelper::Base64::decode(decodedSequence, "BQIDAAABAgM=");
    CPPUNIT_ASSERT(
        std::equal(expectedSequence.begin(), expectedSequence.end(), decodedSequence.begin()));

    expectedSequence = { sal_Int8(sal_uInt8(200)), 31, 77, 111, 0, 1, 2, 3 };
    comphelper::Base64::decode(decodedSequence, "yB9NbwABAgM=");
    CPPUNIT_ASSERT(
        std::equal(expectedSequence.begin(), expectedSequence.end(), decodedSequence.begin()));
}

void Base64Test::testBase64EncodeForOStringBuffer()
{
    OStringBuffer aBuffer(32);
    uno::Sequence<sal_Int8> inputSequence;

    inputSequence = { 0, 0, 0, 0, 0, 1, 2, 3 };
    comphelper::Base64::encode(aBuffer, inputSequence);
    CPPUNIT_ASSERT_EQUAL(OString("AAAAAAABAgM="), aBuffer.toString());
    aBuffer.setLength(0);

    inputSequence = { 5, 2, 3, 0, 0, 1, 2, 3 };
    comphelper::Base64::encode(aBuffer, inputSequence);
    CPPUNIT_ASSERT_EQUAL(OString("BQIDAAABAgM="), aBuffer.toString());
    aBuffer.setLength(0);

    inputSequence = { sal_Int8(sal_uInt8(200)), 31, 77, 111, 0, 1, 2, 3 };
    comphelper::Base64::encode(aBuffer, inputSequence);
    CPPUNIT_ASSERT_EQUAL(OString("yB9NbwABAgM="), aBuffer.makeStringAndClear());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Base64Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
