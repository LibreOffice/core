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

#include "document.hxx"
#include "smdll.hxx"
#include "node.hxx"
#include "parse.hxx"

#include <memory>

namespace {

using namespace ::com::sun::star;

typedef tools::SvRef<SmDocShell> SmDocShellRef;

class MathMLExportTest : public test::BootstrapFixture, public XmlTestTools
{
public:
    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    void testTdf101022();

    CPPUNIT_TEST_SUITE(MathMLExportTest);
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
    mxDocShell->DoInitUnitTest();
}

void MathMLExportTest::tearDown()
{
    if (mxDocShell)
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
    //aTempFile.EnableKillingFile();
    SfxMedium aStoreMedium(aTempFile.GetURL(), STREAM_STD_WRITE);
    std::shared_ptr<const SfxFilter> pExportFilter = SfxFilter::GetFilterByName(MATHML_XML);
    aStoreMedium.SetFilter(pExportFilter);
    CPPUNIT_ASSERT(mxDocShell->ConvertTo(aStoreMedium));
    aStoreMedium.Commit();
    xmlDocPtr pDoc = parseXml(aTempFile);
    CPPUNIT_ASSERT(pDoc);
    return pDoc;
}

void MathMLExportTest::testTdf101022()
{
    xmlDocPtr pDoc;
    OUString aText("{ital %GAMMA} + %GAMMA + %iGAMMA + {nitalic %iGAMMA} + %gamma + %igamma");
    SmFormat aFormat = mxDocShell->GetFormat();

#define CHECK_MATHVARIANT(capital, small) do                            \
    {                                                                   \
        mxDocShell->SetText(aText);                                     \
        pDoc = exportAndParse();                                        \
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[1]/m:mi[1]", "mathvariant", "italic"); \
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mi[1]", "mathvariant", capital); \
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mi[2]", "mathvariant", "italic"); \
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[2]/m:mi[1]", "mathvariant", "normal"); \
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mi[3]", "mathvariant", small); \
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mi[4]", "mathvariant", "italic"); \
    }                                                                   \
    while (false)

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aFormat.GetGreekCharStyle()); // default format
    CHECK_MATHVARIANT("normal", "normal");

    aFormat.SetGreekCharStyle(1); // mode 1
    mxDocShell->SetFormat(aFormat);
    CHECK_MATHVARIANT("italic", "italic");

    aFormat.SetGreekCharStyle(2); // mode 2
    mxDocShell->SetFormat(aFormat);
    CHECK_MATHVARIANT("normal", "italic");

#undef CHECK_MATHVARIANT
}

CPPUNIT_TEST_SUITE_REGISTRATION(MathMLExportTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
