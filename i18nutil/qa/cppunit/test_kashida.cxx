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

    CPPUNIT_TEST_SUITE(KashidaTest);
    CPPUNIT_TEST(testCharacteristic);
    CPPUNIT_TEST_SUITE_END();
};

void KashidaTest::testCharacteristic()
{
    // Characteristic tests for kashida candidate selection.
    // Uses words from sample documents.
    CPPUNIT_ASSERT(!GetWordKashidaPosition(u"متن"_ustr).has_value());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"فارسی"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), GetWordKashidaPosition(u"با"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"نویسه"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), GetWordKashidaPosition(u"کشیده"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), GetWordKashidaPosition(u"برای"_ustr).value().nIndex);
    CPPUNIT_ASSERT(!GetWordKashidaPosition(u"چینش"_ustr).has_value());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), GetWordKashidaPosition(u"بهتر"_ustr).value().nIndex);
    CPPUNIT_ASSERT(!GetWordKashidaPosition(u"ببببب"_ustr).has_value());
    CPPUNIT_ASSERT(!GetWordKashidaPosition(u"بپپپپ"_ustr).has_value());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"تطویل"_ustr).value().nIndex);
    CPPUNIT_ASSERT(!GetWordKashidaPosition(u"بپ"_ustr).has_value());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), GetWordKashidaPosition(u"تطوی"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"تحویل"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), GetWordKashidaPosition(u"تشویل"_ustr).value().nIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), GetWordKashidaPosition(u"تمثیل"_ustr).value().nIndex);
}

CPPUNIT_TEST_SUITE_REGISTRATION(KashidaTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
