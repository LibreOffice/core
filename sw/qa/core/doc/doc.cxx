/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
