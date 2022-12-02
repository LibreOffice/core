/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <comphelper/fileformat.h>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#include <document.hxx>
#include <smdll.hxx>

namespace
{
using namespace ::com::sun::star;

typedef tools::SvRef<SmDocShell> SmDocShellRef;

class Test : public test::BootstrapFixture
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

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
    CPPUNIT_TEST_SUITE_END();

private:
    void loadURL(const OUString& rURL)
    {
        // Cf.
        // filter/source/config/fragments/filters/MathML_XML__Math_.xcu
        auto pFilter = std::make_shared<SfxFilter>(
            MATHML_XML, OUString(),
            SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::TEMPLATE,
            SotClipboardFormatId::STARCALC_8, "MathML 2.0", OUString(), OUString(),
            "private:factory/smath*");
        pFilter->SetVersion(SOFFICE_FILEFORMAT_60);

        mxDocShell = new SmDocShell(SfxModelFlags::EMBEDDED_OBJECT
                                    | SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS
                                    | SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);

        SfxMedium* pSrcMed = new SfxMedium(rURL, StreamMode::STD_READ);
        pSrcMed->SetFilter(pFilter);
        pSrcMed->UseInteractionHandler(false);
        bool bLoaded = mxDocShell->DoLoad(pSrcMed);
        CPPUNIT_ASSERT_MESSAGE(
            OString("failed to load " + OUStringToOString(rURL, RTL_TEXTENCODING_UTF8)).getStr(),
            bLoaded);
    }

    SmDocShellRef mxDocShell;
};

void Test::setUp()
{
    BootstrapFixture::setUp();
    SmGlobals::ensure();
}

void Test::tearDown()
{
    if (mxDocShell.is())
        mxDocShell->DoClose();
    BootstrapFixture::tearDown();
}

void Test::testColor()
{
    loadURL(m_directories.getURLFromSrc(u"starmath/qa/extras/data/color.mml"));
    CPPUNIT_ASSERT_EQUAL(OUString("{ color black b"
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
                                  " color rgb 220 20 61 x }"),
                         mxDocShell->GetText());
}

void Test::testSimple()
{
    loadURL(m_directories.getURLFromSrc(u"starmath/qa/extras/data/simple.mml"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("loaded text", OUString("left ( { a + b } right ) ^ 2"),
                                 mxDocShell->GetText());
}

void Test::testNsPrefixMath()
{
    loadURL(m_directories.getURLFromSrc(u"starmath/qa/extras/data/ns-prefix-math.mml"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("loaded text", OUString("left ( { a + b } right ) ^ 2"),
                                 mxDocShell->GetText());
}

void Test::testMaction()
{
    loadURL(m_directories.getURLFromSrc(u"starmath/qa/extras/data/maction.mml"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("loaded text", OUString("matrix{ 1 ## 2 ## 3 }"),
                                 mxDocShell->GetText());
}

void Test::testMspace()
{
    loadURL(m_directories.getURLFromSrc(u"starmath/qa/extras/data/mspace.mml"));
    CPPUNIT_ASSERT_EQUAL(OUString("{ a b ~ c ~~``` d }"), mxDocShell->GetText());
}

void Test::testtdf99556()
{
    loadURL(m_directories.getURLFromSrc(u"starmath/qa/extras/data/tdf99556-1.mml"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("loaded text", OUString("sqrt { }"), mxDocShell->GetText());
}

void Test::testTdf103430()
{
    loadURL(m_directories.getURLFromSrc(u"starmath/qa/extras/data/tdf103430.mml"));
    CPPUNIT_ASSERT_EQUAL(OUString("frac { { nitalic d ^ 2 nitalic color blue y } } { { color dvip "
                                  "apricot nitalic d font sans bold italic color red x } }"),
                         mxDocShell->GetText());
}

void Test::testTdf103500()
{
    loadURL(m_directories.getURLFromSrc(u"starmath/qa/extras/data/tdf103500.mml"));
    CPPUNIT_ASSERT_EQUAL(OUString("{ { int csup b csub a { frac { 1 } { x } ` nitalic d x } } = { "
                                  "intd csup b csub a { frac { 1 } { y } ` nitalic d y } } }"),
                         mxDocShell->GetText());
}

void Test::testTdf137008()
{
    // Without the fix in place, this test would have crashed
    loadURL(m_directories.getURLFromSrc(u"starmath/qa/extras/data/tdf137008.mml"));
    CPPUNIT_ASSERT_EQUAL(OUString("matrix{ { } # ## # }"), mxDocShell->GetText());
}

void Test::testTdf151842()
{
    // Without the fix in place, this test would have crashed
    loadURL(m_directories.getURLFromSrc(u"starmath/qa/extras/data/tdf151842.odf"));
    CPPUNIT_ASSERT_EQUAL(OUString("test"), mxDocShell->GetText());
    SmFormat aFormat = mxDocShell->GetFormat();

    // Without the fix in place, this test would have failed with
    // - Expected: 4233
    // - Actual  : 423
    CPPUNIT_ASSERT_EQUAL(tools::Long(4233), aFormat.GetBaseSize().Height());
}

void Test::testMathmlEntities()
{
    loadURL(m_directories.getURLFromSrc(u"starmath/qa/extras/data/mthmlentities.mml"));
    CPPUNIT_ASSERT_EQUAL(OUString(u"{ \u03C3 \u221E \u221E \u03C3 }"), mxDocShell->GetText());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
