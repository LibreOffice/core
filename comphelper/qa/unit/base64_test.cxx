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

#include <limits>

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>

#include <comphelper/sequence.hxx>
#include <comphelper/base64.hxx>
#include <sal/log.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::util;


namespace {

class Base64Test
    : public ::CppUnit::TestFixture
{
public:

    void testBase64();

    CPPUNIT_TEST_SUITE(Base64Test);
    CPPUNIT_TEST(testBase64);
    CPPUNIT_TEST_SUITE_END();

private:
};



void doTestEncodeBase64(char const*const pis, const uno::Sequence<sal_Int8>& aPass)
{
    OUString const is(OUString::createFromAscii(pis));
    OUStringBuffer buf;
    comphelper::Base64::encode(buf, aPass);
    SAL_INFO("sax.cppunit","" << buf.toString());
    CPPUNIT_ASSERT_EQUAL(is, buf.makeStringAndClear());
}

void doTestDecodeBase64(const uno::Sequence<sal_Int8>& aPass, char const*const pis)
{
    OUString const is(OUString::createFromAscii(pis));
    uno::Sequence< sal_Int8 > tempSequence;
    comphelper::Base64::decode(tempSequence, is);
    SAL_INFO("sax.cppunit","" << is);
    bool b = (tempSequence==aPass);
    CPPUNIT_ASSERT(b);
}

void Base64Test::testBase64()
{
    std::vector< sal_Int8 > tempSeq { 0, 0, 0, 0, 0, 1, 2, 3 };
    uno::Sequence< sal_Int8 > tempSequence = comphelper::containerToSequence(tempSeq);
    doTestEncodeBase64("AAAAAAABAgM=", tempSequence);
    doTestDecodeBase64(tempSequence, "AAAAAAABAgM=");
    tempSeq[0] = sal_Int8(5);
    tempSeq[1] = sal_Int8(2);
    tempSeq[2] = sal_Int8(3);
    tempSequence = comphelper::containerToSequence(tempSeq);
    doTestEncodeBase64("BQIDAAABAgM=", tempSequence);
    doTestDecodeBase64(tempSequence, "BQIDAAABAgM=");
    tempSeq[0] = sal_Int8(sal_uInt8(200));
    tempSeq[1] = sal_Int8(31);
    tempSeq[2] = sal_Int8(77);
    tempSeq[3] = sal_Int8(111);
    tempSequence = comphelper::containerToSequence(tempSeq);
    doTestEncodeBase64("yB9NbwABAgM=", tempSequence);
    doTestDecodeBase64(tempSequence, "yB9NbwABAgM=");
}

CPPUNIT_TEST_SUITE_REGISTRATION(Base64Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
