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
#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <svl/lngmisc.hxx>

#include <rtl/ustrbuf.hxx>

namespace
{
class LngMiscTest : public CppUnit::TestFixture
{
private:
    void testRemoveHyphens();
    void testRemoveControlChars();
    void testReplaceControlChars();
    void testGetThesaurusReplaceText();

    CPPUNIT_TEST_SUITE(LngMiscTest);

    CPPUNIT_TEST(testRemoveHyphens);
    CPPUNIT_TEST(testRemoveControlChars);
    CPPUNIT_TEST(testReplaceControlChars);
    CPPUNIT_TEST(testGetThesaurusReplaceText);

    CPPUNIT_TEST_SUITE_END();
};

void LngMiscTest::testRemoveHyphens()
{
    OUString str1(u""_ustr);
    OUString str2(u"a-b--c---"_ustr);

    OUString str3 = OUStringChar(SVT_SOFT_HYPHEN) + OUStringChar(SVT_HARD_HYPHEN)
                    + OUStringChar(SVT_HARD_HYPHEN);

    OUString str4(u"asdf"_ustr);

    bool bModified = linguistic::RemoveHyphens(str1);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str1.isEmpty());

    // Note that '-' isn't a hyphen to RemoveHyphens.
    bModified = linguistic::RemoveHyphens(str2);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT_EQUAL(u"a-b--c---"_ustr, str2);

    bModified = linguistic::RemoveHyphens(str3);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT(str3.isEmpty());

    bModified = linguistic::RemoveHyphens(str4);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT_EQUAL(u"asdf"_ustr, str4);
}

void LngMiscTest::testRemoveControlChars()
{
    OUString str1(u""_ustr);
    OUString str2(u"asdf"_ustr);
    OUString str3(u"asdf\nasdf"_ustr);

    OUStringBuffer str4Buf(33);
    str4Buf.setLength(33);
    for (int i = 0; i < 33; i++)
        str4Buf[i] = static_cast<sal_Unicode>(i);
    //    TODO: is this a bug? shouldn't RemoveControlChars remove this?
    //    str4Buf[33] = static_cast<sal_Unicode>(0x7F);
    OUString str4(str4Buf.makeStringAndClear());

    bool bModified = linguistic::RemoveControlChars(str1);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str1.isEmpty());

    bModified = linguistic::RemoveControlChars(str2);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT_EQUAL(u"asdf"_ustr, str2);

    bModified = linguistic::RemoveControlChars(str3);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT_EQUAL(u"asdfasdf"_ustr, str3);

    bModified = linguistic::RemoveControlChars(str4);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, str4);
}

void LngMiscTest::testReplaceControlChars()
{
    OUString str1(u""_ustr);
    OUString str2(u"asdf"_ustr);
    OUString str3(u"asdf\nasdf"_ustr);

    OUStringBuffer str4Buf(33);
    str4Buf.setLength(33);
    for (int i = 0; i < 33; i++)
        str4Buf[i] = static_cast<sal_Unicode>(i);
    //    TODO: is this a bug? shouldn't RemoveControlChars remove this?
    //    str4Buf[33] = static_cast<sal_Unicode>(0x7F);
    OUString str4(str4Buf.makeStringAndClear());

    bool bModified = linguistic::ReplaceControlChars(str1);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str1.isEmpty());

    bModified = linguistic::ReplaceControlChars(str2);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT_EQUAL(u"asdf"_ustr, str2);

    bModified = linguistic::ReplaceControlChars(str3);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT_EQUAL(u"asdf asdf"_ustr, str3);

    bModified = linguistic::ReplaceControlChars(str4);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(32), str4.getLength());
    for (int i = 0; i < 32; i++)
        CPPUNIT_ASSERT_EQUAL(u' ', str4[i]);
}

void LngMiscTest::testGetThesaurusReplaceText()
{
    constexpr OUString str2(u"asdf"_ustr);

    OUString r = linguistic::GetThesaurusReplaceText(u""_ustr);
    CPPUNIT_ASSERT(r.isEmpty());

    r = linguistic::GetThesaurusReplaceText(str2);
    CPPUNIT_ASSERT_EQUAL(str2, r);

    r = linguistic::GetThesaurusReplaceText(u"asdf (abc)"_ustr);
    CPPUNIT_ASSERT_EQUAL(str2, r);

    r = linguistic::GetThesaurusReplaceText(u"asdf*"_ustr);
    CPPUNIT_ASSERT_EQUAL(str2, r);

    r = linguistic::GetThesaurusReplaceText(u"asdf * "_ustr);
    CPPUNIT_ASSERT_EQUAL(str2, r);

    r = linguistic::GetThesaurusReplaceText(u"asdf (abc) *"_ustr);
    CPPUNIT_ASSERT_EQUAL(str2, r);

    r = linguistic::GetThesaurusReplaceText(u"asdf asdf * (abc)"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"asdf asdf"_ustr, r);

    r = linguistic::GetThesaurusReplaceText(u" * (abc) asdf *"_ustr);
    CPPUNIT_ASSERT(r.isEmpty());
}

CPPUNIT_TEST_SUITE_REGISTRATION(LngMiscTest);
}
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
