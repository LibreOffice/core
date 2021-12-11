
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <string_view>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <vcl/errcode.hxx>
#include <tools/gen.hxx>
#include <swmodeltestbase.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <shellio.hxx>
#include <expfld.hxx>
#include <drawdoc.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <section.hxx>
#include <fmtclds.hxx>
#include <dcontact.hxx>
#include <textboxhelper.hxx>
#include <view.hxx>
#include <hhcwrp.hxx>
#include <swacorr.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <charatr.hxx>
#include <editeng/acorrcfg.hxx>
#include <unotools/streamwrap.hxx>
#include <unocrsr.hxx>
#include <unocrsrhelper.hxx>
#include <unotbl.hxx>
#include <IMark.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <pagedesc.hxx>
#include <PostItMgr.hxx>
#include <AnnotationWin.hxx>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <vcl/TypeSerializer.hxx>

#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svl/itemiter.hxx>
#include <svx/svxids.hrc>
#include <unotools/localfilehelper.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <i18nutil/transliteration.hxx>
#include <i18nutil/searchopt.hxx>
#include <reffld.hxx>
#include <dbfld.hxx>
#include <txatbase.hxx>
#include <txtftn.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentState.hxx>
#include <unofldmid.h>
#include <UndoManager.hxx>
#include <textsh.hxx>
#include <frmatr.hxx>
#include <frmmgr.hxx>
#include <tblafmt.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchAlgorithms.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/util/XPropertyReplace.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <o3tl/cppunittraitshelper.hxx>
#include <o3tl/unit_conversion.hxx>
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <paratr.hxx>
#include <drawfont.hxx>
#include <txtfrm.hxx>
#include <txttypes.hxx>
#include <SwPortionHandler.hxx>
#include <hyp.hxx>
#include <swdtflvr.hxx>
#include <editeng/svxenum.hxx>
#include <comphelper/propertysequence.hxx>
#include <sfx2/classificationhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/docfilt.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <comphelper/configurationhelper.hxx>
#include <editeng/unolingu.hxx>
#include <vcl/scheduler.hxx>
#include <config_features.h>
#include <sfx2/watermarkitem.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <test/htmltesttools.hxx>
#include <fmthdft.hxx>
#include <iodetect.hxx>
#include <wrthtml.hxx>
#include <dbmgr.hxx>
#include <frameformats.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <comphelper/processfactory.hxx>
#include <rootfrm.hxx>

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/uiwriter/data/";

sal_Int32 lcl_getAttributeIDFromHints(const SwpHints& hints)
{
    for (size_t i = 0; i < hints.Count(); ++i)
    {
        const SwTextAttr* hint = hints.Get(i);
        if (hint->Which() == RES_TXTATR_AUTOFMT)
        {
            const SwFormatAutoFormat& rFmt = hint->GetAutoFormat();
            SfxItemIter aIter(*rFmt.GetStyleHandle());
            return aIter.GetCurItem()->Which();
        }
    }
    return -1;
}
}

class SwUiWriterTest4 : public SwModelTestBase, public HtmlTestTools
{
public:
    void testTdf96515();
    void testTdf96943();
    void testTdf96536();
    void testTdf96479();
    void testBookmarkCollapsed();
    void testRemoveBookmarkText();
    void testRemoveBookmarkTextAndAddNew();
    void testRemoveBookmarkTextAndAddNewAfterReload();
    void testTdf96961();
    void testTdf88453();
    void testTdf88453Table();
    void testClassificationPaste();
    void testSmallCaps();
    void testTdf98987();
    void testTdf99004();
    void testTdf84695();
    void testTdf84695NormalChar();
    void testTdf84695Tab();
    void testTableStyleUndo();
    void testRedlineCopyPaste();
    void testTdf135260();
    void testRedlineParam();
    void testRedlineViewAuthor();
    void testTdf91292();
    void testTdf78727();
    void testRedlineTimestamp();
    void testCursorWindows();
    void testLandscape();
    void testTdf95699();
    void testTdf104032();
    void testTdf104440();
    void testTdf104425();
    void testTdf104814();
    void testTableRedlineRedoCrash();
    void testTdf66405();
    void testTdf35021_tabOverMarginDemo();
    void testTdf106701_tabOverMarginAutotab();
    void testTdf104492();
    void testTdf107025();
    void testTdf107362();
    void testTdf105417();
    void testTdf105625();
    void testTdf125151_protected();
    void testTdf125151_protectedB();
    void testTdf106736();
    void testTdf58604();
    void testTdf112025();
    void testTdf72942();

    void testTdf113877_mergeDocs(const char* aDestDoc, const char* aInsertDoc);
    void testTdf113877();
    void testTdf113877NoMerge();
    void testTdf113877_default_style();
    void testTdf113877_Standard_style();
    void testTdf113877_blank_bold_on();
    void testTdf113877_blank_bold_off();

    void testMsWordCompTrailingBlanks();
    void testCreateDocxAnnotation();
    void testTdf107976();
    void testTdf142157();
    void testTdf116640();
    void testTdf108524();
    void testRhbz1810732();
    void testTableInSection();
    void testTableInNestedSection();
    void testTableInSectionInTable();
    void testSectionInTableInTable();
    void testSectionInTableInTable2();
    void testSectionInTableInTable3();
    void testSectionInTableInTable4();
    void testTdf112160();
    void testLinesMoveBackwardsInSectionInTable();
    void testTdf112741();
    void testTdf112860();
    void testTdf113287();
    void testTdf113445();
    void testTdf113686();
    void testFontEmbedding();
    void testLinesInSectionInTable();
    void testParagraphOfTextRange();
    void testTdf99689TableOfContents();
    void testTdf99689TableOfFigures();
    void testTdf99689TableOfTables();
    void testTdf112448();
    void testTdf113790();
    void testTdf108048();
    void testTdf114306();
    void testTdf114306_2();
    void testTdf113481();
    void testTdf115013();
    void testTdf114536();
    void testTdf115065();
    void testTdf115132();
    void testXDrawPagesSupplier();
    void testTdf116403();
    void testHtmlCopyImages();
    void testTdf116789();
    void testTdf91801();
    void testTdf51223();
    void testTdf108423();
    void testTdf106164();
    void testTdf54409();
    void testTdf38394();
    void testTdf59666();
    void testTdf133524();
    void testTdf133524_Romanian();
    void testTdf128860();
    void testTdf123786();
    void testTdf133589();
    void testTdf143176();
    void testInconsistentBookmark();
    void testInsertLongDateFormat();
    void testSpellOnlineParameter();
    void testRedlineAutoCorrect();
    void testRedlineAutoCorrect2();
    void testEmojiAutoCorrect();
    void testTdf129270();
    void testInsertPdf();
    void testTdf143760WrapContourToOff();

    CPPUNIT_TEST_SUITE(SwUiWriterTest4);
    CPPUNIT_TEST(testTdf96515);
    CPPUNIT_TEST(testTdf96943);
    CPPUNIT_TEST(testTdf96536);
    CPPUNIT_TEST(testTdf96479);
    CPPUNIT_TEST(testBookmarkCollapsed);
    CPPUNIT_TEST(testRemoveBookmarkText);
    CPPUNIT_TEST(testRemoveBookmarkTextAndAddNew);
    CPPUNIT_TEST(testRemoveBookmarkTextAndAddNewAfterReload);
    CPPUNIT_TEST(testTdf96961);
    CPPUNIT_TEST(testTdf88453);
    CPPUNIT_TEST(testTdf88453Table);
    CPPUNIT_TEST(testClassificationPaste);
    CPPUNIT_TEST(testSmallCaps);
    CPPUNIT_TEST(testTdf98987);
    CPPUNIT_TEST(testTdf99004);
    CPPUNIT_TEST(testTdf84695);
    CPPUNIT_TEST(testTdf84695NormalChar);
    CPPUNIT_TEST(testTdf84695Tab);
    CPPUNIT_TEST(testTableStyleUndo);
    CPPUNIT_TEST(testRedlineCopyPaste);
    CPPUNIT_TEST(testTdf135260);
    CPPUNIT_TEST(testRedlineParam);
    CPPUNIT_TEST(testRedlineViewAuthor);
    CPPUNIT_TEST(testTdf91292);
    CPPUNIT_TEST(testTdf78727);
    CPPUNIT_TEST(testRedlineTimestamp);
    CPPUNIT_TEST(testCursorWindows);
    CPPUNIT_TEST(testLandscape);
    CPPUNIT_TEST(testTdf95699);
    CPPUNIT_TEST(testTdf104032);
    CPPUNIT_TEST(testTdf104440);
    CPPUNIT_TEST(testTdf104425);
    CPPUNIT_TEST(testTdf104814);
    CPPUNIT_TEST(testTableRedlineRedoCrash);
    CPPUNIT_TEST(testTdf66405);
    CPPUNIT_TEST(testTdf35021_tabOverMarginDemo);
    CPPUNIT_TEST(testTdf106701_tabOverMarginAutotab);
    CPPUNIT_TEST(testTdf104492);
    CPPUNIT_TEST(testTdf107025);
    CPPUNIT_TEST(testTdf107362);
    CPPUNIT_TEST(testTdf105417);
    CPPUNIT_TEST(testTdf105625);
    CPPUNIT_TEST(testTdf125151_protected);
    CPPUNIT_TEST(testTdf125151_protectedB);
    CPPUNIT_TEST(testTdf106736);
    CPPUNIT_TEST(testTdf58604);
    CPPUNIT_TEST(testTdf112025);
    CPPUNIT_TEST(testTdf72942);
    CPPUNIT_TEST(testTdf113877);
    CPPUNIT_TEST(testTdf113877NoMerge);
    CPPUNIT_TEST(testTdf113877_default_style);
    CPPUNIT_TEST(testTdf113877_Standard_style);
    CPPUNIT_TEST(testTdf113877_blank_bold_on);
    CPPUNIT_TEST(testTdf113877_blank_bold_off);
    CPPUNIT_TEST(testMsWordCompTrailingBlanks);
    CPPUNIT_TEST(testCreateDocxAnnotation);
    CPPUNIT_TEST(testTdf107976);
    CPPUNIT_TEST(testTdf142157);
    CPPUNIT_TEST(testTdf116640);
    CPPUNIT_TEST(testTdf108524);
    CPPUNIT_TEST(testRhbz1810732);
    CPPUNIT_TEST(testTableInSection);
    CPPUNIT_TEST(testTableInNestedSection);
    CPPUNIT_TEST(testTableInSectionInTable);
    CPPUNIT_TEST(testSectionInTableInTable);
    CPPUNIT_TEST(testSectionInTableInTable2);
    CPPUNIT_TEST(testSectionInTableInTable3);
    CPPUNIT_TEST(testSectionInTableInTable4);
    CPPUNIT_TEST(testTdf112160);
    CPPUNIT_TEST(testLinesMoveBackwardsInSectionInTable);
    CPPUNIT_TEST(testTdf112741);
    CPPUNIT_TEST(testTdf112860);
    CPPUNIT_TEST(testTdf113287);
    CPPUNIT_TEST(testTdf113445);
    CPPUNIT_TEST(testTdf113686);
    CPPUNIT_TEST(testFontEmbedding);
    CPPUNIT_TEST(testLinesInSectionInTable);
    CPPUNIT_TEST(testParagraphOfTextRange);
    CPPUNIT_TEST(testTdf99689TableOfContents);
    CPPUNIT_TEST(testTdf99689TableOfFigures);
    CPPUNIT_TEST(testTdf99689TableOfTables);
    CPPUNIT_TEST(testTdf112448);
    CPPUNIT_TEST(testTdf113790);
    CPPUNIT_TEST(testTdf108048);
    CPPUNIT_TEST(testTdf114306);
    CPPUNIT_TEST(testTdf114306_2);
    CPPUNIT_TEST(testTdf113481);
    CPPUNIT_TEST(testTdf115013);
    CPPUNIT_TEST(testTdf114536);
    CPPUNIT_TEST(testTdf115065);
    CPPUNIT_TEST(testTdf115132);
    CPPUNIT_TEST(testXDrawPagesSupplier);
    CPPUNIT_TEST(testTdf116403);
    CPPUNIT_TEST(testHtmlCopyImages);
    CPPUNIT_TEST(testTdf116789);
    CPPUNIT_TEST(testTdf91801);
    CPPUNIT_TEST(testTdf51223);
    CPPUNIT_TEST(testInconsistentBookmark);
    CPPUNIT_TEST(testTdf108423);
    CPPUNIT_TEST(testTdf106164);
    CPPUNIT_TEST(testTdf54409);
    CPPUNIT_TEST(testTdf38394);
    CPPUNIT_TEST(testTdf59666);
    CPPUNIT_TEST(testTdf133524);
    CPPUNIT_TEST(testTdf133524_Romanian);
    CPPUNIT_TEST(testTdf128860);
    CPPUNIT_TEST(testTdf123786);
    CPPUNIT_TEST(testTdf133589);
    CPPUNIT_TEST(testTdf143176);
    CPPUNIT_TEST(testInsertLongDateFormat);
    CPPUNIT_TEST(testSpellOnlineParameter);
    CPPUNIT_TEST(testRedlineAutoCorrect);
    CPPUNIT_TEST(testRedlineAutoCorrect2);
    CPPUNIT_TEST(testEmojiAutoCorrect);
    CPPUNIT_TEST(testTdf129270);
    CPPUNIT_TEST(testInsertPdf);
    CPPUNIT_TEST(testTdf143760WrapContourToOff);
    CPPUNIT_TEST_SUITE_END();
};

static void lcl_selectCharacters(SwPaM& rPaM, sal_Int32 first, sal_Int32 end)
{
    rPaM.GetPoint()->nContent.Assign(rPaM.GetContentNode(), first);
    rPaM.SetMark();
    rPaM.GetPoint()->nContent.Assign(rPaM.GetContentNode(), end);
}

void SwUiWriterTest4::testTdf96515()
{
    // Enable hide whitespace mode.
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsWhitespaceHidden());

    // Insert a new paragraph at the end of the document.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XParagraphAppend> xParagraphAppend(xTextDocument->getText(),
                                                            uno::UNO_QUERY);
    xParagraphAppend->finishParagraph(uno::Sequence<beans::PropertyValue>());
    calcLayout();

    // This was 2, a new page was created for the new paragraph.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

void SwUiWriterTest4::testTdf96943()
{
    // Enable hide whitespace mode.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf96943.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    // Insert a new character at the end of the document.
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Insert("d");

    // This was 2, a new page was created for the new layout line.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

void SwUiWriterTest4::testTdf96536()
{
    // Enable hide whitespace mode.
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsWhitespaceHidden());

    // Insert a page break and go back to the first page.
    pWrtShell->InsertPageBreak();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    calcLayout();
    sal_Int32 nSingleParaPageHeight = parseDump("/root/page[1]/infos/bounds", "height").toInt32();
    discardDumpedLayout();

    // Insert a 2nd paragraph at the end of the first page, so the page height grows at least twice...
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XParagraphAppend> xParagraphAppend(xTextDocument->getText(),
                                                            uno::UNO_QUERY);
    const uno::Reference<text::XTextRange> xInsertPos = getRun(getParagraph(1), 1);
    xParagraphAppend->finishParagraphInsert(uno::Sequence<beans::PropertyValue>(), xInsertPos);
    calcLayout();
    CPPUNIT_ASSERT(parseDump("/root/page[1]/infos/bounds", "height").toInt32()
                   >= 2 * nSingleParaPageHeight);
    discardDumpedLayout();

    // ... and then delete the 2nd paragraph, which shrinks the page to the previous size.
    uno::Reference<lang::XComponent> xParagraph(getParagraph(2), uno::UNO_QUERY);
    xParagraph->dispose();
    calcLayout();
    CPPUNIT_ASSERT_EQUAL(nSingleParaPageHeight,
                         parseDump("/root/page[1]/infos/bounds", "height").toInt32());
}

