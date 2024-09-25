/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <i18nutil/kashida.hxx>

using namespace i18nutil;

namespace
{
class KashidaTest : public CppUnit::TestFixture
{
public:
    void testCharacteristic();
    void testFinalYeh();
    void testNoZwnjExpansion();

    CPPUNIT_TEST_SUITE(KashidaTest);
    CPPUNIT_TEST(testCharacteristic);
    CPPUNIT_TEST(testFinalYeh);
    CPPUNIT_TEST(testNoZwnjExpansion);
    CPPUNIT_TEST_SUITE_END();
};

void KashidaTest::testCharacteristic()
{
    // Characteristic tests for kashida candidate selection.
    // Uses words from sample documents.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), GetWordKashidaPosition(u"متن"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"فارسی"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), GetWordKashidaPosition(u"با"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"نویسه"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), GetWordKashidaPosition(u"کشیده"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), GetWordKashidaPosition(u"برای"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), GetWordKashidaPosition(u"چینش"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), GetWordKashidaPosition(u"بهتر"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"ببببب"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"بپپپپ"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"تطویل"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), GetWordKashidaPosition(u"بپ"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), GetWordKashidaPosition(u"تطوی"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"تحویل"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), GetWordKashidaPosition(u"تشویل"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"تمثیل"_ustr).value().nIndex);
}

// tdf#65344: Do not insert kashida before a final Yeh
void KashidaTest::testFinalYeh()
{
    CPPUNIT_ASSERT(!GetWordKashidaPosition(u"نیمِي"_ustr).has_value());

    // Should always insert kashida after Seen, even before a final Yeh
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), GetWordKashidaPosition(u"كرسي"_ustr).value().nIndex);
}

// #i98410#: Do not insert kashida under a ZWNJ
void KashidaTest::testNoZwnjExpansion()
{
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"نویسه"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), GetWordKashidaPosition(u"نویس\u200Cه"_ustr).value().nIndex);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), GetWordKashidaPosition(u"متن"_ustr).value().nIndex);
    CPPUNIT_ASSERT(!GetWordKashidaPosition(u"مت\u200Cن"_ustr).has_value());
}

CPPUNIT_TEST_SUITE_REGISTRATION(KashidaTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
