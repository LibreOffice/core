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
    void testManualKashida();
    void testFinalYeh();
    void testNoZwnjExpansion();
    void testExcludeInvalid();
    void testSyriac();

    CPPUNIT_TEST_SUITE(KashidaTest);
    CPPUNIT_TEST(testCharacteristic);
    CPPUNIT_TEST(testManualKashida);
    CPPUNIT_TEST(testFinalYeh);
    CPPUNIT_TEST(testNoZwnjExpansion);
    CPPUNIT_TEST(testExcludeInvalid);
    CPPUNIT_TEST(testSyriac);
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

void KashidaTest::testManualKashida()
{
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), GetWordKashidaPosition(u"برـای"_ustr).value().nIndex);

    // Normally, a kashida would not be inserted after a final Yeh.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), GetWordKashidaPosition(u"نیمِـي"_ustr).value().nIndex);
}

// tdf#65344: Do not insert kashida before a final Yeh
void KashidaTest::testFinalYeh()
{
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), GetWordKashidaPosition(u"يييي"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), GetWordKashidaPosition(u"ييي"_ustr).value().nIndex);
    CPPUNIT_ASSERT(!GetWordKashidaPosition(u"يي"_ustr).has_value());

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

// tdf#163105: Do not insert kashida if the position is invalid
void KashidaTest::testExcludeInvalid()
{
    std::vector<bool> aValid;
    aValid.resize(5, true);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         GetWordKashidaPosition(u"نویسه"_ustr, aValid).value().nIndex);

    aValid[3] = false;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         GetWordKashidaPosition(u"نویسه"_ustr, aValid).value().nIndex);

    // Calls after this use the last resort (positions in aValid from end to start)
    aValid[0] = false;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                         GetWordKashidaPosition(u"نویسه"_ustr, aValid).value().nIndex);

    aValid[2] = false;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         GetWordKashidaPosition(u"نویسه"_ustr, aValid).value().nIndex);

    aValid[1] = false;
    CPPUNIT_ASSERT(!GetWordKashidaPosition(u"نویسه"_ustr, aValid).has_value());
}

// tdf#140767: Kashida justification for Syriac
void KashidaTest::testSyriac()
{
    // - Prefer user-inserted kashida
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), GetWordKashidaPosition(u"ܥܥـܥܥܥܥ"_ustr).value().nIndex);

    std::vector<bool> aValid;
    aValid.resize(7, true);

    // - Start from end and work toward midpoint, then reverse direction
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5),
                         GetWordKashidaPosition(u"ܥܥܥܥܥܥܥ"_ustr, aValid).value().nIndex);
    aValid[5] = false;

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4),
                         GetWordKashidaPosition(u"ܥܥܥܥܥܥܥ"_ustr, aValid).value().nIndex);
    aValid[4] = false;

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         GetWordKashidaPosition(u"ܥܥܥܥܥܥܥ"_ustr, aValid).value().nIndex);
    aValid[0] = false;

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         GetWordKashidaPosition(u"ܥܥܥܥܥܥܥ"_ustr, aValid).value().nIndex);
    aValid[1] = false;

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                         GetWordKashidaPosition(u"ܥܥܥܥܥܥܥ"_ustr, aValid).value().nIndex);
    aValid[2] = false;

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         GetWordKashidaPosition(u"ܥܥܥܥܥܥܥ"_ustr, aValid).value().nIndex);
    aValid[3] = false;

    CPPUNIT_ASSERT(!GetWordKashidaPosition(u"ܥܥܥܥܥܥܥ"_ustr, aValid).has_value());
}

CPPUNIT_TEST_SUITE_REGISTRATION(KashidaTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
