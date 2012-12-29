/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

#include "osl/file.hxx"
#include "osl/process.h"

#include "scanner.hxx"

namespace
{
  struct Symbol
  {
    sal_uInt16 line;
    sal_uInt16 col1;
    sal_uInt16 col2;
    OUString text;
    double number;
    SbxDataType type;
  };

  /**
   * Perform tests on Scanner.
   */
  class ScannerTest : public CppUnit::TestFixture
  {
  private:
    void testBlankLines();
    void testOperators();
    void testAlphanum();
    void testComments();
    void testGoto();
    void testGotoCompatible();
    void testExclamation();
    void testNumbers();
    void testDataType();
    void testHexOctal();

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(ScannerTest);

    // Declares the method as a test to call
    CPPUNIT_TEST(testBlankLines);
    CPPUNIT_TEST(testOperators);
    CPPUNIT_TEST(testAlphanum);
    CPPUNIT_TEST(testComments);
    CPPUNIT_TEST(testGoto);
    CPPUNIT_TEST(testGotoCompatible);
    CPPUNIT_TEST(testExclamation);
    CPPUNIT_TEST(testNumbers);
    CPPUNIT_TEST(testDataType);
    CPPUNIT_TEST(testHexOctal);

    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();
  };

  const static OUString cr("\n");
  const static OUString rem("REM");
  const static OUString asdf("asdf");
  const static OUString dot(".");
  const static OUString goto_("goto");
  const static OUString excl("!");

  std::vector<Symbol> getSymbols(const OUString& source, sal_Int32& errors, bool bCompatible = false)
  {
    std::vector<Symbol> symbols;
    SbiScanner scanner(source);
    scanner.EnableErrors();
    scanner.SetCompatible(bCompatible);
    while(scanner.NextSym())
    {
      Symbol symbol;
      symbol.line = scanner.GetLine();
      symbol.col1 = scanner.GetCol1();
      symbol.col2 = scanner.GetCol2();
      symbol.text = scanner.GetSym();
      symbol.number = scanner.GetDbl();
      symbol.type = scanner.GetType();
      symbols.push_back(symbol);
    }
    errors = scanner.GetErrors();
    return symbols;
  }

  std::vector<Symbol> getSymbols(const OUString& source, bool bCompatible = false)
  {
    sal_Int32 i;
    return getSymbols(source, i, bCompatible);
  }

  void ScannerTest::testBlankLines()
  {
    const OUString source1("");
    const OUString source2("\r\n");
    const OUString source3("\n");
    const OUString source4("\r");
    const OUString source5("\r\n\r\n");
    const OUString source6("\n\r");
    const OUString source7("\n\r\n");
    const OUString source8("\r\n\r");
    const OUString source9("      ");

    std::vector<Symbol> symbols;
    symbols = getSymbols(source1);
    CPPUNIT_ASSERT(symbols.empty());

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT(symbols.size() == 1);
    CPPUNIT_ASSERT(symbols[0].text == cr);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT(symbols.size() == 1);
    CPPUNIT_ASSERT(symbols[0].text == cr);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);

