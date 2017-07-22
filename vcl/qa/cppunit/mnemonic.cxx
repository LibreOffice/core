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

#include <vcl/mnemonic.hxx>

class VclMnemonicTest : public test::BootstrapFixture
{
public:
    VclMnemonicTest() : BootstrapFixture(true, false) {}

    void testMnemonic();

    CPPUNIT_TEST_SUITE(VclMnemonicTest);
    CPPUNIT_TEST(testMnemonic);
    CPPUNIT_TEST_SUITE_END();
};

void VclMnemonicTest::testMnemonic()
{
    MnemonicGenerator aGenerator;

    {
        const OString sIn(u8"ßa");
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), sIn.getLength());
        CPPUNIT_ASSERT_EQUAL(-61, static_cast<int>(sIn[0]));
        CPPUNIT_ASSERT_EQUAL(-97, static_cast<int>(sIn[1]));
        CPPUNIT_ASSERT_EQUAL(97, static_cast<int>(sIn[2]));
        const OUString sOut(OUString::fromUtf8(sIn));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), sOut.getLength());
        CPPUNIT_ASSERT_EQUAL(223, static_cast<int>(sOut[0]));
        CPPUNIT_ASSERT_EQUAL(97, static_cast<int>(sOut[1]));
        OUString sResult = aGenerator.CreateMnemonic(sOut);
        CPPUNIT_ASSERT_EQUAL(u'~', sResult[1]);
    }

    {
        const sal_Unicode TEST[] = { 0x4E00, 'b' };
        OUString sResult = aGenerator.CreateMnemonic(OUString(TEST, SAL_N_ELEMENTS(TEST)));
        CPPUNIT_ASSERT_EQUAL(u'~', sResult[1]);
    }

    {
        const sal_Unicode TEST[] = { 0x4E00 };
        OUString sResult = aGenerator.CreateMnemonic(OUString(TEST, SAL_N_ELEMENTS(TEST)));
        CPPUNIT_ASSERT_EQUAL(OUString("(~C)"), sResult.copy(sResult.getLength() - 4));
    }

}

CPPUNIT_TEST_SUITE_REGISTRATION(VclMnemonicTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
