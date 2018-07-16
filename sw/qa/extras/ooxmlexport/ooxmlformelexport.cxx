/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <config_features.h>
#include <string>
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override
    {
        const char* aBlacklist[] = {
            "math-escape.docx",
            "math-mso2k7.docx",
        };
        std::vector<const char*> vBlacklist(aBlacklist, aBlacklist + SAL_N_ELEMENTS(aBlacklist));

        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx")
                && std::find(vBlacklist.begin(), vBlacklist.end(), filename) == vBlacklist.end());
    }

protected:
};

DECLARE_OOXMLEXPORT_TEST(testMathEscape, "math-escape.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("\\{ left [ right ] left ( right ) \\}"),
                         getFormula(getRun(getParagraph(1), 1)));
}

// Remove all spaces, as LO export/import may change that.
// Replace symbol - (i.e. U+2212) with ASCII - , LO does this change and it shouldn't matter.
void CHECK_FORMULA(OUString const& expected, OUString const& actual)
{
    CPPUNIT_ASSERT_EQUAL(expected.replaceAll(" ", "").replaceAll(OUString(u"\u2212"), "-"),
                         actual.replaceAll(" ", "").replaceAll(OUString(u"\u2212"), "-"));
}

DECLARE_OOXMLEXPORT_TEST(testMathAccents, "math-accents.docx")
{
    CHECK_FORMULA("acute {a} grave {a} check {a} breve {a} circle {a} widevec {a} widetilde {a}"
                  " widehat {a} dot {a} widevec {a} widevec {a} widetilde {a} underline {a}",
                  getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_OOXMLEXPORT_TEST(testMathD, "math-d.docx")
{
    CHECK_FORMULA("left (x mline y mline z right )", getFormula(getRun(getParagraph(1), 1)));
    CHECK_FORMULA("left (1 right )", getFormula(getRun(getParagraph(1), 2)));
    CHECK_FORMULA("left [2 right ]", getFormula(getRun(getParagraph(1), 3)));
    CHECK_FORMULA("left ldbracket 3 right rdbracket", getFormula(getRun(getParagraph(1), 4)));
    CHECK_FORMULA("left lline 4 right rline", getFormula(getRun(getParagraph(1), 5)));
    CHECK_FORMULA("left ldline 5 right rdline", getFormula(getRun(getParagraph(1), 6)));
    CHECK_FORMULA("left langle 6 right rangle", getFormula(getRun(getParagraph(1), 7)));
    CHECK_FORMULA("left langle a mline b right rangle", getFormula(getRun(getParagraph(1), 8)));
    CHECK_FORMULA("left ({x} over {y} right )", getFormula(getRun(getParagraph(1), 9)));
}

DECLARE_OOXMLEXPORT_TEST(testMathEscaping, "math-escaping.docx")
{
    CHECK_FORMULA(u"\u2212 \u221E < x < \u221E", getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_OOXMLEXPORT_TEST(testMathLim, "math-lim.docx")
{
    CHECK_FORMULA(u"lim from {x \u2192 1} {x}", getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_OOXMLEXPORT_TEST(testMathMatrix, "math-matrix.docx")
{
    CHECK_FORMULA("left [matrix {1 # 2 ## 3 # 4} right ]", getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_OOXMLEXPORT_TEST(testMathMso2k7, "math-mso2k7.docx")
{
    CHECK_FORMULA(u"A = \u03C0 {r} ^ {2}", getFormula(getRun(getParagraph(1), 1)));
    // TODO check the stack/binom difference
    //    CHECK_FORMULA( "{left (x+a right )} ^ {n} = sum from {k=0} to {n} {left (binom {n} {k} right ) {x} ^ {k} {a} ^ {n-k}}",
    CHECK_FORMULA("{left (x+a right )} ^ {n} = sum from {k=0} to {n} {left (stack {n # k} right ) "
                  "{x} ^ {k} {a} ^ {n-k}}",
                  getFormula(getRun(getParagraph(2), 1)));
    CHECK_FORMULA(u"{left (1+x right )} ^ {n} =1+ {nx} over {1!} + {n left (n-1 right ) {x} ^ {2}} "
                  u"over {2!} +\u2026",
                  getFormula(getRun(getParagraph(3), 1)));
    // TODO check (cos/sin miss {})
    //    CHECK_FORMULA( "f left (x right ) = {a} rsub {0} + sum from {n=1} to {\xe2\x88\x9e} {left ({a} rsub {n} cos {{n\xcf\x80x} over {L}} + {b} rsub {n} sin {{n\xcf\x80x} over {L}} right )}",
    CHECK_FORMULA(u"f left (x right ) = {a} rsub {0} + sum from {n=1} to {\u221E} {left ({a} rsub "
                  u"{n} cos {n\u03C0x} over {L} + {b} rsub {n} sin {n\u03C0x} over {L} right )}",
                  getFormula(getRun(getParagraph(4), 1)));
    CHECK_FORMULA("{a} ^ {2} + {b} ^ {2} = {c} ^ {2}", getFormula(getRun(getParagraph(5), 1)));
    CHECK_FORMULA(u"x = {- b \u00B1 sqrt {{b} ^ {2} -4 ac}} over {2 a}",
                  getFormula(getRun(getParagraph(6), 1)));
    CHECK_FORMULA(u"{e} ^ {x} =1+ {x} over {1!} + {{x} ^ {2}} over {2!} + {{x} ^ {3}} over {3!} "
                  u"+\u2026,    -\u221E<x<\u221E",
                  getFormula(getRun(getParagraph(7), 1)));
    CHECK_FORMULA(
        //        "sin {\xce\xb1} \xc2\xb1 sin {\xce\xb2} =2 sin {{1} over {2} left (\xce\xb1\xc2\xb1\xce\xb2 right )} cos {{1} over {2} left (\xce\xb1\xe2\x88\x93\xce\xb2 right )}",
        // TODO check (cos/in miss {})
        u"sin \u03B1 \u00B1 sin \u03B2 =2 sin {1} over {2} left (\u03B1\u00B1\u03B2 right ) cos "
        u"{1} over {2} left (\u03B1\u2213\u03B2 right )",
        getFormula(getRun(getParagraph(8), 1)));
    CHECK_FORMULA(
        //        "cos {\xce\xb1} + cos {\xce\xb2} =2 cos {{1} over {2} left (\xce\xb1+\xce\xb2 right )} cos {{1} over {2} left (\xce\xb1-\xce\xb2 right )}",
        // TODO check (cos/sin miss {})
        u"cos \u03B1 + cos \u03B2 =2 cos {1} over {2} left (\u03B1+\u03B2 right ) cos {1} over {2} "
        u"left (\u03B1-\u03B2 right )",
        getFormula(getRun(getParagraph(9), 1)));
}

DECLARE_OOXMLEXPORT_TEST(testMathNary, "math-nary.docx")
{
    CHECK_FORMULA("lllint from {1} to {2} {x + 1}", getFormula(getRun(getParagraph(1), 1)));
    CHECK_FORMULA("prod from {a} {b}", getFormula(getRun(getParagraph(1), 2)));
    CHECK_FORMULA("sum to {2} {x}", getFormula(getRun(getParagraph(1), 3)));
}

DECLARE_OOXMLEXPORT_TEST(testMathOverbraceUnderbrace, "math-overbrace_underbrace.docx")
{
    CHECK_FORMULA("{abcd} overbrace {4}", getFormula(getRun(getParagraph(1), 1)));
    CHECK_FORMULA("{xyz} underbrace {3}", getFormula(getRun(getParagraph(2), 1)));
}

DECLARE_OOXMLEXPORT_TEST(testMathOverstrike, "math-overstrike.docx")
{
    CHECK_FORMULA("overstrike {abc}", getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_OOXMLEXPORT_TEST(testMathPlaceholders, "math-placeholders.docx")
{
    CHECK_FORMULA("sum from <?> to <?> <?>", getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_OOXMLEXPORT_TEST(testMathRad, "math-rad.docx")
{
    CHECK_FORMULA("sqrt {4}", getFormula(getRun(getParagraph(1), 1)));
    CHECK_FORMULA("nroot {3} {x + 1}", getFormula(getRun(getParagraph(1), 2)));
}

DECLARE_OOXMLEXPORT_TEST(testMathSubscripts, "math-subscripts.docx")
{
    CHECK_FORMULA("{x} ^ {y} + {e} ^ {x}", getFormula(getRun(getParagraph(1), 1)));
    CHECK_FORMULA("{x} ^ {b}", getFormula(getRun(getParagraph(1), 2)));
    CHECK_FORMULA("{x} rsub {b}", getFormula(getRun(getParagraph(1), 3)));
    CHECK_FORMULA("{a} rsub {c} rsup {b}", getFormula(getRun(getParagraph(1), 4)));
    CHECK_FORMULA("{x} lsub {2} lsup {1}", getFormula(getRun(getParagraph(1), 5)));
    CHECK_FORMULA("{{x csup {6} csub {3}} lsub {4} lsup {5}} rsub {2} rsup {1}",
                  getFormula(getRun(getParagraph(1), 6)));
}

DECLARE_OOXMLEXPORT_TEST(testMathVerticalStacks, "math-vertical_stacks.docx")
{
    CHECK_FORMULA("{a} over {b}", getFormula(getRun(getParagraph(1), 1)));
    CHECK_FORMULA("{a} / {b}", getFormula(getRun(getParagraph(2), 1)));
    // TODO check these
    //    CHECK_FORMULA( "binom {a} {b}", getFormula( getRun( getParagraph( 3 ), 1 )));
    //    CHECK_FORMULA( "binom {a} {binom {b} {c}}", getFormula( getRun( getParagraph( 4 ), 1 )));
}

DECLARE_OOXMLEXPORT_TEST(testMathLiteral, "math-literal.docx")
{
    CHECK_FORMULA(u"iiint from {V} to <?> {\"div\" \"F\"}  dV= llint from {S} to <?> {\"F\" \u2219 "
                  u"\"n \" dS}",
                  getFormula(getRun(getParagraph(1), 1)));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
