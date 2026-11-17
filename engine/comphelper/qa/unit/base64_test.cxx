/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <cpo/uno/Sequence.hxx>

#include <comphelper/base64.hxx>

#include <iterator>
#include <string>

using namespace css;

namespace
{
class Base64Test : public CppUnit::TestFixture
{
public:
    void testBase64Encode();
    void testBase64Decode();
    void testBase64EncodeForOStringBuffer();
    void testBase64EncodeIntoBuffer();

    CPPUNIT_TEST_SUITE(Base64Test);
    CPPUNIT_TEST(testBase64Encode);
    CPPUNIT_TEST(testBase64Decode);
    CPPUNIT_TEST(testBase64EncodeForOStringBuffer);
    CPPUNIT_TEST(testBase64EncodeIntoBuffer);
    CPPUNIT_TEST_SUITE_END();
};

void Base64Test::testBase64Encode()
{
    OUStringBuffer aBuffer(32);
    cpo::uno::Sequence<sal_Int8> inputSequence;

    inputSequence = { 0, 0, 0, 0, 0, 1, 2, 3 };
    comphelper::Base64::encode(aBuffer, inputSequence);
    CPPUNIT_ASSERT_EQUAL(u"AAAAAAABAgM="_ustr, aBuffer.toString());
    aBuffer.setLength(0);

    inputSequence = { 5, 2, 3, 0, 0, 1, 2, 3 };
    comphelper::Base64::encode(aBuffer, inputSequence);
    CPPUNIT_ASSERT_EQUAL(u"BQIDAAABAgM="_ustr, aBuffer.toString());
    aBuffer.setLength(0);

    inputSequence = { sal_Int8(sal_uInt8(200)), 31, 77, 111, 0, 1, 2, 3 };
    comphelper::Base64::encode(aBuffer, inputSequence);
    CPPUNIT_ASSERT_EQUAL(u"yB9NbwABAgM="_ustr, aBuffer.makeStringAndClear());
}

void Base64Test::testBase64EncodeIntoBuffer()
{
    // Each length from 0 to 9 bytes encodes as the string buffer form does, and no more than
    // encodedSize() characters are written.
    const sal_Int8 aBytes[] = { 5, 2, 3, 0, sal_Int8(sal_uInt8(200)), 31, 77, 111, 1 };
    for (std::size_t nSize = 0; nSize <= std::size(aBytes); ++nSize)
    {
        OStringBuffer aExpected;
        comphelper::Base64::encode(aExpected,
                                   cpo::uno::Sequence<sal_Int8>(aBytes, sal_Int32(nSize)));

        const std::size_t nEncoded = comphelper::Base64::encodedSize(nSize);
        std::string aActual(nEncoded + 1, '#');
        comphelper::Base64::encode(aActual.data(), aBytes, nSize);
        CPPUNIT_ASSERT_EQUAL('#', aActual[nEncoded]);
        aActual.resize(nEncoded);
        CPPUNIT_ASSERT_EQUAL(std::string(aExpected.getStr(), aExpected.getLength()), aActual);
    }
}

void Base64Test::testBase64Decode()
{
    cpo::uno::Sequence<sal_Int8> decodedSequence;

    cpo::uno::Sequence<sal_Int8> expectedSequence = { 0, 0, 0, 0, 0, 1, 2, 3 };
    comphelper::Base64::decode(decodedSequence, u"AAAAAAABAgM=");
    CPPUNIT_ASSERT(std::equal(std::cbegin(expectedSequence), std::cend(expectedSequence),
                              std::cbegin(decodedSequence)));

    expectedSequence = { 5, 2, 3, 0, 0, 1, 2, 3 };
    comphelper::Base64::decode(decodedSequence, u"BQIDAAABAgM=");
    CPPUNIT_ASSERT(std::equal(std::cbegin(expectedSequence), std::cend(expectedSequence),
                              std::cbegin(decodedSequence)));

    expectedSequence = { sal_Int8(sal_uInt8(200)), 31, 77, 111, 0, 1, 2, 3 };
    comphelper::Base64::decode(decodedSequence, u"yB9NbwABAgM=");
    CPPUNIT_ASSERT(std::equal(std::cbegin(expectedSequence), std::cend(expectedSequence),
                              std::cbegin(decodedSequence)));
}

void Base64Test::testBase64EncodeForOStringBuffer()
{
    OStringBuffer aBuffer(32);
    cpo::uno::Sequence<sal_Int8> inputSequence;

    inputSequence = { 0, 0, 0, 0, 0, 1, 2, 3 };
    comphelper::Base64::encode(aBuffer, inputSequence);
    CPPUNIT_ASSERT_EQUAL("AAAAAAABAgM="_ostr, aBuffer.toString());
    aBuffer.setLength(0);

    inputSequence = { 5, 2, 3, 0, 0, 1, 2, 3 };
    comphelper::Base64::encode(aBuffer, inputSequence);
    CPPUNIT_ASSERT_EQUAL("BQIDAAABAgM="_ostr, aBuffer.toString());
    aBuffer.setLength(0);

    inputSequence = { sal_Int8(sal_uInt8(200)), 31, 77, 111, 0, 1, 2, 3 };
    comphelper::Base64::encode(aBuffer, inputSequence);
    CPPUNIT_ASSERT_EQUAL("yB9NbwABAgM="_ostr, aBuffer.makeStringAndClear());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Base64Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
