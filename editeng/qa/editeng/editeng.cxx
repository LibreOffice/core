/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <memory>

#include <editeng/editeng.hxx>
#include <sfx2/app.hxx>
#include <svtools/parrtf.hxx>
#include <svtools/rtftoken.h>

#include <editdoc.hxx>
#include <eeobj.hxx>

using namespace com::sun::star;

namespace
{
/// Covers editeng/source/editeng/ fixes.
class Test : public test::BootstrapFixture
{
public:
    Test() {}

    void setUp() override
    {
        test::BootstrapFixture::setUp();
        mpItemPool = new EditEngineItemPool();
        SfxApplication::GetOrCreate();
    }

    void tearDown() override
    {
        mpItemPool.clear();
        test::BootstrapFixture::tearDown();
    }

protected:
    rtl::Reference<EditEngineItemPool> mpItemPool;
};

/// RTF parser that counts the styles in the document.
class StyleCounter : public SvRTFParser
{
public:
    StyleCounter(SvStream& rStream);
    void NextToken(int nToken) override;

    int m_nStyles = 0;
};

StyleCounter::StyleCounter(SvStream& rStream)
    : SvRTFParser(rStream)
{
}

void StyleCounter::NextToken(int nToken)
{
    if (nToken == RTF_S)
    {
        ++m_nStyles;
    }
}

CPPUNIT_TEST_FIXTURE(Test, testRTFStyleExport)
{
    // Given a document with an unreferenced style:
    EditEngine aEditEngine(mpItemPool.get());
    rtl::Reference<SfxStyleSheetPool> xStyles(new SfxStyleSheetPool(*mpItemPool));
    xStyles->Make("mystyle", SfxStyleFamily::Para);
    aEditEngine.SetStyleSheetPool(xStyles.get());
    OUString aText = u"mytest"_ustr;
    aEditEngine.SetText(aText);

    // When copying a word from that document:
    uno::Reference<datatransfer::XTransferable> xData
        = aEditEngine.CreateTransferable(ESelection(0, 0, 0, aText.getLength()));

    // Then make sure the RTF result doesn't contain the style:
    auto pData = dynamic_cast<EditDataObject*>(xData.get());
    SvMemoryStream& rStream = pData->GetRTFStream();
    tools::SvRef<StyleCounter> xReader(new StyleCounter(rStream));
    CPPUNIT_ASSERT(xReader->CallParser() != SvParserState::Error);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. unreferenced paragraph styles were exported.
    CPPUNIT_ASSERT_EQUAL(0, xReader->m_nStyles);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