void SwUiWriterTest4::testTdf96479()
{
    // We want to verify the empty input text field in the bookmark
    static const OUString emptyInputTextField
        = OUStringChar(CH_TXT_ATR_INPUTFIELDSTART) + OUStringChar(CH_TXT_ATR_INPUTFIELDEND);

    SwDoc* pDoc = createSwDoc();

    // So we can clean up all references for reload
    {
        // Append bookmark
        SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
        SwPaM aPaM(aIdx);
        IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
        sw::mark::IMark* pMark = rIDMA.makeMark(
            aPaM, "original", IDocumentMarkAccess::MarkType::BOOKMARK, ::sw::mark::InsertMode::New);
        CPPUNIT_ASSERT(!pMark->IsExpanded());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());

        // Get helper objects
        uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);

        // Create cursor from bookmark
        uno::Reference<text::XTextContent> xTextContent(
            xBookmarksSupplier->getBookmarks()->getByName("original"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
        uno::Reference<text::XTextCursor> xCursor
            = xRange->getText()->createTextCursorByRange(xRange);
        CPPUNIT_ASSERT(xCursor->isCollapsed());

        // Remove bookmark
        xRange->getText()->removeTextContent(xTextContent);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rIDMA.getBookmarksCount());

        // Insert replacement bookmark
        uno::Reference<text::XTextContent> xBookmarkNew(
            xFactory->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
        uno::Reference<container::XNamed> xBookmarkName(xBookmarkNew, uno::UNO_QUERY);
        xBookmarkName->setName("replacement");
        CPPUNIT_ASSERT(xCursor->isCollapsed());
        // Force bookmark expansion
        xCursor->getText()->insertString(xCursor, ".", true);
        xCursor->getText()->insertTextContent(xCursor, xBookmarkNew, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        auto mark = *(rIDMA.getBookmarksBegin());
        CPPUNIT_ASSERT(mark->IsExpanded());

        // Create and insert input textfield with some content
        uno::Reference<text::XTextField> xTextField(
            xFactory->createInstance("com.sun.star.text.TextField.Input"), uno::UNO_QUERY);
        uno::Reference<text::XTextCursor> xCursorNew(
            xBookmarkNew->getAnchor()->getText()->createTextCursorByRange(
                xBookmarkNew->getAnchor()));
        CPPUNIT_ASSERT(!xCursorNew->isCollapsed());
        xCursorNew->getText()->insertTextContent(xCursorNew, xTextField, true);
        xBookmarkNew = uno::Reference<text::XTextContent>(
            xBookmarksSupplier->getBookmarks()->getByName("replacement"), uno::UNO_QUERY);
        xCursorNew = xBookmarkNew->getAnchor()->getText()->createTextCursorByRange(
            xBookmarkNew->getAnchor());
        CPPUNIT_ASSERT(!xCursorNew->isCollapsed());

        // Can't check the actual content of the text node via UNO
        mark = *(rIDMA.getBookmarksBegin());
        CPPUNIT_ASSERT(mark->IsExpanded());
        SwPaM pam(mark->GetMarkStart(), mark->GetMarkEnd());
        // Check for the actual bug, which didn't include CH_TXT_ATR_INPUTFIELDEND in the bookmark
        CPPUNIT_ASSERT_EQUAL(emptyInputTextField, pam.GetText());
    }

    {
        // Save and load cycle
        // Actually not needed, but the bug symptom of a missing bookmark
        // occurred because a broken bookmark was saved and loading silently
        // dropped the broken bookmark!
        reload("writer8", "testTdf96479.odt");
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        pDoc = pTextDoc->GetDocShell()->GetDoc();

        // Lookup "replacement" bookmark
        IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xBookmarksSupplier->getBookmarks()->hasByName("replacement"));

        uno::Reference<text::XTextContent> xTextContent(
            xBookmarksSupplier->getBookmarks()->getByName("replacement"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
        uno::Reference<text::XTextCursor> xCursor
            = xRange->getText()->createTextCursorByRange(xRange);
        CPPUNIT_ASSERT(!xCursor->isCollapsed());

        // Verify bookmark content via text node / PaM
        auto mark = *(rIDMA.getBookmarksBegin());
        CPPUNIT_ASSERT(mark->IsExpanded());
        SwPaM pam(mark->GetMarkStart(), mark->GetMarkEnd());
        CPPUNIT_ASSERT_EQUAL(emptyInputTextField, pam.GetText());
    }
}

// If you resave original document the bookmark will be changed from
//
//  <text:p text:style-name="Standard">
//      <text:bookmark-start text:name="test"/>
//      <text:bookmark-end text:name="test"/>
//      def
//  </text:p>
//
// to
//
//  <text:p text:style-name="Standard">
//      <text:bookmark text:name="test"/>
//      def
//  </text:p>
//
void SwUiWriterTest4::testBookmarkCollapsed()
{
    // load document
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "collapsed_bookmark.odt");
    CPPUNIT_ASSERT(pDoc);

    // save original document
    utl::TempFile aTempFile;
    save("writer8", aTempFile);

    // load only content.xml from the resaved document
    if (xmlDocUniquePtr pXmlDoc = parseExportInternal(aTempFile.GetURL(), "content.xml"))
    {
        const OString aPath("/office:document-content/office:body/office:text/text:p");

        const int pos1 = getXPathPosition(pXmlDoc, aPath, "bookmark");
        CPPUNIT_ASSERT_EQUAL(0, pos1); // found, and it is first

        CPPUNIT_ASSERT_ASSERTION_FAIL(
            getXPathPosition(pXmlDoc, aPath, "bookmark-start")); // not found
        CPPUNIT_ASSERT_ASSERTION_FAIL(
            getXPathPosition(pXmlDoc, aPath, "bookmark-end")); // not found
    }
}

// 1. Open a new writer document
// 2. Enter the text "abcdef"
// 3. Select "abc"
// 4. Insert a bookmark on "abc" using Insert->Bookmark. Name the bookmark "test".
// 5. Open the navigator (F5)
//    Select the bookmark "test" using the navigator.
// 6. Hit Del, thus deleting "abc" (The bookmark "test" is still there).
// 7. Save the document:
//      <text:p text:style-name="Standard">
//          <text:bookmark-start text:name="test"/>
//          <text:bookmark-end text:name="test"/>
//          def
//      </text:p>
//
void SwUiWriterTest4::testRemoveBookmarkText()
{
    // create document
    {
        // create a text document with "abcdef"
        SwDoc* pDoc = createSwDoc();

        {
            SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
            SwPaM aPaM(aIdx);
            pDoc->getIDocumentContentOperations().InsertString(aPaM, "abcdef");
        }

        // mark "abc" with "testBookmark" bookmark
        {
            SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
            SwPaM aPaM(aIdx);

            lcl_selectCharacters(aPaM, 0, 3);
            IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
            sw::mark::IMark* pMark
                = rIDMA.makeMark(aPaM, "testBookmark", IDocumentMarkAccess::MarkType::BOOKMARK,
                                 ::sw::mark::InsertMode::New);

            // verify
            CPPUNIT_ASSERT(pMark->IsExpanded());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        }

        // remove text marked with bookmark
        {
            SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
            SwPaM aPaM(aIdx);

            lcl_selectCharacters(aPaM, 0, 3);
            pDoc->getIDocumentContentOperations().DeleteRange(aPaM);

            // verify: bookmark is still exist
            IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        }
    }

    // save document
    utl::TempFile aTempFile;
    save("writer8", aTempFile);

    // load only content.xml from the resaved document
    if (xmlDocUniquePtr pXmlDoc = parseExportInternal(aTempFile.GetURL(), "content.xml"))
    {
        const OString aPath("/office:document-content/office:body/office:text/text:p");

        CPPUNIT_ASSERT_ASSERTION_FAIL(getXPathPosition(pXmlDoc, aPath, "bookmark")); // not found
        const int pos2 = getXPathPosition(pXmlDoc, aPath, "bookmark-start");
        const int pos3 = getXPathPosition(pXmlDoc, aPath, "bookmark-end");

        CPPUNIT_ASSERT_EQUAL(0, pos2); // found, and it is first
        CPPUNIT_ASSERT_EQUAL(1, pos3); // found, and it is second
    }
}

// 1. Open a new writer document
// 2. Enter the text "abcdef"
// 3. Select "abc"
// 4. Insert a bookmark on "abc" using Insert->Bookmark. Name the bookmark "test".
// 5. Open the navigator (F5)
//    Select the bookmark "test" using the navigator.
// 6. Hit Del, thus deleting "abc" (The bookmark "test" is still there).
// 7. Call our macro
//
//      Sub Main
//          bookmark = ThisComponent.getBookmarks().getByName("test")
//          bookmark.getAnchor().setString("abc")
//      End Sub
//
//    The text "abc" gets inserted inside the bookmark "test", and the document now contains the string "abcdef".
// 7. Save the document:
//      <text:p text:style-name="Standard">
//          <text:bookmark-start text:name="test"/>
//          abc
//          <text:bookmark-end text:name="test"/>
//          def
//      </text:p>
//
void SwUiWriterTest4::testRemoveBookmarkTextAndAddNew()
{
    // create document
    {
        // create a text document with "abcdef"
        SwDoc* pDoc = createSwDoc();
        {
            SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
            SwPaM aPaM(aIdx);
            pDoc->getIDocumentContentOperations().InsertString(aPaM, "abcdef");
        }

        // mark "abc" with "testBookmark" bookmark
        {
            SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
            SwPaM aPaM(aIdx);

            lcl_selectCharacters(aPaM, 0, 3);
            IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
            sw::mark::IMark* pMark
                = rIDMA.makeMark(aPaM, "testBookmark", IDocumentMarkAccess::MarkType::BOOKMARK,
                                 ::sw::mark::InsertMode::New);

            // verify
            CPPUNIT_ASSERT(pMark->IsExpanded());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        }

        // remove text marked with bookmark
        {
            SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
            SwPaM aPaM(aIdx);

            lcl_selectCharacters(aPaM, 0, 3);
            pDoc->getIDocumentContentOperations().DeleteRange(aPaM);

            // verify: bookmark is still exist
            IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        }

        // write "abc" to area marked with "testBookmark" bookmark
        {
            // Get helper objects
            uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent,
                                                                        uno::UNO_QUERY);

            // Create cursor from bookmark
            uno::Reference<text::XTextContent> xTextContent(
                xBookmarksSupplier->getBookmarks()->getByName("testBookmark"), uno::UNO_QUERY);
            uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
            CPPUNIT_ASSERT_EQUAL(OUString(""), xRange->getString());

            // write "abc"
            xRange->setString("abc");

            // verify: bookmark is still exist
            IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
        }
    }

    // save document
    utl::TempFile aTempFile;
    save("writer8", aTempFile);

    // load only content.xml from the resaved document
    if (xmlDocUniquePtr pXmlDoc = parseExportInternal(aTempFile.GetURL(), "content.xml"))
    {
        const OString aPath("/office:document-content/office:body/office:text/text:p");

        CPPUNIT_ASSERT_ASSERTION_FAIL(getXPathPosition(pXmlDoc, aPath, "bookmark")); // not found
        const int pos2 = getXPathPosition(pXmlDoc, aPath, "bookmark-start");
        const int pos3 = getXPathPosition(pXmlDoc, aPath, "text");
        const int pos4 = getXPathPosition(pXmlDoc, aPath, "bookmark-end");

        CPPUNIT_ASSERT_EQUAL(0, pos2);
        CPPUNIT_ASSERT_EQUAL(1, pos3);
        CPPUNIT_ASSERT_EQUAL(2, pos4);
    }
}

// 1. Load document:
//  <text:p text:style-name="Standard">
//      <text:bookmark-start text:name="test"/>
//      <text:bookmark-end text:name="test"/>
//      def
//  </text:p>
//
// 2. Call our macro
//
//      Sub Main
//          bookmark = ThisComponent.getBookmarks().getByName("test")
//          bookmark.getAnchor().setString("abc")
//      End Sub
//
//    The text "abc" gets inserted inside the bookmark "test", and the document now contains the string "abcdef".
// 3. Save the document:
//      <text:p text:style-name="Standard">
//          <text:bookmark text:name="test"/>
//          abcdef
//      </text:p>
//
void SwUiWriterTest4::testRemoveBookmarkTextAndAddNewAfterReload()
{
    // load document
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "collapsed_bookmark.odt");
    CPPUNIT_ASSERT(pDoc);

    // write "abc" to area marked with "testBookmark" bookmark
    {
        // Get helper objects
        uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);

        // Create cursor from bookmark
        uno::Reference<text::XTextContent> xTextContent(
            xBookmarksSupplier->getBookmarks()->getByName("test"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString(""), xRange->getString());

        // write "abc"
        xRange->setString("abc");

        // verify: bookmark is still exist
        IDocumentMarkAccess& rIDMA = *pDoc->getIDocumentMarkAccess();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getBookmarksCount());
    }

    // save original document
    utl::TempFile aTempFile;
    save("writer8", aTempFile);

    // load only content.xml from the resaved document
    if (xmlDocUniquePtr pXmlDoc = parseExportInternal(aTempFile.GetURL(), "content.xml"))
    {
        const OString aPath("/office:document-content/office:body/office:text/text:p");

        const int pos1 = getXPathPosition(pXmlDoc, aPath, "bookmark");
        const int pos2 = getXPathPosition(pXmlDoc, aPath, "text");

        CPPUNIT_ASSERT_EQUAL(0, pos1);
        CPPUNIT_ASSERT_EQUAL(1, pos2);

        CPPUNIT_ASSERT_ASSERTION_FAIL(
            getXPathPosition(pXmlDoc, aPath, "bookmark-start")); // not found
        CPPUNIT_ASSERT_ASSERTION_FAIL(
            getXPathPosition(pXmlDoc, aPath, "bookmark-end")); // not found
    }
}

void SwUiWriterTest4::testTdf96961()
{
    // Insert a page break.
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertPageBreak();

    // Enable hide whitespace mode.
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    calcLayout();

    // Assert that the height of the last page is larger than the height of other pages.
    sal_Int32 nOther = parseDump("/root/page[1]/infos/bounds", "height").toInt32();
    sal_Int32 nLast = parseDump("/root/page[2]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT(nLast > nOther);
}

void SwUiWriterTest4::testTdf88453()
{
    createSwDoc(DATA_DIRECTORY, "tdf88453.odt");
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 0: the table does not fit the first page, but it wasn't split
    // to continue on the second page.
    assertXPath(pXmlDoc, "/root/page[2]/body/tab", 1);
}

void SwUiWriterTest4::testTdf88453Table()
{
    createSwDoc(DATA_DIRECTORY, "tdf88453-table.odt");
    calcLayout();
    // This was 2: layout could not split the large outer table in the document
    // into 3 pages.
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

namespace
{
int checkShells(const SwDocShell* pSource, const SwDocShell* pDestination)
{
    return int(SfxClassificationHelper::CheckPaste(pSource->getDocProperties(),
                                                   pDestination->getDocProperties()));
}
}

void SwUiWriterTest4::testClassificationPaste()
{
    SwDocShell* pSourceShell = createSwDoc()->GetDocShell();
    uno::Reference<lang::XComponent> xSourceComponent = mxComponent;
    mxComponent.clear();

    SwDocShell* pDestinationShell = createSwDoc()->GetDocShell();

    // Not classified source, not classified destination.
    CPPUNIT_ASSERT_EQUAL(int(SfxClassificationCheckPasteResult::None),
                         checkShells(pSourceShell, pDestinationShell));

    // Classified source, not classified destination.
    uno::Sequence<beans::PropertyValue> aInternalOnly
        = comphelper::InitPropertySequence({ { "Name", uno::makeAny(OUString("Internal Only")) } });
    dispatchCommand(xSourceComponent, ".uno:ClassificationApply", aInternalOnly);
    CPPUNIT_ASSERT_EQUAL(int(SfxClassificationCheckPasteResult::TargetDocNotClassified),
                         checkShells(pSourceShell, pDestinationShell));

    // Classified source and classified destination -- internal only has a higher level than confidential.
    uno::Sequence<beans::PropertyValue> aConfidential
        = comphelper::InitPropertySequence({ { "Name", uno::makeAny(OUString("Confidential")) } });
    dispatchCommand(mxComponent, ".uno:ClassificationApply", aConfidential);
    CPPUNIT_ASSERT_EQUAL(int(SfxClassificationCheckPasteResult::DocClassificationTooLow),
                         checkShells(pSourceShell, pDestinationShell));

    xSourceComponent->dispose();
}

void SwUiWriterTest4::testSmallCaps()
{
    // Create a document, add some characters and select them.
    createSwDoc();
    SwDoc* pDoc = createSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("text");
    pWrtShell->SelAll();

    // Dispatch the command to make them formatted small capitals.
    dispatchCommand(mxComponent, ".uno:SmallCaps", {});

    // This was css::style::CaseMap::NONE as the shell didn't handle the command.
    CPPUNIT_ASSERT_EQUAL(css::style::CaseMap::SMALLCAPS,
                         getProperty<sal_Int16>(getRun(getParagraph(1), 1), "CharCaseMap"));
}

void SwUiWriterTest4::testTdf98987()
{
    createSwDoc(DATA_DIRECTORY, "tdf98987.docx");
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[2]/SdrObject", "name",
                "Rectangle 1");
    sal_Int32 nRectangle1
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[2]/bounds", "top")
              .toInt32();
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[1]/SdrObject", "name",
                "Rectangle 2");
    sal_Int32 nRectangle2
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[1]/bounds", "top")
              .toInt32();
    CPPUNIT_ASSERT_GREATER(nRectangle1, nRectangle2);

    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[3]/SdrObject", "name",
                "Rectangle 3");
    sal_Int32 nRectangle3
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[3]/bounds", "top")
              .toInt32();
    // This failed: the 3rd rectangle had a smaller "top" value than the 2nd one, it even overlapped with the 1st one.
    CPPUNIT_ASSERT_GREATER(nRectangle2, nRectangle3);
}

