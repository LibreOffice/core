/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <o3tl/cppunittraitshelper.hxx>
#include <test/bootstrapfixture.hxx>
#include <cppunit/TestAssert.h>

#include <vcl/mnemonic.hxx>

class VclMnemonicTest : public test::BootstrapFixture
{
public:
    VclMnemonicTest()
        : BootstrapFixture(true, false)
    {
    }

    void testMnemonic();

    CPPUNIT_TEST_SUITE(VclMnemonicTest);
    CPPUNIT_TEST(testMnemonic);
    CPPUNIT_TEST_SUITE_END();
};

void VclMnemonicTest::testMnemonic()
{
    MnemonicGenerator aGenerator;

    {
        OUString sResult = aGenerator.CreateMnemonic(u"ßa");
        CPPUNIT_ASSERT_EQUAL(u'~', sResult[1]);
    }

    {
        static constexpr OUStringLiteral TEST = u"\u4E00b";
        OUString sResult = aGenerator.CreateMnemonic(TEST);
        CPPUNIT_ASSERT_EQUAL(u'~', sResult[1]);
    }

    {
        static constexpr OUStringLiteral TEST = u"\u4E00";
        OUString sResult = aGenerator.CreateMnemonic(TEST);
        CPPUNIT_ASSERT_EQUAL(OUString("(~C)"), sResult.copy(sResult.getLength() - 4));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclMnemonicTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
