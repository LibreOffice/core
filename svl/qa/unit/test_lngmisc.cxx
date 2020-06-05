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
    OUString str1("");
    OUString str2("a-b--c---");

    OUString str3 = OUStringChar(SVT_SOFT_HYPHEN) + OUStringChar(SVT_HARD_HYPHEN) + OUStringChar(SVT_HARD_HYPHEN);

    OUString str4("asdf");

    bool bModified = linguistic::RemoveHyphens(str1);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str1.isEmpty());

    // Note that '-' isn't a hyphen to RemoveHyphens.
    bModified = linguistic::RemoveHyphens(str2);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT_EQUAL( OUString("a-b--c---"), str2 );

    bModified = linguistic::RemoveHyphens(str3);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT(str3.isEmpty());

    bModified = linguistic::RemoveHyphens(str4);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT_EQUAL( OUString("asdf"), str4 );
  }

  void LngMiscTest::testRemoveControlChars()
  {
    OUString str1("");
    OUString str2("asdf");
    OUString str3("asdf\nasdf");

    OUStringBuffer str4Buf(33);
    str4Buf.setLength(33);
    for(int i = 0; i < 33; i++)
      str4Buf[i] = static_cast<sal_Unicode>(i);
    //    TODO: is this a bug? shouldn't RemoveControlChars remove this?
    //    str4Buf[33] = static_cast<sal_Unicode>(0x7F);
    OUString str4(str4Buf.makeStringAndClear());

    bool bModified = linguistic::RemoveControlChars(str1);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str1.isEmpty());

    bModified = linguistic::RemoveControlChars(str2);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT_EQUAL( OUString("asdf"), str2 );

    bModified = linguistic::RemoveControlChars(str3);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT_EQUAL( OUString("asdfasdf"), str3 );

    bModified = linguistic::RemoveControlChars(str4);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT_EQUAL( OUString(" "), str4 );
  }

  void LngMiscTest::testReplaceControlChars()
  {
    OUString str1("");
    OUString str2("asdf");
    OUString str3("asdf\nasdf");

    OUStringBuffer str4Buf(33);
    str4Buf.setLength(33);
    for(int i = 0; i < 33; i++)
      str4Buf[i] = static_cast<sal_Unicode>(i);
    //    TODO: is this a bug? shouldn't RemoveControlChars remove this?
    //    str4Buf[33] = static_cast<sal_Unicode>(0x7F);
    OUString str4(str4Buf.makeStringAndClear());

    bool bModified = linguistic::ReplaceControlChars(str1);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str1.isEmpty());

    bModified = linguistic::ReplaceControlChars(str2);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT_EQUAL( OUString("asdf"), str2 );

    bModified = linguistic::ReplaceControlChars(str3);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT_EQUAL(OUString("asdf asdf"),  str3 );

    bModified = linguistic::ReplaceControlChars(str4);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(32), str4.getLength());
    for(int i = 0; i < 32; i++)
        CPPUNIT_ASSERT_EQUAL(u' ', str4[i]);
  }

  void LngMiscTest::testGetThesaurusReplaceText()
  {
    const OUString str2("asdf");

    OUString r = linguistic::GetThesaurusReplaceText("");
    CPPUNIT_ASSERT(r.isEmpty());

    r = linguistic::GetThesaurusReplaceText(str2);
    CPPUNIT_ASSERT_EQUAL(str2, r);

    r = linguistic::GetThesaurusReplaceText("asdf (abc)");
    CPPUNIT_ASSERT_EQUAL(str2, r);

    r = linguistic::GetThesaurusReplaceText("asdf*");
    CPPUNIT_ASSERT_EQUAL(str2, r);

    r = linguistic::GetThesaurusReplaceText("asdf * ");
    CPPUNIT_ASSERT_EQUAL(str2, r);

    r = linguistic::GetThesaurusReplaceText("asdf (abc) *");
    CPPUNIT_ASSERT_EQUAL(str2, r);

    r = linguistic::GetThesaurusReplaceText("asdf asdf * (abc)");
    CPPUNIT_ASSERT_EQUAL(OUString("asdf asdf"), r);

    r = linguistic::GetThesaurusReplaceText(" * (abc) asdf *");
    CPPUNIT_ASSERT(r.isEmpty());
  }

  CPPUNIT_TEST_SUITE_REGISTRATION(LngMiscTest);
}
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