void SwUiWriterTest4::testTdf99004()
{
    createSwDoc(DATA_DIRECTORY, "tdf99004.docx");
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nTextbox1Top
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "top").toInt32();
    sal_Int32 nTextBox1Height
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "height").toInt32();
    sal_Int32 nTextBox1Bottom = nTextbox1Top + nTextBox1Height;

    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[1]/SdrObject", "name",
                "Rectangle 2");
    sal_Int32 nRectangle2Top
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject[1]/bounds", "top")
              .toInt32();
    // This was 3291 and 2531, should be now around 2472 and 2531, i.e. the two rectangles should not overlap anymore.
    CPPUNIT_ASSERT(nTextBox1Bottom < nRectangle2Top);
}

void SwUiWriterTest4::testTdf84695()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf84695.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwContact* pTextBox = static_cast<SwContact*>(pObject->GetUserCall());
    // First, make sure that pTextBox is a fly frame (textbox of a shape).
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_FLYFRMFMT), pTextBox->GetFormat()->Which());

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);

    // Now Enter + a key should add some text.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'a', 0);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was empty, Enter did not start the fly frame edit mode.
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xShape->getString());
}

void SwUiWriterTest4::testTdf84695NormalChar()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf84695.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwContact* pTextBox = static_cast<SwContact*>(pObject->GetUserCall());
    // First, make sure that pTextBox is a fly frame (textbox of a shape).
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_FLYFRMFMT), pTextBox->GetFormat()->Which());

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);

    // Now pressing 'a' should add a character.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'a', 0);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was empty, pressing a normal character did not start the fly frame edit mode.
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xShape->getString());
}

void SwUiWriterTest4::testTdf84695Tab()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf84695-tab.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SwContact* pShape = static_cast<SwContact*>(pObject->GetUserCall());
    // First, make sure that pShape is a draw shape.
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_DRAWFRMFMT), pShape->GetFormat()->Which());

    // Then select it.
    pWrtShell->SelectObj(Point(), 0, pObject);

    // Now pressing 'tab' should jump to the other shape.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    // And finally make sure the selection has changed.
    const SdrMarkList& rMarkList = pWrtShell->GetDrawView()->GetMarkedObjectList();
    SwContact* pOtherShape
        = static_cast<SwContact*>(rMarkList.GetMark(0)->GetMarkedSdrObj()->GetUserCall());
    // This failed, 'tab' didn't do anything -> the selected shape was the same.
    CPPUNIT_ASSERT(pOtherShape != pShape);
}

void SwUiWriterTest4::testTableStyleUndo()
{
    SwDoc* pDoc = createSwDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    sal_Int32 nStyleCount = pDoc->GetTableStyles().size();
    SwTableAutoFormat* pStyle = pDoc->MakeTableStyle("Test Style");
    SvxBrushItem aBackground(Color(0xFF00FF), RES_BACKGROUND);
    pStyle->GetBoxFormat(0).SetBackground(aBackground);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount + 1);
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount);
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount + 1);
    // check if attributes are preserved
    pStyle = pDoc->GetTableStyles().FindAutoFormat(u"Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground));

    pDoc->DelTableStyle("Test Style");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount);
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount + 1);
    pStyle = pDoc->GetTableStyles().FindAutoFormat(u"Test Style");
    // check if attributes are preserved
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground));
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount);

    // undo delete so we can replace the style
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pDoc->GetTableStyles().size()), nStyleCount + 1);
    pStyle = pDoc->GetTableStyles().FindAutoFormat(u"Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground));

    SwTableAutoFormat aNewStyle("Test Style2");
    SvxBrushItem aBackground2(Color(0x00FF00), RES_BACKGROUND);
    aNewStyle.GetBoxFormat(0).SetBackground(aBackground2);

    pDoc->ChgTableStyle("Test Style", aNewStyle);
    pStyle = pDoc->GetTableStyles().FindAutoFormat(u"Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground2));
    rUndoManager.Undo();
    pStyle = pDoc->GetTableStyles().FindAutoFormat(u"Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground));
    rUndoManager.Redo();
    pStyle = pDoc->GetTableStyles().FindAutoFormat(u"Test Style");
    CPPUNIT_ASSERT(pStyle);
    CPPUNIT_ASSERT(bool(pStyle->GetBoxFormat(0).GetBackground() == aBackground2));
}

void SwUiWriterTest4::testRedlineCopyPaste()
{
    // regressed in tdf#106746
    SwDoc* pDoc = createSwDoc();

    SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);

    pDoc->getIDocumentContentOperations().InsertString(aPaM, "abzdezgh");
    SwTextNode* pTextNode = aPaM.GetNode().GetTextNode();

    // Turn on track changes, make changes, turn off track changes
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    lcl_selectCharacters(aPaM, 2, 3);
    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, "c", false);
    lcl_selectCharacters(aPaM, 6, 7);
    pDoc->getIDocumentContentOperations().ReplaceRange(aPaM, "f", false);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(false));

    // Create the clipboard document.
    SwDoc aClipboard;
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Select the whole content, copy, delete the original and paste the copied content
    pWrtShell->SelAll();
    pWrtShell->Copy(aClipboard);
    pWrtShell->Delete();
    pWrtShell->Paste(aClipboard);

    // With the bug this is "abzcdefgh", ie. contains the first deleted piece, too
    CPPUNIT_ASSERT_EQUAL(OUString("abcdefgh"), pTextNode->GetText());
}

void SwUiWriterTest4::testTdf135260()
{
    SwDoc* pDoc = createSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("test");

    // Turn on track changes
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));

    for (int i = 0; i < 4; i++)
    {
        pWrtShell->DelLeft();
    }

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    // accept all redlines
    while (pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);

    // Without the fix in place, this test would have failed with
    // - Expected:
    // - Actual  : tes
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());
}

void SwUiWriterTest4::testRedlineParam()
{
    // Create a document with minimal content.
    SwDoc* pDoc = createSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("middle");

    // Turn on track changes, and add changes to the start and end of the document.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    pWrtShell->StartOfSection();
    pWrtShell->Insert("aaa");
    pWrtShell->EndOfSection();
    pWrtShell->Insert("zzz");

    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), rTable.size());

    // Select the first redline.
    pWrtShell->StartOfSection();
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
        { { "NextTrackedChange",
            uno::makeAny(o3tl::narrowing<sal_uInt16>(rTable[0]->GetId())) } }));
    dispatchCommand(mxComponent, ".uno:NextTrackedChange", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // This failed: the parameter wasn't handled so the next change (zzz) was
    // selected, not the first one (aaa).
    CPPUNIT_ASSERT_EQUAL(OUString("aaa"), pShellCursor->GetText());

    // Select the second redline.
    pWrtShell->StartOfSection();
    aPropertyValues = comphelper::InitPropertySequence(
        { { "NextTrackedChange", uno::makeAny(o3tl::narrowing<sal_uInt16>(rTable[1]->GetId())) } });
    dispatchCommand(mxComponent, ".uno:NextTrackedChange", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
    pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("zzz"), pShellCursor->GetText());

    // Move the cursor to the start again, and reject the second change.
    pWrtShell->StartOfSection();
    aPropertyValues = comphelper::InitPropertySequence(
        { { "RejectTrackedChange",
            uno::makeAny(o3tl::narrowing<sal_uInt16>(rTable[1]->GetId())) } });
    dispatchCommand(mxComponent, ".uno:RejectTrackedChange", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
    pShellCursor = pWrtShell->getShellCursor(false);

    // This was 'middlezzz', the uno command rejected the redline under the
    // cursor, instead of the requested one.
    CPPUNIT_ASSERT_EQUAL(OUString("aaamiddle"),
                         pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

void SwUiWriterTest4::testRedlineViewAuthor()
{
    // Test that setting an author at an SwView level has effect.

    // Create a document with minimal content.
    SwDoc* pDoc = createSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("middle");
    SwView* pView = pDocShell->GetView();
    const OUString aAuthor("A U. Thor");
    pView->SetRedlineAuthor(aAuthor);
    pDocShell->SetView(pView);

    // Turn on track changes, and add changes to the start of the document.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    pWrtShell->StartOfSection();
    pWrtShell->Insert("aaa");

    // Now assert that SwView::SetRedlineAuthor() had an effect.
    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), rTable.size());
    SwRangeRedline* pRedline = rTable[0];
    // This was 'Unknown Author' instead of 'A U. Thor'.
    CPPUNIT_ASSERT_EQUAL(aAuthor, pRedline->GetAuthorString());

    // Insert a comment and assert that SwView::SetRedlineAuthor() affects this as well.
    dispatchCommand(mxComponent, ".uno:.uno:InsertAnnotation", {});
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xField(xFields->nextElement(), uno::UNO_QUERY);
    // This was 'Unknown Author' instead of 'A U. Thor'.
    CPPUNIT_ASSERT_EQUAL(aAuthor, xField->getPropertyValue("Author").get<OUString>());

    //Reset the redline author after using it, otherwise, it might interfere with other unittests
    pView->SetRedlineAuthor("Unknown Author");
    pDocShell->SetView(pView);
}

void SwUiWriterTest4::testTdf91292()
{
    createSwDoc(DATA_DIRECTORY, "tdf91292_paraBackground.docx");
    uno::Reference<beans::XPropertySet> xPropertySet(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Solid background color", drawing::FillStyle_SOLID,
                                 getProperty<drawing::FillStyle>(xPropertySet, "FillStyle"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Background Color", static_cast<sal_Int32>(0x5C2D91),
                                 getProperty<sal_Int32>(xPropertySet, "FillColor"));

    // remove background color
    xPropertySet->setPropertyValue("FillStyle", uno::makeAny(drawing::FillStyle_NONE));

    // Save it and load it back.
    reload("Office Open XML Text", "tdf91292_paraBackground.docx");

    xPropertySet.set(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No background color", drawing::FillStyle_NONE,
                                 getProperty<drawing::FillStyle>(xPropertySet, "FillStyle"));
}

void SwUiWriterTest4::testTdf78727()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf78727.docx");
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    // This was 1: make sure we don't loose the TextBox anchored inside the
    // table that is moved inside a text frame.
    CPPUNIT_ASSERT(SwTextBoxHelper::getCount(pPage) > 1);
}

void SwUiWriterTest4::testRedlineTimestamp()
{
    // Test that a redline timestamp's second is not always 0.

    // Create a document with minimal content.
    SwDoc* pDoc = createSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("middle");

    // Turn on track changes, and add changes to the start and to the end of
    // the document.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    pWrtShell->StartOfSection();
    pWrtShell->Insert("aaa");
    osl::Thread::wait(std::chrono::seconds(1));
    pWrtShell->EndOfSection();
    pWrtShell->Insert("zzz");

    // Inserting additional characters at the start changed the table size to
    // 3, i.e. the first and the second "aaa" wasn't combined.
    pWrtShell->StartOfSection();
    pWrtShell->Insert("aaa");

    // Now assert that at least one of the seconds are not 0.
    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    if (rTable.size() >= 2
        && rTable[0]->GetRedlineData().GetTimeStamp().GetMin()
               != rTable[1]->GetRedlineData().GetTimeStamp().GetMin())
        // The relatively rare case when waiting for a second also changes the minute.
        return;

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), rTable.size());
    sal_uInt16 nSec1 = rTable[0]->GetRedlineData().GetTimeStamp().GetSec();
    sal_uInt16 nSec2 = rTable[1]->GetRedlineData().GetTimeStamp().GetSec();
    // This failed, seconds was always 0.
    CPPUNIT_ASSERT(nSec1 != 0 || nSec2 != 0);
}

void SwUiWriterTest4::testCursorWindows()
{
    // Create a new document with one window.
    SwDoc* pDoc = createSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell1 = pDocShell->GetWrtShell();

    // Create a second view and type something.
    pDocShell->GetViewShell()->GetViewFrame()->GetDispatcher()->Execute(
        SID_NEWWINDOW, SfxCallMode::SYNCHRON | SfxCallMode::RECORD);
    SwWrtShell* pWrtShell2 = pDocShell->GetWrtShell();
    OUString aText("foo");
    pWrtShell2->Insert(aText);

    // Assert that only the cursor of the actual window move, not other cursors.
    SwShellCursor* pShellCursor1 = pWrtShell1->getShellCursor(false);
    SwShellCursor* pShellCursor2 = pWrtShell2->getShellCursor(false);
    // This was 3, not 0 -- cursor of the other window moved.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), pShellCursor1->Start()->nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(aText.getLength(), pShellCursor2->Start()->nContent.GetIndex());
}

