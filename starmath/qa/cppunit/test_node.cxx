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

#include <o3tl/cppunittraitshelper.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <vcl/virdev.hxx>

#include <document.hxx>
#include <smdll.hxx>
#include <node.hxx>
#include <parse5.hxx>
#include <utility.hxx>

#include <memory>

namespace {

using namespace ::com::sun::star;

typedef rtl::Reference<SmDocShell> SmDocShellRef;

class NodeTest : public test::BootstrapFixture
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

private:
    void testTdf47813();
    void CHECK_GREEK_SYMBOL(OUString const & text, sal_Unicode code, bool bItalic);
    void testTdf52225();

    CPPUNIT_TEST_SUITE(NodeTest);
    CPPUNIT_TEST(testTdf47813);
    CPPUNIT_TEST(testTdf52225);
    CPPUNIT_TEST_SUITE_END();

    SmDocShellRef mxDocShell;
};

void NodeTest::setUp()
{
    BootstrapFixture::setUp();
    SmGlobals::ensure();
    mxDocShell = new SmDocShell(SfxModelFlags::EMBEDDED_OBJECT |
                                SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
                                SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    mxDocShell->DoInitNew();
}

void NodeTest::tearDown()
{
    if (mxDocShell.is())
        mxDocShell->DoClose();
    BootstrapFixture::tearDown();
}

void NodeTest::testTdf47813()
{
    SmParser5 aParser;
#define MATRIX "matrix {-2#33##4#-5##6,0#7}"
    auto pNodeA = aParser.Parse(u"" MATRIX ""_ustr);
    auto pNodeC = aParser.Parse(u"alignc " MATRIX ""_ustr);
    auto pNodeL = aParser.Parse(u"alignl " MATRIX ""_ustr);
    auto pNodeR = aParser.Parse(u"alignr " MATRIX ""_ustr);
#undef MATRIX
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;
    SmFormat aFmt;
    pNodeA->Prepare(aFmt, *mxDocShell, 0);
    pNodeA->Arrange(*pOutputDevice, aFmt);
    pNodeC->Prepare(aFmt, *mxDocShell, 0);
    pNodeC->Arrange(*pOutputDevice, aFmt);
    pNodeL->Prepare(aFmt, *mxDocShell, 0);
    pNodeL->Arrange(*pOutputDevice, aFmt);
    pNodeR->Prepare(aFmt, *mxDocShell, 0);
    pNodeR->Arrange(*pOutputDevice, aFmt);
    tools::Long nWidthA = pNodeA->GetRect().GetWidth();
    tools::Long nWidthC = pNodeC->GetRect().GetWidth();
    tools::Long nWidthL = pNodeL->GetRect().GetWidth();
    tools::Long nWidthR = pNodeR->GetRect().GetWidth();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, nWidthC/static_cast<double>(nWidthA), 0.01);
    // these values appear to change slightly with display scaling
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, nWidthL/static_cast<double>(nWidthA), 0.03);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, nWidthR/static_cast<double>(nWidthA), 0.03);
}

void NodeTest::CHECK_GREEK_SYMBOL(OUString const & text, sal_Unicode code, bool bItalic) {
    mxDocShell->SetText(text);
    const SmTableNode *pTree= mxDocShell->GetFormulaTree();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pTree->GetNumSubNodes());
    const SmNode *pLine = pTree->GetSubNode(0);
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SmNodeType::Line, pLine->GetType());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pLine->GetNumSubNodes());
    const SmNode *pNode = pLine->GetSubNode(0);
    CPPUNIT_ASSERT(pNode);
    CPPUNIT_ASSERT_EQUAL(SmNodeType::Special, pNode->GetType());
    const SmSpecialNode *pSn = static_cast<const SmSpecialNode *>(pNode);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pSn->GetText().getLength());
    CPPUNIT_ASSERT_EQUAL(code, pSn->GetText()[0]);
    CPPUNIT_ASSERT_EQUAL(text, pSn->GetToken().aText);
    CPPUNIT_ASSERT_EQUAL(bItalic, IsItalic(pSn->GetFont()));
}

void NodeTest::testTdf52225()
{
    SmFormat aFormat = mxDocShell->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), aFormat.GetGreekCharStyle()); // default format = 2
    CHECK_GREEK_SYMBOL(u"%ALPHA"_ustr, u'\x0391', false);
    CHECK_GREEK_SYMBOL(u"%iALPHA"_ustr, u'\x0391', true);
    CHECK_GREEK_SYMBOL(u"%alpha"_ustr, u'\x03b1', true);
    CHECK_GREEK_SYMBOL(u"%ialpha"_ustr, u'\x03b1', true);

    // mode 1
    aFormat.SetGreekCharStyle(1);
    mxDocShell->SetFormat(aFormat);
    CHECK_GREEK_SYMBOL(u"%BETA"_ustr, u'\x0392', true);
    CHECK_GREEK_SYMBOL(u"%iBETA"_ustr, u'\x0392', true);
    CHECK_GREEK_SYMBOL(u"%beta"_ustr, u'\x03b2', true);
    CHECK_GREEK_SYMBOL(u"%ibeta"_ustr, u'\x03b2', true);

    // mode 0
    aFormat.SetGreekCharStyle(0);
    mxDocShell->SetFormat(aFormat);
    CHECK_GREEK_SYMBOL(u"%GAMMA"_ustr, u'\x0393', false);
    CHECK_GREEK_SYMBOL(u"%iGAMMA"_ustr, u'\x0393', true);
    CHECK_GREEK_SYMBOL(u"%gamma"_ustr, u'\x03b3', false);
    CHECK_GREEK_SYMBOL(u"%igamma"_ustr, u'\x03b3', true);

#undef CHECK_GREEK_SYMBOL
}

CPPUNIT_TEST_SUITE_REGISTRATION(NodeTest);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
