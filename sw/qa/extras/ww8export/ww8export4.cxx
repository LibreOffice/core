/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/string_view.hxx>
#include <svx/svdpage.hxx>

#include <docsh.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <unotxdoc.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf77964)
{
    loadAndReload("tdf77964.doc");
    // both images were loading as AT_PARA instead of AS_CHAR. Image2 visually had text wrapping.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, getProperty<text::TextContentAnchorType>(getShapeByName(u"Image2"), "AnchorType"));
}

DECLARE_WW8EXPORT_TEST(testTdf150197_anlv2ListFormat, "tdf150197_anlv2ListFormat.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(getParagraph(2), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString("2."), getProperty<OUString>(getParagraph(3), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Did you fix me? I should be 2.1", OUString("4.1"),
                                 getProperty<OUString>(getParagraph(4), "ListLabelString"));
}

DECLARE_WW8EXPORT_TEST(testTdf117994_CRnumformatting, "tdf117994_CRnumformatting.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("1."), parseDump("//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']", "expand"));
    //Without this fix in place, it would become 200 (and non-bold).
    CPPUNIT_ASSERT_EQUAL(OUString("160"), parseDump("//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']", "font-height"));
}

DECLARE_WW8EXPORT_TEST(testTdf155465_paraAdjustDistribute, "tdf155465_paraAdjustDistribute.doc")
{
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 2; Actual  : 0', i.e. the first paragraph's ParaAdjust was left, not block.
    const style::ParagraphAdjust eBlock = style::ParagraphAdjust_BLOCK;
    auto nAdjust = getProperty<sal_Int16>(getParagraph(1), "ParaAdjust");
    CPPUNIT_ASSERT_EQUAL(eBlock, static_cast<style::ParagraphAdjust>(nAdjust));

    nAdjust = getProperty<sal_Int16>(getParagraph(1), "ParaLastLineAdjust");
    CPPUNIT_ASSERT_EQUAL(eBlock, static_cast<style::ParagraphAdjust>(nAdjust));

    nAdjust = getProperty<sal_Int16>(getParagraph(2), "ParaAdjust");
    CPPUNIT_ASSERT_EQUAL(eBlock, static_cast<style::ParagraphAdjust>(nAdjust));

    nAdjust = getProperty<sal_Int16>(getParagraph(2), "ParaLastLineAdjust");
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT, static_cast<style::ParagraphAdjust>(nAdjust));
}

DECLARE_WW8EXPORT_TEST(testTdf104704_mangledFooter, "tdf104704_mangledFooter.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testEmptyGroup)
{
    // Given a document with an empty group
    createSwDoc("empty_group.docx");

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);

    CPPUNIT_ASSERT_EQUAL(OUString("Empty group"), pObject->GetName());
    CPPUNIT_ASSERT(pObject->IsGroupObject());
    CPPUNIT_ASSERT_EQUAL(size_t(0), pObject->GetSubList()->GetObjCount());

    // it must not assert/crash on save
    saveAndReload(OUString::createFromAscii(mpFilter));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