void SwUiWriterTest4::testLandscape()
{
    // Set page orientation to landscape.
    SwDoc* pDoc = createSwDoc();
    uno::Sequence<beans::PropertyValue> aPropertyValues(
        comphelper::InitPropertySequence({ { "AttributePage.Landscape", uno::Any(true) } }));
    dispatchCommand(mxComponent, ".uno:AttributePage", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    // Assert that the document model was modified.
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    size_t nPageDesc = pWrtShell->GetCurPageDesc();
    // This failed, page was still portrait.
    CPPUNIT_ASSERT(pWrtShell->GetPageDesc(nPageDesc).GetLandscape());
}

void SwUiWriterTest4::testTdf95699()
{
    // Open the document with single FORMCHECKBOX field, select all and copy to clipboard
    // then check that clipboard contains the FORMCHECKBOX in text body.
    // Previously that failed.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf95699.odt");
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    SwDoc aClipboard;
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SelAll();
    pWrtShell->Copy(aClipboard);
    pMarkAccess = aClipboard.getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    ::sw::mark::IFieldmark* pFieldMark
        = pMarkAccess->getFieldmarkAfter(SwPosition(pDoc->GetNodes().GetEndOfExtras()));
    CPPUNIT_ASSERT_EQUAL(OUString("vnd.oasis.opendocument.field.FORMCHECKBOX"),
                         pFieldMark->GetFieldname());
}

void SwUiWriterTest4::testTdf104032()
{
    // Open the document with FORMCHECKBOX field, select it and copy to clipboard
    // Go to end of document and paste it, then undo
    // Previously that asserted in debug build.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf104032.odt");
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    SwDoc aClipboard;
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->StartOfSection();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Copy(aClipboard);
    pWrtShell->EndOfSection();
    pWrtShell->Paste(aClipboard);
    rUndoManager.Undo();
}

void SwUiWriterTest4::testTdf104440()
{
    createSwDoc(DATA_DIRECTORY, "tdf104440.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//page[2]/body/txt/anchored");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    // This was 0: both Text Frames in the document were anchored to a
    // paragraph on page 1, while we expect that the second Text Frame is
    // anchored to a paragraph on page 2.
    CPPUNIT_ASSERT_EQUAL(1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);
}

void SwUiWriterTest4::testTdf104425()
{
    createSwDoc(DATA_DIRECTORY, "tdf104425.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // The document contains one top-level 1-cell table with minimum row height set to 70 cm,
    // and the cell contents does not exceed the minimum row height.
    // It should span over 3 pages.
    assertXPath(pXmlDoc, "//page", 3);
    sal_Int32 nHeight1
        = getXPath(pXmlDoc, "//page[1]/body/tab/row/infos/bounds", "height").toInt32();
    sal_Int32 nHeight2
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/infos/bounds", "height").toInt32();
    sal_Int32 nHeight3
        = getXPath(pXmlDoc, "//page[3]/body/tab/row/infos/bounds", "height").toInt32();
    double fSumHeight_mm = o3tl::convert<double>(nHeight1 + nHeight2 + nHeight3, o3tl::Length::twip,
                                                 o3tl::Length::mm);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(700.0, fSumHeight_mm, 0.05);
}

// accepting change tracking gets stuck on change
void SwUiWriterTest4::testTdf104814()
{
    SwDoc* const pDoc1(createSwDoc(DATA_DIRECTORY, "tdf104814.docx"));

    SwEditShell* const pEditShell(pDoc1->GetEditShell());

    // accept all redlines
    while (pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);
}

// crash at redo of accepting table change tracking imported from DOCX
void SwUiWriterTest4::testTableRedlineRedoCrash()
{
    SwDoc* const pDoc(createSwDoc(DATA_DIRECTORY, "TC-table-del-add.docx"));
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    // accept all redlines, Undo and accept all redlines again

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(/*bAccept=*/true);

    rUndoManager.Undo();

    // without the fix, it crashes
    rIDRA.AcceptAllRedline(true);
}

void SwUiWriterTest4::testTdf66405()
{
    // Imported formula should have zero margins
    createSwDoc(DATA_DIRECTORY, "tdf66405.docx");
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xEmbeddedObjectsSupplier(mxComponent,
                                                                                uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEmbeddedObjects
        = xEmbeddedObjectsSupplier->getEmbeddedObjects();
    uno::Reference<beans::XPropertySet> xFormula;
    xEmbeddedObjects->getByName(xEmbeddedObjects->getElementNames()[0]) >>= xFormula;
    uno::Reference<beans::XPropertySet> xComponent;
    xFormula->getPropertyValue("Component") >>= xComponent;

    // Test embedded object's margins
    sal_Int32 nLeftMargin, nRightMargin, nTopMargin, nBottomMargin;
    xFormula->getPropertyValue("LeftMargin") >>= nLeftMargin;
    xFormula->getPropertyValue("RightMargin") >>= nRightMargin;
    xFormula->getPropertyValue("TopMargin") >>= nTopMargin;
    xFormula->getPropertyValue("BottomMargin") >>= nBottomMargin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nLeftMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nRightMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nTopMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nBottomMargin);

    // Test embedded object component's margins
    xComponent->getPropertyValue("LeftMargin") >>= nLeftMargin;
    xComponent->getPropertyValue("RightMargin") >>= nRightMargin;
    xComponent->getPropertyValue("TopMargin") >>= nTopMargin;
    xComponent->getPropertyValue("BottomMargin") >>= nBottomMargin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nLeftMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nRightMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nTopMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nBottomMargin);
}

void SwUiWriterTest4::testTdf35021_tabOverMarginDemo()
{
#if HAVE_MORE_FONTS
    createSwDoc(DATA_DIRECTORY, "tdf35021_tabOverMarginDemo.doc");
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Tabs should go past the margin @ ~3381
    sal_Int32 nMargin = getXPath(pXmlDoc, "//body/txt[1]/infos/prtBounds", "width").toInt32();
    // left tab was 3381 because it got its own full line
    sal_Int32 nWidth
        = getXPath(pXmlDoc, "//SwLinePortion[@type='PortionType::TabLeft']", "width").toInt32();
    CPPUNIT_ASSERT_MESSAGE("Left Tab width is ~4479", nMargin < nWidth);
    // center tab was 842
    nWidth
        = getXPath(pXmlDoc, "//SwLinePortion[@type='PortionType::TabCenter']", "width").toInt32();
    CPPUNIT_ASSERT_MESSAGE("Center Tab width is ~3521", nMargin < nWidth);
    // right tab was probably the same as center tab.
    nWidth = getXPath(pXmlDoc, "//SwLinePortion[@type='PortionType::TabRight']", "width").toInt32();
    CPPUNIT_ASSERT_MESSAGE("Right Tab width is ~2907", sal_Int32(2500) < nWidth);
    // decimal tab was 266
    nWidth
        = getXPath(pXmlDoc, "//SwLinePortion[@type='PortionType::TabDecimal']", "width").toInt32();
    CPPUNIT_ASSERT_MESSAGE("Decimal Tab width is ~4096", nMargin < nWidth);
#endif
}

void SwUiWriterTest4::testTdf106701_tabOverMarginAutotab()
{
    createSwDoc(DATA_DIRECTORY, "tdf106701_tabOverMarginAutotab.doc");
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // The right margin is ~3378
    sal_Int32 nRightMargin = getXPath(pXmlDoc, "//body/txt[1]/infos/prtBounds", "width").toInt32();
    // Automatic tabstops should never be affected by tabOverMargin compatibility
    // The 1st line's width previously was ~9506
    sal_Int32 nWidth = getXPath(pXmlDoc, "//LineBreak[1]", "nWidth").toInt32();
    CPPUNIT_ASSERT_MESSAGE("1st line's width is less than the right margin", nWidth < nRightMargin);
}

void SwUiWriterTest4::testTdf104492()
{
    createSwDoc(DATA_DIRECTORY, "tdf104492.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // The document should split table over 3 pages.
    assertXPath(pXmlDoc, "//page", 3);
}

void SwUiWriterTest4::testTdf107025()
{
    // Tdf107025 - characters advance with wrong distance, so that
    // they are cluttered because of negative value or
    // break into multiple lines because of overflow.
    // The test document uses DFKAI-SB shipped with Windows.
    createSwDoc(DATA_DIRECTORY, "tdf107025.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Verify the number of characters in each line.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getXPath(pXmlDoc, "(//SwLinePortion)[1]", "length").toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9),
                         getXPath(pXmlDoc, "(//SwLinePortion)[2]", "length").toInt32());

    // Do the subsequent test only if the first line can be displayed,
    // in case that the required font does not exist.
    sal_Int32 nWidth1 = getXPath(pXmlDoc, "(//SwLinePortion)[1]", "width").toInt32();
    if (!nWidth1)
        return;

    CPPUNIT_ASSERT(!parseDump("(//SwLinePortion)[2]", "width").isEmpty());
    // Width of the second line is expected to be 9 times of the first.
    sal_Int32 nWidth2 = getXPath(pXmlDoc, "(//SwLinePortion)[2]", "width").toInt32();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), nWidth2 / nWidth1);
}

void SwUiWriterTest4::testTdf107362()
{
    createSwDoc(DATA_DIRECTORY, "tdf107362.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nHeight
        = getXPath(pXmlDoc, "(//Text[@nType='PortionType::Text'])[1]", "nHeight").toInt32();
    sal_Int32 nWidth1
        = getXPath(pXmlDoc, "(//Text[@nType='PortionType::Text'])[1]", "nWidth").toInt32();
    sal_Int32 nWidth2
        = getXPath(pXmlDoc, "(//Text[@nType='PortionType::Text'])[2]", "nWidth").toInt32();
    sal_Int32 nLineWidth = getXPath(pXmlDoc, "//LineBreak", "nWidth").toInt32();
    sal_Int32 nKernWidth = nLineWidth - nWidth1 - nWidth2;
    // Test only if fonts are available
    if (nWidth1 > 500 && nWidth2 > 200)
    {
        // Kern width should be smaller than 1/3 of the CJK font height.
        CPPUNIT_ASSERT(nKernWidth * 3 < nHeight);
    }
}

void SwUiWriterTest4::testTdf105417()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf105417.odt");
    CPPUNIT_ASSERT(pDoc);
    SwView* pView = pDoc->GetDocShell()->GetView();
    CPPUNIT_ASSERT(pView);
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    CPPUNIT_ASSERT(xHyphenator.is());
    // If there are no English hyphenation rules installed, we can't test
    // hyphenation.
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    uno::Reference<linguistic2::XLinguProperties> xLinguProperties(LinguMgr::GetLinguPropertySet());
    // Automatic hyphenation means not opening a dialog, but going ahead
    // non-interactively.
    xLinguProperties->setIsHyphAuto(true);
    SwHyphWrapper aWrap(pView, xHyphenator, /*bStart=*/false, /*bOther=*/true,
                        /*bSelection=*/false);
    // This never returned, it kept trying to hyphenate the last word
    // (greenbacks) again and again.
    aWrap.SpellDocument();
}

void SwUiWriterTest4::testTdf105625()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf105625.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    uno::Reference<uno::XComponentContext> xComponentContext(
        comphelper::getProcessComponentContext());
    // Ensure correct initial setting
    comphelper::ConfigurationHelper::writeDirectKey(
        xComponentContext, "org.openoffice.Office.Writer/", "Cursor/Option", "IgnoreProtectedArea",
        css::uno::Any(false), comphelper::EConfigurationModes::Standard);
    // We should be able to edit at positions adjacent to fields.
    // Check if the start and the end of the 1st paragraph are not protected
    // (they are adjacent to FORMCHECKBOX)
    pWrtShell->SttPara();
    CPPUNIT_ASSERT_EQUAL(false, pWrtShell->HasReadonlySel());
    pWrtShell->EndPara();
    CPPUNIT_ASSERT_EQUAL(false, pWrtShell->HasReadonlySel());
    // 2nd paragraph - FORMTEXT
    pWrtShell->Down(/*bSelect=*/false);
    // Check selection across FORMTEXT field boundary - must be read-only
    pWrtShell->SttPara();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    CPPUNIT_ASSERT_EQUAL(true, pWrtShell->HasReadonlySel());
    // Test deletion of whole field with single backspace
    // Previously it only removed right boundary of FORMTEXT, or failed removal at all
    const IDocumentMarkAccess* pMarksAccess = pDoc->getIDocumentMarkAccess();
    sal_Int32 nMarksBefore = pMarksAccess->getAllMarksCount();
    pWrtShell->EndPara();
    pWrtShell->DelLeft();
    sal_Int32 nMarksAfter = pMarksAccess->getAllMarksCount();
    CPPUNIT_ASSERT_EQUAL(nMarksBefore, nMarksAfter + 1);
}

void SwUiWriterTest4::testTdf125151_protected()
{
    // Similar to testTdf105625 except this is in a protected section,
    // so read-only is already true when fieldmarks are considered.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf125151_protected.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    uno::Reference<uno::XComponentContext> xComponentContext(
        comphelper::getProcessComponentContext());
    // Ensure correct initial setting
    comphelper::ConfigurationHelper::writeDirectKey(
        xComponentContext, "org.openoffice.Office.Writer/", "Cursor/Option", "IgnoreProtectedArea",
        css::uno::Any(false), comphelper::EConfigurationModes::Standard);
    pWrtShell->Down(/*bSelect=*/false);
    // The cursor moved inside of the FieldMark textbox.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Readonly 1", false, pWrtShell->HasReadonlySel());
    // Move left to the start/definition of the textbox
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Readonly 2", true, pWrtShell->HasReadonlySel());
}

void SwUiWriterTest4::testTdf125151_protectedB()
{
    // Similar to testTdf105625 except this is protected with the Protect_Form compat setting
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf125151_protectedB.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    uno::Reference<uno::XComponentContext> xComponentContext(
        comphelper::getProcessComponentContext());
    // Ensure correct initial setting
    comphelper::ConfigurationHelper::writeDirectKey(
        xComponentContext, "org.openoffice.Office.Writer/", "Cursor/Option", "IgnoreProtectedArea",
        css::uno::Any(false), comphelper::EConfigurationModes::Standard);
    // The cursor starts inside of the FieldMark textbox.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Readonly 1", false, pWrtShell->HasReadonlySel());
    // Move left to the start/definition of the textbox
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Readonly 2", true, pWrtShell->HasReadonlySel());
}

void SwUiWriterTest4::testTdf106736()
{
    createSwDoc(DATA_DIRECTORY, "tdf106736-grid.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nWidth
        = getXPath(pXmlDoc, "(//Text[@nType='PortionType::TabLeft'])[1]", "nWidth").toInt32();
    // In tdf106736, width of tab overflow so that it got
    // width value around 9200, expected value is around 103
    CPPUNIT_ASSERT_MESSAGE("Left Tab width is ~103", nWidth < 150);
}

void SwUiWriterTest4::testMsWordCompTrailingBlanks()
{
    // The option is true in settings.xml
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "MsWordCompTrailingBlanksTrue.odt");
    CPPUNIT_ASSERT_EQUAL(true, pDoc->getIDocumentSettingAccess().get(
                                   DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS));
    calcLayout();
    // Check that trailing spaces spans have no width if option is enabled

    CPPUNIT_ASSERT_EQUAL(OUString(), parseDump("/root/page/body/txt[2]/Text[4]", "nWidth"));
    CPPUNIT_ASSERT_EQUAL(OUString(), parseDump("/root/page/body/txt[2]/Text[5]", "nWidth"));
    CPPUNIT_ASSERT_EQUAL(OUString(), parseDump("/root/page/body/txt[3]/Text[4]", "nWidth"));
    CPPUNIT_ASSERT_EQUAL(OUString(), parseDump("/root/page/body/txt[3]/Text[5]", "nWidth"));

    // The option is false in settings.xml
    pDoc = createSwDoc(DATA_DIRECTORY, "MsWordCompTrailingBlanksFalse.odt");
    CPPUNIT_ASSERT_EQUAL(false, pDoc->getIDocumentSettingAccess().get(
                                    DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS));
    calcLayout();
    // Check that trailing spaces spans have width if option is disabled
    CPPUNIT_ASSERT(!parseDump("/root/page/body/txt[2]/Text[4]", "nWidth").isEmpty());
    CPPUNIT_ASSERT(!parseDump("/root/page/body/txt[2]/Text[5]", "nWidth").isEmpty());
    CPPUNIT_ASSERT(!parseDump("/root/page/body/txt[3]/Text[4]", "nWidth").isEmpty());
    CPPUNIT_ASSERT(!parseDump("/root/page/body/txt[3]/Text[5]", "nWidth").isEmpty());

    // MsWordCompTrailingBlanks option should be false by default in new documents
    pDoc = createSwDoc();
    CPPUNIT_ASSERT_EQUAL(false, pDoc->getIDocumentSettingAccess().get(
                                    DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS));

    // The option should be true if a .docx, .doc or .rtf document is opened
    pDoc = createSwDoc(DATA_DIRECTORY, "MsWordCompTrailingBlanks.docx");
    CPPUNIT_ASSERT_EQUAL(true, pDoc->getIDocumentSettingAccess().get(
                                   DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS));
}

void SwUiWriterTest4::testCreateDocxAnnotation()
{
    createSwDoc();

    // insert an annotation with a text
    const OUString aSomeText("some text");
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Text", uno::makeAny(aSomeText) },
        { "Author", uno::makeAny(OUString("me")) },
    });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aPropertyValues);

    // Save it as DOCX & load it again
    reload("Office Open XML Text", "create-docx-annotation.docx");

    // get the annotation
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xField(xFields->nextElement(), uno::UNO_QUERY);

    // this was empty instead of "some text"
    CPPUNIT_ASSERT_EQUAL(aSomeText, xField->getPropertyValue("Content").get<OUString>());
}

void SwUiWriterTest4::testTdf107976()
{
    // Create a document and create two transferables.
    SwDoc* pDoc = createSwDoc();
    SwWrtShell& rShell = *pDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> pTransferable(new SwTransferable(rShell));
    rtl::Reference<SwTransferable> pTransferable2(new SwTransferable(rShell));
    // Now close the document.
    mxComponent->dispose();
    mxComponent.clear();
    // This failed: the first shell had a pointer to the deleted shell.
    CPPUNIT_ASSERT(!pTransferable->GetShell());
    CPPUNIT_ASSERT(!pTransferable2->GetShell());
}

