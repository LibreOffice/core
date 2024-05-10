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
    void testRemoveMnemonicFromString();
    void testRemoveDoubleMarkedMnemonicFromString();
    void testRemoveMultipleMnemonicsFromString();
    void testRemoveDoubleMarkingsThenMnemonicFromString();
    void testRemoveMnemonicThenDoubleMarkingsFromString();
    void testRemoveMnemonicFromEndOfString();
    void testRemoveNoMnemonicFromString();

    CPPUNIT_TEST_SUITE(VclMnemonicTest);
    CPPUNIT_TEST(testMnemonic);
    CPPUNIT_TEST(testRemoveMnemonicFromString);
    CPPUNIT_TEST(testRemoveDoubleMarkedMnemonicFromString);
    CPPUNIT_TEST(testRemoveMultipleMnemonicsFromString);
    CPPUNIT_TEST(testRemoveDoubleMarkingsThenMnemonicFromString);
    CPPUNIT_TEST(testRemoveMnemonicThenDoubleMarkingsFromString);
    CPPUNIT_TEST(testRemoveMnemonicFromEndOfString);
    CPPUNIT_TEST(testRemoveNoMnemonicFromString);
    CPPUNIT_TEST_SUITE_END();
};

void VclMnemonicTest::testMnemonic()
{
    MnemonicGenerator aGenerator;

    {
        OUString sResult = aGenerator.CreateMnemonic(u"ßa"_ustr);
        CPPUNIT_ASSERT_EQUAL(u'~', sResult[1]);
    }

    {
        static constexpr OUStringLiteral TEST = u"\u4E00b";
        OUString sResult = aGenerator.CreateMnemonic(TEST);
        CPPUNIT_ASSERT_EQUAL(u'~', sResult[1]);
    }

    {
        static constexpr OUString TEST = u"\u4E00"_ustr;
        OUString sResult = aGenerator.CreateMnemonic(TEST);
        CPPUNIT_ASSERT_EQUAL(u"(~C)"_ustr, sResult.copy(sResult.getLength() - 4));
        sResult = MnemonicGenerator::EraseAllMnemonicChars(sResult);
        CPPUNIT_ASSERT_EQUAL(TEST, sResult);
    }
}

void VclMnemonicTest::testRemoveMnemonicFromString()
{
    sal_Int32 nMnemonicIndex;
    OUString sNonMnemonicString = removeMnemonicFromString(u"this is a ~test"_ustr, nMnemonicIndex);
    CPPUNIT_ASSERT_EQUAL(u"this is a test"_ustr, sNonMnemonicString);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(10), nMnemonicIndex);
}

void VclMnemonicTest::testRemoveDoubleMarkedMnemonicFromString()
{
    sal_Int32 nMnemonicIndex;
    OUString sNonMnemonicString
        = removeMnemonicFromString(u"this ~~is a test"_ustr, nMnemonicIndex);
    CPPUNIT_ASSERT_EQUAL(u"this ~is a test"_ustr, sNonMnemonicString);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), nMnemonicIndex);
}

void VclMnemonicTest::testRemoveMultipleMnemonicsFromString()
{
    sal_Int32 nMnemonicIndex;
    OUString sNonMnemonicString
        = removeMnemonicFromString(u"t~his is a ~test"_ustr, nMnemonicIndex);
    CPPUNIT_ASSERT_EQUAL(u"this is a test"_ustr, sNonMnemonicString);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nMnemonicIndex);
}

void VclMnemonicTest::testRemoveDoubleMarkingsThenMnemonicFromString()
{
    sal_Int32 nMnemonicIndex;
    OUString sNonMnemonicString
        = removeMnemonicFromString(u"t~~his is a ~test"_ustr, nMnemonicIndex);
    CPPUNIT_ASSERT_EQUAL(u"t~his is a test"_ustr, sNonMnemonicString);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11), nMnemonicIndex);
}

void VclMnemonicTest::testRemoveMnemonicThenDoubleMarkingsFromString()
{
    sal_Int32 nMnemonicIndex;
    OUString sNonMnemonicString
        = removeMnemonicFromString(u"t~his is a ~~test"_ustr, nMnemonicIndex);
    CPPUNIT_ASSERT_EQUAL(u"this is a ~test"_ustr, sNonMnemonicString);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nMnemonicIndex);
}

void VclMnemonicTest::testRemoveMnemonicFromEndOfString()
{
    sal_Int32 nMnemonicIndex;
    OUString sNonMnemonicString = removeMnemonicFromString(u"this is a test~"_ustr, nMnemonicIndex);
    CPPUNIT_ASSERT_EQUAL(u"this is a test~"_ustr, sNonMnemonicString);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), nMnemonicIndex);
}

void VclMnemonicTest::testRemoveNoMnemonicFromString()
{
    sal_Int32 nMnemonicIndex;
    OUString sNonMnemonicString = removeMnemonicFromString(u"this is a test"_ustr, nMnemonicIndex);
    CPPUNIT_ASSERT_EQUAL(u"this is a test"_ustr, sNonMnemonicString);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), nMnemonicIndex);
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclMnemonicTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
