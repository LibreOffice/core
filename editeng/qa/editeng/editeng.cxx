/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <editeng/editeng.hxx>
#include <editeng/wghtitem.hxx>
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
    std::vector<int> m_aStyleValues;
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
        m_aStyleValues.push_back(nTokenValue);
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

CPPUNIT_TEST_FIXTURE(Test, testRTFStyleExportReferToStyle)
{
    // Given a document with one unused and one used style:
    EditEngine aEditEngine(mpItemPool.get());
    rtl::Reference<SfxStyleSheetPool> xStyles(new SfxStyleSheetPool(*mpItemPool));
    xStyles->Make("mystyle", SfxStyleFamily::Para);
    xStyles->Make("mystyle2", SfxStyleFamily::Para);
    auto pStyle = static_cast<SfxStyleSheet*>(xStyles->Find("mystyle2", SfxStyleFamily::Para));
    aEditEngine.SetStyleSheetPool(xStyles.get());
    OUString aText = u"mytest"_ustr;
    aEditEngine.SetText(aText);
    aEditEngine.SetStyleSheet(0, pStyle);

    // When copying a word from that document:
    uno::Reference<datatransfer::XTransferable> xData
        = aEditEngine.CreateTransferable(ESelection(0, 0, 0, aText.getLength()));

    // Then make sure the declared and referred style indexes for the used style match:
    auto pData = dynamic_cast<EditDataObject*>(xData.get());
    SvMemoryStream& rStream = pData->GetRTFStream();
    tools::SvRef<StyleCounter> xReader(new StyleCounter(rStream));
    CPPUNIT_ASSERT(xReader->CallParser() != SvParserState::Error);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), xReader->m_aStyleValues.size());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. \s2 was used to refer to \s1, so the paragraph style was lost.
    CPPUNIT_ASSERT_EQUAL(xReader->m_aStyleValues[0], xReader->m_aStyleValues[1]);
}

CPPUNIT_TEST_FIXTURE(Test, testRTFStyleExportParentRecursive)
{
    // Given a document with text that has a paragraph style with a parent that itself has a parent:
    EditEngine aEditEngine(mpItemPool.get());
    rtl::Reference<SfxStyleSheetPool> xStyles(new SfxStyleSheetPool(*mpItemPool));
    xStyles->Make("mystyle1", SfxStyleFamily::Para);
    xStyles->Make("mystyle2", SfxStyleFamily::Para);
    xStyles->Make("mystyle3", SfxStyleFamily::Para);
    auto pStyle1 = static_cast<SfxStyleSheet*>(xStyles->Find("mystyle1", SfxStyleFamily::Para));
    auto pStyle2 = static_cast<SfxStyleSheet*>(xStyles->Find("mystyle2", SfxStyleFamily::Para));
    pStyle2->SetParent(pStyle1->GetName());
    auto pStyle3 = static_cast<SfxStyleSheet*>(xStyles->Find("mystyle3", SfxStyleFamily::Para));
    pStyle3->SetParent(pStyle2->GetName());
    pStyle3->GetItemSet().SetRanges(svl::Items<WEIGHT_BOLD, EE_CHAR_WEIGHT>);
    SvxWeightItem aItem(WEIGHT_BOLD, EE_CHAR_WEIGHT);
    pStyle3->GetItemSet().Put(aItem);
    aEditEngine.SetStyleSheetPool(xStyles.get());
    OUString aText = u"mytest"_ustr;
    aEditEngine.SetText(aText);
    aEditEngine.SetStyleSheet(0, pStyle3);

    // When copying to the clipboard as RTF:
    // Without the accompanying fix in place, this test would have crashed here:
    uno::Reference<datatransfer::XTransferable> xData
        = aEditEngine.CreateTransferable(ESelection(0, 0, 0, aText.getLength()));

    // Then make sure we produce RTF and not crash:
    auto pData = dynamic_cast<EditDataObject*>(xData.get());
    SvMemoryStream& rStream = pData->GetRTFStream();
    CPPUNIT_ASSERT_GREATER(static_cast<sal_uInt64>(0), rStream.remainingSize());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