void SwUiWriterTest4::testTdf58604()
{
#ifdef _WIN32
    // Allow linebreak character follows hanging punctuation immediately instead of
    // breaking at the start of the next line.
    createSwDoc(DATA_DIRECTORY, "tdf58604.odt");
    CPPUNIT_ASSERT_EQUAL(
        OUString("PortionType::Break"),
        parseDump("(/root/page/body/txt/LineBreak[1]/preceding::Text)[last()]", "nType"));
#endif
}

void SwUiWriterTest4::testTdf112025()
{
    createSwDoc(DATA_DIRECTORY, "fdo112025.odt");
    const int numberOfParagraphs = getParagraphs();
    CPPUNIT_ASSERT_EQUAL(1, numberOfParagraphs);

    // get a page cursor
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToEndOfPage();

    OUString insertFileid = m_directories.getURLFromSrc(DATA_DIRECTORY) + "fdo112025-insert.docx";
    uno::Sequence<beans::PropertyValue> aPropertyValues(
        comphelper::InitPropertySequence({ { "Name", uno::makeAny(insertFileid) } }));
    dispatchCommand(mxComponent, ".uno:InsertDoc", aPropertyValues);
    // something has been inserted + an additional paragraph
    CPPUNIT_ASSERT_GREATER(numberOfParagraphs, getParagraphs());

    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"),
                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStyle, "IsLandscape"));
}

void SwUiWriterTest4::testTdf72942()
{
    createSwDoc(DATA_DIRECTORY, "fdo72942.docx");

    // get a page cursor
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToEndOfPage();

    OUString insertFileid = m_directories.getURLFromSrc(DATA_DIRECTORY) + "fdo72942-insert.docx";
    uno::Sequence<beans::PropertyValue> aPropertyValues(
        comphelper::InitPropertySequence({ { "Name", uno::makeAny(insertFileid) } }));
    dispatchCommand(mxComponent, ".uno:InsertDoc", aPropertyValues);

    // check styles of paragraphs added from [fdo72942.docx]
    const uno::Reference<text::XTextRange> xRun1 = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("Default English (Liberation serif) text with "),
                         xRun1->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Liberation Serif"),
                         getProperty<OUString>(xRun1, "CharFontName"));

    const uno::Reference<text::XTextRange> xRun2 = getRun(getParagraph(2), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("Header 1 English text (Liberation sans) with "),
                         xRun2->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Liberation Sans"), getProperty<OUString>(xRun2, "CharFontName"));

    // check styles of paragraphs added from [fdo72942-insert.docx]
    const uno::Reference<text::XTextRange> xRun3 = getRun(getParagraph(4), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("Default German text (Calibri) with "), xRun3->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Liberation Serif"),
                         getProperty<OUString>(xRun3, "CharFontName"));

    const uno::Reference<text::XTextRange> xRun4 = getRun(getParagraph(5), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("Header 1 German text (Calibri Light) with "),
                         xRun4->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Liberation Sans"), getProperty<OUString>(xRun4, "CharFontName"));
}

void SwUiWriterTest4::testTdf114306()
{
    createSwDoc(DATA_DIRECTORY, "fdo114306.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // There are 2 long paragraphs in cell A1.
    // A part of paragraph 2 should flow over to the second page but
    // *not* the whole paragraph. There should be 2 paragraphs on
    // page 1 and 1 paragraph on page 2.
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt", 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/tab[1]/row[1]/cell[1]/txt", 1);
}

void SwUiWriterTest4::testTdf114306_2()
{
    // tdf#114306 fix unexpected page break in row-spanned table
    // load regression document without writer crash
    createSwDoc(DATA_DIRECTORY, "fdo114306_2.odt");

    // correct number of pages
    CPPUNIT_ASSERT_EQUAL(4, getPages());
}

void SwUiWriterTest4::testTdf113877_mergeDocs(const char* aDestDoc, const char* aInsertDoc)
{
    createSwDoc(DATA_DIRECTORY, aDestDoc);

    // set a page cursor into the end of the document
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToEndOfPage();

    // insert the same document at current cursor position
    {
        const OUString insertFileid
            = m_directories.getURLFromSrc(DATA_DIRECTORY) + OUString::createFromAscii(aInsertDoc);
        uno::Sequence<beans::PropertyValue> aPropertyValues(
            comphelper::InitPropertySequence({ { "Name", uno::makeAny(insertFileid) } }));
        dispatchCommand(mxComponent, ".uno:InsertDoc", aPropertyValues);
    }
}

// During insert of the document with list inside into the main document inside the list
// we should merge both lists into one, when they have the same list properties
void SwUiWriterTest4::testTdf113877()
{
    testTdf113877_mergeDocs("tdf113877_insert_numbered_list.odt",
                            "tdf113877_insert_numbered_list.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId4 = getProperty<OUString>(getParagraph(4), "ListId");
    const OUString listId5 = getProperty<OUString>(getParagraph(5), "ListId");
    const OUString listId6 = getProperty<OUString>(getParagraph(6), "ListId");
    const OUString listId7 = getProperty<OUString>(getParagraph(7), "ListId");

    // the initial list with 4 list items
    CPPUNIT_ASSERT_EQUAL(listId1, listId4);

    // the last of the first list, and the first of the inserted list
    CPPUNIT_ASSERT_EQUAL(listId4, listId5);
    CPPUNIT_ASSERT_EQUAL(listId5, listId6);
    CPPUNIT_ASSERT_EQUAL(listId6, listId7);
}

// The same test as testTdf113877() but merging of two list should not be performed.
void SwUiWriterTest4::testTdf113877NoMerge()
{
    testTdf113877_mergeDocs("tdf113877_insert_numbered_list.odt",
                            "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId4 = getProperty<OUString>(getParagraph(4), "ListId");
    const OUString listId5 = getProperty<OUString>(getParagraph(5), "ListId");
    const OUString listId6 = getProperty<OUString>(getParagraph(6), "ListId");
    const OUString listId7 = getProperty<OUString>(getParagraph(7), "ListId");

    // the initial list with 4 list items
    CPPUNIT_ASSERT_EQUAL(listId1, listId4);

    // the last of the first list, and the first of the inserted list
    CPPUNIT_ASSERT(listId4 != listId5);
    CPPUNIT_ASSERT_EQUAL(listId5, listId6);
    CPPUNIT_ASSERT(listId6 != listId7);
}

// Related test to testTdf113877(): Inserting into empty document a new document with list.
// Insert position has NO its own paragraph style ("Standard" will be used).
//
// Resulting document should be the same for following tests:
// - testTdf113877_default_style()
// - testTdf113877_Standard_style()
//
void SwUiWriterTest4::testTdf113877_default_style()
{
    testTdf113877_mergeDocs("tdf113877_blank.odt", "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

// Related test to testTdf113877(): Inserting into empty document a new document with list.
// Insert position has its own paragraph style derived from "Standard", but this style is the same as "Standard".
//
// Resulting document should be the same for following tests:
// - testTdf113877_default_style()
// - testTdf113877_Standard_style()
//
void SwUiWriterTest4::testTdf113877_Standard_style()
{
    testTdf113877_mergeDocs("tdf113877_blank_ownStandard.odt",
                            "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

void SwUiWriterTest4::testTdf113877_blank_bold_on()
{
    testTdf113877_mergeDocs("tdf113877_blank_bold_on.odt",
                            "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

void SwUiWriterTest4::testTdf113877_blank_bold_off()
{
    testTdf113877_mergeDocs("tdf113877_blank_bold_off.odt",
                            "tdf113877_insert_numbered_list_abcd.odt");

    const OUString listId1 = getProperty<OUString>(getParagraph(1), "ListId");
    const OUString listId2 = getProperty<OUString>(getParagraph(2), "ListId");
    const OUString listId3 = getProperty<OUString>(getParagraph(3), "ListId");

    CPPUNIT_ASSERT_EQUAL(listId1, listId2);
    CPPUNIT_ASSERT_EQUAL(listId1, listId3);
}

// just care that this does crash/assert
void SwUiWriterTest4::testRhbz1810732()
{
    createSwDoc(DATA_DIRECTORY, "tdf113877_blank.odt");

    // set a page cursor into the end of the document
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToEndOfPage();

    // insert the same document at current cursor position
    {
        const OUString insertFileid
            = m_directories.getURLFromSrc(DATA_DIRECTORY) + "rhbz1810732.docx";
        uno::Sequence<beans::PropertyValue> aPropertyValues(
            comphelper::InitPropertySequence({ { "Name", uno::makeAny(insertFileid) } }));
        dispatchCommand(mxComponent, ".uno:InsertDoc", aPropertyValues);
    }
}

void SwUiWriterTest4::testTdf142157()
{
    createSwDoc();

    const OUString insertFileid = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf142157.odt";
    uno::Sequence<beans::PropertyValue> aPropertyValues(
        comphelper::InitPropertySequence({ { "Name", uno::makeAny(insertFileid) } }));
    dispatchCommand(mxComponent, ".uno:InsertDoc", aPropertyValues);

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
}

void SwUiWriterTest4::testTdf116640()
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Columns", uno::makeAny(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, ".uno:InsertSection", aArgs);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextSection(xSections->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    uno::Reference<text::XTextColumns> xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
}

void SwUiWriterTest4::testTdf108524()
{
    createSwDoc(DATA_DIRECTORY, "tdf108524.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // In total we expect two cells containing a section.
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/section", 2);

    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell/section", 1);
    // This was 0, section wasn't split, instead it was only on the first page
    // and it was cut off.
    assertXPath(pXmlDoc, "/root/page[2]/body/tab/row/cell/section", 1);
}

void SwUiWriterTest4::testLinesInSectionInTable()
{
    // This is similar to testTdf108524(), but the page boundary now is not in
    // the middle of a multi-line paragraph: the section only contains oneliner
    // paragraphs instead.
    createSwDoc(DATA_DIRECTORY, "lines-in-section-in-table.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // In total we expect two cells containing a section.
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/section", 2);

    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell/section", 1);
    // This was 0, section wasn't split, instead it was only on the first page
    // and it was cut off.
    assertXPath(pXmlDoc, "/root/page[2]/body/tab/row/cell/section", 1);
}

void SwUiWriterTest4::testLinesMoveBackwardsInSectionInTable()
{
#if HAVE_MORE_FONTS
    // Assert that paragraph "4" is on page 1 and "5" is on page 2.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "lines-in-section-in-table.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 2);
    SwNodeOffset nPara4Node(
        getXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell[1]/section/txt[last()]", "txtNodeIndex")
            .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("4"), pDoc->GetNodes()[nPara4Node]->GetTextNode()->GetText());
    SwNodeOffset nPara5Node(
        getXPath(pXmlDoc, "/root/page[2]/body/tab/row/cell[1]/section/txt[1]", "txtNodeIndex")
            .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("5"), pDoc->GetNodes()[nPara5Node]->GetTextNode()->GetText());

    // Remove paragraph "4".
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    while (pWrtShell->GetCursor()->GetNode().GetIndex() < nPara4Node)
        pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara();
    pWrtShell->Up(/*bSelect=*/true);
    pWrtShell->DelLeft();

    // Assert that paragraph "5" is now moved back to page 1 and is the last paragraph there.
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    SwNodeOffset nPage1LastNode(
        getXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell[1]/section/txt[last()]", "txtNodeIndex")
            .toUInt32());
    // This was "3", paragraph "4" was deleted, but "5" was not moved backwards from page 2.
    CPPUNIT_ASSERT_EQUAL(OUString("5"), pDoc->GetNodes()[nPage1LastNode]->GetTextNode()->GetText());
#endif
}

void SwUiWriterTest4::testTableInSection()
{
#if HAVE_MORE_FONTS
    // The document has a section, containing a table that spans over 2 pages.
    createSwDoc(DATA_DIRECTORY, "table-in-sect.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // In total we expect 4 cells.
    assertXPath(pXmlDoc, "/root/page/body/section/tab/row/cell", 4);

    // Assert that on both pages the section contains 2 cells.
    assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row/cell", 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/row/cell", 2);
#endif
}

void SwUiWriterTest4::testTableInNestedSection()
{
#if HAVE_MORE_FONTS
    // The document has a nested section, containing a table that spans over 2 pages.
    // This crashed the layout.
    createSwDoc(DATA_DIRECTORY, "rhbz739252-3.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure the table is inside a section and spans over 2 pages.
    assertXPath(pXmlDoc, "//page[1]//section/tab", 1);
    assertXPath(pXmlDoc, "//page[2]//section/tab", 1);
#endif
}

void SwUiWriterTest4::testTdf112741()
{
#if HAVE_MORE_FONTS
    createSwDoc(DATA_DIRECTORY, "tdf112741.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 5 pages.
    assertXPath(pXmlDoc, "//page", 4);
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell/section", 1);
    assertXPath(pXmlDoc, "//page[2]/body/tab/row/cell/tab/row/cell/section", 1);
    // This failed, 3rd page contained no sections.
    assertXPath(pXmlDoc, "//page[3]/body/tab/row/cell/tab/row/cell/section", 1);
    assertXPath(pXmlDoc, "//page[4]/body/tab/row/cell/tab/row/cell/section", 1);
#endif
}

void SwUiWriterTest4::testTdf112860()
{
#if HAVE_MORE_FONTS
    // The document has a split section inside a nested table, and also a table
    // in the footer.
    // This crashed the layout.
    createSwDoc(DATA_DIRECTORY, "tdf112860.fodt");
#endif
}

void SwUiWriterTest4::testTdf113287()
{
#if HAVE_MORE_FONTS
    createSwDoc(DATA_DIRECTORY, "tdf113287.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page", 2);
    sal_uInt32 nCellTop
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell[1]/infos/bounds", "top").toUInt32();
    sal_uInt32 nSectionTop
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell[1]/section/infos/bounds", "top")
              .toUInt32();
    // Make sure section frame is inside the cell frame.
    // Expected greater than 4593, was only 3714.
    CPPUNIT_ASSERT_GREATER(nCellTop, nSectionTop);
#endif
}

void SwUiWriterTest4::testTdf113445()
{
#if HAVE_MORE_FONTS
    // Force multiple-page view.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf113445.fodt");
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwView* pView = pDocShell->GetView();
    pView->SetViewLayout(/*nColumns=*/2, /*bBookMode=*/false);
    calcLayout();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page", 2);
    sal_uInt32 nPage1Left = getXPath(pXmlDoc, "//page[1]/infos/bounds", "left").toUInt32();
    sal_uInt32 nPage2Left = getXPath(pXmlDoc, "//page[2]/infos/bounds", "left").toUInt32();
    // Make sure that page 2 is on the right hand side of page 1, not below it.
    CPPUNIT_ASSERT_GREATER(nPage1Left, nPage2Left);

    // Insert a new paragraph at the start of the document.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->StartOfSection();
    pWrtShell->SplitNode();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    // Make sure that Table2:C5 and Table2:D5 has its section frame inside the cell frame.
    sal_uInt32 nCell3Top
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell/tab/row[4]/cell[3]/infos/bounds", "top")
              .toUInt32();
    sal_uInt32 nSection3Top
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell/tab/row[4]/cell[3]/section/infos/bounds",
                   "top")
              .toUInt32();
    CPPUNIT_ASSERT_GREATER(nCell3Top, nSection3Top);
    sal_uInt32 nCell4Top
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell/tab/row[4]/cell[4]/infos/bounds", "top")
              .toUInt32();
    sal_uInt32 nSection4Top
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell/tab/row[4]/cell[4]/section/infos/bounds",
                   "top")
              .toUInt32();
    CPPUNIT_ASSERT_GREATER(nCell4Top, nSection4Top);
    // Also check if the two cells in the same row have the same top position.
    // This was 4818, expected only 1672.
    CPPUNIT_ASSERT_EQUAL(nCell3Top, nCell4Top);
#endif
}

void SwUiWriterTest4::testTdf113686()
{
#if HAVE_MORE_FONTS
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf113686.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 2);
    SwNodeOffset nPage1LastNode(
        getXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell[1]/tab/row/cell[1]/txt[last()]",
                 "txtNodeIndex")
            .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("Table2:A1-P10"),
                         pDoc->GetNodes()[nPage1LastNode]->GetTextNode()->GetText());
    SwNodeOffset nPage2FirstNode(
        getXPath(pXmlDoc, "/root/page[2]/body/tab/row/cell[1]/section/txt[1]", "txtNodeIndex")
            .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("Table1:A1"),
                         pDoc->GetNodes()[nPage2FirstNode]->GetTextNode()->GetText());

    // Remove page 2.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    while (pWrtShell->GetCursor()->Start()->nNode.GetIndex() < nPage1LastNode)
        pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara();
    for (int i = 0; i < 3; ++i)
        pWrtShell->Up(/*bSelect=*/true);
    pWrtShell->DelLeft();

    // Assert that the second page is removed.
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was still 2, content from 2nd page was not moved.
    assertXPath(pXmlDoc, "/root/page", 1);
#endif
}

void SwUiWriterTest4::testTableInSectionInTable()
{
#if HAVE_MORE_FONTS
    // The document has a table, containing a section, containing a nested
    // table.
    // This crashed the layout.
    createSwDoc(DATA_DIRECTORY, "i95698.odt");
#endif
}

void SwUiWriterTest4::testSectionInTableInTable()
{
#if HAVE_MORE_FONTS
    // The document has a nested table, containing a multi-line section at a
    // page boundary.
    // This crashed the layout later in SwFrame::IsFootnoteAllowed().
    createSwDoc(DATA_DIRECTORY, "tdf112109.fodt");
#endif
}

void SwUiWriterTest4::testSectionInTableInTable2()
{
#if HAVE_MORE_FONTS
    createSwDoc(DATA_DIRECTORY, "split-section-in-nested-table.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_uInt32 nSection1
        = getXPath(pXmlDoc, "//page[1]//body/tab/row/cell/tab/row/cell/section", "id").toUInt32();
    sal_uInt32 nSection1Follow
        = getXPath(pXmlDoc, "//page[1]//body/tab/row/cell/tab/row/cell/section", "follow")
              .toUInt32();
    // This failed, the section wasn't split inside a nested table.
    sal_uInt32 nSection2
        = getXPath(pXmlDoc, "//page[2]//body/tab/row/cell/tab/row/cell/section", "id").toUInt32();
    sal_uInt32 nSection2Precede
        = getXPath(pXmlDoc, "//page[2]//body/tab/row/cell/tab/row/cell/section", "precede")
              .toUInt32();

    // Make sure that the first's follow and the second's precede is correct.
    CPPUNIT_ASSERT_EQUAL(nSection2, nSection1Follow);
    CPPUNIT_ASSERT_EQUAL(nSection1, nSection2Precede);
#endif
}

void SwUiWriterTest4::testSectionInTableInTable3()
{
#if HAVE_MORE_FONTS
    createSwDoc(DATA_DIRECTORY, "tdf113153.fodt");

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<container::XNamed> xTable(xTables->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Table16"), xTable->getName());

    uno::Reference<text::XTextTable> xRowSupplier(xTable, uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xRows = xRowSupplier->getRows();
    uno::Reference<beans::XPropertySet> xRow(xRows->getByIndex(1), uno::UNO_QUERY);
    xRow->setPropertyValue("IsSplitAllowed", uno::makeAny(true));
    // This never returned.
    calcLayout();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_uInt32 nTable1 = getXPath(pXmlDoc, "//page[1]//body/tab", "id").toUInt32();
    sal_uInt32 nTable1Follow = getXPath(pXmlDoc, "//page[1]//body/tab", "follow").toUInt32();
    sal_uInt32 nTable2 = getXPath(pXmlDoc, "//page[2]//body/tab", "id").toUInt32();
    sal_uInt32 nTable2Precede = getXPath(pXmlDoc, "//page[2]//body/tab", "precede").toUInt32();
    sal_uInt32 nTable2Follow = getXPath(pXmlDoc, "//page[2]//body/tab", "follow").toUInt32();
    sal_uInt32 nTable3 = getXPath(pXmlDoc, "//page[3]//body/tab", "id").toUInt32();
    sal_uInt32 nTable3Precede = getXPath(pXmlDoc, "//page[3]//body/tab", "precede").toUInt32();

    // Make sure the outer table frames are linked together properly.
    CPPUNIT_ASSERT_EQUAL(nTable2, nTable1Follow);
    CPPUNIT_ASSERT_EQUAL(nTable1, nTable2Precede);
    CPPUNIT_ASSERT_EQUAL(nTable3, nTable2Follow);
    CPPUNIT_ASSERT_EQUAL(nTable2, nTable3Precede);
#endif
}

void SwUiWriterTest4::testSectionInTableInTable4()
{
#if HAVE_MORE_FONTS
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf113520.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 3);
    SwNodeOffset nPage1LastNode(
        getXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell[1]/tab/row/cell[1]/section/txt[last()]",
                 "txtNodeIndex")
            .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("Section1:P10"),
                         pDoc->GetNodes()[nPage1LastNode]->GetTextNode()->GetText());
    SwNodeOffset nPage3FirstNode(
        getXPath(pXmlDoc, "/root/page[3]/body/tab/row/cell[1]/tab/row/cell[1]/section/txt[1]",
                 "txtNodeIndex")
            .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("Section1:P23"),
                         pDoc->GetNodes()[nPage3FirstNode]->GetTextNode()->GetText());

    // Remove page 2.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    while (pWrtShell->GetCursor()->Start()->nNode.GetIndex() < nPage1LastNode)
        pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara();
    while (pWrtShell->GetCursor()->End()->nNode.GetIndex() < nPage3FirstNode)
        pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->DelLeft();

    // Assert that the page is removed.
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was 3, page 2 was emptied, but it wasn't removed.
    assertXPath(pXmlDoc, "/root/page", 2);

    // Make sure the outer table frames are linked together properly.
    sal_uInt32 nTable1 = getXPath(pXmlDoc, "//page[1]//body/tab", "id").toUInt32();
    sal_uInt32 nTable1Follow = getXPath(pXmlDoc, "//page[1]//body/tab", "follow").toUInt32();
    sal_uInt32 nTable2 = getXPath(pXmlDoc, "//page[2]//body/tab", "id").toUInt32();
    sal_uInt32 nTable2Precede = getXPath(pXmlDoc, "//page[2]//body/tab", "precede").toUInt32();
    CPPUNIT_ASSERT_EQUAL(nTable2, nTable1Follow);
    CPPUNIT_ASSERT_EQUAL(nTable1, nTable2Precede);
#endif
}

void SwUiWriterTest4::testTdf112160()
{
#if HAVE_MORE_FONTS
    // Assert that the A2 cell is on page 1.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf112160.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    SwNodeOffset nA2CellNode(getXPath(pXmlDoc,
                                      "/root/page[1]/body/tab/row[2]/cell[1]/section/txt[last()]",
                                      "txtNodeIndex")
                                 .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("Table1.A2"),
                         pDoc->GetNodes()[nA2CellNode]->GetTextNode()->GetText());

    // Append a new paragraph to the end of the A2 cell.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    while (pWrtShell->GetCursor()->GetNode().GetIndex() < nA2CellNode)
        pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara();
    pWrtShell->SplitNode();

    // Assert that after A2 got extended, D2 stays on page 1.
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    sal_uInt32 nD2CellNode
        = getXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[last()]/section/txt[last()]",
                   "txtNodeIndex")
              .toUInt32();
    // This was Table1.C2, Table1.D2 was moved to the next page, unexpected.
    CPPUNIT_ASSERT_EQUAL(OUString("Table1.D2"),
                         pDoc->GetNodes()[SwNodeOffset(nD2CellNode)]->GetTextNode()->GetText());
#endif
}

void SwUiWriterTest4::testTdf114536()
{
    // This crashed in SwTextFormatter::MergeCharacterBorder() due to a
    // use after free.
    createSwDoc(DATA_DIRECTORY, "tdf114536.odt");
}

void SwUiWriterTest4::testParagraphOfTextRange()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "paragraph-of-text-range.odt");

    // Enter the table.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    CPPUNIT_ASSERT(pWrtShell->IsCursorInTable());
    // Enter the section.
    pWrtShell->Down(/*bSelect=*/false);
    CPPUNIT_ASSERT(pWrtShell->IsDirectlyInSection());

    // Assert that we get the right paragraph object.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xController(xModel->getCurrentController(),
                                                              uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xViewCursor = xController->getViewCursor();
    // This failed as there were no TextParagraph property.
    auto xParagraph
        = getProperty<uno::Reference<text::XTextRange>>(xViewCursor->getStart(), "TextParagraph");
    CPPUNIT_ASSERT_EQUAL(OUString("In section"), xParagraph->getString());
}

void SwUiWriterTest4::testTdf99689TableOfContents()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf99689.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoNextTOXBase();
    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwCursorShell* pShell(pDoc->GetEditShell());
    SwTextNode* pTitleNode = pShell->GetCursor()->GetNode().GetTextNode();
    SwNodeIndex aIdx(*pTitleNode);
    // skip the title
    pDoc->GetNodes().GoNext(&aIdx);

    // skip the first header. No attributes there.
    // next node should contain superscript
    SwTextNode* pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT(pNext->HasHints());
    sal_uInt16 nAttrType = lcl_getAttributeIDFromHints(pNext->GetSwpHints());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_CHRATR_ESCAPEMENT), nAttrType);

    // next node should contain subscript
    pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT(pNext->HasHints());
    nAttrType = lcl_getAttributeIDFromHints(pNext->GetSwpHints());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_CHRATR_ESCAPEMENT), nAttrType);
}

