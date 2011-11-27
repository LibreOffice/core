#include "sal/config.h"
#include "sal/precppunit.hxx"

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

#include "svl/lngmisc.hxx"

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
    ::rtl::OUString str1(RTL_CONSTASCII_USTRINGPARAM(""));
    ::rtl::OUString str2(RTL_CONSTASCII_USTRINGPARAM("a-b--c---"));

    ::rtl::OUStringBuffer str3Buf;
    str3Buf.append(SVT_SOFT_HYPHEN);
    str3Buf.append(SVT_HARD_HYPHEN);
    str3Buf.append(SVT_HARD_HYPHEN);
    ::rtl::OUString str3(str3Buf.makeStringAndClear());

    ::rtl::OUString str4(RTL_CONSTASCII_USTRINGPARAM("asdf"));

    bool bModified = linguistic::RemoveHyphens(str1);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str1.isEmpty());

    // Note that '-' isn't a hyphen to RemoveHyphens.
    bModified = linguistic::RemoveHyphens(str2);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str2.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("a-b--c---")));

    bModified = linguistic::RemoveHyphens(str3);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT(str3.isEmpty());

    bModified = linguistic::RemoveHyphens(str4);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str4.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("asdf")));
  }

  void LngMiscTest::testRemoveControlChars()
  {
    ::rtl::OUString str1(RTL_CONSTASCII_USTRINGPARAM(""));
    ::rtl::OUString str2(RTL_CONSTASCII_USTRINGPARAM("asdf"));
    ::rtl::OUString str3(RTL_CONSTASCII_USTRINGPARAM("asdf\nasdf"));

    ::rtl::OUStringBuffer str4Buf(33);
    str4Buf.setLength(33);
    for(int i = 0; i < 33; i++)
      str4Buf[i] = static_cast<sal_Unicode>(i);
    //    TODO: is this a bug? shouldn't RemoveControlChars remove this?
    //    str4Buf[33] = static_cast<sal_Unicode>(0x7F);
    ::rtl::OUString str4(str4Buf.makeStringAndClear());

    bool bModified = linguistic::RemoveControlChars(str1);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str1.isEmpty());

    bModified = linguistic::RemoveControlChars(str2);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str2.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("asdf")));

    bModified = linguistic::RemoveControlChars(str3);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT(str3.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("asdfasdf")));

    bModified = linguistic::RemoveControlChars(str4);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT(str4.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(" ")));
  }

  void LngMiscTest::testReplaceControlChars()
  {
    ::rtl::OUString str1(RTL_CONSTASCII_USTRINGPARAM(""));
    ::rtl::OUString str2(RTL_CONSTASCII_USTRINGPARAM("asdf"));
    ::rtl::OUString str3(RTL_CONSTASCII_USTRINGPARAM("asdf\nasdf"));

    ::rtl::OUStringBuffer str4Buf(33);
    str4Buf.setLength(33);
    for(int i = 0; i < 33; i++)
      str4Buf[i] = static_cast<sal_Unicode>(i);
    //    TODO: is this a bug? shouldn't RemoveControlChars remove this?
    //    str4Buf[33] = static_cast<sal_Unicode>(0x7F);
    ::rtl::OUString str4(str4Buf.makeStringAndClear());

    bool bModified = linguistic::ReplaceControlChars(str1);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str1.isEmpty());

    bModified = linguistic::ReplaceControlChars(str2);
    CPPUNIT_ASSERT(!bModified);
    CPPUNIT_ASSERT(str2.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("asdf")));

    bModified = linguistic::ReplaceControlChars(str3);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT(str3.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("asdf asdf")));

    bModified = linguistic::ReplaceControlChars(str4);
    CPPUNIT_ASSERT(bModified);
    CPPUNIT_ASSERT(str4.getLength() == 32);
    for(int i = 0; i < 32; i++)
      CPPUNIT_ASSERT(str4[i] == ' ');
  }

  void LngMiscTest::testGetThesaurusReplaceText()
  {
    const static ::rtl::OUString str1(RTL_CONSTASCII_USTRINGPARAM(""));
    const static ::rtl::OUString str2(RTL_CONSTASCII_USTRINGPARAM("asdf"));
    const static ::rtl::OUString str3(RTL_CONSTASCII_USTRINGPARAM("asdf (abc)"));
    const static ::rtl::OUString str4(RTL_CONSTASCII_USTRINGPARAM("asdf*"));
    const static ::rtl::OUString str5(RTL_CONSTASCII_USTRINGPARAM("asdf * "));
    const static ::rtl::OUString str6(RTL_CONSTASCII_USTRINGPARAM("asdf (abc) *"));
    const static ::rtl::OUString str7(RTL_CONSTASCII_USTRINGPARAM("asdf asdf * (abc)"));
    const static ::rtl::OUString str8(RTL_CONSTASCII_USTRINGPARAM(" * (abc) asdf *"));

    ::rtl::OUString r = linguistic::GetThesaurusReplaceText(str1);
    CPPUNIT_ASSERT(r.isEmpty());

    r = linguistic::GetThesaurusReplaceText(str2);
    CPPUNIT_ASSERT(r == str2);

    r = linguistic::GetThesaurusReplaceText(str3);
    CPPUNIT_ASSERT(r == str2);

    r = linguistic::GetThesaurusReplaceText(str4);
    CPPUNIT_ASSERT(r == str2);

    r = linguistic::GetThesaurusReplaceText(str5);
    CPPUNIT_ASSERT(r == str2);

    r = linguistic::GetThesaurusReplaceText(str6);
    CPPUNIT_ASSERT(r == str2);

    r = linguistic::GetThesaurusReplaceText(str7);
    CPPUNIT_ASSERT(r == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("asdf asdf")));

    r = linguistic::GetThesaurusReplaceText(str8);
    CPPUNIT_ASSERT(r.isEmpty());
  }

  CPPUNIT_TEST_SUITE_REGISTRATION(LngMiscTest);
}
CPPUNIT_PLUGIN_IMPLEMENT();
