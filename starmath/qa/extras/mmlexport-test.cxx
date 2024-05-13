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

    CPPUNIT_TEST_SUITE(MathMLExportTest);
    CPPUNIT_TEST(testBlank);
    CPPUNIT_TEST(testTdf97049);
    CPPUNIT_TEST(testTdf101022);
    CPPUNIT_TEST(testMaj);
    CPPUNIT_TEST(testHadd);
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
    mxComponent = loadFromDesktop(u"private:factory/smath"_ustr);
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    pDocShell->SetText(u"x`y~~z"_ustr);
    save(u"MathML XML (Math)"_ustr);
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mspace[1]"_ostr, "width"_ostr, u"0.5em"_ustr);
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mspace[2]"_ostr, "width"_ostr, u"4em"_ustr);
}

void MathMLExportTest::testTdf97049()
{
    mxComponent = loadFromDesktop(u"private:factory/smath"_ustr);
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    pDocShell->SetText(u"intd {{1 over x} dx}"_ustr);
    save(u"MathML XML (Math)"_ustr);
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mo[1]"_ostr, "stretchy"_ostr, u"true"_ustr);
    auto aContent = getXPathContent(pDoc, "/m:math/m:semantics/m:mrow/m:mo[1]"_ostr);
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
        assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[1]"_ostr, "mathvariant"_ostr);
    else
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mi[1]"_ostr, "mathvariant"_ostr,
                    u"normal"_ustr);
    assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[1]/m:mi[1]"_ostr,
                           "mathvariant"_ostr);
    assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[2]"_ostr, "mathvariant"_ostr);
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[2]/m:mi[1]"_ostr, "mathvariant"_ostr,
                u"normal"_ustr);
    if (bSmall)
        assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[3]"_ostr, "mathvariant"_ostr);
    else
        assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mi[3]"_ostr, "mathvariant"_ostr,
                    u"normal"_ustr);
    assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[3]/m:mi[1]"_ostr,
                           "mathvariant"_ostr);
    assertXPathNoAttribute(pDoc, "/m:math/m:semantics/m:mrow/m:mi[4]"_ostr, "mathvariant"_ostr);
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:mstyle[4]/m:mi[1]"_ostr, "mathvariant"_ostr,
                u"normal"_ustr);
    rDocShell.SetText(u""_ustr);
}

void MathMLExportTest::testTdf101022()
{
    mxComponent = loadFromDesktop(u"private:factory/smath"_ustr);
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
    mxComponent = loadFromDesktop(u"private:factory/smath"_ustr);
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    pDocShell->SetText(
        u"maj to { \u0661 } from { \U0001EE0A = \u0660 } { \u0661 over \U0001EE0A }"_ustr);
    save(u"MathML XML (Math)"_ustr);
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/m:math/m:semantics/m:mrow/m:munderover/m:mo"_ostr, "stretchy"_ostr,
                u"false"_ustr);
    assertXPathContent(pDoc, "/m:math/m:semantics/m:mrow/m:munderover/m:mo"_ostr,
                       u"\U0001EEF0"_ustr);
}

void MathMLExportTest::testHadd()
{
    mxComponent = loadFromDesktop(u"private:factory/smath"_ustr);
    SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    pDocShell->SetText(u"hadd to { \U0001EE4E } from { \U0001EE4E } \U0001EE4E"_ustr);
    save(u"MathML XML (Math)"_ustr);
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    assertXPathContent(pDoc, "/m:math/m:semantics/m:mrow/m:munderover/m:mi"_ostr,
                       u"\U0001EEF1"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(MathMLExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