void SwUiWriterTest4::testTdf99689TableOfFigures()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf99689_figures.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoNextTOXBase();
    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwCursorShell* pShell(pDoc->GetEditShell());
    SwTextNode* pTitleNode = pShell->GetCursor()->GetNode().GetTextNode();
    SwNodeIndex aIdx(*pTitleNode);

    // skip the title
    // next node should contain subscript
    SwTextNode* pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT(pNext->HasHints());
    sal_uInt16 nAttrType = lcl_getAttributeIDFromHints(pNext->GetSwpHints());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_CHRATR_ESCAPEMENT), nAttrType);

    // next node should contain superscript
    pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT(pNext->HasHints());
    nAttrType = lcl_getAttributeIDFromHints(pNext->GetSwpHints());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_CHRATR_ESCAPEMENT), nAttrType);
}

void SwUiWriterTest4::testTdf99689TableOfTables()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf99689_tables.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoNextTOXBase();
    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwCursorShell* pShell(pDoc->GetEditShell());
    SwTextNode* pTitleNode = pShell->GetCursor()->GetNode().GetTextNode();
    SwNodeIndex aIdx(*pTitleNode);

    // skip the title
    // next node should contain superscript
    SwTextNode* pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT(pNext->HasHints());
    sal_uInt16 nAttrType = lcl_getAttributeIDFromHints(pNext->GetSwpHints());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_CHRATR_ESCAPEMENT), nAttrType);

    // next node should contain subscript
    pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT(pNext->HasHints());
    nAttrType = lcl_getAttributeIDFromHints(pNext->GetSwpHints());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_CHRATR_ESCAPEMENT), nAttrType);
}

// tdf#112448: Fix: take correct line height
//
// When line metrics is not calculated we need to call CalcRealHeight()
// before usage of the Height() and GetRealHeight().
void SwUiWriterTest4::testTdf112448()
{
    createSwDoc(DATA_DIRECTORY, "tdf112448.odt");

    // check actual number of line breaks in the paragraph
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/txt/LineBreak", 2);
}

void SwUiWriterTest4::testTdf113790()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf113790.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Create the clipboard document.
    SwDoc aClipboard;
    aClipboard.SetClipBoard(true);

    // Go to fourth line - to "ABCD" bulleted list item
    pWrtShell->Down(/*bSelect=*/false, 4);
    pWrtShell->SelPara(nullptr);
    CPPUNIT_ASSERT_EQUAL(OUString("ABCD"), pWrtShell->GetSelText());
    pWrtShell->Copy(aClipboard);

    // Go down to next-to-last (empty) line above "Title3"
    pWrtShell->Down(/*bSelect=*/false, 4);
    pWrtShell->Paste(aClipboard);

    // Save it as DOCX & load it again
    reload("Office Open XML Text", "tdf113790.docx");
    CPPUNIT_ASSERT(dynamic_cast<SwXTextDocument*>(mxComponent.get()));
}

void SwUiWriterTest4::testTdf108048()
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Kind", uno::makeAny(sal_Int16(3)) },
        { "TemplateName", uno::makeAny(OUString("Default Page Style")) },
        { "PageNumber",
          uno::makeAny(sal_uInt16(6)) }, // Even number to avoid auto-inserted blank page
        { "PageNumberFilled", uno::makeAny(true) },
    });
    dispatchCommand(mxComponent, ".uno:InsertBreak", aPropertyValues);
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // The inserted page must have page number set to 6
    uno::Reference<text::XTextRange> xPara = getParagraph(2);
    sal_uInt16 nPageNumber = getProperty<sal_uInt16>(xPara, "PageNumberOffset");
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), nPageNumber);
}

void SwUiWriterTest4::testTdf113481()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf113481-IVS.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // One backspace should completely remove the CJK ideograph variation sequence
    pWrtShell->EndPara();
    // Before: U+8FBA U+E0102. After: empty
    pWrtShell->DelLeft();
    const uno::Reference<text::XTextRange> xPara1 = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xPara1->getString().getLength());

    // In case that weak script is treated as CJK script, remove one character.
    pWrtShell->Down(false);
    pWrtShell->EndPara();
    // Before: U+4E2D U+2205 U+FE00. After: U+4E2D U+2205
    if (pWrtShell->GetScriptType() == SvtScriptType::ASIAN)
    {
        pWrtShell->DelLeft();
        const uno::Reference<text::XTextRange> xPara2 = getParagraph(2);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xPara2->getString().getLength());
        CPPUNIT_ASSERT_EQUAL(u'\x2205', xPara2->getString()[1]);
    }

    // Characters of other scripts, remove one character.
    pWrtShell->Down(false);
    pWrtShell->EndPara();
    // Before: U+1820 U+180B. After: U+1820
    pWrtShell->DelLeft();
    const uno::Reference<text::XTextRange> xPara3 = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPara3->getString().getLength());
    CPPUNIT_ASSERT_EQUAL(u'\x1820', xPara3->getString()[0]);
}

void SwUiWriterTest4::testTdf115013()
{
    const OUString sColumnName("Name with spaces, \"quotes\" and \\backslashes");

    utl::TempFile aTempDir(nullptr, true);
    aTempDir.EnableKillingFile();
    const OUString aWorkDir = aTempDir.GetURL();

    //create new writer document
    SwDoc* pDoc = createSwDoc();

    {
        // Load and register data source
        const OUString aDataSourceURI(m_directories.getURLFromSrc(DATA_DIRECTORY)
                                      + "datasource.ods");
        OUString sDataSource = SwDBManager::LoadAndRegisterDataSource(aDataSourceURI, &aWorkDir);
        CPPUNIT_ASSERT(!sDataSource.isEmpty());

        // Insert a new field type for the mailmerge field
        SwDBData aDBData;
        aDBData.sDataSource = sDataSource;
        aDBData.sCommand = "Sheet1";
        SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
        CPPUNIT_ASSERT(pWrtShell);
        SwDBFieldType* pFieldType = static_cast<SwDBFieldType*>(
            pWrtShell->InsertFieldType(SwDBFieldType(pDoc, sColumnName, aDBData)));
        CPPUNIT_ASSERT(pFieldType);

        // Insert the field into document
        SwDBField aField(pFieldType);
        pWrtShell->InsertField2(aField);
    }
    // Save it as DOCX & load it again
    reload("Office Open XML Text", "mm-field.docx");

    auto pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pDoc = pXTextDocument->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    CPPUNIT_ASSERT(pCursor);

    // Get the field at the beginning of the document
    SwDBField* pField = dynamic_cast<SwDBField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT(pField);
    OUString sColumn = static_cast<SwDBFieldType*>(pField->GetTyp())->GetColumnName();
    // The column name must come correct after round trip
    CPPUNIT_ASSERT_EQUAL(sColumnName, sColumn);
}

