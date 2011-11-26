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
    //    void testRemoveControlChars();
    //    void testReplaceControlChars();
    //    void testGetThesaurusReplaceText();

    CPPUNIT_TEST_SUITE(LngMiscTest);

    CPPUNIT_TEST(testRemoveHyphens);
    //    CPPUNIT_TEST(testRemoveControlChars);
    //    CPPUNIT_TEST(testReplaceControlChars);
    //    CPPUNIT_TEST(testGetThesaurusReplaceText);

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

  /*
  void LngMiscTest::testRemoveControlChars()
  {
    CPPUNIT_ASSERT(true);
  }

  void LngMiscTest::testReplaceControlChars()
  {
    CPPUNIT_ASSERT(true);
  }

  void LngMiscTest::testGetThesaurusReplaceText()
  {
    CPPUNIT_ASSERT(true);
  }
  */

  CPPUNIT_TEST_SUITE_REGISTRATION(LngMiscTest);
}
CPPUNIT_PLUGIN_IMPLEMENT();
