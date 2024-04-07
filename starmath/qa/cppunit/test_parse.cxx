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

#include <document.hxx>
#include <smdll.hxx>
#include <node.hxx>
#include <parse5.hxx>

#include <memory>

namespace {

using namespace ::com::sun::star;

typedef rtl::Reference<SmDocShell> SmDocShellRef;

class ParseTest : public test::BootstrapFixture
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

private:
    void testMinus();
    void testNospace();

    CPPUNIT_TEST_SUITE(ParseTest);
    CPPUNIT_TEST(testMinus);
    CPPUNIT_TEST(testNospace);
    CPPUNIT_TEST_SUITE_END();

    SmDocShellRef mxDocShell;
};

void ParseTest::setUp()
{
    BootstrapFixture::setUp();
    SmGlobals::ensure();
    mxDocShell = new SmDocShell(SfxModelFlags::EMBEDDED_OBJECT |
                                SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
                                SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
}

void ParseTest::tearDown()
{
    if (mxDocShell.is())
        mxDocShell->DoClose();
    BootstrapFixture::tearDown();
}

/*
 * This shows that input "-" is recognized as a separate token even when
 * it is immediately followed by a number.
 */
void ParseTest::testMinus()
{
    auto pNode = SmParser5().Parse("-1.2");
    CPPUNIT_ASSERT_EQUAL(size_t(1), pNode->GetNumSubNodes());
    const SmNode *pNode0 = pNode->GetSubNode(0);
    CPPUNIT_ASSERT(pNode0);
    CPPUNIT_ASSERT_EQUAL(SmNodeType::Line, pNode0->GetType());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pNode0->GetNumSubNodes());
    const SmNode *pNode00 = pNode0->GetSubNode(0);
    CPPUNIT_ASSERT(pNode00);
    CPPUNIT_ASSERT_EQUAL(SmNodeType::UnHor, pNode00->GetType());
    CPPUNIT_ASSERT_EQUAL(size_t(2), pNode00->GetNumSubNodes());
    const SmNode *pNode000 = pNode00->GetSubNode(0);
    CPPUNIT_ASSERT(pNode000);
    CPPUNIT_ASSERT_EQUAL(SmNodeType::Math, pNode000->GetType());
    // GetText() vs GetToken().aText
    CPPUNIT_ASSERT_EQUAL(OUString(MS_MINUS),
                         static_cast<const SmMathSymbolNode *>(pNode000)->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("-"),
                         static_cast<const SmMathSymbolNode *>(pNode000)->GetToken().aText);
    const SmNode *pNode001 = pNode00->GetSubNode(1);
    CPPUNIT_ASSERT(pNode001);
    CPPUNIT_ASSERT_EQUAL(SmNodeType::Text, pNode001->GetType());
    // GetText() vs GetToken().aText
    CPPUNIT_ASSERT(static_cast<const SmTextNode *>(pNode001)->GetText().isEmpty());
    CPPUNIT_ASSERT_EQUAL(OUString("1.2"),
                         static_cast<const SmTextNode *>(pNode001)->GetToken().aText);
}

/*
 * This shows that "nospace" turns off the expression's IsUseExtraSpaces(),
 * but leaves its descendants' flag on.
 */
void ParseTest::testNospace()
{
    auto pNode = SmParser5().Parse("nospace{ nitalic d {F(x) G(x)} }");
    CPPUNIT_ASSERT_EQUAL(size_t(1), pNode->GetNumSubNodes());
    const SmNode *pNode0 = pNode->GetSubNode(0);
    CPPUNIT_ASSERT(pNode0);
    CPPUNIT_ASSERT_EQUAL(SmNodeType::Line, pNode0->GetType());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pNode0->GetNumSubNodes());
    const SmNode *pNode00 = pNode0->GetSubNode(0);
    CPPUNIT_ASSERT(pNode00);
    CPPUNIT_ASSERT_EQUAL(SmNodeType::Expression, pNode00->GetType());
    CPPUNIT_ASSERT(!static_cast<const SmExpressionNode *>(pNode00)->IsUseExtraSpaces());
    CPPUNIT_ASSERT_EQUAL(size_t(2), pNode00->GetNumSubNodes());
    const SmNode *pNode000 = pNode00->GetSubNode(0);
    CPPUNIT_ASSERT(pNode000);
    CPPUNIT_ASSERT_EQUAL(SmNodeType::Font, pNode000->GetType());
    CPPUNIT_ASSERT_EQUAL(OUString("nitalic"),
                         static_cast<const SmFontNode *>(pNode000)->GetToken().aText);
    const SmNode *pNode001 = pNode00->GetSubNode(1);
    CPPUNIT_ASSERT(pNode001);
    CPPUNIT_ASSERT_EQUAL(SmNodeType::Expression, pNode001->GetType());
    CPPUNIT_ASSERT(static_cast<const SmExpressionNode *>(pNode001)->IsUseExtraSpaces());
    CPPUNIT_ASSERT_EQUAL(size_t(2), pNode00->GetNumSubNodes());
}

CPPUNIT_TEST_SUITE_REGISTRATION(ParseTest);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