    symbols = getSymbols(source4);
    CPPUNIT_ASSERT(symbols.size() == 1);
    CPPUNIT_ASSERT(symbols[0].text == cr);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);

    symbols = getSymbols(source5);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == cr);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source6);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == cr);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source7);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == cr);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source8);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == cr);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source9);
    CPPUNIT_ASSERT(symbols.size() == 1);
    CPPUNIT_ASSERT(symbols[0].text == cr);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
  }

  void ScannerTest::testOperators()
  {
    const OUString sourceE("=");
    const OUString sourceLT("<");
    const OUString sourceGT(">");
    const OUString sourceLTE("<=");
    const OUString sourceGTE(">=");
    const OUString sourceEE("==");
    const OUString sourceNE("<>");
    const OUString sourceA(":=");

    std::vector<Symbol> symbols;

    symbols = getSymbols(sourceE);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == sourceE);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(sourceLT);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == sourceLT);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(sourceGT);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == sourceGT);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(sourceLTE);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == sourceLTE);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(sourceGTE);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == sourceGTE);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(sourceEE);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text == sourceE);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == sourceE);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[2].text == cr);
    CPPUNIT_ASSERT(symbols[2].type == SbxVARIANT);

    symbols = getSymbols(sourceNE);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == sourceNE);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(sourceA);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == sourceA);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);
  }

  void ScannerTest::testAlphanum()
  {
    const OUString source1("asdfghefg");
    const OUString source2("1asfdasfd");
    const OUString source3("AdfsaAUdsl10987");
    const OUString source4("asdfa_mnvcnm");
    const OUString source5("_asdf1");
    const OUString source6("_6");
    const OUString source7("joxclk_");
    const OUString source8("   asdf    ");
    const OUString source9(" 19395  asdfa ");
    const OUString source10("\n1\n2\na sdf");
    const OUString source11("asdf.asdf");
    const OUString source12("..");

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == source1);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text.isEmpty()); // Can't start symbol with a digit
    CPPUNIT_ASSERT(symbols[0].number == 1);
    CPPUNIT_ASSERT(symbols[0].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[1].text == OUString("asfdasfd"));
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[2].text == cr);
    CPPUNIT_ASSERT(symbols[2].type == SbxVARIANT);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == source3);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source4);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == source4);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source5);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == source5);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source6);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == source6);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source7);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("joxclk_")));
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(source7 == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("joxclk "))); // Change the trailing '_' to a ' '
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source8);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("asdf")));
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source9);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text.isEmpty());
    CPPUNIT_ASSERT(symbols[0].number = 19395);
    CPPUNIT_ASSERT(symbols[0].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[1].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("asdfa")));
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[2].text == cr);
    CPPUNIT_ASSERT(symbols[2].type == SbxVARIANT);

    symbols = getSymbols(source10);
    CPPUNIT_ASSERT(symbols.size() == 8);
    CPPUNIT_ASSERT(symbols[0].text == cr);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text.isEmpty());
    CPPUNIT_ASSERT(symbols[1].number == 1);
    CPPUNIT_ASSERT(symbols[1].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[2].text == cr);
    CPPUNIT_ASSERT(symbols[2].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[3].text.isEmpty());
    CPPUNIT_ASSERT(symbols[3].number == 2);
    CPPUNIT_ASSERT(symbols[3].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[4].text == cr);
    CPPUNIT_ASSERT(symbols[4].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[5].text.getLength() == 1);
    CPPUNIT_ASSERT(symbols[5].text[0] == 'a');
    CPPUNIT_ASSERT(symbols[5].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[6].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdf")));
    CPPUNIT_ASSERT(symbols[6].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[7].text == cr);
    CPPUNIT_ASSERT(symbols[7].type == SbxVARIANT);

    symbols = getSymbols(source11);
    CPPUNIT_ASSERT(symbols.size() == 4);
    CPPUNIT_ASSERT(symbols[0].text == asdf);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == dot);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[2].text == asdf);
    CPPUNIT_ASSERT(symbols[2].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[3].text == cr);
    CPPUNIT_ASSERT(symbols[3].type == SbxVARIANT);

    symbols = getSymbols(source12);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text == dot);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == dot);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[2].text == cr);
    CPPUNIT_ASSERT(symbols[2].type == SbxVARIANT);
  }

  void ScannerTest::testComments()
  {
    const rtl::OUString source1(RTL_CONSTASCII_USTRINGPARAM("REM asdf"));
    const rtl::OUString source2(RTL_CONSTASCII_USTRINGPARAM("REMasdf"));
    const rtl::OUString source3(RTL_CONSTASCII_USTRINGPARAM("'asdf"));
    const rtl::OUString source4(RTL_CONSTASCII_USTRINGPARAM("asdf _\n'100"));
    const rtl::OUString source5(RTL_CONSTASCII_USTRINGPARAM("'asdf _\n100"));
    const rtl::OUString source6(RTL_CONSTASCII_USTRINGPARAM("'asdf _\n'100"));
    const rtl::OUString source7(RTL_CONSTASCII_USTRINGPARAM("'asdf _\n 1234 _\n asdf'"));

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1);
    CPPUNIT_ASSERT(symbols.size() == 1);
    CPPUNIT_ASSERT(symbols[0].text == rem);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REMasdf")));
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT(symbols.size() == 1);
    CPPUNIT_ASSERT(symbols[0].text == rem);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);

    symbols = getSymbols(source4);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == asdf);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == rem);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source5);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text == rem);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text.isEmpty());
    CPPUNIT_ASSERT(symbols[1].number == 100);
    CPPUNIT_ASSERT(symbols[1].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[2].text == cr);
    CPPUNIT_ASSERT(symbols[2].type == SbxVARIANT);

    symbols = getSymbols(source6);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == rem);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == rem);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source7);
    CPPUNIT_ASSERT(symbols.size() == 4);
    CPPUNIT_ASSERT(symbols[0].text == rem);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text.isEmpty());
    CPPUNIT_ASSERT(symbols[1].number == 1234);
    CPPUNIT_ASSERT(symbols[1].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[2].text == asdf);
    CPPUNIT_ASSERT(symbols[2].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[3].text == rem);
    CPPUNIT_ASSERT(symbols[3].type == SbxVARIANT);
  }

  void ScannerTest::testGoto()
  {
    const rtl::OUString source1(RTL_CONSTASCII_USTRINGPARAM("goto"));
    const rtl::OUString source2(RTL_CONSTASCII_USTRINGPARAM("go  to"));
    const rtl::OUString source3(RTL_CONSTASCII_USTRINGPARAM("go\nto"));

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == goto_);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("go")));
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("to")));
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[2].text == cr);
    CPPUNIT_ASSERT(symbols[2].type == SbxVARIANT);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT(symbols.size() == 4);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("go")));
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[2].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("to")));
    CPPUNIT_ASSERT(symbols[2].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[3].text == cr);
    CPPUNIT_ASSERT(symbols[3].type == SbxVARIANT);
  }

  void ScannerTest::testGotoCompatible()
  {
    const rtl::OUString source1(RTL_CONSTASCII_USTRINGPARAM("goto"));
    const rtl::OUString source2(RTL_CONSTASCII_USTRINGPARAM("go  to"));
    const rtl::OUString source3(RTL_CONSTASCII_USTRINGPARAM("go\nto"));

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1, true);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == goto_);
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source2, true);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString(goto_));
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source3, true);
    CPPUNIT_ASSERT(symbols.size() == 4);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("go")));
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(symbols[2].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("to")));
    CPPUNIT_ASSERT(symbols[3].text == cr);
  }

  void ScannerTest::testExclamation()
  {
    const rtl::OUString source1(RTL_CONSTASCII_USTRINGPARAM("asdf!asdf"));
    const rtl::OUString source2(RTL_CONSTASCII_USTRINGPARAM("!1234"));
    const rtl::OUString source3(RTL_CONSTASCII_USTRINGPARAM("!_3"));
    const rtl::OUString source4(RTL_CONSTASCII_USTRINGPARAM("!$"));
    const rtl::OUString source5(RTL_CONSTASCII_USTRINGPARAM("!%"));
    const rtl::OUString source6(RTL_CONSTASCII_USTRINGPARAM("!\n"));

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1);
    CPPUNIT_ASSERT(symbols.size() == 4);
    CPPUNIT_ASSERT(symbols[0].text == asdf);
    CPPUNIT_ASSERT(symbols[1].text == excl);
    CPPUNIT_ASSERT(symbols[2].text == asdf);
    CPPUNIT_ASSERT(symbols[3].text == cr);

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text == excl);
    CPPUNIT_ASSERT(symbols[1].text.isEmpty());
    CPPUNIT_ASSERT(symbols[1].number == 1234);
    CPPUNIT_ASSERT(symbols[2].text == cr);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text == excl);
    CPPUNIT_ASSERT(symbols[1].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_3")));
    CPPUNIT_ASSERT(symbols[2].text == cr);

    symbols = getSymbols(source4);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text == excl);
    CPPUNIT_ASSERT(symbols[1].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("$")));
    CPPUNIT_ASSERT(symbols[2].text == cr);

    symbols = getSymbols(source5);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text == excl);
    CPPUNIT_ASSERT(symbols[1].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")));
    CPPUNIT_ASSERT(symbols[2].text == cr);

    symbols = getSymbols(source6);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].text == excl);
    CPPUNIT_ASSERT(symbols[1].text == cr);
  }

  void ScannerTest::testNumbers()
  {
    const rtl::OUString source1(RTL_CONSTASCII_USTRINGPARAM("12345"));
    const rtl::OUString source2(RTL_CONSTASCII_USTRINGPARAM("1.2.3"));
    const rtl::OUString source3(RTL_CONSTASCII_USTRINGPARAM("123.4"));
    const rtl::OUString source4(RTL_CONSTASCII_USTRINGPARAM("0.5"));
    const rtl::OUString source5(RTL_CONSTASCII_USTRINGPARAM("5.0"));
    const rtl::OUString source6(RTL_CONSTASCII_USTRINGPARAM("0.0"));
    const rtl::OUString source7(RTL_CONSTASCII_USTRINGPARAM("-3"));
    const rtl::OUString source8(RTL_CONSTASCII_USTRINGPARAM("-0.0"));
    const rtl::OUString source9(RTL_CONSTASCII_USTRINGPARAM("12dE3"));
    const rtl::OUString source10(RTL_CONSTASCII_USTRINGPARAM("12e3"));
    const rtl::OUString source11(RTL_CONSTASCII_USTRINGPARAM("12D+3"));
    const rtl::OUString source12(RTL_CONSTASCII_USTRINGPARAM("12e++3"));
    const rtl::OUString source13(RTL_CONSTASCII_USTRINGPARAM("12e-3"));
    const rtl::OUString source14(RTL_CONSTASCII_USTRINGPARAM("12e-3+"));
    const rtl::OUString source15(RTL_CONSTASCII_USTRINGPARAM("1,2,3"));
    const rtl::OUString source16(RTL_CONSTASCII_USTRINGPARAM("1.0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"));

    std::vector<Symbol> symbols;
    sal_Int32 errors;

    symbols = getSymbols(source1, errors);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 12345);
    CPPUNIT_ASSERT(symbols[0].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source2, errors);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 1.23);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(errors == 1);

    symbols = getSymbols(source3, errors);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 123.4);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source4, errors);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == .5);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source5, errors);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 5);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source6, errors);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 0);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source7, errors);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-")));
    CPPUNIT_ASSERT(symbols[1].number == 3);
    CPPUNIT_ASSERT(symbols[1].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[2].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source8, errors);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-")));
    CPPUNIT_ASSERT(symbols[1].number == 0);
    CPPUNIT_ASSERT(symbols[1].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[2].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source9, errors);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 12000);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(errors == 1);

    symbols = getSymbols(source10, errors);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 12000);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source11, errors);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 12000);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source12, errors);
    CPPUNIT_ASSERT(symbols.size() == 4);
    CPPUNIT_ASSERT(symbols[0].number == 12);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("+")));
    CPPUNIT_ASSERT(symbols[2].number == 3);
    CPPUNIT_ASSERT(symbols[2].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[3].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source13, errors);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == .012);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source14, errors);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].number == .012);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("+")));
    CPPUNIT_ASSERT(symbols[2].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source15, errors);
    CPPUNIT_ASSERT(symbols.size() == 6);
    CPPUNIT_ASSERT(symbols[0].number == 1);
    CPPUNIT_ASSERT(symbols[0].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[1].text == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(",")));
    CPPUNIT_ASSERT(symbols[2].number == 2);
    CPPUNIT_ASSERT(symbols[2].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[3].text == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(",")));
    CPPUNIT_ASSERT(symbols[4].number == 3);
    CPPUNIT_ASSERT(symbols[4].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[5].text == cr);
    CPPUNIT_ASSERT(errors == 0);

    symbols = getSymbols(source16, errors);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 1);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == cr);
    // This error is from a "buffer overflow" which is stupid because
    // the buffer is artificially constrained by the scanner.
    CPPUNIT_ASSERT(errors == 1); // HACK
  }

  void ScannerTest::testDataType()
  {
    const rtl::OUString source1(RTL_CONSTASCII_USTRINGPARAM("asdf%"));
    const rtl::OUString source2(RTL_CONSTASCII_USTRINGPARAM("asdf&"));
    const rtl::OUString source3(RTL_CONSTASCII_USTRINGPARAM("asdf!"));
    const rtl::OUString source4(RTL_CONSTASCII_USTRINGPARAM("asdf#"));
    const rtl::OUString source5(RTL_CONSTASCII_USTRINGPARAM("asdf@"));
    const rtl::OUString source6(RTL_CONSTASCII_USTRINGPARAM("asdf$"));
    const rtl::OUString source7(RTL_CONSTASCII_USTRINGPARAM("asdf "));

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].type == SbxLONG);
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].type == SbxSINGLE);
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source4);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source5);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].type == SbxCURRENCY);
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source6);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].type == SbxSTRING);
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source7);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].text == cr);
  }

  void ScannerTest::testHexOctal()
  {
    const rtl::OUString source1(RTL_CONSTASCII_USTRINGPARAM("&HA"));
    const rtl::OUString source2(RTL_CONSTASCII_USTRINGPARAM("&HASDF"));
    const rtl::OUString source3(RTL_CONSTASCII_USTRINGPARAM("&H10"));
    const rtl::OUString source4(RTL_CONSTASCII_USTRINGPARAM("&&H&1H1&H1"));
    const rtl::OUString source5(RTL_CONSTASCII_USTRINGPARAM("&O&O12"));
    const rtl::OUString source6(RTL_CONSTASCII_USTRINGPARAM("&O10"));
    const rtl::OUString source7(RTL_CONSTASCII_USTRINGPARAM("&HO"));
    const rtl::OUString source8(RTL_CONSTASCII_USTRINGPARAM("&O123000000000000000000000"));
    const rtl::OUString source9(RTL_CONSTASCII_USTRINGPARAM("&H1.23"));

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 10);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString());
    CPPUNIT_ASSERT(symbols[0].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 2783);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString());
    CPPUNIT_ASSERT(symbols[0].type = SbxINTEGER);
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 16);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString());
    CPPUNIT_ASSERT(symbols[0].type = SbxINTEGER);
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source4);
    CPPUNIT_ASSERT(symbols.size() == 6);
    CPPUNIT_ASSERT(symbols[0].number == 0);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("&")));
    CPPUNIT_ASSERT(symbols[0].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[1].number == 0);
    CPPUNIT_ASSERT(symbols[1].text == rtl::OUString());
    CPPUNIT_ASSERT(symbols[1].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[2].number == 1);
    CPPUNIT_ASSERT(symbols[2].text == rtl::OUString());
    CPPUNIT_ASSERT(symbols[2].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[3].number == 1);
    CPPUNIT_ASSERT(symbols[3].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("H1")));
    CPPUNIT_ASSERT(symbols[3].type == SbxLONG);
    CPPUNIT_ASSERT(symbols[4].number == 1);
    CPPUNIT_ASSERT(symbols[4].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("H1")));
    CPPUNIT_ASSERT(symbols[4].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[5].text == cr);

    symbols = getSymbols(source5);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].number == 0);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString());
    CPPUNIT_ASSERT(symbols[0].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[1].number == 0);
    CPPUNIT_ASSERT(symbols[1].text == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("O12")));
    CPPUNIT_ASSERT(symbols[1].type == SbxVARIANT);
    CPPUNIT_ASSERT(symbols[2].text == cr);

    symbols = getSymbols(source6);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 8);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString());
    CPPUNIT_ASSERT(symbols[0].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source7);
    CPPUNIT_ASSERT(symbols.size() == 2);
    CPPUNIT_ASSERT(symbols[0].number == 0);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString());
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source8);
    CPPUNIT_ASSERT(symbols.size() == 2);
    // TODO: this line fails on 64 bit systems!!!
    //    CPPUNIT_ASSERT(symbols[0].number == -1744830464);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString());
    CPPUNIT_ASSERT(symbols[1].text == cr);

    symbols = getSymbols(source9);
    CPPUNIT_ASSERT(symbols.size() == 3);
    CPPUNIT_ASSERT(symbols[0].text == rtl::OUString());
    CPPUNIT_ASSERT(symbols[0].number == 1);
    CPPUNIT_ASSERT(symbols[0].type == SbxINTEGER);
    CPPUNIT_ASSERT(symbols[1].number == .23);
    CPPUNIT_ASSERT(symbols[1].text == rtl::OUString());
    CPPUNIT_ASSERT(symbols[1].type == SbxDOUBLE);
    CPPUNIT_ASSERT(symbols[2].text == cr);
  }

  // Put the test suite in the registry
  CPPUNIT_TEST_SUITE_REGISTRATION(ScannerTest);
} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
