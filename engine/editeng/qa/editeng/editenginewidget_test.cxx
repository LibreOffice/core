/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <editeng/editeng.hxx>
#include <editeng/editenginewidget.hxx>
#include <editeng/editview.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <o3tl/unit_conversion.hxx>
#include <sfx2/app.hxx>
#include <tools/gen.hxx>
#include <tools/json_writer.hxx>
#include <tools/mapunit.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/wrkwin.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <sstream>

#include <editdoc.hxx>

namespace
{
/// Covers editeng/source/editeng/editenginewidget.cxx.
class EditEngineWidgetTest : public test::BootstrapFixture
{
public:
    EditEngineWidgetTest() {}

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

CPPUNIT_TEST_FIXTURE(EditEngineWidgetTest, testSelectionBelowParagraphStart)
{
    // Given a widget on a paragraph of text.
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetPaperSize(Size(5000, 5000));
    aEditEngine.SetText(u"hello world"_ustr);

    ScopedVclPtrInstance<WorkWindow> xWin(nullptr, WB_APP | WB_STDWORK);
    EditView aEditView(aEditEngine, xWin.get());
    aEditEngine.InsertView(&aEditView);

    EditEngineWidgetController aController(aEditView);

    // When a client names an index below the start of the paragraph.
    aController.HandleCustomEvent(
        u"selection"_ustr,
        u"{\"startPara\":0,\"startIndex\":-5,\"endPara\":0,\"endIndex\":2}"_ustr);

    // Then the selection stands at the start of the paragraph and not before it. The edit
    // engine clamps an index that reaches past the end of a paragraph but not one below its
    // start, so without the accompanying fix the selection held an index of -5 and the next
    // edit of it read and wrote before the text.
    const ESelection aSelection = aEditView.GetSelection();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aSelection.start.nPara);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aSelection.start.nIndex);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aSelection.end.nPara);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aSelection.end.nIndex);

    aEditEngine.RemoveView(&aEditView);
}

CPPUNIT_TEST_FIXTURE(EditEngineWidgetTest, testCtrlASelectsAllText)
{
    // Given a widget on two paragraphs of text, with the caret inside the first one.
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetPaperSize(Size(5000, 5000));
    aEditEngine.SetText(u"hello\nworld"_ustr);

    ScopedVclPtrInstance<WorkWindow> xWin(nullptr, WB_APP | WB_STDWORK);
    EditView aEditView(aEditEngine, xWin.get());
    aEditEngine.InsertView(&aEditView);
    aEditView.SetSelection(ESelection(0, 2));

    EditEngineWidgetController aController(aEditView);

    // When the client sends Ctrl+A.
    const OUString aKey = "{\"keyCode\":" + OUString::number(KEY_MOD1 | KEY_A)
                          + ",\"charCode\":0,\"repeat\":0}";
    aController.HandleCustomEvent(u"key"_ustr, aKey);

    // Then the whole text is selected.
    const ESelection aSelection = aEditView.GetSelection();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aSelection.start.nPara);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aSelection.start.nIndex);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aSelection.end.nPara);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), aSelection.end.nIndex);

    aEditEngine.RemoveView(&aEditView);
}

CPPUNIT_TEST_FIXTURE(EditEngineWidgetTest, testParagraphCarriesTheFontInEffect)
{
    // Given a paragraph whose font size comes from the paragraph formatting and whose font family
    // is the default, with no formatting on the text itself.
    EditEngine aEditEngine(mpItemPool.get());
    aEditEngine.SetPaperSize(Size(5000, 5000));
    aEditEngine.SetText(u"hello"_ustr);

    const o3tl::Length eLength = MapToO3tlLength(mpItemPool->GetMetric(EE_CHAR_FONTHEIGHT));
    SfxItemSet aParagraphSet(aEditEngine.GetEmptyItemSet());
    aParagraphSet.Put(SvxFontHeightItem(o3tl::convert(20, o3tl::Length::pt, eLength), 100,
                                        EE_CHAR_FONTHEIGHT));
    aEditEngine.SetParaAttribs(0, aParagraphSet);

    ScopedVclPtrInstance<WorkWindow> xWin(nullptr, WB_APP | WB_STDWORK);
    EditView aEditView(aEditEngine, xWin.get());
    aEditEngine.InsertView(&aEditView);

    EditEngineWidgetController aController(aEditView);

    // When the widget writes its model for the client.
    tools::JsonWriter aWriter;
    aController.DumpWidgetData(aWriter);
    std::stringstream aStream(std::string(aWriter.finishAndGetAsOString()));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);

    // Then the paragraph names the size and the family the text is shown in. Without the
    // accompanying fix in place, both were missing, because only the formatting of the text itself
    // was written, so the client showed the text in a font of its own.
    const boost::property_tree::ptree& rParagraph = aTree.get_child("paragraphs").front().second;
    CPPUNIT_ASSERT_EQUAL(20, rParagraph.get<int>("size", 0));
    const OUString aDefaultFamily
        = aEditEngine.GetEmptyItemSet().Get(EE_CHAR_FONTINFO).GetFamilyName();
    CPPUNIT_ASSERT(!aDefaultFamily.isEmpty());
    CPPUNIT_ASSERT_EQUAL(aDefaultFamily,
                         OUString::fromUtf8(rParagraph.get<std::string>("family", std::string())));

    aEditEngine.RemoveView(&aEditView);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
