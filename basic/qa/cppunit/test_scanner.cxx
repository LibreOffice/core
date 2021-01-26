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

#include <rtl/math.hxx>

#include <scanner.hxx>

namespace
{
struct Symbol
{
    sal_uInt16 line;
    sal_uInt16 col1;
    OUString text;
    double number;
    SbxDataType type;
    bool ws;
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
    void testTdf103104();
    void testTdf136032();

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
    CPPUNIT_TEST(testTdf103104);
    CPPUNIT_TEST(testTdf136032);

    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();
};

const OUString cr = "\n";
const OUString rem = "REM";
const OUString asdf = "asdf";
const OUString dot = ".";
const OUString goto_ = "goto";
const OUString excl = "!";

std::vector<Symbol> getSymbols(const OUString& source, sal_Int32& errors, bool bCompatible = false)
{
    std::vector<Symbol> symbols;
    SbiScanner scanner(source);
    scanner.EnableErrors();
    scanner.SetCompatible(bCompatible);
    while (scanner.NextSym())
    {
        Symbol symbol;
        symbol.line = scanner.GetLine();
        symbol.col1 = scanner.GetCol1();
        symbol.text = scanner.GetSym();
        symbol.number = scanner.GetDbl();
        symbol.type = scanner.GetType();
        symbol.ws = scanner.WhiteSpace();
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
    std::vector<Symbol> symbols;
    symbols = getSymbols("");
    CPPUNIT_ASSERT(symbols.empty());

    symbols = getSymbols("\r\n");
    CPPUNIT_ASSERT_EQUAL(size_t(1), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);

    symbols = getSymbols("\n");
    CPPUNIT_ASSERT_EQUAL(size_t(1), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);

    symbols = getSymbols("\r");
    CPPUNIT_ASSERT_EQUAL(size_t(1), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);

    symbols = getSymbols("\r\n\r\n");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols("\n\r");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols("\n\r\n");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols("\r\n\r");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols("      ");
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
    const OUString sourceNE("<>");
    const OUString sourceA(":=");
    const OUString sourceNot("Not");

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

    symbols = getSymbols("==");
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

    symbols = getSymbols(sourceNot);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(sourceNot, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
}

void ScannerTest::testAlphanum()
{
    const OUString source1("asdfghefg");
    const OUString source3("AdfsaAUdsl10987");
    const OUString source4("asdfa_mnvcnm");
    const OUString source5("_asdf1");
    const OUString source6("_6");
    const OUString source7("joxclk_");

    std::vector<Symbol> symbols;

    symbols = getSymbols(source1);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(source1, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols("1asfdasfd");
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT(symbols[0].text.isEmpty()); // Can't start symbol with a digit
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(OUString("asfdasfd"), symbols[1].text);
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
    CPPUNIT_ASSERT_EQUAL(OUString("joxclk_"), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(OUString("joxclk "), source7); // Change the trailing '_' to a ' '
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols("   asdf    ");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(OUString("asdf"), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols(" 19395  asdfa ");
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT(symbols[0].text.isEmpty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(19395.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(OUString("asdfa"), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);

    symbols = getSymbols("\n1\n2\na sdf");
    CPPUNIT_ASSERT_EQUAL(size_t(8), symbols.size());
    CPPUNIT_ASSERT_EQUAL(cr, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT(symbols[1].text.isEmpty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, symbols[1].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);
    CPPUNIT_ASSERT(symbols[3].text.isEmpty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, symbols[3].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[3].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[4].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[4].type);
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(symbols[5].text.getLength()));
    CPPUNIT_ASSERT_EQUAL('a', static_cast<char>(symbols[5].text[0]));
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[5].type);
    CPPUNIT_ASSERT_EQUAL(OUString("sdf"), symbols[6].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[6].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[7].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[7].type);

    symbols = getSymbols("asdf.asdf");
    CPPUNIT_ASSERT_EQUAL(size_t(4), symbols.size());
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(dot, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[3].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[3].type);

    symbols = getSymbols("..");
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
    std::vector<Symbol> symbols;

    symbols = getSymbols("REM asdf");
    CPPUNIT_ASSERT_EQUAL(size_t(1), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rem, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);

    symbols = getSymbols("REMasdf");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(OUString("REMasdf"), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols("'asdf");
    CPPUNIT_ASSERT_EQUAL(size_t(1), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rem, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);

    symbols = getSymbols("asdf _\n'100");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(rem, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols("'asdf _\n100");
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rem, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT(symbols[1].text.isEmpty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, symbols[1].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);

    symbols = getSymbols("'asdf _\n'100");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rem, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(rem, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols("'asdf _\n 1234 _\n asdf'");
    CPPUNIT_ASSERT_EQUAL(size_t(4), symbols.size());
    CPPUNIT_ASSERT_EQUAL(rem, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT(symbols[1].text.isEmpty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1234.0, symbols[1].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);
    CPPUNIT_ASSERT_EQUAL(rem, symbols[3].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[3].type);
}

void ScannerTest::testGoto()
{
    std::vector<Symbol> symbols;

    symbols = getSymbols("goto");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(goto_, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);

    symbols = getSymbols("go  to");
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(OUString("go"), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(OUString("to"), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);

    symbols = getSymbols("go\nto");
    CPPUNIT_ASSERT_EQUAL(size_t(4), symbols.size());
    CPPUNIT_ASSERT_EQUAL(OUString("go"), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(OUString("to"), symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[2].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[3].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[3].type);
}

void ScannerTest::testGotoCompatible()
{
    std::vector<Symbol> symbols;

    symbols = getSymbols("goto", true);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(goto_, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("go  to", true);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(goto_, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("go\nto", true);
    CPPUNIT_ASSERT_EQUAL(size_t(4), symbols.size());
    CPPUNIT_ASSERT_EQUAL(OUString("go"), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(OUString("to"), symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[3].text);
}

void ScannerTest::testExclamation()
{
    std::vector<Symbol> symbols;

    symbols = getSymbols("asdf!asdf");
    CPPUNIT_ASSERT_EQUAL(size_t(4), symbols.size());
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(excl, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[3].text);

    symbols = getSymbols("!1234");
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(excl, symbols[0].text);
    CPPUNIT_ASSERT(symbols[1].text.isEmpty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1234.0, symbols[1].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);

    symbols = getSymbols("!_3");
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(excl, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(OUString("_3"), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);

    symbols = getSymbols("!$");
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(excl, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(OUString("$"), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);

    symbols = getSymbols("!%");
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(excl, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(OUString("%"), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);

    symbols = getSymbols("!\n");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(excl, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
}

void ScannerTest::testNumbers()
{
    std::vector<Symbol> symbols;
    sal_Int32 errors;

    symbols = getSymbols("12345", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12345.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));

    symbols = getSymbols("1.2.3", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.2, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(.3, symbols[1].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(errors));

    symbols = getSymbols("123.4", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(123.4, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));

    symbols = getSymbols("0.5", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(.5, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));

    symbols = getSymbols("5.0", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));

    symbols = getSymbols("0.0", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));

    symbols = getSymbols("-3", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(OUString("-"), symbols[0].text);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, symbols[1].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));

    symbols = getSymbols("-0.0", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(OUString("-"), symbols[0].text);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[1].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));

    symbols = getSymbols("12dE3", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(OUString("dE3"), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(errors));

    symbols = getSymbols("12e3", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12000.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));

    symbols = getSymbols("12D+3", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12000.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));

    symbols = getSymbols("12e++3", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(6), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(OUString("e"), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(OUString("+"), symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(OUString("+"), symbols[3].text);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, symbols[4].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[4].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[5].text);
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(errors));

    symbols = getSymbols("12e-3", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(.012, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));

    symbols = getSymbols("12e-3+", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(.012, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(OUString("+"), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));

    symbols = getSymbols("1,2,3", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(6), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(OUString(","), symbols[1].text);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, symbols[2].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[2].type);
    CPPUNIT_ASSERT_EQUAL(OUString(","), symbols[3].text);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, symbols[4].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[4].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[5].text);
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));

    symbols = getSymbols("1."
                         "0000000000000000000000000000000000000000000000000000000000000000000000000"
                         "000000000000000000000000000000000000000000000000000",
                         errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    // This error is from a "buffer overflow" which is stupid because
    // the buffer is artificially constrained by the scanner.
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(errors)); // HACK

    double fInf = 0.0;
    rtl::math::setInf(&fInf, false);
    symbols = getSymbols("10e308", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(fInf, symbols[0].number);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(errors)); // math error, overflow

    // trailing data type character % = SbxINTEGER
    symbols = getSymbols("1.23%");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // trailing data type character & = SbxLONG
    symbols = getSymbols("1.23&");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // trailing data type character ! = SbxSINGLE
    symbols = getSymbols("1.23!");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxSINGLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // trailing data type character # = SbxDOUBLE
    symbols = getSymbols("1.23#");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // trailing data type character @ = SbxCURRENCY
    symbols = getSymbols("1.23@");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxCURRENCY, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // trailing data type character $ = SbxSTRING
    symbols = getSymbols("1.23$", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxSTRING, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    // ERRCODE_BASIC_SYNTAX
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(errors));
}

void ScannerTest::testDataType()
{
    std::vector<Symbol> symbols;

    symbols = getSymbols("asdf%");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("asdf&");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("asdf!");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxSINGLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("asdf#");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("asdf@");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxCURRENCY, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("asdf$");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxSTRING, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("asdf ");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
}

void ScannerTest::testHexOctal()
{
    sal_Int32 errors;
    std::vector<Symbol> symbols;

    symbols = getSymbols("&HA");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("&HASDF");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2783.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("&H10");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("&&H&1H1&H1");
    CPPUNIT_ASSERT_EQUAL(size_t(6), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString("&"), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[0].type);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[1].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[1].type);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, symbols[2].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[2].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[2].type);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, symbols[3].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString("H1"), symbols[3].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[3].type);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, symbols[4].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString("H1"), symbols[4].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[4].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[5].text);

    symbols = getSymbols("&O&O12");
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[1].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString("O12"), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxVARIANT, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);

    symbols = getSymbols("&O10");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("&HO");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("&O123000000000000000000000");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    // TODO: this line fails on 64 bit systems!!!
    //    CPPUNIT_ASSERT_EQUAL(symbols[0].number, -1744830464);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("&H1.23");
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(.23, symbols[1].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[1].text);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[1].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);

    /* tdf#62323, tdf#62326 - conversion of Hex literals to basic signed Integers */

    // &H0 = 0
    symbols = getSymbols("&H0");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // &H8000 = -32768
    symbols = getSymbols("&H8000");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(SbxMININT, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // &H80000000 = -2147483648
    symbols = getSymbols("&H80000000");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(SbxMINLNG, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // &HFFFF = -1
    symbols = getSymbols("&HFFFF");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // &HFFFFFFFF = -1
    symbols = getSymbols("&HFFFFFFFF");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // &H7FFF = 32767
    symbols = getSymbols("&H7FFF");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(SbxMAXINT, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // &H7FFFFFFF = 2147483647
    symbols = getSymbols("&H7FFFFFFF");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(SbxMAXLNG, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    /* tdf#130476 - trailing data type characters */

    // % = SbxINTEGER
    symbols = getSymbols("&H0%");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // & = SbxLONG
    symbols = getSymbols("&H0&");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // ! = SbxSINGLE
    symbols = getSymbols("&H0!");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxSINGLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // # = SbxDOUBLE
    symbols = getSymbols("&H0#");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // @ = SbxCURRENCY
    symbols = getSymbols("&H0@");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxCURRENCY, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // $ = SbxSTRING
    symbols = getSymbols("&H0$", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxSTRING, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    // ERRCODE_BASIC_SYNTAX
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(errors));

    // % = SbxINTEGER
    symbols = getSymbols("&O0%");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // & = SbxLONG
    symbols = getSymbols("&O0&");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // ! = SbxSINGLE
    symbols = getSymbols("&O0!");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxSINGLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // # = SbxDOUBLE
    symbols = getSymbols("&O0#");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxDOUBLE, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // @ = SbxCURRENCY
    symbols = getSymbols("&O0@");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxCURRENCY, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // $ = SbxSTRING
    symbols = getSymbols("&O0$", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxSTRING, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    // ERRCODE_BASIC_SYNTAX
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(errors));

    // maximum for Hex % = SbxINTEGER
    symbols = getSymbols("&HFFFF%");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // overflow for Hex % = SbxINTEGER
    symbols = getSymbols("&H10000%", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    // ERRCODE_BASIC_MATH_OVERFLOW
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(errors));

    // maximum for Octal % = SbxINTEGER
    symbols = getSymbols("&O177777%");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // overflow for Octal % = SbxINTEGER
    symbols = getSymbols("&O200000%", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    // ERRCODE_BASIC_MATH_OVERFLOW
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(errors));

    // maximum for Hex & = SbxLONG
    symbols = getSymbols("&H7FFFFFFF&");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2147483647.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // minimum for Hex & = SbxLONG
    symbols = getSymbols("&H80000000&");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-2147483648.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // overflow for Hex & = SbxLONG
    symbols = getSymbols("&H100000000&", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    // ERRCODE_BASIC_MATH_OVERFLOW
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(errors));

    // maximum for Octal & = SbxLONG
    symbols = getSymbols("&O17777777777&");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2147483647.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // minimum for Octal & = SbxLONG
    symbols = getSymbols("&O20000000000&", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-2147483648.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // overflow for Octal & = SbxLONG
    symbols = getSymbols("&O40000000000&", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
    // ERRCODE_BASIC_MATH_OVERFLOW
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned int>(errors));

    /* test for leading zeros */

    // &H0000000FFFF = 65535
    symbols = getSymbols("&H0000000FFFF");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    // &O00000123 = 83
    symbols = getSymbols("&O00000123");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(83.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxINTEGER, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);

    symbols = getSymbols("&O7777777");
    CPPUNIT_ASSERT_EQUAL(size_t(2), symbols.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2097151.0, symbols[0].number, 1E-12);
    CPPUNIT_ASSERT_EQUAL(OUString(), symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(SbxLONG, symbols[0].type);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[1].text);
}

void ScannerTest::testTdf103104()
{
    std::vector<Symbol> symbols;

    symbols = getSymbols("asdf _\n asdf");
    CPPUNIT_ASSERT_EQUAL(size_t(3), symbols.size());
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[1].text);
    CPPUNIT_ASSERT(symbols[1].ws);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[2].text);

    symbols = getSymbols("asdf. _\n asdf");
    CPPUNIT_ASSERT_EQUAL(size_t(4), symbols.size());
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(dot, symbols[1].text);
    CPPUNIT_ASSERT(!symbols[1].ws);
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[2].text);
    CPPUNIT_ASSERT(symbols[2].ws);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[3].text);

    symbols = getSymbols("asdf _\n .asdf");
    CPPUNIT_ASSERT_EQUAL(size_t(4), symbols.size());
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[0].text);
    CPPUNIT_ASSERT_EQUAL(dot, symbols[1].text);
    CPPUNIT_ASSERT(!symbols[1].ws);
    CPPUNIT_ASSERT_EQUAL(asdf, symbols[2].text);
    CPPUNIT_ASSERT(!symbols[2].ws);
    CPPUNIT_ASSERT_EQUAL(cr, symbols[3].text);
}

void ScannerTest::testTdf136032()
{
    std::vector<Symbol> symbols;
    sal_Int32 errors;

    // tdf#136032 - abort scan of a string beginning with a hashtag,
    // if a comma/whitespace is found. Otherwise, the compiler raises a syntax error.
    symbols = getSymbols("Print #i,\"A#B\"", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(5), symbols.size());
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));
    symbols = getSymbols("Print #i, \"A#B\"", errors);
    CPPUNIT_ASSERT_EQUAL(size_t(5), symbols.size());
    CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(errors));
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(ScannerTest);
} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