void SwUiWriterTest4::testTdf115065()
{
    // In the document, the tables have table style assigned
    // Source table (first one) has two rows;
    // destination (second one) has only one row
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf115065.odt");
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    pWrtShell->GotoTable("Table2");
    SwRect aRect = pWrtShell->GetCurrFrame()->getFrameArea();
    // Destination point is the middle of the first cell of second table
    Point ptTo(aRect.Left() + aRect.Width() / 2, aRect.Top() + aRect.Height() / 2);

    pWrtShell->GotoTable("Table1");
    aRect = pWrtShell->GetCurrFrame()->getFrameArea();
    // Source point is the middle of the first cell of first table
    Point ptFrom(aRect.Left() + aRect.Width() / 2, aRect.Top() + aRect.Height() / 2);

    pWrtShell->SelTableCol();
    // The copy operation (or closing document after that) segfaulted
    pWrtShell->Copy(*pWrtShell, ptFrom, ptTo);
}

void SwUiWriterTest4::testTdf115132()
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    std::vector<OUString> vTestTableNames;

    // Create an empty paragraph that will separate first table from the rest
    pWrtShell->SplitNode();
    pWrtShell->StartOfSection();
    // Create a table at the start of document body
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    const SwTable* pTable = &pWrtShell->InsertTable(TableOpt, 2, 3);
    const SwTableFormat* pFormat = pTable->GetFrameFormat();
    CPPUNIT_ASSERT(pFormat);
    vTestTableNames.push_back(pFormat->GetName());
    pWrtShell->EndOfSection();
    // Create a table after a paragraph
    pTable = &pWrtShell->InsertTable(TableOpt, 2, 3);
    pFormat = pTable->GetFrameFormat();
    CPPUNIT_ASSERT(pFormat);
    vTestTableNames.push_back(pFormat->GetName());
    // Create a table immediately after the previous
    pTable = &pWrtShell->InsertTable(TableOpt, 2, 3);
    pFormat = pTable->GetFrameFormat();
    CPPUNIT_ASSERT(pFormat);
    vTestTableNames.push_back(pFormat->GetName());
    // Create a nested table in the middle of last row
    pWrtShell->GotoTable(vTestTableNames.back());
    for (int i = 0; i < 4; ++i)
        pWrtShell->GoNextCell(false);
    pTable = &pWrtShell->InsertTable(TableOpt, 2, 3);
    pFormat = pTable->GetFrameFormat();
    CPPUNIT_ASSERT(pFormat);
    vTestTableNames.push_back(pFormat->GetName());

    // Now check that in any cell in all tables we don't go out of a cell
    // using Delete or Backspace. We test cases when a table is the first node;
    // when we are in a first/middle/last cell in a row; when there's a paragraph
    // before/after this cell; when there's another table before/after this cell;
    // in nested table.
    for (const auto& rTableName : vTestTableNames)
    {
        pWrtShell->GotoTable(rTableName);
        do
        {
            const SwStartNode* pNd = pWrtShell->GetCursor()->GetNode().FindTableBoxStartNode();
            pWrtShell->DelRight();
            CPPUNIT_ASSERT_EQUAL(pNd, pWrtShell->GetCursor()->GetNode().FindTableBoxStartNode());
            pWrtShell->DelLeft();
            CPPUNIT_ASSERT_EQUAL(pNd, pWrtShell->GetCursor()->GetNode().FindTableBoxStartNode());
        } while (pWrtShell->GoNextCell(false));
    }
}

void SwUiWriterTest4::testXDrawPagesSupplier()
{
    createSwDoc();
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("XDrawPagesSupplier interface is unavailable", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    CPPUNIT_ASSERT(xDrawPages.is());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There must be only a single DrawPage in Writer documents",
                                 sal_Int32(1), xDrawPages->getCount());
    uno::Any aDrawPage = xDrawPages->getByIndex(0);
    uno::Reference<drawing::XDrawPage> xDrawPageFromXDrawPages(aDrawPage, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDrawPageFromXDrawPages.is());

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "The DrawPage accessed using XDrawPages must be the same as using XDrawPageSupplier",
        xDrawPage.get(), xDrawPageFromXDrawPages.get());
}

void SwUiWriterTest4::testTdf116403()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf116403-considerborders.odt");
    // Check that before ToX update, the tab stop position is the old one
    uno::Reference<text::XTextRange> xParagraph = getParagraph(2, "1\t1");
    auto aTabs = getProperty<uno::Sequence<style::TabStop>>(xParagraph, "ParaTabStops");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aTabs.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(17000), aTabs[0].Position);

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    const SwTOXBase* pTOX = pWrtShell->GetTOX(0);
    CPPUNIT_ASSERT(pTOX);
    pWrtShell->UpdateTableOf(*pTOX);

    xParagraph = getParagraph(2, "1\t1");
    aTabs = getProperty<uno::Sequence<style::TabStop>>(xParagraph, "ParaTabStops");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aTabs.getLength());
    // This was still 17000, refreshing ToX didn't take borders spacings and widths into account
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Page borders must be considered for right-aligned tabstop",
                                 static_cast<sal_Int32>(17000 - 2 * 500 - 2 * 1),
                                 aTabs[0].Position);
}

void SwUiWriterTest4::testHtmlCopyImages()
{
    // Load a document with an image.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "image.odt");

    // Trigger the copy part of HTML copy&paste.
    WriterRef xWrt = new SwHTMLWriter(/*rBaseURL=*/OUString());
    CPPUNIT_ASSERT(xWrt.is());

    xWrt->m_bWriteClipboardDoc = true;
    xWrt->m_bWriteOnlyFirstTable = false;
    xWrt->SetShowProgress(false);
    {
        SvFileStream aStream(maTempFile.GetURL(), StreamMode::WRITE | StreamMode::TRUNC);
        SwWriter aWrt(aStream, *pDoc);
        aWrt.Write(xWrt);
    }
    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pHtmlDoc);

    // This failed, image was lost during HTML copy.
    OUString aImage = getXPath(pHtmlDoc, "/html/body/p/img", "src");
    // Also make sure that the image is not embedded (e.g. Word doesn't handle
    // embedded images).
    CPPUNIT_ASSERT(aImage.startsWith("file:///"));
}

void SwUiWriterTest4::testTdf116789()
{
    createSwDoc(DATA_DIRECTORY, "tdf116789.fodt");
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText1;
    uno::Reference<text::XText> xText2;
    {
        uno::Reference<text::XTextContent> xBookmark(
            xBookmarksSupplier->getBookmarks()->getByName("Bookmark 1"), uno::UNO_QUERY);
        xText1 = xBookmark->getAnchor()->getText();
    }
    {
        uno::Reference<text::XTextContent> xBookmark(
            xBookmarksSupplier->getBookmarks()->getByName("Bookmark 1"), uno::UNO_QUERY);
        xText2 = xBookmark->getAnchor()->getText();
    }
    // This failed, we got two different SwXCell for the same bookmark anchor text.
    CPPUNIT_ASSERT_EQUAL(xText1, xText2);
}

void SwUiWriterTest4::testTdf91801()
{
    // Tests calculation with several user field variables without prior user fields
    createSwDoc(DATA_DIRECTORY, "tdf91801.fodt");
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell(xTable->getCellByName("A1"));
    CPPUNIT_ASSERT_EQUAL(555.0, xCell->getValue());
}

void SwUiWriterTest4::testTdf51223()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    pWrtShell->Insert("i");
    pWrtShell->SplitNode(true);
    CPPUNIT_ASSERT_EQUAL(OUString("I"),
                         static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(OUString("i"),
                         static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testFontEmbedding()
{
#if HAVE_MORE_FONTS && !defined(MACOSX)
    createSwDoc(DATA_DIRECTORY, "testFontEmbedding.odt");

    OString aContentBaseXpath("/office:document-content/office:font-face-decls");
    OString aSettingsBaseXpath("/office:document-settings/office:settings/config:config-item-set");

    xmlDocUniquePtr pXmlDoc;
    uno::Sequence<beans::PropertyValue> aDescriptor;
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();

    // Get document settings
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xProps(
        xFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY_THROW);

    // Check font embedding state
    CPPUNIT_ASSERT_EQUAL(false, xProps->getPropertyValue("EmbedFonts").get<bool>());
    CPPUNIT_ASSERT_EQUAL(false, xProps->getPropertyValue("EmbedOnlyUsedFonts").get<bool>());
    // Font scripts should be enabled by default, however this has no effect unless "EmbedOnlyUsedFonts" is enabled
    CPPUNIT_ASSERT_EQUAL(true, xProps->getPropertyValue("EmbedLatinScriptFonts").get<bool>());
    CPPUNIT_ASSERT_EQUAL(true, xProps->getPropertyValue("EmbedAsianScriptFonts").get<bool>());
    CPPUNIT_ASSERT_EQUAL(true, xProps->getPropertyValue("EmbedComplexScriptFonts").get<bool>());

    // CASE 1 - no font embedding enabled

    // Save the document
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    CPPUNIT_ASSERT(aTempFile.IsValid());

    // Check setting - No font embedding should be enabled
    pXmlDoc = parseExportInternal(aTempFile.GetURL(), "settings.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedFonts']", "false");

    // Check content - No font-face-src nodes should be present
    pXmlDoc = parseExportInternal(aTempFile.GetURL(), "content.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face", 6);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans']");
    assertXPath(
        pXmlDoc,
        aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans']/svg:font-face-src", 0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans1']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Sans1']/svg:font-face-src",
                0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Serif']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Serif']/svg:font-face-src",
                0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Serif1']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Serif1']/svg:font-face-src",
                0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Carlito']");
    assertXPath(pXmlDoc,
                aContentBaseXpath + "/style:font-face[@style:name='Carlito']/svg:font-face-src", 0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Caladea']");
    assertXPath(pXmlDoc,
                aContentBaseXpath + "/style:font-face[@style:name='Caladea']/svg:font-face-src", 0);

    // CASE 2 - font embedding enabled, but embed used fonts disabled

    // Enable font embedding, disable embedding used font only
    xProps->setPropertyValue("EmbedFonts", uno::makeAny(true));
    xProps->setPropertyValue("EmbedOnlyUsedFonts", uno::makeAny(false));

    // Save the document again
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    CPPUNIT_ASSERT(aTempFile.IsValid());

    // Check setting - font embedding should be enabled + embed only used fonts and scripts
    pXmlDoc = parseExportInternal(aTempFile.GetURL(), "settings.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedFonts']", "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedOnlyUsedFonts']",
        "false");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedLatinScriptFonts']",
        "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedAsianScriptFonts']",
        "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedComplexScriptFonts']",
        "true");

    // Check content - font-face-src should be present only for "Liberation Sans" fonts

    pXmlDoc = parseExportInternal(aTempFile.GetURL(), "content.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face", 6);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans']");
    assertXPath(
        pXmlDoc,
        aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans']/svg:font-face-src", 1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans1']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Sans1']/svg:font-face-src",
                1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Serif']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Serif']/svg:font-face-src",
                1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Serif1']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Serif1']/svg:font-face-src",
                1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Carlito']");
    assertXPath(pXmlDoc,
                aContentBaseXpath + "/style:font-face[@style:name='Carlito']/svg:font-face-src", 1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Caladea']");
    assertXPath(pXmlDoc,
                aContentBaseXpath + "/style:font-face[@style:name='Caladea']/svg:font-face-src", 1);

    // CASE 3 - font embedding enabled, embed only used fonts enabled

    // Enable font embedding and setting to embed used fonts only
    xProps->setPropertyValue("EmbedFonts", uno::makeAny(true));
    xProps->setPropertyValue("EmbedOnlyUsedFonts", uno::makeAny(true));
    xProps->setPropertyValue("EmbedLatinScriptFonts", uno::makeAny(true));
    xProps->setPropertyValue("EmbedAsianScriptFonts", uno::makeAny(true));
    xProps->setPropertyValue("EmbedComplexScriptFonts", uno::makeAny(true));

    // Save the document again
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    CPPUNIT_ASSERT(aTempFile.IsValid());

    // Check setting - font embedding should be enabled + embed only used fonts and scripts
    pXmlDoc = parseExportInternal(aTempFile.GetURL(), "settings.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedFonts']", "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedOnlyUsedFonts']",
        "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedLatinScriptFonts']",
        "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedAsianScriptFonts']",
        "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedComplexScriptFonts']",
        "true");

    // Check content - font-face-src should be present only for "Liberation Sans" fonts

    pXmlDoc = parseExportInternal(aTempFile.GetURL(), "content.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face", 6);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans']");
    assertXPath(
        pXmlDoc,
        aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans']/svg:font-face-src", 0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans1']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Sans1']/svg:font-face-src",
                0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Serif']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Serif']/svg:font-face-src",
                1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Serif1']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Serif1']/svg:font-face-src",
                1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Carlito']");
    assertXPath(pXmlDoc,
                aContentBaseXpath + "/style:font-face[@style:name='Carlito']/svg:font-face-src", 1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Caladea']");
    assertXPath(pXmlDoc,
                aContentBaseXpath + "/style:font-face[@style:name='Caladea']/svg:font-face-src", 0);
#endif
}

// Unit test for fix inconsistent bookmark behavior around at-char/as-char anchored frames
//
// We have a placeholder character in the sw doc model for as-char anchored frames,
// so it's possible to have a bookmark before/after the frame or a non-collapsed bookmark
// which covers the frame. The same is not true for at-char anchored frames,
// where the anchor points to a doc model position, but there is no placeholder character.
// If a bookmark is created covering the start and end of the anchor of the frame,
// internally we create a collapsed bookmark which has the same position as the anchor of the frame.
// When this doc model is handled by SwXParagraph::createEnumeration(),
// first the frame and then the bookmark is appended to the text portion enumeration,
// so your bookmark around the frame is turned into a collapsed bookmark after the frame.
// (The same happens when we roundtrip an ODT document representing this doc model.)
//
// Fix the problem by inserting collapsed bookmarks with affected anchor positions
// (same position is the anchor for an at-char frame) into the enumeration in two stages:
// first the start of them before frames and then the end of them + other bookmarks.
// This way UNO API users get their non-collapsed bookmarks around at-char anchored frames,
// similar to as-char ones.
void SwUiWriterTest4::testInconsistentBookmark()
{
    // create test document with text and bookmark
    {
        SwDoc* pDoc(createSwDoc(DATA_DIRECTORY, "testInconsistentBookmark.ott"));
        IDocumentMarkAccess& rIDMA(*pDoc->getIDocumentMarkAccess());
        SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
        SwCursor aPaM(SwPosition(aIdx), nullptr);
        aPaM.SetMark();
        aPaM.MovePara(GoCurrPara, fnParaStart);
        aPaM.MovePara(GoCurrPara, fnParaEnd);
        rIDMA.makeMark(aPaM, "Mark", IDocumentMarkAccess::MarkType::BOOKMARK,
                       ::sw::mark::InsertMode::New);
        aPaM.Exchange();
        aPaM.DeleteMark();
    }

    // save document and verify the bookmark scoup
    {
        // save document
        utl::TempFile aTempFile;
        save("writer8", aTempFile);

        // load only content.xml
        if (xmlDocUniquePtr pXmlDoc = parseExportInternal(aTempFile.GetURL(), "content.xml"))
        {
            const OString aPath("/office:document-content/office:body/office:text/text:p");

            const int pos1 = getXPathPosition(pXmlDoc, aPath, "bookmark-start");
            const int pos2 = getXPathPosition(pXmlDoc, aPath, "control");
            const int pos3 = getXPathPosition(pXmlDoc, aPath, "bookmark-end");

            CPPUNIT_ASSERT_GREATER(pos1, pos2);
            CPPUNIT_ASSERT_GREATER(pos2, pos3);
        }
    }
}

