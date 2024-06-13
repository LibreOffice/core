/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/propertyvalue.hxx>

#include <com/sun/star/text/TextContentAnchorType.hpp>

#include <docsh.hxx>
#include <wrtsh.hxx>
#include <frmmgr.hxx>
#include <itabenum.hxx>

/// Covers sw/source/uibase/frmdlg/ fixes.
class SwUibaseFrmdlgTest : public SwModelTestBase
{
public:
    SwUibaseFrmdlgTest()
        : SwModelTestBase(u"/sw/qa/uibase/frmdlg/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUibaseFrmdlgTest, testWrappedMathObject)
{
    // The document includes a Math object with explicit wrapping.
    createSwDoc("wrapped-math-object.docx");
    uno::Reference<drawing::XShape> xMath = getShape(1);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4 (AT_CHARACTER)
    // - Actual  : 1 (AS_CHARACTER)
    // i.e. the object lost its wrapping, leading to an incorrect position.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(1), u"AnchorType"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUibaseFrmdlgTest, testAnchorTypeFromStyle)
{
    // Given a document with aGraphics style with anchor type set to as-character:
    createSwDoc();
    uno::Reference<beans::XPropertySet> xGraphics(
        getStyles(u"FrameStyles"_ustr)->getByName(u"Graphics"_ustr), uno::UNO_QUERY);
    xGraphics->setPropertyValue(u"AnchorType"_ustr,
                                uno::Any(text::TextContentAnchorType_AS_CHARACTER));

    // When inserting an image:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FileName"_ustr, createFileURL(u"image.png")),
    };
    dispatchCommand(mxComponent, u".uno:InsertGraphic"_ustr, aArgs);

    // Then make sure the image's anchor type is as-char:
    auto eActual = getProperty<text::TextContentAnchorType>(getShape(1), u"AnchorType"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1 (AS_CHARACTER)
    // - Actual  : 4 (AT_CHARACTER)
    // i.e. the anchor type from the style was ignored.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, eActual);
}

CPPUNIT_TEST_FIXTURE(SwUibaseFrmdlgTest, testInsertFrameWidth)
{
    // Given a document with an inline table, its width is set to 6000 twips:
    createSwDoc();
    // Insert a table:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/1, /*nCols=*/1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    SwTwips nExpectedWidth = 6000;
    {
        SfxItemSetFixed<RES_FRMATR_BEGIN, RES_FRMATR_END - 1> aSet(pWrtShell->GetAttrPool());
        SwFormatFrameSize aSize(SwFrameSize::Variable, nExpectedWidth);
        aSet.Put(aSize);
        pWrtShell->SetTableAttr(aSet);
    }
    pWrtShell->GoPrevCell();
    pWrtShell->Insert(u"A1"_ustr);
    SwFormatFrameSize aRowSize(SwFrameSize::Minimum);
    pWrtShell->SetRowHeight(aRowSize);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"A2"_ustr);
    pWrtShell->SetRowHeight(aRowSize);
    // Select cell:
    pWrtShell->SelAll();
    // Select table:
    pWrtShell->SelAll();

    // When converting that table to a floating table:
    SwFlyFrameAttrMgr aMgr(/*bNew=*/true, pWrtShell, Frmmgr_Type::TEXT, nullptr);

    // Then make sure that the fly width will be based on the table width:
    const SwFormatFrameSize* pFrameSize = aMgr.GetAttrSet().GetItem(RES_FRM_SIZE);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6000 (nExpectedWidth)
    // - Actual  : 1134 (2cm)
    // i.e. the fly width was the default, not inherited from the selected table.
    CPPUNIT_ASSERT_EQUAL(nExpectedWidth, pFrameSize->GetWidth());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
