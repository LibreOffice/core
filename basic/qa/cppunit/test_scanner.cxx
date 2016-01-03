/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/math.hxx>

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

  static const OUString cr = "\n";
  static const OUString rem = "REM";
  static const OUString asdf = "asdf";
  static const OUString dot = ".";
  static const OUString goto_ = "goto";
  static const OUString excl = "!";

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
    CPPUNIT_ASSERT_EQUAL(size_t(1), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT_EQUAL(size_t(1), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);

    symbols = getSymbols(source4);
    CPPUNIT_ASSERT_EQUAL(size_t(1), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);

    symbols = getSymbols(source5);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source6);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source7);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source8);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source9);
    CPPUNIT_ASSERT_EQUAL(size_t(1), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
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
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(sourceE, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(sourceLT);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(sourceLT, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(sourceGT);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(sourceGT, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(sourceLTE);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(sourceLTE, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(sourceGTE);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(sourceGTE, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(sourceEE);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(sourceE, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(sourceE, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);

    symbols = getSymbols(sourceNE);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(sourceNE, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(sourceA);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(sourceA, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
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
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(source1, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT(symbols[0].text.isEmpty()); // Can't start symbol with a digit
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)(symbols[0].number));
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"asfdasfd", symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(source3, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source4);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(source4, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source5);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(source5, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source6);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(source6, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source7);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"joxclk_", symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"joxclk ", source7); // Change the trailing '_' to a ' '
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source8);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"asdf", symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source9);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT(symbols[0].text.isEmpty());
    CPPUNIT_ASSERT_EQUAL(19395u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"asdfa", symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);

    symbols = getSymbols(source10);
    CPPUNIT_ASSERT_EQUAL(size_t(8), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT(symbols[1].text.isEmpty());
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)symbols[1].number);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);
    CPPUNIT_ASSERT(symbols[3].text.isEmpty());
    CPPUNIT_ASSERT_EQUAL(2u, (unsigned int)symbols[3].number);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[3].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[4].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[4].type);
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)symbols[5].text.getLength());
    CPPUNIT_ASSERT_EQUAL('a', (char)symbols[5].text[0]);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[5].type);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"sdf", symbols[6].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[6].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[7].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[7].type);

    symbols = getSymbols(source11);
    CPPUNIT_ASSERT_EQUAL(size_t(4), symbols.size());
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(dot, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[3].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[3].type);

    symbols = getSymbols(source12);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(dot, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(dot, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);
  }

  void ScannerTest::testComments()
  {
    const OUString source1("REM asdf");
    const OUString source2("REMasdf");
    const OUString source3("'asdf");
    const OUString source4("asdf _\n'100");
    const OUString source5("'asdf _\n100");
    const OUString source6("'asdf _\n'100");
    const OUString source7("'asdf _\n 1234 _\n asdf'");

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1);
    CPPUNIT_ASSERT_EQUAL(size_t(1), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rem, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"REMasdf", symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT_EQUAL(size_t(1), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rem, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);

    symbols = getSymbols(source4);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(rem, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source5);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rem, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT(symbols[1].text.isEmpty());
    CPPUNIT_ASSERT_EQUAL(100u, (unsigned int)symbols[1].number);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);

    symbols = getSymbols(source6);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rem, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(rem, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source7);
    CPPUNIT_ASSERT_EQUAL(size_t(4), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rem, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT(symbols[1].text.isEmpty());
    CPPUNIT_ASSERT_EQUAL(1234u, (unsigned int)symbols[1].number);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);
    CPPUNIT_ASSERT_EQUAL(rem, symbols[3].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[3].type);
  }

  void ScannerTest::testGoto()
  {
    const OUString source1("goto");
    const OUString source2("go  to");
    const OUString source3("go\nto");

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(goto_, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"go", symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"to", symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT_EQUAL(size_t(4), symbols.size());
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"go", symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"to", symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[3].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[3].type);
  }

  void ScannerTest::testGotoCompatible()
  {
    const OUString source1("goto");
    const OUString source2("go  to");
    const OUString source3("go\nto");

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1, true);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(goto_, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source2, true);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(goto_, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source3, true);
    CPPUNIT_ASSERT_EQUAL(size_t(4), symbols.size());
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"go", symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"to", symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[3].text);
  }

  void ScannerTest::testExclamation()
  {
    const OUString source1("asdf!asdf");
    const OUString source2("!1234");
    const OUString source3("!_3");
    const OUString source4("!$");
    const OUString source5("!%");
    const OUString source6("!\n");

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1);
    CPPUNIT_ASSERT_EQUAL(size_t(4), symbols.size());
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(excl, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[3].text);

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(excl, symbols[0].text);
    CPPUNIT_ASSERT(symbols[1].text.isEmpty());
    CPPUNIT_ASSERT_EQUAL(1234u, (unsigned int)symbols[1].number);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(excl, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"_3", symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);

    symbols = getSymbols(source4);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(excl, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"$", symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);

    symbols = getSymbols(source5);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(excl, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"%", symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);

    symbols = getSymbols(source6);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(excl, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
  }

  void ScannerTest::testNumbers()
  {
    const OUString source1("12345");
    const OUString source2("1.2.3");
    const OUString source3("123.4");
    const OUString source4("0.5");
    const OUString source5("5.0");
    const OUString source6("0.0");
    const OUString source7("-3");
    const OUString source8("-0.0");
    const OUString source9("12dE3");
    const OUString source10("12e3");
    const OUString source11("12D+3");
    const OUString source12("12e++3");
    const OUString source13("12e-3");
    const OUString source14("12e-3+");
    const OUString source15("1,2,3");
    const OUString source16("1.0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    const OUString source17("10e308");

    std::vector<Symbol> symbols;
    sal_Int32 errors;

    symbols = getSymbols(source1, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(12345u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)errors);

    symbols = getSymbols(source2, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rtl::math::round(1.2, 12), rtl::math::round(symbols[0].number, 12));
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(rtl::math::round( .3, 12), rtl::math::round( symbols[1].number, 12));
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)errors);

    symbols = getSymbols(source3, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rtl::math::round(123.4, 12), rtl::math::round( symbols[0].number, 12));
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)errors);

    symbols = getSymbols(source4, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rtl::math::round( .5, 12), rtl::math::round( symbols[0].number, 12));
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)errors);

    symbols = getSymbols(source5, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(5u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)errors);

    symbols = getSymbols(source6, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)errors);

    symbols = getSymbols(source7, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"-", symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(3u, (unsigned int)symbols[1].number);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)errors);

    symbols = getSymbols(source8, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"-", symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)symbols[1].number);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)errors);

    symbols = getSymbols(source9, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(12u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"dE3", symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)errors);

    symbols = getSymbols(source10, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(12000u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)errors);

    symbols = getSymbols(source11, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(12000u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)errors);

    symbols = getSymbols(source12, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(6), symbols.size());
    CPPUNIT_ASSERT_EQUAL(12u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"e", symbols[1].text);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"+", symbols[2].text);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"+", symbols[3].text);
    CPPUNIT_ASSERT_EQUAL(3u, (unsigned int)symbols[4].number);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[4].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[5].text);
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)errors);

    symbols = getSymbols(source13, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rtl::math::round( .012, 12), rtl::math::round( symbols[0].number, 12));
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)errors);

    symbols = getSymbols(source14, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rtl::math::round( .012, 12), rtl::math::round( symbols[0].number, 12));
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"+", symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)errors);

    symbols = getSymbols(source15, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(6), symbols.size());
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)",", symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(2u, (unsigned int)symbols[2].number);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[2].type);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)",", symbols[3].text);
    CPPUNIT_ASSERT_EQUAL(3u, (unsigned int)symbols[4].number);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[4].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[5].text);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)errors);

    symbols = getSymbols(source16, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    // This error is from a "buffer overflow" which is stupid because
    // the buffer is artificially constrained by the scanner.
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)errors); // HACK

    double fInf = 0.0;
    rtl::math::setInf( &fInf, false);
    symbols = getSymbols(source17, errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rtl::math::round( fInf, 12), rtl::math::round( symbols[0].number, 12));
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)errors);    // math error, overflow
  }

  void ScannerTest::testDataType()
  {
    const OUString source1("asdf%");
    const OUString source2("asdf&");
    const OUString source3("asdf!");
    const OUString source4("asdf#");
    const OUString source5("asdf@");
    const OUString source6("asdf$");
    const OUString source7("asdf ");

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxSINGLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source4);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source5);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxCURRENCY, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source6);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxSTRING, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source7);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
  }

  void ScannerTest::testHexOctal()
  {
    const OUString source1("&HA");
    const OUString source2("&HASDF");
    const OUString source3("&H10");
    const OUString source4("&&H&1H1&H1");
    const OUString source5("&O&O12");
    const OUString source6("&O10");
    const OUString source7("&HO");
    const OUString source8("&O123000000000000000000000");
    const OUString source9("&H1.23");

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(10u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source2);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(2783u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source3);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(16u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source4);
    CPPUNIT_ASSERT_EQUAL(size_t(6), symbols.size());
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"&", symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)symbols[1].number);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)OUString(), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)symbols[2].number);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)OUString(), symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[2].type);
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)symbols[3].number);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"H1", symbols[3].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[3].type);
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)symbols[4].number);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"H1", symbols[4].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[4].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[5].text);

    symbols = getSymbols(source5);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)symbols[1].number);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)"O12", symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);

    symbols = getSymbols(source6);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(8u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source7);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(0u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source8);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    // TODO: this line fails on 64 bit systems!!!
    //    CPPUNIT_ASSERT_EQUAL(symbols[0].number, -1744830464);
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols(source9);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(1u, (unsigned int)symbols[0].number);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(rtl::math::round( .23, 12), rtl::math::round( symbols[1].number, 12));
    CPPUNIT_ASSERT_EQUAL((rtl::OUString)OUString(), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
  }

  // Put the test suite in the registry
  CPPUNIT_TEST_SUITE_REGISTRATION(ScannerTest);
} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
