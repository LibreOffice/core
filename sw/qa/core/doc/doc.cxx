/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/classids.hxx>
#include <tools/globname.hxx>
#include <svtools/embedhlp.hxx>
#include <editeng/frmdiritem.hxx>
#include <vcl/errinf.hxx>

#include <wrtsh.hxx>
#include <fmtanchr.hxx>
#include <frameformats.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/core/doc/data/";

/// Covers sw/source/core/doc/ fixes.
class SwCoreDocTest : public SwModelTestBase
{
public:
    SwDoc* createDoc(const char* pName = nullptr);
};

SwDoc* SwCoreDocTest::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testMathInsertAnchorType)
{
    // Given an empty document.
    SwDoc* pDoc = createDoc();

    // When inserting an a math object.
    SwWrtShell* pShell = pDoc->GetDocShell()->GetWrtShell();
    SvGlobalName aGlobalName(SO3_SM_CLASSID);
    pShell->InsertObject(svt::EmbeddedObjectRef(), &aGlobalName);

    // Then the anchor type should be as-char.
    SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rFormats.size());
    const SwFrameFormat& rFormat = *rFormats[0];
    const SwFormatAnchor& rAnchor = rFormat.GetAnchor();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 4
    // i.e. the anchor type was at-char, not as-char.
    CPPUNIT_ASSERT_EQUAL(RndStdIds::FLY_AS_CHAR, rAnchor.GetAnchorId());
    ErrorRegistry::Reset();
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testTextboxTextRotateAngle)
{
    // Check the writing direction of the only TextFrame in the document.
    SwDoc* pDoc = createDoc("textbox-textrotateangle.odt");
    SwFrameFormats& rFrameFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rFrameFormats.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(RES_DRAWFRMFMT), rFrameFormats[0]->Which());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(RES_FLYFRMFMT), rFrameFormats[1]->Which());
    SvxFrameDirection eActual = rFrameFormats[1]->GetAttrSet().GetItem(RES_FRAMEDIR)->GetValue();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5 (btlr)
    // - Actual  : 0 (lrtb)
    // i.e. the writing direction was in the ODT file, but it was lost on import in the textbox
    // case.
    CPPUNIT_ASSERT_EQUAL(SvxFrameDirection::Vertical_LR_BT, eActual);
    ErrorRegistry::Reset();
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testImageHyperlinkStyle)
{
    // Given a document with an image with a hyperlink:
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xImage(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xImage, /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xImageProps(xImage, uno::UNO_QUERY);
    OUString aExpected = "http://www.example.com";
    xImageProps->setPropertyValue("HyperLinkURL", uno::makeAny(aExpected));

    // When applying a frame style on it:
    xImageProps->setPropertyValue("FrameStyleName", uno::makeAny(OUString("Frame")));

    // Then make sure that the hyperlink is not lost:
    auto aActual = getProperty<OUString>(xImageProps, "HyperLinkURL");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: http://www.example.com
    // - Actual  :
    // i.e. the link was lost, even if the frame style dialog doesn't allow specifying a link on
    // frames.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

namespace
{
/// This selection listener calls getAnchor() on selection change, which creates UNO cursors and is
/// invoked in the middle of a bookmark deletion.
struct SelectionChangeListener : public cppu::WeakImplHelper<view::XSelectionChangeListener>
{
    uno::Reference<container::XNameAccess> m_xBookmarks;
    std::vector<uno::Reference<text::XTextRange>> m_aAnchors;

public:
    SelectionChangeListener(const uno::Reference<container::XNameAccess>& xBookmarks);
    // view::XSelectionChangeListener
    void SAL_CALL selectionChanged(const lang::EventObject& rEvent) override;

    // lang::XEventListener
    void SAL_CALL disposing(const lang::EventObject& rSource) override;
};
}

SelectionChangeListener::SelectionChangeListener(
    const uno::Reference<container::XNameAccess>& xBookmarks)
    : m_xBookmarks(xBookmarks)
{
}

void SelectionChangeListener::selectionChanged(const lang::EventObject& /*rEvent*/)
{
    uno::Sequence<OUString> aElementNames = m_xBookmarks->getElementNames();
    for (const auto& rName : aElementNames)
    {
        uno::Reference<text::XTextContent> xTextContent(m_xBookmarks->getByName(rName),
                                                        uno::UNO_QUERY);
        m_aAnchors.push_back(xTextContent->getAnchor());
    }
}

void SelectionChangeListener::disposing(const lang::EventObject& /*rSource*/) {}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testBookmarkDeleteListeners)
{
    // Given a document with 2 bookmarks:
    createDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    {
        xText->insertString(xCursor, "test", /*bAbsorb=*/false);
        xCursor->gotoStart(/*bExpand=*/false);
        xCursor->gotoEnd(/*bExpand=*/true);
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextContent> xBookmark(
            xFactory->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
        uno::Reference<container::XNamed> xBookmarkNamed(xBookmark, uno::UNO_QUERY);
        xBookmarkNamed->setName("mybookmark");
        xText->insertTextContent(xCursor, xBookmark, /*bAbsorb=*/true);
    }
    {
        xCursor->gotoEnd(/*bExpand=*/false);
        xText->insertString(xCursor, "test2", /*bAbsorb=*/false);
        xCursor->goLeft(4, /*bExpand=*/true);
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextContent> xBookmark(
            xFactory->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
        uno::Reference<container::XNamed> xBookmarkNamed(xBookmark, uno::UNO_QUERY);
        xBookmarkNamed->setName("mybookmark2");
        xText->insertTextContent(xCursor, xBookmark, /*bAbsorb=*/true);
    }
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarks = xBookmarksSupplier->getBookmarks();

    // When registering a selection listener that creates uno marks:
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xController(xModel->getCurrentController(),
                                                         uno::UNO_QUERY);
    xController->addSelectionChangeListener(new SelectionChangeListener(xBookmarks));

    // Then make sure that deleting a bookmark doesn't crash:
    uno::Reference<lang::XComponent> xBookmark(xBookmarks->getByName("mybookmark2"),
                                               uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have crashed, an invalidated iterator
    // was used with erase().
    xBookmark->dispose();
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
