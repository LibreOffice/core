/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/unoapixml_test.hxx>

#include <o3tl/cppunittraitshelper.hxx>
#include <sfx2/sfxbasemodel.hxx>

#include <document.hxx>
#include <smdll.hxx>

#include <memory>

using namespace ::com::sun::star;

class MathMLExportTest : public UnoApiXmlTest
{
public:
    MathMLExportTest()
        : UnoApiXmlTest("starmath/qa/extras/data/")
    {
    }

    void testBlank();
    void testTdf97049();
    void testTdf101022();

    CPPUNIT_TEST_SUITE(MathMLExportTest);
    CPPUNIT_TEST(testBlank);
    CPPUNIT_TEST(testTdf97049);
    CPPUNIT_TEST(testTdf101022);
    CPPUNIT_TEST_SUITE_END();

protected:
    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override;

    void checkMathVariant(SmDocShell& rDocShell, bool bCapital, bool bSmall);
};

void MathMLExportTest::registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx)
{
    xmlXPathRegisterNs(pXmlXPathCtx, BAD_CAST("m"), BAD_CAST("http://www.w3.org/1998/Math/MathML"));
}

void MathMLExportTest::testBlank()
{
    mxComponent = loadFromDesktop("private:factory/smath");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    pDocShell->SetText("x`y~~z");
    save("MathML XML (Math)");
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mspace[1]", "width", "0.5em");
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mspace[2]", "width", "4em");
}

void MathMLExportTest::testTdf97049()
{
    mxComponent = loadFromDesktop("private:factory/smath");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    pDocShell->SetText("intd {{1 over x} dx}");
    save("MathML XML (Math)");
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mo[1]", "stretchy", "true");
    auto aContent = getXPathContent(pDoc, "/m:math/m:semantics/m:mrow/m:mo[1]");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aContent.getLength());
    CPPUNIT_ASSERT_EQUAL(u'\x222B', aContent[0]);
}

void MathMLExportTest::checkMathVariant(SmDocShell& rDocShell, bool bCapital, bool bSmall)
{
    rDocShell.SetText("%GAMMA %iGAMMA {ital %GAMMA} {nitalic %iGAMMA} "
                      "%gamma %igamma {ital %gamma} {nitalic %igamma}");
    save("MathML XML (Math)");
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    if (bCapital)
        assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[1]", "mathvariant");
    else
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mi[1]", "mathvariant", "normal");
    assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[1]/m:mi[1]", "mathvariant");
    assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[2]", "mathvariant");
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[2]/m:mi[1]", "mathvariant", "normal");
    if (bSmall)
        assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[3]", "mathvariant");
    else
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mi[3]", "mathvariant", "normal");
    assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[3]/m:mi[1]", "mathvariant");
    assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[4]", "mathvariant");
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[4]/m:mi[1]", "mathvariant", "normal");
    rDocShell.SetText("");
}

void MathMLExportTest::testTdf101022()
{
    mxComponent = loadFromDesktop("private:factory/smath");
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());

    checkMathVariant(*pDocShell, false, true); // default mode 2

    pDocShell->SetGreekCharStyle(1); // mode 1
    checkMathVariant(*pDocShell, true, true);

    pDocShell->SetGreekCharStyle(0); // mode 0
    checkMathVariant(*pDocShell, false, false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(MathMLExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
