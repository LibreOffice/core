/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Luke Dixon <6b8b4567@gmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <sal/types.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include <sal/config.h>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/svapp.hxx>
#include <smdll.hxx>

#include <document.hxx>
#include <node.hxx>
#include <visitors.hxx>
#include <cursor.hxx>

namespace CppUnit {
template<>
struct assertion_traits<String>
{
    static bool equal(const String& x, const String& y)
    {
        return x == y;
    }

    static std::string toString(const String& x)
    {
        OStringStream ost;
        ost << rtl::OUStringToOString(x, RTL_TEXTENCODING_UTF8).getStr();
        return ost.str();
    }
};
}

SO2_DECL_REF(SmDocShell)
SO2_IMPL_REF(SmDocShell)

class TestOutputDevice : public OutputDevice
{
public:
    TestOutputDevice()
    {
    }
};

using namespace ::com::sun::star;

namespace {

class Test : public CppUnit::TestFixture {
public:
    Test();
    ~Test();

    // init
    virtual void setUp();
    virtual void tearDown();

    // tests
    void SimpleUnaryOp();
    void SimpleBinaryOp();
    void SimpleRelationalOp();
    void SimpleSetOp();
    void SimpleFunctions();
    void SimpleOperators();
    void SimpleAttributes();
    void SimpleMisc();
    void SimpleBrackets();
    void SimpleFormats();
    void SimpleGreekChars();
    void SimpleSpecialChars();
    void testBinomInBinHor();
    void testBinVerInUnary();
    void testBinHorInSubSup();
    void testUnaryInMixedNumberAsNumerator();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(SimpleUnaryOp);
    CPPUNIT_TEST(SimpleBinaryOp);
    CPPUNIT_TEST(SimpleRelationalOp);
    CPPUNIT_TEST(SimpleSetOp);
    CPPUNIT_TEST(SimpleFunctions);
    CPPUNIT_TEST(SimpleOperators);
    CPPUNIT_TEST(SimpleAttributes);
    CPPUNIT_TEST(SimpleMisc);
    CPPUNIT_TEST(SimpleBrackets);
    CPPUNIT_TEST(SimpleFormats);
    CPPUNIT_TEST(SimpleGreekChars);
    CPPUNIT_TEST(SimpleSpecialChars);
    CPPUNIT_TEST(testBinomInBinHor);
    CPPUNIT_TEST(testBinVerInUnary);
    CPPUNIT_TEST(testBinHorInSubSup);
    CPPUNIT_TEST(testUnaryInMixedNumberAsNumerator);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< uno::XComponentContext > m_context;
    SmDocShellRef xDocShRef;
    void parseandparseagain(const char *input, const char *test_name);
    void ParseAndCheck(const char *input, const char *expected, const char *test_name);
};

Test::Test()
{
    m_context = cppu::defaultBootstrap_InitialComponentContext();

    uno::Reference<lang::XMultiComponentFactory> xFactory(m_context->getServiceManager());
    uno::Reference<lang::XMultiServiceFactory> xSM(xFactory, uno::UNO_QUERY_THROW);

    //Without this we're crashing because callees are using
    //getProcessServiceFactory.  In general those should be removed in favour
    //of retaining references to the root ServiceFactory as its passed around
    comphelper::setProcessServiceFactory(xSM);

    InitVCL();

    SmGlobals::ensure();
}

void Test::setUp()
{
    xDocShRef = new SmDocShell(SFXOBJECTSHELL_STD_NORMAL);
}

void Test::tearDown()
{
    xDocShRef.Clear();
}

Test::~Test()
{
}

/*
 * Most of the formula commands in this file came from:
 * http://wiki.services.openoffice.org/wiki/Template:Math_commands_reference
 * which was licensed with a
 * Creative Common Attribution 3.0 license and written by:
 * Jeanweber, Weegreenblobbie, Jdpipe, TJFrazier, Ysangkok, B michaelsen, Spellbreaker
 */

void Test::SimpleUnaryOp()
{
    parseandparseagain("+1", "Positive (plus)");
    parseandparseagain("-2", "Negative (minus)");
    parseandparseagain("+-3", "Plus/minus");
    parseandparseagain("-+4", "Minus/plus");
    parseandparseagain("neg a", "Boolean 'not'");
    parseandparseagain("fact a", "Factorial");
    parseandparseagain(" - { 1 over 2 } ", "BinVer in Unary 1");
    ParseAndCheck(" - { 1 over 2 } ", " - { 1 over 2 } ", "BinVer in Unary 1");
    parseandparseagain(" { - { 1 over 2 } } ", "BinVer in Unary 2");
    parseandparseagain(" - 1 over 2 ", "Unary in BinVer as numerator 1");
    parseandparseagain(" { - 1 } over 2 ", "Unary in BinVer as numerator 2");
    parseandparseagain(" 1 over - 2 ", "Unary in BinVer as denominator 1");
    parseandparseagain(" 1 over { - 2 } ", "Unary in BinVer as denominator 2");
    parseandparseagain(" 2 { - 1 over 2 } ", "Mixed number with Unary in denominator 1");
    parseandparseagain(" 2 { - 1 } over 2 ", "Mixed number with Unary in denominator 2");
    parseandparseagain(" - 1 + 2 ", "Unary in BinHor");
}

void Test::SimpleBinaryOp()
{
    parseandparseagain("a + b", "Addition");
    parseandparseagain("a cdot b", "Dot product");
    parseandparseagain("a times b", "Cross product");
    parseandparseagain("a * b", "Multiplication (asterisk)");
    parseandparseagain("a and b", "Boolean 'and'");
    parseandparseagain("a - b", "Subtraction");
    parseandparseagain("a over b", "Division (as a fraction)");
    parseandparseagain("a div b", "Division (as an operator)");
    parseandparseagain("a / b", "Division (with a slash)");
    parseandparseagain("a or b", "Boolean 'or'");
    parseandparseagain("a circ b", "Concatenation");
}

void Test::SimpleRelationalOp()
{
    parseandparseagain("a = b", "Is equal");
    parseandparseagain("a <> b", "Is not equal");
    parseandparseagain("a approx 2", "Approximately");
    parseandparseagain("a divides b", "Divides");
    parseandparseagain("a ndivides b", "Does not divide");
    parseandparseagain("a < 2", "Less than");
    parseandparseagain("a > 2", "Greater than");
    parseandparseagain("a simeq b", "Similar to or equal");
    parseandparseagain("a parallel b", "Parallel");
    parseandparseagain("a ortho b", "Orthogonal to");
    parseandparseagain("a leslant b", "Less than or equal to");
    parseandparseagain("a geslant b", "Greater than or equal to");
    parseandparseagain("a sim b", "Similar to");
    parseandparseagain("a equiv b", "Congruent");
    parseandparseagain("a <= b", "Less than or equal to");
    parseandparseagain("a >= b", "Greater than or equal to");
    parseandparseagain("a prop b", "Proportional");
    parseandparseagain("a toward b", "Toward");
    parseandparseagain("a dlarrow b", "Arrow left");
    parseandparseagain("a dlrarrow b", "Double arrow left and right");
    parseandparseagain("drarrow b", "Arrow right");
}

void Test::SimpleSetOp()
{
    parseandparseagain("a in B", "Is in");
    parseandparseagain("a notin B", "Is not in");
    parseandparseagain("A owns b", "Owns");
    parseandparseagain("emptyset", "Empty set");
    parseandparseagain("A intersection B", "Intersection");
    parseandparseagain("A union B", "Union");
    parseandparseagain("A setminus B", "Difference");
    parseandparseagain("A slash B", "Quotient");
    parseandparseagain("aleph", "Aleph");
    parseandparseagain("A subset B", "Subset");
    parseandparseagain("A subseteq B", "Subset or equal to");
    parseandparseagain("A supset B", "Superset");
    parseandparseagain("A supseteq B", "Superset or equal to");
    parseandparseagain("A nsubset B", "Not subset");
    parseandparseagain("A nsubseteq B", "Not subset or equal");
    parseandparseagain("A nsupset B", "Not superset");
    parseandparseagain("A nsupseteq B", "Not superset or equal");
    parseandparseagain("setN", "Set of natural numbers");
    parseandparseagain("setZ", "Set of integers");
    parseandparseagain("setQ", "Set of rational numbers");
    parseandparseagain("setR", "Set of real numbers");
    parseandparseagain("setC", "Set of complex numbers");
}

void Test::SimpleFunctions()
{
    parseandparseagain("func e^{a}", "Exponential");
    parseandparseagain("ln(a)", "Natural logarithm");
    parseandparseagain("exp(a)", "Exponential function");
    parseandparseagain("log(a)", "Logarithm");
    parseandparseagain("a^{b}", "Power");
    parseandparseagain("sin(a)", "Sine");
    parseandparseagain("cos(a)", "Cosine");
    parseandparseagain("tan(a)", "Tangent");
    parseandparseagain("cot(a)", "Cotangent");
    parseandparseagain("sqrt{a}", "Square root");
    parseandparseagain("arcsin(a)", "Arcsine");
    parseandparseagain("arccos(a)", "Arccosine");
    parseandparseagain("arctan(a)", "Arctangent");
    parseandparseagain("arccot(a)", "Arc cotangent");
    parseandparseagain("nroot{a}{b}", "nth root");
    parseandparseagain("sinh(a)", "Hyperbolic sine");
    parseandparseagain("cosh(a)", "Hyperbolic cosine");
    parseandparseagain("tanh(a)", "Hyperbolic tangent");
    parseandparseagain("coth(a)", "Hyperbolic cotangent");
    parseandparseagain("abs{a}", "Absolute value");
    parseandparseagain("arsinh(a)", "Arc hyperbolic sine");
    parseandparseagain("arcosh(a)", "Arc hyperbolic cosine");
    parseandparseagain("artanh(a)", "Arc hyperbolic tangent");
    parseandparseagain("arcoth(a)", "Arc hyperbolic cotangent");
}

void Test::SimpleOperators()
{
    parseandparseagain("lim{a}", "Limit");
    parseandparseagain("sum{a}", "Sum");
    parseandparseagain("prod{a}", "Product");
    parseandparseagain("coprod{a}", "Coproduct");
    parseandparseagain("int from {r_0} to {r_t} a", "Upper and lower bounds shown with integral (from & to)");
    ParseAndCheck("int csup {r_0} csub {r_t} a", "int csup { r rsub 0 } csub { r rsub t } a ", "Upper and lower bounds shown with integral (csub & csup)");
    ParseAndCheck("sum csup { size 8 { x - 1 } } csub { size 8 a } b ", "sum csup { size 8 { x - 1 } } csub { size 8 a } b ", "Sum with sized upper and lower bounds");
    parseandparseagain("int{a}", "Integral");
    parseandparseagain("iint{a}", "Double integral");
    parseandparseagain("iiint{a}", "Triple integral");
    parseandparseagain("sum from{3}b", "Lower bound shown with summation symbol");
    parseandparseagain("lint a", "Contour integral");
    parseandparseagain("llint a", "Double curved integral");
    parseandparseagain("lllint a", "Triple curved integral");
    parseandparseagain("prod from {i=1} to {n} {(i+1)}", "Product with range");
}

void Test::SimpleAttributes()
{
    parseandparseagain("acute a", "Acute accent");
    parseandparseagain("grave a", "Grave accent");
    parseandparseagain("check a", "Reverse circumflex");
    parseandparseagain("breve a", "Breve");
    parseandparseagain("circle a", "Circle");
    parseandparseagain("vec a", "Vector arrow");
    parseandparseagain("tilde a", "Tilde");
    parseandparseagain("hat a", "Circumflex");
    parseandparseagain("bar a", "Line above");
    parseandparseagain("dot a", "Dot");
    parseandparseagain("widevec abc", "Wide vector arrow");
    parseandparseagain("widetilde abc", "Wide tilde");
    parseandparseagain("widehat abc", "Wide circumflex");
    parseandparseagain("ddot a", "Double dot");
    parseandparseagain("overline abc", "Line over");
    parseandparseagain("underline abc", "Line under");
    parseandparseagain("overstrike abc", "Line through");
    parseandparseagain("dddot a", "Triple dot");
    parseandparseagain("phantom a", "Transparent (useful to get a placeholder of a given size)");
    parseandparseagain("bold a", "Bold font");
    parseandparseagain("ital a", "Italic font");
    parseandparseagain("nitalic a", "Roman (non-italic) font 1");
    parseandparseagain("\"a\"", "Roman (non-italic) font 2");
    parseandparseagain("size 16 qv", "Resize font");
    parseandparseagain("font sans qv", "Sans serif font");
    parseandparseagain("font serif qv", "Serif font");
    parseandparseagain("font fixed qv", "Fixed font");
    parseandparseagain("color cyan qv", "Cyan color");
    parseandparseagain("color yellow qv", "Yellow color");
    parseandparseagain("color white qv", "White color");
    parseandparseagain("color green qv", "Green color");
    parseandparseagain("color blue qv", "Blue color");
    parseandparseagain("color red qv", "Red color");
    parseandparseagain("color green X qv", "Green color changes back");
    parseandparseagain("color green {X qv}", "Green color, more than one item");
}

void Test::SimpleMisc()
{
    parseandparseagain("infinity", "Infinity");
    parseandparseagain("partial", "Partial");
    parseandparseagain("nabla", "Nabla");
    parseandparseagain("exists", "There exists");
    parseandparseagain("notexists", "There not exists");
    parseandparseagain("forall", "For all");
    parseandparseagain("hbar", "H bar");
    parseandparseagain("lambdabar", "Lambda bar");
    parseandparseagain("re", "Real part");
    parseandparseagain("im", "Imaginary part");
    parseandparseagain("wp", "Weierstrass p");
    parseandparseagain("leftarrow", "Left arrow");
    parseandparseagain("rightarrow", "Right arrow");
    parseandparseagain("uparrow", "Up arrow");
    parseandparseagain("downarrow", "Down arrow");
    parseandparseagain("dotslow", "Dots at bottom");
    parseandparseagain("dotsaxis", "Dots at middle");
    parseandparseagain("dotsvert", "Dots vertical");
    parseandparseagain("dotsup", "Dots diagonal upward");
    parseandparseagain("dotsdown", "Dots diagonal downward");
}

void Test::SimpleBrackets()
{
    parseandparseagain("(a)", "Round Brackets");
    parseandparseagain("[b]", "Square Brackets");
    parseandparseagain("ldbracket c rdbracket", "Double Square Brackets");
    parseandparseagain("lline a rline", "Single line or absolute");
    parseandparseagain("abs a", "Single line or absolute 2");
    parseandparseagain("ldline a rdline", "Double line");
    parseandparseagain("lbrace w rbrace", "Braces");
    parseandparseagain("left lbrace stack{0, n <> 0 # 1, n = 1} right none", "Single left brace");
    parseandparseagain("langle d rangle", "Angle Brackets");
    parseandparseagain("langle a mline b rangle", "Operator Brackets");
    parseandparseagain("{a}", "Group brackets (used for program control)");
    parseandparseagain("left ( stack{a # b # z} right )", "Round brackets scalable");
    parseandparseagain("left [ stack{x # y} right ]", "Square brackets scalable");
    parseandparseagain("left ldbracket c right rdbracket", "Double square brackets scalable");
    parseandparseagain("left lline a right rline", "Line scalable");
    parseandparseagain("left ldline d right rdline", "Double line scalable");
    parseandparseagain("left lbrace e right rbrace", "Brace scalable");
    parseandparseagain("left langle f right rangle", "Angle bracket scalable");
    parseandparseagain("left langle g mline h right rangle", "Operator brackets scalable");
    parseandparseagain("{a} overbrace b", "Over brace scalable");
    parseandparseagain("{b} underbrace a", "Under brace scalable");
}

void Test::SimpleFormats()
{
    parseandparseagain("a lsup{b}", "Left superscript");
    parseandparseagain("a csup{b}", "Center superscript");
    parseandparseagain("a^{b}", "Right superscript");
    parseandparseagain("a lsub{b}", "Left subscript");
    parseandparseagain("a csub{b}", "Center subscript");
    parseandparseagain("a_{b}", "Right subscript");
    parseandparseagain("stack { Hello world # alignl (a) }", "Align character to left");
    parseandparseagain("stack{Hello world # alignc(a)}", "Align character to center");
    parseandparseagain("stack { Hello world # alignr(a)}", "Align character to right");
    parseandparseagain("binom{a}{b}", "Vertical stack of 2");
    parseandparseagain("stack{a # b # z}", "Vertical stack, more than 2");
    parseandparseagain("matrix{a # b ## c # d}", "Matrix");
    parseandparseagain("matrix{a # \"=\" # alignl{b} ## {} # \"=\" # alignl{c+1}}", "Equations aligned at '=' (using 'matrix') ");
    parseandparseagain("stack{alignl{a} = b # alignl{phantom{a} = c+1}}", "Equations aligned at '=' (using 'phantom') ");
    parseandparseagain("asldkfjo newline sadkfj", "New line");
    parseandparseagain("stuff `stuff", "Small gap (grave)");
    parseandparseagain("stuff~stuff", "Large gap (tilde)");
}

void Test::SimpleGreekChars()
{
    parseandparseagain("%ALPHA", "Capital alpha");
    parseandparseagain("%BETA", "Capital beta");
    parseandparseagain("%CHI", "Capital chi");
    parseandparseagain("%DELTA", "Capital delta");
    parseandparseagain("%EPSILON", "Capital epsilon");
    parseandparseagain("%ETA", "Capital eta");
    parseandparseagain("%GAMMA", "Capital gamma");
    parseandparseagain("%IOTA", "Capital iota");
    parseandparseagain("%LAMBDA", "Capital lambda");
    parseandparseagain("%MU", "Capital mu");
    parseandparseagain("%NU", "Capital nu");
    parseandparseagain("%OMEGA", "Capital omega");
    parseandparseagain("%OMICRON", "Capital omicron");
    parseandparseagain("%PHI", "Capital phi");
    parseandparseagain("%PI", "Capital pi");
    parseandparseagain("%PSI", "Capital psi");
    parseandparseagain("%RHO", "Capital rho");
    parseandparseagain("%SIGMA", "Capital sigma");
    parseandparseagain("%TAU", "Capital tau");
    parseandparseagain("%THETA", "Capital theta");
    parseandparseagain("%UPSILON", "Capital upsilon");
    parseandparseagain("%XI", "Capital xi");
    parseandparseagain("%ZETA", "Capital zeta");
    parseandparseagain("%alpha", "lowercase alpha");
    parseandparseagain("%beta", "lowercase beta");
    parseandparseagain("%chi", "lowercase chi");
    parseandparseagain("%delta", "lowercase delta");
    parseandparseagain("%epsilon", "lowercase epsilon");
    parseandparseagain("%eta", "lowercase eta");
    parseandparseagain("%gamma", "lowercase gamma");
    parseandparseagain("%iota", "lowercase iota");
    parseandparseagain("%kappa", "lowercase kappa");
    parseandparseagain("%lambda", "lowercase lambda");
    parseandparseagain("%mu", "lowercase mu");
    parseandparseagain("%nu", "lowercase nu");
    parseandparseagain("%omega", "lowercase omega");
    parseandparseagain("%omicron", "lowercase omicron");
    parseandparseagain("%phi", "lowercase phi");
    parseandparseagain("%pi", "lowercase pi");
    parseandparseagain("%psi", "lowercase psi");
    parseandparseagain("%rho", "lowercase rho");
    parseandparseagain("%sigma", "lowercase sigma");
    parseandparseagain("%tau", "lowercase tau");
    parseandparseagain("%theta", "lowercase theta");
    parseandparseagain("%upsilon", "lowercase upsilon");
    parseandparseagain("%varepsilon", "Varepsilon");
    parseandparseagain("%varphi", "Varphi");
    parseandparseagain("%varpi", "Varpi");
    parseandparseagain("%varrho", "Varrho");
    parseandparseagain("%varsigma", "Varsigma");
    parseandparseagain("%vartheta", "Vartheta");
    parseandparseagain("%xi", "lowercase xi");
    parseandparseagain("%zeta", "lowercase zeta");
}

void Test::SimpleSpecialChars()
{
    parseandparseagain("%and", "And");
    parseandparseagain("%angle", "Angle");
    parseandparseagain("%element", "Element");
    parseandparseagain("%identical", "Identical");
    parseandparseagain("%infinite", "Infinite");
    parseandparseagain("%noelement", "No element");
    parseandparseagain("%notequal", "Not equal");
    parseandparseagain("%or", "Or");
    parseandparseagain("%perthousand", "Per thousand");
    parseandparseagain("%strictlygreaterthan", "Strictly greater than");
    parseandparseagain("%strictlylessthan", "Strictly less than");
    parseandparseagain("%tendto", "Tend to");
}

/* This test takes a formula command, parses it, converts the node to text,
 * parses it again, converts it to text again, and compares the values.
 * Doing this doesn't prove that it is correct, but it should prove that the
 * meaning of the original command is not being changed.
 */
void Test::parseandparseagain(const char *formula, const char *test_name)
{
    OUString output1, output2;
    SmNode *pNode1, *pNode2;

    // parse 1
    OUString input = OUString::createFromAscii(formula);
    pNode1 = SmParser().ParseExpression(input);
    pNode1->Prepare(xDocShRef->GetFormat(), *xDocShRef);
    SmNodeToTextVisitor(pNode1, output1);

    // parse 2
    pNode2 = SmParser().ParseExpression(output1);
    pNode2->Prepare(xDocShRef->GetFormat(), *xDocShRef);
    SmNodeToTextVisitor(pNode2, output2);

    // compare
    CPPUNIT_ASSERT_EQUAL_MESSAGE(test_name,
        output1,
        output2);

    delete pNode1;
    delete pNode2;
}

void Test::ParseAndCheck(const char *formula, const char * expected, const char *test_name)
{
    OUString sOutput;
    SmNode *pNode;

    // parse
    OUString sInput = OUString::createFromAscii(formula);
    pNode = SmParser().ParseExpression(sInput);
    pNode->Prepare(xDocShRef->GetFormat(), *xDocShRef);
    SmNodeToTextVisitor(pNode, sOutput);

    // compare
    OUString sExpected = OUString::createFromAscii(expected);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(test_name,
        sExpected,
        sOutput);

    delete pNode;
}

void Test::testBinomInBinHor()
{
    String sInput, sExpected, sOutput;
    SmNode* pTree;

    // set up a binom (table) node
    sInput.AppendAscii("binom a b + c");
    pTree = SmParser().Parse(sInput);
    pTree->Prepare(xDocShRef->GetFormat(), *xDocShRef);

    SmCursor aCursor(pTree, xDocShRef);
    TestOutputDevice aOutputDevice;

    // move forward (more than) enough places to be at the end
    int i;
    for (i = 0; i < 8; ++i)
        aCursor.Move(&aOutputDevice, MoveRight);

    // tack +d on the end, which will put the binom into an SmBinHorNode
    aCursor.InsertElement(PlusElement);
    aCursor.InsertText("d");

    sExpected.AppendAscii(" { { binom a b + c } + d } ");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Binom Node in BinHor Node", sExpected, xDocShRef->GetText());

    delete pTree;
}

void Test::testBinVerInUnary()
{
    String sInput, sExpected, sOutput;
    SmNode* pTree;

    // set up a unary operator with operand
    sInput.AppendAscii("- 1");
    pTree = SmParser().Parse(sInput);
    pTree->Prepare(xDocShRef->GetFormat(), *xDocShRef);

    SmCursor aCursor(pTree, xDocShRef);
    TestOutputDevice aOutputDevice;

    // move forward (more than) enough places to be at the end
    int i;
    for (i = 0; i < 3; ++i)
        aCursor.Move(&aOutputDevice, MoveRight);

    // select the operand
    aCursor.Move(&aOutputDevice, MoveLeft, false);
    // set up a fraction
    aCursor.InsertFraction();
    aCursor.Move(&aOutputDevice, MoveDown);
    aCursor.InsertText("2");

    sExpected.AppendAscii(" - { 1 over 2 } ");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Binary Vertical in Unary Operator", sExpected, xDocShRef->GetText());

    delete pTree;
}

void Test::testBinHorInSubSup()
{
    String sInput, sExpected, sOutput;
    SmNode* pTree;

    // set up a blank formula
    sInput.AppendAscii("");
    pTree = SmParser().Parse(sInput);
    pTree->Prepare(xDocShRef->GetFormat(), *xDocShRef);

    SmCursor aCursor(pTree, xDocShRef);
    TestOutputDevice aOutputDevice;

    // Insert an RSup expression with a BinHor for the exponent
    aCursor.InsertText("a");
    aCursor.InsertSubSup(RSUP);
    aCursor.InsertText("b");
    aCursor.InsertElement(PlusElement);
    aCursor.InsertText("c");

    // Move to the end and add d to the expression
    aCursor.Move(&aOutputDevice, MoveRight);
    aCursor.InsertElement(PlusElement);
    aCursor.InsertText("d");

    sExpected.AppendAscii(" { a rsup { b + c } + d } ");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("BinHor in SubSup", sExpected, xDocShRef->GetText());

    delete pTree;
}

void Test::testUnaryInMixedNumberAsNumerator()
{
    String sInput, sExpected, sOutput;
    SmNode* pTree;

    // set up a unary operator
    sInput.AppendAscii("- 1");
    pTree = SmParser().Parse(sInput);
    pTree->Prepare(xDocShRef->GetFormat(), *xDocShRef);

    SmCursor aCursor(pTree, xDocShRef);
    TestOutputDevice aOutputDevice;

    // move forward (more than) enough places to be at the end
    int i;
    for (i = 0; i < 3; ++i)
        aCursor.Move(&aOutputDevice, MoveRight);

    // Select the whole Unary Horizontal Node
    aCursor.Move(&aOutputDevice, MoveLeft, false);
    aCursor.Move(&aOutputDevice, MoveLeft, false);

    // Set up a fraction
    aCursor.InsertFraction();
    aCursor.Move(&aOutputDevice, MoveDown);
    aCursor.InsertText("2");

    // Move left and turn this into a mixed number
    // (bad form, but this could happen right?)
    aCursor.Move(&aOutputDevice, MoveLeft);
    aCursor.Move(&aOutputDevice, MoveLeft);
    aCursor.InsertText("2");

    // move forward (more than) enough places to be at the end
    for (i = 0; i < 8; ++i)
        aCursor.Move(&aOutputDevice, MoveRight);

    // add 4 to the end
    aCursor.InsertElement(PlusElement);
    aCursor.InsertText("4");

    sExpected.AppendAscii(" { 2 { - 1 over 2 } + 4 } ");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unary in mixed number as Numerator", sExpected, xDocShRef->GetText());

    delete pTree;
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