void SwUiWriterTest4::testSpellOnlineParameter()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    const SwViewOption* pOpt = pWrtShell->GetViewOptions();
    bool bSet = pOpt->IsOnlineSpell();

    uno::Sequence<beans::PropertyValue> params
        = comphelper::InitPropertySequence({ { "Enable", uno::makeAny(!bSet) } });
    dispatchCommand(mxComponent, ".uno:SpellOnline", params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pOpt->IsOnlineSpell());

    // set the same state as now and we don't expect any change (no-toggle)
    params = comphelper::InitPropertySequence({ { "Enable", uno::makeAny(!bSet) } });
    dispatchCommand(mxComponent, ".uno:SpellOnline", params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pOpt->IsOnlineSpell());
}

void SwUiWriterTest4::testRedlineAutoCorrect()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "redline-autocorrect.fodt");

    dispatchCommand(mxComponent, ".uno:GoToEndOfDoc", {});

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // show tracked deletion with enabled change tracking
    RedlineFlags const nMode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(nMode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(nMode);
    CPPUNIT_ASSERT(nMode & RedlineFlags::ShowDelete);

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    pWrtShell->AutoCorrect(corr, ' ');
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();

    // tdf#83419 This was "Ts " removing the deletion of "t" silently by sentence capitalization
    OUString sReplaced("ts ");
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());

    // hide delete redlines
    pWrtShell->SetRedlineFlags(nMode & ~RedlineFlags::ShowDelete);

    // repeat it with not visible redlining
    dispatchCommand(mxComponent, ".uno:Undo", {});

    pWrtShell->AutoCorrect(corr, ' ');
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();

    sReplaced = "S ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());

    // show delete redlines
    pWrtShell->SetRedlineFlags(nMode);
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();

    // This still keep the tracked deletion, capitalize only the visible text "s"
    // with tracked deletion of the original character
    sReplaced = "tsS ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());

    // repeat it with visible redlining and word auto replacement of "tset"
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});

    pWrtShell->Insert("et");
    pWrtShell->AutoCorrect(corr, ' ');
    // This was "Ttest" removing the tracked deletion silently.
    // Don't replace, if a redline starts or ends within the text.
    sReplaced = "tset ";
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());

    // Otherwise replace it
    pWrtShell->Insert("tset");
    pWrtShell->AutoCorrect(corr, ' ');
    sReplaced = "tset test ";
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());

    // Including capitalization
    pWrtShell->Insert("end. word");
    pWrtShell->AutoCorrect(corr, ' ');
    sReplaced = "tset test end. Word ";
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());

    // tracked deletions after the correction point doesn't affect autocorrect
    dispatchCommand(mxComponent, ".uno:GoToStartOfDoc", {});
    pWrtShell->Insert("a");
    pWrtShell->AutoCorrect(corr, ' ');
    sReplaced = "A tset test end. Word ";
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testRedlineAutoCorrect2()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "redline-autocorrect2.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    dispatchCommand(mxComponent, ".uno:GoToEndOfDoc", {});

    // show tracked deletion
    RedlineFlags const nMode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(nMode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(nMode);
    CPPUNIT_ASSERT(nMode & RedlineFlags::ShowDelete);

    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    pWrtShell->Insert("...");
    pWrtShell->AutoCorrect(corr, ' ');
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();

    // This was "LoremLorem,…," (duplicating the deleted comma, but without deletion)
    // Don't replace, if a redline starts or ends within the text.
    OUString sReplaced = "Lorem,... ";
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());

    // Continue it:
    pWrtShell->Insert("Lorem,...");
    pWrtShell->AutoCorrect(corr, ' ');
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    sReplaced = u"Lorem,... Lorem,… ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testEmojiAutoCorrect()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "redline-autocorrect2.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Emoji replacement (:snowman: -> ☃)

    // without change tracking
    CPPUNIT_ASSERT(!(pWrtShell->GetRedlineFlags() & RedlineFlags::On));
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    pWrtShell->Insert(":snowman");
    pWrtShell->AutoCorrect(corr, ':');
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    OUString sReplaced = u"☃Lorem,";
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());

    // with change tracking (showing redlines)
    RedlineFlags const nMode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(nMode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(nMode);
    CPPUNIT_ASSERT(nMode & RedlineFlags::On);
    CPPUNIT_ASSERT(nMode & RedlineFlags::ShowDelete);

    pWrtShell->Insert(":snowman");
    pWrtShell->AutoCorrect(corr, ':');
    sReplaced = u"☃☃Lorem,";
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();

    // tdf#140674 This was ":snowman:" instead of autocorrect
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testTdf108423()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // testing autocorrect of i' -> I' on start of first paragraph
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    pWrtShell->Insert("i");
    const sal_Unicode cChar = '\'';
    pWrtShell->AutoCorrect(corr, cChar);
    // The word "i" should be capitalized due to autocorrect, followed by a typographical apostrophe
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    OUString sIApostrophe(u"I\u2019");
    CPPUNIT_ASSERT_EQUAL(sIApostrophe,
                         static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    pWrtShell->Insert(" i");
    pWrtShell->AutoCorrect(corr, cChar);
    OUString sText(sIApostrophe + u" " + sIApostrophe);
    CPPUNIT_ASSERT_EQUAL(sText, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testTdf106164()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // testing autocorrect of we're -> We're on start of first paragraph
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    pWrtShell->Insert(u"we\u2019re");
    const sal_Unicode cChar = ' ';
    pWrtShell->AutoCorrect(corr, cChar);
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(OUString(u"We\u2019re "),
                         static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testTdf54409()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // testing autocorrect of "tset -> "test with typographical double quotation mark U+201C
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    pWrtShell->Insert(u"\u201Ctset");
    const sal_Unicode cChar = ' ';
    pWrtShell->AutoCorrect(corr, cChar);
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    OUString sReplaced(u"\u201Ctest ");
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // testing autocorrect of test" -> test" with typographical double quotation mark U+201D
    pWrtShell->Insert(u"and tset\u201D");
    pWrtShell->AutoCorrect(corr, cChar);
    OUString sReplaced2(sReplaced + u"and test\u201D ");
    CPPUNIT_ASSERT_EQUAL(sReplaced2, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // testing autocorrect of "tset" -> "test" with typographical double quotation mark U+201C and U+201D
    pWrtShell->Insert(u"\u201Ctset\u201D");
    pWrtShell->AutoCorrect(corr, cChar);
    OUString sReplaced3(sReplaced2 + u"\u201Ctest\u201D ");
    CPPUNIT_ASSERT_EQUAL(sReplaced3, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testTdf38394()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf38394.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // testing autocorrect of French l'" -> l'« (instead of l'»)
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    pWrtShell->Insert(u"l\u2019");
    const sal_Unicode cChar = '"';
    pWrtShell->AutoCorrect(corr, cChar);
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    OUString sReplaced(u"l\u2019« ");
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // tdf#132301 autocorrect of qu'«
    pWrtShell->Insert(u" qu\u2019");
    pWrtShell->AutoCorrect(corr, cChar);
    sReplaced += u" qu\u2019« ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testTdf59666()
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // testing missing autocorrect of single Greek letters
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    pWrtShell->Insert(u"\u03C0");
    const sal_Unicode cChar = ' ';
    pWrtShell->AutoCorrect(corr, cChar);
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(OUString(u"\u03C0 "),
                         static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testTdf133524()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf133524.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // 1. Testing autocorrect of >> and <<
    // Example: »word«
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    // >>
    pWrtShell->Insert(u">");
    pWrtShell->AutoCorrect(corr, '>');
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    OUString sReplaced(u"»");
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // <<
    pWrtShell->Insert(u"word<");
    pWrtShell->AutoCorrect(corr, '<');
    sReplaced += u"word«";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // 2. Testing autocorrect of " to >> and << inside „...”
    // Example: „Sentence and »word«.”
    // opening primary level quote
    pWrtShell->Insert(u" ");
    pWrtShell->AutoCorrect(corr, '"');
    sReplaced += u" „";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // opening second level quote
    pWrtShell->Insert(u"Sentence and ");
    pWrtShell->AutoCorrect(corr, '"');
    sReplaced += u"Sentence and »";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // closing second level quote
    pWrtShell->Insert(u"word");
    pWrtShell->AutoCorrect(corr, '"');
    sReplaced += u"word«";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // closing primary level quote
    pWrtShell->Insert(u".");
    pWrtShell->AutoCorrect(corr, '"');
    sReplaced += u".”";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // tdf#134940 avoid premature replacement of "--" in "-->"
    pWrtShell->Insert(u" --");
    pWrtShell->AutoCorrect(corr, '>');
    OUString sReplaced2(sReplaced + u" -->");
    // This was "–>" instead of "-->"
    CPPUNIT_ASSERT_EQUAL(sReplaced2, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    pWrtShell->AutoCorrect(corr, ' ');
    sReplaced += u" → ";
    // This was "–>" instead of "→"
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testTdf133524_Romanian()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf133524_ro.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    // 1. Testing autocorrect of " to << and >> inside „...”
    // Example: „Sentence and «word».”
    // opening primary level quote
    pWrtShell->AutoCorrect(corr, '"');
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    OUString sReplaced(u"„");
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // opening second level quote
    pWrtShell->Insert(u"Sentence and ");
    pWrtShell->AutoCorrect(corr, '"');
    sReplaced += u"Sentence and «";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // closing second level quote
    pWrtShell->Insert(u"word");
    pWrtShell->AutoCorrect(corr, '"');
    sReplaced += u"word»";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // closing primary level quote
    pWrtShell->Insert(u".");
    pWrtShell->AutoCorrect(corr, '"');
    sReplaced += u".”";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // 2. Testing recognition of closing double quotation mark ”
    pWrtShell->Insert(u" ");
    pWrtShell->AutoCorrect(corr, '"');
    sReplaced += u" „";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // 3. Testing recognition of alternative closing double quotation mark “
    pWrtShell->Insert(u"Alternative.“ ");
    pWrtShell->AutoCorrect(corr, '"');
    sReplaced += u"Alternative.“ „";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testTdf128860()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf128860.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Second level ending quote: ‚word' -> ,word‘
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    pWrtShell->Insert(u"‚word");
    pWrtShell->AutoCorrect(corr, '\'');
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    OUString sReplaced(u"‚word‘");
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // Us apostrophe without preceding starting quote: word' -> word’
    pWrtShell->Insert(u" word");
    pWrtShell->AutoCorrect(corr, '\'');
    sReplaced += u" word’";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // But only after letters: word.' -> word.‘
    pWrtShell->Insert(u" word.");
    pWrtShell->AutoCorrect(corr, '\'');
    sReplaced += u" word.‘";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testTdf123786()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf123786.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Second level ending quote: „word' -> „word“
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    pWrtShell->Insert(u"„слово");
    pWrtShell->AutoCorrect(corr, '\'');
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    OUString sReplaced(u"„слово“");
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // Us apostrophe without preceding starting quote: word' -> word’
    pWrtShell->Insert(u" слово");
    pWrtShell->AutoCorrect(corr, '\'');
    sReplaced += u" слово’";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // But only after letters: word.' -> word.“
    pWrtShell->Insert(u" слово.");
    pWrtShell->AutoCorrect(corr, '\'');
    sReplaced += u" слово.“";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testTdf133589()
{
    // Hungarian test document with right-to-left paragraph setting
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf133589.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // translitere words to Old Hungarian
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    pWrtShell->Insert(u"székely");
    pWrtShell->AutoCorrect(corr, ' ');
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    OUString sReplaced(u"𐳥𐳋𐳓𐳉𐳗 ");
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // disambiguate consonants: asszony -> asz|szony
    pWrtShell->Insert(u"asszony");
    pWrtShell->AutoCorrect(corr, ' ');
    sReplaced += u"𐳀𐳥𐳥𐳛𐳚 ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // disambiguate consonants: kosszarv -> kos|szarv
    // (add explicit ZWSP temporarily for consonant disambiguation, because the requested
    // hu_HU hyphenation dictionary isn't installed on all testing platform)
    // pWrtShell->Insert(u"kosszarv");
    pWrtShell->Insert(u"kos​szarv");
    pWrtShell->AutoCorrect(corr, ' ');
    sReplaced += u"𐳓𐳛𐳤𐳥𐳀𐳢𐳮 ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    // transliterate numbers to Old Hungarian
    pWrtShell->Insert(u"2020");
    pWrtShell->AutoCorrect(corr, ' ');
    sReplaced += u"𐳺𐳺𐳿𐳼𐳼 ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

void SwUiWriterTest4::testTdf143176()
{
    // Hungarian test document with right-to-left paragraph setting
    createSwDoc(DATA_DIRECTORY, "tdf143176.fodt");

    // transliterate the document to Old Hungarian (note: it only works
    // with right-to-left text direction and Default Paragraph Style)
    dispatchCommand(mxComponent, ".uno:AutoFormatApply", {});

    // This was the original "Lorem ipsum..."
    CPPUNIT_ASSERT_EQUAL(OUString(u"𐲖𐳛𐳢𐳉𐳘 𐳐𐳠𐳤𐳪𐳘 𐳇𐳛𐳖𐳛𐳢 "
                                  u"𐳤𐳐𐳦 𐳀𐳘𐳉𐳦⹁"),
                         getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(u"𐳄𐳛𐳙𐳤𐳉𐳄𐳦𐳉𐳦𐳪𐳢 "
                                  u"𐳀𐳇𐳐𐳠𐳐𐳤𐳄𐳐𐳙𐳍 𐳉𐳖𐳐𐳦."),
                         getParagraph(2)->getString());
}

void SwUiWriterTest4::testInsertLongDateFormat()
{
    // only for Hungarian, yet
    createSwDoc(DATA_DIRECTORY, "tdf133524.fodt");
    dispatchCommand(mxComponent, ".uno:InsertDateField", {});
    // Make sure that the document starts with a field now, and its expanded string value contains space
    const uno::Reference<text::XTextRange> xField = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(xField, "TextPortionType"));
    // the date format was "YYYY-MM-DD", but now "YYYY. MMM DD."
    CPPUNIT_ASSERT(xField->getString().indexOf(" ") > -1);
}

void SwUiWriterTest4::testTdf129270()
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf129270.odt");
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);

    // Go to document end
    pWrtShell->SttEndDoc(/*bStt=*/false);

    // Press enter
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // Numbering for previous outline should remain the same "2"
    CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(getParagraph(4), "ListLabelString"));

    // Numbering for newly created outline should be "2.1"
    CPPUNIT_ASSERT_EQUAL(OUString("2.1"),
                         getProperty<OUString>(getParagraph(5), "ListLabelString"));
}

void SwUiWriterTest4::testInsertPdf()
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    createSwDoc();

    // insert the PDF into the document
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "FileName",
            uno::Any(m_directories.getURLFromSrc(DATA_DIRECTORY) + "hello-world.pdf") } }));
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);

    // Save and load cycle
    reload("writer8", "testInsertPdf.odt");

    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Assert that we have a replacement graphics
    auto xReplacementGraphic
        = getProperty<uno::Reference<graphic::XGraphic>>(xShape, "ReplacementGraphic");
    CPPUNIT_ASSERT(xReplacementGraphic.is());

    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(xShape, "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
    // Assert that the graphic is a PDF
    CPPUNIT_ASSERT_EQUAL(OUString("application/pdf"), getProperty<OUString>(xGraphic, "MimeType"));
}

void SwUiWriterTest4::testTdf143760WrapContourToOff()
{
    // Actually, this is an ooxmlexport test. It is here because here is a ready environment
    // to change a shape by dispatchCommand.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf143760_ContourToWrapOff.docx");
    CPPUNIT_ASSERT(pDoc);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "SurroundContour"));

    // Mark the object
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObject);
    SdrView* pView = pWrtShell->GetDrawView();
    pView->MarkObj(pObject, pView->GetSdrPageView());

    // Set "wrap off"
    dispatchCommand(mxComponent, ".uno:WrapOff", {});
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(1), "SurroundContour"));

    // Without fix this had failed, because the shape was written to file with contour.
    reload("Office Open XML Text", "tdf143760_ContourToWrapOff.docx");
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(1), "SurroundContour"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest4);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
