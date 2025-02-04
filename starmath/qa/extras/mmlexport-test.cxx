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
        : UnoApiXmlTest(u"starmath/qa/extras/data/"_ustr)
    {
    }

    void testBlank();
    void testTdf97049();
    void testTdf101022();
    void testMaj();
    void testHadd();
    void testTdf158867();

    CPPUNIT_TEST_SUITE(MathMLExportTest);
    CPPUNIT_TEST(testBlank);
    CPPUNIT_TEST(testTdf97049);
    CPPUNIT_TEST(testTdf101022);
    CPPUNIT_TEST(testMaj);
    CPPUNIT_TEST(testHadd);
    CPPUNIT_TEST(testTdf158867);
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
    loadFromURL(u"private:factory/smath"_ustr);
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    pDocShell->SetText(u"x`y~~z"_ustr);
    save(u"MathML XML (Math)"_ustr);
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mspace[1]", "width", u"0.5em");
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mspace[2]", "width", u"4em");
}

void MathMLExportTest::testTdf97049()
{
    loadFromURL(u"private:factory/smath"_ustr);
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    pDocShell->SetText(u"intd {{1 over x} dx}"_ustr);
    save(u"MathML XML (Math)"_ustr);
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mo[1]", "stretchy", u"true");
    auto aContent = getXPathContent(pDoc, "/m:math/m:semantics/m:mrow/m:mo[1]");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aContent.getLength());
    CPPUNIT_ASSERT_EQUAL(u'\x222B', aContent[0]);
}

void MathMLExportTest::checkMathVariant(SmDocShell& rDocShell, bool bCapital, bool bSmall)
{
    rDocShell.SetText(u"%GAMMA %iGAMMA {ital %GAMMA} {nitalic %iGAMMA} "
                      "%gamma %igamma {ital %gamma} {nitalic %igamma}"_ustr);
    save(u"MathML XML (Math)"_ustr);
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    if (bCapital)
        assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[1]", "mathvariant");
    else
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mi[1]", "mathvariant", u"normal");
    assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[1]/m:mi[1]", "mathvariant");
    assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[2]", "mathvariant");
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[2]/m:mi[1]", "mathvariant", u"normal");
    if (bSmall)
        assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[3]", "mathvariant");
    else
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mi[3]", "mathvariant", u"normal");
    assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[3]/m:mi[1]", "mathvariant");
    assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[4]", "mathvariant");
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[4]/m:mi[1]", "mathvariant", u"normal");
    rDocShell.SetText(u""_ustr);
}

void MathMLExportTest::testTdf101022()
{
    loadFromURL(u"private:factory/smath"_ustr);
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());

    checkMathVariant(*pDocShell, false, true); // default mode 2

    pDocShell->SetGreekCharStyle(1); // mode 1
    checkMathVariant(*pDocShell, true, true);

    pDocShell->SetGreekCharStyle(0); // mode 0
    checkMathVariant(*pDocShell, false, false);
}

void MathMLExportTest::testMaj()
{
    loadFromURL(u"private:factory/smath"_ustr);
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    pDocShell->SetText(
        u"maj to { \u0661 } from { \U0001EE0A = \u0660 } { \u0661 over \U0001EE0A }"_ustr);
    save(u"MathML XML (Math)"_ustr);
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:munderover/m:mo", "stretchy", u"false");
    assertXPathContent(pDoc, "/m:math/m:semantics/m:mrow/m:munderover/m:mo", u"\U0001EEF0");
}

void MathMLExportTest::testHadd()
{
    loadFromURL(u"private:factory/smath"_ustr);
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    pDocShell->SetText(u"hadd to { \U0001EE4E } from { \U0001EE4E } \U0001EE4E"_ustr);
    save(u"MathML XML (Math)"_ustr);
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    assertXPathContent(pDoc, "/m:math/m:semantics/m:mrow/m:munderover/m:mi", u"\U0001EEF1");
}

void MathMLExportTest::testTdf158867()
{
    loadFromURL(u"private:factory/smath"_ustr);
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    pDocShell->SetText(u"1,2 over 2 = 0,65"_ustr);
    save(u"MathML XML (Math)"_ustr);
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    assertXPathContent(pDoc, "/m:math/m:semantics/m:mrow/m:mfrac/m:mn[1]", u"1,2");
    assertXPathContent(pDoc, "/m:math/m:semantics/m:mrow/m:mfrac/m:mn[2]", u"2");
    assertXPathContent(pDoc, "/m:math/m:semantics/m:mrow/m:mo", u"=");
    assertXPathContent(pDoc, "/m:math/m:semantics/m:mrow/m:mn", u"0,65");
}

CPPUNIT_TEST_SUITE_REGISTRATION(MathMLExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
