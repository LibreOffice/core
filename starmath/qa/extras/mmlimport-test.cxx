/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/unoapi_test.hxx>

#include <sfx2/sfxbasemodel.hxx>

#include <document.hxx>
#include <smdll.hxx>

using namespace ::com::sun::star;

class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"starmath/qa/extras/data/"_ustr)
    {
    }

    void testColor();
    void testSimple();
    void testNsPrefixMath();
    void testMaction();
    void testMspace();
    void testtdf99556();
    void testTdf103430();
    void testTdf103500();
    void testTdf137008();
    void testTdf151842();
    void testMathmlEntities();
    void testMaj();
    void testHadd();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testColor);
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testNsPrefixMath);
    CPPUNIT_TEST(testMaction);
    CPPUNIT_TEST(testMspace);
    CPPUNIT_TEST(testtdf99556);
    CPPUNIT_TEST(testTdf103430);
    CPPUNIT_TEST(testTdf103500);
    CPPUNIT_TEST(testTdf137008);
    CPPUNIT_TEST(testTdf151842);
    CPPUNIT_TEST(testMathmlEntities);
    CPPUNIT_TEST(testMaj);
    CPPUNIT_TEST(testHadd);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testColor()
{
    loadFromFile(u"color.mml");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL(u"{ color black b"
                         " color white w"
                         " color red r"
                         " color green g"
                         " color blue b"
                         " color yellow y"
                         " color silver s"
                         " color gray g"
                         " color maroon m"
                         " color purple p"
                         " color lime l"
                         " color olive o"
                         " color navy n"
                         " color teal t"
                         " color aqua a"
                         " color fuchsia f"
                         " color crimson c"
                         " color dvip apricot"
                         " a color yellow y"
                         " color rgb 220 20 61 x }"_ustr,
                         pDocShell->GetText());
}

void Test::testSimple()
{
    loadFromFile(u"simple.mml");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("loaded text", u"left ( { a + b } right ) ^ 2"_ustr,
                                 pDocShell->GetText());
}

void Test::testNsPrefixMath()
{
    loadFromFile(u"ns-prefix-math.mml");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("loaded text", u"left ( { a + b } right ) ^ 2"_ustr,
                                 pDocShell->GetText());
}

void Test::testMaction()
{
    loadFromFile(u"maction.mml");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("loaded text", u"matrix{ 1 ## 2 ## 3 }"_ustr,
                                 pDocShell->GetText());
}

void Test::testMspace()
{
    loadFromFile(u"mspace.mml");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL(u"{ a b ~ c ~~``` d }"_ustr, pDocShell->GetText());
}

void Test::testtdf99556()
{
    loadFromFile(u"tdf99556-1.mml");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("loaded text", u"sqrt { }"_ustr, pDocShell->GetText());
}

void Test::testTdf103430()
{
    loadFromFile(u"tdf103430.mml");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL(u"{ frac { { nitalic d ^ 2 nitalic color blue y } } { { color dvip "
                         "apricot nitalic d font sans bold italic color red x } } }"_ustr,
                         pDocShell->GetText());
}

void Test::testTdf103500()
{
    loadFromFile(u"tdf103500.mml");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL(u"{ { int csup b csub a { { frac { 1 } { x } } ` nitalic d x } } = { "
                         "intd csup b csub a { { frac { 1 } { y } } ` nitalic d y } } }"_ustr,
                         pDocShell->GetText());
}

void Test::testTdf137008()
{
    // Without the fix in place, this test would have crashed
    loadFromFile(u"tdf137008.mml");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL(u"matrix{ { } # ## # }"_ustr, pDocShell->GetText());
}

void Test::testTdf151842()
{
    // Without the fix in place, this test would have crashed
    loadFromFile(u"tdf151842.odf");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL(u"test"_ustr, pDocShell->GetText());
    SmFormat aFormat = pDocShell->GetFormat();

    // Without the fix in place, this test would have failed with
    // - Expected: 2400
    // - Actual  : 423
    CPPUNIT_ASSERT_EQUAL(tools::Long(2400), aFormat.GetBaseSize().Height());
}

void Test::testMathmlEntities()
{
    loadFromFile(u"mthmlentities.mml");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT_EQUAL(u"{ \u03C3 \u221E \u221E \u03C3 }"_ustr, pDocShell->GetText());
}

void Test::testMaj()
{
    loadFromFile(u"maj.mml");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pModel);
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT(pDocShell);
    CPPUNIT_ASSERT_EQUAL(u"{ maj csup \u0661 csub { nitalic \U0001EE0A = \u0660 } { frac "
                         u"{ \u0661 } { nitalic \U0001EE0A } } }"_ustr,
                         pDocShell->GetText());
}

void Test::testHadd()
{
    loadFromFile(u"hadd.mml");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pModel);
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    CPPUNIT_ASSERT(pDocShell);
    CPPUNIT_ASSERT_EQUAL(u"{ nitalic \U0001EEF1 csup nitalic \U0001EE4E csub nitalic "
                         u"\U0001EE4E nitalic \U0001EE4E }"_ustr,
                         pDocShell->GetText());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
