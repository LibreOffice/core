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

#include <sfx2/sfxmodelfactory.hxx>
#include <vcl/virdev.hxx>

#include <document.hxx>
#include <smdll.hxx>
#include <node.hxx>
#include <parse.hxx>
#include <utility.hxx>

#include <memory>

namespace {

using namespace ::com::sun::star;

typedef tools::SvRef<SmDocShell> SmDocShellRef;

class NodeTest : public test::BootstrapFixture
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

private:
    void testTdf47813();
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
}

void NodeTest::tearDown()
{
    if (mxDocShell.is())
        mxDocShell->DoClose();
    BootstrapFixture::tearDown();
}

void NodeTest::testTdf47813()
{
    SmParser aParser;
#define MATRIX "matrix {-2#33##4#-5##6,0#7}"
    auto pNodeA = aParser.Parse(MATRIX);
    auto pNodeC = aParser.Parse("alignc " MATRIX);
    auto pNodeL = aParser.Parse("alignl " MATRIX);
    auto pNodeR = aParser.Parse("alignr " MATRIX);
#undef MATRIX
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;
    SmFormat aFmt;
    pNodeA->Arrange(*pOutputDevice, aFmt);
    pNodeC->Arrange(*pOutputDevice, aFmt);
    pNodeL->Arrange(*pOutputDevice, aFmt);
    pNodeR->Arrange(*pOutputDevice, aFmt);
    long nWidthA = pNodeA->GetRect().GetWidth();
    long nWidthC = pNodeC->GetRect().GetWidth();
    long nWidthL = pNodeL->GetRect().GetWidth();
    long nWidthR = pNodeR->GetRect().GetWidth();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, nWidthC/static_cast<double>(nWidthA), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, nWidthL/static_cast<double>(nWidthA), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, nWidthR/static_cast<double>(nWidthA), 0.01);
}

void NodeTest::testTdf52225()
{
#define CHECK_GREEK_SYMBOL(text, code, bItalic) do {                    \
        mxDocShell->SetText(text);                                      \
        const SmTableNode *pTree= mxDocShell->GetFormulaTree();         \
        CPPUNIT_ASSERT_EQUAL(size_t(1), pTree->GetNumSubNodes());       \
        const SmNode *pLine = pTree->GetSubNode(0);                     \
        CPPUNIT_ASSERT(pLine);                                          \
        CPPUNIT_ASSERT_EQUAL(SmNodeType::Line, pLine->GetType());       \
        CPPUNIT_ASSERT_EQUAL(size_t(1), pLine->GetNumSubNodes());       \
        const SmNode *pNode = pLine->GetSubNode(0);                     \
        CPPUNIT_ASSERT(pNode);                                          \
        CPPUNIT_ASSERT_EQUAL(SmNodeType::Special, pNode->GetType());    \
        const SmSpecialNode *pSn = static_cast<const SmSpecialNode *>(pNode); \
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pSn->GetText().getLength()); \
        CPPUNIT_ASSERT_EQUAL(code, pSn->GetText()[0]);     \
        CPPUNIT_ASSERT_EQUAL(OUString(text), pSn->GetToken().aText);    \
        CPPUNIT_ASSERT_EQUAL(bItalic, IsItalic(pSn->GetFont()));        \
    } while (false)

    SmFormat aFormat = mxDocShell->GetFormat();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), aFormat.GetGreekCharStyle()); // default format = 2
    CHECK_GREEK_SYMBOL("%ALPHA", u'\x0391', false);
    CHECK_GREEK_SYMBOL("%iALPHA", u'\x0391', true);
    CHECK_GREEK_SYMBOL("%alpha", u'\x03b1', true);
    CHECK_GREEK_SYMBOL("%ialpha", u'\x03b1', true);

    // mode 1
    aFormat.SetGreekCharStyle(1);
    mxDocShell->SetFormat(aFormat);
    CHECK_GREEK_SYMBOL("%BETA", u'\x0392', true);
    CHECK_GREEK_SYMBOL("%iBETA", u'\x0392', true);
    CHECK_GREEK_SYMBOL("%beta", u'\x03b2', true);
    CHECK_GREEK_SYMBOL("%ibeta", u'\x03b2', true);

    // mode 0
    aFormat.SetGreekCharStyle(0);
    mxDocShell->SetFormat(aFormat);
    CHECK_GREEK_SYMBOL("%GAMMA", u'\x0393', false);
    CHECK_GREEK_SYMBOL("%iGAMMA", u'\x0393', true);
    CHECK_GREEK_SYMBOL("%gamma", u'\x03b3', false);
    CHECK_GREEK_SYMBOL("%igamma", u'\x03b3', true);

#undef CHECK_GREEK_SYMBOL
}

CPPUNIT_TEST_SUITE_REGISTRATION(NodeTest);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
