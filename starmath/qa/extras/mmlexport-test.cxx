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
#include <test/xmltesttools.hxx>
#include <unotools/tempfile.hxx>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#include <document.hxx>
#include <smdll.hxx>

#include <memory>

namespace {

using namespace ::com::sun::star;

typedef tools::SvRef<SmDocShell> SmDocShellRef;

class MathMLExportTest : public test::BootstrapFixture, public XmlTestTools
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    void testBlank();
    void testTdf97049();
    void testTdf101022();

    CPPUNIT_TEST_SUITE(MathMLExportTest);
    CPPUNIT_TEST(testBlank);
    CPPUNIT_TEST(testTdf97049);
    CPPUNIT_TEST(testTdf101022);
    CPPUNIT_TEST_SUITE_END();

protected:
    virtual void registerNamespaces(xmlXPathContextPtr &pXmlXPathCtx) override;

private:
    xmlDocPtr exportAndParse();

    SmDocShellRef mxDocShell;
};

void MathMLExportTest::setUp()
{
    BootstrapFixture::setUp();
    SmGlobals::ensure();
    mxDocShell = new SmDocShell(SfxModelFlags::EMBEDDED_OBJECT |
                                SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
                                SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
}

void MathMLExportTest::tearDown()
{
    if (mxDocShell.is())
        mxDocShell->DoClose();
    BootstrapFixture::tearDown();
}

void MathMLExportTest::registerNamespaces(xmlXPathContextPtr &pXmlXPathCtx)
{
    xmlXPathRegisterNs(pXmlXPathCtx, BAD_CAST("m"), BAD_CAST("http://www.w3.org/1998/Math/MathML"));
}

xmlDocPtr MathMLExportTest::exportAndParse()
{
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    SfxMedium aStoreMedium(aTempFile.GetURL(), StreamMode::STD_WRITE);
    std::shared_ptr<const SfxFilter> pExportFilter = SfxFilter::GetFilterByName(MATHML_XML);
    aStoreMedium.SetFilter(pExportFilter);
    CPPUNIT_ASSERT(mxDocShell->ConvertTo(aStoreMedium));
    aStoreMedium.Commit();
    xmlDocPtr pDoc = parseXml(aTempFile);
    CPPUNIT_ASSERT(pDoc);
    return pDoc;
}

void MathMLExportTest::testBlank()
{
    mxDocShell->SetText("x`y~~z");
    xmlDocPtr pDoc = exportAndParse();
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mspace[1]", "width", "0.5em");
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mspace[2]", "width", "4em");
}

void MathMLExportTest::testTdf97049()
{
    mxDocShell->SetText("intd {{1 over x} dx}");
    xmlDocPtr pDoc = exportAndParse();
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mo[1]", "stretchy", "true");
    auto aContent = getXPathContent(pDoc, "/m:math/m:semantics/m:mrow/m:mo[1]");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aContent.getLength());
    CPPUNIT_ASSERT_EQUAL(u'\x222B', aContent[0]);
}

void MathMLExportTest::testTdf101022()
{
#define CHECK_MATHVARIANT(capital, small) do                            \
    {                                                                   \
        mxDocShell->SetText("%GAMMA %iGAMMA {ital %GAMMA} {nitalic %iGAMMA} " \
                            "%gamma %igamma {ital %gamma} {nitalic %igamma}"); \
        xmlDocPtr pDoc = exportAndParse();                              \
        if (capital)                                                    \
            assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[1]", "mathvariant"); \
        else                                                            \
            assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mi[1]", "mathvariant", "normal"); \
        assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[1]/m:mi[1]", "mathvariant"); \
        assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[2]", "mathvariant"); \
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[2]/m:mi[1]", "mathvariant", "normal"); \
        if (small)                                                      \
            assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[3]", "mathvariant"); \
        else                                                            \
            assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mi[3]", "mathvariant", "normal"); \
        assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[3]/m:mi[1]", "mathvariant"); \
        assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[4]", "mathvariant"); \
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[4]/m:mi[1]", "mathvariant", "normal"); \
        mxDocShell->SetText("");                                        \
    }                                                                   \
    while (false)

    CHECK_MATHVARIANT(false, true); // default mode 2

    mxDocShell->SetGreekCharStyle(1); // mode 1
    CHECK_MATHVARIANT(true, true);

    mxDocShell->SetGreekCharStyle(0); // mode 0
    CHECK_MATHVARIANT(false, false);

#undef CHECK_MATHVARIANT
}

CPPUNIT_TEST_SUITE_REGISTRATION(MathMLExportTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
