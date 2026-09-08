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
#include <sfx2/app.hxx>
#include <tools/gen.hxx>
#include <vcl/wrkwin.hxx>

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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
